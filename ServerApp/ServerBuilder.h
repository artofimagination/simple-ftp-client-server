#ifndef SERVER_BUILDER_H
#define SERVER_BUILDER_H

#include <Common/GeneralFactory.h>

class Server;

class ServerBuilder
{
public:
  ServerBuilder() = default;

  //! Assembles a server instance.
  Server* buildBasicServer(const std::string& storagePath) const;

private:
  Common::GeneralFactory m_Factory{};
};

#endif // SERVER_BUILDER_H