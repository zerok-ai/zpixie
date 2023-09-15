#ifndef STORE_H
#define STORE_H

#include <iostream>
#include <map>
#include <string>
#include "./db/memory.h"
#include "src/zerok/filters/fetch/AsyncTask.h"
#include "/home/avin/.cache/bazel/_bazel_avin/54060b0ed2e63c063d495ae4fb1a7d19/execroot/px/external/com_github_redis_hiredis/hiredis.h"
#include <cstdarg>
#include <map>
#include "src/zerok/common/ZkConfigProvider.h"
#include "src/zerok/common/ZkRedisConfig.h"


namespace zk {

    void readerTask(){
        std::string identifier = "abc01";
        zk::ZkMemory* zkMemory = zk::ZkMemory::instance(identifier);
        std::string output = zkMemory->get(100);
    }

    void writerTask(){
        std::string identifier = "abc01";
        zk::ZkMemory* zkMemory = zk::ZkMemory::instance(identifier);
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime.time_since_epoch()).count();
        std::string timestampStr = std::to_string(nanoseconds);
        zkMemory->push(timestampStr);
    }

    class ZkStore{
        public:
            virtual bool connect() = 0;
            virtual void disconnect() = 0;
            virtual bool set(const std::string& key, const std::string& value) = 0;
            virtual void addToSet(const char* key, ...) = 0;
            virtual void addToSetWithExpiry(const int expiryaInSeconds, const char* key, ...) = 0;
            virtual std::string get(const std::string& key) = 0;
            virtual std::vector<std::string> hkeys(const std::string& key) = 0;
            virtual std::string hget(const std::string& key) = 0;
            // virtual std::vector<std::string> hgetall(const std::string& key) = 0;
            virtual std::map<std::string, std::string> hgetall(const std::string& key) = 0;
            // virtual bool del(const std::string& key) = 0;
            // virtual bool exists(const std::string& key) = 0;
    };

    class ZkRedis : public ZkStore{
        private:
            redisContext* redisConnection;
            int database = 0;
            // bool setOnce = false;

        public:
            ZkRedis() : redisConnection(nullptr) {}
            
            ZkRedis(int databaseNum) : redisConnection(nullptr) {
                database = databaseNum;
            }

            bool connect() override {
                if(redisConnection == nullptr){
                    zk::ZkRedisConfig zkRedisConfig = zk::ZkConfigProvider::getZkRedisConfig();
                    std::cout << "\nAVIN_DEBUG_STORE00_ Connecting " << zkRedisConfig.getHost() << std::endl;
                    redisConnection = redisConnect(zkRedisConfig.getHost().c_str(), zkRedisConfig.getPort());
                    bool authSuccess = auth(zkRedisConfig.getPassword().c_str());
                    if(authSuccess){
                        select();
                    }
                }else{
                    // std::cout << "\nAVIN_DEBUG_STORE00_ Already Connected\n" << std::endl;
                    // printf("AVIN_DEBUG_STORE00_ Already connected\n");
                }
                if (redisConnection == nullptr || redisConnection->err) {
                    if (redisConnection) {
                        // Handle connection error
                        printf("AVIN_DEBUG_STORE01_ Connection error: %s\n", redisConnection->errstr);
                        disconnect();
                        return false;
                    } else {
                        // Handle memory allocation error
                        printf("AVIN_DEBUG_STORE02_ Failed to allocate redis context\n");
                    }
                    return false;
                }
                return true;
            }

            bool select() {
                if(checkForConnection() == false){
                    return false;
                }
                if(database == 0){
                    return true;
                }

                // If you want to select a different database, use redisCommand to send the SELECT command
                redisReply* reply = (redisReply*)redisCommand(redisConnection, "SELECT %d", database);
                if(reply == nullptr){
                    return false;
                }else if (reply->type == REDIS_REPLY_ERROR) {
                    freeReplyObject(reply);
                    return false;
                }

                freeReplyObject(reply);
                return true;
            }

            bool auth(const char* password) {
                if(checkForConnection() == false){
                    return false;
                }
                if(database == 0){
                    return true;
                }

                // If you want to select a different database, use redisCommand to send the SELECT command
                redisReply* reply = (redisReply*)redisCommand(redisConnection, "AUTH %s", password);
                if(reply == nullptr){
                    return false;
                }else if (reply->type == REDIS_REPLY_ERROR) {
                    std::cout << "\nAVIN_DEBUG Reply error auth: " << reply->type << std::endl;
                    freeReplyObject(reply);
                    return false;
                }

                freeReplyObject(reply);
                return true;
            }

            void disconnect() override {
                if (redisConnection) {
                    redisFree(redisConnection);
                    redisConnection = nullptr;
                }
            }

            void expire(const char* key, const int expiryaInSeconds){
                if(checkForConnection() == false){
                    return;
                }
                redisReply* reply = (redisReply*)redisCommand(redisConnection, "EXPIRE %s %d", key, expiryaInSeconds);
                if(reply == nullptr){
                    return;
                }else if (reply->type == REDIS_REPLY_ERROR) {
                    // Handle error
                    std::cout << "\nAVIN_DEBUG Reply error expire: " << reply->type << std::endl;
                    freeReplyObject(reply);
                    return;
                }
                freeReplyObject(reply);
            }

            void addToSetWithExpiry(const int expiryaInSeconds, const char* key, ...) override {
                bool transactionStarted = startTransaction();
                if(transactionStarted){
                    va_list args;
                    va_start(args, key);
                    addToSet(key, args);
                    va_end(args);
                    expire(key, expiryaInSeconds);
                    endTransaction();
                }
            }

            bool checkForConnection(){
                if(redisConnection == nullptr || redisConnection->err ){
                    return false;
                }
                return true;
            }

            bool startTransaction(){
                if(checkForConnection() == false){
                    return false;
                }
                redisReply* reply = (redisReply*)redisCommand(redisConnection, "MULTI");
                if(reply == nullptr){
                    std::cout << "\nAVIN_DEBUG Reply error startTransaction: reply null" <<  std::endl;
                    return false;
                }else if (reply->type == REDIS_REPLY_ERROR) {
                    // Handle error
                    std::cout << "\nAVIN_DEBUG Reply error startTransaction: " << reply->type << std::endl;
                    freeReplyObject(reply);
                    return false;
                }
                freeReplyObject(reply);
                return true;
            }

            bool endTransaction(){
                if(checkForConnection() == false){
                    return false;
                }
                redisReply* reply = (redisReply*)redisCommand(redisConnection, "EXEC");
                if(reply == nullptr){
                    std::cout << "\nAVIN_DEBUG Reply error endTransaction: reply null" <<  std::endl;
                    return false;
                }else if (reply->type == REDIS_REPLY_ERROR) {
                    // Handle error
                    std::cout << "\nAVIN_DEBUG Reply error endTransaction: " << reply->type << std::endl;
                    freeReplyObject(reply);
                    return false;
                }
                freeReplyObject(reply);
                return true;
            }

            void addToSet(const char* key, ...) override{
                va_list args;
                va_start(args, key);
                addToSet(key, args);
                va_end(args);
            }

            bool addToSet(const char* key, va_list args) {
                if(checkForConnection() == false){
                    return false;
                }
                std::string finalArgs;
                const char* arg = va_arg(args, const char*);
                while (arg != nullptr) {
                    if (finalArgs.length() > 0){
                        finalArgs += " ";    
                    }
                    finalArgs += arg;
                    arg = va_arg(args, const char*);
                }
                // std::cout << "DerivedClass finalArgs. " << finalArgs << std::endl;

                redisReply* reply = (redisReply*)redisCommand(redisConnection, "SADD %s %s", key, finalArgs.c_str());
                if(reply == nullptr){
                    std::cout << "\nAVIN_DEBUG Reply error addToSet: reply null" <<  std::endl;
                    return false;
                }else if (reply->type == REDIS_REPLY_ERROR) {
                    // Handle error
                    std::cout << "\nAVIN_DEBUG Reply error addToSet: " << reply->type << std::endl;
                    freeReplyObject(reply);
                    return false;
                }
                freeReplyObject(reply);
                return true;
            }

            bool set(const std::string& key, const std::string& value) override {
                if(checkForConnection() == false){
                    return false;
                }
                // printf("AVIN_DEBUG_STORE04_ store.set\n");
                redisReply* reply = (redisReply*)redisCommand(redisConnection, "SET %s %s", key.c_str(), value.c_str());
                if(reply == nullptr){
                    return false;
                }else if (reply->type == REDIS_REPLY_ERROR) {
                    // Handle error
                    freeReplyObject(reply);
                    return false;
                }
                freeReplyObject(reply);
                return true;
            }

            std::string get(const std::string& key) override {
                if(checkForConnection() == false){
                    return "";
                }
                // printf("AVIN_DEBUG_STORE07_ store.get\n");
                redisReply* reply = static_cast<redisReply*>(redisCommand(redisConnection, "GET %s", key.c_str()));
                // redisReply* reply = (redisReply*)redisCommand(redisConnection, "GET %s", key.c_str());
                if(reply == nullptr){
                    return "";
                }else if (reply->type == REDIS_REPLY_ERROR) {
                    // Handle error
                    // printf("AVIN_DEBUG_STORE08_ store.get %s\n", reply ? reply->str : "Unknown error");
                    freeReplyObject(reply);
                    return "";
                }
                std::string value = reply->str;
                freeReplyObject(reply);
                return value;
            }

            std::vector<std::string> hkeys(const std::string& key) override{
                if(checkForConnection() == false){
                    return std::vector<std::string>();
                }
                redisReply* reply = static_cast<redisReply*>(redisCommand(redisConnection, "HKEYS %s", key.c_str()));
                if(reply == nullptr){
                    return std::vector<std::string>();
                }else if (reply->type == REDIS_REPLY_ERROR) {
                    // Handle error
                    freeReplyObject(reply);
                    return std::vector<std::string>();
                }
                std::vector<std::string> keys;
                for (int i = 0; static_cast<size_t>(i) < reply->elements; i++) {
                    keys.push_back(reply->element[i]->str);
                }
                freeReplyObject(reply);
                return keys;
            }

            std::string hget(const std::string& key) override{
                if(checkForConnection() == false){
                    return "";
                }
                redisReply* reply = static_cast<redisReply*>(redisCommand(redisConnection, "HGET %s", key.c_str()));
                if(reply == nullptr){
                    return "";
                }else if (reply->type == REDIS_REPLY_ERROR) {
                    // Handle error
                    freeReplyObject(reply);
                    return "";
                }
                std::string value = reply->str;
                freeReplyObject(reply);
                return value;
            }

            std::map<std::string, std::string> hgetall(const std::string& key) override{
                if(checkForConnection() == false){
                    return std::map<std::string, std::string>();
                }
                redisReply* reply = static_cast<redisReply*>(redisCommand(redisConnection, "HGETALL %s", key.c_str()));
                if(reply == nullptr){
                    return std::map<std::string, std::string>();
                }else if (reply->type == REDIS_REPLY_ERROR) {
                    // Handle error
                    freeReplyObject(reply);
                    return std::map<std::string, std::string>();
                }
                std::map<std::string, std::string> values;
                for (int i = 0; static_cast<size_t>(i) < reply->elements; i += 2) {
                    values[reply->element[i]->str] = reply->element[i + 1]->str;
                }
                freeReplyObject(reply);
                return values;
            }
    };

    class ZkStoreProvider {
        private:
            static std::map<int, ZkStore*> storeProvider;
            static ZkStore* zkStore;
        public:
            static ZkStore* instance(){
                if(zkStore != nullptr){
                    return zkStore;
                }
                ZkRedis* hiredisClient = new ZkRedis();
                ZkStore* redisClient = hiredisClient;
                ZkStoreProvider::zkStore = hiredisClient;

                // zk::AsyncTask readerAsyncTask(&readerTask, 1000);
                // readerAsyncTask.Start();

                // zk::AsyncTask writerAsyncTask(&writerTask, 200);
                // writerAsyncTask.Start();

                return redisClient;
            }

            static ZkStore* instance(int database){
                if(storeProvider.find(database) != storeProvider.end()){
                    return storeProvider[database];
                }
                ZkRedis* hiredisClient = new ZkRedis(database);
                ZkStore* redisClient = hiredisClient;
                storeProvider[database] = redisClient;

                return redisClient;
            }
    };
    ZkStore* ZkStoreProvider::zkStore = nullptr;
    std::map<int, ZkStore*> ZkStoreProvider::storeProvider;
}

#endif // STORE_H