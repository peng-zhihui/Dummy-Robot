#ifndef CTRL_STEP_FW_DRIVER_BASE_H
#define CTRL_STEP_FW_DRIVER_BASE_H

#include <cstdint>


class DriverBase
{
public:
    virtual void Init() = 0;

    /*
     * FOC current vector direction is described as counts, that means
     * we divide a 360° circle into N counts, and _directionInCount is
     * between (0 ~ N-1), and after calculation the current range is (0 ~ 3300mA)
     */
    virtual void SetFocCurrentVector(uint32_t _directionInCount, int32_t _current_mA) = 0;

    virtual void Sleep() = 0;

    virtual void Brake() = 0;


protected:
    // Used to composite the FOC current vector
    virtual void SetTwoCoilsCurrent(uint16_t _currentA_mA, uint16_t _currentB_mA) = 0;

    typedef struct
    {
        uint16_t sinMapPtr;
        int16_t sinMapData;
        uint16_t dacValue12Bits;
    } FastSinToDac_t;

    FastSinToDac_t phaseB{};
    FastSinToDac_t phaseA{};
};

#endif
