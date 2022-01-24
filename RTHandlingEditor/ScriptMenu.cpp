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
#include "AdvancedDataNames.h"

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
        if (menu.Option("Respawn vehicle")) {
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

        if (menu.Option("Save")) {
            RTHE::CHandlingData* currentHandling = reinterpret_cast<RTHE::CHandlingData*>(VExt::GetHandlingPtr(vehicle));

            PromptSave(vehicle, currentHandling->NameHash);
        }
    }
    else {
        menu.Option("Unavailable", { "Only available while in a vehicle." });
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

                    if (word == "MF" || word == "HF" || word == "DF" || word == "AF")
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
        "", { "Move selection with Left and Right. Space (KB) or RB (Controller) to toggle. Enter to edit flags string." })) {
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

        if (NativeMenu::IsKeyJustUp(VK_SPACE, true) ||
            PAD::IS_CONTROL_JUST_RELEASED(0, eControl::ControlFrontendRb)) {
            flags ^= 1u << flagIndex;
        }
    }
}

void UpdateEditMenu() {
    menu.Title("Edit Handling");

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

    RTHE::CHandlingData* currentHandling = nullptr;

    uint64_t addr = VExt::GetHandlingPtr(vehicle);
    currentHandling = reinterpret_cast<RTHE::CHandlingData*>(addr);

    if (currentHandling == nullptr) {
        menu.Option("Could not find handling pointer!");
        return;
    }

    void* handlingDataOrig = nullptr;
    bool customHandling = false;
    bool hrActive = false;
    {
        void* handlingDataReplace = nullptr;

        if (HR_GetHandlingData(vehicle, &handlingDataOrig, &handlingDataReplace)) {
            customHandling = true;
            hrActive = true;
        }
    }

    if (hrActive) {
        menu.BoolOption("Edit original?", editStock,
            { "HandlingReplacement is active. Check box to edit original data.",
              "If active, new vehicle instances spawn with the applied changes, but the current instance does not update.",
              "If not active, new vehicle instances spawn with the unedited original data.",
              "Recommended to enable if HandlingReplacement is active. Remember to respawn after edits!"});
    }

    if (customHandling && editStock && handlingDataOrig != nullptr) {
        currentHandling = reinterpret_cast<RTHE::CHandlingData*>(handlingDataOrig);
    }

    Utils::DrawCOMAndRollCenters(vehicle, currentHandling);

    menu.FloatOptionCb("fMass", currentHandling->fMass, 0.0f, 1000000000.0f, 5.0f, GetKbEntry);

    {
        float fInitialDragCoeff = currentHandling->fInitialDragCoeff * 10000.0f;
        if (menu.FloatOptionCb("fInitialDragCoeff", fInitialDragCoeff, 0.0f, 100.0f, 0.05f, GetKbEntry)) {
            currentHandling->fInitialDragCoeff = fInitialDragCoeff / 10000.0f;
        }
    }
    menu.FloatOptionCb("fDownforceModifier", currentHandling->fDownforceModifier, 0.0f, 1000.0f, 0.5f, GetKbEntry);

    if (menu.FloatOptionCb("fPercentSubmerged", currentHandling->fPercentSubmerged, 0.0f, 100.0f, 1.0f, GetKbEntry)) {
        currentHandling->fSubmergedRatio_ = 100.0f / currentHandling->fPercentSubmerged;
    }

    bool comModified = false;
    comModified |= menu.FloatOptionCb("vecCentreOfMassOffset.x", currentHandling->vecCentreOfMassOffset.x, -20.0f, 20.0f, 0.01f, GetKbEntry);
    comModified |= menu.FloatOptionCb("vecCentreOfMassOffset.y", currentHandling->vecCentreOfMassOffset.y, -20.0f, 20.0f, 0.01f, GetKbEntry);
    comModified |= menu.FloatOptionCb("vecCentreOfMassOffset.z", currentHandling->vecCentreOfMassOffset.z, -20.0f, 20.0f, 0.01f, GetKbEntry);

    if (comModified) {
        // Update COM.
        PHYSICS::SET_CGOFFSET(vehicle,
            currentHandling->vecCentreOfMassOffset.x,
            currentHandling->vecCentreOfMassOffset.y,
            currentHandling->vecCentreOfMassOffset.z);
    }

    menu.FloatOptionCb("vecInertiaMultiplier.x", currentHandling->vecInertiaMultiplier.x, -100.0f, 100.0f, 0.1f, GetKbEntry);
    menu.FloatOptionCb("vecInertiaMultiplier.y", currentHandling->vecInertiaMultiplier.y, -100.0f, 100.0f, 0.1f, GetKbEntry);
    menu.FloatOptionCb("vecInertiaMultiplier.z", currentHandling->vecInertiaMultiplier.z, -100.0f, 100.0f, 0.1f, GetKbEntry);

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

        if (menu.FloatOptionCb("fDriveBiasFront", fDriveBiasFrontNorm, 0.0f, 1.0f, 0.01f, GetKbEntry)) {
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

    //menu.FloatOptionCb("fAcceleration", currentHandling->fAcceleration, -1000.0f, 1000.0f, 0.01f);

    menu.UInt8Option("nInitialDriveGears", currentHandling->nInitialDriveGears, 1u, 10u, 1u);
    menu.FloatOptionCb("fInitialDriveForce", currentHandling->fInitialDriveForce, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
    menu.FloatOptionCb("fDriveInertia", currentHandling->fDriveInertia, -1000.0f, 1000.0f, 0.01f, GetKbEntry);

    menu.FloatOptionCb("fClutchChangeRateScaleUpShift", currentHandling->fClutchChangeRateScaleUpShift, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
    menu.FloatOptionCb("fClutchChangeRateScaleDownShift", currentHandling->fClutchChangeRateScaleDownShift, -1000.0f, 1000.0f, 0.01f, GetKbEntry);

    {
        float fInitialDriveMaxFlatVel = currentHandling->fInitialDriveMaxFlatVel_ * 3.6f;
        if (menu.FloatOptionCb("fInitialDriveMaxFlatVel", fInitialDriveMaxFlatVel, 0.0f, 1000.0f, 0.5f, GetKbEntry)) {
            currentHandling->fInitialDriveMaxFlatVel_ = fInitialDriveMaxFlatVel / 3.6f;
            currentHandling->fDriveMaxFlatVel_ = fInitialDriveMaxFlatVel / 3.0f;
        }
    }

    menu.FloatOptionCb("fBrakeForce", currentHandling->fBrakeForce, -1000.0f, 1000.0f, 0.01f, GetKbEntry);

    {
        float fBrakeBiasFront = currentHandling->fBrakeBiasFront_ / 2.0f;
        if (menu.FloatOptionCb("fBrakeBiasFront", fBrakeBiasFront, -1000.0f, 1000.0f, 0.01f, GetKbEntry)) {
            currentHandling->fBrakeBiasFront_ = fBrakeBiasFront * 2.0f;
            currentHandling->fBrakeBiasRear_ = 2.0f * (1.0f - fBrakeBiasFront);
        }
    }
    
    menu.FloatOptionCb("fHandBrakeForce", currentHandling->fHandBrakeForce2, -1000.0f, 1000.0f, 0.01f, GetKbEntry);

    {
        // rad 2 deg
        float fSteeringLock = currentHandling->fSteeringLock_ / 0.017453292f;
        if (menu.FloatOptionCb("fSteeringLock", fSteeringLock, 0.0f, 90.0f, 0.10f, GetKbEntry)) {
            currentHandling->fSteeringLock_ = fSteeringLock * 0.017453292f;
            currentHandling->fSteeringLockRatio_ = 1.0f / (fSteeringLock * 0.017453292f);
        }
    }

    bool tcModified = false;
    {
        float fTractionCurveMax = currentHandling->fTractionCurveMax;
        if (menu.FloatOptionCb("fTractionCurveMax", fTractionCurveMax, 0.0f, 1000.0f, 0.01f, GetKbEntry)) {
            currentHandling->fTractionCurveMax = fTractionCurveMax;
            currentHandling->fTractionCurveMaxRatio_ = 1.0f / fTractionCurveMax;
            tcModified = true;
        }
    }

    {
        float fTractionCurveMin = currentHandling->fTractionCurveMin;
        if (menu.FloatOptionCb("fTractionCurveMin", fTractionCurveMin, 0.0f, 1000.0f, 0.01f, GetKbEntry)) {
            currentHandling->fTractionCurveMin = fTractionCurveMin;
            tcModified = true;
        }
    }

    if (tcModified) {
        currentHandling->fTractionCurveRatio_ = 1.0f / (currentHandling->fTractionCurveMax - currentHandling->fTractionCurveMin);
    }

    {
        // rad 2 deg
        float fTractionCurveLateral = currentHandling->fTractionCurveLateral_ / 0.017453292f;
        if (menu.FloatOptionCb("fTractionCurveLateral", fTractionCurveLateral, -1000.0f, 1000.0f, 0.01f, GetKbEntry)) {
            currentHandling->fTractionCurveLateral_ = fTractionCurveLateral * 0.017453292f;
            currentHandling->fTractionCurveLateralRatio_ = 1.0f / (fTractionCurveLateral * 0.017453292f);
        }
    }

    {
        float fTractionSpringDeltaMax = currentHandling->fTractionSpringDeltaMax;
        if (menu.FloatOptionCb("fTractionSpringDeltaMax", fTractionSpringDeltaMax, -1000.0f, 1000.0f, 0.01f, GetKbEntry)) {
            currentHandling->fTractionSpringDeltaMax = fTractionSpringDeltaMax;
            currentHandling->fTractionSpringDeltaMaxRatio_ = 1.0f / fTractionSpringDeltaMax;
        }
    }

    menu.FloatOptionCb("fLowSpeedTractionLossMult", currentHandling->fLowSpeedTractionLossMult, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
    menu.FloatOptionCb("fCamberStiffnesss", currentHandling->fCamberStiffness, -1000.0f, 1000.0f, 0.01f, GetKbEntry);

    { // todo: ???
        float fTractionBiasFront = currentHandling->fTractionBiasFront_ / 2.0f;
        if (menu.FloatOptionCb("fTractionBiasFront", fTractionBiasFront, 0.0f, 1.0f, 0.01f, GetKbEntry)) {
            currentHandling->fTractionBiasFront_ = 2.0f * fTractionBiasFront;
            currentHandling->fTractionBiasRear = 2.0f * (1.0f - (fTractionBiasFront));
        }
    }

    menu.FloatOptionCb("fTractionLossMult", currentHandling->fTractionLossMult, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
    menu.FloatOptionCb("fSuspensionForce", currentHandling->fSuspensionForce, -1000.0f, 1000.0f, 0.01f, GetKbEntry);

    {
        float fSuspensionCompDamp = currentHandling->fSuspensionCompDamp * 10.0f;
        if (menu.FloatOptionCb("fSuspensionCompDamp", fSuspensionCompDamp, -1000.0f, 1000.0f, 0.01f, GetKbEntry)) {
            currentHandling->fSuspensionCompDamp = fSuspensionCompDamp / 10.0f;
        }
    }

    {
        float fSuspensionReboundDamp = currentHandling->fSuspensionReboundDamp * 10.0f;
        if (menu.FloatOptionCb("fSuspensionReboundDamp", fSuspensionReboundDamp, -1000.0f, 1000.0f, 0.01f, GetKbEntry)) {
            currentHandling->fSuspensionReboundDamp = fSuspensionReboundDamp / 10.0f;
        }
    }

    menu.FloatOptionCb("fSuspensionUpperLimit", currentHandling->fSuspensionUpperLimit, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
    menu.FloatOptionCb("fSuspensionLowerLimit", currentHandling->fSuspensionLowerLimit, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
    menu.FloatOptionCb("fSuspensionRaise_", currentHandling->fSuspensionRaise_, -1000.0f, 1000.0f, 0.01f, GetKbEntry);

    {
        float fSuspensionBiasFront = currentHandling->fSuspensionBiasFront_ / 2.0f;
        if (menu.FloatOptionCb("fSuspensionBiasFront_", fSuspensionBiasFront, -1000.0f, 1000.0f, 0.01f, GetKbEntry)) {
            currentHandling->fSuspensionBiasFront_ = fSuspensionBiasFront * 2.0f;
            currentHandling->fSuspensionBiasRear_ = 2.0f * (1.0f - (fSuspensionBiasFront));
        }
    }

    menu.FloatOptionCb("fAntiRollBarForce", currentHandling->fAntiRollBarForce, -1000.0f, 1000.0f, 0.01f, GetKbEntry);

    {
        float fAntiRollBarBiasFront = currentHandling->fAntiRollBarBiasFront_ / 2.0f;
        if (menu.FloatOptionCb("fAntiRollBarBiasFront_", fAntiRollBarBiasFront, -1000.0f, 1000.0f, 0.01f, GetKbEntry)) {
            currentHandling->fAntiRollBarBiasFront_ = fAntiRollBarBiasFront * 2.0f;
            currentHandling->fAntiRollBarBiasRear_ = 2.0f * (1.0f - (fAntiRollBarBiasFront));
        }
    }

    menu.FloatOptionCb("fRollCentreHeightFront", currentHandling->fRollCentreHeightFront, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
    menu.FloatOptionCb("fRollCentreHeightRear", currentHandling->fRollCentreHeightRear, -1000.0f, 1000.0f, 0.01f, GetKbEntry);

    menu.FloatOptionCb("fCollisionDamageMult", currentHandling->fCollisionDamageMult, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
    menu.FloatOptionCb("fWeaponDamageMult", currentHandling->fWeaponDamageMult, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
    menu.FloatOptionCb("fDeformationDamageMult", currentHandling->fDeformationDamageMult, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
    menu.FloatOptionCb("fEngineDamageMult", currentHandling->fEngineDamageMult, -1000.0f, 1000.0f, 0.01f, GetKbEntry);

    menu.FloatOptionCb("fPetrolTankVolume", currentHandling->fPetrolTankVolume, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
    menu.FloatOptionCb("fOilVolume", currentHandling->fOilVolume, -1000.0f, 1000.0f, 0.01f, GetKbEntry);

    menu.FloatOptionCb("vecSeatOffsetDistX", currentHandling->vecSeatOffsetDist.x, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
    menu.FloatOptionCb("vecSeatOffsetDistY", currentHandling->vecSeatOffsetDist.y, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
    menu.FloatOptionCb("vecSeatOffsetDistZ", currentHandling->vecSeatOffsetDist.z, -1000.0f, 1000.0f, 0.01f, GetKbEntry);

    menu.IntOption("nMonetaryValue", currentHandling->nMonetaryValue, 0, 1000000, 1);

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
        if (menu.Option(fmt::format("AIHandling: {}", AIHandling), 
            {
                fmt::format("AVERAGE: {:X}", StrUtil::joaat("AVERAGE")),
                fmt::format("SPORTS_CAR: {:X}", StrUtil::joaat("SPORTS_CAR")),
                fmt::format("TRUCK: {:X}", StrUtil::joaat("TRUCK")),
                fmt::format("CRAP: {:X}", StrUtil::joaat("CRAP")),
            })) {
        }
    }

    if (1) {
        for(uint16_t idx = 0; idx < currentHandling->m_subHandlingData.GetCount(); ++idx) {
            RTHE::CBaseSubHandlingData* subHandlingData = currentHandling->m_subHandlingData.Get(idx);

            if (subHandlingData) {
                menu.Option("SubHandlingData found",
                    { "Only CCarHandlingData supported currently." });

                auto type = subHandlingData->GetHandlingType();
                menu.Option(fmt::format("SHD[{}] {}", idx, type));

                if (type == RTHE::HANDLING_TYPE_CAR) {
                    RTHE::CCarHandlingData* carHandling = static_cast<RTHE::CCarHandlingData*>(subHandlingData);
                    menu.FloatOptionCb("fBackEndPopUpCarImpulseMult", carHandling->fBackEndPopUpCarImpulseMult, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
                    menu.FloatOptionCb("fBackEndPopUpBuildingImpulseMult", carHandling->fBackEndPopUpBuildingImpulseMult, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
                    menu.FloatOptionCb("fBackEndPopUpMaxDeltaSpeed", carHandling->fBackEndPopUpMaxDeltaSpeed, -1000.0f, 1000.0f, 0.01f, GetKbEntry);

                    menu.FloatOptionCb("fToeFront", carHandling->fToeFront, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
                    menu.FloatOptionCb("fToeRear", carHandling->fToeRear, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
                    menu.FloatOptionCb("fCamberFront", carHandling->fCamberFront, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
                    menu.FloatOptionCb("fCamberRear", carHandling->fCamberRear, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
                    menu.FloatOptionCb("fCastor", carHandling->fCastor, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
                    menu.FloatOptionCb("fEngineResistance", carHandling->fEngineResistance, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
                    menu.FloatOptionCb("fMaxDriveBiasTransfer", carHandling->fMaxDriveBiasTransfer, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
                    menu.FloatOptionCb("fJumpForceScale", carHandling->fJumpForceScale, -1000.0f, 1000.0f, 0.01f, GetKbEntry);
                    //menu.FloatOptionCb("fUnk_0x034", carHandling->fUnk_0x034, -1000.0f, 1000.0f, 0.01f, GetKbEntry);

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
                            slotName = fmt::format("Index ({}) out of bounds",advancedData->Slot);
                        }
                        else {
                            slotName = AdvancedDataSlotIdName[advancedData->Slot];
                        }
                        menu.IntOption(fmt::format("[{}] Slot ID", iAdv), advancedData->Slot, 0, 255, 1,
                            { fmt::format("Slot ID name: {}", slotName) });
                        
                        menu.IntOption(fmt::format("[{}] Index", iAdv), advancedData->Index, 0, 255);
                        menu.FloatOptionCb(fmt::format("[{}] Value", iAdv), advancedData->Value, -1000.0f, 1000.0f, 0.1f, GetKbEntry);
                     }
                }
            }
        }
    }
}

bool onlyCurrent = true;

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
        if (menu.OptionPlus(handlingDataItem.handlingName, {}, &selected)) {
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

    // MainMenu -> LoadHandlingMenu
    if (menu.CurrentMenu("LoadMenu")) { UpdateLoadMenu(); }

    menu.EndMenu();
}
