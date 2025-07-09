#include "CharacterCreator.h"
#include "script.h"
#include "input.h"
#include <cstdio>
#include <cstring>
#include <cctype>

// Clothing slot indices
#define SLOT_UNDERSHIRT 8
#define SLOT_TOP        11
#define SLOT_LEGS       4
#define SLOT_SHOES      6


// Static data
struct ParentEntry { const char* name; int index; };
static const ParentEntry dads[] = {
    {"Benjamin",0},{"Daniel",1},{"Joshua",2},{"Noah",3},{"Andrew",4},{"Juan",5},
    {"Alex",6},{"Isaac",7},{"Evan",8},{"Ethan",9},{"Vincent",10},{"Angel",11},
    {"Diego",12},{"Adrian",13},{"Gabriel",14},{"Michael",15},{"Santiago",16},
    {"Kevin",17},{"Louis",18},{"Samuel",19},{"Anthony",20},{"Hannah",42},
    {"Audrey",43},{"Jasmine",44},{"Giselle",45},{"Amelia",46}
};
static const ParentEntry moms[] = {
    {"Hannah",21},{"Audrey",22},{"Jasmine",23},{"Giselle",24},{"Amelia",25},
    {"Isabella",26},{"Zoe",27},{"Ava",28},{"Camila",29},{"Violet",30},
    {"Sophia",31},{"Evelyn",32},{"Nicole",33},{"Ashley",34},{"Grace",35},
    {"Brianna",36},{"Natalie",37},{"Olivia",38},{"Elizabeth",39},{"Charlotte",40},{"Emma",41}
};
static const int NUM_DADS = sizeof(dads) / sizeof(dads[0]);
static const int NUM_MOMS = sizeof(moms) / sizeof(moms[0]);
static const int NUM_FACE_FEATURES = 20;
static const char* FACE_FEATURE_NAMES[NUM_FACE_FEATURES] = {
    "Nose Width", "Nose Peak", "Nose Length", "Nose Curve", "Nose Tip",
    "Nose Twist", "Brow Height", "Brow Forward", "Cheekbone Height", "Cheekbone Width",
    "Cheek Width", "Eye Opening", "Lip Thickness", "Jaw Width", "Jaw Shape",
    "Chin Bone", "Chin Length", "Chin Shape", "Chin Hole", "Neck Thickness"
};
static const int NUM_HAIRSTYLES = 39, NUM_HAIRCOLORS = 64, NUM_EYEBROWS = 33, NUM_EYEBROWCOLORS = 64, NUM_EYECOLORS = 32, NUM_TOPS = 136, NUM_UNDERSHIRTS = 60, NUM_LEGS = 80, NUM_SHOES = 50;
static const char* GENDERS[] = { "Male", "Female" };

// State
static int gender = 0, dadIdx = 0, momIdx = 0;
static int blend = 50, skin = 50;
static int hairIdx = 0, hairColor = 0, eyebrowIdx = 0, eyebrowColor = 0, eyeColor = 0;
static float faceFeatures[NUM_FACE_FEATURES] = { 0 };
static int topIdx = 0, undershirtIdx = 0, legIdx = 0, shoeIdx = 0;
// Camera state
bool wardrobeCamActive = false;
bool creatorCamEnabled = false;
static Cam customCam = 0;


// Clamp helper
template<typename T>
static T clamp(T v, T lo, T hi) { return (v < lo) ? lo : (v > hi) ? hi : v; }

// --- Camera Management ---
void update_character_camera() {
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) return;

    Vector3 headPos = PED::GET_PED_BONE_COORDS(playerPed, 31086, 0, 0, 0); // SKEL_Head

    if (customCam == 0) {
        customCam = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", true);
    }

    if (wardrobeCamActive) {
        // Full body "wardrobe" view
        Vector3 camPos = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, 0.0, 2.5, 0.5);
        CAM::SET_CAM_COORD(customCam, camPos.x, camPos.y, camPos.z);
        CAM::POINT_CAM_AT_COORD(customCam, headPos.x, headPos.y, headPos.z);
    }
    else {
        // Default face close-up view
        Vector3 camPos = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, 0.0, 1.0, 0.5);
        CAM::SET_CAM_COORD(customCam, camPos.x, camPos.y, camPos.z);
        CAM::POINT_CAM_AT_COORD(customCam, headPos.x, headPos.y, headPos.z + 0.1f);
    }

    CAM::SET_CAM_ACTIVE(customCam, true);
    CAM::RENDER_SCRIPT_CAMS(true, false, 0, true, false);
}

void stop_character_camera() {
    if (customCam != 0) {
        CAM::SET_CAM_ACTIVE(customCam, false);
        CAM::DESTROY_CAM(customCam, false);
        customCam = 0;
    }
    CAM::RENDER_SCRIPT_CAMS(false, false, 0, true, false);
}


// PED Application
void CharacterCreator_Apply() {
    Ped ped = PLAYER::PLAYER_PED_ID();
    Hash mpModel = GAMEPLAY::GET_HASH_KEY(gender ? "mp_f_freemode_01" : "mp_m_freemode_01");
    if (ENTITY::GET_ENTITY_MODEL(ped) != mpModel) {
        STREAMING::REQUEST_MODEL(mpModel);
        int timeout = 0;
        while (!STREAMING::HAS_MODEL_LOADED(mpModel) && timeout++ < 500) WAIT(0);
        if (!STREAMING::HAS_MODEL_LOADED(mpModel)) return;
        PLAYER::SET_PLAYER_MODEL(PLAYER::PLAYER_ID(), mpModel);
        WAIT(100); // Give time for the model to apply
        ped = PLAYER::PLAYER_PED_ID();
    }
    PED::SET_PED_HEAD_BLEND_DATA(
        ped, dads[dadIdx].index, moms[momIdx].index, 0, dads[dadIdx].index, moms[momIdx].index, 0,
        blend / 100.0f, skin / 100.0f, 0.0f, false
    );
    PED::SET_PED_COMPONENT_VARIATION(ped, 2, hairIdx, 0, 2);
    PED::_SET_PED_HAIR_COLOR(ped, hairColor, 0);
    PED::SET_PED_HEAD_OVERLAY(ped, 1, eyebrowIdx, 1.0f);
    PED::_SET_PED_HEAD_OVERLAY_COLOR(ped, 1, 1, eyebrowColor, 0);
    PED::_SET_PED_EYE_COLOR(ped, eyeColor);
    for (int i = 0; i < NUM_FACE_FEATURES; ++i)
        PED::_SET_PED_FACE_FEATURE(ped, i, faceFeatures[i]);
    PED::SET_PED_COMPONENT_VARIATION(ped, SLOT_TOP, topIdx, 0, 2);
    PED::SET_PED_COMPONENT_VARIATION(ped, SLOT_UNDERSHIRT, undershirtIdx, 0, 2);
    PED::SET_PED_COMPONENT_VARIATION(ped, SLOT_LEGS, legIdx, 0, 2);
    PED::SET_PED_COMPONENT_VARIATION(ped, SLOT_SHOES, shoeIdx, 0, 2);
}

void CharacterCreator_Init() {}
// This function is preserved to prevent errors with script.cpp, but its logic is now handled elsewhere.
void CharacterCreator_Tick() {}

void CharacterCreator_Save(const char* path) {
    FILE* f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "gender=%d\ndad=%d\nmom=%d\nblend=%d\nskin=%d\nhair=%d\nhairColor=%d\neyebrow=%d\neyebrowColor=%d\neyeColor=%d\n",
        gender, dadIdx, momIdx, blend, skin, hairIdx, hairColor, eyebrowIdx, eyebrowColor, eyeColor);
    for (int i = 0; i < NUM_FACE_FEATURES; ++i) fprintf(f, "face%d=%.3f\n", i, faceFeatures[i]);
    fprintf(f, "top=%d\nundershirt=%d\nlegs=%d\nshoes=%d\n", topIdx, undershirtIdx, legIdx, shoeIdx);
    fclose(f);
}
void CharacterCreator_Load(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return;
    char line[128];
    while (fgets(line, sizeof(line), f)) {
        char* eq = strchr(line, '='); if (!eq) continue; *eq = 0; char* val = eq + 1;
        if (strcmp(line, "gender") == 0) gender = atoi(val);
        else if (strcmp(line, "dad") == 0) dadIdx = atoi(val);
        else if (strcmp(line, "mom") == 0) momIdx = atoi(val);
        else if (strcmp(line, "blend") == 0) blend = atoi(val);
        else if (strcmp(line, "skin") == 0) skin = atoi(val);
        else if (strcmp(line, "hair") == 0) hairIdx = atoi(val);
        else if (strcmp(line, "hairColor") == 0) hairColor = atoi(val);
        else if (strcmp(line, "eyebrow") == 0) eyebrowIdx = atoi(val);
        else if (strcmp(line, "eyebrowColor") == 0) eyebrowColor = atoi(val);
        else if (strcmp(line, "eyeColor") == 0) eyeColor = atoi(val);
        else if (strncmp(line, "face", 4) == 0 && isdigit(line[4])) {
            int idx = atoi(line + 4);
            if (idx >= 0 && idx < NUM_FACE_FEATURES) faceFeatures[idx] = (float)atof(val);
        }
        else if (strcmp(line, "top") == 0) topIdx = atoi(val);
        else if (strcmp(line, "undershirt") == 0) undershirtIdx = atoi(val);
        else if (strcmp(line, "legs") == 0) legIdx = atoi(val);
        else if (strcmp(line, "shoes") == 0) shoeIdx = atoi(val);
    }
    fclose(f);
    CharacterCreator_Apply();
    UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
    UI::_ADD_TEXT_COMPONENT_STRING("Character Loaded!");
    UI::_DRAW_NOTIFICATION(false, false);
}

// --- Modification ---
void CharacterCreator_ToggleGender() { gender = 1 - gender; CharacterCreator_Apply(); }
void CharacterCreator_NextDad(int dir) { dadIdx = (dadIdx + dir + NUM_DADS) % NUM_DADS; CharacterCreator_Apply(); }
void CharacterCreator_NextMom(int dir) { momIdx = (momIdx + dir + NUM_MOMS) % NUM_MOMS; CharacterCreator_Apply(); }
void CharacterCreator_AdjustShapeBlend(int delta) { blend = clamp(blend + delta, 0, 100); CharacterCreator_Apply(); }
void CharacterCreator_AdjustSkinBlend(int delta) { skin = clamp(skin + delta, 0, 100); CharacterCreator_Apply(); }
void CharacterCreator_NextHairStyle(int dir) { hairIdx = (hairIdx + dir + NUM_HAIRSTYLES) % NUM_HAIRSTYLES; CharacterCreator_Apply(); }
void CharacterCreator_NextHairColor(int dir) { hairColor = (hairColor + dir + NUM_HAIRCOLORS) % NUM_HAIRCOLORS; CharacterCreator_Apply(); }
void CharacterCreator_NextEyebrow(int dir) { eyebrowIdx = (eyebrowIdx + dir + NUM_EYEBROWS) % NUM_EYEBROWS; CharacterCreator_Apply(); }
void CharacterCreator_NextEyebrowColor(int dir) { eyebrowColor = (eyebrowColor + dir + NUM_EYEBROWCOLORS) % NUM_EYEBROWCOLORS; CharacterCreator_Apply(); }
void CharacterCreator_NudgeFaceFeature(int idx, float delta) {
    if (idx < 0 || idx >= NUM_FACE_FEATURES) return;
    faceFeatures[idx] = clamp(faceFeatures[idx] + delta, -1.0f, 1.0f);
    CharacterCreator_Apply();
}
void CharacterCreator_NextClothes(int slot, int dir) {
    if (slot == 0) { topIdx = (topIdx + dir + NUM_TOPS) % NUM_TOPS; }
    else if (slot == 1) { undershirtIdx = (undershirtIdx + dir + NUM_UNDERSHIRTS) % NUM_UNDERSHIRTS; }
    else if (slot == 2) { legIdx = (legIdx + dir + NUM_LEGS) % NUM_LEGS; }
    else if (slot == 3) { shoeIdx = (shoeIdx + dir + NUM_SHOES) % NUM_SHOES; }
    CharacterCreator_Apply();
}
void CharacterCreator_NextEyeColor(int dir) { eyeColor = (eyeColor + dir + NUM_EYECOLORS) % NUM_EYECOLORS; CharacterCreator_Apply(); }

// --- Menu logic with controller support ---
enum CreatorPage { CREATOR_MAIN = 0, CREATOR_FACE, CREATOR_CLOTHES };
static int creatorPage = 0;

// =================================================================================
// --- CORRECTED MENU DRAWING FUNCTION ---
// =================================================================================
void CharacterCreator_DrawMenu(int& menuIndex, int& menuCategory) {
    // --- Page Tab Drawing & Navigation ---
    const char* pages[3] = { "Main", "Face", "Clothes" };
    float x = 0.02f, y = 0.13f, w = 0.29f, h = 0.038f;
    for (int i = 0; i < 3; ++i) {
        GRAPHICS::DRAW_RECT(x + (w * 0.33f) * i + w * 0.17f, y - 0.045f, w * 0.31f, 0.028f,
            i == creatorPage ? 70 : 24, i == creatorPage ? 140 : 40, i == creatorPage ? 215 : 90, 210);
        UI::SET_TEXT_FONT(0);
        UI::SET_TEXT_SCALE(0.0f, 0.36f);
        UI::SET_TEXT_COLOUR(255, 255, 255, 245);
        UI::SET_TEXT_CENTRE(1);
        UI::_SET_TEXT_ENTRY("STRING");
        UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>(pages[i]));
        UI::_DRAW_TEXT(x + (w * 0.33f) * i + w * 0.17f, y - 0.051f);
    }

    if (IsKeyJustUp(VK_NUMPAD7) || PadPressed(BTN_LB)) { creatorPage = (creatorPage + 2) % 3; menuIndex = 0; }
    if (IsKeyJustUp(VK_NUMPAD9) || PadPressed(BTN_RB)) { creatorPage = (creatorPage + 1) % 3; menuIndex = 0; }

    // --- Page-Specific Logic ---
    if (creatorPage == CREATOR_MAIN) {
        wardrobeCamActive = false; // Always use face cam on main page
        const int numOptions = 13;
        const char* labels[numOptions] = {
            "Creator Camera", "Gender","Dad","Mom","Blend","Skin","Hair Style","Hair Color",
            "Eyebrow","Eyebrow Color","Eye Color","Go to Face Features","Go to Clothes"
        };
        GRAPHICS::DRAW_RECT(x + w * 0.5f, y + h * (numOptions / 2.0f) + (h / 2.0f), w, h * (numOptions + 1), 24, 24, 32, 220);
        UI::SET_TEXT_FONT(0); UI::SET_TEXT_SCALE(0.0f, 0.40f); UI::SET_TEXT_COLOUR(255, 255, 220, 252);
        UI::_SET_TEXT_ENTRY("STRING"); UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>("Character Creator")); UI::_DRAW_TEXT(x + 0.012f, y - 0.037f);

        for (int i = 0; i < numOptions; ++i) {
            float cy = y + h * i;
            bool active = (i == menuIndex);
            GRAPHICS::DRAW_RECT(x + w * 0.5f, cy + (h - 0.004f) * 0.5f, w, h - 0.004f, active ? 120 : 60, active ? 205 : 80, active ? 215 : 80, active ? 255 : 135);
            UI::SET_TEXT_FONT(0); UI::SET_TEXT_SCALE(0.0f, 0.36f); UI::SET_TEXT_COLOUR(0, 0, 0, 255);
            UI::_SET_TEXT_ENTRY("STRING"); UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>(labels[i])); UI::_DRAW_TEXT(x + 0.017f, cy + 0.007f);

            char val[64] = {};
            if (i == 0) sprintf_s(val, "%s", creatorCamEnabled ? "[ON]" : "[OFF]");
            if (i == 1) sprintf_s(val, "%s", GENDERS[gender]);
            if (i == 2) sprintf_s(val, "%s", dads[dadIdx].name);
            if (i == 3) sprintf_s(val, "%s", moms[momIdx].name);
            if (i == 4) sprintf_s(val, "%d", blend);
            if (i == 5) sprintf_s(val, "%d", skin);
            if (i == 6) sprintf_s(val, "%d", hairIdx);
            if (i == 7) sprintf_s(val, "%d", hairColor);
            if (i == 8) sprintf_s(val, "%d", eyebrowIdx);
            if (i == 9) sprintf_s(val, "%d", eyebrowColor);
            if (i == 10) sprintf_s(val, "%d", eyeColor);
            if (i < numOptions - 2) {
                UI::SET_TEXT_FONT(0); UI::SET_TEXT_SCALE(0.0f, 0.34f); UI::SET_TEXT_COLOUR(60, 60, 130, 255); UI::SET_TEXT_CENTRE(0);
                UI::_SET_TEXT_ENTRY("STRING"); UI::_ADD_TEXT_COMPONENT_STRING(val); UI::_DRAW_TEXT(x + w - 0.06f, cy + 0.007f);
            }
        }
        int up = 0, down = 0, left = 0, right = 0;
        if (IsKeyJustUp(VK_NUMPAD8) || PadPressed(DPAD_UP)) up = 1;
        if (IsKeyJustUp(VK_NUMPAD2) || PadPressed(DPAD_DOWN)) down = 1;
        if (IsKeyJustUp(VK_NUMPAD4) || PadPressed(DPAD_LEFT)) left = 1;
        if (IsKeyJustUp(VK_NUMPAD6) || PadPressed(DPAD_RIGHT)) right = 1;
        if (up) menuIndex = (menuIndex - 1 + numOptions) % numOptions;
        if (down) menuIndex = (menuIndex + 1) % numOptions;
        int dir = right - left;
        if (dir) {
            switch (menuIndex) {
            case 1: CharacterCreator_ToggleGender(); break;
            case 2: CharacterCreator_NextDad(dir); break;
            case 3: CharacterCreator_NextMom(dir); break;
            case 4: CharacterCreator_AdjustShapeBlend(dir); break;
            case 5: CharacterCreator_AdjustSkinBlend(dir); break;
            case 6: CharacterCreator_NextHairStyle(dir); break;
            case 7: CharacterCreator_NextHairColor(dir); break;
            case 8: CharacterCreator_NextEyebrow(dir); break;
            case 9: CharacterCreator_NextEyebrowColor(dir); break;
            case 10: CharacterCreator_NextEyeColor(dir); break;
            }
        }
        if (IsKeyJustUp(VK_NUMPAD5) || PadPressed(BTN_A)) {
            if (menuIndex == 0) creatorCamEnabled = !creatorCamEnabled;
            if (menuIndex == numOptions - 2) { creatorPage = CREATOR_FACE; menuIndex = 0; }
            if (menuIndex == numOptions - 1) { creatorPage = CREATOR_CLOTHES; menuIndex = 0; }
        }
        if (PadPressed(BTN_B)) {
            creatorCamEnabled = false;
            wardrobeCamActive = false;
            stop_character_camera();
            menuCategory = 0;
            menuIndex = 0;
            creatorPage = 0;
        }
    }
    else if (creatorPage == CREATOR_FACE) {
        wardrobeCamActive = false;
        const int numOptions = NUM_FACE_FEATURES + 2;
        GRAPHICS::DRAW_RECT(x + w * 0.5f, y + h * (numOptions / 2.0f) + (h / 2.0f), w, h * (numOptions + 1), 34, 28, 48, 210);
        UI::SET_TEXT_FONT(0); UI::SET_TEXT_SCALE(0.0f, 0.40f); UI::SET_TEXT_COLOUR(255, 255, 220, 252);
        UI::_SET_TEXT_ENTRY("STRING"); UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>("Face Features")); UI::_DRAW_TEXT(x + 0.012f, y - 0.037f);

        for (int i = 0; i < numOptions; ++i) {
            float cy = y + h * i;
            bool active = (i == menuIndex);
            GRAPHICS::DRAW_RECT(x + w * 0.5f, cy + (h - 0.004f) * 0.5f, w, h - 0.004f, active ? 120 : 60, active ? 180 : 70, active ? 230 : 90, active ? 255 : 130);
            UI::SET_TEXT_FONT(0); UI::SET_TEXT_SCALE(0.0f, 0.33f); UI::SET_TEXT_COLOUR(0, 0, 0, 255);
            UI::_SET_TEXT_ENTRY("STRING");
            if (i < NUM_FACE_FEATURES) UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>(FACE_FEATURE_NAMES[i]));
            else if (i == NUM_FACE_FEATURES) UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>("Go to Clothes"));
            else UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>("Back to Main"));
            UI::_DRAW_TEXT(x + 0.017f, cy + 0.008f);

            if (i < NUM_FACE_FEATURES) {
                char vbuf[32]; sprintf_s(vbuf, "%.2f", faceFeatures[i]);
                UI::SET_TEXT_FONT(0); UI::SET_TEXT_SCALE(0.0f, 0.32f); UI::SET_TEXT_COLOUR(70, 40, 150, 255); UI::SET_TEXT_CENTRE(0);
                UI::_SET_TEXT_ENTRY("STRING"); UI::_ADD_TEXT_COMPONENT_STRING(vbuf); UI::_DRAW_TEXT(x + w - 0.08f, cy + 0.008f);
            }
        }
        int up = 0, down = 0, left = 0, right = 0;
        if (IsKeyJustUp(VK_NUMPAD8) || PadPressed(DPAD_UP)) up = 1;
        if (IsKeyJustUp(VK_NUMPAD2) || PadPressed(DPAD_DOWN)) down = 1;
        if (IsKeyJustUp(VK_NUMPAD4) || PadPressed(DPAD_LEFT)) left = 1;
        if (IsKeyJustUp(VK_NUMPAD6) || PadPressed(DPAD_RIGHT)) right = 1;
        if (up) menuIndex = (menuIndex - 1 + numOptions) % numOptions;
        if (down) menuIndex = (menuIndex + 1) % numOptions;
        int dir = right - left;
        if (dir && menuIndex < NUM_FACE_FEATURES) CharacterCreator_NudgeFaceFeature(menuIndex, dir * 0.05f);

        if (IsKeyJustUp(VK_NUMPAD5) || PadPressed(BTN_A)) {
            if (menuIndex == numOptions - 2) { creatorPage = CREATOR_CLOTHES; menuIndex = 0; }
            if (menuIndex == numOptions - 1) { creatorPage = CREATOR_MAIN; menuIndex = 0; }
        }
        if (PadPressed(BTN_B)) { creatorPage = CREATOR_MAIN; menuIndex = 0; }
    }
    else if (creatorPage == CREATOR_CLOTHES) {
        const int numOptions = 6;
        const char* labels[numOptions] = {
            "Top", "Undershirt", "Legs", "Shoes", "Wardrobe Cam", "Back to Main"
        };
        GRAPHICS::DRAW_RECT(x + w * 0.5f, y + h * (numOptions / 2.0f) + (h / 2.0f), w, h * (numOptions + 1), 34, 28, 48, 210);
        UI::SET_TEXT_FONT(0); UI::SET_TEXT_SCALE(0.0f, 0.40f); UI::SET_TEXT_COLOUR(255, 255, 220, 252);
        UI::_SET_TEXT_ENTRY("STRING"); UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>("Clothes")); UI::_DRAW_TEXT(x + 0.012f, y - 0.037f);

        for (int i = 0; i < numOptions; ++i) {
            float cy = y + h * i;
            bool active = (i == menuIndex);
            GRAPHICS::DRAW_RECT(x + w * 0.5f, cy + (h - 0.004f) * 0.5f, w, h - 0.004f, active ? 120 : 60, active ? 180 : 70, active ? 230 : 90, active ? 255 : 130);
            UI::SET_TEXT_FONT(0); UI::SET_TEXT_SCALE(0.0f, 0.36f); UI::SET_TEXT_COLOUR(0, 0, 0, 255);
            UI::_SET_TEXT_ENTRY("STRING"); UI::_ADD_TEXT_COMPONENT_STRING(const_cast<char*>(labels[i])); UI::_DRAW_TEXT(x + 0.017f, cy + 0.007f);

            if (i < 4) {
                char vbuf[32];
                if (i == 0) sprintf_s(vbuf, "%d", topIdx);
                if (i == 1) sprintf_s(vbuf, "%d", undershirtIdx);
                if (i == 2) sprintf_s(vbuf, "%d", legIdx);
                if (i == 3) sprintf_s(vbuf, "%d", shoeIdx);
                UI::SET_TEXT_FONT(0); UI::SET_TEXT_SCALE(0.0f, 0.32f); UI::SET_TEXT_COLOUR(70, 40, 150, 255); UI::SET_TEXT_CENTRE(0);
                UI::_SET_TEXT_ENTRY("STRING"); UI::_ADD_TEXT_COMPONENT_STRING(vbuf); UI::_DRAW_TEXT(x + w - 0.08f, cy + 0.008f);
            }
            if (i == 4) { // Wardrobe Cam Toggle
                char vbuf[32]; sprintf_s(vbuf, "%s", wardrobeCamActive ? "[ON]" : "[OFF]");
                UI::SET_TEXT_FONT(0); UI::SET_TEXT_SCALE(0.0f, 0.32f); UI::SET_TEXT_COLOUR(wardrobeCamActive ? 36 : 100, wardrobeCamActive ? 220 : 100, wardrobeCamActive ? 60 : 100, 255);
                UI::SET_TEXT_CENTRE(0); UI::_SET_TEXT_ENTRY("STRING"); UI::_ADD_TEXT_COMPONENT_STRING(vbuf); UI::_DRAW_TEXT(x + w - 0.08f, cy + 0.008f);
            }
        }
        int up = 0, down = 0, left = 0, right = 0;
        if (IsKeyJustUp(VK_NUMPAD8) || PadPressed(DPAD_UP)) up = 1;
        if (IsKeyJustUp(VK_NUMPAD2) || PadPressed(DPAD_DOWN)) down = 1;
        if (IsKeyJustUp(VK_NUMPAD4) || PadPressed(DPAD_LEFT)) left = 1;
        if (IsKeyJustUp(VK_NUMPAD6) || PadPressed(DPAD_RIGHT)) right = 1;
        if (up) menuIndex = (menuIndex - 1 + numOptions) % numOptions;
        if (down) menuIndex = (menuIndex + 1) % numOptions;
        int dir = right - left;
        if (dir && menuIndex < 4) CharacterCreator_NextClothes(menuIndex, dir);

        if (IsKeyJustUp(VK_NUMPAD5) || PadPressed(BTN_A)) {
            if (menuIndex == 4) { // Wardrobe Cam toggle
                // Flip the state of the wardrobe cam
                wardrobeCamActive = !wardrobeCamActive;
                // The master camera's state now mirrors the wardrobe cam's state.
                creatorCamEnabled = wardrobeCamActive;
            }
            if (menuIndex == 5) { // Back to Main
                creatorPage = CREATOR_MAIN;
                menuIndex = 0;
            }
        }
        if (PadPressed(BTN_B)) { creatorPage = CREATOR_MAIN; menuIndex = 0; }
    }

    // --- Camera Update (Moved to the end) ---
    // Now the camera updates AFTER all the logic for the current frame is complete.
    if (creatorCamEnabled) {
        update_character_camera();
    }
    else {
        stop_character_camera();
    }
}
