#pragma once

#include <string>

namespace zk {
    class ZkRedisConfig{
        private:
            std::string host;
            int port;
            int readTimeout;
        
        public:
            ZkRedisConfig(){
                host = "";
                port = 0;
                readTimeout = 0;
            }
            
            ZkRedisConfig(std::string host, int port, int readTimeout){
                this->host = host;
                this->port = port;
                this->readTimeout = readTimeout;
            }

            std::string getHost(){
                return host;
            }

            int getPort(){
                return port;
            }

            int getReadTimeout(){
                return readTimeout;
            }

    };
}