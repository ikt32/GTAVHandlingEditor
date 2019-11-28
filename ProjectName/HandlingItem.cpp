#include "HandlingItem.h"
#include "tinyxml2.h"
#include "Util/Logger.hpp"
#include "fmt/format.h"
#include "Memory/HandlingInfo.h"
#include "Util/StrUtil.h"

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
void GetAttribute(tinyxml2::XMLNode* node, const char* elementName, const char* attributeName, T& out) {
    const auto& doc = *node->GetDocument();
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

std::string GetElementStr(tinyxml2::XMLNode* node, const char* elementName) {
    const auto& doc = *node->GetDocument();
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

    pRoot = doc.FirstChildElement("Item");
    if (!pRoot) {
        logger.Write(ERROR, "[Parse] Error during reading file [%s]", sourceFile.c_str());
        logger.Write(ERROR, "[Parse] Error details: %s", GetXMLError(doc).c_str());
        return {};
    }

    handlingDataItem.handlingName = GetElementStr(pRoot, "handlingName");

    if (handlingDataItem.handlingName.empty()) {
        // GetElementStr should already have logged
        return {};
    }

    // Parse additional info
    tinyxml2::XMLNode* rtheNode = doc.FirstChildElement("RTHE");
    handlingDataItem.metaData.fileName = fs::path(sourceFile).filename().string();
    if (rtheNode) {
        handlingDataItem.metaData.description = GetElementStr(rtheNode, "description");
    }

    logger.Write(DEBUG, "[Parse] Reading handling [%s]", handlingDataItem.handlingName.c_str());

    GetAttribute(pRoot, "fMass", "value", handlingDataItem.fMass);
    GetAttribute(pRoot, "fInitialDragCoeff", "value", handlingDataItem.fInitialDragCoeff);
    GetAttribute(pRoot, "fPercentSubmerged", "value", handlingDataItem.fPercentSubmerged);
    GetAttribute(pRoot, "vecCentreOfMassOffset", "x", handlingDataItem.vecCentreOfMassOffsetX);
    GetAttribute(pRoot, "vecCentreOfMassOffset", "y", handlingDataItem.vecCentreOfMassOffsetY);
    GetAttribute(pRoot, "vecCentreOfMassOffset", "z", handlingDataItem.vecCentreOfMassOffsetZ);
    GetAttribute(pRoot, "vecInertiaMultiplier", "x", handlingDataItem.vecInertiaMultiplierX);
    GetAttribute(pRoot, "vecInertiaMultiplier", "y", handlingDataItem.vecInertiaMultiplierY);
    GetAttribute(pRoot, "vecInertiaMultiplier", "z", handlingDataItem.vecInertiaMultiplierZ);
    GetAttribute(pRoot, "fDriveBiasFront", "value", handlingDataItem.fDriveBiasFront);
    GetAttribute(pRoot, "nInitialDriveGears", "value", handlingDataItem.nInitialDriveGears);
    GetAttribute(pRoot, "fInitialDriveForce", "value", handlingDataItem.fInitialDriveForce);
    GetAttribute(pRoot, "fDriveInertia", "value", handlingDataItem.fDriveInertia);
    GetAttribute(pRoot, "fClutchChangeRateScaleUpShift", "value", handlingDataItem.fClutchChangeRateScaleUpShift);
    GetAttribute(pRoot, "fClutchChangeRateScaleDownShift", "value", handlingDataItem.fClutchChangeRateScaleDownShift);
    GetAttribute(pRoot, "fInitialDriveMaxFlatVel", "value", handlingDataItem.fInitialDriveMaxFlatVel);
    GetAttribute(pRoot, "fBrakeForce", "value", handlingDataItem.fBrakeForce);
    GetAttribute(pRoot, "fBrakeBiasFront", "value", handlingDataItem.fBrakeBiasFront);
    GetAttribute(pRoot, "fHandBrakeForce", "value", handlingDataItem.fHandBrakeForce);
    GetAttribute(pRoot, "fSteeringLock", "value", handlingDataItem.fSteeringLock);
    GetAttribute(pRoot, "fTractionCurveMax", "value", handlingDataItem.fTractionCurveMax);
    GetAttribute(pRoot, "fTractionCurveMin", "value", handlingDataItem.fTractionCurveMin);
    GetAttribute(pRoot, "fTractionCurveLateral", "value", handlingDataItem.fTractionCurveLateral);
    GetAttribute(pRoot, "fTractionSpringDeltaMax", "value", handlingDataItem.fTractionSpringDeltaMax);
    GetAttribute(pRoot, "fLowSpeedTractionLossMult", "value", handlingDataItem.fLowSpeedTractionLossMult);
    GetAttribute(pRoot, "fCamberStiffnesss", "value", handlingDataItem.fCamberStiffness); // TODO: fCamberStiffnesss???
    GetAttribute(pRoot, "fTractionBiasFront", "value", handlingDataItem.fTractionBiasFront);
    GetAttribute(pRoot, "fTractionLossMult", "value", handlingDataItem.fTractionLossMult);
    GetAttribute(pRoot, "fSuspensionForce", "value", handlingDataItem.fSuspensionForce);
    GetAttribute(pRoot, "fSuspensionCompDamp", "value", handlingDataItem.fSuspensionCompDamp);
    GetAttribute(pRoot, "fSuspensionReboundDamp", "value", handlingDataItem.fSuspensionReboundDamp);
    GetAttribute(pRoot, "fSuspensionUpperLimit", "value", handlingDataItem.fSuspensionUpperLimit);
    GetAttribute(pRoot, "fSuspensionLowerLimit", "value", handlingDataItem.fSuspensionLowerLimit);
    GetAttribute(pRoot, "fSuspensionRaise", "value", handlingDataItem.fSuspensionRaise);
    GetAttribute(pRoot, "fSuspensionBiasFront", "value", handlingDataItem.fSuspensionBiasFront);
    GetAttribute(pRoot, "fAntiRollBarForce", "value", handlingDataItem.fAntiRollBarForce);
    GetAttribute(pRoot, "fAntiRollBarBiasFront", "value", handlingDataItem.fAntiRollBarBiasFront);
    GetAttribute(pRoot, "fRollCentreHeightFront", "value", handlingDataItem.fRollCentreHeightFront);
    GetAttribute(pRoot, "fRollCentreHeightRear", "value", handlingDataItem.fRollCentreHeightRear);
    GetAttribute(pRoot, "fCollisionDamageMult", "value", handlingDataItem.fCollisionDamageMult);
    GetAttribute(pRoot, "fWeaponDamageMult", "value", handlingDataItem.fWeaponDamageMult);
    GetAttribute(pRoot, "fDeformationDamageMult", "value", handlingDataItem.fDeformationDamageMult);
    GetAttribute(pRoot, "fEngineDamageMult", "value", handlingDataItem.fEngineDamageMult);
    GetAttribute(pRoot, "fPetrolTankVolume", "value", handlingDataItem.fPetrolTankVolume);
    GetAttribute(pRoot, "fOilVolume", "value", handlingDataItem.fOilVolume);
    GetAttribute(pRoot, "fSeatOffsetDistX", "value", handlingDataItem.fSeatOffsetDistX);
    GetAttribute(pRoot, "fSeatOffsetDistY", "value", handlingDataItem.fSeatOffsetDistY);
    GetAttribute(pRoot, "fSeatOffsetDistZ", "value", handlingDataItem.fSeatOffsetDistZ);
    GetAttribute(pRoot, "nMonetaryValue", "value", handlingDataItem.nMonetaryValue);

    handlingDataItem.strModelFlags = std::stoi(GetElementStr(pRoot, "strModelFlags"), nullptr, 16);
    handlingDataItem.strHandlingFlags = std::stoi(GetElementStr(pRoot, "strHandlingFlags"), nullptr, 16);
    handlingDataItem.strDamageFlags = std::stoi(GetElementStr(pRoot, "strDamageFlags"), nullptr, 16);

    return handlingDataItem;
}

void InsertElement(tinyxml2::XMLNode* rootNode, const char* elemName, const char* value) {
    tinyxml2::XMLElement* element = rootNode->GetDocument()->NewElement(elemName);
    element->SetText(value);
    rootNode->InsertEndChild(element);
}

void InsertElement(tinyxml2::XMLNode* rootNode, const char* elemName, float value) {
    tinyxml2::XMLElement* element = rootNode->GetDocument()->NewElement(elemName);
    element->SetAttribute("value", fmt::format("{:.6f}", value).c_str());
    rootNode->InsertEndChild(element);
}

void InsertElement(tinyxml2::XMLNode* rootNode, const char* elemName, int value) {
    tinyxml2::XMLElement* element = rootNode->GetDocument()->NewElement(elemName);
    element->SetAttribute("value", value);
    rootNode->InsertEndChild(element);
}

// Contiguous for initializer list
void InsertElement(tinyxml2::XMLNode* rootNode, const char* elemName, RTHE::Vector3 value) {
    tinyxml2::XMLElement* element = rootNode->GetDocument()->NewElement(elemName);
    element->SetAttribute("x", fmt::format("{:.6f}", value.x).c_str());
    element->SetAttribute("y", fmt::format("{:.6f}", value.y).c_str());
    element->SetAttribute("z", fmt::format("{:.6f}", value.z).c_str());
    rootNode->InsertEndChild(element);
}

void InsertElementSubHandlingNULL(tinyxml2::XMLNode* rootNode) {
    tinyxml2::XMLElement* element = rootNode->GetDocument()->NewElement("SubHandlingData");

    // 1
    tinyxml2::XMLElement* subHandlingDataItem = rootNode->GetDocument()->NewElement("Item");
    subHandlingDataItem->SetAttribute("type", "NULL");
    element->InsertEndChild(subHandlingDataItem);

    // 2
    subHandlingDataItem = rootNode->GetDocument()->NewElement("Item");
    subHandlingDataItem->SetAttribute("type", "NULL");
    element->InsertEndChild(subHandlingDataItem);

    // 3
    subHandlingDataItem = rootNode->GetDocument()->NewElement("Item");
    subHandlingDataItem->SetAttribute("type", "NULL");
    element->InsertEndChild(subHandlingDataItem);

    rootNode->InsertEndChild(element);
}

bool RTHE::SaveXMLItem(const CHandlingDataItem& handlingDataItem, const std::string& targetFile) {
    tinyxml2::XMLDocument doc{};
    tinyxml2::XMLError err = tinyxml2::XMLError::XML_SUCCESS;
    tinyxml2::XMLNode* pRoot = nullptr;

    logger.Write(DEBUG, "[Write] Writing handling file [%s]", targetFile.c_str());
    auto comment = doc.NewComment(" Generated by RTHandlingEditor ");
    doc.InsertFirstChild(comment);

    pRoot = doc.NewElement("Item");
    if (!pRoot) {
        logger.Write(ERROR, "[Write] Error creating root element [Item]");
        logger.Write(ERROR, "[Write] Error details: %s", GetXMLError(doc).c_str());
        return false;
    }
    
    pRoot->ToElement()->SetAttribute("type", "CHandlingData");

    pRoot = doc.InsertEndChild(pRoot);

    InsertElement(pRoot, "handlingName", handlingDataItem.handlingName.c_str());
    InsertElement(pRoot, "fMass", handlingDataItem.fMass);
    InsertElement(pRoot, "fInitialDragCoeff", handlingDataItem.fInitialDragCoeff);
    InsertElement(pRoot, "fPercentSubmerged", handlingDataItem.fPercentSubmerged);
    InsertElement(pRoot, "vecCentreOfMassOffset", {
        handlingDataItem.vecCentreOfMassOffsetX,
        handlingDataItem.vecCentreOfMassOffsetY,
        handlingDataItem.vecCentreOfMassOffsetZ
    });
    InsertElement(pRoot, "vecInertiaMultiplier", { 
        handlingDataItem.vecInertiaMultiplierX,
        handlingDataItem.vecInertiaMultiplierY,
        handlingDataItem.vecInertiaMultiplierZ
    });
    InsertElement(pRoot, "fDriveBiasFront", handlingDataItem.fDriveBiasFront);
    InsertElement(pRoot, "nInitialDriveGears", handlingDataItem.nInitialDriveGears);
    InsertElement(pRoot, "fInitialDriveForce", handlingDataItem.fInitialDriveForce);
    InsertElement(pRoot, "fDriveInertia", handlingDataItem.fDriveInertia);
    InsertElement(pRoot, "fClutchChangeRateScaleUpShift", handlingDataItem.fClutchChangeRateScaleUpShift);
    InsertElement(pRoot, "fClutchChangeRateScaleDownShift", handlingDataItem.fClutchChangeRateScaleDownShift);
    InsertElement(pRoot, "fInitialDriveMaxFlatVel", handlingDataItem.fInitialDriveMaxFlatVel);
    InsertElement(pRoot, "fBrakeForce", handlingDataItem.fBrakeForce);
    InsertElement(pRoot, "fBrakeBiasFront", handlingDataItem.fBrakeBiasFront);
    InsertElement(pRoot, "fHandBrakeForce", handlingDataItem.fHandBrakeForce);
    InsertElement(pRoot, "fSteeringLock", handlingDataItem.fSteeringLock);
    InsertElement(pRoot, "fTractionCurveMax", handlingDataItem.fTractionCurveMax);
    InsertElement(pRoot, "fTractionCurveMin", handlingDataItem.fTractionCurveMin);
    InsertElement(pRoot, "fTractionCurveLateral", handlingDataItem.fTractionCurveLateral);
    InsertElement(pRoot, "fTractionSpringDeltaMax", handlingDataItem.fTractionSpringDeltaMax);
    InsertElement(pRoot, "fLowSpeedTractionLossMult", handlingDataItem.fLowSpeedTractionLossMult);
    InsertElement(pRoot, "fCamberStiffnesss", handlingDataItem.fCamberStiffness);
    InsertElement(pRoot, "fTractionBiasFront", handlingDataItem.fTractionBiasFront);
    InsertElement(pRoot, "fTractionLossMult", handlingDataItem.fTractionLossMult);
    InsertElement(pRoot, "fSuspensionForce", handlingDataItem.fSuspensionForce);
    InsertElement(pRoot, "fSuspensionCompDamp", handlingDataItem.fSuspensionCompDamp);
    InsertElement(pRoot, "fSuspensionReboundDamp", handlingDataItem.fSuspensionReboundDamp);
    InsertElement(pRoot, "fSuspensionUpperLimit", handlingDataItem.fSuspensionUpperLimit);
    InsertElement(pRoot, "fSuspensionLowerLimit", handlingDataItem.fSuspensionLowerLimit);
    InsertElement(pRoot, "fSuspensionRaise", handlingDataItem.fSuspensionRaise);
    InsertElement(pRoot, "fSuspensionBiasFront", handlingDataItem.fSuspensionBiasFront);
    InsertElement(pRoot, "fAntiRollBarForce", handlingDataItem.fAntiRollBarForce);
    InsertElement(pRoot, "fAntiRollBarBiasFront", handlingDataItem.fAntiRollBarBiasFront);
    InsertElement(pRoot, "fRollCentreHeightFront", handlingDataItem.fRollCentreHeightFront);
    InsertElement(pRoot, "fRollCentreHeightRear", handlingDataItem.fRollCentreHeightRear);
    InsertElement(pRoot, "fCollisionDamageMult", handlingDataItem.fCollisionDamageMult);
    InsertElement(pRoot, "fWeaponDamageMult", handlingDataItem.fWeaponDamageMult);
    InsertElement(pRoot, "fDeformationDamageMult", handlingDataItem.fDeformationDamageMult);
    InsertElement(pRoot, "fEngineDamageMult", handlingDataItem.fEngineDamageMult);
    InsertElement(pRoot, "fPetrolTankVolume", handlingDataItem.fPetrolTankVolume);
    InsertElement(pRoot, "fOilVolume", handlingDataItem.fOilVolume);
    InsertElement(pRoot, "fSeatOffsetDistX", handlingDataItem.fSeatOffsetDistX);
    InsertElement(pRoot, "fSeatOffsetDistY", handlingDataItem.fSeatOffsetDistY);
    InsertElement(pRoot, "fSeatOffsetDistZ", handlingDataItem.fSeatOffsetDistZ);
    InsertElement(pRoot, "nMonetaryValue", handlingDataItem.nMonetaryValue);

    // flags
    InsertElement(pRoot, "strModelFlags", fmt::format("{:X}", handlingDataItem.strModelFlags).c_str());
    InsertElement(pRoot, "strHandlingFlags", fmt::format("{:X}", handlingDataItem.strHandlingFlags).c_str());
    InsertElement(pRoot, "strDamageFlags", fmt::format("{:X}", handlingDataItem.strDamageFlags).c_str());

    // joaat
    std::string AIHandling;
    switch (handlingDataItem.AIHandling) {
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
            AIHandling = "AVERAGE";
    }
    InsertElement(pRoot, "AIHandling", AIHandling.c_str());

    InsertElementSubHandlingNULL(pRoot);

    err = doc.SaveFile(targetFile.c_str());

    if (err != tinyxml2::XMLError::XML_SUCCESS) {
        logger.Write(ERROR, "[Write] Can't load file [%s]", targetFile.c_str());
        logger.Write(ERROR, "[Write] Error details: %s", GetXMLError(doc).c_str());
        return false;
    }

    return true;
}
