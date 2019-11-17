#pragma once
#include <cstdint>

namespace RTH {
    struct Vector3 {
        float x;
        float y;
        float z;
    };

    class CHandlingData {
    public:
        virtual void Function0();				//

        char pad_0x0008[0x4];					//0x0008
        float fMass;							//0x000C
        float fInitialDragCoeff;				//0x0010
        float fDownforceModifier;				//0x0014
        float N000019C0;						//0x0018
        float N000019E5;						//0x001C
        Vector3 vecCentreOfMassOffset;			//0x0020
        float N000019E9;						//0x002C
        Vector3 vecInteriaMultiplier;			//0x0030
        char pad_0x003C[0x4];					//0x003C
        float fPercentSubmerged;				//0x0040
        float N000019EF;						//0x0044
        float fDriveBiasFront;					//0x0048
        float fAcceleration;					//0x004C // ?????
        __int32 nInitialDriveGears;				//0x0050
        float fDriveIntertia;					//0x0054
        float fClutchChangeRateScaleUpShift;	//0x0058
        float fClutchChangeRateScaleDownShift;	//0x005C
        float fInitialDriveForce;				//0x0060
        float fDriveMaxFlatVel_;				//0x0064
        float fInitialDriveMaxFlatVel_;			//0x0068
        float fBrakeForce;						//0x006C
        float N000019CB;						//0x0070
        float fBrakeBiasFront_;					//0x0074
        float fBrakeBiasRear_;					//0x0078
        float fHandBrakeForce2;					//0x007C
        float fSteeringLock_;					//0x0080
        float fSteeringLockRatio_;				//0x0084
        float fTractionCurveMax;				//0x0088
        float fTractionCurveMaxRatio_;			//0x008C
        float fTractionCurveMin;				//0x0090
        float fTractionCurveMinRatio_;			//0x0094
        float fTractionCurveLateral_;			//0x0098
        float fTractionCurveLateralRatio_;		//0x009C
        float fTractionSpringDeltaMax;			//0x00A0
        float fTractionSpringDeltaMaxRatio_;	//0x00A4
        float fLowSpeedTractionLossMult;		//0x00A8
        float fCamberStiffnesss;				//0x00AC
        float fTractionBiasFront_;				//0x00B0
        float fTractionBiasRear;				//0x00B4
        float fTractionLossMult;				//0x00B8
        float fSuspensionForce;					//0x00BC
        float fSuspensionCompDamp;				//0x00C0
        float fSuspensionReboundDamp;			//0x00C4
        float fSuspensionUpperLimit;			//0x00C8
        float fSuspensionLowerLimit;			//0x00CC
        float fSuspensionRaise_;				//0x00D0
        float fSuspensionBiasFront_;			//0x00D4
        float fSuspensionBiasRear_;				//0x00D8
        float fAntiRollBarForce;				//0x00DC
        float fAntiRollBarBiasFront_;			//0x00E0
        float fAntiRollBarBiasRear_;			//0x00E4
        float fRollCentreHeightFront;			//0x00E8
        float fRollCentreHeightRear;			//0x00EC
        float fCollisionDamageMult;				//0x00F0
        float fWeaponDamageMult;				//0x00F4
        float fDeformationDamageMult;			//0x00F8
        float fEngineDamageMult;				//0x00FC
        float fPetrolTankVolume;				//0x0100
        float fOilVolume; 						//0x0104
        float N00001A75;						//0x0108
        Vector3 vecSeatOffsetDist;				//0x010C
        __int32 nMonetaryValue;					//0x0118
        DWORD strModelFlags;					//0x011C
        DWORD strHandlingFlags;					//0x0120
        DWORD strDamageFlags;					//0x0124
        DWORD AIHandling;						//0x0128
        char pad_0x012C[0x34];					//0x012C
        float N00001A80;						//0x0160
        float N00001AB9;						//0x0164
        float N00001A81;						//0x0168
        float N00001ABB;						//0x016C
        char pad_0x0170[0x8];					//0x0170
        char pad_0x0178[0x4];
        char pad_0x017C[0x4];
        char pad_0x0180[0x4];
        //float fBackEndPopUpCarImpulseMult;		//0x0178
        //float fBackEndPopUpBuildingImpulseMult; //0x017C
        //float fBackEndPopUpMaxDeltaSpeed;		//0x0180
        char pad_0x0184[0x3C];					//0x0184

    }; //Size=0x01C0

const struct HandlingOffsets {
    uint32_t dwHandlingNameHash = 0x0008;
    int fMass = 0x000C;
    int fInitialDragCoeff = 0x0010;
    int fDownforceModifier = 0x0014;
    // 0x0018
    // 0x001C
    Vector3 vecCentreOfMass = {
        0x0020,
        0x0024,
        0x0028
    };
    // 0x002C
    Vector3 vecInertiaMultiplier = {
        0x0030,
        0x0034,
        0x0038
    };
    // 0x003C
    int fPercentSubmerged = 0x0040;
    int fSubmergedRatio = 0x0044;
    int fDriveBiasFront = 0x0048;
    int fDriveBiasRear = 0x004C;
    uint8_t nInitialDriveGears = 0x0050;
    int fDriveInertia = 0x0054;
    int fClutchChangeRateScaleUpShift = 0x0058;
    int fClutchChangeRateScaleDownShift = 0x005C;
    int fInitialDriveForce = 0x0060;
    int fDriveMaxFlatVel = 0x0064;
    int fInitialDriveMaxFlatVel = 0x0068;
    int fBrakeForce = 0x006C;
    // 0x0070
    int fBrakeBiasFront = 0x0074;
    int fBrakeBiasRear = 0x0078;
    int fHandBrakeForce = 0x007C;
    int fSteeringLock = 0x0080;
    int fSteeringLockRatio = 0x0084;
    int fTractionCurveMax = 0x0088;
    int fTractionCurveMaxRatio = 0x008C;
    int fTractionCurveMin = 0x0090;
    int fTractionCurveRatio = 0x0094;
    int fTractionCurveLateral = 0x0098;
    int fTractionCurveLateralRatio = 0x009C;
    int fTractionSpringDeltaMax = 0x00A0;
    int fTractionSpringDeltaMaxRatio = 0x00A4;
    int fLowSpeedTractionLossMult = 0x00A8;
    int fCamberStiffness = 0x00AC;
    int fTractionBiasFront = 0x00B0;
    int fTractionBiasRear = 0x00B4;
    int fTractionLossMult = 0x00B8;
    int fSuspensionForce = 0x00BC;
    int fSuspensionCompDamp = 0x00C0;
    int fSuspensionReboundDamp = 0x00C4;
    int fSuspensionUpperLimit = 0x00C8;
    int fSuspensionLowerLimit = 0x00CC;
    int fSuspensionRaise = 0x00D0;
    int fSuspensionBiasFront = 0x00D4;
    int fSuspensionBiasRear = 0x00D8;
    int fAntiRollBarForce = 0x00DC;
    int fAntiRollBarBiasFront = 0x00E0;
    int fAntiRollBarBiasRear = 0x00E4;
    int fRollCentreHeightFront = 0x00E8;
    int fRollCentreHeightRear = 0x00EC;
    int fCollisionDamageMult = 0x00F0;
    int fWeaponDamageMult = 0x00F4;
    int fDeformationDamageMult = 0x00F8;
    int fEngineDamageMult = 0x00FC;
    int fPetrolTankVolume = 0x0100;
    int fOilVolume = 0x0104;
    // 0x0108                           // 0.5f?
    int fSeatOffsetDistX = 0x010C;
    int fSeatOffsetDistY = 0x0110;
    int fSeatOffsetDistZ = 0x0114;
    int nMonetaryValue = 0x0118;
    // 0x011C // Hex  , 0x3FA00000 ?
    // 0x0120 // Float, 70.0f?
    uint32_t dwStrModelFlags = 0x0124;
    uint32_t dwStrHandlingFlags = 0x0128;
    uint32_t dwStrDamageFlags = 0x012C;
    // 0x0130 // Some hash?
    // 0x0134 // some hash? joaat "0"
    // 0x0138 // joaat("0") ??? wtf
    uint32_t dwAIHandlingHash = 0x013C;
    // 0x0140 // Float 42.45703506 @ Deviant or 0x4229D401

    // 0x0150 // CBaseSubHandlingData* ptr?
    // 0x0154 // some ptr

    // 0x0330 // some ptr?
    // ccarhandlingdata area
    // 0x0338 // fBackEndPopUpCarImpulseMult
    // 0x033C // fBackEndPopUpBuildingImpulseMult
    // 0x0340 // fBackEndPopUpMaxDeltaSpeed
    // 0x0344 // fToeFront
    // 0x0348 // fToeRear
    // 0x034C // fCamberFront
    // 0x0350 // fCamberRear
    // 0x0354 // fCastor
    // 0x0358 // fEngineResistance
    // 0x035C // 
    // 0x0360 // fJumpForceScale
    // 0x0364 //
    // 0x0368 // 
    // 0x036C // strAdvancedFlags

    // 0x0660 // some ptr
    // 0x0668 // uWeaponHash array

    // 0x0698 // hex (0x1F @ Issi4)
    // 0x069C // hex (0x0A @ Issi4)
    // 0x06A0 // hex
    // 0x06A4 // hex
    // 0x06A8 // hex
    // 0x06AC // hex
} Offsets = {};
}
