#include "analog.hpp"

float Analog::GetChipTemperature()
{
    return AdcGetChipTemperature();
}

float Analog::GetVoltage(Analog::AdcChannel_t _channel)
{
    switch (_channel)
    {
        case CH1:
            return AdcGetVoltage(ADC_CH1);
        case CH2:
            return AdcGetVoltage(ADC_CH2);
        case CH3:
            return AdcGetVoltage(ADC_CH3);
        case CH4:
            return AdcGetVoltage(ADC_CH4);
    }

    return 0;
}

uint16_t Analog::GetRaw(Analog::AdcChannel_t _channel)
{
    switch (_channel)
    {
        case CH1:
            return AdcGetRaw(ADC_CH1);
        case CH2:
            return AdcGetRaw(ADC_CH2);
        case CH3:
            return AdcGetRaw(ADC_CH3);
        case CH4:
            return AdcGetRaw(ADC_CH4);
    }

    return 0;
}
