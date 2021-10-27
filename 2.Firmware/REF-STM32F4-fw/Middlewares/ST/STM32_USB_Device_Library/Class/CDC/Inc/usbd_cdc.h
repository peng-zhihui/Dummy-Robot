/**
  ******************************************************************************
  * @file    usbd_cdc.h
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   header file for the usbd_cdc.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_CDC_H
#define __USB_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup usbd_cdc
  * @brief This file is the Header file for usbd_cdc.c
  * @{
  */


/** @defgroup usbd_cdc_Exported_Defines
  * @{
  */
#define CDC_IN_EP                                   0x81  /* EP1 for data IN */
#define CDC_OUT_EP                                  0x01  /* EP1 for data OUT */
#define CDC_CMD_EP                                  0x82  /* EP2 for CDC commands */
#define ODRIVE_IN_EP                                0x83  /* EP3 IN: ODrive device TX endpoint */
#define ODRIVE_OUT_EP                               0x03  /* EP3 OUT: ODrive device RX endpoint */

/* CDC Endpoints parameters: you can fine tune these values depending on the needed baudrates and performance. */
#define CDC_DATA_HS_MAX_PACKET_SIZE                 64  /* Endpoint IN & OUT Packet size */
#define CDC_DATA_FS_MAX_PACKET_SIZE                 64  /* Endpoint IN & OUT Packet size */
#define CDC_CMD_PACKET_SIZE                         8  /* Control Endpoint Packet size */

#define USB_CDC_CONFIG_DESC_SIZ                     (67 + 39)
#define CDC_DATA_HS_IN_PACKET_SIZE                  CDC_DATA_HS_MAX_PACKET_SIZE
#define CDC_DATA_HS_OUT_PACKET_SIZE                 CDC_DATA_HS_MAX_PACKET_SIZE

#define CDC_DATA_FS_IN_PACKET_SIZE                  CDC_DATA_FS_MAX_PACKET_SIZE
#define CDC_DATA_FS_OUT_PACKET_SIZE                 CDC_DATA_FS_MAX_PACKET_SIZE

/*---------------------------------------------------------------------*/
/*  CDC definitions                                                    */
/*---------------------------------------------------------------------*/
#define CDC_SEND_ENCAPSULATED_COMMAND               0x00
#define CDC_GET_ENCAPSULATED_RESPONSE               0x01
#define CDC_SET_COMM_FEATURE                        0x02
#define CDC_GET_COMM_FEATURE                        0x03
#define CDC_CLEAR_COMM_FEATURE                      0x04
#define CDC_SET_LINE_CODING                         0x20
#define CDC_GET_LINE_CODING                         0x21
#define CDC_SET_CONTROL_LINE_STATE                  0x22
#define CDC_SEND_BREAK                              0x23

/**
  * @}
  */


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */

/**
  * @}
  */
typedef struct
{
    uint32_t bitrate;
    uint8_t  format;
    uint8_t  paritytype;
    uint8_t  datatype;
}USBD_CDC_LineCodingTypeDef;

typedef struct _USBD_CDC_Itf
{
    int8_t (* Init)          (void);
    int8_t (* DeInit)        (void);
    int8_t (* Control)       (uint8_t, uint8_t * , uint16_t);
    int8_t (* Receive)       (uint8_t *, uint32_t *, uint8_t);

}USBD_CDC_ItfTypeDef;

typedef struct
{
    uint8_t* Buffer;
    uint32_t Length;
    volatile uint8_t State;
}
    USBD_CDC_EP_HandleTypeDef;

typedef struct
{
    uint32_t data[CDC_DATA_HS_MAX_PACKET_SIZE/4];      /* Force 32bits alignment */
    uint8_t  CmdOpCode;
    uint8_t  CmdLength;

    USBD_CDC_EP_HandleTypeDef CDC_Tx;
    USBD_CDC_EP_HandleTypeDef CDC_Rx;

    USBD_CDC_EP_HandleTypeDef REF_Tx;
    USBD_CDC_EP_HandleTypeDef ODRIVE_Rx;
}
    USBD_CDC_HandleTypeDef;



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */

extern USBD_ClassTypeDef  USBD_CDC;
#define USBD_CDC_CLASS    &USBD_CDC
/**
  * @}
  */

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t  USBD_CDC_RegisterInterface  (USBD_HandleTypeDef   *pdev,
                                      USBD_CDC_ItfTypeDef *fops);

uint8_t  USBD_CDC_SetTxBuffer        (USBD_HandleTypeDef   *pdev,
                                      uint8_t  *pbuff,
                                      uint16_t length,
                                      uint8_t endpoint_pair);

uint8_t  USBD_CDC_SetRxBuffer        (USBD_HandleTypeDef   *pdev,
                                      uint8_t  *pbuff, uint8_t endpoint_pair);

uint8_t  USBD_CDC_ReceivePacket      (USBD_HandleTypeDef *pdev, uint8_t endpoint_pair);

uint8_t  USBD_CDC_TransmitPacket     (USBD_HandleTypeDef *pdev, uint8_t endpoint_pair);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USB_CDC_H */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
