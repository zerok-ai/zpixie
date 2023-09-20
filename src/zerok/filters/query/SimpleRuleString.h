#pragma once

// #include "../utils.h"
#include "SimpleRuleDefault.h"
#include <algorithm>
#include "src/zerok/common/utils.h"

namespace zk {
    class SimpleRuleString : public SimpleRuleDefault {
    public:
        std::string value;

        bool evaluateEquals(std::map<std::string, std::string> propsMap) const override{
            if(propsMap.count(id)){
                std::string foundValue = this->extractValueFromJson(propsMap);
                return foundValue == value;
            }
            return false;
        };
        
        bool evaluateNotEquals(std::map<std::string, std::string> propsMap) const override{
            if(propsMap.count(id)){
                std::string foundValue = this->extractValueFromJson(propsMap);
                return foundValue != value;
            }
            return false;
        };

        bool evaluateIn(std::map<std::string, std::string> propsMap) const override{
            if(propsMap.count(id)){
                std::string foundValue = this->extractValueFromJson(propsMap);
                std::vector<std::string> splits = CommonUtils::splitString(value, ", ");

                return std::find(splits.begin(), splits.end(), foundValue) != splits.end();
            }
            return false;
        };

        bool evaluateNotIn(std::map<std::string, std::string> propsMap) const override{
            if(propsMap.count(id)){
                std::string foundValue = this->extractValueFromJson(propsMap);
                std::vector<std::string> splits = CommonUtils::splitString(value, ", ");

                return std::find(splits.begin(), splits.end(), foundValue) == splits.end();
            }
            return false;
        };
    };
}