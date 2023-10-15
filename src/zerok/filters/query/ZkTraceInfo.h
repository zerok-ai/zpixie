#pragma once

#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include "src/zerok/common/utils.h"


namespace zk {
    class ZkTraceInfo{
         private:
            std::string traceId;
            std::string spanId;
            std::set<std::string> workloadIds;
        
        public:
            //default and full constructor
            ZkTraceInfo(std::string traceId, std::string spanId, std::set<std::string> workloadIds){
                this->traceId = traceId;
                this->spanId = spanId;
                this->workloadIds = workloadIds;
            }

            ZkTraceInfo(std::string traceId, std::string spanId){
                this->traceId = traceId;
                this->spanId = spanId;
                this->workloadIds = std::set<std::string>();
            }

            ZkTraceInfo(){
                this->traceId = "";
                this->spanId = "";
                this->workloadIds = std::set<std::string>();
            }

            ZkTraceInfo(std::string traceParent){
                fromTraceParent(traceParent);
            }

            ZkTraceInfo fromTraceParent(std::string traceParent) {
                std::vector<std::string> splitString = CommonUtils::splitString(traceParent, "-");
                if(splitString.size() <= static_cast<size_t>(1)){
                    return *this;
                }
                traceId = splitString.at(1);
                if(traceId == ""){
                    // printf("\nzk-log/builder traceparent header value is invalid");
                    return *this;
                }
                spanId = splitString.at(2);
                this->setTraceId(traceId);
                this->setSpanId(spanId);
                return *this;
            }

            //all the getters and setters
            std::string getTraceId(){
                return traceId;
            }

            void setTraceId(std::string traceId){
                this->traceId = traceId;
            }

            std::string getSpanId(){
                return spanId;
            }

            void setSpanId(std::string spanId){
                this->spanId = spanId;
            }

            std::set<std::string> getWorkloadIds(){
                return workloadIds;
            }

            void setWorkloadIds(std::set<std::string> workloadIds){
                this->workloadIds = workloadIds;
            }

            //add workloadid method
            void addWorkloadId(std::string workloadId){
                workloadIds.insert(workloadId);
            }

            //method to get the string representation of the workloadIds with comma separated values
            std::string getWorkloadIdsString(){
                std::string workloadIdsString = "";
                for (const auto& workloadId : workloadIds){
                    //if workloadIdsString is empty, then don't add comma
                    if (workloadIdsString != ""){
                        workloadIdsString += ",";
                    }
                    workloadIdsString += workloadId;
                }

                return workloadIdsString;
            }

            //method to check if this object is valid or not. if traceid and spanid are empty, then it is invalid
            bool isValid(){
                return traceId != "" && traceId != "ZK_NULL" && spanId != "" && spanId != "ZK_NULL";
            }

            // std::string toString(){
            //     return "TraceId: " + traceId + " SpanId: " + spanId + " WorkloadIds: " + workloadIds;
            // }

            bool operator==(const ZkTraceInfo& other) const{
                return traceId == other.traceId && spanId == other.spanId && workloadIds == other.workloadIds;
            }

            bool operator!=(const ZkTraceInfo& other) const{
                return !(*this == other);
            }

    };
}