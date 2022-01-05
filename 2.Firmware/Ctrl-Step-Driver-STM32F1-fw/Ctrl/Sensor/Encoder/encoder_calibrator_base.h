#ifndef CTRL_STEP_FW_ENCODER_CALIBRATOR_BASE_H
#define CTRL_STEP_FW_ENCODER_CALIBRATOR_BASE_H

#include "Motor/motor.h"

class EncoderCalibratorBase
{
public:
    static const int32_t MOTOR_ONE_CIRCLE_HARD_STEPS = 200;   // for 1.8° step-motors
    static const uint8_t SAMPLE_COUNTS_PER_STEP = 16;
    static const uint8_t AUTO_CALIB_SPEED = 2;
    static const uint8_t FINE_TUNE_CALIB_SPEED = 1;


    typedef enum
    {
        CALI_NO_ERROR = 0x00,
        CALI_ERROR_AVERAGE_DIR,
        CALI_ERROR_AVERAGE_CONTINUTY,
        CALI_ERROR_PHASE_STEP,
        CALI_ERROR_ANALYSIS_QUANTITY,
    } Error_t;

    typedef enum
    {
        CALI_DISABLE = 0x00,
        CALI_FORWARD_PREPARE,
        CALI_FORWARD_MEASURE,
        CALI_BACKWARD_RETURN,
        CALI_BACKWARD_GAP_DISMISS,
        CALI_BACKWARD_MEASURE,
        CALI_CALCULATING,
    } State_t;


    explicit EncoderCalibratorBase(Motor* _motor)
    {
        motor = _motor;

        isTriggered = false;
        errorCode = CALI_NO_ERROR;
        state = CALI_DISABLE;
        goPosition = 0;
        rcdX = 0;
        rcdY = 0;
        resultNum = 0;
    }


    bool isTriggered;


    void Tick20kHz();
    void TickMainLoop();


private:
    Motor* motor;

    Error_t errorCode;
    State_t state;
    uint32_t goPosition;
    bool goDirection;
    uint16_t sampleCount = 0;
    uint16_t sampleDataRaw[SAMPLE_COUNTS_PER_STEP]{};
    uint16_t sampleDataAverageForward[MOTOR_ONE_CIRCLE_HARD_STEPS + 1]{};
    uint16_t sampleDataAverageBackward[MOTOR_ONE_CIRCLE_HARD_STEPS + 1]{};
    int32_t rcdX, rcdY;
    uint32_t resultNum;


    void CalibrationDataCheck();
    static uint32_t CycleMod(uint32_t _a, uint32_t _b);
    static int32_t CycleSubtract(int32_t _a, int32_t _b, int32_t _cyc);
    static int32_t CycleAverage(int32_t _a, int32_t _b, int32_t _cyc);
    static int32_t CycleDataAverage(const uint16_t* _data, uint16_t _length, int32_t _cyc);


    /***** Port Specified Implements *****/
    virtual void BeginWriteFlash() = 0;
    virtual void EndWriteFlash() = 0;
    virtual void ClearFlash() = 0;
    virtual void WriteFlash16bitsAppend(uint16_t _data) = 0;
};

#endif
