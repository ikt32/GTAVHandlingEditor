#include "inc/natives.h"

#include "VehicleData.h"

VehicleData::VehicleData() {
    zeroSamples();
}

void VehicleData::Clear() {
    Speed = 0.0f;
    Velocity = 0.0f;
    prevVelocities.x = 0;
    prevVelocities.y = 0;
    prevVelocities.z = 0;
    zeroSamples();
}

// Updates values from memory and natives
void VehicleData::UpdateValues(Vehicle vehicle) {
    Hash model = ENTITY::GET_ENTITY_MODEL(vehicle);

    Speed = ENTITY::GET_ENTITY_SPEED(vehicle);
    Velocity = ENTITY::GET_ENTITY_SPEED_VECTOR(vehicle, true).y;
    RotationVelocity = ENTITY::GET_ENTITY_ROTATION_VELOCITY(vehicle);

}

// Takes velocities in m/s, returns acceleration vectors in m/s^2
Vector3 VehicleData::getAccelerationVectors(Vector3 velocities) {
    long long time = std::chrono::steady_clock::now().time_since_epoch().count(); // 1ns

    Vector3 result{
        (velocities.x - prevVelocities.x) / ((time - prevAccelTime) / 1e9f),
        (velocities.y - prevVelocities.y) / ((time - prevAccelTime) / 1e9f),
        (velocities.z - prevVelocities.z) / ((time - prevAccelTime) / 1e9f)
    };

    prevAccelTime = time;
    prevVelocities = velocities;

    accelSamples[averageAccelIndex] = result;
    averageAccelIndex = (averageAccelIndex + 1) % (SAMPLES - 1);

    return result;
}

Vector3 VehicleData::getAccelerationVectorsAverage() const {
    Vector3 result{};

    for (int i = 0; i < SAMPLES; i++) {
        result.x += accelSamples[i].x;
        result.y += accelSamples[i].y;
        result.z += accelSamples[i].z;
    }

    result.x = result.x / SAMPLES;
    result.y = result.y / SAMPLES;
    result.z = result.z / SAMPLES;
    return result;
}


void VehicleData::zeroSamples() {
    for (int i = 0; i < SAMPLES; i++) {
        accelSamples[i].x = 0.0f;
        accelSamples[i].y = 0.0f;
        accelSamples[i].z = 0.0f;
    }
}
