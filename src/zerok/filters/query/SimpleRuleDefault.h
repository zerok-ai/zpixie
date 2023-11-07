#pragma once

#include <iostream>
#include <random>
#include <string>
#include "SimpleRule.h"

namespace zk {
    class SimpleRuleDefault : public SimpleRule {
    public:
        bool evaluateExists(std::map<std::string, std::string> propsMap) const{
            std::string resolvedId = this->extractId();
            //add a debug log
            std::cout << "zk-log/rule id: " << id << ", resolvedId: " << resolvedId << std::endl;
            if(propsMap.count(resolvedId)){
                return true;
            }
            return false;
        };
        bool evaluateNotExists(std::map<std::string, std::string> propsMap) const{
            return evaluateExists(propsMap) ? false : true;
        };
        bool evaluateEquals(std::map<std::string, std::string> propsMap) const{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        };
        bool evaluateNotEquals(std::map<std::string, std::string> propsMap) const{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        };
        bool evaluateIn(std::map<std::string, std::string> propsMap) const{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        };
        
        bool evaluateNotIn(std::map<std::string, std::string> propsMap) const{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        };

        bool evaluateLessThan(std::map<std::string, std::string> propsMap) const{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        }

        bool evaluateLessThanEquals(std::map<std::string, std::string> propsMap) const{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        }

        bool evaluateGreaterThan(std::map<std::string, std::string> propsMap) const{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        }

        bool evaluateGreaterThanEquals(std::map<std::string, std::string> propsMap) const{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        }
    };
}