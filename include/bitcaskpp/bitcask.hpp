#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <utility>

#include "art/art.hpp"
#include "bitcaskpp/common.hpp"
#include "bitcaskpp/exception.hpp"
#include "cxxutils/byteorder.hpp"

namespace bitcaskpp {
namespace fs = std::filesystem;

typedef int (*scan_callback_t)(std::string key, std::string value);

/*
+-------+--------+----------+-----+-------+--------+
| crc32 | key_sz | value_sz | key | value | offset |
+-------+--------+----------+-----+-------+--------+
*/
struct BitcaskLayout {
    size_t base;

    inline BitcaskLayout(size_t offset) : base{offset} {}

    inline size_t GetChecksumOffset() const { return base; }

    inline size_t GetKeySizeOffset() const { return base + sizeof(uint32_t); }

    inline size_t GetValueSizeOffset() const {
        return base + sizeof(uint32_t) + sizeof(size_t);
    }

    inline size_t GetKeyOffset() const {
        return base + sizeof(uint32_t) + (sizeof(size_t) * 2);
    }

    inline size_t GetValueOffset(size_t key_size) const {
        return base + sizeof(uint32_t) + (sizeof(size_t) * 2) + key_size;
    }

    inline size_t GetOffsetOffset(size_t key_size, size_t value_size) const {
        return base + sizeof(uint32_t) + (sizeof(size_t) * 2) + key_size +
               value_size;
    }

    inline static size_t GetRecordSize(size_t key_size, size_t value_size) {
        return sizeof(uint32_t) + (sizeof(size_t) * 3) + key_size + value_size;
    }

    inline size_t GetHintKeySizeOffset() { return base; }

    inline size_t GetHintKeyOffset() { return base + sizeof(size_t); }

    inline size_t GetHintRecordSizeOffset(size_t key_size) {
        return base + sizeof(size_t) + key_size;
    }

    inline size_t GetHintRecordOffsetOffset(size_t key_size) {
        return base + (sizeof(size_t) * 2) + key_size;
    }

    inline static size_t GetHintRecordSize(size_t key_size) {
        return (sizeof(size_t) * 3) + key_size;
    }
};

struct BitcaskStats {
    size_t disposable;
    size_t total;
    size_t num_files;
    size_t num_entries;

    inline BitcaskStats(size_t disposable, size_t total, size_t num_files,
                        size_t num_entries)
        : disposable{disposable},
          total{total},
          num_files{num_files},
          num_entries{num_entries} {}
};

struct BitcaskEntry {
    uint64_t file_id;
    size_t record_size;
    size_t record_offset;

    inline BitcaskEntry(uint64_t f_id, size_t r_size, size_t r_offset)
        : file_id{f_id}, record_size{r_size}, record_offset{r_offset} {}
};

struct BitcaskFile {
    std::fstream file_stream;
    size_t total_size;
    size_t disposable_size;

    BitcaskFile(fs::path file_path) {
        file_stream.open(file_path, std::ios::binary | std::ios::in | std::ios::app |
                                 std::ios::out);
        total_size = fs::file_size(file_path);
        disposable_size = 0;
    }

    inline std::fstream &GetFileStream() { return file_stream; }

    inline size_t GetTotalSize() { return total_size; }

    inline size_t GetDisposableSize() { return disposable_size; }
};

class Bitcask {
   public:
    Bitcask(std::string path, BitcaskOption options);
    ~Bitcask();
    void Open();
    void Close();

    void Put(const char *key, const char *value);
    bool Has(const char *key);
    std::string Get(const char *key);
    void Delete(const char *key);

    size_t Size() noexcept(false);
    void Scan(char *prefix, scan_callback_t func);

    void Sync();
    BitcaskStats Statistics();
    void Compact();

   private:
    BitcaskOption options;
    fs::path storage_dir;
    art::art<BitcaskEntry> key_dir;
    std::unordered_map<uint64_t, BitcaskFile> open_files;
    uint64_t active_file_id;
    size_t size;
    std::shared_mutex mutex;

    void load_data(uint64_t file_id);
    void load_hint_file(uint64_t file_id);
    std::tuple<size_t, std::string, std::string> get_value(std::istream &reader,
                                                 size_t offset);

    std::string read_data(std::istream &reader, size_t offset, size_t size);

    template <typename T>
    T read(std::istream &reader, size_t offset) {
        std::string buffer = read_data(reader, offset, sizeof(T));
        return ByteOrder::fromLittleEndian<T>(buffer.data());
    }

    BitcaskFile& bitcask_file(uint64_t file_id) {
        auto entry = open_files.find(file_id);
        if(entry == open_files.end())
            throw new Exception(ExceptionType::FILE_NOT_FOUND, "TODO:");
        return entry->second;
    }

    inline fs::path data_file(uint64_t fileId) {
        return storage_dir / (std::to_string(fileId) + ".data");
    }

    inline fs::path hint_file(uint64_t fileId) {
        return storage_dir / (std::to_string(fileId) + ".hint");
    }

    inline fs::path lock_file() { return storage_dir / ".lock"; }

    inline static const char *TOMBSTONE = "BITCASKPP_TOMBSTONE_VALUE";
    inline static const char *DATA_FILE_EXTENTION = ".data";
    inline static const char *HINT_FILE_EXTENTION = ".hint";
    inline static const char *TEMP_FILE_EXTENTION = ".tmp";
};

}  // namespace bitcaskpp
