#pragma once

#include "Rule.h"
#include "Query.h"
#include "ConditionType.h"
#include "FieldType.h"
#include "SimpleRuleInteger.h"
#include "SimpleRuleString.h"
#include "SimpleRuleKeyValue.h"
// #include "src/zerok/store/store.h"
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
            static std::vector<Query*> parseQueries(const char* jsonRule){
                rapidjson::Document doc;
                doc.Parse(jsonRule);

                std::vector<Query*> vector;
                rapidjson::Value& rulesDoc = doc["rules"];
                int filtersSize = static_cast<int>(rulesDoc.Size());
                for (int i = 0; i < filtersSize; i++) {
                    rapidjson::Value& filterDoc = rulesDoc[i];
                    rapidjson::Value& workloadsDoc = filterDoc["workloads"];
                    for (auto& member : workloadsDoc.GetObject()) {
                        const char* key = member.name.GetString();
                        rapidjson::Value& workloadDoc = workloadsDoc[key];
                        // Query* query = parseWorkload(key, workloadDoc);
                        Query* query = parseQuery(workloadDoc);
                        std::string keyString(key);
                        query->workloadId = keyString;
                        vector.push_back(query);
                    }
                }
                return vector;
            }

            static Query* parseWorkload(const char* key, const rapidjson::Value& doc){
                // std::vector<Query*> vector;
                // int filtersSize = static_cast<int>(doc.Size());
                // for (int i = 0; i < filtersSize; i++) {
                    Query* query = parseQuery(doc);
                    std::string keyString(key);
                    query->workloadId = keyString;
                    // vector.push_back(query);
                // }
                return query;
            }

            static Query* parseQuery(const char* jsonRule){
                rapidjson::Document doc;
                doc.Parse(jsonRule);
                Query* parsedQuery;
                parsedQuery = new Query();
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
                parsedQuery->rule = parse(doc);
                return parsedQuery;
            }

            static Query* parseQuery(const rapidjson::Value& doc){
                Query* parsedQuery;
                parsedQuery = new Query();
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

                //////////
                CompositeRule* andRule = new CompositeRule();
                andRule->condition = conditionTypeMap["AND"];
                SimpleRuleString* traceRule = new SimpleRuleString();
                traceRule->id = "trace_role";
                traceRule->type = STRING;
                traceRule->input = "string";
                traceRule->value = parsedQuery->traceRole;
                andRule->rules.push_back(traceRule);
                //////////
                Rule* parsedRule = parse(doc);
                andRule->rules.push_back(parsedRule);

                parsedQuery->rule = andRule;
                return parsedQuery;
            }

        private:
            static Rule* parse(rapidjson::Document& doc){
                Rule* parsedRule;
                bool isCompositeRule = doc.HasMember("condition");
                if(isCompositeRule){
                    parsedRule = parseCompositeRule(doc);
                }else{
                    parsedRule = parseSimpleRule(doc);
                }

                return parsedRule;
            }

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

            static Rule* parseCompositeRule(rapidjson::Document& compositeRuleDoc){
                CompositeRule* rule = new CompositeRule();
                std::string conditionString = compositeRuleDoc["condition"].GetString();
                rule->condition = conditionTypeMap[conditionString];
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

            static Rule* parseWorkloadIdentifierRule(rapidjson::Document& ruleDoc){
                (void)ruleDoc; // Cast to void to suppress the warning/error
                CompositeRule* rule = nullptr;
                // rule->condition = ConditionType::OR;
                
                // rapidjson::Value value = ruleDoc["value"];
                // std::string sourceOrDestination = ruleDoc["id"].GetString();

                // //TraceRule
                // SimpleRuleString* traceRule;
                // traceRule->id = "trace_role";
                // traceRule->type = STRING;
                // traceRule->input = STRING;
                // traceRule->value = "server";
                // rule->rules.push_back(traceRule);

                // if(value.HasMember("ip")){
                //     SimpleRuleString* ipRule;
                //     ipRule->id = "remote_addr";
                //     ipRule->type = STRING;
                //     ipRule->input = STRING;
                //     ipRule->value = "10.0.0.4";
                //     //remote_addr
                //     rule->rules.push_back(ipRule);
                // }

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

            static Rule* parseSimpleRule(rapidjson::Document& ruleDoc){
                SimpleRule* rule;
                FieldType fieldType = fieldTypeMap[ruleDoc["type"].GetString()];
                switch(fieldType){
                    case STRING:
                        rule = new SimpleRuleString();
                        ((SimpleRuleString*)rule)->value = ruleDoc["value"].GetString();
                        break;
                    case INTEGER:
                        rule = new SimpleRuleInteger();
                        ((SimpleRuleInteger*)rule)->value = ruleDoc["value"].GetInt();
                        break;
                    case KEY_MAP:
                        rule = new SimpleRuleKeyValue();
                        ((SimpleRuleKeyValue*)rule)->value = ruleDoc["value"].GetString();
                        break;
                    case WORKLOAD_IDENTIFIER:
                        return parseWorkloadIdentifierRule(ruleDoc);
                        break;

                    default:
                        break;
                }
                // rule->field = ruleDoc["field"].GetString();
                rule->id = ruleDoc["id"].GetString();
                rule->type = fieldType;
                if (ruleDoc.HasMember("key")){
                    rule->key = ruleDoc["key"].GetString();
                }
                
                rule->operatorType = operatorTypeMap[ruleDoc["operator"].GetString()];
                return rule;
            }

            static Rule* parseSimpleRule(const rapidjson::Value& ruleDoc){
                SimpleRule* rule;
                std::string typeString = ruleDoc["type"].GetString();
                FieldType fieldType = fieldTypeMap[typeString];
                switch(fieldType){
                    case STRING:
                        rule = new SimpleRuleString();
                        ((SimpleRuleString*)rule)->value = ruleDoc["value"].GetString();
                        break;
                    case INTEGER:
                        rule = new SimpleRuleInteger();
                        ((SimpleRuleInteger*)rule)->value = ruleDoc["value"].GetInt();
                        break;
                    case KEY_MAP:
                        rule = new SimpleRuleKeyValue();
                        ((SimpleRuleKeyValue*)rule)->value = ruleDoc["value"].GetString();
                        break;
                    case WORKLOAD_IDENTIFIER:
                        return parseWorkloadIdentifierRule(ruleDoc);
                        break;

                    default:
                        break;
                }
                // rule->field = ruleDoc["field"].GetString();
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