#pragma once

// ---------------------------------------------------
// all common functions/constants for this project
// ---------------------------------------------------

#include <string>
#include <vector>

#define forever for (;;)


const std::string LocalServer = "127.0.0.1";

inline std::ostream& operator << (std::ostream& os, std::vector<char> vec)
{
    for (auto c : vec) {
        os << c;
    }
    return os;
}