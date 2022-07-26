/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "common_inc.h"
#include "communication.hpp"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

// List of semaphores
osSemaphoreId sem_usb_irq;
osSemaphoreId sem_uart4_dma;
osSemaphoreId sem_uart5_dma;
osSemaphoreId sem_usb_rx;
osSemaphoreId sem_usb_tx;
osSemaphoreId sem_can1_tx;
osSemaphoreId sem_can2_tx;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 2000,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */


/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
    // Init usb irq binary semaphore, and start with no tokens by removing the starting one.
    osSemaphoreDef(sem_usb_irq);
    sem_usb_irq = osSemaphoreNew(1, 0, osSemaphore(sem_usb_irq));

    // Create a semaphore for UART DMA and remove a token
    osSemaphoreDef(sem_uart4_dma);
    sem_uart4_dma = osSemaphoreNew(1, 1, osSemaphore(sem_uart4_dma));
    osSemaphoreDef(sem_uart5_dma);
    sem_uart5_dma = osSemaphoreNew(1, 1, osSemaphore(sem_uart5_dma));

    // Create a semaphore for USB RX, and start with no tokens by removing the starting one.
    osSemaphoreDef(sem_usb_rx);
    sem_usb_rx = osSemaphoreNew(1, 0, osSemaphore(sem_usb_rx));

    // Create a semaphore for USB TX
    osSemaphoreDef(sem_usb_tx);
    sem_usb_tx = osSemaphoreNew(1, 1, osSemaphore(sem_usb_tx));

    // Create a semaphore for CAN TX
    osSemaphoreDef(sem_can1_tx);
    sem_can1_tx = osSemaphoreNew(1, 1, osSemaphore(sem_can1_tx));
    osSemaphoreDef(sem_can2_tx);
    sem_can2_tx = osSemaphoreNew(1, 1, osSemaphore(sem_can2_tx));

  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */

  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
    // This Task must run before MX_USB_DEVICE_Init(), so have to put it here.
    const osThreadAttr_t usbIrqTask_attributes = {
        .name = "usbIrqTask",
        .stack_size = 500,
        .priority = (osPriority_t) osPriorityAboveNormal,
    };
    usbIrqTaskHandle = osThreadNew(UsbDeferredInterruptTask, NULL, &usbIrqTask_attributes);

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */

    // Invoke cpp-version main().
    Main();

    vTaskDelete(defaultTaskHandle);
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
