#pragma once

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "ConditionType.h"
#include "FieldType.h"
#include "OperatorType.h"
#include "Rule.h"

namespace zk {
class CompositeRule : public Rule {
 public:
  ConditionType condition = AND;
  std::vector<Rule> rules;

  bool isInitialized() const {
    return true;
  }

  bool evaluate(std::map<std::string, std::string> propsMap) const {
    if (condition == AND) {
      for (Rule rule : rules) {
        bool evaluationResult = rule.evaluate(propsMap);
        if (!evaluationResult) {
          return false;
        }
      }

      return true;
    } else if (condition == OR) {
      for (Rule rule : rules) {
        bool evaluationResult = rule.evaluate(propsMap);
        if (evaluationResult) {
          return true;
        }
      }

      return false;
    }

    return false;
  }
};
}  // namespace zk