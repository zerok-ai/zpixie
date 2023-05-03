#pragma once

#include "Rule.h"
#include "Query.h"
#include "ConditionType.h"
#include "FieldType.h"
#include "SimpleRuleInteger.h"
#include "SimpleRuleString.h"
#include "SimpleRuleKeyValue.h"

namespace zk {
    class QueryBuilder{
        public:
            static Query* parseQuery(const char* jsonRule){
                rapidjson::Document doc;
                doc.Parse(jsonRule);
                Query* parsedQuery;
                parsedQuery = new Query();
                parsedQuery->queryType = queryTypeMap[doc["zk_request_type"].GetString()];
                parsedQuery->rule = parse(doc);

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
                for (int i = 0; i < rulesDoc.Size(); i++) {
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
                for (int i = 0; i < rulesDoc.Size(); i++) {
                    Rule* rule = parse(rulesDoc[i]);
                    vector.push_back(rule);
                }
                rule->rules = vector;
                return rule;
            }

            static Rule* parseWorkloadIdentifierRule(rapidjson::Document& ruleDoc){
                CompositeRule* rule;
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
                traceRule->value = "server";
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