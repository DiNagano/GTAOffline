#define _CRT_SECURE_NO_WARNINGS
#include "script.h"
#include "Vehicle.h"
#include "RankBar.h"
#include "CharacterCreator.h"
#include "RpEvents.h"
#include "Money.h"
#include "Cheats.h"
#include "input.h"
#include "CarShop.h"
#include "Garage.h"
#include "GunStore.h"
#include "Credits.h" // <-- ADDED
#include <windows.h>
#include <ctime>
#include <cstdio>
#include <vector>
#include <algorithm>

// Defensive macros
#define VALID_PED(ped)     ((ped) != 0 && ENTITY::DOES_ENTITY_EXIST(ped))
#define VALID_PLAYER(p)   ((p) != -1 && PLAYER::IS_PLAYER_PLAYING(p))

#pragma warning(disable : 4244 4305)

// --- FIX: Define constants for our save files to keep things clean ---
const char* characterFile = "GTAOfflineChar.ini";
const char* playerStatsFile = "GTAOfflinePlayerStats.ini";
const char* xpFile = "GTAOfflineXP.ini";


// Menu categories
enum Category {
    CAT_MAIN = 0,
    CAT_CHARACTER,
    CAT_CHEATS,
    CAT_VEHICLE,
    CAT_SAVELOAD,
    CAT_CAR_SHOP,
    CAT_GARAGE,
    CAT_GUN_STORE,
    CAT_CREDITS // <-- ADDED
};

int menuCategory = CAT_MAIN;
int menuIndex = 0;
bool menuOpen = false;

const float MENU_X = 0.02f, MENU_Y = 0.13f, MENU_W = 0.29f, MENU_H = 0.038f;

int inputDelayFrames = 0;

// NEW: Variables for spawn protection and welcome messages
static bool spawnProtectionActive = true;
static DWORD spawnTime = 0;
static bool welcomeMessagesShown = false;


// Forward declarations
void draw_car_shop_menu();
void draw_garage_menu();
void draw_gun_store_menu();
void draw_credits_menu(); // <-- ADDED
void LoadGameData(); // Declaration for our new loading function

// Clamp menu index for safety
inline void ClampMenuIndex(int& idx, int max) {
    if (idx < 0) idx = 0;
    if (idx >= max) idx = max - 1;
}

void draw_main_menu() {
    const int numOptions = 9; // <-- CORRECTED
    const char* labels[numOptions] = {
        "Character Creator", "Cheats", "Vehicle", "Save/Load",
        "Car Shop", "Garage", "Gun Store", "Credits", "Close Menu" // <-- CORRECTED
    };

    float x = MENU_X, y = MENU_Y, w = MENU_W, h = MENU_H;
    GRAPHICS::DRAW_RECT(x + w * 0.5f, y - 0.038f + h * (numOptions + 1) * 0.5f, w, h * (numOptions + 1), 14, 17, 22, 228);
    GRAPHICS::DRAW_RECT(x + w * 0.5f, y - 0.072f + 0.019f, w, 0.038f, 27, 31, 36, 232);

    UI::SET_TEXT_FONT(0);
    UI::SET_TEXT_SCALE(0.0f, 0.43f);
    UI::SET_TEXT_COLOUR(255, 255, 220, 252);
    UI::_SET_TEXT_ENTRY("STRING");
    UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>("GTA OFFLINE"));
    UI::_DRAW_TEXT(x + 0.014f, y - 0.062f);

    for (int i = 0; i < numOptions; ++i) {
        float cy = y + h * i;
        bool active = (i == menuIndex);
        GRAPHICS::DRAW_RECT(x + w * 0.5f, cy + (h - 0.004f) * 0.5f, w, h - 0.004f,
            active ? 190 : 60, active ? 130 : 80, active ? 215 : 80, active ? 255 : 135);
        UI::SET_TEXT_FONT(0);
        UI::SET_TEXT_SCALE(0.0f, 0.38f);
        UI::SET_TEXT_COLOUR(0, 0, 0, 255);
        UI::_SET_TEXT_ENTRY("STRING");
        UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>(labels[i]));
        UI::_DRAW_TEXT(x + 0.017f, cy + 0.007f);
    }

    ClampMenuIndex(menuIndex, numOptions);

    if (inputDelayFrames > 0) return;

    int up = 0, down = 0;
    if (IsKeyJustUp(VK_NUMPAD8) || PadPressed(DPAD_UP))   up = 1;
    if (IsKeyJustUp(VK_NUMPAD2) || PadPressed(DPAD_DOWN)) down = 1;
    if (up)   menuIndex = (menuIndex - 1 + numOptions) % numOptions;
    if (down) menuIndex = (menuIndex + 1) % numOptions;

    static bool prevA = false;
    bool currA = PadPressed(BTN_A);
    if ((IsKeyJustUp(VK_NUMPAD5) || (currA && !prevA))) {
        switch (menuIndex) {
        case 0: menuCategory = CAT_CHARACTER; menuIndex = 0; inputDelayFrames = 10; break;
        case 1: menuCategory = CAT_CHEATS;    menuIndex = 0; inputDelayFrames = 10; break;
        case 2: menuCategory = CAT_VEHICLE;   menuIndex = 0; inputDelayFrames = 10; break;
        case 3: menuCategory = CAT_SAVELOAD;  menuIndex = 0; inputDelayFrames = 10; break;
        case 4: menuCategory = CAT_CAR_SHOP;  menuIndex = 0; inputDelayFrames = 10; break;
        case 5: menuCategory = CAT_GARAGE;    menuIndex = 0; inputDelayFrames = 10; break;
        case 6: menuCategory = CAT_GUN_STORE; menuIndex = 0; inputDelayFrames = 10; break;
        case 7: menuCategory = CAT_CREDITS;   menuIndex = 0; inputDelayFrames = 10; break; // <-- CORRECTED
        case 8: menuOpen = false;             menuIndex = 0; inputDelayFrames = 10; break; // <-- CORRECTED
        }
    }
    prevA = currA;
}


int saveloadMenuIndex = 0;
void draw_saveload_menu() {
    const int numOptions = 3;
    const char* labels[numOptions] = { "Save Game", "Load Game", "Back" };

    float x = MENU_X, y = MENU_Y, w = MENU_W, h = MENU_H;
    GRAPHICS::DRAW_RECT(x + w * 0.5f, y + h * (numOptions / 2.0f) + (h / 2.0f), w, h * (numOptions + 1), 22, 31, 31, 230);


    UI::SET_TEXT_FONT(0);
    UI::SET_TEXT_SCALE(0.0f, 0.40f);
    UI::SET_TEXT_COLOUR(255, 255, 220, 252);
    UI::_SET_TEXT_ENTRY("STRING");
    UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>("Save/Load"));
    UI::_DRAW_TEXT(x + 0.012f, y - 0.037f);

    for (int i = 0; i < numOptions; ++i) {
        float cy = y + h * i;
        bool active = (i == saveloadMenuIndex);
        GRAPHICS::DRAW_RECT(x + w * 0.5f, cy + (h - 0.004f) * 0.5f, w, h - 0.004f,
            active ? 140 : 60, active ? 205 : 80, active ? 215 : 80, active ? 255 : 135);
        UI::SET_TEXT_FONT(0);
        UI::SET_TEXT_SCALE(0.0f, 0.36f);
        UI::SET_TEXT_COLOUR(0, 0, 0, 255);
        UI::_SET_TEXT_ENTRY("STRING");
        UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>(labels[i]));
        UI::_DRAW_TEXT(x + 0.017f, cy + 0.007f);
    }

    ClampMenuIndex(saveloadMenuIndex, numOptions);

    if (inputDelayFrames > 0) return;

    int up = 0, down = 0;
    if (IsKeyJustUp(VK_NUMPAD8) || PadPressed(DPAD_UP))   up = 1;
    if (IsKeyJustUp(VK_NUMPAD2) || PadPressed(DPAD_DOWN)) down = 1;
    if (up)   saveloadMenuIndex = (saveloadMenuIndex - 1 + numOptions) % numOptions;
    if (down) saveloadMenuIndex = (saveloadMenuIndex + 1) % numOptions;

    static bool prevA = false;
    bool currA = PadPressed(BTN_A);
    if ((IsKeyJustUp(VK_NUMPAD5) || (currA && !prevA))) {
        if (saveloadMenuIndex == 0) {
            // --- FIX: SAVING LOGIC ---
            // We now save stats (Money, Rank) to a separate file to prevent overwriting.
            // Character appearance is saved to its own file.
            CharacterCreator_Save(characterFile);
            Money_Save(playerStatsFile);
            RankBar_Save(playerStatsFile);
            RpEvents_Save(xpFile);
            GunStore_Save();

            // Notify player
            UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
            UI::_ADD_TEXT_COMPONENT_STRING("Game Saved.");
            UI::_DRAW_NOTIFICATION(false, true);
        }
        else if (saveloadMenuIndex == 1) {
            // --- FIX: LOADING LOGIC ---
            // Use the new centralized loading function.
            LoadGameData();

            // Notify player
            UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
            UI::_ADD_TEXT_COMPONENT_STRING("Game Loaded.");
            UI::_DRAW_NOTIFICATION(false, true);
        }
        else if (saveloadMenuIndex == 2) {
            menuCategory = CAT_MAIN; menuIndex = 3; saveloadMenuIndex = 0; inputDelayFrames = 10;
        }
    }
    prevA = currA;
}

void LoadGameData()
{
    // --- FIX: MIGRATION-FRIENDLY LOADING SEQUENCE ---
    // This function will now handle loading all game data correctly,
    // ensuring that data from older save files is not lost.

    // 1. Load data from the old, combined character file first.
    // This loads the last-known money and rank into memory. If the user
    // has never saved with this new script version, this is the data we want.
    CharacterCreator_Load(characterFile);
    RankBar_Load(characterFile);
    Money_Load(characterFile);
    WAIT(100);

    // 2. Now, attempt to load from the NEW, separated player stats file.
    // If this file exists, it means the user has saved with the new system,
    // and this data is more current. It will safely overwrite the old data
    // we just loaded into memory. If it doesn't exist, nothing happens.
    RankBar_Load(playerStatsFile);
    Money_Load(playerStatsFile);
    WAIT(100);

    // 3. Load other data from their specific files.
    RpEvents_Load(xpFile);
    GunStore_Load();
    WAIT(100);

    // 4. Re-apply the character model after a short delay.
    // This helps ensure the appearance "sticks" as the game can sometimes reset it on load.
    CharacterCreator_Apply();
}


void SkipIntroAndLoadCharacter()
{
    // Start the timer for spawn protection
    spawnTime = GetTickCount64();

    CAM::DO_SCREEN_FADE_OUT(0);
    WAIT(700);

    NETWORK::NETWORK_END_TUTORIAL_SESSION();

    Player player = PLAYER::PLAYER_ID();
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    ENTITY::FREEZE_ENTITY_POSITION(playerPed, false);

    WAIT(1200);

    while (!PLAYER::IS_PLAYER_PLAYING(player)) WAIT(100);

    PLAYER::SET_PLAYER_CONTROL(player, true, 0);
    WAIT(400);

    CAM::DO_SCREEN_FADE_IN(800);

    // Wait a second for the game world to settle after fade-in
    WAIT(1000);

    // Use our new centralized loading function
    LoadGameData();
}

void ScriptMain() {
    Cheats_Init();
    RankBar_Init();
    Money_Init();
    CharacterCreator_Init();
    RpEvents_Init();
    CarShop_Init();
    Garage_Init();
    GunStore_Init();
    Credits_Init(); // <-- ADDED

    SkipIntroAndLoadCharacter();

    extern void (*Vehicle_DrawMenu)(int& menuIndex, int& menuCategory);
    srand((unsigned int)GetTickCount64());

    bool prevMenuCombo = false;
    bool prevB = false;

    while (true)
    {
        PollPad();

        // FIXED: Handle spawn protection
        if (spawnProtectionActive) {
            Ped playerPed = PLAYER::PLAYER_PED_ID();
            ENTITY::SET_ENTITY_INVINCIBLE(playerPed, true);
            // Check if 15 seconds have passed
            if (GetTickCount64() - spawnTime > 15000) {
                ENTITY::SET_ENTITY_INVINCIBLE(playerPed, false);
                spawnProtectionActive = false;
                // Notify the player
                UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
                UI::_ADD_TEXT_COMPONENT_STRING("Spawn protection has worn off.");
                UI::_DRAW_NOTIFICATION(false, true);
            }
        }

        // FIXED: Show welcome messages once, now with clear text.
        if (!welcomeMessagesShown && GetTickCount64() - spawnTime > 2000) { // Show after 2 seconds
            UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
            UI::_ADD_TEXT_COMPONENT_STRING("~b~GTA Offline by CreamyPlaytime Loaded");
            UI::_DRAW_NOTIFICATION(false, true);

            WAIT(4000); // Wait 4 seconds before showing the next one

            UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
            UI::_ADD_TEXT_COMPONENT_STRING("Press F4 (Keyboard) or RB + A (Controller) to open the menu.");
            UI::_DRAW_NOTIFICATION(false, true);

            welcomeMessagesShown = true;
        }


        Player player = PLAYER::PLAYER_ID();
        Ped playerPed = PLAYER::PLAYER_PED_ID();

        Cheats_Tick();
        RpEvents_Tick();
        RankBar_Tick();
        CharacterCreator_Tick();
        CarShop_Tick();
        Garage_Tick();
        GunStore_Tick();

        g_vehicleMenu.Tick();

        Money_PickupScan();

        bool menuCombo = PadHeld(BTN_RB) && PadHeld(BTN_A);

        if (!menuOpen && (IsKeyJustUp(VK_F4) || (menuCombo && !prevMenuCombo))) {
            menuOpen = true;
            menuIndex = 0;
            menuCategory = CAT_MAIN;
            inputDelayFrames = 15;
        }
        prevMenuCombo = menuCombo;

        bool currB = PadPressed(BTN_B);
        if (menuOpen && inputDelayFrames == 0 && ((currB && !prevB) || IsKeyJustUp(VK_NUMPAD0))) {
            if (menuCategory == CAT_MAIN) {
                menuOpen = false;
                menuIndex = 0;
                inputDelayFrames = 10;
            }
            else {
                menuCategory = CAT_MAIN;
                menuIndex = 0;
                inputDelayFrames = 10;
            }
        }
        prevB = currB;

        if (menuOpen)
        {
            CONTROLS::DISABLE_CONTROL_ACTION(0, 27, true);
            switch (menuCategory) {
            case CAT_MAIN:
                draw_main_menu();
                break;
            case CAT_CHARACTER:
                CharacterCreator_DrawMenu(menuIndex, menuCategory);
                break;
            case CAT_CHEATS:
                Cheats_DrawMenu(menuIndex, MENU_X, MENU_Y, MENU_W, MENU_H);
                break;
            case CAT_VEHICLE:
                Vehicle_DrawMenu(menuIndex, menuCategory);
                break;
            case CAT_SAVELOAD:
                draw_saveload_menu();
                break;
            case CAT_CAR_SHOP:
                draw_car_shop_menu();
                break;
            case CAT_GARAGE:
                draw_garage_menu();
                break;
            case CAT_GUN_STORE:
                draw_gun_store_menu();
                break;
            case CAT_CREDITS: // <-- CORRECTED
                draw_credits_menu();
                break;
            }
        }

        RankBar_DrawBar();
        Money_Draw();

        if (inputDelayFrames > 0) inputDelayFrames--;

        WAIT(0);
    }
}