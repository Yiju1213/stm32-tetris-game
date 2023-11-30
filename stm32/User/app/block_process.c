#include "includes.h"
#include "block_process.h"

/* 1. 方块抽象方法 */

// 计算坐标映射
void CalculateBlockCoordinates(int len, int i, int j, int *x, int *y)
{
	*x = i * len;
	*y = j * len;
}

// 绘制方块
void DrawBlock(int len, int mar, int pixel_x, int pixel_y, GUI_COLOR color)
{
	GUI_SetColor(color);
	GUI_FillRect(pixel_x + mar, pixel_y + mar, pixel_x + len - mar - 1, pixel_y + len - mar - 1);
}

// 清除方块
void ClearBlock(int len, int pixel_x, int pixel_y)
{
	GUI_ClearRect(pixel_x, pixel_y, pixel_x + len - 1, pixel_y + len - 1);
}


// 在屏幕上绘制所有方块
void Test_DrawBlocksOnWholeScreen(GUI_COLOR color)
{
	int p_x, p_y;
	for (p_x = 0; p_x < SCREEN_WIDTH; p_x += STD_BLOCK_LEN)
		for (p_y = 0; p_y < SCREEN_HEIGHT; p_y += STD_BLOCK_LEN)
			DrawBlock(STD_BLOCK_LEN, STD_BLOCK_MAR, p_x, p_y, color); 
}
