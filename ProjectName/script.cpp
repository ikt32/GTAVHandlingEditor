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

namespace fs = std::filesystem;

VehicleExtensions g_ext;
std::vector<RTHE::CHandlingDataItem> g_handlingDataItems;


void setHandling(Vehicle vehicle, const RTHE::CHandlingDataItem& handlingDataItem) {
    RTHE::CHandlingData* currentHandling = nullptr;

    uint64_t addr = g_ext.GetHandlingPtr(vehicle);
    currentHandling = reinterpret_cast<RTHE::CHandlingData*>(addr);

    if (currentHandling == nullptr) {
        logger.Write(ERROR, "[Set handling] Couldn't find handling ptr?");
        return;
    }

    currentHandling->fMass = handlingDataItem.fMass;
    currentHandling->fInitialDragCoeff = handlingDataItem.fInitialDragCoeff / 10000.0f;

    currentHandling->fPercentSubmerged = handlingDataItem.fPercentSubmerged;
    currentHandling->fSubmergedRatio_ = 100.0f / handlingDataItem.fPercentSubmerged;

    currentHandling->vecCentreOfMassOffset.x = handlingDataItem.vecCentreOfMassOffsetX;
    currentHandling->vecCentreOfMassOffset.y = handlingDataItem.vecCentreOfMassOffsetY;
    currentHandling->vecCentreOfMassOffset.z = handlingDataItem.vecCentreOfMassOffsetZ;

    currentHandling->vecInertiaMultiplier.x = handlingDataItem.vecInertiaMultiplierX;
    currentHandling->vecInertiaMultiplier.y = handlingDataItem.vecInertiaMultiplierY;
    currentHandling->vecInertiaMultiplier.z = handlingDataItem.vecInertiaMultiplierZ;

    if (handlingDataItem.fDriveBiasFront == 1.0f) {
        currentHandling->fDriveBiasFront = 1.0f;
        currentHandling->fDriveBiasRear = 0.0f;
    }
    else if (handlingDataItem.fDriveBiasFront == 0.0f) {
        currentHandling->fDriveBiasFront = 0.0f;
        currentHandling->fDriveBiasRear = 1.0f;
    }
    else {
        currentHandling->fDriveBiasFront = handlingDataItem.fDriveBiasFront * 2.0f;
        currentHandling->fDriveBiasRear = 2.0f * (1.0f - (handlingDataItem.fDriveBiasFront));
    }

    currentHandling->nInitialDriveGears = handlingDataItem.nInitialDriveGears;

    currentHandling->fInitialDriveForce = handlingDataItem.fInitialDriveForce;

    currentHandling->fDriveInertia = handlingDataItem.fDriveInertia;

    currentHandling->fClutchChangeRateScaleUpShift = handlingDataItem.fClutchChangeRateScaleUpShift;

    currentHandling->fClutchChangeRateScaleDownShift = handlingDataItem.fClutchChangeRateScaleDownShift;

    currentHandling->fInitialDriveMaxFlatVel_ = handlingDataItem.fInitialDriveMaxFlatVel / 3.6f;
    currentHandling->fDriveMaxFlatVel_ = handlingDataItem.fInitialDriveMaxFlatVel / 3.0f;

    currentHandling->fBrakeForce = handlingDataItem.fBrakeForce;

    currentHandling->fBrakeBiasFront_ = handlingDataItem.fBrakeBiasFront * 2.0f;
    currentHandling->fBrakeBiasRear_ = 2.0f * (1.0f - (handlingDataItem.fBrakeBiasFront));

    currentHandling->fHandBrakeForce2 = handlingDataItem.fHandBrakeForce;

    currentHandling->fSteeringLock_ = handlingDataItem.fSteeringLock * 0.017453292f;
    currentHandling->fSteeringLockRatio_ = 1.0f / (handlingDataItem.fSteeringLock * 0.017453292f);

    currentHandling->fTractionCurveMax = handlingDataItem.fTractionCurveMax;
    currentHandling->fTractionCurveMaxRatio_ = 1.0f / handlingDataItem.fTractionCurveMax;

    currentHandling->fTractionCurveMin = handlingDataItem.fTractionCurveMin;
    currentHandling->fTractionCurveRatio_ = 1.0f / handlingDataItem.fTractionCurveMin;

    currentHandling->fTractionCurveRatio_ = 1.0f / (currentHandling->fTractionCurveMax - currentHandling->fTractionCurveMin);

    currentHandling->fTractionCurveLateral_ = handlingDataItem.fTractionCurveLateral * 0.017453292f;
    currentHandling->fTractionCurveLateralRatio_ = 1.0f / (handlingDataItem.fTractionCurveLateral * 0.017453292f);

    currentHandling->fTractionSpringDeltaMax = handlingDataItem.fTractionSpringDeltaMax;
    currentHandling->fTractionSpringDeltaMaxRatio_ = 1.0f / handlingDataItem.fTractionSpringDeltaMax;

    currentHandling->fLowSpeedTractionLossMult = handlingDataItem.fLowSpeedTractionLossMult;

    currentHandling->fCamberStiffness = handlingDataItem.fCamberStiffness;

    currentHandling->fTractionBiasFront_ = handlingDataItem.fTractionBiasFront * 2.0f;
    currentHandling->fTractionBiasRear = 2.0f * (1.0f - (handlingDataItem.fTractionBiasFront));

    currentHandling->fTractionLossMult = handlingDataItem.fTractionLossMult;

    currentHandling->fSuspensionForce = handlingDataItem.fSuspensionForce;

    currentHandling->fSuspensionCompDamp = handlingDataItem.fSuspensionCompDamp / 10.0f;

    currentHandling->fSuspensionReboundDamp = handlingDataItem.fSuspensionReboundDamp / 10.0f;

    currentHandling->fSuspensionUpperLimit = handlingDataItem.fSuspensionUpperLimit;

    currentHandling->fSuspensionLowerLimit = handlingDataItem.fSuspensionLowerLimit;

    currentHandling->fSuspensionRaise_ = handlingDataItem.fSuspensionRaise;

    currentHandling->fSuspensionBiasFront_ = handlingDataItem.fSuspensionBiasFront * 2.0f;
    currentHandling->fSuspensionBiasRear_ = 2.0f * (1.0f - handlingDataItem.fSuspensionBiasFront);

    currentHandling->fAntiRollBarForce = handlingDataItem.fAntiRollBarForce;

    currentHandling->fAntiRollBarBiasFront_ = handlingDataItem.fAntiRollBarBiasFront * 2.0f;
    currentHandling->fAntiRollBarBiasRear_ = 2.0f * (1.0f - (handlingDataItem.fAntiRollBarBiasFront));

    currentHandling->fRollCentreHeightFront = handlingDataItem.fRollCentreHeightFront;

    currentHandling->fRollCentreHeightRear = handlingDataItem.fRollCentreHeightRear;

    currentHandling->fCollisionDamageMult = handlingDataItem.fCollisionDamageMult;

    currentHandling->fWeaponDamageMult = handlingDataItem.fWeaponDamageMult;

    currentHandling->fDeformationDamageMult = handlingDataItem.fDeformationDamageMult;

    currentHandling->fEngineDamageMult = handlingDataItem.fEngineDamageMult;

    currentHandling->fPetrolTankVolume = handlingDataItem.fPetrolTankVolume;

    currentHandling->fOilVolume = handlingDataItem.fOilVolume;

    currentHandling->vecSeatOffsetDist.x = handlingDataItem.fSeatOffsetDistX;
    currentHandling->vecSeatOffsetDist.y = handlingDataItem.fSeatOffsetDistY;
    currentHandling->vecSeatOffsetDist.z = handlingDataItem.fSeatOffsetDistZ;

    currentHandling->nMonetaryValue = handlingDataItem.nMonetaryValue;

    currentHandling->strModelFlags = handlingDataItem.strModelFlags;
    currentHandling->strHandlingFlags = handlingDataItem.strHandlingFlags;
    currentHandling->strDamageFlags = handlingDataItem.strDamageFlags;
}

void UpdateHandlingDataItems() {
    logger.Write(DEBUG, "Clearing and handling items...");
    g_handlingDataItems.clear();
    const std::string absoluteModPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + Constants::ModDir;
    const std::string handlingsPath = absoluteModPath + "\\HandlingFiles";

    if (!(fs::exists(fs::path(handlingsPath)) && fs::is_directory(fs::path(handlingsPath)))) {
        logger.Write(WARN, "Directory [%s] not found!", handlingsPath.c_str());
        return;
    }

    for (auto& file : fs::directory_iterator(handlingsPath)) {
        // Only parse meta/xml files
        if (StrUtil::toLower(fs::path(file).extension().string()) != ".meta" &&
            StrUtil::toLower(fs::path(file).extension().string()) != ".xml")
            continue;

        RTHE::CHandlingDataItem handling = RTHE::ParseXMLItem(file.path().string());
        if (handling.handlingName.empty()) {
            logger.Write(WARN,
                "Handling file [%s] failed to parse, skipping...",
                file.path().string().c_str());
            continue;
        }
        g_handlingDataItems.push_back(handling);
        logger.Write(DEBUG, "Added handling [%s]", handling.handlingName.c_str());
    }
    logger.Write(INFO, "Configs loaded: %d", g_handlingDataItems.size());
}

// Warning: handlingName not set!
RTHE::CHandlingDataItem getHandling(Vehicle vehicle) {
    RTHE::CHandlingDataItem handlingDataItem{};

    RTHE::CHandlingData* currentHandling = nullptr;

    uint64_t addr = g_ext.GetHandlingPtr(vehicle);
    currentHandling = reinterpret_cast<RTHE::CHandlingData*>(addr);

    if (currentHandling == nullptr) {
        logger.Write(ERROR, "[Set handling] Couldn't find handling ptr?");
        return {};
    }

    handlingDataItem.fMass = currentHandling->fMass;
    handlingDataItem.fInitialDragCoeff = currentHandling->fInitialDragCoeff * 10000.0f;

    handlingDataItem.fPercentSubmerged = currentHandling->fPercentSubmerged;
    handlingDataItem.fPercentSubmerged = 100.0f / currentHandling->fSubmergedRatio_;

    handlingDataItem.vecCentreOfMassOffsetX = currentHandling->vecCentreOfMassOffset.x;
    handlingDataItem.vecCentreOfMassOffsetY = currentHandling->vecCentreOfMassOffset.y;
    handlingDataItem.vecCentreOfMassOffsetZ = currentHandling->vecCentreOfMassOffset.z;

    handlingDataItem.vecInertiaMultiplierX = currentHandling->vecInertiaMultiplier.x;
    handlingDataItem.vecInertiaMultiplierY = currentHandling->vecInertiaMultiplier.y;
    handlingDataItem.vecInertiaMultiplierZ = currentHandling->vecInertiaMultiplier.z;

    {
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
        handlingDataItem.fDriveBiasFront = fDriveBiasFrontNorm;
    }

    handlingDataItem.nInitialDriveGears = currentHandling->nInitialDriveGears;

    handlingDataItem.fInitialDriveForce = currentHandling->fInitialDriveForce;

    handlingDataItem.fDriveInertia = currentHandling->fDriveInertia;

    handlingDataItem.fClutchChangeRateScaleUpShift = currentHandling->fClutchChangeRateScaleUpShift;

    handlingDataItem.fClutchChangeRateScaleDownShift = currentHandling->fClutchChangeRateScaleDownShift;

    handlingDataItem.fInitialDriveMaxFlatVel = currentHandling->fInitialDriveMaxFlatVel_ * 3.6f;

    handlingDataItem.fBrakeForce = currentHandling->fBrakeForce;

    handlingDataItem.fBrakeBiasFront = currentHandling->fBrakeBiasFront_ / 2.0f;

    handlingDataItem.fHandBrakeForce = currentHandling->fHandBrakeForce2;

    handlingDataItem.fSteeringLock  = currentHandling->fSteeringLock_ / 0.017453292f;

    handlingDataItem.fTractionCurveMax = currentHandling->fTractionCurveMax;

    handlingDataItem.fTractionCurveMin = currentHandling->fTractionCurveMin;

    handlingDataItem.fTractionCurveLateral = currentHandling->fTractionCurveLateral_ / 0.017453292f;

    handlingDataItem.fTractionSpringDeltaMax = currentHandling->fTractionSpringDeltaMax;

    handlingDataItem.fLowSpeedTractionLossMult = currentHandling->fLowSpeedTractionLossMult;

    handlingDataItem.fCamberStiffness = currentHandling->fCamberStiffness;

    handlingDataItem.fTractionBiasFront = currentHandling->fTractionBiasFront_ / 2.0f;

    handlingDataItem.fTractionLossMult = currentHandling->fTractionLossMult;

    handlingDataItem.fSuspensionForce = currentHandling->fSuspensionForce;

    handlingDataItem.fSuspensionCompDamp = currentHandling->fSuspensionCompDamp * 10.0f;

    handlingDataItem.fSuspensionReboundDamp = currentHandling->fSuspensionReboundDamp * 10.0f;

    handlingDataItem.fSuspensionUpperLimit = currentHandling->fSuspensionUpperLimit;

    handlingDataItem.fSuspensionLowerLimit = currentHandling->fSuspensionLowerLimit;

    handlingDataItem.fSuspensionRaise = currentHandling->fSuspensionRaise_;

    handlingDataItem.fSuspensionBiasFront = currentHandling->fSuspensionBiasFront_ / 2.0f;

    handlingDataItem.fAntiRollBarForce = currentHandling->fAntiRollBarForce;

    handlingDataItem.fAntiRollBarBiasFront = currentHandling->fAntiRollBarBiasFront_ / 2.0f;

    handlingDataItem.fRollCentreHeightFront = currentHandling->fRollCentreHeightFront;

    handlingDataItem.fRollCentreHeightRear = currentHandling->fRollCentreHeightRear;

    handlingDataItem.fCollisionDamageMult = currentHandling->fCollisionDamageMult;

    handlingDataItem.fWeaponDamageMult = currentHandling->fWeaponDamageMult;

    handlingDataItem.fDeformationDamageMult = currentHandling->fDeformationDamageMult;

    handlingDataItem.fEngineDamageMult = currentHandling->fEngineDamageMult;

    handlingDataItem.fPetrolTankVolume = currentHandling->fPetrolTankVolume;

    handlingDataItem.fOilVolume = currentHandling->fOilVolume;

    handlingDataItem.fSeatOffsetDistX = currentHandling->vecSeatOffsetDist.x;
    handlingDataItem.fSeatOffsetDistY = currentHandling->vecSeatOffsetDist.y;
    handlingDataItem.fSeatOffsetDistZ = currentHandling->vecSeatOffsetDist.z;

    handlingDataItem.nMonetaryValue = currentHandling->nMonetaryValue;

    handlingDataItem.strModelFlags = currentHandling->strModelFlags;
    handlingDataItem.strHandlingFlags = currentHandling->strHandlingFlags;
    handlingDataItem.strDamageFlags = currentHandling->strDamageFlags;

    handlingDataItem.AIHandling = currentHandling->AIHandling;

    // TODO: Extra handling data stuff

    return handlingDataItem;
}

void showNotification(const std::string& message, int* prevNotification) {
    if (prevNotification != nullptr && *prevNotification != 0) {
        UI::_REMOVE_NOTIFICATION(*prevNotification);
    }
    UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");

    UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME((char*)message.c_str());

    int id = UI::_DRAW_NOTIFICATION(false, false);
    if (prevNotification != nullptr) {
        *prevNotification = id;
    }
}

int prevNotif;
void showNotification(const std::string& message) {
    showNotification(message, &prevNotif);
}

void PromptSave(Vehicle vehicle) {
    showNotification("Enter handling name");
    WAIT(0);
    GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(UNK::_GET_CURRENT_LANGUAGE_ID() == 0, "FMMC_KEY_TIP8", "", "", "", "", "", 64);
    while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0) WAIT(0);
    if (!GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT()) {
        showNotification("Cancelled save");
        return;
    }

    std::string handlingName = GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT();
    if (handlingName.empty()) {
        showNotification("No handlingName entered");
        return;
    }

    uint32_t saveFileSuffix = 0;
    std::string outFile = handlingName;

    const std::string absoluteModPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + Constants::ModDir;
    const std::string handlingsPath = absoluteModPath + "\\HandlingFiles";

    bool duplicate;
    do {
        duplicate = false;
        for (auto& p : std::filesystem::directory_iterator(handlingsPath)) {
            if (StrUtil::toLower(p.path().stem().string()) == StrUtil::toLower(outFile)) {
                duplicate = true;
                outFile = fmt::format("{}_{:02d}", handlingName.c_str(), saveFileSuffix++);
            }
        }
    } while (duplicate);

    auto h = getHandling(vehicle);
    h.handlingName = handlingName;
    outFile = fmt::format("{}/{}.xml", handlingsPath, outFile);
    std::replace(outFile.begin(), outFile.end(), '\\', '/');
    RTHE::SaveXMLItem(h, outFile);
    showNotification(fmt::format("Saved as {}", outFile));
}

void Update() {

}

void onMenuInit() {
    UpdateHandlingDataItems();
    GetMenu().ReadSettings();
}
void onMenuClose() {

}

void main() {
    mem::init();
    g_ext.initOffsets();

    GetMenu().RegisterOnMain([] { onMenuInit(); });
    GetMenu().RegisterOnExit([] { onMenuClose(); });
    GetMenu().SetFiles(Constants::MenuFile);
    GetMenu().Initialize();
    GetMenu().ReadSettings();

    UpdateHandlingDataItems();

    while (true)
    {
        UpdateMenu();
        Update();
        WAIT(0);
    }
}

void ScriptMain() {
    srand(GetTickCount());
    main();
}
