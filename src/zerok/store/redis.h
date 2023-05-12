#include "cpp_redis/cpp_redis"
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
                cpp_redis::client client;
                client.connect("127.0.0.1", 6379);
            }
    };
}