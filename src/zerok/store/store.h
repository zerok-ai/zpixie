#ifndef STORE_H
#define STORE_H

#include <iostream>
#include "/home/avin/.cache/bazel/_bazel_avin/54060b0ed2e63c063d495ae4fb1a7d19/execroot/px/external/com_github_redis_hiredis/hiredis.h"

namespace zk {
    class ZkStore{
        public:
            virtual bool connect() = 0;
            virtual void disconnect() = 0;
            // virtual bool set(const std::string& key, const std::string& value) = 0;
            // virtual std::string get(const std::string& key) = 0;
            // virtual bool del(const std::string& key) = 0;
            // virtual bool exists(const std::string& key) = 0;
    };

    class ZkRedis : public ZkStore{
        private:
            redisContext* redisConnection;

        public:
            ZkRedis() : redisConnection(nullptr) {}
            bool connect() override {
                printf("AVIN_DEBUG_STORE00_ Connection error: %s\n", redisConnection->errstr);
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
                }
                return true;
            }

            void disconnect() override {
                if (redisConnection) {
                    redisFree(redisConnection);
                    redisConnection = nullptr;
                }
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