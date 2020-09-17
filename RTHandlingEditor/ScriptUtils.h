#pragma once
#include "inc/natives.h"
#include "Memory/HandlingInfo.h"

namespace Utils {
    struct Color {
        int R;
        int G;
        int B;
    };

    void RespawnVehicle(Vehicle oldVehicle, Ped ped);

    void DrawCOMAndRollCenters(Vehicle vehicle, RTHE::CHandlingData* currentHandling);
}
