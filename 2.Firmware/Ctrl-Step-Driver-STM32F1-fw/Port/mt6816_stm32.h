#ifndef CTRL_STEP_FW_MT6816_STM32_H
#define CTRL_STEP_FW_MT6816_STM32_H

#include "Sensor/Encoder/mt6816_base.h"

class MT6816 : public MT6816Base
{
public:
    /*
     * _quickCaliDataPtr is the start address where calibration data stored,
     * in STM32F103CBT6 flash size is 128K (0x08000000 ~ 0x08020000), we use
     * last 33K (32K clib + 1K user) for storage, and it starts from 0x08017C00.
     */
    explicit MT6816() : MT6816Base((uint16_t*) (0x08017C00))
    {}


private:
    void SpiInit() override;

    uint16_t SpiTransmitAndRead16Bits(uint16_t _data) override;
};

#endif
