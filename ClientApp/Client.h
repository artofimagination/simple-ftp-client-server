#ifndef CLIENT_H
#define CLIENT_H

#include <UI/CommandLineImpl.h>
#include <Network/BasicNetwork.h>
#include <FileProcessor/BasicFileProcessor.h>
#include <Common/CommandDefs.h>

#include <memory>
#include <atomic>

typedef std::map<int, std::string> OptionsMap;

//! Handles network, file processing based on the user input.
//! There is no server polling implemented, if server is down it is not known until user interaction.
//! There are two threads besides the main. One handles server packet polling through listenToPackets().
//! And the second to handle command line input, through runCommandLine().
class Client 
{
public:
  Client(
    const UI::CommandLineImpl& ui, 
    const Network::BasicNetwork& network, 
    const File::BasicFileProcessor& fileProcessor);
  Client(const Client& client);

  //! Runs the client application.
  void run();

private:
  //! Sends "list files on server" command.
  void sendListFilesOnServer();
  //! Sends files over the server. The transfer is broken down into chunks. 
  //! If there are too many files or the file is too large it will be sent by multiple chunks.
  //! For ease of implementation network packets are almost the same size as the chunks (the difference is only the added packet header)
  void sendFilesToServer();
  //! Sends a "file get" request to the server.
  //! Similarly to file send the transfer is broken down into chunks.
  void sendGetFilesFromServer();
  //! Sends and receives a test echo to the server.
  void sendEcho();
  //! Transfers a single chunk of data.
  void transfer();
  
  //! Shows ui content
  void showUI() const;
  //! Terminates the application.
  void quit();
  //! Attempts to connect to the server.
  void connect();
  //! Handles command line user inputs.
  void runCommandLine();
  //! Selects the appropriate execution path based on user input.
  void commandSelection(int selection);
  //! Process incoming packet.
  void processPacket(const char* packet);
  //! Thread function to listen to server messages.
  void listenToPackets();
  //! Processes the server response on "list files on server" request.
  void listFilesFromServer(const Common::Packet& packet);
  //! Processes the server response on "file get" request.
  void getFilesFromServer(const Common::Packet& packet);
  //! Processes test echo response.
  void processEcho(const Common::Packet& packet) const;

private:
  std::unique_ptr<UI::CommandLineImpl>      m_xUI{};
  std::unique_ptr<Network::BasicNetwork>    m_xNetwork{};
  std::unique_ptr<File::BasicFileProcessor> m_xFileProcessor{};

  int                                       m_Socket{};           ///< Socket of the client
  OptionsMap                                m_Options{};          ///< Available command options.
  std::string                               m_SelectedFiles{};
  std::string                               m_DestinationFolder{};
  std::atomic<bool>                         m_IsListening{false};
};

#endif // CLIENT_H