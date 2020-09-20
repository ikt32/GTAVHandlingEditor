#pragma once
#include "atArray.h"
#include <cstdint>

namespace RTHE {
    struct Vector3 {
        float x;
        float y;
        float z;
    };

    class CBaseSubHandlingData
    {
    public:
        virtual ~CBaseSubHandlingData() = default;
        virtual void* GetParser() = 0;
        virtual int GetUnk() = 0;
        virtual void ProcessOnLoad() = 0;
    };

    class CHandlingData {
    public:
        virtual ~CHandlingData() = default;
        uint32_t NameHash;                      //0x0008
        float fMass;                            //0x000C
        float fInitialDragCoeff;                //0x0010
        float fDownforceModifier;               //0x0014
        float fPopUpLightRotation;              //0x0018
        float N000019E5;                        //0x001C
        Vector3 vecCentreOfMassOffset;          //0x0020
        float N000019E9;                        //0x002C
        Vector3 vecInertiaMultiplier;           //0x0030
        float fUnk_0x003C;                      //0x003C // vecInertiaMultiplier.x?
        float fPercentSubmerged;                //0x0040
        float fSubmergedRatio_;                 //0x0044
        float fDriveBiasFront;                  //0x0048
        float fDriveBiasRear;                   //0x004C // fDriveBiasRear
        int nInitialDriveGears;                 //0x0050
        float fDriveInertia;                    //0x0054
        float fClutchChangeRateScaleUpShift;    //0x0058
        float fClutchChangeRateScaleDownShift;  //0x005C
        float fInitialDriveForce;               //0x0060
        float fDriveMaxFlatVel_;                //0x0064
        float fInitialDriveMaxFlatVel_;         //0x0068
        float fBrakeForce;                      //0x006C
        float N000019CB;                        //0x0070
        float fBrakeBiasFront_;                 //0x0074
        float fBrakeBiasRear_;                  //0x0078
        float fHandBrakeForce2;                 //0x007C
        float fSteeringLock_;                   //0x0080
        float fSteeringLockRatio_;              //0x0084
        float fTractionCurveMax;                //0x0088
        float fTractionCurveMaxRatio_;          //0x008C
        float fTractionCurveMin;                //0x0090
        float fTractionCurveRatio_;             //0x0094
        float fTractionCurveLateral_;           //0x0098
        float fTractionCurveLateralRatio_;      //0x009C
        float fTractionSpringDeltaMax;          //0x00A0
        float fTractionSpringDeltaMaxRatio_;    //0x00A4
        float fLowSpeedTractionLossMult;        //0x00A8
        float fCamberStiffness;                 //0x00AC
        float fTractionBiasFront_;              //0x00B0
        float fTractionBiasRear;                //0x00B4
        float fTractionLossMult;                //0x00B8
        float fSuspensionForce;                 //0x00BC
        float fSuspensionCompDamp;              //0x00C0
        float fSuspensionReboundDamp;           //0x00C4
        float fSuspensionUpperLimit;            //0x00C8
        float fSuspensionLowerLimit;            //0x00CC
        float fSuspensionRaise_;                //0x00D0
        float fSuspensionBiasFront_;            //0x00D4
        float fSuspensionBiasRear_;             //0x00D8
        float fAntiRollBarForce;                //0x00DC
        float fAntiRollBarBiasFront_;           //0x00E0
        float fAntiRollBarBiasRear_;            //0x00E4
        float fRollCentreHeightFront;           //0x00E8
        float fRollCentreHeightRear;            //0x00EC
        float fCollisionDamageMult;             //0x00F0
        float fWeaponDamageMult;                //0x00F4
        float fDeformationDamageMult;           //0x00F8
        float fEngineDamageMult;                //0x00FC
        float fPetrolTankVolume;                //0x0100
        float fOilVolume;                       //0x0104
        float N00001A75;                        //0x0108
        Vector3 vecSeatOffsetDist;              //0x010C
        int nMonetaryValue;                     //0x0118
        float fUnk_0x11C;                       //0x011C // 1.25f ?
        float fUnk_0x120;                       //0x0120 // 70.0f ?
        uint32_t strModelFlags;                 //0x0124
        uint32_t strHandlingFlags;              //0x0128
        uint32_t strDamageFlags;                //0x012C
        char pad_0x130[0x08];                   //0x0130
        uint32_t UnkHash0;                      //0x0138 // Seems to be joaat(0)
        uint32_t AIHandling;                    //0x013C
        char pad_0x140[0x18];                   //0x0140
        atArray<CBaseSubHandlingData*> m_subHandlingData;//0x0158
    }; //Size=0x01C0 ?
    static_assert(offsetof(CHandlingData, strModelFlags) == 0x124, "wrong offset");
    static_assert(offsetof(CHandlingData, AIHandling)    == 0x13C, "wrong offset");
    static_assert(offsetof(CHandlingData, m_subHandlingData) == 0x158, "wrong offset");

    // CCarHandlingData: (@0?)
    // vtable? (ptr)                           //0x0000
    // fBackEndPopUpCarImpulseMult             //0x0008
    // fBackEndPopUpBuildingImpulseMult        //0x000C
    // fBackEndPopUpMaxDeltaSpeed              //0x0010
    // fToeFront                               //0x0014
    // fToeRear                                //0x0018
    // fCamberFront                            //0x001C
    // fCamberRear                             //0x0020
    // fCastor                                 //0x0024
    // fEngineResistance                       //0x0028
    // ? -1.0                                  //0x002C
    // ?  1.0                                  //0x0030
    // ?  1.0                                  //0x0034
    // ? 0x9E58B8A9                            //0x0038
    // strAdvancedFlags                        //0x003C
}
