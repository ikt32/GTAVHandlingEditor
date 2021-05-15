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

namespace fs = std::filesystem;

using VExt = VehicleExtensions;
std::vector<RTHE::CHandlingDataItem> g_handlingDataItems;


void setHandling(Vehicle vehicle, const RTHE::CHandlingDataItem& handlingDataItem) {
    RTHE::CHandlingData* currentHandling = nullptr;

    uint64_t addr = VExt::GetHandlingPtr(vehicle);
    currentHandling = reinterpret_cast<RTHE::CHandlingData*>(addr);

    if (currentHandling == nullptr) {
        logger.Write(ERROR, "[Set handling] Couldn't find handling ptr?");
        return;
    }

    currentHandling->fMass = handlingDataItem.fMass;
    currentHandling->fInitialDragCoeff = handlingDataItem.fInitialDragCoeff / 10000.0f;
    currentHandling->fDownforceModifier = handlingDataItem.fDownforceModifier;

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

    // AI is skipped for now

    auto localLog = [&](const std::string& arg) {
        logger.Write(DEBUG, fmt::format("[Set handling] [{}] {}", handlingDataItem.handlingName, arg));
    };

    for (uint16_t idx = 0; idx < currentHandling->m_subHandlingData.GetCount(); ++idx) {
        RTHE::CBaseSubHandlingData* subHandlingData = currentHandling->m_subHandlingData.Get(idx);

        if (subHandlingData == nullptr) {
            continue;
        }

        if (idx >= handlingDataItem.subHandlingData.size()) {
            localLog(fmt::format("Skipped SubHandlingData[{}], vehicle doesn't have this SubHandlingData", idx));
            continue;
        }

        if (subHandlingData->GetHandlingType() != handlingDataItem.subHandlingData[idx].HandlingType) {
            localLog(fmt::format("Skipped SubHandlingData[{}], vehicle type '{}' does not match handling typ '{}'",
                idx, subHandlingData->GetHandlingType(), handlingDataItem.subHandlingData[idx].HandlingType));
            continue;
        }

        RTHE::CCarHandlingData* carHandlingData = reinterpret_cast<RTHE::CCarHandlingData*>(subHandlingData);

        carHandlingData->fBackEndPopUpCarImpulseMult      = handlingDataItem.subHandlingData[idx].fBackEndPopUpCarImpulseMult     ;
        carHandlingData->fBackEndPopUpBuildingImpulseMult = handlingDataItem.subHandlingData[idx].fBackEndPopUpBuildingImpulseMult;
        carHandlingData->fBackEndPopUpMaxDeltaSpeed       = handlingDataItem.subHandlingData[idx].fBackEndPopUpMaxDeltaSpeed      ;
        carHandlingData->fToeFront                        = handlingDataItem.subHandlingData[idx].fToeFront                       ;
        carHandlingData->fToeRear                         = handlingDataItem.subHandlingData[idx].fToeRear                        ;
        carHandlingData->fCamberFront                     = handlingDataItem.subHandlingData[idx].fCamberFront                    ;
        carHandlingData->fCamberRear                      = handlingDataItem.subHandlingData[idx].fCamberRear                     ;
        carHandlingData->fCastor                          = handlingDataItem.subHandlingData[idx].fCastor                         ;
        carHandlingData->fEngineResistance                = handlingDataItem.subHandlingData[idx].fEngineResistance               ;
        carHandlingData->fMaxDriveBiasTransfer            = handlingDataItem.subHandlingData[idx].fMaxDriveBiasTransfer           ;
        carHandlingData->fJumpForceScale                  = handlingDataItem.subHandlingData[idx].fJumpForceScale                 ;

        //float fUnk_0x034;
        //uint32_t Unk_0x038;

        carHandlingData->strAdvancedFlags = handlingDataItem.subHandlingData[idx].strAdvancedFlags;

        localLog(fmt::format("Loaded SubHandlingData.Item[{}]<CCarHandlingData>", idx));

        // Only apply AdvancedData if the size matches.
        auto numAdvData = carHandlingData->pAdvancedData.GetCount();
        auto numAdvDataFile = handlingDataItem.subHandlingData[idx].pAdvancedData.size();
        if (numAdvData != numAdvDataFile) {
            localLog(fmt::format("Skipped SubHandlingData.Item[{}]<CCarHandlingData>.AdvancedData, size mismatch. "
                "Vehicle has {}, handling has {}", idx, numAdvData, numAdvDataFile));
            continue;
        }

        if (numAdvData == 0) {
            localLog(fmt::format("Skipped SubHandlingData.Item[{}]<CCarHandlingData>.AdvancedData, empty", idx));
            continue;
        }
        
        for (uint32_t iAdv = 0; iAdv < numAdvData; ++iAdv) {
            // Not sure about Slot and Index, I'll just leave that free to mess up for the user.
            carHandlingData->pAdvancedData.Get(iAdv).Index = handlingDataItem.subHandlingData[idx].pAdvancedData[iAdv].Index;
            carHandlingData->pAdvancedData.Get(iAdv).Slot = handlingDataItem.subHandlingData[idx].pAdvancedData[iAdv].Slot;
            carHandlingData->pAdvancedData.Get(iAdv).Value = handlingDataItem.subHandlingData[idx].pAdvancedData[iAdv].Value;
        }

        localLog(fmt::format("Loaded SubHandlingData.Item[{}]<CCarHandlingData>.AdvancedData", idx));
    }
}

void UpdateHandlingDataItems() {
    logger.Write(DEBUG, "[Files] Clearing and handling items...");
    g_handlingDataItems.clear();
    const std::string absoluteModPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + Constants::ModDir;
    const std::string handlingsPath = absoluteModPath + "\\HandlingFiles";

    if (!(fs::exists(fs::path(handlingsPath)) && fs::is_directory(fs::path(handlingsPath)))) {
        logger.Write(WARN, "[Files] Directory [%s] not found!", handlingsPath.c_str());
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
                "[Files] Handling file [%s] failed to parse, skipping...",
                file.path().string().c_str());
            continue;
        }
        g_handlingDataItems.push_back(handling);
        logger.Write(DEBUG, "[Files] Added handling [%s]", handling.handlingName.c_str());
    }
    logger.Write(INFO, "[Files] Configs loaded: %d", g_handlingDataItems.size());
}

// Warning: handlingName not set!
RTHE::CHandlingDataItem getHandling(Vehicle vehicle) {
    RTHE::CHandlingDataItem handlingDataItem{};

    RTHE::CHandlingData* currentHandling = nullptr;

    uint64_t addr = VExt::GetHandlingPtr(vehicle);
    currentHandling = reinterpret_cast<RTHE::CHandlingData*>(addr);

    if (currentHandling == nullptr) {
        logger.Write(ERROR, "[Get handling] Couldn't find handling ptr?");
        return {};
    }

    handlingDataItem.fMass = currentHandling->fMass;
    handlingDataItem.fInitialDragCoeff = currentHandling->fInitialDragCoeff * 10000.0f;
    handlingDataItem.fDownforceModifier = currentHandling->fDownforceModifier;

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

    auto localLog = [&](const std::string& arg) {
        auto it = ASCache::Get().find(currentHandling->NameHash);
        std::string handlingName = (it != ASCache::Get().end()) ? it->second : fmt::format("{:08X}", currentHandling->NameHash);
        logger.Write(DEBUG, fmt::format("[Get handling] [{}] {}", handlingName, arg));
    };

    for (uint16_t idx = 0; idx < currentHandling->m_subHandlingData.GetCount(); ++idx) {
        RTHE::CBaseSubHandlingData* subHandlingData = currentHandling->m_subHandlingData.Get(idx);

        if (!subHandlingData)
            continue;

        if (subHandlingData->GetHandlingType() == RTHE::eHandlingType::HANDLING_TYPE_CAR) {
            localLog(fmt::format("SubHandlingData[{}] Type: CCarHandlingData", idx));

            RTHE::CCarHandlingData* carHandlingData = reinterpret_cast<RTHE::CCarHandlingData *>(subHandlingData);
            RTHE::CCarHandlingDataItem carHandlingDataItem;
            carHandlingDataItem.HandlingType = RTHE::eHandlingType::HANDLING_TYPE_CAR;
            carHandlingDataItem.fBackEndPopUpCarImpulseMult      = carHandlingData->fBackEndPopUpCarImpulseMult     ;
            carHandlingDataItem.fBackEndPopUpBuildingImpulseMult = carHandlingData->fBackEndPopUpBuildingImpulseMult;
            carHandlingDataItem.fBackEndPopUpMaxDeltaSpeed       = carHandlingData->fBackEndPopUpMaxDeltaSpeed      ;
            carHandlingDataItem.fToeFront                        = carHandlingData->fToeFront                       ;
            carHandlingDataItem.fToeRear                         = carHandlingData->fToeRear                        ;
            carHandlingDataItem.fCamberFront                     = carHandlingData->fCamberFront                    ;
            carHandlingDataItem.fCamberRear                      = carHandlingData->fCamberRear                     ;
            carHandlingDataItem.fCastor                          = carHandlingData->fCastor                         ;
            carHandlingDataItem.fEngineResistance                = carHandlingData->fEngineResistance               ;
            carHandlingDataItem.fMaxDriveBiasTransfer            = carHandlingData->fMaxDriveBiasTransfer           ;
            carHandlingDataItem.fJumpForceScale                  = carHandlingData->fJumpForceScale                 ;
            //float fUnk_0x034;
            //uint32_t Unk_0x038;
            carHandlingDataItem.strAdvancedFlags                 = carHandlingData->strAdvancedFlags;
            carHandlingDataItem.pAdvancedData = std::vector<RTHE::CAdvancedDataItem>();
            for (uint16_t iAdv = 0; iAdv < carHandlingData->pAdvancedData.GetCount(); ++iAdv) {
                RTHE::CAdvancedData* advancedData = &carHandlingData->pAdvancedData.Get(iAdv);
                carHandlingDataItem.pAdvancedData.push_back({ advancedData->Slot , advancedData->Index, advancedData->Value });
            }

            handlingDataItem.subHandlingData.push_back(carHandlingDataItem);
        }
        else {
            localLog(fmt::format("SubHandlingData[{}] Type: {}", idx, subHandlingData->GetHandlingType()));

            RTHE::CCarHandlingDataItem dummyHandlingDataItem;
            dummyHandlingDataItem.HandlingType = subHandlingData->GetHandlingType();
            handlingDataItem.subHandlingData.push_back(dummyHandlingDataItem);
        }
    }

    return handlingDataItem;
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

    const std::string absoluteModPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + Constants::ModDir;
    const std::string handlingsPath = absoluteModPath + "\\HandlingFiles";

    bool duplicate;
    do {
        duplicate = false;
        for (auto& p : std::filesystem::directory_iterator(handlingsPath)) {
            if (StrUtil::toLower(p.path().stem().string()) == StrUtil::toLower(outFile)) {
                duplicate = true;
                outFile = fmt::format("{}_{:02d}", fileName.c_str(), saveFileSuffix++);
            }
        }
    } while (duplicate);

    auto h = getHandling(vehicle);
    h.handlingName = handlingName;
    std::string outPath = fmt::format("{}/{}.xml", handlingsPath, outFile);
    std::replace(outPath.begin(), outPath.end(), '\\', '/');
    RTHE::SaveXMLItem(h, outPath);
    UI::Notify(fmt::format("Saved as {}", outFile));
}

void UpdateCheats() {
    if (MISC::_HAS_CHEAT_STRING_JUST_BEEN_ENTERED(StrUtil::joaat("rtherld"))) {
        Flags::Load();
    }
}

void onMenuInit() {
    UpdateHandlingDataItems();
    GetMenu().ReadSettings();
}
void onMenuClose() {

}

void main() {
    setupCompatibility();
    mem::init();
    VExt::Init();

    GetMenu().RegisterOnMain([] { onMenuInit(); });
    GetMenu().RegisterOnExit([] { onMenuClose(); });
    GetMenu().SetFiles(Constants::MenuFile);
    GetMenu().Initialize();
    GetMenu().ReadSettings();

    Flags::Load();
    UpdateHandlingDataItems();

    while (true) {
        UpdateMenu();
        UpdateCheats();
        WAIT(0);
    }
}

void ScriptMain() {
    srand(GetTickCount());
    main();
}
