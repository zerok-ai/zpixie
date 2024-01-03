#include "socket.h"  // Include the header file with the ZkSocketImpl class

int main() {
  zk::ZkSocketImpl client;

  // Connect to the server
  if (client.zkConnect()) {
    // Send a message to the server
    std::string message = "Hello, Server!";
    if (client.zkSend(message)) {
      std::cout << "Message sent successfully." << std::endl;
    } else {
      std::cerr << "Error sending message." << std::endl;
    }

    // Disconnect from the server
    client.zkDisconnect();
  } else {
    std::cerr << "Error connecting to the server." << std::endl;
  }

  return 0;
}