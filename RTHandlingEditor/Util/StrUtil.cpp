#include "StrUtil.h"
#include <algorithm>
#include <sstream>

namespace {
    template<typename Out>
    void split(const std::string& s, char delim, Out result) {
        std::stringstream ss;
        ss.str(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            *(result++) = item;
        }
    }
}

std::vector<std::string> StrUtil::split(const std::string& s, char delim) {
    std::vector<std::string> elems;
    ::split(s, delim, std::back_inserter(elems));
    return elems;
}

std::string StrUtil::toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}
