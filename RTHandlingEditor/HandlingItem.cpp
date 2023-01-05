#include "HandlingItem.h"
#include "Memory/HandlingInfo.h"
#include "Memory/Vector.hpp"

#include "Util/Logger.hpp"
#include "Util/StrUtil.h"

#include <tinyxml2.h>
#include <fmt/format.h>

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
        if (!tolerateMissing) {
            logger.Write(ERROR, "[Parse] Error reading element [%s]", elementName);
            logger.Write(ERROR, "[Parse] Error details: %s", GetXMLError(doc).c_str());
        }
        out = result;
        return;
    }
    logger.Write(DEBUG, fmt::format("[Parse] {}: {}", elementName, result));
    out = result;
}

std::string GetElementStr(tinyxml2::XMLNode* node, const char* elementName, bool tolerateMissing = false) {
    const auto& doc = *node->GetDocument();
    std::string result;
    auto* element = node->FirstChildElement(elementName);
    if (!element) {
        if (!tolerateMissing) {
            logger.Write(ERROR, "[Parse] Error reading element [%s]", elementName);
            logger.Write(ERROR, "[Parse] Error details: %s", GetXMLError(doc).c_str());
        }
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

void ParseCBikeHandlingData(std::string& fileName, tinyxml2::XMLNode* shdItemElement, RTHE::CHandlingDataItem& handlingDataItem) {
    logger.Write(DEBUG, fmt::format("[Parse] [{}] {}", fileName, "Parsing CBikeHandlingData"));

    RTHE::CBikeHandlingDataItem subHandlingDataItem{};
    GetAttribute(shdItemElement, "fLeanFwdCOMMult", "value", subHandlingDataItem.fLeanFwdCOMMult, true);
    GetAttribute(shdItemElement, "fLeanFwdForceMult", "value", subHandlingDataItem.fLeanFwdForceMult, true);
    GetAttribute(shdItemElement, "fLeanBakCOMMult", "value", subHandlingDataItem.fLeanBakCOMMult, true);
    GetAttribute(shdItemElement, "fLeanBakForceMult", "value", subHandlingDataItem.fLeanBakForceMult, true);
    GetAttribute(shdItemElement, "fMaxBankAngle", "value", subHandlingDataItem.fMaxBankAngle, true);
    GetAttribute(shdItemElement, "fFullAnimAngle", "value", subHandlingDataItem.fFullAnimAngle, true);
    GetAttribute(shdItemElement, "fDesLeanReturnFrac", "value", subHandlingDataItem.fDesLeanReturnFrac, true);
    GetAttribute(shdItemElement, "fStickLeanMult", "value", subHandlingDataItem.fStickLeanMult, true);
    GetAttribute(shdItemElement, "fBrakingStabilityMult", "value", subHandlingDataItem.fBrakingStabilityMult, true);
    GetAttribute(shdItemElement, "fInAirSteerMult", "value", subHandlingDataItem.fInAirSteerMult, true);
    GetAttribute(shdItemElement, "fWheelieBalancePoint", "value", subHandlingDataItem.fWheelieBalancePoint, true);
    GetAttribute(shdItemElement, "fStoppieBalancePoint", "value", subHandlingDataItem.fStoppieBalancePoint, true);
    GetAttribute(shdItemElement, "fWheelieSteerMult", "value", subHandlingDataItem.fWheelieSteerMult, true);
    GetAttribute(shdItemElement, "fRearBalanceMult", "value", subHandlingDataItem.fRearBalanceMult, true);
    GetAttribute(shdItemElement, "fFrontBalanceMult", "value", subHandlingDataItem.fFrontBalanceMult, true);
    GetAttribute(shdItemElement, "fBikeGroundSideFrictionMult", "value", subHandlingDataItem.fBikeGroundSideFrictionMult, true);
    GetAttribute(shdItemElement, "fBikeWheelGroundSideFrictionMult", "value", subHandlingDataItem.fBikeWheelGroundSideFrictionMult, true);
    GetAttribute(shdItemElement, "fBikeOnStandLeanAngle", "value", subHandlingDataItem.fBikeOnStandLeanAngle, true);
    GetAttribute(shdItemElement, "fBikeOnStandSteerAngle", "value", subHandlingDataItem.fBikeOnStandSteerAngle, true);
    GetAttribute(shdItemElement, "fJumpForce", "value", subHandlingDataItem.fJumpForce, true);
    handlingDataItem.SubHandlingData.CBikeHandlingData.push_back(subHandlingDataItem);
}

void ParseCFlyingHandlingData(std::string& fileName, tinyxml2::XMLNode* shdItemElement, RTHE::CHandlingDataItem& handlingDataItem) {
    logger.Write(DEBUG, fmt::format("[Parse] [{}] {}", fileName, "Parsing CFlyingHandlingData"));

    RTHE::CFlyingHandlingDataItem subHandlingDataItem{};

    GetAttribute(shdItemElement, "fThrust", "value", subHandlingDataItem.fThrust, true);
    GetAttribute(shdItemElement, "fThrustFallOff", "value", subHandlingDataItem.fThrustFallOff, true);
    GetAttribute(shdItemElement, "fThrustVectoring", "value", subHandlingDataItem.fThrustVectoring, true);
    GetAttribute(shdItemElement, "fInitialThrust", "value", subHandlingDataItem.fInitialThrust, true);
    GetAttribute(shdItemElement, "fInitialThrustFallOff", "value", subHandlingDataItem.fInitialThrustFallOff, true);
    GetAttribute(shdItemElement, "fYawMult", "value", subHandlingDataItem.fYawMult, true);
    GetAttribute(shdItemElement, "fYawStabilise", "value", subHandlingDataItem.fYawStabilise, true);
    GetAttribute(shdItemElement, "fSideSlipMult", "value", subHandlingDataItem.fSideSlipMult, true);
    GetAttribute(shdItemElement, "fInitialYawMult", "value", subHandlingDataItem.fInitialYawMult, true);
    GetAttribute(shdItemElement, "fRollMult", "value", subHandlingDataItem.fRollMult, true);
    GetAttribute(shdItemElement, "fRollStabilise", "value", subHandlingDataItem.fRollStabilise, true);
    GetAttribute(shdItemElement, "fInitialRollMult", "value", subHandlingDataItem.fInitialRollMult, true);
    GetAttribute(shdItemElement, "fPitchMult", "value", subHandlingDataItem.fPitchMult, true);
    GetAttribute(shdItemElement, "fPitchStabilise", "value", subHandlingDataItem.fPitchStabilise, true);
    GetAttribute(shdItemElement, "fInitialPitchMult", "value", subHandlingDataItem.fInitialPitchMult, true);
    GetAttribute(shdItemElement, "fFormLiftMult", "value", subHandlingDataItem.fFormLiftMult, true);
    GetAttribute(shdItemElement, "fAttackLiftMult", "value", subHandlingDataItem.fAttackLiftMult, true);
    GetAttribute(shdItemElement, "fAttackDiveMult", "value", subHandlingDataItem.fAttackDiveMult, true);
    GetAttribute(shdItemElement, "fGearDownDragV", "value", subHandlingDataItem.fGearDownDragV, true);
    GetAttribute(shdItemElement, "fGearDownLiftMult", "value", subHandlingDataItem.fGearDownLiftMult, true);
    GetAttribute(shdItemElement, "fWindMult", "value", subHandlingDataItem.fWindMult, true);
    GetAttribute(shdItemElement, "fMoveRes", "value", subHandlingDataItem.fMoveRes, true);
    GetAttribute(shdItemElement, "vecTurnRes", "x", subHandlingDataItem.vecTurnRes.x, true);
    GetAttribute(shdItemElement, "vecTurnRes", "y", subHandlingDataItem.vecTurnRes.y, true);
    GetAttribute(shdItemElement, "vecTurnRes", "z", subHandlingDataItem.vecTurnRes.z, true);
    GetAttribute(shdItemElement, "vecSpeedRes", "x", subHandlingDataItem.vecSpeedRes.x, true);
    GetAttribute(shdItemElement, "vecSpeedRes", "y", subHandlingDataItem.vecSpeedRes.y, true);
    GetAttribute(shdItemElement, "vecSpeedRes", "z", subHandlingDataItem.vecSpeedRes.z, true);
    GetAttribute(shdItemElement, "fGearDoorFrontOpen", "value", subHandlingDataItem.fGearDoorFrontOpen, true);
    GetAttribute(shdItemElement, "fGearDoorRearOpen", "value", subHandlingDataItem.fGearDoorRearOpen, true);
    GetAttribute(shdItemElement, "fGearDoorRearOpen2", "value", subHandlingDataItem.fGearDoorRearOpen2, true);
    GetAttribute(shdItemElement, "fGearDoorRearMOpen", "value", subHandlingDataItem.fGearDoorRearMOpen, true);
    GetAttribute(shdItemElement, "fTurublenceMagnitudeMax", "value", subHandlingDataItem.fTurublenceMagnitudeMax, true);
    GetAttribute(shdItemElement, "fTurublenceForceMulti", "value", subHandlingDataItem.fTurublenceForceMulti, true);
    GetAttribute(shdItemElement, "fTurublenceRollTorqueMulti", "value", subHandlingDataItem.fTurublenceRollTorqueMulti, true);
    GetAttribute(shdItemElement, "fTurublencePitchTorqueMulti", "value", subHandlingDataItem.fTurublencePitchTorqueMulti, true);
    GetAttribute(shdItemElement, "fBodyDamageControlEffectMult", "value", subHandlingDataItem.fBodyDamageControlEffectMult, true);
    GetAttribute(shdItemElement, "fInputSensitivityForDifficulty", "value", subHandlingDataItem.fInputSensitivityForDifficulty, true);
    GetAttribute(shdItemElement, "fOnGroundYawBoostSpeedPeak", "value", subHandlingDataItem.fOnGroundYawBoostSpeedPeak, true);
    GetAttribute(shdItemElement, "fOnGroundYawBoostSpeedCap", "value", subHandlingDataItem.fOnGroundYawBoostSpeedCap, true);
    GetAttribute(shdItemElement, "fEngineOffGlideMulti", "value", subHandlingDataItem.fEngineOffGlideMulti, true);
    GetAttribute(shdItemElement, "fAfterburnerEffectRadius", "value", subHandlingDataItem.fAfterburnerEffectRadius, true);
    GetAttribute(shdItemElement, "fAfterburnerEffectDistance", "value", subHandlingDataItem.fAfterburnerEffectDistance, true);
    GetAttribute(shdItemElement, "fAfterburnerEffectForceMulti", "value", subHandlingDataItem.fAfterburnerEffectForceMulti, true);
    GetAttribute(shdItemElement, "fSubmergeLevelToPullHeliUnderwater", "value", subHandlingDataItem.fSubmergeLevelToPullHeliUnderwater, true);
    GetAttribute(shdItemElement, "fExtraLiftWithRoll", "value", subHandlingDataItem.fExtraLiftWithRoll, true);

    auto typeStr = GetElementStr(shdItemElement, "handlingType", true);

    if (typeStr == "HANDLING_TYPE_BIKE") subHandlingDataItem.handlingType = RTHE::HANDLING_TYPE_BIKE;
    else if (typeStr == "HANDLING_TYPE_FLYING") subHandlingDataItem.handlingType = RTHE::HANDLING_TYPE_FLYING;
    else if (typeStr == "HANDLING_TYPE_VERTICAL_FLYING") subHandlingDataItem.handlingType = RTHE::HANDLING_TYPE_VERTICAL_FLYING;
    else if (typeStr == "HANDLING_TYPE_BOAT") subHandlingDataItem.handlingType = RTHE::HANDLING_TYPE_BOAT;
    else if (typeStr == "HANDLING_TYPE_SEAPLANE") subHandlingDataItem.handlingType = RTHE::HANDLING_TYPE_SEAPLANE;
    else if (typeStr == "HANDLING_TYPE_SUBMARINE") subHandlingDataItem.handlingType = RTHE::HANDLING_TYPE_SUBMARINE;
    else if (typeStr == "HANDLING_TYPE_TRAIN") subHandlingDataItem.handlingType = RTHE::HANDLING_TYPE_TRAIN;
    else if (typeStr == "HANDLING_TYPE_TRAILER") subHandlingDataItem.handlingType = RTHE::HANDLING_TYPE_TRAILER;
    else if (typeStr == "HANDLING_TYPE_CAR") subHandlingDataItem.handlingType = RTHE::HANDLING_TYPE_CAR;
    else if (typeStr == "HANDLING_TYPE_WEAPON") subHandlingDataItem.handlingType = RTHE::HANDLING_TYPE_WEAPON;
    else if (typeStr == "HANDLING_TYPE_SPECIALFLIGHT") subHandlingDataItem.handlingType = RTHE::HANDLING_TYPE_SPECIALFLIGHT;
    else logger.Write(ERROR, fmt::format("[Parse] [{}] Unrecognized handlingType '{}'", fileName, typeStr));

    handlingDataItem.SubHandlingData.CFlyingHandlingData.push_back(subHandlingDataItem);
}

void ParseCSpecialFlightHandlingData(std::string& fileName, tinyxml2::XMLNode* shdItemElement, RTHE::CHandlingDataItem& handlingDataItem) {
    logger.Write(DEBUG, fmt::format("[Parse] [{}] {}", fileName, "Parsing CSpecialFlightHandlingData"));

    RTHE::CSpecialFlightHandlingDataItem subHandlingDataItem{};

    GetAttribute(shdItemElement, "vecAngularDamping", "x", subHandlingDataItem.vecAngularDamping.x, true);
    GetAttribute(shdItemElement, "vecAngularDamping", "y", subHandlingDataItem.vecAngularDamping.y, true);
    GetAttribute(shdItemElement, "vecAngularDamping", "z", subHandlingDataItem.vecAngularDamping.z, true);
    GetAttribute(shdItemElement, "vecAngularDampingMin", "x", subHandlingDataItem.vecAngularDampingMin.x, true);
    GetAttribute(shdItemElement, "vecAngularDampingMin", "y", subHandlingDataItem.vecAngularDampingMin.y, true);
    GetAttribute(shdItemElement, "vecAngularDampingMin", "z", subHandlingDataItem.vecAngularDampingMin.z, true);
    GetAttribute(shdItemElement, "vecLinearDamping", "x", subHandlingDataItem.vecLinearDamping.x, true);
    GetAttribute(shdItemElement, "vecLinearDamping", "y", subHandlingDataItem.vecLinearDamping.y, true);
    GetAttribute(shdItemElement, "vecLinearDamping", "z", subHandlingDataItem.vecLinearDamping.z, true);
    GetAttribute(shdItemElement, "vecLinearDampingMin", "x", subHandlingDataItem.vecLinearDampingMin.x, true);
    GetAttribute(shdItemElement, "vecLinearDampingMin", "y", subHandlingDataItem.vecLinearDampingMin.y, true);
    GetAttribute(shdItemElement, "vecLinearDampingMin", "z", subHandlingDataItem.vecLinearDampingMin.z, true);
    GetAttribute(shdItemElement, "fLiftCoefficient", "value", subHandlingDataItem.fLiftCoefficient, true);
    GetAttribute(shdItemElement, "fCriticalLiftAngle", "value", subHandlingDataItem.fCriticalLiftAngle, true);
    GetAttribute(shdItemElement, "fInitialLiftAngle", "value", subHandlingDataItem.fInitialLiftAngle, true);
    GetAttribute(shdItemElement, "fMaxLiftAngle", "value", subHandlingDataItem.fMaxLiftAngle, true);
    GetAttribute(shdItemElement, "fDragCoefficient", "value", subHandlingDataItem.fDragCoefficient, true);
    GetAttribute(shdItemElement, "fBrakingDrag", "value", subHandlingDataItem.fBrakingDrag, true);
    GetAttribute(shdItemElement, "fMaxLiftVelocity", "value", subHandlingDataItem.fMaxLiftVelocity, true);
    GetAttribute(shdItemElement, "fMinLiftVelocity", "value", subHandlingDataItem.fMinLiftVelocity, true);
    GetAttribute(shdItemElement, "fRollTorqueScale", "value", subHandlingDataItem.fRollTorqueScale, true);
    GetAttribute(shdItemElement, "fMaxTorqueVelocity", "value", subHandlingDataItem.fMaxTorqueVelocity, true);
    GetAttribute(shdItemElement, "fMinTorqueVelocity", "value", subHandlingDataItem.fMinTorqueVelocity, true);
    GetAttribute(shdItemElement, "fYawTorqueScale", "value", subHandlingDataItem.fYawTorqueScale, true);
    GetAttribute(shdItemElement, "fSelfLevelingPitchTorqueScale", "value", subHandlingDataItem.fSelfLevelingPitchTorqueScale, true);
    GetAttribute(shdItemElement, "fInitalOverheadAssist", "value", subHandlingDataItem.fInitalOverheadAssist, true);
    GetAttribute(shdItemElement, "fMaxPitchTorque", "value", subHandlingDataItem.fMaxPitchTorque, true);
    GetAttribute(shdItemElement, "fMaxSteeringRollTorque", "value", subHandlingDataItem.fMaxSteeringRollTorque, true);
    GetAttribute(shdItemElement, "fPitchTorqueScale", "value", subHandlingDataItem.fPitchTorqueScale, true);
    GetAttribute(shdItemElement, "fSteeringTorqueScale", "value", subHandlingDataItem.fSteeringTorqueScale, true);
    GetAttribute(shdItemElement, "fMaxThrust", "value", subHandlingDataItem.fMaxThrust, true);
    GetAttribute(shdItemElement, "fTransitionDuration", "value", subHandlingDataItem.fTransitionDuration, true);
    GetAttribute(shdItemElement, "fHoverVelocityScale", "value", subHandlingDataItem.fHoverVelocityScale, true);
    GetAttribute(shdItemElement, "fStabilityAssist", "value", subHandlingDataItem.fStabilityAssist, true);
    GetAttribute(shdItemElement, "fMinSpeedForThrustFalloff", "value", subHandlingDataItem.fMinSpeedForThrustFalloff, true);
    GetAttribute(shdItemElement, "fBrakingThrustScale", "value", subHandlingDataItem.fBrakingThrustScale, true);

    GetAttribute(shdItemElement, "mode", "value", subHandlingDataItem.mode, true);
    subHandlingDataItem.strFlags = StoU32(GetElementStr(shdItemElement, "strFlags", true), 16);

    handlingDataItem.SubHandlingData.CSpecialFlightHandlingData.push_back(subHandlingDataItem);
}

void ParseCBoatHandlingData(std::string& fileName, tinyxml2::XMLNode* shdItemElement, RTHE::CHandlingDataItem& handlingDataItem) {
    logger.Write(DEBUG, fmt::format("[Parse] [{}] {}", fileName, "Parsing CBoatHandlingData"));

    RTHE::CBoatHandlingDataItem subHandlingDataItem{};

    GetAttribute(shdItemElement, "fBoxFrontMult", "value", subHandlingDataItem.fBoxFrontMult, true);
    GetAttribute(shdItemElement, "fBoxRearMult", "value", subHandlingDataItem.fBoxRearMult, true);
    GetAttribute(shdItemElement, "fBoxSideMult", "value", subHandlingDataItem.fBoxSideMult, true);
    GetAttribute(shdItemElement, "fSampleTop", "value", subHandlingDataItem.fSampleTop, true);
    GetAttribute(shdItemElement, "fSampleBottom", "value", subHandlingDataItem.fSampleBottom, true);
    GetAttribute(shdItemElement, "fSampleBottomTestCorrection", "value", subHandlingDataItem.fSampleBottomTestCorrection, true);
    GetAttribute(shdItemElement, "fAquaplaneForce", "value", subHandlingDataItem.fAquaplaneForce, true);
    GetAttribute(shdItemElement, "fAquaplanePushWaterMult", "value", subHandlingDataItem.fAquaplanePushWaterMult, true);
    GetAttribute(shdItemElement, "fAquaplanePushWaterCap", "value", subHandlingDataItem.fAquaplanePushWaterCap, true);
    GetAttribute(shdItemElement, "fAquaplanePushWaterApply", "value", subHandlingDataItem.fAquaplanePushWaterApply, true);
    GetAttribute(shdItemElement, "fRudderForce", "value", subHandlingDataItem.fRudderForce, true);
    GetAttribute(shdItemElement, "fRudderOffsetSubmerge", "value", subHandlingDataItem.fRudderOffsetSubmerge, true);
    GetAttribute(shdItemElement, "fRudderOffsetForce", "value", subHandlingDataItem.fRudderOffsetForce, true);
    GetAttribute(shdItemElement, "fRudderOffsetForceZMult", "value", subHandlingDataItem.fRudderOffsetForceZMult, true);
    GetAttribute(shdItemElement, "fWaveAudioMult", "value", subHandlingDataItem.fWaveAudioMult, true);
    GetAttribute(shdItemElement, "vecMoveResistance", "x", subHandlingDataItem.vecMoveResistance.x, true);
    GetAttribute(shdItemElement, "vecMoveResistance", "y", subHandlingDataItem.vecMoveResistance.y, true);
    GetAttribute(shdItemElement, "vecMoveResistance", "z", subHandlingDataItem.vecMoveResistance.z, true);
    GetAttribute(shdItemElement, "vecTurnResistance", "x", subHandlingDataItem.vecTurnResistance.x, true);
    GetAttribute(shdItemElement, "vecTurnResistance", "y", subHandlingDataItem.vecTurnResistance.y, true);
    GetAttribute(shdItemElement, "vecTurnResistance", "z", subHandlingDataItem.vecTurnResistance.z, true);
    GetAttribute(shdItemElement, "fLook_L_R_CamHeight", "value", subHandlingDataItem.fLook_L_R_CamHeight, true);
    GetAttribute(shdItemElement, "fDragCoefficient", "value", subHandlingDataItem.fDragCoefficient, true);
    GetAttribute(shdItemElement, "fKeelSphereSize", "value", subHandlingDataItem.fKeelSphereSize, true);
    GetAttribute(shdItemElement, "fPropRadius", "value", subHandlingDataItem.fPropRadius, true);
    GetAttribute(shdItemElement, "fLowLodAngOffset", "value", subHandlingDataItem.fLowLodAngOffset, true);
    GetAttribute(shdItemElement, "fLowLodDraughtOffset", "value", subHandlingDataItem.fLowLodDraughtOffset, true);
    GetAttribute(shdItemElement, "fImpellerOffset", "value", subHandlingDataItem.fImpellerOffset, true);
    GetAttribute(shdItemElement, "fImpellerForceMult", "value", subHandlingDataItem.fImpellerForceMult, true);
    GetAttribute(shdItemElement, "fDinghySphereBuoyConst", "value", subHandlingDataItem.fDinghySphereBuoyConst, true);
    GetAttribute(shdItemElement, "fProwRaiseMult", "value", subHandlingDataItem.fProwRaiseMult, true);
    GetAttribute(shdItemElement, "fDeepWaterSampleBuoyancyMult", "value", subHandlingDataItem.fDeepWaterSampleBuoyancyMult, true);
    GetAttribute(shdItemElement, "fTransmissionMultiplier", "value", subHandlingDataItem.fTransmissionMultiplier, true);
    GetAttribute(shdItemElement, "fTractionMultiplier", "value", subHandlingDataItem.fTractionMultiplier, true);

    handlingDataItem.SubHandlingData.CBoatHandlingData.push_back(subHandlingDataItem);
}

void ParseCSeaPlaneHandlingData(std::string& fileName, tinyxml2::XMLNode* shdItemElement, RTHE::CHandlingDataItem& handlingDataItem) {
    logger.Write(DEBUG, fmt::format("[Parse] [{}] {}", fileName, "Parsing CSeaPlaneHandlingData"));

    RTHE::CSeaPlaneHandlingDataItem subHandlingDataItem{};

    GetAttribute(shdItemElement, "fLeftPontoonComponentId", "value", subHandlingDataItem.fLeftPontoonComponentId, true);
    GetAttribute(shdItemElement, "fRightPontoonComponentId", "value", subHandlingDataItem.fRightPontoonComponentId, true);
    GetAttribute(shdItemElement, "fPontoonBuoyConst", "value", subHandlingDataItem.fPontoonBuoyConst, true);
    GetAttribute(shdItemElement, "fPontoonSampleSizeFront", "value", subHandlingDataItem.fPontoonSampleSizeFront, true);
    GetAttribute(shdItemElement, "fPontoonSampleSizeMiddle", "value", subHandlingDataItem.fPontoonSampleSizeMiddle, true);
    GetAttribute(shdItemElement, "fPontoonSampleSizeRear", "value", subHandlingDataItem.fPontoonSampleSizeRear, true);
    GetAttribute(shdItemElement, "fPontoonLengthFractionForSamples", "value", subHandlingDataItem.fPontoonLengthFractionForSamples, true);
    GetAttribute(shdItemElement, "fPontoonDragCoefficient", "value", subHandlingDataItem.fPontoonDragCoefficient, true);
    GetAttribute(shdItemElement, "fPontoonVerticalDampingCoefficientUp", "value", subHandlingDataItem.fPontoonVerticalDampingCoefficientUp, true);
    GetAttribute(shdItemElement, "fPontoonVerticalDampingCoefficientDown", "value", subHandlingDataItem.fPontoonVerticalDampingCoefficientDown, true);
    GetAttribute(shdItemElement, "fKeelSphereSize", "value", subHandlingDataItem.fKeelSphereSize, true);

    handlingDataItem.SubHandlingData.CSeaPlaneHandlingData.push_back(subHandlingDataItem);
}

void ParseCSubmarineHandlingData(std::string& fileName, tinyxml2::XMLNode* shdItemElement, RTHE::CHandlingDataItem& handlingDataItem) {
    logger.Write(DEBUG, fmt::format("[Parse] [{}] {}", fileName, "Parsing CSubmarineHandlingData"));

    RTHE::CSubmarineHandlingDataItem subHandlingDataItem{};

    GetAttribute(shdItemElement, "vTurnRes", "x", subHandlingDataItem.vTurnRes.x, true);
    GetAttribute(shdItemElement, "vTurnRes", "y", subHandlingDataItem.vTurnRes.y, true);
    GetAttribute(shdItemElement, "vTurnRes", "z", subHandlingDataItem.vTurnRes.z, true);
    GetAttribute(shdItemElement, "fMoveResXY", "value", subHandlingDataItem.fMoveResXY, true);
    GetAttribute(shdItemElement, "fMoveResZ", "value", subHandlingDataItem.fMoveResZ, true);
    GetAttribute(shdItemElement, "fPitchMult", "value", subHandlingDataItem.fPitchMult, true);
    GetAttribute(shdItemElement, "fPitchAngle", "value", subHandlingDataItem.fPitchAngle, true);
    GetAttribute(shdItemElement, "fYawMult", "value", subHandlingDataItem.fYawMult, true);
    GetAttribute(shdItemElement, "fDiveSpeed", "value", subHandlingDataItem.fDiveSpeed, true);
    GetAttribute(shdItemElement, "fRollMult", "value", subHandlingDataItem.fRollMult, true);
    GetAttribute(shdItemElement, "fRollStab", "value", subHandlingDataItem.fRollStab, true);

    handlingDataItem.SubHandlingData.CSubmarineHandlingData.push_back(subHandlingDataItem);
}

void ParseCTrailerHandlingData(std::string& fileName, tinyxml2::XMLNode* shdItemElement, RTHE::CHandlingDataItem& handlingDataItem) {
    logger.Write(DEBUG, fmt::format("[Parse] [{}] {}", fileName, "Parsing CTrailerHandlingData"));

    RTHE::CTrailerHandlingDataItem subHandlingDataItem{};

    GetAttribute(shdItemElement, "fAttachLimitPitch", "value", subHandlingDataItem.fAttachLimitPitch, true);
    GetAttribute(shdItemElement, "fAttachLimitRoll", "value", subHandlingDataItem.fAttachLimitRoll, true);
    GetAttribute(shdItemElement, "fAttachLimitYaw", "value", subHandlingDataItem.fAttachLimitYaw, true);
    GetAttribute(shdItemElement, "fUprightSpringConstant", "value", subHandlingDataItem.fUprightSpringConstant, true);
    GetAttribute(shdItemElement, "fUprightDampingConstant", "value", subHandlingDataItem.fUprightDampingConstant, true);
    GetAttribute(shdItemElement, "fAttachedMaxDistance", "value", subHandlingDataItem.fAttachedMaxDistance, true);
    GetAttribute(shdItemElement, "fAttachedMaxPenetration", "value", subHandlingDataItem.fAttachedMaxPenetration, true);
    GetAttribute(shdItemElement, "fAttachRaiseZ", "value", subHandlingDataItem.fAttachRaiseZ, true);
    GetAttribute(shdItemElement, "fPosConstraintMassRatio", "value", subHandlingDataItem.fPosConstraintMassRatio, true);

    handlingDataItem.SubHandlingData.CTrailerHandlingData.push_back(subHandlingDataItem);
}

void ParseCCarHandlingData(std::string& fileName, tinyxml2::XMLNode* shdItemElement, RTHE::CHandlingDataItem& handlingDataItem) {
    logger.Write(DEBUG, fmt::format("[Parse] [{}] {}", fileName, "Parsing CCarHandlingData"));

    RTHE::CCarHandlingDataItem carHandlingDataItem{};
    GetAttribute(shdItemElement, "fBackEndPopUpCarImpulseMult", "value", carHandlingDataItem.fBackEndPopUpCarImpulseMult, true);
    GetAttribute(shdItemElement, "fBackEndPopUpBuildingImpulseMult", "value", carHandlingDataItem.fBackEndPopUpBuildingImpulseMult, true);
    GetAttribute(shdItemElement, "fBackEndPopUpMaxDeltaSpeed", "value", carHandlingDataItem.fBackEndPopUpMaxDeltaSpeed, true);

    GetAttribute(shdItemElement, "fToeFront", "value", carHandlingDataItem.fToeFront, true);
    GetAttribute(shdItemElement, "fToeRear", "value", carHandlingDataItem.fToeRear, true);
    GetAttribute(shdItemElement, "fCamberFront", "value", carHandlingDataItem.fCamberFront, true);
    GetAttribute(shdItemElement, "fCamberRear", "value", carHandlingDataItem.fCamberRear, true);
    GetAttribute(shdItemElement, "fCastor", "value", carHandlingDataItem.fCastor, true);
    GetAttribute(shdItemElement, "fEngineResistance", "value", carHandlingDataItem.fEngineResistance, true);
    GetAttribute(shdItemElement, "fMaxDriveBiasTransfer", "value", carHandlingDataItem.fMaxDriveBiasTransfer, true);
    GetAttribute(shdItemElement, "fJumpForceScale", "value", carHandlingDataItem.fJumpForceScale, true);
    GetAttribute(shdItemElement, "fIncreasedRammingForceScale", "value", carHandlingDataItem.fIncreasedRammingForceScale, true);

    carHandlingDataItem.strAdvancedFlags = StoU32(GetElementStr(shdItemElement, "strAdvancedFlags", true), 16);

    tinyxml2::XMLNode* advancedDataNode = shdItemElement->FirstChildElement("AdvancedData");
    if (advancedDataNode != nullptr) {
        logger.Write(DEBUG, fmt::format("[Parse] [{}] {}", fileName, "Parsing CCarHandlingData->AdvancedData"));

        tinyxml2::XMLNode* advDataItemElement = advancedDataNode->FirstChildElement("Item");
        while (advDataItemElement != nullptr) {
            // Type is "CAdvancedData", dunno if any other type/base occurs
            RTHE::CAdvancedData advancedDataItem;
            GetAttribute(advDataItemElement, "Slot", "value", advancedDataItem.Slot);
            GetAttribute(advDataItemElement, "Index", "value", advancedDataItem.Index);
            GetAttribute(advDataItemElement, "Value", "value", advancedDataItem.Value);
            carHandlingDataItem.AdvancedData.push_back(advancedDataItem);

            advDataItemElement = advDataItemElement->NextSiblingElement();
        }
    }

    handlingDataItem.SubHandlingData.CCarHandlingData.push_back(carHandlingDataItem);
}

RTHE::CHandlingDataItem RTHE::ParseXMLItem(const std::string& sourceFile) {
    CHandlingDataItem handlingDataItem{};

    tinyxml2::XMLDocument doc{};
    tinyxml2::XMLError err = tinyxml2::XMLError::XML_SUCCESS;

    std::string fileName = fs::path(sourceFile).filename().string();
    logger.Write(DEBUG, "[Parse] Reading handling file [%s]", fileName.c_str());

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

    handlingDataItem.Metadata.HandlingName = GetElementStr(itemNode, "handlingName");

    if (handlingDataItem.Metadata.HandlingName.empty()) {
        // GetElementStr should already have logged
        return {};
    }

    // Parse additional info
    tinyxml2::XMLNode* rtheNode = doc.FirstChildElement("RTHE");
    handlingDataItem.Metadata.FileName = fs::path(sourceFile).filename().string();
    if (rtheNode) {
        handlingDataItem.Metadata.Description = GetElementStr(rtheNode, "description");
    }

    logger.Write(DEBUG, "[Parse] Reading handling [%s]", handlingDataItem.Metadata.HandlingName.c_str());

    GetAttribute(itemNode, "fMass", "value", handlingDataItem.fMass);
    GetAttribute(itemNode, "fInitialDragCoeff", "value", handlingDataItem.fInitialDragCoeff);
    GetAttribute(itemNode, "fDownforceModifier", "value", handlingDataItem.fDownforceModifier, true);
    GetAttribute(itemNode, "fPopUpLightRotation", "value", handlingDataItem.fPopUpLightRotation, true);
    GetAttribute(itemNode, "fPercentSubmerged", "value", handlingDataItem.fPercentSubmerged);
    GetAttribute(itemNode, "vecCentreOfMassOffset", "x", handlingDataItem.vecCentreOfMassOffset.x);
    GetAttribute(itemNode, "vecCentreOfMassOffset", "y", handlingDataItem.vecCentreOfMassOffset.y);
    GetAttribute(itemNode, "vecCentreOfMassOffset", "z", handlingDataItem.vecCentreOfMassOffset.z);
    GetAttribute(itemNode, "vecInertiaMultiplier", "x", handlingDataItem.vecInertiaMultiplier.x);
    GetAttribute(itemNode, "vecInertiaMultiplier", "y", handlingDataItem.vecInertiaMultiplier.y);
    GetAttribute(itemNode, "vecInertiaMultiplier", "z", handlingDataItem.vecInertiaMultiplier.z);
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
    GetAttribute(itemNode, "fCamberStiffnesss", "value", handlingDataItem.fCamberStiffnesss);
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
    GetAttribute(itemNode, "fSeatOffsetDistX", "value", handlingDataItem.vecSeatOffsetDist.x);
    GetAttribute(itemNode, "fSeatOffsetDistY", "value", handlingDataItem.vecSeatOffsetDist.y);
    GetAttribute(itemNode, "fSeatOffsetDistZ", "value", handlingDataItem.vecSeatOffsetDist.z);
    GetAttribute(itemNode, "nMonetaryValue", "value", handlingDataItem.nMonetaryValue);
    GetAttribute(itemNode, "fRocketBoostCapacity", "value", handlingDataItem.fRocketBoostCapacity, true);
    GetAttribute(itemNode, "fBoostMaxSpeed", "value", handlingDataItem.fBoostMaxSpeed, true);

    handlingDataItem.strModelFlags = StoU32(GetElementStr(itemNode, "strModelFlags"), 16);
    handlingDataItem.strHandlingFlags = StoU32(GetElementStr(itemNode, "strHandlingFlags"), 16);
    handlingDataItem.strDamageFlags = StoU32(GetElementStr(itemNode, "strDamageFlags"), 16);
    handlingDataItem.AIHandling = StrUtil::joaat(GetElementStr(itemNode, "AIHandling").c_str());

    tinyxml2::XMLNode* shdNode = itemNode->FirstChildElement("SubHandlingData");
    if (shdNode) {
        logger.Write(DEBUG, fmt::format("[Parse] [{}] {}", fileName, "Parsing SubHandlingData"));

        tinyxml2::XMLNode* shdItemElement = shdNode->FirstChildElement("Item");
        while (shdItemElement != nullptr) {
            const char* type = nullptr;
            ((tinyxml2::XMLElement*)shdItemElement)->QueryStringAttribute("type", &type);

            if (std::string("CBikeHandlingData") == type) {
                ParseCBikeHandlingData(fileName, shdItemElement, handlingDataItem);
            }
            else if (std::string("CFlyingHandlingData") == type) {
                ParseCFlyingHandlingData(fileName, shdItemElement, handlingDataItem);
            }
            else if (std::string("CSpecialFlightHandlingData") == type) {
                ParseCSpecialFlightHandlingData(fileName, shdItemElement, handlingDataItem);
            }
            else if (std::string("CBoatHandlingData") == type) {
                ParseCBoatHandlingData(fileName, shdItemElement, handlingDataItem);
            }
            else if (std::string("CSeaPlaneHandlingData") == type) {
                ParseCSeaPlaneHandlingData(fileName, shdItemElement, handlingDataItem);
            }
            else if (std::string("CSubmarineHandlingData") == type) {
                ParseCSubmarineHandlingData(fileName, shdItemElement, handlingDataItem);
            }
            else if (std::string("CTrailerHandlingData") == type) {
                ParseCTrailerHandlingData(fileName, shdItemElement, handlingDataItem);
            }
            else if (std::string("CCarHandlingData") == type) {
                ParseCCarHandlingData(fileName, shdItemElement, handlingDataItem);
            }
            else if (std::string("NULL") == type) {
                // Skip dummy Item
            }
            else {
                const char* type_ = type != nullptr ? type : "nullptr";
                logger.Write(DEBUG, fmt::format("[Parse] [{}] {} {}", fileName, "Unknown SubHandlingData member", type_));
            }
            shdItemElement = shdItemElement->NextSibling();
        }
    }

    logger.Write(DEBUG, "[Parse] Finished parsing [%s]", fileName.c_str());

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
void InsertElement(tinyxml2::XMLNode* rootNode, const char* elemName, rage::Vec3 value) {
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

void SaveCBikeHandlingData(std::string& fileName, const RTHE::CHandlingDataItem& handlingDataItem, tinyxml2::XMLElement* shdElement) {
    for (const auto& subHandlingData : handlingDataItem.SubHandlingData.CBikeHandlingData) {
        logger.Write(DEBUG, fmt::format("[Write] [{}] {}", handlingDataItem.Metadata.HandlingName, "Writing CBikeHandlingData"));

        tinyxml2::XMLElement* subHandlingDataItem = shdElement->GetDocument()->NewElement("Item");
        subHandlingDataItem->SetAttribute("type", "CBikeHandlingData");

        InsertElement(subHandlingDataItem, "fLeanFwdCOMMult", subHandlingData.fLeanFwdCOMMult);
        InsertElement(subHandlingDataItem, "fLeanFwdForceMult", subHandlingData.fLeanFwdForceMult);
        InsertElement(subHandlingDataItem, "fLeanBakCOMMult", subHandlingData.fLeanBakCOMMult);
        InsertElement(subHandlingDataItem, "fLeanBakForceMult", subHandlingData.fLeanBakForceMult);
        InsertElement(subHandlingDataItem, "fMaxBankAngle", subHandlingData.fMaxBankAngle);
        InsertElement(subHandlingDataItem, "fFullAnimAngle", subHandlingData.fFullAnimAngle);
        InsertElement(subHandlingDataItem, "fDesLeanReturnFrac", subHandlingData.fDesLeanReturnFrac);
        InsertElement(subHandlingDataItem, "fStickLeanMult", subHandlingData.fStickLeanMult);
        InsertElement(subHandlingDataItem, "fBrakingStabilityMult", subHandlingData.fBrakingStabilityMult);
        InsertElement(subHandlingDataItem, "fInAirSteerMult", subHandlingData.fInAirSteerMult);
        InsertElement(subHandlingDataItem, "fWheelieBalancePoint", subHandlingData.fWheelieBalancePoint);
        InsertElement(subHandlingDataItem, "fStoppieBalancePoint", subHandlingData.fStoppieBalancePoint);
        InsertElement(subHandlingDataItem, "fWheelieSteerMult", subHandlingData.fWheelieSteerMult);
        InsertElement(subHandlingDataItem, "fRearBalanceMult", subHandlingData.fRearBalanceMult);
        InsertElement(subHandlingDataItem, "fFrontBalanceMult", subHandlingData.fFrontBalanceMult);
        InsertElement(subHandlingDataItem, "fBikeGroundSideFrictionMult", subHandlingData.fBikeGroundSideFrictionMult);
        InsertElement(subHandlingDataItem, "fBikeWheelGroundSideFrictionMult", subHandlingData.fBikeWheelGroundSideFrictionMult);
        InsertElement(subHandlingDataItem, "fBikeOnStandLeanAngle", subHandlingData.fBikeOnStandLeanAngle);
        InsertElement(subHandlingDataItem, "fBikeOnStandSteerAngle", subHandlingData.fBikeOnStandSteerAngle);
        InsertElement(subHandlingDataItem, "fJumpForce", subHandlingData.fJumpForce);

        shdElement->InsertEndChild(subHandlingDataItem);
    }
}

void SaveCFlyingHandlingData(std::string& fileName, const RTHE::CHandlingDataItem& handlingDataItem, tinyxml2::XMLElement* shdElement) {
    for (const auto& subHandlingData : handlingDataItem.SubHandlingData.CFlyingHandlingData) {
        logger.Write(DEBUG, fmt::format("[Write] [{}] {}", handlingDataItem.Metadata.HandlingName, "Writing CFlyingHandlingData"));

        tinyxml2::XMLElement* subHandlingDataItem = shdElement->GetDocument()->NewElement("Item");
        subHandlingDataItem->SetAttribute("type", "CFlyingHandlingData");

        InsertElement(subHandlingDataItem, "fThrust", subHandlingData.fThrust);
        InsertElement(subHandlingDataItem, "fThrustFallOff", subHandlingData.fThrustFallOff);
        InsertElement(subHandlingDataItem, "fThrustVectoring", subHandlingData.fThrustVectoring);
        InsertElement(subHandlingDataItem, "fInitialThrust", subHandlingData.fInitialThrust);
        InsertElement(subHandlingDataItem, "fInitialThrustFallOff", subHandlingData.fInitialThrustFallOff);
        InsertElement(subHandlingDataItem, "fYawMult", subHandlingData.fYawMult);
        InsertElement(subHandlingDataItem, "fYawStabilise", subHandlingData.fYawStabilise);
        InsertElement(subHandlingDataItem, "fSideSlipMult", subHandlingData.fSideSlipMult);
        InsertElement(subHandlingDataItem, "fInitialYawMult", subHandlingData.fInitialYawMult);
        InsertElement(subHandlingDataItem, "fRollMult", subHandlingData.fRollMult);
        InsertElement(subHandlingDataItem, "fRollStabilise", subHandlingData.fRollStabilise);
        InsertElement(subHandlingDataItem, "fInitialRollMult", subHandlingData.fInitialRollMult);
        InsertElement(subHandlingDataItem, "fPitchMult", subHandlingData.fPitchMult);
        InsertElement(subHandlingDataItem, "fPitchStabilise", subHandlingData.fPitchStabilise);
        InsertElement(subHandlingDataItem, "fInitialPitchMult", subHandlingData.fInitialPitchMult);
        InsertElement(subHandlingDataItem, "fFormLiftMult", subHandlingData.fFormLiftMult);
        InsertElement(subHandlingDataItem, "fAttackLiftMult", subHandlingData.fAttackLiftMult);
        InsertElement(subHandlingDataItem, "fAttackDiveMult", subHandlingData.fAttackDiveMult);
        InsertElement(subHandlingDataItem, "fGearDownDragV", subHandlingData.fGearDownDragV);
        InsertElement(subHandlingDataItem, "fGearDownLiftMult", subHandlingData.fGearDownLiftMult);
        InsertElement(subHandlingDataItem, "fWindMult", subHandlingData.fWindMult);
        InsertElement(subHandlingDataItem, "fMoveRes", subHandlingData.fMoveRes);
        InsertElement(subHandlingDataItem, "vecTurnRes", {
            subHandlingData.vecTurnRes.x,
            subHandlingData.vecTurnRes.y,
            subHandlingData.vecTurnRes.z
        });
        InsertElement(subHandlingDataItem, "vecSpeedRes", {
            subHandlingData.vecSpeedRes.x,
            subHandlingData.vecSpeedRes.y,
            subHandlingData.vecSpeedRes.z
        });
        InsertElement(subHandlingDataItem, "fGearDoorFrontOpen", subHandlingData.fGearDoorFrontOpen);
        InsertElement(subHandlingDataItem, "fGearDoorRearOpen", subHandlingData.fGearDoorRearOpen);
        InsertElement(subHandlingDataItem, "fGearDoorRearOpen2", subHandlingData.fGearDoorRearOpen2);
        InsertElement(subHandlingDataItem, "fGearDoorRearMOpen", subHandlingData.fGearDoorRearMOpen);
        InsertElement(subHandlingDataItem, "fTurublenceMagnitudeMax", subHandlingData.fTurublenceMagnitudeMax);
        InsertElement(subHandlingDataItem, "fTurublenceForceMulti", subHandlingData.fTurublenceForceMulti);
        InsertElement(subHandlingDataItem, "fTurublenceRollTorqueMulti", subHandlingData.fTurublenceRollTorqueMulti);
        InsertElement(subHandlingDataItem, "fTurublencePitchTorqueMulti", subHandlingData.fTurublencePitchTorqueMulti);
        InsertElement(subHandlingDataItem, "fBodyDamageControlEffectMult", subHandlingData.fBodyDamageControlEffectMult);
        InsertElement(subHandlingDataItem, "fInputSensitivityForDifficulty", subHandlingData.fInputSensitivityForDifficulty);
        InsertElement(subHandlingDataItem, "fOnGroundYawBoostSpeedPeak", subHandlingData.fOnGroundYawBoostSpeedPeak);
        InsertElement(subHandlingDataItem, "fOnGroundYawBoostSpeedCap", subHandlingData.fOnGroundYawBoostSpeedCap);
        InsertElement(subHandlingDataItem, "fEngineOffGlideMulti", subHandlingData.fEngineOffGlideMulti);
        InsertElement(subHandlingDataItem, "fAfterburnerEffectRadius", subHandlingData.fAfterburnerEffectRadius);
        InsertElement(subHandlingDataItem, "fAfterburnerEffectDistance", subHandlingData.fAfterburnerEffectDistance);
        InsertElement(subHandlingDataItem, "fAfterburnerEffectForceMulti", subHandlingData.fAfterburnerEffectForceMulti);
        InsertElement(subHandlingDataItem, "fSubmergeLevelToPullHeliUnderwater", subHandlingData.fSubmergeLevelToPullHeliUnderwater);
        InsertElement(subHandlingDataItem, "fExtraLiftWithRoll", subHandlingData.fExtraLiftWithRoll);

        std::string handlingTypeStr = "HANDLING_TYPE_UNKNOWN";
        
        if (subHandlingData.handlingType == RTHE::HANDLING_TYPE_BIKE) handlingTypeStr = "HANDLING_TYPE_BIKE";
        if (subHandlingData.handlingType == RTHE::HANDLING_TYPE_FLYING) handlingTypeStr = "HANDLING_TYPE_FLYING";
        if (subHandlingData.handlingType == RTHE::HANDLING_TYPE_VERTICAL_FLYING) handlingTypeStr = "HANDLING_TYPE_VERTICAL_FLYING";
        if (subHandlingData.handlingType == RTHE::HANDLING_TYPE_BOAT) handlingTypeStr = "HANDLING_TYPE_BOAT";
        if (subHandlingData.handlingType == RTHE::HANDLING_TYPE_SEAPLANE) handlingTypeStr = "HANDLING_TYPE_SEAPLANE";
        if (subHandlingData.handlingType == RTHE::HANDLING_TYPE_SUBMARINE) handlingTypeStr = "HANDLING_TYPE_SUBMARINE";
        if (subHandlingData.handlingType == RTHE::HANDLING_TYPE_TRAIN) handlingTypeStr = "HANDLING_TYPE_TRAIN";
        if (subHandlingData.handlingType == RTHE::HANDLING_TYPE_TRAILER) handlingTypeStr = "HANDLING_TYPE_TRAILER";
        if (subHandlingData.handlingType == RTHE::HANDLING_TYPE_CAR) handlingTypeStr = "HANDLING_TYPE_CAR";
        if (subHandlingData.handlingType == RTHE::HANDLING_TYPE_WEAPON) handlingTypeStr = "HANDLING_TYPE_WEAPON";
        if (subHandlingData.handlingType == RTHE::HANDLING_TYPE_SPECIALFLIGHT) handlingTypeStr = "HANDLING_TYPE_SPECIALFLIGHT";

        InsertElement(subHandlingDataItem, "handlingType", handlingTypeStr.c_str());

        shdElement->InsertEndChild(subHandlingDataItem);
    }
}

void SaveCSpecialFlightHandlingData(std::string& fileName, const RTHE::CHandlingDataItem& handlingDataItem, tinyxml2::XMLElement* shdElement) {
    for (const auto& subHandlingData : handlingDataItem.SubHandlingData.CSpecialFlightHandlingData) {
        logger.Write(DEBUG, fmt::format("[Write] [{}] {}", handlingDataItem.Metadata.HandlingName, "Writing CSpecialFlightHandlingData"));

        tinyxml2::XMLElement* subHandlingDataItem = shdElement->GetDocument()->NewElement("Item");
        subHandlingDataItem->SetAttribute("type", "CSpecialFlightHandlingData");

        InsertElement(subHandlingDataItem, "vecAngularDamping", {
            subHandlingData.vecAngularDamping.x,
            subHandlingData.vecAngularDamping.y,
            subHandlingData.vecAngularDamping.z
        });
        InsertElement(subHandlingDataItem, "vecAngularDampingMin", {
            subHandlingData.vecAngularDampingMin.x,
            subHandlingData.vecAngularDampingMin.y,
            subHandlingData.vecAngularDampingMin.z
        });
        InsertElement(subHandlingDataItem, "vecLinearDamping", {
            subHandlingData.vecLinearDamping.x,
            subHandlingData.vecLinearDamping.y,
            subHandlingData.vecLinearDamping.z
        });
        InsertElement(subHandlingDataItem, "vecLinearDampingMin", {
            subHandlingData.vecLinearDampingMin.x,
            subHandlingData.vecLinearDampingMin.y,
            subHandlingData.vecLinearDampingMin.z
        });
        InsertElement(subHandlingDataItem, "fLiftCoefficient", subHandlingData.fLiftCoefficient);
        InsertElement(subHandlingDataItem, "fCriticalLiftAngle", subHandlingData.fCriticalLiftAngle);
        InsertElement(subHandlingDataItem, "fInitialLiftAngle", subHandlingData.fInitialLiftAngle);
        InsertElement(subHandlingDataItem, "fMaxLiftAngle", subHandlingData.fMaxLiftAngle);
        InsertElement(subHandlingDataItem, "fDragCoefficient", subHandlingData.fDragCoefficient);
        InsertElement(subHandlingDataItem, "fBrakingDrag", subHandlingData.fBrakingDrag);
        InsertElement(subHandlingDataItem, "fMaxLiftVelocity", subHandlingData.fMaxLiftVelocity);
        InsertElement(subHandlingDataItem, "fMinLiftVelocity", subHandlingData.fMinLiftVelocity);
        InsertElement(subHandlingDataItem, "fRollTorqueScale", subHandlingData.fRollTorqueScale);
        InsertElement(subHandlingDataItem, "fMaxTorqueVelocity", subHandlingData.fMaxTorqueVelocity);
        InsertElement(subHandlingDataItem, "fMinTorqueVelocity", subHandlingData.fMinTorqueVelocity);
        InsertElement(subHandlingDataItem, "fYawTorqueScale", subHandlingData.fYawTorqueScale);
        InsertElement(subHandlingDataItem, "fSelfLevelingPitchTorqueScale", subHandlingData.fSelfLevelingPitchTorqueScale);
        InsertElement(subHandlingDataItem, "fInitalOverheadAssist", subHandlingData.fInitalOverheadAssist);
        InsertElement(subHandlingDataItem, "fMaxPitchTorque", subHandlingData.fMaxPitchTorque);
        InsertElement(subHandlingDataItem, "fMaxSteeringRollTorque", subHandlingData.fMaxSteeringRollTorque);
        InsertElement(subHandlingDataItem, "fPitchTorqueScale", subHandlingData.fPitchTorqueScale);
        InsertElement(subHandlingDataItem, "fSteeringTorqueScale", subHandlingData.fSteeringTorqueScale);
        InsertElement(subHandlingDataItem, "fMaxThrust", subHandlingData.fMaxThrust);
        InsertElement(subHandlingDataItem, "fTransitionDuration", subHandlingData.fTransitionDuration);
        InsertElement(subHandlingDataItem, "fHoverVelocityScale", subHandlingData.fHoverVelocityScale);
        InsertElement(subHandlingDataItem, "fStabilityAssist", subHandlingData.fStabilityAssist);
        InsertElement(subHandlingDataItem, "fMinSpeedForThrustFalloff", subHandlingData.fMinSpeedForThrustFalloff);
        InsertElement(subHandlingDataItem, "fBrakingThrustScale", subHandlingData.fBrakingThrustScale);
        InsertElement(subHandlingDataItem, "mode", subHandlingData.mode);
        InsertElement(subHandlingDataItem, "strFlags", fmt::format("{:08X}", subHandlingData.strFlags).c_str());

        shdElement->InsertEndChild(subHandlingDataItem);
    }
}

void SaveCBoatHandlingData(std::string& fileName, const RTHE::CHandlingDataItem& handlingDataItem, tinyxml2::XMLElement* shdElement) {
    for (const auto& subHandlingData : handlingDataItem.SubHandlingData.CBoatHandlingData) {
        logger.Write(DEBUG, fmt::format("[Write] [{}] {}", handlingDataItem.Metadata.HandlingName, "Writing CBoatHandlingData"));

        tinyxml2::XMLElement* subHandlingDataItem = shdElement->GetDocument()->NewElement("Item");
        subHandlingDataItem->SetAttribute("type", "CBoatHandlingData");

        InsertElement(subHandlingDataItem, "fBoxFrontMult", subHandlingData.fBoxFrontMult);
        InsertElement(subHandlingDataItem, "fBoxRearMult", subHandlingData.fBoxRearMult);
        InsertElement(subHandlingDataItem, "fBoxSideMult", subHandlingData.fBoxSideMult);
        InsertElement(subHandlingDataItem, "fSampleTop", subHandlingData.fSampleTop);
        InsertElement(subHandlingDataItem, "fSampleBottom", subHandlingData.fSampleBottom);
        InsertElement(subHandlingDataItem, "fSampleBottomTestCorrection", subHandlingData.fSampleBottomTestCorrection);
        InsertElement(subHandlingDataItem, "fAquaplaneForce", subHandlingData.fAquaplaneForce);
        InsertElement(subHandlingDataItem, "fAquaplanePushWaterMult", subHandlingData.fAquaplanePushWaterMult);
        InsertElement(subHandlingDataItem, "fAquaplanePushWaterCap", subHandlingData.fAquaplanePushWaterCap);
        InsertElement(subHandlingDataItem, "fAquaplanePushWaterApply", subHandlingData.fAquaplanePushWaterApply);
        InsertElement(subHandlingDataItem, "fRudderForce", subHandlingData.fRudderForce);
        InsertElement(subHandlingDataItem, "fRudderOffsetSubmerge", subHandlingData.fRudderOffsetSubmerge);
        InsertElement(subHandlingDataItem, "fRudderOffsetForce", subHandlingData.fRudderOffsetForce);
        InsertElement(subHandlingDataItem, "fRudderOffsetForceZMult", subHandlingData.fRudderOffsetForceZMult);
        InsertElement(subHandlingDataItem, "fWaveAudioMult", subHandlingData.fWaveAudioMult);
        InsertElement(subHandlingDataItem, "vecMoveResistance", {
            subHandlingData.vecMoveResistance.x,
            subHandlingData.vecMoveResistance.y,
            subHandlingData.vecMoveResistance.z
            });
        InsertElement(subHandlingDataItem, "vecTurnResistance", {
            subHandlingData.vecTurnResistance.x,
            subHandlingData.vecTurnResistance.y,
            subHandlingData.vecTurnResistance.z
            });
        InsertElement(subHandlingDataItem, "fLook_L_R_CamHeight", subHandlingData.fLook_L_R_CamHeight);
        InsertElement(subHandlingDataItem, "fDragCoefficient", subHandlingData.fDragCoefficient);
        InsertElement(subHandlingDataItem, "fKeelSphereSize", subHandlingData.fKeelSphereSize);
        InsertElement(subHandlingDataItem, "fPropRadius", subHandlingData.fPropRadius);
        InsertElement(subHandlingDataItem, "fLowLodAngOffset", subHandlingData.fLowLodAngOffset);
        InsertElement(subHandlingDataItem, "fLowLodDraughtOffset", subHandlingData.fLowLodDraughtOffset);
        InsertElement(subHandlingDataItem, "fImpellerOffset", subHandlingData.fImpellerOffset);
        InsertElement(subHandlingDataItem, "fImpellerForceMult", subHandlingData.fImpellerForceMult);
        InsertElement(subHandlingDataItem, "fDinghySphereBuoyConst", subHandlingData.fDinghySphereBuoyConst);
        InsertElement(subHandlingDataItem, "fProwRaiseMult", subHandlingData.fProwRaiseMult);
        InsertElement(subHandlingDataItem, "fDeepWaterSampleBuoyancyMult", subHandlingData.fDeepWaterSampleBuoyancyMult);
        InsertElement(subHandlingDataItem, "fTransmissionMultiplier", subHandlingData.fTransmissionMultiplier);
        InsertElement(subHandlingDataItem, "fTractionMultiplier", subHandlingData.fTractionMultiplier);

        shdElement->InsertEndChild(subHandlingDataItem);
    }
}

void SaveCSeaPlaneHandlingData(std::string& fileName, const RTHE::CHandlingDataItem& handlingDataItem, tinyxml2::XMLElement* shdElement) {
    for (const auto& subHandlingData : handlingDataItem.SubHandlingData.CSeaPlaneHandlingData) {
        logger.Write(DEBUG, fmt::format("[Write] [{}] {}", handlingDataItem.Metadata.HandlingName, "Writing CSeaPlaneHandlingData"));

        tinyxml2::XMLElement* subHandlingDataItem = shdElement->GetDocument()->NewElement("Item");
        subHandlingDataItem->SetAttribute("type", "CSeaPlaneHandlingData");

        InsertElement(subHandlingDataItem, "fLeftPontoonComponentId", subHandlingData.fLeftPontoonComponentId);
        InsertElement(subHandlingDataItem, "fRightPontoonComponentId", subHandlingData.fRightPontoonComponentId);
        InsertElement(subHandlingDataItem, "fPontoonBuoyConst", subHandlingData.fPontoonBuoyConst);
        InsertElement(subHandlingDataItem, "fPontoonSampleSizeFront", subHandlingData.fPontoonSampleSizeFront);
        InsertElement(subHandlingDataItem, "fPontoonSampleSizeMiddle", subHandlingData.fPontoonSampleSizeMiddle);
        InsertElement(subHandlingDataItem, "fPontoonSampleSizeRear", subHandlingData.fPontoonSampleSizeRear);
        InsertElement(subHandlingDataItem, "fPontoonLengthFractionForSamples", subHandlingData.fPontoonLengthFractionForSamples);
        InsertElement(subHandlingDataItem, "fPontoonDragCoefficient", subHandlingData.fPontoonDragCoefficient);
        InsertElement(subHandlingDataItem, "fPontoonVerticalDampingCoefficientUp", subHandlingData.fPontoonVerticalDampingCoefficientUp);
        InsertElement(subHandlingDataItem, "fPontoonVerticalDampingCoefficientDown", subHandlingData.fPontoonVerticalDampingCoefficientDown);
        InsertElement(subHandlingDataItem, "fKeelSphereSize", subHandlingData.fKeelSphereSize);

        shdElement->InsertEndChild(subHandlingDataItem);
    }
}

void SaveCSubmarineHandlingData(std::string& fileName, const RTHE::CHandlingDataItem& handlingDataItem, tinyxml2::XMLElement* shdElement) {
    for (const auto& subHandlingData : handlingDataItem.SubHandlingData.CSubmarineHandlingData) {
        logger.Write(DEBUG, fmt::format("[Write] [{}] {}", handlingDataItem.Metadata.HandlingName, "Writing CSubmarineHandlingData"));

        tinyxml2::XMLElement* subHandlingDataItem = shdElement->GetDocument()->NewElement("Item");
        subHandlingDataItem->SetAttribute("type", "CSubmarineHandlingData");

        InsertElement(subHandlingDataItem, "vTurnRes", {
            subHandlingData.vTurnRes.x,
            subHandlingData.vTurnRes.y,
            subHandlingData.vTurnRes.z
        });

        InsertElement(subHandlingDataItem, "fMoveResXY", subHandlingData.fMoveResXY);
        InsertElement(subHandlingDataItem, "fMoveResZ", subHandlingData.fMoveResZ);
        InsertElement(subHandlingDataItem, "fPitchMult", subHandlingData.fPitchMult);
        InsertElement(subHandlingDataItem, "fPitchAngle", subHandlingData.fPitchAngle);
        InsertElement(subHandlingDataItem, "fYawMult", subHandlingData.fYawMult);
        InsertElement(subHandlingDataItem, "fDiveSpeed", subHandlingData.fDiveSpeed);
        InsertElement(subHandlingDataItem, "fRollMult", subHandlingData.fRollMult);
        InsertElement(subHandlingDataItem, "fRollStab", subHandlingData.fRollStab);

        shdElement->InsertEndChild(subHandlingDataItem);
    }
}

void SaveCTrailerHandlingData(std::string& fileName, const RTHE::CHandlingDataItem& handlingDataItem, tinyxml2::XMLElement* shdElement) {
    for (const auto& subHandlingData : handlingDataItem.SubHandlingData.CTrailerHandlingData) {
        logger.Write(DEBUG, fmt::format("[Write] [{}] {}", handlingDataItem.Metadata.HandlingName, "Writing CTrailerHandlingData"));

        tinyxml2::XMLElement* subHandlingDataItem = shdElement->GetDocument()->NewElement("Item");
        subHandlingDataItem->SetAttribute("type", "CTrailerHandlingData");

        InsertElement(subHandlingDataItem, "fAttachLimitPitch", subHandlingData.fAttachLimitPitch);
        InsertElement(subHandlingDataItem, "fAttachLimitRoll", subHandlingData.fAttachLimitRoll);
        InsertElement(subHandlingDataItem, "fAttachLimitYaw", subHandlingData.fAttachLimitYaw);
        InsertElement(subHandlingDataItem, "fUprightSpringConstant", subHandlingData.fUprightSpringConstant);
        InsertElement(subHandlingDataItem, "fUprightDampingConstant", subHandlingData.fUprightDampingConstant);
        InsertElement(subHandlingDataItem, "fAttachedMaxDistance", subHandlingData.fAttachedMaxDistance);
        InsertElement(subHandlingDataItem, "fAttachedMaxPenetration", subHandlingData.fAttachedMaxPenetration);
        InsertElement(subHandlingDataItem, "fAttachRaiseZ", subHandlingData.fAttachRaiseZ);
        InsertElement(subHandlingDataItem, "fPosConstraintMassRatio", subHandlingData.fPosConstraintMassRatio);

        shdElement->InsertEndChild(subHandlingDataItem);
    }
}

void SaveCCarHandlingData(std::string& fileName, const RTHE::CHandlingDataItem& handlingDataItem, tinyxml2::XMLElement* shdElement) {
    for (const auto& carHandlingData : handlingDataItem.SubHandlingData.CCarHandlingData) {
        logger.Write(DEBUG, fmt::format("[Write] [{}] {}", handlingDataItem.Metadata.HandlingName, "Writing CCarHandlingData"));

        tinyxml2::XMLElement* subHandlingDataItem = shdElement->GetDocument()->NewElement("Item");
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
        InsertElement(subHandlingDataItem, "fIncreasedRammingForceScale", carHandlingData.fIncreasedRammingForceScale);
        InsertElement(subHandlingDataItem, "strAdvancedFlags", fmt::format("{:08X}", carHandlingData.strAdvancedFlags).c_str());

        if (carHandlingData.AdvancedData.size() > 0) {
            auto advancedDataElement = subHandlingDataItem->GetDocument()->NewElement("AdvancedData");
            for (uint32_t i = 0; i < carHandlingData.AdvancedData.size(); ++i) {
                auto advancedDataItemElement = advancedDataElement->GetDocument()->NewElement("Item");
                advancedDataItemElement->SetAttribute("type", "CAdvancedData");

                InsertElement(advancedDataItemElement, "Slot", carHandlingData.AdvancedData[i].Slot);
                InsertElement(advancedDataItemElement, "Index", carHandlingData.AdvancedData[i].Index);
                InsertElement(advancedDataItemElement, "Value", carHandlingData.AdvancedData[i].Value);

                advancedDataElement->InsertEndChild(advancedDataItemElement);
            }
            subHandlingDataItem->InsertEndChild(advancedDataElement);
        }

        shdElement->InsertEndChild(subHandlingDataItem);
    }
}

bool RTHE::SaveXMLItem(const CHandlingDataItem& handlingDataItem, const std::string& targetFile) {
    tinyxml2::XMLDocument doc{};
    tinyxml2::XMLError err = tinyxml2::XMLError::XML_SUCCESS;

    std::string fileName = fs::path(targetFile).filename().string();
    logger.Write(DEBUG, "[Write] Writing handling file [%s]", fileName.c_str());
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

    InsertElement(itemNode, "handlingName", handlingDataItem.Metadata.HandlingName.c_str());
    InsertElement(itemNode, "fMass", handlingDataItem.fMass);
    InsertElement(itemNode, "fInitialDragCoeff", handlingDataItem.fInitialDragCoeff);
    if (handlingDataItem.fDownforceModifier != 0.0f)
        InsertElement(itemNode, "fDownforceModifier", handlingDataItem.fDownforceModifier);
    if (handlingDataItem.fPopUpLightRotation != 0.0f)
        InsertElement(itemNode, "fPopUpLightRotation", handlingDataItem.fPopUpLightRotation);
    InsertElement(itemNode, "fPercentSubmerged", handlingDataItem.fPercentSubmerged);
    InsertElement(itemNode, "vecCentreOfMassOffset", {
        handlingDataItem.vecCentreOfMassOffset.x,
        handlingDataItem.vecCentreOfMassOffset.y,
        handlingDataItem.vecCentreOfMassOffset.z
    });
    InsertElement(itemNode, "vecInertiaMultiplier", { 
        handlingDataItem.vecInertiaMultiplier.x,
        handlingDataItem.vecInertiaMultiplier.y,
        handlingDataItem.vecInertiaMultiplier.z
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
    InsertElement(itemNode, "fCamberStiffnesss", handlingDataItem.fCamberStiffnesss);
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
    InsertElement(itemNode, "fSeatOffsetDistX", handlingDataItem.vecSeatOffsetDist.x);
    InsertElement(itemNode, "fSeatOffsetDistY", handlingDataItem.vecSeatOffsetDist.y);
    InsertElement(itemNode, "fSeatOffsetDistZ", handlingDataItem.vecSeatOffsetDist.z);
    InsertElement(itemNode, "nMonetaryValue", handlingDataItem.nMonetaryValue);
    InsertElement(itemNode, "fRocketBoostCapacity", handlingDataItem.fRocketBoostCapacity);
    InsertElement(itemNode, "fBoostMaxSpeed", handlingDataItem.fBoostMaxSpeed);

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

    if (handlingDataItem.SubHandlingData.CBikeHandlingData.empty() &&
        handlingDataItem.SubHandlingData.CFlyingHandlingData.empty() &&
        handlingDataItem.SubHandlingData.CSpecialFlightHandlingData.empty() &&
        handlingDataItem.SubHandlingData.CBoatHandlingData.empty() &&
        handlingDataItem.SubHandlingData.CSeaPlaneHandlingData.empty() &&
        handlingDataItem.SubHandlingData.CSubmarineHandlingData.empty() &&
        handlingDataItem.SubHandlingData.CTrailerHandlingData.empty() &&
        handlingDataItem.SubHandlingData.CCarHandlingData.empty()) {

        logger.Write(DEBUG, fmt::format("[Write] [{}] {}", fileName, "No SubHandlingData"));
        InsertElementSubHandlingNULL(itemNode);
    }
    else {
        tinyxml2::XMLElement* shdElement = itemNode->GetDocument()->NewElement("SubHandlingData");

        SaveCBikeHandlingData(fileName, handlingDataItem, shdElement);
        SaveCFlyingHandlingData(fileName, handlingDataItem, shdElement);
        SaveCSpecialFlightHandlingData(fileName, handlingDataItem, shdElement);
        SaveCBoatHandlingData(fileName, handlingDataItem, shdElement);
        SaveCSeaPlaneHandlingData(fileName, handlingDataItem, shdElement);
        SaveCSubmarineHandlingData(fileName, handlingDataItem, shdElement);
        SaveCTrailerHandlingData(fileName, handlingDataItem, shdElement);
        SaveCCarHandlingData(fileName, handlingDataItem, shdElement);

        itemNode->InsertEndChild(shdElement);
    }

    err = doc.SaveFile(targetFile.c_str());

    if (err != tinyxml2::XMLError::XML_SUCCESS) {
        logger.Write(ERROR, "[Write] Can't load file [%s]", targetFile.c_str());
        logger.Write(ERROR, "[Write] Error details: %s", GetXMLError(doc).c_str());
        return false;
    }

    logger.Write(DEBUG, "[Write] Finished writing [%s]", fileName.c_str());

    return true;
}
