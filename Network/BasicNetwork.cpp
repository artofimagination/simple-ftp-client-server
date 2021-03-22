#include "BasicNetwork.h"

#include <arpa/inet.h>
#include <unistd.h> 

namespace Network 
{

//-----------------------------------------------------------------------------
BasicNetwork::BasicNetwork(const Config& config)
: m_Config(config)
{}

//-----------------------------------------------------------------------------
Common::Error BasicNetwork::connectClient(int& s)
{
  struct sockaddr_in serv_addr; 
  
  s = socket(AF_INET, SOCK_STREAM, 0);
  if ( s < 0) 
  { 
    return Common::Error("\n Socket creation error\n"); 
  }
  
  serv_addr.sin_family = AF_INET; 
  serv_addr.sin_port = htons(m_Config.port); 
      
  if(inet_pton(AF_INET, m_Config.address.data(), &serv_addr.sin_addr)<=0)  
  { 
    return Common::Error("\nInvalid address/ Address not supported\n"); 
  } 
  
  if(connect(s, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
  { 
    return Common::Error("\nConnection Failed\n"); 
  } 
   
  return Common::Error(); 
}

//-----------------------------------------------------------------------------
Common::Error BasicNetwork::connectServer()
{
  struct sockaddr_in address; 
  int opt = 1; 
    
  // Creating socket file descriptor
  m_SocketFd = socket(m_Config.addressType, m_Config.type, 0); 
  if(m_SocketFd == 0) 
  { 
    return Common::Error("socket failed"); 
  } 
      
  // Forcefully attaching socket to the port
  if(setsockopt(m_SocketFd, SOL_SOCKET, SO_REUSEADDR|SO_REUSEPORT, &opt, sizeof(opt))) 
  { 
    return Common::Error("setsockopt failed"); 
  } 
  address.sin_family = m_Config.addressType; 
  address.sin_addr.s_addr = inet_addr(m_Config.address.data()); 
  address.sin_port = htons(m_Config.port); 
      
  // Forcefully attaching socket to the port
  if(bind(m_SocketFd, (struct sockaddr *)&address, sizeof(address)) < 0) 
  { 
    return Common::Error("bind failed"); 
  } 

  return Common::Error(); 
}

//-----------------------------------------------------------------------------
Common::Error BasicNetwork::listenToConnections()
{
  if(listen(m_SocketFd, m_Config.maxConnection) < 0) 
  { 
    return Common::Error("listen failed"); 
  }

  return Common::Error(); 
}

//-----------------------------------------------------------------------------
Common::Error BasicNetwork::acceptConnection(int& clientSocket)
{
  struct sockaddr_in address; 
  int addrlen = sizeof(address); 
  address.sin_family = m_Config.addressType; 
  address.sin_addr.s_addr = inet_addr(m_Config.address.data()); 
  address.sin_port = htons(m_Config.port);

  clientSocket = accept(m_SocketFd, (struct sockaddr *)&address, (socklen_t*)&addrlen); 
  if(clientSocket < 0) 
  { 
    return Common::Error("accept failed"); 
  }
  return Common::Error(); 
}

//-----------------------------------------------------------------------------
Common::Error BasicNetwork::sendData(int socket, const char* data, size_t size) const
{
  auto valSend = send(socket, data, size, MSG_NOSIGNAL); 
  if (valSend == -1)
  {
    return Common::Error("\nFailed to send data\n"); 
  }
  return Common::Error();
}

//-----------------------------------------------------------------------------
int BasicNetwork::pollSocket(int socket, const char* data, size_t size)
{
  int disconnectCount = 0;
  if (sendData(socket, data, size).empty() == false)
  {
    return -1;
  }
  return 0;
}

//-----------------------------------------------------------------------------
ReadBuffer BasicNetwork::readData(int socket) const
{
  ReadBuffer reader{};
  reader.valueRead = recv(socket, reader.buffer, sizeof(Common::Header), MSG_DONTWAIT);
  auto header = reinterpret_cast<const Common::Header&>(reader.buffer);
  reader.valueRead += recv(socket, reader.buffer + sizeof(Common::Header), header.dataSize, MSG_DONTWAIT);
  return reader;
}

} // Network