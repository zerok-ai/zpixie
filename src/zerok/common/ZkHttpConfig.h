#pragma once

#include <string>
#include <utility>

namespace zk {
    class ZkHttpConfig{
        private:
            bool initialized;
            bool enabled;
            bool traceEnabled;
            bool allowNonTraced;
        
        public:
            ZkHttpConfig(){
                initialized = false;
                enabled = true;
                traceEnabled = true;
                allowNonTraced = false;
            }
            
            ZkHttpConfig(bool enabled, bool traceEnabled, bool allowNonTraced){
                this->enabled = enabled;
                this->traceEnabled = traceEnabled;
                this->allowNonTraced = allowNonTraced;
            }

            void setEnabled(bool enabled){
                this->enabled = enabled;
            }

            bool isEnabled(){
                return enabled;
            }

            void setTraceEnabled(bool traceEnabled){
                this->traceEnabled = traceEnabled;
            }

            bool isTraceEnabled(){
                return traceEnabled;
            }

            void setAllowNonTraced(bool allowNonTraced){
                this->allowNonTraced = allowNonTraced;
            }

            bool isAllowNonTraced(){
                return allowNonTraced;
            }

            void setInitialized(bool initialized){
                this->initialized = initialized;
            }

            bool isInitialized(){
                return initialized;
            }

    };
}