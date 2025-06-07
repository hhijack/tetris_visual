// score.h
#pragma once
#include <string>
#include <vector>
#include <algorithm> // 添加 algorithm 以便其他文件如果需要 sort (虽然这里主要给 score.cpp 用)

struct ScoreEntry {
    std::string name; // 使用 std::string 更灵活
    int score;
    ScoreEntry() : name(""), score(0) {} // 默认构造函数
};

// 声明全局高分榜变量 (定义在 score.cpp 中)
extern std::vector<ScoreEntry> gHighScores;

// 声明函数
void AddHighScore(int finalScore);
// void LoadHighScores(); // (可选)
// void SaveHighScores(); // (可选)
// void DisplayHighScores(); // (可选)