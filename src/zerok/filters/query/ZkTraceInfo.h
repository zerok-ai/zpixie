#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace zk {
    class ZkTraceInfo{
         private:
            std::string traceId;
            std::string spanId;
            std::vector<std::string> workloadIds;
        
        public:
            //default and full constructor
            ZkTraceInfo(std::string traceId, std::string spanId, std::vector<std::string> workloadIds){
                this->traceId = traceId;
                this->spanId = spanId;
                this->workloadIds = workloadIds;
            }

            ZkTraceInfo(std::string traceId, std::string spanId){
                this->traceId = traceId;
                this->spanId = spanId;
                this->workloadIds = std::vector<std::string>();
            }

            ZkTraceInfo(){
                this->traceId = "";
                this->spanId = "";
                this->workloadIds = std::vector<std::string>();
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

            std::vector<std::string> getWorkloadIds(){
                return workloadIds;
            }

            void setWorkloadIds(std::vector<std::string> workloadIds){
                this->workloadIds = workloadIds;
            }

            //add workloadid method
            void addWorkloadId(std::string workloadId){
                workloadIds.push_back(workloadId);
            }

            //method to get the string representation of the workloadIds with comma separated values
            std::string getWorkloadIdsString(){
                std::string workloadIdsString = "";
                for(auto i = 0; i < workloadIds.size(); i++){
                    workloadIdsString += workloadIds[i];
                    if(i != workloadIds.size() - 1){
                        workloadIdsString += ",";
                    }
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