#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE( "Top level test", "[sample]" ) {
    REQUIRE( (9 + 11) == 20 );
}

