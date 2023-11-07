#pragma once

#include <iostream>
#include <random>
#include <string>
#include "Rule.h"

namespace zk {
    class SimpleRuleDefault : public SimpleRule {
    public:
        bool evaluateExists(const std::map<std::string, std::string>& propsMap) const override{
            std::string resolvedId = this->extractId();
            //add a debug log
            std::cout << "zk-log/rule id: " << id << ", resolvedId: " << resolvedId << std::endl;
            if(propsMap.count(resolvedId)){
                return true;
            }
            return false;
        };
        bool evaluateNotExists(const std::map<std::string, std::string>& propsMap) const override{
            return evaluateExists(propsMap) ? false : true;
        };
        bool evaluateEquals(const std::map<std::string, std::string>& propsMap) const override{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        };
        bool evaluateNotEquals(const std::map<std::string, std::string>& propsMap) const override{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        };
        bool evaluateIn(const std::map<std::string, std::string>& propsMap) const override{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        };
        
        bool evaluateNotIn(const std::map<std::string, std::string>& propsMap) const override{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        };

        bool evaluateLessThan(const std::map<std::string, std::string>& propsMap) const override{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        }

        bool evaluateLessThanEquals(const std::map<std::string, std::string>& propsMap) const override{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        }

        bool evaluateGreaterThan(const std::map<std::string, std::string>& propsMap) const override{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        }

        bool evaluateGreaterThanEquals(const std::map<std::string, std::string>& propsMap) const override{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        }
    };
}