#pragma once

// Menu API
void CharacterCreator_Init();
// This function is preserved to prevent errors with script.cpp, but its logic is now handled elsewhere.
void CharacterCreator_Tick();
void CharacterCreator_Save(const char* path);
void CharacterCreator_Load(const char* path);
void CharacterCreator_Apply();

void CharacterCreator_ToggleGender();
void CharacterCreator_NextDad(int dir);
void CharacterCreator_NextMom(int dir);
void CharacterCreator_AdjustShapeBlend(int delta);
void CharacterCreator_AdjustSkinBlend(int delta);
void CharacterCreator_NextHairStyle(int dir);
void CharacterCreator_NextHairColor(int dir);
void CharacterCreator_NextEyebrow(int dir);
void CharacterCreator_NextEyebrowColor(int dir);
void CharacterCreator_NudgeFaceFeature(int idx, float delta);
void CharacterCreator_NextClothes(int slot, int dir);
void CharacterCreator_NextEyeColor(int dir);

int  CharacterCreator_NumFaceFeatures();
const char* CharacterCreator_GetFaceFeatureName(int idx);
float CharacterCreator_GetFaceFeatureValue(int idx);

int CharacterCreator_GetGender();
int CharacterCreator_GetDadId();
int CharacterCreator_GetMomId();
int CharacterCreator_GetHairId();
int CharacterCreator_GetHairColor();
int CharacterCreator_GetEyebrowId();
int CharacterCreator_GetEyebrowColor();
int CharacterCreator_GetEyeColor();
int CharacterCreator_GetClothesTop();
int CharacterCreator_GetClothesLegs();
int CharacterCreator_GetClothesShoes();

void CharacterCreator_DrawMenu(int& menuIndex, int& menuCategory);

// State for camera views
extern bool wardrobeCamActive;
// NEW: Master toggle for the creator camera system
extern bool creatorCamEnabled;

struct CharacterCreatorState {
    int gender, dadIdx, momIdx, blend, skin;
    int hairIdx, hairColor, eyebrowIdx, eyebrowColor, eyeColor;
    float faceFeatures[20];
    int topIdx, legIdx, shoeIdx;
};
CharacterCreatorState CharacterCreator_GetState();
