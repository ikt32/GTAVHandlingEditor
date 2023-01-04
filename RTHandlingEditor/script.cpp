/*
 * GTA V Real Time Handling editor
 * This plugin is intended for help with creating handling mods, preventing
 * the need to restart the game to test each change to each parameter. As such
 * this plugin is not intended for normal usage during gameplay.
 */

#include "script.h"

#include "Memory/HandlingInfo.h"

#include "keyboard.h"
#include "Memory/NativeMemory.hpp"
#include "Util/Logger.hpp"

#include "inc/natives.h"
#include "inc/types.h"
#include "inc/enums.h"
#include "inc/main.h"

#include "tinyxml2.h"
#include "Memory/VehicleExtensions.hpp"
#include "HandlingItem.h"
#include "Constants.h"
#include "Util/Paths.h"
#include <filesystem>
#include "Util/StrUtil.h"
#include "fmt/format.h"
#include "Util/UI.h"
#include "Compatibility.h"
#include "Util/AddonSpawnerCache.h"
#include "HandlingFlags.h"
#include "HandlingNotes.h"
#include "HandlingConversion.h"

namespace fs = std::filesystem;

using VExt = VehicleExtensions;
std::vector<RTHE::CHandlingDataItem> g_handlingDataItems;

void UpdateHandlingDataItems() {
    logger.Write(DEBUG, "[Files] Clearing and handling items...");
    g_handlingDataItems.clear();
    const std::string absoluteModPath = Paths::GetModPath();
    const std::string handlingsPath = absoluteModPath + "\\HandlingFiles";

    if (!(fs::exists(fs::path(handlingsPath)) && fs::is_directory(fs::path(handlingsPath)))) {
        logger.Write(WARN, "[Files] Directory [%s] not found!", handlingsPath.c_str());
        return;
    }
    //else
    //    logger.Write(DEBUG, "[Files] Directory [%s] found :)", handlingsPath.c_str());

    for (auto& file : fs::directory_iterator(handlingsPath)) {
        // logger.Write(DEBUG, "[Files] Finding Handling Files...");
        // Only parse meta/xml files
        if (StrUtil::toLower(fs::path(file).extension().string()) != ".meta" &&
            StrUtil::toLower(fs::path(file).extension().string()) != ".xml")
            continue;

        RTHE::CHandlingDataItem handling = RTHE::ParseXMLItem(file.path().string());
        if (handling.Metadata.HandlingName.empty()) {
            logger.Write(WARN,
                "[Files] Handling file [%s] failed to parse, skipping...",
                file.path().string().c_str());
            continue;
        }
        g_handlingDataItems.push_back(handling);
        logger.Write(DEBUG, "[Files] Added handling [%s]", handling.Metadata.HandlingName.c_str());
    }
    logger.Write(INFO, "[Files] Configs loaded: %d", g_handlingDataItems.size());
}

void PromptSave(Vehicle vehicle, Hash handlingNameHash) {
    // Find out handling name.
    std::string autoHandlingName;

    auto modelNamesCache = ASCache::Get();
    auto foundNameIt = modelNamesCache.find(handlingNameHash);
    auto foundModelNameIt = modelNamesCache.find(ENTITY::GET_ENTITY_MODEL(vehicle));
    bool found = false;
    if (foundNameIt != modelNamesCache.end()) {
        autoHandlingName = foundNameIt->second;
        found = true;
    }
    else if (foundModelNameIt != modelNamesCache.end()){
        // handling hash doesn't correspond to any model, so try with the modelName.
        autoHandlingName = foundModelNameIt->second;
    }

    std::string handlingNameMessage = "Enter handling name.";
    if (!found && !autoHandlingName.empty()) {
        handlingNameMessage += " This is a guess. Check and fix if needed.";
    }

    UI::Notify(handlingNameMessage);
    MISC::DISPLAY_ONSCREEN_KEYBOARD(LOCALIZATION::GET_CURRENT_LANGUAGE() == 0, "FMMC_KEY_TIP8", "", autoHandlingName.c_str(), "", "", "", 64);
    while (MISC::UPDATE_ONSCREEN_KEYBOARD() == 0) {
        WAIT(0);
    }
    if (!MISC::GET_ONSCREEN_KEYBOARD_RESULT()) {
        UI::Notify("Cancelled save");
        return;
    }

    std::string handlingName = MISC::GET_ONSCREEN_KEYBOARD_RESULT();
    if (handlingName.empty()) {
        UI::Notify("Cancelled save - No handling name entered");
        return;
    }

    UI::Notify("Enter file name");
    MISC::DISPLAY_ONSCREEN_KEYBOARD(LOCALIZATION::GET_CURRENT_LANGUAGE() == 0, "FMMC_KEY_TIP8", "", autoHandlingName.c_str(), "", "", "", 64);
    while (MISC::UPDATE_ONSCREEN_KEYBOARD() == 0) {
        WAIT(0);
    }
    if (!MISC::GET_ONSCREEN_KEYBOARD_RESULT()) {
        UI::Notify("Cancelled save");
        return;
    }

    std::string fileName = MISC::GET_ONSCREEN_KEYBOARD_RESULT();
    if (fileName.empty()) {
        UI::Notify("Cancelled save - No file name entered");
        return;
    }

    uint32_t saveFileSuffix = 0;
    std::string outFile = fileName;

    const std::string absoluteModPath = Paths::GetModPath();
    const std::string handlingsPath = absoluteModPath + "\\HandlingFiles";

    bool duplicate;
    do {
        duplicate = false;
        for (auto& p : std::filesystem::directory_iterator(handlingsPath)) {
            if (StrUtil::toLower(p.path().stem().string()) == StrUtil::toLower(outFile)) {
                duplicate = true;
                outFile = fmt::format("{}_{:02d}", fileName.c_str(), saveFileSuffix++);
                break;
            }
        }
    } while (duplicate);

    auto h = getHandling(vehicle);
    h.Metadata.HandlingName = handlingName;
    std::string outPath = fmt::format("{}/{}.xml", handlingsPath, outFile);
    std::replace(outPath.begin(), outPath.end(), '\\', '/');
    RTHE::SaveXMLItem(h, outPath);
    UI::Notify(fmt::format("Saved as {}", outFile));
}

void UpdateCheats() {
    if (MISC::_HAS_CHEAT_STRING_JUST_BEEN_ENTERED(StrUtil::joaat("rtherld"))) {
        Notes::Load();
        Flags::Load();
    }
}

void onMenuInit() {
    UpdateHandlingDataItems();
    GetMenu().ReadSettings();
}
void onMenuClose() {

}

void ScriptMain() {
    setupCompatibility();
    VExt::Init();

    GetMenu().RegisterOnMain([] { onMenuInit(); });
    GetMenu().RegisterOnExit([] { onMenuClose(); });

    std::string absoluteModPath = Paths::GetModPath();
    std::string settingsMenuFile = absoluteModPath + "\\settings_menu.ini";
    GetMenu().SetFiles(settingsMenuFile);
    GetMenu().Initialize();
    GetMenu().ReadSettings();

    Notes::Load();
    Flags::Load();
    UpdateHandlingDataItems();

    while (true) {
        UpdateMenu();
        UpdateCheats();
        WAIT(0);
    }
}
