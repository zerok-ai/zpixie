#include "src/zerok/store/store.h"
#include "/home/avin/.cache/bazel/_bazel_avin/54060b0ed2e63c063d495ae4fb1a7d19/execroot/px/external/com_github_redis_hiredis/hiredis.h"
#include <iostream>
#include "./Query.h"
#include "./QueryBuilder.h"
#include "./ZkTraceInfo.h"
#include "./QueryManager.h"
#include <random>
#include <string>
#include <map>
#include <vector>
#include <set>
#include "src/zerok/common/utils.h"
#include "src/zerok/common/ZkConfigProvider.h"
// //////
// #include <iostream>
// #include <fstream>
// #include <yaml-cpp/yaml.h>
// //////

namespace zk{
    class ZkQueryExecutor{
      public:
        static void init(){
            zk::ZkConfigProvider::init();
            ZkQueryManager::refresh();

            ///////
            // std::ifstream inputFile("/opt/zk-client-db-configmap.yaml"); // Open the file

            // if (!inputFile) {
            //     std::cerr << "Failed to open the file." << std::endl;
            // }

            // // std::string line = "";
            // // while (std::getline(inputFile, line)) {
            // //     // Process each line of the file
            // //     std::cout << line << std::endl;
            // // }

            // // Parse the YAML file
            // YAML::Node yamlNode = YAML::Load(inputFile);

            // // Access data from the YAML structure
            // std::string value = yamlNode["key"].as<std::string>();
            // std::cout << "Value: " << value << std::endl;


            // inputFile.close(); // Close the file

            ///////

        }

        static SimpleRuleKeyValue* generateTraceparentRule(std::string ruleId, bool isCaps) {
            SimpleRuleKeyValue* traceIdRule = new SimpleRuleKeyValue();
            traceIdRule->id = ruleId;
            traceIdRule->type = KEY_MAP;
            traceIdRule->input = "string";
            traceIdRule->value = "/traceparent";
            traceIdRule->key = "/traceparent";
            if (isCaps) {
                traceIdRule->key = "/Traceparent";
            }
            return traceIdRule;
        }

        static ZkTraceInfo apply(std::string protocol, std::map<std::string, std::string> propsMap){
            ZkTraceInfo zkTraceInfo = ZkTraceInfo();
            std::string traceId = "";
            std::string spanId = "";
            if(protocol == "HTTP"){
                /* Generate rules to check traceparent or Traceparent header in req_headers OR resp_headers */
                SimpleRuleKeyValue* traceIdReqRuleSmall = generateTraceparentRule("req_headers", false);
                SimpleRuleKeyValue* traceIdReqRuleCaps = generateTraceparentRule("req_headers", true);
                SimpleRuleKeyValue* traceIdResRuleSmall = generateTraceparentRule("resp_headers", false);
                SimpleRuleKeyValue* traceIdResRuleCaps = generateTraceparentRule("resp_headers", true);

                const int ruleCount = 4;
                SimpleRuleKeyValue* traceRuleArray[ruleCount] = {traceIdReqRuleSmall, traceIdReqRuleCaps, traceIdResRuleSmall, traceIdResRuleCaps};
                std::string traceParent = "ZK_NULL";
                for(int ruleIdx=0; ruleIdx<ruleCount; ruleIdx++) {
                    traceParent = traceRuleArray[ruleIdx]->extractValue(propsMap);
                    printf("\nAVIN_DEBUG_STORE_apply010001 ruleIdx %d : %s", ruleIdx, traceParent);
                    printf("\nAVIN_DEBUG_STORE_apply010002 propsMap[req_headers] : %s", propsMap.get("req_headers"));
                    printf("\nAVIN_DEBUG_STORE_apply010003 propsMap[resp_headers] : %s", propsMap.get("resp_headers"));
                    printf("\nAVIN_DEBUG_STORE_apply010004 -----------------------------------\n\n");
                    if(traceParent != "ZK_NULL"){
                        break;
                    }
                }
                if(traceParent == "ZK_NULL"){
                    /* no trace parent found in both req & resp headers */
                    printf("\nAVIN_DEBUG_STORE_apply0100 traceparent header missing");
                    return zkTraceInfo;
                } 

                printf("\nAVIN_DEBUG_STORE_apply0101 traceparent header present %s", traceParent.c_str());
                
                zkTraceInfo.fromTraceParent(traceParent);
                if(zkTraceInfo.isValid() == false){
                    printf("\nAVIN_DEBUG_STORE_apply03 traceparent header value is invalid");
                    return zkTraceInfo;
                }

                traceId = zkTraceInfo.getTraceId();
                spanId = zkTraceInfo.getSpanId();
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
                                zkTraceInfo.addWorkloadId(query->workloadId);
                                std::cout << "\nAVIN_DEBUG_STORE_apply0105" << std::endl;
                                ZkQueryManager::zkStoreWriter->addToSetWithExpiry(900, traceIdsSetKey.c_str(), traceId.c_str(), nullptr);
                            }
                        }
                    }
                }
            }else{
                //TODO: Check if trace id is present, if not return false
            }
            return zkTraceInfo;
        }
    };

}