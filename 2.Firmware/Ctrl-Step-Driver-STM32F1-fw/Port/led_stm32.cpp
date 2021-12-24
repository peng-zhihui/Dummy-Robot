#include "led_stm32.h"
#include <gpio.h>

void Led::SetLedState(uint8_t _id, bool _state)
{
    switch (_id)
    {
        case 0:
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, _state ? GPIO_PIN_RESET : GPIO_PIN_SET);
            break;
        case 1:
            HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, _state ? GPIO_PIN_RESET : GPIO_PIN_SET);
            break;

        default:
            break;
    }
}
