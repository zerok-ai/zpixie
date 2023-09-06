#pragma once

#include <string>
#include <utility>
#include <stdlib.h>

namespace zk {
    class ZkRedisConfig{
        private:
            std::string host;
            std::string password;
            int port;
            int readTimeout;
        
        public:
            ZkRedisConfig(){
                // host = "";
                port = 0;
                readTimeout = 0;
                this->readEnv();
            }
            
            ZkRedisConfig(std::string host, int port, int readTimeout){
                // this->host = std::move(host);
                if (host == ""){
                }
                this->port = port;
                this->readTimeout = readTimeout;
                this->readEnv();
            }

            void readEnv(){
                this->password = getenv("PL_REDIS_PASSWORD");
                this->host = getenv("PL_REDIS_HOST");
            }

            void setHost(std::string host){
                this->host = std::move(host);
            }

            std::string getHost(){
                return host;
            }

            void setPassword(std::string password){
                this->password = std::move(password);
            }

            std::string getPassword(){
                return password;
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