#pragma once
#include <string>
#include <vector>

namespace StrUtil {
    constexpr unsigned long joaat(const char* s) {
        unsigned long hash = 0;
        for (; *s != '\0'; ++s) {
            auto c = *s;
            if (c >= 0x41 && c <= 0x5A) {
                c += 0x20;
            }
            hash += c;
            hash += hash << 10;
            hash ^= hash >> 6;
        }
        hash += hash << 3;
        hash ^= hash >> 11;
        hash += hash << 15;
        return hash;
    }

    std::string toLower(std::string s);

    std::vector<std::string> split(const std::string& s, char delim);
}
