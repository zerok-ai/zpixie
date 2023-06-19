#include "src/zerok/store/store.h"
#include "/home/avin/.cache/bazel/_bazel_avin/54060b0ed2e63c063d495ae4fb1a7d19/execroot/px/external/com_github_redis_hiredis/hiredis.h"
#include <iostream>
#include "./Query.h"
#include "./QueryBuilder.h"
#include "./QueryManager.h"
#include <random>
#include <string>
#include <map>
#include <set>
#include "src/zerok/common/utils.h"

namespace zk{
    class ZkQueryExecutor{
      public:
        static void init(){
            ZkQueryManager::refresh();
        }

        static std::string apply(std::string protocol, std::map<std::string, std::string> propsMap){
            std::string traceId = "";
            if(protocol == "HTTP"){
                SimpleRuleKeyValue* traceIdRule = new SimpleRuleKeyValue();
                traceIdRule->id = "req_headers";
                traceIdRule->type = KEY_MAP;
                traceIdRule->input = "string";
                traceIdRule->key = "/traceparent";
                traceIdRule->value = "/traceparent";
                std::string traceParent = traceIdRule->extractValue(propsMap);
                if(traceParent == "ZK_NULL" || traceParent == ""){
                    // printf("\nAVIN_DEBUG_STORE_apply01 no traceparent header");
                    return "ZK_NULL";
                }else{
                    printf("\nAVIN_DEBUG_STORE_apply0101 traceparent header present");
                }
                std::vector<std::string> splitString = CommonUtils::splitString(traceParent, "-");
                if(splitString.size() <= static_cast<size_t>(1)){
                    printf("\nAVIN_DEBUG_STORE_apply02 traceparent header value is invalid: %s", traceParent.c_str());
                    return "ZK_NULL";
                }
                traceId = splitString.at(1);
                if(traceId == ""){
                    printf("\nAVIN_DEBUG_STORE_apply03 traceparent header value is invalid");
                    return "ZK_NULL";
                }
                std::cout << "\nAVIN_DEBUG_STORE_apply0102" << std::endl;
                //TODO: Check if trace id is present, if not return false
                if(ZkQueryManager::protocolToQueries.count(protocol) > 0){
                    std::cout << "\nAVIN_DEBUG_STORE_apply0103" << std::endl;
                    std::vector<Query*> queries = ZkQueryManager::protocolToQueries[protocol];
                    if(!queries.empty()){
                        std::cout << "\nAVIN_DEBUG_STORE_apply0104" << std::endl;
                        for (const auto& query : queries) {
                            bool evaluation = query->rule->evaluate(propsMap);
                            std::cout << "\nAVIN_DEBUG_STORE_apply010401 " << query->workloadId << ":eval--" << evaluation << std::endl;
                            //Print all entries from propsMap
                            // std::string myString = "";
                            // for (const auto& pair : propsMap) {
                            //     myString += pair.first + ": " + pair.second + "@@@@";
                            // }
                            // std::cout << "\nAVIN_DEBUG_STORE_apply010402 propsMap " << myString  << std::endl;
                            int currentMinutes = CommonUtils::systemMinutes();
                            // std::string traceIdsSetKey = query->workloadId + "_" + uuid + "_" + std::to_string(currentMinutes/5);
                            std::string traceIdsSetKey = query->workloadId + "_" + std::to_string(currentMinutes/5);
                            if(evaluation){
                                std::cout << "\nAVIN_DEBUG_STORE_apply0105" << std::endl;
                                ZkQueryManager::zkStoreWriter->addToSetWithExpiry(900, traceIdsSetKey.c_str(), traceId.c_str(), nullptr);
                            }
                        }
                    }
                }
            }else{
                //TODO: Check if trace id is present, if not return false
            }
            return traceId;
        }
    };

}