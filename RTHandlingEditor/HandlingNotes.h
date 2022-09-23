#pragma once
#include <map>
#include <string>
#include <vector>

namespace Notes {
    struct SHandlingParamInfo {
        std::string Details;
        std::vector<std::string> ExtraLines;
    };

    void Load();

    std::string GetVersion();
    const std::map<std::string, SHandlingParamInfo>& GetHandlingNotes();
}
