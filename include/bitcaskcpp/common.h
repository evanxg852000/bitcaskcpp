#pragma once

#include <string>

#include "cxxutils/byteorder.h"

namespace bitcaskcpp {

class BitcaskOption {};

uint32_t timestamp();

uint32_t crc32_checksum(const char* , size_t);

}
