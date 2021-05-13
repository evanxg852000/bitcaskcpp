#include <ctime>
#include <filesystem>
#include <fstream>

#include "bitcaskpp/common.hpp"
#include "crc/crc.hpp"

namespace bitcaskpp {

uint32_t timestamp() { return static_cast<uint32_t>(time(0)); }

uint32_t crc32_checksum(const char *data, size_t length) {
  return CRC::Calculate(data, length, CRC::CRC_32());
}

} // namespace bitcaskpp
