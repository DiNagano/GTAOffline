#pragma once

#define NOMINMAX 
#include <windows.h>
#include <math.h>
#include <algorithm>
#include "script.h"
#include "input.h"
#include <cstdio>

#define _CRT_SECURE_NO_WARNINGS
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class VehicleMenu {
public:
    VehicleMenu();
    Vehicle GetPlayerVehicle();
    void DrawMenu(int& menuIndex, int& menuCategory);
    void Tick();
    void Repair();
    void Flip();

private:
    void ApplyMods();
    void HandleFlyIfActive();
    void HandleImpactForceIfActive();
    void HandleEnterDeadCar();
}; 

// === MENU OPTION ENUMS (order matches labels in .cpp) ===
enum VehicleOption {
    VEHOPT_GODMODE,
    VEHOPT_DRIVEDEAD,
    VEHOPT_FLY,
    VEHOPT_AUTOREPAIR,
    VEHOPT_AUTOREPAIR_NEARBY,
    VEHOPT_IMPACT_FORCE,
    VEHOPT_TRACTION,
    VEHOPT_ENGINE_POWER,
    VEHOPT_TORQUE,
    VEHOPT_GRAVITY,
    VEHOPT_DENSITY,
    VEHOPT_REMOTECONTROL,
    VEHOPT_REMOTECONTROL_ALL,
    VEHOPT_REPAIR,
    VEHOPT_BACK,
    VEHOPT_COUNT
};
// Externs for toggles/values
extern bool godMode, driveDeadCars, vehicleFly, autoRepair, autoRepairNearby, remoteControlEnabled , remoteControlAllEnabled;
extern float impactForce, customSpeed, customTorque, customGravity, customTraction, vehicleDensity;
extern VehicleMenu g_vehicleMenu;
extern void (*Vehicle_DrawMenu)(int&, int&);