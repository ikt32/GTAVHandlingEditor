#pragma once
#include <inc/types.h>
#include <string>
#include <vector>

struct VehicleModData {
    struct VehicleMod {
        enum class ModType {
            Color,
            ColorCombo,
            ColorCustom1,
            ColorCustom2,
            Mod,
            ModToggle,
            WheelType,
            WindowTint,
            TyresCanBurst,
            ExtraColors,
            Livery,
            TyreSmoke,
            ModColor1,
            ModColor2,
            Extra,
            Neon,
            NeonColor,
            XenonColor,
            PlateType,
            InteriorColor,
            DashboardColor,
        };

        Vehicle Handle;
        ModType Type;
        int ModId;

        int Value0;
        int Value1;
        int Value2;
        int Value3;

        VehicleMod Get();

        void Set();
    };

    Vehicle Handle;

    std::string Plate;


    std::vector<VehicleMod> VehicleMods;

    void GetAll();

    void SetAll();
};
