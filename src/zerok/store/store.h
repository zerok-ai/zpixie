#ifndef STORE_H
#define STORE_H

#include <cstdarg>
#include <iostream>
#include <map>
#include <string>
#include "hiredis.h"
#include "redis/RedisClient.h"
#include "src/zerok/common/ZkConfigProvider.h"
#include "src/zerok/common/ZkRedisConfig.h"
#include "src/zerok/filters/fetch/AsyncTask.h"

namespace zk {

class ZkStore {
 public:
  virtual bool connect() = 0;
  virtual void disconnect() = 0;
  virtual bool set(const std::string& key, const std::string& value) = 0;
  virtual void addToSet(const char* key, ...) = 0;
  virtual void addToSetWithExpiry(const int expiryaInSeconds, const char* key, ...) = 0;
  virtual std::string get(const std::string& key) = 0;
  virtual std::vector<std::string> hkeys(const std::string& key) = 0;
  virtual std::string hget(const std::string& key) = 0;
  // virtual std::vector<std::string> hgetall(const std::string& key) = 0;
  virtual std::map<std::string, std::string> hgetall(const std::string& key) = 0;
  // virtual bool del(const std::string& key) = 0;
  // virtual bool exists(const std::string& key) = 0;
};

class ZkRedis : public ZkStore {
 private:
  // redisContext* redisConnection;
  RedisClient redisConnection;
  int database = 0;
  // bool setOnce = false;

 public:
  ZkRedis() : ZkStore() {}

  ZkRedis(int databaseNum) : ZkStore() { database = databaseNum; }

  bool connect() override {
    if(redisConnection.isInitialized() == true){
      return true;
    }
    zk::ZkRedisConfig zkRedisConfig = zk::ZkConfigProvider::getZkRedisConfig();
    // REDIS_ENDPOINT endpoints[1] = {{zkRedisConfig.getHost().c_str(), zkRedisConfig.getPort()}};
    REDIS_ENDPOINT endpoints[1];
    std::string host = zkRedisConfig.getHost();
    strcpy(endpoints[0].host, host.c_str());
    endpoints[0].port = zkRedisConfig.getPort();

    REDIS_CONFIG conf = {
        (REDIS_ENDPOINT*)&endpoints, 1, 500, 500, 20, 1,
    };

    RedisClient client(conf);
    redisConnection = client;
    bool authSuccess = auth(zkRedisConfig.getPassword().c_str());
    if (authSuccess) {
      select();
    }else{
      return false;
      printf("zk-log/stores Failed to allocate redis context\n");
    }
    return true;
  }

  // bool connect() override {
  //   if (redisConnection == nullptr) {
  //     zk::ZkRedisConfig zkRedisConfig = zk::ZkConfigProvider::getZkRedisConfig();
  //     // std::cout << "\nAVIN_DEBUG_STORE00_ Connecting " << zkRedisConfig.getHost() << std::endl;
  //     redisConnection = redisConnect(zkRedisConfig.getHost().c_str(), zkRedisConfig.getPort());
  //     bool authSuccess = auth(zkRedisConfig.getPassword().c_str());
  //     if (authSuccess) {
  //       select();
  //     }
  //   } else {
  //     // std::cout << "\nAVIN_DEBUG_STORE00_ Already Connected\n" << std::endl;
  //     // printf("AVIN_DEBUG_STORE00_ Already connected\n");
  //   }
  //   if (redisConnection == nullptr || redisConnection->err) {
  //     if (redisConnection) {
  //       // Handle connection error
  //       printf("zk-log/stores Connection error: %s\n", redisConnection->errstr);
  //       disconnect();
  //       return false;
  //     } else {
  //       // Handle memory allocation error
  //       printf("zk-log/stores Failed to allocate redis context\n");
  //     }
  //     return false;
  //   }
  //   return true;
  // }

  bool select() {
    if (checkForConnection() == false) {
      return false;
    }
    if (database == 0) {
      return true;
    }

    // If you want to select a different database, use redisCommand to send the SELECT command
    RedisReplyPtr reply = redisConnection.redisCommand("SELECT %d", database);
    if (!reply.notNull()) {
      return false;
    }

    return true;
  }

  bool auth(const char* password) {
    if (checkForConnection() == false) {
      return false;
    }
    if (database == 0) {
      return true;
    }

    RedisReplyPtr reply = redisConnection.redisCommand("AUTH %s", password);
    if (!reply.notNull()) {
      return false;
    }

    return true;
  }

  void disconnect() override {
    // if (redisConnection) {
    //   redisFree(redisConnection);
    //   redisConnection = nullptr;
    // }
  }

  void expire(const char* key, const int expiryaInSeconds) {
    if (checkForConnection() == false) {
      return;
    }
    RedisReplyPtr reply = redisConnection.redisCommand("EXPIRE %s %d", key, expiryaInSeconds);
    if (!reply.notNull()) {
      return;
    }
  }

  void addToSetWithExpiry(const int expiryaInSeconds, const char* key, ...) override {
    bool transactionStarted = startTransaction();
    if (transactionStarted) {
      va_list args;
      va_start(args, key);
      addToSet(key, args);
      va_end(args);
      expire(key, expiryaInSeconds);
      endTransaction();
    }
  }

  bool checkForConnection() {
    if(redisConnection.isInitialized() == false){
      return false;
    }
    return true;
  }

  bool startTransaction() {
    if (checkForConnection() == false) {
      return false;
    }
    RedisReplyPtr reply = redisConnection.redisCommand("MULTI");
    if (reply.isNull()) {
      return false;
    }
    return true;
  }

  bool endTransaction() {
    if (checkForConnection() == false) {
      return false;
    }
    RedisReplyPtr reply = redisConnection.redisCommand("EXEC");
    if (!reply.notNull()) {
      return false;
    }
    return true;
  }

  void addToSet(const char* key, ...) override {
    va_list args;
    va_start(args, key);
    addToSet(key, args);
    va_end(args);
  }

  bool addToSet(const char* key, va_list args) {
    if (checkForConnection() == false) {
      return false;
    }
    std::string finalArgs;
    const char* arg = va_arg(args, const char*);
    while (arg != nullptr) {
      if (finalArgs.length() > 0) {
        finalArgs += " ";
      }
      finalArgs += arg;
      arg = va_arg(args, const char*);
    }
    // std::cout << "DerivedClass finalArgs. " << finalArgs << std::endl;

    RedisReplyPtr reply = redisConnection.redisCommand("SADD %s %s", key, finalArgs.c_str());
    if (!reply.notNull()) {
      return false;
    }
    return true;
  }

  bool set(const std::string& key, const std::string& value) override {
    if (checkForConnection() == false) {
      return false;
    }
    // printf("AVIN_DEBUG_STORE04_ store.set\n");
    RedisReplyPtr reply = redisConnection.redisCommand("SET %s %s", key.c_str(), value.c_str());
    if (!reply.notNull()) {
      return false;
    }
    return true;
  }

  std::string get(const std::string& key) override {
    // For testing ebpf attributes
    //  if(key == "2023"){
    //      return
    //      "{\"version\":\"1694015003\",\"scenario_id\":\"2023\",\"scenario_title\":\"test-ebpf\",\"scenario_type\":\"USER\",\"enabled\":true,\"workloads\":{\"55661a0e-25cb-5a1c-ebpf-fad172b0caa2\":{\"service\":\"*/*\",\"trace_role\":\"server\",\"protocol\":\"HTTP\",\"rule\":{\"type\":\"rule_group\",\"condition\":\"AND\",\"rules\":[{\"type\":\"rule\",\"id\":\"http_req_headers\",\"field\":\"req_method\",\"datatype\":\"string\",\"input\":\"string\",\"operator\":\"equal\",\"value\":\"bookstore1.bookstore.svc.cluster.local\"}]}}},\"filter\":{\"type\":\"workload\",\"condition\":\"AND\",\"workload_ids\":[\"55661a0e-25cb-5a1c-ebpf-fad172b0caa2\"]},\"group_by\":[{\"workload_id\":\"55661a0e-25cb-5a1c-ebpf-fad172b0caa2\",\"title\":\"source\",\"hash\":\"source\"}],\"rate_limit\":[{\"bucket_max_size\":5,\"bucket_refill_size\":5,\"tick_duration\":\"1m\"}]}";
    //  }
    if (checkForConnection() == false) {
      return "";
    }
    RedisReplyPtr reply = redisConnection.redisCommand("GET %s", key.c_str());
    if (!reply.notNull()) {
      return "";
    }
    std::string value = reply.get()->str;
    return value;
  }

  std::vector<std::string> hkeys(const std::string& key) override {
    if (checkForConnection() == false) {
      return std::vector<std::string>();
    }
    RedisReplyPtr reply = redisConnection.redisCommand("HKEYS %s", key.c_str());
    if (reply.isNull()) {
      return std::vector<std::string>();
    }
    std::vector<std::string> keys;
    for (int i = 0; static_cast<size_t>(i) < reply.get()->elements; i++) {
      keys.push_back(reply.get()->element[i]->str);
    }
    return keys;
  }

  std::string hget(const std::string& key) override {
    if (checkForConnection() == false) {
      return "";
    }
    RedisReplyPtr reply = redisConnection.redisCommand("HGET %s", key.c_str());
    if (reply.isNull()) {
      return "";
    }
    std::string value = reply.get()->str;
    return value;
  }

  std::map<std::string, std::string> hgetall(const std::string& key) override {
    if (checkForConnection() == false) {
      return std::map<std::string, std::string>();
    }
    RedisReplyPtr reply = redisConnection.redisCommand("HGETALL %s", key.c_str());
    if (reply.isNull()) {
      return std::map<std::string, std::string>();
    }
    std::map<std::string, std::string> values;
    for (int i = 0; static_cast<size_t>(i) < reply.get()->elements; i += 2) {
      values[reply.get()->element[i]->str] = reply.get()->element[i + 1]->str;
    }
    return values;
  }
};

class ZkStoreProvider {
 private:
  static std::map<int, ZkStore*> storeProvider;
  static ZkStore* zkStore;

 public:
  static ZkStore* instance(int database) {
    if (storeProvider.find(database) != storeProvider.end()) {
      return storeProvider[database];
    }
    ZkRedis* hiredisClient = new ZkRedis(database);
    ZkStore* redisClient = hiredisClient;
    storeProvider[database] = redisClient;

    return redisClient;
  }
};
ZkStore* ZkStoreProvider::zkStore = nullptr;
std::map<int, ZkStore*> ZkStoreProvider::storeProvider;
}  // namespace zk

#endif  // STORE_H