#pragma once

#include "script.h"
#include <vector>

// --- Structs for Shop Data ---
struct VehicleForSale {
    const char* name;
    Hash hash;
    int price;
};

struct VehicleCategory {
    const char* name;
    std::vector<VehicleForSale> vehicles;
};

// --- Function Declarations ---
void CarShop_Init();
void CarShop_Tick();
void draw_car_shop_menu();

// --- Garage Functions (to be implemented in Garage.cpp) ---
void Garage_AddVehicle(Hash vehicleHash);
void Garage_Save();
void Garage_Load();
