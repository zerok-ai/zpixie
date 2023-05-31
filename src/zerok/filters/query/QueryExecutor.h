#include "src/zerok/store/store.h"
#include "/home/avin/.cache/bazel/_bazel_avin/54060b0ed2e63c063d495ae4fb1a7d19/execroot/px/external/com_github_redis_hiredis/hiredis.h"
#include <iostream>
#include "./Query.h"
#include "./QueryBuilder.h"
#include <random>
#include <string>
#include <map>
#include <set>
#include "src/zerok/common/utils.h"

namespace zk{
    class ZkQueryExecutor{
      private:
        static zk::ZkStore* zkStore;
        static std::string uuid;
        static std::set<std::string> possibleIdentifiers;
        static std::map<std::string, std::vector<Query*> > protocolToQueries;
        // static std::vector<Query*> relevantQueries;

        // static std::vector<Query*> extractRelevantQueries(){
        //     std::vector<Query*> queries;
        //     if(!identifierToQueries.empty()){
        //         zkStore->connect();

        //         for (const auto& identifier : possibleIdentifiers) {
        //             if(identifierToQueries.count(identifier)){
        //                 std::vector<Query*> foundQueries = identifierToQueries[identifier];
        //                 for (const auto& element : foundQueries) {
        //                     queries.push_back(element);
        //                 }
        //             }
        //         }
        //     }
        //     return queries;
        // }

      public:
        static void initializeQueries(){
            if(protocolToQueries.empty()){
                //1 - Get filters JSON from redis
                // std::string filtersJson = zkStore->get("filters");
                std::string filtersJson = "{\"rules\":[{\"version\":1684149787,\"workloads\":{\"mQHLY2dY\":{\"condition\":\"AND\",\"service\":\"demo/sofa\",\"trace_role\":\"server\",\"protocol\":\"HTTP\",\"rules\":[{\"id\":\"req_method\",\"field\":\"req_method\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"POST\"},{\"id\":\"req_path\",\"field\":\"req_path\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"ends_with\",\"value\":\"/exception\"}]}},\"filter_id\":\"0ceD7cx\",\"filters\":{\"type\":\"workload\",\"condition\":\"AND\",\"workloads\":[\"mQHLY2dY\"]}},{\"version\":1684149743,\"workloads\":{\"mQHfY2dY\":{\"condition\":\"AND\",\"service\":\"*/*\",\"trace_role\":\"server\",\"protocol\":\"HTTP\",\"rules\":[{\"condition\":\"AND\",\"rules\":[{\"id\":\"resp_status\",\"field\":\"resp_status\",\"type\":\"integer\",\"input\":\"integer\",\"operator\":\"equals\",\"value\":201}]}]}},\"filter_id\":\"ic234Dcs\",\"filters\":{\"type\":\"workload\",\"condition\":\"OR\",\"workloads\":[\"mQHfY2dY\"]}}]}";
                //2 - Since re-parsing, clear the local maps
                //TODO: replace it with local maps being populated and then replace the class static maps with local maps
                // relevantQueries.clear();
                protocolToQueries.clear();
                if(filtersJson != ""){
                    //3 - Parse the queries
                    std::vector<Query*> queries = QueryBuilder::parseQueries(filtersJson.c_str());
                    for (const auto& query : queries) {
                        std::string identifier = query->ns + "/" + query->service;
                        //4 - Check if the identifier is allowed as per the possibleIdentifiers set 
                        if (possibleIdentifiers.count(identifier) > 0){
                            //5 - if protocolToQueries doesn;t contain the expected protocol, 
                            // initialize the map entry with that protocol
                            std::string queryTypeString(queryTypeStringMap[query->queryType]);
                            if(protocolToQueries.count(queryTypeString) <= 0){
                                protocolToQueries[queryTypeString] = {};
                            }
                            //6 - insert the query against the protocol vector in the map
                            protocolToQueries[queryTypeString].push_back(query);
                            // relevantQueries.push_back(query);
                        }
                    }
                }
            }
        }

        static void init(){
            // printf("\nAVIN_DEBUG_STORE_INIT_01 initializing zk::zk-query-executor");
            zkStore = zk::ZkStoreProvider::instance();
            zkStore->connect();
            uuid = CommonUtils::generateUUID();
            possibleIdentifiers.insert("*/*");
            possibleIdentifiers.insert("NS01/*");
            possibleIdentifiers.insert("NS01/SVC01");
        }

        static std::string apply(std::string protocol, std::map<std::string, std::string> propsMap){
            if(protocol == "HTTP"){
                SimpleRuleKeyValue* traceIdRule = new SimpleRuleKeyValue();
                traceIdRule->id = "resp_headers";
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
                if(splitString.size() <= 1){
                    printf("\nAVIN_DEBUG_STORE_apply02 traceparent header value is invalid: %s", traceParent.c_str());
                    return "ZK_NULL";
                }
                std::string traceId = splitString.at(1);
                if(traceId == ""){
                    printf("\nAVIN_DEBUG_STORE_apply03 traceparent header value is invalid");
                    return "ZK_NULL";
                }
                std::cout << "\nAVIN_DEBUG_STORE_apply0102" << std::endl;
                // printf("\nAVIN_DEBUG_STORE_apply0102");
                //TODO: Check if trace id is present, if not return false
                if(protocolToQueries.count(protocol) > 0){
                    std::cout << "\nAVIN_DEBUG_STORE_apply0103" << std::endl;
                    // printf("\nAVIN_DEBUG_STORE_apply0103");
                    std::vector<Query*> queries = protocolToQueries[protocol];
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
                            int currentMinutes = CommonUtils::systemMinutes();
                            // std::string traceIdsSetKey = query->workloadId + "_" + uuid + "_" + std::to_string(currentMinutes/5);
                            std::string traceIdsSetKey = query->workloadId + "_" + std::to_string(currentMinutes/5);
                            if(evaluation){
                                std::cout << "\nAVIN_DEBUG_STORE_apply0105" << std::endl;
                                // printf("\nAVIN_DEBUG_STORE_apply04 applying value");
                                zkStore->addToSet(traceIdsSetKey.c_str(), traceId.c_str(), nullptr);
                                //TODO: Extract the traceid and put it into 
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

    std::set<std::string> ZkQueryExecutor::possibleIdentifiers;
    std::map<std::string, std::vector<Query*> > ZkQueryExecutor::protocolToQueries;
    zk::ZkStore* ZkQueryExecutor::zkStore; 
    std::string ZkQueryExecutor::uuid; 

}