#define PICOBENCH_IMPLEMENT_WITH_MAIN

#include <filesystem>
#include <functional>
#include <tuple>
#include <iostream>
#include <cstdlib>
#include <string>

#include "bitcaskcpp/bitcask.h"
#include "picobench/picobench.hpp"

namespace fs = std::filesystem;

bool with(fs::path path, const std::function<void(fs::path&)>& callback) {
    if(!fs::create_directories(path))
        return false;

    callback(path);
    return fs::remove_all(path) != 0;
}

static void bistcask_put(picobench::state& s){
    bitcaskcpp::BitcaskOption options;
    with("tempdir", [&](fs::path& dir) {
        auto db_path = dir / "testdb";
        bitcaskcpp::Bitcask bitcsk(db_path, options);
        bitcsk.Open();
        for (auto _ : s) {
            auto key = std::string(rand() % 5, '-');
            auto value = std::string(rand() % 20, '-');
            bitcsk.Put(key.data(), value.data());
        }
        bitcsk.Close();
    });
}

static void bistcask_get(picobench::state& s) {
    bitcaskcpp::BitcaskOption options;
    with("tempdir", [&](fs::path& dir) {
        auto db_path = dir / "testdb";
        bitcaskcpp::Bitcask bitcsk(db_path, options);
        bitcsk.Open();
        for(auto i = 0; i < 100; ++i) {
            auto key = std::to_string(i);
            auto value = std::string(rand() % 20, '-');
            bitcsk.Put(key.data(), value.data());
        }

        for (auto _ : s) {
            int i = rand() % 100;
            bitcsk.Get(std::to_string(i).data());
        }
        bitcsk.Close();
    });
}

// register
PICOBENCH(bistcask_put);
PICOBENCH(bistcask_get);
