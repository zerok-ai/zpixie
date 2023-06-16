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
      private:
        // static bool storeInitializedOnce;
        // static zk::ZkStore* zkStore;
        // static zk::ZkStore* zkStoreReader;
        // static zk::ZkStore* zkStoreWriter;
        // static std::string uuid;
        // static std::set<std::string> possibleIdentifiers;
        // static std::map<std::string, std::vector<Query*> > protocolToQueries;
        // static std::vector<Query*> relevantQueries;

      public:
        // static void initializeQueries(){
        //     if(protocolToQueries.empty()){
        //         //1 - Get filters JSON from redis
        //         // std::string filtersJson = zkStore->get("filters");
        //         //New structure
        //         // std::string filtersJson = "{\"scenarios\":[{\"version\":1684149787,\"workloads\":{\"mQHLY2dY\":{\"service\":\"demo/sofa\",\"trace_role\":\"server\",\"protocol\":\"HTTP\",\"rule\":{\"type\":\"rule_group\",\"condition\":\"AND\",\"rules\":[{\"type\":\"rule\",\"id\":\"req_method\",\"field\":\"req_method\",\"datatype\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"POST\"},{\"type\":\"rule\",\"id\":\"req_path\",\"field\":\"req_path\",\"datatype\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"/exception\"}]}}},\"scenario_id\":\"0ceD7cx\",\"filter\":{\"type\":\"workload\",\"condition\":\"AND\",\"workloads\":[\"mQHLY2dY\"]}},{\"version\":1684149743,\"workloads\":{\"mQHfY2dY\":{\"service\":\"*/*\",\"trace_role\":\"server\",\"protocol\":\"HTTP\",\"rule\":{\"type\":\"rule_group\",\"condition\":\"AND\",\"rules\":[{\"type\":\"rule\",\"id\":\"resp_status\",\"field\":\"resp_status\",\"datatype\":\"integer\",\"input\":\"integer\",\"operator\":\"equal\",\"value\":201}]}}},\"scenario_id\":\"ic234Dcs\",\"filter\":{\"type\":\"workload\",\"condition\":\"OR\",\"workloads\":[\"mQHfY2dY\"]}}]}";
        //         std::string filtersJson = "{\"scenarios\":[{\"version\":1684149787,\"workloads\":{\"mQHLY2dY\":{\"service\":\"*/*\",\"trace_role\":\"server\",\"protocol\":\"HTTP\",\"rule\":{\"type\":\"rule_group\",\"condition\":\"AND\",\"rules\":[{\"type\":\"rule\",\"id\":\"req_method\",\"field\":\"req_method\",\"datatype\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"POST\"},{\"type\":\"rule\",\"id\":\"req_path\",\"field\":\"req_path\",\"datatype\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"/exception\"}]}}},\"scenario_id\":\"0ceD7cx\",\"filter\":{\"type\":\"workload\",\"condition\":\"AND\",\"workloads\":[\"mQHLY2dY\"]}},{\"version\":1684149743,\"workloads\":{\"mQHfY2dY\":{\"service\":\"*/*\",\"trace_role\":\"server\",\"protocol\":\"HTTP\",\"rule\":{\"type\":\"rule_group\",\"condition\":\"AND\",\"rules\":[{\"type\":\"rule\",\"id\":\"resp_status\",\"field\":\"resp_status\",\"datatype\":\"integer\",\"input\":\"integer\",\"operator\":\"greater_than\",\"value\":399}]}}},\"scenario_id\":\"ic234Dcs\",\"filter\":{\"type\":\"workload\",\"condition\":\"OR\",\"workloads\":[\"mQHfY2dY\"]}}]}";
        //         //Old structure
        //         // std::string filtersJson = "{\"rules\":[{\"version\":1684149787,\"workloads\":{\"mQHLY2dY\":{\"condition\":\"AND\",\"service\":\"demo/sofa\",\"trace_role\":\"server\",\"protocol\":\"HTTP\",\"rules\":[{\"id\":\"req_method\",\"field\":\"req_method\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"POST\"},{\"id\":\"req_path\",\"field\":\"req_path\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"ends_with\",\"value\":\"/exception\"}]}},\"filter_id\":\"0ceD7cx\",\"filters\":{\"type\":\"workload\",\"condition\":\"AND\",\"workloads\":[\"mQHLY2dY\"]}},{\"version\":1684149743,\"workloads\":{\"mQHfY2dY\":{\"condition\":\"AND\",\"service\":\"*/*\",\"trace_role\":\"server\",\"protocol\":\"HTTP\",\"rules\":[{\"condition\":\"AND\",\"rules\":[{\"id\":\"resp_status\",\"field\":\"resp_status\",\"type\":\"integer\",\"input\":\"integer\",\"operator\":\"equals\",\"value\":201}]}]}},\"filter_id\":\"ic234Dcs\",\"filters\":{\"type\":\"workload\",\"condition\":\"OR\",\"workloads\":[\"mQHfY2dY\"]}}]}";
        //         //2 - Since re-parsing, clear the local maps
        //         //TODO: replace it with local maps being populated and then replace the class static maps with local maps
        //         // relevantQueries.clear();
        //         protocolToQueries.clear();
        //         if(filtersJson != ""){
        //             //3 - Parse the queries
        //             std::vector<Query*> queries = QueryBuilder::parseScenarios(filtersJson.c_str());
        //             for (const auto& query : queries) {
        //                 std::string identifier = query->ns + "/" + query->service;
        //                 //4 - Check if the identifier is allowed as per the possibleIdentifiers set 
        //                 if (possibleIdentifiers.count(identifier) > 0){
        //                     //5 - if protocolToQueries doesn;t contain the expected protocol, 
        //                     // initialize the map entry with that protocol
        //                     std::string queryTypeString(queryTypeStringMap[query->queryType]);
        //                     if(protocolToQueries.count(queryTypeString) <= 0){
        //                         protocolToQueries[queryTypeString] = {};
        //                     }
        //                     //6 - insert the query against the protocol vector in the map
        //                     protocolToQueries[queryTypeString].push_back(query);
        //                     // relevantQueries.push_back(query);
        //                 }
        //             }
        //         }
        //     }
        // }

        static void init(){
            ZkQueryManager::refresh();
            // if(!storeInitializedOnce){
                // storeInitializedOnce = true;
                // zkStore = zk::ZkStoreProvider::instance();
                // zkStore->connect();
                // zkStoreReader = zk::ZkStoreProvider::instance(6);
                // zkStoreReader->connect();
                // zkStoreWriter = zk::ZkStoreProvider::instance(1);
                // zkStoreWriter->connect();
                // uuid = CommonUtils::generateUUID();
                // possibleIdentifiers.insert("*/*");
                // possibleIdentifiers.insert("NS01/*");
                // possibleIdentifiers.insert("NS01/SVC01");
            // }
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
                // printf("\nAVIN_DEBUG_STORE_apply0102");
                //TODO: Check if trace id is present, if not return false
                if(ZkQueryManager::protocolToQueries.count(protocol) > 0){
                    std::cout << "\nAVIN_DEBUG_STORE_apply0103" << std::endl;
                    // printf("\nAVIN_DEBUG_STORE_apply0103");
                    std::vector<Query*> queries = ZkQueryManager::protocolToQueries[protocol];
                    if(!queries.empty()){
                        std::cout << "\nAVIN_DEBUG_STORE_apply0104" << std::endl;
                        ///////
                        // std::string myString = "";
                        // for (const auto& pair : propsMap) {
                        //     myString += pair.first + ": " + pair.second + "@@@@";
                        // }
                        // std::cout << "nAVIN_DEBUG_STORE_apply0105 propsMap " << myString  << std::endl;
                        
                        // printf("\nAVIN_DEBUG_STORE_apply0104");
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
                                // zkStore->addToSet(traceIdsSetKey.c_str(), traceId.c_str(), nullptr);
                                // zkStoreWriter->addToSet(traceIdsSetKey.c_str(), traceId.c_str(), nullptr);
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

    // std::set<std::string> ZkQueryExecutor::possibleIdentifiers;
    // std::map<std::string, std::vector<Query*> > ZkQueryExecutor::protocolToQueries;
    // zk::ZkStore* ZkQueryExecutor::zkStore; 
    // zk::ZkStore* ZkQueryExecutor::zkStoreReader; 
    // zk::ZkStore* ZkQueryExecutor::zkStoreWriter; 
    // std::string ZkQueryExecutor::uuid; 
    // bool ZkQueryExecutor::storeInitializedOnce; 

}