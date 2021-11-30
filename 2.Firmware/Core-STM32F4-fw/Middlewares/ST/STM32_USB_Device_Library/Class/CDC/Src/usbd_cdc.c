/**
  ******************************************************************************
  * @file    usbd_cdc.c
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   This file provides the high layer firmware functions to manage the
  *          following functionalities of the USB CDC Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as CDC Device (and enumeration for each implemented memory interface)
  *           - OUT/IN data transfer
  *           - Command IN transfer (class requests management)
  *           - Error management
  *
  *  @verbatim
  *
  *          ===================================================================
  *                                CDC Class Driver Description
  *          ===================================================================
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as CDC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Union Functional collection (using 1 IN endpoint for control)
  *             - Data interface class
  *
  *           These aspects may be enriched or modified for a specific user application.
  *
  *            This driver doesn't implement the following aspects of the specification
  *            (but it is possible to manage these features with some modifications on this driver):
  *             - Any class-specific aspect relative to communication classes should be managed by user application.
  *             - All communication classes other than PSTN are not managed
  *
  *  @endverbatim
  *
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

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include <cmsis_os.h>
#include <freertos_inc.h>

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_CDC
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_CDC_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CDC_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CDC_Private_Macros
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_CDC_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_CDC_Init (USBD_HandleTypeDef *pdev,
                               uint8_t cfgidx);

static uint8_t  USBD_CDC_DeInit (USBD_HandleTypeDef *pdev,
                                 uint8_t cfgidx);

static uint8_t  USBD_CDC_Setup (USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req);

static uint8_t  USBD_CDC_DataIn (USBD_HandleTypeDef *pdev,
                                 uint8_t epnum);

static uint8_t  USBD_CDC_DataOut (USBD_HandleTypeDef *pdev,
                                  uint8_t epnum);

static uint8_t  USBD_CDC_EP0_RxReady (USBD_HandleTypeDef *pdev);

static uint8_t  *USBD_CDC_GetFSCfgDesc (uint16_t *length);

static uint8_t  *USBD_CDC_GetHSCfgDesc (uint16_t *length);

static uint8_t  *USBD_CDC_GetOtherSpeedCfgDesc (uint16_t *length);

static uint8_t  *USBD_CDC_GetOtherSpeedCfgDesc (uint16_t *length);

uint8_t  *USBD_CDC_GetDeviceQualifierDescriptor (uint16_t *length);

static uint8_t  USBD_WinUSBComm_SetupVendor(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
//static uint8_t * USBD_GetUsrStrDescriptor(struct _USBD_HandleTypeDef *pdev, uint8_t index,  uint16_t *length);

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CDC_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
    {
        USB_LEN_DEV_QUALIFIER_DESC,
        USB_DESC_TYPE_DEVICE_QUALIFIER,
        0x00,
        0x02,
        0x00,
        0x00,
        0x00,
        0x40,
        0x01,
        0x00,
    };

/**
  * @}
  */

/** @defgroup USBD_CDC_Private_Variables
  * @{
  */


/* CDC interface class callbacks structure */
USBD_ClassTypeDef  USBD_CDC =
    {
        USBD_CDC_Init,
        USBD_CDC_DeInit,
        USBD_CDC_Setup,
        NULL,                 /* EP0_TxSent, */
        USBD_CDC_EP0_RxReady,
        USBD_CDC_DataIn,
        USBD_CDC_DataOut,
        NULL,
        NULL,
        NULL,
        USBD_CDC_GetHSCfgDesc,
        USBD_CDC_GetFSCfgDesc,
        USBD_CDC_GetOtherSpeedCfgDesc,
        USBD_CDC_GetDeviceQualifierDescriptor,
        USBD_UsrStrDescriptor
    };

/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_CDC_CfgDesc[USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END =
    {
        /*Configuration Descriptor*/
        0x09,   /* bLength: Configuration Descriptor size */
        USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType: Configuration */
        USB_CDC_CONFIG_DESC_SIZ,                /* wTotalLength:no of returned bytes */
        0x00,
        0x03,   /* bNumInterfaces: 3 interfaces (2 for CDC, 1 custom) */
        0x01,   /* bConfigurationValue: Configuration value */
        0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
        0xC0,   /* bmAttributes: self powered */
        0x32,   /* MaxPower 0 mA */

        ///////////////////////////////////////////////////////////////////////////////

        /* Interface Association Descriptor: CDC device (virtual com port) */
        0x08,   /* bLength: IAD size */
        0x0B,   /* bDescriptorType: Interface Association Descriptor */
        0x00,   /* bFirstInterface */
        0x02,   /* bInterfaceCount */
        0x02,   /* bFunctionClass: Communication Interface Class */
        0x02,   /* bFunctionSubClass: Abstract Control Model */
        0x01,   /* bFunctionProtocol: Common AT commands */
        0x00,   /* iFunction */

        /*---------------------------------------------------------------------------*/

        /*Interface Descriptor */
        0x09,   /* bLength: Interface Descriptor size */
        USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
        /* Interface descriptor type */
        0x00,   /* bInterfaceNumber: Number of Interface */
        0x00,   /* bAlternateSetting: Alternate setting */
        0x01,   /* bNumEndpoints: One endpoints used */
        0x02,   /* bInterfaceClass: Communication Interface Class */
        0x02,   /* bInterfaceSubClass: Abstract Control Model */
        0x01,   /* bInterfaceProtocol: Common AT commands */
        0x00,   /* iInterface: */

        /*Header Functional Descriptor*/
        0x05,   /* bLength: Endpoint Descriptor size */
        0x24,   /* bDescriptorType: CS_INTERFACE */
        0x00,   /* bDescriptorSubtype: Header Func Desc */
        0x10,   /* bcdCDC: spec release number */
        0x01,

        /*Call Management Functional Descriptor*/
        0x05,   /* bFunctionLength */
        0x24,   /* bDescriptorType: CS_INTERFACE */
        0x01,   /* bDescriptorSubtype: Call Management Func Desc */
        0x00,   /* bmCapabilities: D0+D1 */
        0x01,   /* bDataInterface: 1 */

        /*ACM Functional Descriptor*/
        0x04,   /* bFunctionLength */
        0x24,   /* bDescriptorType: CS_INTERFACE */
        0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
        0x02,   /* bmCapabilities */

        /*Union Functional Descriptor*/
        0x05,   /* bFunctionLength */
        0x24,   /* bDescriptorType: CS_INTERFACE */
        0x06,   /* bDescriptorSubtype: Union func desc */
        0x00,   /* bMasterInterface: Communication class interface */
        0x01,   /* bSlaveInterface0: Data Class Interface */

        /*Endpoint 2 Descriptor*/
        0x07,                           /* bLength: Endpoint Descriptor size */
        USB_DESC_TYPE_ENDPOINT,   /* bDescriptorType: Endpoint */
        CDC_CMD_EP,                     /* bEndpointAddress */
        0x03,                           /* bmAttributes: Interrupt */
        LOBYTE(CDC_CMD_PACKET_SIZE),     /* wMaxPacketSize: */
        HIBYTE(CDC_CMD_PACKET_SIZE),
        0x10,                           /* bInterval: */
        /*---------------------------------------------------------------------------*/

        /*Data class interface descriptor*/
        0x09,   /* bLength: Endpoint Descriptor size */
        USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: */
        0x01,   /* bInterfaceNumber: Number of Interface */
        0x00,   /* bAlternateSetting: Alternate setting */
        0x02,   /* bNumEndpoints: Two endpoints used */
        0x0A,   /* bInterfaceClass: CDC */
        0x00,   /* bInterfaceSubClass: */
        0x00,   /* bInterfaceProtocol: */
        0x00,   /* iInterface: */

        /*Endpoint OUT Descriptor*/
        0x07,   /* bLength: Endpoint Descriptor size */
        USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
        CDC_OUT_EP,                        /* bEndpointAddress */
        0x02,                              /* bmAttributes: Bulk */
        LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
        HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
        0x00,                              /* bInterval: ignore for Bulk transfer */

        /*Endpoint IN Descriptor*/
        0x07,   /* bLength: Endpoint Descriptor size */
        USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
        CDC_IN_EP,                         /* bEndpointAddress */
        0x02,                              /* bmAttributes: Bulk */
        LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
        HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
        0x00,                              /* bInterval: ignore for Bulk transfer */

        ///////////////////////////////////////////////////////////////////////////////

        /* Interface Association Descriptor: custom device */
        0x08,   /* bLength: IAD size */
        0x0B,   /* bDescriptorType: Interface Association Descriptor */
        0x02,   /* bFirstInterface */
        0x01,   /* bInterfaceCount */
        0x00,   /* bFunctionClass: */
        0x00,   /* bFunctionSubClass: */
        0x00,   /* bFunctionProtocol: */
        0x06,   /* iFunction */

        /*---------------------------------------------------------------------------*/

        /*Data class interface descriptor*/
        0x09,   /* bLength: Endpoint Descriptor size */
        USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: */
        0x02,   /* bInterfaceNumber: Number of Interface */
        0x00,   /* bAlternateSetting: Alternate setting */
        0x02,   /* bNumEndpoints: Two endpoints used */
        0x00,   /* bInterfaceClass: vendor specific */
        0x01,   /* bInterfaceSubClass: ODrive Communication */
        0x00,   /* bInterfaceProtocol: */
        0x00,   /* iInterface: */

        /*Endpoint OUT Descriptor*/
        0x07,   /* bLength: Endpoint Descriptor size */
        USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
        ODRIVE_OUT_EP,                        /* bEndpointAddress */
        0x02,                              /* bmAttributes: Bulk */
        LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
        HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
        0x00,                              /* bInterval: ignore for Bulk transfer */

        /*Endpoint IN Descriptor*/
        0x07,   /* bLength: Endpoint Descriptor size */
        USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
        ODRIVE_IN_EP,                         /* bEndpointAddress */
        0x02,                              /* bmAttributes: Bulk */
        LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
        HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
        0x00,                              /* bInterval: ignore for Bulk transfer */
    } ;


/**
  * @}
  */

/** @defgroup USBD_CDC_Private_Functions
  * @{
  */

/**
  * @brief  USBD_CDC_Init
  *         Initialize the CDC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CDC_Init (USBD_HandleTypeDef *pdev,
                               uint8_t cfgidx)
{
    uint8_t ret = 0;
    USBD_CDC_HandleTypeDef   *hcdc;

    if(pdev->dev_speed == USBD_SPEED_HIGH  )
    {
        /* Open EP IN */
        USBD_LL_OpenEP(pdev,
                       CDC_IN_EP,
                       USBD_EP_TYPE_BULK,
                       CDC_DATA_HS_IN_PACKET_SIZE);

        /* Open EP OUT */
        USBD_LL_OpenEP(pdev,
                       CDC_OUT_EP,
                       USBD_EP_TYPE_BULK,
                       CDC_DATA_HS_OUT_PACKET_SIZE);

    }
    else
    {
        /* Open EP IN */
        USBD_LL_OpenEP(pdev,
                       CDC_IN_EP,
                       USBD_EP_TYPE_BULK,
                       CDC_DATA_FS_IN_PACKET_SIZE);

        /* Open EP OUT */
        USBD_LL_OpenEP(pdev,
                       CDC_OUT_EP,
                       USBD_EP_TYPE_BULK,
                       CDC_DATA_FS_OUT_PACKET_SIZE);
    }

    /* Open ODrive IN endpoint */
    USBD_LL_OpenEP(pdev,
                   ODRIVE_IN_EP,
                   USBD_EP_TYPE_BULK,
                   pdev->dev_speed == USBD_SPEED_HIGH ? CDC_DATA_HS_IN_PACKET_SIZE : CDC_DATA_FS_IN_PACKET_SIZE);

    /* Open ODrive OUT endpoint */
    USBD_LL_OpenEP(pdev,
                   ODRIVE_OUT_EP,
                   USBD_EP_TYPE_BULK,
                   pdev->dev_speed == USBD_SPEED_HIGH ? CDC_DATA_HS_OUT_PACKET_SIZE : CDC_DATA_FS_OUT_PACKET_SIZE);

    /* Open Command IN EP */
    USBD_LL_OpenEP(pdev,
                   CDC_CMD_EP,
                   USBD_EP_TYPE_INTR,
                   CDC_CMD_PACKET_SIZE);


    pdev->pClassData = USBD_malloc(sizeof (USBD_CDC_HandleTypeDef));

    if(pdev->pClassData == NULL)
    {
        ret = 1;
    }
    else
    {
        hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;

        /* Init  physical Interface components */
        ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Init();

        /* Init Xfer states */
        hcdc->CDC_Tx.State =0;
        hcdc->CDC_Rx.State =0;
        hcdc->REF_Tx.State =0;
        hcdc->ODRIVE_Rx.State =0;

        if(pdev->dev_speed == USBD_SPEED_HIGH  )
        {
            /* Prepare Out endpoint to receive next packet */
            USBD_LL_PrepareReceive(pdev,
                                   CDC_OUT_EP,
                                   hcdc->CDC_Rx.Buffer,
                                   CDC_DATA_HS_OUT_PACKET_SIZE);
        }
        else
        {
            /* Prepare Out endpoint to receive next packet */
            USBD_LL_PrepareReceive(pdev,
                                   CDC_OUT_EP,
                                   hcdc->CDC_Rx.Buffer,
                                   CDC_DATA_FS_OUT_PACKET_SIZE);
        }

        /* Prepare ODrive Out endpoint to receive next packet */
        USBD_LL_PrepareReceive(pdev,
                               ODRIVE_OUT_EP,
                               hcdc->ODRIVE_Rx.Buffer,
                               CDC_DATA_FS_OUT_PACKET_SIZE);
    }
    return ret;
}

/**
  * @brief  USBD_CDC_Init
  *         DeInitialize the CDC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CDC_DeInit (USBD_HandleTypeDef *pdev,
                                 uint8_t cfgidx)
{
    uint8_t ret = 0;

    /* Close EP IN */
    USBD_LL_CloseEP(pdev,
                    CDC_IN_EP);

    /* Close EP OUT */
    USBD_LL_CloseEP(pdev,
                    CDC_OUT_EP);

    /* Close Command IN EP */
    USBD_LL_CloseEP(pdev,
                    CDC_CMD_EP);

    /* Close EP IN */
    USBD_LL_CloseEP(pdev,
                    ODRIVE_IN_EP);

    /* Close EP OUT */
    USBD_LL_CloseEP(pdev,
                    ODRIVE_OUT_EP);


    /* DeInit  physical Interface components */
    if(pdev->pClassData != NULL)
    {
        ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->DeInit();
        USBD_free(pdev->pClassData);
        pdev->pClassData = NULL;
    }

    return ret;
}

/**
  * @brief  USBD_CDC_Setup
  *         Handle the CDC specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_CDC_Setup (USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req)
{
    USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;
    static uint8_t ifalt = 0;

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
        case USB_REQ_TYPE_CLASS :
            if (req->wLength)
            {
                if (req->bmRequest & 0x80)
                {
                    ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Control(req->bRequest,
                                                                      (uint8_t *)hcdc->data,
                                                                      req->wLength);
                    USBD_CtlSendData (pdev,
                                      (uint8_t *)hcdc->data,
                                      req->wLength);
                }
                else
                {
                    hcdc->CmdOpCode = req->bRequest;
                    hcdc->CmdLength = req->wLength;

                    USBD_CtlPrepareRx (pdev,
                                       (uint8_t *)hcdc->data,
                                       req->wLength);
                }

            }
            else
            {
                ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Control(req->bRequest,
                                                                  (uint8_t*)req,
                                                                  0);
            }
            break;

        case USB_REQ_TYPE_STANDARD:
            switch (req->bRequest)
            {
                case USB_REQ_GET_INTERFACE :
                    USBD_CtlSendData (pdev,
                                      &ifalt,
                                      1);
                    break;

                case USB_REQ_SET_INTERFACE :
                    break;
            }

        case USB_REQ_TYPE_VENDOR:
            return USBD_WinUSBComm_SetupVendor(pdev, req);

        default:
            break;
    }
    return USBD_OK;
}

/**
  * @brief  USBD_CDC_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_CDC_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;

    if(pdev->pClassData != NULL)
    {
        // NOTE: We would logically expect xx_IN_EP here, but we actually get the xx_OUT_EP
        if (epnum == CDC_OUT_EP)
            hcdc->CDC_Tx.State = 0;
        if (epnum == ODRIVE_OUT_EP)
            hcdc->REF_Tx.State = 0;
        //Note: We could use independent semaphores for simoultainous USB transmission.
        osSemaphoreRelease(sem_usb_tx);
        return USBD_OK;
    }
    else
    {
        return USBD_FAIL;
    }
}

/**
  * @brief  USBD_CDC_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_CDC_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;

    USBD_CDC_EP_HandleTypeDef* hEP_Rx;
    if (epnum == CDC_OUT_EP) {
        hEP_Rx = &hcdc->CDC_Rx;
    } else if (epnum == ODRIVE_OUT_EP) {
        hEP_Rx = &hcdc->ODRIVE_Rx;
    } else {
        return USBD_FAIL;
    }

    /* Get the received data length */
    hEP_Rx->Length = USBD_LL_GetRxDataSize (pdev, epnum);

    /* USB data will be immediately processed, this allow next USB traffic being
    NAKed till the end of the application Xfer */
    if(pdev->pClassData != NULL)
    {
        ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Receive(hEP_Rx->Buffer, &hEP_Rx->Length, epnum);

        return USBD_OK;
    }
    else
    {
        return USBD_FAIL;
    }
}



/**
  * @brief  USBD_CDC_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_CDC_EP0_RxReady (USBD_HandleTypeDef *pdev)
{
    USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;

    if((pdev->pUserData != NULL) && (hcdc->CmdOpCode != 0xFF))
    {
        ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->Control(hcdc->CmdOpCode,
                                                          (uint8_t *)hcdc->data,
                                                          hcdc->CmdLength);
        hcdc->CmdOpCode = 0xFF;

    }
    return USBD_OK;
}

/**
  * @brief  USBD_CDC_GetFSCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CDC_GetFSCfgDesc (uint16_t *length)
{
    *length = sizeof (USBD_CDC_CfgDesc);
    return USBD_CDC_CfgDesc;
}

/**
  * @brief  USBD_CDC_GetHSCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CDC_GetHSCfgDesc (uint16_t *length)
{
    *length = sizeof (USBD_CDC_CfgDesc);
    return USBD_CDC_CfgDesc;
}

/**
  * @brief  USBD_CDC_GetCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CDC_GetOtherSpeedCfgDesc (uint16_t *length)
{
    *length = sizeof (USBD_CDC_CfgDesc);
    return USBD_CDC_CfgDesc;
}

/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t  *USBD_CDC_GetDeviceQualifierDescriptor (uint16_t *length)
{
    *length = sizeof (USBD_CDC_DeviceQualifierDesc);
    return USBD_CDC_DeviceQualifierDesc;
}

/**
* @brief  USBD_CDC_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CD  Interface callback
  * @retval status
  */
uint8_t  USBD_CDC_RegisterInterface  (USBD_HandleTypeDef   *pdev,
                                      USBD_CDC_ItfTypeDef *fops)
{
    uint8_t  ret = USBD_FAIL;

    if(fops != NULL)
    {
        pdev->pUserData= fops;
        ret = USBD_OK;
    }

    return ret;
}

/**
  * @brief  USBD_CDC_SetTxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Tx Buffer
  * @retval status
  */
uint8_t  USBD_CDC_SetTxBuffer  (USBD_HandleTypeDef   *pdev,
                                uint8_t  *pbuff,
                                uint16_t length,
                                uint8_t endpoint_pair)
{
    USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;

    USBD_CDC_EP_HandleTypeDef* hEP_Tx;
    if (endpoint_pair == CDC_OUT_EP) {
        hEP_Tx = &hcdc->CDC_Tx;
    } else if (endpoint_pair == ODRIVE_OUT_EP) {
        hEP_Tx = &hcdc->REF_Tx;
    } else {
        return USBD_FAIL;
    }

    hEP_Tx->Buffer = pbuff;
    hEP_Tx->Length = length;

    return USBD_OK;
}


/**
  * @brief  USBD_CDC_SetRxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Rx Buffer
  * @retval status
  */
uint8_t  USBD_CDC_SetRxBuffer  (USBD_HandleTypeDef   *pdev,
                                uint8_t  *pbuff, uint8_t endpoint_pair)
{
    USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;

    USBD_CDC_EP_HandleTypeDef* hEP_Rx;
    if (endpoint_pair == CDC_OUT_EP) {
        hEP_Rx = &hcdc->CDC_Rx;
    } else if (endpoint_pair == ODRIVE_OUT_EP) {
        hEP_Rx = &hcdc->ODRIVE_Rx;
    } else {
        return USBD_FAIL;
    }

    hEP_Rx->Buffer = pbuff;

    return USBD_OK;
}

/**
  * @brief  USBD_CDC_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
uint8_t  USBD_CDC_TransmitPacket(USBD_HandleTypeDef *pdev, uint8_t endpoint_pair)
{
    USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;

    if(pdev->pClassData != NULL)
    {
        // Select Endpoint
        USBD_CDC_EP_HandleTypeDef* hEP_Tx;
        uint8_t in_ep;
        if (endpoint_pair == CDC_OUT_EP) {
            hEP_Tx = &hcdc->CDC_Tx;
            in_ep = CDC_IN_EP;
        } else if (endpoint_pair == ODRIVE_OUT_EP) {
            hEP_Tx = &hcdc->REF_Tx;
            in_ep = ODRIVE_IN_EP;
        } else {
            return USBD_FAIL;
        }

        if(hEP_Tx->State == 0)
        {
            /* Tx Transfer in progress */
            hEP_Tx->State = 1;

            /* Transmit next packet */
            USBD_LL_Transmit(pdev,
                             in_ep,
                             hEP_Tx->Buffer,
                             hEP_Tx->Length);

            return USBD_OK;
        }
        else
        {
            return USBD_BUSY;
        }
    }
    else
    {
        return USBD_FAIL;
    }
}


/**
  * @brief  USBD_CDC_ReceivePacket
  *         prepare OUT Endpoint for reception
  * @param  pdev: device instance
  * @retval status
  */
uint8_t  USBD_CDC_ReceivePacket(USBD_HandleTypeDef *pdev, uint8_t endpoint_pair)
{
    USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;

    /* Suspend or Resume USB Out process */
    if(pdev->pClassData != NULL)
    {
        // Select Endpoint
        USBD_CDC_EP_HandleTypeDef* hEP_Rx;
        uint8_t out_ep;
        if (endpoint_pair == CDC_OUT_EP) {
            hEP_Rx = &hcdc->CDC_Rx;
            out_ep = CDC_OUT_EP;
        } else if (endpoint_pair == ODRIVE_OUT_EP) {
            hEP_Rx = &hcdc->ODRIVE_Rx;
            out_ep = ODRIVE_OUT_EP;
        } else {
            return USBD_FAIL;
        }

        /* Prepare Out endpoint to receive next packet */
        USBD_LL_PrepareReceive(pdev,
                               out_ep,
                               hEP_Rx->Buffer,
                               pdev->dev_speed == USBD_SPEED_HIGH ? CDC_DATA_HS_OUT_PACKET_SIZE : CDC_DATA_FS_OUT_PACKET_SIZE);

        return USBD_OK;
    }
    else
    {
        return USBD_FAIL;
    }
}


/* WinUSB support ------------------------------------------------------------*/
/*
* This section tells Windows that it should automatically load the WinUSB driver
* for the device (more specifically, interface 2 because it's a composite device).
* This allows for driverless communication with the device.
*/

#define NUM_INTERFACES 1

#if NUM_INTERFACES == 2
#define USB_WINUSBCOMM_COMPAT_ID_OS_DESC_SIZ       (16 + 24 + 24)
#else
#define USB_WINUSBCOMM_COMPAT_ID_OS_DESC_SIZ       (16 + 24)
#endif


// This associates winusb driver with the device
__ALIGN_BEGIN uint8_t USBD_WinUSBComm_Extended_Compat_ID_OS_Desc[USB_WINUSBCOMM_COMPAT_ID_OS_DESC_SIZ]  __ALIGN_END =
    {
        //    +-- Offset in descriptor
        //    |             +-- Size
        //    v             v
        USB_WINUSBCOMM_COMPAT_ID_OS_DESC_SIZ, 0, 0, 0,    //    0 dwLength    4 DWORD The length, in bytes, of the complete extended compat ID descriptor
        0x00, 0x01,                                       //    4 bcdVersion  2 BCD The descriptor’s version number, in binary coded decimal (BCD) format
        0x04, 0x00,                                       //    6 wIndex      2 WORD  An index that identifies the particular OS feature descriptor
        NUM_INTERFACES,                                   //    8 bCount      1 BYTE  The number of custom property sections
        0, 0, 0, 0, 0, 0, 0,                              //    9 RESERVED    7 BYTEs Reserved
        //    =====================
        //                 16

        //   +-- Offset from function section start
        //   |                        +-- Size
        //   v                        v
        2,                                                //   0  bFirstInterfaceNumber 1 BYTE  The interface or function number
        0,                                                //   1  RESERVED              1 BYTE  Reserved
        0x57, 0x49, 0x4E, 0x55, 0x53, 0x42, 0x00, 0x00,   //   2  compatibleID          8 BYTEs The function’s compatible ID      ("WINUSB")
        0, 0, 0, 0, 0, 0, 0, 0,                           //  10  subCompatibleID       8 BYTEs The function’s subcompatible ID
        0, 0, 0, 0, 0, 0,                                 //  18  RESERVED              6 BYTEs Reserved
        //  =================================
        //                           24
#if NUM_INTERFACES == 2
        //   +-- Offset from function section start
                                                    //   |                        +-- Size
                                                    //   v                        v
  2,                                                //   0  bFirstInterfaceNumber 1 BYTE  The interface or function number
  0,                                                //   1  RESERVED              1 BYTE  Reserved
  0x57, 0x49, 0x4E, 0x55, 0x53, 0x42, 0x00, 0x00,   //   2  compatibleID          8 BYTEs The function’s compatible ID      ("WINUSB")
  0, 0, 0, 0, 0, 0, 0, 0,                           //  10  subCompatibleID       8 BYTEs The function’s subcompatible ID
  0, 0, 0, 0, 0, 0,                                 //  18  RESERVED              6 BYTEs Reserved
                                                    //  =================================
                                                    //                           24
#endif
    };


// Properties are added to:
// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\USB\VID_xxxx&PID_xxxx\sssssssss\Device Parameters
// Use USBDeview or similar to uninstall

__ALIGN_BEGIN uint8_t USBD_WinUSBComm_Extended_Properties_OS_Desc[0xB6]  __ALIGN_END =
    {
        0xB6, 0x00, 0x00, 0x00,   // 0 dwLength   4 DWORD The length, in bytes, of the complete extended properties descriptor
        0x00, 0x01,               // 4 bcdVersion 2 BCD   The descriptor’s version number, in binary coded decimal (BCD) format
        0x05, 0x00,               // 6 wIndex     2 WORD  The index for extended properties OS descriptors
        0x02, 0x00,               // 8 wCount     2 WORD  The number of custom property sections that follow the header section
        // ====================
        //             10
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        0x84, 0x00, 0x00, 0x00,   //  0       dwSize                  4 DWORD             The size of this custom properties section
        0x01, 0x00, 0x00, 0x00,   //  4       dwPropertyDataType      4 DWORD             Property data format
        0x28, 0x00,               //  8       wPropertyNameLength     2 DWORD             Property name length
        // ========================================
        //                                 10
        // 10       bPropertyName         PNL WCHAR[]           The property name
        'D',0, 'e',0, 'v',0, 'i',0, 'c',0, 'e',0, 'I',0, 'n',0,
        't',0, 'e',0, 'r',0, 'f',0, 'a',0, 'c',0, 'e',0, 'G',0,
        'U',0, 'I',0, 'D',0, 0,0,
        // ========================================
        //                                 40 (0x28)

        0x4E, 0x00, 0x00, 0x00,   // 10 + PNL dwPropertyDataLength    4 DWORD             Length of the buffer holding the property data
        // ========================================
        //                                  4
        // 14 + PNL bPropertyData         PDL Format-dependent  Property data
        '{',0, 'E',0, 'A',0, '0',0, 'B',0, 'D',0, '5',0, 'C',0,
        '3',0, '-',0, '5',0, '0',0, 'F',0, '3',0, '-',0, '4',0,
        '8',0, '8',0, '8',0, '-',0, '8',0, '4',0, 'B',0, '4',0,
        '-',0, '7',0, '4',0, 'E',0, '5',0, '0',0, 'E',0, '1',0,
        '6',0, '4',0, '9',0, 'D',0, 'B',0, '}',0,  0 ,0,
        // ========================================
        //                                 78 (0x4E)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        0x3E, 0x00, 0x00, 0x00,   //  0 dwSize 0x00000030 (62 bytes)
        0x01, 0x00, 0x00, 0x00,   //  4 dwPropertyDataType 0x00000001 (Unicode string)
        0x0C, 0x00,               //  8 wPropertyNameLength 0x000C (12 bytes)
        // ========================================
        //                                  10
        'L',0, 'a',0, 'b',0, 'e',0, 'l',0, 0,0,
        // 10 bPropertyName “Label”
        // ========================================
        //                                  12
        0x24, 0x00, 0x00, 0x00,   // 22 dwPropertyDataLength 0x00000016 (36 bytes)
        // ========================================
        //                                  4
        'O',0, 'D',0, 'r',0, 'i',0, 'v',0, 'e',0, 0,0
        // 26 bPropertyData “ODrive”
        // ========================================
        //                                  14

    };



static uint8_t  USBD_WinUSBComm_GetMSExtendedCompatIDOSDescriptor (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    switch (req->wIndex)
    {
        case 0x04:
            USBD_CtlSendData (pdev, USBD_WinUSBComm_Extended_Compat_ID_OS_Desc, req->wLength);
            break;
        default:
            USBD_CtlError(pdev , req);
            return USBD_FAIL;
    }
    return USBD_OK;
}
static uint8_t  USBD_WinUSBComm_GetMSExtendedPropertiesOSDescriptor (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t byInterfaceIndex = (uint8_t)req->wValue;
    if ( req->wIndex != 0x05 )
    {
        USBD_CtlError(pdev , req);
        return USBD_FAIL;
    }
    switch ( byInterfaceIndex )
    {
        case 0:
#if NUM_INTERFACES == 2
            case 1:
#endif
            USBD_CtlSendData (pdev, USBD_WinUSBComm_Extended_Properties_OS_Desc, req->wLength);
            break;
        default:
            USBD_CtlError(pdev , req);
            return USBD_FAIL;
    }
    return USBD_OK;
}
static uint8_t  USBD_WinUSBComm_SetupVendorDevice(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    USBD_CtlError(pdev , req);
    return USBD_FAIL;
}
static uint8_t  USBD_WinUSBComm_SetupVendorInterface(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    USBD_CtlError(pdev , req);
    // TODO: check if this is important
    return USBD_FAIL;
}
static uint8_t  USBD_WinUSBComm_SetupVendor(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    switch ( req->bmRequest & USB_REQ_RECIPIENT_MASK )
    {
        case USB_REQ_RECIPIENT_DEVICE:
            return ( MS_VendorCode == req->bRequest ) ? USBD_WinUSBComm_GetMSExtendedCompatIDOSDescriptor(pdev, req) : USBD_WinUSBComm_SetupVendorDevice(pdev, req);
        case USB_REQ_RECIPIENT_INTERFACE:
            return ( MS_VendorCode == req->bRequest ) ? USBD_WinUSBComm_GetMSExtendedPropertiesOSDescriptor(pdev, req) : USBD_WinUSBComm_SetupVendorInterface(pdev, req);
        case USB_REQ_RECIPIENT_ENDPOINT:
            // fall through
        default:
            break;
    }
    USBD_CtlError(pdev , req);
    return USBD_FAIL;
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
