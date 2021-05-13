#pragma once

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

namespace bitcaskpp {

enum class ExceptionType {
    FILE_NOT_FOUND = 0,
    KEY_NOT_FOUND,
    CORRUPTED_ENTRY,
    NOT_IMPLEMENTED,
    UNKNOWN,
};

class Exception : public std::runtime_error {
   public:
    explicit Exception(const std::string &message)
        : std::runtime_error(message), _type(ExceptionType::UNKNOWN) {
        std::cerr << "Message :: " + message + "\n";
    }

    Exception(enum ExceptionType type, const std::string &message)
        : std::runtime_error(message), _type(type) {
        std::cerr << "\nException Type :: " + to_string(_type) +
                         "\nMessage :: " + message + "\n";
    }

   private:
    ExceptionType _type;

    std::string to_string(const ExceptionType &type) {
        switch (type) {
            case ExceptionType::FILE_NOT_FOUND:
                return "FILE_NOT_FOUND";
            case ExceptionType::KEY_NOT_FOUND:
                return "KEY_NOT_FOUND";
            case ExceptionType::CORRUPTED_ENTRY:
                return "CORRUPTED_ENTRY";
            case ExceptionType::NOT_IMPLEMENTED:
                return "NOT_IMPLEMENTED";
            default:
                return "UNKNOWN";
        }
    }
};

class NotImplementedException : public Exception {
   public:
    NotImplementedException() = delete;
    explicit NotImplementedException(const std::string &message)
        : Exception(ExceptionType::NOT_IMPLEMENTED, message) {}
};

}  // namespace bitcaskpp
