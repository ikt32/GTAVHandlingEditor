#include "HandlingConversion.h"

#include "Memory/VehicleExtensions.hpp"
#include "Util/AddonSpawnerCache.h"
#include "Util/Logger.hpp"
#include "Util/MathExt.h"

#include <inc/natives.h>
#include <fmt/format.h>

using VExt = VehicleExtensions;

void setCBikeHandlingData(RTHE::CBaseSubHandlingData* baseSubHandlingData, const RTHE::CHandlingDataItem& handlingDataItem) {
    if (handlingDataItem.SubHandlingData.CBikeHandlingData.empty())
        return;

    const auto& subHandlingDataItem = handlingDataItem.SubHandlingData.CBikeHandlingData[0];
    RTHE::CBikeHandlingData* subHandlingData = reinterpret_cast<RTHE::CBikeHandlingData*>(baseSubHandlingData);

    subHandlingData->fLeanFwdCOMMult = subHandlingDataItem.fLeanFwdCOMMult;
    subHandlingData->fLeanFwdForceMult = subHandlingDataItem.fLeanFwdForceMult;
    subHandlingData->fLeanBakCOMMult = subHandlingDataItem.fLeanBakCOMMult;
    subHandlingData->fLeanBakForceMult = subHandlingDataItem.fLeanBakForceMult;
    subHandlingData->fMaxBankAngle = deg2rad(subHandlingDataItem.fMaxBankAngle);
    subHandlingData->fFullAnimAngle = deg2rad(subHandlingDataItem.fFullAnimAngle);
    subHandlingData->fDesLeanReturnFrac = subHandlingDataItem.fDesLeanReturnFrac;
    subHandlingData->fStickLeanMult = subHandlingDataItem.fStickLeanMult;
    subHandlingData->fBrakingStabilityMult = subHandlingDataItem.fBrakingStabilityMult;
    subHandlingData->fInAirSteerMult = subHandlingDataItem.fInAirSteerMult;
    subHandlingData->fWheelieBalancePoint = deg2rad(subHandlingDataItem.fWheelieBalancePoint);
    subHandlingData->fStoppieBalancePoint = deg2rad(subHandlingDataItem.fStoppieBalancePoint);
    subHandlingData->fWheelieSteerMult = subHandlingDataItem.fWheelieSteerMult;
    subHandlingData->fRearBalanceMult = subHandlingDataItem.fRearBalanceMult;
    subHandlingData->fFrontBalanceMult = subHandlingDataItem.fFrontBalanceMult;
    subHandlingData->fBikeGroundSideFrictionMult = subHandlingDataItem.fBikeGroundSideFrictionMult;
    subHandlingData->fBikeWheelGroundSideFrictionMult = subHandlingDataItem.fBikeWheelGroundSideFrictionMult;
    subHandlingData->fBikeOnStandLeanAngle = deg2rad(subHandlingDataItem.fBikeOnStandLeanAngle);
    subHandlingData->fBikeOnStandSteerAngle = subHandlingDataItem.fBikeOnStandSteerAngle;
    subHandlingData->fJumpForce = subHandlingDataItem.fJumpForce;
}

void setCFlyingHandlingData(RTHE::CBaseSubHandlingData* baseSubHandlingData, const RTHE::CHandlingDataItem& handlingDataItem) {
    if (handlingDataItem.SubHandlingData.CFlyingHandlingData.empty())
        return;

    // Only apply matching subHandlingData->handlingType
    RTHE::CFlyingHandlingData* subHandlingData = reinterpret_cast<RTHE::CFlyingHandlingData*>(baseSubHandlingData);

    const RTHE::CFlyingHandlingDataItem* matchingCFlyingHandlingDataItem = nullptr;
    for (const auto& flyingHandlingDataItem : handlingDataItem.SubHandlingData.CFlyingHandlingData) {
        if (flyingHandlingDataItem.handlingType == subHandlingData->handlingType) {
            matchingCFlyingHandlingDataItem = &flyingHandlingDataItem;
            break;
        }
    }

    if (!matchingCFlyingHandlingDataItem)
        return;

    const auto& subHandlingDataItem = *matchingCFlyingHandlingDataItem;

    subHandlingData->fThrust = subHandlingDataItem.fThrust;
    subHandlingData->fThrustFallOff = subHandlingDataItem.fThrustFallOff;
    subHandlingData->fThrustVectoring = subHandlingDataItem.fThrustVectoring;
    subHandlingData->fInitialThrust = subHandlingDataItem.fInitialThrust;
    subHandlingData->fInitialThrustFallOff = subHandlingDataItem.fInitialThrustFallOff;
    subHandlingData->fYawMult = subHandlingDataItem.fYawMult;
    subHandlingData->fYawStabilise = subHandlingDataItem.fYawStabilise;
    subHandlingData->fSideSlipMult = subHandlingDataItem.fSideSlipMult;
    subHandlingData->fInitialYawMult = subHandlingDataItem.fInitialYawMult;
    subHandlingData->fRollMult = subHandlingDataItem.fRollMult;
    subHandlingData->fRollStabilise = subHandlingDataItem.fRollStabilise;
    subHandlingData->fInitialRollMult = subHandlingDataItem.fInitialRollMult;
    subHandlingData->fPitchMult = subHandlingDataItem.fPitchMult;
    subHandlingData->fPitchStabilise = subHandlingDataItem.fPitchStabilise;
    subHandlingData->fInitialPitchMult = subHandlingDataItem.fInitialPitchMult;
    subHandlingData->fFormLiftMult = subHandlingDataItem.fFormLiftMult;
    subHandlingData->fAttackLiftMult = subHandlingDataItem.fAttackLiftMult;
    subHandlingData->fAttackDiveMult = subHandlingDataItem.fAttackDiveMult;
    subHandlingData->fGearDownDragV = subHandlingDataItem.fGearDownDragV;
    subHandlingData->fGearDownLiftMult = subHandlingDataItem.fGearDownLiftMult;
    subHandlingData->fWindMult = subHandlingDataItem.fWindMult;
    subHandlingData->fMoveRes = subHandlingDataItem.fMoveRes;
    subHandlingData->vecTurnRes = subHandlingDataItem.vecTurnRes;
    subHandlingData->vecSpeedRes = subHandlingDataItem.vecSpeedRes;
    subHandlingData->fGearDoorFrontOpen = deg2rad(subHandlingDataItem.fGearDoorFrontOpen);
    subHandlingData->fGearDoorRearOpen = deg2rad(subHandlingDataItem.fGearDoorRearOpen);
    subHandlingData->fGearDoorRearOpen2 = deg2rad(subHandlingDataItem.fGearDoorRearOpen2);
    subHandlingData->fGearDoorRearMOpen = deg2rad(subHandlingDataItem.fGearDoorRearMOpen);
    subHandlingData->fTurublenceMagnitudeMax = subHandlingDataItem.fTurublenceMagnitudeMax;
    subHandlingData->fTurublenceForceMulti = subHandlingDataItem.fTurublenceForceMulti;
    subHandlingData->fTurublenceRollTorqueMulti = subHandlingDataItem.fTurublenceRollTorqueMulti;
    subHandlingData->fTurublencePitchTorqueMulti = subHandlingDataItem.fTurublencePitchTorqueMulti;
    subHandlingData->fBodyDamageControlEffectMult = subHandlingDataItem.fBodyDamageControlEffectMult;
    subHandlingData->fInputSensitivityForDifficulty = subHandlingDataItem.fInputSensitivityForDifficulty;
    subHandlingData->fOnGroundYawBoostSpeedPeak = subHandlingDataItem.fOnGroundYawBoostSpeedPeak;
    subHandlingData->fOnGroundYawBoostSpeedCap = subHandlingDataItem.fOnGroundYawBoostSpeedCap;
    subHandlingData->fEngineOffGlideMulti = subHandlingDataItem.fEngineOffGlideMulti;
    subHandlingData->fAfterburnerEffectRadius = subHandlingDataItem.fAfterburnerEffectRadius;
    subHandlingData->fAfterburnerEffectDistance = subHandlingDataItem.fAfterburnerEffectDistance;
    subHandlingData->fAfterburnerEffectForceMulti = subHandlingDataItem.fAfterburnerEffectForceMulti;
    subHandlingData->fSubmergeLevelToPullHeliUnderwater = subHandlingDataItem.fSubmergeLevelToPullHeliUnderwater;
    subHandlingData->fExtraLiftWithRoll = subHandlingDataItem.fExtraLiftWithRoll;
    subHandlingData->handlingType = subHandlingDataItem.handlingType;
}

void setCSpecialFlightHandlingData(RTHE::CBaseSubHandlingData* baseSubHandlingData, const RTHE::CHandlingDataItem& handlingDataItem) {
    if (handlingDataItem.SubHandlingData.CSpecialFlightHandlingData.empty())
        return;

    const auto& subHandlingDataItem = handlingDataItem.SubHandlingData.CSpecialFlightHandlingData[0];
    RTHE::CSpecialFlightHandlingData* subHandlingData = reinterpret_cast<RTHE::CSpecialFlightHandlingData*>(baseSubHandlingData);

    subHandlingData->vecAngularDamping = subHandlingDataItem.vecAngularDamping;
    subHandlingData->vecAngularDampingMin = subHandlingDataItem.vecAngularDampingMin;
    subHandlingData->vecLinearDamping = subHandlingDataItem.vecLinearDamping;
    subHandlingData->vecLinearDampingMin = subHandlingDataItem.vecLinearDampingMin;
    subHandlingData->fLiftCoefficient = subHandlingDataItem.fLiftCoefficient;
    subHandlingData->fCriticalLiftAngle = subHandlingDataItem.fCriticalLiftAngle;
    subHandlingData->fInitialLiftAngle = subHandlingDataItem.fInitialLiftAngle;
    subHandlingData->fMaxLiftAngle = subHandlingDataItem.fMaxLiftAngle;
    subHandlingData->fDragCoefficient = subHandlingDataItem.fDragCoefficient;
    subHandlingData->fBrakingDrag = subHandlingDataItem.fBrakingDrag;
    subHandlingData->fMaxLiftVelocity = subHandlingDataItem.fMaxLiftVelocity;
    subHandlingData->fMinLiftVelocity = subHandlingDataItem.fMinLiftVelocity;
    subHandlingData->fRollTorqueScale = subHandlingDataItem.fRollTorqueScale;
    subHandlingData->fMaxTorqueVelocity = subHandlingDataItem.fMaxTorqueVelocity;
    subHandlingData->fMinTorqueVelocity = subHandlingDataItem.fMinTorqueVelocity;
    subHandlingData->fYawTorqueScale = subHandlingDataItem.fYawTorqueScale;
    subHandlingData->fSelfLevelingPitchTorqueScale = subHandlingDataItem.fSelfLevelingPitchTorqueScale;
    subHandlingData->fInitalOverheadAssist = subHandlingDataItem.fInitalOverheadAssist;
    subHandlingData->fMaxPitchTorque = subHandlingDataItem.fMaxPitchTorque;
    subHandlingData->fMaxSteeringRollTorque = subHandlingDataItem.fMaxSteeringRollTorque;
    subHandlingData->fPitchTorqueScale = subHandlingDataItem.fPitchTorqueScale;
    subHandlingData->fSteeringTorqueScale = subHandlingDataItem.fSteeringTorqueScale;
    subHandlingData->fMaxThrust = subHandlingDataItem.fMaxThrust;
    subHandlingData->fTransitionDuration = subHandlingDataItem.fTransitionDuration;
    subHandlingData->fHoverVelocityScale = subHandlingDataItem.fHoverVelocityScale;
    subHandlingData->fStabilityAssist = subHandlingDataItem.fStabilityAssist;
    subHandlingData->fMinSpeedForThrustFalloff = subHandlingDataItem.fMinSpeedForThrustFalloff;
    subHandlingData->fBrakingThrustScale = subHandlingDataItem.fBrakingThrustScale;
    subHandlingData->mode = subHandlingDataItem.mode;
    subHandlingData->strFlags = subHandlingDataItem.strFlags;
}

void setCBoatHandlingData(RTHE::CBaseSubHandlingData* baseSubHandlingData, const RTHE::CHandlingDataItem& handlingDataItem) {
    if (handlingDataItem.SubHandlingData.CBoatHandlingData.empty())
        return;

    const auto& subHandlingDataItem = handlingDataItem.SubHandlingData.CBoatHandlingData[0];
    RTHE::CBoatHandlingData* subHandlingData = reinterpret_cast<RTHE::CBoatHandlingData*>(baseSubHandlingData);

    subHandlingData->fBoxFrontMult = subHandlingDataItem.fBoxFrontMult;
    subHandlingData->fBoxRearMult = subHandlingDataItem.fBoxRearMult;
    subHandlingData->fBoxSideMult = subHandlingDataItem.fBoxSideMult;
    subHandlingData->fSampleTop = subHandlingDataItem.fSampleTop;
    subHandlingData->fSampleBottom = subHandlingDataItem.fSampleBottom;
    subHandlingData->fSampleBottomTestCorrection = subHandlingDataItem.fSampleBottomTestCorrection;
    subHandlingData->fAquaplaneForce = subHandlingDataItem.fAquaplaneForce;
    subHandlingData->fAquaplanePushWaterMult = subHandlingDataItem.fAquaplanePushWaterMult;
    subHandlingData->fAquaplanePushWaterCap = subHandlingDataItem.fAquaplanePushWaterCap;
    subHandlingData->fAquaplanePushWaterApply = subHandlingDataItem.fAquaplanePushWaterApply;
    subHandlingData->fRudderForce = subHandlingDataItem.fRudderForce;
    subHandlingData->fRudderOffsetSubmerge = subHandlingDataItem.fRudderOffsetSubmerge;
    subHandlingData->fRudderOffsetForce = subHandlingDataItem.fRudderOffsetForce;
    subHandlingData->fRudderOffsetForceZMult = subHandlingDataItem.fRudderOffsetForceZMult;
    subHandlingData->fWaveAudioMult = subHandlingDataItem.fWaveAudioMult;
    subHandlingData->vecMoveResistance = subHandlingDataItem.vecMoveResistance;
    subHandlingData->vecTurnResistance = subHandlingDataItem.vecTurnResistance;
    subHandlingData->fLook_L_R_CamHeight = subHandlingDataItem.fLook_L_R_CamHeight;
    subHandlingData->fDragCoefficient = subHandlingDataItem.fDragCoefficient;
    subHandlingData->fKeelSphereSize = subHandlingDataItem.fKeelSphereSize;
    subHandlingData->fPropRadius = subHandlingDataItem.fPropRadius;
    subHandlingData->fLowLodAngOffset = subHandlingDataItem.fLowLodAngOffset;
    subHandlingData->fLowLodDraughtOffset = subHandlingDataItem.fLowLodDraughtOffset;
    subHandlingData->fImpellerOffset = subHandlingDataItem.fImpellerOffset;
    subHandlingData->fImpellerForceMult = subHandlingDataItem.fImpellerForceMult;
    subHandlingData->fDinghySphereBuoyConst = subHandlingDataItem.fDinghySphereBuoyConst;
    subHandlingData->fProwRaiseMult = subHandlingDataItem.fProwRaiseMult;
    subHandlingData->fDeepWaterSampleBuoyancyMult = subHandlingDataItem.fDeepWaterSampleBuoyancyMult;
    subHandlingData->fTransmissionMultiplier = subHandlingDataItem.fTransmissionMultiplier;
    subHandlingData->fTractionMultiplier = subHandlingDataItem.fTractionMultiplier;
}

void setCSeaPlaneHandlingData(RTHE::CBaseSubHandlingData* baseSubHandlingData, const RTHE::CHandlingDataItem& handlingDataItem) {
    if (handlingDataItem.SubHandlingData.CSeaPlaneHandlingData.empty())
        return;

    const auto& subHandlingDataItem = handlingDataItem.SubHandlingData.CSeaPlaneHandlingData[0];
    RTHE::CSeaPlaneHandlingData* subHandlingData = reinterpret_cast<RTHE::CSeaPlaneHandlingData*>(baseSubHandlingData);

    subHandlingData->fLeftPontoonComponentId = subHandlingDataItem.fLeftPontoonComponentId;
    subHandlingData->fRightPontoonComponentId = subHandlingDataItem.fRightPontoonComponentId;
    subHandlingData->fPontoonBuoyConst = subHandlingDataItem.fPontoonBuoyConst;
    subHandlingData->fPontoonSampleSizeFront = subHandlingDataItem.fPontoonSampleSizeFront;
    subHandlingData->fPontoonSampleSizeMiddle = subHandlingDataItem.fPontoonSampleSizeMiddle;
    subHandlingData->fPontoonSampleSizeRear = subHandlingDataItem.fPontoonSampleSizeRear;
    subHandlingData->fPontoonLengthFractionForSamples = subHandlingDataItem.fPontoonLengthFractionForSamples;
    subHandlingData->fPontoonDragCoefficient = subHandlingDataItem.fPontoonDragCoefficient;
    subHandlingData->fPontoonVerticalDampingCoefficientUp = subHandlingDataItem.fPontoonVerticalDampingCoefficientUp;
    subHandlingData->fPontoonVerticalDampingCoefficientDown = subHandlingDataItem.fPontoonVerticalDampingCoefficientDown;
    subHandlingData->fKeelSphereSize = subHandlingDataItem.fKeelSphereSize;
}

void setCSubmarineHandlingData(RTHE::CBaseSubHandlingData* baseSubHandlingData, const RTHE::CHandlingDataItem& handlingDataItem) {
    if (handlingDataItem.SubHandlingData.CSubmarineHandlingData.empty())
        return;

    const auto& subHandlingDataItem = handlingDataItem.SubHandlingData.CSubmarineHandlingData[0];
    RTHE::CSubmarineHandlingData* subHandlingData = reinterpret_cast<RTHE::CSubmarineHandlingData*>(baseSubHandlingData);

    subHandlingData->vTurnRes = subHandlingDataItem.vTurnRes;
    subHandlingData->fMoveResXY = subHandlingDataItem.fMoveResXY;
    subHandlingData->fMoveResZ = subHandlingDataItem.fMoveResZ;
    subHandlingData->fPitchMult = subHandlingDataItem.fPitchMult;
    subHandlingData->fPitchAngle = subHandlingDataItem.fPitchAngle;
    subHandlingData->fYawMult = subHandlingDataItem.fYawMult;
    subHandlingData->fDiveSpeed = subHandlingDataItem.fDiveSpeed;
    subHandlingData->fRollMult = subHandlingDataItem.fRollMult;
    subHandlingData->fRollStab = subHandlingDataItem.fRollStab;
}

void setCTrailerHandlingData(RTHE::CBaseSubHandlingData* baseSubHandlingData, const RTHE::CHandlingDataItem& handlingDataItem) {
    if (handlingDataItem.SubHandlingData.CTrailerHandlingData.empty())
        return;

    const auto& subHandlingDataItem = handlingDataItem.SubHandlingData.CTrailerHandlingData[0];
    RTHE::CTrailerHandlingData* subHandlingData = reinterpret_cast<RTHE::CTrailerHandlingData*>(baseSubHandlingData);

    subHandlingData->fAttachLimitPitch = subHandlingDataItem.fAttachLimitPitch;
    subHandlingData->fAttachLimitRoll = subHandlingDataItem.fAttachLimitRoll;
    subHandlingData->fAttachLimitYaw = subHandlingDataItem.fAttachLimitYaw;
    subHandlingData->fUprightSpringConstant = subHandlingDataItem.fUprightSpringConstant;
    subHandlingData->fUprightDampingConstant = subHandlingDataItem.fUprightDampingConstant;
    subHandlingData->fAttachedMaxDistance = subHandlingDataItem.fAttachedMaxDistance;
    subHandlingData->fAttachedMaxPenetration = subHandlingDataItem.fAttachedMaxPenetration;
    subHandlingData->fAttachRaiseZ = subHandlingDataItem.fAttachRaiseZ;
    subHandlingData->fPosConstraintMassRatio = subHandlingDataItem.fPosConstraintMassRatio;
}

void setCCarHandlingData(RTHE::CBaseSubHandlingData* baseSubHandlingData, const RTHE::CHandlingDataItem& handlingDataItem) {
    if (handlingDataItem.SubHandlingData.CCarHandlingData.empty())
        return;

    const auto& subHandlingDataItem = handlingDataItem.SubHandlingData.CCarHandlingData[0];
    RTHE::CCarHandlingData* subHandlingData = reinterpret_cast<RTHE::CCarHandlingData*>(baseSubHandlingData);

    subHandlingData->fBackEndPopUpCarImpulseMult = subHandlingDataItem.fBackEndPopUpCarImpulseMult;
    subHandlingData->fBackEndPopUpBuildingImpulseMult = subHandlingDataItem.fBackEndPopUpBuildingImpulseMult;
    subHandlingData->fBackEndPopUpMaxDeltaSpeed = subHandlingDataItem.fBackEndPopUpMaxDeltaSpeed;
    subHandlingData->fToeFront = subHandlingDataItem.fToeFront;
    subHandlingData->fToeRear = subHandlingDataItem.fToeRear;
    subHandlingData->fCamberFront = subHandlingDataItem.fCamberFront;
    subHandlingData->fCamberRear = subHandlingDataItem.fCamberRear;
    subHandlingData->fCastor = subHandlingDataItem.fCastor;
    subHandlingData->fEngineResistance = subHandlingDataItem.fEngineResistance;
    subHandlingData->fMaxDriveBiasTransfer = subHandlingDataItem.fMaxDriveBiasTransfer;
    subHandlingData->fJumpForceScale = subHandlingDataItem.fJumpForceScale;
    subHandlingData->fIncreasedRammingForceScale = subHandlingDataItem.fIncreasedRammingForceScale;
    subHandlingData->strAdvancedFlags = subHandlingDataItem.strAdvancedFlags;

    // Only apply AdvancedData if the size matches.
    auto numAdvData = subHandlingData->pAdvancedData.GetCount();
    auto numAdvDataFile = handlingDataItem.SubHandlingData.CCarHandlingData[0].AdvancedData.size();
    if (numAdvData != numAdvDataFile) {
        logger.Write(WARN, fmt::format("[Set handling] [{}] {}",
            handlingDataItem.handlingName,
            fmt::format("Skipped SubHandlingData.<CCarHandlingData>.AdvancedData, size mismatch. "
                "Vehicle has {}, handling has {}", numAdvData, numAdvDataFile)));
        return;
    }

    if (numAdvData == 0) {
        return;
    }

    for (uint32_t iAdv = 0; iAdv < numAdvData; ++iAdv) {
        subHandlingData->pAdvancedData.Get(iAdv).Index = handlingDataItem.SubHandlingData.CCarHandlingData[0].AdvancedData[iAdv].Index;
        subHandlingData->pAdvancedData.Get(iAdv).Slot = handlingDataItem.SubHandlingData.CCarHandlingData[0].AdvancedData[iAdv].Slot;
        subHandlingData->pAdvancedData.Get(iAdv).Value = handlingDataItem.SubHandlingData.CCarHandlingData[0].AdvancedData[iAdv].Value;
    }
}

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

    currentHandling->vecCentreOfMassOffset.x = handlingDataItem.vecCentreOfMassOffset.x;
    currentHandling->vecCentreOfMassOffset.y = handlingDataItem.vecCentreOfMassOffset.x;
    currentHandling->vecCentreOfMassOffset.z = handlingDataItem.vecCentreOfMassOffset.x;

    currentHandling->vecInertiaMultiplier.x = handlingDataItem.vecInertiaMultiplier.x;
    currentHandling->vecInertiaMultiplier.y = handlingDataItem.vecInertiaMultiplier.y;
    currentHandling->vecInertiaMultiplier.z = handlingDataItem.vecInertiaMultiplier.z;

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

    currentHandling->fInitialDriveMaxFlatVel = handlingDataItem.fInitialDriveMaxFlatVel / 3.6f;
    currentHandling->fDriveMaxFlatVel_ = handlingDataItem.fInitialDriveMaxFlatVel / 3.0f;

    currentHandling->fBrakeForce = handlingDataItem.fBrakeForce;

    currentHandling->fBrakeBiasFront = handlingDataItem.fBrakeBiasFront * 2.0f;
    currentHandling->fBrakeBiasRear_ = 2.0f * (1.0f - (handlingDataItem.fBrakeBiasFront));

    currentHandling->fHandBrakeForce = handlingDataItem.fHandBrakeForce;

    currentHandling->fSteeringLock = deg2rad(handlingDataItem.fSteeringLock);
    currentHandling->fSteeringLockRatio_ = 1.0f / (deg2rad(handlingDataItem.fSteeringLock));

    currentHandling->fTractionCurveMax = handlingDataItem.fTractionCurveMax;
    currentHandling->fTractionCurveMaxRatio_ = 1.0f / handlingDataItem.fTractionCurveMax;

    currentHandling->fTractionCurveMin = handlingDataItem.fTractionCurveMin;

    currentHandling->fTractionCurveRatio_ = 1.0f / (currentHandling->fTractionCurveMax - currentHandling->fTractionCurveMin);

    currentHandling->fTractionCurveLateral = deg2rad(handlingDataItem.fTractionCurveLateral);
    currentHandling->fTractionCurveLateralRatio_ = 1.0f / (deg2rad(handlingDataItem.fTractionCurveLateral));

    currentHandling->fTractionSpringDeltaMax = handlingDataItem.fTractionSpringDeltaMax;
    currentHandling->fTractionSpringDeltaMaxRatio_ = 1.0f / handlingDataItem.fTractionSpringDeltaMax;

    currentHandling->fLowSpeedTractionLossMult = handlingDataItem.fLowSpeedTractionLossMult;

    currentHandling->fCamberStiffnesss = handlingDataItem.fCamberStiffnesss;

    currentHandling->fTractionBiasFront = handlingDataItem.fTractionBiasFront * 2.0f;
    currentHandling->fTractionBiasRear = 2.0f * (1.0f - (handlingDataItem.fTractionBiasFront));

    currentHandling->fTractionLossMult = handlingDataItem.fTractionLossMult;

    currentHandling->fSuspensionForce = handlingDataItem.fSuspensionForce;

    currentHandling->fSuspensionCompDamp = handlingDataItem.fSuspensionCompDamp / 10.0f;

    currentHandling->fSuspensionReboundDamp = handlingDataItem.fSuspensionReboundDamp / 10.0f;

    currentHandling->fSuspensionUpperLimit = handlingDataItem.fSuspensionUpperLimit;

    currentHandling->fSuspensionLowerLimit = handlingDataItem.fSuspensionLowerLimit;

    currentHandling->fSuspensionRaise = handlingDataItem.fSuspensionRaise;

    currentHandling->fSuspensionBiasFront = handlingDataItem.fSuspensionBiasFront * 2.0f;
    currentHandling->fSuspensionBiasRear_ = 2.0f * (1.0f - handlingDataItem.fSuspensionBiasFront);

    currentHandling->fAntiRollBarForce = handlingDataItem.fAntiRollBarForce;

    currentHandling->fAntiRollBarBiasFront = handlingDataItem.fAntiRollBarBiasFront * 2.0f;
    currentHandling->fAntiRollBarBiasRear_ = 2.0f * (1.0f - (handlingDataItem.fAntiRollBarBiasFront));

    currentHandling->fRollCentreHeightFront = handlingDataItem.fRollCentreHeightFront;

    currentHandling->fRollCentreHeightRear = handlingDataItem.fRollCentreHeightRear;

    currentHandling->fCollisionDamageMult = handlingDataItem.fCollisionDamageMult;

    currentHandling->fWeaponDamageMult = handlingDataItem.fWeaponDamageMult;

    currentHandling->fDeformationDamageMult = handlingDataItem.fDeformationDamageMult;

    currentHandling->fEngineDamageMult = handlingDataItem.fEngineDamageMult;

    currentHandling->fPetrolTankVolume = handlingDataItem.fPetrolTankVolume;

    currentHandling->fOilVolume = handlingDataItem.fOilVolume;

    currentHandling->vecSeatOffsetDist.x = handlingDataItem.vecSeatOffsetDist.x;
    currentHandling->vecSeatOffsetDist.y = handlingDataItem.vecSeatOffsetDist.y;
    currentHandling->vecSeatOffsetDist.z = handlingDataItem.vecSeatOffsetDist.z;

    currentHandling->nMonetaryValue = handlingDataItem.nMonetaryValue;

    currentHandling->strModelFlags = handlingDataItem.strModelFlags;
    currentHandling->strHandlingFlags = handlingDataItem.strHandlingFlags;
    currentHandling->strDamageFlags = handlingDataItem.strDamageFlags;
    currentHandling->AIHandling = handlingDataItem.AIHandling;

    for (uint16_t idx = 0; idx < currentHandling->m_subHandlingData.GetCount(); ++idx) {
        RTHE::CBaseSubHandlingData* subHandlingData = currentHandling->m_subHandlingData.Get(idx);

        if (subHandlingData == nullptr) {
            continue;
        }

        // It might be possible that the handling has multiple of the same type
        // The first registered one is loaded in in this case, as I haven't seen
        // any handling with same multiple entries
        // Edit: Well fuck me sideways, Avenger (and likely other VTOLs) have 2x CFlyingHandlingData
        auto type = subHandlingData->GetHandlingType();
        switch (type) {
            case RTHE::HANDLING_TYPE_BIKE:              setCBikeHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_FLYING:            setCFlyingHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_VERTICAL_FLYING:   setCFlyingHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_BOAT:              setCBoatHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_SEAPLANE:          setCSeaPlaneHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_SUBMARINE:         setCSubmarineHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_TRAILER:           setCTrailerHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_CAR:               setCCarHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_SPECIALFLIGHT:     setCSpecialFlightHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_WEAPON:
                logger.Write(WARN, fmt::format("[Set handling] [{}] Unsupported subhandling type: {}",
                    handlingDataItem.handlingName, "HANDLING_TYPE_WEAPON"));
                break;
            case RTHE::HANDLING_TYPE_TRAIN:
                logger.Write(WARN, fmt::format("[Set handling] [{}] Unsupported subhandling type: {}",
                    handlingDataItem.handlingName, "HANDLING_TYPE_TRAIN"));
                break;
            default:
                logger.Write(WARN, fmt::format("[Set handling] [{}] Unknown subhandling type: {}",
                    handlingDataItem.handlingName, static_cast<int>(type)));
                break;
        }
    }

    PHYSICS::SET_CGOFFSET(vehicle,
        currentHandling->vecCentreOfMassOffset.x,
        currentHandling->vecCentreOfMassOffset.y,
        currentHandling->vecCentreOfMassOffset.z);
}

void storeCBikeHandlingData(RTHE::CBaseSubHandlingData* baseSubHandlingData, RTHE::CHandlingDataItem& handlingDataItem) {
    RTHE::CBikeHandlingData* subHandlingData = reinterpret_cast<RTHE::CBikeHandlingData*>(baseSubHandlingData);
    RTHE::CBikeHandlingDataItem subHandlingDataItem;

    subHandlingDataItem.fLeanFwdCOMMult = subHandlingData->fLeanFwdCOMMult;
    subHandlingDataItem.fLeanFwdForceMult = subHandlingData->fLeanFwdForceMult;
    subHandlingDataItem.fLeanBakCOMMult = subHandlingData->fLeanBakCOMMult;
    subHandlingDataItem.fLeanBakForceMult = subHandlingData->fLeanBakForceMult;
    subHandlingDataItem.fMaxBankAngle = rad2deg(subHandlingData->fMaxBankAngle);
    subHandlingDataItem.fFullAnimAngle = rad2deg(subHandlingData->fFullAnimAngle);
    subHandlingDataItem.fDesLeanReturnFrac = subHandlingData->fDesLeanReturnFrac;
    subHandlingDataItem.fStickLeanMult = subHandlingData->fStickLeanMult;
    subHandlingDataItem.fBrakingStabilityMult = subHandlingData->fBrakingStabilityMult;
    subHandlingDataItem.fInAirSteerMult = subHandlingData->fInAirSteerMult;
    subHandlingDataItem.fWheelieBalancePoint = rad2deg(subHandlingData->fWheelieBalancePoint);
    subHandlingDataItem.fStoppieBalancePoint = rad2deg(subHandlingData->fStoppieBalancePoint);
    subHandlingDataItem.fWheelieSteerMult = subHandlingData->fWheelieSteerMult;
    subHandlingDataItem.fRearBalanceMult = subHandlingData->fRearBalanceMult;
    subHandlingDataItem.fFrontBalanceMult = subHandlingData->fFrontBalanceMult;
    subHandlingDataItem.fBikeGroundSideFrictionMult = subHandlingData->fBikeGroundSideFrictionMult;
    subHandlingDataItem.fBikeWheelGroundSideFrictionMult = subHandlingData->fBikeWheelGroundSideFrictionMult;
    subHandlingDataItem.fBikeOnStandLeanAngle = rad2deg(subHandlingData->fBikeOnStandLeanAngle);
    subHandlingDataItem.fBikeOnStandSteerAngle = subHandlingData->fBikeOnStandSteerAngle;
    subHandlingDataItem.fJumpForce = subHandlingData->fJumpForce;

    handlingDataItem.SubHandlingData.CBikeHandlingData.push_back(subHandlingDataItem);
}

void storeCFlyingHandlingData(RTHE::CBaseSubHandlingData* baseSubHandlingData, RTHE::CHandlingDataItem& handlingDataItem) {
    RTHE::CFlyingHandlingData* subHandlingData = reinterpret_cast<RTHE::CFlyingHandlingData*>(baseSubHandlingData);
    RTHE::CFlyingHandlingDataItem subHandlingDataItem;

    subHandlingDataItem.fThrust = subHandlingData->fThrust;
    subHandlingDataItem.fThrustFallOff = subHandlingData->fThrustFallOff;
    subHandlingDataItem.fThrustVectoring = subHandlingData->fThrustVectoring;
    subHandlingDataItem.fInitialThrust = subHandlingData->fInitialThrust;
    subHandlingDataItem.fInitialThrustFallOff = subHandlingData->fInitialThrustFallOff;
    subHandlingDataItem.fYawMult = subHandlingData->fYawMult;
    subHandlingDataItem.fYawStabilise = subHandlingData->fYawStabilise;
    subHandlingDataItem.fSideSlipMult = subHandlingData->fSideSlipMult;
    subHandlingDataItem.fInitialYawMult = subHandlingData->fInitialYawMult;
    subHandlingDataItem.fRollMult = subHandlingData->fRollMult;
    subHandlingDataItem.fRollStabilise = subHandlingData->fRollStabilise;
    subHandlingDataItem.fInitialRollMult = subHandlingData->fInitialRollMult;
    subHandlingDataItem.fPitchMult = subHandlingData->fPitchMult;
    subHandlingDataItem.fPitchStabilise = subHandlingData->fPitchStabilise;
    subHandlingDataItem.fInitialPitchMult = subHandlingData->fInitialPitchMult;
    subHandlingDataItem.fFormLiftMult = subHandlingData->fFormLiftMult;
    subHandlingDataItem.fAttackLiftMult = subHandlingData->fAttackLiftMult;
    subHandlingDataItem.fAttackDiveMult = subHandlingData->fAttackDiveMult;
    subHandlingDataItem.fGearDownDragV = subHandlingData->fGearDownDragV;
    subHandlingDataItem.fGearDownLiftMult = subHandlingData->fGearDownLiftMult;
    subHandlingDataItem.fWindMult = subHandlingData->fWindMult;
    subHandlingDataItem.fMoveRes = subHandlingData->fMoveRes;
    subHandlingDataItem.vecTurnRes = subHandlingData->vecTurnRes;
    subHandlingDataItem.vecSpeedRes = subHandlingData->vecSpeedRes;
    subHandlingDataItem.fGearDoorFrontOpen = rad2deg(subHandlingData->fGearDoorFrontOpen);
    subHandlingDataItem.fGearDoorRearOpen = rad2deg(subHandlingData->fGearDoorRearOpen);
    subHandlingDataItem.fGearDoorRearOpen2 = rad2deg(subHandlingData->fGearDoorRearOpen2);
    subHandlingDataItem.fGearDoorRearMOpen = rad2deg(subHandlingData->fGearDoorRearMOpen);
    subHandlingDataItem.fTurublenceMagnitudeMax = subHandlingData->fTurublenceMagnitudeMax;
    subHandlingDataItem.fTurublenceForceMulti = subHandlingData->fTurublenceForceMulti;
    subHandlingDataItem.fTurublenceRollTorqueMulti = subHandlingData->fTurublenceRollTorqueMulti;
    subHandlingDataItem.fTurublencePitchTorqueMulti = subHandlingData->fTurublencePitchTorqueMulti;
    subHandlingDataItem.fBodyDamageControlEffectMult = subHandlingData->fBodyDamageControlEffectMult;
    subHandlingDataItem.fInputSensitivityForDifficulty = subHandlingData->fInputSensitivityForDifficulty;
    subHandlingDataItem.fOnGroundYawBoostSpeedPeak = subHandlingData->fOnGroundYawBoostSpeedPeak;
    subHandlingDataItem.fOnGroundYawBoostSpeedCap = subHandlingData->fOnGroundYawBoostSpeedCap;
    subHandlingDataItem.fEngineOffGlideMulti = subHandlingData->fEngineOffGlideMulti;
    subHandlingDataItem.fAfterburnerEffectRadius = subHandlingData->fAfterburnerEffectRadius;
    subHandlingDataItem.fAfterburnerEffectDistance = subHandlingData->fAfterburnerEffectDistance;
    subHandlingDataItem.fAfterburnerEffectForceMulti = subHandlingData->fAfterburnerEffectForceMulti;
    subHandlingDataItem.fSubmergeLevelToPullHeliUnderwater = subHandlingData->fSubmergeLevelToPullHeliUnderwater;
    subHandlingDataItem.fExtraLiftWithRoll = subHandlingData->fExtraLiftWithRoll;
    subHandlingDataItem.handlingType = subHandlingData->handlingType;

    handlingDataItem.SubHandlingData.CFlyingHandlingData.push_back(subHandlingDataItem);
}

void storeCSpecialFlightHandlingData(RTHE::CBaseSubHandlingData* baseSubHandlingData, RTHE::CHandlingDataItem& handlingDataItem) {
    RTHE::CSpecialFlightHandlingData* subHandlingData = reinterpret_cast<RTHE::CSpecialFlightHandlingData*>(baseSubHandlingData);
    RTHE::CSpecialFlightHandlingDataItem subHandlingDataItem;

    subHandlingDataItem.vecAngularDamping = subHandlingData->vecAngularDamping;
    subHandlingDataItem.vecAngularDampingMin = subHandlingData->vecAngularDampingMin;
    subHandlingDataItem.vecLinearDamping = subHandlingData->vecLinearDamping;
    subHandlingDataItem.vecLinearDampingMin = subHandlingData->vecLinearDampingMin;
    subHandlingDataItem.fLiftCoefficient = subHandlingData->fLiftCoefficient;
    subHandlingDataItem.fCriticalLiftAngle = subHandlingData->fCriticalLiftAngle;
    subHandlingDataItem.fInitialLiftAngle = subHandlingData->fInitialLiftAngle;
    subHandlingDataItem.fMaxLiftAngle = subHandlingData->fMaxLiftAngle;
    subHandlingDataItem.fDragCoefficient = subHandlingData->fDragCoefficient;
    subHandlingDataItem.fBrakingDrag = subHandlingData->fBrakingDrag;
    subHandlingDataItem.fMaxLiftVelocity = subHandlingData->fMaxLiftVelocity;
    subHandlingDataItem.fMinLiftVelocity = subHandlingData->fMinLiftVelocity;
    subHandlingDataItem.fRollTorqueScale = subHandlingData->fRollTorqueScale;
    subHandlingDataItem.fMaxTorqueVelocity = subHandlingData->fMaxTorqueVelocity;
    subHandlingDataItem.fMinTorqueVelocity = subHandlingData->fMinTorqueVelocity;
    subHandlingDataItem.fYawTorqueScale = subHandlingData->fYawTorqueScale;
    subHandlingDataItem.fSelfLevelingPitchTorqueScale = subHandlingData->fSelfLevelingPitchTorqueScale;
    subHandlingDataItem.fInitalOverheadAssist = subHandlingData->fInitalOverheadAssist;
    subHandlingDataItem.fMaxPitchTorque = subHandlingData->fMaxPitchTorque;
    subHandlingDataItem.fMaxSteeringRollTorque = subHandlingData->fMaxSteeringRollTorque;
    subHandlingDataItem.fPitchTorqueScale = subHandlingData->fPitchTorqueScale;
    subHandlingDataItem.fSteeringTorqueScale = subHandlingData->fSteeringTorqueScale;
    subHandlingDataItem.fMaxThrust = subHandlingData->fMaxThrust;
    subHandlingDataItem.fTransitionDuration = subHandlingData->fTransitionDuration;
    subHandlingDataItem.fHoverVelocityScale = subHandlingData->fHoverVelocityScale;
    subHandlingDataItem.fStabilityAssist = subHandlingData->fStabilityAssist;
    subHandlingDataItem.fMinSpeedForThrustFalloff = subHandlingData->fMinSpeedForThrustFalloff;
    subHandlingDataItem.fBrakingThrustScale = subHandlingData->fBrakingThrustScale;
    subHandlingDataItem.mode = subHandlingData->mode;
    subHandlingDataItem.strFlags = subHandlingData->strFlags;

    handlingDataItem.SubHandlingData.CSpecialFlightHandlingData.push_back(subHandlingDataItem);
}

void storeCBoatHandlingData(RTHE::CBaseSubHandlingData* baseSubHandlingData, RTHE::CHandlingDataItem& handlingDataItem) {
    RTHE::CBoatHandlingData* subHandlingData = reinterpret_cast<RTHE::CBoatHandlingData*>(baseSubHandlingData);
    RTHE::CBoatHandlingDataItem subHandlingDataItem;

    subHandlingDataItem.fBoxFrontMult = subHandlingData->fBoxFrontMult;
    subHandlingDataItem.fBoxRearMult = subHandlingData->fBoxRearMult;
    subHandlingDataItem.fBoxSideMult = subHandlingData->fBoxSideMult;
    subHandlingDataItem.fSampleTop = subHandlingData->fSampleTop;
    subHandlingDataItem.fSampleBottom = subHandlingData->fSampleBottom;
    subHandlingDataItem.fSampleBottomTestCorrection = subHandlingData->fSampleBottomTestCorrection;
    subHandlingDataItem.fAquaplaneForce = subHandlingData->fAquaplaneForce;
    subHandlingDataItem.fAquaplanePushWaterMult = subHandlingData->fAquaplanePushWaterMult;
    subHandlingDataItem.fAquaplanePushWaterCap = subHandlingData->fAquaplanePushWaterCap;
    subHandlingDataItem.fAquaplanePushWaterApply = subHandlingData->fAquaplanePushWaterApply;
    subHandlingDataItem.fRudderForce = subHandlingData->fRudderForce;
    subHandlingDataItem.fRudderOffsetSubmerge = subHandlingData->fRudderOffsetSubmerge;
    subHandlingDataItem.fRudderOffsetForce = subHandlingData->fRudderOffsetForce;
    subHandlingDataItem.fRudderOffsetForceZMult = subHandlingData->fRudderOffsetForceZMult;
    subHandlingDataItem.fWaveAudioMult = subHandlingData->fWaveAudioMult;
    subHandlingDataItem.vecMoveResistance = subHandlingData->vecMoveResistance;
    subHandlingDataItem.vecTurnResistance = subHandlingData->vecTurnResistance;
    subHandlingDataItem.fLook_L_R_CamHeight = subHandlingData->fLook_L_R_CamHeight;
    subHandlingDataItem.fDragCoefficient = subHandlingData->fDragCoefficient;
    subHandlingDataItem.fKeelSphereSize = subHandlingData->fKeelSphereSize;
    subHandlingDataItem.fPropRadius = subHandlingData->fPropRadius;
    subHandlingDataItem.fLowLodAngOffset = subHandlingData->fLowLodAngOffset;
    subHandlingDataItem.fLowLodDraughtOffset = subHandlingData->fLowLodDraughtOffset;
    subHandlingDataItem.fImpellerOffset = subHandlingData->fImpellerOffset;
    subHandlingDataItem.fImpellerForceMult = subHandlingData->fImpellerForceMult;
    subHandlingDataItem.fDinghySphereBuoyConst = subHandlingData->fDinghySphereBuoyConst;
    subHandlingDataItem.fProwRaiseMult = subHandlingData->fProwRaiseMult;
    subHandlingDataItem.fDeepWaterSampleBuoyancyMult = subHandlingData->fDeepWaterSampleBuoyancyMult;
    subHandlingDataItem.fTransmissionMultiplier = subHandlingData->fTransmissionMultiplier;
    subHandlingDataItem.fTractionMultiplier = subHandlingData->fTractionMultiplier;

    handlingDataItem.SubHandlingData.CBoatHandlingData.push_back(subHandlingDataItem);
}

void storeCSeaPlaneHandlingData(RTHE::CBaseSubHandlingData* baseSubHandlingData, RTHE::CHandlingDataItem& handlingDataItem) {
    RTHE::CSeaPlaneHandlingData* subHandlingData = reinterpret_cast<RTHE::CSeaPlaneHandlingData*>(baseSubHandlingData);
    RTHE::CSeaPlaneHandlingDataItem subHandlingDataItem;

    subHandlingDataItem.fLeftPontoonComponentId = subHandlingData->fLeftPontoonComponentId;
    subHandlingDataItem.fRightPontoonComponentId = subHandlingData->fRightPontoonComponentId;
    subHandlingDataItem.fPontoonBuoyConst = subHandlingData->fPontoonBuoyConst;
    subHandlingDataItem.fPontoonSampleSizeFront = subHandlingData->fPontoonSampleSizeFront;
    subHandlingDataItem.fPontoonSampleSizeMiddle = subHandlingData->fPontoonSampleSizeMiddle;
    subHandlingDataItem.fPontoonSampleSizeRear = subHandlingData->fPontoonSampleSizeRear;
    subHandlingDataItem.fPontoonLengthFractionForSamples = subHandlingData->fPontoonLengthFractionForSamples;
    subHandlingDataItem.fPontoonDragCoefficient = subHandlingData->fPontoonDragCoefficient;
    subHandlingDataItem.fPontoonVerticalDampingCoefficientUp = subHandlingData->fPontoonVerticalDampingCoefficientUp;
    subHandlingDataItem.fPontoonVerticalDampingCoefficientDown = subHandlingData->fPontoonVerticalDampingCoefficientDown;
    subHandlingDataItem.fKeelSphereSize = subHandlingData->fKeelSphereSize;

    handlingDataItem.SubHandlingData.CSeaPlaneHandlingData.push_back(subHandlingDataItem);
}

void storeCSubmarineHandlingData(RTHE::CBaseSubHandlingData* baseSubHandlingData, RTHE::CHandlingDataItem& handlingDataItem) {
    RTHE::CSubmarineHandlingData* subHandlingData = reinterpret_cast<RTHE::CSubmarineHandlingData*>(baseSubHandlingData);
    RTHE::CSubmarineHandlingDataItem subHandlingDataItem;

    subHandlingDataItem.vTurnRes = subHandlingData->vTurnRes;
    subHandlingDataItem.fMoveResXY = subHandlingData->fMoveResXY;
    subHandlingDataItem.fMoveResZ = subHandlingData->fMoveResZ;
    subHandlingDataItem.fPitchMult = subHandlingData->fPitchMult;
    subHandlingDataItem.fPitchAngle = subHandlingData->fPitchAngle;
    subHandlingDataItem.fYawMult = subHandlingData->fYawMult;
    subHandlingDataItem.fDiveSpeed = subHandlingData->fDiveSpeed;
    subHandlingDataItem.fRollMult = subHandlingData->fRollMult;
    subHandlingDataItem.fRollStab = subHandlingData->fRollStab;

    handlingDataItem.SubHandlingData.CSubmarineHandlingData.push_back(subHandlingDataItem);
}

void storeCTrailerHandlingData(RTHE::CBaseSubHandlingData* baseSubHandlingData, RTHE::CHandlingDataItem& handlingDataItem) {
    RTHE::CTrailerHandlingData* subHandlingData = reinterpret_cast<RTHE::CTrailerHandlingData*>(baseSubHandlingData);
    RTHE::CTrailerHandlingDataItem subHandlingDataItem;

    subHandlingDataItem.fAttachLimitPitch = subHandlingData->fAttachLimitPitch;
    subHandlingDataItem.fAttachLimitRoll = subHandlingData->fAttachLimitRoll;
    subHandlingDataItem.fAttachLimitYaw = subHandlingData->fAttachLimitYaw;
    subHandlingDataItem.fUprightSpringConstant = subHandlingData->fUprightSpringConstant;
    subHandlingDataItem.fUprightDampingConstant = subHandlingData->fUprightDampingConstant;
    subHandlingDataItem.fAttachedMaxDistance = subHandlingData->fAttachedMaxDistance;
    subHandlingDataItem.fAttachedMaxPenetration = subHandlingData->fAttachedMaxPenetration;
    subHandlingDataItem.fAttachRaiseZ = subHandlingData->fAttachRaiseZ;
    subHandlingDataItem.fPosConstraintMassRatio = subHandlingData->fPosConstraintMassRatio;

    handlingDataItem.SubHandlingData.CTrailerHandlingData.push_back(subHandlingDataItem);
}

void storeCCarHandlingData(RTHE::CBaseSubHandlingData* baseSubHandlingData, RTHE::CHandlingDataItem& handlingDataItem) {
    RTHE::CCarHandlingData* subHandlingData = reinterpret_cast<RTHE::CCarHandlingData*>(baseSubHandlingData);
    RTHE::CCarHandlingDataItem subHandlingDataItem;

    subHandlingDataItem.fBackEndPopUpCarImpulseMult = subHandlingData->fBackEndPopUpCarImpulseMult;
    subHandlingDataItem.fBackEndPopUpBuildingImpulseMult = subHandlingData->fBackEndPopUpBuildingImpulseMult;
    subHandlingDataItem.fBackEndPopUpMaxDeltaSpeed = subHandlingData->fBackEndPopUpMaxDeltaSpeed;
    subHandlingDataItem.fToeFront = subHandlingData->fToeFront;
    subHandlingDataItem.fToeRear = subHandlingData->fToeRear;
    subHandlingDataItem.fCamberFront = subHandlingData->fCamberFront;
    subHandlingDataItem.fCamberRear = subHandlingData->fCamberRear;
    subHandlingDataItem.fCastor = subHandlingData->fCastor;
    subHandlingDataItem.fEngineResistance = subHandlingData->fEngineResistance;
    subHandlingDataItem.fMaxDriveBiasTransfer = subHandlingData->fMaxDriveBiasTransfer;
    subHandlingDataItem.fJumpForceScale = subHandlingData->fJumpForceScale;
    subHandlingDataItem.fIncreasedRammingForceScale = subHandlingData->fIncreasedRammingForceScale;
    subHandlingDataItem.strAdvancedFlags = subHandlingData->strAdvancedFlags;

    subHandlingDataItem.AdvancedData.clear();
    for (uint16_t iAdv = 0; iAdv < subHandlingData->pAdvancedData.GetCount(); ++iAdv) {
        subHandlingDataItem.AdvancedData.push_back(subHandlingData->pAdvancedData.Get(iAdv));
    }

    handlingDataItem.SubHandlingData.CCarHandlingData.push_back(subHandlingDataItem);
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

    handlingDataItem.vecCentreOfMassOffset.x = currentHandling->vecCentreOfMassOffset.x;
    handlingDataItem.vecCentreOfMassOffset.y = currentHandling->vecCentreOfMassOffset.y;
    handlingDataItem.vecCentreOfMassOffset.z = currentHandling->vecCentreOfMassOffset.z;

    handlingDataItem.vecInertiaMultiplier.x = currentHandling->vecInertiaMultiplier.x;
    handlingDataItem.vecInertiaMultiplier.y = currentHandling->vecInertiaMultiplier.y;
    handlingDataItem.vecInertiaMultiplier.z = currentHandling->vecInertiaMultiplier.z;

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

    handlingDataItem.fInitialDriveMaxFlatVel = currentHandling->fInitialDriveMaxFlatVel * 3.6f;

    handlingDataItem.fBrakeForce = currentHandling->fBrakeForce;

    handlingDataItem.fBrakeBiasFront = currentHandling->fBrakeBiasFront / 2.0f;

    handlingDataItem.fHandBrakeForce = currentHandling->fHandBrakeForce;

    handlingDataItem.fSteeringLock = rad2deg(currentHandling->fSteeringLock);

    handlingDataItem.fTractionCurveMax = currentHandling->fTractionCurveMax;

    handlingDataItem.fTractionCurveMin = currentHandling->fTractionCurveMin;

    handlingDataItem.fTractionCurveLateral = rad2deg(currentHandling->fTractionCurveLateral);

    handlingDataItem.fTractionSpringDeltaMax = currentHandling->fTractionSpringDeltaMax;

    handlingDataItem.fLowSpeedTractionLossMult = currentHandling->fLowSpeedTractionLossMult;

    handlingDataItem.fCamberStiffnesss = currentHandling->fCamberStiffnesss;

    handlingDataItem.fTractionBiasFront = currentHandling->fTractionBiasFront / 2.0f;

    handlingDataItem.fTractionLossMult = currentHandling->fTractionLossMult;

    handlingDataItem.fSuspensionForce = currentHandling->fSuspensionForce;

    handlingDataItem.fSuspensionCompDamp = currentHandling->fSuspensionCompDamp * 10.0f;

    handlingDataItem.fSuspensionReboundDamp = currentHandling->fSuspensionReboundDamp * 10.0f;

    handlingDataItem.fSuspensionUpperLimit = currentHandling->fSuspensionUpperLimit;

    handlingDataItem.fSuspensionLowerLimit = currentHandling->fSuspensionLowerLimit;

    handlingDataItem.fSuspensionRaise = currentHandling->fSuspensionRaise;

    handlingDataItem.fSuspensionBiasFront = currentHandling->fSuspensionBiasFront / 2.0f;

    handlingDataItem.fAntiRollBarForce = currentHandling->fAntiRollBarForce;

    handlingDataItem.fAntiRollBarBiasFront = currentHandling->fAntiRollBarBiasFront / 2.0f;

    handlingDataItem.fRollCentreHeightFront = currentHandling->fRollCentreHeightFront;

    handlingDataItem.fRollCentreHeightRear = currentHandling->fRollCentreHeightRear;

    handlingDataItem.fCollisionDamageMult = currentHandling->fCollisionDamageMult;

    handlingDataItem.fWeaponDamageMult = currentHandling->fWeaponDamageMult;

    handlingDataItem.fDeformationDamageMult = currentHandling->fDeformationDamageMult;

    handlingDataItem.fEngineDamageMult = currentHandling->fEngineDamageMult;

    handlingDataItem.fPetrolTankVolume = currentHandling->fPetrolTankVolume;

    handlingDataItem.fOilVolume = currentHandling->fOilVolume;

    handlingDataItem.vecSeatOffsetDist.x = currentHandling->vecSeatOffsetDist.x;
    handlingDataItem.vecSeatOffsetDist.y = currentHandling->vecSeatOffsetDist.y;
    handlingDataItem.vecSeatOffsetDist.z = currentHandling->vecSeatOffsetDist.z;

    handlingDataItem.nMonetaryValue = currentHandling->nMonetaryValue;

    handlingDataItem.strModelFlags = currentHandling->strModelFlags;
    handlingDataItem.strHandlingFlags = currentHandling->strHandlingFlags;
    handlingDataItem.strDamageFlags = currentHandling->strDamageFlags;

    handlingDataItem.AIHandling = currentHandling->AIHandling;

    auto& asCache = ASCache::Get();
    auto it = asCache.find(currentHandling->handlingName);
    std::string handlingName = (it != asCache.end()) ? it->second : fmt::format("{:08X}", currentHandling->handlingName);

    for (uint16_t idx = 0; idx < currentHandling->m_subHandlingData.GetCount(); ++idx) {
        RTHE::CBaseSubHandlingData* subHandlingData = currentHandling->m_subHandlingData.Get(idx);

        if (!subHandlingData)
            continue;

        auto type = subHandlingData->GetHandlingType();
        switch (type) {
            case RTHE::HANDLING_TYPE_BIKE:              storeCBikeHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_FLYING:            storeCFlyingHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_VERTICAL_FLYING:   storeCFlyingHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_BOAT:              storeCBoatHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_SEAPLANE:          storeCSeaPlaneHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_SUBMARINE:         storeCSubmarineHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_TRAILER:           storeCTrailerHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_CAR:               storeCCarHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_SPECIALFLIGHT:     storeCSpecialFlightHandlingData(subHandlingData, handlingDataItem); break;
            case RTHE::HANDLING_TYPE_WEAPON:
                logger.Write(WARN, fmt::format("[Get handling] [{}] Unsupported subhandling type: {}",
                    handlingName, "HANDLING_TYPE_WEAPON"));
                break;
            case RTHE::HANDLING_TYPE_TRAIN:
                logger.Write(WARN, fmt::format("[Get handling] [{}] Unsupported subhandling type: {}",
                    handlingName, "HANDLING_TYPE_TRAIN"));
                break;
            default:
                logger.Write(WARN, fmt::format("[Get handling] [{}] Unknown subhandling type: {}",
                    handlingName, static_cast<int>(type)));
                break;
        }
    }

    return handlingDataItem;
}
