#pragma once

namespace zk {
    class ZkConfig{
        private:
            static bool allowAllCalls;
        
        public:

            static bool isAllowAllCalls(){
                return allowAllCalls;
            }

    };

    bool ZkConfig::allowAllCalls = false;
}