#include "ThreadController.h"

//-----------------------------------------------------------------------------
ThreadController::ThreadController(const ThreadConfig& config, const Common::GeneralFactory& factory)
: m_Config(config)
, m_rFactory(factory)
{}

//-----------------------------------------------------------------------------
void ThreadController::initAccessMap(const fs::path& root, const std::string& relative)
{
  for(auto it = fs::directory_iterator(root); it != fs::directory_iterator(); ++it)
  {
    auto path = relative;
    auto shortPath = path.append(it->path().string().erase(0, root.string().size()));
    auto size = shortPath.size();
    m_CommonResources.emplace(shortPath, File::Access{});
    auto s = fs::status(*it);
    if (fs::is_directory(s))
    {
      auto subFolder = root;
      subFolder.append((*it).path().string());
      initAccessMap(subFolder, shortPath);
    }
  }
}

//-----------------------------------------------------------------------------
void ThreadController::newThread(int socket, const fs::path rootPath)
{
	auto ui = m_rFactory.buildCommandLineUI();
  auto config = Network::Config{};
  auto network = m_rFactory.buildBasicNetwork(config);
  auto fileProcessorConfig = File::Config{};
  auto fileProcessor =  m_rFactory.buildBasicFileProcessor(fileProcessorConfig);

  ClientHandler handler{*ui, *network, *fileProcessor, rootPath, std::ref(m_CommonResources), socket};
	m_ThreadPool.emplace(socket, handler);

  auto thread = std::thread(&ClientHandler::listenToPackets, m_ThreadPool.at(socket).xHandler.get());
  m_ThreadPool.at(socket).listener = std::move(thread);
}

//-----------------------------------------------------------------------------
void ThreadController::stopThread(int socket)
{
  m_ThreadPool.at(socket).xHandler.reset();
  m_ThreadPool.at(socket).listener.join();
  m_ThreadPool.erase(socket);
}
