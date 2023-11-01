#pragma once

#include <iostream>
#include <memory>
#include <random>
#include <string>
#include "ConditionType.h"
#include "FieldType.h"
#include "Query.h"
#include "Rule.h"
#include "SimpleRuleInteger.h"
#include "SimpleRuleString.h"

namespace zk {
class QueryBuilder {
 public:
  // static std::string generateRandomString(int length) {
  //   const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  //   const int charactersLength = characters.length();

  //   std::random_device rd;
  //   std::mt19937 generator(rd());

  //   std::string randomString;
  //   for (int i = 0; i < length; ++i) {
  //     randomString += characters[generator() % charactersLength];
  //   }

  //   return randomString;
  // }

  static std::vector<std::unique_ptr<Query> > extractQueriesFromScenario(
      const char* jsonRule,
      const std::map<std::string, std::map<std::string, std::string>> protocolToAttributesMap) {
    if (jsonRule == nullptr) {
      // std::cout << "AVIN_DEBUG_ QueryBuilder JSON rule is nullptr."
      //           << " line: " << __LINE__ << std::endl;
      return {};  // Return an empty vector or handle the error case appropriately
    }
    rapidjson::Document scenarioDoc;
    scenarioDoc.Parse(jsonRule);
    return extractQueriesFromScenario(scenarioDoc, protocolToAttributesMap);
  }

 private:
  static std::vector<std::unique_ptr<Query>> extractQueriesFromScenario(
      const rapidjson::Value& scenarioDoc,
      const std::map<std::string, std::map<std::string, std::string>> protocolToAttributesMap) {
    std::vector<std::unique_ptr<Query>> vector;

    if (!scenarioDoc.IsObject()) {
      // std::cout << "AVIN_DEBUG_ QueryBuilder Invalid JSON format. Expected an object."
      //           << " line: " << __LINE__ << std::endl;
      return vector;  // Return empty vector if JSON is not an object
    }

    if (!scenarioDoc.HasMember("workloads")) {
      // std::cout << "AVIN_DEBUG_ QueryBuilder Missing 'workloads' field in the JSON."
      //           << " line: " << __LINE__ << std::endl;
      return vector;  // Return empty vector if 'workloads' field is missing
    }

    const rapidjson::Value& workloadsDoc = scenarioDoc["workloads"];
    if (!workloadsDoc.IsObject()) {
      // std::cout
      //     << "AVIN_DEBUG_ QueryBuilder Invalid JSON format. 'workloads' field is not an object."
      //     << " line: " << __LINE__ << std::endl;
      return vector;  // Return empty vector if 'workloads' is not an object
    }

    for (auto& member : workloadsDoc.GetObject()) {
      const char* key = member.name.GetString();
      const rapidjson::Value& workloadDoc = workloadsDoc[key];
      std::unique_ptr<Query> query = parseWorkload(key, workloadDoc, protocolToAttributesMap);

      if (query) {
        std::string keyString(key);
        query->workloadId = keyString;
        vector.push_back(std::move(query));
      } else {
        std::cout << "zk-log/builder QueryBuilder Failed to parse workload with key: " << key
                  << " line: " << __LINE__ << std::endl;
        // Log warning or perform error handling as necessary
      }
    }

    return vector;
  }
  
  static std::unique_ptr<Query> parseWorkload(
      const char* key, const rapidjson::Value& doc,
      const std::map<std::string, std::map<std::string, std::string>> protocolToAttributesMap) {
    std::unique_ptr<Query> query = nullptr;

    if (!doc.IsObject()) {
      // std::cout << "AVIN_DEBUG_ QueryBuilder Invalid JSON format for workload. Expected an
      // object."
      //           << " line: " << __LINE__ << std::endl;
      return query;  // Return nullptr if JSON is not an object
    }

    query = parseQuery(doc, protocolToAttributesMap);
    if (query) {
      std::string keyString(key);
      query->workloadId = keyString;
    } else {
      // std::cout << "AVIN_DEBUG_ QueryBuilder Failed to parse workload with key: " << key
      //           << " line: " << __LINE__ << std::endl;
      // Log warning or perform error handling as necessary
    }

    return query;
  }

  static std::unique_ptr<Query> parseQuery(
      const rapidjson::Value& doc,
      const std::map<std::string, std::map<std::string, std::string>> protocolToAttributesMap) {
    std::unique_ptr<Query> parsedQuery = nullptr;

    if (!doc.IsObject()) {
      // std::cout << "AVIN_DEBUG_ QueryBuilder Invalid JSON format for query. Expected an object."
      //           << " line: " << __LINE__ << std::endl;
      return parsedQuery;  // Return nullptr if JSON is not an object
    }

    // Check for required fields in the JSON object
    if (!doc.HasMember("protocol") || !doc.HasMember("trace_role") || !doc.HasMember("service") ||
        !doc.HasMember("rule")) {
      // std::cout << "AVIN_DEBUG_ QueryBuilder Missing required fields in the query JSON."
      //           << " line: " << __LINE__ << std::endl;
      return parsedQuery;  // Return nullptr if any required field is missing
    }

    // parsedQuery = std::unique_ptr<Query>(new Query());
    parsedQuery = std::make_unique<Query>();
    std::string protocolString = doc["protocol"].GetString();
    std::map<std::string, std::string> attributesMap = protocolToAttributesMap.at(protocolString);
    std::string traceRoleString = doc["trace_role"].GetString();
    std::string serviceString = doc["service"].GetString();
    std::vector<std::string> splits = CommonUtils::splitString(serviceString, "/");
    std::string ns = splits.at(0);
    std::string service = splits.at(1);
    parsedQuery->traceRole = traceRoleString;
    parsedQuery->queryType = queryTypeMap[protocolString];
    parsedQuery->ns = ns;
    parsedQuery->service = service;

    // Handle parsing errors and log warnings if necessary
    // std::unique_ptr<CompositeRule> andRule = std::make_unique<CompositeRule>();
    std::unique_ptr<Rule> andRule = std::make_unique<CompositeRule>();
    // andRule->condition = conditionTypeMap["AND"];
    static_cast<CompositeRule*>(andRule.get())->condition = conditionTypeMap["AND"];
    std::unique_ptr<Rule> traceRule = std::make_unique<SimpleRuleString>();
    // std::unique_ptr<SimpleRuleString>(new SimpleRuleString());
    // traceRule->id = "trace_role";
    // traceRule->type = STRING;
    // traceRule->input = "string";
    // traceRule->value = parsedQuery->traceRole;
    static_cast<SimpleRuleString*>(traceRule.get())->id = "trace_role";
    static_cast<SimpleRuleString*>(traceRule.get())->type = STRING;
    static_cast<SimpleRuleString*>(traceRule.get())->input = "string";
    static_cast<SimpleRuleString*>(traceRule.get())->value = parsedQuery->traceRole;
    static_cast<CompositeRule*>(andRule.get())->rules.push_back(traceRule);
    //////////
    const rapidjson::Value& ruleDoc = doc["rule"];
    std::unique_ptr<Rule> parsedRule = parse(ruleDoc, attributesMap);
    if (parsedRule != nullptr) {
      static_cast<CompositeRule*>(andRule.get())->rules.push_back(parsedRule);
    }

    parsedQuery->rule = std::move(andRule);
    return parsedQuery;
  }

  static std::unique_ptr<Rule> parse(
      const rapidjson::Value& doc, const std::map<std::string, std::string> attributesMap) {
    std::unique_ptr<Rule> parsedRule;
    bool isCompositeRule = doc.HasMember("condition");
    if (isCompositeRule) {
      parsedRule = parseCompositeRule(doc, attributesMap);
    } else {
      parsedRule = parseSimpleRule(doc, attributesMap);
    }

    return parsedRule;
  }

  static std::unique_ptr<CompositeRule> parseCompositeRule(
      const rapidjson::Value& compositeRuleDoc,
      const std::map<std::string, std::string> attributesMap) {
    std::unique_ptr<CompositeRule> rule = std::make_unique<CompositeRule>();
    rule->condition = conditionTypeMap[compositeRuleDoc["condition"].GetString()];
    const rapidjson::Value& rulesDoc = compositeRuleDoc["rules"];
    std::vector<std::unique_ptr<Rule>> vector;
    int rulesDocSize = static_cast<int>(rulesDoc.Size());
    for (int i = 0; i < rulesDocSize; i++) {
      std::unique_ptr<Rule> rule = parse(rulesDoc[i], attributesMap);
      if (rule != nullptr) {
        vector.push_back(rule);
      }
    }
    rule->rules = std::move(vector);
    return rule;
  }

  static std::unique_ptr<SimpleRule> parseSimpleRule(
      const rapidjson::Value& ruleDoc, const std::map<std::string, std::string> attributesMap) {
    std::unique_ptr<SimpleRule> rule = nullptr;

    // Check if the rule document is an object
    if (!ruleDoc.IsObject()) {
      // std::cout << "AVIN_DEBUG_ QueryBuilder Invalid JSON format for simple rule. Expected an "
      //              "object. Line: "
                // << __LINE__ << std::endl;
      return rule;  // Return nullptr if JSON is not an object
    }

    // Check if the required fields exist in the rule document
    if (!ruleDoc.HasMember("id") || !ruleDoc.HasMember("datatype") ||
        !ruleDoc.HasMember("operator") || !ruleDoc.HasMember("value")) {
      // std::cout
          // << "AVIN_DEBUG_ QueryBuilder Missing required fields in the simple rule JSON. Line: "
          // << __LINE__ << std::endl;
      return rule;  // Return nullptr if any required field is missing
    }

    // Get the field values from the rule document
    std::string id = ruleDoc["id"].GetString();
    // check if attributesMap is not null and is not empty
    if (attributesMap.size() > 0) {
      if (attributesMap.find(id) != attributesMap.end()) {
        id = attributesMap.at(id);
      }
    }
    std::string datatype = ruleDoc["datatype"].GetString();
    std::string op = ruleDoc["operator"].GetString();

    // Check if the field values are valid
    if (id.empty() || datatype.empty() || op.empty()) {
      // std::cout << "AVIN_DEBUG_ QueryBuilder Invalid field values in the simple rule JSON. Line: "
      //           << __LINE__ << std::endl;
      return rule;  // Return nullptr if any field value is invalid
    }

    // Check if the datatype is a valid FieldType
    FieldType fieldType = fieldTypeMap[datatype];
    if (fieldTypeMap.find(datatype) == fieldTypeMap.end()) {
      // std::cout << "AVIN_DEBUG_ QueryBuilder Unknown datatype in the simple rule JSON. Line: "
      //           << __LINE__ << std::endl;
      return rule;  // Return nullptr if the operator is unknown
    }

    // Create the appropriate SimpleRule based on the datatype
    if (fieldType == STRING) {
    //   if (id == "http_req_headers") {
    //     std::cout << "\nAVIN_DEBUG_QUERY_init02 http_req_headers processed " << std::endl;
    //   }
      rule = std::make_unique<SimpleRuleString>();
      static_cast<SimpleRuleString*>(rule.get())->value = ruleDoc["value"].GetString();
    } else if (fieldType == INTEGER) {
      rule = std::make_unique<SimpleRuleInteger>();

      if (ruleDoc["value"].IsString()) {
        std::string value = ruleDoc["value"].GetString();
        try {
          long valueLong = std::stol(value);
          static_cast<SimpleRuleInteger*>(rule.get())->value = valueLong;
        } catch (const std::exception& e) {
          std::cout
              << "zk-log/builder QueryBuilder Failed to parse integer value in the simple rule "
                 "JSON. Line: "
              << __LINE__ << std::endl;
          // delete rule;     // Delete the created rule object
          return nullptr;  // Return nullptr if failed to parse integer value
        }
      } else if (ruleDoc["value"].IsInt()) {
        // TODO:AVIN Validate for long
        static_cast<SimpleRuleInteger*>(rule.get())->value = ruleDoc["value"].GetInt();
      } else {
        // std::cout << "AVIN_DEBUG_ QueryBuilder Invalid value type in the simple rule JSON. Line: "
        //           << __LINE__ << std::endl;
        // delete rule;     // Delete the created rule object
        return nullptr;  // Return nullptr if value type is invalid
      }
    } else {
      // std::cout << "AVIN_DEBUG_ QueryBuilder Unsupported datatype in the simple rule JSON. Line: "
      //           << __LINE__ << std::endl;
      return rule;  // Return nullptr if the datatype is unsupported
    }

    // Set the remaining fields of the rule object
    rule->id = id;
    rule->type = fieldType;
    // if (ruleDoc.HasMember("key")) {
    //   rule->key = ruleDoc["key"].GetString();
    // }
    if (ruleDoc.HasMember("json_path")) {
      //json_path is an array
      //create a single string from it, with delimeter /
      const rapidjson::Value& jsonPathDoc = ruleDoc["json_path"];
      std::string jsonPath = "";
      int jsonPathSize = static_cast<int>(jsonPathDoc.Size());
      for (int i = 0; i < jsonPathSize; i++) {
        std::string jsonPathElement = jsonPathDoc[i].GetString();
        jsonPath = jsonPath + "/" + jsonPathElement;
      }
      rule->json_path = jsonPath;
      // rule->json_path = ruleDoc["json_path"].GetString();
    }

    // Check if the operator is a valid OperatorType
    if (operatorTypeMap.find(op) == operatorTypeMap.end()) {
      // std::cout << "AVIN_DEBUG_ QueryBuilder Unknown operator in the simple rule JSON. Line: "
      //           << __LINE__ << std::endl;
      // delete rule;     // Delete the created rule object
      return nullptr;  // Return nullptr if the operator is unknown
    }
    rule->operatorType = operatorTypeMap[op];

    return rule;
  }
};
}  // namespace zk