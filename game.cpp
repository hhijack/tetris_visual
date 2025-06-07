// game.cpp
#include "game.h"
#include "block.h"   // For currentShape, curX, curY, curColor, Rotate()
#include <string.h>  // For memcpy, memset
#include <stdio.h>   // For NULL

// 全局游戏数据变量定义
int gameMap[HEIGHT][WIDTH] = { 0 };
int score = 0;
// gState, currentMode, nextShapeIndex 在 main.cpp 中定义


bool CheckCollision(int testX, int testY) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentShape[i][j]) { // 只检查方块的实体部分
                int boardX = testX + j;
                int boardY = testY + i;

                if (boardX < 0 || boardX >= WIDTH || boardY >= HEIGHT) {
                    return true; // 撞墙或底
                }
                if (boardY >= 0 && gameMap[boardY][boardX]) { // 和已固定方块碰撞
                    return true;
                }
            }
        }
    }
    return false;
}

void MergeToMap() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentShape[i][j]) {
                if (curY + i >= 0 && curY + i < HEIGHT &&
                    curX + j >= 0 && curX + j < WIDTH) {
                    gameMap[curY + i][curX + j] = curColor;
                }
            }
        }
    }
}

void ClearLines() {
    int linesCleared = 0;
    for (int i = HEIGHT - 1; i >= 0; i--) {
        bool full = true;
        for (int j = 0; j < WIDTH; j++) {
            if (gameMap[i][j] == 0) {
                full = false;
                break;
            }
        }
        if (full) {
            linesCleared++;
            for (int k = i; k > 0; k--) {
                memcpy(gameMap[k], gameMap[k - 1], sizeof(gameMap[0]));
            }
            memset(gameMap[0], 0, sizeof(gameMap[0]));
            i++;
        }
    }
    if (gState == GS_Playing && currentMode == MODE_NORMAL && linesCleared > 0) { // 只在普通模式且游戏中计分
        score += linesCleared * linesCleared * 100;
    }
}

bool IsGameOver() {
    // 游戏结束的条件是：新生成的方块在它的初始位置就发生碰撞
    // NewBlock() 或 NewBlockSpecific() 已经设置了 curX, curY
    return CheckCollision(curX, curY);
}

// game.cpp - ResetGame()
void ResetGame() {
    memset(gameMap, 0, sizeof(gameMap));
    score = 0; // 普通模式分数
    
    // placedTerrainBlocks 应该在进入 GS_GodMode_PlaceTerrain 状态时重置
}

// AI_SimplePlaceBlock 函数定义已从此文件移除
// 新的AI逻辑在 ai.cpp 中实现