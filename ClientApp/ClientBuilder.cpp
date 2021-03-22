#include "ClientBuilder.h"

#include "Client.h"


//-----------------------------------------------------------------------------
Client* ClientBuilder::buildBasicClient() const
{
  auto ui = m_Factory.buildCommandLineUI();

  auto networkConfig = Network::Config{};
  auto network = m_Factory.buildBasicNetwork(networkConfig);

  auto fileProcessorConfig = File::Config{};
  auto fileProcessor =  m_Factory.buildBasicFileProcessor(fileProcessorConfig);
  return new Client(*ui, *network, *fileProcessor);
}