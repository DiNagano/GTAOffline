#pragma once
#include "script.h"


// State variables for the Weapons tab
extern bool infAmmo, explosiveAmmo, fireAmmo, explosiveMelee, rapidFire, giveAllWeapons, forceGun, magnetGun;
extern float forceMultiplier, damageMultiplier;
extern float magnetGripStrength, magnetLaunchPower, magnetBoostPower;

// List of all weapon hashes (used in Give All Weapons)
extern const Hash weaponHashes[];
extern const int numWeaponHashes;
