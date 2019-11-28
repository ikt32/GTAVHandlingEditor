#pragma once
#include <string>

namespace RTHE {
struct CHandlingDataItem {
    struct MetaData {
        std::string fileName;
        std::string description;
    } metaData;
    std::string handlingName;
    float fMass;
    float fInitialDragCoeff;
    float fDownforceModifier;
    float fPercentSubmerged;
    float vecCentreOfMassOffsetX;
    float vecCentreOfMassOffsetY;
    float vecCentreOfMassOffsetZ;
    float vecInertiaMultiplierX;
    float vecInertiaMultiplierY;
    float vecInertiaMultiplierZ;
    float fDriveBiasFront;
    int   nInitialDriveGears;
    float fInitialDriveForce;
    float fDriveInertia;
    float fClutchChangeRateScaleUpShift;
    float fClutchChangeRateScaleDownShift;
    float fInitialDriveMaxFlatVel;
    float fBrakeForce;
    float fBrakeBiasFront;
    float fHandBrakeForce;
    float fSteeringLock;
    float fTractionCurveMax;
    float fTractionCurveMin;
    float fTractionCurveLateral;
    float fTractionSpringDeltaMax;
    float fLowSpeedTractionLossMult;
    float fCamberStiffness;
    float fTractionBiasFront;
    float fTractionLossMult;
    float fSuspensionForce;
    float fSuspensionCompDamp;
    float fSuspensionReboundDamp;
    float fSuspensionUpperLimit;
    float fSuspensionLowerLimit;
    float fSuspensionRaise;
    float fSuspensionBiasFront;
    float fAntiRollBarForce;
    float fAntiRollBarBiasFront;
    float fRollCentreHeightFront;
    float fRollCentreHeightRear;
    float fCollisionDamageMult;
    float fWeaponDamageMult;
    float fDeformationDamageMult;
    float fEngineDamageMult;
    float fPetrolTankVolume;
    float fOilVolume;
    float fSeatOffsetDistX;
    float fSeatOffsetDistY;
    float fSeatOffsetDistZ;
    int   nMonetaryValue;
    int   strModelFlags;          // hex
    int   strHandlingFlags;       // hex
    int   strDamageFlags;         // hex
    int   AIHandling;             // joaat hash
};

// Returns {} on failure
CHandlingDataItem ParseXMLItem(const std::string& sourceFile);

// Returns false on failure
bool SaveXMLItem(const CHandlingDataItem& handlingDataItem, const std::string& targetFile);
}

// Reference
/*
<?xml version="1.0" encoding="UTF-8"?>
<CHandlingDataMgr>
  <HandlingData>
    <!-- This is our main level. Files provided only contain <Item />-level elements and lower. -->
    <Item type="CHandlingData">
      <handlingName>JUGULAR</handlingName>
      <fMass value="1745.000000" />
      <fInitialDragCoeff value="11.500000" />
      <fDownforceModifier value="200.000" />
      <fPercentSubmerged value="85.000000" />
      <vecCentreOfMassOffset x="0.000000" y="0.02000" z="0.000000" />
      <vecInertiaMultiplier x="1.200000" y="1.200000" z="1.400000" />
      <fDriveBiasFront value="0.300000" />
      <nInitialDriveGears value="8" />
      <fInitialDriveForce value="0.37800" />
      <fDriveInertia value="1.000000" />
      <fClutchChangeRateScaleUpShift value="8.100000" />
      <fClutchChangeRateScaleDownShift value="8.100000" />
      <fInitialDriveMaxFlatVel value="157.50000" />
      <fBrakeForce value="1.10000" />
      <fBrakeBiasFront value="0.540000" />
      <fHandBrakeForce value="0.650000" />
      <fSteeringLock value="40.000000" />
      <fTractionCurveMax value="2.620000" />
      <fTractionCurveMin value="2.380000" />
      <fTractionCurveLateral value="22.500000" />
      <fTractionSpringDeltaMax value="0.150000" />
      <fLowSpeedTractionLossMult value="1.200000" />
      <fCamberStiffnesss value="0.000000" />
      <fTractionBiasFront value="0.49000" />
      <fTractionLossMult value="1.000000" />
      <fSuspensionForce value="2.500000" />
      <fSuspensionCompDamp value="1.500000" />
      <fSuspensionReboundDamp value="2.900000" />
      <fSuspensionUpperLimit value="0.090000" />
      <fSuspensionLowerLimit value="-0.100000" />
      <fSuspensionRaise value="0.000000" />
      <fSuspensionBiasFront value="0.510000" />
      <fAntiRollBarForce value="0.900000" />
      <fAntiRollBarBiasFront value="0.650000" />
      <fRollCentreHeightFront value="0.210000" />
      <fRollCentreHeightRear value="0.210000" />
      <fCollisionDamageMult value="1.000000" />
      <fWeaponDamageMult value="1.000000" />
      <fDeformationDamageMult value="0.600000" />
      <fEngineDamageMult value="1.500000" />
      <fPetrolTankVolume value="60.000000" />
      <fOilVolume value="6.500000" />
      <fSeatOffsetDistX value="0.000000" />
      <fSeatOffsetDistY value="-0.250000" />
      <fSeatOffsetDistZ value="0.000000" />
      <nMonetaryValue value="150000" />
      <strModelFlags>440010</strModelFlags>
      <strHandlingFlags>0</strHandlingFlags>
      <strDamageFlags>0</strDamageFlags>
      <AIHandling>AVERAGE</AIHandling>
      <SubHandlingData>
        <Item type="CCarHandlingData">
          <strAdvancedFlags>4000000</strAdvancedFlags>
        </Item>
        <Item type="NULL" />
        <Item type="NULL" />
      </SubHandlingData>
    </Item>
  </HandlingData>
</CHandlingDataMgr>
*/
