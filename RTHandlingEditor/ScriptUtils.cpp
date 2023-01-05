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

        Vehicle veh = VEHICLE::CREATE_VEHICLE(hash, pos, heading, 0, 1, false);

        VEHICLE::SET_VEHICLE_DIRT_LEVEL(veh, 0.0f);
        ENTITY::SET_ENTITY_ROTATION(veh, rot.x, rot.y, rot.z, 0, false);

        PED::SET_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), veh, -1);

        VEHICLE::SET_VEHICLE_ENGINE_ON(veh, true, true, false);

        STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);
        ENTITY::SET_ENTITY_VELOCITY(veh, velocity);

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

// Draw lines for COM (red) and RollHeight (blue)
void Utils::DrawCOMAndRollCenters(Vehicle vehicle, RTHE::CHandlingData* currentHandling) {
    if (!currentHandling)
        return;

    Vector3 modelDimMin, modelDimMax;
    MISC::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(vehicle), &modelDimMin, &modelDimMax);

    Vector3 lfd = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(vehicle, { modelDimMin.x, modelDimMax.y, modelDimMin.z });
    Vector3 lfu = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(vehicle, { modelDimMin.x, modelDimMax.y, modelDimMax.z });
    Vector3 rfd = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(vehicle, { modelDimMax.x, modelDimMax.y, modelDimMin.z });
    Vector3 rfu = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(vehicle, { modelDimMax.x, modelDimMax.y, modelDimMax.z });
    Vector3 lrd = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(vehicle, { modelDimMin.x, -modelDimMax.y, modelDimMin.z });
    Vector3 lru = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(vehicle, { modelDimMin.x, -modelDimMax.y, modelDimMax.z });
    Vector3 rrd = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(vehicle, { modelDimMax.x, -modelDimMax.y, modelDimMin.z });
    Vector3 rru = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(vehicle, { modelDimMax.x, -modelDimMax.y, modelDimMax.z });

    GRAPHICS::DRAW_LINE(lfd, rfd, 255, 255, 255, 255); // Front low
    GRAPHICS::DRAW_LINE(lfu, rfu, 255, 255, 255, 255); // Front high
    GRAPHICS::DRAW_LINE(lfd, lfu, 255, 255, 255, 255); // Front left
    GRAPHICS::DRAW_LINE(rfd, rfu, 255, 255, 255, 255); // Front right

    GRAPHICS::DRAW_LINE(lrd, rrd, 255, 255, 255, 255); // Rear low
    GRAPHICS::DRAW_LINE(lru, rru, 255, 255, 255, 255); // Rear high
    GRAPHICS::DRAW_LINE(lrd, lru, 255, 255, 255, 255); // Rear left
    GRAPHICS::DRAW_LINE(rrd, rru, 255, 255, 255, 255); // Rear right

    GRAPHICS::DRAW_LINE(lfu, lru, 255, 255, 255, 255); // Left up
    GRAPHICS::DRAW_LINE(rfu, rru, 255, 255, 255, 255); // Right up
    GRAPHICS::DRAW_LINE(lfd, lrd, 255, 255, 255, 255); // Left down
    GRAPHICS::DRAW_LINE(rfd, rrd, 255, 255, 255, 255); // Right down

    Vector3 comFrontProjection = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(
        vehicle, {
        currentHandling->vecCentreOfMassOffset.x,
        modelDimMax.y + 0.5f,
        currentHandling->vecCentreOfMassOffset.z });

    Vector3 comRearProjection = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(
        vehicle, {
        currentHandling->vecCentreOfMassOffset.x,
        modelDimMin.y - 0.5f,
        currentHandling->vecCentreOfMassOffset.z });

    GRAPHICS::DRAW_LINE(
        comFrontProjection, comRearProjection,
        255, 0, 0, 255);

    Vector3 comLeftProjection = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(
        vehicle,
        { modelDimMin.x - 0.5f,
        currentHandling->vecCentreOfMassOffset.y,
        currentHandling->vecCentreOfMassOffset.z });

    Vector3 comRightProjection = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(
        vehicle,
        { modelDimMax.x + 0.5f,
        currentHandling->vecCentreOfMassOffset.y,
        currentHandling->vecCentreOfMassOffset.z });

    GRAPHICS::DRAW_LINE(
        comLeftProjection, comRightProjection,
        255, 0, 0, 255);

    auto lfBoneCoord = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(
        vehicle,
        ENTITY::GET_ENTITY_BONE_INDEX_BY_NAME(
            vehicle,
            "wheel_lf"));

    auto rfBoneCoord = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(
        vehicle,
        ENTITY::GET_ENTITY_BONE_INDEX_BY_NAME(
            vehicle,
            "wheel_rf"));

    auto lrBoneCoord = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(
        vehicle,
        ENTITY::GET_ENTITY_BONE_INDEX_BY_NAME(
            vehicle,
            "wheel_lr"));

    auto rrBoneCoord = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(
        vehicle,
        ENTITY::GET_ENTITY_BONE_INDEX_BY_NAME(
            vehicle,
            "wheel_rr"));

    Vector3 lfBoneCoordRel = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(
        vehicle, lfBoneCoord);

    Vector3 rfBoneCoordRel = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(
        vehicle, rfBoneCoord);

    Vector3 lrBoneCoordRel = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(
        vehicle, lrBoneCoord);

    Vector3 rrBoneCoordRel = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(
        vehicle, rrBoneCoord);

    Vector3 rollFrontL = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(
        vehicle, {
        lfBoneCoordRel.x - 0.15f,
        lfBoneCoordRel.y,
        modelDimMin.z + currentHandling->fRollCentreHeightFront });

    Vector3 rollFrontR = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(
        vehicle, {
        rfBoneCoordRel.x + 0.15f,
        rfBoneCoordRel.y,
        modelDimMin.z + currentHandling->fRollCentreHeightFront });

    Vector3 rollRearL =
        ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(
            vehicle, {
            lrBoneCoordRel.x - 0.15f,
            lrBoneCoordRel.y,
            modelDimMin.z + currentHandling->fRollCentreHeightRear });

    Vector3 rollRearR =
        ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(
            vehicle, {
            rrBoneCoordRel.x + 0.15f,
            rrBoneCoordRel.y,
            modelDimMin.z + currentHandling->fRollCentreHeightRear });

    GRAPHICS::DRAW_LINE(
        rollFrontL, rollFrontR,
        0, 255, 0, 255);

    GRAPHICS::DRAW_LINE(
        rollRearL, rollRearR,
        255, 0, 0, 255);
}
