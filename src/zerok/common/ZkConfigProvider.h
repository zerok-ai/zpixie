#pragma once

#include "./ZkServiceConfig.h"
#include "./ZkRedisConfig.h"

namespace zk {
    class ZkConfigProvider{
        private:
            static bool initialized;
            static ZkServiceConfig zkConfig;
            static ZkRedisConfig zkRedisConfig;
        
        public:
            static ZkServiceConfig getZkConfig(){
                return zkConfig;
            }

            static ZkRedisConfig getZkRedisConfig(){
                return zkRedisConfig;
            }

            static void init(){
                if(initialized){
                    return;
                }
                initialized = true;
                zkConfig = ZkServiceConfig(false);
                zkRedisConfig = ZkRedisConfig("redis.zk-client.svc.cluster.local", 6379, 1000);
            }

    };

    bool ZkConfigProvider::initialized = false;
    ZkServiceConfig ZkConfigProvider::zkConfig = ZkServiceConfig(false);
    ZkRedisConfig ZkConfigProvider::zkRedisConfig = ZkRedisConfig("redis.zk-client.svc.cluster.local", 6379, 1000);
}