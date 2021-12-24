#ifndef CTRL_STEP_FW_ENCODER_CALIBRATOR_H
#define CTRL_STEP_FW_ENCODER_CALIBRATOR_H

#include "Sensor/Encoder/encoder_calibrator_base.h"

class EncoderCalibrator : public EncoderCalibratorBase
{
public:
    explicit EncoderCalibrator(Motor* _motor) : EncoderCalibratorBase(_motor)
    {}


private:
    void BeginWriteFlash() override;
    void EndWriteFlash() override;
    void ClearFlash() override;
    void WriteFlash16bitsAppend(uint16_t _data) override;
};

#endif
