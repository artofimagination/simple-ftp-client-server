#ifndef BASIC_NETWORK_H
#define BASIC_NETWORK_H

#include <Common/Error.h>
#include <Common/CommandDefs.h>

#include <sys/socket.h> 
#include <vector>
#include <map>

namespace Network 
{

struct Config
{
  int         type{SOCK_STREAM|SOCK_NONBLOCK};
  int         addressType{AF_INET};
  int         port {8080};
  std::string address{"127.0.0.1"};
  int         maxConnection{};
};

struct ReadBuffer {
  int     valueRead{};
  char    buffer[Common::cMaxPayloadSize]{};
};

class BasicNetwork 
{
public:
  BasicNetwork(const Config& config);

  Common::Error connectClient(int& s);
  Common::Error connectServer();
  Common::Error listenToConnections();
  Common::Error acceptConnection(int& clientSocket);
  Common::Error sendData(int socket, const char* data, size_t size) const;
  ReadBuffer readData(int socket) const;
  Common::Error disconnectNetwork();
  int pollSocket(int socket, const char* data, size_t size);

private:

  Config            m_Config{};
  int               m_SocketFd{};
};

} // Network

#endif // BASIC_NETWORK_H