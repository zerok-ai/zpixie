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
            FieldType type;
            std::string input;
            OperatorType operatorType;

            bool evaluate(std::map<std::string, std::string> propsMap) const override{
                switch (operatorType)
                {
                    case EQUALS:
                        return evaluateEquals(propsMap);
                    
                    case NOT_EQUALS:
                        return evaluateNotEquals(propsMap);
                    
                    default:
                        break;
                }

                return false;
            }

            virtual bool evaluateEquals(std::map<std::string, std::string> propsMap) const = 0;
            virtual bool evaluateNotEquals(std::map<std::string, std::string> propsMap) const = 0;
    };
}