#pragma once

#include "src/zerok/common/ZkConfigProvider.h"

namespace zk {
    class ZkConfig{
        public:
            static bool isAllowAllCalls(){
                return zk::ZkConfigProvider::getZkConfig().isAllowAllCalls();
            }

            static bool isMySqlEnabled(){
                return zk::ZkConfigProvider::getZkMySqlConfig().isEnabled();
            }

            static bool isMySqlTraceEnabled(){
                return zk::ZkConfigProvider::getZkMySqlConfig().isTraceEnabled();
            }

            static bool isMySqlNonTracedAllowed(){
                return zk::ZkConfigProvider::getZkMySqlConfig().isAllowNonTraced();
            }

            static bool isPgSqlEnabled(){
                return zk::ZkConfigProvider::getZkPgSqlConfig().isEnabled();
            }

            static bool isPgSqlTraceEnabled(){
                return zk::ZkConfigProvider::getZkPgSqlConfig().isTraceEnabled();
            }

            static bool isPgSqlNonTracedAllowed(){
                return zk::ZkConfigProvider::getZkPgSqlConfig().isAllowNonTraced();
            }

    };
}