#ifndef COMMAND_LINE_IMPL_H
#define COMMAND_LINE_IMPL_H

#include <Common/Error.h>

#include <map>

namespace UI
{

class CommandLineImpl 
{
public:
  CommandLineImpl();

  //! Prints available options in command line.
  void listOptions(const std::map<int, std::string>& options) const;

  //! Prints the welcome message in command line.
  void printWelcome() const;

  //! Prints the error in comand line.
  void printError(const Common::Error& error) const;

    //! Prints the message in comand line.
  void printMessage(const std::string& message) const;

  //! Waits for user input.
  std::string listenToInput();
};

} // UI

#endif // COMMAND_LINE_IMPL_H