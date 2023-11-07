#pragma once

#include <string>
#include "SimpleRuleDefault.h"

namespace zk {
class SimpleRuleInteger : public SimpleRuleDefault {
 public:
  long value;

  bool evaluateEquals(const std::map<std::string, std::string>& propsMap) const override {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      long foundValue = std::stol(this->extractValue(propsMap));
      return foundValue == value;
    }
    return false;
  };

  bool evaluateNotEquals(const std::map<std::string, std::string>& propsMap) const override {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      long foundValue = std::stol(this->extractValue(propsMap));
      return foundValue != value;
    }
    return false;
  };

  bool evaluateLessThan(const std::map<std::string, std::string>& propsMap) const override {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      long foundValue = std::stol(this->extractValue(propsMap));
      return foundValue < value;
    }
    return false;
  }

  bool evaluateLessThanEquals(const std::map<std::string, std::string>& propsMap) const override {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      long foundValue = std::stol(this->extractValue(propsMap));
      return foundValue <= value;
    }
    return false;
  }

  bool evaluateGreaterThan(const std::map<std::string, std::string>& propsMap) const override {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      long foundValue = std::stol(this->extractValue(propsMap));
      return foundValue > value;
    }
    return false;
  }

  bool evaluateGreaterThanEquals(const std::map<std::string, std::string>& propsMap) const override {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      long foundValue = std::stol(this->extractValue(propsMap));
      return foundValue >= value;
    }
    return false;
  }
};
}  // namespace zk