#include "HandlingUtils.h"

#include "Memory/VehicleExtensions.hpp"
#include <HandlingReplacement.h>
#include <inc/natives.h>

using VExt = VehicleExtensions;

RTHE::CHandlingData* GetEditHandlingData(bool editStock, bool& hrActive, std::string& vehicleName) {
    Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::GET_PLAYER_PED(PLAYER::GET_PLAYER_INDEX()), false);

    if (!ENTITY::DOES_ENTITY_EXIST(vehicle)) {
        return nullptr;
    }

    std::string vehicleNameLabel = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(ENTITY::GET_ENTITY_MODEL(vehicle));
    vehicleName = HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION(vehicleNameLabel.c_str());

    RTHE::CHandlingData* currentHandling = nullptr;

    uint64_t addr = VExt::GetHandlingPtr(vehicle);
    currentHandling = reinterpret_cast<RTHE::CHandlingData*>(addr);

    if (currentHandling == nullptr) {
        return nullptr;
    }

    void* handlingDataOrig = nullptr;
    bool customHandling = false;

    void* handlingDataReplace = nullptr;

    if (HR_GetHandlingData(vehicle, &handlingDataOrig, &handlingDataReplace)) {
        customHandling = true;
        hrActive = true;
    }

    if (customHandling && editStock && handlingDataOrig != nullptr) {
        currentHandling = reinterpret_cast<RTHE::CHandlingData*>(handlingDataOrig);
    }

    return currentHandling;
}

RTHE::CBaseSubHandlingData* GetSubHandlingData(RTHE::CHandlingData* handlingData, RTHE::eHandlingType handlingType) {
    if (!handlingData)
        return nullptr;

    for (uint16_t idx = 0; idx < handlingData->m_subHandlingData.GetCount(); ++idx) {
        RTHE::CBaseSubHandlingData* subHandlingData = handlingData->m_subHandlingData.Get(idx);

        if (!subHandlingData)
            continue;

        auto type = subHandlingData->GetHandlingType();
        if (subHandlingData->GetHandlingType() == handlingType)
            return subHandlingData;
    }

    return nullptr;
}
