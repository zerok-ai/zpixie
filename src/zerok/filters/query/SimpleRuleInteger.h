#pragma once

#include <string>
#include "SimpleRuleDefault.h"

namespace zk {
class SimpleRuleInteger : public SimpleRuleDefault {
 public:
  long value;

  bool evaluateEquals(std::map<std::string, std::string> propsMap) const {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      long foundValue = std::stol(this->extractValue(propsMap));
      return foundValue == value;
    }
    return false;
  };

  bool evaluateNotEquals(std::map<std::string, std::string> propsMap) const {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      long foundValue = std::stol(this->extractValue(propsMap));
      return foundValue != value;
    }
    return false;
  };

  bool evaluateLessThan(std::map<std::string, std::string> propsMap) const {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      long foundValue = std::stol(this->extractValue(propsMap));
      return foundValue < value;
    }
    return false;
  }

  bool evaluateLessThanEquals(std::map<std::string, std::string> propsMap) const {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      long foundValue = std::stol(this->extractValue(propsMap));
      return foundValue <= value;
    }
    return false;
  }

  bool evaluateGreaterThan(std::map<std::string, std::string> propsMap) const {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      long foundValue = std::stol(this->extractValue(propsMap));
      return foundValue > value;
    }
    return false;
  }

  bool evaluateGreaterThanEquals(std::map<std::string, std::string> propsMap) const {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      long foundValue = std::stol(this->extractValue(propsMap));
      return foundValue >= value;
    }
    return false;
  }
};
}  // namespace zk