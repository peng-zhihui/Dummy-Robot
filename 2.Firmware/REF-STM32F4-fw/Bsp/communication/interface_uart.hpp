#ifndef __INTERFACE_UART_HPP
#define __INTERFACE_UART_HPP

#ifdef __cplusplus

#include "fibre/protocol.hpp"

extern StreamSink *uart4_stream_output_ptr;

extern "C" {
#endif

#include <cmsis_os.h>

extern osThreadId uart_thread;

void StartUartServer(void);

#ifdef __cplusplus
}
#endif

#endif // __INTERFACE_UART_HPP
