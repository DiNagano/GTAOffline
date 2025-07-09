#pragma once

void RankBar_Init();
void RankBar_Tick();
void RankBar_DrawBar();
void RankBar_DrawMenu(int& menuIndex, float x, float y, float w, float h);
void RankBar_Load(const char* path);
void RankBar_Save(const char* path);
