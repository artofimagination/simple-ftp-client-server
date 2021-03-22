#include "Client.h"
#include "ClientBuilder.h"

int main()
{
  ClientBuilder builder;
  auto client = std::make_unique<Client>(*builder.buildBasicClient());
  client->run();
  return 0;
}