#pragma once

namespace zk {
    class ZkServiceConfig{
        private:
            bool allowAllCalls;
            bool initilaized;
        
        public:
            ZkServiceConfig(){
                allowAllCalls = false;
                initilaized = false;
            }

            ZkServiceConfig(bool allowAllCalls){
                this->allowAllCalls = allowAllCalls;
                this->initilaized = true;
            }

            void setAllowAllCalls(bool allowAllCalls){
                this->allowAllCalls = allowAllCalls;
            }

            bool isAllowAllCalls(){
                return allowAllCalls;
            }

            void setInitilaized(bool initilaized){
                this->initilaized = initilaized;
            }

            bool isInitilaized(){
                return initilaized;
            }

    };
}