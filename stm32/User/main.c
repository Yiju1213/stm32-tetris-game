/**
 ******************************************************************************
 * @file    main.c
 * @author  fire
 * @version V1.0
 * @date    2013-xx-xx
 * @brief   测试led
 ******************************************************************************
 * @attention
 *
 * 实验平台:野火 iSO STM32 开发板
 * 论坛    :http://www.firebbs.cn
 * 淘宝    :https://fire-stm32.taobao.com
 *
 ******************************************************************************
 */

#include "includes.h"

static OS_TCB AppTaskStartTCB;                           // 定义任务控制块
static CPU_STK AppTaskStartStk[APP_TASK_START_STK_SIZE]; // 定义任务堆栈

static OS_TCB AppTaskGameRenderTCB;
static CPU_STK AppTaskGameRenderStk[APP_TASK_GAMERENDER_STK_SIZE];

static OS_TCB  AppTaskGameLogicTCB;
static CPU_STK AppTaskGameLogicStk[APP_TASK_GAMELOGIC_STK_SIZE];

static void AppTaskStart(void *p_arg);
static void AppTaskCreate(void);
static void AppTaskGameRender(void *p_arg);

/**
 * @brief  主函数
 * @param  无
 * @retval 无
 */
int main(void)
{
  OS_ERR err;

  CPU_IntDis(); // BSP_IntDisAll();                           /* Disable all interrupts.
  /* 初始化"uC/OS-III"内核 */
  OSInit(&err);

  /*创建任务*/
  OSTaskCreate((OS_TCB *)&AppTaskStartTCB,                 // 任务控制块指针
               (CPU_CHAR *)"App Task Start",               // 任务名称
               (OS_TASK_PTR)AppTaskStart,                  // 任务代码指针
               (void *)0,                                  // 传递给任务的参数parg
               (OS_PRIO)APP_TASK_START_PRIO,               // 任务优先级
               (CPU_STK *)&AppTaskStartStk[0],             // 任务堆栈基地址
               (CPU_STK_SIZE)APP_TASK_START_STK_SIZE / 10, // 堆栈剩余警戒线
               (CPU_STK_SIZE)APP_TASK_START_STK_SIZE,      // 堆栈大小
               (OS_MSG_QTY)5u,                             // 可接收的最大消息队列数
               (OS_TICK)0u,                                // 时间片轮转时间
               (void *)0,                                  // 任务控制块扩展信息
               (OS_OPT)(OS_OPT_TASK_STK_CHK |
                        OS_OPT_TASK_STK_CLR), // 任务选项
               (OS_ERR *)&err);               // 返回值

  /* 启动多任务系统，控制权交给uC/OS-II */
  OSStart(&err);
}

/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static void AppTaskStart(void *p_arg)
{
  OS_ERR err;

  (void)p_arg;

  /* 板级初始化 */
  BSP_Init(); /* Initialize BSP functions                             */
  CPU_Init();
  BSP_Tick_Init();

  Mem_Init(); /* Initialize Memory Management Module                  */

#if OS_CFG_STAT_TASK_EN > 0u
  OSStatTaskCPUUsageInit(&err); /* Compute CPU capacity with no task running            */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
  CPU_IntDisMeasMaxCurReset();
#endif

  APP_TRACE_INFO(("Creating Application Tasks...\n\r"));
  AppTaskCreate(); /* Create Application Tasks                             */

  /*Delete task*/
  OSTaskDel(&AppTaskStartTCB, &err);
  // while (DEF_TRUE)
  // { /* Task body, always written as an infinite loop.       */
  //   printf("hello");
  //   LED1_TOGGLE;
  //   OSTimeDlyHMSM(0, 0, 1, 0,
  //                 OS_OPT_TIME_HMSM_STRICT,
  //                 &err);
  // }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskGameRender
*	功能说明: 俄罗斯方块GUI界面渲染任务线程
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
  优 先 级：7
*********************************************************************************************************
*/
static void AppTaskGameRender(void *p_arg)
{
  (void)p_arg;
  LCD_BK_EN;
  ExecGameRender();
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskGameLogic
*	功能说明: 俄罗斯方块GUI游戏逻辑任务线程
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
  优 先 级：6
*********************************************************************************************************
*/
static void AppTaskGameLogic(void *p_arg)
{
  (void)p_arg;
  LCD_BK_EN;
  ExecGameLogic();
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskCreate(void)
{
  OS_ERR err;
  //
  OSTaskCreate((OS_TCB *)&AppTaskGameRenderTCB,
               (CPU_CHAR *)"Game Render Task",
               (OS_TASK_PTR)AppTaskGameRender,
               (void *)0,
               (OS_PRIO)APP_TASK_GAMERENDER_PRIO,
               (CPU_STK *)&AppTaskGameRenderStk[0],
               (CPU_STK_SIZE)APP_TASK_GAMERENDER_STK_SIZE / 10,
               (CPU_STK_SIZE)APP_TASK_GAMERENDER_STK_SIZE,
               (OS_MSG_QTY)0,
               (OS_TICK)0,
               (void *)0,
               (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               (OS_ERR *)&err);
  OSTaskCreate((OS_TCB *)&AppTaskGameLogicTCB,
               (CPU_CHAR *)"Game Logic Task",
               (OS_TASK_PTR)AppTaskGameLogic,
               (void *)0,
               (OS_PRIO)APP_TASK_GAMELOGIC_PRIO,
               (CPU_STK *)&AppTaskGameLogicStk[0],
               (CPU_STK_SIZE)APP_TASK_GAMELOGIC_STK_SIZE / 10,
               (CPU_STK_SIZE)APP_TASK_GAMELOGIC_STK_SIZE,
               (OS_MSG_QTY)0,
               (OS_TICK)0,
               (void *)0,
               (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               (OS_ERR *)&err);             
}
/*********************************************END OF FILE**********************/
