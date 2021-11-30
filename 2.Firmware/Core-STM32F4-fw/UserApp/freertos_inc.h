#ifndef __FREERTOS_H
#define __FREERTOS_H

#ifdef __cplusplus
extern "C" {
#endif

// List of semaphores
extern osSemaphoreId sem_usb_irq;
extern osSemaphoreId sem_uart4_dma;
extern osSemaphoreId sem_uart5_dma;
extern osSemaphoreId sem_usb_rx;
extern osSemaphoreId sem_usb_tx;
extern osSemaphoreId sem_can1_tx;
extern osSemaphoreId sem_can2_tx;

// List of Tasks
/*--------------------------------- System Tasks -------------------------------------*/
extern osThreadId_t defaultTaskHandle;      // Usage: 2000 Bytes stack
extern osThreadId_t commTaskHandle;         // Usage: 48000 Bytes stack
extern osThreadId_t usbIrqTaskHandle;       // Usage: 512  Bytes stack
extern osThreadId_t usbServerTaskHandle;    // Usage: 2048 Bytes stack
extern osThreadId_t uartServerTaskHandle;   // Usage: 2048 Bytes stack

/*---------------------------------- User Tasks --------------------------------------*/
extern osThreadId_t oledTaskHandle;         // Usage: 4000 Bytes stack
extern osThreadId_t controlLoopFixUpdateHandle;  // Usage: 4000 Bytes stack

/*---------------- 60K (used) / 64K (for FreeRTOS on ccram) ------------------*/


#ifdef __cplusplus
}
#endif

#endif