#include "time_utils.h"

__STATIC_INLINE uint32_t LL_SYSTICK_IsActiveCounterFlag(void)
{
    //判断COUNTFLAG位是否为1，1则计数器已经递减到0了至少一次。读取该位后该位自动清零。
    return ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == (SysTick_CTRL_COUNTFLAG_Msk));
}

uint32_t micros(void)
{
    /* Ensure COUNTFLAG is reset by reading SysTick control and status register */
    LL_SYSTICK_IsActiveCounterFlag();           //清除计数器"溢出"标志位
    uint32_t m = HAL_GetTick();
    const uint32_t tms = SysTick->LOAD + 1;
    __IO uint32_t u = tms - SysTick->VAL;
    if (LL_SYSTICK_IsActiveCounterFlag())
    {
        m = HAL_GetTick();
        u = tms - SysTick->VAL;
    }
    return (m * 1000 + (u * 1000) / tms);
}

uint32_t millis(void)
{
    return HAL_GetTick();
}

void delayMicroseconds(uint32_t us)
{
    us *= 23;
    while (us--)
        __NOP();
}

