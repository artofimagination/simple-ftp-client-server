#ifndef COMMAND_DEFS_H
#define COMMAND_DEFS_H

#include <cstring>

namespace Common
{

enum class CommandType : int
{
  None = -1,
  SendToServer,
  GetFromServer,
  ListServerFiles,
  SendEcho,
  Error,
  WritingToServerDone,
  WritingToClientDone,
  ReadingFromServerDone,
  PollClient,
};

struct Header 
{
  size_t      dataSize{};
  CommandType command{CommandType::None}; 
};

constexpr int cMaxPayloadSize = 2 * 1024;

//! Command packets sent over TCP.
struct Packet 
{
  Header  header{};
  char    data[cMaxPayloadSize]{};
};

//! Convert raw data to packet.
inline void convertRawDataToPacket(const char* data, Common::Packet& packet)
{
  auto header = reinterpret_cast<const Common::Header&>(*data);
  packet.header = header;
  if (header.dataSize > 0)
    memcpy(packet.data, data + sizeof(Common::Header), header.dataSize);
}

} // Common

#endif // COMMAND_DEFS_H