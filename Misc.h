#pragma once
#include "script.h"

// State variables for the Misc tab
extern bool slowmo, refillHPArmor, moneyCheat, hashGunActive, wantedUp, wantedDown, populateNow, noHostilePeds, noClipMode, savePlayerCoords;
// NEW: Replaced allPedsFollowPlayer with a more detailed system
extern bool pedsFollowEnabled;
extern int followerCount;
extern bool followAllPeds;

extern int bulletExplosionType;
extern const char* bulletTypeNames[];

// Function Declarations
void Misc_Init();
void Misc_Tick();
void Misc_DrawMenu(int& menuIndex, float x, float y, float w, float h);

// Hash Gun logic (needed for notification)
void HandleHashGunNotification();
