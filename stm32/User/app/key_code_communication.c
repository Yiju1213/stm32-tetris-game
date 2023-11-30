#include "key_code_communication.h"

#define KMQ_SIZE 10
OS_Q KeyMsgQueue;

void InitKeyMsgQueue(void)
{
  OS_ERR err;
  OSQCreate(&KeyMsgQueue, "KeyMsgQueue", KMQ_SIZE, &err);
}

void USART1_IRQHandler(void)
{
    static PACKET_FSM fsm = HEAD;
    static uint8_t data;
    static uint8_t crc = 0x00;
    uint8_t head = 0xf0;
    uint8_t tail = 0x0f;
    OS_ERR err;
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        // 读取接收到的数据
        uint8_t recv = USART_ReceiveData(USART1);
        
        // 处理接收到的数据
        switch (fsm)
        {
        case HEAD:
            if (recv == head)
                fsm = DATA;
            break;
        case DATA:
            data = recv;
            fsm = CRC_;
            break;
        case CRC_:
            crc = recv;
            fsm = TAIL;
            break;
        case TAIL:
            if (recv == tail)
                fsm = SEND;
            else
                fsm = HEAD; // tail接收错误 这个Packet直接作废
            break;

        default:
            break;
        }

        if (fsm == SEND)
        {
            // 照理说需要CRC校验 此处省略
            // crc_check()
            // 把接收到的data放进Queue
            OSQPost(&KeyMsgQueue, &data, sizeof(uint8_t), OS_OPT_POST_FIFO, &err);
            fsm = HEAD;
        }

        // 清除中断标志位
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

uint8_t ReceiveKeyCodeFromSerialQueue(void)
{
    OS_ERR err;
    OS_MSG_SIZE msg_size;
    void* p_msg;
    p_msg = OSQPend(&KeyMsgQueue, 0, OS_OPT_PEND_NON_BLOCKING, &msg_size, NULL, &err);
    if (err == OS_ERR_NONE)
    {
#if (DEBUG)
        USART_SendData(USART1, 0xF0);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
        USART_SendData(USART1, *(uint8_t*)p_msg);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
        USART_SendData(USART1, 0x00);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
        USART_SendData(USART1, 0x0F);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
#endif
        return *((uint8_t *)p_msg);
    }
    else
        return KEY_ERR;
}

void TransformKeyCodeToTetrisMove(uint8_t key_code, int* x, int* y, int* rotate)
{
    switch (key_code)
    {
    case KEY_ERR:
        (*x) = 0;
        (*y) = 0;
        (*rotate) = 0;
        break;
    case KEY_DOWN:
        (*x) = 0;
        (*y) = 1;
        (*rotate) = 0;
        break;
    case KEY_LEFT:
        (*x) = -1;
        (*y) = 0;
        (*rotate) = 0;
        break;
    case KEY_RIGHT:
        (*x) = 1;
        (*y) = 0;
        (*rotate) = 0;
        break;
    case KEY_SPACE:
        (*x) = 0;
        (*y) = 0;
        (*rotate) = 1;
        break;
    
    default:
        (*x) = 0;
        (*y) = 0;
        (*rotate) = 0;
        break;
    }
}
