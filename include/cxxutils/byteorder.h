#pragma once

#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>


/* Example
if(ByteOrder::endianness() == ByteOrder::Endianness::Big){
    std::cout << "big endian machine" << std::endl;
} else {
    std::cout << "little endian machine" << std::endl;
}

std::cout << (int) ByteOrder::endianness() << std::endl;

uint16_t n = ByteOrder::toLittleEndian<uint16_t>(1);
std::cout << "litle: " << (std::bitset<16>(n)) << std::endl;

n = ByteOrder::toBigEndian<uint16_t>(1);
std::cout << "big: " << (std::bitset<16>(n)) << std::endl;

int n = 12;
char* data = (char *) & n;
int n2 = ByteOrder::toLittleEndian<int>(data);

*/


class ByteOrder {
   public:
    enum class Endianness { Little = 0, Big = 1 };

    inline static Endianness endianness() {
        const uint16_t n = 1;
        return (n >> 8) == 0 ? Endianness::Big : Endianness::Little;
    }

    template <class T>
    static std::array<char, sizeof(T)> toLittleEndian(T v) {
        return std::move(to_bytes<T>(v, Endianness::Little));
    }

    template <class T>
    static std::string toLittleEndianString(T v) {
        auto buffer = to_bytes<T>(v, Endianness::Little);
        std::string str(buffer.begin(), buffer.end());
        return std::move(str);
    }

    template <class T>
    static T fromLittleEndian(const char *data) {
        return from_bytes<T>(data, Endianness::Little);
    }

    template<class T>
    static T readLittleEndian(std::istream &reader, size_t offset) {
        reader.seekg(offset, std::ios_base::beg);
        size_t size = sizeof(T);
        std::string buffer(size, '\0');
        reader.read(buffer.data(), size);
        return from_bytes<T>(buffer.data(), Endianness::Little);
    }

    template<class T>
    static void writeLittleEndian(std::ostream &writer, T v) {
        auto buffer = to_bytes<T>(v, Endianness::Little);
        writer.write(buffer.data(), buffer.size());
    }


    template <class T>
    static std::array<char, sizeof(T)> toBigEndian(T v) {
        return std::move(to_bytes<T>(v, Endianness::Big));
    }

    template <class T>
    static std::string toBigEndianString(T v) {
        auto buffer = to_bytes<T>(v, Endianness::Big);
        std::string str(buffer.begin(), buffer.end());
        return std::move(str);
    }

    template <class T>
    static T fromBigEndian(const char *data) {
        return from_bytes<T>(data, Endianness::Big);
    }

    template<class T>
    static T readBigEndian(std::istream &reader, size_t offset) {
        reader.seekg(offset, std::ios_base::beg);
        size_t size = sizeof(T);
        std::string buffer(size, '\0');
        reader.read(buffer.data(), size);
        return from_bytes<T>(buffer.data(), Endianness::Little);
    }

    template<class T>
    static void writeBigEndian(std::ostream &writer, T v) {
        auto buffer = to_bytes<T>(v, Endianness::Big);
        writer.write(buffer.data(), buffer.size());
    }

   private:
    template <class T>
    static std::array<char, sizeof(T)> to_bytes(T v, enum Endianness e) {
        static_assert(std::is_pod<T>::value, "Expected pod type.");
        std::array<char, sizeof(T)> v_arrray;
        std::memcpy(v_arrray.data(), &v, sizeof(T));
        if (e != endianness()) std::reverse(v_arrray.begin(), v_arrray.end());
        return std::move(v_arrray);
    }

    template <class T>
    static T from_bytes(const char *data, enum Endianness e) {
        static_assert(std::is_pod<T>::value, "Expected pod type.");
        std::array<char, sizeof(T)> v_arrray;
        std::memcpy(v_arrray.data(), data, sizeof(T));
        if (e != endianness()) std::reverse(v_arrray.begin(), v_arrray.end());
        T v;
        std::memcpy(&v, v_arrray.data(), sizeof(T));
        return v;
    }

};
