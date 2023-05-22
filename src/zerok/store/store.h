#ifndef STORE_H
#define STORE_H

#include <iostream>
#include <string>
#include "./db/memory.h"
#include "src/zerok/filters/fetch/AsyncTask.h"
#include "/home/avin/.cache/bazel/_bazel_avin/54060b0ed2e63c063d495ae4fb1a7d19/execroot/px/external/com_github_redis_hiredis/hiredis.h"
#include <cstdarg>


namespace zk {

    void readerTask(){
        std::cout << "\nAVIN_DEBUG_STORE_ASYNC01_reader task" << std::endl;
        std::string identifier = "abc01";
        zk::ZkMemory* zkMemory = zk::ZkMemory::instance(identifier);
        std::string output = zkMemory->get(100);
        std::cout << "\nAVIN_DEBUG_STORE_ASYNC02_found data " << output.c_str() << std::endl;
    }

    void writerTask(){
        std::cout << "\nAVIN_DEBUG_STORE_ASYNC03_writer task" << std::endl;
        std::string identifier = "abc01";
        zk::ZkMemory* zkMemory = zk::ZkMemory::instance(identifier);
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime.time_since_epoch()).count();
        std::string timestampStr = std::to_string(nanoseconds);
        zkMemory->push(timestampStr);
        // printf("\nAVIN_DEBUG_ASYNC04_wrote %s", timestampStr.c_str());
    }

    class ZkStore{
        public:
            virtual bool connect() = 0;
            virtual void disconnect() = 0;
            virtual bool set(const std::string& key, const std::string& value) = 0;
            virtual void addToSet(const std::string& key, ...) = 0;
            virtual std::string get(const std::string& key) = 0;
            // virtual bool del(const std::string& key) = 0;
            // virtual bool exists(const std::string& key) = 0;
    };

    class ZkRedis : public ZkStore{
        private:
            redisContext* redisConnection;
            bool setOnce = false;

        public:
            ZkRedis() : redisConnection(nullptr) {}
            bool connect() override {
                // printf("AVIN_DEBUG_STORE00_ Trying to connect\n");
                if(redisConnection == nullptr){
                    std::cout << "\nAVIN_DEBUG_STORE00_ Connecting\n" << std::endl;
                    redisConnection = redisConnect("redis.redis.svc.cluster.local", 6379);
                }else{
                    std::cout << "\nAVIN_DEBUG_STORE00_ Already Connected\n" << std::endl;
                    // printf("AVIN_DEBUG_STORE00_ Already connected\n");
                }
                if (redisConnection == nullptr || redisConnection->err) {
                    if (redisConnection) {
                        // Handle connection error
                        printf("AVIN_DEBUG_STORE01_ Connection error: %s\n", redisConnection->errstr);
                        disconnect();
                    } else {
                        // Handle memory allocation error
                        printf("AVIN_DEBUG_STORE02_ Failed to allocate redis context\n");
                    }
                    return false;
                }else{
                    // printf("AVIN_DEBUG_STORE03_ Connected\n");
                    if (setOnce == false){
                        // auto reply = redisCommand(redisConnection, "SET key02 value02");
                        // if (reply != nullptr) {
                        //     printf("AVIN_DEBUG_STORE044_ Set done - Reply not null\n");
                        //     redisReply* replyObj = (redisReply*)reply;
                        //     printf("AVIN_DEBUG_STORE046_ Set done - Casting done %d\n", replyObj->type);
                        // }else{
                        //     printf("AVIN_DEBUG_STORE045_ Set done - Reply null\n");
                        // }

                        setOnce = true;
                        // printf("\nAVIN_DEBUG_ASYNC00_reader task starting");
                        // zk::AsyncTask readerAsyncTask(&readerTask, 1000);
                        // readerAsyncTask.Start();

                        // printf("\nAVIN_DEBUG_ASYNC00_writer task starting");
                        // zk::AsyncTask writerAsyncTask(&writerTask, 200);
                        // writerAsyncTask.Start();
                    }else{
                        // printf("AVIN_DEBUG_STORE045_ Already set once\n");
                    }
                    
                    // printf("AVIN_DEBUG_STORE03333333333_ Set done\n");
                }
                // std::this_thread::sleep_for(std::chrono::milliseconds(100));
                return true;
            }

            void disconnect() override {
                if (redisConnection) {
                    redisFree(redisConnection);
                    redisConnection = nullptr;
                }
            }

            void addToSet(const std::string& key, ...) override{
                printf("AVIN_DEBUG_STORE04_ store.addToSet\n");

                va_list args;
                va_start(args, key);

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
                va_end(args);
                

                redisReply* reply = (redisReply*)redisCommand(redisConnection, "SADD %s %s", key.c_str(), finalArgs.c_str());
                if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
                    // Handle error
                    printf("AVIN_DEBUG_STORE05_ store.addToSet %s\n", reply ? reply->str : "Unknown error");
                    freeReplyObject(reply);
                    return false;
                }else{
                    printf("AVIN_DEBUG_STORE06_ store.addToSet success\n");
                }
                freeReplyObject(reply);
            }

            bool set(const std::string& key, const std::string& value) override {
                printf("AVIN_DEBUG_STORE04_ store.set\n");
                redisReply* reply = (redisReply*)redisCommand(redisConnection, "SET %s %s", key.c_str(), value.c_str());
                if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
                    // Handle error
                    printf("AVIN_DEBUG_STORE05_ store.set %s\n", reply ? reply->str : "Unknown error");
                    freeReplyObject(reply);
                    return false;
                }else{
                    printf("AVIN_DEBUG_STORE06_ store.set success\n");
                }
                freeReplyObject(reply);
                return true;
            }

            std::string get(const std::string& key) override {
                printf("AVIN_DEBUG_STORE07_ store.get\n");
                redisReply* reply = static_cast<redisReply*>(redisCommand(redisConnection, "GET %s", key.c_str()));
                // redisReply* reply = (redisReply*)redisCommand(redisConnection, "GET %s", key.c_str());
                if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
                    // Handle error
                    printf("AVIN_DEBUG_STORE08_ store.get %s\n", reply ? reply->str : "Unknown error");
                    freeReplyObject(reply);
                    return "";
                }else{
                    printf("AVIN_DEBUG_STORE09_ store.get success\n");
                }
                std::string value = reply->str;
                freeReplyObject(reply);
                return value;
            }
    };

    class ZkStoreProvider {
        private:
            static ZkStore* zkStore;
        public:
            static ZkStore* instance(){
                if(zkStore != nullptr){
                    std::cout << "\nAVIN_DEBUG_STORE_INIT_01 not_initializing zk::zk-store" << std::endl;
                    return zkStore;
                }
                std::cout << "\nAVIN_DEBUG_STORE_INIT_01 initializing zk::zk-store" << std::endl;
                ZkRedis* hiredisClient = new ZkRedis();
                ZkStore* redisClient = hiredisClient;
                ZkStoreProvider::zkStore = hiredisClient;

                // std::cout << "\nAVIN_DEBUG_STORE_ASYNC00_reader task starting" << std::endl;
                // zk::AsyncTask readerAsyncTask(&readerTask, 1000);
                // readerAsyncTask.Start();

                // std::cout << "\nAVIN_DEBUG_STORE_ASYNC00_writer task starting" << std::endl;
                // zk::AsyncTask writerAsyncTask(&writerTask, 200);
                // writerAsyncTask.Start();

                return redisClient;
            }
    };
    ZkStore* ZkStoreProvider::zkStore = nullptr;
}

#endif // STORE_H