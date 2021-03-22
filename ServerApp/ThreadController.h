#ifndef THREAD_CONTROLLER_H
#define THREAD_CONTROLLER_H

#include "ClientHandler.h"
#include <Common/GeneralFactory.h>

#include <thread>
#include <vector>
#include <memory>

struct ThreadConfig 
{
  int maxThreadCount{10};
};

//! This struct encapsulates a single instance of Client handler.
//! For every client a separate Thread instance is created.
//! It contains the thread itself and the acting object.
struct Thread
{
  Thread(const ClientHandler& handler)
    : xHandler(std::make_unique<ClientHandler>(handler))
  {}

  Thread(const Thread& right)
  : xHandler(std::make_unique<ClientHandler>(*right.xHandler))
  {}

  std::thread                     listener;
  std::unique_ptr<ClientHandler>  xHandler{};
};

//! Maintains the client handling threads.
class ThreadController 
{
public:
  ThreadController(const ThreadConfig& config, const Common::GeneralFactory& factory);

  void newThread(int socket, const fs::path rootPath);
  void stopThread(int socket);

  const std::map<int, Thread>& getThreadPool() { return m_ThreadPool; }

  //! Initializes the file access map.
  void initAccessMap(const fs::path& root, const std::string& relative);
private:
  std::map<int, Thread>         m_ThreadPool{};
  File::AccessMap               m_CommonResources{};  ///< Handles the access to every file being stored in the server folder
                                                      ///< The map is filled during writing and access type is changed based on read/write
                                                      ///< There can only a single writer at the same time and multiple readers.
                                                      ///< If there is a writer no reader can access.
                                                      ///< If there is at elast one reader no writing can happen.
  const Common::GeneralFactory& m_rFactory{};
  ThreadConfig                  m_Config{};
};

#endif // THREAD_CONTROLLER_H