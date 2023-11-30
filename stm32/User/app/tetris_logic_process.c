#include "tetris_logic_process.h"
#include "tetriminos_process.h"
#include "block_process.h"

// 游戏区域（用于记录静态的方块）
struct _GameZone
{
  uint8_t data[GM_ROW][GM_COL]; // 用于标记指定位置是否有方块（1为有，0为无）
  int color[GM_ROW][GM_COL];    // 用于记录指定位置的方块颜色编码
} GameZone;

// 当前运动的俄罗斯方块 以及下一个要运动的俄罗斯方块
struct _RenderedTetrimino
{
  struct _TetrisForm *body; // 指针
  int shape;
  int form;
  GUI_COLOR color;
  int len;
  int mar;
  int p_x; // 左上角x坐标
  int p_y; // 左上角y坐标
} CurrentT, NextT;

void PickRandomTetrimino(int *shape, int *form)
{
  *shape = rand() % TETRIS_SHAPES;
  *form = rand() % TETRIS_FORMS;
}

// 从像素坐标系到游戏区域内的方块坐标系的映射
void MapFromPixelToGameZone(int pixel_x, int pixel_y, int *block_x, int *block_y)
{
  *block_x = (pixel_x - BORDER_BLOCK_LEN) / STD_BLOCK_LEN;
  *block_y = (pixel_y - BORDER_BLOCK_LEN) / STD_BLOCK_LEN;
}

void MapFromGameZoneToPixel(int block_x, int block_y, int *pixel_x, int *pixel_y)
{
  *pixel_x = block_x * STD_BLOCK_LEN + BORDER_BLOCK_LEN;
  *pixel_y = block_y * STD_BLOCK_LEN + BORDER_BLOCK_LEN;
}

// 后台删除一行的数据
void ClearOneRow(int row)
{
  int block_x, block_y = row;
  // int pixel_x, pixel_y;

  for (block_x = 0; block_x < GM_COL; ++block_x)
  {
    GameZone.data[block_y][block_x] = 0;
    // MapFromGameZoneToPixel(block_x, block_y, &pixel_x, &pixel_y);
    // ClearBlock(CurrentT.len, pixel_x, pixel_y);
  }
}

// 后台转移一行数据 前台渲染一行数据
void HighlightRowsBlocks(uint8_t *row_arr)
{
  int block_x, block_y;
  int pixel_x, pixel_y;
  int stay_ms = 500;

  GUI_Delay(stay_ms);
  for (block_y = 0; block_y < GM_ROW; ++ block_y)
  {
    if (row_arr[block_y] == 1)
    {
      for (block_x = 0; block_x < GM_COL; ++block_x)
      {
        MapFromGameZoneToPixel(block_x, block_y, &pixel_x, &pixel_y);
        DrawBlock(STD_BLOCK_LEN, STD_BLOCK_MAR, pixel_x, pixel_y, GUI_WHITE);
      }
    }
  }
  GUI_Delay(stay_ms);
}

void InitStartingTetriminos(void)
{
  SwitchNextTetrimino();
  SwitchCurrentTetrimino();
  SwitchNextTetrimino();
}

void UpdateNextTetriminoInPreviewArea()
{
  GUI_ClearRect(173, 90, 233, 150);
  DrawTetriminos(NextT.body, NextT.len, NextT.mar, NextT.p_x, NextT.p_y, NextT.color);
}

void UpdateCurrentTetriminoInGameZone(int block_step_x, int block_step_y, int rotate_times)
{
  ClearTetriminos(CurrentT.body, CurrentT.len, CurrentT.p_x, CurrentT.p_y);
  CurrentT.p_x += block_step_x * CurrentT.len;
  CurrentT.p_y += block_step_y * CurrentT.len;

  if (rotate_times > 0) // 有旋转
    CurrentT.body = GetNextTetriminosForm(CurrentT.shape, &(CurrentT.form), rotate_times);

  DrawTetriminos(CurrentT.body, CurrentT.len, CurrentT.mar, CurrentT.p_x, CurrentT.p_y, CurrentT.color);
}

int CheckIfNextMoveCollide(int block_step_x, int block_step_y, int rotate_times)
{
  int x, y;
  int block_x, block_y;
  int tmp_form = CurrentT.form;
  struct _TetrisForm *tmp_body;

  // 把没有运动的情况当作是碰撞 这样的结果就是不会进行运动
  if (block_step_x == 0 && block_step_y == 0 && rotate_times == 0)
    return -1;

  // 获取旋转后的body 注意创建了临时变量避免CurrentT被修改
  // 因为对于4x4矩阵块左上角坐标p_x p_y没变 所以计算GameZone的位置仍然可以用CurrentT的变量计算
  tmp_body = GetNextTetriminosForm(CurrentT.shape, &tmp_form, rotate_times); // 避免CurrentT.form以及CurrentT.body被修改
  MapFromPixelToGameZone(CurrentT.p_x, CurrentT.p_y, &block_x, &block_y);

  // 将tmp_body有方块的地址tmp_body->space[y - block_y][x - block_x]
  // 投影到GameZone中运动后的地址后
  //  1. 检查是否超出边界
  //  2. 检查该地址GameZone.data[y + block_step_y][x + block_step_x]是否已经有方块占据
  for (y = block_y; y < block_y + TETRIS_BLKS; ++y)
  {
    for (x = block_x; x < block_x + TETRIS_BLKS; ++x)
    {
      if (tmp_body->space[y - block_y][x - block_x] == 1)
      {
        if (y + block_step_y >= GM_ROW || x + block_step_x >= GM_COL) // 超出右边界及下边界
          return -1;
        if (y + block_step_y < 0 || x + block_step_x < 0) // 超出上边界及左边界
          return -1;
        if (GameZone.data[y + block_step_y][x + block_step_x] == 1) // 运动后地址已经被占据
          return -1;
      }
    }
  }

  return 0;
}

// 保存当前CurrentT到GameZone (可以通过先全局消除再刷新来检查)
void SaveCurrentTetriminoToGameZone(void)
{
  int x, y;
  int block_x, block_y;
  MapFromPixelToGameZone(CurrentT.p_x, CurrentT.p_y, &block_x, &block_y);
  for (y = block_y; y < block_y + TETRIS_BLKS; ++y)
  {
    for (x = block_x; x < block_x + TETRIS_BLKS; ++x)
    {
      if (CurrentT.body->space[y - block_y][x - block_x] == 1)
      {
        GameZone.data[y][x] = 1;
        GameZone.color[y][x] = CurrentT.color;
      }
    }
  }
#if (DEBUG)
  // RefreshGameZone();
#endif
}

// 用NextT替换CurrentT
void SwitchCurrentTetrimino(void)
{
  CurrentT.body = NextT.body;
  CurrentT.shape = NextT.shape;
  CurrentT.form = NextT.form;
  CurrentT.color = NextT.color;
  CurrentT.len = NextT.len;
  CurrentT.mar = NextT.mar;
  CurrentT.p_x = GAME_ZONE_START_X; // 在游戏区的中间生成
  CurrentT.p_y = GAME_ZONE_START_Y;
}

// 更换新的预览俄罗斯方块类型（随机选取）
void SwitchNextTetrimino(void)
{
  PickRandomTetrimino(&NextT.shape, &NextT.form);
  NextT.body = GetTetriminos(NextT.shape, NextT.form);
  NextT.color = GetTetrisColor(NextT.shape);
  NextT.len = STD_BLOCK_LEN;
  NextT.mar = STD_BLOCK_MAR;
  NextT.p_x = PREVIEW_RENDER_X; // 在预览区生成
  NextT.p_y = PREVIEW_RENDER_Y;
}

// 清除满行并更新游戏区域
void ClearFullRowsAndUpdateGameZone(void)
{
  uint8_t full_flag[GM_ROW];
  int block_y, block_x; 
  int cnt_rows = 0;
  // 1. 从底部按行寻找 找到了直接删除数据 
  for (block_y = GM_ROW - 1; block_y >= 0; --block_y)
  {
    full_flag[block_y] = 1;
    // 寻找当前行
    for (block_x = GM_COL - 1; block_x >= 0; --block_x)
    {
      if (GameZone.data[block_y][block_x] == 0) // 有一个是空的
      {
        full_flag[block_y] = 0;
        break;
      }
    }
    // 如果行满 直接在后台上清除
    if (full_flag[block_y] == 1)
      ClearOneRow(block_y);
  }
  // 2. 下坠操作(以行为单位向下移动一个单位)
  for (block_y = GM_ROW - 1; block_y >= 0; --block_y)
  {
    if (full_flag[block_y] == 1) // 行满 并且已经被清除了 则此时该行为空 也不需要下移
    {
      ++cnt_rows;
      continue;
    }
    else // 需要下移 并且下移单位由cnt_rows决定
    {
      if (cnt_rows > 0)
      {
        Mem_Copy(&(GameZone.data[block_y + cnt_rows][0]), &(GameZone.data[block_y][0]), GM_COL * sizeof(uint8_t));
        Mem_Copy(&(GameZone.color[block_y + cnt_rows][0]), &(GameZone.color[block_y][0]), GM_COL * sizeof(int));
        ClearOneRow(block_y);
      }
    }
  }

  HighlightRowsBlocks(full_flag);
  RefreshGameZone();
}

// 测试行清除功能
void Test_ImplementGameZoneForFullRowClear(void)
{
  int block_x, test_row, rand_empty_x;
  test_row = GM_ROW - rand() % 8 - 2;
  rand_empty_x = rand() % GM_COL;
  for (block_x = GM_COL - 1; block_x >= 0; --block_x)
  {
    if (GameZone.data[test_row][block_x] == 0 && block_x != rand_empty_x) // 随机行除rand_empty_x补1
    {
      GameZone.data[test_row][block_x] = 1;
      GameZone.color[test_row][block_x] = GUI_MAGENTA;
    }
    if (GameZone.data[GM_ROW - 1][block_x] == 0 && block_x != rand_empty_x) // 最后一行除rand_empty_x补1
    {
      GameZone.data[GM_ROW - 1][block_x] = 1;
      GameZone.color[GM_ROW - 1][block_x] = GUI_MAGENTA;
    }
  }

  RefreshGameZone();
}

void RefreshGameZone(void)
{
  int map_px, map_py, y, x;
  GUI_ClearRect(5, 5, 165, 315);
  GUI_Delay(500);
  for (y = 0; y < GM_ROW; ++y)
  {
    for (x = 0; x < GM_COL; ++x)
    {
      if (GameZone.data[y][x] == 1)
      {
        MapFromGameZoneToPixel(x, y, &map_px, &map_py);
        DrawBlock(CurrentT.len, CurrentT.mar, map_px, map_py, GameZone.color[y][x]);
      }
    }
  }
  GUI_Delay(500);
}
