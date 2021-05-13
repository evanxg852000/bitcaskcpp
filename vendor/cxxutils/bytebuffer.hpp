#pragma once

#include <vector>
#include <memory>

class ByteBuffer {
  std::vector<uint8_t> content;
  size_t pos;

public:
    ByteBuffer(){
      clear();
    }

    ByteBuffer(uint8_t* data, size_t n){
      content.reserve(n);
      clear();
      if (data != NULL){
        std::copy(data, data + n, content.begin());
      }
    }

  virtual ~ByteBuffer() = default;

  void clear(){
    pos = 0;
    content.clear();
  }

  void resize(size_t n){
    content.resize(n);
  }

  size_t size(){
    return content.size();
  }

  size_t capacity(){
    return content.capacity();
  }

  void move(size_t index = 0) {
    if(index < 0 || index >= content.size())
      throw std::out_of_range("position is out of range");
    pos = index;
  }

  size_t position(){
    return pos;
  }

  uint8_t* data(){
    return content.data();
  }

  uint8_t get(size_t index){
    return content.at(index);
  }

  uint8_t get(){
    return get(pos);
  }

  void put(size_t index, uint8_t b){
    content.insert(content.begin() + index, b);
  }

  void put(uint8_t b){
    put(pos, b);
  }

  template<typename T> T read(size_t index) {
    size_t n = sizeof(T);
		if (content.size() < index + n){
      throw std::out_of_range ("read index out of range");
    }
			
    T data = *(reinterpret_cast<T*>(&content.at(index)));
    pos += n;
    return data;
	}

  template<typename T> T read() {
	  return read<T>(pos);
	}

	template<typename T> void write(T data, size_t index) {
    size_t n = sizeof(data);
		if (content.size() < (index + n))
      resize(index + n);

    uint8_t* raw_data = reinterpret_cast<uint8_t*>(&data);
    std::copy(raw_data, raw_data + n, content.begin() + index);
		pos = index + n;
	}

	template<typename T> void write(T data) {
    write(data, pos);
  }

};
