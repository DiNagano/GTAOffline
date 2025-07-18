#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include "Properties.h"
#include "script.h"      // For UI drawing functions, ClampMenuIndex, etc.
#include "input.h"       // For keyboard and controller input (provides BTN_A)
#include "Money.h"       // For money operations (provides Money_Get, Money_Spend)
#include "RpEvents.h"    // Include RpEvents.h for RP functions
#include <cstdio>
#include <windows.h>     // Explicitly include for VK_E (0x45)
#include <algorithm>     // For std::remove_if
#include <fstream>       // For file operations (saving/loading)
#include <string>        // For std::to_string
#include "..\..\inc\natives.h" // Directly include natives.h for all native function definitions
#include "..\..\inc\main.h" // Still include main.h for other shared definitions

// --- Constants for RP and Money Generation ---
#define RP_INTERVAL_MS 300000    // 5 minutes in milliseconds (Adjusted for real users)
#define RP_AMOUNT 50            // RP granted per interval (K_ept as is)
#define MONEY_INTERVAL_MS 1800000 // 30 minutes in milliseconds for business money generation (Adjusted for real users)
#define BUSINESS_MONEY_CAP 100000 // Max money a business can hold (K_ept as is)

// Daily Visit Bonus
#define DAILY_BONUS_INTERVAL_MS 2880000 // 48 minutes (approx. 1 in-game day) (K_ept as is)
#define DAILY_BONUS_RP 100              // (K_ept as is)
#define DAILY_BONUS_CASH 10000          // (K_ept as is)

// Global vector to store all properties
std::vector<Property> g_properties;

// Global variable to store the name of the last used house for spawning
std::string g_lastUsedHouseName = ""; // CORRECTED: Definition of global variable

// Global state for tracking if player is inside a property that required teleport (e.g., a residence)
bool g_isPlayerInsideProperty = false;
// Global variable to store the interior ID of the property the player is currently teleported into
int g_currentTeleportedInteriorID = 0; // CORRECTED: Definition of global variable
// Global pointer to the property object the player is currently inside (only for teleported residences)
Property* g_activeTeleportedProperty = nullptr; // CORRECTED: Definition of global variable


// --- Internal Helper Functions ---

// Function to draw a 3D marker in the world
void DrawPropertyMarker(Vector3 coords, float scale, int colorR, int colorG, int colorB, int colorA) {
    GRAPHICS::DRAW_MARKER(
        1, // Type 1 is a vertical cylinder
        coords.x, coords.y, coords.z - 1.0f, // Adjust Z to sit on the ground
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        scale, scale, 1.0f, // X, Y, Z scale
        colorR, colorG, colorB, colorA,
        false, false, 2, false, nullptr, nullptr, false
    );
}

// Function to display notifications
void ShowNotification(const char* message) {
    UI::_SET_NOTIFICATION_TEXT_ENTRY(const_cast<char*>("STRING"));
    UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>(message));
    UI::_DRAW_NOTIFICATION(false, false); // Changed to false for auto-fade
}

// --- Property System Functions ---

void Properties_Init() {
    // Franklin's Strawberry House
    Property strawberryHouse;
    strawberryHouse.name = "Franklin's Strawberry House";
    strawberryHouse.type = RESIDENCE;
    strawberryHouse.exteriorCoords.x = -14.0491f; // Updated coordinate
    strawberryHouse.exteriorCoords.y = -1442.0615f; // Updated coordinate
    strawberryHouse.exteriorCoords.z = 31.1011f; // Updated coordinate
    strawberryHouse.exteriorHeading = 120.0f;
    strawberryHouse.interiorCoords.x = -14.2699f;
    strawberryHouse.interiorCoords.y = -1440.5303f;
    strawberryHouse.interiorCoords.z = 31.0949f;
    strawberryHouse.interiorHeading = 120.0f;
    strawberryHouse.price = 50000;
    strawberryHouse.isBought = false;
    strawberryHouse.blipSprite = 40; // House icon
    strawberryHouse.blipColorSale = 2; // Green for sale
    strawberryHouse.blipColorOwned = 5; // Blue for owned
    strawberryHouse.blipHandle = 0;
    // For RESIDENCES, interiorID can be 0 initially. It will be dynamically captured on entry.
    // However, if you want to use INTERIOR::_ENABLE_INTERIOR_PROP, you might need a valid ID here.
    strawberryHouse.interiorID = 0; // Set this to the actual interior ID if you know it for props
    strawberryHouse.lastRPTickTime = GAMEPLAY::GET_GAME_TIMER(); // Initialize RP timer
    strawberryHouse.lastCapNotifyTime = 0; // Not applicable for residences, but initialize
    strawberryHouse.lastDailyBonusTime = 0; // Not applicable for residences, but initialize
    g_properties.push_back(strawberryHouse);

    // Franklin's Vinewood Hills House
    Property vinewoodHillsHouse;
    vinewoodHillsHouse.name = "Franklin's Vinewood Hills House";
    vinewoodHillsHouse.type = RESIDENCE;
    vinewoodHillsHouse.exteriorCoords.x = 8.1841f;
    vinewoodHillsHouse.exteriorCoords.y = 539.5052f;
    vinewoodHillsHouse.exteriorCoords.z = 176.0280f;
    vinewoodHillsHouse.exteriorHeading = 270.0f;
    vinewoodHillsHouse.interiorCoords.x = 7.3074f;
    vinewoodHillsHouse.interiorCoords.y = 538.5178f;
    vinewoodHillsHouse.interiorCoords.z = 176.0280f;
    vinewoodHillsHouse.interiorHeading = 270.0f;
    vinewoodHillsHouse.price = 500000;
    vinewoodHillsHouse.isBought = false;
    vinewoodHillsHouse.blipSprite = 40; // House icon
    vinewoodHillsHouse.blipColorSale = 2; // Green for sale
    vinewoodHillsHouse.blipColorOwned = 5; // Blue for owned
    vinewoodHillsHouse.blipHandle = 0;
    vinewoodHillsHouse.interiorID = 0; // Set this to the actual interior ID if you know it for props
    vinewoodHillsHouse.lastRPTickTime = GAMEPLAY::GET_GAME_TIMER(); // Initialize RP timer
    vinewoodHillsHouse.lastCapNotifyTime = 0; // Not applicable for residences, but initialize
    vinewoodHillsHouse.lastDailyBonusTime = 0; // Not applicable for residences, but initialize
    g_properties.push_back(vinewoodHillsHouse);

    // Michael's Rockford Hills House
    Property michaelsHouse;
    michaelsHouse.name = "Michael's Rockford Hills House";
    michaelsHouse.type = RESIDENCE;
    michaelsHouse.exteriorCoords.x = -817.4041f; // Updated coordinate for Michael's House
    michaelsHouse.exteriorCoords.y = 177.9487f;  // Updated coordinate for Michael's House
    michaelsHouse.exteriorCoords.z = 72.2273f; // Updated coordinate for Michael's House
    michaelsHouse.exteriorHeading = 90.0f;
    michaelsHouse.interiorCoords.x = -815.8201f;
    michaelsHouse.interiorCoords.y = 178.7084f;
    michaelsHouse.interiorCoords.z = 72.1531f;
    michaelsHouse.interiorHeading = 90.0f;
    michaelsHouse.price = 750000;
    michaelsHouse.isBought = false;
    michaelsHouse.blipSprite = 40; // House icon
    michaelsHouse.blipColorSale = 2; // Green for sale
    michaelsHouse.blipColorOwned = 5; // Blue for owned
    michaelsHouse.blipHandle = 0;
    michaelsHouse.interiorID = 0; // Set this to the actual interior ID if you know it for props
    michaelsHouse.lastRPTickTime = GAMEPLAY::GET_GAME_TIMER(); // Initialize RP timer
    michaelsHouse.lastCapNotifyTime = 0; // Not applicable for residences, but initialize
    michaelsHouse.lastDailyBonusTime = 0; // Not applicable for residences, but initialize
    g_properties.push_back(michaelsHouse);

    // Trevor's Sandy Shores Trailer
    Property trevorsTrailer;
    trevorsTrailer.name = "Trevor's Sandy Shores Trailer";
    trevorsTrailer.type = RESIDENCE;
    trevorsTrailer.exteriorCoords.x = 1973.9448f;
    trevorsTrailer.exteriorCoords.y = 3815.1233f;
    trevorsTrailer.exteriorCoords.z = 33.4242f;
    trevorsTrailer.exteriorHeading = 180.0f; // Assuming a general heading
    trevorsTrailer.interiorCoords.x = 1973.2861f;
    trevorsTrailer.interiorCoords.y = 3816.1938f;
    trevorsTrailer.interiorCoords.z = 33.4257f;
    trevorsTrailer.interiorHeading = 180.0f; // Assuming a general heading
    trevorsTrailer.price = 25000; // Example price
    trevorsTrailer.isBought = false;
    trevorsTrailer.blipSprite = 40; // House icon
    trevorsTrailer.blipColorSale = 2; // Green for sale
    trevorsTrailer.blipColorOwned = 5; // Blue for owned
    trevorsTrailer.blipHandle = 0;
    trevorsTrailer.interiorID = 0; // Set this to the actual interior ID if you know it for props
    trevorsTrailer.lastRPTickTime = GAMEPLAY::GET_GAME_TIMER(); // Initialize RP timer
    trevorsTrailer.lastCapNotifyTime = 0; // Not applicable for residences, but initialize
    trevorsTrailer.lastDailyBonusTime = 0; // Not applicable for residences, but initialize
    g_properties.push_back(trevorsTrailer);

    // Vanilla Unicorn Strip Club (Business Property)
    Property vanillaUnicorn;
    vanillaUnicorn.name = "Vanilla Unicorn Strip Club";
    vanillaUnicorn.type = BUSINESS;
    vanillaUnicorn.exteriorCoords.x = 129.1989f;
    vanillaUnicorn.exteriorCoords.y = -1299.3987f;
    vanillaUnicorn.exteriorCoords.z = 29.2327f;
    vanillaUnicorn.exteriorHeading = 0.0f; // Default heading
    vanillaUnicorn.interiorCoords.x = 127.32f; // Example interior coord (not used for teleport, but for consistency)
    vanillaUnicorn.interiorCoords.y = -1299.27f;
    vanillaUnicorn.interiorCoords.z = 29.23f;
    vanillaUnicorn.interiorHeading = 0.0f; // Default heading
    vanillaUnicorn.price = 1000000; // Example price for a business
    vanillaUnicorn.isBought = false;
    vanillaUnicorn.blipSprite = 108; // Money Sign blip
    vanillaUnicorn.blipColorSale = 2; // Green for sale
    vanillaUnicorn.blipColorOwned = 5; // Blue for owned
    vanillaUnicorn.blipHandle = 0;
    // For BUSINESSES, it's good to have a known interiorID if you plan to enable specific props.
    vanillaUnicorn.interiorID = 0x81156B86; // Common Interior ID for Vanilla Unicorn. Verify in-game.
    vanillaUnicorn.generatedMoney = 0;
    vanillaUnicorn.moneyCap = BUSINESS_MONEY_CAP;
    vanillaUnicorn.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER(); // Initialize money timer
    vanillaUnicorn.collectCoords.x = 95.1566f; // Money collect marker
    vanillaUnicorn.collectCoords.y = -1294.4326f;
    vanillaUnicorn.collectCoords.z = 29.2688f;
    vanillaUnicorn.lastCapNotifyTime = 0; // Initialize notification timer
    vanillaUnicorn.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER(); // Initialize daily bonus timer
    g_properties.push_back(vanillaUnicorn);

    // Smoke on Water (Business)
    Property smokeOnWater;
    smokeOnWater.name = "Smoke on Water";
    smokeOnWater.type = BUSINESS;
    smokeOnWater.exteriorCoords.x = -1174.1201f;
    smokeOnWater.exteriorCoords.y = -1573.3291f;
    smokeOnWater.exteriorCoords.z = 4.3702f;
    smokeOnWater.exteriorHeading = 0.0f;
    smokeOnWater.interiorCoords = smokeOnWater.exteriorCoords; // Not used for teleport
    smokeOnWater.interiorHeading = 0.0f;
    smokeOnWater.price = 750000;
    smokeOnWater.isBought = false;
    smokeOnWater.blipSprite = 108; // Money Sign blip
    smokeOnWater.blipColorSale = 2;
    smokeOnWater.blipColorOwned = 5;
    smokeOnWater.blipHandle = 0;
    smokeOnWater.interiorID = 0; // No specific interior props to enable
    smokeOnWater.generatedMoney = 0;
    smokeOnWater.moneyCap = BUSINESS_MONEY_CAP;
    smokeOnWater.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();
    smokeOnWater.collectCoords = smokeOnWater.exteriorCoords; // Collect at exterior marker
    smokeOnWater.lastCapNotifyTime = 0;
    smokeOnWater.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();
    g_properties.push_back(smokeOnWater);

    // Towing Impound (Business)
    Property towingImpound;
    towingImpound.name = "Towing Impound";
    towingImpound.type = BUSINESS;
    towingImpound.exteriorCoords.x = 409.4931f;
    towingImpound.exteriorCoords.y = -1623.6489f;
    towingImpound.exteriorCoords.z = 29.2920f;
    towingImpound.exteriorHeading = 0.0f;
    towingImpound.interiorCoords = towingImpound.exteriorCoords;
    towingImpound.interiorHeading = 0.0f;
    towingImpound.price = 1200000;
    towingImpound.isBought = false;
    towingImpound.blipSprite = 108; // Money Sign blip
    towingImpound.blipColorSale = 2;
    towingImpound.blipColorOwned = 5;
    towingImpound.blipHandle = 0;
    towingImpound.interiorID = 0;
    towingImpound.generatedMoney = 0;
    towingImpound.moneyCap = BUSINESS_MONEY_CAP;
    towingImpound.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();
    towingImpound.collectCoords = towingImpound.exteriorCoords;
    towingImpound.lastCapNotifyTime = 0;
    towingImpound.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();
    g_properties.push_back(towingImpound);

    // Car ScrapYard (Business)
    Property carScrapYard;
    carScrapYard.name = "Car ScrapYard";
    carScrapYard.type = BUSINESS;
    carScrapYard.exteriorCoords.x = 1526.0765f;
    carScrapYard.exteriorCoords.y = -2108.8037f;
    carScrapYard.exteriorCoords.z = 76.7611f;
    carScrapYard.exteriorHeading = 0.0f;
    carScrapYard.interiorCoords = carScrapYard.exteriorCoords;
    carScrapYard.interiorHeading = 0.0f;
    carScrapYard.price = 900000;
    carScrapYard.isBought = false;
    carScrapYard.blipSprite = 108; // Money Sign blip
    carScrapYard.blipColorSale = 2;
    carScrapYard.blipColorOwned = 5;
    carScrapYard.blipHandle = 0;
    carScrapYard.interiorID = 0;
    carScrapYard.generatedMoney = 0;
    carScrapYard.moneyCap = BUSINESS_MONEY_CAP;
    carScrapYard.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();
    carScrapYard.collectCoords = carScrapYard.exteriorCoords;
    carScrapYard.lastCapNotifyTime = 0;
    carScrapYard.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();
    g_properties.push_back(carScrapYard);

    // Downtown Cab Co. (Business)
    Property downtownCabCo;
    downtownCabCo.name = "Downtown Cab Co.";
    downtownCabCo.type = BUSINESS;
    downtownCabCo.exteriorCoords.x = 895.7385f;
    downtownCabCo.exteriorCoords.y = -178.5212f;
    downtownCabCo.exteriorCoords.z = 74.7003f;
    downtownCabCo.exteriorHeading = 0.0f;
    downtownCabCo.interiorCoords = downtownCabCo.exteriorCoords;
    downtownCabCo.interiorHeading = 0.0f;
    downtownCabCo.price = 1500000;
    downtownCabCo.isBought = false;
    downtownCabCo.blipSprite = 108; // Money Sign blip
    downtownCabCo.blipColorSale = 2;
    downtownCabCo.blipColorOwned = 5;
    downtownCabCo.blipHandle = 0;
    downtownCabCo.interiorID = 0;
    downtownCabCo.generatedMoney = 0;
    downtownCabCo.moneyCap = BUSINESS_MONEY_CAP;
    downtownCabCo.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();
    downtownCabCo.collectCoords = downtownCabCo.exteriorCoords;
    downtownCabCo.lastCapNotifyTime = 0;
    downtownCabCo.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();
    g_properties.push_back(downtownCabCo);

    // Pitchers (Business)
    Property pitchers;
    pitchers.name = "Pitchers";
    pitchers.type = BUSINESS;
    pitchers.exteriorCoords.x = 224.2534f;
    pitchers.exteriorCoords.y = 337.4411f;
    pitchers.exteriorCoords.z = 105.6045f;
    pitchers.exteriorHeading = 0.0f;
    pitchers.interiorCoords = pitchers.exteriorCoords;
    pitchers.interiorHeading = 0.0f;
    pitchers.price = 600000;
    pitchers.isBought = false;
    pitchers.blipSprite = 108; // Money Sign blip
    pitchers.blipColorSale = 2;
    pitchers.blipColorOwned = 5;
    pitchers.blipHandle = 0;
    pitchers.interiorID = 0;
    pitchers.generatedMoney = 0;
    pitchers.moneyCap = BUSINESS_MONEY_CAP;
    pitchers.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();
    pitchers.collectCoords = pitchers.exteriorCoords;
    pitchers.lastCapNotifyTime = 0;
    pitchers.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();
    g_properties.push_back(pitchers);

    // Tequi-la-la (Business)
    Property tequilaLa;
    tequilaLa.name = "Tequi-la-la";
    tequilaLa.type = BUSINESS;
    tequilaLa.exteriorCoords.x = -561.8891f;
    tequilaLa.exteriorCoords.y = 273.8662f;
    tequilaLa.exteriorCoords.z = 83.0197f;
    tequilaLa.exteriorHeading = 0.0f;
    tequilaLa.interiorCoords = tequilaLa.exteriorCoords;
    tequilaLa.interiorHeading = 0.0f;
    tequilaLa.price = 800000;
    tequilaLa.isBought = false;
    tequilaLa.blipSprite = 108; // Money Sign blip
    tequilaLa.blipColorSale = 2;
    tequilaLa.blipColorOwned = 5;
    tequilaLa.blipHandle = 0;
    tequilaLa.interiorID = 0;
    tequilaLa.generatedMoney = 0;
    tequilaLa.moneyCap = BUSINESS_MONEY_CAP;
    tequilaLa.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();
    tequilaLa.collectCoords = tequilaLa.exteriorCoords;
    tequilaLa.lastCapNotifyTime = 0;
    tequilaLa.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();
    g_properties.push_back(tequilaLa);

    // Cinema Dopler (Business)
    Property cinemaDopler;
    cinemaDopler.name = "Cinema Dopler";
    cinemaDopler.type = BUSINESS;
    cinemaDopler.exteriorCoords.x = 340.0865f;
    cinemaDopler.exteriorCoords.y = 177.7011f;
    cinemaDopler.exteriorCoords.z = 103.0660f;
    cinemaDopler.exteriorHeading = 0.0f;
    cinemaDopler.interiorCoords = cinemaDopler.exteriorCoords;
    cinemaDopler.interiorHeading = 0.0f;
    cinemaDopler.price = 2000000;
    cinemaDopler.isBought = false;
    cinemaDopler.blipSprite = 108; // Money Sign blip
    cinemaDopler.blipColorSale = 2;
    cinemaDopler.blipColorOwned = 5;
    cinemaDopler.blipHandle = 0;
    cinemaDopler.interiorID = 0;
    cinemaDopler.generatedMoney = 0;
    cinemaDopler.moneyCap = BUSINESS_MONEY_CAP;
    cinemaDopler.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();
    cinemaDopler.collectCoords = cinemaDopler.exteriorCoords;
    cinemaDopler.lastCapNotifyTime = 0;
    cinemaDopler.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();
    g_properties.push_back(cinemaDopler);

    // Mckenzie Field Hanger (Business)
    Property mckenzieHanger;
    mckenzieHanger.name = "Mckenzie Field Hanger";
    mckenzieHanger.type = BUSINESS;
    mckenzieHanger.exteriorCoords.x = 2130.8220f;
    mckenzieHanger.exteriorCoords.y = 4791.7935f;
    mckenzieHanger.exteriorCoords.z = 41.0709f;
    mckenzieHanger.exteriorHeading = 0.0f;
    mckenzieHanger.interiorCoords = mckenzieHanger.exteriorCoords;
    mckenzieHanger.interiorHeading = 0.0f;
    mckenzieHanger.price = 2500000;
    mckenzieHanger.isBought = false;
    mckenzieHanger.blipSprite = 108; // Money Sign blip
    mckenzieHanger.blipColorSale = 2;
    mckenzieHanger.blipColorOwned = 5;
    mckenzieHanger.blipHandle = 0;
    mckenzieHanger.interiorID = 0;
    mckenzieHanger.generatedMoney = 0;
    mckenzieHanger.moneyCap = BUSINESS_MONEY_CAP;
    mckenzieHanger.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();
    mckenzieHanger.collectCoords = mckenzieHanger.exteriorCoords;
    mckenzieHanger.lastCapNotifyTime = 0;
    mckenzieHanger.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();
    g_properties.push_back(mckenzieHanger);

    // The Hen House (Business)
    Property henHouse;
    henHouse.name = "The Hen House";
    henHouse.type = BUSINESS;
    henHouse.exteriorCoords.x = -297.4973f;
    henHouse.exteriorCoords.y = 6255.6665f;
    henHouse.exteriorCoords.z = 31.4873f;
    henHouse.exteriorHeading = 0.0f;
    henHouse.interiorCoords = henHouse.exteriorCoords;
    henHouse.interiorHeading = 0.0f;
    henHouse.price = 500000;
    henHouse.isBought = false;
    henHouse.blipSprite = 108; // Money Sign blip
    henHouse.blipColorSale = 2;
    henHouse.blipColorOwned = 5;
    henHouse.blipHandle = 0;
    henHouse.interiorID = 0;
    henHouse.generatedMoney = 0;
    henHouse.moneyCap = BUSINESS_MONEY_CAP;
    henHouse.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();
    henHouse.collectCoords = henHouse.exteriorCoords;
    henHouse.lastCapNotifyTime = 0;
    henHouse.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();
    g_properties.push_back(henHouse);

    // Sonar Collections Dock (Business)
    Property sonarDock;
    sonarDock.name = "Sonar Collections Dock";
    sonarDock.type = BUSINESS;
    sonarDock.exteriorCoords.x = -1582.2504f;
    sonarDock.exteriorCoords.y = 5196.1128f;
    sonarDock.exteriorCoords.z = 3.9882f;
    sonarDock.exteriorHeading = 0.0f;
    sonarDock.interiorCoords = sonarDock.exteriorCoords;
    sonarDock.interiorCoords.y = sonarDock.exteriorCoords.y;
    sonarDock.interiorCoords.z = sonarDock.exteriorCoords.z;
    sonarDock.interiorHeading = 0.0f;
    sonarDock.price = 2000000;
    sonarDock.isBought = false;
    sonarDock.blipSprite = 108; // Money Sign blip
    sonarDock.blipColorSale = 2;
    sonarDock.blipColorOwned = 5;
    sonarDock.blipHandle = 0;
    sonarDock.interiorID = 0;
    sonarDock.generatedMoney = 0;
    sonarDock.moneyCap = BUSINESS_MONEY_CAP;
    sonarDock.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();
    sonarDock.collectCoords = sonarDock.exteriorCoords;
    sonarDock.lastCapNotifyTime = 0;
    sonarDock.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();
    g_properties.push_back(sonarDock);

    // Hookies (Business)
    Property hookies;
    hookies.name = "Hookies";
    hookies.type = BUSINESS;
    hookies.exteriorCoords.x = -2194.3696f;
    hookies.exteriorCoords.y = 4290.9189f;
    hookies.exteriorCoords.z = 49.1738f;
    hookies.exteriorHeading = 0.0f;
    hookies.interiorCoords = hookies.exteriorCoords; // Not used for teleport
    hookies.interiorHeading = 0.0f;
    hookies.price = 700000; // Example price
    hookies.isBought = false;
    hookies.blipSprite = 108; // Money Sign blip
    hookies.blipColorSale = 2;
    hookies.blipColorOwned = 5;
    hookies.blipHandle = 0;
    hookies.interiorID = 0;
    hookies.generatedMoney = 0;
    hookies.moneyCap = BUSINESS_MONEY_CAP;
    hookies.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();
    hookies.collectCoords = hookies.exteriorCoords; // Collect at exterior marker
    hookies.lastCapNotifyTime = 0;
    hookies.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();
    g_properties.push_back(hookies);

    // Los Santos Golf Course (Business)
    Property golfCourse;
    golfCourse.name = "Los Santos Golf Course";
    golfCourse.type = BUSINESS;
    golfCourse.exteriorCoords.x = -1371.2993f;
    golfCourse.exteriorCoords.y = 49.2612f;
    golfCourse.exteriorCoords.z = 53.7014f;
    golfCourse.exteriorHeading = 0.0f;
    golfCourse.interiorCoords = golfCourse.exteriorCoords; // Not used for teleport
    golfCourse.interiorHeading = 0.0f;
    golfCourse.price = 750000; // Custom price
    golfCourse.isBought = false;
    golfCourse.blipSprite = 108; // Money Sign blip
    golfCourse.blipColorSale = 2;
    golfCourse.blipColorOwned = 5;
    golfCourse.blipHandle = 0;
    golfCourse.interiorID = 0;
    golfCourse.generatedMoney = 0;
    golfCourse.moneyCap = 80000; // Custom money cap
    golfCourse.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();
    golfCourse.collectCoords = golfCourse.exteriorCoords; // Collect at exterior marker
    golfCourse.lastCapNotifyTime = 0;
    golfCourse.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();
    g_properties.push_back(golfCourse);

    // Tivoli Cinema (Business)
    Property tivoliCinema;
    tivoliCinema.name = "Tivoli Cinema";
    tivoliCinema.type = BUSINESS;
    tivoliCinema.exteriorCoords.x = -1422.6783f;
    tivoliCinema.exteriorCoords.y = -214.2846f;
    tivoliCinema.exteriorCoords.z = 46.5004f;
    tivoliCinema.exteriorHeading = 0.0f;
    tivoliCinema.interiorCoords = tivoliCinema.exteriorCoords; // Not used for teleport
    tivoliCinema.interiorHeading = 0.0f;
    tivoliCinema.price = 600000; // Custom price
    tivoliCinema.isBought = false;
    tivoliCinema.blipSprite = 108; // Money Sign blip
    tivoliCinema.blipColorSale = 2;
    tivoliCinema.blipColorOwned = 5;
    tivoliCinema.blipHandle = 0;
    tivoliCinema.interiorID = 0;
    tivoliCinema.generatedMoney = 0;
    tivoliCinema.moneyCap = 70000; // Custom money cap
    tivoliCinema.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();
    tivoliCinema.collectCoords = tivoliCinema.exteriorCoords; // Collect at exterior marker
    tivoliCinema.lastCapNotifyTime = 0;
    tivoliCinema.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();
    g_properties.push_back(tivoliCinema);

    // Los Santos Custom (Business)
    Property lsCustom;
    lsCustom.name = "Los Santos Custom";
    lsCustom.type = BUSINESS;
    lsCustom.exteriorCoords.x = 1188.3818f; // Buy marker
    lsCustom.exteriorCoords.y = 2645.7317f;
    lsCustom.exteriorCoords.z = 38.3641f;
    lsCustom.exteriorHeading = 0.0f;
    lsCustom.interiorCoords = lsCustom.exteriorCoords; // Not used for teleport
    lsCustom.interiorHeading = 0.0f;
    lsCustom.price = 1200000; // Custom price
    lsCustom.isBought = false;
    lsCustom.blipSprite = 108; // Money Sign blip
    lsCustom.blipColorSale = 2;
    lsCustom.blipColorOwned = 5;
    lsCustom.blipHandle = 0;
    lsCustom.interiorID = 0;
    lsCustom.generatedMoney = 0;
    lsCustom.moneyCap = 120000; // Custom money cap
    lsCustom.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();
    lsCustom.collectCoords.x = 1187.2616f; // Specific money collect marker
    lsCustom.collectCoords.y = 2637.0435f;
    lsCustom.collectCoords.z = 38.4019f;
    lsCustom.lastCapNotifyTime = 0;
    lsCustom.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();
    g_properties.push_back(lsCustom);

    // Lester's House (Residence)
    Property lestersHouse;
    lestersHouse.name = "Lester's House";
    lestersHouse.type = RESIDENCE;
    lestersHouse.exteriorCoords.x = 1275.0961f;
    lestersHouse.exteriorCoords.y = -1721.5277f;
    lestersHouse.exteriorCoords.z = 54.6513f;
    lestersHouse.exteriorHeading = 0.0f; // Default heading
    lestersHouse.interiorCoords.x = 1274.1108f;
    lestersHouse.interiorCoords.y = -1719.6858f;
    lestersHouse.interiorCoords.z = 54.7682f;
    lestersHouse.interiorHeading = 0.0f; // Default heading
    lestersHouse.price = 300000; // Example price
    lestersHouse.isBought = false;
    lestersHouse.blipSprite = 40; // House icon
    lestersHouse.blipColorSale = 2; // Green for sale
    lestersHouse.blipColorOwned = 5; // Blue for owned
    lestersHouse.blipHandle = 0;
    lestersHouse.interiorID = 0; // Will be dynamically captured
    lestersHouse.lastRPTickTime = GAMEPLAY::GET_GAME_TIMER();
    lestersHouse.lastCapNotifyTime = 0;
    lestersHouse.lastDailyBonusTime = 0;
    g_properties.push_back(lestersHouse);

    // Split Sides Comedy Club (Business) West Vinewood - Eclipse Boulevard
    Property splitSides;                                           // property name ID
    splitSides.name = "Split Sides Comedy Club";                   // Ingame property name
    splitSides.type = BUSINESS;                                    // Property type
    splitSides.exteriorCoords.x = -430.4311f;                      // Buy marker X pos
    splitSides.exteriorCoords.y = 288.9801f;                       // Buy marker Y pos
    splitSides.exteriorCoords.z = 86.0618f;                        // Buy marker Z pos
    splitSides.exteriorHeading = 0.0f;                             // Default heading
    splitSides.interiorCoords = splitSides.exteriorCoords;         // Not used for teleport
    splitSides.interiorHeading = 0.0f;                             // Default heading
    splitSides.price = 750000;                                     // Custom price
    splitSides.isBought = false;                                   // Property for sale - sold
    splitSides.blipSprite = 108;                                   // Money Sign blip
    splitSides.blipColorSale = 2;                                  // Green for sale
    splitSides.blipColorOwned = 5;                                 // Blue for owned
    splitSides.blipHandle = 0;                                     // Property Map Blip creation int
    splitSides.interiorID = 0;                                     // Set this to the actual interior ID if you know it for props
    splitSides.generatedMoney = 0;                                 // Money Generated
    splitSides.moneyCap = BUSINESS_MONEY_CAP;                      // Default Money Cap 
    splitSides.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();     // Initialize money timer
    splitSides.collectCoords = splitSides.exteriorCoords;          // Collect at exterior marker
    splitSides.lastCapNotifyTime = 0;                              // Initialize notification timer
    splitSides.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();    // Initialize daily bonus timer
    g_properties.push_back(splitSides);                            // get props count 

    // Bahama Mamas Nightclub (Business) Del Perro - Marathon Avenue/Prosperity Street Promenade
    Property bahamaMamas;                                          // property name ID
    bahamaMamas.name = "Bahama Mamas Nightclub";                   // Ingame property name
    bahamaMamas.type = BUSINESS;                                   // Property type
    bahamaMamas.exteriorCoords.x = -1389.1651f;                    // Buy marker X pos
    bahamaMamas.exteriorCoords.y = -585.6961f;                     // Buy marker Y pos
    bahamaMamas.exteriorCoords.z = 30.2203f;                       // Buy marker Z pos
    bahamaMamas.exteriorHeading = 0.0f;                            // Default heading
    bahamaMamas.interiorCoords = bahamaMamas.exteriorCoords;       // Not used for teleport
    bahamaMamas.interiorHeading = 0.0f;                            // Default heading
    bahamaMamas.price = 1250000;                                   // Custom price
    bahamaMamas.isBought = false;                                  // Property for sale - sold
    bahamaMamas.blipSprite = 108;                                  // Money Sign blip
    bahamaMamas.blipColorSale = 2;                                 // Green for sale
    bahamaMamas.blipColorOwned = 5;                                // Blue for owned
    bahamaMamas.blipHandle = 0;                                    // Property Map Blip creation int
    bahamaMamas.interiorID = 0;                                    // Set this to the actual interior ID if you know it for props
    bahamaMamas.generatedMoney = 0;                                // Money Generated
    bahamaMamas.moneyCap = BUSINESS_MONEY_CAP;                     // Default Money Cap
    bahamaMamas.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();    // Initialize money timer
    bahamaMamas.collectCoords = bahamaMamas.exteriorCoords;        // Collect at exterior marker
    bahamaMamas.lastCapNotifyTime = 0;                             // Initialize notification timer
    bahamaMamas.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();   // Initialize daily bonus timer
    g_properties.push_back(bahamaMamas);                           // get props count 

    // Up-n-Atom Burger (Business) Downtown Vinewood - Vinewood Boulevard/Las Lagunas Boulevard
    Property upnAtom;                                              // property name ID
    upnAtom.name = "Up-n-Atom Burger";                             // Ingame property name
    upnAtom.type = BUSINESS;                                       // Property type
    upnAtom.exteriorCoords.x = 80.7293f;                           // Buy marker X pos
    upnAtom.exteriorCoords.y = 273.4818f;                          // Buy marker Y pos
    upnAtom.exteriorCoords.z = 110.2102f;                          // Buy marker Z pos
    upnAtom.exteriorHeading = 0.0f;                                // Default heading
    upnAtom.interiorCoords = upnAtom.exteriorCoords;               // Not used for teleport
    upnAtom.interiorHeading = 0.0f;                                // Default heading
    upnAtom.price = 900000;                                        // Custom price
    upnAtom.isBought = false;                                      // Property for sale - sold
    upnAtom.blipSprite = 108;                                      // Money Sign blip
    upnAtom.blipColorSale = 2;                                     // Green for sale
    upnAtom.blipColorOwned = 5;                                    // Blue for owned
    upnAtom.blipHandle = 0;                                        // Property Map Blip creation int
    upnAtom.interiorID = 0;                                        // Set this to the actual interior ID if you know it for props
    upnAtom.generatedMoney = 0;                                    // Money Generated
    upnAtom.moneyCap = BUSINESS_MONEY_CAP;                         // Default Money Cap
    upnAtom.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();        // Initialize money timer
    upnAtom.collectCoords = upnAtom.exteriorCoords;                // Collect at exterior marker
    upnAtom.lastCapNotifyTime = 0;                                 // Initialize notification timer
    upnAtom.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();       // Initialize daily bonus timer
    g_properties.push_back(upnAtom);                               // get props count

    // Oriental Theater (Business) Downtown Vinewood - Vinewood Boulevard
    Property orientTheater;                                        // property name ID
    orientTheater.name = "Oriental Theater";                       // Ingame property name
    orientTheater.type = BUSINESS;                                 // Property type
    orientTheater.exteriorCoords.x = 301.0014f;                    // Buy marker X pos
    orientTheater.exteriorCoords.y = 201.7462f;                    // Buy marker Y pos
    orientTheater.exteriorCoords.z = 104.3731f;                    // Buy marker Z pos
    orientTheater.exteriorHeading = 0.0f;                          // Default heading
    orientTheater.interiorCoords = orientTheater.exteriorCoords;   // Not used for teleport
    orientTheater.interiorHeading = 0.0f;                          // Default heading
    orientTheater.price = 8000000;                                 // Custom price
    orientTheater.isBought = false;                                // Property for sale - sold
    orientTheater.blipSprite = 108;                                // Money Sign blip
    orientTheater.blipColorSale = 2;                               // Green for sale
    orientTheater.blipColorOwned = 5;                              // Blue for owned
    orientTheater.blipHandle = 0;                                  // Property Map Blip creation int
    orientTheater.interiorID = 0;                                  // Set this to the actual interior ID if you know it for props
    orientTheater.generatedMoney = 0;                              // Money Generated
    orientTheater.moneyCap = BUSINESS_MONEY_CAP;                   // Default Money Cap
    orientTheater.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();  // Initialize money timer
    orientTheater.collectCoords = orientTheater.exteriorCoords;    // Collect at exterior marker
    orientTheater.lastCapNotifyTime = 0;                           // Initialize notification timer
    orientTheater.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER(); // Initialize daily bonus timer
    g_properties.push_back(orientTheater);                         // get props count

    // Hornbills Strip Club (Business) West Vinewood - Eclipse Boulevard
    Property hornBills;                                            // property name ID
    hornBills.name = "Hornbills Strip Club";                       // Ingame property name
    hornBills.type = BUSINESS;                                     // Property type
    hornBills.exteriorCoords.x = -379.4113f;                       // Buy marker X pos
    hornBills.exteriorCoords.y = 220.8403f;                        // Buy marker Y pos
    hornBills.exteriorCoords.z = 83.9893f;                         // Buy marker Z pos
    hornBills.exteriorHeading = 0.0f;                              // Default heading
    hornBills.interiorCoords = hornBills.exteriorCoords;           // Not used for teleport
    hornBills.interiorHeading = 0.0f;                              // Default heading
    hornBills.price = 2000000;                                     // Custom price
    hornBills.isBought = false;                                    // Property for sale - sold
    hornBills.blipSprite = 108;                                    // Money Sign blip
    hornBills.blipColorSale = 2;                                   // Green for sale
    hornBills.blipColorOwned = 5;                                  // Blue for owned
    hornBills.blipHandle = 0;                                      // Property Map Blip creation int
    hornBills.interiorID = 0;                                      // Set this to the actual interior ID if you know it for props
    hornBills.generatedMoney = 0;                                  // Money Generated
    hornBills.moneyCap = BUSINESS_MONEY_CAP;                       // Default Money Cap
    hornBills.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();      // Initialize money timer
    hornBills.collectCoords = hornBills.exteriorCoords;            // Collect at exterior marker
    hornBills.lastCapNotifyTime = 0;                               // Initialize notification timer
    hornBills.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();     // Initialize daily bonus timer
    g_properties.push_back(hornBills);                             // get props count

    // You Tool Store (Business) Gran Senora Desert - Senora Freeway
    Property youTool;                                              // property name ID
    youTool.name = "You Tool Store";                               // Ingame property name
    youTool.type = BUSINESS;                                       // Property type
    youTool.exteriorCoords.x = 2748.6299f;                         // Buy marker X pos
    youTool.exteriorCoords.y = 3472.5069f;                         // Buy marker Y pos
    youTool.exteriorCoords.z = 55.6767f;                           // Buy marker Z pos
    youTool.exteriorHeading = 0.0f;                                // Default heading
    youTool.interiorCoords = youTool.exteriorCoords;               // Not used for teleport
    youTool.interiorHeading = 0.0f;                                // Default heading
    youTool.price = 1000000;                                       // Custom price
    youTool.isBought = false;                                      // Property for sale - sold
    youTool.blipSprite = 108;                                      // Money Sign blip
    youTool.blipColorSale = 2;                                     // Green for sale
    youTool.blipColorOwned = 5;                                    // Blue for owned
    youTool.blipHandle = 0;                                        // Property Map Blip creation int
    youTool.interiorID = 0;                                        // Set this to the actual interior ID if you know it for props
    youTool.generatedMoney = 0;                                    // Money Generated
    youTool.moneyCap = BUSINESS_MONEY_CAP;                         // Default Money Cap
    youTool.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();        // Initialize money timer
    youTool.collectCoords = youTool.exteriorCoords;                // Collect at exterior marker
    youTool.lastCapNotifyTime = 0;                                 // Initialize notification timer
    youTool.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();       // Initialize daily bonus timer
    g_properties.push_back(youTool);                               // get props count

    // Davis Mega Mall (Business) Davis - Davis Avenue
    Property davisMega;                                            // property name ID
    davisMega.name = "Davis Mega Mall";                            // Ingame property name
    davisMega.type = BUSINESS;                                     // Property type
    davisMega.exteriorCoords.x = 28.9999f;                         // Buy marker X pos
    davisMega.exteriorCoords.y = -1770.0999f;                      // Buy marker Y pos
    davisMega.exteriorCoords.z = 29.6001f;                         // Buy marker Z pos
    davisMega.exteriorHeading = 0.0f;                              // Default heading
    davisMega.interiorCoords = davisMega.exteriorCoords;           // Not used for teleport
    davisMega.interiorHeading = 0.0f;                              // Default heading
    davisMega.price = 1000000;                                     // Custom price
    davisMega.isBought = false;                                    // Property for sale - sold
    davisMega.blipSprite = 108;                                    // Money Sign blip
    davisMega.blipColorSale = 2;                                   // Green for sale
    davisMega.blipColorOwned = 5;                                  // Blue for owned
    davisMega.blipHandle = 0;                                      // Property Map Blip creation int
    davisMega.interiorID = 0;                                      // Set this to the actual interior ID if you know it for props
    davisMega.generatedMoney = 0;                                  // Money Generated
    davisMega.moneyCap = BUSINESS_MONEY_CAP;                       // Default Money Cap
    davisMega.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();      // Initialize money timer
    davisMega.collectCoords = davisMega.exteriorCoords;            // Collect at exterior marker
    davisMega.lastCapNotifyTime = 0;                               // Initialize notification timer
    davisMega.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();     // Initialize daily bonus timer
    g_properties.push_back(davisMega);                             // get props count

    // Animal Ark (Business) Harmony - Route 68
    Property animalArk;                                            // property name ID
    animalArk.name = "Animal Ark";                                 // Ingame property name
    animalArk.type = BUSINESS;                                     // Property type
    animalArk.exteriorCoords.x = 561.9999f;                        // Buy marker X pos
    animalArk.exteriorCoords.y = 2741.2999f;                       // Buy marker Y pos
    animalArk.exteriorCoords.z = 42.8999f;                         // Buy marker Z pos
    animalArk.exteriorHeading = 0.0f;                              // Default heading
    animalArk.interiorCoords = animalArk.exteriorCoords;           // Not used for teleport
    animalArk.interiorHeading = 0.0f;                              // Default heading
    animalArk.price = 800000;                                      // Custom price
    animalArk.isBought = false;                                    // Property for sale - sold
    animalArk.blipSprite = 108;                                    // Money Sign blip
    animalArk.blipColorSale = 2;                                   // Green for sale
    animalArk.blipColorOwned = 5;                                  // Blue for owned
    animalArk.blipHandle = 0;                                      // Property Map Blip creation int
    animalArk.interiorID = 0;                                      // Set this to the actual interior ID if you know it for props
    animalArk.generatedMoney = 0;                                  // Money Generated
    animalArk.moneyCap = BUSINESS_MONEY_CAP;                       // Default Money Cap
    animalArk.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();      // Initialize money timer
    animalArk.collectCoords = animalArk.exteriorCoords;            // Collect at exterior marker
    animalArk.lastCapNotifyTime = 0;                               // Initialize notification timer
    animalArk.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();     // Initialize daily bonus timer
    g_properties.push_back(animalArk);                             // get props count

    // Pearls Seafood (Business) Del Perro - Del Perro Pier
    Property pearlSeaf;                                            // property name ID
    pearlSeaf.name = "Pearls Seafood";                             // Ingame property name
    pearlSeaf.type = BUSINESS;                                     // Property type
    pearlSeaf.exteriorCoords.x = -1816.1009f;                        // Buy marker X pos
    pearlSeaf.exteriorCoords.y = -1192.2409f;                       // Buy marker Y pos
    pearlSeaf.exteriorCoords.z = 14.3048f;                         // Buy marker Z pos
    pearlSeaf.exteriorHeading = 0.0f;                              // Default heading
    pearlSeaf.interiorCoords = pearlSeaf.exteriorCoords;           // Not used for teleport
    pearlSeaf.interiorHeading = 0.0f;                              // Default heading
    pearlSeaf.price = 900000;                                      // Custom price
    pearlSeaf.isBought = false;                                    // Property for sale - sold
    pearlSeaf.blipSprite = 108;                                    // Money Sign blip
    pearlSeaf.blipColorSale = 2;                                   // Green for sale
    pearlSeaf.blipColorOwned = 5;                                  // Blue for owned
    pearlSeaf.blipHandle = 0;                                      // Property Map Blip creation int
    pearlSeaf.interiorID = 0;                                      // Set this to the actual interior ID if you know it for props
    pearlSeaf.generatedMoney = 0;                                  // Money Generated
    pearlSeaf.moneyCap = BUSINESS_MONEY_CAP;                       // Default Money Cap
    pearlSeaf.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();      // Initialize money timer
    pearlSeaf.collectCoords = pearlSeaf.exteriorCoords;            // Collect at exterior marker
    pearlSeaf.lastCapNotifyTime = 0;                               // Initialize notification timer
    pearlSeaf.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();     // Initialize daily bonus timer
    g_properties.push_back(pearlSeaf);                             // get props count

    // Burger Shot (Business) Vespucci Canals - Prosperity Street
    Property burgerShot;                                           // property name ID
    burgerShot.name = "Burger Shot";                               // Ingame property name
    burgerShot.type = BUSINESS;                                    // Property type
    burgerShot.exteriorCoords.x = -1182.3859f;                     // Buy marker X pos
    burgerShot.exteriorCoords.y = -883.5986f;                      // Buy marker Y pos
    burgerShot.exteriorCoords.z = 13.7759f;                        // Buy marker Z pos
    burgerShot.exteriorHeading = 0.0f;                             // Default heading
    burgerShot.interiorCoords = burgerShot.exteriorCoords;         // Not used for teleport
    burgerShot.interiorHeading = 0.0f;                             // Default heading
    burgerShot.price = 900000;                                     // Custom price
    burgerShot.isBought = false;                                   // Property for sale - sold
    burgerShot.blipSprite = 108;                                   // Money Sign blip
    burgerShot.blipColorSale = 2;                                  // Green for sale
    burgerShot.blipColorOwned = 5;                                 // Blue for owned
    burgerShot.blipHandle = 0;                                     // Property Map Blip creation int
    burgerShot.interiorID = 0;                                     // Set this to the actual interior ID if you know it for props
    burgerShot.generatedMoney = 0;                                 // Money Generated
    burgerShot.moneyCap = BUSINESS_MONEY_CAP;                      // Default Money Cap
    burgerShot.lastMoneyTickTime = GAMEPLAY::GET_GAME_TIMER();     // Initialize money timer
    burgerShot.collectCoords = burgerShot.exteriorCoords;          // Collect at exterior marker
    burgerShot.lastCapNotifyTime = 0;                              // Initialize notification timer
    burgerShot.lastDailyBonusTime = GAMEPLAY::GET_GAME_TIMER();    // Initialize daily bonus timer
    g_properties.push_back(burgerShot);                            // get props count

    // Initialize blips for all properties
    for (size_t i = 0; i < g_properties.size(); ++i) {
        Property& prop = g_properties[i];
        // Create blip at exterior coords initially
        prop.blipHandle = UI::ADD_BLIP_FOR_COORD(prop.exteriorCoords.x, prop.exteriorCoords.y, prop.exteriorCoords.z);
        UI::SET_BLIP_SPRITE(prop.blipHandle, prop.blipSprite);
        UI::SET_BLIP_SCALE(prop.blipHandle, 0.8f); // Smaller blip
        UI::SET_BLIP_AS_SHORT_RANGE(prop.blipHandle, true); // Only show when nearby
        UI::BEGIN_TEXT_COMMAND_SET_BLIP_NAME(const_cast<char*>("STRING"));
        UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>(prop.name.c_str())); // Using UI::_ADD_TEXT_COMPONENT_STRING as a fallback
        UI::END_TEXT_COMMAND_SET_BLIP_NAME(prop.blipHandle);

        // Set initial blip color based on bought status
        if (prop.isBought) {
            UI::SET_BLIP_COLOUR(prop.blipHandle, prop.blipColorOwned); // Use defined owned color
            // If already bought and a business, move blip to collect coords on init
            if (prop.type == BUSINESS) {
                UI::SET_BLIP_COORDS(prop.blipHandle, prop.collectCoords.x, prop.collectCoords.y, prop.collectCoords.z);
            }
        }
        else {
            UI::SET_BLIP_COLOUR(prop.blipHandle, prop.blipColorSale); // Use defined sale color
        }
    }
    g_isPlayerInsideProperty = false; // Ensure initial state is outside
    g_currentTeleportedInteriorID = 0; // No interior active on init
    g_activeTeleportedProperty = nullptr; // No active property on init
}

void Properties_Tick() {
    Player player = PLAYER::PLAYER_ID();
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    Vector3 playerCoords = ENTITY::GET_ENTITY_COORDS(playerPed, true);
    DWORD currentTime = GAMEPLAY::GET_GAME_TIMER();
    extern int inputDelayFrames; // Declare extern to use global inputDelayFrames

    // Check if player has left a teleported interior (e.g., by walking out through an unintended exit)
    // This logic ensures g_isPlayerInsideProperty and g_currentTeleportedInteriorID are reset correctly.
    if (g_isPlayerInsideProperty && g_currentTeleportedInteriorID != 0 && g_activeTeleportedProperty != nullptr) {
        int actualInteriorID = INTERIOR::GET_INTERIOR_FROM_ENTITY(playerPed);
        // If player is no longer in any interior, or in a different interior than the one they teleported into
        if (actualInteriorID == 0 || actualInteriorID != g_currentTeleportedInteriorID) {
            ShowNotification(std::string("~y~You have left " + g_activeTeleportedProperty->name + ".").c_str());
            g_isPlayerInsideProperty = false;
            g_currentTeleportedInteriorID = 0;
            g_activeTeleportedProperty = nullptr; // Reset active property
        }
    }

    // Iterate through properties to check for player proximity and handle interactions
    for (size_t i = 0; i < g_properties.size(); ++i) {
        Property& prop = g_properties[i];
        float distanceToExterior = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(playerCoords.x, playerCoords.y, playerCoords.z,
            prop.exteriorCoords.x, prop.exteriorCoords.y, prop.exteriorCoords.z, true);
        float distanceToInterior = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(playerCoords.x, playerCoords.y, playerCoords.z,
            prop.interiorCoords.x, prop.interiorCoords.y, prop.interiorCoords.z, true);

        // --- Passive RP for Residences ---
        // Only grant RP if it's a residence, owned, and the player is physically inside the *active* teleported interior
        if (prop.type == RESIDENCE && prop.isBought && g_isPlayerInsideProperty && g_activeTeleportedProperty == &prop && g_currentTeleportedInteriorID != 0 && INTERIOR::GET_INTERIOR_FROM_ENTITY(playerPed) == g_currentTeleportedInteriorID) {
            if ((currentTime - prop.lastRPTickTime) > RP_INTERVAL_MS) {
                RpEvents_Reward(RP_AMOUNT, std::string("~g~Passive Rp For Staying At " + prop.name).c_str()); // Updated RP notification
                prop.lastRPTickTime = currentTime;
            }
        }

        // --- Business Money Generation ---
        if (prop.type == BUSINESS && prop.isBought) {
            if ((currentTime - prop.lastMoneyTickTime) > MONEY_INTERVAL_MS) {
                if (prop.generatedMoney < prop.moneyCap) {
                    // Generate a random amount between 20000 and 50000
                    int generatedAmount = (rand() % 30001) + 20000;
                    prop.generatedMoney += generatedAmount;
                    if (prop.generatedMoney > prop.moneyCap) {
                        prop.generatedMoney = prop.moneyCap;
                    }
                    ShowNotification(std::string("~g~" + prop.name + " generated ~w~$" + std::to_string(generatedAmount) + "!").c_str());
                }
                prop.lastMoneyTickTime = currentTime;
            }

            // --- Money Collection Marker for Businesses ---
            if (prop.generatedMoney > 0) {
                float distanceToCollect = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(playerCoords.x, playerCoords.y, playerCoords.z,
                    prop.collectCoords.x, prop.collectCoords.y, prop.collectCoords.z, true);

                // Draw the marker if within 75f
                if (distanceToCollect < 75.0f) {
                    DrawPropertyMarker(prop.collectCoords, 1.0f, 255, 255, 0, 150); // Yellow marker for money
                }

                // Show combined prompt only when within interaction range (1.5f)
                if (distanceToCollect < 1.5f) {
                    char displayMsg[256];
                    sprintf_s(displayMsg, "~w~%s~n~~y~Collect Money: ~g~$%d~n~~y~Press E / (A) to collect", prop.name.c_str(), prop.generatedMoney);
                    UI::_SET_NOTIFICATION_TEXT_ENTRY(const_cast<char*>("STRING"));
                    UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>(displayMsg));
                    UI::_DRAW_NOTIFICATION(false, false); // Auto-fade

                    if (inputDelayFrames == 0 && (IsKeyJustUp(0x45) || PadPressed(BTN_A))) { // 'E' key or A button
                        Money_Add(prop.generatedMoney);
                        ShowNotification(std::string("~g~Collected ~w~$" + std::to_string(prop.generatedMoney) + " from " + prop.name + "!").c_str());

                        // Daily Visit Bonus
                        if ((currentTime - prop.lastDailyBonusTime) > DAILY_BONUS_INTERVAL_MS) {
                            RpEvents_Reward(DAILY_BONUS_RP, std::string("~g~Daily Visit Bonus! +" + std::to_string(DAILY_BONUS_RP) + " RP at " + prop.name).c_str());
                            Money_Add(DAILY_BONUS_CASH);
                            ShowNotification(std::string("~g~Daily Visit Bonus! +$" + std::to_string(DAILY_BONUS_CASH) + " at " + prop.name).c_str());
                            prop.lastDailyBonusTime = currentTime; // Reset daily bonus timer
                        }
                        prop.generatedMoney = 0; // Reset generated money
                        inputDelayFrames = 10; // Apply input delay
                    }
                }
            }
            else {
                // Notify if money has reached cap and player is nearby, but not collecting
                if (prop.generatedMoney >= prop.moneyCap && (currentTime - prop.lastCapNotifyTime) > 30000) { // Notify every 30 seconds
                    ShowNotification(std::string("~y~Maximum collectable money reached at " + prop.name + ". Visit soon to collect!").c_str());
                    prop.lastCapNotifyTime = currentTime;
                }
            }
        }


        // --- Exterior Marker Interaction (Buy/Enter) ---
        // Only show exterior prompts for residences if player is NOT inside ANY teleported residence.
        // Business exterior markers (for buying) only show if not bought.
        bool showExteriorBuyOrEnterMarker = false;
        if (prop.type == RESIDENCE && !g_isPlayerInsideProperty) {
            showExteriorBuyOrEnterMarker = true;
        }
        else if (prop.type == BUSINESS && !prop.isBought) { // Only show buy marker for businesses if not bought
            showExteriorBuyOrEnterMarker = true;
        }

        if (showExteriorBuyOrEnterMarker) {
            Vector3 adjustedExteriorCoords = prop.exteriorCoords;
            // You might need to adjust Z slightly if the marker appears too high or low
            // adjustedExteriorCoords.z -= 1.0f; 

            // Show buy/enter marker from 75f distance
            if (distanceToExterior < 75.0f) {
                // Color based on whether it's bought or for sale
                if (prop.isBought) {
                    DrawPropertyMarker(adjustedExteriorCoords, 1.0f, 0, 0, 255, 150); // Blue for owned residence
                }
                else {
                    DrawPropertyMarker(adjustedExteriorCoords, 1.0f, 0, 255, 0, 150); // Green for sale
                }
            }

            // Show combined prompt and allow interaction only when very close (1.5f)
            if (distanceToExterior < 1.5f) {
                if (!prop.isBought) {
                    // Property is for sale (applies to both RESIDENCE and BUSINESS)
                    char priceText[256];
                    sprintf_s(priceText, "~w~%s~n~~g~FOR SALE: ~w~$%d~n~~y~Press E / (A) to buy", prop.name.c_str(), prop.price);
                    UI::_SET_NOTIFICATION_TEXT_ENTRY(const_cast<char*>("STRING"));
                    UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>(priceText));
                    UI::_DRAW_NOTIFICATION(false, false); // Auto-fade

                    if (inputDelayFrames == 0 && (IsKeyJustUp(0x45) || PadPressed(BTN_A))) { // 'E' key or A button
                        if (Money_Get() >= prop.price) {
                            Money_Add(-prop.price); // Take the money
                            prop.isBought = true;
                            UI::SET_BLIP_COLOUR(prop.blipHandle, prop.blipColorOwned); // Change blip to owned color
                            // Move business blip to collect coords upon purchase
                            if (prop.type == BUSINESS) {
                                UI::SET_BLIP_COORDS(prop.blipHandle, prop.collectCoords.x, prop.collectCoords.y, prop.collectCoords.z);
                            }
                            ShowNotification(std::string("~g~Successfully bought ~w~" + prop.name + "!").c_str());
                            Properties_Save("GTAOfflineProperties.ini"); // Save the house status
                            inputDelayFrames = 10; // Apply input delay
                        }
                        else {
                            // Display "Not enough money" notification instantly, without input delay
                            ShowNotification("~r~Not enough money to buy this property.");
                            // Do NOT apply inputDelayFrames here, so the prompt can be immediately re-evaluated
                        }
                    }
                }
                else { // Property is owned
                    if (prop.type == RESIDENCE) {
                        // Residence is owned, offer to enter (teleport)
                        char enterText[128];
                        sprintf_s(enterText, "~w~%s~n~~b~OWNED~n~~y~Press E / (A) to enter", prop.name.c_str());
                        UI::_SET_NOTIFICATION_TEXT_ENTRY(const_cast<char*>("STRING"));
                        UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>(enterText));
                        UI::_DRAW_NOTIFICATION(false, false); // Auto-fade

                        if (inputDelayFrames == 0 && (IsKeyJustUp(0x45) || PadPressed(BTN_A))) { // 'E' key or A button
                            // Teleport the player
                            ENTITY::SET_ENTITY_COORDS(playerPed, prop.interiorCoords.x, prop.interiorCoords.y, prop.interiorCoords.z, false, false, false, true);
                            ENTITY::SET_ENTITY_HEADING(playerPed, prop.interiorHeading);
                            ShowNotification(std::string("~g~Entering ~w~" + prop.name + ".").c_str());

                            // Wait for interior to be ready and capture its ID
                            int attempts = 0;
                            int currentInterior = 0;
                            while (currentInterior == 0 && attempts < 100) { // Max 100 attempts to get interior ID
                                WAIT(0); // Wait one game frame
                                currentInterior = INTERIOR::GET_INTERIOR_FROM_ENTITY(playerPed);
                                attempts++;
                            }

                            if (currentInterior != 0) {
                                g_isPlayerInsideProperty = true; // Player is now inside a teleported property
                                g_currentTeleportedInteriorID = currentInterior; // Set the current interior ID dynamically
                                g_activeTeleportedProperty = &prop; // Set the active property pointer
                                // ShowNotification(std::string("~g~Entered interior ID: " + std::to_string(currentInterior)).c_str()); // Debugging notification

                                // If the property has a defined interiorID for props, enable them
                                if (prop.interiorID != 0) {
                                    INTERIOR::DISABLE_INTERIOR(g_currentTeleportedInteriorID, false); // Enable interior
                                    // Note: "default_prop" might not be the correct prop name for all interiors.
                                    // You might need to find specific prop names for each interior if you want to enable/disable them.
                                    // For now, I'm keeping the existing logic but be aware it might not work as expected for all interiors.
                                    // INTERIOR::_ENABLE_INTERIOR_PROP(g_currentTeleportedInteriorID, const_cast<char*>("default_prop")); // Use g_currentTeleportedInteriorID
                                    INTERIOR::REFRESH_INTERIOR(g_currentTeleportedInteriorID); // Refresh after enabling props
                                }
                            }
                            else {
                                ShowNotification("~r~Failed to load interior. Try again.");
                                g_isPlayerInsideProperty = false;
                                g_currentTeleportedInteriorID = 0;
                                g_activeTeleportedProperty = nullptr;
                            }
                            inputDelayFrames = 10; // Apply input delay
                        }
                    }
                    // For businesses, no "enter" prompt here, as interaction is usually "collect money" at specific marker
                    // or simply walking in for non-teleporting businesses.
                }
            }
        }

        // --- Interior Marker Interaction (Exit) ---
        // This section only applies to the specific residence that was teleported into
        if (prop.type == RESIDENCE && prop.isBought && g_isPlayerInsideProperty && g_activeTeleportedProperty == &prop && g_currentTeleportedInteriorID != 0 && INTERIOR::GET_INTERIOR_FROM_ENTITY(playerPed) == g_currentTeleportedInteriorID) {
            if (distanceToInterior < 1.5f) { // Increased interaction radius for visibility
                DrawPropertyMarker(prop.interiorCoords, 1.0f, 255, 0, 0, 150); // Red marker inside
                char exitText[128];
                sprintf_s(exitText, "~w~%s Interior~n~~r~Press E / (A) to exit", prop.name.c_str());
                UI::_SET_NOTIFICATION_TEXT_ENTRY(const_cast<char*>("STRING"));
                UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>(exitText));
                UI::_DRAW_NOTIFICATION(false, false); // Auto-fade

                if (inputDelayFrames == 0 && (IsKeyJustUp(0x45) || PadPressed(BTN_A))) { // 'E' key or A button
                    ENTITY::SET_ENTITY_COORDS(playerPed, prop.exteriorCoords.x, prop.exteriorCoords.y, prop.exteriorCoords.z, false, false, false, true);
                    ENTITY::SET_ENTITY_HEADING(playerPed, prop.exteriorHeading);
                    ShowNotification(std::string("~g~Exiting ~w~" + prop.name + ".").c_str());
                    g_isPlayerInsideProperty = false; // Player is now outside a teleported property
                    g_currentTeleportedInteriorID = 0; // Reset current interior ID
                    g_activeTeleportedProperty = nullptr; // Reset active property
                    inputDelayFrames = 10; // Apply input delay
                }
            }
        }
    }
}

void Properties_Save(const char* filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        for (const auto& prop : g_properties) {
            // Save type, isBought, generatedMoney, lastRPTickTime, lastMoneyTickTime, lastCapNotifyTime, lastDailyBonusTime
            file << prop.name << "," << prop.type << "," << prop.isBought << "," << prop.generatedMoney << "," << prop.lastRPTickTime << "," << prop.lastMoneyTickTime << "," << prop.lastCapNotifyTime << "," << prop.lastDailyBonusTime << "\n";
        }
        // NEW: Save last used house name
        file << "LAST_USED_HOUSE," << g_lastUsedHouseName << "\n";
        file.close();
        ShowNotification("~g~Properties saved!");
    }
    else {
        ShowNotification("~r~Failed to save properties.");
    }
}

void Properties_Load(const char* filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            size_t currentPos = 0;
            size_t nextComma;

            // NEW: Check for LAST_USED_HOUSE line
            if (line.rfind("LAST_USED_HOUSE,", 0) == 0) { // Checks if line starts with "LAST_USED_HOUSE,"
                nextComma = line.find(',', currentPos); // Find comma after "LAST_USED_HOUSE"
                if (nextComma == std::string::npos) continue;
                currentPos = nextComma + 1; // Move past "LAST_USED_HOUSE,"

                g_lastUsedHouseName = line.substr(currentPos); // Rest of the line is the house name
                continue; // Move to next line in file
            }


            std::string nameFromFile;
            int typeFromFile;
            bool boughtStatus;
            int generatedMoneyFromFile;
            DWORD lastRPTickTimeFromFile;
            DWORD lastMoneyTickTimeFromFile;
            DWORD lastCapNotifyTimeFromFile;
            DWORD lastDailyBonusTimeFromFile;

            // Read name
            nextComma = line.find(',', currentPos);
            if (nextComma == std::string::npos) continue;
            nameFromFile = line.substr(currentPos, nextComma - currentPos);
            currentPos = nextComma + 1;

            // Read type
            nextComma = line.find(',', currentPos);
            if (nextComma == std::string::npos) continue;
            typeFromFile = std::stoi(line.substr(currentPos, nextComma - currentPos));
            currentPos = nextComma + 1;

            // Read isBought
            nextComma = line.find(',', currentPos);
            if (nextComma == std::string::npos) continue;
            boughtStatus = (line.substr(currentPos, nextComma - currentPos) == "1");
            currentPos = nextComma + 1;

            // Read generatedMoney
            nextComma = line.find(',', currentPos);
            if (nextComma == std::string::npos) continue;
            generatedMoneyFromFile = std::stoi(line.substr(currentPos, nextComma - currentPos));
            currentPos = nextComma + 1;

            // Read lastRPTickTime
            nextComma = line.find(',', currentPos);
            if (nextComma == std::string::npos) continue;
            lastRPTickTimeFromFile = std::stoul(line.substr(currentPos, nextComma - currentPos));
            currentPos = nextComma + 1;

            // Read lastMoneyTickTime
            nextComma = line.find(',', currentPos);
            if (nextComma == std::string::npos) continue;
            lastMoneyTickTimeFromFile = std::stoul(line.substr(currentPos, nextComma - currentPos));
            currentPos = nextComma + 1;

            // Read lastCapNotifyTime
            nextComma = line.find(',', currentPos);
            if (nextComma == std::string::npos) { // Handle older saves without this field
                lastCapNotifyTimeFromFile = 0;
            }
            else {
                lastCapNotifyTimeFromFile = std::stoul(line.substr(currentPos, nextComma - currentPos));
                currentPos = nextComma + 1;
            }

            // Read lastDailyBonusTime
            nextComma = line.find(',', currentPos); // This might be npos if it's the last field
            if (nextComma == std::string::npos) { // Handle older saves or if it's the last field
                lastDailyBonusTimeFromFile = 0;
            }
            else {
                lastDailyBonusTimeFromFile = std::stoul(line.substr(currentPos, nextComma - currentPos));
                currentPos = nextComma + 1;
            }


            // Find the property by name and update its status
            for (auto& prop : g_properties) {
                if (prop.name == nameFromFile) {
                    prop.type = static_cast<PropertyType>(typeFromFile); // Cast back to enum
                    prop.isBought = boughtStatus;
                    prop.generatedMoney = generatedMoneyFromFile; // Load generated money
                    prop.lastRPTickTime = lastRPTickTimeFromFile; // Load last RP tick time
                    prop.lastMoneyTickTime = lastMoneyTickTimeFromFile; // Load last money tick time
                    prop.lastCapNotifyTime = lastCapNotifyTimeFromFile; // Load last cap notify time
                    prop.lastDailyBonusTime = lastDailyBonusTimeFromFile; // Load last daily bonus time

                    // Update blip color immediately upon loading
                    if (prop.blipHandle != 0) { // Ensure blip exists
                        if (prop.isBought) {
                            UI::SET_BLIP_COLOUR(prop.blipHandle, prop.blipColorOwned); // Use defined owned color
                        }
                        else {
                            UI::SET_BLIP_COLOUR(prop.blipHandle, prop.blipColorSale); // Use defined sale color
                        }
                    }
                    break; // Found and updated, move to next line
                }
            }
        }
        file.close();
        ShowNotification("~g~Properties loaded!");
    }
    else {
        ShowNotification("~y~Properties save file not found. Starting fresh.");
    }
}
