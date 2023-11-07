#pragma once

// #include "../utils.h"
#include <algorithm>
#include "SimpleRuleDefault.h"
#include "src/zerok/common/utils.h"

namespace zk {
class SimpleRuleString : public SimpleRuleDefault {
 public:
  std::string value;

  bool evaluateEquals(const std::map<std::string, std::string>& propsMap) const override {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      std::string foundValue = this->extractValue(propsMap);
      return foundValue == value;
    }
    return false;
  };

  bool evaluateNotEquals(const std::map<std::string, std::string>& propsMap) const override {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      std::string foundValue = this->extractValue(propsMap);
      return foundValue != value;
    }
    return false;
  };

  bool evaluateIn(const std::map<std::string, std::string>& propsMap) const override {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      std::string foundValue = this->extractValue(propsMap);
      std::vector<std::string> splits = CommonUtils::splitString(value, ", ");

      return std::find(splits.begin(), splits.end(), foundValue) != splits.end();
    }
    return false;
  };

  bool evaluateNotIn(const std::map<std::string, std::string>& propsMap) const override {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      std::string foundValue = this->extractValue(propsMap);
      std::vector<std::string> splits = CommonUtils::splitString(value, ", ");

      return std::find(splits.begin(), splits.end(), foundValue) == splits.end();
    }
    return false;
  };
};
}  // namespace zk