#include "StrUtil.h"
#include <algorithm>

std::string StrUtil::toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}
