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
            static bool isTtlExpiredPassed(){
                auto currentTime = std::chrono::high_resolution_clock::now();
                auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime.time_since_epoch()).count();
                long currentTimestampInMilliseconds = nanoseconds/1000000;
                if(lastTimestampInMilliseconds == 0){
                    lastTimestampInMilliseconds = currentTimestampInMilliseconds;
                    std::cout << "\nAVIN_DEBUG_QUERY_check01" << std::endl;
                    return true;
                }

                if(currentTimestampInMilliseconds - lastTimestampInMilliseconds > ttlForRedisCheckInMilliseconds){
                    lastTimestampInMilliseconds = currentTimestampInMilliseconds;
                    std::cout << "\nAVIN_DEBUG_QUERY_check02" << std::endl;
                    return true;
                }
                return false;
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
                                std::cout << "\nAVIN_DEBUG_QUERY_change01" << std::endl;
                                // printf("\nAVIN_DEBUG_QUERY_change01 ");
                                expiredKeys.push_back(key);
                            }
                        }else{
                            queryToVersion[key] = std::stoi(version);
                            std::cout << "\nAVIN_DEBUG_QUERY_change02" << std::endl;
                            // printf("\nAVIN_DEBUG_QUERY_change02 ");
                            expiredKeys.push_back(key);
                        }
                    }
                }
                return expiredKeys;
            }

            static void initializeQueriesV2(){
                if(isTtlExpiredPassed()){
                    std::cout << "\nAVIN_DEBUG_QUERY_init01" << std::endl;
                    // printf("\nAVIN_DEBUG_QUERY_init01 ");
                    //1 - Identify changed scenarios
                    std::vector<std::string> changedScenarios = identifyChangedScenarios();

                    //1.2 - Extract the attributes from redis
                    std::map<std::string, std::map<std::string, std::string>> protocolToAttributesMap;
                    std::map<std::string, std::string> attributesMap = zkStoreReader->hgetall("EBPF_0.1.0-alpha_HTTP");
                    protocolToAttributesMap["HTTP"] = attributesMap;

                    //1.5 - Check for the size of changedScenarios and return if it is 0
                    if(changedScenarios.size() == 0){
                        return;
                    }

                    //2 - for each such scenario, get the scenairo json from redis
                    for (const auto& scenairo : changedScenarios) {
                        std::cout << "\nAVIN_DEBUG_QUERY_init02 ScenarioId processed " << scenairo.c_str() << std::endl;
                        // printf("\nAVIN_DEBUG_QUERY_init02 ScenarioId processed - %s", scenairo.c_str());
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
                        std::vector<Query*> queries = QueryBuilder::extractQueriesFromScenario(scenarioJson.c_str(), protocolToAttributesMap);

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
                                std::cout << "\nAVIN_DEBUG_QUERY_init03 " << std::endl;
                                // printf("\nAVIN_DEBUG_QUERY_init03 ");
                                protocolToScenarioToQueries[queryTypeString][scenairo].push_back(query);
                            }
                        }
                    }

                    std::cout << "\nAVIN_DEBUG_QUERY_init04 " << std::endl;
                    // printf("\nAVIN_DEBUG_QUERY_init04 ");
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
                                std::cout << "\nAVIN_DEBUG_QUERY_init05 " << std::endl;
                                // printf("\nAVIN_DEBUG_QUERY_init05 ");
                                protocolToQueries[queryTypeString].push_back(query);
                            }
                        }
                    }
                }
            }

            static void init(){
                if(!storeInitializedOnce){
                    std::cout << "\nAVIN_DEBUG_QUERY_init00 " << std::endl;
                    storeInitializedOnce = true;
                    ttlForRedisCheckInMilliseconds = 300000;
                    zkStoreReader = zk::ZkStoreProvider::instance(6);
                    zkStoreWriter = zk::ZkStoreProvider::instance(1);
                    zkStoreAttributedReader = zk::ZkStoreProvider::instance(4);
                    uuid = CommonUtils::generateUUID();
                    possibleIdentifiers.insert("*/*");
                    possibleIdentifiers.insert("NS01/*");
                    possibleIdentifiers.insert("NS01/SVC01");
                }
                bool readerConnected = zkStoreReader->connect();
                if(!readerConnected){
                    zkStoreReader->connect();
                }
                bool attributesReaderConnected = zkStoreAttributedReader->connect();
                if(!attributesReaderConnected){
                    zkStoreAttributedReader->connect();
                }
                bool writerConnected = zkStoreWriter->connect();
                if(!writerConnected){
                    zkStoreWriter->connect();
                }
            }

        public:
            static bool storeInitializedOnce;
            static zk::ZkStore* zkStoreReader;
            static zk::ZkStore* zkStoreWriter;
            static zk::ZkStore* zkStoreAttributedReader;
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
    zk::ZkStore* ZkQueryManager::zkStoreAttributedReader; 
    std::string ZkQueryManager::uuid;
    std::map<std::string, std::map<std::string, std::vector<Query*> > > ZkQueryManager::protocolToScenarioToQueries;
    bool ZkQueryManager::storeInitializedOnce; 
    long ZkQueryManager::lastTimestampInMilliseconds = 0;
    long ZkQueryManager::ttlForRedisCheckInMilliseconds;
    std::map<std::string, int > ZkQueryManager::queryToVersion;
}