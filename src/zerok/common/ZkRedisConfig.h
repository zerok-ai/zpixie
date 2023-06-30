#pragma once

#include <string>
#include <utility>

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
                this->host = std::move(host);
                this->port = port;
                this->readTimeout = readTimeout;
            }

            void setHost(std::string host){
                this->host = std::move(host);
            }

            std::string getHost(){
                return host;
            }

            void setPort(int port){
                this->port = port;
            }

            int getPort(){
                return port;
            }

            void setReadTimeout(int readTimeout){
                this->readTimeout = readTimeout;
            }

            int getReadTimeout(){
                return readTimeout;
            }

    };
}