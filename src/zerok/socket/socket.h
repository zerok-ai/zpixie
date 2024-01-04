#ifndef SOCKET_H
#define SOCKET_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>

namespace zk {

class ZkSocket {
 public:
  virtual bool zkConnect() = 0;
  virtual void zkDisconnect() = 0;
  virtual bool zkSend(const std::string& value) = 0;
};

class ZkSocketImpl : public ZkSocket {
 private:
  int clientSocket;
  // const char* SERVER_IP = "127.0.0.1";
  // const int PORT = 12345;
  const char* SERVER_IP = "socket-server.zk-client.svc.cluster.local";
  const int PORT = 8080;

 public:
  ZkSocketImpl() : clientSocket(-1) {}

  bool zkConnect() override {
    // Create a socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
      std::cerr << "Error creating socket." << std::endl;
      return false;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddress.sin_port = htons(PORT);

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
      std::cerr << "Error connecting to server." << std::endl;
      zkDisconnect();
      return false;
    }
    std::cout << "Connected to server." << std::endl;

    return true;
  }

  bool isSocketConnected() {
    int socket = clientSocket;
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1) {
      // Handle error, fcntl failed
      return false;
    }

    // Set the socket to non-blocking mode
    if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1) {
      // Handle error, fcntl failed
      return false;
    }

    // Try to send a small amount of data to check the connection
    char testBuffer[1] = {' '};
    ssize_t result = send(socket, testBuffer, sizeof(testBuffer), MSG_DONTWAIT);

    // Set the socket back to blocking mode
    if (fcntl(socket, F_SETFL, flags) == -1) {
      // Handle error, fcntl failed
      return false;
    }

    if (result == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      // Connection is still valid
      return true;
    }

    // Connection might be closed or in an error state
    return false;
  }

  void zkDisconnect() override {
    // Close the socket
    close(clientSocket);
    clientSocket = -1;
  }

  bool zkSend(const std::string& value) override {
    if (clientSocket == -1) {
      std::cerr << "Error sending data. Socket is not connected." << std::endl;
      bool gotConnected = zkConnect();
      std::cerr << "Got connected: " << gotConnected << std::endl;
      if (!gotConnected) {
        return false;
      }
    }
    std::vector<char> buffer(value.begin(), value.end());
    buffer.push_back('\0');  // Ensure null-termination

    ssize_t bytesSent = send(clientSocket, buffer.data(), buffer.size(), 0);
    if (bytesSent == -1) {
      std::cerr << "Error sending data." << std::endl;
      if (isSocketConnected()) {
        std::cout << "Socket is still connected." << std::endl;
      } else {
        std::cout << "Socket is not connected." << std::endl;
        zkDisconnect();
      }
      return false;
    }

    return true;
  };
};
}  // namespace zk

#endif  // SOCKET_H