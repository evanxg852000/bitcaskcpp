#include <filesystem>
#include <functional>
#include <iostream>

#include <catch2/catch.hpp>

#include "bitcaskpp/bitcask.hpp"

namespace fs = std::filesystem;

bool with(fs::path path, const std::function<void(fs::path&)>& callback) {
    if(!fs::create_directories(path))
        return false;

    callback(path);
    return fs::remove_all(path) != 0;
}

void debug(const std::function<void()>& callback) {
    try {
        callback();
    } catch (const std::exception & ex ) {
        std::cout << ex.what() << std::endl;
    }
}


TEST_CASE("Opening bitcask", "[open]") {
    bitcaskpp::BitcaskOption options;

    bool status = with("tempdir", [&](fs::path& dir) {
        auto db_path = dir / "testdb";
        bitcaskpp::Bitcask bitcsk(db_path, options);
        bitcsk.Open();

        // check bd folder & all necessary files exist
        REQUIRE(fs::exists(db_path) == true);
        REQUIRE(fs::exists(db_path / "1.data") == true);
        REQUIRE(fs::exists(db_path / ".lock") == true);

        bitcsk.Close();
    });

    REQUIRE(status == true);
}

TEST_CASE("bitcask operation not allowed when not-opened or closed", "[not-open-closed]") {
    bitcaskpp::BitcaskOption options;
    bool status = with("tempdir", [&](fs::path& dir) {
        auto db_path = dir / "testdb";
        bitcaskpp::Bitcask bitcsk(db_path, options);

        REQUIRE_THROWS(bitcsk.Put("a", "a"));
        REQUIRE_THROWS(bitcsk.Get("a"));
        REQUIRE_THROWS(bitcsk.Delete("a"));

        bitcsk.Open();
        bitcsk.Close();

        REQUIRE_THROWS(bitcsk.Put("a", "a"));
        REQUIRE_THROWS(bitcsk.Get("a"));
        REQUIRE_THROWS(bitcsk.Delete("a"));
    });
}

TEST_CASE("CRUD operation on bitcask", "[crud]") {
    bitcaskpp::BitcaskOption options;
    bool status = with("tempdir", [&](fs::path& dir) {
        auto db_path = dir / "testdb";
        bitcaskpp::Bitcask bitcsk(db_path, options);
        bitcsk.Open();

        // insert
        bitcsk.Put("name", "Timo Werner");
        bitcsk.Put("height", "180");
        bitcsk.Put("weight", "76");
        bitcsk.Put("age", "25");
        bitcsk.Put("foot", "right");
        bitcsk.Put("positions", "[ST, LW]");
        REQUIRE(bitcsk.Size() == 6);
        REQUIRE(bitcsk.Get("name") == "Timo Werner");
        REQUIRE(bitcsk.Get("height") == "180");
        REQUIRE(bitcsk.Get("weight") == "76");
        REQUIRE_THROWS(bitcsk.Get("not_found"));

        // update
        bitcsk.Put("name", "Mr. Timo Werner");
        bitcsk.Put("height", "190");
        bitcsk.Put("weight", "78");
        REQUIRE(bitcsk.Get("name") == "Mr. Timo Werner");
        REQUIRE(bitcsk.Get("height") == "190");
        REQUIRE(bitcsk.Get("weight") == "78");
        REQUIRE(bitcsk.Size() == 6);

        // delete
        bitcsk.Delete("height");
        bitcsk.Delete("weight");
        REQUIRE_THROWS(bitcsk.Delete("not_found"));
        REQUIRE_THROWS(bitcsk.Get("height"));
        REQUIRE_THROWS(bitcsk.Get("weight"));
        REQUIRE(bitcsk.Size() == 4);

        bitcsk.Close();
    });

    REQUIRE(status == true);
}

TEST_CASE("CRUD operation on bitcask with close", "[crud-close]") {
    bitcaskpp::BitcaskOption options;
    bool status = with("tempdir", [&](fs::path& dir) {
        auto db_path = dir / "testdb";
        bitcaskpp::Bitcask bitcsk(db_path, options);
        bitcsk.Open();

        // insert
        bitcsk.Put("name", "Timo Werner");
        bitcsk.Put("height", "180");
        bitcsk.Put("weight", "76");
        bitcsk.Put("age", "25");
        bitcsk.Put("foot", "right");
        bitcsk.Put("positions", "[ST, LW]");
        REQUIRE(bitcsk.Size() == 6);
        REQUIRE(bitcsk.Get("name") == "Timo Werner");
        REQUIRE(bitcsk.Get("height") == "180");
        REQUIRE(bitcsk.Get("weight") == "76");
        REQUIRE_THROWS(bitcsk.Get("not_found"));
        bitcsk.Close();

        // update
        bitcsk.Open();
        bitcsk.Put("salry", "$65k/week");
        bitcsk.Put("sponsor", "Nike");
        bitcsk.Put("name", "Mr. Timo Werner");
        bitcsk.Put("height", "190");
        bitcsk.Put("weight", "78");
        REQUIRE(bitcsk.Get("name") == "Mr. Timo Werner");
        REQUIRE(bitcsk.Get("height") == "190");
        REQUIRE(bitcsk.Get("weight") == "78");
        REQUIRE(bitcsk.Get("age") == "25");
        REQUIRE(bitcsk.Get("foot") == "right");
        REQUIRE(bitcsk.Size() == 8);
        bitcsk.Close();

        // delete
        bitcsk.Open();
        bitcsk.Delete("height");
        bitcsk.Delete("weight");
        REQUIRE(bitcsk.Get("name") == "Mr. Timo Werner");
        REQUIRE(bitcsk.Get("age") == "25");
        REQUIRE(bitcsk.Get("foot") == "right");
        REQUIRE_THROWS(bitcsk.Delete("not_found"));
        REQUIRE_THROWS(bitcsk.Get("height"));
        REQUIRE_THROWS(bitcsk.Get("weight"));
        REQUIRE(bitcsk.Size() == 6);
        bitcsk.Close();
    });

    REQUIRE(status == true);
}
