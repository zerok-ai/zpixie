#pragma once

namespace zk {
    class ZkServiceConfig{
        private:
            bool allowAllCalls;
        
        public:
            ZkServiceConfig(){
                allowAllCalls = false;
            }

            ZkServiceConfig(bool allowAllCalls){
                this->allowAllCalls = allowAllCalls;
            }

            void setAllowAllCalls(bool allowAllCalls){
                this->allowAllCalls = allowAllCalls;
            }

            bool isAllowAllCalls(){
                return allowAllCalls;
            }

    };
}