#ifndef CLIENT_BUILDER_H
#define CLIENT_BUILDER_H

#include <Common/GeneralFactory.h>

class Client;


//! Assembles client instance, using components created by the general factory.
class ClientBuilder
{
public:
  ClientBuilder() = default;

  //! Assembles a basic client instance.
  Client* buildBasicClient() const;

private:
  Common::GeneralFactory m_Factory{};
};

#endif // CLIENT_BUILDER_H