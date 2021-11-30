#ifndef REF_STM32F4_ENCODER_HPP
#define REF_STM32F4_ENCODER_HPP

#include <fibre/protocol.hpp>
#include "tim.h"

class Encoder
{
private:
    const float RAD_TO_DEG = 57.295777754771045f;

    TIM_HandleTypeDef *htim;

public:
    explicit Encoder(TIM_HandleTypeDef *_htim, uint16_t _cpr = 4096, bool _inverse = false);

    void Start();

    int64_t GetCount();

    float GetAngle(bool _useRAD = false);

    struct Config_t
    {
        uint16_t cpr;

        bool inverse;
    };

    // Communication protocol definitions
    auto MakeProtocolDefinitions()
    {
        return make_protocol_member_list(
            make_protocol_object("config",
                                 make_protocol_property("cpr", &config.cpr),
                                 make_protocol_property("inverse", &config.inverse)
            ),
            make_protocol_function("get_count", *this, &Encoder::GetCount),
            make_protocol_function("get_angle", *this, &Encoder::GetAngle, "use_rad")
        );
    }

    Config_t config;
};

#endif //REF_STM32F4_ENCODER_HPP
