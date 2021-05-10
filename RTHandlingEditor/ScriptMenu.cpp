#include "script.h"

#include "Memory/HandlingInfo.h"
#include "Memory/VehicleExtensions.hpp"
#include "ScriptUtils.h"
#include "HandlingItem.h"
#include "Util/StrUtil.h"
#include "Util/UI.h"
#include "Constants.h"
#include "Table.h"

#include <HandlingReplacement.h>
#include <menu.h>
#include <inc/natives.h>
#include <fmt/format.h>

extern VehicleExtensions g_ext;
extern std::vector<RTHE::CHandlingDataItem> g_handlingDataItems;

void setHandling(Vehicle vehicle, const RTHE::CHandlingDataItem& handlingDataItem);
void PromptSave(Vehicle vehicle);

namespace {
NativeMenu::Menu menu;
bool editStock = true;
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
        }
    }
    
    menu.MenuOption("Load handling", "LoadMenu");

    if (menu.Option("Save")) {
        PromptSave(vehicle);
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

void UpdateEditMenu() {
    menu.Title("Edit Handling");

    Ped playerPed = PLAYER::GET_PLAYER_PED(PLAYER::GET_PLAYER_INDEX());
    Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);

    if (!ENTITY::DOES_ENTITY_EXIST(vehicle)) {
        menu.Subtitle("Not in a vehicle");
        menu.Option("Not in a vehicle");
        return;
    }

    std::string vehicleNameLabel = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(ENTITY::GET_ENTITY_MODEL(vehicle));
    std::string vehicleName = HUD::_GET_LABEL_TEXT(vehicleNameLabel.c_str());
    menu.Subtitle(fmt::format("{}", vehicleName));

    RTHE::CHandlingData* currentHandling = nullptr;

    uint64_t addr = g_ext.GetHandlingPtr(vehicle);
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

    menu.FloatOptionCb("vecCentreOfMassOffset.x", currentHandling->vecCentreOfMassOffset.x, -20.0f, 20.0f, 0.01f, GetKbEntry);
    menu.FloatOptionCb("vecCentreOfMassOffset.y", currentHandling->vecCentreOfMassOffset.y, -20.0f, 20.0f, 0.01f, GetKbEntry);
    menu.FloatOptionCb("vecCentreOfMassOffset.z", currentHandling->vecCentreOfMassOffset.z, -20.0f, 20.0f, 0.01f, GetKbEntry);

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

    {
        bool show = false;
        std::string strModelFlags = fmt::format("{:X}", currentHandling->strModelFlags);
        if (menu.OptionPlus(fmt::format("strModelFlags: {}", strModelFlags), {}, &show, nullptr, nullptr, "", {})) {
            std::string newFlags = GetKbEntryStr(strModelFlags);
            SetFlags(currentHandling->strModelFlags, newFlags);
        }

        std::vector<std::string> extra;
        if (show) {
            extra.push_back("See also https://eddlm.github.io/Handling-Tools/flags");
            menu.OptionPlusPlus(extra, "Flags");

            auto& f = currentHandling->strModelFlags;

            STable modelFlagsTable{{
                {SCell{"IS_VAN",            (f & 0x00000001)>0 }, SCell{"IS_BUS",         (f & 0x00000002)>0}, SCell{"IS_LOW",             (f & 0x00000004)>0}, SCell{"IS_BIG",                    (f & 0x00000008)>0}},
                {SCell{"ABS_STD",           (f & 0x00000010)>0 }, SCell{"ABS_OPTION",     (f & 0x00000020)>0}, SCell{"ABS_ALT_STD",        (f & 0x00000040)>0}, SCell{"ABS_ALT_OPTION",            (f & 0x00000080)>0}},
                {SCell{"NO_DOORS",          (f & 0x00000100)>0 }, SCell{"TANDEM_SEATS",   (f & 0x00000200)>0}, SCell{"SIT_IN_BOAT",        (f & 0x00000400)>0}, SCell{"HAS_TRACKS",                (f & 0x00000800)>0}},
                {SCell{"NO_EXHAUST",        (f & 0x00001000)>0 }, SCell{"DOUBLE_EXHAUST", (f & 0x00002000)>0}, SCell{"NO1FPS_LOOK_BEHIND", (f & 0x00004000)>0}, SCell{"CAN_ENTER_IF_NO_DOOR",      (f & 0x00008000)>0}},
                {SCell{"AXLE_F_TORSION",    (f & 0x00010000)>0 }, SCell{"AXLE_F_SOLID",   (f & 0x00020000)>0}, SCell{"AXLE_F_MCPHERSON",   (f & 0x00040000)>0}, SCell{"ATTACH_PED_TO_BODYSHELL",   (f & 0x00080000)>0}},
                {SCell{"AXLE_R_TORSION",    (f & 0x00100000)>0 }, SCell{"AXLE_R_SOLID",   (f & 0x00200000)>0}, SCell{"AXLE_R_MCPHERSON",   (f & 0x00400000)>0}, SCell{"DONT_FORCE_GRND_CLEARANCE", (f & 0x00800000)>0}},
                {SCell{"DONT_RENDER_STEER", (f & 0x01000000)>0 }, SCell{"NO_WHEEL_BURST", (f & 0x02000000)>0}, SCell{"INDESTRUCTIBLE",     (f & 0x04000000)>0}, SCell{"DOUBLE_FRONT_WHEELS",       (f & 0x08000000)>0}},
                {SCell{"RC",                (f & 0x10000000)>0 }, SCell{"DOUBLE_RWHEELS", (f & 0x20000000)>0}, SCell{"MF_NO_WHEEL_BREAK",  (f & 0x40000000)>0}, SCell{"IS_HATCHBACK",              (f & 0x80000000)>0}},
            }};

            const uint32_t tableRows = 8;
            const uint32_t rowCells = 4;

            const UI::SColor enabled =  {   0, 255,   0, 255 };
            const UI::SColor disabled = { 127, 127, 127, 255 };

            const float cellWidth  = 0.100f;
            const float cellHeight = 0.025f;

            for (uint32_t row = 0; row < tableRows; ++row) {
                for (uint32_t cell = 0; cell < rowCells; ++cell) {
                    auto color = modelFlagsTable.Cells[row][cell].Enable ? enabled : disabled;
                    UI::ShowText(0.30f + cell * cellWidth, 0.30f + row * cellHeight, 0.25f,
                        modelFlagsTable.Cells[row][cell].Contents,
                        color);
                }
            }

            GRAPHICS::DRAW_RECT(0.5f, 0.4f, cellWidth * rowCells, cellHeight * tableRows, 0, 0, 0, 91, false);
        }
    }

    {
        bool show = false;
        std::string strHandlingFlags = fmt::format("{:X}", currentHandling->strHandlingFlags);
        if (menu.OptionPlus(fmt::format("strHandlingFlags: {}", strHandlingFlags), {}, &show, nullptr, nullptr, "", {})) {
            std::string newFlags = GetKbEntryStr(strHandlingFlags);
            SetFlags(currentHandling->strHandlingFlags, newFlags);
        }

        std::vector<std::string> extra;
        if (show) {
            extra.push_back("See also https://eddlm.github.io/Handling-Tools/flags");
            menu.OptionPlusPlus(extra, "Flags");

            auto& f = currentHandling->strHandlingFlags;

            STable modelFlagsTable{ {
                {SCell{"SMOOTH_COMPRESN",        (f & 0x00000001) > 0 }, SCell{"REDUCED_MOD_MASS",         (f & 0x00000002) > 0}, SCell{"?",                                    (f & 0x00000004) > 0}, SCell{"_INVERT_GRIP",      (f & 0x00000008) > 0}},
                {SCell{"NO_HANDBRAKE",           (f & 0x00000010) > 0 }, SCell{"STEER_REARWHEELS",         (f & 0x00000020) > 0}, SCell{"HB_REARWHEEL_STEER",                   (f & 0x00000040) > 0}, SCell{"STEER_ALL_WHEELS",  (f & 0x00000080) > 0}},
                {SCell{"FREEWHEEL_NO_GAS",       (f & 0x00000100) > 0 }, SCell{"NO_REVERSE",               (f & 0x00000200) > 0}, SCell{"?",                                    (f & 0x00000400) > 0}, SCell{"STEER_NO_WHEELS",   (f & 0x00000800) > 0}},
                {SCell{"CVT",                    (f & 0x00001000) > 0 }, SCell{"ALT_EXT_WHEEL_BOUNDS_BEH", (f & 0x00002000) > 0}, SCell{"DONT_RAISE_BOUNDS_AT_SPEED",           (f & 0x00004000) > 0}, SCell{"?",                 (f & 0x00008000) > 0}},
                {SCell{"LESS_SNOW_SINK",         (f & 0x00010000) > 0 }, SCell{"TYRES_CAN_CLI",            (f & 0x00020000) > 0}, SCell{"?",                                    (f & 0x00040000) > 0}, SCell{"?",                 (f & 0x00080000) > 0}},
                {SCell{"OFFROAD_ABILITY",        (f & 0x00100000) > 0 }, SCell{"OFFROAD_ABILITY2",         (f & 0x00200000) > 0}, SCell{"HF_TYRES_RAISE_SIDE_IMPACT_THRESHOLD", (f & 0x00400000) > 0}, SCell{"_INCREASE_GRAVITY", (f & 0x00800000) > 0}},
                {SCell{"ENABLE_LEAN",            (f & 0x01000000) > 0 }, SCell{"?",                        (f & 0x02000000) > 0}, SCell{"HEAVYARMOUR",                          (f & 0x04000000) > 0}, SCell{"ARMOURED",          (f & 0x08000000) > 0}},
                {SCell{"SELF_RIGHTING_IN_WATER", (f & 0x10000000) > 0 }, SCell{"IMPROVED_RIGHTING_FORCE",  (f & 0x20000000) > 0}, SCell{"?",                                    (f & 0x40000000) > 0}, SCell{"?",                 (f & 0x80000000) > 0}},
            } };

            const uint32_t tableRows = 8;
            const uint32_t rowCells = 4;

            const UI::SColor enabled = { 0, 255,   0, 255 };
            const UI::SColor disabled = { 127, 127, 127, 255 };

            const float cellWidth = 0.100f;
            const float cellHeight = 0.025f;

            for (uint32_t row = 0; row < tableRows; ++row) {
                for (uint32_t cell = 0; cell < rowCells; ++cell) {
                    auto color = modelFlagsTable.Cells[row][cell].Enable ? enabled : disabled;
                    UI::ShowText(0.30f + cell * cellWidth, 0.30f + row * cellHeight, 0.25f,
                        modelFlagsTable.Cells[row][cell].Contents,
                        color);
                }
            }

            GRAPHICS::DRAW_RECT(0.5f, 0.4f, cellWidth * rowCells, cellHeight * tableRows, 0, 0, 0, 91, false);
        }
    }

    {
        std::string strDamageFlags = fmt::format("{:X}", currentHandling->strDamageFlags);
        if (menu.Option(fmt::format("strDamageFlags: {}", strDamageFlags))) {
            std::string newFlags = GetKbEntryStr(strDamageFlags);
            SetFlags(currentHandling->strDamageFlags, newFlags);
        }
    }

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

    {
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
                    menu.FloatOptionCb("fUnk_0x034", carHandling->fUnk_0x034, -1000.0f, 1000.0f, 0.01f, GetKbEntry);

                    {
                        std::string strUnk_0x038_Flags = fmt::format("{:X}", carHandling->Unk_0x038);
                        if (menu.Option(fmt::format("Unk_0x038: {}", strUnk_0x038_Flags))) {
                            std::string newFlags = GetKbEntryStr(strUnk_0x038_Flags);
                            SetFlags(carHandling->Unk_0x038, newFlags);
                        }
                    }

                    {
                        bool show = false;
                        std::string strAdvancedFlags = fmt::format("{:X}", carHandling->strAdvancedFlags);
                        if (menu.OptionPlus(fmt::format("strAdvancedFlags: {}", strAdvancedFlags), {}, &show, nullptr, nullptr, "", {})) {
                            std::string newFlags = GetKbEntryStr(strAdvancedFlags);
                            SetFlags(carHandling->strAdvancedFlags, newFlags);
                        }

                        std::vector<std::string> extra;
                        if (show) {
                            extra.push_back("See also https://eddlm.github.io/Handling-Tools/flags");
                            menu.OptionPlusPlus(extra, "Flags");

                            auto& f = currentHandling->strHandlingFlags;

                            STable modelFlagsTable{ {
                                {SCell{"?",                         (f & 0x00000001) > 0 }, SCell{"?",                 (f & 0x00000002) > 0}, SCell{"?",                         (f & 0x00000004) > 0}, SCell{"?",                                 (f & 0x00000008) > 0}},
                                {SCell{"?",                         (f & 0x00000010) > 0 }, SCell{"?",                 (f & 0x00000020) > 0}, SCell{"?",                         (f & 0x00000040) > 0}, SCell{"?",                                 (f & 0x00000080) > 0}},
                                {SCell{"?",                         (f & 0x00000100) > 0 }, SCell{"?",                 (f & 0x00000200) > 0}, SCell{"?",                         (f & 0x00000400) > 0}, SCell{"?",                                 (f & 0x00000800) > 0}},
                                {SCell{"?",                         (f & 0x00001000) > 0 }, SCell{"?",                 (f & 0x00002000) > 0}, SCell{"?",                         (f & 0x00004000) > 0}, SCell{"?",                                 (f & 0x00008000) > 0}},
                                {SCell{"_TRACTION_CONTROL",         (f & 0x00010000) > 0 }, SCell{"_HOLD_GEAR_LONGER", (f & 0x00020000) > 0}, SCell{"_STIFFER_SPRING_SPEED",     (f & 0x00040000) > 0}, SCell{"_FAKE_WHEELSPIN",                   (f & 0x00080000) > 0}},
                                {SCell{"?",                         (f & 0x00100000) > 0 }, SCell{"?",                 (f & 0x00200000) > 0}, SCell{"_SMOOTH_REV_1ST",           (f & 0x00400000) > 0}, SCell{"?",                                 (f & 0x00800000) > 0}},
                                {SCell{"?",                         (f & 0x01000000) > 0 }, SCell{"?",                 (f & 0x02000000) > 0}, SCell{"_IGNORE_TUNED_WHEELS_CLIP", (f & 0x04000000) > 0}, SCell{"_OPENWHEEL_DOWNFORCE_NO_CURBBOOST", (f & 0x08000000) > 0}},
                                {SCell{"_DECREASE_BODYROLL_TUNING", (f & 0x10000000) > 0 }, SCell{"?",                 (f & 0x20000000) > 0}, SCell{"?",                         (f & 0x40000000) > 0}, SCell{"?",                                 (f & 0x80000000) > 0}},
                            } };

                            const uint32_t tableRows = 8;
                            const uint32_t rowCells = 4;

                            const UI::SColor enabled = { 0, 255,   0, 255 };
                            const UI::SColor disabled = { 127, 127, 127, 255 };

                            const float cellWidth = 0.100f;
                            const float cellHeight = 0.025f;

                            for (uint32_t row = 0; row < tableRows; ++row) {
                                for (uint32_t cell = 0; cell < rowCells; ++cell) {
                                    auto color = modelFlagsTable.Cells[row][cell].Enable ? enabled : disabled;
                                    UI::ShowText(0.30f + cell * cellWidth, 0.30f + row * cellHeight, 0.25f,
                                        modelFlagsTable.Cells[row][cell].Contents,
                                        color);
                                }
                            }

                            GRAPHICS::DRAW_RECT(0.5f, 0.4f, cellWidth * rowCells, cellHeight * tableRows, 0, 0, 0, 91, false);
                        }
                    }

                    auto numAdvData = carHandling->pAdvancedData.GetCount();
                    if (numAdvData > 0) {
                        menu.Option(fmt::format("CAdvancedData found ({})", numAdvData));
                    }

                    for (uint16_t iAdv = 0; iAdv < carHandling->pAdvancedData.GetCount(); ++iAdv) {
                        RTHE::CAdvancedData* advancedData = &carHandling->pAdvancedData.Get(iAdv);
                        menu.IntOption(fmt::format("[{}] Slot", iAdv), advancedData->Slot, 0, 255);
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
        menu.Subtitle("Not in a vehicle");
        menu.Option("Not in a vehicle");
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
