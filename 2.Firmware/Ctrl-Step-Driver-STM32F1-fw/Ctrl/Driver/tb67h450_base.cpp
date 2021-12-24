#include "tb67h450_base.h"
#include "sin_map.h"

void TB67H450Base::Init()
{
    InitGpio();
    InitPwm();
}

void TB67H450Base::SetFocCurrentVector(uint32_t _directionInCount, int32_t _current_mA)
{
    phaseB.sinMapPtr = (_directionInCount) & (0x000003FF);
    phaseA.sinMapPtr = (phaseB.sinMapPtr + (256)) & (0x000003FF);

    phaseA.sinMapData = sin_pi_m2[phaseA.sinMapPtr];
    phaseB.sinMapData = sin_pi_m2[phaseB.sinMapPtr];

    uint32_t dac_reg = abs(_current_mA);
    dac_reg = (uint32_t) (dac_reg * 5083) >> 12;
    dac_reg = dac_reg & (0x00000FFF);
    phaseA.dacValue12Bits =
        (uint32_t) (dac_reg * abs(phaseA.sinMapData)) >> sin_pi_m2_dpiybit;
    phaseB.dacValue12Bits =
        (uint32_t) (dac_reg * abs(phaseB.sinMapData)) >> sin_pi_m2_dpiybit;

    SetTwoCoilsCurrent(phaseA.dacValue12Bits, phaseB.dacValue12Bits);

    if (phaseA.sinMapData > 0)
        SetInputA(true, false);
    else if (phaseA.sinMapData < 0)
        SetInputA(false, true);
    else
        SetInputA(true, true);

    if (phaseB.sinMapData > 0)
        SetInputB(true, false);
    else if (phaseB.sinMapData < 0)
        SetInputB(false, true);
    else
        SetInputB(true, true);
}


void TB67H450Base::SetTwoCoilsCurrent(uint16_t _currentA_3300mAIn12Bits, uint16_t _currentB_3300mAIn12Bits)
{
    /*
     * After SetFocCurrentVector calculation a 12bits value was mapped to 0~3300mA.
     * And due to used 0.1Ohm shank resistor, 0~3300mV V-ref means 0~3300mA CurrentSetPoint,
     * For more details, see TB67H450 Datasheet page.10 .
     */

    DacOutputVoltage(_currentA_3300mAIn12Bits, _currentB_3300mAIn12Bits);
}


void TB67H450Base::Sleep()
{
    phaseA.dacValue12Bits = 0;
    phaseB.dacValue12Bits = 0;

    SetTwoCoilsCurrent(phaseA.dacValue12Bits, phaseB.dacValue12Bits);

    SetInputA(false, false);
    SetInputB(false, false);
}


void TB67H450Base::Brake()
{
    phaseA.dacValue12Bits = 0;
    phaseB.dacValue12Bits = 0;

    SetTwoCoilsCurrent(phaseA.dacValue12Bits, phaseB.dacValue12Bits);

    SetInputA(true, true);
    SetInputB(true, true);
}


