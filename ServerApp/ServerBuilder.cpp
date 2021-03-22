#include "ServerBuilder.h"
#include "Server.h"

//-----------------------------------------------------------------------------
Server* ServerBuilder::buildBasicServer(const std::string& storagePath) const
{
  auto ui = m_Factory.buildCommandLineUI();
  auto config = Network::Config{};
  auto network = m_Factory.buildBasicNetwork(config);
  auto fileProcessorConfig = File::Config{};
  auto fileProcessor =  m_Factory.buildBasicFileProcessor(fileProcessorConfig);
  auto threadConfig = ThreadConfig{};
  auto threadController =  new ThreadController(threadConfig, m_Factory);
  return new Server(*ui, *network, *threadController, storagePath);
}