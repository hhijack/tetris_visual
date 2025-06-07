// graphics.h
#pragma once
#include <graphics.h> // EasyX (需要 MOUSEMSG 等)
// #include "state.h" // 如果需要 WIDTH, HEIGHT 等常量，确保已包含

void InitGraphics();
void CloseGraphics();
void Draw();                // 主绘制函数
// void DrawBackground();   // 如果背景绘制复杂，可以单独一个函数
void DrawPause();           // 绘制暂停界面
void DrawMainMenu();        // 绘制主菜单/模式选择界面
void DrawGodModeShapeSelection(); // 绘制上帝模式的方块选择界面
void DrawGameOverScreen(const TCHAR* title, const TCHAR* subtitle); // 游戏结束/胜利界面

extern const int PREVIEW_BLOCK_SIZE_GOD_MODE; // 声明
extern const int UI_PREVIEW_PADDING_Y;       // 上帝模式下右侧UI面板中预览块之间的垂直间距

extern const int PAUSE_BTN_WIDTH;  // 中央暂停菜单按钮宽度
extern const int PAUSE_BTN_HEIGHT;   // 中央暂停菜单按钮高度
extern const int PAUSE_BTN_SPACING;     // 中央暂停菜单按钮间距

extern const int MAIN_UI_TEXT_SIZE;    // UI面板主要文本的字体大小