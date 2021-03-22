#include "CommandLineImpl.h"

#include <iostream>

namespace UI 
{

//-----------------------------------------------------------------------------
CommandLineImpl::CommandLineImpl()
{}

//-----------------------------------------------------------------------------
void CommandLineImpl::printWelcome() const
{
  std::cout << "Welcome to the file transfer service." << std::endl;
}

//-----------------------------------------------------------------------------
void CommandLineImpl::listOptions(const std::map<int, std::string>& options) const
{
  for ( auto& option : options)
  {
    std::cout << option.first << ". " << option.second << std::endl;
  }
}

//-----------------------------------------------------------------------------
std::string CommandLineImpl::listenToInput()
{
  std::string input{};
  std::cin >> input;
  return input;
}

//-----------------------------------------------------------------------------
void CommandLineImpl::printError(const Common::Error& error) const
{
  std::cout << "Error: " << error << std::endl;
}

//-----------------------------------------------------------------------------
void CommandLineImpl::printMessage(const std::string& message) const
{
  std::cout << message << std::endl;
}

} // UI