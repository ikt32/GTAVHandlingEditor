#include "script.h"

#include "Memory/HandlingInfo.h"
#include "Memory/VehicleExtensions.hpp"
#include "ScriptUtils.h"
#include "HandlingItem.h"
#include "Util/StrUtil.h"
#include "Util/UI.h"
#include "Constants.h"
#include "Table.h"
#include "HandlingFlags.h"
#include "HandlingNotes.h"
#include "AdvancedDataNames.h"
#include "HandlingUtils.h"

#include <HandlingReplacement.h>
#include <menu.h>
#include <menukeyboard.h>
#include <inc/natives.h>
#include <fmt/format.h>

using VExt = VehicleExtensions;

extern std::vector<RTHE::CHandlingDataItem> g_handlingDataItems;

void setHandling(Vehicle vehicle, const RTHE::CHandlingDataItem& handlingDataItem);
void PromptSave(Vehicle vehicle, Hash handlingNameHash);

namespace {
    NativeMenu::Menu menu;
    bool editStock = true;

    const uint32_t tableRows = 8;
    const uint32_t rowCells = 4;

    uint8_t modelFlagsIndex = 0;
    uint8_t handlingFlagsIndex = 0;
    uint8_t damageFlagsIndex = 0;
    uint8_t advancedFlagsIndex = 0;

    std::unordered_map<std::string, std::string> optionNoteMap {
        { "vecCentreOfMassOffset.x", "vecCentreOfMassOffset" },
        { "vecCentreOfMassOffset.y", "vecCentreOfMassOffset" },
        { "vecCentreOfMassOffset.z", "vecCentreOfMassOffset" },

        { "vecInertiaMultiplier.x", "vecInertiaMultiplier" },
        { "vecInertiaMultiplier.y", "vecInertiaMultiplier" },
        { "vecInertiaMultiplier.z", "vecInertiaMultiplier" },

        { "vecSeatOffsetDistX", "vecSeatOffsetDist" },
        { "vecSeatOffsetDistY", "vecSeatOffsetDist" },
        { "vecSeatOffsetDistZ", "vecSeatOffsetDist" },
    };
}

NativeMenu::Menu& GetMenu() {
    return menu;
}

void UpdateMainMenu() {
    menu.Title("Handling Editor");
    menu.Subtitle(Constants::DisplayVersion);

    menu.MenuOption("Edit current handling", "EditMenu");

    Ped playerPed = PLAYER::PLAYER_PED_ID();
    Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);

    if (ENTITY::DOES_ENTITY_EXIST(vehicle)) {
        if (menu.Option("Respawn vehicle").Triggered) {
            Utils::RespawnVehicle(vehicle, playerPed);
            vehicle = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);
            RTHE::CHandlingData* currentHandling = reinterpret_cast<RTHE::CHandlingData*>(VExt::GetHandlingPtr(vehicle));
            if (currentHandling) {
                PHYSICS::SET_CGOFFSET(vehicle,
                    currentHandling->vecCentreOfMassOffset.x,
                    currentHandling->vecCentreOfMassOffset.y,
                    currentHandling->vecCentreOfMassOffset.z);
            }
        }

        menu.MenuOption("Load handling", "LoadMenu");

        if (menu.Option("Save").Triggered) {
            RTHE::CHandlingData* currentHandling = reinterpret_cast<RTHE::CHandlingData*>(VExt::GetHandlingPtr(vehicle));

            PromptSave(vehicle, currentHandling->handlingName);
        }
    }
    else {
        menu.Option("Unavailable", { "Only available while in a vehicle." });
    }

    auto result = menu.Option("About", {
        "Real Time Handling Editor",
        "by ikt"
    });
    if (result.Highlighted) {
        menu.OptionPlusPlus({
            fmt::format("Parameter notes version: {}", Notes::GetVersion()),
            fmt::format("Flags notes version: {}", Flags::GetVersion()),
            "github.com/E66666666/GTAVHandlingInfo"
        });
    }
}

template <typename T>
bool IsNear(T a, T b, T x) {
    return abs(a-b) <= x;
}

bool GetKbEntry(float& val) {
    UI::Notify("Enter value");
    MISC::DISPLAY_ONSCREEN_KEYBOARD(LOCALIZATION::GET_CURRENT_LANGUAGE() == 0, "FMMC_KEY_TIP8", "", 
        fmt::format("{:f}", val).c_str(), "", "", "", 64);
    while (MISC::UPDATE_ONSCREEN_KEYBOARD() == 0) {
        WAIT(0);
    }
    if (!MISC::GET_ONSCREEN_KEYBOARD_RESULT()) {
        UI::Notify("Cancelled value entry");
        return false;
    }

    std::string floatStr = MISC::GET_ONSCREEN_KEYBOARD_RESULT();
    if (floatStr.empty()) {
        UI::Notify("Cancelled value entry");
        return false;
    }

    float parsedValue;
    try {
        parsedValue = std::stof(floatStr, nullptr);
    }
    catch (std::invalid_argument&) {
        UI::Notify("Failed to parse entry.");
        return false;
    }

    val = parsedValue;
    return true;
}

std::string GetKbEntryStr(const std::string& existingString) {
    std::string val;
    UI::Notify("Enter value");
    MISC::DISPLAY_ONSCREEN_KEYBOARD(LOCALIZATION::GET_CURRENT_LANGUAGE() == 0, "FMMC_KEY_TIP8", "",
        existingString.c_str(), "", "", "", 64);
    while (MISC::UPDATE_ONSCREEN_KEYBOARD() == 0) {
        WAIT(0);
    }
    if (!MISC::GET_ONSCREEN_KEYBOARD_RESULT()) {
        UI::Notify("Cancelled value entry");
        return {};
    }

    std::string enteredVal = MISC::GET_ONSCREEN_KEYBOARD_RESULT();
    if (enteredVal.empty()) {
        UI::Notify("Cancelled value entry");
        return {};
    }

    return enteredVal;
}

void SetFlags(unsigned& flagArea, std::string& newFlags) {
    if (!newFlags.empty()) {
        try {
            flagArea = std::stoul(newFlags, nullptr, 16);
        }
        catch (std::invalid_argument&) {
            UI::Notify("Error: Couldn't convert entered value to int.");
        }
        catch (std::out_of_range&) {
            UI::Notify("Error: Entered value out of range.");
        }
    }
    else {
        UI::Notify("Error: No flags entered.");
    }
}

float GetStringWidth(const std::string& text, float scale, int font) {
    HUD::_BEGIN_TEXT_COMMAND_GET_WIDTH("STRING");
    HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text.c_str());
    HUD::SET_TEXT_FONT(font);
    HUD::SET_TEXT_SCALE(scale, scale);
    return HUD::_END_TEXT_COMMAND_GET_WIDTH(true);
}

void DrawRect(float x, float y, float w, float h, UI::SColor color) {
    GRAPHICS::DRAW_RECT(x, y, w, h, color.R, color.G, color.B, color.A, false);
}

void DrawFlagsTable(const STable& flagsTable, uint8_t selectedIndex) {
    const UI::SColor enabledBg =  {   0, 128,   0, 219 };
    const UI::SColor disabledBg = {   0,   0,   0, 169 };

    const UI::SColor enabledFg =  {   0,   0,   0, 255 };
    const UI::SColor disabledFg = { 219, 219, 219, 255 };

    const float cellWidth   = 0.110f;
    const float cellWidthBg = 0.110f;

    const float cellHeight   = 0.040f;
    const float cellHeightBg = 0.040f;

    const float offsetX = menu.MenuWidth() * 2.0f;
    const float offsetY = 0.0f;

    GRAPHICS::SET_SCRIPT_GFX_ALIGN('L', 'T');
    GRAPHICS::SET_SCRIPT_GFX_ALIGN_PARAMS(0, 0, 0, 0);
    for (uint32_t row = 0; row < tableRows; ++row) {
        for (uint32_t cell = 0; cell < rowCells; ++cell) {
            bool selected = selectedIndex == row * rowCells + cell;

            auto colorFg = flagsTable.Cells[row][cell].Enable ? enabledFg : disabledFg;

            auto pX = offsetX + cell * cellWidth;
            auto pY = offsetY + row * cellHeight;

            std::string formattedText = flagsTable.Cells[row][cell].Contents;
            if (formattedText != "N/A" && formattedText.size() > 0) {
                bool guess = formattedText[0] == '_';

                auto words = StrUtil::split(formattedText, '_');

                std::string camelText;
                for (const auto& word : words) {
                    if (word.size() == 0)
                        continue;

                    if (word == "MF" || word == "HF" || word == "DF" || word == "CF")
                        continue;

                    std::string withCapital = StrUtil::toLower(word);
                    withCapital[0] = toupper(withCapital[0]);
                    camelText += withCapital;
                }

                formattedText = camelText;
                if (guess)
                    formattedText.insert(formattedText.begin(), '_');
            }

            std::string shortenedText = formattedText;
            while (GetStringWidth(shortenedText, 0.25f, 0) > cellWidthBg * 0.90f && shortenedText.length() > 5) {
                shortenedText.pop_back();
            }

            if (shortenedText != formattedText) {
                shortenedText += "...";
            }

            UI::ShowText(pX + 0.005f, pY + 0.010f, 0.25f,
                shortenedText,
                colorFg, false);

            auto colorBg = flagsTable.Cells[row][cell].Enable ? enabledBg : disabledBg;
            if (selected) {
                const UI::SColor enabledBg =  {  63, 255,   63, 255 };
                const UI::SColor disabledBg = { 107, 107,  107, 255 };
                colorBg = flagsTable.Cells[row][cell].Enable ? enabledBg : disabledBg;
            }
            DrawRect(pX + cellWidthBg / 2.0f, pY + cellHeightBg / 2.0f,
                cellWidthBg, cellHeightBg, colorBg);
        }
    }
    GRAPHICS::RESET_SCRIPT_GFX_ALIGN();
}

STable CreateFlagsTable(const std::vector<Flags::SFlag>& flagDefinitions, uint32_t flags) {
    STable flagsTable;
    flagsTable.Cells.resize(tableRows);
    for (uint32_t row = 0; row < tableRows; ++row) {
        std::vector<SCell>& rowData = flagsTable.Cells[row];
        rowData.resize(rowCells);
        for (uint32_t cell = 0; cell < rowCells; ++cell) {
            uint32_t flag = 1u << (row * rowCells + cell);
            rowData[cell] = SCell{ flagDefinitions[row * rowCells + cell].Name, (flags & flag) > 0 };
        }
    }
    return flagsTable;
}

void OptionFlags(const std::string& optionName, const std::vector<Flags::SFlag>& flagList, uint32_t& flags, uint8_t& flagIndex) {
    bool show = false;
    std::string strFlags = fmt::format("{:X}", flags);
    if (menu.OptionPlus(fmt::format("{}: {}", optionName, strFlags),
        {},
        &show,
        [&] { if (flagIndex < 31) ++flagIndex; },
        [&] { if (flagIndex > 0) --flagIndex; },
        "", { "Move selection with Left and Right. Space (KB) or RB (Controller) to toggle. Enter to edit flags string." })
            .Triggered) {
        std::string newFlags = GetKbEntryStr(strFlags);
        SetFlags(flags, newFlags);
    }

    if (show) {
        if (flagList.empty()) {
            std::vector<std::string> extra;
            extra.push_back("Table editor disabled.");
            extra.push_back("Error reading flag descriptions, see HandlingEditor.log.");
            menu.OptionPlusPlus(extra, "Flags");
            return;
        }

        std::vector<std::string> extra;
        extra.push_back("Underscores are prefixed to guessed flag names.");

        STable flagsTable = CreateFlagsTable(flagList, flags);

        extra.push_back(fmt::format("Flag 0x{:08X}:", 1u << flagIndex));
        extra.push_back(fmt::format("Name: {}", flagList[flagIndex].Name));
        extra.push_back(fmt::format("Description: {}", flagList[flagIndex].Description));
        menu.OptionPlusPlus(extra, "Flags");

        DrawFlagsTable(flagsTable, flagIndex);

        if (NativeMenu::IsKeyJustUp(VK_SPACE) ||
            PAD::IS_CONTROL_JUST_RELEASED(0, eControl::ControlFrontendRb)) {
            flags ^= 1u << flagIndex;
        }
    }
}

bool IntOptionExtra(const std::string& option, int& var, int min, int max, int step) {
    auto optionNameAltIt = optionNoteMap.find(option);
    std::string optionNameAlt = optionNameAltIt == optionNoteMap.end() ? option : optionNameAltIt->second;
    auto notes = Notes::GetHandlingNotes();
    auto noteIt = notes.find(optionNameAlt);
    std::vector<std::string> details;
    std::vector<std::string> extra;
    if (noteIt != notes.end()) {
        if (!noteIt->second.Details.empty())
            details = { noteIt->second.Details };
        extra = noteIt->second.ExtraLines;
    }

    auto result = menu.IntOption(option, var, min, max, step, {}, details);
    if (result.Highlighted && extra.size() > 0) {
        menu.OptionPlusPlus(extra);
    }
    return result.Triggered || result.ValueChanged;
}

bool FloatOptionExtra(const std::string& option, float& var, float min, float max, float step) {
    auto optionNameAltIt = optionNoteMap.find(option);
    std::string optionNameAlt = optionNameAltIt == optionNoteMap.end() ? option : optionNameAltIt->second;
    auto notes = Notes::GetHandlingNotes();
    auto noteIt = notes.find(optionNameAlt);
    std::vector<std::string> details;
    std::vector<std::string> extra;
    if (noteIt != notes.end()) {
        if (!noteIt->second.Details.empty())
            details = { noteIt->second.Details };
        extra = noteIt->second.ExtraLines;
    }

    auto result = menu.FloatOption(option, var, min, max, step, GetKbEntry, details);
    if (result.Highlighted && extra.size() > 0) {
        menu.OptionPlusPlus(extra);
    }
    return result.Triggered || result.ValueChanged;
}

void UpdateEditMenu() {
    std::string vehicleName = "Unavailable";
    bool hrActive = false;
    Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED(PLAYER::GET_PLAYER_INDEX()), false);
    RTHE::CHandlingData* currentHandling = GetEditHandlingData(editStock, hrActive, vehicleName);

    menu.Title("Edit Handling");
    menu.Subtitle(vehicleName);

    if (!currentHandling) {
        menu.Option("Unavailable", { "Only available while in a vehicle." });
    }

    if (hrActive) {
        menu.BoolOption("Edit original?", editStock,
            { "HandlingReplacement is active. Check box to edit original data.",
              "If active, new vehicle instances spawn with the applied changes, but the current instance does not update.",
              "If not active, new vehicle instances spawn with the unedited original data.",
              "Recommended to enable if HandlingReplacement is active. Remember to respawn after edits!"});
    }

    Utils::DrawCOMAndRollCenters(vehicle, currentHandling);

    FloatOptionExtra("fMass", currentHandling->fMass, 0.0f, 1000000000.0f, 5.0f);

    {
        float fInitialDragCoeff = currentHandling->fInitialDragCoeff * 10000.0f;
        if (FloatOptionExtra("fInitialDragCoeff", fInitialDragCoeff, 0.0f, 100.0f, 0.05f)) {
            currentHandling->fInitialDragCoeff = fInitialDragCoeff / 10000.0f;
        }
    }
    FloatOptionExtra("fDownforceModifier", currentHandling->fDownforceModifier, 0.0f, 1000.0f, 0.5f);

    if (FloatOptionExtra("fPercentSubmerged", currentHandling->fPercentSubmerged, 0.0f, 100.0f, 1.0f)) {
        currentHandling->fSubmergedRatio_ = 100.0f / currentHandling->fPercentSubmerged;
    }

    bool comModified = false;
    comModified |= FloatOptionExtra("vecCentreOfMassOffset.x", currentHandling->vecCentreOfMassOffset.x, -20.0f, 20.0f, 0.01f);
    comModified |= FloatOptionExtra("vecCentreOfMassOffset.y", currentHandling->vecCentreOfMassOffset.y, -20.0f, 20.0f, 0.01f);
    comModified |= FloatOptionExtra("vecCentreOfMassOffset.z", currentHandling->vecCentreOfMassOffset.z, -20.0f, 20.0f, 0.01f);

    if (comModified) {
        // Update COM.
        PHYSICS::SET_CGOFFSET(vehicle,
            currentHandling->vecCentreOfMassOffset.x,
            currentHandling->vecCentreOfMassOffset.y,
            currentHandling->vecCentreOfMassOffset.z);
    }

    FloatOptionExtra("vecInertiaMultiplier.x", currentHandling->vecInertiaMultiplier.x, -100.0f, 100.0f, 0.1f);
    FloatOptionExtra("vecInertiaMultiplier.y", currentHandling->vecInertiaMultiplier.y, -100.0f, 100.0f, 0.1f);
    FloatOptionExtra("vecInertiaMultiplier.z", currentHandling->vecInertiaMultiplier.z, -100.0f, 100.0f, 0.1f);

    {
        // FWD      F1.0 R0.0 XML_F 1.0
        // 60/40    F1.2 R0.8 XML_F 0.6
        // 50/50    F1.0 R1.0 XML_F 0.5
        // 40/60    F0.8 R1.2 XML_F 0.4
        // RWD      F0.0 R1.0 XML_F 0.0
        float fDriveBiasFront = currentHandling->fDriveBiasFront;
        float fDriveBiasRear = currentHandling->fDriveBiasRear;

        float fDriveBiasFrontNorm;

        // Full FWD
        if (fDriveBiasFront == 1.0f && fDriveBiasRear == 0.0f) {
            fDriveBiasFrontNorm = 1.0f;
        }
        // Full RWD
        else if (fDriveBiasFront == 0.0f && fDriveBiasRear == 1.0f) {
            fDriveBiasFrontNorm = 0.0f;
        }
        else {
            fDriveBiasFrontNorm = fDriveBiasFront / 2.0f;
        }

        if (FloatOptionExtra("fDriveBiasFront", fDriveBiasFrontNorm, 0.0f, 1.0f, 0.01f)) {
            // Full FWD
            if (IsNear(fDriveBiasFrontNorm, 1.0f, 0.005f)) {
                currentHandling->fDriveBiasFront = 1.0f;
                currentHandling->fDriveBiasRear = 0.0f;
            }
            // Full RWD
            else if (IsNear(fDriveBiasFrontNorm, 0.0f, 0.005f)) {
                currentHandling->fDriveBiasFront = 0.0f;
                currentHandling->fDriveBiasRear = 1.0f;
            }
            else {
                currentHandling->fDriveBiasFront = fDriveBiasFrontNorm * 2.0f;
                currentHandling->fDriveBiasRear = 2.0f * (1.0f - (fDriveBiasFrontNorm));
            }
        }
    }

    //FloatOptionExtra("fAcceleration", currentHandling->fAcceleration, -1000.0f, 1000.0f, 0.01f);

    {
        int nInitialDriveGears = static_cast<int>(currentHandling->nInitialDriveGears);
        if (IntOptionExtra("nInitialDriveGears", nInitialDriveGears, 1, 10, 1)) {
            currentHandling->nInitialDriveGears = static_cast<uint8_t>(nInitialDriveGears);
        }
    }
    FloatOptionExtra("fInitialDriveForce", currentHandling->fInitialDriveForce, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fDriveInertia", currentHandling->fDriveInertia, -1000.0f, 1000.0f, 0.01f);

    FloatOptionExtra("fClutchChangeRateScaleUpShift", currentHandling->fClutchChangeRateScaleUpShift, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fClutchChangeRateScaleDownShift", currentHandling->fClutchChangeRateScaleDownShift, -1000.0f, 1000.0f, 0.01f);

    {
        float fInitialDriveMaxFlatVel = currentHandling->fInitialDriveMaxFlatVel * 3.6f;
        if (FloatOptionExtra("fInitialDriveMaxFlatVel", fInitialDriveMaxFlatVel, 0.0f, 1000.0f, 0.5f)) {
            currentHandling->fInitialDriveMaxFlatVel = fInitialDriveMaxFlatVel / 3.6f;
            currentHandling->fDriveMaxFlatVel_ = fInitialDriveMaxFlatVel / 3.0f;
        }
    }

    FloatOptionExtra("fBrakeForce", currentHandling->fBrakeForce, -1000.0f, 1000.0f, 0.01f);

    {
        float fBrakeBiasFront = currentHandling->fBrakeBiasFront / 2.0f;
        if (FloatOptionExtra("fBrakeBiasFront", fBrakeBiasFront, -1000.0f, 1000.0f, 0.01f)) {
            currentHandling->fBrakeBiasFront = fBrakeBiasFront * 2.0f;
            currentHandling->fBrakeBiasRear_ = 2.0f * (1.0f - fBrakeBiasFront);
        }
    }
    
    FloatOptionExtra("fHandBrakeForce", currentHandling->fHandBrakeForce, -1000.0f, 1000.0f, 0.01f);

    {
        // rad 2 deg
        float fSteeringLock = currentHandling->fSteeringLock / 0.017453292f;
        if (FloatOptionExtra("fSteeringLock", fSteeringLock, 0.0f, 90.0f, 0.10f)) {
            currentHandling->fSteeringLock = fSteeringLock * 0.017453292f;
            currentHandling->fSteeringLockRatio_ = 1.0f / (fSteeringLock * 0.017453292f);
        }
    }

    bool tcModified = false;
    {
        float fTractionCurveMax = currentHandling->fTractionCurveMax;
        if (FloatOptionExtra("fTractionCurveMax", fTractionCurveMax, 0.0f, 1000.0f, 0.01f)) {
            currentHandling->fTractionCurveMax = fTractionCurveMax;
            currentHandling->fTractionCurveMaxRatio_ = 1.0f / fTractionCurveMax;
            tcModified = true;
        }
    }

    {
        float fTractionCurveMin = currentHandling->fTractionCurveMin;
        if (FloatOptionExtra("fTractionCurveMin", fTractionCurveMin, 0.0f, 1000.0f, 0.01f)) {
            currentHandling->fTractionCurveMin = fTractionCurveMin;
            tcModified = true;
        }
    }

    if (tcModified) {
        currentHandling->fTractionCurveRatio_ = 1.0f / (currentHandling->fTractionCurveMax - currentHandling->fTractionCurveMin);
    }

    {
        // rad 2 deg
        float fTractionCurveLateral = currentHandling->fTractionCurveLateral / 0.017453292f;
        if (FloatOptionExtra("fTractionCurveLateral", fTractionCurveLateral, -1000.0f, 1000.0f, 0.01f)) {
            currentHandling->fTractionCurveLateral = fTractionCurveLateral * 0.017453292f;
            currentHandling->fTractionCurveLateralRatio_ = 1.0f / (fTractionCurveLateral * 0.017453292f);
        }
    }

    {
        float fTractionSpringDeltaMax = currentHandling->fTractionSpringDeltaMax;
        if (FloatOptionExtra("fTractionSpringDeltaMax", fTractionSpringDeltaMax, -1000.0f, 1000.0f, 0.01f)) {
            currentHandling->fTractionSpringDeltaMax = fTractionSpringDeltaMax;
            currentHandling->fTractionSpringDeltaMaxRatio_ = 1.0f / fTractionSpringDeltaMax;
        }
    }

    FloatOptionExtra("fLowSpeedTractionLossMult", currentHandling->fLowSpeedTractionLossMult, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fCamberStiffnesss", currentHandling->fCamberStiffnesss, -1000.0f, 1000.0f, 0.01f);

    { // todo: ???
        float fTractionBiasFront = currentHandling->fTractionBiasFront / 2.0f;
        if (FloatOptionExtra("fTractionBiasFront", fTractionBiasFront, 0.0f, 1.0f, 0.01f)) {
            currentHandling->fTractionBiasFront = 2.0f * fTractionBiasFront;
            currentHandling->fTractionBiasRear = 2.0f * (1.0f - (fTractionBiasFront));
        }
    }

    FloatOptionExtra("fTractionLossMult", currentHandling->fTractionLossMult, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fSuspensionForce", currentHandling->fSuspensionForce, -1000.0f, 1000.0f, 0.01f);

    {
        float fSuspensionCompDamp = currentHandling->fSuspensionCompDamp * 10.0f;
        if (FloatOptionExtra("fSuspensionCompDamp", fSuspensionCompDamp, -1000.0f, 1000.0f, 0.01f)) {
            currentHandling->fSuspensionCompDamp = fSuspensionCompDamp / 10.0f;
        }
    }

    {
        float fSuspensionReboundDamp = currentHandling->fSuspensionReboundDamp * 10.0f;
        if (FloatOptionExtra("fSuspensionReboundDamp", fSuspensionReboundDamp, -1000.0f, 1000.0f, 0.01f)) {
            currentHandling->fSuspensionReboundDamp = fSuspensionReboundDamp / 10.0f;
        }
    }

    FloatOptionExtra("fSuspensionUpperLimit", currentHandling->fSuspensionUpperLimit, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fSuspensionLowerLimit", currentHandling->fSuspensionLowerLimit, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fSuspensionRaise", currentHandling->fSuspensionRaise, -1000.0f, 1000.0f, 0.01f);

    {
        float fSuspensionBiasFront = currentHandling->fSuspensionBiasFront / 2.0f;
        if (FloatOptionExtra("fSuspensionBiasFront", fSuspensionBiasFront, -1000.0f, 1000.0f, 0.01f)) {
            currentHandling->fSuspensionBiasFront = fSuspensionBiasFront * 2.0f;
            currentHandling->fSuspensionBiasRear_ = 2.0f * (1.0f - (fSuspensionBiasFront));
        }
    }

    FloatOptionExtra("fAntiRollBarForce", currentHandling->fAntiRollBarForce, -1000.0f, 1000.0f, 0.01f);

    {
        float fAntiRollBarBiasFront = currentHandling->fAntiRollBarBiasFront / 2.0f;
        if (FloatOptionExtra("fAntiRollBarBiasFront", fAntiRollBarBiasFront, -1000.0f, 1000.0f, 0.01f)) {
            currentHandling->fAntiRollBarBiasFront = fAntiRollBarBiasFront * 2.0f;
            currentHandling->fAntiRollBarBiasRear_ = 2.0f * (1.0f - (fAntiRollBarBiasFront));
        }
    }

    FloatOptionExtra("fRollCentreHeightFront", currentHandling->fRollCentreHeightFront, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fRollCentreHeightRear", currentHandling->fRollCentreHeightRear, -1000.0f, 1000.0f, 0.01f);

    FloatOptionExtra("fCollisionDamageMult", currentHandling->fCollisionDamageMult, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fWeaponDamageMult", currentHandling->fWeaponDamageMult, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fDeformationDamageMult", currentHandling->fDeformationDamageMult, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fEngineDamageMult", currentHandling->fEngineDamageMult, -1000.0f, 1000.0f, 0.01f);

    FloatOptionExtra("fPetrolTankVolume", currentHandling->fPetrolTankVolume, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fOilVolume", currentHandling->fOilVolume, -1000.0f, 1000.0f, 0.01f);

    FloatOptionExtra("vecSeatOffsetDistX", currentHandling->vecSeatOffsetDist.x, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecSeatOffsetDistY", currentHandling->vecSeatOffsetDist.y, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecSeatOffsetDistZ", currentHandling->vecSeatOffsetDist.z, -1000.0f, 1000.0f, 0.01f);

    IntOptionExtra("nMonetaryValue", currentHandling->nMonetaryValue, 0, 1000000, 1);

    OptionFlags("strModelFlags", Flags::GetModelFlags(), currentHandling->strModelFlags, modelFlagsIndex);
    OptionFlags("strHandlingFlags", Flags::GetHandlingFlags(), currentHandling->strHandlingFlags, handlingFlagsIndex);
    OptionFlags("strDamageFlags", Flags::GetDamageFlags(), currentHandling->strDamageFlags, damageFlagsIndex);

    {
        std::string AIHandling;
        switch (currentHandling->AIHandling) {
            case StrUtil::joaat("AVERAGE"):
                AIHandling = "AVERAGE";
                break;
            case StrUtil::joaat("SPORTS_CAR"):
                AIHandling = "SPORTS_CAR";
                break;
            case StrUtil::joaat("TRUCK"):
                AIHandling = "TRUCK";
                break;
            case StrUtil::joaat("CRAP"):
                AIHandling = "CRAP";
                break;
            default:
                AIHandling = fmt::format("{:X}", currentHandling->AIHandling);
        }
        menu.Option(fmt::format("AIHandling: {}", AIHandling), {
            fmt::format("AVERAGE: {:X}", StrUtil::joaat("AVERAGE")),
            fmt::format("SPORTS_CAR: {:X}", StrUtil::joaat("SPORTS_CAR")),
            fmt::format("TRUCK: {:X}", StrUtil::joaat("TRUCK")),
            fmt::format("CRAP: {:X}", StrUtil::joaat("CRAP")),
        });
    }

    for (uint16_t idx = 0; idx < currentHandling->m_subHandlingData.GetCount(); ++idx) {
        RTHE::CBaseSubHandlingData* subHandlingData = currentHandling->m_subHandlingData.Get(idx);

        if (!subHandlingData)
            continue;

        auto type = subHandlingData->GetHandlingType();
        switch (type) {
            case RTHE::HANDLING_TYPE_BIKE:
                menu.MenuOption("CBikeHandlingData", "EditCBikeHandlingDataMenu");
                break;
            case RTHE::HANDLING_TYPE_FLYING:
                menu.MenuOption("CFlyingHandlingData", "EditCFlyingHandlingDataMenu");
                break;
            case RTHE::HANDLING_TYPE_VERTICAL_FLYING:
                menu.MenuOption("CSpecialFlightHandlingData", "EditCSpecialFlightHandlingDataMenu");
                break;
            case RTHE::HANDLING_TYPE_BOAT:
                menu.MenuOption("CBoatHandlingData", "EditCBoatHandlingDataMenu");
                break;
            case RTHE::HANDLING_TYPE_SEAPLANE:
                menu.MenuOption("CSeaPlaneHandlingData", "EditCSeaPlaneHandlingDataMenu");
                break;
            case RTHE::HANDLING_TYPE_SUBMARINE:
                menu.MenuOption("CSubmarineHandlingData", "EditCSubmarineHandlingDataMenu");
                break;
            case RTHE::HANDLING_TYPE_TRAILER:
                menu.MenuOption("CTrailerHandlingData", "EditCTrailerHandlingDataMenu");
                break;
            case RTHE::HANDLING_TYPE_CAR:
                menu.MenuOption("CCarHandlingData", "EditCCarHandlingDataMenu");
                break;
            case RTHE::HANDLING_TYPE_WEAPON:
                menu.Option("HANDLING_TYPE_WEAPON",
                    { "Unsupported handling type" });
                break;
            case RTHE::HANDLING_TYPE_TRAIN:
                menu.Option("HANDLING_TYPE_TRAIN",
                    { "Unsupported handling type" });
                break;
            default:
                menu.Option(fmt::format("Handling type: {}", static_cast<int>(type)),
                    { "Unknown handling type" });
                break;
        }
    }
}

void UpdateCBikeHandlingDataMenu() {
    std::string vehicleName = "Unavailable";
    bool hrActive = false;
    Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED(PLAYER::GET_PLAYER_INDEX()), false);
    RTHE::CHandlingData* currentHandling = GetEditHandlingData(editStock, hrActive, vehicleName);

    menu.Title("CBikeHandlingData");
    menu.Subtitle(vehicleName);

    RTHE::CBaseSubHandlingData* subHandlingData = GetSubHandlingData(currentHandling, RTHE::eHandlingType::HANDLING_TYPE_BIKE);

    if (!subHandlingData) {
        menu.Option("No CCarHandlingData");
        return;
    }

    RTHE::CBikeHandlingData* bikeHandlingData = static_cast<RTHE::CBikeHandlingData*>(subHandlingData);

    FloatOptionExtra("fLeanFwdCOMMult",                  bikeHandlingData->floatfLeanFwdCOMMult,        -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fLeanFwdForceMult",                bikeHandlingData->floatfLeanFwdForceMult,      -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fLeanBakCOMMult",                  bikeHandlingData->floatfLeanBakCOMMult,        -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fLeanBakForceMult",                bikeHandlingData->floatfLeanBakForceMult,      -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fMaxBankAngle",                    bikeHandlingData->floatfMaxBankAngle,          -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fFullAnimAngle",                   bikeHandlingData->floatfFullAnimAngle,         -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fDesLeanReturnFrac",               bikeHandlingData->floatfDesLeanReturnFrac,     -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fStickLeanMult",                   bikeHandlingData->floatfStickLeanMult,         -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fBrakingStabilityMult",            bikeHandlingData->floatfBrakingStabilityMult,  -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fInAirSteerMult",                  bikeHandlingData->floatfInAirSteerMult,        -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fWheelieBalancePoint",             bikeHandlingData->floatfWheelieBalancePoint,   -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fStoppieBalancePoint",             bikeHandlingData->floatfStoppieBalancePoint,   -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fWheelieSteerMult",                bikeHandlingData->floatfWheelieSteerMult,      -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fRearBalanceMult",                 bikeHandlingData->floatfRearBalanceMult,       -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fFrontBalanceMult",                bikeHandlingData->floatfFrontBalanceMult,      -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fBikeGroundSideFrictionMult",      bikeHandlingData->floatfBikeGroundSideFrictionMult, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fBikeWheelGroundSideFrictionMult", bikeHandlingData->floatfBikeWheelGroundSideFrictionMult, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fBikeOnStandLeanAngle",            bikeHandlingData->floatfBikeOnStandLeanAngle,  -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fBikeOnStandSteerAngle",           bikeHandlingData->floatfBikeOnStandSteerAngle, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fJumpForce",                       bikeHandlingData->floatfJumpForce,             -1000.0f, 1000.0f, 0.01f);
}

void UpdateCFlyingHandlingDataMenu() {
    std::string vehicleName = "Unavailable";
    bool hrActive = false;
    Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED(PLAYER::GET_PLAYER_INDEX()), false);
    RTHE::CHandlingData* currentHandling = GetEditHandlingData(editStock, hrActive, vehicleName);

    menu.Title("CFlyingHandlingData");
    menu.Subtitle(vehicleName);

    RTHE::CBaseSubHandlingData* subHandlingData = GetSubHandlingData(currentHandling, RTHE::eHandlingType::HANDLING_TYPE_FLYING);

    if (!subHandlingData) {
        menu.Option("No CCarHandlingData");
        return;
    }

    RTHE::CFlyingHandlingData* flyingHandlingData = static_cast<RTHE::CFlyingHandlingData*>(subHandlingData);

    FloatOptionExtra("fThrust",               flyingHandlingData->fThrust,               -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fThrustFallOff",        flyingHandlingData->fThrustFallOff,        -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fThrustVectoring",      flyingHandlingData->fThrustVectoring,      -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fInitialThrust",        flyingHandlingData->fInitialThrust,        -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fInitialThrustFallOff", flyingHandlingData->fInitialThrustFallOff, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fYawMult",              flyingHandlingData->fYawMult,              -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fYawStabilise",         flyingHandlingData->fYawStabilise,         -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fSideSlipMult",         flyingHandlingData->fSideSlipMult,         -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fInitialYawMult",       flyingHandlingData->fInitialYawMult,       -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fRollMult",             flyingHandlingData->fRollMult,             -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fRollStabilise",        flyingHandlingData->fRollStabilise,        -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fInitialRollMult",      flyingHandlingData->fInitialRollMult,      -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fPitchMult",            flyingHandlingData->fPitchMult,            -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fPitchStabilise",       flyingHandlingData->fPitchStabilise,       -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fInitialPitchMult",     flyingHandlingData->fInitialPitchMult,     -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fFormLiftMult",         flyingHandlingData->fFormLiftMult,         -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fAttackLiftMult",       flyingHandlingData->fAttackLiftMult,       -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fAttackDiveMult",       flyingHandlingData->fAttackDiveMult,       -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fGearDownDragV",        flyingHandlingData->fGearDownDragV,        -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fGearDownLiftMult",     flyingHandlingData->fGearDownLiftMult,     -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fWindMult",             flyingHandlingData->fWindMult,             -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fMoveRes",              flyingHandlingData->fMoveRes,              -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecTurnRes.x",          flyingHandlingData->vecTurnRes.x,          -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecTurnRes.y",          flyingHandlingData->vecTurnRes.y,          -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecTurnRes.z",          flyingHandlingData->vecTurnRes.z,          -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecSpeedRes.x",         flyingHandlingData->vecSpeedRes.x,         -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecSpeedRes.y",         flyingHandlingData->vecSpeedRes.y,         -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecSpeedRes.z",         flyingHandlingData->vecSpeedRes.z,         -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fGearDoorFrontOpen",                 flyingHandlingData->fGearDoorFrontOpen,                 -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fGearDoorRearOpen",                  flyingHandlingData->fGearDoorRearOpen,                  -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fGearDoorRearOpen2",                 flyingHandlingData->fGearDoorRearOpen2,                 -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fGearDoorRearMOpen",                 flyingHandlingData->fGearDoorRearMOpen,                 -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fTurublenceMagnitudeMax",            flyingHandlingData->fTurublenceMagnitudeMax,            -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fTurublenceForceMulti",              flyingHandlingData->fTurublenceForceMulti,              -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fTurublenceRollTorqueMulti",         flyingHandlingData->fTurublenceRollTorqueMulti,         -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fTurublencePitchTorqueMulti",        flyingHandlingData->fTurublencePitchTorqueMulti,        -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fBodyDamageControlEffectMult",       flyingHandlingData->fBodyDamageControlEffectMult,       -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fInputSensitivityForDifficulty",     flyingHandlingData->fInputSensitivityForDifficulty,     -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fOnGroundYawBoostSpeedPeak",         flyingHandlingData->fOnGroundYawBoostSpeedPeak,         -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fOnGroundYawBoostSpeedCap",          flyingHandlingData->fOnGroundYawBoostSpeedCap,          -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fEngineOffGlideMulti",               flyingHandlingData->fEngineOffGlideMulti,               -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fAfterburnerEffectRadius",           flyingHandlingData->fAfterburnerEffectRadius,           -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fAfterburnerEffectDistance",         flyingHandlingData->fAfterburnerEffectDistance,         -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fAfterburnerEffectForceMulti",       flyingHandlingData->fAfterburnerEffectForceMulti,       -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fSubmergeLevelToPullHeliUnderwater", flyingHandlingData->fSubmergeLevelToPullHeliUnderwater, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fExtraLiftWithRoll",                 flyingHandlingData->fExtraLiftWithRoll,                 -1000.0f, 1000.0f, 0.01f);

    std::string handlingTypeName;

    switch (flyingHandlingData->handlingType) {
        case RTHE::eHandlingType::HANDLING_TYPE_BIKE:            handlingTypeName = "HANDLING_TYPE_BIKE"; break;
        case RTHE::eHandlingType::HANDLING_TYPE_FLYING:          handlingTypeName = "HANDLING_TYPE_FLYING"; break;
        case RTHE::eHandlingType::HANDLING_TYPE_VERTICAL_FLYING: handlingTypeName = "HANDLING_TYPE_VERTICAL_FLYING"; break;
        case RTHE::eHandlingType::HANDLING_TYPE_BOAT:            handlingTypeName = "HANDLING_TYPE_BOAT"; break;
        case RTHE::eHandlingType::HANDLING_TYPE_SEAPLANE:        handlingTypeName = "HANDLING_TYPE_SEAPLANE"; break;
        case RTHE::eHandlingType::HANDLING_TYPE_SUBMARINE:       handlingTypeName = "HANDLING_TYPE_SUBMARINE"; break;
        case RTHE::eHandlingType::HANDLING_TYPE_TRAIN:           handlingTypeName = "HANDLING_TYPE_TRAIN"; break;
        case RTHE::eHandlingType::HANDLING_TYPE_TRAILER:         handlingTypeName = "HANDLING_TYPE_TRAILER"; break;
        case RTHE::eHandlingType::HANDLING_TYPE_CAR:             handlingTypeName = "HANDLING_TYPE_CAR"; break;
        case RTHE::eHandlingType::HANDLING_TYPE_WEAPON:          handlingTypeName = "HANDLING_TYPE_WEAPON"; break;
        default:
            handlingTypeName = fmt::format("Unknown ({})", static_cast<int>(flyingHandlingData->handlingType));
    }
}

void UpdateCSpecialFlightHandlingDataMenu() {
    std::string vehicleName = "Unavailable";
    bool hrActive = false;
    Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED(PLAYER::GET_PLAYER_INDEX()), false);
    RTHE::CHandlingData* currentHandling = GetEditHandlingData(editStock, hrActive, vehicleName);

    menu.Title("CSpecialFlightHandlingData");
    menu.Subtitle(vehicleName);

    RTHE::CBaseSubHandlingData* subHandlingData = GetSubHandlingData(currentHandling, RTHE::eHandlingType::HANDLING_TYPE_VERTICAL_FLYING);

    if (!subHandlingData) {
        menu.Option("SubHandlingData not found");
        return;
    }

    RTHE::CSpecialFlightHandlingData* flyingHandlingData = static_cast<RTHE::CSpecialFlightHandlingData*>(subHandlingData);

    FloatOptionExtra("vecAngularDamping.x", flyingHandlingData->vecAngularDamping.x, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecAngularDamping.y", flyingHandlingData->vecAngularDamping.y, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecAngularDamping.z", flyingHandlingData->vecAngularDamping.z, -1000.0f, 1000.0f, 0.01f);

    FloatOptionExtra("vecAngularDampingMin.x", flyingHandlingData->vecAngularDampingMin.x, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecAngularDampingMin.y", flyingHandlingData->vecAngularDampingMin.y, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecAngularDampingMin.z", flyingHandlingData->vecAngularDampingMin.z, -1000.0f, 1000.0f, 0.01f);

    FloatOptionExtra("vecLinearDamping.x", flyingHandlingData->vecLinearDamping.x, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecLinearDamping.y", flyingHandlingData->vecLinearDamping.y, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecLinearDamping.z", flyingHandlingData->vecLinearDamping.z, -1000.0f, 1000.0f, 0.01f);

    FloatOptionExtra("vecLinearDampingMin", flyingHandlingData->vecLinearDampingMin.x, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecLinearDampingMin", flyingHandlingData->vecLinearDampingMin.y, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecLinearDampingMin", flyingHandlingData->vecLinearDampingMin.z, -1000.0f, 1000.0f, 0.01f);

    FloatOptionExtra("fLiftCoefficient",              flyingHandlingData->fLiftCoefficient,              -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fCriticalLiftAngle",            flyingHandlingData->fCriticalLiftAngle,            -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fInitialLiftAngle",             flyingHandlingData->fInitialLiftAngle,             -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fMaxLiftAngle",                 flyingHandlingData->fMaxLiftAngle,                 -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fDragCoefficient",              flyingHandlingData->fDragCoefficient,              -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fBrakingDrag",                  flyingHandlingData->fBrakingDrag,                  -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fMaxLiftVelocity",              flyingHandlingData->fMaxLiftVelocity,              -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fMinLiftVelocity",              flyingHandlingData->fMinLiftVelocity,              -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fRollTorqueScale",              flyingHandlingData->fRollTorqueScale,              -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fMaxTorqueVelocity",            flyingHandlingData->fMaxTorqueVelocity,            -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fMinTorqueVelocity",            flyingHandlingData->fMinTorqueVelocity,            -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fYawTorqueScale",               flyingHandlingData->fYawTorqueScale,               -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fSelfLevelingPitchTorqueScale", flyingHandlingData->fSelfLevelingPitchTorqueScale, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fInitalOverheadAssist",         flyingHandlingData->fInitalOverheadAssist,         -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fMaxPitchTorque",               flyingHandlingData->fMaxPitchTorque,               -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fMaxSteeringRollTorque",        flyingHandlingData->fMaxSteeringRollTorque,        -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fPitchTorqueScale",             flyingHandlingData->fPitchTorqueScale,             -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fSteeringTorqueScale",          flyingHandlingData->fSteeringTorqueScale,          -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fMaxThrust",                    flyingHandlingData->fMaxThrust,                    -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fTransitionDuration",           flyingHandlingData->fTransitionDuration,           -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fHoverVelocityScale",           flyingHandlingData->fHoverVelocityScale,           -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fStabilityAssist",              flyingHandlingData->fStabilityAssist,              -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fMinSpeedForThrustFalloff",     flyingHandlingData->fMinSpeedForThrustFalloff,     -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fBrakingThrustScale",           flyingHandlingData->fBrakingThrustScale,           -1000.0f, 1000.0f, 0.01f);

    menu.Option(fmt::format("mode: {:X}", flyingHandlingData->mode));
    menu.Option(fmt::format("strFlags: {:X}", flyingHandlingData->strFlags));
}

void UpdateCBoatHandlingDataMenu() {
    std::string vehicleName = "Unavailable";
    bool hrActive = false;
    Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED(PLAYER::GET_PLAYER_INDEX()), false);
    RTHE::CHandlingData* currentHandling = GetEditHandlingData(editStock, hrActive, vehicleName);

    menu.Title("CBoatHandlingData");
    menu.Subtitle(vehicleName);

    RTHE::CBaseSubHandlingData* subHandlingData = GetSubHandlingData(currentHandling, RTHE::eHandlingType::HANDLING_TYPE_BOAT);

    if (!subHandlingData) {
        menu.Option("SubHandlingData not found");
        return;
    }

    RTHE::CBoatHandlingData* boatHandlingData = static_cast<RTHE::CBoatHandlingData*>(subHandlingData);

    FloatOptionExtra("fBoxFrontMult",                boatHandlingData->fBoxFrontMult,                -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fBoxRearMult",                 boatHandlingData->fBoxRearMult,                 -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fBoxSideMult",                 boatHandlingData->fBoxSideMult,                 -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fSampleTop",                   boatHandlingData->fSampleTop,                   -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fSampleBottom",                boatHandlingData->fSampleBottom,                -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fSampleBottomTestCorrection",  boatHandlingData->fSampleBottomTestCorrection,  -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fAquaplaneForce",              boatHandlingData->fAquaplaneForce,              -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fAquaplanePushWaterMult",      boatHandlingData->fAquaplanePushWaterMult,      -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fAquaplanePushWaterCap",       boatHandlingData->fAquaplanePushWaterCap,       -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fAquaplanePushWaterApply",     boatHandlingData->fAquaplanePushWaterApply,     -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fRudderForce",                 boatHandlingData->fRudderForce,                 -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fRudderOffsetSubmerge",        boatHandlingData->fRudderOffsetSubmerge,        -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fRudderOffsetForce",           boatHandlingData->fRudderOffsetForce,           -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fRudderOffsetForceZMult",      boatHandlingData->fRudderOffsetForceZMult,      -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fWaveAudioMult",               boatHandlingData->fWaveAudioMult,               -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecMoveResistance.x",          boatHandlingData->vecMoveResistance.x,          -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecMoveResistance.y",          boatHandlingData->vecMoveResistance.y,          -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecMoveResistance.z",          boatHandlingData->vecMoveResistance.z,          -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecTurnResistance.x",          boatHandlingData->vecTurnResistance.x,          -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecTurnResistance.y",          boatHandlingData->vecTurnResistance.y,          -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vecTurnResistance.z",          boatHandlingData->vecTurnResistance.z,          -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fLook_L_R_CamHeight",          boatHandlingData->fLook_L_R_CamHeight,          -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fDragCoefficient",             boatHandlingData->fDragCoefficient,             -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fKeelSphereSize",              boatHandlingData->fKeelSphereSize,              -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fPropRadius",                  boatHandlingData->fPropRadius,                  -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fLowLodAngOffset",             boatHandlingData->fLowLodAngOffset,             -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fLowLodDraughtOffset",         boatHandlingData->fLowLodDraughtOffset,         -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fImpellerOffset",              boatHandlingData->fImpellerOffset,              -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fImpellerForceMult",           boatHandlingData->fImpellerForceMult,           -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fDinghySphereBuoyConst",       boatHandlingData->fDinghySphereBuoyConst,       -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fProwRaiseMult",               boatHandlingData->fProwRaiseMult,               -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fDeepWaterSampleBuoyancyMult", boatHandlingData->fDeepWaterSampleBuoyancyMult, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fTransmissionMultiplier",      boatHandlingData->fTransmissionMultiplier,      -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fTractionMultiplier",          boatHandlingData->fTractionMultiplier,          -1000.0f, 1000.0f, 0.01f);
}

void UpdateCSeaPlaneHandlingDataMenu() {
    std::string vehicleName = "Unavailable";
    bool hrActive = false;
    Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED(PLAYER::GET_PLAYER_INDEX()), false);
    RTHE::CHandlingData* currentHandling = GetEditHandlingData(editStock, hrActive, vehicleName);

    menu.Title("CSeaPlaneHandlingData");
    menu.Subtitle(vehicleName);

    RTHE::CBaseSubHandlingData* subHandlingData = GetSubHandlingData(currentHandling, RTHE::eHandlingType::HANDLING_TYPE_SEAPLANE);

    if (!subHandlingData) {
        menu.Option("SubHandlingData not found");
        return;
    }

    RTHE::CSeaPlaneHandlingData* seaPlaneHandlingData = static_cast<RTHE::CSeaPlaneHandlingData*>(subHandlingData);

    IntOptionExtra("fLeftPontoonComponentId",  seaPlaneHandlingData->fLeftPontoonComponentId,  -1000, 1000, 1);
    IntOptionExtra("fRightPontoonComponentId", seaPlaneHandlingData->fRightPontoonComponentId, -1000, 1000, 1);
    FloatOptionExtra("fPontoonBuoyConst",                      seaPlaneHandlingData->fPontoonBuoyConst,                      -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fPontoonSampleSizeFront",                seaPlaneHandlingData->fPontoonSampleSizeFront,                -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fPontoonSampleSizeMiddle",               seaPlaneHandlingData->fPontoonSampleSizeMiddle,               -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fPontoonSampleSizeRear",                 seaPlaneHandlingData->fPontoonSampleSizeRear,                 -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fPontoonLengthFractionForSamples",       seaPlaneHandlingData->fPontoonLengthFractionForSamples,       -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fPontoonDragCoefficient",                seaPlaneHandlingData->fPontoonDragCoefficient,                -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fPontoonVerticalDampingCoefficientUp",   seaPlaneHandlingData->fPontoonVerticalDampingCoefficientUp,   -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fPontoonVerticalDampingCoefficientDown", seaPlaneHandlingData->fPontoonVerticalDampingCoefficientDown, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fKeelSphereSize",                        seaPlaneHandlingData->fKeelSphereSize,                        -1000.0f, 1000.0f, 0.01f);
}

void UpdateCSubmarineHandlingDataMenu() {
    std::string vehicleName = "Unavailable";
    bool hrActive = false;
    Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED(PLAYER::GET_PLAYER_INDEX()), false);
    RTHE::CHandlingData* currentHandling = GetEditHandlingData(editStock, hrActive, vehicleName);

    menu.Title("CSubmarineHandlingData");
    menu.Subtitle(vehicleName);

    RTHE::CBaseSubHandlingData* subHandlingData = GetSubHandlingData(currentHandling, RTHE::eHandlingType::HANDLING_TYPE_SUBMARINE);

    if (!subHandlingData) {
        menu.Option("SubHandlingData not found");
        return;
    }

    RTHE::CSubmarineHandlingData* submarineHandlingData = static_cast<RTHE::CSubmarineHandlingData*>(subHandlingData);

    FloatOptionExtra("vTurnRes.x",  submarineHandlingData->vTurnRes.x,  -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vTurnRes.y",  submarineHandlingData->vTurnRes.y,  -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("vTurnRes.z",  submarineHandlingData->vTurnRes.z,  -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fMoveResXY",  submarineHandlingData->fMoveResXY,  -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fMoveResZ",   submarineHandlingData->fMoveResZ,   -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fPitchMult",  submarineHandlingData->fPitchMult,  -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fPitchAngle", submarineHandlingData->fPitchAngle, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fYawMult",    submarineHandlingData->fYawMult,    -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fDiveSpeed",  submarineHandlingData->fDiveSpeed,  -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fRollMult",   submarineHandlingData->fRollMult,   -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fRollStab",   submarineHandlingData->fRollStab,   -1000.0f, 1000.0f, 0.01f);
}
void UpdateCTrailerHandlingDataMenu() {
    std::string vehicleName = "Unavailable";
    bool hrActive = false;
    Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED(PLAYER::GET_PLAYER_INDEX()), false);
    RTHE::CHandlingData* currentHandling = GetEditHandlingData(editStock, hrActive, vehicleName);

    menu.Title("CTrailerHandlingData");
    menu.Subtitle(vehicleName);

    RTHE::CBaseSubHandlingData* subHandlingData = GetSubHandlingData(currentHandling, RTHE::eHandlingType::HANDLING_TYPE_TRAILER);

    if (!subHandlingData) {
        menu.Option("SubHandlingData not found");
        return;
    }

    RTHE::CTrailerHandlingData* trailerHandlingData = static_cast<RTHE::CTrailerHandlingData*>(subHandlingData);

    FloatOptionExtra("fAttachLimitPitch",       trailerHandlingData->fAttachLimitPitch,       -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fAttachLimitRoll",        trailerHandlingData->fAttachLimitRoll,        -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fAttachLimitYaw",         trailerHandlingData->fAttachLimitYaw,         -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fUprightSpringConstant",  trailerHandlingData->fUprightSpringConstant,  -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fUprightDampingConstant", trailerHandlingData->fUprightDampingConstant, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fAttachedMaxDistance",    trailerHandlingData->fAttachedMaxDistance,    -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fAttachedMaxPenetration", trailerHandlingData->fAttachedMaxPenetration, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fAttachRaiseZ",           trailerHandlingData->fAttachRaiseZ,           -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fPosConstraintMassRatio", trailerHandlingData->fPosConstraintMassRatio, -1000.0f, 1000.0f, 0.01f);
}

void UpdateCCarHandlingDataMenu() {
    std::string vehicleName = "Unavailable";
    bool hrActive = false;
    Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED(PLAYER::GET_PLAYER_INDEX()), false);
    RTHE::CHandlingData* currentHandling = GetEditHandlingData(editStock, hrActive, vehicleName);

    menu.Title("CCarHandlingData");
    menu.Subtitle(vehicleName);

    RTHE::CBaseSubHandlingData* subHandlingData = GetSubHandlingData(currentHandling, RTHE::eHandlingType::HANDLING_TYPE_CAR);

    if (!subHandlingData) {
        menu.Option("SubHandlingData not found");
        return;
    }

    RTHE::CCarHandlingData* carHandling = static_cast<RTHE::CCarHandlingData*>(subHandlingData);

    FloatOptionExtra("fBackEndPopUpCarImpulseMult", carHandling->fBackEndPopUpCarImpulseMult, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fBackEndPopUpBuildingImpulseMult", carHandling->fBackEndPopUpBuildingImpulseMult, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fBackEndPopUpMaxDeltaSpeed", carHandling->fBackEndPopUpMaxDeltaSpeed, -1000.0f, 1000.0f, 0.01f);

    FloatOptionExtra("fToeFront", carHandling->fToeFront, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fToeRear", carHandling->fToeRear, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fCamberFront", carHandling->fCamberFront, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fCamberRear", carHandling->fCamberRear, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fCastor", carHandling->fCastor, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fEngineResistance", carHandling->fEngineResistance, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fMaxDriveBiasTransfer", carHandling->fMaxDriveBiasTransfer, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fJumpForceScale", carHandling->fJumpForceScale, -1000.0f, 1000.0f, 0.01f);
    FloatOptionExtra("fIncreasedRammingForceScale", carHandling->fIncreasedRammingForceScale, -1000.0f, 1000.0f, 0.01f);

    //{
    //    std::string strUnk_0x038_Flags = fmt::format("{:X}", carHandling->Unk_0x038);
    //    if (menu.Option(fmt::format("Unk_0x038: {}", strUnk_0x038_Flags))) {
    //        std::string newFlags = GetKbEntryStr(strUnk_0x038_Flags);
    //        SetFlags(carHandling->Unk_0x038, newFlags);
    //    }
    //}

    OptionFlags("strAdvancedFlags", Flags::GetAdvancedFlags(), carHandling->strAdvancedFlags, advancedFlagsIndex);

    auto numAdvData = carHandling->pAdvancedData.GetCount();
    if (numAdvData > 0) {
        menu.Option(fmt::format("CAdvancedData found ({})", numAdvData));
    }

    for (uint16_t iAdv = 0; iAdv < carHandling->pAdvancedData.GetCount(); ++iAdv) {
        RTHE::CAdvancedData* advancedData = &carHandling->pAdvancedData.Get(iAdv);

        std::string slotName;
        if (advancedData->Slot >= AdvancedDataSlotIdName.size()) {
            slotName = fmt::format("Index ({}) out of bounds", advancedData->Slot);
        }
        else {
            slotName = AdvancedDataSlotIdName[advancedData->Slot];
        }
        menu.IntOption(fmt::format("[{}] Slot ID", iAdv), advancedData->Slot, 0, 255, 1, {},
            { fmt::format("Slot ID name: {}", slotName) });

        menu.IntOption(fmt::format("[{}] Index", iAdv), advancedData->Index, 0, 255, 1);
        FloatOptionExtra(fmt::format("[{}] Value", iAdv), advancedData->Value, -1000.0f, 1000.0f, 0.1f);
    }
}

void UpdateLoadMenu() {
    menu.Title("Load Handling");

    Ped playerPed = PLAYER::GET_PLAYER_PED(PLAYER::GET_PLAYER_INDEX());
    Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);

    if (!ENTITY::DOES_ENTITY_EXIST(vehicle)) {
        menu.Subtitle("Unavailable");
        menu.Option("Unavailable", { "Only available while in a vehicle." });
        return;
    }

    std::string vehicleNameLabel = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(ENTITY::GET_ENTITY_MODEL(vehicle));
    std::string vehicleName = HUD::_GET_LABEL_TEXT(vehicleNameLabel.c_str());
    menu.Subtitle(fmt::format("{}", vehicleName));

    // TODO: Filter option

    for (const auto& handlingDataItem : g_handlingDataItems) {
        bool selected = false;
        if (menu.OptionPlus(handlingDataItem.handlingName, {}, &selected).Triggered) {
            setHandling(vehicle, handlingDataItem);
            UI::Notify(fmt::format("Applied {} handling", handlingDataItem.handlingName));
        }
        if (selected) {
            float maxKph = handlingDataItem.fInitialDriveMaxFlatVel / 0.75f;
            float maxMph = maxKph / 1.609344f;
            std::vector<std::string> extra{
                fmt::format("File: {}", handlingDataItem.metaData.fileName),
                fmt::format("Description: {}", handlingDataItem.metaData.description),
                fmt::format("Top speed: {:.2f} kph / {:.2f} mph", maxKph, maxMph),
                fmt::format("Drive bias (front): {:.2f}", handlingDataItem.fDriveBiasFront),
                fmt::format("Traction: max {:.2f}, min {:.2f}", handlingDataItem.fTractionCurveMax, handlingDataItem.fTractionCurveMin),
            };
            menu.OptionPlusPlus(extra, "Handling overview");
        }
    }
}

void UpdateMenu() {
    menu.CheckKeys();

    // MainMenu
    if (menu.CurrentMenu("mainmenu")) { UpdateMainMenu(); }

    // MainMenu -> EditHandlingMenu
    if (menu.CurrentMenu("EditMenu")) { UpdateEditMenu(); }

    // MainMenu -> EditHandlingMenu -> EditCBikeHandlingDataMenu
    if (menu.CurrentMenu("EditCBikeHandlingDataMenu")) { UpdateCBikeHandlingDataMenu(); }

    // MainMenu -> EditHandlingMenu -> EditCFlyingHandlingDataMenu
    if (menu.CurrentMenu("EditCFlyingHandlingDataMenu")) { UpdateCFlyingHandlingDataMenu(); }

    // MainMenu -> EditHandlingMenu -> EditCSpecialFlightHandlingDataMenu
    if (menu.CurrentMenu("EditCSpecialFlightHandlingDataMenu")) { UpdateCSpecialFlightHandlingDataMenu(); }

    // MainMenu -> EditHandlingMenu -> EditCBoatHandlingDataMenu
    if (menu.CurrentMenu("EditCBoatHandlingDataMenu")) { UpdateCBoatHandlingDataMenu(); }

    // MainMenu -> EditHandlingMenu -> EditCSeaPlaneHandlingDataMenu
    if (menu.CurrentMenu("EditCSeaPlaneHandlingDataMenu")) { UpdateCSeaPlaneHandlingDataMenu(); }

    // MainMenu -> EditHandlingMenu -> EditCSubmarineHandlingDataMenu
    if (menu.CurrentMenu("EditCSubmarineHandlingDataMenu")) { UpdateCSubmarineHandlingDataMenu(); }

    // MainMenu -> EditHandlingMenu -> EditCTrailerHandlingDataMenu
    if (menu.CurrentMenu("EditCTrailerHandlingDataMenu")) { UpdateCTrailerHandlingDataMenu(); }

    // MainMenu -> EditHandlingMenu -> EditCCarHandlingDataMenu
    if (menu.CurrentMenu("EditCCarHandlingDataMenu")) { UpdateCCarHandlingDataMenu(); }

    // MainMenu -> LoadHandlingMenu
    if (menu.CurrentMenu("LoadMenu")) { UpdateLoadMenu(); }

    menu.EndMenu();
}
