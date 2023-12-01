# STM32-TetrisGame

#### 介绍
- BUAA研一实时嵌入式系统大作业
- 野火指南者开发板开发
- 完成了μC/OS-III嵌入式实时操作系统以及STemWin嵌入式图形驱动库的移植
- 完成了俄罗斯方块游戏在STM32上的适配
- 上位机利用py完成了串口控制通信

#### 软件架构
- 下位机主要编写代码集中于`/stm32/User/app`目录下：
  - `block_process`以及`terminos_process`为游戏底层，其中：
    - `block_process`提供了LCD屏幕绘制带留白方块的方法
    - `terminos_process`提供了俄罗斯方块共7种构型28种表示的定义，并提供了绘制、清除、取得这些俄罗斯方块的方法
  - `tetris_game`为游戏逻辑层，其中：
    - 首先定义了游戏区域`GameZone`以及两个需要跟踪的俄罗斯方块对象`CurrentT`以及`NextT`
    - 定义了一系列支持游戏逻辑的方法，如“初始化”、“检查下一个运动是否碰撞”、“清除满行”等（都是基于上述三个对象的操作）
    - 此外，还在这里提供了静态GUI的绘制方法
  - `key_code_communication`则是实现了基于串口通信的上位机键码值的传递以及解析功能
    - 利用了操作系统的消息队列功能传递消息到`app`
  - `app`则是游戏应用层，其中：
    - `ForceMoveInformTimer`以及`ForceMoveInformSem`是操作系统提供的软件定时器以及信号量，用于周期性的设置强制向下运动的信号
    - `ExecGame()`运行在`AppTaskGameLogic`任务中，通过调用上述模块提到的方法来完成游戏全流程的逻辑以及渲染工作
- 上位机主要编写代码为`/pc/serial_communication.py`：
  - 主要利用`serial` `numpy` `keyboard`库来通过与下位机的串口通信实现按键的跟踪以及对应键值的发送
     