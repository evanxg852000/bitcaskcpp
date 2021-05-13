#pragma once

#include <filesystem>
#include <fstream>
#include <shared_mutex>

#include "cxxutils/bytebuffer.hpp"
#include "sdblib/block.hpp"

namespace sdblib {

namespace fs = std::filesystem;

class FileMgr {
  fs::path dbDirectory;
  bool isNew;
  std::unordered_map<std::string, std::fstream> openFiles;
  std::shared_mutex mutex;
  
  std::fstream& getFile(std::string);
  
public:
  explicit FileMgr(std::string);
  void read(Block, ByteBuffer);
  void write(Block, ByteBuffer);
  append(std::string, ByteBuffer);
  size(std::string);
  bool isNew();

};

}
