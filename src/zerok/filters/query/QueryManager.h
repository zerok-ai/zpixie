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
                    // std::cout << "\nAVIN_DEBUG_QUERY_check01" << std::endl;
                    return true;
                }

                if(currentTimestampInMilliseconds - lastTimestampInMilliseconds > ttlForRedisCheckInMilliseconds){
                    lastTimestampInMilliseconds = currentTimestampInMilliseconds;
                    // std::cout << "\nAVIN_DEBUG_QUERY_check02" << std::endl;
                    return true;
                }
                return false;
            }

            static bool isUpidsTtlExpiredPassed(){
                auto currentTime = std::chrono::high_resolution_clock::now();
                auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime.time_since_epoch()).count();
                long currentTimestampInMilliseconds = nanoseconds/1000000;
                if(lastTimestampUpidsSyncInMilliseconds == 0){
                    lastTimestampUpidsSyncInMilliseconds = currentTimestampInMilliseconds;
                    // std::cout << "\nAVIN_DEBUG_QUERY_check01" << std::endl;
                    return true;
                }

                if (currentTimestampInMilliseconds - lastTimestampUpidsSyncInMilliseconds > ttlForUpidsCheckInMilliseconds) {
                    lastTimestampUpidsSyncInMilliseconds = currentTimestampInMilliseconds;
                    // std::cout << "\nAVIN_DEBUG_QUERY_check02" << std::endl;
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
                                // std::cout << "\nAVIN_DEBUG_QUERY_change01" << std::endl;
                                // printf("\nAVIN_DEBUG_QUERY_change01 ");
                                expiredKeys.push_back(key);
                            }
                        }else{
                            queryToVersion[key] = std::stoi(version);
                            // std::cout << "\nAVIN_DEBUG_QUERY_change02" << std::endl;
                            // printf("\nAVIN_DEBUG_QUERY_change02 ");
                            expiredKeys.push_back(key);
                        }
                    }
                }
                return expiredKeys;
            }

            static void initializeUpidsMap(){
                if(isUpidsTtlExpiredPassed()){
                    std::map<std::string, std::string> upidsMap =
                        zkStoreUpIdsReader->hgetall("upid_service_map");
                    if(upidsMap.size() > 0){
                        for (const auto& upidPair : upidsMap) {
                            std::string upid = upidPair.first;
                            std::string value = upidPair.second;
                            std::cout << "\nzk-log/manager upid " << upid << " value " << value << std::endl;
                        }
                    }else{
                        std::cout << "\nzk-log/manager no upids" << std::endl;
                    }
                    upidsServiceMap = upidsMap;
                }
            }

            static void initializeQueriesV2(){
                if(isTtlExpiredPassed()){
                    // std::cout << "\nAVIN_DEBUG_QUERY_init01" << std::endl;
                    //1 - Identify changed scenarios
                    std::vector<std::string> changedScenarios = identifyChangedScenarios();

                    //1.2 - Extract the attributes from redis
                    std::map<std::string, std::map<std::string, std::string>> protocolToAttributesMap;
                    std::map<std::string, std::string> attributesMap = zkStoreAttributedReader->hgetall("EBPF_0.1.0-alpha_HTTP");
                    protocolToAttributesMap["HTTP"] = attributesMap;
                    // print the attributesMap map
                    std::cout << "\nzk-log/manager found attributes: " << std::endl;
                    for (const auto& attribute : attributesMap) {
                        std::cout << "\nzk-log/manager attribute " << attribute.first.c_str() << " value " << attribute.second.c_str() << std::endl;
                    }

                    // For testing ebpf attributes
                    //  protocolToAttributesMap["HTTP"]["http_req_headers"] =
                    //  "req_headers.#extractJSON(\"/Host\")";

                    //1.5 - Check for the size of changedScenarios and return if it is 0
                    if(changedScenarios.size() == 0){
                        std::cout << "\nzk-log/manager scenarios changed "
                                  << 0 << std::endl;
                        return;
                    }else{
                        std::cout << "\nzk-log/manager scenarios changed "
                                  << changedScenarios.size() << std::endl;
                    }
                    //changedScenarios.push_back("2023");
                    //2 - for each such scenario, get the scenairo json from redis
                    for (const auto& scenairo : changedScenarios) {
                        // std::cout << "\nAVIN_DEBUG_QUERY_init02 ScenarioId processed " << scenairo.c_str() << std::endl;
                        // printf("\nAVIN_DEBUG_QUERY_init02 ScenarioId processed - %s", scenairo.c_str());
                        //2.5 - Clear the queries corresponding to the scenario from protocolToScenarioToQueries for all the query types
                        for (const auto& queryTypeStringPair : protocolToScenarioToQueries) {
                            std::string queryTypeString = queryTypeStringPair.first;
                            std::map<std::string, std::vector<Query*> > scenarioToQueries = queryTypeStringPair.second;
                            if(scenarioToQueries.count(scenairo) > 0){
                                std::vector<Query*>& queries = scenarioToQueries[scenairo];

                                for (Query* query : queries) {
                                  delete query;  // Delete the object pointed by the raw pointer
                                }

                                queries.clear();  // Clear the vector after deleting the objects
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
                        std::cout << "\nzk-log/manager " << scenairo << " queries found: " << queries.size() << std::endl;
                        // if(scenairo == "2023"){
                        //     std::cout << "\nAVIN_DEBUG_QUERY_init02 ScenarioId processed " << scenairo.c_str() << " query.workloadId " << queries[0]->workloadId.c_str() << std::endl;
                        // }
                        //4 - for each query, check if the query is allowed as per the possibleIdentifiers set
                        for (const auto& query : queries) {
                            std::string identifier = query->ns + "/" + query->service;
                            // if (possibleIdentifiers.count(identifier) > 0){
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
                            // std::cout << "\nAVIN_DEBUG_QUERY_init03 " << std::endl;
                            // printf("\nAVIN_DEBUG_QUERY_init03 ");
                            protocolToScenarioToQueries[queryTypeString][scenairo].push_back(query);
                            std::cout << "\nzk-log/manager " << " mapped a query for " << scenairo << std::endl;
                            // }
                        }
                    }

                    // std::cout << "\nAVIN_DEBUG_QUERY_init04 " << std::endl;
                    // printf("\nAVIN_DEBUG_QUERY_init04 ");
                    std::cout << "\nzk-log/manager " << " queries cleared " << std::endl;
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
                                // std::cout << "\nAVIN_DEBUG_QUERY_init05 " << std::endl;
                                // printf("\nAVIN_DEBUG_QUERY_init05 ");
                                protocolToQueries[queryTypeString].push_back(query);
                                std::cout << "\nzk-log/manager " << " queries saved " << std::endl;
                            }
                        }
                    }
                }
            }

            static void init(){
                if(!storeInitializedOnce){
                    // std::cout << "\nAVIN_DEBUG_QUERY_init00 " << std::endl;
                    storeInitializedOnce = true;
                    ttlForRedisCheckInMilliseconds = 300000;
                    ttlForUpidsCheckInMilliseconds = 60000;
                    zkStoreReader = zk::ZkStoreProvider::instance(2);
                    zkStoreWriter = zk::ZkStoreProvider::instance(1);
                    zkStoreAttributedReader = zk::ZkStoreProvider::instance(4);
                    zkStoreUpIdsReader = zk::ZkStoreProvider::instance(5);
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
                bool upIdReaderConnected = zkStoreUpIdsReader->connect();
                if (!upIdReaderConnected) {
                    zkStoreUpIdsReader->connect();
                }
            }

        public:
            static bool storeInitializedOnce;
            static std::map<std::string, std::string> upidsServiceMap;
            static zk::ZkStore* zkStoreReader;
            static zk::ZkStore* zkStoreWriter;
            static zk::ZkStore* zkStoreAttributedReader;
            static zk::ZkStore* zkStoreUpIdsReader;
            static std::string uuid;
            static long lastTimestampInMilliseconds;
            static long lastTimestampUpidsSyncInMilliseconds;
            static long ttlForRedisCheckInMilliseconds;
            static long ttlForUpidsCheckInMilliseconds;
            static std::set<std::string> possibleIdentifiers;
            static std::map<std::string, std::vector<Query*> > protocolToQueries;
            static std::map<std::string, int > queryToVersion;
            static std::map<std::string, std::map<std::string, std::vector<Query*> > > protocolToScenarioToQueries;

            static void refresh(){
                init();
                initializeQueriesV2();
                initializeUpidsMap();
            }

            static void get(){
                init();
                initializeQueriesV2();
                initializeUpidsMap();
            }

    };

    std::set<std::string> ZkQueryManager::possibleIdentifiers;
    std::map<std::string, std::vector<Query*> > ZkQueryManager::protocolToQueries;
    zk::ZkStore* ZkQueryManager::zkStoreReader; 
    zk::ZkStore* ZkQueryManager::zkStoreWriter; 
    zk::ZkStore* ZkQueryManager::zkStoreAttributedReader; 
    zk::ZkStore* ZkQueryManager::zkStoreUpIdsReader; 
    std::string ZkQueryManager::uuid;
    std::map<std::string, std::map<std::string, std::vector<Query*> > > ZkQueryManager::protocolToScenarioToQueries;
    std::map<std::string, std::string> ZkQueryManager::upidsServiceMap;
    bool ZkQueryManager::storeInitializedOnce; 
    long ZkQueryManager::lastTimestampInMilliseconds = 0;
    long ZkQueryManager::lastTimestampUpidsSyncInMilliseconds = 0;
    long ZkQueryManager::ttlForRedisCheckInMilliseconds;
    long ZkQueryManager::ttlForUpidsCheckInMilliseconds;
    std::map<std::string, int > ZkQueryManager::queryToVersion;
}