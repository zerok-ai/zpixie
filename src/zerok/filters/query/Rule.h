#pragma once

#include <vector>
#include <string>
#include <map>
#include "ConditionType.h"
#include "FieldType.h"
#include "OperatorType.h"

namespace zk {
    class Rule{
        public:
            virtual ~Rule() = default;
            virtual bool evaluate(std::map<std::string, std::string> propsMap) const = 0;
    };

    class CompositeRule : public Rule {
         public:
            ConditionType condition;
            std::vector<Rule*> rules;

            bool evaluate(std::map<std::string, std::string> propsMap) const override{
                if(condition == AND){
                    for (Rule* rule : rules) {
                        bool evaluationResult = rule->evaluate(propsMap);
                        if(!evaluationResult){
                            return false;
                        }
                    }

                    return true;
                }
                else if(condition == OR){
                    for (Rule* rule : rules) {
                        bool evaluationResult = rule->evaluate(propsMap);
                        if(evaluationResult){
                            return true;
                        }
                    }

                    return false;
                }
                
                return false;
            }
    };

    class SimpleRule : public Rule {
        public:
            std::string id;
            std::string key;
            std::string json_path;
            FieldType type;
            std::string input;
            OperatorType operatorType;

            std::string extractValueFromJson(std::map<std::string, std::string> propsMap) const {
                if(propsMap.count(id)){
                    const std::string json = propsMap[id];
                    if(json_path == ""){
                        return json;
                    }
                    const char* jsonCstr = json.c_str();
                    rapidjson::Document doc;
                    doc.Parse(jsonCstr);

                    const char* keyCstr = json_path.c_str();
                    //https://rapidjson.org/md_doc_pointer.html
                    rapidjson::Pointer pointer(keyCstr);

                    // Extract the value using JSONPath
                    if (!pointer.IsValid()) {
                        return "ZK_NULL";
                    }

                    const rapidjson::Value* result = pointer.Get(doc);
                    if (result != nullptr && result->IsString()) {
                        std::string foundValue = result->GetString();
                        return foundValue;
                    }else{
                        return "ZK_NULL";
                    }
                }

                return "ZK_NULL";
            }

            bool evaluate(std::map<std::string, std::string> propsMap) const override{
                switch (operatorType)
                {
                    case EQUALS:
                        return evaluateEquals(propsMap);
                    
                    case NOT_EQUALS:
                        return evaluateNotEquals(propsMap);
                    
                    case IN:
                        return evaluateIn(propsMap);
                    
                    case NOT_IN:
                        return evaluateNotIn(propsMap);
                    
                    case GREATER_THAN:
                        return evaluateGreaterThan(propsMap);
                    
                    case GREATER_THAN_EQUALS:
                        return evaluateGreaterThanEquals(propsMap);
                    
                    case LESS_THAN:
                        return evaluateLessThan(propsMap);
                    
                    case LESS_THAN_EQUALS:
                        return evaluateLessThanEquals(propsMap);
                    
                    default:
                        break;
                }

                return false;
            }

            virtual bool evaluateEquals(std::map<std::string, std::string> propsMap) const = 0;
            virtual bool evaluateNotEquals(std::map<std::string, std::string> propsMap) const = 0;
            virtual bool evaluateIn(std::map<std::string, std::string> propsMap) const = 0;
            virtual bool evaluateNotIn(std::map<std::string, std::string> propsMap) const = 0;
            virtual bool evaluateLessThan(std::map<std::string, std::string> propsMap) const = 0;
            virtual bool evaluateLessThanEquals(std::map<std::string, std::string> propsMap) const = 0;
            virtual bool evaluateGreaterThan(std::map<std::string, std::string> propsMap) const = 0;
            virtual bool evaluateGreaterThanEquals(std::map<std::string, std::string> propsMap) const = 0;
    };
}