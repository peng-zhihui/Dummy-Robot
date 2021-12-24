#ifndef CTRL_STEP_FW_LED_STM32_H
#define CTRL_STEP_FW_LED_STM32_H

#include "led_base.h"


class Led :public LedBase
{

private:
    void SetLedState(uint8_t _id, bool _state) override;

};

#endif
