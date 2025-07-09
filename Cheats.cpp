#include "Cheats.h"
#include "script.h"
#include "Self.h"
#include "Weapons.h"
#include "Misc.h"
#include "input.h"
// Which tab is open?
int cheatTab = 0;

// Names for each tab
static const char* tabNames[TAB_COUNT] = { "Self", "Weapons", "Misc" };

// Main init: just call each module's init
void Cheats_Init() {
    cheatTab = 0;
    Self_Init();
    Weapons_Init();
    Misc_Init();
}

// Main tick: call each tick
void Cheats_Tick() {
    Self_Tick();
    Weapons_Tick();
    Misc_Tick();
}

// Draw menu: handles tab switching and routes to current tab
void Cheats_DrawMenu(int& menuIndex, float x, float y, float w, float h) {
    // Draw tab headers
    for (int i = 0; i < TAB_COUNT; ++i) {
        GRAPHICS::DRAW_RECT(
            x + (w * 0.35f) * i + w * 0.18f, y - 0.045f, w * 0.32f, 0.027f,
            i == cheatTab ? 80 : 35,
            i == cheatTab ? 160 : 60,
            i == cheatTab ? 230 : 90, 220
        );
        UI::SET_TEXT_FONT(0);
        UI::SET_TEXT_SCALE(0.0f, 0.36f);
        UI::SET_TEXT_COLOUR(255, 255, 255, 245);
        UI::SET_TEXT_CENTRE(1);
        UI::_SET_TEXT_ENTRY("STRING");
        UI::_ADD_TEXT_COMPONENT_STRING((char*)tabNames[i]);
        UI::_DRAW_TEXT(x + (w * 0.35f) * i + w * 0.18f, y - 0.051f);
    }

    // Handle tab switching
    if (IsKeyJustUp(VK_NUMPAD7) || PadPressed(BTN_LB)) { cheatTab = (cheatTab + TAB_COUNT - 1) % TAB_COUNT; menuIndex = 0; }
    if (IsKeyJustUp(VK_NUMPAD9) || PadPressed(BTN_RB)) { cheatTab = (cheatTab + 1) % TAB_COUNT; menuIndex = 0; }

    // Route to the right module's menu
    if (cheatTab == TAB_SELF)         Self_DrawMenu(menuIndex, x, y, w, h);
    else if (cheatTab == TAB_WEAPONS) Weapons_DrawMenu(menuIndex, x, y, w, h);
    else if (cheatTab == TAB_MISC)    Misc_DrawMenu(menuIndex, x, y, w, h);
}
