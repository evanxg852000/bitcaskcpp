#pragma once

#include <string>
#include <chrono>
#include <ctime>

#include "cxxutils/byteorder.hpp"

namespace bitcaskpp {

//using file_id_t = uint64_t;
using byte = char;

class BitcaskOption {};

uint32_t timestamp();

uint32_t crc32_checksum(const char* , size_t);

}
