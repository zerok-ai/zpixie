#pragma once

namespace zk {
    class ZkServiceConfig{
        private:
            bool allowAllCalls;
            
            bool initialized;
        
        public:
            ZkServiceConfig(){
                allowAllCalls = false;
                initialized = false;
            }

            ZkServiceConfig(bool allowAllCalls){
                this->allowAllCalls = allowAllCalls;
                this->initialized = true;
            }

            void setAllowAllCalls(bool allowAllCalls){
                this->allowAllCalls = allowAllCalls;
            }

            bool isAllowAllCalls(){
                return allowAllCalls;
            }

            void setInitialized(bool initialized){
                this->initialized = initialized;
            }

            bool isInitialized(){
                return initialized;
            }

    };
}