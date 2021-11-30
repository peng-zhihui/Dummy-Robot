#ifndef __INTERFACE_USB_HPP
#define __INTERFACE_USB_HPP

#ifdef __cplusplus

#include "fibre/protocol.hpp"

extern StreamSink *usbStreamOutputPtr;

extern "C" {
#endif

#include <cmsis_os.h>
#include <stdint.h>

typedef struct
{
    uint32_t rx_cnt;
    uint32_t tx_cnt;
    uint32_t tx_overrun_cnt;
} USBStats_t;

extern USBStats_t usb_stats_;

void usb_rx_process_packet(uint8_t *buf, uint32_t len, uint8_t endpoint_pair);
void StartUsbServer(void);

#ifdef __cplusplus
}
#endif

#endif // __INTERFACE_USB_HPP
