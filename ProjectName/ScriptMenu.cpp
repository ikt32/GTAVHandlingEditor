#include "script.h"

#include "menu.h"

#include "inc/natives.h"

#include "fmt/format.h"
#include "Memory/HandlingInfo.h"
#include "Memory/VehicleExtensions.hpp"
#include "ScriptUtils.h"
#include "HandlingItem.h"

extern VehicleExtensions g_ext;
extern std::vector<RTHE::CHandlingDataItem> g_handlingDataItems;

void setHandling(Vehicle vehicle, const RTHE::CHandlingDataItem& handlingDataItem);

// TODO: MOVE
constexpr unsigned long joaat(const char* s) {
    unsigned long hash = 0;
    for (; *s != '\0'; ++s) {
        auto c = *s;
        if (c >= 0x41 && c <= 0x5A) {
            c += 0x20;
        }
        hash += c;
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

float g_stepSz = 0.005f;

namespace {
NativeMenu::Menu menu;
}

NativeMenu::Menu& GetMenu() {
    return menu;
}

void UpdateMainMenu() {
    menu.Title("Handling Editor");
    menu.Subtitle("v2.0.0");

    menu.MenuOption("Edit current handling", "EditMenu");

    menu.FloatOption("Step size", g_stepSz, 0.0001f, 1.0f, 0.00005f);

    Ped playerPed = PLAYER::PLAYER_PED_ID();
    Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);

    if (ENTITY::DOES_ENTITY_EXIST(vehicle)) {
        if (menu.Option("Respawn vehicle")) {
            Utils::RespawnVehicle(vehicle, playerPed);
        }
    }

    // TODO:
    // 1. Save current as .meta/.xml
    // 2. Load .meta/.xml to current vehicle
    // Pipe dream: Multiple meta/xml editing
    
    menu.MenuOption("Load handling", "LoadMenu");

    menu.Option("Save - TODO");
}

template <typename T>
bool IsNear(T a, T b, T x) {
    return abs(a-b) <= x;
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
    std::string vehicleName = UI::_GET_LABEL_TEXT(vehicleNameLabel.c_str());
    menu.Subtitle(fmt::format("{}", vehicleName));

    RTHE::CHandlingData* currentHandling = nullptr;

    uint64_t addr = g_ext.GetHandlingPtr(vehicle);
    currentHandling = reinterpret_cast<RTHE::CHandlingData*>(addr);

    if (currentHandling == nullptr) {
        menu.Option("Could not find handling pointer!");
        return;
    }

    // TODO:
    // 1. Combine the ratio things to meta spec
    // 2. Reload vehicle for:
    //    - Suspension stuff
    //    - ???
    // 3. Precision handling
    menu.FloatOption("fMass", currentHandling->fMass, 0.0f, 1000000000.0f, 5.0f);

    {
        float fInitialDragCoeff = currentHandling->fInitialDragCoeff * 10000.0f;
        if (menu.FloatOption("fInitialDragCoeff", fInitialDragCoeff, 0.0f, 100.0f, 0.05f)) {
            currentHandling->fInitialDragCoeff = fInitialDragCoeff / 10000.0f;
        }
    }
    menu.FloatOption("fDownforceModifier", currentHandling->fDownforceModifier, 0.0f, 1000.0f, 0.5f);

    if (menu.FloatOption("fPercentSubmerged", currentHandling->fPercentSubmerged, 0.0f, 100.0f, 1.0f)) {
        currentHandling->fSubmergedRatio_ = 100.0f / currentHandling->fPercentSubmerged;
    }

    menu.FloatOption("vecCentreOfMassOffset.x", currentHandling->vecCentreOfMassOffset.x, -100.0f, 100.0f);
    menu.FloatOption("vecCentreOfMassOffset.y", currentHandling->vecCentreOfMassOffset.y, -100.0f, 100.0f);
    menu.FloatOption("vecCentreOfMassOffset.z", currentHandling->vecCentreOfMassOffset.z, -100.0f, 100.0f);

    menu.FloatOption("vecInteriaMultiplier.x", currentHandling->vecInertiaMultiplier.x, -100.0f, 100.0f);
    menu.FloatOption("vecInteriaMultiplier.y", currentHandling->vecInertiaMultiplier.y, -100.0f, 100.0f);
    menu.FloatOption("vecInteriaMultiplier.z", currentHandling->vecInertiaMultiplier.z, -100.0f, 100.0f);

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

        if (menu.FloatOption("fDriveBiasFront", fDriveBiasFrontNorm, 0.0f, 1.0f, 0.01f)) {
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

    //menu.FloatOption("fAcceleration", currentHandling->fAcceleration, -1000.0f, 1000.0f, 0.01f);

    menu.IntOption("nInitialDriveGears", currentHandling->nInitialDriveGears, 1, 7, 1);
    menu.FloatOption("fInitialDriveForce", currentHandling->fInitialDriveForce, -1000.0f, 1000.0f, 0.01f);
    menu.FloatOption("fDriveIntertia", currentHandling->fDriveInertia, -1000.0f, 1000.0f, 0.01f);

    menu.FloatOption("fClutchChangeRateScaleUpShift", currentHandling->fClutchChangeRateScaleUpShift, -1000.0f, 1000.0f, 0.01f);
    menu.FloatOption("fClutchChangeRateScaleDownShift", currentHandling->fClutchChangeRateScaleDownShift, -1000.0f, 1000.0f, 0.01f);

    {
        float fInitialDriveMaxFlatVel = currentHandling->fInitialDriveMaxFlatVel_ * 3.6f;
        if (menu.FloatOption("fInitialDriveMaxFlatVel", fInitialDriveMaxFlatVel, 0.0f, 1000.0f, 0.5f)) {
            currentHandling->fInitialDriveMaxFlatVel_ = fInitialDriveMaxFlatVel / 3.6f;
            currentHandling->fDriveMaxFlatVel_ = fInitialDriveMaxFlatVel / 3.0f;
        }
    }

    menu.FloatOption("fBrakeForce", currentHandling->fBrakeForce, -1000.0f, 1000.0f, 0.01f);

    {
        float fBrakeBiasFront = currentHandling->fBrakeBiasFront_ / 2.0f;
        if (menu.FloatOption("fBrakeBiasFront", fBrakeBiasFront, -1000.0f, 1000.0f, 0.01f)) {
            currentHandling->fBrakeBiasFront_ = fBrakeBiasFront * 2.0f;
            currentHandling->fBrakeBiasRear_ = 2.0f * (1.0f - fBrakeBiasFront);
        }
    }
    
    menu.FloatOption("fHandBrakeForce", currentHandling->fHandBrakeForce2, -1000.0f, 1000.0f, 0.01f);

    {
        // rad 2 deg
        float fSteeringLock = currentHandling->fSteeringLock_ / 0.017453292f;
        if (menu.FloatOption("fSteeringLock", fSteeringLock, 0.0f, 90.0f, 0.10f)) {
            currentHandling->fSteeringLock_ = fSteeringLock * 0.017453292f;
            currentHandling->fSteeringLockRatio_ = 1.0f / (fSteeringLock * 0.017453292f);
        }
    }

    bool tcModified = false;
    {
        float fTractionCurveMax = currentHandling->fTractionCurveMax;
        if (menu.FloatOption("fTractionCurveMax", fTractionCurveMax, 0.0f, 1000.0f, 0.01f)) {
            currentHandling->fTractionCurveMax = fTractionCurveMax;
            currentHandling->fTractionCurveMaxRatio_ = 1.0f / fTractionCurveMax;
            tcModified = true;
        }
    }

    {
        float fTractionCurveMin = currentHandling->fTractionCurveMin;
        if (menu.FloatOption("fTractionCurveMin", fTractionCurveMin, 0.0f, 1000.0f, 0.01f)) {
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
        if (menu.FloatOption("fTractionCurveLateral", fTractionCurveLateral, -1000.0f, 1000.0f, 0.01f)) {
            currentHandling->fTractionCurveLateral_ = fTractionCurveLateral * 0.017453292f;
            currentHandling->fTractionCurveLateralRatio_ = 1.0f / (fTractionCurveLateral * 0.017453292f);
        }
    }

    {
        float fTractionSpringDeltaMax = currentHandling->fTractionSpringDeltaMax;
        if (menu.FloatOption("fTractionSpringDeltaMax", fTractionSpringDeltaMax, -1000.0f, 1000.0f, 0.01f)) {
            currentHandling->fTractionSpringDeltaMax = fTractionSpringDeltaMax;
            currentHandling->fTractionSpringDeltaMaxRatio_ = 1.0f / fTractionSpringDeltaMax;
        }
    }

    menu.FloatOption("fLowSpeedTractionLossMult", currentHandling->fLowSpeedTractionLossMult, -1000.0f, 1000.0f, 0.01f);
    menu.FloatOption("fCamberStiffnesss", currentHandling->fCamberStiffness, -1000.0f, 1000.0f, 0.01f);

    { // todo: ???
        float fTractionBiasFront = currentHandling->fTractionBiasFront_ / 2.0f;
        if (menu.FloatOption("fTractionBiasFront", fTractionBiasFront, 0.0f, 1.0f, 0.01f)) {
            currentHandling->fTractionBiasFront_ = 2.0f * fTractionBiasFront;
            currentHandling->fTractionBiasRear = 2.0f * (1.0f - (fTractionBiasFront));
        }
    }

    menu.FloatOption("fTractionLossMult", currentHandling->fTractionLossMult, -1000.0f, 1000.0f, 0.01f);
    menu.FloatOption("fSuspensionForce", currentHandling->fSuspensionForce, -1000.0f, 1000.0f, 0.01f);

    {
        float fSuspensionCompDamp = currentHandling->fSuspensionCompDamp * 10.0f;
        if (menu.FloatOption("fSuspensionCompDamp", fSuspensionCompDamp, -1000.0f, 1000.0f, 0.01f)) {
            currentHandling->fSuspensionCompDamp = fSuspensionCompDamp / 10.0f;
        }
    }

    {
        float fSuspensionReboundDamp = currentHandling->fSuspensionReboundDamp * 10.0f;
        if (menu.FloatOption("fSuspensionReboundDamp", fSuspensionReboundDamp, -1000.0f, 1000.0f, 0.01f)) {
            currentHandling->fSuspensionReboundDamp = fSuspensionReboundDamp / 10.0f;
        }
    }

    menu.FloatOption("fSuspensionUpperLimit", currentHandling->fSuspensionUpperLimit, -1000.0f, 1000.0f, 0.01f);
    menu.FloatOption("fSuspensionLowerLimit", currentHandling->fSuspensionLowerLimit, -1000.0f, 1000.0f, 0.01f);
    menu.FloatOption("fSuspensionRaise_", currentHandling->fSuspensionRaise_, -1000.0f, 1000.0f, 0.01f);

    {
        float fSuspensionBiasFront = currentHandling->fSuspensionBiasFront_ / 2.0f;
        if (menu.FloatOption("fSuspensionBiasFront_", fSuspensionBiasFront, -1000.0f, 1000.0f, 0.01f)) {
            currentHandling->fSuspensionBiasFront_ = fSuspensionBiasFront * 2.0f;
            currentHandling->fSuspensionBiasRear_ = 2.0f * (1.0f - (fSuspensionBiasFront));
        }
    }

    menu.FloatOption("fAntiRollBarForce", currentHandling->fAntiRollBarForce, -1000.0f, 1000.0f, 0.01f);

    {
        float fAntiRollBarBiasFront = currentHandling->fAntiRollBarBiasFront_ / 2.0f;
        if (menu.FloatOption("fAntiRollBarBiasFront_", fAntiRollBarBiasFront, -1000.0f, 1000.0f, 0.01f)) {
            currentHandling->fAntiRollBarBiasFront_ = fAntiRollBarBiasFront * 2.0f;
            currentHandling->fAntiRollBarBiasRear_ = 2.0f * (1.0f - (fAntiRollBarBiasFront));
        }
    }

    menu.FloatOption("fRollCentreHeightFront", currentHandling->fRollCentreHeightFront, -1000.0f, 1000.0f, 0.01f);
    menu.FloatOption("fRollCentreHeightRear", currentHandling->fRollCentreHeightRear, -1000.0f, 1000.0f, 0.01f);

    menu.FloatOption("fCollisionDamageMult", currentHandling->fCollisionDamageMult, -1000.0f, 1000.0f, 0.01f);
    menu.FloatOption("fWeaponDamageMult", currentHandling->fWeaponDamageMult, -1000.0f, 1000.0f, 0.01f);
    menu.FloatOption("fDeformationDamageMult", currentHandling->fDeformationDamageMult, -1000.0f, 1000.0f, 0.01f);
    menu.FloatOption("fEngineDamageMult", currentHandling->fEngineDamageMult, -1000.0f, 1000.0f, 0.01f);

    menu.FloatOption("fPetrolTankVolume", currentHandling->fPetrolTankVolume, -1000.0f, 1000.0f, 0.01f);
    menu.FloatOption("fOilVolume", currentHandling->fOilVolume, -1000.0f, 1000.0f, 0.01f);

    menu.FloatOption("vecSeatOffsetDistX", currentHandling->vecSeatOffsetDist.x, -1000.0f, 1000.0f, 0.01f);
    menu.FloatOption("vecSeatOffsetDistY", currentHandling->vecSeatOffsetDist.y, -1000.0f, 1000.0f, 0.01f);
    menu.FloatOption("vecSeatOffsetDistZ", currentHandling->vecSeatOffsetDist.z, -1000.0f, 1000.0f, 0.01f);

    menu.IntOption("nMonetaryValue", currentHandling->nMonetaryValue, 0, 1000000, 1);

    {
        std::string strModelFlags = fmt::format("{:X}", currentHandling->strModelFlags);
        if (menu.Option(fmt::format("strModelFlags: {}", strModelFlags))) {
            
        }
    }

    {
        std::string strHandlingFlags = fmt::format("{:X}", currentHandling->strHandlingFlags);
        if (menu.Option(fmt::format("strHandlingFlags: {}", strHandlingFlags))) {

        }
    }

    {
        std::string strDamageFlags = fmt::format("{:X}", currentHandling->strDamageFlags);
        if (menu.Option(fmt::format("strDamageFlags: {}", strDamageFlags))) {

        }
    }

    {
        std::string AIHandling;
        switch (currentHandling->AIHandling) {
            case joaat("AVERAGE"):
                AIHandling = "AVERAGE";
                break;
            case joaat("SPORTS_CAR"):
                AIHandling = "SPORTS_CAR";
                break;
            case joaat("TRUCK"):
                AIHandling = "TRUCK";
                break;
            case joaat("CRAP"):
                AIHandling = "CRAP";
                break;
            default:
                AIHandling = fmt::format("{:X}", currentHandling->AIHandling);
        }
        if (menu.Option(fmt::format("AIHandling: {}", AIHandling), 
            {
                fmt::format("AVERAGE: {:X}", joaat("AVERAGE")),
                fmt::format("SPORTS_CAR: {:X}", joaat("SPORTS_CAR")),
                fmt::format("TRUCK: {:X}", joaat("TRUCK")),
                fmt::format("CRAP: {:X}", joaat("CRAP")),
            })) {
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
    std::string vehicleName = UI::_GET_LABEL_TEXT(vehicleNameLabel.c_str());
    menu.Subtitle(fmt::format("{}", vehicleName));

    // TODO: Filter option

    for (const auto& handlingDataItem : g_handlingDataItems) {
        if (menu.Option(handlingDataItem.handlingName)) {
            setHandling(vehicle, handlingDataItem);
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
