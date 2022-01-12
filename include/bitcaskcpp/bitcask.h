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
#include "bitcaskcpp/common.h"
#include "bitcaskcpp/exception.h"
#include "cxxutils/byteorder.h"

namespace bitcaskcpp {
namespace fs = std::filesystem;

// typedef void (*scan_callback_t)(std::string key, std::string value);
using scan_callback_t = std::function<void(std::string key, std::string value)>;

/*
Represents the layout of a bitcask record item layout.

+-------+--------+----------+-----+-------+------------+
| crc32 | key_sz | value_sz | key | value | rec_offset |
+-------+--------+----------+-----+-------+------------+
▲                                               |
|                                               |
+-----------------------------------------------+
*/
struct BitcaskItemLayout {
    // Base offset of the record.
    size_t base;

    // Constructs a new `BitcaskItemLayout` instance.
    inline BitcaskItemLayout(size_t offset) : base{offset} {}

    // Returns the offset of the checksum.
    inline size_t GetChecksumOffset() const { return base; }

    // Returns the key size offset.
    inline size_t GetKeySizeOffset() const { return base + sizeof(uint32_t); }

    // Returns the key size offset.
    inline size_t GetValueSizeOffset() const {
        return base + sizeof(uint32_t) + sizeof(size_t);
    }

    // Returns the key offset.
    inline size_t GetKeyOffset() const {
        return base + sizeof(uint32_t) + (sizeof(size_t) * 2);
    }

    // Returns the value offset.
    inline size_t GetValueOffset(size_t key_size) const {
        return base + sizeof(uint32_t) + (sizeof(size_t) * 2) + key_size;
    }

    // Returns the offset of the record offset
    inline size_t GetOffsetOffset(size_t key_size, size_t value_size) const {
        return base + sizeof(uint32_t) + (sizeof(size_t) * 2) + key_size +
               value_size;
    }

    // Resturns the record size.
    inline static size_t GetRecordSize(size_t key_size, size_t value_size) {
        return sizeof(uint32_t) + (sizeof(size_t) * 3) + key_size + value_size;
    }

    // Resurns the key size offset [TODO bug suspected]
    inline size_t GetHintKeySizeOffset() { return base; }

    // Resturns key offset for write.
    inline size_t GetHintKeyOffset() { return base + sizeof(size_t); }

    // Returns record size offset
    inline size_t GetHintRecordSizeOffset(size_t key_size) {
        return base + sizeof(size_t) + key_size;
    }

    // Returns the offset of the record offset.
    inline size_t GetHintRecordOffsetOffset(size_t key_size) {
        return base + (sizeof(size_t) * 2) + key_size;
    }

    // Returns the record size.
    inline static size_t GetHintRecordSize(size_t key_size) {
        return (sizeof(size_t) * 3) + key_size;
    }
};

// A struct for holding statistics on bitcask instance.
// This can help choose the right time to compact.
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

// A struct for holding the in-memory info of a bistcask record item.
struct BitcaskEntry {
    uint64_t file_id;
    size_t record_size;
    size_t record_offset;

    inline BitcaskEntry(uint64_t f_id, size_t r_size, size_t r_offset)
        : file_id{f_id}, record_size{r_size}, record_offset{r_offset} {}
};

// A struct representing a bitcask file handle.
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

// The bitcask engine.
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
    size_t Scan(const char *prefix, scan_callback_t func);

    void Sync();
    BitcaskStats Statistics();
    void Compact();

   private:
    // The bitcask options.
    BitcaskOption options;
    // The storage directory.
    fs::path storage_dir;
    // The key directory implement using an adaptive radix tree.
    art::art<BitcaskEntry> key_dir;
    // A map of ids to files currently opened by bistcask.
    std::unordered_map<uint64_t, BitcaskFile> open_files;
    // The active writable file.
    uint64_t active_file_id;
    // The size of record items in the bitcask.
    size_t size;
    // Tells if bistcask is opened.
    bool is_opened;
    // A read/write mutex to make bitcask thread safe.
    std::shared_mutex mutex;

    // Loads the data from a bitcask file.
    void load_data(uint64_t file_id);

    // Loads the hint file.
    void load_hint_file(uint64_t file_id);

    // Returns the (size, key, value) tuple of a record located at offset.
    std::tuple<size_t, std::string, std::string> get_value(std::istream &reader,
                                                 size_t offset);
    // Writes the (key, value) pair in bitcaks
    std::tuple<size_t, size_t> write_value(const char *key, const char *value);

    // Reads the value from a bitcask file at offset given a size.
    std::string read_data(std::istream &reader, size_t offset, size_t size);

    // Returns the bitcask file given its id.
    inline BitcaskFile& bitcask_file(uint64_t file_id) {
        auto entry = open_files.find(file_id);
        if(entry == open_files.end()) {
            throw Exception("Unabe to find file in the bitcask storage.");
        }
            
        return entry->second;
    }

    // Ensures bistcask is open before proceeding.
    inline void ensure() {
        if(!is_opened) {
            throw Exception("bistcask storage is not opened yet.");
        }
    }

    // Returns the data file's path given its id.
    inline fs::path data_file(uint64_t file_id) {
        return storage_dir / (std::to_string(file_id) + DATA_FILE_EXTENTION);
    }

     // Returns the hint file's path given its id.
    inline fs::path hint_file(uint64_t file_id) {
        return storage_dir / (std::to_string(file_id) + HINT_FILE_EXTENTION);
    }

     // Returns the lock file's path. 
    inline fs::path lock_file() { return storage_dir / LOCK_FILE; }

    constexpr static const char *TOMBSTONE = "BITCASKCPP_TOMBSTONE_VALUE";
    constexpr static const char *DATA_FILE_EXTENTION = ".data";
    constexpr static const char *HINT_FILE_EXTENTION = ".hint";
    constexpr static const char *TEMP_FILE_EXTENTION = ".tmp";
    constexpr static const char *LOCK_FILE = ".lock";
};

}  // namespace bitcaskcpp
