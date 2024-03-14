#ifndef CTRL_STEP_FW_TB67H450_BASE_H
#define CTRL_STEP_FW_TB67H450_BASE_H

#include "driver_base.h"

class TB67H450Base : public DriverBase
{
public:
    explicit TB67H450Base()
    = default;

    void Init() override;

    void SetFocCurrentVector(uint32_t _directionInCount, int32_t _current_mA) override;

    void Sleep() override;

    void Brake() override;


protected:
    void SetTwoCoilsCurrent(uint16_t _currentA_3300mAIn12Bits, uint16_t _currentB_3300mAIn12Bits) override;


    /***** Port Specified Implements *****/
    virtual void InitGpio() = 0;

    virtual void InitPwm() = 0;

    virtual void DacOutputVoltage(uint16_t _voltageA_3300mVIn12bits, uint16_t _voltageB_3300mVIn12bits) = 0;

    virtual void SetInputA(bool _statusAp, bool _statusAm) = 0;

    virtual void SetInputB(bool _statusBp, bool _statusBm) = 0;
};

#endif
