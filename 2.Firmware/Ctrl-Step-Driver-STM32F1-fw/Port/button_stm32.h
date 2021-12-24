#ifndef CTRL_STEP_FW_BUTTON_STM32_H
#define CTRL_STEP_FW_BUTTON_STM32_H

#include "button_base.h"

class Button : public ButtonBase
{
public:
    explicit Button(uint8_t _id) : ButtonBase(_id)
    {}

    Button(uint8_t _id, uint32_t _longPressTime) : ButtonBase(_id, _longPressTime)
    {}

    bool IsPressed();

private:
    bool ReadButtonPinIO(uint8_t _id) override;
};

#endif
