// game.h
#pragma once
#include "state.h"  // 包含 EGameState, EGameMode, WIDTH, HEIGHT 等

// 全局游戏数据变量声明 (定义在 game.cpp 中)
extern int gameMap[HEIGHT][WIDTH];
extern int score;
// gState, currentMode, nextShapeIndex 在 state.h 中声明 extern，在 main.cpp 中定义

// 游戏逻辑函数声明
bool CheckCollision(int testX, int testY); // 检查当前活动方块在指定位置是否碰撞
void MergeToMap();                         // 将当前活动方块合并到游戏地图
void ClearLines();                         // 清除满行并计分
bool IsGameOver();                         // 检查游戏是否结束 (新方块无法生成)
void ResetGame();                          // 重置游戏状态 (用于开始新游戏)

// AI_SimplePlaceBlock 的声明已从此文件移除
// ai.h 将包含新的AI函数声明，main.cpp 会直接包含 ai.h