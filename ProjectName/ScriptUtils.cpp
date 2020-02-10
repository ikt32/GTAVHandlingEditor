#include "ScriptUtils.h"

Utils::VehicleInfo Utils::GetVehicleInfo(Vehicle vehicle) {
    Hash model = ENTITY::GET_ENTITY_MODEL(vehicle);

    //VEHICLE::MOD

    Color color{};
    VEHICLE::GET_VEHICLE_COLOR(vehicle, &color.R, &color.G, &color.B);
}

Vehicle Utils::SpawnVehicle(Hash hash, Vector3 coords, float heading, Vector3 velocity, VehicleInfo info, DWORD timeout) {
    if (!(STREAMING::IS_MODEL_IN_CDIMAGE(hash) && STREAMING::IS_MODEL_A_VEHICLE(hash))) {
        // Vehicle doesn't exist
        return 0;
    }
    STREAMING::REQUEST_MODEL(hash);
    DWORD startTime = GetTickCount();

    while (!STREAMING::HAS_MODEL_LOADED(hash)) {
        WAIT(0);
        if (GetTickCount() > startTime + timeout) {
            // Couldn't load model
            WAIT(0);
            STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);
            return 0;
        }
    }

    Vehicle veh = VEHICLE::CREATE_VEHICLE(hash, coords.x, coords.y, coords.z, heading, 0, 1);
    VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
    WAIT(0);
    STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);

    ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, false, true);
    ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&veh);

    ENTITY::SET_ENTITY_VELOCITY(veh, velocity.x, velocity.y, velocity.z);

    return veh;
}

Vehicle spawnVehicle(Hash hash, Vector3 velocity) {
    if (STREAMING::IS_MODEL_IN_CDIMAGE(hash) && STREAMING::IS_MODEL_A_VEHICLE(hash)) {
        Ped playerPed = PLAYER::PLAYER_PED_ID();
        STREAMING::REQUEST_MODEL(hash);
        DWORD startTime = GetTickCount();
        DWORD timeout = 3000; // in millis

        while (!STREAMING::HAS_MODEL_LOADED(hash)) {
            WAIT(0);
            if (GetTickCount() > startTime + timeout) {
                //showSubtitle("Couldn't load model");
                WAIT(0);
                STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);
                return 0;
            }
        }
        bool persist = false;
        float offsetX = 0.0f;
        if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, false)) {
            Vehicle oldVeh = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);
            Hash oldHash = ENTITY::GET_ENTITY_MODEL(oldVeh);
            Vector3 newMin, newMax;
            Vector3 oldMin, oldMax;
            GAMEPLAY::GET_MODEL_DIMENSIONS(hash, &newMin, &newMax);
            GAMEPLAY::GET_MODEL_DIMENSIONS(oldHash, &oldMin, &oldMax);
            if (!ENTITY::DOES_ENTITY_EXIST(oldVeh)) {
                oldMax.x = oldMin.x = 0.0f;
            }
            // to the right
            // width + margin + width again 
            offsetX = ((newMax.x - newMin.x) / 2.0f) + 1.0f + ((oldMax.x - oldMin.x) / 2.0f);
        }

        Vector3 pos = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, offsetX, 0.0, 0);

        if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, false)) {
            Vehicle oldVeh = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);
            persist = ENTITY::IS_ENTITY_A_MISSION_ENTITY(oldVeh);
            Vector3 oldVehiclePos = ENTITY::GET_ENTITY_COORDS(playerPed, true);
            oldVehiclePos = ENTITY::GET_ENTITY_COORDS(oldVeh, true);
            ENTITY::SET_ENTITY_AS_MISSION_ENTITY(oldVeh, true, true);
            VEHICLE::DELETE_VEHICLE(&oldVeh);
            pos = oldVehiclePos;
        }


        Vehicle veh = VEHICLE::CREATE_VEHICLE(hash, pos.x, pos.y, pos.z, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()), 0, 1);
        //VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
        VEHICLE::SET_VEHICLE_DIRT_LEVEL(veh, 0.0f);

        ENTITY::SET_ENTITY_HEADING(veh, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()));
        PED::SET_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), veh, -1);

        VEHICLE::SET_VEHICLE_ENGINE_ON(veh, true, true, false);
        //WAIT(0);
        STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);
        ENTITY::SET_ENTITY_VELOCITY(veh, velocity.x, velocity.y, velocity.z);

        if (persist) {
            ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, true, false);
        }
        else {
            ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, false, true);
            ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&veh);
        }

        //showSubtitle("Spawned " + getGxtName(hash) + " (" + getModelName(hash) + ")");
        return veh;
    }
    else {
        //showSubtitle("Vehicle doesn't exist");
        return 0;
    }
}

Vehicle Utils::RespawnVehicle(Vehicle oldVehicle, Ped ped) {
    Hash model = ENTITY::GET_ENTITY_MODEL(oldVehicle);
    //Vector3 coords = ENTITY::GET_ENTITY_COORDS(oldVehicle, true);
    //float heading = ENTITY::GET_ENTITY_HEADING(oldVehicle);
    Vector3 velocity = ENTITY::GET_ENTITY_VELOCITY(oldVehicle);
    Vector3 rotation = ENTITY::GET_ENTITY_ROTATION_VELOCITY(oldVehicle);
    //ENTITY::SET_ENTITY_AS_MISSION_ENTITY(oldVehicle, true, true);
    //VEHICLE::DELETE_VEHICLE(&oldVehicle);
    ////
    

    Vehicle newVehicle = spawnVehicle(model, velocity);//SpawnVehicle(model, coords, heading, velocity, {}, 1000);
    //
    if (newVehicle) {
        //WAIT(0);
        
    }
    //ENTITY::SET_ENTITY_AS_MISSION_ENTITY(newVehicle, true, false);
    //PED::SET_PED_INTO_VEHICLE(ped, newVehicle, -1);
    //ENTITY::SET_ENTITY_AS_MISSION_ENTITY(newVehicle, false, true);
    //ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&newVehicle);
    return newVehicle;
}
