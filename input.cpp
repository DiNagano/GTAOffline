#include "input.h"
#include <cstring>

// Globals (define ONCE, here)
XINPUT_STATE g_padPrev = {};
XINPUT_STATE g_padCurr = {};

void PollPad() {
    g_padPrev = g_padCurr;
    ZeroMemory(&g_padCurr, sizeof(g_padCurr));
    XInputGetState(0, &g_padCurr);
}


bool PadHeld(int btn) {
    return (g_padCurr.Gamepad.wButtons & btn) != 0;
}
