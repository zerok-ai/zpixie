#pragma once

#include "Rule.h"
#include "Query.h"
#include "ConditionType.h"
#include "FieldType.h"
#include "SimpleRuleInteger.h"
#include "SimpleRuleString.h"
#include "SimpleRuleKeyValue.h"
#include <iostream>
#include <random>
#include <string>

namespace zk {
    class QueryBuilder{
        public:
            static std::string generateRandomString(int length) {
                const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
                const int charactersLength = characters.length();

                std::random_device rd;
                std::mt19937 generator(rd());

                std::string randomString;
                for (int i = 0; i < length; ++i) {
                    randomString += characters[generator() % charactersLength];
                }

                return randomString;
            }
            static std::vector<Query*> extractQueriesFromScenario(const rapidjson::Value& scenarioDoc){
                std::vector<Query*> vector;

                if (!scenarioDoc.IsObject()) {
                    std::cout << "AVIN_DEBUG_ QueryBuilder Invalid JSON format. Expected an object." << " line: " << __LINE__ << std::endl;
                    return vector; // Return empty vector if JSON is not an object
                }

                if (!scenarioDoc.HasMember("workloads")) {
                    std::cout << "AVIN_DEBUG_ QueryBuilder Missing 'workloads' field in the JSON." << " line: " << __LINE__ << std::endl;
                    return vector; // Return empty vector if 'workloads' field is missing
                }

                const rapidjson::Value& workloadsDoc = scenarioDoc["workloads"];
                if (!workloadsDoc.IsObject()) {
                    std::cout << "AVIN_DEBUG_ QueryBuilder Invalid JSON format. 'workloads' field is not an object." << " line: " << __LINE__ << std::endl;
                    return vector; // Return empty vector if 'workloads' is not an object
                }

                for (auto& member : workloadsDoc.GetObject()) {
                    const char* key = member.name.GetString();
                    const rapidjson::Value& workloadDoc = workloadsDoc[key];
                    Query* query = parseWorkload(key, workloadDoc);

                    if (query) {
                        std::string keyString(key);
                        query->workloadId = keyString;
                        vector.push_back(query);
                    } else {
                        std::cout << "AVIN_DEBUG_ QueryBuilder Failed to parse workload with key: " << key << " line: " << __LINE__ << std::endl;
                        // Log warning or perform error handling as necessary
                    }
                }

                return vector;
            }

            static std::vector<Query*> extractQueriesFromScenario(const char* jsonRule){
                if (jsonRule == nullptr) {
                    std::cout << "AVIN_DEBUG_ QueryBuilder JSON rule is nullptr." << " line: " << __LINE__ << std::endl;
                    return {}; // Return an empty vector or handle the error case appropriately
                }
                rapidjson::Document scenarioDoc;
                scenarioDoc.Parse(jsonRule);
                return extractQueriesFromScenario(scenarioDoc);
                // rapidjson::Document scenarioDoc;
                // scenarioDoc.Parse(jsonRule);

                // std::vector<Query*> vector;

                // if (scenarioDoc.HasParseError()) {
                //     std::cout << "AVIN_DEBUG_ QueryBuilder JSON parse error: " << GetParseError_En(scenarioDoc.GetParseError()) << std::endl;
                //     return vector; // Return empty vector if JSON parsing failed
                // }

                // if (!scenarioDoc.IsObject()) {
                //     std::cout << "AVIN_DEBUG_ QueryBuilder Invalid JSON format. Expected an object." << std::endl;
                //     return vector; // Return empty vector if JSON is not an object
                // }

                // if (!scenarioDoc.HasMember("workloads")) {
                //     std::cout << "AVIN_DEBUG_ QueryBuilder Missing 'workloads' field in the JSON." << std::endl;
                //     return vector; // Return empty vector if 'workloads' field is missing
                // }

                // rapidjson::Value& workloadsDoc = scenarioDoc["workloads"];
                // if (!workloadsDoc.IsObject()) {
                //     std::cout << "AVIN_DEBUG_ QueryBuilder Invalid JSON format. 'workloads' field is not an object." << std::endl;
                //     return vector; // Return empty vector if 'workloads' is not an object
                // }

                // for (auto& member : workloadsDoc.GetObject()) {
                //     const char* key = member.name.GetString();
                //     rapidjson::Value& workloadDoc = workloadsDoc[key];
                //     Query* query = parseWorkload(key, workloadDoc);

                //     if (query) {
                //         std::string keyString(key);
                //         query->workloadId = keyString;
                //         vector.push_back(query);
                //     } else {
                //         std::cout << "AVIN_DEBUG_ QueryBuilder Failed to parse workload with key: " << key << std::endl;
                //         // Log warning or perform error handling as necessary
                //     }
                // }

                // return vector;
            }
            
            static std::vector<Query*> parseScenarios(const char* jsonRule){
                rapidjson::Document doc;
                doc.Parse(jsonRule);

                std::vector<Query*> vector;
                rapidjson::Value& scenariosDoc = doc["scenarios"];
                int scenariosSize = static_cast<int>(scenariosDoc.Size());
                for (int i = 0; i < scenariosSize; i++) {
                    rapidjson::Value& scenarioDoc = scenariosDoc[i];
                    std::vector<Query*> queriesFromOneScenario = extractQueriesFromScenario(scenarioDoc);
                    vector.insert(vector.end(), queriesFromOneScenario.begin(), queriesFromOneScenario.end());
                }
                return vector;
            }

            static Query* parseWorkload(const char* key, const rapidjson::Value& doc){
                Query* query = nullptr;

                if (!doc.IsObject()) {
                    std::cout << "AVIN_DEBUG_ QueryBuilder Invalid JSON format for workload. Expected an object." << " line: " << __LINE__ << std::endl;
                    return query; // Return nullptr if JSON is not an object
                }

                query = parseQuery(doc);
                if (query) {
                    std::string keyString(key);
                    query->workloadId = keyString;
                } else {
                    std::cout << "AVIN_DEBUG_ QueryBuilder Failed to parse workload with key: " << key << " line: " << __LINE__ << std::endl;
                    // Log warning or perform error handling as necessary
                }

                return query;
            }

            static Query* parseQuery(const rapidjson::Value& doc){
                Query* parsedQuery = nullptr;

                if (!doc.IsObject()) {
                    std::cout << "AVIN_DEBUG_ QueryBuilder Invalid JSON format for query. Expected an object." << " line: " << __LINE__ << std::endl;
                    return parsedQuery; // Return nullptr if JSON is not an object
                }

                // Check for required fields in the JSON object
                if (!doc.HasMember("protocol") || !doc.HasMember("trace_role") || !doc.HasMember("service") || !doc.HasMember("rule")) {
                    std::cout << "AVIN_DEBUG_ QueryBuilder Missing required fields in the query JSON." << " line: " << __LINE__ << std::endl;
                    return parsedQuery; // Return nullptr if any required field is missing
                }

                std::string protocolString = doc["protocol"].GetString();
                std::string traceRoleString = doc["trace_role"].GetString();
                std::string serviceString = doc["service"].GetString();
                std::vector<std::string> splits = CommonUtils::splitString(serviceString, "/");
                std::string ns = splits.at(0);
                std::string service = splits.at(1);
                parsedQuery->traceRole = traceRoleString;
                parsedQuery->queryType = queryTypeMap[protocolString];
                parsedQuery->ns = ns;
                parsedQuery->service = service;

                // Handle parsing errors and log warnings if necessary
                CompositeRule* andRule = new CompositeRule();
                andRule->condition = conditionTypeMap["AND"];
                SimpleRuleString* traceRule = new SimpleRuleString();
                traceRule->id = "trace_role";
                traceRule->type = STRING;
                traceRule->input = "string";
                traceRule->value = parsedQuery->traceRole;
                andRule->rules.push_back(traceRule);
                //////////
                const rapidjson::Value& ruleDoc = doc["rule"];
                Rule* parsedRule = parse(ruleDoc);
                andRule->rules.push_back(parsedRule);

                parsedQuery->rule = andRule;
                return parsedQuery;
            }

        private:
            static Rule* parse(const rapidjson::Value& doc){
                Rule* parsedRule;
                bool isCompositeRule = doc.HasMember("condition");
                if(isCompositeRule){
                    parsedRule = parseCompositeRule(doc);
                }else{
                    parsedRule = parseSimpleRule(doc);
                }

                return parsedRule;
            }

            static Rule* parseCompositeRule(const rapidjson::Value& compositeRuleDoc){
                CompositeRule* rule = new CompositeRule();
                rule->condition = conditionTypeMap[compositeRuleDoc["condition"].GetString()];
                const rapidjson::Value& rulesDoc = compositeRuleDoc["rules"];
                std::vector<Rule*> vector;
                int rulesDocSize = static_cast<int>(rulesDoc.Size());
                for (int i = 0; i < rulesDocSize; i++) {
                    Rule* rule = parse(rulesDoc[i]);
                    vector.push_back(rule);
                }
                rule->rules = vector;
                return rule;
            }

            static Rule* parseWorkloadIdentifierRule(const rapidjson::Value& ruleDoc){
                CompositeRule* rule = new CompositeRule();
                rule->condition = AND;
                
                const rapidjson::Value& value = ruleDoc["value"];
                std::string sourceOrDestination = ruleDoc["id"].GetString();

                //TraceRule
                SimpleRuleString* traceRule = new SimpleRuleString();
                traceRule->id = "trace_role";
                traceRule->type = STRING;
                traceRule->input = "string";
                if(sourceOrDestination == "source"){
                    traceRule->value = "server";
                }else{
                    traceRule->value = "client";
                }
                
                rule->rules.push_back(traceRule);

                //delete once done:
                // for (auto r : rule->rules) {
                //     delete r;
                // }

                //remote_addr
                if(value.HasMember("ip")){
                    SimpleRuleString* ipRule = new SimpleRuleString();
                    ipRule->id = "remote_addr";
                    ipRule->type = STRING;
                    ipRule->input = "string";
                    ipRule->value = "10.0.0.4";
                    rule->rules.push_back(ipRule);
                }

                return rule;
            }

            static Rule* parseSimpleRule(const rapidjson::Value& ruleDoc){
                SimpleRule* rule = nullptr;
                FieldType fieldType = fieldTypeMap[ruleDoc["datatype"].GetString()];

                if (fieldType == STRING){
                    rule = new SimpleRuleString();
                    ((SimpleRuleString*)rule)->value = ruleDoc["value"].GetString();
                }else if(fieldType == INTEGER){
                    rule = new SimpleRuleInteger();
                    // ((SimpleRuleInteger*)rule)->value = ruleDoc["value"].GetInt();

                    if (ruleDoc["value"].IsString()) {
                        std::string value = ruleDoc["value"].GetString();
                        int valueInt = std::stoi(value);
                        ((SimpleRuleInteger*)rule)->value = valueInt;
                    }
                }else if(fieldType == KEY_MAP){
                    rule = new SimpleRuleKeyValue();
                    ((SimpleRuleKeyValue*)rule)->value = ruleDoc["value"].GetString();
                }else if(fieldType == WORKLOAD_IDENTIFIER){
                    return parseWorkloadIdentifierRule(ruleDoc);
                }else{
                    return rule;
                }

                rule->id = ruleDoc["id"].GetString();
                rule->type = fieldType;
                if (ruleDoc.HasMember("key")){
                    rule->key = ruleDoc["key"].GetString();
                }
                
                rule->operatorType = operatorTypeMap[ruleDoc["operator"].GetString()];
                return rule;
            }

    };
}