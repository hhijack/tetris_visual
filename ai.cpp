// ai.cpp
#include "ai.h"
#include "block.h" // For Rotate, currentShape, curX, curY
#include "game.h"  // For CheckCollision, gameMap (for simulation)
#include <string.h>
#include <limits.h>
#include <vector>
#include <cmath>

// AnalyzeTemporaryBoard, EvaluateBoardState, SimulateDropAndGetLandingY 函数保持不变
// ... (这些函数的代码如前一个回复所示) ...
void AnalyzeTemporaryBoard(int tempGameMap[HEIGHT][WIDTH],
    int& aggregateHeight,
    int& completeLines,
    int& holes,
    int& bumpiness) {
    aggregateHeight = 0;
    completeLines = 0;
    holes = 0;
    bumpiness = 0;
    int columnHeights[WIDTH] = { 0 };
    for (int j = 0; j < WIDTH; j++) {
        int r = 0;
        while (r < HEIGHT && tempGameMap[r][j] == 0) r++;
        columnHeights[j] = HEIGHT - r;
        aggregateHeight += columnHeights[j];
    }
    for (int j = 0; j < WIDTH - 1; j++) bumpiness += std::abs(columnHeights[j] - columnHeights[j + 1]);
    for (int r_loop = 0; r_loop < HEIGHT; r_loop++) {
        bool lineIsFull = true;
        for (int j = 0; j < WIDTH; j++) if (tempGameMap[r_loop][j] == 0) lineIsFull = false;
        if (lineIsFull) completeLines++;
    }
    for (int j = 0; j < WIDTH; ++j) {
        bool blockAbove = false;
        for (int r_loop = 0; r_loop < HEIGHT; ++r_loop) {
            if (tempGameMap[r_loop][j] != 0) blockAbove = true;
            else if (blockAbove) holes++;
        }
    }
}

double EvaluateBoardState(int tempGameMap[HEIGHT][WIDTH], int linesClearedDuringPlacement) {
    int aggregateHeight, completeLinesAfterPlacement, holes, bumpiness;
    AnalyzeTemporaryBoard(tempGameMap, aggregateHeight, completeLinesAfterPlacement, holes, bumpiness);
    double w_linesCleared = 50.0, w_aggregateHeight = -1.0, w_holes = -7.0, w_bumpiness = -0.5;
    double score = 0;
    score += linesClearedDuringPlacement * w_linesCleared;
    score += aggregateHeight * w_aggregateHeight;
    score += holes * w_holes;
    score += bumpiness * w_bumpiness;
    return score;
}

int SimulateDropAndGetLandingY(int shape[4][4], int startX, int& landingY) {
    int backup_currentShape[4][4]; int backup_curX = curX, backup_curY = curY;
    memcpy(backup_currentShape, currentShape, sizeof(currentShape));
    memcpy(currentShape, shape, sizeof(currentShape));
    curX = startX; int initialY = 0;
    for (int r = 0; r < 4; ++r) { bool found = false; for (int c = 0; c < 4; ++c) if (currentShape[r][c]) { initialY = -r; found = true; break; } if (found)break; }
    curY = initialY; landingY = curY;
    if (CheckCollision(curX, curY)) {
        bool partBelowTop = false;
        for (int r = 0; r < 4; ++r) for (int c = 0; c < 4; ++c) if (currentShape[r][c] && (curY + r >= 0)) partBelowTop = true;
        if (partBelowTop) {
            memcpy(currentShape, backup_currentShape, sizeof(currentShape));
            curX = backup_curX; curY = backup_curY; return HEIGHT + 2;
        }
    }
    while (!CheckCollision(curX, landingY + 1)) landingY++;
    bool topped_out = false;
    for (int i = 0; i < 4; i++) { for (int j = 0; j < 4; j++) if (currentShape[i][j] && (landingY + i < 0)) { topped_out = true; break; } if (topped_out) break; }
    memcpy(currentShape, backup_currentShape, sizeof(currentShape));
    curX = backup_curX; curY = backup_curY;
    if (topped_out) return HEIGHT + 1;
    return landingY;
}

AIMove FindBestAIMove() {
    AIMove bestMove = { 0, 0, 0, false, -1e9 };
    int originalShapeData[4][4]; memcpy(originalShapeData, currentShape, sizeof(originalShapeData));
    int shapeToTest[4][4];
    for (int rotCount = 0; rotCount < 4; rotCount++) {
        memcpy(shapeToTest, originalShapeData, sizeof(shapeToTest));
        for (int r_rot = 0; r_rot < rotCount; r_rot++) {
            int tempRot[4][4];
            for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) tempRot[i][j] = shapeToTest[3 - j][i];
            memcpy(shapeToTest, tempRot, sizeof(tempRot));
        }
        for (int testX = -3; testX < WIDTH; ++testX) {
            int landingY_output;
            int finalLandingY = SimulateDropAndGetLandingY(shapeToTest, testX, landingY_output);
            if (finalLandingY > HEIGHT) continue;
            int tempGameMap[HEIGHT][WIDTH]; memcpy(tempGameMap, gameMap, sizeof(gameMap));
            for (int r_place = 0; r_place < 4; r_place++) {
                for (int c_place = 0; c_place < 4; c_place++) {
                    if (shapeToTest[r_place][c_place]) {
                        if (finalLandingY + r_place >= 0 && finalLandingY + r_place < HEIGHT &&
                            testX + c_place >= 0 && testX + c_place < WIDTH) {
                            tempGameMap[finalLandingY + r_place][testX + c_place] = 1;
                        }
                    }
                }
            }
            int linesClearedThisMove = 0;
            for (int r_clear = HEIGHT - 1; r_clear >= 0; r_clear--) {
                bool full = true;
                for (int c_clear = 0; c_clear < WIDTH; c_clear++) if (tempGameMap[r_clear][c_clear] == 0) { full = false; break; }
                if (full) {
                    linesClearedThisMove++;
                    for (int k = r_clear; k > 0; k--) memcpy(tempGameMap[k], tempGameMap[k - 1], sizeof(tempGameMap[0]));
                    memset(tempGameMap[0], 0, sizeof(tempGameMap[0]));
                    r_clear++;
                }
            }
            double currentScore = EvaluateBoardState(tempGameMap, linesClearedThisMove);
            if (currentScore > bestMove.score) {
                bestMove.score = currentScore; bestMove.rotationCount = rotCount;
                bestMove.x = testX; bestMove.y = finalLandingY; bestMove.possible = true;
            }
        }
    }
    return bestMove;
}


// AI执行移动 - 只设置方块姿态和位置，不合并或消行
bool ExecuteAIMove(const AIMove& move) {
    if (!move.possible) {
        return false; // AI 认为此移动不可行（例如，所有尝试都会导致顶部出界）
    }

    // 确保 currentShape 是玩家选择的那个方块的原始（未旋转）状态
    // NewBlockSpecific(nextShapeIndex) 应该在调用 FindBestAIMove 之前已经完成
    // FindBestAIMove 内部会备份和操作 currentShape 的副本进行模拟

    // 1. 应用旋转到全局的 currentShape
    // 假设 currentShape 在调用此函数前是 NewBlockSpecific 刚生成的原始形状
    for (int i = 0; i < move.rotationCount; i++) {
        Rotate(); // Rotate 会修改全局的 currentShape
    }

    // 2. 设置最终的X, Y坐标
    curX = move.x;
    curY = move.y; // AI 已经计算好了精确的着陆Y坐标

    // ExecuteAIMove 不再调用 MergeToMap 或 ClearLines
    // 它只负责将方块移动到最终位置和姿态
    // 后续的合并、消行、游戏结束判断由 main 循环处理

    return true; // 表示AI成功地“准备好”了方块的放置
}