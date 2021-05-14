#pragma once

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

namespace bitcaskcpp {

class Exception : public std::runtime_error {
   public:
    explicit Exception(const std::string &message)
        : std::runtime_error(message){
    }

};

}  // namespace bitcaskcpp
