#ifndef REF_STM32F4_PWM_H
#define REF_STM32F4_PWM_H

#include <cstdint>
#include <tim.h>

class PWM
{
private:

public:
    enum PwmChannel_t
    {
        CH_ALL,
        // TIM
        CH_A1,
        CH_A2,
        CH_B1,
        CH_B2
    };

    explicit PWM(uint32_t _freqHzA = 21000, uint32_t _freqHzB = 21000);

    void Start(PwmChannel_t _channel = CH_ALL);

    void SetDuty(PwmChannel_t _channel = CH_ALL, float _duty = 0);

};

#endif //REF_STM32F4_PWM_H
