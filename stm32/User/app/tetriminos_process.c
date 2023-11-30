#include "tetriminos_process.h"
#include "block_process.h"

struct _TetrisForm TetrisForm[TETRIS_SHAPES][TETRIS_FORMS]; // 用于存储7种基本形状方块的各自的4种形态的信息，共28种

// 初始化方块信息
void InitTetrisForm(void)
{
  int i, j;
  int temp[4][4];
  int shape;
  int form;
  // “T”形
  for (i = 0; i <= 2; i++)
    TetrisForm[0][0].space[1][i] = 1;
  TetrisForm[0][0].space[2][1] = 1;

  // “L”形
  for (i = 1; i <= 3; i++)
    TetrisForm[1][0].space[i][1] = 1;
  TetrisForm[1][0].space[3][2] = 1;

  // “J”形
  for (i = 1; i <= 3; i++)
    TetrisForm[2][0].space[i][2] = 1;
  TetrisForm[2][0].space[3][1] = 1;

  for (i = 0; i <= 1; i++)
  {
    // “Z”形
    TetrisForm[3][0].space[1][i] = 1;
    TetrisForm[3][0].space[2][i + 1] = 1;
    // “S”形
    TetrisForm[4][0].space[1][i + 1] = 1;
    TetrisForm[4][0].space[2][i] = 1;
    // “O”形
    TetrisForm[5][0].space[1][i + 1] = 1;
    TetrisForm[5][0].space[2][i + 1] = 1;
  }

  // “I”形
  for (i = 0; i <= 3; i++)
    TetrisForm[6][0].space[i][1] = 1;

  for (shape = 0; shape < 7; shape++) // 7种形状
  {
    for (form = 0; form < 3; form++) // 4种形态（已经有了一种，这里每个还需增加3种）
    {
      // 获取第form种形态
      for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
          temp[i][j] = TetrisForm[shape][form].space[i][j];
      // 将第form种形态顺时针旋转，得到第form+1种形态
      for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
          TetrisForm[shape][form + 1].space[i][j] = temp[3 - j][i];
    }
  }
}

// 不同形状Tetriminos颜色设置
GUI_COLOR GetTetrisColor(int shape_index)
{
  switch (shape_index)
  {
  case 0:
    return GUI_BLUE; // “T”形方块设置为蓝色
  case 1:
    return GUI_GREEN; // “L”形方块设置为绿色
  case 2:
    return GUI_RED; // “J”形方块设置为红色
  case 3:
    return GUI_YELLOW; // “Z”形方块设置为黄色
  case 4:
    return GUI_BROWN; // “S”形方块设置为灰色
  case 5:
    return GUI_ORANGE; // “O”形方块设置为橘色
  case 6:
    return GUI_CYAN; // “I”形方块设置为蓝绿色
  default:
    return GUI_WHITE; // 其他默认设置为白色
  }
}

int DrawTetriminos(struct _TetrisForm* body, int len, int mar, int pixel_x, int pixel_y, GUI_COLOR color)
{
  int x, y;
  
  for (y = 0; y < 4; ++y)
  {
    for (x = 0; x < 4; ++x)
    {
      // printf("%d ", body->space[y][x]);
      if (body->space[y][x] == 1)
        DrawBlock(len, mar, pixel_x + x * len, pixel_y + y * len, color);
    }
    // printf("\n");
  }
  return 0;
}

int ClearTetriminos(struct _TetrisForm* body, int len, int pixel_x, int pixel_y)
{
  int x, y;
  for (y = 0; y < 4; ++y)
  {
    for (x = 0; x < 4; ++x)
    {
      if (body->space[y][x] == 1)
        ClearBlock(len, pixel_x + x * len, pixel_y + y * len);
    }
  }
  return 0;
}

struct _TetrisForm* GetTetriminos(int shape, int form)
{
  if ((shape >= 0 && shape < TETRIS_SHAPES) && (form >= 0 && form < TETRIS_FORMS)) 
    return &TetrisForm[shape][form];
  else
  {
    printf("shape: %d or form: %d error!", shape, form);
    return NULL;
  }
}

// 获取当前形状块的下一个旋转形态
struct _TetrisForm* GetNextTetriminosForm(int shape, int* form, int rotate_times)
{
  if ((shape >= 0 && shape < TETRIS_SHAPES) && (*form >= 0 && *form < TETRIS_FORMS)) 
  {
    *form = ((*form) + rotate_times) % TETRIS_FORMS;
    return &TetrisForm[shape][*form];
  }
  else
  {
    printf("shape: %d or form: %d error!", shape, *form);
    return NULL;
  }
}

// 
void Test_ShowAllTetriminosInPreviewArea(void)
{
  int shape, form;
  for (shape = 0; shape < TETRIS_SHAPES; ++shape)
  {
    for (form = 0; form < TETRIS_FORMS; ++form)
    {
      struct _TetrisForm *body = GetTetriminos(shape, form);
      GUI_COLOR color = GetTetrisColor(shape);
      DrawTetriminos(body, STD_BLOCK_LEN, STD_BLOCK_MAR, 183, 100, color);
      GUI_Delay(500);
      ClearTetriminos(body, STD_BLOCK_LEN, 183, 100);
    }
  }
  // struct _TetrisForm *body = GetTetriminos(0, 0);
  // GUI_COLOR color = GetTetrisColor(0);
  // DrawTetriminos(body, STD_BLOCK_LEN, STD_BLOCK_MAR, 183, 100, color);
}
