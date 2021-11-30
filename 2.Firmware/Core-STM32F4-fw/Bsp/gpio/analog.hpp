#ifndef REF_STM32F4_ANALOG_H
#define REF_STM32F4_ANALOG_H

#include <cstdint>
#include <adc.h>

class Analog
{
private:

public:
    enum AdcChannel_t
    {
        CH1,
        CH2,
        CH3,
        CH4
    };

    Analog()
    = default;

    float GetChipTemperature();

    float GetVoltage(AdcChannel_t _channel);

    uint16_t GetRaw(AdcChannel_t _channel);
};

#endif //REF_STM32F4_ANALOG_H
