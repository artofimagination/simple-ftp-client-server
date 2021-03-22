#include "GeneralFactory.h"

namespace Common 
{

//-----------------------------------------------------------------------------
UI::CommandLineImpl* GeneralFactory::buildCommandLineUI() const
{
  return new UI::CommandLineImpl();
}

//-----------------------------------------------------------------------------
Network::BasicNetwork* GeneralFactory::buildBasicNetwork(const Network::Config& config) const
{
  return new Network::BasicNetwork(config);
}

//-----------------------------------------------------------------------------
File::BasicFileProcessor* GeneralFactory::buildBasicFileProcessor(const File::Config& config) const
{
  return new File::BasicFileProcessor(config);
}

} // Common