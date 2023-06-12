#include "src/zerok/store/store.h"
#include <chrono>
#include <string>
#include <iostream>
#include "./Query.h"
#include "./QueryBuilder.h"
#include "src/zerok/common/utils.h"
#include <set>
#include <map>


namespace zk{
    class ZkQueryManager{
        private:
            // static bool testBoolOnce;
            // static bool storeInitializedOnce;
            // static zk::ZkStore* zkStoreReader;
            // static zk::ZkStore* zkStoreWriter;
            // static std::string uuid;
            // static long lastTimestampInMilliseconds;
            // static long ttlForRedisCheckInMilliseconds;
            // static std::set<std::string> possibleIdentifiers;
            // static std::map<std::string, std::vector<Query*> > protocolToQueries;
            // static std::map<std::string, int > queryToVersion;
            // static std::map<std::string, std::map<std::string, std::vector<Query*> > > protocolToScenarioToQueries;

            static bool isTtlExpiredPassed(){
                auto currentTime = std::chrono::high_resolution_clock::now();
                auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime.time_since_epoch()).count();
                long currentTimestampInMilliseconds = nanoseconds/1000000;
                if(lastTimestampInMilliseconds == 0L){
                    lastTimestampInMilliseconds = currentTimestampInMilliseconds;
                    return true;
                }

                if(currentTimestampInMilliseconds - lastTimestampInMilliseconds > ttlForRedisCheckInMilliseconds){
                    return true;
                }
                return true;
            }

            static std::vector<std::string> identifyChangedScenarios(){
                std::vector<std::string> expiredKeys;
                std::map<std::string, std::string> keyVersionMap = zkStoreReader->hgetall("zk_value_version");
                if(keyVersionMap.size() > 0){
                    for (const auto& keyVersionPair : keyVersionMap) {
                        std::string key = keyVersionPair.first;
                        std::string version = keyVersionPair.second;
                        if(queryToVersion.count(key) > 0){
                            if(queryToVersion[key] != std::stoi(version)){
                                queryToVersion[key] = std::stoi(version);
                                expiredKeys.push_back(key);
                            }
                        }else{
                            queryToVersion[key] = std::stoi(version);
                            expiredKeys.push_back(key);
                        }
                    }
                }
                return expiredKeys;
            }

            static void initializeQueriesV2(){
                if(isTtlExpiredPassed()){
                    //1 - identify changed scenarios
                    std::vector<std::string> changedScenarios = identifyChangedScenarios();
                    //2 - for each such scenario, get the scenairo json from redis
                    for (const auto& scenairo : changedScenarios) {
                        //2.5 - Clear the queries corresponding to the scenario from protocolToScenarioToQueries for all the query types
                        for (const auto& queryTypeStringPair : protocolToScenarioToQueries) {
                            std::string queryTypeString = queryTypeStringPair.first;
                            std::map<std::string, std::vector<Query*> > scenarioToQueries = queryTypeStringPair.second;
                            if(scenarioToQueries.count(scenairo) > 0){
                                scenarioToQueries[scenairo].clear();
                            }
                        }
                        std::string scenarioJson = zkStoreReader->get(scenairo);
                        //Check if scenarioJson is nil or empty
                        if(scenarioJson.empty()){
                            continue;
                        }
                        //convert scenarioJson to rapidjson::Value&
                        rapidjson::Document scenarioJsonDocument;
                        scenarioJsonDocument.Parse(scenarioJson.c_str());
                        rapidjson::Value scenarioDocValue(scenarioJsonDocument, scenarioJsonDocument.GetAllocator());

                        //3 - extract the queries from scenario json by calling extractQueriesFromScenario on QueryBuilder
                        std::vector<Query*> queries = QueryBuilder::extractQueriesFromScenario(scenarioJson.c_str());

                        //4 - for each query, check if the query is allowed as per the possibleIdentifiers set
                        for (const auto& query : queries) {
                            std::string identifier = query->ns + "/" + query->service;
                            if (possibleIdentifiers.count(identifier) > 0){
                                //5 - if protocolToScenarioToQueries doesn;t contain the expected protocol, 
                                // initialize the map entry with that protocol
                                std::string queryTypeString(queryTypeStringMap[query->queryType]);
                                if(protocolToScenarioToQueries.count(queryTypeString) <= 0){
                                    protocolToScenarioToQueries[queryTypeString] = {};
                                }
                                //6 - if protocolToScenarioToQueries[queryTypeString] doesn't contain the scenario, 
                                // initialize the map entry with that scenario
                                if(protocolToScenarioToQueries[queryTypeString].count(scenairo) <= 0){
                                    protocolToScenarioToQueries[queryTypeString][scenairo] = {};
                                }
                                //7 - insert the query against the protocol vector in the map
                                protocolToScenarioToQueries[queryTypeString][scenairo].push_back(query);
                            }
                        }
                    }

                    protocolToQueries.clear();
                    for (const auto& queryTypeStringPair : protocolToScenarioToQueries) {
                        std::string queryTypeString = queryTypeStringPair.first;
                        std::map<std::string, std::vector<Query*> > scenarioToQueries = queryTypeStringPair.second;
                        for (const auto& scenario : scenarioToQueries) {
                            std::string scenarioId = scenario.first;
                            std::vector<Query*> queries = scenario.second;
                            for (const auto& query : queries) {
                                if(protocolToQueries.count(queryTypeString) <= 0){
                                    protocolToQueries[queryTypeString] = {};
                                }
                                //6 - insert the query against the protocol vector in the map
                                protocolToQueries[queryTypeString].push_back(query);
                            }
                        }
                    }
                }
            }

            static void initializeQueries(){
                if(isTtlExpiredPassed()){
                    //1 - Get filters JSON from redis
                    // std::string filtersJson = zkStore->get("filters");
                    //New structure
                    // std::string filtersJson = "{\"scenarios\":[{\"version\":1684149787,\"workloads\":{\"mQHLY2dY\":{\"service\":\"demo/sofa\",\"trace_role\":\"server\",\"protocol\":\"HTTP\",\"rule\":{\"type\":\"rule_group\",\"condition\":\"AND\",\"rules\":[{\"type\":\"rule\",\"id\":\"req_method\",\"field\":\"req_method\",\"datatype\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"POST\"},{\"type\":\"rule\",\"id\":\"req_path\",\"field\":\"req_path\",\"datatype\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"/exception\"}]}}},\"scenario_id\":\"0ceD7cx\",\"filter\":{\"type\":\"workload\",\"condition\":\"AND\",\"workloads\":[\"mQHLY2dY\"]}},{\"version\":1684149743,\"workloads\":{\"mQHfY2dY\":{\"service\":\"*/*\",\"trace_role\":\"server\",\"protocol\":\"HTTP\",\"rule\":{\"type\":\"rule_group\",\"condition\":\"AND\",\"rules\":[{\"type\":\"rule\",\"id\":\"resp_status\",\"field\":\"resp_status\",\"datatype\":\"integer\",\"input\":\"integer\",\"operator\":\"equal\",\"value\":201}]}}},\"scenario_id\":\"ic234Dcs\",\"filter\":{\"type\":\"workload\",\"condition\":\"OR\",\"workloads\":[\"mQHfY2dY\"]}}]}";
                    std::string filtersJson = "{\"scenarios\":[{\"version\":1684149787,\"workloads\":{\"mQHLY2dY\":{\"service\":\"*/*\",\"trace_role\":\"server\",\"protocol\":\"HTTP\",\"rule\":{\"type\":\"rule_group\",\"condition\":\"AND\",\"rules\":[{\"type\":\"rule\",\"id\":\"req_method\",\"field\":\"req_method\",\"datatype\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"POST\"},{\"type\":\"rule\",\"id\":\"req_path\",\"field\":\"req_path\",\"datatype\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"/exception\"}]}}},\"scenario_id\":\"0ceD7cx\",\"filter\":{\"type\":\"workload\",\"condition\":\"AND\",\"workloads\":[\"mQHLY2dY\"]}},{\"version\":1684149743,\"workloads\":{\"mQHfY2dY\":{\"service\":\"*/*\",\"trace_role\":\"server\",\"protocol\":\"HTTP\",\"rule\":{\"type\":\"rule_group\",\"condition\":\"AND\",\"rules\":[{\"type\":\"rule\",\"id\":\"resp_status\",\"field\":\"resp_status\",\"datatype\":\"integer\",\"input\":\"integer\",\"operator\":\"greater_than\",\"value\":399}]}}},\"scenario_id\":\"ic234Dcs\",\"filter\":{\"type\":\"workload\",\"condition\":\"OR\",\"workloads\":[\"mQHfY2dY\"]}}]}";
                    //2 - Since re-parsing, clear the local maps
                    //TODO: replace it with local maps being populated and then replace the class static maps with local maps
                    // relevantQueries.clear();
                    protocolToQueries.clear();
                    if(filtersJson != ""){
                        //3 - Parse the queries
                        std::vector<Query*> queries = QueryBuilder::parseScenarios(filtersJson.c_str());
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
                if(!storeInitializedOnce){
                    storeInitializedOnce = true;
                    ttlForRedisCheckInMilliseconds = 300000;
                    // zkStore = zk::ZkStoreProvider::instance();
                    // zkStore->connect();
                    zkStoreReader = zk::ZkStoreProvider::instance(6);
                    zkStoreReader->connect();
                    zkStoreWriter = zk::ZkStoreProvider::instance(1);
                    zkStoreWriter->connect();
                    uuid = CommonUtils::generateUUID();
                    possibleIdentifiers.insert("*/*");
                    possibleIdentifiers.insert("NS01/*");
                    possibleIdentifiers.insert("NS01/SVC01");
                }
            }

        public:
            static bool storeInitializedOnce;
            static zk::ZkStore* zkStoreReader;
            static zk::ZkStore* zkStoreWriter;
            static std::string uuid;
            static long lastTimestampInMilliseconds;
            static long ttlForRedisCheckInMilliseconds;
            static std::set<std::string> possibleIdentifiers;
            static std::map<std::string, std::vector<Query*> > protocolToQueries;
            static std::map<std::string, int > queryToVersion;
            static std::map<std::string, std::map<std::string, std::vector<Query*> > > protocolToScenarioToQueries;

            static void refresh(){
                init();
                initializeQueriesV2();
            }

            static void get(){
                init();
                initializeQueriesV2();
            }

    };

    std::set<std::string> ZkQueryManager::possibleIdentifiers;
    std::map<std::string, std::vector<Query*> > ZkQueryManager::protocolToQueries;
    zk::ZkStore* ZkQueryManager::zkStoreReader; 
    zk::ZkStore* ZkQueryManager::zkStoreWriter; 
    std::string ZkQueryManager::uuid;
    std::map<std::string, std::map<std::string, std::vector<Query*> > > ZkQueryManager::protocolToScenarioToQueries;
    bool ZkQueryManager::storeInitializedOnce; 
    long ZkQueryManager::lastTimestampInMilliseconds;
    long ZkQueryManager::ttlForRedisCheckInMilliseconds;
    std::map<std::string, int > ZkQueryManager::queryToVersion;
}