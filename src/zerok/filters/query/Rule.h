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
#include "CompositeRule.h"
#include "SimpleRule.h"

namespace zk {
class Rule {
 public:
  bool evaluate(std::map<std::string, std::string> propsMap) const {
    const Rule* basePtr = this;

    if (CompositeRule* derivedPtr = dynamic_cast<CompositeRule*>(basePtr)) {
      return derivedPtr->evaluate(propsMap);
    } else if (SimpleRule* derived2Ptr = dynamic_cast<SimpleRule*>(basePtr)) {
      return derived2Ptr->evaluate(propsMap);
    }

    return false;
  };
  bool isInitialized() const {
    const Rule* basePtr = this;

    if (CompositeRule* derivedPtr = dynamic_cast<CompositeRule*>(basePtr)) {
      return derivedPtr->isInitialized();
    } else if (SimpleRule* derived2Ptr = dynamic_cast<SimpleRule*>(basePtr)) {
      return derived2Ptr->isInitialized();
    }

    return false;
  };
};
}  // namespace zk