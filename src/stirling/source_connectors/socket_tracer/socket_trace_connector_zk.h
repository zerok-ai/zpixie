/*
 * Copyright 2018- The Pixie Authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/stirling/source_connectors/socket_tracer/socket_trace_connector.h"

#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <filesystem>
#include <utility>
// #include <thread>

#include <absl/container/flat_hash_map.h>
#include <absl/strings/match.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/delimited_message_util.h>
#include <magic_enum.hpp>

#include "src/common/base/base.h"
#include "src/common/base/utils.h"
#include "src/common/json/json.h"
#include "src/common/system/proc_pid_path.h"
#include "src/common/system/socket_info.h"
#include "src/shared/metadata/metadata.h"
#include "src/stirling/bpf_tools/macros.h"
#include "src/stirling/bpf_tools/utils.h"
#include "src/stirling/source_connectors/socket_tracer/bcc_bpf_intf/go_grpc_types.hpp"
#include "src/stirling/source_connectors/socket_tracer/bcc_bpf_intf/socket_trace.hpp"
#include "src/stirling/source_connectors/socket_tracer/conn_stats.h"
#include "src/stirling/source_connectors/socket_tracer/proto/sock_event.pb.h"
#include "src/stirling/source_connectors/socket_tracer/protocols/http/utils.h"
#include "src/stirling/source_connectors/socket_tracer/protocols/http2/grpc.h"
#include "src/stirling/utils/linux_headers.h"
#include "src/stirling/utils/proc_path_tools.h"
#include "src/zerok/filters/query/Query.h"
#include "src/zerok/filters/query/QueryBuilder.h"
#include "src/zerok/filters/query/QueryExecutor.h"
// #include "src/zerok/store/redis.h"

namespace px {
  namespace stirling {

    using px::utils::ToJSONString;

    class ZkRulesExecutor{
      private:
        static int64_t calculateLatency(int64_t req_timestamp_ns, int64_t resp_timestamp_ns) {
          int64_t latency_ns = 0;
          if (req_timestamp_ns > 0 && resp_timestamp_ns > 0) {
            latency_ns = resp_timestamp_ns - req_timestamp_ns;
            LOG_IF_EVERY_N(WARNING, latency_ns < 0, 100)
                << absl::Substitute("Negative latency implies req resp mismatch [t_req=$0, t_resp=$1].",
                                    req_timestamp_ns, resp_timestamp_ns);
          }
          return latency_ns;
        }

      public:
        static void init(){
          // std::thread::id threadId = std::this_thread::get_id();
          // LOG(INFO) << "\nAVIN_DEBUG_STORE_INIT_01 initializing striling::zk-executor " << threadId;
          zk::ZkQueryExecutor::init();
          zk::ZkQueryExecutor::initializeQueries();
        }

        //returns passthrough value - as in if the given record is allowed to be writtent o the db (apache arrow)
        static bool httpEvaluate(const ConnTracker& conn_tracker, protocols::http::Message& req_message, 
            protocols::http::Message& resp_message, HTTPContentType content_type, md::UPID upid){
          init();
          (void)req_message;
          (void)resp_message;
          // zk::ZkStore zkStore;
          // zkStore.connect();
          std::map<std::string, std::string> propsMap;
          //TODO:ZEROK Remove the following debug values
          //Debug values START
          propsMap["zk_req_type"] = "HTTP";
          propsMap["int_field"] = "35";
          // propsMap["trace_role"] = "server";
          propsMap["remote_addr"] = "10.0.0.4";
          propsMap["key_value_field"] = "{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value2\":{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value3\":\"HTTP\"}}}";
          //Debug values END 

          propsMap["time_"] = std::to_string(static_cast<long>(resp_message.timestamp_ns));
          propsMap["upid"] = std::to_string(absl::Uint128High64(upid.value())) + std::to_string(absl::Uint128Low64(upid.value()));
          // Note that there is a string copy here,
          // But std::move is not allowed because we re-use conn object.
          propsMap["remote_addr"] = conn_tracker.remote_endpoint().AddrStr();
          propsMap["remote_port"] = std::to_string(conn_tracker.remote_endpoint().port());
          int traceRoleInt = conn_tracker.role();
          std::string traceRoleString = "";
          if(traceRoleInt == 2){
            traceRoleString = "server";
          }else if(traceRoleInt == 1){
            traceRoleString = "client";
          }
          propsMap["trace_role"] = traceRoleString;//std::to_string(conn_tracker.role());
          propsMap["major_version"] = std::to_string(1);
          propsMap["minor_version"] = std::to_string(resp_message.minor_version);
          propsMap["content_type"] = std::to_string(static_cast<uint64_t>(content_type));
          propsMap["req_headers"] = ToJSONString(req_message.headers);
          propsMap["req_method"] = req_message.req_method;
          propsMap["req_path"] = req_message.req_path;
          propsMap["req_body_size"] = std::to_string(req_message.body_size);
          propsMap["req_body"] = req_message.body;
          propsMap["resp_headers"] = ToJSONString(resp_message.headers);
          propsMap["resp_status"] = std::to_string(resp_message.resp_status);
          propsMap["resp_message"] = resp_message.resp_message;
          propsMap["resp_body_size"] = std::to_string(resp_message.body_size);
          propsMap["resp_body"] = resp_message.body;
          propsMap["latency"] = std::to_string(calculateLatency(req_message.timestamp_ns, resp_message.timestamp_ns));
          ///////
          // std::string myString = "";
          // for (const auto& pair : propsMap) {
          //     myString += pair.first + ": " + pair.second + "@@@@";
          // }
          // LOG(INFO) << "AVIN_DEBUG05__SocketTraceConnector::AppendMessage myString " << myString;
          bool outcome = zk::ZkQueryExecutor::apply("HTTP", propsMap);
          // LOG(INFO) << "AVIN_DEBUG06__SocketTraceConnector::AppendMessage query->rule->evaluate(propsMap) " << zk::ZkQueryExecutor::apply("HTTP", propsMap);
          return outcome;
        }
    };
  }
}
