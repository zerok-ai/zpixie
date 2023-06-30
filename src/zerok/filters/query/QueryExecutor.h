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
// //////
// #include <iostream>
// #include <fstream>
// #include <yaml-cpp/yaml.h>
// //////

namespace zk{
    class ZkQueryExecutor{
      public:
        static void init(){
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

        static ZkTraceInfo apply(std::string protocol, std::map<std::string, std::string> propsMap){
            ZkTraceInfo zkTraceInfo = ZkTraceInfo();
            std::string traceId = "";
            std::string spanId = "";
            if(protocol == "HTTP"){
                SimpleRuleKeyValue* traceIdRule = new SimpleRuleKeyValue();
                traceIdRule->id = "req_headers";
                traceIdRule->type = KEY_MAP;
                traceIdRule->input = "string";
                traceIdRule->key = "/traceparent";
                traceIdRule->value = "/traceparent";
                std::string traceParent = traceIdRule->extractValue(propsMap);
                if(traceParent == "ZK_NULL" || traceParent == ""){
                    traceIdRule->key = "/Traceparent";
                    traceParent = traceIdRule->extractValue(propsMap);
                    if(traceParent == "ZK_NULL" || traceParent == ""){
                        return zkTraceInfo;
                    }else{
                        printf("\nAVIN_DEBUG_STORE_apply0101 traceparent header present %s", traceParent.c_str());
                    }
                }else{
                    printf("\nAVIN_DEBUG_STORE_apply0101 traceparent header present %s", traceParent.c_str());
                }
                std::vector<std::string> splitString = CommonUtils::splitString(traceParent, "-");
                if(splitString.size() <= static_cast<size_t>(1)){
                    printf("\nAVIN_DEBUG_STORE_apply02 traceparent header value is invalid: %s", traceParent.c_str());
                    return zkTraceInfo;
                }
                traceId = splitString.at(1);
                if(traceId == ""){
                    printf("\nAVIN_DEBUG_STORE_apply03 traceparent header value is invalid");
                    return zkTraceInfo;
                }
                spanId = splitString.at(2);
                zkTraceInfo.setTraceId(traceId);
                zkTraceInfo.setSpanId(spanId);
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