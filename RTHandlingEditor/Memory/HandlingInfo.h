#pragma once
#include "atArray.h"
#include <cstdint>
#include "Vector.hpp"
#include <inc/types.h>
#include "atArray.h"

// https://alexguirre.github.io/rage-parser-dumps/
// Alignment - 16!

namespace RTHE
{
	enum eHandlingType
	{
		HANDLING_TYPE_BIKE,
		HANDLING_TYPE_FLYING,
		HANDLING_TYPE_VERTICAL_FLYING,
		HANDLING_TYPE_BOAT,
		HANDLING_TYPE_SEAPLANE,
		HANDLING_TYPE_SUBMARINE,
		HANDLING_TYPE_TRAIN,
		HANDLING_TYPE_TRAILER,
		HANDLING_TYPE_CAR,
		HANDLING_TYPE_WEAPON,
		HANDLING_TYPE_MAX_TYPES
	};

	enum VehicleType
	{
		VEHICLE_TYPE_NONE = 4294967295,
		VEHICLE_TYPE_CAR = 0,
		VEHICLE_TYPE_PLANE = 1,
		VEHICLE_TYPE_TRAILER = 2,
		VEHICLE_TYPE_QUADBIKE = 3,
		VEHICLE_TYPE_DRAFT = 4,
		VEHICLE_TYPE_SUBMARINECAR = 5,
		VEHICLE_TYPE_AMPHIBIOUS_AUTOMOBILE = 6,
		VEHICLE_TYPE_AMPHIBIOUS_QUADBIKE = 7,
		VEHICLE_TYPE_HELI = 8,
		VEHICLE_TYPE_BLIMP = 9,
		VEHICLE_TYPE_AUTOGYRO = 10,
		VEHICLE_TYPE_BIKE = 11,
		VEHICLE_TYPE_BICYCLE = 12,
		VEHICLE_TYPE_BOAT = 13,
		VEHICLE_TYPE_TRAIN = 14,
		VEHICLE_TYPE_SUBMARINE = 15,
	};

	enum eVehicleModType
	{
		VMT_SPOILER = 0,
		VMT_BUMPER_F = 1,
		VMT_BUMPER_R = 2,
		VMT_SKIRT = 3,
		VMT_EXHAUST = 4,
		VMT_CHASSIS = 5,
		VMT_GRILL = 6,
		VMT_BONNET = 7,
		VMT_WING_L = 8,
		VMT_WING_R = 9,
		VMT_ROOF = 10,
		VMT_PLTHOLDER = 11,
		VMT_PLTVANITY = 12,
		VMT_INTERIOR1 = 13,
		VMT_INTERIOR2 = 14,
		VMT_INTERIOR3 = 15,
		VMT_INTERIOR4 = 16,
		VMT_INTERIOR5 = 17,
		VMT_SEATS = 18,
		VMT_STEERING = 19,
		VMT_KNOB = 20,
		VMT_PLAQUE = 21,
		VMT_ICE = 22,
		VMT_TRUNK = 23,
		VMT_HYDRO = 24,
		VMT_ENGINEBAY1 = 25,
		VMT_ENGINEBAY2 = 26,
		VMT_ENGINEBAY3 = 27,
		VMT_CHASSIS2 = 28,
		VMT_CHASSIS3 = 29,
		VMT_CHASSIS4 = 30,
		VMT_CHASSIS5 = 31,
		VMT_DOOR_L = 32,
		VMT_DOOR_R = 33,
		VMT_LIVERY_MOD = 34,
		VMT_LIGHTBAR = 35,
		VMT_ENGINE = 36,
		VMT_BRAKES = 37,
		VMT_GEARBOX = 38,
		VMT_HORN = 39,
		VMT_SUSPENSION = 40,
		VMT_ARMOUR = 41,
		VMT_NITROUS = 42,
		VMT_TURBO = 43,
		VMT_SUBWOOFER = 44,
		VMT_TYRE_SMOKE = 45,
		VMT_HYDRAULICS = 46,
		VMT_XENON_LIGHTS = 47,
		VMT_WHEELS = 48,
		VMT_WHEELS_REAR_OR_HYDRAULICS = 49,
	};

	class CAdvancedData
	{
	public:
		virtual ~CAdvancedData() = default;
		int Slot;
		int Index;
		float Value;
	};

	class CHandlingObject
	{
	public:
		virtual ~CHandlingObject() = 0;
		virtual void* parser_GetStructure() = 0; //ret rage::parStructure
	};

	class CBaseSubHandlingData : public CHandlingObject
	{
	public:
		virtual eHandlingType GetHandlingType() = 0;
		virtual void OnPostLoad() = 0;
	};

	class CHandlingData : CHandlingObject
	{
	public:
		virtual ~CHandlingData() = default;
		uint32_t handlingName;                  //0x0008
		float fMass;                            //0x000C
		float fInitialDragCoeff;                //0x0010
		float fDownforceModifier;               //0x0014
		float fPopUpLightRotation;              //0x0018
		float N000019E5;                        //0x001C
		rage::Vec3V vecCentreOfMassOffset;      //0x0020
		rage::Vec3V vecInertiaMultiplier;       //0x0030
		float fPercentSubmerged;                //0x0040
		float fSubmergedRatio_;                 //0x0044
		float fDriveBiasFront;                  //0x0048
		float fDriveBiasRear;                   //0x004C // fDriveBiasRear
		uint8_t nInitialDriveGears;             //0x0050
		uint8_t nPad0x51;                       //0x0051
		uint8_t nPad0x52;                       //0x0052
		uint8_t nPad0x53;                       //0x0053
		float fDriveInertia;                    //0x0054
		float fClutchChangeRateScaleUpShift;    //0x0058
		float fClutchChangeRateScaleDownShift;  //0x005C
		float fInitialDriveForce;               //0x0060
		float fDriveMaxFlatVel_;                //0x0064
		float fInitialDriveMaxFlatVel;          //0x0068
		float fBrakeForce;                      //0x006C
		float N000019CB;                        //0x0070
		float fBrakeBiasFront;                  //0x0074
		float fBrakeBiasRear_;                  //0x0078
		float fHandBrakeForce;                  //0x007C
		float fSteeringLock;                    //0x0080
		float fSteeringLockRatio_;              //0x0084
		float fTractionCurveMax;                //0x0088
		float fTractionCurveMaxRatio_;          //0x008C
		float fTractionCurveMin;                //0x0090
		float fTractionCurveRatio_;             //0x0094
		float fTractionCurveLateral;            //0x0098
		float fTractionCurveLateralRatio_;      //0x009C
		float fTractionSpringDeltaMax;          //0x00A0
		float fTractionSpringDeltaMaxRatio_;    //0x00A4
		float fLowSpeedTractionLossMult;        //0x00A8
		float fCamberStiffnesss;                //0x00AC // R* typo
		float fTractionBiasFront;               //0x00B0
		float fTractionBiasRear;                //0x00B4
		float fTractionLossMult;                //0x00B8
		float fSuspensionForce;                 //0x00BC
		float fSuspensionCompDamp;              //0x00C0
		float fSuspensionReboundDamp;           //0x00C4
		float fSuspensionUpperLimit;            //0x00C8
		float fSuspensionLowerLimit;            //0x00CC
		float fSuspensionRaise;                 //0x00D0
		float fSuspensionBiasFront;             //0x00D4
		float fSuspensionBiasRear_;             //0x00D8
		float fAntiRollBarForce;                //0x00DC
		float fAntiRollBarBiasFront;            //0x00E0
		float fAntiRollBarBiasRear_;            //0x00E4
		float fRollCentreHeightFront;           //0x00E8
		float fRollCentreHeightRear;            //0x00EC
		float fCollisionDamageMult;             //0x00F0
		float fWeaponDamageMult;                //0x00F4
		float fDeformationDamageMult;           //0x00F8
		float fEngineDamageMult;                //0x00FC
		float fPetrolTankVolume;                //0x0100
		float fOilVolume;                       //0x0104
		float fPetrolConsumptionRate;           //0x0108
		rage::Vec3 vecSeatOffsetDist;           //0x010C // actually 3 floats in the par struct, no rage::Vec3V!
		int nMonetaryValue;                     //0x0118
		float fRocketBoostCapacity;             //0x011C // 1.25f ?
		float fBoostMaxSpeed;                   //0x0120 // 70.0f ?
		uint32_t strModelFlags;                 //0x0124
		uint32_t strHandlingFlags;              //0x0128
		uint32_t strDamageFlags;                //0x012C
		char pad_0x130[0x08];                   //0x0130
		uint32_t UnkHash0;                      //0x0138 // Seems to be joaat(0)
		uint32_t AIHandling;                    //0x013C
		char pad_0x140[0x18];                   //0x0140
		atArray<CBaseSubHandlingData*> m_subHandlingData;//0x0158
		float fWeaponDamageScaledToVehHealthMult; //0x0168
	}; //Size=0x01C0 ?
	static_assert(offsetof(CHandlingData, strModelFlags) == 0x124, "wrong offset");
	static_assert(offsetof(CHandlingData, AIHandling) == 0x13C, "wrong offset");
	static_assert(offsetof(CHandlingData, m_subHandlingData) == 0x158, "wrong offset");

	class CCarHandlingData : public CBaseSubHandlingData
	{
	public:
		// vtable? (ptr)                                //0x0000
		float fBackEndPopUpCarImpulseMult;              //0x0008
		float fBackEndPopUpBuildingImpulseMult;         //0x000C
		float fBackEndPopUpMaxDeltaSpeed;               //0x0010
		float fToeFront;                                //0x0014
		float fToeRear;                                 //0x0018
		float fCamberFront;                             //0x001C
		float fCamberRear;                              //0x0020
		float fCastor;                                  //0x0024
		float fEngineResistance;                        //0x0028
		float fMaxDriveBiasTransfer;                    //0x002C // Seems to be ignored
		float fJumpForceScale;                          //0x0030
		float fIncreasedRammingForceScale;              //0x0034 // Always 1.0?
		uint32_t Unk_0x038;                             //0x0038 // Could be some hash? 0 when no strAdvancedFlags
		uint32_t strAdvancedFlags;                      //0x003C
		atArray<CAdvancedData> pAdvancedData;           //0x0040
	};

	class CBoatHandlingData : public CBaseSubHandlingData
	{
	public:
		float fBoxFrontMult;                   //0x0008
		float fBoxRearMult;                    //0x000C
		float fBoxSideMult;                    //0x0010
		float fSampleTop;                      //0x0014
		float fSampleBottom;                   //0x0018
		float fSampleBottomTestCorrection;     //0x001C
		float fAquaplaneForce;                 //0x0020
		float fAquaplanePushWaterMult;         //0x0024
		float fAquaplanePushWaterCap;          //0x0028
		float fAquaplanePushWaterApply;        //0x002C
		float fRudderForce;                    //0x0030
		float fRudderOffsetSubmerge;           //0x0034
		float fRudderOffsetForce;              //0x0038
		float fRudderOffsetForceZMult;         //0x003C
		float fWaveAudioMult;                  //0x0040
		rage::Vec3V vecMoveResistance;         //0x0050
		rage::Vec3V vecTurnResistance;         //0x0060
		float fLook_L_R_CamHeight;             //0x0070
		float fDragCoefficient;                //0x0074
		float fKeelSphereSize;                 //0x0078
		float fPropRadius;                     //0x007C
		float fLowLodAngOffset;                //0x0080
		float fLowLodDraughtOffset;            //0x0084
		float fImpellerOffset;                 //0x0088
		float fImpellerForceMult;              //0x008C
		float fDinghySphereBuoyConst;          //0x0090
		float fProwRaiseMult;                  //0x0094
		float fDeepWaterSampleBuoyancyMult;    //0x0098
		float fTransmissionMultiplier;         //0x009C
		float fTractionMultiplier;             //0x00A0
	};

	class CBikeHandlingData : public CBaseSubHandlingData
	{
	public:
		float floatfLeanFwdCOMMult;                 //0x0008
		float floatfLeanFwdForceMult;               //0x000C
		float floatfLeanBakCOMMult;                 //0x0010
		float floatfLeanBakForceMult;               //0x0014
		float floatfMaxBankAngle;                   //0x0018
		float floatfFullAnimAngle;                  //0x001C
		float floatfDesLeanReturnFrac;              //0x0024
		float floatfStickLeanMult;                  //0x0028
		float floatfBrakingStabilityMult;           //0x002C
		float floatfInAirSteerMult;                 //0x0030
		float floatfWheelieBalancePoint;            //0x0034
		float floatfStoppieBalancePoint;            //0x0038
		float floatfWheelieSteerMult;               //0x003C
		float floatfRearBalanceMult;                //0x0040
		float floatfFrontBalanceMult;               //0x0044
		float floatfBikeGroundSideFrictionMult;     //0x0048
		float floatfBikeWheelGroundSideFrictionMult;//0x004C
		float floatfBikeOnStandLeanAngle;           //0x0050
		float floatfBikeOnStandSteerAngle;          //0x0054
		float floatfJumpForce;                      //0x0058
	};

	class CFlyingHandlingData : public CBaseSubHandlingData
	{
	public:
		float fThrust;                              //0x0008
		float fThrustFallOff;                       //0x000C
		float fThrustVectoring;                     //0x0010
		float fInitialThrust;                       //0x0014
		float fInitialThrustFallOff;                //0x0018
		float fYawMult;                             //0x001C
		float fYawStabilise;                        //0x0020
		float fSideSlipMult;                        //0x0024
		float fInitialYawMult;                      //0x0028
		float fRollMult;                            //0x002C
		float fRollStabilise;                       //0x0030
		float fInitialRollMult;                     //0x0034
		float fPitchMult;                           //0x0038
		float fPitchStabilise;                      //0x003C
		float fInitialPitchMult;                    //0x0040
		float fFormLiftMult;                        //0x0044
		float fAttackLiftMult;                      //0x0048
		float fAttackDiveMult;                      //0x004C
		float fGearDownDragV;                       //0x0050
		float fGearDownLiftMult;                    //0x0054
		float fWindMult;                            //0x0058
		float fMoveRes;                             //0x005C
		rage::Vec3V vecTurnRes;                     //0x0060
		rage::Vec3V vecSpeedRes;                    //0x0070
		float fGearDoorFrontOpen;                   //0x0080
		float fGearDoorRearOpen;                    //0x0084
		float fGearDoorRearOpen2;                   //0x0088
		float fGearDoorRearMOpen;                   //0x008C
		float fTurublenceMagnitudeMax;              //0x0090
		float fTurublenceForceMulti;                //0x0094
		float fTurublenceRollTorqueMulti;           //0x0098
		float fTurublencePitchTorqueMulti;          //0x009C
		float fBodyDamageControlEffectMult;         //0x00A0
		float fInputSensitivityForDifficulty;       //0x00A4
		float fOnGroundYawBoostSpeedPeak;           //0x00A8
		float fOnGroundYawBoostSpeedCap;            //0x00AC
		float fEngineOffGlideMulti;                 //0x00B0
		float fAfterburnerEffectRadius;             //0x00B4
		float fAfterburnerEffectDistance;           //0x00B8
		float fAfterburnerEffectForceMulti;         //0x00BC
		float fSubmergeLevelToPullHeliUnderwater;   //0x00C0
		float fExtraLiftWithRoll;                   //0x00C4
		eHandlingType handlingType;                 //0x00C8
	};

	struct sTurretPitchLimits // CVehicleWeaponHandlingData__sTurretPitchLimits
	{
		float fForwardAngleMin;
		float fForwardAngleMax;
		float fBackwardAngleMin;
		float fBackwardAngleMid;
		float fBackwardAngleMax;
		float fBackwardForcedPitch;
	};

	class CVehicleWeaponHandlingData : public CBaseSubHandlingData
	{
	public:
		atFixedArray <Hash, 6>                           m_WeaponHash;                  // 0x0008
		atFixedArray<int, 6>                             m_WeaponSeats;                 // 0x0020
		atFixedArray<enum eVehicleModType, 6>            m_WeaponVehicleModType;        // 0x0038
		atFixedArray<float, 12>                          m_TurretSpeed;                 // 0x0050
		atFixedArray<float, 12>                          m_TurretPitchMin;              // 0x0080
		atFixedArray<float, 12>                          m_TurretPitchMax;              // 0x00B0
		atFixedArray<float, 12>                          m_TurretCamPitchMin;           // 0x00E0
		atFixedArray<float, 12>                          m_TurretCamPitchMax;           // 0x0110
		atFixedArray<float, 12>                          m_BulletVelocityForGravity;    // 0x0140
		atFixedArray<float, 12>                          m_TurretPitchForwardMin;       // 0x0170
		atFixedArray<struct sTurretPitchLimits, 12>      m_TurretPitchLimitData;        // 0x01A0
		float                                            m_UvAnimationMult;             // 0x0320
		float                                            m_MiscGadgetVar;               // 0x0324
		float                                            m_WheelImpactOffset;           // 0x0328
	};

	class CSubmarineHandlingData : public CBaseSubHandlingData
	{
	public:
		rage::Vec3V vTurnRes;                  //0x0010
		float fMoveResXY;                      //0x0020
		float fMoveResZ;                       //0x0024
		float fPitchMult;                      //0x0028
		float fPitchAngle;                     //0x002C
		float fYawMult;                        //0x0030
		float fDiveSpeed;                      //0x0034
		float fRollMult;                       //0x0038
		float fRollStab;                       //0x003C
	};

	class CTrailerHandlingData : public CBaseSubHandlingData
	{
	public:
		float fAttachLimitPitch;               //0x0008
		float fAttachLimitRoll;                //0x000C
		float fAttachLimitYaw;                 //0x0010
		float fUprightSpringConstant;          //0x0014
		float fUprightDampingConstant;         //0x0018
		float fAttachedMaxDistance;            //0x001C
		float fAttachedMaxPenetration;         //0x0020
		float fAttachRaiseZ;                   //0x0024
		float fPosConstraintMassRatio;         //0x0028
	};

	class CSeaPlaneHandlingData : public CBaseSubHandlingData
	{
	public:
		int fLeftPontoonComponentId;                    //0x0008
		int fRightPontoonComponentId;                   //0x000C
		float fPontoonBuoyConst;                        //0x0010
		float fPontoonSampleSizeFront;                  //0x0014
		float fPontoonSampleSizeMiddle;                 //0x0018
		float fPontoonSampleSizeRear;                   //0x001C
		float fPontoonLengthFractionForSamples;         //0x0020
		float fPontoonDragCoefficient;                  //0x0024
		float fPontoonVerticalDampingCoefficientUp;     //0x0028
		float fPontoonVerticalDampingCoefficientDown;   //0x002C
		float fKeelSphereSize;                          //0x0030
	};

	class CSpecialFlightHandlingData : public CBaseSubHandlingData
	{
	public:
		rage::Vec3V vecAngularDamping;         //0x0010
		rage::Vec3V vecAngularDampingMin;      //0x0020
		rage::Vec3V vecLinearDamping;          //0x0030
		rage::Vec3V vecLinearDampingMin;       //0x0040
		float fLiftCoefficient;                //0x0050
		float fCriticalLiftAngle;              //0x0054
		float fInitialLiftAngle;               //0x0058
		float fMaxLiftAngle;                   //0x005C
		float fDragCoefficient;                //0x0060
		float fBrakingDrag;                    //0x0064
		float fMaxLiftVelocity;                //0x0068
		float fMinLiftVelocity;                //0x006C
		float fRollTorqueScale;                //0x0070
		float fMaxTorqueVelocity;              //0x0074
		float fMinTorqueVelocity;              //0x0078
		float fYawTorqueScale;                 //0x007C
		float fSelfLevelingPitchTorqueScale;   //0x0080
		float fInitalOverheadAssist;           //0x0084
		float fMaxPitchTorque;                 //0x0088
		float fMaxSteeringRollTorque;          //0x008C
		float fPitchTorqueScale;               //0x0090
		float fSteeringTorqueScale;            //0x0094
		float fMaxThrust;                      //0x0098
		float fTransitionDuration;             //0x009C
		float fHoverVelocityScale;             //0x00A0
		float fStabilityAssist;                //0x00A4
		float fMinSpeedForThrustFalloff;       //0x00A8
		float fBrakingThrustScale;             //0x00AC
		int mode;                              //0x00B0
		Hash strFlags;                         //0x00B4
	};
}