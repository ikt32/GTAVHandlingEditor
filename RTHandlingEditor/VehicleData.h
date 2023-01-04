/*
 * Simplified VehicleData from Manual Transmission
 */

#pragma once


#include <inc/types.h>

#include <Windows.h>
#include <array>
#include <vector>
#include <chrono>

#define SAMPLES 4


class VehicleData {
public:
    VehicleData();
    void Clear();

    void UpdateValues(Vehicle vehicle);
    
    // Absolute speed, in m/s
    float Speed = 0.0f;

    // Directional speed, in m/s
    float Velocity = 0.0f;

    Vector3 RotationVelocity = {};
    Vector3 getAccelerationVectors(Vector3 velocities);

    // Should be called after getAccelerationVectors has been called in a loop
    Vector3 getAccelerationVectorsAverage() const;

private:

    Vector3 prevVelocities = {};

    long long prevAccelTime = 0;

    std::array<Vector3, SAMPLES> accelSamples = {};
    int averageAccelIndex = 0;
    void zeroSamples();
};
