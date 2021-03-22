#include "ClientHandler.h"

#include <thread>

//-----------------------------------------------------------------------------
ClientHandler::ClientHandler(
    const UI::CommandLineImpl& ui, 
    const Network::BasicNetwork& network, 
    const File::BasicFileProcessor& fileProcessor,
    const fs::path& rootPath,
    File::AccessMap& fileAccessMap,
    int socket)
  : m_xUI(std::make_unique<UI::CommandLineImpl>(ui))
  , m_xNetwork(std::make_unique<Network::BasicNetwork>(network))
  , m_xFileProcessor(std::make_unique<File::BasicFileProcessor>(fileProcessor))
  , m_rRootPath(rootPath)
  , m_rFileAccessMap(fileAccessMap)
  , m_Socket(socket)
{}

//-----------------------------------------------------------------------------
ClientHandler::ClientHandler(const ClientHandler& handler)
  : m_xUI(std::make_unique<UI::CommandLineImpl>(*handler.m_xUI.get()))
  , m_xNetwork(std::make_unique<Network::BasicNetwork>(*handler.m_xNetwork.get()))
  , m_xFileProcessor(std::make_unique<File::BasicFileProcessor>(*handler.m_xFileProcessor.get()))
  , m_rRootPath(handler.m_rRootPath)
  , m_rFileAccessMap(handler.m_rFileAccessMap)
  , m_Socket(handler.m_Socket)
{}

//-----------------------------------------------------------------------------
ClientHandler::~ClientHandler()
{
  m_IsListening = false;
}

//-----------------------------------------------------------------------------
void ClientHandler::listenToPackets()
{
  m_IsListening = true;
  while (m_IsListening)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto packet = m_xNetwork->readData(m_Socket);
    if (packet.valueRead > 0)
      processPacket(reinterpret_cast<const char*>(&packet.buffer)); 
  }
}

//-----------------------------------------------------------------------------
Common::Error ClientHandler::getSelectedFiles(const Common::Packet& packet)
{
  std::string selectedFiles(packet.data);
  if (selectedFiles.size() != packet.header.dataSize)
  {
    auto error = std::string(__PRETTY_FUNCTION__) + ": corrupted packet";
    m_xUI->printError(error);
    respondError(error);
    return error;
  }
  m_SelectedPathMap = selectedFiles;
  return Common::Error();
}

//-----------------------------------------------------------------------------
void ClientHandler::transfer()
{
  if(m_xFileProcessor->isReadingFinished() == false)
  {
    File::FsBuffer chunk{};
    auto error = m_xFileProcessor->processReadRequest(m_rFileAccessMap, m_SelectedPathMap, m_rRootPath, chunk);
    if (error.empty() == false)
    {
      m_xUI->printError(error);
      respondError(error);
      return;
    }
    Common::Packet packet{};
    packet.header.command = Common::CommandType::GetFromServer;
    packet.header.dataSize = File::cChunkSize_bytes + sizeof(File::FsBufferHeader);
    memcpy(packet.data, reinterpret_cast<char*>(&chunk), packet.header.dataSize);
    m_xNetwork->sendData(m_Socket, reinterpret_cast<char*>(&packet), sizeof(Common::Header) + packet.header.dataSize);
    return;
  }
  respondSuccess(Common::CommandType::ReadingFromServerDone);
}

//-----------------------------------------------------------------------------
void ClientHandler::processPacket(const char* data)
{
  Common::Packet packet{};
  Common::convertRawDataToPacket(data, packet);
  switch (packet.header.command)
  {
  case Common::CommandType::ListServerFiles:
    getFileList();
    break;
  case Common::CommandType::GetFromServer:
    if (getSelectedFiles(packet).empty())
      transfer();
    break;
  case Common::CommandType::WritingToClientDone:
    transfer();
    break;
  case Common::CommandType::SendToServer:
    getFilesFromClient(packet);
    break;
  case Common::CommandType::SendEcho:
    processEcho(packet);
    break;
  default:
    std::string error = "Invalid command opcode: " + static_cast<int>(packet.header.command);
    m_xUI->printError(error);
    respondError(error);
    break;
  }
}

//-----------------------------------------------------------------------------
void ClientHandler::respondError(const Common::Error& error) const
{
  Common::Packet packetReturn{};
  packetReturn.header.command = Common::CommandType::Error;
  packetReturn.header.dataSize = error.size();
  memcpy(packetReturn.data, error.data(), error.size());
  m_xNetwork->sendData(m_Socket, reinterpret_cast<char*>(&packetReturn), sizeof(Common::Header) + packetReturn.header.dataSize);
}

//-----------------------------------------------------------------------------
void ClientHandler::respondSuccess(Common::CommandType responseType) const
{
  Common::Packet packetReturn{};
  packetReturn.header.command = responseType;
  m_xNetwork->sendData(m_Socket, reinterpret_cast<char*>(&packetReturn), sizeof(Common::Header));
}

//-----------------------------------------------------------------------------
void ClientHandler::getFileList()
{
  // Note: Can't handle list longer than the cChunkSize.
  Common::Packet packet{};
  File::getContent(m_rRootPath, "", packet.data, packet.header.dataSize);
  packet.header.command = Common::CommandType::ListServerFiles;
  m_xNetwork->sendData(m_Socket, reinterpret_cast<char*>(&packet), sizeof(Common::Header) + packet.header.dataSize);
}

//-----------------------------------------------------------------------------
void ClientHandler::getFilesFromClient(const Common::Packet& packet)
{ 
  auto error = m_xFileProcessor->writeFiles(m_rFileAccessMap, m_Socket, packet.data, packet.header.dataSize, m_rRootPath);
  if (error.empty() == false)
  {
    respondError(error);
    return;
  }
  respondSuccess(Common::CommandType::WritingToServerDone);
}

//-----------------------------------------------------------------------------
void ClientHandler::processEcho(const Common::Packet& packet) const
{
  std::string echo(packet.data, packet.header.dataSize);
  if (echo.size() != packet.header.dataSize)
  {
    auto error = std::string(__PRETTY_FUNCTION__) + ": corrupted packet";
    m_xUI->printError(error);
    respondError(error);
    return;
  }
  m_xUI->printMessage(echo);

  std::string echoReturn{"Server echo!"};
  Common::Packet packetReturn{};
  packetReturn.header.command = Common::CommandType::SendEcho;
  packetReturn.header.dataSize = echoReturn.size();
  memcpy(packetReturn.data, echoReturn.data(), echoReturn.size());
  m_xNetwork->sendData(m_Socket, reinterpret_cast<char*>(&packetReturn), sizeof(Common::Header) + packetReturn.header.dataSize);
}
