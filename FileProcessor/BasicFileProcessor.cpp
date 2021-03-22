#include "BasicFileProcessor.h"

#include <iostream>

namespace File
{

//-----------------------------------------------------------------------------
std::vector<std::string> getFileVector(const std::string& expression)
{
  std::vector<std::string> strings;
  std::istringstream f(expression);
  std::string s;    
  while (getline(f, s, ';')) {
      strings.push_back(s);
  }
  return strings;
}

//-----------------------------------------------------------------------------
void getContent(const fs::path& root, const std::string& relative, char* contentVector, size_t& totalSize)
{
  for(auto it = fs::directory_iterator(root); it != fs::directory_iterator(); ++it)
  {
    auto path = relative;
    auto shortPath = path.append(it->path().string().erase(0, root.string().size()));
    auto size = shortPath.size();
    memcpy(contentVector + totalSize, &size, sizeof(size_t));
    memcpy(contentVector + totalSize + sizeof(size_t), shortPath.data(), size);
    totalSize += sizeof(size_t) + shortPath.size();
    auto s = fs::status(*it);
    if (fs::is_directory(s))
    {
      auto subFolder = root;
      subFolder.append((*it).path().string());
      getContent(subFolder, shortPath, contentVector, totalSize);
    }
  }
}

//-----------------------------------------------------------------------------
Common::Error convertRawToStringVector(const char* rawData, size_t size, std::vector<std::string>& contentVector)
{
  size_t position = 0;
  while (position < size)
  {
    auto nameSize = *reinterpret_cast<const size_t*>(rawData + position);
    position += sizeof(size_t);
    std::string name{rawData + position, nameSize};
    position += nameSize;
    contentVector.emplace_back(name);
  }

  if (position > size)
  {
    return Common::Error(std::string(__PRETTY_FUNCTION__) + ": corrupted data");
  }
  return Common::Error();
}

//-----------------------------------------------------------------------------
BasicFileProcessor::BasicFileProcessor(const Config& config)
: m_Config(config)
{}

//-----------------------------------------------------------------------------
BasicFileProcessor::BasicFileProcessor(const BasicFileProcessor& right)
: m_Config(right.m_Config)
{}

//-----------------------------------------------------------------------------
Common::Error BasicFileProcessor::prepareFileRead(const fs::path& fileName, InfoHeader& info)
{
  auto strippedFileName = fileName.string();
  strippedFileName.erase(0, m_ProcessState.sourcePath.size());
  auto size = strippedFileName.size() + sizeof(InfoHeader);
  if (size > m_ProcessState.remainingChunk_bytes)
  {
    return Common::Error("File name and header is too long");
  }
  m_ProcessState.remainingChunk_bytes -= size;

  auto s = fs::status(fileName);
  info.type = s.type();
  info.nameSize = strippedFileName.size();
  if (info.type == fs::file_type::directory)
  {
    info.fileSize = -1;
    info.filePosition = 0;
    info.prevFilePosition = 0;
    ++m_ProcessState.lastProcessedIndex;
    return Common::Error(); 
  }

  if (info.fileSize == 0)
    info.fileSize = fs::file_size(fileName);

  info.prevFilePosition = info.filePosition;
  if ((info.fileSize - info.filePosition) > m_ProcessState.remainingChunk_bytes)
  {
    info.filePosition += m_ProcessState.remainingChunk_bytes;
    m_ProcessState.lastFilePosition = info.filePosition;
    m_ProcessState.remainingChunk_bytes = 0;
  }
  else
  {
    info.filePosition = info.fileSize;
    m_ProcessState.remainingChunk_bytes -= (info.filePosition - info.prevFilePosition);
    ++m_ProcessState.lastProcessedIndex;
  }
  return Common::Error();
}

//-----------------------------------------------------------------------------
Common::Error BasicFileProcessor::writeFiles(
  AccessMap& accessMap, 
  int writer, 
  const char* rawData, 
  size_t size, 
  const fs::path& dest)
{
  m_Buffer.header = *reinterpret_cast<const FsBufferHeader*>(rawData);
  size_t filePosition = sizeof(FsBufferHeader);
  for (auto i = 0; i < m_Buffer.header.elementsInChunk; ++i)
  {
    auto header = reinterpret_cast<const InfoHeader*>(rawData + filePosition);
    filePosition += sizeof(InfoHeader);
    std::string fileName{rawData + filePosition, header->nameSize};
    filePosition += header->nameSize;
    fs::path destination{dest};
    destination.append(fileName);
    if (header->type == fs::file_type::regular)
    { 
      m_Mutex.lock();
      auto it = accessMap.find(fileName);
      if (it != accessMap.end() && it->second.type != AccessType::None && it->second.owner != writer)
      {
        m_Mutex.unlock();
        return Common::Error("File " + fileName + " is in use. Skipping");
      }
      if (it != accessMap.end() && it->second.type == AccessType::None)
        accessMap[fileName] = Access(writer, AccessType::Write);

      m_Mutex.unlock();

      std::cout << std::to_string(100.0 * header->filePosition/header->fileSize) + "% " << destination << std::endl;
      writeFile(
        destination, 
        rawData + filePosition, 
        header->filePosition - header->prevFilePosition,
        header->prevFilePosition == 0);

      if (header->fileTransferred())
      {
        m_Mutex.lock();
        accessMap[fileName] = Access(-1, AccessType::None);
        m_Mutex.unlock();
      }
    }
    else if (header->type == fs::file_type::directory)
    {
      if (fs::exists(destination) == false && fs::create_directory(destination) == false)
      {
        return Common::Error("Failed to create " + fileName + " directory");
      }
    }
    filePosition += header->filePosition - header->prevFilePosition;
  }
  return Common::Error();
}

//-----------------------------------------------------------------------------
void BasicFileProcessor::reset()
{
  m_ProcessState = ProcessState{};
  m_Buffer = FsBuffer{};
  m_ContentMap.clear();
}

//-----------------------------------------------------------------------------
Common::Error BasicFileProcessor::readFile(const fs::path& fileName, InfoHeader& info)
{
  auto strippedFileName = fileName.string();
  strippedFileName.erase(0, m_ProcessState.sourcePath.size());
  m_Buffer.addElement(strippedFileName, info);
  m_Buffer.header.bytesWritten += strippedFileName.size() + sizeof(InfoHeader);
  std::ifstream source(fileName, std::ios::binary);
  source.seekg(info.prevFilePosition);
  source.read(m_Buffer.data + m_Buffer.header.bytesWritten, info.filePosition - info.prevFilePosition);
  source.close();
  m_Buffer.header.bytesWritten += info.filePosition - info.prevFilePosition;
  
  return Common::Error();
}

//-----------------------------------------------------------------------------
Common::Error BasicFileProcessor::writeFile(const std::string& fileName, const char* buffer, size_t size, bool isNewFile) const
{
  if (fs::exists(fileName) && isNewFile)
  {
    fs::remove_all(fileName);
  }
  std::fstream dest(fileName, std::ios::app);

  dest.write(buffer, size);
  dest.close();

  return Common::Error();
}

//-----------------------------------------------------------------------------
Common::Error BasicFileProcessor::traverseFileSystem(AccessMap& accessMap, const fs::path& root)
{
  ++m_Buffer.header.elementCount;
  ++m_ProcessState.deepestLevel;
  if (m_ProcessState.deepestLevel > m_Config.maxFileSystemDepth)
  {
    return Common::Error("Too many levels in file system hierarchy.");
  }

  if (fs::exists(root) == false)
  {
    return Common::Error("Path " + root.string() + " not found");
  }

  InfoHeader header{};
  auto s = fs::status(root);
  if (fs::is_directory(s))
  {
    header.type = s.type();
    m_ContentMap.emplace(root, header);
    for(auto it = fs::directory_iterator(root); it != fs::directory_iterator(); ++it)
    {
      auto error = traverseFileSystem(accessMap, *it);
      if (error.empty() == false)
      {
        return error;
      }
    }
  }
  else if (fs::is_regular_file(s))
  {
    header.type = s.type();
    m_ContentMap.emplace(root, header);
  }
  return Common::Error();
}

//-----------------------------------------------------------------------------
bool BasicFileProcessor::isReadingFinished() const
{
  if (m_ProcessState.lastProcessedIndex != 0 && m_ProcessState.lastProcessedIndex == m_Buffer.header.elementCount)
  {
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
Common::Error BasicFileProcessor::preProcessFileReadAccess(AccessMap& accessMap, const std::string& fileName, const InfoHeader& header)
{
  if (accessMap.empty())
    return Common::Error();

  // Access map is a common resource between threads.
  // Read only happens if nobody is writing in it.
  m_Mutex.lock();
  auto it = accessMap.find(fileName);
  if (it != accessMap.end() && it->second.type == AccessType::Write)
  {
    m_Mutex.unlock();
    return Common::Error("File " + fileName + " is being written. Skipping");
  }
  // First read block.
  if (it != accessMap.end() && header.prevFilePosition == 0)
  {
    it->second.type = AccessType::Read;
    ++it->second.referenceCount;
  }
  m_Mutex.unlock();
  return Common::Error();
}

//-----------------------------------------------------------------------------
Common::Error BasicFileProcessor::postProcessFileReadAccess(AccessMap& accessMap, const std::string& fileName, const InfoHeader& header)
{
  if (accessMap.empty())
    return Common::Error();

  m_Mutex.lock();
  // Reading of the last block. File is guaranteed to be found, since there is no delete.
  auto it = accessMap.find(fileName);
  if (it != accessMap.end() && header.filePosition == header.fileSize)
  {
    --it->second.referenceCount;
    if (it->second.referenceCount == 0)
      it->second.type = AccessType::None;
    if(it->second.referenceCount < 0)
    {
      m_Mutex.unlock();
      return Common::Error("Negativ reference count in access map");
    }
  }
  m_Mutex.unlock();
  return Common::Error();
}

//-----------------------------------------------------------------------------
Common::Error BasicFileProcessor::processReadRequest(AccessMap& accessMap, const std::string& expression, const fs::path& sourcePath, FsBuffer& buffer)
{
  // Subtract the element count from remaining chunk size.
  m_ProcessState.remainingChunk_bytes = cChunkSize_bytes;
  m_ProcessState.deepestLevel = 0;
  m_Buffer.header.bytesWritten = 0;
  m_Buffer.header.elementsInChunk = 0;

  if (m_ContentMap.empty())
  {
    auto fileVector = getFileVector(expression);
    for (auto file : fileVector)
    { 
      fs::path fileToHandle = file;
      if (sourcePath.empty())
      {
        auto s = fs::status(file);
        auto path = fs::path(file);
        if (fs::is_directory(s))
          m_ProcessState.sourcePath = path.stem().string();
        else if (fs::is_regular_file(s))
          m_ProcessState.sourcePath = path.filename().string();

        m_ProcessState.sourcePath = path.string().erase(path.string().size() - m_ProcessState.sourcePath.size(), m_ProcessState.sourcePath.size());
      }
      else
      {
        m_ProcessState.sourcePath = sourcePath.string();
        fileToHandle = fs::path(m_ProcessState.sourcePath);
        fileToHandle.append(file);
      }
      
      m_ProcessState.deepestLevel = 0;
      auto error = traverseFileSystem(accessMap, fileToHandle);
      if (error.empty() == false)
      {
        return error;
      }
    }
  }

  for (auto& fileInfo : m_ContentMap)
  {
    if (m_ProcessState.remainingChunk_bytes == 0)
      break;

    if(fileInfo.second.fileTransferred())
      continue;

    auto s = fs::status(fileInfo.first);
    auto error = prepareFileRead(fileInfo.first, fileInfo.second);
    if (error.empty() == false)
      continue;

    auto strippedFileName = fileInfo.first;
    strippedFileName.erase(0, m_ProcessState.sourcePath.size());
    if (fileInfo.second.type == fs::file_type::regular)
    {
      if ((error = preProcessFileReadAccess(accessMap, strippedFileName, fileInfo.second)).empty() == false)
        return error;
      if (readFile(fileInfo.first, fileInfo.second).empty() == false)
        return error;
      if ((error = postProcessFileReadAccess(accessMap, strippedFileName, fileInfo.second)).empty() == false)
        return error;
      std::cout << std::to_string(100.0 * fileInfo.second.filePosition/fileInfo.second.fileSize) + "% " << fileInfo.first << std::endl;
    } 
    else if (fileInfo.second.type == fs::file_type::directory)
    {
      m_Buffer.addElement(strippedFileName, fileInfo.second);
      m_Buffer.header.bytesWritten += strippedFileName.size() + sizeof(InfoHeader);
    }
    ++m_Buffer.header.elementsInChunk;
  }

  buffer = m_Buffer; 
  return Common::Error();
}

} // Common