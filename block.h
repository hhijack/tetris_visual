// block.h
#pragma once
#include <windows.h> // For COLORREF
#include "state.h"   // For WIDTH, HEIGHT (确保这些常量已定义)

// 全局方块相关变量声明 (定义在 block.cpp 中)
extern COLORREF colors[];
extern int shapes[7][4][4];
extern int currentShape[4][4];
extern int curX, curY;
extern int curColor;

// 函数声明
void NewBlock();                       // 普通模式生成随机方块
void NewBlockSpecific(int shapeIndex); // 根据索引生成特定方块
void Rotate();

// CheckCollision 在 game.h 中声明