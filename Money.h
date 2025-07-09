#pragma once

void Money_Init();
int  Money_Get();
void Money_Set(int value);
void Money_Add(int amount);
void Money_Spend(int amount);
void Money_Draw();
void Money_Save(const char* iniPath);
void Money_Load(const char* iniPath);
void Money_PickupScan(); // Call every frame or tick

// Optionally: expose the random money logic
int GetWeightedRandomMoney();
