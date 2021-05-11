#include "UI.h"

#include "../Constants.h"
#include "inc/natives.h"
#include "fmt/format.h"

namespace {
    int prevNotif;
}

void UI::ShowText(float x, float y, float scale, const std::string& text) {
    UI::ShowText(x, y, scale, text, { 255, 255, 255, 255 }, true);
}

void UI::ShowText(float x, float y, float scale, const std::string& text, SColor color, bool outline) {
    HUD::SET_TEXT_FONT(0);
    HUD::SET_TEXT_SCALE(scale, scale);
    HUD::SET_TEXT_COLOUR(color.R, color.G, color.B, color.A);
    HUD::SET_TEXT_WRAP(0.0, 1.0);
    HUD::SET_TEXT_CENTRE(0);
    if (outline) HUD::SET_TEXT_OUTLINE();
    HUD::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
    HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text.c_str());
    HUD::END_TEXT_COMMAND_DISPLAY_TEXT(x, y, 0);
}

void showNotification(const std::string& message, int* prevNotification) {
    if (prevNotification != nullptr && *prevNotification != 0) {
        HUD::THEFEED_REMOVE_ITEM(*prevNotification);
    }
    HUD::BEGIN_TEXT_COMMAND_THEFEED_POST("STRING");

    HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(message.c_str());

    int id = HUD::END_TEXT_COMMAND_THEFEED_POST_TICKER(false, false);
    if (prevNotification != nullptr) {
        *prevNotification = id;
    }
}

void showNotification(const std::string& message) {
    showNotification(message, &prevNotif);
}

void UI::Notify(const std::string& message, bool removePrevious) {
    int* notifHandleAddr = nullptr;
    if (removePrevious) {
        notifHandleAddr = &prevNotif;
    }
    showNotification(fmt::format("{}\n{}", Constants::NotificationPrefix, message), notifHandleAddr);
}

void UI::Notify(const std::string& message) {
    Notify(message, true);
}
