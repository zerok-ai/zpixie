#pragma once

#include "SimpleRuleDefault.h"

namespace zk {
    class SimpleRuleInteger : public SimpleRuleDefault {
    public:
        int value;

        bool evaluateEquals(std::map<std::string, std::string> propsMap) const override{
            if(propsMap.count(id)){
                int foundValue = std::stoi(propsMap[id]);
                return foundValue == value;
            }
            return false;
        };
        bool evaluateNotEquals(std::map<std::string, std::string> propsMap) const override{
            if(propsMap.count(id)){
                int foundValue = std::stoi(propsMap[id]);
                return foundValue != value;
            }
            return false;
        };
    };
}