#include "Server.h"
#include "ServerBuilder.h"

#include <iostream>

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << "Missing server storage path" << std::endl; 
    return 1;
  }
  ServerBuilder builder;
  auto server = std::make_unique<Server>(*builder.buildBasicServer(std::string(argv[1])));
  server->run();
  return 0;
}