#include "HandlingItem.h"
#include "tinyxml2.h"
#include "Util/Logger.hpp"
#include "fmt/format.h"
#include "Memory/HandlingInfo.h"
#include "Util/StrUtil.h"
#include <filesystem>

namespace fs = std::filesystem;

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
void GetAttribute(
    tinyxml2::XMLNode* node,
    const char* elementName,
    const char* attributeName,
    T& out,
    bool tolerateMissing = false) {
    const auto& doc = *node->GetDocument();
    T result{};
    auto* element = node->FirstChildElement(elementName);
    tinyxml2::XMLError err = tinyxml2::XMLError::XML_SUCCESS;
    if (element != nullptr)
        err = element->QueryAttribute(attributeName, &result);

    if (!element || err != tinyxml2::XMLError::XML_SUCCESS) {
        auto errLvl = ERROR;
        if (tolerateMissing)
            errLvl = DEBUG;
        logger.Write(errLvl, "[Parse] Error reading element [%s]", elementName);
        logger.Write(errLvl, "[Parse] Error details: %s", GetXMLError(doc).c_str());
        out = result;
        return;
    }

    out = result;
}

std::string GetElementStr(tinyxml2::XMLNode* node, const char* elementName) {
    const auto& doc = *node->GetDocument();
    std::string result;
    auto* element = node->FirstChildElement(elementName);
    if (!element) {
        logger.Write(ERROR, "[Parse] Error reading element [%s]", elementName);
        logger.Write(ERROR, "[Parse] Error details: %s", GetXMLError(doc).c_str());
        return result;
    }
    return element->GetText();
}

uint32_t StoU32(const std::string& str, int base = 10) {
    try {
        return std::stoul(str, nullptr, base);
    }
    catch(...) {
        logger.Write(WARN, "    Failed to convert hex value.");
        return 0;
    }
}

RTHE::CHandlingDataItem RTHE::ParseXMLItem(const std::string& sourceFile) {
    CHandlingDataItem handlingDataItem{};

    tinyxml2::XMLDocument doc{};
    tinyxml2::XMLError err = tinyxml2::XMLError::XML_SUCCESS;

    logger.Write(DEBUG, "[Parse] Reading handling file [%s]", sourceFile.c_str());

    err = doc.LoadFile(sourceFile.c_str());
    
    if (err != tinyxml2::XMLError::XML_SUCCESS) {
        logger.Write(ERROR, "[Parse] Can't load file [%s]",  sourceFile.c_str());
        logger.Write(ERROR, "[Parse] Error details: %s", GetXMLError(doc).c_str());
        return {};
    }

    tinyxml2::XMLNode* itemNode = doc.FirstChildElement("Item");
    if (!itemNode) {
        logger.Write(ERROR, "[Parse] Error during reading file [%s]", sourceFile.c_str());
        logger.Write(ERROR, "[Parse] Error details: %s", GetXMLError(doc).c_str());
        return {};
    }

    handlingDataItem.handlingName = GetElementStr(itemNode, "handlingName");

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

    GetAttribute(itemNode, "fMass", "value", handlingDataItem.fMass);
    GetAttribute(itemNode, "fInitialDragCoeff", "value", handlingDataItem.fInitialDragCoeff);
    GetAttribute(itemNode, "fDownforceModifier", "value", handlingDataItem.fDownforceModifier, true);
    GetAttribute(itemNode, "fPercentSubmerged", "value", handlingDataItem.fPercentSubmerged);
    GetAttribute(itemNode, "vecCentreOfMassOffset", "x", handlingDataItem.vecCentreOfMassOffsetX);
    GetAttribute(itemNode, "vecCentreOfMassOffset", "y", handlingDataItem.vecCentreOfMassOffsetY);
    GetAttribute(itemNode, "vecCentreOfMassOffset", "z", handlingDataItem.vecCentreOfMassOffsetZ);
    GetAttribute(itemNode, "vecInertiaMultiplier", "x", handlingDataItem.vecInertiaMultiplierX);
    GetAttribute(itemNode, "vecInertiaMultiplier", "y", handlingDataItem.vecInertiaMultiplierY);
    GetAttribute(itemNode, "vecInertiaMultiplier", "z", handlingDataItem.vecInertiaMultiplierZ);
    GetAttribute(itemNode, "fDriveBiasFront", "value", handlingDataItem.fDriveBiasFront);

    uint32_t initialDriveGears = 0;
    GetAttribute(itemNode, "nInitialDriveGears", "value", initialDriveGears);
    handlingDataItem.nInitialDriveGears = static_cast<uint8_t>(initialDriveGears);

    GetAttribute(itemNode, "fInitialDriveForce", "value", handlingDataItem.fInitialDriveForce);
    GetAttribute(itemNode, "fDriveInertia", "value", handlingDataItem.fDriveInertia);
    GetAttribute(itemNode, "fClutchChangeRateScaleUpShift", "value", handlingDataItem.fClutchChangeRateScaleUpShift);
    GetAttribute(itemNode, "fClutchChangeRateScaleDownShift", "value", handlingDataItem.fClutchChangeRateScaleDownShift);
    GetAttribute(itemNode, "fInitialDriveMaxFlatVel", "value", handlingDataItem.fInitialDriveMaxFlatVel);
    GetAttribute(itemNode, "fBrakeForce", "value", handlingDataItem.fBrakeForce);
    GetAttribute(itemNode, "fBrakeBiasFront", "value", handlingDataItem.fBrakeBiasFront);
    GetAttribute(itemNode, "fHandBrakeForce", "value", handlingDataItem.fHandBrakeForce);
    GetAttribute(itemNode, "fSteeringLock", "value", handlingDataItem.fSteeringLock);
    GetAttribute(itemNode, "fTractionCurveMax", "value", handlingDataItem.fTractionCurveMax);
    GetAttribute(itemNode, "fTractionCurveMin", "value", handlingDataItem.fTractionCurveMin);
    GetAttribute(itemNode, "fTractionCurveLateral", "value", handlingDataItem.fTractionCurveLateral);
    GetAttribute(itemNode, "fTractionSpringDeltaMax", "value", handlingDataItem.fTractionSpringDeltaMax);
    GetAttribute(itemNode, "fLowSpeedTractionLossMult", "value", handlingDataItem.fLowSpeedTractionLossMult);
    GetAttribute(itemNode, "fCamberStiffnesss", "value", handlingDataItem.fCamberStiffness); // TODO: fCamberStiffnesss???
    GetAttribute(itemNode, "fTractionBiasFront", "value", handlingDataItem.fTractionBiasFront);
    GetAttribute(itemNode, "fTractionLossMult", "value", handlingDataItem.fTractionLossMult);
    GetAttribute(itemNode, "fSuspensionForce", "value", handlingDataItem.fSuspensionForce);
    GetAttribute(itemNode, "fSuspensionCompDamp", "value", handlingDataItem.fSuspensionCompDamp);
    GetAttribute(itemNode, "fSuspensionReboundDamp", "value", handlingDataItem.fSuspensionReboundDamp);
    GetAttribute(itemNode, "fSuspensionUpperLimit", "value", handlingDataItem.fSuspensionUpperLimit);
    GetAttribute(itemNode, "fSuspensionLowerLimit", "value", handlingDataItem.fSuspensionLowerLimit);
    GetAttribute(itemNode, "fSuspensionRaise", "value", handlingDataItem.fSuspensionRaise);
    GetAttribute(itemNode, "fSuspensionBiasFront", "value", handlingDataItem.fSuspensionBiasFront);
    GetAttribute(itemNode, "fAntiRollBarForce", "value", handlingDataItem.fAntiRollBarForce);
    GetAttribute(itemNode, "fAntiRollBarBiasFront", "value", handlingDataItem.fAntiRollBarBiasFront);
    GetAttribute(itemNode, "fRollCentreHeightFront", "value", handlingDataItem.fRollCentreHeightFront);
    GetAttribute(itemNode, "fRollCentreHeightRear", "value", handlingDataItem.fRollCentreHeightRear);
    GetAttribute(itemNode, "fCollisionDamageMult", "value", handlingDataItem.fCollisionDamageMult);
    GetAttribute(itemNode, "fWeaponDamageMult", "value", handlingDataItem.fWeaponDamageMult);
    GetAttribute(itemNode, "fDeformationDamageMult", "value", handlingDataItem.fDeformationDamageMult);
    GetAttribute(itemNode, "fEngineDamageMult", "value", handlingDataItem.fEngineDamageMult);
    GetAttribute(itemNode, "fPetrolTankVolume", "value", handlingDataItem.fPetrolTankVolume);
    GetAttribute(itemNode, "fOilVolume", "value", handlingDataItem.fOilVolume);
    GetAttribute(itemNode, "fSeatOffsetDistX", "value", handlingDataItem.fSeatOffsetDistX);
    GetAttribute(itemNode, "fSeatOffsetDistY", "value", handlingDataItem.fSeatOffsetDistY);
    GetAttribute(itemNode, "fSeatOffsetDistZ", "value", handlingDataItem.fSeatOffsetDistZ);
    GetAttribute(itemNode, "nMonetaryValue", "value", handlingDataItem.nMonetaryValue);

    handlingDataItem.strModelFlags = StoU32(GetElementStr(itemNode, "strModelFlags"), 16);
    handlingDataItem.strHandlingFlags = StoU32(GetElementStr(itemNode, "strHandlingFlags"), 16);
    handlingDataItem.strDamageFlags = StoU32(GetElementStr(itemNode, "strDamageFlags"), 16);

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

    logger.Write(DEBUG, "[Write] Writing handling file [%s]", targetFile.c_str());
    auto comment = doc.NewComment(" Generated by RTHandlingEditor ");
    doc.InsertFirstChild(comment);

    tinyxml2::XMLNode* itemNode = doc.NewElement("Item");
    if (!itemNode) {
        logger.Write(ERROR, "[Write] Error creating root element [Item]");
        logger.Write(ERROR, "[Write] Error details: %s", GetXMLError(doc).c_str());
        return false;
    }
    
    itemNode->ToElement()->SetAttribute("type", "CHandlingData");

    itemNode = doc.InsertEndChild(itemNode);

    InsertElement(itemNode, "handlingName", handlingDataItem.handlingName.c_str());
    InsertElement(itemNode, "fMass", handlingDataItem.fMass);
    InsertElement(itemNode, "fInitialDragCoeff", handlingDataItem.fInitialDragCoeff);
    if (handlingDataItem.fDownforceModifier != 0.0f)
        InsertElement(itemNode, "fDownforceModifier", handlingDataItem.fDownforceModifier);
    InsertElement(itemNode, "fPercentSubmerged", handlingDataItem.fPercentSubmerged);
    InsertElement(itemNode, "vecCentreOfMassOffset", {
        handlingDataItem.vecCentreOfMassOffsetX,
        handlingDataItem.vecCentreOfMassOffsetY,
        handlingDataItem.vecCentreOfMassOffsetZ
    });
    InsertElement(itemNode, "vecInertiaMultiplier", { 
        handlingDataItem.vecInertiaMultiplierX,
        handlingDataItem.vecInertiaMultiplierY,
        handlingDataItem.vecInertiaMultiplierZ
    });
    InsertElement(itemNode, "fDriveBiasFront", handlingDataItem.fDriveBiasFront);
    InsertElement(itemNode, "nInitialDriveGears", handlingDataItem.nInitialDriveGears);
    InsertElement(itemNode, "fInitialDriveForce", handlingDataItem.fInitialDriveForce);
    InsertElement(itemNode, "fDriveInertia", handlingDataItem.fDriveInertia);
    InsertElement(itemNode, "fClutchChangeRateScaleUpShift", handlingDataItem.fClutchChangeRateScaleUpShift);
    InsertElement(itemNode, "fClutchChangeRateScaleDownShift", handlingDataItem.fClutchChangeRateScaleDownShift);
    InsertElement(itemNode, "fInitialDriveMaxFlatVel", handlingDataItem.fInitialDriveMaxFlatVel);
    InsertElement(itemNode, "fBrakeForce", handlingDataItem.fBrakeForce);
    InsertElement(itemNode, "fBrakeBiasFront", handlingDataItem.fBrakeBiasFront);
    InsertElement(itemNode, "fHandBrakeForce", handlingDataItem.fHandBrakeForce);
    InsertElement(itemNode, "fSteeringLock", handlingDataItem.fSteeringLock);
    InsertElement(itemNode, "fTractionCurveMax", handlingDataItem.fTractionCurveMax);
    InsertElement(itemNode, "fTractionCurveMin", handlingDataItem.fTractionCurveMin);
    InsertElement(itemNode, "fTractionCurveLateral", handlingDataItem.fTractionCurveLateral);
    InsertElement(itemNode, "fTractionSpringDeltaMax", handlingDataItem.fTractionSpringDeltaMax);
    InsertElement(itemNode, "fLowSpeedTractionLossMult", handlingDataItem.fLowSpeedTractionLossMult);
    InsertElement(itemNode, "fCamberStiffnesss", handlingDataItem.fCamberStiffness);
    InsertElement(itemNode, "fTractionBiasFront", handlingDataItem.fTractionBiasFront);
    InsertElement(itemNode, "fTractionLossMult", handlingDataItem.fTractionLossMult);
    InsertElement(itemNode, "fSuspensionForce", handlingDataItem.fSuspensionForce);
    InsertElement(itemNode, "fSuspensionCompDamp", handlingDataItem.fSuspensionCompDamp);
    InsertElement(itemNode, "fSuspensionReboundDamp", handlingDataItem.fSuspensionReboundDamp);
    InsertElement(itemNode, "fSuspensionUpperLimit", handlingDataItem.fSuspensionUpperLimit);
    InsertElement(itemNode, "fSuspensionLowerLimit", handlingDataItem.fSuspensionLowerLimit);
    InsertElement(itemNode, "fSuspensionRaise", handlingDataItem.fSuspensionRaise);
    InsertElement(itemNode, "fSuspensionBiasFront", handlingDataItem.fSuspensionBiasFront);
    InsertElement(itemNode, "fAntiRollBarForce", handlingDataItem.fAntiRollBarForce);
    InsertElement(itemNode, "fAntiRollBarBiasFront", handlingDataItem.fAntiRollBarBiasFront);
    InsertElement(itemNode, "fRollCentreHeightFront", handlingDataItem.fRollCentreHeightFront);
    InsertElement(itemNode, "fRollCentreHeightRear", handlingDataItem.fRollCentreHeightRear);
    InsertElement(itemNode, "fCollisionDamageMult", handlingDataItem.fCollisionDamageMult);
    InsertElement(itemNode, "fWeaponDamageMult", handlingDataItem.fWeaponDamageMult);
    InsertElement(itemNode, "fDeformationDamageMult", handlingDataItem.fDeformationDamageMult);
    InsertElement(itemNode, "fEngineDamageMult", handlingDataItem.fEngineDamageMult);
    InsertElement(itemNode, "fPetrolTankVolume", handlingDataItem.fPetrolTankVolume);
    InsertElement(itemNode, "fOilVolume", handlingDataItem.fOilVolume);
    InsertElement(itemNode, "fSeatOffsetDistX", handlingDataItem.fSeatOffsetDistX);
    InsertElement(itemNode, "fSeatOffsetDistY", handlingDataItem.fSeatOffsetDistY);
    InsertElement(itemNode, "fSeatOffsetDistZ", handlingDataItem.fSeatOffsetDistZ);
    InsertElement(itemNode, "nMonetaryValue", handlingDataItem.nMonetaryValue);

    // flags
    InsertElement(itemNode, "strModelFlags", fmt::format("{:08X}", handlingDataItem.strModelFlags).c_str());
    InsertElement(itemNode, "strHandlingFlags", fmt::format("{:08X}", handlingDataItem.strHandlingFlags).c_str());
    InsertElement(itemNode, "strDamageFlags", fmt::format("{:08X}", handlingDataItem.strDamageFlags).c_str());

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
    InsertElement(itemNode, "AIHandling", AIHandling.c_str());

    logger.Write(DEBUG, fmt::format("SubHandlingData"));
    logger.Write(DEBUG, fmt::format("{} SHDs", handlingDataItem.subHandlingData.size()));
    if (handlingDataItem.subHandlingData.size() == 0) {
        InsertElementSubHandlingNULL(itemNode);
    }
    else {
        tinyxml2::XMLElement* shdElement = itemNode->GetDocument()->NewElement("SubHandlingData");
        for (size_t i = 0; i < handlingDataItem.subHandlingData.size(); ++i) {
            auto type = handlingDataItem.subHandlingData[i].HandlingType;
            logger.Write(DEBUG, fmt::format("[{}] Type: {}", i, type));

            if (type == RTHE::HANDLING_TYPE_CAR) {
                auto carHandlingData = handlingDataItem.subHandlingData[i];

                // 1
                tinyxml2::XMLElement* subHandlingDataItem = itemNode->GetDocument()->NewElement("Item");
                subHandlingDataItem->SetAttribute("type", "CCarHandlingData");

                InsertElement(subHandlingDataItem, "fBackEndPopUpCarImpulseMult", carHandlingData.fBackEndPopUpCarImpulseMult);
                InsertElement(subHandlingDataItem, "fBackEndPopUpBuildingImpulseMult", carHandlingData.fBackEndPopUpBuildingImpulseMult);
                InsertElement(subHandlingDataItem, "fBackEndPopUpMaxDeltaSpeed", carHandlingData.fBackEndPopUpMaxDeltaSpeed);
                InsertElement(subHandlingDataItem, "fToeFront", carHandlingData.fToeFront);
                InsertElement(subHandlingDataItem, "fToeRear", carHandlingData.fToeRear);
                InsertElement(subHandlingDataItem, "fCamberFront", carHandlingData.fCamberFront);
                InsertElement(subHandlingDataItem, "fCamberRear", carHandlingData.fCamberRear);
                InsertElement(subHandlingDataItem, "fCastor", carHandlingData.fCastor);
                InsertElement(subHandlingDataItem, "fEngineResistance", carHandlingData.fEngineResistance);
                InsertElement(subHandlingDataItem, "fMaxDriveBiasTransfer", carHandlingData.fMaxDriveBiasTransfer);
                InsertElement(subHandlingDataItem, "fJumpForceScale", carHandlingData.fJumpForceScale);
                InsertElement(subHandlingDataItem, "strAdvancedFlags", fmt::format("{:08X}", carHandlingData.strAdvancedFlags).c_str());
                //InsertElement(subHandlingDataItem, "fBackEndPopUpCarImpulseMult", carHandlingData.pAdvancedData);

                shdElement->InsertEndChild(subHandlingDataItem);

                // TODO: Figure out why there are 2 additional things for cars, + if they're required.
                // 2
                subHandlingDataItem = itemNode->GetDocument()->NewElement("Item");
                subHandlingDataItem->SetAttribute("type", "NULL");
                shdElement->InsertEndChild(subHandlingDataItem);

                // 3
                subHandlingDataItem = itemNode->GetDocument()->NewElement("Item");
                subHandlingDataItem->SetAttribute("type", "NULL");
                shdElement->InsertEndChild(subHandlingDataItem);

            }
        }
        itemNode->InsertEndChild(shdElement);
    }

    err = doc.SaveFile(targetFile.c_str());

    if (err != tinyxml2::XMLError::XML_SUCCESS) {
        logger.Write(ERROR, "[Write] Can't load file [%s]", targetFile.c_str());
        logger.Write(ERROR, "[Write] Error details: %s", GetXMLError(doc).c_str());
        return false;
    }

    return true;
}
