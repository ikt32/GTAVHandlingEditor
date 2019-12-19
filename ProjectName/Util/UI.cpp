#include "UI.h"

#include "../Constants.h"
#include "inc/natives.h"
#include "fmt/format.h"

namespace {
    int prevNotif;
}

void UI::ShowText(float x, float y, float scale, const std::string& text) {
    UI::SET_TEXT_FONT(0);
    UI::SET_TEXT_SCALE(scale, scale);
    UI::SET_TEXT_COLOUR(255, 255, 255, 255);
    UI::SET_TEXT_WRAP(0.0, 1.0);
    UI::SET_TEXT_CENTRE(0);
    UI::SET_TEXT_OUTLINE();
    UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
    UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text.c_str());
    UI::END_TEXT_COMMAND_DISPLAY_TEXT(x, y);
}

void showNotification(const std::string& message, int* prevNotification) {
    if (prevNotification != nullptr && *prevNotification != 0) {
        UI::_REMOVE_NOTIFICATION(*prevNotification);
    }
    UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");

    UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(message.c_str());

    int id = UI::_DRAW_NOTIFICATION(false, false);
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
