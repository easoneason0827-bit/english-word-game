#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "rlutil.h"

// Map rlutil_* names to C functions in rlutil.h
#ifndef __cplusplus
#define rlutil_hidecursor hidecursor
#define rlutil_showcursor showcursor
#define rlutil_cls cls
#define rlutil_locate locate
#define rlutil_setColor setColor
#define rlutil_msleep msleep
#endif

#define WIDTH 60
#define HEIGHT 25
#define MAX_OBJECTS 20
#define DICTIONARY_SIZE 10

// 單字資料結構
typedef struct {
    char english[20];
    char chinese[20];
} Word;

// 掉落物體結構
typedef struct {
    int x, y;
    int wordIndex; // 指向 dictionary 的索引
    int active;    // 1=存在, 0=空插槽
} FallingObject;

// 題庫
Word dictionary[DICTIONARY_SIZE] = {
    {"apple", "蘋果"}, {"book", "書本"}, {"cat", "貓"}, {"dog", "狗"},
    {"egg", "雞蛋"}, {"fish", "魚"}, {"girl", "女孩"}, {"hat", "帽子"},
    {"ice", "冰塊"}, {"jump", "跳"}
};

// 遊戲狀態
int playerX = WIDTH / 2;
int score = 0;
int lives = 3;
int currentTargetIndex = 0; // 目前要接的目標單字索引
FallingObject objects[MAX_OBJECTS];

// Function prototypes
void initObjects(void);
void spawnObject(void);


int main() {
    srand((unsigned int)time(NULL));
    rlutil_hidecursor();
    rlutil_cls();
    
    initObjects();
    currentTargetIndex = rand() % DICTIONARY_SIZE; // 隨機選一個題目
    int frameCount = 0;
    time_t startTime = time(NULL); // 60 秒時間限制起點

    while (lives > 0) {
        // 計算剩餘時間（60秒限制）
        time_t currentTime = time(NULL);
        int elapsed = (int)difftime(currentTime, startTime);
        int remaining = 60 - elapsed;
        if (remaining <= 0) break; // 時間到結束遊戲

        // --- 1. 輸入處理 (左右移動) ---
        if (kbhit()) {
            int k = getkey(); // rlutil 的 getkey
            if (k == KEY_LEFT || k == 'a') {
                // 加快左右移動速度 (步長 4)
                if (playerX > 4) playerX -= 4;
                else playerX = 2;
            } else if (k == KEY_RIGHT || k == 'd') {
                if (playerX < WIDTH - 4) playerX += 4;
                else playerX = WIDTH - 2;
            } else if (k == KEY_ESCAPE) {
                break;
            }
        }

        // --- 2. 遊戲邏輯 ---
        frameCount++;
        
        // 生成控制 (每 10 幀生成一個)
        if (frameCount % 10 == 0) {
            spawnObject();
        }

        // 移動所有掉落物
        for (int i = 0; i < MAX_OBJECTS; i++) {
            if (objects[i].active) {
                objects[i].y++; // 往下掉
                
                // 碰撞判定 (簡單判定：Y軸到底 且 X軸範圍重疊)
                // 假設單字寬度約 5-8 格，玩家寬度 1 格
                int wordLen = strlen(dictionary[objects[i].wordIndex].english);
                
                // 如果掉到玩家高度 (HEIGHT-2 是地面)
                if (objects[i].y >= HEIGHT - 2) {
                    // 檢查是否接觸玩家 (簡單碰撞盒：玩家位置 +- 單字長度的一半)
                    if (playerX >= objects[i].x - 1 && playerX <= objects[i].x + wordLen) {
                        // **撞到了！**
                        if (objects[i].wordIndex == currentTargetIndex) {
                            // 接到正確答案
                            score += 10;
                            // 換下一題
                            currentTargetIndex = rand() % DICTIONARY_SIZE;
                            // 清除畫面上所有正確答案，避免混淆(可選)
                        } else {
                            // 接到障礙物 (錯誤單字)
                            lives--;
                            rlutil_locate(playerX, objects[i].y);
                            rlutil_setColor(12); // 紅色
                            printf("X"); // 撞擊特效
                            rlutil_msleep(200);
                        }
                        objects[i].active = 0; // 撞到後消失
                    } else if (objects[i].y > HEIGHT - 1) {
                        // 沒接到，掉出畫面
                        objects[i].active = 0;
                    }
                }
            }
        }

        // --- 3. 畫面繪製 ---
        rlutil_cls(); // 清除畫面

        // 畫邊框 (地板)
        rlutil_setColor(7); // 白色
        for(int x=1; x<=WIDTH; x++) {
            rlutil_locate(x, HEIGHT-1); printf("=");
        }

        // 畫 UI 資訊
        rlutil_locate(1, 1); 
        rlutil_setColor(14); // 黃色
        printf("Time: %ds | SCORE: %d  |  LIVES: %d", remaining, score, lives);
        
        rlutil_locate(WIDTH / 2 - 10, 1);
        rlutil_setColor(11); // 青色 (題目)
        // 顯示目前目標 (英文)
        printf("Target: [ %s ]", dictionary[currentTargetIndex].english);

        // 畫掉落物
        for (int i = 0; i < MAX_OBJECTS; i++) {
            if (objects[i].active) {
                rlutil_locate(objects[i].x, objects[i].y);
                
                // 如果是正確答案，顯示綠色，否則紅色(障礙)
                if (objects[i].wordIndex == currentTargetIndex) {
                    rlutil_setColor(10); // 綠色 (目標)
                } else {
                    rlutil_setColor(12); // 紅色 (障礙)
                }
                printf("%s", dictionary[objects[i].wordIndex].english);
            }
        }

        // 畫玩家
        rlutil_locate(playerX, HEIGHT - 2);
        rlutil_setColor(15); // 亮白
        printf("^"); // 玩家角色
        rlutil_locate(playerX-1, HEIGHT - 1);
        printf("---"); // 玩家底座

        rlutil_msleep(100); // 遊戲速度
    }

    // --- 遊戲結束 ---
    rlutil_cls();
    rlutil_locate(WIDTH/2 - 5, HEIGHT/2);
    rlutil_setColor(12);
    printf("GAME OVER");
    rlutil_locate(WIDTH/2 - 6, HEIGHT/2 + 1);
    rlutil_setColor(15);
    printf("Final Score: %d", score);
    rlutil_showcursor();
    getchar(); // 等待按鍵

    return 0;
}

// Initialize object array
void initObjects(void) {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        objects[i].active = 0;
        objects[i].x = 0;
        objects[i].y = 0;
        objects[i].wordIndex = 0;
    }
}

// Spawn a new falling object into the first free slot
void spawnObject(void) {
    int slot = -1;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (!objects[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return; // no free slot

    objects[slot].active = 1;
    objects[slot].y = 1;
    // keep x inside bounds (leave margin)
    objects[slot].x = rand() % (WIDTH - 6) + 3;

    // 30% 機率生成「正確答案」，70% 生成「障礙物」
    if (rand() % 100 < 30) {
        objects[slot].wordIndex = currentTargetIndex;
    } else {
        int wrongIndex;
        do {
            wrongIndex = rand() % DICTIONARY_SIZE;
        } while (wrongIndex == currentTargetIndex);
        objects[slot].wordIndex = wrongIndex;
    }
}