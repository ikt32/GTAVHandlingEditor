#pragma once
#include <vector>
#include <string>

struct SCell {
    std::string Contents;
    bool Enable;
};

struct STable {
    std::vector<std::vector<SCell>> Cells;
};
