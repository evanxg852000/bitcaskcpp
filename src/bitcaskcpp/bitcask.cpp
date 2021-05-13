#include <ctime>
#include <filesystem>
#include <fstream>
#include <unordered_set>
#include <iostream>

#include "bitcaskcpp/bitcask.hpp"

namespace bitcaskcpp {
namespace fs = std::filesystem;

Bitcask::Bitcask(std::string path, BitcaskOption options)
    : storage_dir{fs::path(path)}, options{options},
      active_file_id{0}, size{0}, is_opened{false} {}

Bitcask::~Bitcask() {}

void Bitcask::Open() {
  std::unique_lock lock(mutex);

  // check if dir exist & not locked
  // create db dir
  bool is_new = false;

  // create directry if db is new
  if (!fs::exists(storage_dir)) {
    is_new = true;
    if (!fs::create_directory(storage_dir))
      throw Exception("Unable to create bitcask storage.");
  }

  // check if db is locked
  fs::path lock_file_path = storage_dir / ".lock";
  if (fs::exists(lock_file_path)) {
    throw Exception("bitcask storage already in use by another process");
  }

  // create the lock file
  std::fstream lock_file;
  lock_file.open(lock_file_path, std::ios::app | std::ios::out);
  lock_file << timestamp();
  lock_file.close();

  if (is_new) {
    // create active file
    //_currentGeneration = 1;
    active_file_id = 1;
    open_files.insert({active_file_id, BitcaskFile{data_file(active_file_id)}});
    is_opened = true;
    return;
  }

  // load all files
  // remove any temp file
  for (auto &p : fs::directory_iterator(storage_dir)) {
    if (p.path().extension() == Bitcask::TEMP_FILE_EXTENTION) {
      fs::remove(p);
    }

    if (p.path().extension() != Bitcask::DATA_FILE_EXTENTION)
      continue;

    // load records while counting disposable space
    uint64_t file_id = std::stoull(p.path().stem()); // TODO execption
    load_data(file_id);
  }
  is_opened = true;
}

void Bitcask::Close() {
  std::unique_lock lock(mutex);

  for (auto &[_, file] : open_files) {
    file.GetFileStream().close();
  }
  fs::remove(lock_file());
  open_files.clear();
  is_opened = false;
}

void Bitcask::Put(const char *key, const char *value) {
  assert(key != nullptr);
  assert(value != nullptr);
  if (value == Bitcask::TOMBSTONE) {
    throw Exception("The specified value is a sentinel that cannot be used in bitcask storage.");
  }

  std::unique_lock lock(mutex);
  ensure();

  bool is_new = (key_dir.get(key) == nullptr);
  auto [record_size, record_offset] = write_value(key, value);
  key_dir.set(key, new BitcaskEntry(active_file_id, record_size, record_offset));
  if(is_new) {
    size += 1;
  }
}

bool Bitcask::Has(const char *key) {
  assert(key != nullptr);
  
  std::shared_lock lock(mutex);
  ensure();

  return key_dir.get(key) != nullptr; 
}

std::string Bitcask::Get(const char *key) {
  assert(key != nullptr);
  
  std::shared_lock lock(mutex);
  ensure();

  BitcaskEntry *entry = key_dir.get(key);
  if (entry == nullptr) {
    throw Exception("Requested key not found in bistcask storage.");
  }

  std::fstream& reader = bitcask_file(entry->file_id).GetFileStream();
  auto [_, __, value] = get_value(reader, entry->record_offset);
  return value;
}

void Bitcask::Delete(const char *key) {
  assert(key != nullptr);

  std::unique_lock lock(mutex);
  ensure();

  BitcaskEntry *entry = key_dir.get(key);
  if (entry == nullptr) {
    throw Exception("Requested key not found in bistcask storage.");
  }

  write_value(key, Bitcask::TOMBSTONE);
  key_dir.del(key);
  size -= 1;
}

size_t Bitcask::Size() {
  std::shared_lock lock(mutex);
  ensure();
  return size; 
}

void Bitcask::Scan(char *prefix, scan_callback_t func) {
  assert(prefix != nullptr);
  assert(func != nullptr);

  std::shared_lock lock(mutex);
  ensure();
  
  for (auto it = key_dir.begin(prefix); it != key_dir.end(); ++it) {
    std::fstream& reader = bitcask_file((*it)->file_id).GetFileStream();
    auto [_, key, value] = get_value(reader, (*it)->record_offset);
    func(key, value);
  }
}

void Bitcask::Sync() {
  std::unique_lock lock(mutex);
  ensure();

  std::fstream &writter = bitcask_file(active_file_id).GetFileStream();
  writter.flush();
}

BitcaskStats Bitcask::Statistics() {
  std::shared_lock lock(mutex);
  ensure();

  size_t disposable = 0;
  size_t total = 0;
  size_t num_files = 0;
  size_t num_entries = size;
  for (const auto &entry : open_files) {
    disposable += entry.second.disposable_size;
    total += entry.second.total_size;
    num_files++;
  }
  return BitcaskStats(disposable, total, num_files, num_entries);
}

void Bitcask::Compact() {
  std::unique_lock lock(mutex);
  ensure();

  active_file_id += 2;
  uint64_t compation_file_id = active_file_id - 1;
  BitcaskFile active_file{data_file(active_file_id)};
  open_files.insert({active_file_id, std::move(active_file)});

  // loop through all keys in key_dir
  std::unordered_set<uint64_t> trash_files{};
  BitcaskFile compaction_file{data_file(compation_file_id)};
  open_files.insert({compation_file_id, std::move(compaction_file)});
  std::fstream &writer = bitcask_file(compation_file_id).GetFileStream();
  writer.seekp(0, std::ios_base::beg);

  // create hint file
  std::fstream hint_writer;
  hint_writer.open(hint_file(compation_file_id),
                   std::ios::binary | std::ios::out | std::ios::app);
  hint_writer.seekg(0, std::ios_base::end);

  for (const auto entry : key_dir) {
    trash_files.insert(entry->file_id);

    size_t record_offset = writer.tellp();
    std::fstream &reader = bitcask_file(entry->file_id).GetFileStream();
    reader.seekg(entry->record_offset, std::ios_base::beg);
    std::string buffer(entry->record_size, '\0');
    reader.read(buffer.data(), buffer.length());
    writer.write(buffer.data(), buffer.length());

    entry->file_id = compation_file_id;
    entry->record_offset = record_offset;

    // create hint_file entry
    // TODO: try to make key available via iterator, it's realy annoying that we
    // have to read this via disk data
    // https://github.com/rafaelkallis/adaptive-radix-tree/issues/9
    std::stringstream data_reader;
    data_reader.str(buffer.data());
    auto [_, key, __] = get_value(data_reader, 0);
    size_t key_size = key.length();
    size_t record_size = buffer.length();

    buffer.resize(BitcaskLayout::GetHintRecordSize(key_size), '\0');
    buffer.append(ByteOrder::toLittleEndianString<size_t>(key_size));
    buffer.append(key);
    buffer.append(ByteOrder::toLittleEndianString<size_t>(record_size));
    buffer.append(ByteOrder::toLittleEndianString<size_t>(record_offset));
    hint_writer.write(buffer.data(), buffer.length());
  }
  // sync & close hint file;
  hint_writer.close();

  // remove unused files
  for (const auto file_id : trash_files) {
    open_files.erase(file_id);
    fs::remove(data_file(file_id));
    fs::remove(hint_file(file_id));
  }
}

void Bitcask::load_data(uint64_t file_id) {
  // wlock
  BitcaskFile file(this->data_file(file_id));
  open_files.insert({file_id, std::move(file)});
  if (fs::exists(hint_file(file_id))) {
    // load binary hint file
    this->load_hint_file(file_id);
    return;
  }

  // load file manually by replaying the log
  BitcaskFile& btcsk_file = bitcask_file(file_id); 
  std::fstream &reader = btcsk_file.GetFileStream();
  reader.seekg(0, std::ios_base::end);
  size_t file_size = reader.tellp();
  btcsk_file.total_size = file_size;

  // traverse from end of file
  std::unordered_set<char *> processed_keys{};
  size_t record_offset = read<size_t>(reader, file_size - sizeof(size_t));
  size_t disposable_size = 0;
  while (true) {
    auto [record_size, key, value] = get_value(reader, record_offset);
    if (processed_keys.find(key.data()) == processed_keys.end()) {
      if (record_offset == 0)
        break;
      record_offset = read<size_t>(reader, record_offset - sizeof(size_t));
      continue;
    }
    processed_keys.insert(key.data());

    if (value == Bitcask::TOMBSTONE) {
      disposable_size += record_size;
      if (record_offset == 0)
        break;
      record_offset = read<size_t>(reader, record_offset - sizeof(size_t));
      continue;
    }

    key_dir.set(key.data(), new BitcaskEntry(file_id, record_size, record_offset));
    if (record_offset == 0)
      break;
    record_offset = read<size_t>(reader, record_offset - sizeof(size_t));
  }

  btcsk_file.disposable_size = disposable_size;
}

void Bitcask::load_hint_file(uint64_t file_id) {
  std::fstream reader;
  reader.open(data_file(file_id),
              std::ios::binary | std::ios::in | std::ios::app);
  reader.seekg(0, std::ios_base::end);
  size_t total_size = reader.tellg();
  reader.seekg(0, std::ios_base::beg);
  size_t offset = 0;

  // traverse hint file forward
  while (offset <= total_size) {
    BitcaskLayout layout(offset);
    size_t key_size = read<size_t>(reader, layout.GetHintKeySizeOffset());
    std::string key = read_data(reader, layout.GetHintKeyOffset(), key_size);
    size_t record_size =
        read<size_t>(reader, layout.GetHintRecordSizeOffset(key_size));
    size_t record_offset =
        read<size_t>(reader, layout.GetHintRecordOffsetOffset(key_size));
    key_dir.set(key.data(),
                new BitcaskEntry(file_id, record_size, record_offset));
    offset += BitcaskLayout::GetHintRecordSize(key_size);
  }
}

std::tuple<size_t, std::string, std::string> Bitcask::get_value(std::istream &reader,
                                                      size_t offset) {
  BitcaskLayout layout(offset);

  uint32_t checksum = read<uint32_t>(reader, layout.GetChecksumOffset());
  size_t key_size = read<size_t>(reader, layout.GetKeySizeOffset());
  size_t value_size = read<size_t>(reader, layout.GetValueSizeOffset());

  std::string key = read_data(reader, layout.GetKeyOffset(), key_size);
  std::string value = read_data(reader, layout.GetValueOffset(key_size), value_size);
  size_t record_size = BitcaskLayout::GetRecordSize(key_size, value_size);

  return std::make_tuple(record_size, key, value);
}

std::tuple<size_t, size_t> Bitcask::write_value(const char *key, const char *value) {
  // position the writer
  std::fstream &writer = bitcask_file(active_file_id).GetFileStream();
  writer.seekp(0, std::ios_base::end);
  size_t record_offset = writer.tellp();

  // calculate checksum
  std::string buffer(key);
  buffer.append(value);
  uint32_t checksum = crc32_checksum(buffer.data(), buffer.length());

  size_t key_size = std::strlen(key);
  size_t value_size = std::strlen(value);

  buffer.clear();
  buffer.append(ByteOrder::toLittleEndianString<uint32_t>(checksum));
  buffer.append(ByteOrder::toLittleEndianString<size_t>(key_size));
  buffer.append(ByteOrder::toLittleEndianString<size_t>(value_size));
  buffer.append(key);
  buffer.append(value);
  buffer.append(ByteOrder::toLittleEndianString<size_t>(record_offset));

  writer.write(buffer.data(), buffer.length());

  return std::make_tuple(buffer.length(), record_offset);
}

std::string Bitcask::read_data(std::istream &reader, size_t offset, size_t size) {
  reader.seekg(offset, std::ios_base::beg);
  std::string buffer(size, '\0');
  reader.read(buffer.data(), size);
  return buffer;
}

} // namespace bitcaskcpp
