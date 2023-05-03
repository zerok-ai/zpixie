#pragma once

#include "SimpleRuleDefault.h"

namespace zk {
    class SimpleRuleString : public SimpleRuleDefault {
    public:
        std::string value;

        bool evaluateEquals(std::map<std::string, std::string> propsMap) const override{
            if(propsMap.count(id)){
                std::string foundValue = propsMap[id];
                return foundValue == value;
            }
            return false;
        };
        
        bool evaluateNotEquals(std::map<std::string, std::string> propsMap) const override{
            if(propsMap.count(id)){
                std::string foundValue = propsMap[id];
                return foundValue != value;
            }
            return false;
        };
    };
}