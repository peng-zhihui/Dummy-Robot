/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED1_Pin GPIO_PIN_13
#define LED1_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_14
#define LED2_GPIO_Port GPIOC
#define POWER_U_Pin GPIO_PIN_0
#define POWER_U_GPIO_Port GPIOA
#define DRV_TEMP_Pin GPIO_PIN_1
#define DRV_TEMP_GPIO_Port GPIOA
#define HW_ELEC_BM_Pin GPIO_PIN_2
#define HW_ELEC_BM_GPIO_Port GPIOA
#define HW_ELEC_BP_Pin GPIO_PIN_3
#define HW_ELEC_BP_GPIO_Port GPIOA
#define HW_ELEC_AM_Pin GPIO_PIN_4
#define HW_ELEC_AM_GPIO_Port GPIOA
#define HW_ELEC_AP_Pin GPIO_PIN_5
#define HW_ELEC_AP_GPIO_Port GPIOA
#define SIGNAL_COUNT_DIR_Pin GPIO_PIN_7
#define SIGNAL_COUNT_DIR_GPIO_Port GPIOA
#define SIGNAL_COUNT_DIR_EXTI_IRQn EXTI9_5_IRQn
#define SIGNAL_COUNT_EN_Pin GPIO_PIN_0
#define SIGNAL_COUNT_EN_GPIO_Port GPIOB
#define SIGNAL_ALERT_Pin GPIO_PIN_1
#define SIGNAL_ALERT_GPIO_Port GPIOB
#define BUTTON2_Pin GPIO_PIN_2
#define BUTTON2_GPIO_Port GPIOB
#define HW_ELEC_BPWM_Pin GPIO_PIN_10
#define HW_ELEC_BPWM_GPIO_Port GPIOB
#define HW_ELEC_APWM_Pin GPIO_PIN_11
#define HW_ELEC_APWM_GPIO_Port GPIOB
#define BUTTON1_Pin GPIO_PIN_12
#define BUTTON1_GPIO_Port GPIOB
#define SPI1_CS_Pin GPIO_PIN_15
#define SPI1_CS_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
