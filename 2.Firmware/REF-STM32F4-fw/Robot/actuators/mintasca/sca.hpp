#ifndef REF_STM32F4_SCA_HPP
#define REF_STM32F4_SCA_HPP

#include "sca_api.h"
#include "fibre/protocol.hpp"
#include <can.h>

class SCA
{
private:
    uint8_t id;
    float reductionRatio;

    CAN_Handler_t canHandler;
    bool runCmdBlock = false;
    CAN_HandleTypeDef *hcan;

public:
    static uint8_t data[2][8];
    static CAN_TxHeaderTypeDef txHeader[2];
    static uint8_t Can1SendMsg(uint8_t _id, uint8_t *_msg, uint8_t _len);
    static uint8_t Can2SendMsg(uint8_t _id, uint8_t *_msg, uint8_t _len);
    static CanRxMsg RxMsg;
    static void OnCanReceiveMsg(CAN_RxHeaderTypeDef *_rxHeader, uint8_t *_data);

    SCA(CAN_HandleTypeDef *_hcan, uint8_t _id, float _reductionRatio = 36);

    SCA_Handler_t *scaHandler;

    bool Init(bool _enableActuator = true);
    void LookupActuators();
    void SetupActuators();

    /***************控制相关******************/
    void Homing();
    void ResetController();
    void RegainAttributes();
    bool IsOnline();
    bool IsEnable();
    bool IsUpdate();
    bool EnableActuator();
    bool DisableActuator();
    bool SetMode(uint8_t _actuatorMode);
    uint8_t GetMode();
    uint16_t GetErrorCode();
    bool ClearError();
    bool SaveAllParams();

    /***************位置相关******************/
    void SetPosition(float _pos);
    float GetPosition();
    void SetPositionKp(float _kp);
    float GetPositionKp();
    void SetPositionKi(float _ki);
    float GetPositionKi();
    void SetPositionUmax(float _max);
    float GetPositionUmax();
    void SetPositionUmin(float _min);
    float GetPositionUmin();
    void SetPositionOffSet(float _offSet);
    float GetPositionOffSet();
    void SetMaximumPosition(float _maxPos);
    float GetMaximumPosition();
    void SetMinimumPosition(float _minPos);
    float GetMinimumPosition();
    uint8_t EnablePositionLimit(uint8_t _enable);
    uint8_t IsPositionLimitEnable();
    uint8_t SetHomingPosition(float _homingPos);
    uint8_t EnablePositionFilter(uint8_t _enable);
    uint8_t IsPositionFilterEnable();
    void SetPositionCutoffFrequency(float _frequency);
    float GetPositionCutoffFrequency();
    uint8_t ClearHomingInfo();
    uint8_t SetProfilePositionAcceleration(float _acceleration);
    float GetProfilePositionAcceleration();
    uint8_t SetProfilePositionDeceleration(float _deceleration);
    float GetProfilePositionDeceleration();
    uint8_t SetProfilePositionMaxVelocity(float _maxVelocity);
    float GetProfilePositionMaxVelocity();

    /***************速度相关******************/
    void SetVelocity(float _vel);
    float GetVelocity();
    uint8_t GetVelocityKp();
    uint8_t SetVelocityKp(float _kp);
    uint8_t GetVelocityKi();
    uint8_t SetVelocityKi(float _ki);
    uint8_t GetVelocityUmax();
    uint8_t SetVelocityUmax(float _max);
    uint8_t GetVelocityUmin();
    uint8_t SetVelocityUmin(float _min);
    uint8_t EnableVelocityFilter(uint8_t _enable);
    uint8_t IsVelocityFilterEnable();
    uint8_t GetVelocityCutoffFrequency();
    uint8_t SetVelocityCutoffFrequency(float _frequency);
    uint8_t SetVelocityLimit(float _limit);
    uint8_t GetVelocityLimit();
    uint8_t SetProfileVelocityAcceleration(float _acceleration);
    uint8_t GetProfileVelocityAcceleration();
    uint8_t SetProfileVelocityDeceleration(float _deceleration);
    uint8_t GetProfileVelocityDeceleration();
    uint8_t SetProfileVelocityMaxVelocity(float _maxVelocity);
    uint8_t GetProfileVelocityMaxVelocity();
    float GetVelocityRange();

    /***************电流相关******************/
    uint8_t SetCurrent(float _current);
    uint8_t GetCurrent();
    uint8_t GetCurrentKp();
    uint8_t GetCurrentKi();
    uint8_t GetCurrentRange();
    uint8_t EnableCurrentFilter(uint8_t _enable);
    uint8_t IsCurrentFilterEnable();
    uint8_t GetCurrentCutoffFrequency();
    uint8_t SetCurrentCutoffFrequency(float _frequency);
    uint8_t SetCurrentLimit(float _limit);
    uint8_t GetCurrentLimit();

    /***************其他参数******************/
    uint8_t GetVoltage();
    uint8_t GetLockEnergy();
    uint8_t SetLockEnergy(float _energy);
    uint8_t GetActuatorSerialNumber();
    uint8_t GetMotorTemperature();
    uint8_t GetInverterTemperature();
    uint8_t GetMotorProtectedTemperature();
    uint8_t SetMotorProtectedTemperature(float _temp);
    uint8_t GetMotorRecoveryTemperature();
    uint8_t SetMotorRecoveryTemperature(float _temp);
    uint8_t GetInverterProtectedTemperature();
    uint8_t SetInverterProtectedTemperature(float _temp);
    uint8_t GetInverterRecoveryTemperature();
    uint8_t SetInverterRecoveryTemperature(float _temp);
    uint8_t SetActuatorID(uint8_t currEntID, uint8_t _newId);
    uint8_t GetActuatorLastState();
    uint8_t RequestCVPValue();

    // Communication protocol definitions
    auto MakeProtocolDefinitions()
    {
        return make_protocol_member_list(
            make_protocol_function("enable", *this, &SCA::EnableActuator),
            make_protocol_function("disable", *this, &SCA::DisableActuator),
            make_protocol_function("homing", *this, &SCA::Homing),
            make_protocol_function("get_error_code", *this, &SCA::GetErrorCode),
            make_protocol_function("clear_error", *this, &SCA::ClearError),
            make_protocol_function("is_online", *this, &SCA::IsOnline),
            make_protocol_function("is_enable", *this, &SCA::IsEnable),
            make_protocol_function("set_mode", *this, &SCA::SetMode, "mode"),
            make_protocol_function("get_mode", *this, &SCA::GetMode),
            make_protocol_function("set_position", *this, &SCA::SetPosition, "pos"),
            make_protocol_function("get_position", *this, &SCA::GetPosition),
            make_protocol_function("set_velocity", *this, &SCA::SetVelocity, "vel"),
            make_protocol_function("get_velocity", *this, &SCA::GetVelocity),
            make_protocol_function("set_current", *this, &SCA::SetCurrent, "current"),
            make_protocol_function("get_current", *this, &SCA::GetCurrent)
        );
    }
};

#endif //REF_STM32F4_SCA_HPP
