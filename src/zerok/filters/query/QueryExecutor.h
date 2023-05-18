#include "src/zerok/store/store.h"
#include <iostream>
#include <random>
#include <string>

namespace zk{
    class ZkQueryExecutor{
      public:
        static void init(){
            printf("\nAVIN_DEBUG_STORE_INIT_01 initializing zk::zk-query-executor");
            zk::ZkStore* zkStore = zk::ZkStoreProvider::instance();
            zkStore->connect();
        }
    };
}