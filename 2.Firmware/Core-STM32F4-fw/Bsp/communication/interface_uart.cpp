#include "common_inc.h"
#include "interface_uart.hpp"
#include "ascii_processor.hpp"
#include "fibre/protocol.hpp"
#include "usart.h"

#define UART_TX_BUFFER_SIZE 64
#define UART_RX_BUFFER_SIZE 64

// DMA open loop continous circular buffer
// 1ms delay periodic, chase DMA ptr around
static uint8_t dma_rx_buffer[2][UART_RX_BUFFER_SIZE];
static uint32_t dma_last_rcv_idx[2];

// FIXME: the stdlib doesn't know about CMSIS threads, so this is just a global variable
// static thread_local uint32_t deadline_ms = 0;

osThreadId_t uartServerTaskHandle;


class UART4Sender : public StreamSink
{
public:
    UART4Sender()
    {
        channelType = CHANNEL_TYPE_UART4;
    }

    int process_bytes(const uint8_t* buffer, size_t length, size_t* processed_bytes) override
    {
        // Loop to ensure all bytes get sent
        while (length)
        {
            size_t chunk = length < UART_TX_BUFFER_SIZE ? length : UART_TX_BUFFER_SIZE;
            // wait for USB interface to become ready
            // TODO: implement ring buffer to get a more continuous stream of data
            // if (osSemaphoreWait(sem_uart_dma, deadline_to_timeout(deadline_ms)) != osOK)
            if (osSemaphoreAcquire(sem_uart4_dma, PROTOCOL_SERVER_TIMEOUT_MS) != osOK)
                return -1;
            // transmit chunk
            memcpy(tx_buf_, buffer, chunk);
            if (HAL_UART_Transmit_DMA(&huart4, tx_buf_, chunk) != HAL_OK)
                return -1;
            buffer += chunk;
            length -= chunk;
            if (processed_bytes)
                *processed_bytes += chunk;
        }
        return 0;
    }

    size_t get_free_space() override
    { return SIZE_MAX; }

private:
    uint8_t tx_buf_[UART_TX_BUFFER_SIZE];
} uart4_stream_output;

class UART5Sender : public StreamSink
{
public:
    UART5Sender()
    {
        channelType = CHANNEL_TYPE_UART5;
    }

    int process_bytes(const uint8_t* buffer, size_t length, size_t* processed_bytes) override
    {
        // Loop to ensure all bytes get sent
        while (length)
        {
            size_t chunk = length < UART_TX_BUFFER_SIZE ? length : UART_TX_BUFFER_SIZE;
            // wait for USB interface to become ready
            // TODO: implement ring buffer to get a more continuous stream of data
            // if (osSemaphoreWait(sem_uart_dma, deadline_to_timeout(deadline_ms)) != osOK)
            if (osSemaphoreAcquire(sem_uart5_dma, PROTOCOL_SERVER_TIMEOUT_MS) != osOK)
                return -1;
            // transmit chunk
            memcpy(tx_buf_, buffer, chunk);
            if (HAL_UART_Transmit_DMA(&huart5, tx_buf_, chunk) != HAL_OK)
                return -1;
            buffer += chunk;
            length -= chunk;
            if (processed_bytes)
                *processed_bytes += chunk;
        }
        return 0;
    }

    size_t get_free_space() override
    { return SIZE_MAX; }

private:
    uint8_t tx_buf_[UART_TX_BUFFER_SIZE];
} uart5_stream_output;

StreamSink* uart4StreamOutputPtr = &uart4_stream_output;
StreamBasedPacketSink uart4_packet_output(uart4_stream_output);
BidirectionalPacketBasedChannel uart4_channel(uart4_packet_output);
StreamToPacketSegmenter uart4_stream_input(uart4_channel);

StreamSink* uart5StreamOutputPtr = &uart5_stream_output;
StreamBasedPacketSink uart5_packet_output(uart5_stream_output);
BidirectionalPacketBasedChannel uart5_channel(uart5_packet_output);
StreamToPacketSegmenter uart5_stream_input(uart5_channel);

static void UartServerTask(void* ctx)
{
    (void) ctx;

    for (;;)
    {
        // Check for UART errors and restart recieve DMA transfer if required
        if (huart4.ErrorCode != HAL_UART_ERROR_NONE)
        {
            HAL_UART_AbortReceive(&huart4);
            HAL_UART_Receive_DMA(&huart4, dma_rx_buffer[0], sizeof(dma_rx_buffer[0]));
        }
        // Fetch the circular buffer "write pointer", where it would write next
        uint32_t new_rcv_idx = UART_RX_BUFFER_SIZE - huart4.hdmarx->Instance->NDTR;

        // deadline_ms = timeout_to_deadline(PROTOCOL_SERVER_TIMEOUT_MS);
        // Process bytes in one or two chunks (two in case there was a wrap)
        if (new_rcv_idx < dma_last_rcv_idx[0])
        {
            uart4_stream_input.process_bytes(dma_rx_buffer[0] + dma_last_rcv_idx[0],
                                             UART_RX_BUFFER_SIZE - dma_last_rcv_idx[0],
                                             nullptr); // TODO: use process_all
            ASCII_protocol_parse_stream(dma_rx_buffer[0] + dma_last_rcv_idx[0],
                                        UART_RX_BUFFER_SIZE - dma_last_rcv_idx[0], uart4_stream_output);
            dma_last_rcv_idx[0] = 0;
        }
        if (new_rcv_idx > dma_last_rcv_idx[0])
        {
            uart4_stream_input.process_bytes(dma_rx_buffer[0] + dma_last_rcv_idx[0],
                                             new_rcv_idx - dma_last_rcv_idx[0],
                                             nullptr); // TODO: use process_all
            ASCII_protocol_parse_stream(dma_rx_buffer[0] + dma_last_rcv_idx[0],
                                        new_rcv_idx - dma_last_rcv_idx[0], uart4_stream_output);
            dma_last_rcv_idx[0] = new_rcv_idx;
        }


        // Check for UART errors and restart recieve DMA transfer if required
        if (huart5.ErrorCode != HAL_UART_ERROR_NONE)
        {
            HAL_UART_AbortReceive(&huart5);
            HAL_UART_Receive_DMA(&huart5, dma_rx_buffer[1], sizeof(dma_rx_buffer[1]));
        }
        // Fetch the circular buffer "write pointer", where it would write next
        new_rcv_idx = UART_RX_BUFFER_SIZE - huart5.hdmarx->Instance->NDTR;

        // deadline_ms = timeout_to_deadline(PROTOCOL_SERVER_TIMEOUT_MS);
        // Process bytes in one or two chunks (two in case there was a wrap)
        if (new_rcv_idx < dma_last_rcv_idx[1])
        {
            uart4_stream_input.process_bytes(dma_rx_buffer[1] + dma_last_rcv_idx[1],
                                             UART_RX_BUFFER_SIZE - dma_last_rcv_idx[1],
                                             nullptr); // TODO: use process_all
            ASCII_protocol_parse_stream(dma_rx_buffer[1] + dma_last_rcv_idx[1],
                                        UART_RX_BUFFER_SIZE - dma_last_rcv_idx[1], uart5_stream_output);
            dma_last_rcv_idx[1] = 0;
        }
        if (new_rcv_idx > dma_last_rcv_idx[1])
        {
            uart4_stream_input.process_bytes(dma_rx_buffer[1] + dma_last_rcv_idx[1],
                                             new_rcv_idx - dma_last_rcv_idx[1],
                                             nullptr); // TODO: use process_all
            ASCII_protocol_parse_stream(dma_rx_buffer[1] + dma_last_rcv_idx[1],
                                        new_rcv_idx - dma_last_rcv_idx[1], uart5_stream_output);
            dma_last_rcv_idx[1] = new_rcv_idx;
        }


        osDelay(1);
    };
}

const osThreadAttr_t uartServerTask_attributes = {
    .name = "UartServerTask",
    .stack_size = 2000,
    .priority = (osPriority_t) osPriorityNormal,
};

void StartUartServer()
{
    // DMA is set up to receive in a circular buffer forever.
    // We don't use interrupts to fetch the data, instead we periodically read
    // data out of the circular buffer into a parse buffer, controlled by a state machine
    HAL_UART_Receive_DMA(&huart4, dma_rx_buffer[0], sizeof(dma_rx_buffer[0]));
    dma_last_rcv_idx[0] = UART_RX_BUFFER_SIZE - huart4.hdmarx->Instance->NDTR;

    HAL_UART_Receive_DMA(&huart5, dma_rx_buffer[1], sizeof(dma_rx_buffer[1]));
    dma_last_rcv_idx[1] = UART_RX_BUFFER_SIZE - huart5.hdmarx->Instance->NDTR;

    // Start UART communication thread
    uartServerTaskHandle = osThreadNew(UartServerTask, nullptr, &uartServerTask_attributes);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
    if (huart->Instance == UART4)
        osSemaphoreRelease(sem_uart4_dma);
    else if (huart->Instance == UART5)
        osSemaphoreRelease(sem_uart5_dma);
}
