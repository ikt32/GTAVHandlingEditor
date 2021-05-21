#pragma once
#include <string>
#include <vector>

namespace Flags {
    struct SFlag {
        std::string Name;
        std::string Description;
    };

    void Load();

    const std::vector<SFlag>& GetModelFlags();
    const std::vector<SFlag>& GetHandlingFlags();
    const std::vector<SFlag>& GetDamageFlags();
    const std::vector<SFlag>& GetAdvancedFlags();
}
