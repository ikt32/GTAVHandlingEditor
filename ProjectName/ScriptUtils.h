#pragma once
#include "inc/natives.h"

namespace Utils {
    struct Color {
        int R;
        int G;
        int B;
    };

    void RespawnVehicle(Vehicle oldVehicle, Ped ped);
}
