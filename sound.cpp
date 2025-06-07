//sound.cpp
#include "sound.h"  
#include <tchar.h>  // 添加此头文件以定义 _T 宏  

void PlayBGM() {  
    mciSendString(_T("open \"bgm.mp3\" alias BGM"), NULL, 0, NULL);  
    mciSendString(_T("play BGM repeat"), NULL, 0, NULL);  // 循环播放  
    mciSendString(_T("setaudio BGM volume to 200"), NULL, 0, NULL);  // 0-1000
}  

void StopBGM() {  
    mciSendString(_T("close BGM"), NULL, 0, NULL);  
}
