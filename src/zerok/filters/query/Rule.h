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

namespace zk {
class Rule {
 public:
  virtual ~Rule() = default;
  virtual bool evaluate(std::map<std::string, std::string> propsMap) const = 0;
};

class CompositeRule : public Rule {
 public:
  ConditionType condition;
  std::vector<Rule*> rules;

  bool evaluate(std::map<std::string, std::string> propsMap) const override {
    if (condition == AND) {
      for (Rule* rule : rules) {
        bool evaluationResult = rule->evaluate(propsMap);
        if (!evaluationResult) {
          return false;
        }
      }

      return true;
    } else if (condition == OR) {
      for (Rule* rule : rules) {
        bool evaluationResult = rule->evaluate(propsMap);
        if (evaluationResult) {
          return true;
        }
      }

      return false;
    }

    return false;
  }
};

class SimpleRule : public Rule {
 public:
  std::string id;
  std::string key;
  std::string json_path;
  FieldType type;
  std::string input;
  OperatorType operatorType;

  // id can contain this string: req_body.#extractJSON("message").#upperCase()
  std::string evaluateIdAndExtractValue(std::map<std::string, std::string> propsMap) const {
    std::string idToEvaluate = extractId(id);
    std::string jsonPath = extractJsonPath(id);
    std::string foundValue = "";
    foundValue = extractValueFromJson(propsMap, idToEvaluate, jsonPath);

    if (json_path == "/traceparent") {
      std::cout << "\nAVIN_DEBUG_ATTRIBUTES_00 Id " << id << ", jsonPath " << jsonPath << std::endl;
    }
    if (id.find("#upperCase") != std::string::npos) {
      // convert to uppercase
      std::transform(foundValue.begin(), foundValue.end(), foundValue.begin(), ::toupper);
    }

    return foundValue;
  }

  std::string extractId(std::string id) const {
    std::string idToEvaluate = id;
    if (id.find("#") != std::string::npos) {
      idToEvaluate = id.substr(0, id.find("#") - 1);
    }

    return idToEvaluate;
  }

  std::string extractJsonPath(std::string id) const {
    std::string jsonPath = "";
    if (id.find("#extractJSON") != std::string::npos) {
      // extract json
      jsonPath = id.substr(id.find("(\"") + 2, id.find("\")") - id.find("(\"") - 2);
    }
    return jsonPath;
  }

  // function to extract value from json passed in the arguments
  // if json_path is empty then return the json as it is
  std::string extractValueFromJson(std::map<std::string, std::string> propsMap,
                                   std::string idToEvaluate, std::string jsonPath) const {
    if (propsMap.count(idToEvaluate)) {
      const std::string json = propsMap[idToEvaluate];
      std::string finalJsonPath = "";
      if (jsonPath == "") {
        finalJsonPath = json_path;
      } else {
        finalJsonPath = jsonPath;
      }
      if (finalJsonPath == "") {
        std::cout << "\nAVIN_DEBUG_ATTRIBUTES_01 Id " << idToEvaluate << ", jsonPath " << jsonPath
                  << ", json_path " << json_path << ", value " << json << std::endl;
        return json;
      }
      const char* jsonCstr = json.c_str();
      rapidjson::Document doc;
      doc.Parse(jsonCstr);

      const char* keyCstr = finalJsonPath.c_str();
      // https://rapidjson.org/md_doc_pointer.html
      rapidjson::Pointer pointer(keyCstr);

      // Extract the value using JSONPath
      if (!pointer.IsValid()) {
        if (json_path == "/traceparent") {
          std::cout << "\nAVIN_DEBUG_ATTRIBUTES_02 Id " << idToEvaluate << ", jsonPath " << jsonPath
                    << ", json_path " << json_path << ", value " << json << std::endl;
        }
        return "ZK_NULL";
      }

      const rapidjson::Value* result = pointer.Get(doc);
      if (result != nullptr && result->IsString()) {
        std::string foundValue = result->GetString();
        if (json_path == "/traceparent") {
          std::cout << "\nAVIN_DEBUG_ATTRIBUTES_03 Id " << idToEvaluate << ", jsonPath " << jsonPath
                    << ", json_path " << json_path << ", value " << foundValue << std::endl;
        }
        return foundValue;
      } else {
        if (json_path == "/traceparent") {
          std::cout << "\nAVIN_DEBUG_ATTRIBUTES_04 Id " << idToEvaluate << ", jsonPath " << jsonPath
                    << ", json_path " << json_path << ", value " << json << std::endl;
        }
        return "ZK_NULL";
      }
    }

    if (json_path == "/traceparent") {
      std::cout << "\nAVIN_DEBUG_ATTRIBUTES_05 Id " << idToEvaluate << ", jsonPath " << jsonPath
                << ", json_path " << json_path << ", value is null " << std::endl;
    }
    return "ZK_NULL";
  }

  std::string extractValue(std::map<std::string, std::string> propsMap) const {
    return evaluateIdAndExtractValue(propsMap);
  }

  bool evaluate(std::map<std::string, std::string> propsMap) const override {
    switch (operatorType) {
      case EXISTS:
        return evaluateExists(propsMap);

      case NOT_EXISTS:
        return evaluateNotExists(propsMap);

      case EQUALS:
        return evaluateEquals(propsMap);

      case NOT_EQUALS:
        return evaluateNotEquals(propsMap);

      case IN:
        return evaluateIn(propsMap);

      case NOT_IN:
        return evaluateNotIn(propsMap);

      case GREATER_THAN:
        return evaluateGreaterThan(propsMap);

      case GREATER_THAN_EQUALS:
        return evaluateGreaterThanEquals(propsMap);

      case LESS_THAN:
        return evaluateLessThan(propsMap);

      case LESS_THAN_EQUALS:
        return evaluateLessThanEquals(propsMap);

      default:
        break;
    }

    return false;
  }

  virtual bool evaluateExists(std::map<std::string, std::string> propsMap) const = 0;
  virtual bool evaluateNotExists(std::map<std::string, std::string> propsMap) const = 0;
  virtual bool evaluateEquals(std::map<std::string, std::string> propsMap) const = 0;
  virtual bool evaluateNotEquals(std::map<std::string, std::string> propsMap) const = 0;
  virtual bool evaluateIn(std::map<std::string, std::string> propsMap) const = 0;
  virtual bool evaluateNotIn(std::map<std::string, std::string> propsMap) const = 0;
  virtual bool evaluateLessThan(std::map<std::string, std::string> propsMap) const = 0;
  virtual bool evaluateLessThanEquals(std::map<std::string, std::string> propsMap) const = 0;
  virtual bool evaluateGreaterThan(std::map<std::string, std::string> propsMap) const = 0;
  virtual bool evaluateGreaterThanEquals(std::map<std::string, std::string> propsMap) const = 0;
};
}  // namespace zk