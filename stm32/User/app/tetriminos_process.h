#include "includes.h"

#define TETRIS_BLKS 4
#define TETRIS_SHAPES 7
#define TETRIS_FORMS 4

struct _TetrisForm
{
    uint8_t space[TETRIS_BLKS][TETRIS_BLKS];
};

// 初始化方块信息
void InitTetrisForm(void);

// 不同形状Tetriminos颜色设置
GUI_COLOR GetTetrisColor(int shape_index);

// 画出俄罗斯方块的形状块
int DrawTetriminos(struct _TetrisForm* body, int len, int mar, int pixel_x, int pixel_y, GUI_COLOR color);

// 清除俄罗斯方块的形状块
int ClearTetriminos(struct _TetrisForm* body, int len, int pixel_x, int pixel_y);

// 获得某一个特定的形状块
struct _TetrisForm* GetTetriminos(int shape, int form);

// 获取当前形状块的下一个旋转形态
struct _TetrisForm* GetNextTetriminosForm(int shape, int* form, int rotate_times);

//
void Test_ShowAllTetriminosInPreviewArea(void);
