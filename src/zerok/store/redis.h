// #include "cpp_redis/cpp_redis"
// #include <hiredis.h>
#include "/home/avin/.cache/bazel/_bazel_avin/54060b0ed2e63c063d495ae4fb1a7d19/execroot/px/external/com_github_redis_hiredis/hiredis.h"
// #define REDISCPP_HEADER_ONLY
// #include "redis-cpp/stream.h"
// #include "redis-cpp/execute.h"
#include "store.h"
#include <iostream>

namespace zk {
    class ZkRedis : public ZkStore{
        private:
            redisContext* redisConnection;

        public:
            ZkRedis() : redisConnection(nullptr) {}
            bool connect() override {
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
}