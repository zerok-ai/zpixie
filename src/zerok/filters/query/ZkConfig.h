#pragma once

#include "src/zerok/common/ZkConfigProvider.h"

namespace zk {
    class ZkConfig{
        public:
            static bool isAllowAllCalls(){
                return zk::ZkConfigProvider::getZkConfig()->isAllowAllCalls();
            }

    };
}