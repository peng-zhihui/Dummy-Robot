#ifndef CTRL_STEP_FW_ENCODER_H
#define CTRL_STEP_FW_ENCODER_H

#include <cstdint>


class EncoderBase
{
public:
    typedef struct
    {
        uint16_t rawAngle;          // raw data
        uint16_t rectifiedAngle;    // calibrated rawAngle data
        bool rectifyValid;
    } AngleData_t;
    AngleData_t angleData{0};


    /*
     * Resolution is (2^14 = 16384), each state will use an uint16 data
     * as map, thus total need 32K-flash for calibration.
    */
    const int32_t RESOLUTION = ((int32_t) ((0x00000001U) << 14));

    virtual bool Init() = 0;

    // Get current rawAngle
    virtual uint16_t UpdateAngle() = 0;

    virtual bool IsCalibrated() = 0;


private:

};

#endif
