/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_TIM1_Init(void);
void MX_TIM2_Init(void);
void MX_TIM3_Init(void);
void MX_TIM4_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* USER CODE BEGIN Prototypes */
/********** HwElec **********/
/********** HwElec **********/
/********** HwElec **********/
void REIN_TIM_HwElec_Init(void);       //TIM初始�??(HwElec)

/********** MT6816Base **********/
/********** MT6816Base **********/
/********** MT6816Base **********/
void REIN_TIM_MT6816_ABZ_Init(void);		//TIM初始�??(MT6816_ABZ)
void REIN_TIM_MT6816_PWM_Init(void);		//TIM初始�??(MT6816_PWM)

/********** SIGNAL **********/
/********** SIGNAL **********/
/********** SIGNAL **********/
void REIN_TIM_SIGNAL_COUNT_Init(void);		//TIM初始�??(SIGNAL_COUNT)
void REIN_TIM_SIGNAL_COUNT_DeInit(void);	//TIM清理(SIGNAL_COUNT)
void REIN_TIM_SIGNAL_PWM_Init(void);			//TIM初始�??(SIGNAL_PWM)
void REIN_TIM_SIGNAL_PWM_DeInit(void);		//TIM清理(SIGNAL_PWM)

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
