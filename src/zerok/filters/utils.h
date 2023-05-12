#include <iostream>
#include <string>
#include <any>
#include <vector>
#include <rapidjson/document.h>
// #include <any_cast>

namespace zk {
    class CommonUtils{
        public: 
            bool compare(std::any arg1, std::any arg2) const{
                std::cout << "\n@Debug14" << " arg1.type() " << arg1.type().name();
                std::cout << "\n@Debug14" << " arg2.type() " << arg2.type().name();
                if (arg1.type() == arg2.type()) {
                    // Compare the values of a and b
                    const char* arg1char = std::any_cast<const char*>(arg1);
                    const char* arg2char = std::any_cast<const char*>(arg2);
                    std::cout << "\n@Debug15- " << arg1char << " __ ";
                    std::cout << "\n@Debug15- " << arg2char << " __ ";
                    bool comparisonValue = arg1char == arg2char;
                    std::cout << "\n@Debug15- " << comparisonValue;

                    if (std::any_cast<const char*>(arg1) == std::any_cast<const char*>(arg2)) {
                        std::cout << "\n@Debug16";
                        // std::cout << "a and b are equal" << std::endl;
                        return true;
                    } else {
                        std::cout << "\n@Debug17";
                        // std::cout << "a and b are not equal" << std::endl;
                        return false;
                    }
                } else {
                    std::cout << "\n@Debug18";
                    // std::cout << "a and b have different types" << std::endl;
                    return false;
                }
                std::cout << "\n@Debug19";
                return false;
            }

            static std::vector<std::string> splitString(const std::string& str, const std::string& delimiter) {
                std::vector<std::string> tokens;
                std::size_t pos = 0;
                std::size_t endPos;

                while ((endPos = str.find(delimiter, pos)) != std::string::npos) {
                    tokens.push_back(str.substr(pos, endPos - pos));
                    pos = endPos + delimiter.length();
                }

                tokens.push_back(str.substr(pos));

                return tokens;
            }

            rapidjson::Value toJsonValue(std::any value) const{
                // std::cout << "valuevaluevaluevaluevalue  " << value.type();
                rapidjson::Document document;
                rapidjson::Value jsonValue;
                if (value.type() == typeid(int)) {
                    int intValue = std::any_cast<int>(value);
                    jsonValue.SetInt(intValue);
                } else if (value.type() == typeid(unsigned int)) {
                    unsigned int uintValue = std::any_cast<unsigned int>(value);
                    jsonValue.SetUint(uintValue);
                } else if (value.type() == typeid(long)) {
                    long longValue = std::any_cast<long>(value);
                    jsonValue.SetInt64(longValue);
                } else if (value.type() == typeid(unsigned long)) {
                    unsigned long ulongValue = std::any_cast<unsigned long>(value);
                    jsonValue.SetUint64(ulongValue);
                } else if (value.type() == typeid(float)) {
                    float floatValue = std::any_cast<float>(value);
                    jsonValue.SetFloat(floatValue);
                } else if (value.type() == typeid(double)) {
                    double doubleValue = std::any_cast<double>(value);
                    jsonValue.SetDouble(doubleValue);
                } else if (value.type() == typeid(bool)) {
                    bool boolValue = std::any_cast<bool>(value);
                    jsonValue.SetBool(boolValue);
                } else if (value.type() == typeid(std::string)) {
                    std::string stringValue = std::any_cast<std::string>(value);
                    jsonValue.SetString(stringValue.c_str(), stringValue.length(), document.GetAllocator());
                } else {
                    // Handle other data types
                }

                return jsonValue;
            }

    };
};