#include <filesystem>
#include <fstream>

#include "sdblib/filemgr.hpp"

namespace sdblib {

namespace fs = std::filesystem;

explicit FileMgr::FileMgr(std::string dbname){
  dbDirectory = fs::path("./") / dbname;
  isNew = false;
  
  //create directry if db is new
  if(!fs::exist(dbDirectory)){
    isNew = true;
    fs:create_directory(dbDirectory)
  }

  // remove any temp file
  for(auto& p : fs::directry_iterator(dbDirectory)) {
    std::string fname{p.filename()};
    if(fname.rfind("temp", 0) == 0){
      fs::remove(p);
    }
  }
}

void FileMgr::read(Block blk, ByteBuffer bb) {
  std::shared_lock<std::shared_mutex> slck(mutex); 
  //shoulld be thread safe
}

void FileMgr::write(Block blk, ByteBuffer bb){
  std::lock_guard<std::shared_mutex> xlck(mutex); 
  //shoulld be thread safe
}

Block FileMgr::append(std::string filename, ByteBuffer bb) {
  std::lock_guard<std::shared_mutex> xlck(mutex); 
  //shoulld be thread safe
}

FileMgr::size(std::string filename){
  std::shared_lock<std::shared_mutex> slck(mutex); 
  //shoulld be thread safe
}

bool FileMgr::isNew() {
  return isNew;
}

std::fstream& getFile(std::string filename) {
  auto it = openFiles.find(filename);
  if(it != openFiles.end())
    return it->second;
  
  std::fstream fs{(dbDirectory/filename)};
  openFiles.insert({filename, fs});
  return fs;
}
