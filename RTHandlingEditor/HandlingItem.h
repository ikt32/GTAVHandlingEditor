#pragma once
#include "Memory/HandlingInfo.h"
#include <string>
#include <vector>

// No idea how the rage allocator thing works so I can't straight up
// re-use the classes in HandlingInfo.h

// Also keep in mind these *Item things are in degrees/kph and not rad or m/s
// And contain other conversion factors before applying to game data.
// So can't be used 1:1
namespace RTHE {
class CCarHandlingDataItem : public CCarHandlingData {
public:
    virtual eHandlingType GetHandlingType() override { return HANDLING_TYPE_CAR; }
    virtual void OnPostLoad() override { /* _ */ }
    virtual void* parser_GetStructure() override { return nullptr; }

    std::vector<CAdvancedData> AdvancedData;
};

class CBoatHandlingDataItem : public CBoatHandlingData {
public:
    virtual eHandlingType GetHandlingType() override { return HANDLING_TYPE_BOAT; }
    virtual void OnPostLoad() override { /* _ */ }
    virtual void* parser_GetStructure() override { return nullptr; }
};

class CBikeHandlingDataItem : public CBikeHandlingData
{
public:
    virtual eHandlingType GetHandlingType() override { return HANDLING_TYPE_BIKE; }
    virtual void OnPostLoad() override { /* _ */ }
    virtual void* parser_GetStructure() override { return nullptr; }
};

class CFlyingHandlingDataItem : public CFlyingHandlingData
{
public:
    virtual eHandlingType GetHandlingType() override { return handlingType; }
    virtual void OnPostLoad() override { /* _ */ }
    virtual void* parser_GetStructure() override { return nullptr; }
};

// I'm not gonna do CVehicleWeaponHandlingData

class CSubmarineHandlingDataItem : public CSubmarineHandlingData
{
public:
    virtual eHandlingType GetHandlingType() override { return HANDLING_TYPE_SUBMARINE; }
    virtual void OnPostLoad() override { /* _ */ }
    virtual void* parser_GetStructure() override { return nullptr; }
};

class CTrailerHandlingDataItem : public CTrailerHandlingData
{
public:
    virtual eHandlingType GetHandlingType() override { return HANDLING_TYPE_TRAILER; }
    virtual void OnPostLoad() override { /* _ */ }
    virtual void* parser_GetStructure() override { return nullptr; }
};

class CSeaPlaneHandlingDataItem : public CSeaPlaneHandlingData
{
public:
    virtual eHandlingType GetHandlingType() override { return HANDLING_TYPE_SEAPLANE; }
    virtual void OnPostLoad() override { /* _ */ }
    virtual void* parser_GetStructure() override { return nullptr; }
};

class CSpecialFlightHandlingDataItem : public CSpecialFlightHandlingData
{
public:
    virtual eHandlingType GetHandlingType() override { return HANDLING_TYPE_SPECIALFLIGHT; }
    virtual void OnPostLoad() override { /* _ */ }
    virtual void* parser_GetStructure() override { return nullptr; }
};

class CHandlingDataItem : public CHandlingData {
public:
    virtual void* parser_GetStructure() override { return nullptr; }
    struct Metadata {
        std::string HandlingName;
        std::string FileName;
        std::string Description;
    } Metadata;

    struct {
        std::vector<CBikeHandlingDataItem> CBikeHandlingData;
        std::vector<CFlyingHandlingDataItem> CFlyingHandlingData;
        std::vector<CSpecialFlightHandlingDataItem> CSpecialFlightHandlingData;
        std::vector<CBoatHandlingDataItem> CBoatHandlingData;
        std::vector<CSeaPlaneHandlingDataItem> CSeaPlaneHandlingData;
        std::vector<CSubmarineHandlingDataItem> CSubmarineHandlingData;
        std::vector<CTrailerHandlingDataItem> CTrailerHandlingData;
        std::vector<CCarHandlingDataItem> CCarHandlingData;
    } SubHandlingData;
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
