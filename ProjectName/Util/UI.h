#pragma once
#include <string>

namespace UI {
    void ShowText(float x, float y, float scale, const std::string& text);
    void Notify(const std::string& message, bool removePrevious);
    void Notify(const std::string& message);
}
