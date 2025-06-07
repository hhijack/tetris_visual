// input.cpp
#define _CRT_SECURE_NO_WARNINGS
#include "input.h"
#include "state.h"    // For gState, currentMode, nextShapeIndex, EGameState, EGameMode, WIDTH, HEIGHT, BLOCK_SIZE, MOVE_INTERVAL, ROTATE_INTERVAL
#include "block.h"    // For curX, curY, Rotate()
#include "game.h"     // For CheckCollision(), ResetGame(), NewBlock(), NewBlockSpecific()
#include "graphics.h" // For CloseGraphics(), and constants like PAUSE_BTN_WIDTH, PAUSE_BTN_HEIGHT (if defined there and extern'd)
                      // or re-declare them here if they are specific to input's interpretation of UI elements.
#include "sound.h"    // For StopBGM()
#include <stdlib.h>   // For exit()
#include <string.h>   // For memset, memcpy
#include <stdio.h>

// --- 按键状态检测变量 ---
static bool g_key_states_current[256] = { false };  // 当前帧的按键状态
static bool g_key_states_previous[256] = { false }; // 上一帧的按键状态

// --- 输入计时器 ---
static DWORD g_key_lastMoveTime = 0;
static DWORD g_key_lastRotateTime = 0;

extern int placedTerrainBlocks;       // <--- 添加 extern 声明
extern int playerSelectedBlocksForAI; // <--- 添加 extern 声明

extern const int PAUSE_BTN_WIDTH;
extern const int PAUSE_BTN_HEIGHT;

// --- UI 按钮尺寸 (应与 graphics.cpp 中的绘图逻辑一致) ---
// 如果这些常量在 graphics.h 中声明为 extern const，则可以直接使用
// 否则，在这里重新定义，并确保它们与 graphics.cpp 中的值匹配


const int PAUSE_MENU_BTN_WIDTH_INPUT = 200; // 中央暂停菜单按钮的宽度
const int PAUSE_MENU_BTN_HEIGHT_INPUT = 40; // 中央暂停菜单按钮的高度
const int PAUSE_MENU_BTN_SPACING_INPUT = 20;// 中央暂停菜单按钮的间距


// --- 外部游戏逻辑计时器声明 ---
extern DWORD g_autoDropTimer;
extern DWORD g_aiActionTimer;

// --- 函数实现 ---

void InitInput() {
    memset(g_key_states_current, 0, sizeof(g_key_states_current));
    memset(g_key_states_previous, 0, sizeof(g_key_states_previous));
    g_key_lastMoveTime = GetTickCount();
    g_key_lastRotateTime = GetTickCount();
}

// 此函数应在 main.cpp 主循环的每次迭代开始时调用一次
void UpdateInputStates() {
    memcpy(g_key_states_previous, g_key_states_current, sizeof(g_key_states_current));
    for (int i = 0; i < 256; ++i) {
        // PeekMessage 方式通常更推荐，但 GetAsyncKeyState 也可以工作
        g_key_states_current[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
    }
}

bool IsKeyJustPressed(int vk) {
    if (vk < 0 || vk >= 256) return false; // 基本的边界检查
    return g_key_states_current[vk] && !g_key_states_previous[vk];
}

// 辅助函数：检测UI面板上的暂停按钮点击
// 此函数由 ProcessInput_Playing 和 ProcessInput_GodModeSelect 调用
void CheckUIPauseButtonClick(MOUSEMSG m) {
    if (gState == GS_Playing || gState == GS_GodMode_SelectShape || gState == GS_GodMode_PlaceTerrain) {
        int uiPanelX_input = WIDTH * BLOCK_SIZE + 20;
        // 修改：使用与 graphics.cpp 完全一致的计算方式
        int pauseButtonY_input = getheight() - PAUSE_BTN_HEIGHT - 20;

        // 修改：使用正确的常量名（与 graphics.cpp 一致）
        RECT actualUIPauseButtonRect = {
            uiPanelX_input, pauseButtonY_input,
            uiPanelX_input + PAUSE_BTN_WIDTH, pauseButtonY_input + PAUSE_BTN_HEIGHT
        };

        if (m.uMsg == WM_LBUTTONDOWN) {
            // 添加调试信息
            char debugMsg[200];
            sprintf_s(debugMsg, "Mouse click at (%d, %d), Button rect: (%d, %d, %d, %d)\n",
                m.x, m.y, actualUIPauseButtonRect.left, actualUIPauseButtonRect.top,
                actualUIPauseButtonRect.right, actualUIPauseButtonRect.bottom);
            OutputDebugStringA(debugMsg);

            if (m.x >= actualUIPauseButtonRect.left && m.x <= actualUIPauseButtonRect.right &&
                m.y >= actualUIPauseButtonRect.top && m.y <= actualUIPauseButtonRect.bottom) {
                OutputDebugStringA("UI Pause button clicked!\n");
                if (gState != GS_Paused) {
                    gState = GS_Paused;
                }
            }
        }
    }
}

void ProcessInput_Playing(MOUSEMSG m) { // 添加 MOUSEMSG m 参数

    static bool pKeyWasPressed = false;
    if (GetAsyncKeyState('P') & 0x8000) {
        if (!pKeyWasPressed) {
            OutputDebugStringA("P key pressed (direct check)!\n");
            gState = GS_Paused;
            pKeyWasPressed = true;
            return;
        }
    }
    else {
        pKeyWasPressed = false;
    }

    CheckUIPauseButtonClick(m); // 首先检查UI暂停按钮的点击
    if (gState == GS_Paused) {  // 如果点击UI按钮导致暂停，则立即返回
        return;
    }

    DWORD now = GetTickCount();

    // 左右移动 (单按或长按)
    if (IsKeyJustPressed(VK_LEFT) || IsKeyJustPressed('A')) {
        if (!CheckCollision(curX - 1, curY)) curX--;
        g_key_lastMoveTime = now;
    }
    else if ((g_key_states_current[VK_LEFT] || g_key_states_current['A']) && now - g_key_lastMoveTime > MOVE_INTERVAL) { // 长按用当前状态
        if (!CheckCollision(curX - 1, curY)) curX--;
        g_key_lastMoveTime = now;
    }

    if (IsKeyJustPressed(VK_RIGHT) || IsKeyJustPressed('D')) {
        if (!CheckCollision(curX + 1, curY)) curX++;
        g_key_lastMoveTime = now;
    }
    else if ((g_key_states_current[VK_RIGHT] || g_key_states_current['D']) && now - g_key_lastMoveTime > MOVE_INTERVAL) { // 长按用当前状态
        if (!CheckCollision(curX + 1, curY)) curX++;
        g_key_lastMoveTime = now;
    }

    // 旋转 (仅单次按下)
    if ((IsKeyJustPressed(VK_UP) || IsKeyJustPressed('W'))) { // 移除了旋转的计时器，通常旋转不应有长按延迟
        Rotate();
        // g_key_lastRotateTime = now; // 如果确实需要旋转间隔，可以保留
    }

    // 加速下落 (软降) - 按住时持续生效
    if (g_key_states_current[VK_DOWN] || g_key_states_current['S']) {
        if (!CheckCollision(curX, curY + 1)) {
            curY++;
            // 可以考虑稍微重置一下自动下落计时器，使软降更即时
            // g_autoDropTimer = now; // 或者 g_autoDropTimer += some_small_value;
        }
    }

    // 快速下落 (硬降) - 单次按下
    if (IsKeyJustPressed(VK_SPACE)) {
        while (!CheckCollision(curX, curY + 1)) {
            curY++;
        }
        if (currentMode == MODE_NORMAL) { // 硬降后在普通模式下立即锁定
            g_autoDropTimer = 0; // 触发主循环中的方块落地逻辑
        }
        // AI模式下，硬降由AI决策，这里不需要特殊处理 g_aiActionTimer
    }

    //// 键盘暂停
    //if (IsKeyJustPressed('P') || IsKeyJustPressed(VK_PAUSE)) {
    //    if (gState == GS_Playing) { // 只能从游戏进行中暂停
    //        gState = GS_Paused;
    //        // 无需在此处 InitInput() 或重置游戏计时器
    //    }
    //}
}

void ProcessInput_Paused(MOUSEMSG m) {
    // 计算中央暂停菜单按钮的RECT (与 graphics.cpp DrawPause() 中的逻辑一致)
    RECT continueBtnRect_input, mainMenuBtnRect_input, exitBtnRect_input;
    int totalButtonHeight = 3 * PAUSE_MENU_BTN_HEIGHT_INPUT + 2 * PAUSE_MENU_BTN_SPACING_INPUT;
    int startY_input = (HEIGHT * BLOCK_SIZE) / 2 - totalButtonHeight / 2 + 30;
    int centerX_input = (WIDTH * BLOCK_SIZE) / 2;

    continueBtnRect_input = {
        centerX_input - PAUSE_MENU_BTN_WIDTH_INPUT / 2, startY_input,
        centerX_input + PAUSE_MENU_BTN_WIDTH_INPUT / 2, startY_input + PAUSE_MENU_BTN_HEIGHT_INPUT
    };
    startY_input += PAUSE_MENU_BTN_HEIGHT_INPUT + PAUSE_MENU_BTN_SPACING_INPUT;
    mainMenuBtnRect_input = {
        centerX_input - PAUSE_MENU_BTN_WIDTH_INPUT / 2, startY_input,
        centerX_input + PAUSE_MENU_BTN_WIDTH_INPUT / 2, startY_input + PAUSE_MENU_BTN_HEIGHT_INPUT
    };
    startY_input += PAUSE_MENU_BTN_HEIGHT_INPUT + PAUSE_MENU_BTN_SPACING_INPUT;
    exitBtnRect_input = {
        centerX_input - PAUSE_MENU_BTN_WIDTH_INPUT / 2, startY_input,
        centerX_input + PAUSE_MENU_BTN_WIDTH_INPUT / 2, startY_input + PAUSE_MENU_BTN_HEIGHT_INPUT
    };

    // 鼠标点击检测
    if (m.uMsg == WM_LBUTTONDOWN) {
        if (m.x >= continueBtnRect_input.left && m.x <= continueBtnRect_input.right &&
            m.y >= continueBtnRect_input.top && m.y <= continueBtnRect_input.bottom) {
            // 修改：根据当前模式恢复到正确的状态
            if (currentMode == MODE_GOD) {
                // 检查是否还在地形放置阶段
                if (placedTerrainBlocks < MAX_TERRAIN_BLOCKS) {
                    gState = GS_GodMode_PlaceTerrain;
                }
                else {
                    gState = GS_GodMode_SelectShape;
                }
            }
            else {
                gState = GS_Playing;
            }
            InitInput(); // 清除按键状态，重置输入计时器
            DWORD currentTime = GetTickCount();
            g_autoDropTimer = currentTime; // 重置游戏逻辑计时器
            if (currentMode == MODE_GOD && gState == GS_Playing) {
                g_aiActionTimer = currentTime;
            }
            return; // 处理完一个动作后立即返回
        }
        else if (m.x >= mainMenuBtnRect_input.left && m.x <= mainMenuBtnRect_input.right &&
            m.y >= mainMenuBtnRect_input.top && m.y <= mainMenuBtnRect_input.bottom) {
            gState = GS_MainMenu;
            // ResetGame() 将在从主菜单选择新模式时处理
            return;
        }
        else if (m.x >= exitBtnRect_input.left && m.x <= exitBtnRect_input.right &&
            m.y >= exitBtnRect_input.top && m.y <= exitBtnRect_input.bottom) {
            StopBGM();
            CloseGraphics();
            exit(0);
        }
    }

    // 键盘快捷键
    if (IsKeyJustPressed('P')) { // 继续游戏
        // 修改：同样的逻辑
        if (currentMode == MODE_GOD) {
            if (placedTerrainBlocks < MAX_TERRAIN_BLOCKS) {
                gState = GS_GodMode_PlaceTerrain;
            }
            else {
                gState = GS_GodMode_SelectShape;
            }
        }
        else {
            gState = GS_Playing;
        }
        InitInput();
        DWORD currentTime = GetTickCount();
        g_autoDropTimer = currentTime;
        if (currentMode == MODE_GOD && gState == GS_Playing) {
            g_aiActionTimer = currentTime;
        }
        return;
    }
    if (IsKeyJustPressed('M')) { // 返回主菜单
        gState = GS_MainMenu;
        return;
    }
    if (IsKeyJustPressed(VK_ESCAPE)) { // 退出游戏
        StopBGM();
        CloseGraphics();
        exit(0);
    }
}

void ProcessInput_MainMenu(MOUSEMSG m) {
    // 按钮区域需要与 DrawMainMenu 中的一致
    int buttonWidth = 220; // 这些尺寸应与 graphics.cpp DrawMainMenu() 中的一致
    int buttonHeight = 50;
    // 注意：DrawMainMenu 使用 (WIDTH + 7) * BLOCK_SIZE / 2 作为窗口中心
    // 所以这里的 centerX 也应该匹配
    int window_centerX = ((WIDTH + 7) * BLOCK_SIZE) / 2;

    RECT normalModeBtnRect = { window_centerX - buttonWidth / 2, HEIGHT * BLOCK_SIZE / 2 - buttonHeight - 10,
                           window_centerX + buttonWidth / 2, HEIGHT * BLOCK_SIZE / 2 - 10 };
    RECT godModeBtnRect = { window_centerX - buttonWidth / 2, HEIGHT * BLOCK_SIZE / 2 + 10,
                        window_centerX + buttonWidth / 2, HEIGHT * BLOCK_SIZE / 2 + buttonHeight + 10 };

    if (m.uMsg == WM_LBUTTONDOWN) {
        if (m.x >= normalModeBtnRect.left && m.x <= normalModeBtnRect.right &&
            m.y >= normalModeBtnRect.top && m.y <= normalModeBtnRect.bottom) {
            currentMode = MODE_NORMAL;
            gState = GS_Playing;
            ResetGame();
            NewBlock(); // 普通模式开始时生成第一个方块
            InitInput(); // 初始化输入状态和计时器
            g_autoDropTimer = GetTickCount(); // 初始化游戏计时器
            return; // 处理完一个动作后立即返回
        }
        else if (m.x >= godModeBtnRect.left && m.x <= godModeBtnRect.right &&
            m.y >= godModeBtnRect.top && m.y <= godModeBtnRect.bottom) {
            // *** 这是你询问的修改点 (鼠标点击上帝模式按钮) ***
            currentMode = MODE_GOD;
            gState = GS_GodMode_PlaceTerrain; // 修改：先进入地形放置状态
            ResetGame();                      // 清空场地，为地形和后续游戏做准备
            placedTerrainBlocks = 0;          // 重置计数器 (确保 placedTerrainBlocks 可访问)
            InitInput();                      // 初始化输入
            return;                           // 处理完一个动作后立即返回
        }
    }

    // 键盘快捷键
    if (IsKeyJustPressed('N')) {
        currentMode = MODE_NORMAL;
        gState = GS_Playing;
        ResetGame();
        NewBlock();
        InitInput();
        g_autoDropTimer = GetTickCount();
        return; // 处理完一个动作后立即返回
    }
    if (IsKeyJustPressed('G')) {
        // *** 这是你询问的修改点 (键盘 'G' 快捷键) ***
        currentMode = MODE_GOD;
        gState = GS_GodMode_PlaceTerrain; // 修改：先进入地形放置状态
        ResetGame();
        placedTerrainBlocks = 0;          // 重置计数器

        InitInput();
        return;                           // 处理完一个动作后立即返回
    }
}

void ProcessInput_GodModeSelect(MOUSEMSG m) {
    // 添加直接按键检测（与 ProcessInput_Playing 中相同的逻辑）
    static bool pKeyWasPressed_select = false;
    if (GetAsyncKeyState('P') & 0x8000) {
        if (!pKeyWasPressed_select) {
            OutputDebugStringA("P key pressed in GodModeSelect (direct check)!\n");
            gState = GS_Paused;
            pKeyWasPressed_select = true;
            return;
        }
    }
    else {
        pKeyWasPressed_select = false;
    }

    CheckUIPauseButtonClick(m);
    if (gState == GS_Paused) {
        return;
    }
    CheckUIPauseButtonClick(m); // 首先检查UI暂停按钮的点击
    if (gState == GS_Paused) {  // 如果点击UI按钮导致暂停，则立即返回
        return;
    }

    // 键盘选择 1-7
    for (int i = 0; i < 7; i++) {
        if (IsKeyJustPressed('1' + i)) {
            nextShapeIndex = i;
            gState = GS_Playing;
            NewBlockSpecific(nextShapeIndex);
            // g_aiActionTimer 将在 main.cpp 中 GS_GodMode_SelectShape 状态转换到 GS_Playing 时设置
            return;
        }
    }

    // 鼠标点击选择方块预览 (与 graphics.cpp DrawGodModeShapeSelection() 中的布局匹配)
    if (m.uMsg == WM_LBUTTONDOWN) {
        int uiPanelX_preview = WIDTH * BLOCK_SIZE + 20;
        int startY_preview = 20 + 30 + 30; // 与 DrawGodModeShapeSelection 中起始Y对应
        int previewBoxDrawHeight = 4 * PREVIEW_BLOCK_SIZE_GOD_MODE; // PREVIEW_BLOCK_SIZE_GOD_MODE 来自 graphics.h 或 state.h
        int previewBoxDrawWidth = 4 * PREVIEW_BLOCK_SIZE_GOD_MODE;
        int previewPadding = 5;
        int currentClickY = startY_preview;
        int currentClickX = uiPanelX_preview;

        // 假设 DrawGodModeShapeSelection 的布局逻辑 (例如，是否换列)
        // 这个例子假设简单垂直排列，如果 DrawGodModeShapeSelection 有换列，这里也需要匹配
        int shapesPerRow = 1; // 默认每行一个
        // 简单的换列逻辑示例 (需要与 DrawGodModeShapeSelection 中的一致)
        int maxShapesInColumn = (HEIGHT * BLOCK_SIZE - startY_preview) / (previewBoxDrawHeight + previewPadding);
        if (maxShapesInColumn < 1) maxShapesInColumn = 1; // 至少能放一个
        if (7 > maxShapesInColumn && maxShapesInColumn > 0) { // 如果一列放不下7个
            shapesPerRow = (7 + maxShapesInColumn - 1) / maxShapesInColumn; // 计算需要多少列，这里简化为如果一列不够就尝试两列
            if (shapesPerRow > 2) shapesPerRow = 2; // 最多两列示例
        }


        for (int i = 0; i < 7; i++) {
            // 如果 graphics.cpp 中的 DrawGodModeShapeSelection 有复杂的换列逻辑，这里需要精确匹配
            // 当前的 shapesPerRow 逻辑可能与 graphics.cpp 中的不完全一致，需要仔细校对
            if (i > 0 && i % maxShapesInColumn == 0 && shapesPerRow > 1) { // 换到下一列的简单示例
                currentClickX += previewBoxDrawWidth + 30; // 假设列间距30
                currentClickY = startY_preview;
            }
            else if (i > 0) {
                currentClickY += previewBoxDrawHeight + previewPadding;
            }


            RECT blockRect = { currentClickX, currentClickY,
                               currentClickX + previewBoxDrawWidth,
                               currentClickY + previewBoxDrawHeight };

            if (m.x >= blockRect.left && m.x <= blockRect.right &&
                m.y >= blockRect.top && m.y <= blockRect.bottom) {
                nextShapeIndex = i;
                gState = GS_Playing;
                NewBlockSpecific(nextShapeIndex);
                return;
            }
        }
    }
}

void ProcessInput_GameOver(MOUSEMSG m) {
    if (m.uMsg == WM_LBUTTONDOWN || IsKeyJustPressed(VK_RETURN) || IsKeyJustPressed(VK_SPACE)) {
        gState = GS_MainMenu;
        // InitInput() 和 ResetGame() 会在从主菜单选择新模式时调用
    }
}

void ProcessInput_GodModePlaceTerrain(MOUSEMSG m) {
    // 添加直接按键检测
    static bool pKeyWasPressed_terrain = false;
    if (GetAsyncKeyState('P') & 0x8000) {
        if (!pKeyWasPressed_terrain) {
            OutputDebugStringA("P key pressed in GodModePlaceTerrain (direct check)!\n");
            gState = GS_Paused;
            pKeyWasPressed_terrain = true;
            return;
        }
    }
    else {
        pKeyWasPressed_terrain = false;
    }

    CheckUIPauseButtonClick(m); // UI暂停按钮
    if (gState == GS_Paused) return;

    // --- 1. 选择地形方块形状 ---
    for (int i = 0; i < 7; ++i) {
        if (IsKeyJustPressed('1' + i)) {
            NewBlockSpecific(i); // 使用玩家选择的形状更新 currentShape, curColor
            // NewBlockSpecific 会设置 curX 到中间，curY 到顶部
// curY = 0; // 或者强制设置一个初始的预览Y位置，例如屏幕顶部几行
// 确保新选择的方块在初始位置不碰撞，如果碰撞，可能需要微调初始X/Y
            if (CheckCollision(curX, curY)) {
                // 如果新选的形状在默认出生点就碰撞，尝试向上微调Y
                // 这是一个简化处理，更复杂的可能需要找一个无碰撞的出生点
                curY = -1; // 尝试更高一点
                if (CheckCollision(curX, curY)) curY = -2; // 再高一点
                // 如果还碰撞，可能场地太满了，或者形状太大
            }
            return; // 选择完形状后，等待玩家移动或放置
        }
    }

    // --- 2. 控制当前预览方块 (currentShape at curX, curY) ---
    int tentativoX = curX;
    int tentativoY = curY;

    if (IsKeyJustPressed(VK_LEFT) || IsKeyJustPressed('A')) tentativoX--;
    if (IsKeyJustPressed(VK_RIGHT) || IsKeyJustPressed('D')) tentativoX++;
    if (IsKeyJustPressed(VK_UP) || IsKeyJustPressed('W')) tentativoY--;   // 允许上移
    if (IsKeyJustPressed(VK_DOWN) || IsKeyJustPressed('S')) tentativoY++; // 允许下移

    if (tentativoX != curX || tentativoY != curY) { // 如果有移动意图
        // 检查移动后的预览位置是否与已固定方块或边界冲突
        // 注意：这里的 CheckCollision 是针对预览方块的，它不应该检查与 gameMap 底部的碰撞（因为它还没“落下”）
        // 但它需要检查是否与 gameMap 中已有的块重叠，或是否超出左右边界。
        // 对于上下移动，如果 curY + (方块的最低点) >= HEIGHT，则不能再向下。
        // 如果 curY + (方块的最高点) < 0，则不能再向上。

        // 简化的碰撞检查：只要不与已固定块重叠且在左右边界内即可
        // 实际的上下边界由玩家控制，放置时再做最终检查
        bool canMoveToNewPos = true;
        int tempShape[4][4]; // 创建副本以进行旋转测试或复杂碰撞测试
        memcpy(tempShape, currentShape, sizeof(currentShape));

        // 模拟一个临时的 currentShape 和位置进行检查
        // （或者修改 CheckCollision 使其能接受形状作为参数）
        // 为简单起见，我们直接用全局 currentShape 和 testX, testY
        if (!CheckCollision(tentativoX, tentativoY)) { // 假设 CheckCollision 检查的是与 gameMap 和边界的重叠
            curX = tentativoX;
            curY = tentativoY;
        }
        // 更精细的控制：
        // if (!CheckCollision(tentativoX, curY)) curX = tentativoX;
        // if (!CheckCollision(curX, tentativoY)) curY = tentativoY; // 用更新后的curX
    }

    if (IsKeyJustPressed('R')) { // 假设 'R' 是旋转键
        Rotate(); // Rotate 内部应该有碰撞检测和回滚逻辑
        // 确保旋转后的预览方块仍在合法位置
    }

    // --- 3. 放置方块 (硬降) ---
    if (IsKeyJustPressed(VK_SPACE)) {
        if (currentShape[0][0] == 0 && currentShape[0][1] == 0 && /* ...检查是否为空形状... */
            currentShape[1][0] == 0 && currentShape[1][1] == 0 &&
            currentShape[2][0] == 0 && currentShape[2][1] == 0 &&
            currentShape[3][0] == 0 && currentShape[3][1] == 0 &&
            currentShape[0][2] == 0 && currentShape[0][3] == 0 &&
            currentShape[1][2] == 0 && currentShape[1][3] == 0 &&
            currentShape[2][2] == 0 && currentShape[2][3] == 0 &&
            currentShape[3][2] == 0 && currentShape[3][3] == 0) {
            // 如果 currentShape 是空的 (例如，刚放置完一个，还没选下一个)
            // 则不执行放置操作，或者提示玩家先选形状
            return;
        }


        int landingY = curY;
        // 从当前 curY 开始向下探测，直到碰撞或到达底部
        while (!CheckCollision(curX, landingY + 1)) {
            landingY++;
        }

        // 检查最终着陆点是否有效 (例如，不能完全在屏幕上方)
        bool canPlace = true;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                if (currentShape[r][c]) {
                    if (landingY + r < 0) { // 如果方块的任何部分在硬降后仍在屏幕上方
                        canPlace = false; break;
                    }
                }
            }
            if (!canPlace) break;
        }
        // 并且，硬降后的位置不能与现有固定块重叠（CheckCollision 在 while 中已保证）
        // 但如果硬降后直接就游戏结束了（比如堵死了最上面一行），也算不能放置？
        // 这个模式下，我们通常允许玩家放置，除非它导致一个物理上不可能的状态。

        if (canPlace) {
            int originalCurY = curY; // 保存一下预览的Y
            curY = landingY;         // 设置为着陆点以合并
            MergeToMap();
            curY = originalCurY;     // 恢复预览的Y（虽然马上会被新选择的形状覆盖）

            placedTerrainBlocks++;

            if (placedTerrainBlocks >= MAX_TERRAIN_BLOCKS) {
                gState = GS_GodMode_SelectShape;
                playerSelectedBlocksForAI = 0;
                memset(currentShape, 0, sizeof(currentShape)); // 清空，等待AI模式的第一个块
            }
            else {
                // 允许玩家选择下一个地形块，或者自动给一个默认的预览块
                // 当前currentShape不清空，玩家可以继续移动它或按1-7换新的
                // 或者清空，提示玩家按1-7选择:
                // memset(currentShape, 0, sizeof(currentShape));
                // curColor = 0; // 无效颜色
                // 更好的做法是，放置后立即生成一个新的默认预览块，比如I形：
                NewBlockSpecific(0); // 或者玩家上次选择的形状
                // curY = 0; // 重置到顶部预览
            }
        }
        else {
            // 提示玩家当前预览位置或形状无法有效放置 (例如，硬降后仍在屏幕外)
        }
    }
}