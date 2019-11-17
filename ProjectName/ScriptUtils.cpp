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

Vehicle Utils::RespawnVehicle(Vehicle oldVehicle, Ped ped) {
    Hash model = ENTITY::GET_ENTITY_MODEL(oldVehicle);
    Vector3 coords = ENTITY::GET_ENTITY_COORDS(oldVehicle, true);
    float heading = ENTITY::GET_ENTITY_HEADING(oldVehicle);
    Vector3 velocity = ENTITY::GET_ENTITY_FORWARD_VECTOR(oldVehicle);

    ENTITY::SET_ENTITY_AS_MISSION_ENTITY(oldVehicle, true, true);
    VEHICLE::DELETE_VEHICLE(&oldVehicle);

    Vehicle newVehicle = SpawnVehicle(model, coords, heading, velocity, {}, 1000);
    ENTITY::SET_ENTITY_AS_MISSION_ENTITY(newVehicle, true, false);
    PED::SET_PED_INTO_VEHICLE(ped, newVehicle, -1);
    ENTITY::SET_ENTITY_AS_MISSION_ENTITY(newVehicle, false, true);
    ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&newVehicle);
    return newVehicle;
}
