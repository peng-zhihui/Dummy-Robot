/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   This file contains all the function prototypes for
  *          the gpio.c file
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
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */

/********** Button **********/
/********** Button **********/
/********** Button **********/
void REIN_GPIO_Button_Init(void);  	 //GPIO初始�??(Button)

/********** HwElec **********/
/********** HwElec **********/
/********** HwElec **********/
void REIN_GPIO_HwElec_Init(void);   	//GPIO初始�??(HwElec)

/********** MT6816Base **********/
/********** MT6816Base **********/
/********** MT6816Base **********/
void REIN_GPIO_MT6816_ABZ_Init(void);  //GPIO初始�??(MT6816_ABZ)
void REIN_GPIO_MT6816_SPI_Init(void);  //GPIO初始�??(MT6916_SPI)

/********** Modbus **********/
/********** Modbus **********/
/********** Modbus **********/
void REIN_GPIO_Modbus_Init(void);			//GPIO初始�??(Modbus)

/********** OLED **********/
/********** OLED **********/
/********** OLED **********/
void REIN_GPIO_OLED_Init(void);			 //GPIO初始�??(OLED)

/********** SIGNAL **********/
/********** SIGNAL **********/
/********** SIGNAL **********/
void REIN_GPIO_SIGNAL_COUNT_Init(void);		//GPIO初始�??(SIGNAL_COUNT)
void REIN_GPIO_SIGNAL_COUNT_DeInit(void);	//GPIO清理(SIGNAL_COUNT)
void REIN_GPIO_SIGNAL_PWM_Init(void);			//GPIO初始�??(SIGNAL_PWM)
void REIN_GPIO_SIGNAL_PWM_DeInit(void);		//GPIO清理(SIGNAL_PWM)


/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
