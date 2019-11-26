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
