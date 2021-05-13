#pragma once

#include <string>

namespace sdblib {

class Block {
  std::string filename;
  size_t blocknum;

public:
  Block(std::string f, size_t num);
  virtual ~Block() = default;
  std::string fileName() const;
  size_t number() const;
  std::string toString();
  
  inline friend bool operator==(const Block& lhs, const Block& rhs){
    return lhs.filename == rhs.filename && 
      lhs.blocknum == rhs.blocknum;
  }
  
  inline friend bool operator!=(const Block& lhs, const Block& rhs){
    return !(lhs == rhs);
  }
};

}
