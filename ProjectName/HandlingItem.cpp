#include "HandlingItem.h"
#include "tinyxml2.h"
#include "Util/Logger.hpp"
#include "fmt/format.h"

std::string GetXMLError(const tinyxml2::XMLDocument& doc) {
    if (doc.Error()) {
        return fmt::format("tinyxml2 encountered an error:\n"
            "Error ID: {}\n"
            "Error line: {}\n"
            "Error name: {}\n"
            "Error text: {}\n",
            doc.ErrorID(),
            doc.ErrorLineNum(),
            doc.ErrorName(),
            doc.ErrorStr());
    }
    return "No error";
}

template <typename T>
void GetAttribute(const tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* node, const char* elementName, const char* attributeName, T& out) {
    T result{};
    auto element = node->FirstChildElement(elementName);
    if (!element) {
        logger.Write(ERROR, "[Parse] Error reading element [%s]", elementName);
        logger.Write(ERROR, "[Parse] Error details: %s", GetXMLError(doc).c_str());
        out = result;
        return;
    }

    tinyxml2::XMLError err = element->QueryAttribute(attributeName, &result);
    if (err != tinyxml2::XMLError::XML_SUCCESS) {
        logger.Write(ERROR, "[Parse] Error reading attribute [%s]", attributeName);
        logger.Write(ERROR, "[Parse] Error details: %s", GetXMLError(doc).c_str());
        out = result;
        return;
    }

    out = result;
}

std::string GetElementStr(const tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* node, const char* elementName) {
    std::string result;
    auto element = node->FirstChildElement(elementName);
    if (!element) {
        logger.Write(ERROR, "[Parse] Error reading element [%s]", elementName);
        logger.Write(ERROR, "[Parse] Error details: %s", GetXMLError(doc).c_str());
        return result;
    }
    return element->GetText();
}

RTHE::CHandlingDataItem RTHE::ParseXMLItem(const std::string& sourceFile) {
    CHandlingDataItem handlingDataItem{};

    tinyxml2::XMLDocument doc{};
    tinyxml2::XMLError err = tinyxml2::XMLError::XML_SUCCESS;
    tinyxml2::XMLNode* pRoot = nullptr;

    logger.Write(DEBUG, "[Parse] Reading handling file [%s]", sourceFile.c_str());

    err = doc.LoadFile(sourceFile.c_str());
    
    if (err != tinyxml2::XMLError::XML_SUCCESS) {
        logger.Write(ERROR, "[Parse] Can't load file [%s]",  sourceFile.c_str());
        logger.Write(ERROR, "[Parse] Error details: %s", GetXMLError(doc).c_str());
        return {};
    }

    pRoot = doc.FirstChild();
    if (!pRoot) {
        logger.Write(ERROR, "[Parse] Error during reading file [%s]", sourceFile.c_str());
        logger.Write(ERROR, "[Parse] Error details: %s", GetXMLError(doc).c_str());
        return {};
    }

    handlingDataItem.handlingName = GetElementStr(doc, pRoot, "handlingName");

    if (handlingDataItem.handlingName.empty()) {
        // GetElementStr should already have logged
        return {};
    }

    logger.Write(DEBUG, "[Parse] Reading handling [%s]", handlingDataItem.handlingName.c_str());

    GetAttribute(doc, pRoot, "fMass", "value", handlingDataItem.fMass);
    GetAttribute(doc, pRoot, "fInitialDragCoeff", "value", handlingDataItem.fInitialDragCoeff);
    GetAttribute(doc, pRoot, "fPercentSubmerged", "value", handlingDataItem.fPercentSubmerged);
    GetAttribute(doc, pRoot, "vecCentreOfMassOffset", "x", handlingDataItem.vecCentreOfMassOffsetX);
    GetAttribute(doc, pRoot, "vecCentreOfMassOffset", "y", handlingDataItem.vecCentreOfMassOffsetY);
    GetAttribute(doc, pRoot, "vecCentreOfMassOffset", "z", handlingDataItem.vecCentreOfMassOffsetZ);
    GetAttribute(doc, pRoot, "vecInertiaMultiplier", "x", handlingDataItem.vecInertiaMultiplierX);
    GetAttribute(doc, pRoot, "vecInertiaMultiplier", "y", handlingDataItem.vecInertiaMultiplierY);
    GetAttribute(doc, pRoot, "vecInertiaMultiplier", "z", handlingDataItem.vecInertiaMultiplierZ);
    GetAttribute(doc, pRoot, "fDriveBiasFront", "value", handlingDataItem.fDriveBiasFront);
    GetAttribute(doc, pRoot, "nInitialDriveGears", "value", handlingDataItem.nInitialDriveGears);
    GetAttribute(doc, pRoot, "fInitialDriveForce", "value", handlingDataItem.fInitialDriveForce);
    GetAttribute(doc, pRoot, "fDriveInertia", "value", handlingDataItem.fDriveInertia);
    GetAttribute(doc, pRoot, "fClutchChangeRateScaleUpShift", "value", handlingDataItem.fClutchChangeRateScaleUpShift);
    GetAttribute(doc, pRoot, "fClutchChangeRateScaleDownShift", "value", handlingDataItem.fClutchChangeRateScaleDownShift);
    GetAttribute(doc, pRoot, "fInitialDriveMaxFlatVel", "value", handlingDataItem.fInitialDriveMaxFlatVel);
    GetAttribute(doc, pRoot, "fBrakeForce", "value", handlingDataItem.fBrakeForce);
    GetAttribute(doc, pRoot, "fBrakeBiasFront", "value", handlingDataItem.fBrakeBiasFront);
    GetAttribute(doc, pRoot, "fHandBrakeForce", "value", handlingDataItem.fHandBrakeForce);
    GetAttribute(doc, pRoot, "fSteeringLock", "value", handlingDataItem.fSteeringLock);
    GetAttribute(doc, pRoot, "fTractionCurveMax", "value", handlingDataItem.fTractionCurveMax);
    GetAttribute(doc, pRoot, "fTractionCurveMin", "value", handlingDataItem.fTractionCurveMin);
    GetAttribute(doc, pRoot, "fTractionCurveLateral", "value", handlingDataItem.fTractionCurveLateral);
    GetAttribute(doc, pRoot, "fTractionSpringDeltaMax", "value", handlingDataItem.fTractionSpringDeltaMax);
    GetAttribute(doc, pRoot, "fLowSpeedTractionLossMult", "value", handlingDataItem.fLowSpeedTractionLossMult);
    GetAttribute(doc, pRoot, "fCamberStiffnesss", "value", handlingDataItem.fCamberStiffness); // TODO: fCamberStiffnesss???
    GetAttribute(doc, pRoot, "fTractionBiasFront", "value", handlingDataItem.fTractionBiasFront);
    GetAttribute(doc, pRoot, "fTractionLossMult", "value", handlingDataItem.fTractionLossMult);
    GetAttribute(doc, pRoot, "fSuspensionForce", "value", handlingDataItem.fSuspensionForce);
    GetAttribute(doc, pRoot, "fSuspensionCompDamp", "value", handlingDataItem.fSuspensionCompDamp);
    GetAttribute(doc, pRoot, "fSuspensionReboundDamp", "value", handlingDataItem.fSuspensionReboundDamp);
    GetAttribute(doc, pRoot, "fSuspensionUpperLimit", "value", handlingDataItem.fSuspensionUpperLimit);
    GetAttribute(doc, pRoot, "fSuspensionLowerLimit", "value", handlingDataItem.fSuspensionLowerLimit);
    GetAttribute(doc, pRoot, "fSuspensionRaise", "value", handlingDataItem.fSuspensionRaise);
    GetAttribute(doc, pRoot, "fSuspensionBiasFront", "value", handlingDataItem.fSuspensionBiasFront);
    GetAttribute(doc, pRoot, "fAntiRollBarForce", "value", handlingDataItem.fAntiRollBarForce);
    GetAttribute(doc, pRoot, "fAntiRollBarBiasFront", "value", handlingDataItem.fAntiRollBarBiasFront);
    GetAttribute(doc, pRoot, "fRollCentreHeightFront", "value", handlingDataItem.fRollCentreHeightFront);
    GetAttribute(doc, pRoot, "fRollCentreHeightRear", "value", handlingDataItem.fRollCentreHeightRear);
    GetAttribute(doc, pRoot, "fCollisionDamageMult", "value", handlingDataItem.fCollisionDamageMult);
    GetAttribute(doc, pRoot, "fWeaponDamageMult", "value", handlingDataItem.fWeaponDamageMult);
    GetAttribute(doc, pRoot, "fDeformationDamageMult", "value", handlingDataItem.fDeformationDamageMult);
    GetAttribute(doc, pRoot, "fEngineDamageMult", "value", handlingDataItem.fEngineDamageMult);
    GetAttribute(doc, pRoot, "fPetrolTankVolume", "value", handlingDataItem.fPetrolTankVolume);
    GetAttribute(doc, pRoot, "fOilVolume", "value", handlingDataItem.fOilVolume);
    GetAttribute(doc, pRoot, "fSeatOffsetDistX", "value", handlingDataItem.fSeatOffsetDistX);
    GetAttribute(doc, pRoot, "fSeatOffsetDistY", "value", handlingDataItem.fSeatOffsetDistY);
    GetAttribute(doc, pRoot, "fSeatOffsetDistZ", "value", handlingDataItem.fSeatOffsetDistZ);
    GetAttribute(doc, pRoot, "nMonetaryValue", "value", handlingDataItem.nMonetaryValue);

    handlingDataItem.strModelFlags = std::stoi(GetElementStr(doc, pRoot, "strModelFlags"), nullptr, 16);
    handlingDataItem.strHandlingFlags = std::stoi(GetElementStr(doc, pRoot, "strHandlingFlags"), nullptr, 16);
    handlingDataItem.strDamageFlags = std::stoi(GetElementStr(doc, pRoot, "strDamageFlags"), nullptr, 16);

    return handlingDataItem;
}

bool RTHE::SaveXMLItem(const CHandlingDataItem& handlingData, const std::string& targetFile) {
    return false;
}
