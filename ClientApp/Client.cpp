#include "Client.h"

#include <thread>

//-----------------------------------------------------------------------------
std::map<int, std::string> generateOptions()
{
  return std::map<int, std::string>
  {
    { 1, "List files on server" },
    { 2, "Send files to server" }, 
    { 3, "Get files from server" },
    { 4, "Send echo" },
    { 5, "Quit" }
  };
}

//-----------------------------------------------------------------------------
inline bool isInteger(const std::string & s)
{
   if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

   char * p;
   strtol(s.c_str(), &p, 10);

   return (*p == 0);
}

//-----------------------------------------------------------------------------
Client::Client(
    const UI::CommandLineImpl& ui, 
    const Network::BasicNetwork& network, 
    const File::BasicFileProcessor& fileProcessor)
  : m_xUI(std::make_unique<UI::CommandLineImpl>(ui))
  , m_xNetwork(std::make_unique<Network::BasicNetwork>(network))
  , m_xFileProcessor(std::make_unique<File::BasicFileProcessor>(fileProcessor))
{
  m_Options = generateOptions();
}

//-----------------------------------------------------------------------------
Client::Client(const Client& client)
  : m_xUI(std::make_unique<UI::CommandLineImpl>(*client.m_xUI.get()))
  , m_xNetwork(std::make_unique<Network::BasicNetwork>(*client.m_xNetwork.get()))
  , m_xFileProcessor(std::make_unique<File::BasicFileProcessor>(*client.m_xFileProcessor.get()))
  , m_Options(client.m_Options)
  , m_Socket(client.m_Socket)
{}

//-----------------------------------------------------------------------------
void Client::run()
{
  connect();
}

//-----------------------------------------------------------------------------
void Client::runCommandLine()
{
  m_xUI->printWelcome();
  showUI();
  while(true)
  {
    auto option = m_xUI->listenToInput();
    if (isInteger(option) == false)
    {
      m_xUI->printError("Invalid option string");
      continue;
    }
    commandSelection(std::stoi(option));
  }
}

//-----------------------------------------------------------------------------
void Client::showUI() const
{
  m_xUI->printMessage("");
  m_xUI->listOptions(m_Options);
  m_xUI->printMessage("Select option:");
}

//-----------------------------------------------------------------------------
void Client::commandSelection(int selection)
{
  switch(selection)
  {
    case 1:
      sendListFilesOnServer();
      break;
    case 2:
      sendFilesToServer();
      break;
    case 3:
      sendGetFilesFromServer();
      break;
    case 4:
      sendEcho();
      break;
    case 5:
      quit();
      break;
    default:
      m_xUI->printError("Invalid option: " + selection);
  }  
}

//-----------------------------------------------------------------------------
void Client::sendGetFilesFromServer()
{
  m_xUI->printMessage("Select files to get from the server.");
  m_xUI->printMessage("It is is enough to type in the relative path to the server folder.");
  m_xUI->printMessage("To get multiple files, similarly to send, separate with \";\"");
  m_xUI->printMessage("Example if server folder is /home/test/server/: test.txt;subfolder/test2.txt;test5.txt");
  std::string selectedFiles = m_xUI->listenToInput();
  m_xUI->printMessage("Set the absolute path of destination. If the folder does not exists it will be created.");
  // NOTE: not thread safe
  m_DestinationFolder = m_xUI->listenToInput();
  Common::Packet packet{};
  packet.header.command = Common::CommandType::GetFromServer;
  packet.header.dataSize = selectedFiles.size();
  memcpy(packet.data, selectedFiles.data(), selectedFiles.size());
  m_xNetwork->sendData(m_Socket, reinterpret_cast<char*>(&packet), sizeof(Common::Header) + packet.header.dataSize);
}

//-----------------------------------------------------------------------------
void Client::sendEcho()
{
  m_xUI->printMessage("Sending echo to server");
  std::string echo{"Client echo"};
  Common::Packet packet{};
  packet.header.command = Common::CommandType::SendEcho;
  packet.header.dataSize = echo.size();
  memcpy(packet.data, echo.data(), echo.size());
  m_xNetwork->sendData(m_Socket, reinterpret_cast<char*>(&packet), sizeof(Common::Header) + packet.header.dataSize);
}

//-----------------------------------------------------------------------------
void Client::sendFilesToServer()
{
  m_xUI->printMessage("Select files to send to server from your local filesystem.");
  m_xUI->printMessage("Please define absolute path and separated with \";\" if there are multiple files.");
  m_xUI->printMessage("All files selected shall reside in the same folder. For example the /home/test in case of the example below.");
  m_xUI->printMessage("Example: /home/test/file1.bin;/home/test/testFolder;/home/test/file2.txt");
  // NOTE: not thread safe
  m_SelectedFiles = m_xUI->listenToInput();
  
  transfer();
}

//-----------------------------------------------------------------------------
void Client::sendListFilesOnServer()
{
  Common::Packet packet{};
  packet.header.command = Common::CommandType::ListServerFiles;
  m_xNetwork->sendData(m_Socket, reinterpret_cast<char*>(&packet), sizeof(Common::Header));
}

//-----------------------------------------------------------------------------
void Client::transfer()
{
  if(m_xFileProcessor->isReadingFinished() == false)
  {
    File::FsBuffer chunk{};
    File::AccessMap  accessMap{};
    auto error = m_xFileProcessor->processReadRequest(accessMap, m_SelectedFiles, "", chunk);
    if (error.empty() == false)
    {
      m_xUI->printError(error);
    }
    Common::Packet packet{};
    packet.header.command = Common::CommandType::SendToServer;
    packet.header.dataSize = File::cChunkSize_bytes + sizeof(File::FsBufferHeader);
    memcpy(packet.data, reinterpret_cast<char*>(&chunk), packet.header.dataSize);
    m_xNetwork->sendData(m_Socket, reinterpret_cast<char*>(&packet), sizeof(Common::Header) + packet.header.dataSize);
    return;
  }
  else
  {
    m_xFileProcessor->reset();
  }

  m_xUI->printMessage("Transfer to server completed");
}

//-----------------------------------------------------------------------------
void Client::listenToPackets()
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
void Client::processPacket(const char* data)
{
  Common::Packet packet{};
  Common::convertRawDataToPacket(data, packet);

  switch (packet.header.command)
  {
  case Common::CommandType::ListServerFiles:
    listFilesFromServer(packet);
    break;
  case Common::CommandType::GetFromServer:
    getFilesFromServer(packet);
    break;
  case Common::CommandType::WritingToServerDone:
    transfer();
    break;
  case Common::CommandType::SendToServer:
    transfer();
    break;
  case Common::CommandType::ReadingFromServerDone:
    m_xUI->printMessage("Transfer from server completed");
    break;
  case Common::CommandType::SendEcho:
    processEcho(packet);
    break;
  case Common::CommandType::PollClient:
    break;
  case Common::CommandType::Error:
  {
    std::string error(packet.data);
    if (error.size() != packet.header.dataSize)
    {
      m_xUI->printError(std::string(__PRETTY_FUNCTION__) + ": corrupted packet");
      return;
    }
    m_xUI->printError("Server " + error);
    exit(EXIT_FAILURE);
    break;
  }
  default:
    m_xUI->printError("Invalid command opcode: " + static_cast<int>(packet.header.command));
    break;
  }
}

//-----------------------------------------------------------------------------
void Client::processEcho(const Common::Packet& packet) const
{
  std::string echo(packet.data);
  if (echo.size() != packet.header.dataSize)
  {
    m_xUI->printError(std::string(__PRETTY_FUNCTION__) + ": corrupted packet");
    return;
  }
  m_xUI->printMessage(echo);
}

//-----------------------------------------------------------------------------
void Client::listFilesFromServer(const Common::Packet& packet)
{
  if (packet.header.dataSize == 0)
  {
    return m_xUI->printMessage("No files on server yet");
  }

  std::vector<std::string> contentVector{};
  auto error = File::convertRawToStringVector(packet.data, packet.header.dataSize, contentVector);
  if (error.empty() == false)
  {
    m_xUI->printError(error);
  }

  for (const auto& file : contentVector)
  {
    m_xUI->printMessage(file);
  }
  return m_xUI->printMessage("Listing completed.");  
}

//-----------------------------------------------------------------------------
void Client::getFilesFromServer(const Common::Packet& packet)
{
  File::AccessMap  accessMap{};
  if (fs::exists(m_DestinationFolder) == false)
  {
    if (fs::exists(m_DestinationFolder) == false && fs::create_directory(m_DestinationFolder) == false)
    {
      m_xUI->printError("Failed to create " + m_DestinationFolder + " directory");
      return;
    }
  }

  auto error = m_xFileProcessor->writeFiles(accessMap, m_Socket, packet.data, packet.header.dataSize, m_DestinationFolder);
  if (error.empty() == false)
  {
    m_xUI->printError(error);
    return;
  }
  Common::Packet packetReturn{};
  packetReturn.header.command = Common::CommandType::WritingToClientDone;
  m_xNetwork->sendData(m_Socket, reinterpret_cast<char*>(&packetReturn), sizeof(Common::Header));
}

//-----------------------------------------------------------------------------
void Client::quit()
{
  m_IsListening = false;
  m_xUI->printMessage("Quitting... Thank you for using the app.");
  exit(EXIT_SUCCESS);
}

//-----------------------------------------------------------------------------
void Client::connect()
{
  auto error = m_xNetwork->connectClient(m_Socket);
  if (error.empty() == false)
  {
    m_xUI->printError(error);
    exit(EXIT_FAILURE);
  }
  std::thread packetPoller(&Client::listenToPackets, this);
  std::thread cmdThread(&Client::runCommandLine, this);
  packetPoller.join();
  cmdThread.join();
}
