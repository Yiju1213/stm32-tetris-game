#include "includes.h"

// 游戏区方块行列数
#define GM_ROW 31
#define GM_COL 16

// 游戏区起始坐标（左上）
#define GAME_ZONE_START_X 75
#define GAME_ZONE_START_Y 5

// 预览区像素坐标（左上 右下）
#define PV_X_START 183
#define PV_Y_START 120
#define PV_X_END 223
#define PV_Y_END 160

void GameOnInterface(int len, int mar);
void GameOverInterface(void);

void InitStartingTetriminos(void);
void SaveCurrentTetriminoToGameZone(void);
void SwitchCurrentTetrimino(void);
void SwitchNextTetrimino(void);
void UpdateNextTetriminoInPreviewArea(void);

void ClearFullRowsAndUpdateGameZone(void);
void UpdateCurrentTetriminoInGameZone(int block_step_x, int block_step_y, int rotate_times);
int CheckIfNextMoveCollide(int block_step_x, int block_step_y, int rotate_times);

void Test_ImplementGameZoneForFullRowClear(void);
void RefreshGameZoneGUI(void);
void RefreshGameZoneData(void);
int CheckIfGameOver(void);
void UpdateScoresGUI(void);

