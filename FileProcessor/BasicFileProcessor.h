#ifndef BASIC_FILE_PROCESSOR_H
#define BASIC_FILE_PROCESSOR_H

#include <Common/Error.h>
#include <Common/CommandDefs.h>

#include <filesystem>
#include <vector>
#include <fstream>
#include <mutex>
#include <map>

namespace fs = std::filesystem;

namespace File
{

enum class AccessType {
  None,
  Read,
  Write,
};

struct Access {
  Access() = default;

  Access(int newOwner, AccessType newType) 
  : owner(newOwner)
  , type(newType)
  {}

  Access(const Access& right) 
    : owner(right.owner)
    , type(right.type)
    , referenceCount(right.referenceCount)
    {}

  int         owner{};
  AccessType  type{};
  int         referenceCount{};     ///< Shows how many threads are currently accessing the file. 
                                    ///< Only used after reading
};

typedef std::map<std::string, Access> AccessMap;

struct InfoHeader
{
  InfoHeader(
    fs::file_type           newType,
    std::ifstream::pos_type newFilePosition,                                          
    size_t                  newNameSize,                                            
    size_t                  newFileSize,
    int                     newCRC)
  : type(newType)
  , filePosition(newFilePosition)
  , nameSize(newNameSize)
  , fileSize(newFileSize)
  , crc(newCRC)
  {}

  bool fileTransferred() const 
  { 
    return (fileSize != 0 && fileSize == filePosition && type == fs::file_type::regular) ||
           (fileSize == -1 && type == fs::file_type::directory); 
  };
    
  InfoHeader() = default;

  fs::file_type           type{};
  std::ifstream::pos_type filePosition{};   ///< If not the entire file has been copied,                                           
                                            ///< the position where the read has started has to be sent as well.
  std::ifstream::pos_type prevFilePosition{};                                          
  size_t                  nameSize{};                                            
  size_t                  fileSize{};
  int                     crc{};            ///< Checksum to validate the copied file (filled only if the complete file is transfered)
                                            ///< The server will generate a CRC that will be checked on the client side. Not implemented.
  
};

struct FsBufferHeader
{
  int     bytesWritten{};
  int     elementCount{};
  int     elementsInChunk{};
};

constexpr int cChunkSize_bytes = Common::cMaxPayloadSize - sizeof(Common::Header) - sizeof(FsBufferHeader);

struct FsBuffer
{
  void addElement(
    std::string             name,
    const InfoHeader&       infoHeader
  )
  {
    memcpy(data + header.bytesWritten, reinterpret_cast<const char*>(&infoHeader), sizeof(InfoHeader));
    memcpy(data + header.bytesWritten + sizeof(InfoHeader), name.data(), name.size());
  }

  FsBufferHeader    header{};
  char              data[cChunkSize_bytes]{};
};

struct Config
{
  int maxFileSystemDepth = 7;               ///< Defines the maximum directory depth the traversing shall go.
};

struct FileName 
{
  FileName(
    size_t newSize,
    const std::string& newName
  )
  : size(newSize)
  , name(newName)
  {}

  size_t      size{};
  std::string name{};
};

//! Returns the content of selected folder.
void getContent(const fs::path& root, const std::string& relative, char* contentVector, size_t& totalSize);
//! Converts raw data to string vector.
Common::Error convertRawToStringVector(const char* rawData, size_t size, std::vector<std::string>& contentVector);

class BasicFileProcessor
{
public:
  BasicFileProcessor(const Config& config);
  BasicFileProcessor(const BasicFileProcessor& right);

  Common::Error processReadRequest(AccessMap& accessMap, const std::string& expression, const fs::path& sourcePath, FsBuffer& buffer);
  bool isReadingFinished() const;
  int getChunkSize() const;

  //! Writes the content of the buffer to the file system.
  Common::Error writeFiles(
    AccessMap& accessMap, 
    int writer, 
    const char* rawData, 
    size_t size, 
    const fs::path& dest);

  void reset();

private:
  struct ProcessState 
  {
    int remainingChunk_bytes{cChunkSize_bytes};
    int lastProcessedIndex{};
    int deepestLevel{};
    size_t lastFilePosition{};
    std::string sourcePath{};
  };

  //! Traverse through the filesystem where the root is the one of the user defined files or directories.
  //! During traverse, all details are filled in a buffer. Also files will be read as long as the overall
  //! read size is smaller than the allowed chunk size.
  Common::Error traverseFileSystem(AccessMap& accessMap, const fs::path& root);

  //! Reads the selected file from the file system.
  Common::Error readFile(const fs::path& fileName, InfoHeader& info);
  //! Writes the selected file to the file system.
  Common::Error writeFile(const std::string& fileName, const char* buffer, size_t size, bool isNewFile) const;
  //! Determines the space taken by the file to be read. Updates filesystem buffer related state values.
  Common::Error prepareFileRead(const fs::path& fileName, InfoHeader& info);
  //! Checks and sets file access prior reading.
  Common::Error preProcessFileReadAccess(AccessMap& accessMap, const std::string& fileName, const InfoHeader& header);
  Common::Error postProcessFileReadAccess(AccessMap& accessMap, const std::string& fileName, const InfoHeader& header);

  Config                            m_Config{};
  std::map<std::string, InfoHeader> m_ContentMap{};   ///< Holds all file info headers resulted in traversing the whole folder tree to be copied.
  FsBuffer                          m_Buffer{};       ///< Buffer to store details of the files to be transfered. 
                                                      ///< Its being cleaned once the whole transfer is complete
  ProcessState                      m_ProcessState{}; ///< Current state of the transfer. A transfer can consist of sending multiple chunks.
                                                      ///< This member keeps track of the procedure.
  std::mutex                        m_Mutex{};          
};

} // File

#endif // BASIC_FILE_PROCESSOR_H