#include "Credits.h"
#include "input.h"
#include "script.h" // For menu category enum

void Credits_Init() {
    // Nothing to initialize for now
}

void draw_credits_menu() {
    extern int menuCategory;
    extern int menuIndex;
    extern int inputDelayFrames;

    const float MENU_X = 0.02f;
    const float MENU_Y = 0.13f;
    const float MENU_W = 0.29f;
    const float MENU_H = 0.038f;

    // --- UPDATED: Added new names to the credits list ---
    const char* creditLines[] = {
        "Mod Created By: CreamyPlaytime",
        "Special Thanks: You!",
        "ScriptHookV: Alexander Blade",
        "Assistance: OpenAI & Google Gemini",
        "Game: Rockstar Games"
    };
    const int numCreditLines = sizeof(creditLines) / sizeof(creditLines[0]);
    const int numOptions = numCreditLines + 1; // +1 for the back button

    // --- Draw Menu Background and Title ---
    GRAPHICS::DRAW_RECT(MENU_X + MENU_W * 0.5f, MENU_Y - 0.038f + MENU_H * (numOptions) * 0.5f, MENU_W, MENU_H * (numOptions), 14, 17, 22, 228);
    UI::SET_TEXT_FONT(0);
    UI::SET_TEXT_SCALE(0.0f, 0.43f);
    UI::SET_TEXT_COLOUR(255, 255, 220, 252);
    UI::_SET_TEXT_ENTRY("STRING");
    UI::_ADD_TEXT_COMPONENT_STRING("Credits");
    UI::_DRAW_TEXT(MENU_X + 0.014f, MENU_Y - 0.062f);

    // --- Draw Credits Text ---
    for (int i = 0; i < numCreditLines; ++i) {
        float cy = MENU_Y + MENU_H * i;
        UI::SET_TEXT_FONT(0);
        UI::SET_TEXT_SCALE(0.0f, 0.36f);
        UI::SET_TEXT_COLOUR(255, 255, 255, 255);
        UI::_SET_TEXT_ENTRY("STRING");
        UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>(creditLines[i]));
        UI::_DRAW_TEXT(MENU_X + 0.017f, cy + 0.007f);
    }

    // --- Back Button ---
    float back_cy = MENU_Y + MENU_H * numCreditLines;
    bool back_active = (menuIndex == 0); // Only one option
    GRAPHICS::DRAW_RECT(MENU_X + MENU_W * 0.5f, back_cy + (MENU_H - 0.004f) * 0.5f, MENU_W, MENU_H - 0.004f, back_active ? 215 : 80, back_active ? 60 : 80, back_active ? 60 : 80, 255);
    UI::SET_TEXT_FONT(0);
    UI::SET_TEXT_SCALE(0.0f, 0.38f);
    UI::SET_TEXT_COLOUR(0, 0, 0, 255);
    UI::_SET_TEXT_ENTRY("STRING");
    UI::_ADD_TEXT_COMPONENT_STRING("Back");
    UI::_DRAW_TEXT(MENU_X + 0.017f, back_cy + 0.007f);

    // --- Navigation ---
    if (inputDelayFrames > 0) return;

    // Since there's only one selectable option (Back), we don't need up/down navigation.
    menuIndex = 0;

    if (IsKeyJustUp(VK_NUMPAD5) || PadPressed(BTN_A)) {
        // FIXED: Replaced CAT_MAIN with its integer value 0 to match other files.
        menuCategory = 0; // This was CAT_MAIN
        menuIndex = 7; // Set main menu to highlight "Credits"
        inputDelayFrames = 10;
    }
}
