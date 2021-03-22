#ifndef SERVER_H
#define SERVER_H

#include "ThreadController.h"

#include <UI/CommandLineImpl.h>
#include <Network/BasicNetwork.h>
#include <FileProcessor/BasicFileProcessor.h>
#include <Common/CommandDefs.h>

#include <memory>
#include <atomic>

class Server 
{
public:
  Server(
    const UI::CommandLineImpl& ui, 
    const Network::BasicNetwork& network, 
    const ThreadController& threadController,
    const std::string& storagePath);
  Server(const Server& client);
  ~Server();

  void run();
  
private:
  void listenToConnections();
  void connect();
  void pollClients();

private:
  std::unique_ptr<UI::CommandLineImpl>      m_xUI{};
  std::unique_ptr<Network::BasicNetwork>    m_xNetwork{};
  std::unique_ptr<ThreadController>         m_xThreadController{};

  std::thread                               m_ConnectionListener;
  std::thread                               m_ClientPoller;
  fs::path                                  m_RootPath{};
  std::vector<int>                          m_Clients{};
  std::atomic<bool>                         m_IsListening{false};
};

#endif // SERVER_H