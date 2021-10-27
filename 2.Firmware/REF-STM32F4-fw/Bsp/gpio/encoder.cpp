#include "encoder.hpp"

Encoder::Encoder(TIM_HandleTypeDef *_htim, uint16_t _cpr, bool _inverse) :
    htim(_htim), config(Config_t{})
{
    config.cpr = _cpr;
    config.inverse = _inverse;
}

int64_t Encoder::GetCount()
{
    int64_t count = GetCntLoop(htim->Instance) * 65536 + htim->Instance->CNT;

    return config.inverse ? -count : count;
}

float Encoder::GetAngle(bool _useRAD)
{
    float angle = (float) GetCount() / (float) config.cpr;

    return _useRAD ? angle / RAD_TO_DEG : angle;
}

void Encoder::Start()
{
    ClearCntLoop(htim->Instance);
    htim->Instance->CNT = 0;
    HAL_TIM_Encoder_Start_IT(htim, TIM_CHANNEL_ALL);
}
