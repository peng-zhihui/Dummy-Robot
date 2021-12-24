#include "button_base.h"

void ButtonBase::Tick(uint32_t _timeElapseMillis)
{
    timer += _timeElapseMillis;
    bool pinIO = ReadButtonPinIO(id);

    if (lastPinIO != pinIO)
    {
        if (pinIO)
        {
            OnEventFunc(UP);
            if (timer - pressTime > longPressTime)
                OnEventFunc(LONG_PRESS);
            else
                OnEventFunc(CLICK);
        } else
        {
            OnEventFunc(DOWN);
            pressTime = timer;
        }

        lastPinIO = pinIO;
    }
}

void ButtonBase::SetOnEventListener(void (* _callback)(Event))
{
    lastPinIO =  ReadButtonPinIO(id);

    OnEventFunc = _callback;
}
