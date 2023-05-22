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
        static std::vector<Query*> relevantQueries;
        static void initializeQueries(){
            if(protocolToQueries.empty()){
                //1 - Get filters JSON from redis
                std::string filtersJson = zkStore->get("some_key");
                //2 - Since re-parsing, clear the local maps
                //TODO: replace it with local maps being populated and then replace the class static maps with local maps
                relevantQueries.clear();
                protocolToQueries.clear();
                if(filtersJson != ""){
                    //3 - Parse the queries
                    std::vector<Query*> queries = QueryBuilder::parseQueries(filtersJson.c_str());
                    for (const auto& query : queries) {
                        std::string identifier = query.ns + "/" + query.service;
                        //4 - Check if the identifier is allowed as per the possibleIdentifiers set 
                        if (possibleIdentifiers.count(identifier) > 0){
                            //5 - if protocolToQueries doesn;t contain the expected protocol, 
                            // initialize the map entry with that protocol
                            if(protocolToQueries.count() > 0){
                                protocolToQueries[queryTypeStringMap[query.queryType]] = {};
                            }
                            //6 - insert the query against the protocol vector in the map
                            protocolToQueries[queryTypeStringMap[query.queryType]].push_back(query);
                            relevantQueries.push_back(element);
                        }
                    }
                }
            }
        }

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
}