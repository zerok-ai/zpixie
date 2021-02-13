#include <algorithm>
#include <memory>
#include <queue>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/carnot/planner/distributed/distributed_coordinator.h"
#include "src/carnot/planner/distributed/distributed_splitter.h"
#include "src/carnot/planner/distributed/distributed_stitcher_rules.h"
#include "src/carnot/planner/distributed/grpc_source_conversion.h"
#include "src/carnot/planner/distributed/plan_clusters.h"
#include "src/carnot/planner/distributed/removable_ops_rule.h"
#include "src/carnot/planner/rules/rules.h"
#include "src/carnot/udfspb/udfs.pb.h"
#include "src/common/uuid/uuid.h"
#include "src/shared/upid/upid.h"

namespace pl {
namespace carnot {
namespace planner {
namespace distributed {

StatusOr<std::unique_ptr<Coordinator>> Coordinator::Create(
    const distributedpb::DistributedState& distributed_state) {
  std::unique_ptr<Coordinator> coordinator(new CoordinatorImpl());
  PL_RETURN_IF_ERROR(coordinator->Init(distributed_state));
  return coordinator;
}

Status Coordinator::Init(const distributedpb::DistributedState& distributed_state) {
  return InitImpl(distributed_state);
}

Status Coordinator::ProcessConfig(const CarnotInfo& carnot_info) {
  return ProcessConfigImpl(carnot_info);
}

StatusOr<std::unique_ptr<DistributedPlan>> Coordinator::Coordinate(const IR* logical_plan) {
  return CoordinateImpl(logical_plan);
}

Status CoordinatorImpl::InitImpl(const distributedpb::DistributedState& distributed_state) {
  distributed_state_ = &distributed_state;
  for (int64_t i = 0; i < distributed_state.carnot_info_size(); ++i) {
    PL_RETURN_IF_ERROR(ProcessConfig(distributed_state.carnot_info()[i]));
  }
  if (data_store_nodes_.size() == 0) {
    return error::InvalidArgument(
        "Distributed state does not have a Carnot instance that satisifies the condition "
        "`has_data_store() && processes_data()`.");
  }
  if (remote_processor_nodes_.size() == 0) {
    return error::InvalidArgument(
        "Distributed state does not have a Carnot instance that satisifies the condition "
        "`processes_data() && accepts_remote_sources()`.");
  }
  return Status::OK();
}

Status CoordinatorImpl::ProcessConfigImpl(const CarnotInfo& carnot_info) {
  if (carnot_info.has_data_store() && carnot_info.processes_data()) {
    data_store_nodes_.push_back(carnot_info);
  }
  if (carnot_info.processes_data() && carnot_info.accepts_remote_sources()) {
    remote_processor_nodes_.push_back(carnot_info);
  }
  return Status::OK();
}

bool CoordinatorImpl::HasExecutableNodes(const IR* plan) {
  // TODO(philkuz) (PL-1287) figure out what nodes are leftover that prevent us from using this
  // condition.
  if (plan->dag().nodes().size() == 0) {
    return false;
  }

  return plan->FindNodesThatMatch(Operator()).size() > 0;
}

// Removes the sources and any members of their "independent graphs".
Status CoordinatorImpl::RemoveSourcesAndDependentOperators(
    IR* plan, const std::vector<OperatorIR*>& sources_to_remove) {
  absl::flat_hash_set<int64_t> nodes_to_remove;
  std::queue<OperatorIR*> to_remove_q;
  for (auto src_op : sources_to_remove) {
    DCHECK(Match(src_op, SourceOperator()));
    to_remove_q.push(src_op);
  }
  // extra_parents queue tracks parents of removed operators that are not removed themselves.
  // We need to do extra analysis to determine if we remove those parents.
  std::queue<OperatorIR*> extra_parents;
  while (!to_remove_q.empty()) {
    OperatorIR* parent_op = to_remove_q.front();
    to_remove_q.pop();

    nodes_to_remove.insert(parent_op->id());
    for (OperatorIR* child : parent_op->Children()) {
      for (OperatorIR* other_parent_of_child : child->parents()) {
        // Make sure not to check parent_op in the extra_parents loop.
        if (other_parent_of_child != parent_op) {
          extra_parents.push(other_parent_of_child);
        }
      }
      to_remove_q.push(child);
    }
  }

  // Check to see if we can delete any extra parents of nodes
  while (!extra_parents.empty()) {
    OperatorIR* parent = extra_parents.front();
    extra_parents.pop();
    // The parent might have been deleted after being added to extra_parents.
    if (nodes_to_remove.contains(parent->id())) {
      continue;
    }

    // If all of operator's children have been removed, then we remove the op.
    bool parent_keeps_children = false;
    for (OperatorIR* child : parent->Children()) {
      if (!nodes_to_remove.contains(child->id())) {
        parent_keeps_children = true;
        break;
      }
    }
    // If the parent keeps children, then we don't delete the parent.
    if (parent_keeps_children) {
      continue;
    }
    nodes_to_remove.insert(parent->id());
    // Now check if the parents of the parent can be deleted.
    for (OperatorIR* grandparent : parent->parents()) {
      extra_parents.push(grandparent);
    }
  }

  return plan->Prune(nodes_to_remove);
}

const distributedpb::CarnotInfo& CoordinatorImpl::GetRemoteProcessor() const {
  // TODO(philkuz) update this with a more sophisticated strategy in the future.
  DCHECK_GT(remote_processor_nodes_.size(), 0UL);
  return remote_processor_nodes_[0];
}

/**
 * A mapping of agent IDs to the corresponding plan.
 */
struct AgentToPlanMap {
  absl::flat_hash_map<int64_t, IR*> agent_to_plan_map;
  std::vector<std::unique_ptr<IR>> plan_pool;
  absl::flat_hash_map<IR*, absl::flat_hash_set<int64_t>> plan_to_agents;
};

StatusOr<AgentToPlanMap> GetUniquePEMPlans(IR* query, DistributedPlan* plan,
                                           const std::vector<int64_t>& carnot_instances,
                                           const SchemaToAgentsMap& schema_map) {
  absl::flat_hash_set<int64_t> all_agents(carnot_instances.begin(), carnot_instances.end());
  PL_ASSIGN_OR_RETURN(
      OperatorToAgentSet removable_ops_to_agents,
      MapRemovableOperatorsRule::GetRemovableOperators(plan, schema_map, all_agents, query));
  AgentToPlanMap agent_to_plan_map;
  if (removable_ops_to_agents.empty()) {
    // Create the default single PEM map.
    PL_ASSIGN_OR_RETURN(auto default_ir_uptr, query->Clone());
    auto default_ir = default_ir_uptr.get();
    agent_to_plan_map.plan_pool.push_back(std::move(default_ir_uptr));
    for (int64_t carnot_i : carnot_instances) {
      agent_to_plan_map.agent_to_plan_map[carnot_i] = default_ir;
    }
    agent_to_plan_map.plan_to_agents[default_ir] = all_agents;
    return agent_to_plan_map;
  }

  std::vector<PlanCluster> clusters = ClusterOperators(removable_ops_to_agents);
  // Cluster representing the original plan if any exist.
  auto remaining_agents = RemainingAgents(removable_ops_to_agents, all_agents);
  if (!remaining_agents.empty()) {
    clusters.emplace_back(remaining_agents, absl::flat_hash_set<OperatorIR*>{});
  }
  for (const auto& c : clusters) {
    PL_ASSIGN_OR_RETURN(auto cluster_plan_uptr, c.CreatePlan(query));
    auto cluster_plan = cluster_plan_uptr.get();
    if (cluster_plan->FindNodesThatMatch(Operator()).empty()) {
      continue;
    }
    agent_to_plan_map.plan_pool.push_back(std::move(cluster_plan_uptr));
    // TODO(philkuz) enable this when we move over the Distributed analyzer.
    // plan->AddPlan(std::move(cluster_plan_uptr));
    for (const auto& agent : c.agent_set) {
      agent_to_plan_map.agent_to_plan_map[agent] = cluster_plan;
    }

    agent_to_plan_map.plan_to_agents[cluster_plan] = c.agent_set;
  }
  return agent_to_plan_map;
}

StatusOr<std::unique_ptr<DistributedPlan>> CoordinatorImpl::CoordinateImpl(const IR* logical_plan) {
  // TODO(zasgar) set support_partial_agg to true to enable partial aggs.
  PL_ASSIGN_OR_RETURN(std::unique_ptr<DistributedSplitter> splitter,
                      DistributedSplitter::Create(/* support_partial_agg */ false));
  PL_ASSIGN_OR_RETURN(std::unique_ptr<BlockingSplitPlan> split_plan,
                      splitter->SplitKelvinAndAgents(logical_plan));
  auto distributed_plan = std::make_unique<DistributedPlan>();
  PL_ASSIGN_OR_RETURN(int64_t remote_node_id, distributed_plan->AddCarnot(GetRemoteProcessor()));
  // TODO(philkuz) Need to update the Blocking Split Plan to better represent what we expect.
  // TODO(philkuz) (PL-1469) Future support for grabbing data from multiple Kelvin nodes.

  PL_ASSIGN_OR_RETURN(std::unique_ptr<IR> remote_plan_uptr, split_plan->original_plan->Clone());
  CarnotInstance* remote_carnot = distributed_plan->Get(remote_node_id);

  IR* remote_plan = remote_plan_uptr.get();
  remote_carnot->AddPlan(remote_plan);
  distributed_plan->AddPlan(std::move(remote_plan_uptr));

  std::vector<int64_t> source_node_ids;
  for (const auto& [i, data_store_info] : Enumerate(data_store_nodes_)) {
    PL_ASSIGN_OR_RETURN(int64_t source_node_id, distributed_plan->AddCarnot(data_store_info));
    distributed_plan->AddEdge(source_node_id, remote_node_id);
    source_node_ids.push_back(source_node_id);
  }

  PL_ASSIGN_OR_RETURN(auto agent_schema_map,
                      LoadSchemaMap(*distributed_state_, distributed_plan->uuid_to_id_map()));

  PL_ASSIGN_OR_RETURN(auto agent_to_plan_map,
                      GetUniquePEMPlans(split_plan->before_blocking.get(), distributed_plan.get(),
                                        source_node_ids, agent_schema_map));

  // Add the PEM plans to the distributed plan.
  for (const auto carnot_id : source_node_ids) {
    if (!agent_to_plan_map.agent_to_plan_map.contains(carnot_id)) {
      PL_RETURN_IF_ERROR(distributed_plan->DeleteNode(carnot_id));
      continue;
    }
    distributed_plan->Get(carnot_id)->AddPlan(agent_to_plan_map.agent_to_plan_map[carnot_id]);
  }

  for (size_t i = 0; i < agent_to_plan_map.plan_pool.size(); ++i) {
    distributed_plan->AddPlan(std::move(agent_to_plan_map.plan_pool[i]));
  }

  // Prune unnecessary sources from the Kelvin plan.
  DistributedPruneUnavailableSourcesRule prune_sources_rule(agent_schema_map);
  PL_RETURN_IF_ERROR(prune_sources_rule.Apply(remote_carnot));

  distributed_plan->SetKelvin(remote_carnot);
  distributed_plan->AddPlanToAgentMap(std::move(agent_to_plan_map.plan_to_agents));

  return distributed_plan;
}

}  // namespace distributed
}  // namespace planner
}  // namespace carnot
}  // namespace pl
