/**
 ******************************************************************************
 * @file    main.c
 * @author  fire
 * @version V1.0
 * @date    2013-xx-xx
 * @brief   ����led
 ******************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:Ұ�� iSO STM32 ������
 * ��̳    :http://www.firebbs.cn
 * �Ա�    :https://fire-stm32.taobao.com
 *
 ******************************************************************************
 */

#include "includes.h"

static OS_TCB AppTaskStartTCB;                           // ����������ƿ�
static CPU_STK AppTaskStartStk[APP_TASK_START_STK_SIZE]; // ���������ջ

static OS_TCB AppTaskGameRenderTCB;
static CPU_STK AppTaskGameRenderStk[APP_TASK_GAMERENDER_STK_SIZE];

static OS_TCB  AppTaskGameLogicTCB;
static CPU_STK AppTaskGameLogicStk[APP_TASK_GAMELOGIC_STK_SIZE];

static void AppTaskStart(void *p_arg);
static void AppTaskCreate(void);
static void AppTaskGameRender(void *p_arg);

/**
 * @brief  ������
 * @param  ��
 * @retval ��
 */
int main(void)
{
  OS_ERR err;

  CPU_IntDis(); // BSP_IntDisAll();                           /* Disable all interrupts.
  /* ��ʼ��"uC/OS-III"�ں� */
  OSInit(&err);

  /*��������*/
  OSTaskCreate((OS_TCB *)&AppTaskStartTCB,                 // ������ƿ�ָ��
               (CPU_CHAR *)"App Task Start",               // ��������
               (OS_TASK_PTR)AppTaskStart,                  // �������ָ��
               (void *)0,                                  // ���ݸ�����Ĳ���parg
               (OS_PRIO)APP_TASK_START_PRIO,               // �������ȼ�
               (CPU_STK *)&AppTaskStartStk[0],             // �����ջ����ַ
               (CPU_STK_SIZE)APP_TASK_START_STK_SIZE / 10, // ��ջʣ�ྯ����
               (CPU_STK_SIZE)APP_TASK_START_STK_SIZE,      // ��ջ��С
               (OS_MSG_QTY)5u,                             // �ɽ��յ������Ϣ������
               (OS_TICK)0u,                                // ʱ��Ƭ��תʱ��
               (void *)0,                                  // ������ƿ���չ��Ϣ
               (OS_OPT)(OS_OPT_TASK_STK_CHK |
                        OS_OPT_TASK_STK_CLR), // ����ѡ��
               (OS_ERR *)&err);               // ����ֵ

  /* ����������ϵͳ������Ȩ����uC/OS-II */
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

  /* �弶��ʼ�� */
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
*	�� �� ��: AppTaskGameRender
*	����˵��: ����˹����GUI������Ⱦ�����߳�
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
  �� �� ����7
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
*	�� �� ��: AppTaskGameLogic
*	����˵��: ����˹����GUI��Ϸ�߼������߳�
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
  �� �� ����6
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
*	�� �� ��: AppTaskCreate
*	����˵��: ����Ӧ������
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
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
