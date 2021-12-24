#ifndef CTRL_STEP_FW_TB67H450_STM32_H
#define CTRL_STEP_FW_TB67H450_STM32_H

#include "Driver/tb67h450_base.h"

class TB67H450 : public TB67H450Base
{
public:
    explicit TB67H450() : TB67H450Base()
    {}

private:
    void InitGpio() override;

    void InitPwm() override;

    void DacOutputVoltage(uint16_t _voltageA_3300mVIn12bits, uint16_t _voltageB_3300mVIn12bits) override;

    void SetInputA(bool _statusAp, bool _statusAm) override;

    void SetInputB(bool _statusBp, bool _statusBm) override;
};

#endif
