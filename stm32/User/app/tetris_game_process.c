#include "tetris_game_process.h"
#include "tetriminos_process.h"
#include "beihang.h"
#include "block_process.h"

void MapFromGameZoneToPixel(int block_x, int block_y, int *pixel_x, int *pixel_y);
int Scores = 0;

/********************* 静态GUI框架 ↓ *************************/

void DrawBeihang(void);
void DrawPreviewArea(void);
void DrawScoreArea(void);
void DrawFinishLine(void);

void GameOnInterface(int len, int mar)
{
  int p_x, p_y;
  // 清除当前版面
  GUI_Clear();
  // 边框
  for (p_x = 0; p_x < SCREEN_WIDTH; p_x += len)
  {
    DrawBlock(len, mar, p_x, 0, GUI_WHITE);
    DrawBlock(len, mar, p_x, (SCREEN_HEIGHT / len - 1) * len, GUI_WHITE);
    if (p_x > 160)
    {
      DrawBlock(len, mar, p_x, 70, GUI_WHITE);
      DrawBlock(len, mar, p_x, 195, GUI_WHITE);
    }
  }
  for (p_y = 0; p_y < SCREEN_HEIGHT; p_y += len)
  {
    DrawBlock(len, mar, 0, p_y, GUI_WHITE);
    DrawBlock(len, mar, (SCREEN_WIDTH / len - 1) * len, p_y, GUI_WHITE);
    DrawBlock(len, mar, (160 / len + 1) * len, p_y, GUI_WHITE);
  }
  // 右侧上方校徽区
  DrawBeihang();
  // 右侧中间预览区
  DrawPreviewArea();
  // 右侧下方得分区
  DrawScoreArea();
  // 画出结束线
  DrawFinishLine();
}

void DrawBeihang(void)
{
  // 创建GUI_BITMAP结构
  GUI_BITMAP bitmap;

  // 设置图像大小和颜色格式
  bitmap.XSize = 65;
  bitmap.YSize = 65;
  bitmap.pData = (U8 *)gImage_beihang; // 图片数据
  bitmap.pPal = NULL;                  // 如果有调色板，可以设置
  bitmap.BitsPerPixel = 24;
  bitmap.pMethods = GUI_DRAW_BMP24;
  bitmap.BytesPerLine = 3 * 65; // 16位图像，每像素占3字节
  GUI_DrawBitmap(&bitmap, 170, 5);
}

void DrawScoreArea()
{
  // 在区域下方绘制标题 "Tips Area"
  GUI_SetFont(&GUI_Font16B_ASCII);
  GUI_DispStringHCenterAt("Score", 202, 215);
  GUI_DispStringHCenterAt("0", 202, 255);
}

void DrawPreviewArea()
{
  // 在区域下方绘制标题 "Tips Area"
  GUI_SetFont(&GUI_Font16B_ASCII);
  GUI_DispStringHCenterAt("Preview", 202, 90);
}

void DrawFinishLine()
{
  int p_x = 0, p_y = 4;
  MapFromGameZoneToPixel(0, 4, &p_x, &p_y);

  GUI_SetLineStyle(GUI_LS_DASHDOTDOT);
  GUI_SetColor(GUI_WHITE);
  GUI_SetPenSize(3);
  GUI_DrawHLine(p_y, p_x, p_x + GM_COL * STD_BLOCK_LEN);
}

void GameOverInterface(void)
{
  char str[20];
  //
  GUI_Clear();
  //
  GUI_SetTextAlign(GUI_TA_HCENTER);
  GUI_SetFont(&GUI_Font24B_ASCII);
  GUI_SetColor(GUI_RED);
  GUI_DispStringAt("Game Over", 120, 100);
  //
  GUI_SetTextAlign(GUI_TA_HCENTER);
  GUI_SetFont(&GUI_Font16B_ASCII);
  GUI_SetColor(GUI_YELLOW);
  sprintf(str, "Your Score is: %d", Scores);
  GUI_DispStringAt(str, 120, 160);
  //
  GUI_SetTextAlign(GUI_TA_HCENTER);
  GUI_SetFont(&GUI_Font16B_ASCII);
  GUI_SetColor(GUI_WHITE);
  GUI_DispStringAt("Press \"space\" to Re-play", 120, 200);
  //
  Scores = 0;
}
/********************* 静态GUI框架 ↑ *************************/

/********************* 动态逻辑 ↓ *************************/
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
void ClearOneRowData(int row)
{
  int block_x, block_y = row;

  for (block_x = 0; block_x < GM_COL; ++block_x)
  {
    GameZone.data[block_y][block_x] = 0;
  }
}

// 前台高亮一行方块
void HighlightOneRowGUI(int row)
{
  int block_x, block_y;
  int pixel_x, pixel_y;
  block_y = row;
  for (block_x = 0; block_x < GM_COL; ++block_x)
  {
    MapFromGameZoneToPixel(block_x, block_y, &pixel_x, &pixel_y);
    DrawBlock(STD_BLOCK_LEN, STD_BLOCK_MAR, pixel_x, pixel_y, GUI_WHITE);
  }
}

// 前台渲染要被清除的行数据
void HighlightRowsToBeClearGUI(uint8_t *row_arr)
{
  int block_y;
  int stay_ms = 100;
  GUI_Delay(stay_ms);
  for (block_y = 0; block_y < GM_ROW; ++block_y)
  {
    if (row_arr[block_y] == 1)
    {
      HighlightOneRowGUI(block_y);
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
  GUI_ClearRect(PV_X_START, PV_Y_START, PV_X_END, PV_Y_END);
  DrawTetriminos(NextT.body, NextT.len, NextT.mar, NextT.p_x, NextT.p_y, NextT.color);
}

void UpdateCurrentTetriminoInGameZone(int block_step_x, int block_step_y, int rotate_times)
{
  ClearTetriminos(CurrentT.body, CurrentT.len, CurrentT.mar, CurrentT.p_x, CurrentT.p_y);
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
  NextT.p_x = PV_X_START; // 在预览区生成
  NextT.p_y = PV_Y_START;
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
    {
      ClearOneRowData(block_y);
      Scores += 10;
    }
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
        ClearOneRowData(block_y);
      }
    }
  }

  HighlightRowsToBeClearGUI(full_flag);
  RefreshGameZoneGUI();
}

// 刷新整个游戏界面
void RefreshGameZoneGUI(void)
{
  int map_px, map_py, y, x;
  GUI_ClearRect(5, 5, 165, 315);
  GUI_Delay(100);
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
  DrawFinishLine(); // GUI_ClearRect会清除掉
  GUI_Delay(100);
}

// 清除当前游戏区域数据（GameOver下调用）
void RefreshGameZoneData(void)
{
  int y, x;
  for (y = 0; y < GM_ROW; ++y)
    for (x = 0; x < GM_COL; ++x)
      GameZone.data[y][x] = 0;
}

int CheckIfGameOver(void)
{
  int block_y, block_x;
  MapFromPixelToGameZone(CurrentT.p_x, CurrentT.p_y, &block_x, &block_y);

  // 检查高四行（FinishLine上方）是否已经被占据
  for (block_y = 0; block_y < 4; ++block_y)
  {
    for (block_x = 0; block_x < GM_COL; ++block_x)
    {
      if (GameZone.data[block_y][block_x] == 1)
        return 1;
    }
  }

  return 0;
}

// 测试行清除功能 不用管
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

  RefreshGameZoneGUI();
}

void UpdateScoresGUI(void)
{
  char str[10];
  GUI_SetFont(&GUI_Font16B_ASCII);
  GUI_SetColor(GUI_WHITE);
  sprintf(str, "%d", Scores);
  GUI_DispStringHCenterAt(str, 202, 255);
} 
