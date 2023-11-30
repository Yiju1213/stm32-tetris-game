#include "includes.h"

#define GM_ROW 31
#define GM_COL 16

#define PREVIEW_RENDER_X 183
#define PREVIEW_RENDER_Y 100

#define GAME_ZONE_START_X 5
#define GAME_ZONE_START_Y 5

void InitStartingTetriminos(void);
void SaveCurrentTetriminoToGameZone(void);
void SwitchCurrentTetrimino(void);
void SwitchNextTetrimino(void);
void UpdateNextTetriminoInPreviewArea(void);

void ClearFullRowsAndUpdateGameZone(void);
void UpdateCurrentTetriminoInGameZone(int block_step_x, int block_step_y, int rotate_times);
int CheckIfNextMoveCollide(int block_step_x, int block_step_y, int rotate_times);

void Test_ImplementGameZoneForFullRowClear(void);
void RefreshGameZone(void);

