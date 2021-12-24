#include "st_hardware.h"
#include "stm32f103xb.h"

uint64_t GetSerialNumber()
{
    // This procedure of building a USB serial number should be identical
    // to the way the STM's built-in USB bootloader does it. This means
    // that the device will have the same serial number in normal and DFU mode.
    uint32_t uuid0 = *(uint32_t * )(UID_BASE + 0);
    uint32_t uuid1 = *(uint32_t * )(UID_BASE + 4);
    uint32_t uuid2 = *(uint32_t * )(UID_BASE + 8);
    uint32_t uuid_mixed_part = uuid0 + uuid2;
    uint64_t serialNumber = ((uint64_t) uuid_mixed_part << 16) | (uint64_t)(uuid1 >> 16);

    return serialNumber;
}