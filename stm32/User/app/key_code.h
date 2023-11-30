#include "includes.h"

typedef enum _PACKET_FSM {
    HEAD = 1,
    DATA,
    CRC_,
    TAIL,
    SEND
}PACKET_FSM;

typedef enum _KEY_CODE {
    KEY_ERR = 0,
    KEY_UP = 72,
    KEY_DOWN = 80,
    KEY_LEFT = 75,
    KEY_RIGHT = 77,
    KEY_SPACE = 57
}KEY_CODE;

void InitKeyMsgQueue(void);
uint8_t ReceiveKeyCodeFromSerialQueue(void);
void TransformKeyCodeToTetrisMove(uint8_t key_code, int* x, int* y, int* rotate);
