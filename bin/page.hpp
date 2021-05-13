#pragma once

#include <string>

namespace sdblib {
class Block;

class Page {
  // ByteBuffer contents;
  // FileMgr filemgr;

public:
  static const size_t BLOCK_SIZE = 400;
  
  // returns the size of Integer in bytes
  inline static size_t IntSize() {
    return sizeof(int);
  }

  // returns the size in bytes to store the string of n characters
  // sizeof(size_t) + std::string::size()
  // e.i the size in bytes to store the number of byte the string is made of
  // + the number of bytes the string is made of.
  inline static size_t StrSize(size_t n) {
    return Page::IntSize() + n;
  }

  Page();
  virtual ~Page() = default;


  void read(Block blk);
  void write(Block blk);
  Block append(std::string filenane);
  
  int getInt(size_t offset);
  void setInt(size_t offset, int val);
  std::string getString(size_t offset);
  void setString(size_t offset, std::string val);

  
};




}
