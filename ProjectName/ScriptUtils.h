#pragma once
#include "inc/natives.h"

namespace Utils {
    struct Color {
        int R;
        int G;
        int B;
    };

    struct VehicleInfo {

    };

    VehicleInfo GetVehicleInfo(Vehicle vehicle);

    Vehicle SpawnVehicle(Hash hash, Vector3 coords, float heading, Vector3 velocity, VehicleInfo info, DWORD timeout);

    Vehicle RespawnVehicle(Vehicle oldVehicle, Ped ped);
}
