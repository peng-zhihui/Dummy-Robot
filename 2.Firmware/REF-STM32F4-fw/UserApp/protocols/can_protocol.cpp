#include "common_inc.h"

// Used for response CAN message.
static CAN_TxHeaderTypeDef txHeader =
    {
        .StdId = boardConfig.canNodeId,
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
        if (id == dummy.motorJ1->nodeID)
        {
            float val = *(float*) (data);

            switch (cmd)
            {
                case 0x22:

                    break;
                case 0x23:
                    dummy.motorJ1->UpdateAngleCallback(val);
                    break;
                default:
                    break;
            }

        } else if (id == dummy.motorJ2->nodeID)
        {
            float val = *(float*) (data);

            switch (cmd)
            {
                case 0x22:

                    break;
                case 0x23:
                    dummy.motorJ2->UpdateAngleCallback(val);
                    break;
                default:
                    break;
            }
        } else if (id == dummy.motorJ3->nodeID)
        {
            float val = *(float*) (data);

            switch (cmd)
            {
                case 0x22:

                    break;
                case 0x23:
                    dummy.motorJ3->UpdateAngleCallback(val);
                    break;
                default:
                    break;
            }
        } else if (id == dummy.motorJ4->nodeID)
        {
            float val = *(float*) (data);

            switch (cmd)
            {
                case 0x22:

                    break;
                case 0x23:
                    dummy.motorJ4->UpdateAngleCallback(val);
                    break;
                default:
                    break;
            }
        } else if (id == dummy.motorJ5->nodeID)
        {
            float val = *(float*) (data);

            switch (cmd)
            {
                case 0x22:

                    break;
                case 0x23:
                    dummy.motorJ5->UpdateAngleCallback(val);
                    break;
                default:
                    break;
            }
        } else if (id == dummy.motorJ6->nodeID)
        {
            float val = *(float*) (data);

            switch (cmd)
            {
                case 0x22:

                    break;
                case 0x23:
                    dummy.motorJ6->UpdateAngleCallback(val);
                    break;
                default:
                    break;
            }
        }

        dummy.currentJoints.a[0] = dummy.motorJ1->currentAngle + dummy.initPose.a[0];
        dummy.currentJoints.a[1] = dummy.motorJ2->currentAngle + dummy.initPose.a[1];
        dummy.currentJoints.a[2] = dummy.motorJ3->currentAngle + dummy.initPose.a[2];
        dummy.currentJoints.a[3] = dummy.motorJ4->currentAngle + dummy.initPose.a[3];
        dummy.currentJoints.a[4] = dummy.motorJ5->currentAngle + dummy.initPose.a[4];
        dummy.currentJoints.a[5] = dummy.motorJ6->currentAngle + dummy.initPose.a[5];

    } else if (canCtx->handle->Instance == CAN2)
    {
        /*----------------------- ↓ Add Your CAN2 Packet Protocol Here ↓ ------------------------*/
        if (rxHeader->StdId == (0x100 + canCtx->node_id))
        {
            // Bytes to Float
            float val = *(float*) (data + 4);

            val += 100.0f;

            // Float to Bytes
            auto* b = (unsigned char*) &val;
            for (int i = 4; i < 8; i++)
                data[i] = 0x22;// *(b + i - 4);

            // Send back message
            txHeader.StdId = 0x200;
            txHeader.DLC = 8;

            CanSendMessage(canCtx, data, &txHeader);
        }
    }
    /*----------------------- ↑ Add Your Packet Protocol Here ↑ ------------------------*/
}