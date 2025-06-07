// score.cpp
#include "score.h"    // 包含 ScoreEntry 定义和 gHighScores 声明
// #include <vector>  // score.h 已经包含了
// #include <algorithm> // score.h 已经包含了
// #include <string>  // score.h 已经包含了

// 定义全局高分榜变量
std::vector<ScoreEntry> gHighScores; // <--- 在这里定义它

// 在游戏结束后调用
void AddHighScore(int finalScore) {
    ScoreEntry newEntry;
    newEntry.name = "YOU"; // 直接赋值给 std::string
    newEntry.score = finalScore;

    gHighScores.push_back(newEntry);

    // std::sort 需要知道 ScoreEntry 的比较方式
    std::sort(gHighScores.begin(), gHighScores.end(),
        [](const ScoreEntry& a, const ScoreEntry& b) { // 明确参数类型为 const引用
            return a.score > b.score;
        });

    if (gHighScores.size() > 5) { // 只保留前5名
        gHighScores.resize(5);
    }
    // TODO: 在这里可以添加保存高分到文件的逻辑
}

// (可选) 实现 LoadHighScores, SaveHighScores, DisplayHighScores
// void LoadHighScores() { /* ... */ }
// void SaveHighScores() { /* ... */ }
// void DisplayHighScores() { /* ... */ }