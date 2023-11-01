#include <iostream>
#include <string>
#include <map>
#include <rapidjson/document.h>
#include "query/Query.h"
#include "query/QueryBuilder.h"

void doSomething(){
    std::cout << "This will get printed" << std::flush;
}

int main()
{
    std::cout << "\n--------------------\n";
    std::cout << "Starting the main.cc\n";
    // //string
    // // const char* json = "{\"condition\":\"AND\",\"zk_request_type\":\"HTTP\",\"rules\":[{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP\"}]}";
    // //string + int
    // // const char* json = "{\"condition\":\"AND\",\"zk_request_type\":\"HTTP\",\"rules\":[{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP\"},{\"id\":\"int_field\",\"field\":\"int_field\",\"type\":\"integer\",\"input\":\"string\",\"operator\":\"equal\",\"value\":35}]}";
    // //string + int + keyvalue
    // // const char* json = "{\"condition\":\"AND\",\"zk_request_type\":\"HTTP\",\"rules\":[{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP\"},{\"id\":\"int_field\",\"field\":\"int_field\",\"type\":\"integer\",\"input\":\"string\",\"operator\":\"equal\",\"value\":35},{\"id\":\"key_value_field\",\"field\":\"key_value_field\",\"key\":\"/value/value2/value3\",\"type\":\"key-map\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP\"}]}";
    // //string + int + keyvalue + workload
    // // const char* json = "{\"condition\":\"AND\",\"zk_request_type\":{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP\"},\"rules\":[{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP\"},{\"id\":\"int_field\",\"field\":\"int_field\",\"type\":\"integer\",\"input\":\"string\",\"operator\":\"equal\",\"value\":35},{\"id\":\"key_value_field\",\"field\":\"key_value_field\",\"key\":\"/value/value2/value3\",\"type\":\"key-map\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP\"},{\"id\":\"source\",\"field\":\"source\",\"type\":\"workload-identifier\",\"operator\":\"in\",\"value\":{\"service_name\":\"demo/sofa, demo2/invent\",\"ip\":\"10.43.3.4\",\"pod_name\":\"abc,zxy\"}}]}";
    // //string + int + keyvalue + workload + in
    // const char* json = "{\"condition\":\"AND\",\"zk_request_type\":{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP, JHKJ, UIOSI\"},\"rules\":[{\"id\":\"string_in_field\",\"field\":\"string_in_field\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"in\",\"value\":\"HTTP\"},{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP\"},{\"id\":\"int_field\",\"field\":\"int_field\",\"type\":\"integer\",\"input\":\"string\",\"operator\":\"equal\",\"value\":35},{\"id\":\"key_value_field\",\"field\":\"key_value_field\",\"key\":\"/value/value2/value3\",\"type\":\"key-map\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP\"},{\"id\":\"source\",\"field\":\"source\",\"type\":\"workload-identifier\",\"operator\":\"in\",\"value\":{\"service_name\":\"demo/sofa, demo2/invent\",\"ip\":\"10.43.3.4\",\"pod_name\":\"abc,zxy\"}}]}";
    
    // zk::Query* query = zk::QueryBuilder::parseQuery(json);
    // std::map<std::string, std::string> propsMap = {
    //     {"zk_req_type", "HTTP"},
    //     {"int_field", "35"},
    //     {"trace_role", "server"},
    //     {"remote_addr", "10.0.0.4"},
    //     {"string_in_field", "HTTP"},
    //     {"key_value_field", "{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value2\":{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value3\":\"HTTP\"}}}"},
    // };
    // std::cout << "query->rule->evaluate(propsMap) " << query->rule->evaluate(propsMap) << std::flush;

    // // zk::AsyncTask asyncTask(30*60*1000);
    // // asyncTask.start(doSomething);

    // // const char* query1Str = "{\"condition\":\"AND\",\"zk_request_type\":{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"HTTP\"},\"rules\":[{\"id\":\"req_method\",\"field\":\"req_method\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"POST\"},{\"id\":\"req_path\",\"field\":\"req_path\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"ends_with\",\"value\":\"\/exception\"},{\"id\":\"destination\",\"field\":\"destination\",\"type\":\"workload-identifier\",\"input\":\"workload-identifier\",\"operator\":\"in\",\"value\":{\"pod_name\":\"abc\"}},{\"id\":\"source\",\"field\":\"source\",\"type\":\"workload-identifier\",\"input\":\"workload-identifier\",\"operator\":\"in\",\"value\":{\"ip\":\"10.43.3.4\",\"pod_name\":\"abc,zxy\",\"service_name\":\"demo\/sofa, demo2\/invent\"}}]}";
    // // zk::Query* query1 = zk::QueryBuilder::parseQuery(query1Str);
    // // std::map<std::string, std::string> query1Props = {
    // //     {"zk_req_type", "HTTP"},
    // //     {"int_field", "35"},
    // //     {"trace_role", "server"},
    // //     {"remote_addr", "10.0.0.4"},
    // //     {"key_value_field", "{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value2\":{\"id\":\"zk_req_type\",\"field\":\"zk_req_type\",\"type\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value3\":\"HTTP\"}}}"},
    // // };
    // // std::cout << "query->rule->evaluate(propsMap) " << query1->rule->evaluate(query1Props) << std::flush;

    return 0;
}