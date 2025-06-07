// ai.h
#pragma once
#include "state.h"
#include "block.h"
#include "game.h" // game.h 应该已经包含了 state.h 和 block.h 的必要部分

struct AIMove {
    int rotationCount;
    int x;
    int y;          // AI决策的最终着陆Y坐标
    bool possible;
    double score;   // 评估分数
};

AIMove FindBestAIMove();
bool ExecuteAIMove(const AIMove& move); // 返回false如果move本身不可行