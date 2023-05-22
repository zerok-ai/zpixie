#include "src/zerok/store/store.h"
#include <iostream>
#include "./Query.h"
#include "./QueryBuilder.h"
#include <random>
#include <string>
#include <map>
#include <set>


namespace zk{
    class ZkQueryExecutor{
      private:
        static zk::ZkStore* zkStore;
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
                std::string filtersJson = zkStore->get("some_key");
                // std::string filtersJson = "{\"rules\":[{\"version\":1684149787,\"workloads\":{\"mQHLY2dY\":{\"condition\":\"AND\",\"service\":\"demo/sofa\",\"trace_role\":\"server\",\"protocol\":\"HTTP\",\"rules\":[{\"id\":\"req_method\",\"field\":\"req_method\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"POST\"},{\"id\":\"req_path\",\"field\":\"req_path\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"ends_with\",\"value\":\"/exception\"}]}},\"filter_id\":\"0ceD7cx\",\"filters\":{\"type\":\"workload\",\"condition\":\"AND\",\"workloads\":[\"mQHLY2dY\"]}},{\"version\":1684149743,\"workloads\":{\"mQHfY2dY\":{\"condition\":\"AND\",\"service\":\"*/*\",\"trace_role\":\"server\",\"protocol\":\"HTTP\",\"rules\":[{\"condition\":\"AND\",\"rules\":[{\"id\":\"resp_status\",\"field\":\"resp_status\",\"type\":\"integer\",\"input\":\"integer\",\"operator\":\"not equal\",\"value\":200}]}]}},\"filter_id\":\"ic234Dcs\",\"filters\":{\"type\":\"workload\",\"condition\":\"OR\",\"workloads\":[\"mQHfY2dY\"]}}]}";
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
            printf("\nAVIN_DEBUG_STORE_INIT_01 initializing zk::zk-query-executor");
            ZkQueryExecutor::zkStore = zk::ZkStoreProvider::instance();
            zkStore->connect();
            possibleIdentifiers.insert("*/*");
            possibleIdentifiers.insert("NS01/*");
            possibleIdentifiers.insert("NS01/SVC01");
        }

        static bool apply(std::string protocol, std::map<std::string, std::string> propsMap){
            if(protocol == "HTTP"){
                //TODO: Check if trace id is present, if not return false
                if(protocolToQueries.count(protocol) > 0){
                    std::vector<Query*> queries = protocolToQueries[protocol];
                    if(!queries.empty()){
                        for (const auto& query : queries) {
                            bool evaluation = query->rule->evaluate(propsMap);
                            if(evaluation){
                                //TODO: Extract the traceid and put it into 
                            }
                        }
                    }
                }
            }else{
                //TODO: Check if trace id is present, if not return false
            }
            return true;
        }
    };

    std::set<std::string> ZkQueryExecutor::possibleIdentifiers;
    std::map<std::string, std::vector<Query*> > ZkQueryExecutor::protocolToQueries;

}