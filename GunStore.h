#pragma once

#include "script.h"
#include <vector>

// --- Structs for Gun Store Data ---
struct WeaponForSale {
    const char* name;
    Hash hash;
    int price;
    int rankRequired;
};

struct WeaponCategory {
    const char* name;
    std::vector<WeaponForSale> weapons;
};

// --- Function Declarations ---
void GunStore_Init();
void GunStore_Tick();
void draw_gun_store_menu();

// --- Weapon Locker Management ---
void GunStore_Save();
void GunStore_Load();
bool GunStore_HasWeapon(Hash weaponHash);
void GunStore_AddWeapon(Hash weaponHash);
