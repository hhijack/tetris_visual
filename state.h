// state.h
#pragma once

// 定义游戏板常量
#define WIDTH 10
#define HEIGHT 20
#define BLOCK_SIZE 30 // 请根据你的实际设置调整

// 输入延迟常量 (如果你的 input.h 中有，可以移到这里或保持原样)
#define MOVE_INTERVAL   100
#define ROTATE_INTERVAL 200

const int MAX_TERRAIN_BLOCKS = 5;  // 最大允许放置的地形块

enum EGameState {
    GS_MainMenu,            // 主菜单 (用于模式选择)
    GS_Playing,             // 游戏中 (普通模式或上帝模式AI下落)
    GS_Paused,              // 暂停
    // GS_HighScores,       // 积分榜 (如果暂时不用可以注释掉)
    GS_GodMode_PlaceTerrain, // 上帝模式放置地形
    GS_GodMode_SelectShape, // 上帝模式下，玩家选择方块
    GS_GameOverScreen       // 统一的游戏结束/胜利界面
};

enum EGameMode {
    MODE_NORMAL,
    MODE_GOD
};

// 全局状态变量声明 (定义在 main.cpp 或 game.cpp 中)
extern EGameState gState;
extern EGameMode currentMode;
extern int nextShapeIndex; // 上帝模式选择的下一个方块的索引 (0-6)


const int NORMAL_DROP_SPEED_MS = 500; // 普通模式自动下落间隔(毫秒)
