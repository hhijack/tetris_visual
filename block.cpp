// block.cpp
#include "block.h"
#include "game.h"    // 需要 CheckCollision (game.h 中声明)
#include <time.h>    // for srand, rand
#include <stdlib.h>  // for rand
#include <string.h>  // for memcpy

#define TERRAIN_COLOR_INDEX 7 // 或者其他你想要的颜色索引
                              // 确保 colors 数组有这个索引对应的颜色

// 全局方块相关变量定义
COLORREF colors[] = {
    RGB(0, 0, 0),       // 0. 空白（黑色）
    RGB(255, 0, 0),     // 1. 亮红色 (高对比度)
    RGB(255, 255, 0),   // 2. 明黄色 (醒目)
    RGB(0, 255, 0),     // 3. 荧光绿 (鲜艳)
    RGB(0, 0, 255),     // 4. 电蓝色 (纯净)
    RGB(255, 0, 255),   // 5. 洋红色 (紫红色调)
    RGB(255, 165, 0),   // 6. 橙色 (温暖鲜艳)
    RGB(0, 255, 255),   // 7. 青色 (冷色调)
    RGB(128, 128, 128)  // 8. 中性灰 (背景或分隔)
};

int shapes[7][4][4] = {
    { {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0} }, // I
    { {0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0} }, // O
    { {0,0,0,0}, {0,1,0,0}, {1,1,1,0}, {0,0,0,0} }, // T
    { {0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0} }, // S
    { {0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0} }, // Z
    { {0,0,0,0}, {1,0,0,0}, {1,1,1,0}, {0,0,0,0} }, // L
    { {0,0,0,0}, {0,0,1,0}, {1,1,1,0}, {0,0,0,0} }  // J
};

int currentShape[4][4];
int curX, curY;
int curColor;

void NewBlock() {
    int shapeIndex = rand() % 7;
    NewBlockSpecific(shapeIndex);
}

void NewBlockSpecific(int shapeIndex) {
    if (shapeIndex < 0 || shapeIndex >= 7) shapeIndex = 0; // 安全检查
    curColor = shapeIndex +1 ;

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            currentShape[i][j] = shapes[shapeIndex][i][j];

    curX = WIDTH / 2 - 2; // 初始位置居中

    // 修正初始Y值
    int firstNonEmptyRow = 0;
    bool foundBlockInShape = false;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (currentShape[r][c]) {
                firstNonEmptyRow = r;
                foundBlockInShape = true;
                goto found_row_block_cpp;
            }
        }
    }
found_row_block_cpp:
    if (foundBlockInShape) {
        curY = -firstNonEmptyRow;
    }
    else {
        curY = 0;
    }
}

void Rotate() {
    int temp[4][4];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            temp[i][j] = currentShape[3 - j][i]; // 顺时针旋转

    // 备份当前形状，以便碰撞时恢复
    int backupShape[4][4];
    memcpy(backupShape, currentShape, sizeof(backupShape));
    memcpy(currentShape, temp, sizeof(temp));

    if (CheckCollision(curX, curY)) { // CheckCollision 来自 game.h/game.cpp
        memcpy(currentShape, backupShape, sizeof(backupShape)); // 碰撞，恢复原状
    }
}

