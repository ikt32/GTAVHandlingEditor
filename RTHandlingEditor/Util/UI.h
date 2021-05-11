#pragma once
#include <string>

namespace UI {
    struct SColor {
        int R; int G; int B; int A;
    };

    void ShowText(float x, float y, float scale, const std::string& text);
    void ShowText(float x, float y, float scale, const std::string& text, SColor color, bool outline);
    void Notify(const std::string& message, bool removePrevious);
    void Notify(const std::string& message);
}
