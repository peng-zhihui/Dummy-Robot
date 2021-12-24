#include "led_base.h"

void LedBase::Tick(uint32_t _timeElapseMillis, Motor::State_t _state)
{
    timer += _timeElapseMillis;

    switch (_state)
    {
        case Motor::STATE_NO_CALIB:
            motorEnable = false;
            heartBeatEnable = false;
            targetBlinkNum = 1;
            break;
        case Motor::STATE_RUNNING:
            motorEnable = true;
            heartBeatEnable = true;
            targetBlinkNum = 0;
            break;
        case Motor::STATE_FINISH:
            motorEnable = true;
            heartBeatEnable = false;
            targetBlinkNum = 0;
            break;
        case Motor::STATE_STOP:
            motorEnable = false;
            heartBeatEnable = false;
            targetBlinkNum = 0;
            break;
        case Motor::STATE_OVERLOAD:
            motorEnable = true;
            heartBeatEnable = false;
            targetBlinkNum = 3;
            break;
        case Motor::STATE_STALL:
            motorEnable = false;
            heartBeatEnable = false;
            targetBlinkNum = 2;
            break;
    }

    if (motorEnable)
        if (heartBeatEnable)
        {
            switch (heartBeatPhase)
            {
                case 1:
                    if (timer - timerHeartBeat > 100)
                    {
                        SetLedState(0, false);
                        timerHeartBeat = timer;
                        heartBeatPhase = 2;
                    }
                    break;
                case 2:
                    if (timer - timerHeartBeat > 100)
                    {
                        SetLedState(0, true);
                        timerHeartBeat = timer;
                        heartBeatPhase = 3;
                    }
                    break;
                case 3:
                    if (timer - timerHeartBeat > 100)
                    {
                        SetLedState(0, false);
                        timerHeartBeat = timer;
                        heartBeatPhase = 4;
                    }
                    break;
                case 4:
                    if (timer - timerHeartBeat > 700)
                    {
                        SetLedState(0, true);
                        timerHeartBeat = timer;
                        heartBeatPhase = 1;
                    }
                    break;
            }
        } else
        {
            SetLedState(0, true);
            heartBeatPhase = 1;
        }
    else
        SetLedState(0, false);


    switch (blinkPhase)
    {
        case 1:
            if (timer - timerBlink > 100)
            {
                SetLedState(1, false);
                timerBlink = timer;
                blinkPhase = 2;
            }
            break;
        case 2:
            if (timer - timerBlink > 100)
            {
                blinkNum++;
                if (targetBlinkNum > blinkNum)
                {
                    SetLedState(1, true);
                    blinkPhase = 1;
                    timerBlink = timer;
                } else
                {
                    SetLedState(1, false);
                    blinkPhase = 3;
                    timerBlink = timer;
                }
            }
            break;
        case 3:
            if (timer - timerBlink > 1000)
            {
                blinkNum = 0;
                SetLedState(1, targetBlinkNum > 0);
                timerBlink = timer;
                blinkPhase = 1;
            }
            break;
    }

}


