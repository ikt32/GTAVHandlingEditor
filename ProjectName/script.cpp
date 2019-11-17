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
#include <sstream>
#include "Memory/VehicleExtensions.hpp"

//const char* const SETTINGSFILE = "./HandlingEditor/HandlingEditor.ini";
const char* const LOGFILE = "./HandlingEditor/HandlingEditor.log";
const char* const MENUFILE = "./HandlingEditor/settings_menu.ini";

Player player;
Ped playerPed;
Vehicle vehicle;
Hash model;
int prevNotification = 0;
float disableVal = -1337.0f;

VehicleExtensions g_ext;

namespace Offsets {
    uint64_t Handling;
}

template <typename T>
T getHandlingValue(Vehicle veh, const int valueOffset) {
    const uint64_t address = mem::GetAddressOfEntity(veh);
    uint64_t handlingPtr = *reinterpret_cast<uint64_t*>(address + Offsets::Handling);
    return *reinterpret_cast<T*>(handlingPtr + valueOffset);
}

template <typename T>
void setHandlingValue(Vehicle veh, int valueOffset, T val) {
    const uint64_t address = mem::GetAddressOfEntity(veh);
    uint64_t handlingPtr = *reinterpret_cast<uint64_t*>(address + Offsets::Handling);
    *reinterpret_cast<T*>(handlingPtr + valueOffset) = val;
}

void showNotification(char* message) {
    if (prevNotification)
        UI::_REMOVE_NOTIFICATION(prevNotification);
    UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
    UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(message);
    prevNotification = UI::_DRAW_NOTIFICATION(false, false);
}

void setHandling(float fMass,
    float fInitialDragCoeff,
    float fPercentSubmerged,
    float vecCentreOfMassOffsetX,
    float vecCentreOfMassOffsetY,
    float vecCentreOfMassOffsetZ,
    float vecInertiaMultiplierX,
    float vecInertiaMultiplierY,
    float vecInertiaMultiplierZ,
    float fDriveBiasFront,
    int nInitialDriveGears,
    float fInitialDriveForce,
    float fDriveInertia,
    float fClutchChangeRateScaleUpShift,
    float fClutchChangeRateScaleDownShift,
    float fInitialDriveMaxFlatVel,
    float fBrakeForce,
    float fBrakeBiasFront,
    float fHandBrakeForce,
    float fSteeringLock,
    float fTractionCurveMax,
    float fTractionCurveMin,
    float fTractionCurveLateral,
    float fTractionSpringDeltaMax,
    float fLowSpeedTractionLossMult,
    float fCamberStiffness,
    float fTractionBiasFront,
    float fTractionLossMult,
    float fSuspensionForce,
    float fSuspensionCompDamp,
    float fSuspensionReboundDamp,
    float fSuspensionUpperLimit,
    float fSuspensionLowerLimit,
    float fSuspensionRaise,
    float fSuspensionBiasFront,
    float fAntiRollBarForce,
    float fAntiRollBarBiasFront,
    float fRollCentreHeightFront,
    float fRollCentreHeightRear,
    float fCollisionDamageMult,
    float fWeaponDamageMult,
    float fDeformationDamageMult,
    float fEngineDamageMult,
    float fPetrolTankVolume,
    float fOilVolume,
    float fSeatOffsetDistX,
    float fSeatOffsetDistY,
    float fSeatOffsetDistZ,
    int nMonetaryValue,
    int strModelFlags,
    int strHandlingFlags,
    int strDamageFlags) {
    if (fMass != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fMass, fMass);

    if (fInitialDragCoeff != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fInitialDragCoeff, fInitialDragCoeff / 10000.0f);

    if (fPercentSubmerged != disableVal) {
        setHandlingValue(vehicle, RTH::Offsets.fPercentSubmerged, fPercentSubmerged);
        setHandlingValue(vehicle, RTH::Offsets.fSubmergedRatio, 100.0f / fPercentSubmerged);
    }

    if (vecCentreOfMassOffsetX != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.vecCentreOfMass.x, vecCentreOfMassOffsetX);

    if (vecCentreOfMassOffsetY != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.vecCentreOfMass.y, vecCentreOfMassOffsetY);

    if (vecCentreOfMassOffsetZ != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.vecCentreOfMass.z, vecCentreOfMassOffsetZ);

    if (vecInertiaMultiplierX != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.vecInertiaMultiplier.x, vecInertiaMultiplierX);

    if (vecInertiaMultiplierY != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.vecInertiaMultiplier.y, vecInertiaMultiplierY);

    if (vecInertiaMultiplierZ != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.vecInertiaMultiplier.z, vecInertiaMultiplierZ);

    if (fDriveBiasFront != disableVal) {
        if (fDriveBiasFront == 1.0f) {
            setHandlingValue(vehicle, RTH::Offsets.fDriveBiasRear, 0.0f);
            setHandlingValue(vehicle, RTH::Offsets.fDriveBiasFront, 1.0f);
        }
        else if (fDriveBiasFront == 0.0f) {
            setHandlingValue(vehicle, RTH::Offsets.fDriveBiasRear, 1.0f);
            setHandlingValue(vehicle, RTH::Offsets.fDriveBiasFront, 0.0f);
        }
        else {
            setHandlingValue(vehicle, RTH::Offsets.fDriveBiasRear, 2.0f * (1.0f - (fDriveBiasFront)));
            setHandlingValue(vehicle, RTH::Offsets.fDriveBiasFront, fDriveBiasFront * 2.0f);
        }
    }

    if (nInitialDriveGears != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.nInitialDriveGears, nInitialDriveGears);

    if (fInitialDriveForce != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fInitialDriveForce, fInitialDriveForce);

    if (fDriveInertia != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fDriveInertia, fDriveInertia);

    if (fClutchChangeRateScaleUpShift != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fClutchChangeRateScaleUpShift, fClutchChangeRateScaleUpShift);

    if (fClutchChangeRateScaleDownShift != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fClutchChangeRateScaleDownShift, fClutchChangeRateScaleDownShift);

    if (fInitialDriveMaxFlatVel != disableVal) {
        setHandlingValue(vehicle, RTH::Offsets.fInitialDriveMaxFlatVel, fInitialDriveMaxFlatVel / 3.6f);
        setHandlingValue(vehicle, RTH::Offsets.fDriveMaxFlatVel, fInitialDriveMaxFlatVel / 3.0f);
    }

    if (fBrakeForce != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fBrakeForce, fBrakeForce);

    if (fBrakeBiasFront != disableVal) {
        setHandlingValue(vehicle, RTH::Offsets.fBrakeBiasRear, 2.0f * (1.0f - (fBrakeBiasFront)));
        setHandlingValue(vehicle, RTH::Offsets.fBrakeBiasFront, fBrakeBiasFront * 2.0f);
    }

    if (fHandBrakeForce != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fHandBrakeForce, fHandBrakeForce);

    if (fSteeringLock != disableVal) {
        setHandlingValue(vehicle, RTH::Offsets.fSteeringLock, fSteeringLock * 0.017453292f);
        setHandlingValue(vehicle, RTH::Offsets.fSteeringLockRatio, 1.0f / (fSteeringLock * 0.017453292f));
    }

    if (fTractionCurveMax != disableVal) {
        setHandlingValue(vehicle, RTH::Offsets.fTractionCurveMax, fTractionCurveMax);
        if (fTractionCurveMax == 0.0f)
            setHandlingValue(vehicle, RTH::Offsets.fTractionCurveMaxRatio, 100000000.000000f); // wtf
        else
            setHandlingValue(vehicle, RTH::Offsets.fTractionCurveMaxRatio, 1.0f / fTractionCurveMax);
    }

    if (fTractionCurveMin != disableVal) {
        setHandlingValue(vehicle, RTH::Offsets.fTractionCurveMin, fTractionCurveMin);
    }

    {
        float temp_fTractionCurveMax = getHandlingValue<float>(vehicle, RTH::Offsets.fTractionCurveMax);
        float temp_ftractionCurveMin = getHandlingValue<float>(vehicle, RTH::Offsets.fTractionCurveMin);
        if (temp_fTractionCurveMax <= temp_ftractionCurveMin) {
            setHandlingValue(vehicle, RTH::Offsets.fTractionCurveRatio, 100000000.000000f);
        }
        else {
            setHandlingValue(vehicle, RTH::Offsets.fTractionCurveRatio,
                1.0f / (temp_fTractionCurveMax - temp_ftractionCurveMin));
        }
    }



    if (fTractionCurveLateral != disableVal) {
        setHandlingValue(vehicle, RTH::Offsets.fTractionCurveLateral, fTractionCurveLateral * 0.017453292f);
        setHandlingValue(vehicle, RTH::Offsets.fTractionCurveLateralRatio, 1.0f / (fTractionCurveLateral * 0.017453292f));
    }

    if (fTractionSpringDeltaMax != disableVal) {
        setHandlingValue(vehicle, RTH::Offsets.fTractionSpringDeltaMax, fTractionSpringDeltaMax);
        setHandlingValue(vehicle, RTH::Offsets.fTractionSpringDeltaMaxRatio, 1.0f / fTractionSpringDeltaMax);

    }

    if (fLowSpeedTractionLossMult != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fLowSpeedTractionLossMult, fLowSpeedTractionLossMult);

    if (fCamberStiffness != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fCamberStiffness, fCamberStiffness);

    if (fTractionBiasFront != disableVal) {
        setHandlingValue(vehicle, RTH::Offsets.fTractionBiasRear, 2.0f * (1.0f - (fTractionBiasFront)));
        setHandlingValue(vehicle, RTH::Offsets.fTractionBiasFront, fTractionBiasFront * 2.0f);
    }

    if (fTractionLossMult != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fTractionLossMult, fTractionLossMult);

    if (fSuspensionForce != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fSuspensionForce, fSuspensionForce);

    if (fSuspensionCompDamp != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fSuspensionCompDamp, fSuspensionCompDamp / 10.0f);

    if (fSuspensionReboundDamp != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fSuspensionReboundDamp, fSuspensionReboundDamp / 10.0f);

    if (fSuspensionUpperLimit != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fSuspensionUpperLimit, fSuspensionUpperLimit);

    if (fSuspensionLowerLimit != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fSuspensionLowerLimit, fSuspensionLowerLimit);

    if (fSuspensionRaise != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fSuspensionRaise, fSuspensionRaise);

    if (fSuspensionBiasFront != disableVal) {
        setHandlingValue(vehicle, RTH::Offsets.fSuspensionBiasRear, 2.0f * (1.0f - (fSuspensionBiasFront)));
        setHandlingValue(vehicle, RTH::Offsets.fSuspensionBiasFront, fSuspensionBiasFront * 2.0f);
    }

    if (fAntiRollBarForce != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fAntiRollBarForce, fAntiRollBarForce);

    if (fAntiRollBarBiasFront != disableVal) {
        setHandlingValue(vehicle, RTH::Offsets.fAntiRollBarBiasRear, 2.0f * (1.0f - (fAntiRollBarBiasFront)));
        setHandlingValue(vehicle, RTH::Offsets.fAntiRollBarBiasFront, fAntiRollBarBiasFront * 2.0f);
    }

    if (fRollCentreHeightFront != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fRollCentreHeightFront, fRollCentreHeightFront);

    if (fRollCentreHeightRear != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fRollCentreHeightRear, fRollCentreHeightRear);

    if (fCollisionDamageMult != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fCollisionDamageMult, fCollisionDamageMult);

    if (fWeaponDamageMult != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fWeaponDamageMult, fWeaponDamageMult);

    if (fDeformationDamageMult != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fDeformationDamageMult, fDeformationDamageMult);

    if (fEngineDamageMult != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fEngineDamageMult, fEngineDamageMult);

    if (fPetrolTankVolume != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fPetrolTankVolume, fPetrolTankVolume);

    if (fOilVolume != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fOilVolume, fOilVolume);

    if (fSeatOffsetDistX != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fSeatOffsetDistX, fSeatOffsetDistX);

    if (fSeatOffsetDistY != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fSeatOffsetDistY, fSeatOffsetDistY);

    if (fSeatOffsetDistZ != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.fSeatOffsetDistZ, fSeatOffsetDistZ);

    if (nMonetaryValue != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.nMonetaryValue, nMonetaryValue);

    if (strModelFlags != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.dwStrModelFlags, strModelFlags);

    if (strHandlingFlags != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.dwStrHandlingFlags, strHandlingFlags);

    if (strDamageFlags != disableVal)
        setHandlingValue(vehicle, RTH::Offsets.dwStrDamageFlags, strDamageFlags);

    logger.Write(INFO, "[Used values for setHandlingValue]");
    logger.Write(INFO, "strModelFlags offset: 0x%X", RTH::Offsets.dwStrModelFlags);
    logger.Write(INFO, "strModelFlags set to: 0x%X", strModelFlags);
    logger.Write(INFO, "dwStrHandlingFlags offset: 0x%X", RTH::Offsets.dwStrHandlingFlags);
    logger.Write(INFO, "dwStrHandlingFlags set to: 0x%X", strHandlingFlags);
    logger.Write(INFO, "dwStrDamageFlags offset: 0x%X", RTH::Offsets.dwStrDamageFlags);
    logger.Write(INFO, "dwStrDamageFlags set to: 0x%X", strDamageFlags);
    logger.Write(INFO, "[Read back]");
    logger.Write(INFO, "strModelFlags = 0x%X", getHandlingValue<DWORD>(vehicle, RTH::Offsets.dwStrModelFlags));
    logger.Write(INFO, "strHandlingFlags = 0x%X", getHandlingValue<DWORD>(vehicle, RTH::Offsets.dwStrHandlingFlags));
    logger.Write(INFO, "strDamageFlags = 0x%X", getHandlingValue<DWORD>(vehicle, RTH::Offsets.dwStrDamageFlags));
}

void readXMLFile() {
    using namespace tinyxml2;
    tinyxml2::XMLDocument doc;
    XMLError err = doc.LoadFile("./RTHandlingEditor.meta");
    int err2 = 0;


    std::stringstream handlingText;
    float fMass = disableVal;
    float fInitialDragCoeff = disableVal;
    float fPercentSubmerged = disableVal;
    float vecCentreOfMassOffsetX = disableVal;
    float vecCentreOfMassOffsetY = disableVal;
    float vecCentreOfMassOffsetZ = disableVal;
    float vecInertiaMultiplierX = disableVal;
    float vecInertiaMultiplierY = disableVal;
    float vecInertiaMultiplierZ = disableVal;
    float fDriveBiasFront = disableVal;
    int   nInitialDriveGears = static_cast<int>(disableVal);
    float fInitialDriveForce = disableVal;
    float fDriveInertia = disableVal;
    float fClutchChangeRateScaleUpShift = disableVal;
    float fClutchChangeRateScaleDownShift = disableVal;
    float fInitialDriveMaxFlatVel = disableVal;
    float fBrakeForce = disableVal;
    float fBrakeBiasFront = disableVal;
    float fHandBrakeForce = disableVal;
    float fSteeringLock = disableVal;
    float fTractionCurveMax = disableVal;
    float fTractionCurveMin = disableVal;
    float fTractionCurveLateral = disableVal;
    float fTractionSpringDeltaMax = disableVal;
    float fLowSpeedTractionLossMult = disableVal;
    float fCamberStiffness = disableVal;
    float fTractionBiasFront = disableVal;
    float fTractionLossMult = disableVal;
    float fSuspensionForce = disableVal;
    float fSuspensionCompDamp = disableVal;
    float fSuspensionReboundDamp = disableVal;
    float fSuspensionUpperLimit = disableVal;
    float fSuspensionLowerLimit = disableVal;
    float fSuspensionRaise = disableVal;
    float fSuspensionBiasFront = disableVal;
    float fAntiRollBarForce = disableVal;
    float fAntiRollBarBiasFront = disableVal;
    float fRollCentreHeightFront = disableVal;
    float fRollCentreHeightRear = disableVal;
    float fCollisionDamageMult = disableVal;
    float fWeaponDamageMult = disableVal;
    float fDeformationDamageMult = disableVal;
    float fEngineDamageMult = disableVal;
    float fPetrolTankVolume = disableVal;
    float fOilVolume = disableVal;
    float fSeatOffsetDistX = disableVal;
    float fSeatOffsetDistY = disableVal;
    float fSeatOffsetDistZ = disableVal;
    int nMonetaryValue = disableVal;
    int strModelFlags = disableVal;
    int strHandlingFlags = disableVal;
    int strDamageFlags = disableVal;

    if (err != XML_SUCCESS) {
        showNotification("RTHandlingEditor: Can't load RTHandlingEditor.meta");
        return;
    }
    char* xmlGenericFormatError = "RTHandlingEditor: RTHandlingEditor.meta read error";
    XMLNode* pRoot = doc.FirstChild();
    if (!pRoot) {
        showNotification(xmlGenericFormatError);
        return;
    }

    XMLElement* pElement = pRoot->FirstChildElement("handlingName");
    if (pElement) {
        handlingText << "RTHandlingEditor: " << pElement->GetText();
    }

    // Error checking not done because I'm lazy
    pRoot->FirstChildElement("fMass")->QueryAttribute("value", &fMass);
    pRoot->FirstChildElement("fInitialDragCoeff")->QueryAttribute("value", &fInitialDragCoeff);
    pRoot->FirstChildElement("fPercentSubmerged")->QueryAttribute("value", &fPercentSubmerged);
    pRoot->FirstChildElement("vecCentreOfMassOffset")->QueryAttribute("x", &vecCentreOfMassOffsetX);
    pRoot->FirstChildElement("vecCentreOfMassOffset")->QueryAttribute("y", &vecCentreOfMassOffsetY);
    pRoot->FirstChildElement("vecCentreOfMassOffset")->QueryAttribute("z", &vecCentreOfMassOffsetZ);
    pRoot->FirstChildElement("vecInertiaMultiplier")->QueryAttribute("x", &vecInertiaMultiplierX);
    pRoot->FirstChildElement("vecInertiaMultiplier")->QueryAttribute("y", &vecInertiaMultiplierY);
    pRoot->FirstChildElement("vecInertiaMultiplier")->QueryAttribute("z", &vecInertiaMultiplierZ);
    pRoot->FirstChildElement("fDriveBiasFront")->QueryAttribute("value", &fDriveBiasFront);
    pRoot->FirstChildElement("nInitialDriveGears")->QueryAttribute("value", &nInitialDriveGears);
    pRoot->FirstChildElement("fInitialDriveForce")->QueryAttribute("value", &fInitialDriveForce);
    pRoot->FirstChildElement("fDriveInertia")->QueryAttribute("value", &fDriveInertia);
    pRoot->FirstChildElement("fClutchChangeRateScaleUpShift")->QueryAttribute("value", &fClutchChangeRateScaleUpShift);
    pRoot->FirstChildElement("fClutchChangeRateScaleDownShift")->QueryAttribute("value", &fClutchChangeRateScaleDownShift);
    pRoot->FirstChildElement("fInitialDriveMaxFlatVel")->QueryAttribute("value", &fInitialDriveMaxFlatVel);
    pRoot->FirstChildElement("fBrakeForce")->QueryAttribute("value", &fBrakeForce);
    pRoot->FirstChildElement("fBrakeBiasFront")->QueryAttribute("value", &fBrakeBiasFront);
    pRoot->FirstChildElement("fHandBrakeForce")->QueryAttribute("value", &fHandBrakeForce);
    pRoot->FirstChildElement("fSteeringLock")->QueryAttribute("value", &fSteeringLock);
    pRoot->FirstChildElement("fTractionCurveMax")->QueryAttribute("value", &fTractionCurveMax);
    pRoot->FirstChildElement("fTractionCurveMin")->QueryAttribute("value", &fTractionCurveMin);
    pRoot->FirstChildElement("fTractionCurveLateral")->QueryAttribute("value", &fTractionCurveLateral);
    pRoot->FirstChildElement("fTractionSpringDeltaMax")->QueryAttribute("value", &fTractionSpringDeltaMax);
    pRoot->FirstChildElement("fLowSpeedTractionLossMult")->QueryAttribute("value", &fLowSpeedTractionLossMult);
    pRoot->FirstChildElement("fCamberStiffnesss")->QueryAttribute("value", &fCamberStiffness);
    pRoot->FirstChildElement("fTractionBiasFront")->QueryAttribute("value", &fTractionBiasFront);
    pRoot->FirstChildElement("fTractionLossMult")->QueryAttribute("value", &fTractionLossMult);
    pRoot->FirstChildElement("fSuspensionForce")->QueryAttribute("value", &fSuspensionForce);
    pRoot->FirstChildElement("fSuspensionCompDamp")->QueryAttribute("value", &fSuspensionCompDamp);
    pRoot->FirstChildElement("fSuspensionReboundDamp")->QueryAttribute("value", &fSuspensionReboundDamp);
    pRoot->FirstChildElement("fSuspensionUpperLimit")->QueryAttribute("value", &fSuspensionUpperLimit);
    pRoot->FirstChildElement("fSuspensionLowerLimit")->QueryAttribute("value", &fSuspensionLowerLimit);
    pRoot->FirstChildElement("fSuspensionRaise")->QueryAttribute("value", &fSuspensionRaise);
    pRoot->FirstChildElement("fSuspensionBiasFront")->QueryAttribute("value", &fSuspensionBiasFront);
    pRoot->FirstChildElement("fAntiRollBarForce")->QueryAttribute("value", &fAntiRollBarForce);
    pRoot->FirstChildElement("fAntiRollBarBiasFront")->QueryAttribute("value", &fAntiRollBarBiasFront);
    pRoot->FirstChildElement("fRollCentreHeightFront")->QueryAttribute("value", &fRollCentreHeightFront);
    pRoot->FirstChildElement("fRollCentreHeightRear")->QueryAttribute("value", &fRollCentreHeightRear);
    pRoot->FirstChildElement("fCollisionDamageMult")->QueryAttribute("value", &fCollisionDamageMult);
    pRoot->FirstChildElement("fWeaponDamageMult")->QueryAttribute("value", &fWeaponDamageMult);
    pRoot->FirstChildElement("fDeformationDamageMult")->QueryAttribute("value", &fDeformationDamageMult);
    pRoot->FirstChildElement("fEngineDamageMult")->QueryAttribute("value", &fEngineDamageMult);
    pRoot->FirstChildElement("fPetrolTankVolume")->QueryAttribute("value", &fPetrolTankVolume);
    pRoot->FirstChildElement("fOilVolume")->QueryAttribute("value", &fOilVolume);
    pRoot->FirstChildElement("fSeatOffsetDistX")->QueryAttribute("value", &fSeatOffsetDistX);
    pRoot->FirstChildElement("fSeatOffsetDistY")->QueryAttribute("value", &fSeatOffsetDistY);
    pRoot->FirstChildElement("fSeatOffsetDistZ")->QueryAttribute("value", &fSeatOffsetDistZ);
    pRoot->FirstChildElement("nMonetaryValue")->QueryAttribute("value", &nMonetaryValue);

    pElement = pRoot->FirstChildElement("strModelFlags");
    if (pElement && pElement->GetText() != "-1337") {
        strModelFlags = std::stoi(pElement->GetText(), nullptr, 16);
    }

    pElement = pRoot->FirstChildElement("strHandlingFlags");
    if (pElement && pElement->GetText() != "-1337") {
        strHandlingFlags = std::stoi(pElement->GetText(), nullptr, 16);
    }

    pElement = pRoot->FirstChildElement("strDamageFlags");
    if (pElement && pElement->GetText() != "-1337") {
        strDamageFlags = std::stoi(pElement->GetText(), nullptr, 16);
    }

    setHandling(
        fMass,
        fInitialDragCoeff,
        fPercentSubmerged,
        vecCentreOfMassOffsetX,
        vecCentreOfMassOffsetY,
        vecCentreOfMassOffsetZ,
        vecInertiaMultiplierX,
        vecInertiaMultiplierY,
        vecInertiaMultiplierZ,
        fDriveBiasFront,
        nInitialDriveGears,
        fInitialDriveForce,
        fDriveInertia,
        fClutchChangeRateScaleUpShift,
        fClutchChangeRateScaleDownShift,
        fInitialDriveMaxFlatVel,
        fBrakeForce,
        fBrakeBiasFront,
        fHandBrakeForce,
        fSteeringLock,
        fTractionCurveMax,
        fTractionCurveMin,
        fTractionCurveLateral,
        fTractionSpringDeltaMax,
        fLowSpeedTractionLossMult,
        fCamberStiffness,
        fTractionBiasFront,
        fTractionLossMult,
        fSuspensionForce,
        fSuspensionCompDamp,
        fSuspensionReboundDamp,
        fSuspensionUpperLimit,
        fSuspensionLowerLimit,
        fSuspensionRaise,
        fSuspensionBiasFront,
        fAntiRollBarForce,
        fAntiRollBarBiasFront,
        fRollCentreHeightFront,
        fRollCentreHeightRear,
        fCollisionDamageMult,
        fWeaponDamageMult,
        fDeformationDamageMult,
        fEngineDamageMult,
        fPetrolTankVolume,
        fOilVolume,
        fSeatOffsetDistX,
        fSeatOffsetDistY,
        fSeatOffsetDistZ,
        nMonetaryValue,
        strModelFlags,
        strHandlingFlags,
        strDamageFlags);

    showNotification(const_cast<char*>(handlingText.str().c_str()));
}

void Update() {
    //UpdateMenu();
    //// Update player
    //player = PLAYER::PLAYER_ID();
    //playerPed = PLAYER::PLAYER_PED_ID();

    //if (!ENTITY::DOES_ENTITY_EXIST(playerPed) || !PLAYER::IS_PLAYER_CONTROL_ON(player))
    //    return;

    //if (ENTITY::IS_ENTITY_DEAD(playerPed) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, TRUE))
    //    return;

    //vehicle = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);
    //model = ENTITY::GET_ENTITY_MODEL(vehicle);

    //if (!ENTITY::DOES_ENTITY_EXIST(vehicle))
    //    return;

    //if (IsKeyJustUp(logKey)) {
    //    readMemorytoLog(); // TODO: Build XML
    //    showNotification("RTHandlingEditor: Saved to RTHandlingEditor.log");
    //}

    //if (IsKeyJustUp(metaKey)) {
    //    readXMLFile(); // TODO: Parse XML
    //}
}

uint64_t getHandlingOffset() {

    auto addr = mem::FindPattern("\x3C\x03\x0F\x85\x00\x00\x00\x00\x48\x8B\x41\x20\x48\x8B\x88",
        "xxxx????xxxxxxx");

    if (addr == 0) return 0;

    uint64_t offset = *(int*)(addr + 0x16);

    logger.Write(DEBUG, "Offset @ " + std::to_string(offset));
    return offset;
}

void onMenuInit() {

}
void onMenuClose() {

}

void main() {
    logger.SetFile(LOGFILE);
    logger.SetMinLevel(LogLevel::DEBUG);
    mem::init();
    g_ext.initOffsets();

    GetMenu().RegisterOnMain([] { onMenuInit(); });
    GetMenu().RegisterOnExit([] { onMenuClose(); });
    GetMenu().SetFiles("./HandlingEditor/settings_menu.ini");
    GetMenu().Initialize();
    GetMenu().ReadSettings();


    Offsets::Handling = getHandlingOffset();
    if (Offsets::Handling == 0) {
        logger.Write(ERROR, "Could't find handling offset");
        return;
    }
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
