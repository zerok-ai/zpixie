#ifndef STORE_H
#define STORE_H

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

#endif // STORE_H