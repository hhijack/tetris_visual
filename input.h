// input.h
#pragma once
#include <windows.h> // For GetAsyncKeyState, VK_*, RECT
#include <graphics.h> // For MOUSEMSG (EasyX)
// MOVE_INTERVAL, ROTATE_INTERVAL 在 state.h 中定义

// 输入处理函数声明
void InitInput(); // 初始化输入相关的静态变量
bool IsKeyJustPressed(int vk); // 检测按键是否刚被按下 (单次触发)

void ProcessInput_Playing(MOUSEMSG m);      // GS_Playing 状态下的输入处理
void ProcessInput_Paused();       // GS_Paused 状态下的输入处理
void ProcessInput_MainMenu(MOUSEMSG m); // GS_MainMenu 状态下的鼠标输入
void ProcessInput_GodModeSelect(MOUSEMSG m); // GS_GodMode_SelectShape 状态下的输入
void ProcessInput_GameOver(MOUSEMSG m);   // GS_GameOverScreen 状态下的输入
void ProcessInput_Paused(MOUSEMSG m); // GS_Paused 状态下的输入处理 (添加 MOUSEMSG)
void UpdateInputStates();  // 新增：每帧更新键盘状态
void ProcessInput_GodModePlaceTerrain(MOUSEMSG m);