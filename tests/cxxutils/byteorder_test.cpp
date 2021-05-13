#include <string>

#include <catch2/catch.hpp>

#include "cxxutils/byteorder.hpp"


TEST_CASE( "ByteOrder test", "[sample]" ) {
    size_t num = 0;

    std::string buffer = ByteOrder::toBigEndianString<size_t>(21);
    num = ByteOrder::fromBigEndian<size_t>(buffer.data());
    REQUIRE(21 == num);

    buffer = ByteOrder::toLittleEndianString<size_t>(25);
    num = ByteOrder::fromLittleEndian<size_t>(buffer.data());
    REQUIRE(25 == num);
}
