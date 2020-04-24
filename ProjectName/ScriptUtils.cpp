#include "ScriptUtils.h"
#include "VehicleMod.h"
#include "Util/UI.h"

Vehicle spawnVehicle(Hash hash, Vector3 velocity, Ped playerPed) {
    if (STREAMING::IS_MODEL_IN_CDIMAGE(hash) && STREAMING::IS_MODEL_A_VEHICLE(hash)) {
        STREAMING::REQUEST_MODEL(hash);
        DWORD startTime = GetTickCount();
        DWORD timeout = 3000; // in millis

        while (!STREAMING::HAS_MODEL_LOADED(hash)) {
            WAIT(0);
            if (GetTickCount() > startTime + timeout) {
                UI::Notify("Couldn't load model");
                WAIT(0);
                STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);
                return 0;
            }
        }

        Vehicle oldVeh = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);

        Vector3 rot = ENTITY::GET_ENTITY_ROTATION(oldVeh, 0);
        bool persist = ENTITY::IS_ENTITY_A_MISSION_ENTITY(oldVeh);
        Vector3 pos = ENTITY::GET_ENTITY_COORDS(oldVeh, true);
        float heading = ENTITY::GET_ENTITY_HEADING(oldVeh);

        ENTITY::SET_ENTITY_AS_MISSION_ENTITY(oldVeh, true, true);
        VEHICLE::DELETE_VEHICLE(&oldVeh);

        Vehicle veh = VEHICLE::CREATE_VEHICLE(hash, pos.x, pos.y, pos.z, heading, 0, 1);

        VEHICLE::SET_VEHICLE_DIRT_LEVEL(veh, 0.0f);
        ENTITY::SET_ENTITY_ROTATION(veh, rot.x, rot.y, rot.z, 0, false);

        PED::SET_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), veh, -1);

        VEHICLE::SET_VEHICLE_ENGINE_ON(veh, true, true, false);

        STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);
        ENTITY::SET_ENTITY_VELOCITY(veh, velocity.x, velocity.y, velocity.z);

        if (persist) {
            ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, true, false);
        }
        return veh;
    }
    UI::Notify("Couldn't load model");
    return 0;
}

void Utils::RespawnVehicle(Vehicle oldVehicle, Ped ped) {
    VehicleModData oldMods;
    oldMods.Handle = oldVehicle;
    oldMods.GetAll();

    Hash model = ENTITY::GET_ENTITY_MODEL(oldVehicle);
    
    Vector3 velocity = ENTITY::GET_ENTITY_VELOCITY(oldVehicle);
    Vehicle newVehicle = spawnVehicle(model, velocity, ped);

    oldMods.Handle = newVehicle;
    oldMods.SetAll();

    ENTITY::SET_ENTITY_AS_MISSION_ENTITY(newVehicle, false, true);
    ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&newVehicle);
}
