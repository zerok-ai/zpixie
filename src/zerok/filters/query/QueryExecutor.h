#include <iostream>
#include <map>
#include <random>
#include <set>
#include <string>
#include <vector>
#include "./Query.h"
#include "./QueryBuilder.h"
#include "./QueryManager.h"
#include "./ZkTraceInfo.h"
#include "hiredis.h"
#include "src/zerok/common/ZkConfigProvider.h"
#include "src/zerok/common/utils.h"
#include "src/zerok/store/store.h"
// //////
// #include <iostream>
// #include <fstream>
// #include <yaml-cpp/yaml.h>
// //////

namespace zk {
class ZkQueryExecutor {
 private:
  static SimpleRuleString* generateTraceparentRuleV2(std::string ruleId, bool isCaps) {
    SimpleRuleString* traceIdRule = new SimpleRuleString();
    traceIdRule->id = ruleId;
    traceIdRule->type = STRING;
    traceIdRule->input = "string";
    traceIdRule->value = "/traceparent";
    traceIdRule->json_path = "/traceparent";
    if (isCaps) {
      traceIdRule->json_path = "/Traceparent";
    }
    return traceIdRule;
  }

 public:
  static void init() {
    zk::ZkConfigProvider::init();
    ZkQueryManager::refresh();
  }

  static ZkTraceInfo apply(std::string protocol, std::map<std::string, std::string> propsMap) {
    ZkTraceInfo zkTraceInfo = ZkTraceInfo();
    std::string traceId = "";
    std::string spanId = "";
    if (protocol == "HTTP") {
      /* Generate rules to check traceparent or Traceparent header in req_headers OR resp_headers */
      SimpleRuleString* traceIdReqRuleSmall = generateTraceparentRuleV2("req_headers", false);
      SimpleRuleString* traceIdReqRuleCaps = generateTraceparentRuleV2("req_headers", true);
      SimpleRuleString* traceIdResRuleSmall = generateTraceparentRuleV2("resp_headers", false);
      SimpleRuleString* traceIdResRuleCaps = generateTraceparentRuleV2("resp_headers", true);

      const int ruleCount = 4;
      SimpleRuleString* traceRuleArray[ruleCount] = {traceIdReqRuleSmall, traceIdReqRuleCaps,
                                                     traceIdResRuleSmall, traceIdResRuleCaps};
      std::string traceParent = "ZK_NULL";
      for (int ruleIdx = 0; ruleIdx < ruleCount; ruleIdx++) {
        traceParent = traceRuleArray[ruleIdx]->extractValue(propsMap);
        if (traceParent != "ZK_NULL") {
          break;
        }
      }

      // Deletion
      for (int i = 0; i < ruleCount; ++i) {
        delete traceRuleArray[i];
      }
      // Setting the pointers to null to avoid potential dangling pointers
      for (int i = 0; i < ruleCount; ++i) {
        traceRuleArray[i] = nullptr;
      }

      if (traceParent == "ZK_NULL") {
        /* no trace parent found in both req & resp headers */
        return zkTraceInfo;
      }

      // printf("\nAVIN_DEBUG_STORE_apply0101 traceparent header present %s", traceParent.c_str());

      zkTraceInfo.fromTraceParent(traceParent);
      if (zkTraceInfo.isValid() == false) {
        // printf("\nAVIN_DEBUG_STORE_apply03 traceparent header value is invalid");
        return zkTraceInfo;
      }

      traceId = zkTraceInfo.getTraceId();
      spanId = zkTraceInfo.getSpanId();
      // std::cout << "\nAVIN_DEBUG_STORE_apply0102" << std::endl;
      if (true) {
        std::string myString = "";
        for (const auto& pair : propsMap) {
          myString += pair.first + ": " + pair.second + "@@@@";
        }
        // std::cout << "\nAVIN_DEBUG_STORE_apply010201 propsMap " << myString << std::endl;
      }
      // TODO: Check if trace id is present, if not return false
      if (ZkQueryManager::protocolToQueries.count(protocol) > 0) {
        // std::cout << "\nAVIN_DEBUG_STORE_apply0103" << std::endl;
        std::vector<Query*> queries = ZkQueryManager::protocolToQueries[protocol];
        if (!queries.empty()) {
          // std::cout << "\nAVIN_DEBUG_STORE_apply0104" << std::endl;
          for (const auto& query : queries) {
            bool evaluation = query->rule->evaluate(propsMap);
            std::cout << "\nzk-log/executor " << query->workloadId << ":eval--"
                      << evaluation << std::endl;
            int currentMinutes = CommonUtils::systemMinutes();
            // std::string traceIdsSetKey = query->workloadId + "_" + uuid + "_" +
            // std::to_string(currentMinutes/5);
            std::string traceIdsSetKey =
                query->workloadId + "_" + std::to_string(currentMinutes / 5);
            if (evaluation) {
              zkTraceInfo.addWorkloadId(query->workloadId);
              // std::cout << "\nAVIN_DEBUG_STORE_apply0105" << std::endl;
              ZkQueryManager::zkStoreWriter->addToSetWithExpiry(900, traceIdsSetKey.c_str(),
                                                                traceId.c_str(), nullptr);
            }
          }
        }
      }
    } else {
      // TODO: Check if trace id is present, if not return false
    }
    return zkTraceInfo;
  }
};

}  // namespace zk