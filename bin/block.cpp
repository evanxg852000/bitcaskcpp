#include "sdblib/block.hpp"
#include <fmt/format.h>

namespace sdblib {

  Block::Block(std::string f, size_t n): filename{f}, blocknum{n}{
  }

  std::string Block::fileName() const{
    return filename;
  }

  size_t Block::number() const{
    return blocknum;
  }

  std::string Block::toString(){
    return fmt::format("[filename:{}, blocknum:{}]", filename, blocknum);
  }

}
