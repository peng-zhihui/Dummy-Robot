#include "common_inc.h"


// Used for response CAN message.
static CAN_TxHeaderTypeDef txHeader =
    {
        .StdId = 0,
        .ExtId = 0,
        .IDE = CAN_ID_STD,
        .RTR = CAN_RTR_DATA,
        .DLC = 8,
        .TransmitGlobalTime = DISABLE
    };

extern DummyRobot dummy;

void OnCanMessage(CAN_context* canCtx, CAN_RxHeaderTypeDef* rxHeader, uint8_t* data)
{
    // Common CAN message callback, uses ID 32~0x7FF.
    if (canCtx->handle->Instance == CAN1)
    {
        uint8_t id = rxHeader->StdId >> 7; // 4Bits ID & 7Bits Msg
        uint8_t cmd = rxHeader->StdId & 0x7F; // 4Bits ID & 7Bits Msg

        /*----------------------- ↓ Add Your CAN1 Packet Protocol Here ↓ ------------------------*/
        switch (cmd)
        {
            case 0x23:
                dummy.motorJ[id]->UpdateAngleCallback(*(float*) (data), data[4]);
                break;
            default:
                break;
        }

        dummy.UpdateJointAnglesCallback();

    } else if (canCtx->handle->Instance == CAN2)
    {
        /*----------------------- ↓ Add Your CAN2 Packet Protocol Here ↓ ------------------------*/
    }
    /*----------------------- ↑ Add Your Packet Protocol Here ↑ ------------------------*/
}