#include "RankBar.h"
#include "RpEvents.h"
#include "script.h"
#include <cstdio>
#include <cmath>

// Classic GTA Online rectangular rank bar with matching level numbers
void DrawRankBar(int playerLevel, int playerXP, int xpToNext, int recentRPGain, ULONGLONG gainTime) {
    // Layout
    float barWidth = 0.31f, barHeight = 0.014f;
    float barX = (1.0f - barWidth) * 0.5f;
    float barY = 0.03f;

    // Background (rectangular)
    GRAPHICS::DRAW_RECT(barX + barWidth * 0.5f, barY + barHeight * 0.5f, barWidth, barHeight, 0, 0, 0, 165);

    // Progress bar (rectangular, no round)
    float pct = xpToNext ? float(playerXP) / float(xpToNext) : 0.0f;
    if (pct > 1.0f) pct = 1.0f;
    GRAPHICS::DRAW_RECT(barX + (barWidth * pct) * 0.5f, barY + barHeight * 0.5f, barWidth * pct, barHeight, 0, 130, 255, 245);

    // Ticks (subtle)
    for (int i = 1; i < 10; ++i) {
        float nx = barX + barWidth * i / 10.0f;
        GRAPHICS::DRAW_RECT(nx, barY + barHeight * 0.5f, 0.0014f, barHeight * 0.75f, 80, 80, 120, 85);
    }

    // RP gain float (above bar, fades out)
    if (recentRPGain > 0 && GetTickCount64() - gainTime < 1200) {
        char rpb[32];
        int fade = 255 - int((GetTickCount64() - gainTime) * 1.5f);
        if (fade < 60) fade = 60;
        sprintf_s(rpb, "+%d RP", recentRPGain);
        UI::SET_TEXT_FONT(0);
        UI::SET_TEXT_SCALE(0.0f, 0.33f);
        UI::SET_TEXT_COLOUR(0, 255, 140, fade);
        UI::SET_TEXT_CENTRE(1);
        UI::_SET_TEXT_ENTRY("STRING");
        UI::_ADD_TEXT_COMPONENT_STRING(rpb);
        UI::_DRAW_TEXT(barX + barWidth * 0.5f, barY - 0.012f);
    }

    // --- Matching level numbers, centered ---
    char buf[16];
    // Center of the bar plus a small upward offset
    float numY = barY + barHeight * 0.5f - 0.015f;
    float numScale = 0.49f;
    int numFont = 7; // Use the same for both

    // Level left
    UI::SET_TEXT_FONT(numFont);
    UI::SET_TEXT_SCALE(0.0f, numScale);
    UI::SET_TEXT_COLOUR(255, 255, 255, 232);
    UI::SET_TEXT_CENTRE(1);
    sprintf_s(buf, "%d", playerLevel);
    UI::_SET_TEXT_ENTRY("STRING");
    UI::_ADD_TEXT_COMPONENT_STRING(buf);
    UI::_DRAW_TEXT(barX - 0.013f, numY);

    // Level+1 right
    UI::SET_TEXT_FONT(numFont); // <<< set again to ensure match!
    UI::SET_TEXT_SCALE(0.0f, numScale);
    UI::SET_TEXT_COLOUR(255, 255, 255, 232);
    UI::SET_TEXT_CENTRE(1);
    sprintf_s(buf, "%d", playerLevel + 1);
    UI::_SET_TEXT_ENTRY("STRING");
    UI::_ADD_TEXT_COMPONENT_STRING(buf);
    UI::_DRAW_TEXT(barX + barWidth + 0.013f, numY);
}

// === REQUIRED: RankBar API ===

void RankBar_Init() {}
void RankBar_Tick() {}
void RankBar_Load(const char*) {}
void RankBar_Save(const char*) {}
void RankBar_DrawMenu(int&, float, float, float, float) {}

void RankBar_DrawBar() {
    DrawRankBar(
        RpEvents_GetLevel(),
        RpEvents_GetXP(),
        RpEvents_GetXPToNext(),
        RpEvents_RecentRPGain(),
        RpEvents_RecentRPGainTime()
    );
}
