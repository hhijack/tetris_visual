// graphics.cpp
#define NOMINMAX
#include "graphics.h"
#include "state.h"    // For gState, currentMode, WIDTH, HEIGHT, BLOCK_SIZE, MAX_TERRAIN_BLOCKS
#include "block.h"    // For colors, shapes, currentShape, curX, curY, curColor
#include "game.h"     // For gameMap, score, CheckCollision (for ghost piece)
#include <stdio.h>    // For _stprintf_s
#include <tchar.h>    // For TCHAR, _T
#include <algorithm>  // For std::max and std::min

// --- 全局变量的 extern 声明 ---
// 这些变量在其他 .cpp 文件中定义 (通常是 main.cpp 或各自的模块)
// 确保这些声明与实际定义匹配
extern EGameState gState;
extern EGameMode currentMode;
extern int gameMap[HEIGHT][WIDTH];
extern int score;
extern int currentShape[4][4];
extern int curX, curY;
extern int curColor;
extern int placedTerrainBlocks;
extern int playerSelectedBlocksForAI;
extern COLORREF colors[];      // 定义在 block.cpp
extern int shapes[7][4][4]; // 定义在 block.cpp

// --- 图形常量 ---
const int PREVIEW_BLOCK_SIZE_GOD_MODE = 14; // 用于上帝模式下右侧UI面板的方块预览单元格大小
const int UI_PREVIEW_PADDING_Y = 3;         // 上帝模式下右侧UI面板中预览块之间的垂直间距

const int PAUSE_BTN_WIDTH = 200;            // 中央暂停菜单按钮宽度
const int PAUSE_BTN_HEIGHT = 40;            // 中央暂停菜单按钮高度
const int PAUSE_BTN_SPACING = 20;           // 中央暂停菜单按钮间距

const int MAIN_UI_TEXT_SIZE = 20;           // UI面板主要文本的字体大小


void InitGraphics() {
    // 游戏区域的宽度和高度
    int gameAreaWidth = WIDTH * BLOCK_SIZE;
    int gameAreaHeight = HEIGHT * BLOCK_SIZE;

    // UI面板的名义宽度 (可以根据内容调整)
    int uiPanelNominalWidth = 250; // 假设UI面板需要这么多宽度

    // 计算窗口总宽度
    int totalWindowWidth = gameAreaWidth + uiPanelNominalWidth + 30; // 30是游戏区和UI间的间距以及UI右侧边距

    // 计算窗口总高度 (例如，比游戏区域高一点，或者能容纳最长的UI)
    // int desiredTotalWindowHeight = gameAreaHeight + 100;
    // 或者，使其至少与游戏区域等高
    int desiredTotalWindowHeight = std::max(gameAreaHeight, 650); // 假设UI最高可能到650像素

    initgraph(totalWindowWidth, desiredTotalWindowHeight);
    setbkmode(TRANSPARENT);
    BeginBatchDraw();
}

void CloseGraphics() {
    EndBatchDraw();
    closegraph();
}

void Draw() {
    cleardevice();
    setbkcolor(BLACK);

    // 1. 绘制网格线
    setlinecolor(RGB(50, 50, 50));
    setlinestyle(PS_DOT, 1);
    for (int i = 1; i < WIDTH; i++) { int x = i * BLOCK_SIZE; line(x, 0, x, HEIGHT * BLOCK_SIZE); }
    for (int j = 1; j < HEIGHT; j++) { int y = j * BLOCK_SIZE; line(0, y, WIDTH * BLOCK_SIZE, y); }

    // 2. 绘制游戏区域边框
    setlinecolor(WHITE);
    rectangle(0, 0, WIDTH * BLOCK_SIZE, HEIGHT * BLOCK_SIZE);

    // 3. 绘制已固定的方块 (gameMap)
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (gameMap[i][j]) {
                setfillcolor(colors[gameMap[i][j]]);
                solidrectangle(j * BLOCK_SIZE, i * BLOCK_SIZE,
                    (j + 1) * BLOCK_SIZE, (i + 1) * BLOCK_SIZE);
            }
        }
    }

    // 4. 绘制活动方块/预览方块 和 它的影子 (Ghost Piece)
    if (gState == GS_Playing || gState == GS_GodMode_PlaceTerrain || gState == GS_GodMode_SelectShape) {
        bool shapeIsNotEmpty = false;
        for (int r = 0; r < 4; ++r) for (int c = 0; c < 4; ++c) if (currentShape[r][c]) { shapeIsNotEmpty = true; break; }
        if (!shapeIsNotEmpty && gState == GS_GodMode_SelectShape && playerSelectedBlocksForAI == 0 && placedTerrainBlocks >= MAX_TERRAIN_BLOCKS) {
            // 特殊情况：刚完成地形放置，进入选择形状，但玩家还未选择第一个块给AI，此时currentShape可能是空的
            // 这种情况下不绘制活动块或影子
        }
        else if (shapeIsNotEmpty) {
            // 4a. 绘制影子 (Ghost Piece)
            if ((gState == GS_Playing && currentMode == MODE_NORMAL) || gState == GS_GodMode_PlaceTerrain) {
                int ghostY = curY;
                while (!CheckCollision(curX, ghostY + 1)) {
                    ghostY++;
                }
                if (ghostY >= curY) {
                    for (int i = 0; i < 4; i++) {
                        for (int j = 0; j < 4; j++) {
                            if (currentShape[i][j]) {
                                int x0_g = (curX + j) * BLOCK_SIZE;
                                int y0_g = (ghostY + i) * BLOCK_SIZE;
                                if (y0_g + BLOCK_SIZE > 0 && y0_g < HEIGHT * BLOCK_SIZE &&
                                    x0_g + BLOCK_SIZE > 0 && x0_g < WIDTH * BLOCK_SIZE) {
                                    COLORREF originalColor = colors[curColor];
                                    BYTE r = GetRValue(originalColor) / 3 + 40;
                                    BYTE g = GetGValue(originalColor) / 3 + 40;
                                    BYTE b = GetBValue(originalColor) / 3 + 40;
                                    setfillcolor(RGB(r, g, b));
                                    solidrectangle(x0_g, y0_g, x0_g + BLOCK_SIZE, y0_g + BLOCK_SIZE);
                                }
                            }
                        }
                    }
                }
            }

            // 4b. 绘制当前活动/预览方块
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (currentShape[i][j]) {
                        int x0 = (curX + j) * BLOCK_SIZE;
                        int y0 = (curY + i) * BLOCK_SIZE;
                        if (x0 < WIDTH * BLOCK_SIZE && x0 + BLOCK_SIZE > 0 &&
                            y0 < HEIGHT * BLOCK_SIZE && y0 + BLOCK_SIZE > 0) {
                            setfillcolor(colors[curColor]);
                            int drawX0 = std::max(0, x0);
                            int drawY0 = std::max(0, y0);
                            int drawX1 = std::min(WIDTH * BLOCK_SIZE, x0 + BLOCK_SIZE);
                            int drawY1 = std::min(HEIGHT * BLOCK_SIZE, y0 + BLOCK_SIZE);
                            if (drawX1 > drawX0 && drawY1 > drawY0) {
                                solidrectangle(drawX0, drawY0, drawX1, drawY1);
                            }
                        }
                    }
                }
            }
        }
    }

    // 5. UI 面板
    int uiPanelX = WIDTH * BLOCK_SIZE + 20;
    int uiPanelY_current = 20;
    settextcolor(WHITE);
    settextstyle(MAIN_UI_TEXT_SIZE, 0, _T("微软雅黑"));
    setbkmode(TRANSPARENT);

    if (gState == GS_Playing) {
        if (currentMode == MODE_NORMAL) {
            outtextxy(uiPanelX, uiPanelY_current, _T("模式: 普通"));
            uiPanelY_current += 30;
            TCHAR scoreText[30];
            _stprintf_s(scoreText, _T("分数: %d"), score);
            outtextxy(uiPanelX, uiPanelY_current, scoreText);
            uiPanelY_current += 30;
        }
        else { // MODE_GOD && GS_Playing (AI 正在放置)
            outtextxy(uiPanelX, uiPanelY_current, _T("模式: 上帝 (AI行动)"));
            uiPanelY_current += 30;
            outtextxy(uiPanelX, uiPanelY_current, _T("AI 正在放置..."));
            uiPanelY_current += 30;
            TCHAR aiTurnText[50];
            _stprintf_s(aiTurnText, _T("AI处理第 %d 块"), playerSelectedBlocksForAI);
            outtextxy(uiPanelX, uiPanelY_current, aiTurnText);
            uiPanelY_current += 30;
        }
    }
    else if (gState == GS_GodMode_SelectShape) {
        outtextxy(uiPanelX, uiPanelY_current, _T("模式: 上帝 (选择)"));
        uiPanelY_current += 30;
        outtextxy(uiPanelX, uiPanelY_current, _T("请选择方块给AI"));
        uiPanelY_current += 30;
        TCHAR playerChoiceCountText[50];
        _stprintf_s(playerChoiceCountText, _T("这将是第 %d 个块"), playerSelectedBlocksForAI + 1);
        outtextxy(uiPanelX, uiPanelY_current, playerChoiceCountText);
        uiPanelY_current += 30;
        // DrawGodModeShapeSelection() 会在 main.cpp 中此 Draw() 调用之后被调用
    }
    else if (gState == GS_GodMode_PlaceTerrain) {
        outtextxy(uiPanelX, uiPanelY_current, _T("模式: 上帝 (设置地形)"));
        uiPanelY_current += 25;
        TCHAR terrainCountText[50];
        _stprintf_s(terrainCountText, _T("放置第 %d / %d 个地形块"), placedTerrainBlocks + 1, MAX_TERRAIN_BLOCKS);
        outtextxy(uiPanelX, uiPanelY_current, terrainCountText);
        uiPanelY_current += 25;
        outtextxy(uiPanelX, uiPanelY_current, _T("1-7选块,方向键移动"));
        uiPanelY_current += 20;
        outtextxy(uiPanelX, uiPanelY_current, _T("R旋转, 空格放置"));
        uiPanelY_current += 25;
        outtextxy(uiPanelX, uiPanelY_current, _T("可选方块预览:"));
        uiPanelY_current += 20;

        int previewStartX = uiPanelX;
        int previewStartY = uiPanelY_current;
        int tempCurrentY_preview = previewStartY;

        int actualPreviewSize = PREVIEW_BLOCK_SIZE_GOD_MODE; // 使用统一的预览大小
        int previewBoxHeight = 4 * actualPreviewSize;
        int previewBoxWidth = 4 * actualPreviewSize;
        int previewPaddingY = UI_PREVIEW_PADDING_Y;

        settextstyle(14, 0, _T("Arial")); // 预览数字用小字体
        int previewNumTextHeight = textheight(_T("1"));
        int previewNumTextWidthEst = textwidth(_T("88")); // 估计数字和可能的间距

        for (int i = 0; i < 7; i++) {
            for (int r_shape = 0; r_shape < 4; r_shape++) {
                for (int c_shape = 0; c_shape < 4; c_shape++) {
                    if (shapes[i][r_shape][c_shape]) {
                        setfillcolor(colors[i+1]);
                        solidrectangle(previewStartX + c_shape * actualPreviewSize,
                            tempCurrentY_preview + r_shape * actualPreviewSize,
                            previewStartX + (c_shape + 1) * actualPreviewSize,
                            tempCurrentY_preview + (r_shape + 1) * actualPreviewSize);
                    }
                }
            }
            TCHAR numStr[3];
            _stprintf_s(numStr, _T("%d"), i + 1);
            outtextxy(previewStartX + previewBoxWidth + 5,
                tempCurrentY_preview + previewBoxHeight / 2 - previewNumTextHeight / 2,
                numStr);
            tempCurrentY_preview += previewBoxHeight + previewPaddingY;

            // 简单的换列逻辑：如果当前列的下一个预览会超出窗口底部（预留暂停按钮空间），则换列
            if (tempCurrentY_preview + previewBoxHeight > getheight() - (PAUSE_BTN_HEIGHT + 30) && i < 6) { // i<6 避免最后一个也尝试换列
                previewStartX += previewBoxWidth + previewNumTextWidthEst + 5; // 移动到下一列
                tempCurrentY_preview = previewStartY; // Y坐标重置到当前列的起始
            }
        }
        settextstyle(MAIN_UI_TEXT_SIZE, 0, _T("微软雅黑")); // 恢复主UI字体
        uiPanelY_current = tempCurrentY_preview; // 更新主UI的Y坐标，如果后续还有内容
    }

    // 绘制UI面板上的暂停按钮
    if (gState == GS_Playing || gState == GS_GodMode_SelectShape || gState == GS_GodMode_PlaceTerrain) {
        int pauseButtonY_ui = getheight() - PAUSE_BTN_HEIGHT - 20; // 放在新窗口的相对底部
        // 如果希望按钮在动态UI内容下方，需要确保 uiPanelY_current 在此之前已正确更新
        // int pauseButtonY_ui = std::max(uiPanelY_current + 10, getheight() - PAUSE_BTN_HEIGHT - 20);
        // pauseButtonY_ui = std::min(pauseButtonY_ui, getheight() - PAUSE_BTN_HEIGHT - 10); // 确保不超出

        RECT localUiPauseButtonRect = {
            uiPanelX, pauseButtonY_ui,
            uiPanelX + PAUSE_BTN_WIDTH, pauseButtonY_ui + PAUSE_BTN_HEIGHT
        };
        setfillcolor(RGB(80, 80, 80));
        solidrectangle(localUiPauseButtonRect.left, localUiPauseButtonRect.top, localUiPauseButtonRect.right, localUiPauseButtonRect.bottom);
        settextcolor(WHITE);
        settextstyle(18, 0, _T("微软雅黑"));
        TCHAR pauseBtnText[] = _T("暂停 (P)");
        int btnTextWidth = textwidth(pauseBtnText);
        int btnTextHeight = textheight(pauseBtnText);
        outtextxy(localUiPauseButtonRect.left + (PAUSE_BTN_WIDTH - btnTextWidth) / 2,
            localUiPauseButtonRect.top + (PAUSE_BTN_HEIGHT - btnTextHeight) / 2,
            pauseBtnText);
        settextstyle(MAIN_UI_TEXT_SIZE, 0, _T("微软雅黑")); // 恢复主UI字体
    }
    FlushBatchDraw();
}

// ---------------------------------------------------------------------------
// DrawMainMenu, DrawGodModeShapeSelection, DrawGameOverScreen, DrawPause 函数
// 这些函数应保持与之前提供的版本一致。
// DrawMainMenu 和 DrawGameOverScreen 是全屏绘制，会调用 cleardevice。
// DrawPause 是叠加绘制，不调用 cleardevice。
// DrawGodModeShapeSelection 也是叠加绘制，在 Draw() 之后由 main.cpp 调用。
// ---------------------------------------------------------------------------

void DrawMainMenu() {
    cleardevice();
    setbkcolor(BLACK);
    int window_width = getwidth();
    int window_height = getheight();
    settextcolor(WHITE);
    settextstyle(48, 0, _T("微软雅黑"));
    TCHAR title[] = _T("俄罗斯方块");
    int textWidth = textwidth(title);
    outtextxy(window_width / 2 - textWidth / 2, window_height / 4, title);
    int buttonWidth = 220;
    int buttonHeight = 50;
    int centerX = window_width / 2;
    RECT normalModeBtnRect = { centerX - buttonWidth / 2, window_height / 2 - buttonHeight - 10,
                           centerX + buttonWidth / 2, window_height / 2 - 10 };
    setfillcolor(DARKGRAY);
    solidrectangle(normalModeBtnRect.left, normalModeBtnRect.top, normalModeBtnRect.right, normalModeBtnRect.bottom);
    settextcolor(WHITE);
    settextstyle(24, 0, _T("微软雅黑"));
    TCHAR normalModeText[] = _T("普通模式 (N)");
    textWidth = textwidth(normalModeText);
    int textHeight = textheight(normalModeText);
    outtextxy(normalModeBtnRect.left + (buttonWidth - textWidth) / 2,
        normalModeBtnRect.top + (buttonHeight - textHeight) / 2,
        normalModeText);
    RECT godModeBtnRect = { centerX - buttonWidth / 2, window_height / 2 + 10,
                        centerX + buttonWidth / 2, window_height / 2 + buttonHeight + 10 };
    setfillcolor(DARKGRAY);
    solidrectangle(godModeBtnRect.left, godModeBtnRect.top, godModeBtnRect.right, godModeBtnRect.bottom);
    TCHAR godModeText[] = _T("上帝模式 (G)");
    textWidth = textwidth(godModeText);
    outtextxy(godModeBtnRect.left + (buttonWidth - textWidth) / 2,
        godModeBtnRect.top + (buttonHeight - textHeight) / 2,
        godModeText);
    FlushBatchDraw();
}

void DrawGodModeShapeSelection() {
    // 此函数在 Draw() 之后被调用，用于在 GS_GodMode_SelectShape 状态下绘制7个可选方块
    // 它应该使用与 GS_GodMode_PlaceTerrain 中预览类似的逻辑，但位置可能不同或更突出
    int uiPanelX_select = WIDTH * BLOCK_SIZE + 20;
    // 起始Y坐标需要与 Draw() 中 GS_GodMode_SelectShape 状态下的文本输出协调
    int startY_select = 20 + 30 + 30 + 30 + 10; // 假设这是 "这将是第 X 个块" 文字下方

    int previewStartX_select = uiPanelX_select;
    int tempCurrentY_select = startY_select;

    int actualPreviewSize_select = PREVIEW_BLOCK_SIZE_GOD_MODE; // 可以用统一的预览大小
    int previewBoxHeight_select = 4 * actualPreviewSize_select;
    int previewBoxWidth_select = 4 * actualPreviewSize_select;
    int previewPadding_Y_select = UI_PREVIEW_PADDING_Y;

    settextstyle(14, 0, _T("Arial"));
    int previewNumTextHeight_select = textheight(_T("1"));
    int previewNumTextWidthEst_select = textwidth(_T("88"));

    for (int i = 0; i < 7; i++) {
        for (int r_shape = 0; r_shape < 4; r_shape++) {
            for (int c_shape = 0; c_shape < 4; c_shape++) {
                if (shapes[i][r_shape][c_shape]) {
                    setfillcolor(colors[i+1]);
                    solidrectangle(previewStartX_select + c_shape * actualPreviewSize_select,
                        tempCurrentY_select + r_shape * actualPreviewSize_select,
                        previewStartX_select + (c_shape + 1) * actualPreviewSize_select,
                        tempCurrentY_select + (r_shape + 1) * actualPreviewSize_select);
                }
            }
        }
        TCHAR numStr[3];
        _stprintf_s(numStr, _T("%d"), i + 1);
        outtextxy(previewStartX_select + previewBoxWidth_select + 5,
            tempCurrentY_select + previewBoxHeight_select / 2 - previewNumTextHeight_select / 2,
            numStr);
        tempCurrentY_select += previewBoxHeight_select + previewPadding_Y_select;

        if (tempCurrentY_select + previewBoxHeight_select > getheight() - (PAUSE_BTN_HEIGHT + 30) && i < 6) {
            previewStartX_select += previewBoxWidth_select + previewNumTextWidthEst_select + 5;
            tempCurrentY_select = startY_select;
        }
    }
    // 通常不需要在这里恢复字体，因为下一帧 Draw() 会重置
    FlushBatchDraw(); // Draw() 的末尾会 Flush
}

void DrawGameOverScreen(const TCHAR* titleMsg, const TCHAR* subtitleMsg) {
    cleardevice();
    setbkcolor(BLACK);
    int window_width = getwidth();
    int window_height = getheight();
    settextcolor(WHITE);
    settextstyle(40, 0, _T("微软雅黑"));
    int textWidth = textwidth(titleMsg);
    outtextxy(window_width / 2 - textWidth / 2, window_height / 3, titleMsg);
    if (subtitleMsg && _tcslen(subtitleMsg) > 0) {
        settextstyle(20, 0, _T("微软雅黑"));
        textWidth = textwidth(subtitleMsg);
        outtextxy(window_width / 2 - textWidth / 2, window_height / 2, subtitleMsg);
    }
    settextstyle(18, 0, _T("微软雅黑"));
    TCHAR continueMsg[] = _T("点击任意处或按Enter返回主菜单");
    textWidth = textwidth(continueMsg);
    outtextxy(window_width / 2 - textWidth / 2, window_height / 2 + 60, continueMsg);
    FlushBatchDraw();
}

void DrawPause() {
    // (这个函数绘制 "已暂停" 标题和 "继续游戏", "返回主菜单", "退出游戏" 三个按钮)
    // (代码与之前提供的一致，确保按钮绘制在游戏区域中央)
    settextstyle(30, 0, _T("微软雅黑"));
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);

    TCHAR pauseTitle[] = _T("已暂停");
    int titleWidth = textwidth(pauseTitle);
    outtextxy((WIDTH * BLOCK_SIZE) / 2 - titleWidth / 2, HEIGHT * BLOCK_SIZE / 3 - 50, pauseTitle);

    int totalButtonHeight = 3 * PAUSE_BTN_HEIGHT + 2 * PAUSE_BTN_SPACING;
    int startY = (HEIGHT * BLOCK_SIZE) / 2 - totalButtonHeight / 2 + 30;
    int centerX = (WIDTH * BLOCK_SIZE) / 2;

    RECT currentBtnRect;

    settextstyle(20, 0, _T("微软雅黑")); // 设置按钮文本样式一次

    currentBtnRect = { centerX - PAUSE_BTN_WIDTH / 2, startY, centerX + PAUSE_BTN_WIDTH / 2, startY + PAUSE_BTN_HEIGHT };
    setfillcolor(DARKGRAY);
    solidrectangle(currentBtnRect.left, currentBtnRect.top, currentBtnRect.right, currentBtnRect.bottom);
    TCHAR continueText[] = _T("继续游戏 (P)");
    int btnTextWidth = textwidth(continueText);
    int btnTextHeight = textheight(continueText); // 只需要获取一次按钮文本高度
    outtextxy(currentBtnRect.left + (PAUSE_BTN_WIDTH - btnTextWidth) / 2, currentBtnRect.top + (PAUSE_BTN_HEIGHT - btnTextHeight) / 2, continueText);

    startY += PAUSE_BTN_HEIGHT + PAUSE_BTN_SPACING;
    currentBtnRect = { centerX - PAUSE_BTN_WIDTH / 2, startY, centerX + PAUSE_BTN_WIDTH / 2, startY + PAUSE_BTN_HEIGHT };
    setfillcolor(DARKGRAY);
    solidrectangle(currentBtnRect.left, currentBtnRect.top, currentBtnRect.right, currentBtnRect.bottom);
    TCHAR mainMenuText[] = _T("返回主菜单 (M)");
    btnTextWidth = textwidth(mainMenuText);
    outtextxy(currentBtnRect.left + (PAUSE_BTN_WIDTH - btnTextWidth) / 2, currentBtnRect.top + (PAUSE_BTN_HEIGHT - btnTextHeight) / 2, mainMenuText);

    startY += PAUSE_BTN_HEIGHT + PAUSE_BTN_SPACING;
    currentBtnRect = { centerX - PAUSE_BTN_WIDTH / 2, startY, centerX + PAUSE_BTN_WIDTH / 2, startY + PAUSE_BTN_HEIGHT };
    setfillcolor(DARKGRAY);
    solidrectangle(currentBtnRect.left, currentBtnRect.top, currentBtnRect.right, currentBtnRect.bottom);
    TCHAR exitText[] = _T("退出游戏 (Esc)");
    btnTextWidth = textwidth(exitText);
    outtextxy(currentBtnRect.left + (PAUSE_BTN_WIDTH - btnTextWidth) / 2, currentBtnRect.top + (PAUSE_BTN_HEIGHT - btnTextHeight) / 2, exitText);

    FlushBatchDraw();
}