#pragma once

#include <string>

#include "cxxutils/byteorder.h"

namespace bitcaskcpp {

struct BitcaskOption {
    std::optional<size_t> compaction_treshold;
    bool auto_close;
};

uint32_t timestamp();

uint32_t crc32_checksum(const char* , size_t);

}
