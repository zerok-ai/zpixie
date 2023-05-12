// #include "cpp_redis/cpp_redis"
// #include <hiredis.h>
#include "/home/avin/.cache/bazel/_bazel_avin/54060b0ed2e63c063d495ae4fb1a7d19/execroot/px/external/com_github_redis_hiredis/hiredis.h"
// #define REDISCPP_HEADER_ONLY
// #include "redis-cpp/stream.h"
// #include "redis-cpp/execute.h"
// #include <iostream>

namespace zk {
    class ZkStore{
        public:
            void connect(){
                // auto stream = rediscpp::make_stream("localhost", "6379");
                // auto response = rediscpp::execute(*stream, "ping");
                // std::cout << response.as<std::string>() << std::endl;
                // cpp_redis::client client;
                // client.connect("127.0.0.1", 6379);

                redisContext* context = redisConnect("127.0.0.1", 6379);
            }
    };
}