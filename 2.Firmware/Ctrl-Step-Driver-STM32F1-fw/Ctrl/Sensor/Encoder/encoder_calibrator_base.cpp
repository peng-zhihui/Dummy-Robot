#include <Platform/Memory/stockpile_f103cb.h>
#include <valarray>
#include "encoder_calibrator_base.h"

int32_t EncoderCalibratorBase::CycleDataAverage(const uint16_t* _data, uint16_t _length, int32_t _cyc)
{
    int32_t sumData = 0;
    int32_t subData;
    int32_t diffData;

    sumData += (int32_t) _data[0];
    for (uint16_t i = 1; i < _length; i++)
    {
        diffData = (int32_t) _data[i];
        subData = (int32_t) _data[i] - (int32_t) _data[0];
        if (subData > (_cyc >> 1)) diffData = (int32_t) _data[i] - _cyc;
        if (subData < (-_cyc >> 1)) diffData = (int32_t) _data[i] + _cyc;
        sumData += diffData;
    }

    sumData = sumData / _length;

    if (sumData < 0) sumData += _cyc;
    if (sumData > _cyc) sumData -= _cyc;

    return sumData;
}


void EncoderCalibratorBase::CalibrationDataCheck()
{
    uint32_t count;
    int32_t subData;

    int32_t calibSampleResolution = motor->encoder->RESOLUTION / MOTOR_ONE_CIRCLE_HARD_STEPS;
    for (count = 0; count < MOTOR_ONE_CIRCLE_HARD_STEPS + 1; count++)
    {
        sampleDataAverageForward[count] = (uint16_t) CycleAverage((int32_t) sampleDataAverageForward[count],
                                                                  (int32_t) sampleDataAverageBackward[count],
                                                                  motor->encoder->RESOLUTION);
    }
    subData = CycleSubtract((int32_t) sampleDataAverageForward[0],
                            (int32_t) sampleDataAverageForward[MOTOR_ONE_CIRCLE_HARD_STEPS - 1],
                            motor->encoder->RESOLUTION);
    if (subData == 0)
    {
        errorCode = CALI_ERROR_AVERAGE_DIR;
        return;
    } else
    {
        goDirection = subData > 0;
    }

    for (count = 1; count < MOTOR_ONE_CIRCLE_HARD_STEPS; count++)
    {
        subData = CycleSubtract((int32_t) sampleDataAverageForward[count],
                                (int32_t) sampleDataAverageForward[count - 1],
                                motor->encoder->RESOLUTION);
        if (abs(subData) > (calibSampleResolution * 3 / 2)) // delta-data too large
        {
            errorCode = CALI_ERROR_AVERAGE_CONTINUTY;
            return;
        }
        if (abs(subData) < (calibSampleResolution * 1 / 2)) // delta-data too small
        {
            errorCode = CALI_ERROR_AVERAGE_CONTINUTY;
            return;
        }
        if (subData == 0)
        {
            errorCode = CALI_ERROR_AVERAGE_DIR;
            return;
        }
        if ((subData > 0) && (!goDirection))
        {
            errorCode = CALI_ERROR_AVERAGE_DIR;
            return;
        }
        if ((subData < 0) && (goDirection))
        {
            errorCode = CALI_ERROR_AVERAGE_DIR;
            return;
        }
    }


    uint32_t step_num = 0;
    if (goDirection)
    {
        for (count = 0; count < MOTOR_ONE_CIRCLE_HARD_STEPS; count++)
        {
            subData = (int32_t) sampleDataAverageForward[CycleMod(count + 1, MOTOR_ONE_CIRCLE_HARD_STEPS)] -
                      (int32_t) sampleDataAverageForward[CycleMod(count, MOTOR_ONE_CIRCLE_HARD_STEPS)];
            if (subData < 0)
            {
                step_num++;
                rcdX = (int32_t) count;
                rcdY = (motor->encoder->RESOLUTION - 1) -
                       sampleDataAverageForward[CycleMod(rcdX, MOTOR_ONE_CIRCLE_HARD_STEPS)];
            }
        }
        if (step_num != 1)
        {
            errorCode = CALI_ERROR_PHASE_STEP;
            return;
        }
    } else
    {
        for (count = 0; count < MOTOR_ONE_CIRCLE_HARD_STEPS; count++)
        {
            subData = (int32_t) sampleDataAverageForward[CycleMod(count + 1, MOTOR_ONE_CIRCLE_HARD_STEPS)] -
                      (int32_t) sampleDataAverageForward[CycleMod(count, MOTOR_ONE_CIRCLE_HARD_STEPS)];
            if (subData > 0)
            {
                step_num++;
                rcdX = (int32_t) count;
                rcdY = (motor->encoder->RESOLUTION - 1) -
                       sampleDataAverageForward[CycleMod(rcdX + 1, MOTOR_ONE_CIRCLE_HARD_STEPS)];
            }
        }
        if (step_num != 1)
        {
            errorCode = CALI_ERROR_PHASE_STEP;
            return;
        }
    }

    errorCode = CALI_NO_ERROR;
}


void EncoderCalibratorBase::Tick20kHz()
{
    motor->encoder->UpdateAngle();

    switch (state)
    {
        case CALI_DISABLE:
            if (isTriggered)
            {
                motor->driver->SetFocCurrentVector(goPosition, motor->config.motionParams.caliCurrent);
                goPosition = motor->MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS;
                sampleCount = 0;
                state = CALI_FORWARD_PREPARE;
                errorCode = CALI_NO_ERROR;
            }
            break;
        case CALI_FORWARD_PREPARE:
            goPosition += AUTO_CALIB_SPEED;
            motor->driver->SetFocCurrentVector(goPosition, motor->config.motionParams.caliCurrent);
            if (goPosition == 2 * motor->MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS)
            {
                goPosition = motor->MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS;
                state = CALI_FORWARD_MEASURE;
            }
            break;
        case CALI_FORWARD_MEASURE:
            if ((goPosition % motor->SOFT_DIVIDE_NUM) == 0)
            {
                sampleDataRaw[sampleCount++] = motor->encoder->angleData.rawAngle;
                if (sampleCount == EncoderCalibratorBase::SAMPLE_COUNTS_PER_STEP)
                {
                    sampleDataAverageForward[(goPosition - motor->MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS) /
                                             motor->SOFT_DIVIDE_NUM]
                        = CycleDataAverage(sampleDataRaw, EncoderCalibratorBase::SAMPLE_COUNTS_PER_STEP,
                                           motor->encoder->RESOLUTION);

                    sampleCount = 0;
                    goPosition += FINE_TUNE_CALIB_SPEED;
                }
            } else
            {
                goPosition += FINE_TUNE_CALIB_SPEED;
            }

            motor->driver->SetFocCurrentVector(goPosition, motor->config.motionParams.caliCurrent);

            if (goPosition > (2 * motor->MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS))
            {
                state = CALI_BACKWARD_RETURN;
            }
            break;
        case CALI_BACKWARD_RETURN:
            goPosition += FINE_TUNE_CALIB_SPEED;
            motor->driver->SetFocCurrentVector(goPosition, motor->config.motionParams.caliCurrent);

            if (goPosition ==
                (2 * motor->MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS + motor->SOFT_DIVIDE_NUM * 20))
            {
                state = CALI_BACKWARD_GAP_DISMISS;
            }
            break;
        case CALI_BACKWARD_GAP_DISMISS:
            goPosition -= FINE_TUNE_CALIB_SPEED;
            motor->driver->SetFocCurrentVector(goPosition, motor->config.motionParams.caliCurrent);
            if (goPosition == (2 * motor->MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS))
            {
                state = CALI_BACKWARD_MEASURE;
            }
            break;
        case CALI_BACKWARD_MEASURE:
            if ((goPosition % motor->SOFT_DIVIDE_NUM) == 0)
            {
                sampleDataRaw[sampleCount++] = motor->encoder->angleData.rawAngle;
                if (sampleCount == EncoderCalibratorBase::SAMPLE_COUNTS_PER_STEP)
                {
                    sampleDataAverageBackward[(goPosition - motor->MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS) /
                                              motor->SOFT_DIVIDE_NUM]
                        = CycleDataAverage(sampleDataRaw, EncoderCalibratorBase::SAMPLE_COUNTS_PER_STEP,
                                           motor->encoder->RESOLUTION);

                    sampleCount = 0;
                    goPosition -= FINE_TUNE_CALIB_SPEED;
                }
            } else
            {
                goPosition -= FINE_TUNE_CALIB_SPEED;
            }
            motor->driver->SetFocCurrentVector(goPosition, motor->config.motionParams.caliCurrent);
            if (goPosition < motor->MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS)
            {
                state = CALI_CALCULATING;
            }
            break;
        case CALI_CALCULATING:
            motor->driver->SetFocCurrentVector(0, 0);
            break;
        default:
            break;
    }
}


void EncoderCalibratorBase::TickMainLoop()
{
    int32_t dataI32;
    uint16_t dataU16;

    if (state != CALI_CALCULATING)
        return;

    motor->driver->Sleep();

    CalibrationDataCheck();

    if (errorCode == CALI_NO_ERROR)
    {
        int32_t stepX, stepY;
        resultNum = 0;

        ClearFlash();
        BeginWriteFlash();

        if (goDirection)
        {
            for (stepX = rcdX; stepX < rcdX + motor->MOTOR_ONE_CIRCLE_HARD_STEPS + 1; stepX++)
            {
                dataI32 = CycleSubtract(
                    sampleDataAverageForward[CycleMod(stepX + 1, motor->MOTOR_ONE_CIRCLE_HARD_STEPS)],
                    sampleDataAverageForward[CycleMod(stepX, motor->MOTOR_ONE_CIRCLE_HARD_STEPS)],
                    motor->encoder->RESOLUTION);
                if (stepX == rcdX)
                {
                    for (stepY = rcdY; stepY < dataI32; stepY++)
                    {
                        dataU16 = CycleMod(motor->SOFT_DIVIDE_NUM * stepX +
                                           motor->SOFT_DIVIDE_NUM * stepY / dataI32,
                                           motor->MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS);
                        WriteFlash16bitsAppend(dataU16);
                        resultNum++;
                    }
                } else if (stepX == rcdX + motor->MOTOR_ONE_CIRCLE_HARD_STEPS)
                {
                    for (stepY = 0; stepY < rcdY; stepY++)
                    {
                        dataU16 = CycleMod(motor->SOFT_DIVIDE_NUM * stepX +
                                           motor->SOFT_DIVIDE_NUM * stepY / dataI32,
                                           motor->MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS);
                        WriteFlash16bitsAppend(dataU16);
                        resultNum++;
                    }
                } else
                {
                    for (stepY = 0; stepY < dataI32; stepY++)
                    {
                        dataU16 = CycleMod(motor->SOFT_DIVIDE_NUM * stepX +
                                           motor->SOFT_DIVIDE_NUM * stepY / dataI32,
                                           motor->MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS);
                        WriteFlash16bitsAppend(dataU16);
                        resultNum++;
                    }
                }
            }
        } else
        {
            for (stepX = rcdX + motor->MOTOR_ONE_CIRCLE_HARD_STEPS; stepX > rcdX - 1; stepX--)
            {
                dataI32 = CycleSubtract(
                    sampleDataAverageForward[CycleMod(stepX, motor->MOTOR_ONE_CIRCLE_HARD_STEPS)],
                    sampleDataAverageForward[CycleMod(stepX + 1, motor->MOTOR_ONE_CIRCLE_HARD_STEPS)],
                    motor->encoder->RESOLUTION);
                if (stepX == rcdX + motor->MOTOR_ONE_CIRCLE_HARD_STEPS)
                {
                    for (stepY = rcdY; stepY < dataI32; stepY++)
                    {
                        dataU16 = CycleMod(
                            motor->SOFT_DIVIDE_NUM * (stepX + 1) -
                            motor->SOFT_DIVIDE_NUM * stepY / dataI32,
                            motor->MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS);
                        WriteFlash16bitsAppend(dataU16);
                        resultNum++;
                    }
                } else if (stepX == rcdX)
                {
                    for (stepY = 0; stepY < rcdY; stepY++)
                    {
                        dataU16 = CycleMod(
                            motor->SOFT_DIVIDE_NUM * (stepX + 1) -
                            motor->SOFT_DIVIDE_NUM * stepY / dataI32,
                            motor->MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS);
                        WriteFlash16bitsAppend(dataU16);
                        resultNum++;
                    }
                } else
                {
                    for (stepY = 0; stepY < dataI32; stepY++)
                    {
                        dataU16 = CycleMod(
                            motor->SOFT_DIVIDE_NUM * (stepX + 1) -
                            motor->SOFT_DIVIDE_NUM * stepY / dataI32,
                            motor->MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS);
                        WriteFlash16bitsAppend(dataU16);
                        resultNum++;
                    }
                }
            }
        }

        EndWriteFlash();

        if (resultNum != motor->encoder->RESOLUTION)
            errorCode = CALI_ERROR_ANALYSIS_QUANTITY;
    }

    if (errorCode == CALI_NO_ERROR)
    {
        motor->encoder->angleData.rectifyValid = true;
    } else
    {
        motor->encoder->angleData.rectifyValid = false;
        ClearFlash();
    }

    motor->controller->isStalled = true;

    state = CALI_DISABLE;
    isTriggered = false;

    if (errorCode == CALI_NO_ERROR)
        HAL_NVIC_SystemReset();
}


uint32_t EncoderCalibratorBase::CycleMod(uint32_t _a, uint32_t _b)
{
    return (_a + _b) % _b;
}


int32_t EncoderCalibratorBase::CycleSubtract(int32_t _a, int32_t _b, int32_t _cyc)
{
    int32_t sub_data;

    sub_data = _a - _b;
    if (sub_data > (_cyc >> 1)) sub_data -= _cyc;
    if (sub_data < (-_cyc >> 1)) sub_data += _cyc;
    return sub_data;
}


int32_t EncoderCalibratorBase::CycleAverage(int32_t _a, int32_t _b, int32_t _cyc)
{
    int32_t sub_data;
    int32_t ave_data;

    sub_data = _a - _b;
    ave_data = (_a + _b) >> 1;

    if (abs(sub_data) > (_cyc >> 1))
    {
        if (ave_data >= (_cyc >> 1)) ave_data -= (_cyc >> 1);
        else ave_data += (_cyc >> 1);
    }
    return ave_data;
}