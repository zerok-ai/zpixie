#ifndef STORE_H
#define STORE_H

#include <iostream>
#include <string>
#include "/home/avin/.cache/bazel/_bazel_avin/54060b0ed2e63c063d495ae4fb1a7d19/execroot/px/external/com_github_redis_hiredis/hiredis.h"

namespace zk {
    class ZkStore{
        public:
            virtual bool connect() = 0;
            virtual void disconnect() = 0;
            virtual bool set(const std::string& key, const std::string& value) = 0;
            virtual std::string get(const std::string& key) = 0;
            // virtual bool del(const std::string& key) = 0;
            // virtual bool exists(const std::string& key) = 0;
    };

    class ZkRedis : public ZkStore{
        private:
            redisContext* redisConnection;

        public:
            ZkRedis() : redisConnection(nullptr) {}
            bool connect() override {
                printf("AVIN_DEBUG_STORE00_ Trying to connect\n");
                redisConnection = redisConnect("redis.redis.svc.cluster.local", 6379);
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
                    printf("AVIN_DEBUG_STORE03_ Connected\n");
                }
                return true;
            }

            void disconnect() override {
                if (redisConnection) {
                    redisFree(redisConnection);
                    redisConnection = nullptr;
                }
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
                redisReply* reply = (redisReply*)redisCommand(redisConnection, "GET %s", key.c_str());
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
        public:
            static ZkStore* instance(){
                ZkRedis* hiredisClient = new ZkRedis();
                ZkStore* redisClient = hiredisClient;
                return redisClient;
            }
    };
}

#endif // STORE_H