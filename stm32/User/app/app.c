#include "includes.h"
#include "block_process.h"
#include "tetriminos_process.h"
#include "tetris_game_process.h"
#include "key_code_communication.h"

OS_TMR ForceMoveInformTimer;
OS_SEM ForceMoveInformSem;

void InitForceMoveInformer(int ms);
void ForceMoveInformTimerCallBack(void *p_tmr, void *p_arg);
int TryReceiveForceMoveSignal(void);

// 定时器中断回调 不做具体操作 所以只做简洁通知操作
void ForceMoveInformTimerCallBack(void *p_tmr, void *p_arg)
{
  OS_ERR err;

  OSSemSet(&ForceMoveInformSem, 1, &err);
}

void ExecGame(void)
{
  OS_ERR err;
  uint8_t key_code;
  int x_move, y_move, rotate, game_over = 1 ;
  // 初始化
  InitTetrisForm();
  InitKeyMsgQueue();
  InitForceMoveInformer(250); // TODO:设置为变量
  InitStartingTetriminos();

  while (1)
  {
    GameOnInterface(BORDER_BLOCK_LEN, BORDER_BLOCK_MAR);
    UpdateNextTetriminoInPreviewArea();

    while (!game_over)
    {
      // 先处理用户输入 只需要判断用户输入是否能执行 而不判断当前方块是否停止运动
      key_code = ReceiveKeyCodeFromSerialQueue();
      TransformKeyCodeToTetrisMove(key_code, &x_move, &y_move, &rotate);
      if (CheckIfNextMoveCollide(x_move, y_move, rotate) == 0) // 能执行用户输入
        UpdateCurrentTetriminoInGameZone(x_move, y_move, rotate);

      // 尝试获取来自定时器的定时下移方块通知
      // 如获取到 则执行简单的下移 但还需要判断当前方块是否停止运动
      if (TryReceiveForceMoveSignal())
      {
        if (CheckIfNextMoveCollide(0, 1, 0) == 0) // 尝试下移
          UpdateCurrentTetriminoInGameZone(0, 1, 0);
        else // 无法再下移 则中止当前方块运动 转换到下一个方块
        {
          // 游戏区更新
          SaveCurrentTetriminoToGameZone();
          ClearFullRowsAndUpdateGameZone();
          SwitchCurrentTetrimino();
          SwitchNextTetrimino();
          UpdateNextTetriminoInPreviewArea();
          UpdateScoresGUI();
          // 判断是否结束
          if (CheckIfGameOver())
            game_over = 1;
        }
      }
      OSTimeDlyHMSM(0, 0, 0, 1000 / 33, OS_OPT_TIME_HMSM_STRICT, &err); // 30hz
    }

    //
    RefreshGameZoneData();
    GameOverInterface();
    while (game_over)
    {
      key_code = ReceiveKeyCodeFromSerialQueue();
      if (key_code == KEY_SPACE)
      {
        game_over = 0;
        break;
      }
      OSTimeDlyHMSM(0, 0, 0, 1000 / 100, OS_OPT_TIME_HMSM_STRICT, &err); // 10hz
    }

    OSTimeDlyHMSM(0, 0, 0, 1000 / 100, OS_OPT_TIME_HMSM_STRICT, &err); // 10hz
  }
}

int TryReceiveForceMoveSignal(void)
{
  OS_ERR err;
  OSSemPend(&ForceMoveInformSem, 0, OS_OPT_PEND_NON_BLOCKING, 0, &err);
  if (err == OS_ERR_NONE)
    return 1;
  else
    return 0;
}

void InitForceMoveInformer(int ms)
{
  OS_ERR err;
  int timer_period = 1000 / OS_CFG_TMR_TASK_RATE_HZ;
  // 创建定时器
  OSTmrCreate(&ForceMoveInformTimer,
              "ForceMoveInformTimer",
              0,
              ms / timer_period,
              OS_OPT_TMR_PERIODIC,          // 选项
              ForceMoveInformTimerCallBack, // 回调函数
              NULL,                         // 传递给回调函数的参数
              &err);

  // 创建信号量
  OSSemCreate(&ForceMoveInformSem, "ForceMoveInformSem", 0, &err);

  // 启动定时器
  OSTmrStart(&ForceMoveInformTimer, &err);
}
