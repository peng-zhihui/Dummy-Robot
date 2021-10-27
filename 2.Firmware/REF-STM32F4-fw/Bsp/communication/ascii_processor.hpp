#ifndef __ASCII_PROTOCOL_H
#define __ASCII_PROTOCOL_H


/* Includes ------------------------------------------------------------------*/
#include <fibre/protocol.hpp>

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void ASCII_protocol_parse_stream(const uint8_t* buffer, size_t len, StreamSink& response_channel);
void OnUsbAsciiCmd(const char* _cmd, size_t _len, StreamSink& _responseChannel);
void OnUartAsciiCmd(const char* _cmd, size_t _len, StreamSink& _responseChannel);

// Function to send messages back through specific channel (UART or USB-VCP).
// Use this function instead of printf because printf will send messages over ALL CHANNEL.
template<typename ... TArgs>
void Respond(StreamSink &output, bool include_checksum, const char *fmt, TArgs &&... args)
{
    char response[64];
    size_t len = snprintf(response, sizeof(response), fmt, std::forward<TArgs>(args)...);
    output.process_bytes((uint8_t *) response, len, nullptr);
    if (include_checksum)
    {
        uint8_t checksum = 0;
        for (size_t i = 0; i < len; ++i)
            checksum ^= response[i];
        len = snprintf(response, sizeof(response), "*%u", checksum);
        output.process_bytes((uint8_t *) response, len, nullptr);
    }
    output.process_bytes((const uint8_t *) "\r\n", 2, nullptr);
}


#endif /* __ASCII_PROTOCOL_H */
