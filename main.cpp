// main.cpp
#include <graphics.h> // EasyX - 必须最先包含（或者在预编译头中）
#include <windows.h>  // 用于 Sleep, GetTickCount 等
#include <time.h>     // 用于 srand
#include <stdio.h>    // 用于 _stprintf_s（如果使用 TCHAR 的 sprintf_s）
#include <tchar.h>    // 用于 _T, _tcscpy_s, _stprintf_s（Windows 特定的字符类型）
#include <string.h>   // 用于 memcpy（尽管 <cstring> 更符合 C++ 标准）

#include "state.h"    // 定义 EGameState, EGameMode, WIDTH, HEIGHT, 常量
#include "block.h"    // 方块相关的函数和变量声明 (NewBlock, Rotate, currentShape, curX, curY 等)
#include "game.h"     // 游戏逻辑函数和变量声明 (CheckCollision, MergeToMap, ClearLines, IsGameOver, ResetGame, score 等)
#include "graphics.h" // 绘图相关的函数声明 (InitGraphics, CloseGraphics, Draw, DrawPause 等)
#include "input.h"    // 输入处理函数声明 (InitInput, UpdateInputStates, ProcessInput_xxx 等)
#include "sound.h"    // 声音相关的函数声明 (PlayBGM, StopBGM)
#include "ai.h"       // AI 模块头文件 (FindBestAIMove, ExecuteAIMove)
#include "score.h"    // 高分榜 (AddHighScore)

// --- 全局状态变量 ---
EGameState gState = GS_MainMenu;
EGameMode currentMode = MODE_NORMAL;
int nextShapeIndex = 0; // 用于上帝模式AI选择的下一个方块

// --- 游戏结束信息 ---
TCHAR g_gameOverTitle[100];
TCHAR g_gameOverSubtitle[100];

// --- 计时器 ---
DWORD g_autoDropTimer = 0;      // 用于普通模式和地形放置模式的自动下落
DWORD g_aiActionTimer = 0;      // 用于上帝模式AI行动的计时器
DWORD g_aiLandedDelayTimer = 0;     // AI方块落地后的延迟计时器
const int AI_ACTION_DELAY_MS = 700; // AI行动间隔
bool  g_aiBlockHasLandedAndWaiting = false; // 标志：AI方块已落地，等待消行
const int AI_LANDED_DELAY_MS = 500; // AI方块落地后显示1秒再消行

// --- 上帝模式特定计数器 ---
int placedTerrainBlocks = 0;       // 玩家已放置的地形块数量

int playerSelectedBlocksForAI = 0; // 玩家为AI选择了多少块直到AI失败

// 辅助函数：检查AI放置后游戏是否结束
bool IsGameOverAfterAIPlacement() {
    int temp_curX_backup = curX, temp_curY_backup = curY;
    int temp_currentShape_backup[4][4];
    memcpy(temp_currentShape_backup, currentShape, sizeof(currentShape));

    NewBlockSpecific(0); // 尝试生成一个标准方块进行测试
    bool game_is_over = IsGameOver();

    curX = temp_curX_backup;
    curY = temp_curY_backup;
    memcpy(currentShape, temp_currentShape_backup, sizeof(currentShape));
    return game_is_over;
}


int main() {
    srand((unsigned int)time(NULL));

    InitGraphics();
    InitInput();    // 初始化输入系统 (包括键盘状态数组)
    PlayBGM();

    // 游戏主循环
    while (true) {
        UpdateInputStates(); // 每帧开始时更新键盘状态

        DWORD currentTime = GetTickCount();
        MOUSEMSG mouse = { 0 };
        while (MouseHit()) { // 处理所有待处理的鼠标消息，保留最后一个
            mouse = GetMouseMsg();
        }

        switch (gState) {
        case GS_MainMenu:
            DrawMainMenu();
            ProcessInput_MainMenu(mouse); // 此函数内部会根据选择设置 gState, currentMode,
            // 并调用 ResetGame, NewBlock
            // 以及初始化 g_autoDropTimer (如果进入GS_Playing或GS_GodMode_PlaceTerrain)
            break;

        case GS_Playing:
            ProcessInput_Playing(mouse); // 处理玩家在游戏中的输入

            if (gState == GS_Playing) { // 确保状态未因输入处理（如暂停）而改变
                // --- 所有 GS_Playing 的核心逻辑和绘图都在这个 if 块内部 ---

                if (currentMode == MODE_NORMAL) {
                    // 普通模式：自动方块下落和逻辑
                    if (currentTime - g_autoDropTimer >= NORMAL_DROP_SPEED_MS) {
                        if (!CheckCollision(curX, curY + 1)) {
                            curY++;
                        }
                        else {
                            MergeToMap();
                            ClearLines(); // 普通模式计分
                            NewBlock();
                            if (IsGameOver()) {
                                AddHighScore(score);
                                _tcscpy_s(g_gameOverTitle, _T("游戏结束"));
                                _stprintf_s(g_gameOverSubtitle, _T("您的分数: %d"), score);
                                gState = GS_GameOverScreen;
                            }
                        }
                        g_autoDropTimer = currentTime;
                    }
                }
                else { // currentMode == MODE_GOD (AI 逻辑)
                    if (g_aiBlockHasLandedAndWaiting) { // 如果AI方块已落地，正在等待消行
                        if (currentTime - g_aiLandedDelayTimer >= AI_LANDED_DELAY_MS) {
                            ClearLines();
                            g_aiBlockHasLandedAndWaiting = false;
                            if (IsGameOverAfterAIPlacement()) {
                                _tcscpy_s(g_gameOverTitle, _T("你赢了! (上帝模式)"));
                                _stprintf_s(g_gameOverSubtitle, _T("你选择的第 %d 个方块使AI无路可走!"), playerSelectedBlocksForAI);
                                gState = GS_GameOverScreen;
                            }
                            else {
                                gState = GS_GodMode_SelectShape;
                            }
                        }
                        // 在等待期间，不执行新的AI行动
                    }
                    else { // AI方块未落地或等待已结束，可以进行新的AI行动决策
                        if (currentTime - g_aiActionTimer >= AI_ACTION_DELAY_MS) {
                            AIMove aiMove = FindBestAIMove();
                            if (!ExecuteAIMove(aiMove)) {
                                _tcscpy_s(g_gameOverTitle, _T("你赢了! (上帝模式)"));
                                _stprintf_s(g_gameOverSubtitle, _T("AI无法处理你选择的第 %d 个方块。"), playerSelectedBlocksForAI);
                                gState = GS_GameOverScreen;
                            }
                            else {
                                MergeToMap();
                                g_aiBlockHasLandedAndWaiting = true;
                                g_aiLandedDelayTimer = currentTime;

                                // 尝试在合并后立即清空当前活动块的视觉残留
                                memset(currentShape, 0, sizeof(currentShape));
                                curColor = 0; // 或者一个背景色索引
                            }
                        }
                    }
                }

                // 只有当 gState 仍然是 GS_Playing (并且上面 currentMode 的逻辑执行完毕后) 才绘制
                Draw(); // 绘制游戏画面
            }
            // 如果 ProcessInput_Playing 将 gState 改为 GS_Paused，则上面的 if (gState == GS_Playing) 不会进入，
            // 也不会执行游戏逻辑或 Draw()。下一帧将进入 GS_Paused case。
            break; // GS_Playing 结束

        case GS_Paused:
            ProcessInput_Paused(mouse); // 处理暂停菜单的输入 (恢复、主菜单、退出)
            if (gState == GS_Paused) {  // 确保状态仍然是暂停才绘制
                Draw();                 // 绘制游戏板快照
                DrawPause();            // 绘制暂停菜单
            }
            break;

        case GS_GodMode_PlaceTerrain:
            ProcessInput_GodModePlaceTerrain(mouse); // 核心逻辑都在输入处理中

            if (gState == GS_GodMode_PlaceTerrain) { // 确保状态未因暂停等改变
                Draw(); // Draw() 函数需要能绘制当前玩家控制的预览方块
            }
            // 当 placedTerrainBlocks >= MAX_TERRAIN_BLOCKS 时，
            // ProcessInput_GodModePlaceTerrain 会将 gState 切换到 GS_GodMode_SelectShape。
            break;

        case GS_GodMode_SelectShape:
            ProcessInput_GodModeSelect(mouse); // 处理玩家为AI选择方块的输入
            // 此函数会设置 nextShapeIndex, 调用 NewBlockSpecific, 并可能将 gState 设为 GS_Playing

// 在 ProcessInput_GodModeSelect 之后绘制，以反映可能的即时UI变化（如果它做了的话）
            Draw(); // 绘制游戏板和通用UI（包括UI上的暂停按钮）
            DrawGodModeShapeSelection(); // 绘制特定的方块选择界面

            if (gState == GS_Playing) { // 如果玩家已选择完毕，状态切换到AI行动
                playerSelectedBlocksForAI++; // 玩家为AI选择的块数增加
                g_aiActionTimer = GetTickCount(); // 重置AI行动计时器
            }
            break;

        case GS_GameOverScreen:
            DrawGameOverScreen(g_gameOverTitle, g_gameOverSubtitle);
            ProcessInput_GameOver(mouse); // 处理返回主菜单的输入
            break;

        default:
            gState = GS_MainMenu; // 意外状态则返回主菜单
            break;
        }

        Sleep(10); // 控制帧率，降低CPU使用
    }

    StopBGM();
    CloseGraphics();
    return 0;
}