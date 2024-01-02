#ifndef ZK_SOCKET_H
#define ZK_SOCKET_H

#include <cstdarg>
#include <iostream>
#include <map>
#include <string>
#include "websocketpp/client.hpp"
#include <websocketpp/config/asio_client.hpp>
#include "src/zerok/common/ZkConfigProvider.h"
#include "src/zerok/common/ZkRedisConfig.h"
#include "src/zerok/filters/fetch/AsyncTask.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>

namespace zk {

class ZkSocketClient {
 public:
  virtual bool connect() = 0;
  virtual void disconnect() = 0;
  virtual void send(const std::string& key, const std::string& value) = 0;
  virtual bool recieve(const std::string& key) = 0;
  virtual bool startListening(const std::string& key) = 0;
  virtual void endListening(const char* key, ...) = 0;
  virtual void isConnected() = 0;
};

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;

class ZkSocket : public ZkSocketClient {
 private:
  // Define a WebSocket client type
  int socketConnection = -1;
  client m_endpoint;

 public:
  ZkSocket(){
    // Set up logging
    m_endpoint.set_access_channels(websocketpp::log::alevel::all);
    m_endpoint.clear_access_channels(websocketpp::log::alevel::frame_payload);
  }

  bool connect() override {
  }

  void disconnect() override { 
  }

  void send(const std::string& key, const std::string& value) override {
  }

  bool recieve(const std::string& key) override {
  }

  bool startListening(const std::string& key) override {
  }

  void endListening(const char* key, ...) override {
  }

  void isConnected() override {
  }

  ~ZkSocket() {
  }
};
}  // namespace zk

#endif  // ZK_SOCKET_H