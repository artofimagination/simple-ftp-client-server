#include "Server.h"

#include <Common/CommandDefs.h>

#include <thread>
#include <chrono>

//-----------------------------------------------------------------------------
Server::Server(
    const UI::CommandLineImpl& ui, 
    const Network::BasicNetwork& network, 
    const ThreadController& threadController,
    const std::string& storagePath)
  : m_xUI(std::make_unique<UI::CommandLineImpl>(ui))
  , m_xNetwork(std::make_unique<Network::BasicNetwork>(network))
  , m_xThreadController(std::make_unique<ThreadController>(threadController))
  , m_RootPath(storagePath)
{
  if (fs::exists(m_RootPath) == false)
  {
    fs::create_directory(m_RootPath);
  }

  m_xThreadController->initAccessMap(m_RootPath, "");
}

//-----------------------------------------------------------------------------
Server::Server(const Server& server)
  : m_xUI(std::make_unique<UI::CommandLineImpl>(*server.m_xUI.get()))
  , m_xNetwork(std::make_unique<Network::BasicNetwork>(*server.m_xNetwork.get()))
  , m_xThreadController(std::make_unique<ThreadController>(*server.m_xThreadController))
  , m_RootPath(server.m_RootPath)
{
  if (fs::exists(m_RootPath) == false)
  {
    fs::create_directory(m_RootPath);
  }

  m_xThreadController->initAccessMap(m_RootPath, "");
}

//-----------------------------------------------------------------------------
Server::~Server()
{
  m_IsListening = false;
}

//-----------------------------------------------------------------------------
void Server::run()
{ 
  connect();
  while(true)
  {}
}

//-----------------------------------------------------------------------------
void Server::listenToConnections()
{
  m_xUI->printMessage("Listening to new client connections...");
  m_IsListening = true;
  while (m_IsListening)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    int clientSocket = 0;
    if (m_xNetwork->acceptConnection(clientSocket).empty())
      m_xUI->printMessage("Accepted new client socket(" + std::to_string(clientSocket) + ") connection...");

    if (clientSocket != -1)
      m_xThreadController->newThread(clientSocket, m_RootPath);
  }
}

//-----------------------------------------------------------------------------
void Server::pollClients()
{
  m_IsListening = true;
  while (m_IsListening)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    Common::Packet packetReturn{};
    packetReturn.header.command = Common::CommandType::PollClient;
    for(auto thread : m_xThreadController->getThreadPool())
    {
      auto state = m_xNetwork->pollSocket(thread.first, reinterpret_cast<char*>(&packetReturn), sizeof(Common::Header));
      if (state == -1)
      {
        m_xThreadController->stopThread(thread.first);
        m_xUI->printMessage("Client socket(" + std::to_string(thread.first) + ") have been disconnected");
        break;
      } 
    }
  }
}

//-----------------------------------------------------------------------------
void Server::connect()
{
  auto error = m_xNetwork->connectServer();
  if (error.empty() == false)
  {
    m_xUI->printError(error);
    exit(EXIT_FAILURE);
  }
  m_xUI->printMessage("Server connection is ready");
  m_xNetwork->listenToConnections();
  m_ConnectionListener = std::thread(&Server::listenToConnections, this);
  m_ClientPoller = std::thread(&Server::pollClients, this);
  m_ClientPoller.join();
  m_ConnectionListener.join();
  m_xUI->printMessage("Server connection is finished");
}