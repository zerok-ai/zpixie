#pragma once

#include "Rule.h"

namespace zk {
    class SimpleRuleDefault : public SimpleRule {
    public:
        bool evaluateEquals(std::map<std::string, std::string> propsMap) const override{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        };
        bool evaluateNotEquals(std::map<std::string, std::string> propsMap) const override{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        };
        bool evaluateIn(std::map<std::string, std::string> propsMap) const override{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        };
        
        bool evaluateNotIn(std::map<std::string, std::string> propsMap) const override{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        };

        bool evaluateLessThan(std::map<std::string, std::string> propsMap) const override{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        }

        bool evaluateLessThanEquals(std::map<std::string, std::string> propsMap) const override{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        }

        bool evaluateGreaterThan(std::map<std::string, std::string> propsMap) const override{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        }

        bool evaluateGreaterThanEquals(std::map<std::string, std::string> propsMap) const override{
            (void)propsMap; // Cast to void to suppress the warning/error
            return false;
        }
    };
}