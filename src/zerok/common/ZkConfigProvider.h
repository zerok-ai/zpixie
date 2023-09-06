#pragma once

#include "./ZkServiceConfig.h"
#include "./ZkRedisConfig.h"
#include "./ZkMySqlConfig.h"
#include "./ZkPgSqlConfig.h"
#include "./ZkHttpConfig.h"
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
            static ZkMySqlConfig zkMySqlConfig;
            static ZkHttpConfig zkHttpConfig;
            static ZkPgSqlConfig zkPgSqlConfig;
            
            static ZkServiceConfig parseZkServiceConfig(const std::string& filename) {
                std::cout << "AVIN_DEBUG_ Found parsing service config: " << std::endl;
                ZkServiceConfig localZkServiceConfig = ZkServiceConfig();
                std::ifstream file(filename);
                if (!file.is_open()) {
                    std::cout << "AVIN_DEBUG_ Service Config Failed to open file: " << filename << std::endl;
                    return localZkServiceConfig;
                }

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
                        localZkServiceConfig.setAllowAllCalls(value == "true" || value == "1" || value == "TRUE");
                        localZkServiceConfig.setInitialized(true);
                        std::cout << "AVIN_DEBUG_ Found allowNonTraced: " << value << std::endl;
                    } 
                }

                file.close();
                return localZkServiceConfig;
            }

            // Parse the YAML-like file and extract the 'http' section
            static ZkHttpConfig parseHttpConfig(const std::string& filename) {
                std::cout << "AVIN_DEBUG_ Found parsing http config: " << std::endl;
                ZkHttpConfig localZkHttpConfig = ZkHttpConfig();
                std::ifstream file(filename);
                if (!file.is_open()) {
                    std::cout << "AVIN_DEBUG_ Http Config Failed to open file: " << filename << std::endl;
                    return localZkHttpConfig;
                }

                std::string line;
                std::string currentKey;
                bool inHttpSection = false;

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
                        if (key == "http") {
                            inHttpSection = true;
                            localZkHttpConfig.setInitialized(true);
                            continue;
                        }else{
                            inHttpSection = false;
                        }
                    }

                    if (inHttpSection) {
                        if (key.empty() || value.empty()) {
                            continue;  // Skip malformed lines
                        }

                        // Store key-value pairs
                        if (key == "enabled") {
                            localZkHttpConfig.setEnabled(value == "true" || value == "1" || value == "TRUE");
                            std::cout << "AVIN_DEBUG_ Found enabled: " << value << std::endl;
                        } else if (key == "traceEnabled") {
                            localZkHttpConfig.setTraceEnabled(value == "true" || value == "1" || value == "TRUE");
                            std::cout << "AVIN_DEBUG_ Found traceEnableed: " << value << std::endl;
                        } else if (key == "allowNonTraced") {
                            localZkHttpConfig.setAllowNonTraced(value == "true" || value == "1" || value == "TRUE");
                            std::cout << "AVIN_DEBUG_ Found allowNonTraced: " << value << std::endl;
                        }
                    }
                }

                file.close();
                return localZkHttpConfig;
            }

            // Parse the YAML-like file and extract the 'mysql' section
            static ZkMySqlConfig parseMySqlConfig(const std::string& filename) {
                std::cout << "AVIN_DEBUG_ Found parsing mysql config: " << std::endl;
                ZkMySqlConfig localZkMySqlConfig = ZkMySqlConfig();
                std::ifstream file(filename);
                if (!file.is_open()) {
                    std::cout << "AVIN_DEBUG_ MySql Config Failed to open file: " << filename << std::endl;
                    return localZkMySqlConfig;
                }

                std::string line;
                std::string currentKey;
                bool inMySqlSection = false;

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
                        if (key == "mysql") {
                            inMySqlSection = true;
                            localZkMySqlConfig.setInitialized(true);
                            continue;
                        }else{
                            inMySqlSection = false;
                        }
                    }

                    if (inMySqlSection) {
                        if (key.empty() || value.empty()) {
                            continue;  // Skip malformed lines
                        }

                        // Store key-value pairs
                        if (key == "enabled") {
                            localZkMySqlConfig.setEnabled(value == "true" || value == "1" || value == "TRUE");
                            std::cout << "AVIN_DEBUG_ Found enabled: " << value << std::endl;
                        } else if (key == "traceEnabled") {
                            localZkMySqlConfig.setTraceEnabled(value == "true" || value == "1" || value == "TRUE");
                            std::cout << "AVIN_DEBUG_ Found traceEnableed: " << value << std::endl;
                        } else if (key == "allowNonTraced") {
                            localZkMySqlConfig.setAllowNonTraced(value == "true" || value == "1" || value == "TRUE");
                            std::cout << "AVIN_DEBUG_ Found allowNonTraced: " << value << std::endl;
                        }
                    }
                }

                file.close();
                return localZkMySqlConfig;
            }

            // Parse the YAML-like file and extract the 'pgsql' section
            static ZkPgSqlConfig parsePgSqlConfig(const std::string& filename) {
                std::cout << "AVIN_DEBUG_ Found parsing pgsql config: " << std::endl;
                ZkPgSqlConfig localZkPgSqlConfig = ZkPgSqlConfig();
                std::ifstream file(filename);
                if (!file.is_open()) {
                    std::cout << "AVIN_DEBUG_ PgSql Config Failed to open file: " << filename << std::endl;
                    return localZkPgSqlConfig;
                }

                std::string line;
                std::string currentKey;
                bool inPgSqlSection = false;

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
                        if (key == "pgsql") {
                            localZkPgSqlConfig.setInitialized(true);
                            inPgSqlSection = true;
                            continue;
                        }else{
                            inPgSqlSection = false;
                        }
                    }

                    if (inPgSqlSection) {
                        if (key.empty() || value.empty()) {
                            continue;  // Skip malformed lines
                        }

                        // Store key-value pairs
                        if (key == "enabled") {
                            localZkPgSqlConfig.setEnabled(value == "true" || value == "1" || value == "TRUE");
                            std::cout << "AVIN_DEBUG_ Found enabled: " << value << std::endl;
                        } else if (key == "traceEnabled") {
                            localZkPgSqlConfig.setTraceEnabled(value == "true" || value == "1" || value == "TRUE");
                            std::cout << "AVIN_DEBUG_ Found traceEnableed: " << value << std::endl;
                        } else if (key == "allowNonTraced") {
                            localZkPgSqlConfig.setAllowNonTraced(value == "true" || value == "1" || value == "TRUE");
                            std::cout << "AVIN_DEBUG_ Found allowNonTraced: " << value << std::endl;
                        }
                    }
                }

                file.close();
                return localZkPgSqlConfig;
            }

            // Parse the YAML-like file and extract the 'redis' section
            static ZkRedisConfig parseRedisConfig(const std::string& filename) {
                std::cout << "AVIN_DEBUG_ Found parsing redis config: " << std::endl;
                ZkRedisConfig localZkRedisConfig = ZkRedisConfig("ZK_NULL", 0, 0);
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
                        std::cout << "AVIN_DEBUG_ Found host: " << localZkRedisConfig.getHost() << std::endl;
                        // if (key == "host") {
                        //     localZkRedisConfig.setHost(value);
                        // } else 
                        if (key == "port") {
                            int port = std::stoi(value);
                            localZkRedisConfig.setPort(port);
                            std::cout << "AVIN_DEBUG_ Found port: " << value << std::endl;
                        } else if (key == "readTimeout") {
                            int readTimeout = std::stoi(value);
                            localZkRedisConfig.setReadTimeout(readTimeout);
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

            static ZkPgSqlConfig getZkPgSqlConfig(){
                return zkPgSqlConfig;
            }

            static ZkMySqlConfig getZkMySqlConfig(){
                return zkMySqlConfig;
            }

            static ZkHttpConfig getZkHttpConfig(){
                return zkHttpConfig;
            }

            static void init(){
                if(initialized){
                    return;
                }
                initialized = true;
                //Redis config
                ZkRedisConfig localZkRedisConfig = parseRedisConfig("/opt/zk-client-db-configmap.yaml");
                if(localZkRedisConfig.getHost() != "ZK_NULL"){
                    std::cout << "AVIN_DEBUG_ localZkRedisConfig parsed" << localZkRedisConfig.getHost() << std::endl;
                    zkRedisConfig = ZkRedisConfig(localZkRedisConfig.getHost(), localZkRedisConfig.getPort(), localZkRedisConfig.getReadTimeout());
                }else{
                    std::cout << "AVIN_DEBUG_ localZkRedisConfig not parsed" << std::endl;
                    zkRedisConfig = ZkRedisConfig("redis-master.zk-client.svc.cluster.local", 6379, 1000);
                }

                //Service configs
                ZkServiceConfig localZkServiceConfig = parseZkServiceConfig("/opt/zpixie-configmap.yaml");
                if(localZkServiceConfig.isInitialized()){
                    std::cout << "AVIN_DEBUG_ localZkServiceConfig parsed" << std::endl;
                    zkConfig = ZkServiceConfig(localZkServiceConfig.isAllowAllCalls());
                }else{
                    std::cout << "AVIN_DEBUG_ localZkServiceConfig not parsed" << std::endl;
                    zkConfig = ZkServiceConfig(false);
                }

                //PgSql Configs
                ZkPgSqlConfig localZkPgSqlConfig = parsePgSqlConfig("/opt/zpixie-configmap.yaml");
                if(localZkPgSqlConfig.isInitialized()){
                    std::cout << "AVIN_DEBUG_ localZkPgSqlConfig parsed" << std::endl;
                    zkPgSqlConfig = ZkPgSqlConfig(localZkPgSqlConfig.isEnabled(), localZkPgSqlConfig.isTraceEnabled(), localZkPgSqlConfig.isAllowNonTraced());
                }else{
                    std::cout << "AVIN_DEBUG_ localZkPgSqlConfig not parsed" << std::endl;
                    zkPgSqlConfig = ZkPgSqlConfig();
                }

                //MySql Configs
                ZkMySqlConfig localZkMySqlConfig = parseMySqlConfig("/opt/zpixie-configmap.yaml");
                if(localZkMySqlConfig.isInitialized()){
                    std::cout << "AVIN_DEBUG_ localZkMySqlConfig parsed" << std::endl;
                    zkMySqlConfig = ZkMySqlConfig(localZkMySqlConfig.isEnabled(), localZkMySqlConfig.isTraceEnabled(), localZkMySqlConfig.isAllowNonTraced());
                }else{
                    std::cout << "AVIN_DEBUG_ localZkMySqlConfig not parsed" << std::endl;
                    zkMySqlConfig = ZkMySqlConfig();
                }

                //Http Configs
                ZkHttpConfig localZkHttpConfig = parseHttpConfig("/opt/zpixie-configmap.yaml");
                if(localZkHttpConfig.isInitialized()){
                    std::cout << "AVIN_DEBUG_ localZkHttpConfig parsed" << std::endl;
                    zkHttpConfig = ZkHttpConfig(localZkHttpConfig.isEnabled(), localZkHttpConfig.isTraceEnabled(), localZkHttpConfig.isAllowNonTraced());
                }else{
                    std::cout << "AVIN_DEBUG_ localZkHttpConfig not parsed" << std::endl;
                    zkHttpConfig = ZkHttpConfig();
                }
                
            }

    };

    bool ZkConfigProvider::initialized = false;
    ZkMySqlConfig ZkConfigProvider::zkMySqlConfig = ZkMySqlConfig();
    ZkHttpConfig ZkConfigProvider::zkHttpConfig = ZkHttpConfig();
    ZkPgSqlConfig ZkConfigProvider::zkPgSqlConfig = ZkPgSqlConfig();
    ZkServiceConfig ZkConfigProvider::zkConfig = ZkServiceConfig(false);
    ZkRedisConfig ZkConfigProvider::zkRedisConfig = ZkRedisConfig("redis-master.zk-client.svc.cluster.local", 6379, 1000);
}