#pragma once

#include "SimpleRuleDefault.h"
#include "rapidjson/document.h"
#include "rapidjson/pointer.h"

namespace zk {
    class SimpleRuleKeyValue : public SimpleRuleDefault {
    public:
        std::string value;

        std::string extractValue(std::map<std::string, std::string> propsMap) const {
            if(propsMap.count(id)){
                const std::string json = propsMap[id];
                const char* jsonCstr = json.c_str();
                rapidjson::Document doc;
                doc.Parse(jsonCstr);

                const char* keyCstr = key.c_str();
                //https://rapidjson.org/md_doc_pointer.html
                rapidjson::Pointer pointer(keyCstr);

                // Extract the value using JSONPath
                if (!pointer.IsValid()) {
                    return "ZK_NULL";
                }

                const rapidjson::Value* result = pointer.Get(doc);
                std::string foundValue = result->GetString();

                return foundValue;
            }

            return "ZK_NULL";
        }

        bool evaluateEquals(std::map<std::string, std::string> propsMap) const override{
            std::string foundValue = extractValue(propsMap);
            if (foundValue != "ZK_NULL"){
                return foundValue == value;
            }
            return false;
        };
        bool evaluateNotEquals(std::map<std::string, std::string> propsMap) const override{
            std::string foundValue = extractValue(propsMap);
            if (foundValue != "ZK_NULL"){
                return foundValue != value;
            }
            return false;
        };
    };
}