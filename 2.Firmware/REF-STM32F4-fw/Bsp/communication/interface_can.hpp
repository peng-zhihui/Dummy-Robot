#ifndef __INTERFACE_CAN_HPP
#define __INTERFACE_CAN_HPP

#include "fibre/protocol.hpp"
#include <stm32f4xx_hal.h>
#include <cmsis_os.h>

struct CAN_context
{
    CAN_HandleTypeDef* handle = nullptr;
    uint8_t node_id = 0;
    uint64_t serial_number = 0;

    uint32_t node_ids_in_use_0[4]; // 128 bits (indicate if a node ID was in use up to 1 second ago)
    uint32_t node_ids_in_use_1[4]; // 128 bits (indicats if a node ID was in use 1-2 seconds ago)

    uint32_t last_heartbeat_mailbox = 0;
    uint32_t tx_msg_cnt = 0;

    uint8_t node_id_rng_state = 0;

    osSemaphoreId_t sem_send_heartbeat;

    // count occurrence various callbacks
    uint32_t TxMailboxCompleteCallbackCnt = 0;
    uint32_t TxMailboxAbortCallbackCnt = 0;
    int RxFifo0MsgPendingCallbackCnt = 0;
    int RxFifo0FullCallbackCnt = 0;
    int RxFifo1MsgPendingCallbackCnt = 0;
    int RxFifo1FullCallbackCnt = 0;
    int SleepCallbackCnt = 0;
    int WakeUpFromRxMsgCallbackCnt = 0;
    int ErrorCallbackCnt = 0;

    uint32_t received_msg_cnt = 0;
    uint32_t received_ack = 0;
    uint32_t unexpected_errors = 0;
    uint32_t unhandled_messages = 0;
};

struct CAN_context* get_can_ctx(CAN_HandleTypeDef* hcan);
bool StartCanServer(CAN_TypeDef* hcan);
void CanSendMessage(CAN_context* canCtx, uint8_t* txData, CAN_TxHeaderTypeDef* txHeader);
void OnCanMessage(CAN_context* canCtx, CAN_RxHeaderTypeDef* rxHeader, uint8_t* data);

#endif // __INTERFACE_CAN_HPP
