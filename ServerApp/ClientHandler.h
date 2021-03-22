#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <UI/CommandLineImpl.h>
#include <Network/BasicNetwork.h>
#include <FileProcessor/BasicFileProcessor.h>
#include <Common/CommandDefs.h>

#include <memory>
#include <atomic>

//! This class is meant to handle all requests coming from the client side.
//! Whenever a new client is connected to the server a new instance is generated if this class is generated.
class ClientHandler 
{
public:
  ClientHandler(
    const UI::CommandLineImpl& ui, 
    const Network::BasicNetwork& network, 
    const File::BasicFileProcessor& fileProcessor,
    const fs::path& rootPath,
    File::AccessMap& fileAccessMap,
    int socket);
  ClientHandler(const ClientHandler& client);
  ~ClientHandler();

  //! Listens to incoming packets from the matching client defined by m_Socket.
  void listenToPackets();
  
private:
  //! Retreives the server file list and sends back to the client in response for "list files on server" request
  void getFileList();
  //! Processes the client "get files" request by parsing the list of files and initiate transfer.
  Common::Error getSelectedFiles(const Common::Packet& packet);
  //! Processes incoming client test echo.
  void processEcho(const Common::Packet& packet) const;
  //! Processes the client file sending request.
  void getFilesFromClient(const Common::Packet& packet);
  //! Executes reading file content into chunks (see more detail in FilePRocessor) and sending to the client.
  void transfer();
  //! Responds any error happening on the server to the appropriate client.
  void respondError(const Common::Error& error) const;
  //! Responds success message.
  //! There are multiple messages based on which request did finish succesfully.
  //! See CommandType
  //!   WritingToServerDone,
  //!   WritingToClientDone,
  //!   ReadingFromServerDone, 
  void respondSuccess(Common::CommandType responseType) const;
  //! Precesses incoming packet.
  void processPacket(const char* packet);

private:
  std::unique_ptr<UI::CommandLineImpl>      m_xUI{};
  std::unique_ptr<Network::BasicNetwork>    m_xNetwork{};
  std::unique_ptr<File::BasicFileProcessor> m_xFileProcessor{};
  File::AccessMap&                          m_rFileAccessMap;     ///< Access table for every file on the server.
                                                                  ///< If a file is being written by a client. No one else can read or write.
                                                                  ///< If a file is being read by any number of clients no write can happen.
                                                                  ///< readers are maintained by the reference count.

  std::string                               m_SelectedPathMap{};  ///< Holds the list of files to be sent to the client.
  fs::path                                  m_rRootPath{};        ///< Server file storage folder
  std::atomic<bool>                         m_IsListening{false}; ///< Exit criteria for all threads.
  int                                       m_Socket{};           ///< Client socket the handler belongs to.
};

#endif // CLIENT_HANDLER_H