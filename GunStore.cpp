#include "GunStore.h"
#include "Money.h"
#include "RankBar.h"
#include "RpEvents.h" // <-- ADDED this include
#include "input.h"
#include "script.h"
#include <vector>
#include <fstream>
#include <string>

// --- Global Variables ---
static std::vector<WeaponCategory> g_weaponCategories;
static std::vector<Hash> g_unlockedWeapons;

static int gunCategoryIndex = 0;
static int weaponIndex = 0;
static bool inWeaponSelection = false;

// --- Weapon Locker System ---
bool GunStore_HasWeapon(Hash weaponHash) {
    for (size_t i = 0; i < g_unlockedWeapons.size(); ++i) {
        if (g_unlockedWeapons[i] == weaponHash) {
            return true;
        }
    }
    return false;
}

void GunStore_AddWeapon(Hash weaponHash) {
    if (!GunStore_HasWeapon(weaponHash)) {
        g_unlockedWeapons.push_back(weaponHash);
        GunStore_Save();
    }
}

void GunStore_Save() {
    std::ofstream file("GTAOfflineGunLocker.ini");
    if (file.is_open()) {
        for (const auto& hash : g_unlockedWeapons) {
            file << hash << std::endl;
        }
        file.close();
    }
}

void GunStore_Load() {
    g_unlockedWeapons.clear();
    std::ifstream file("GTAOfflineGunLocker.ini");
    if (file.is_open()) {
        Hash hash;
        while (file >> hash) {
            g_unlockedWeapons.push_back(hash);
        }
        file.close();
    }
}

// --- Initialization ---
void GunStore_Init() {
    // Pistols
    WeaponCategory pistols;
    pistols.name = "Pistols";
    pistols.weapons.push_back({ "Pistol", GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL"), 600, 1 });
    pistols.weapons.push_back({ "Combat Pistol", GAMEPLAY::GET_HASH_KEY("WEAPON_COMBATPISTOL"), 1200, 5 });
    pistols.weapons.push_back({ "AP Pistol", GAMEPLAY::GET_HASH_KEY("WEAPON_APPISTOL"), 5000, 15 });
    pistols.weapons.push_back({ "Pistol .50", GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL50"), 7500, 22 });
    pistols.weapons.push_back({ "SNS Pistol", GAMEPLAY::GET_HASH_KEY("WEAPON_SNSPISTOL"), 1000, 3 });
    pistols.weapons.push_back({ "Heavy Pistol", GAMEPLAY::GET_HASH_KEY("WEAPON_HEAVYPISTOL"), 3500, 10 });
    g_weaponCategories.push_back(pistols);

    // SMGs
    WeaponCategory smgs;
    smgs.name = "Machine Guns";
    smgs.weapons.push_back({ "Micro SMG", GAMEPLAY::GET_HASH_KEY("WEAPON_MICROSMG"), 2500, 8 });
    smgs.weapons.push_back({ "SMG", GAMEPLAY::GET_HASH_KEY("WEAPON_SMG"), 7500, 12 });
    smgs.weapons.push_back({ "Assault SMG", GAMEPLAY::GET_HASH_KEY("WEAPON_ASSAULTSMG"), 12500, 20 });
    smgs.weapons.push_back({ "Combat PDW", GAMEPLAY::GET_HASH_KEY("WEAPON_COMBATPDW"), 15000, 28 });
    smgs.weapons.push_back({ "MG", GAMEPLAY::GET_HASH_KEY("WEAPON_MG"), 14000, 30 });
    smgs.weapons.push_back({ "Combat MG", GAMEPLAY::GET_HASH_KEY("WEAPON_COMBATMG"), 18000, 40 });
    g_weaponCategories.push_back(smgs);

    // Rifles
    WeaponCategory rifles;
    rifles.name = "Assault Rifles";
    rifles.weapons.push_back({ "Assault Rifle", GAMEPLAY::GET_HASH_KEY("WEAPON_ASSAULTRIFLE"), 10000, 18 });
    rifles.weapons.push_back({ "Carbine Rifle", GAMEPLAY::GET_HASH_KEY("WEAPON_CARBINERIFLE"), 15000, 25 });
    rifles.weapons.push_back({ "Advanced Rifle", GAMEPLAY::GET_HASH_KEY("WEAPON_ADVANCEDRIFLE"), 22000, 35 });
    rifles.weapons.push_back({ "Special Carbine", GAMEPLAY::GET_HASH_KEY("WEAPON_SPECIALCARBINE"), 18000, 26 });
    rifles.weapons.push_back({ "Bullpup Rifle", GAMEPLAY::GET_HASH_KEY("WEAPON_BULLPUPRIFLE"), 16500, 24 });
    g_weaponCategories.push_back(rifles);

    // Shotguns
    WeaponCategory shotguns;
    shotguns.name = "Shotguns";
    shotguns.weapons.push_back({ "Pump Shotgun", GAMEPLAY::GET_HASH_KEY("WEAPON_PUMPSHOTGUN"), 3500, 10 });
    shotguns.weapons.push_back({ "Sawed-Off Shotgun", GAMEPLAY::GET_HASH_KEY("WEAPON_SAWNOFFSHOTGUN"), 2000, 6 });
    shotguns.weapons.push_back({ "Assault Shotgun", GAMEPLAY::GET_HASH_KEY("WEAPON_ASSAULTSHOTGUN"), 10000, 30 });
    shotguns.weapons.push_back({ "Heavy Shotgun", GAMEPLAY::GET_HASH_KEY("WEAPON_HEAVYSHOTGUN"), 12000, 32 });
    g_weaponCategories.push_back(shotguns);

    // Sniper Rifles
    WeaponCategory snipers;
    snipers.name = "Sniper Rifles";
    snipers.weapons.push_back({ "Sniper Rifle", GAMEPLAY::GET_HASH_KEY("WEAPON_SNIPERRIFLE"), 20000, 21 });
    snipers.weapons.push_back({ "Heavy Sniper", GAMEPLAY::GET_HASH_KEY("WEAPON_HEAVYSNIPER"), 38000, 50 });
    g_weaponCategories.push_back(snipers);

    // Heavy Weapons
    WeaponCategory heavy;
    heavy.name = "Heavy Weapons";
    heavy.weapons.push_back({ "Grenade Launcher", GAMEPLAY::GET_HASH_KEY("WEAPON_GRENADELAUNCHER"), 35000, 45 });
    heavy.weapons.push_back({ "RPG", GAMEPLAY::GET_HASH_KEY("WEAPON_RPG"), 75000, 60 });
    heavy.weapons.push_back({ "Minigun", GAMEPLAY::GET_HASH_KEY("WEAPON_MINIGUN"), 50000, 70 });
    g_weaponCategories.push_back(heavy);

    // Thrown Weapons
    WeaponCategory thrown;
    thrown.name = "Thrown";
    thrown.weapons.push_back({ "Grenade", GAMEPLAY::GET_HASH_KEY("WEAPON_GRENADE"), 250, 5 });
    thrown.weapons.push_back({ "Sticky Bomb", GAMEPLAY::GET_HASH_KEY("WEAPON_STICKYBOMB"), 600, 12 });
    thrown.weapons.push_back({ "Proximity Mine", GAMEPLAY::GET_HASH_KEY("WEAPON_PROXMINE"), 1000, 18 });
    thrown.weapons.push_back({ "Molotov", GAMEPLAY::GET_HASH_KEY("WEAPON_MOLOTOV"), 500, 10 });
    g_weaponCategories.push_back(thrown);

    // Melee
    WeaponCategory melee;
    melee.name = "Melee";
    melee.weapons.push_back({ "Knife", GAMEPLAY::GET_HASH_KEY("WEAPON_KNIFE"), 200, 1 });
    melee.weapons.push_back({ "Nightstick", GAMEPLAY::GET_HASH_KEY("WEAPON_NIGHTSTICK"), 100, 1 });
    melee.weapons.push_back({ "Hammer", GAMEPLAY::GET_HASH_KEY("WEAPON_HAMMER"), 500, 1 });
    melee.weapons.push_back({ "Bat", GAMEPLAY::GET_HASH_KEY("WEAPON_BAT"), 300, 1 });
    melee.weapons.push_back({ "Crowbar", GAMEPLAY::GET_HASH_KEY("WEAPON_CROWBAR"), 400, 1 });
    g_weaponCategories.push_back(melee);

    GunStore_Load();
}

void GunStore_Tick() {
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    for (const auto& weaponHash : g_unlockedWeapons) {
        if (!WEAPON::HAS_PED_GOT_WEAPON(playerPed, weaponHash, false)) {
            WEAPON::GIVE_WEAPON_TO_PED(playerPed, weaponHash, 250, false, true);
        }
    }
}

void draw_gun_store_menu() {
    extern int menuCategory;
    extern int menuIndex;
    extern int inputDelayFrames;

    const float MENU_X = 0.02f;
    const float MENU_Y = 0.13f;
    const float MENU_W = 0.29f;
    const float MENU_H = 0.038f;

    char moneyBuf[64];
    sprintf_s(moneyBuf, "Your Money: $%d", Money_Get());
    UI::SET_TEXT_FONT(0);
    UI::SET_TEXT_SCALE(0.0f, 0.37f);
    UI::SET_TEXT_COLOUR(130, 255, 130, 220);
    UI::_SET_TEXT_ENTRY("STRING");
    UI::_ADD_TEXT_COMPONENT_STRING(moneyBuf);
    UI::_DRAW_TEXT(MENU_X, MENU_Y - 0.08f);

    if (!inWeaponSelection) {
        const int numOptions = g_weaponCategories.size() + 1;

        GRAPHICS::DRAW_RECT(MENU_X + MENU_W * 0.5f, MENU_Y - 0.038f + MENU_H * (numOptions + 1) * 0.5f, MENU_W, MENU_H * (numOptions + 1), 14, 17, 22, 228);
        UI::SET_TEXT_FONT(0);
        UI::SET_TEXT_SCALE(0.0f, 0.43f);
        UI::SET_TEXT_COLOUR(255, 255, 220, 252);
        UI::_SET_TEXT_ENTRY("STRING");
        UI::_ADD_TEXT_COMPONENT_STRING("Gun Store");
        UI::_DRAW_TEXT(MENU_X + 0.014f, MENU_Y - 0.062f);

        for (size_t i = 0; i < g_weaponCategories.size(); ++i) {
            float cy = MENU_Y + MENU_H * i;
            bool active = (i == gunCategoryIndex);
            GRAPHICS::DRAW_RECT(MENU_X + MENU_W * 0.5f, cy + (MENU_H - 0.004f) * 0.5f, MENU_W, MENU_H - 0.004f, active ? 190 : 60, active ? 130 : 80, active ? 215 : 80, active ? 255 : 135);
            UI::SET_TEXT_FONT(0);
            UI::SET_TEXT_SCALE(0.0f, 0.38f);
            UI::SET_TEXT_COLOUR(0, 0, 0, 255);
            UI::_SET_TEXT_ENTRY("STRING");
            UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>(g_weaponCategories[i].name));
            UI::_DRAW_TEXT(MENU_X + 0.017f, cy + 0.007f);
        }
        float back_cy = MENU_Y + MENU_H * g_weaponCategories.size();
        bool back_active = (g_weaponCategories.size() == gunCategoryIndex);
        GRAPHICS::DRAW_RECT(MENU_X + MENU_W * 0.5f, back_cy + (MENU_H - 0.004f) * 0.5f, MENU_W, MENU_H - 0.004f, back_active ? 215 : 80, back_active ? 60 : 80, back_active ? 60 : 80, 255);
        UI::SET_TEXT_FONT(0);
        UI::SET_TEXT_SCALE(0.0f, 0.38f);
        UI::SET_TEXT_COLOUR(0, 0, 0, 255);
        UI::_SET_TEXT_ENTRY("STRING");
        UI::_ADD_TEXT_COMPONENT_STRING("Back");
        UI::_DRAW_TEXT(MENU_X + 0.017f, back_cy + 0.007f);

        if (IsKeyJustUp(VK_NUMPAD8) || PadPressed(DPAD_UP)) gunCategoryIndex = (gunCategoryIndex - 1 + numOptions) % numOptions;
        if (IsKeyJustUp(VK_NUMPAD2) || PadPressed(DPAD_DOWN)) gunCategoryIndex = (gunCategoryIndex + 1) % numOptions;
        if (IsKeyJustUp(VK_NUMPAD5) || PadPressed(BTN_A)) {
            if (gunCategoryIndex == g_weaponCategories.size()) {
                // FIXED: Replaced CAT_MAIN with its integer value 0
                menuCategory = 0; // CAT_MAIN; 
                menuIndex = 6;
            }
            else {
                inWeaponSelection = true;
                weaponIndex = 0;
            }
        }
    }
    else {
        WeaponCategory& selectedCategory = g_weaponCategories[gunCategoryIndex];
        const int numOptions = selectedCategory.weapons.size() + 1;

        GRAPHICS::DRAW_RECT(MENU_X + MENU_W * 0.5f, MENU_Y - 0.038f + MENU_H * (numOptions + 1) * 0.5f, MENU_W, MENU_H * (numOptions + 1), 14, 17, 22, 228);
        UI::SET_TEXT_FONT(0);
        UI::SET_TEXT_SCALE(0.0f, 0.43f);
        UI::SET_TEXT_COLOUR(255, 255, 220, 252);
        UI::_SET_TEXT_ENTRY("STRING");
        UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>(selectedCategory.name));
        UI::_DRAW_TEXT(MENU_X + 0.014f, MENU_Y - 0.062f);

        for (size_t i = 0; i < selectedCategory.weapons.size(); ++i) {
            float cy = MENU_Y + MENU_H * i;
            bool active = (i == weaponIndex);
            WeaponForSale& gun = selectedCategory.weapons[i];
            bool owned = GunStore_HasWeapon(gun.hash);
            // FIXED: Replaced RankBar_GetRank() with RpEvents_GetLevel()
            bool unlocked = RpEvents_GetLevel() >= gun.rankRequired;

            GRAPHICS::DRAW_RECT(MENU_X + MENU_W * 0.5f, cy + (MENU_H - 0.004f) * 0.5f, MENU_W, MENU_H - 0.004f, active ? 190 : 60, active ? 130 : 80, active ? 215 : 80, active ? 255 : 135);
            UI::SET_TEXT_FONT(0);
            UI::SET_TEXT_SCALE(0.0f, 0.38f);
            UI::SET_TEXT_COLOUR(0, 0, 0, 255);
            UI::_SET_TEXT_ENTRY("STRING");

            char weaponLabel[100];
            if (owned) {
                sprintf_s(weaponLabel, "%s [OWNED]", gun.name);
            }
            else if (!unlocked) {
                sprintf_s(weaponLabel, "%s [LOCKED - RANK %d]", gun.name, gun.rankRequired);
            }
            else {
                sprintf_s(weaponLabel, "%s - $%d", gun.name, gun.price);
            }
            UI::_ADD_TEXT_COMPONENT_STRING(weaponLabel);
            UI::_DRAW_TEXT(MENU_X + 0.017f, cy + 0.007f);
        }
        float back_cy = MENU_Y + MENU_H * selectedCategory.weapons.size();
        bool back_active = (selectedCategory.weapons.size() == weaponIndex);
        GRAPHICS::DRAW_RECT(MENU_X + MENU_W * 0.5f, back_cy + (MENU_H - 0.004f) * 0.5f, MENU_W, MENU_H - 0.004f, back_active ? 215 : 80, back_active ? 60 : 80, back_active ? 60 : 80, 255);
        UI::SET_TEXT_FONT(0);
        UI::SET_TEXT_SCALE(0.0f, 0.38f);
        UI::SET_TEXT_COLOUR(0, 0, 0, 255);
        UI::_SET_TEXT_ENTRY("STRING");
        UI::_ADD_TEXT_COMPONENT_STRING("Back");
        UI::_DRAW_TEXT(MENU_X + 0.017f, back_cy + 0.007f);

        if (IsKeyJustUp(VK_NUMPAD8) || PadPressed(DPAD_UP)) weaponIndex = (weaponIndex - 1 + numOptions) % numOptions;
        if (IsKeyJustUp(VK_NUMPAD2) || PadPressed(DPAD_DOWN)) weaponIndex = (weaponIndex + 1) % numOptions;
        if (IsKeyJustUp(VK_NUMPAD5) || PadPressed(BTN_A)) {
            if (weaponIndex == selectedCategory.weapons.size()) {
                inWeaponSelection = false;
            }
            else {
                WeaponForSale& gun = selectedCategory.weapons[weaponIndex];
                if (GunStore_HasWeapon(gun.hash)) {
                    // Already owned
                }
                // FIXED: Replaced RankBar_GetRank() with RpEvents_GetLevel()
                else if (RpEvents_GetLevel() < gun.rankRequired) {
                    UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
                    UI::_ADD_TEXT_COMPONENT_STRING("~r~Rank not high enough!");
                    UI::_DRAW_NOTIFICATION(false, true);
                }
                else if (Money_Get() >= gun.price) {
                    Money_Add(-gun.price);
                    GunStore_AddWeapon(gun.hash);
                    UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
                    UI::_ADD_TEXT_COMPONENT_STRING("~g~Purchase successful!");
                    UI::_DRAW_NOTIFICATION(false, true);
                }
                else {
                    UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
                    UI::_ADD_TEXT_COMPONENT_STRING("~r~Not enough money!");
                    UI::_DRAW_NOTIFICATION(false, true);
                }
            }
        }
    }
}
