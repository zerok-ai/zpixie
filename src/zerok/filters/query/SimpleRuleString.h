#pragma once

// #include "../utils.h"
#include <algorithm>
#include "SimpleRuleDefault.h"
#include "src/zerok/common/utils.h"

namespace zk {
class SimpleRuleString : public SimpleRuleDefault {
 public:
  std::string value;

  bool evaluateEquals(std::map<std::string, std::string> propsMap) const {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      std::string foundValue = this->extractValue(propsMap);
      return foundValue == value;
    }
    return false;
  };

  bool evaluateNotEquals(std::map<std::string, std::string> propsMap) const {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      std::string foundValue = this->extractValue(propsMap);
      return foundValue != value;
    }
    return false;
  };

  bool evaluateIn(std::map<std::string, std::string> propsMap) const {
    bool exists = this->evaluateExists(propsMap);
    if (exists) {
      std::string foundValue = this->extractValue(propsMap);
      std::vector<std::string> splits = CommonUtils::splitString(value, ", ");

      return std::find(splits.begin(), splits.end(), foundValue) != splits.end();
    }
    return false;
  };

  bool evaluateNotIn(std::map<std::string, std::string> propsMap) const {
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