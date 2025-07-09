#pragma once
#include "script.h"
#include <windows.h>

// ---------------------------------
// Tabs for menu navigation
// ---------------------------------
enum TabType {
    TAB_SELF = 0,
    TAB_WEAPONS,
    TAB_MISC,
    TAB_COUNT
};

// ---------------------------------
// Core menu state
// ---------------------------------
extern int cheatTab;

// ---------------------------------
// Self Tab State
// ---------------------------------
extern bool godMode;
extern bool neverWanted;
extern bool infStamina;
extern bool seatbelt;
extern bool teleportToWaypoint;
extern bool superman;
extern bool ultraJump;
extern bool infiniteJump;
extern bool fastRun;
extern bool fastSwim;
extern bool noRagdoll;
extern bool superJump;
extern int tpLastWaypoint;

// ---------------------------------
// Weapons Tab State
// ---------------------------------
extern bool infAmmo;
extern bool explosiveAmmo;
extern bool fireAmmo;
extern bool explosiveMelee;
extern bool rapidFire;
extern bool giveAllWeapons;
extern bool forceGun;
extern bool soulSwapGun;
extern bool magnetGun;
extern float forceMultiplier;
extern float damageMultiplier;
extern float bulletRange;
extern float magnetGripStrength;
extern float magnetLaunchPower;
extern float magnetBoostPower;

// ---------------------------------
// Misc Tab State
// ---------------------------------
extern bool slowmo;
extern bool refillHPArmor;
extern bool moneyCheat;
extern int bulletExplosionType;
extern bool hashGunActive;
extern bool wantedUp;
extern bool wantedDown;
extern bool populateNow;

// ---------------------------------
// Bullet Types (optional, use if accessed from other files)
// ---------------------------------
extern const char* bulletTypeNames[];
extern const int bulletTypeTags[];
extern int bulletTypeCount;

// ---------------------------------
// Menu Functions (per tab and master)
// ---------------------------------
void Cheats_Init();
void Cheats_Tick();
void Cheats_DrawMenu(int& menuIndex, float x, float y, float w, float h);

void Self_Init();
void Self_Tick();

void Self_DrawMenu(int& menuIndex, float x, float y, float w, float h);

void Weapons_Init();
void Weapons_Tick();
void Weapons_DrawMenu(int& menuIndex, float x, float y, float w, float h);

void Misc_Init();
void Misc_Tick();
void Misc_DrawMenu(int& menuIndex, float x, float y, float w, float h);

