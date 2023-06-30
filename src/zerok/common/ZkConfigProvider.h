#pragma once

#include "./ZkServiceConfig.h"
#include "./ZkRedisConfig.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

namespace zk {
    class ZkConfigProvider{
        private:
            static bool initialized;
            static ZkServiceConfig zkConfig;
            static ZkRedisConfig zkRedisConfig;
            
            static ZkServiceConfig* parseZkServiceConfig(const std::string& filename) {
                std::cout << "AVIN_DEBUG_ Found parsing service config: " << std::endl;
                ZkServiceConfig* localZkServiceConfig = nullptr;
                std::ifstream file(filename);
                if (!file.is_open()) {
                    std::cout << "AVIN_DEBUG_ Service Config Failed to open file: " << filename << std::endl;
                    return localZkServiceConfig;
                }

                ZkServiceConfig zkConfig = ZkServiceConfig();
                localZkServiceConfig = &zkConfig;

                std::string line;
                std::string currentKey;

                while (std::getline(file, line)) {
                    std::stringstream ss(line);
                    std::string key;
                    std::string value;
                    std::getline(ss, key, ':');
                    std::getline(ss, value);

                    if (key.empty() && value.empty()) {
                        continue;  // Skip empty lines
                    }

                    // Trim leading/trailing whitespaces
                    key.erase(0, key.find_first_not_of(" \t"));
                    key.erase(key.find_last_not_of(" \t") + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);

                    if (value.empty()) {
                        continue;  // Skip malformed lines
                    }

                    // Store key-value pairs
                    if (key == "allowNonTraced") {
                        localZkServiceConfig->setAllowAllCalls(value == "true" || value == "1" || value == "TRUE");
                        std::cout << "AVIN_DEBUG_ Found allowNonTraced: " << value << std::endl;
                    } 
                }

                file.close();
                return localZkServiceConfig;
            }

            // Parse the YAML-like file and extract the 'redis' section
            static ZkRedisConfig* parseRedisConfig(const std::string& filename) {
                std::cout << "AVIN_DEBUG_ Found parsing redis config: " << std::endl;
                ZkRedisConfig* localZkRedisConfig = nullptr;
                std::ifstream file(filename);
                if (!file.is_open()) {
                    std::cout << "AVIN_DEBUG_ Redis Config Failed to open file: " << filename << std::endl;
                    return localZkRedisConfig;
                }

                std::string line;
                std::string currentKey;
                bool inRedisSection = false;

                while (std::getline(file, line)) {
                    std::stringstream ss(line);
                    std::string key;
                    std::string value;
                    std::getline(ss, key, ':');
                    std::getline(ss, value);

                    if (key.empty() && value.empty()) {
                        continue;  // Skip empty lines
                    }

                    // Trim leading/trailing whitespaces
                    key.erase(0, key.find_first_not_of(" \t"));
                    key.erase(key.find_last_not_of(" \t") + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);

                    if (value.empty()){
                        if (key == "redis") {
                            ZkRedisConfig zkRedisConfigInstance = ZkRedisConfig();
                            localZkRedisConfig = &zkRedisConfigInstance;
                            inRedisSection = true;
                            continue;
                        }else{
                            inRedisSection = false;
                        }
                    }

                    if (inRedisSection) {
                        if (key.empty() || value.empty()) {
                            continue;  // Skip malformed lines
                        }

                        // Store key-value pairs
                        if (key == "host") {
                            localZkRedisConfig->setHost(value);
                            std::cout << "AVIN_DEBUG_ Found host: " << value << std::endl;
                        } else if (key == "port") {
                            int port = std::stoi(value);
                            localZkRedisConfig->setPort(port);
                            std::cout << "AVIN_DEBUG_ Found port: " << value << std::endl;
                        } else if (key == "readTimeout") {
                            int readTimeout = std::stoi(value);
                            localZkRedisConfig->setReadTimeout(readTimeout);
                            std::cout << "AVIN_DEBUG_ Found readTimeout: " << value << std::endl;
                        }
                    }
                }

                file.close();
                return localZkRedisConfig;
            }
        
        public:
            static ZkServiceConfig getZkConfig(){
                return zkConfig;
            }

            static ZkRedisConfig getZkRedisConfig(){
                return zkRedisConfig;
            }

            static void init(){
                if(initialized){
                    return;
                }
                initialized = true;
                ZkRedisConfig* localZkRedisConfig = parseRedisConfig("/opt/zk-client-db-configmap.yaml");
                if(localZkRedisConfig != nullptr){
                    std::cout << "localZkRedisConfig parsed" << std::endl;
                    zkRedisConfig = ZkRedisConfig("redis.zk-client.svc.cluster.local", 6379, 1000);
                }else{
                    zkRedisConfig = *localZkRedisConfig;
                }
                ZkServiceConfig* localZkServiceConfig = parseZkServiceConfig("/opt/zpixie-configmap.yaml");
                if(localZkServiceConfig != nullptr){
                    std::cout << "localZkServiceConfig parsed" << std::endl;
                    zkConfig = ZkServiceConfig(false);
                }else{
                    zkConfig = *localZkServiceConfig;
                }
                
            }

    };

    bool ZkConfigProvider::initialized = false;
    ZkServiceConfig ZkConfigProvider::zkConfig = ZkServiceConfig(false);
    ZkRedisConfig ZkConfigProvider::zkRedisConfig = ZkRedisConfig("redis.zk-client.svc.cluster.local", 6379, 1000);
}