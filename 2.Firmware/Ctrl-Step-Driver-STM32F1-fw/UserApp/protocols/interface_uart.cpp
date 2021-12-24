#include "common_inc.h"
#include "configurations.h"

extern Motor motor;

void OnUartCmd(uint8_t* _data, uint16_t _len)
{
    float cur, pos, vel, time;
    int ret = 0;

    switch (_data[0])
    {
        case 'c':
            ret = sscanf((char*) _data, "c %f", &cur);
            if (ret < 1)
            {
                printf("[error] Command format error!\r\n");
            } else if (ret == 1)
            {
                if (motor.controller->modeRunning != Motor::MODE_COMMAND_CURRENT)
                    motor.controller->SetCtrlMode(Motor::MODE_COMMAND_CURRENT);
                motor.controller->SetCurrentSetPoint((int32_t) (cur * 1000));
            }
            break;
        case 'v':
            ret = sscanf((char*) _data, "v %f", &vel);
            if (ret < 1)
            {
                printf("[error] Command format error!\r\n");
            } else if (ret == 1)
            {
                if (motor.controller->modeRunning != Motor::MODE_COMMAND_VELOCITY)
                {
                    motor.config.motionParams.ratedVelocity = boardConfig.velocityLimit;
                    motor.controller->SetCtrlMode(Motor::MODE_COMMAND_VELOCITY);
                }
                motor.controller->SetVelocitySetPoint(
                    (int32_t) (vel * (float) motor.MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS));
            }
            break;
        case 'p':
            ret = sscanf((char*) _data, "p %f", &pos);
            if (ret < 1)
            {
                printf("[error] Command format error!\r\n");
            } else if (ret == 1)
            {
                if (motor.controller->modeRunning != Motor::MODE_COMMAND_POSITION)
                    motor.controller->requestMode = Motor::MODE_COMMAND_POSITION;

                motor.controller->SetPositionSetPoint(
                    (int32_t) (pos * (float) motor.MOTOR_ONE_CIRCLE_SUBDIVIDE_STEPS));
            }
            break;
    }
}

