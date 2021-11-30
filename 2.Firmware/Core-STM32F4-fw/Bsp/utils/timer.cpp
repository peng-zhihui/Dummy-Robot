#include "timer.hpp"

static TimerCallback_t timerCallbacks[5];

Timer::Timer(TIM_HandleTypeDef *_htim, uint32_t _freqHz)
{
    htim7.Instance = TIM7;
    htim10.Instance = TIM10;
    htim11.Instance = TIM11;
    htim13.Instance = TIM13;
    htim14.Instance = TIM14;

    if (!(_htim->Instance == TIM7 ||
          _htim->Instance == TIM10 ||
          _htim->Instance == TIM11 ||
          _htim->Instance == TIM13 ||
          _htim->Instance == TIM14))
    {
        Error_Handler();
    }

    if (_freqHz < 1) _freqHz = 1;
    else if (_freqHz > 10000000) _freqHz = 10000000;

    htim = _htim;
    freq = _freqHz;

    CalcRegister(freq);

    HAL_TIM_Base_DeInit(_htim);
    _htim->Init.Prescaler = PSC - 1;
    _htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    _htim->Init.Period = ARR - 1;
    _htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    _htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(_htim) != HAL_OK)
    {
        Error_Handler();
    }
}

void Timer::Start()
{
    HAL_TIM_Base_Start_IT(htim);
}

void Timer::CalcRegister(uint32_t _freq)
{
    float psc = 0.5;
    float arr;

    do
    {
        psc *= 2;
        arr = 84000000.0f / psc / (float) _freq;
    } while (arr > 65535);

    if (htim->Instance == TIM7 || htim->Instance == TIM13 || htim->Instance == TIM14) // APB1 @84MHz
    {
        PSC = (uint16_t) round((double) psc);
        ARR = (uint16_t) (84000000.0f / (float) _freq / psc);
    } else if (htim->Instance == TIM10 || htim->Instance == TIM11) // APB2 @168MHz
    {
        PSC = (uint16_t) round((double) psc) * 2;
        ARR = (uint16_t) (84000000.0f / (float) _freq / psc);
    }
}


void Timer::SetCallback(TimerCallback_t _timerCallback)
{
    if (htim->Instance == TIM7)
    {
        timerCallbacks[0] = _timerCallback;
    } else if (htim->Instance == TIM10)
    {
        timerCallbacks[1] = _timerCallback;
    } else if (htim->Instance == TIM11)
    {
        timerCallbacks[2] = _timerCallback;
    } else if (htim->Instance == TIM13)
    {
        timerCallbacks[3] = _timerCallback;
    } else if (htim->Instance == TIM14)
    {
        timerCallbacks[4] = _timerCallback;
    }
}


extern "C"
void OnTimerCallback(TIM_TypeDef *timInstance)
{
    if (timInstance == TIM7)
    {
        timerCallbacks[0]();
    } else if (timInstance == TIM10)
    {
        timerCallbacks[1]();
    } else if (timInstance == TIM11)
    {
        timerCallbacks[2]();
    } else if (timInstance == TIM13)
    {
        timerCallbacks[3]();
    } else if (timInstance == TIM14)
    {
        timerCallbacks[4]();
    }
}