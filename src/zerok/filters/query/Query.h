#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "Rule.h"
#include "QueryType.h"
#include "OperatorType.h"
#include "ConditionType.h"
#include "FieldType.h"

namespace zk {
    static std::unordered_map<std::string, ConditionType> conditionTypeMap = {
        {"AND", AND},
        {"OR", OR},
    };

    static std::unordered_map<std::string, QueryType> queryTypeMap = {
        {"HTTP", HTTP},
        {"MYSQL", MYSQL},
    };

    static std::unordered_map<QueryType, std::string> queryTypeStringMap = {
        {HTTP, "HTTP"},
        {MYSQL, "MYSQL"},
    };

    static std::unordered_map<std::string, OperatorType> operatorTypeMap = {
        {"exists", EXISTS},
        {"not_exists", NOT_EXISTS},
        {"equal", EQUALS},
        {"not_equal", NOT_EQUALS},
        {"in", IN},
        {"not_in", NOT_IN},
        {"greater_than", GREATER_THAN},
        {"greater_than_equal", GREATER_THAN_EQUALS},
        {"less_than_equal", LESS_THAN_EQUALS},
        {"less_than", LESS_THAN},
    };

    static std::unordered_map<std::string, FieldType> fieldTypeMap = {
        {"string", STRING},
        {"integer", INTEGER}
    };

    class Query{
         public:
          std::unique_ptr<Rule> rule;
          QueryType queryType;
          std::string workloadId;
          std::string traceRole;
          std::string ns;
          std::string service;
    };
}