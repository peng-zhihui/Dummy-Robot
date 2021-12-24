#include "button_stm32.h"
#include <gpio.h>

bool Button::ReadButtonPinIO(uint8_t _id)
{
    switch (_id)
    {
        case 1:
            return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_SET;
        case 2:
            return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_SET;
        default:
            return false;
    }
}
bool Button::IsPressed()
{
    return !ReadButtonPinIO(id);
}
