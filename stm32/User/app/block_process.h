#include "includes.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

#define STD_BLOCK_LEN 10
#define STD_BLOCK_MAR 1

#define PV_BLOCK_SIZE 5

#define BORDER_BLOCK_LEN 5
#define BORDER_BLOCK_MAR 1

// 计算坐标映射
void CalculateBlockCoordinates(int len, int i, int j, int *x, int *y);

// 绘制方块
void DrawBlock(int len, int mar, int pixel_x, int pixel_y, GUI_COLOR color);

// 清除方块
void ClearBlock(int len, int pixel_x, int pixel_y);

// 在屏幕上绘制所有方块
void Test_DrawBlocksOnWholeScreen(GUI_COLOR color);
