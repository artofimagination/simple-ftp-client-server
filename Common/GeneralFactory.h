#ifndef GENERAL_FACTORY_H
#define GENERAL_FACTORY_H

#include <UI/CommandLineImpl.h>
#include <Network/BasicNetwork.h>
#include <FileProcessor/BasicFileProcessor.h>

namespace Common
{
//! This class is responsible for creating instances of all classes appearing in the code.
class GeneralFactory 
{
public:
  GeneralFactory() = default;
  //! Creates a UI instance using command line interface.
  UI::CommandLineImpl* buildCommandLineUI() const;
  //! Creates a basic tcp network handler.
  Network::BasicNetwork* buildBasicNetwork(const Network::Config& config) const;
  //! Creates a basic file processor instance.
  File::BasicFileProcessor* buildBasicFileProcessor(const File::Config& config) const;
};

} // Common

#endif // GENERAL_FACTORY_H