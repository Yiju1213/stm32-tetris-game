#include "includes.h"
#include "tetriminos_process.h"
#include "block_process.h"
#include "tetris_logic_process.h"
#include "key_code.h"

OS_TMR RenderInformTimer;
OS_SEM RenderInformSem;

void InitInterface(int len, int mar);
void InitRenderInformer(int ms);
void RenderInformTimerCallBack(void *p_tmr, void *p_arg);

// 定时器中断回调 不做具体操作 所以只做简洁通知操作
void RenderInformTimerCallBack(void *p_tmr, void *p_arg)
{
  OS_ERR err;
  
  OSSemSet(&RenderInformSem, 1, &err);
}

// AppTaskGameRender 线程的任务函数
void ExecGameRender(void)
{
  OS_ERR err;
  uint8_t key_code;
  int x_move, y_move, rotate;
  // 初始化
  InitInterface(BORDER_BLOCK_LEN, BORDER_BLOCK_MAR);
  InitTetrisForm();
  InitStartingTetriminos();
  // Test_DrawBlocksOnWholeScreen(GUI_BLUE);
  // Test_ShowAllTetriminosInPreviewArea();
  InitKeyMsgQueue();
  InitRenderInformer(250); // TODO:设置为变量

  UpdateNextTetriminoInPreviewArea();
  while (1)
  {
    // 先处理用户输入 只需要判断用户输入是否能执行 而不判断当前方块是否停止运动
    key_code = ReceiveKeyCodeFromSerialQueue();
    TransformKeyCodeToTetrisMove(key_code, &x_move, &y_move, &rotate);
    if (CheckIfNextMoveCollide(x_move, y_move, rotate) == 0)
    {
      UpdateCurrentTetriminoInGameZone(x_move, y_move, rotate);
    }

    // 尝试获取来自定时器的定时下移方块通知
    // 如获取到 则执行简单的下移 但还需要判断当前方块是否停止运动
    OSSemPend(&RenderInformSem, 0, OS_OPT_PEND_NON_BLOCKING, 0, &err);
    if (err == OS_ERR_NONE) // 获取成功
    {
      if (CheckIfNextMoveCollide(0, 1, 0) == 0) // 尝试下移
      {
        UpdateCurrentTetriminoInGameZone(0, 1, 0);
      }
      else // 无法再下移 则中止当前方块运动 转换到下一个方块
      {
        SaveCurrentTetriminoToGameZone();

        // Test_ImplementGameZoneForFullRowClear();
        ClearFullRowsAndUpdateGameZone();

        SwitchCurrentTetrimino();
        SwitchNextTetrimino();
        UpdateNextTetriminoInPreviewArea();
      }
    }
    else  // 获取失败
    {
    }

    OSTimeDlyHMSM(0, 0, 0, 1000 / 30, OS_OPT_TIME_HMSM_STRICT, &err);
  }
}

void ExecGameLogic(void)
{
}

void InitRenderInformer(int ms)
{
  OS_ERR err;
  int timer_period = 1000 / OS_CFG_TMR_TASK_RATE_HZ;
  // 创建定时器
  OSTmrCreate(&RenderInformTimer,
              "RenderInformTimer",
              0,
              ms / timer_period,
              OS_OPT_TMR_PERIODIC,       // 选项
              RenderInformTimerCallBack, // 回调函数
              NULL,                      // 传递给回调函数的参数
              &err);

  // 创建信号量
  OSSemCreate(&RenderInformSem, "RenderInformSem", 0, &err);

  // 启动定时器
  OSTmrStart(&RenderInformTimer, &err);
}



/********* Interface **********/

void DrawTipsArea(void);
void DrawPreviewArea(void);

void InitInterface(int len, int mar)
{
  // 边框
  int p_x, p_y;
  for (p_x = 0; p_x < SCREEN_WIDTH; p_x += len)
  {
    DrawBlock(len, mar, p_x, 0, GUI_WHITE);
    DrawBlock(len, mar, p_x, (SCREEN_HEIGHT / len - 1) * len, GUI_WHITE);
    if (p_x > 160)
    {
      DrawBlock(len, mar, p_x, (SCREEN_HEIGHT / len / 2 - 1) * len, GUI_WHITE);
    }
  }
  for (p_y = 0; p_y < SCREEN_HEIGHT; p_y += len)
  {
    DrawBlock(len, mar, 0, p_y, GUI_WHITE);
    DrawBlock(len, mar, (SCREEN_WIDTH / len - 1) * len, p_y, GUI_WHITE);
    DrawBlock(len, mar, (160 / len + 1) * len, p_y, GUI_WHITE);
  }
  // 右下角提示区
  DrawTipsArea();
  // 右上角预览区
  DrawPreviewArea();
}

void DrawTipsArea()
{
  int left_start = 175;
  // 在区域下方绘制标题 "Tips Area"
  GUI_SetFont(&GUI_Font13B_ASCII);
  GUI_DispStringHCenterAt("Tips Area", 202, 170);
  // 在区域内按左对齐输出选项
  GUI_SetFont(&GUI_Font10_ASCII);
  GUI_DispStringAt("move left", left_start, 190);
  GUI_DispStringAt("move right", left_start, 205);
  GUI_DispStringAt("speed up", left_start, 220);
  GUI_DispStringAt("spin", left_start, 235);
}

void DrawPreviewArea()
{
  // 在区域下方绘制标题 "Tips Area"
  GUI_SetFont(&GUI_Font13B_ASCII);
  GUI_DispStringHCenterAt("Preview Area", 202, 10);
}
