// #include "cpp_redis/cpp_redis"
// #include <hiredis.h>
// #include "/home/avin/.cache/bazel/_bazel_avin/54060b0ed2e63c063d495ae4fb1a7d19/execroot/px/external/com_github_redis_hiredis/hiredis.h"
// #define REDISCPP_HEADER_ONLY
// #include "redis-cpp/stream.h"
// #include "redis-cpp/execute.h"
#include "redis.h"
#include <iostream>

namespace zk {
    class ZkStore{
        public:
            static ZkStore* instance(){
                ZkRedis* hiredisClient = new ZkRedis();
                ZkStore* redisClient = hiredisClient;
                return redisClient;
            }

            virtual bool connect() = 0;
            virtual void disconnect() = 0;
            // virtual bool set(const std::string& key, const std::string& value) = 0;
            // virtual std::string get(const std::string& key) = 0;
            // virtual bool del(const std::string& key) = 0;
            // virtual bool exists(const std::string& key) = 0;
    };
}