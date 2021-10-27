#ifndef DUMMY_CORE_FW_CTRL_STEP_HPP
#define DUMMY_CORE_FW_CTRL_STEP_HPP

#include "fibre/protocol.hpp"
#include "can.h"

class CtrlStepMotor
{
private:
    CAN_HandleTypeDef* hcan;
    uint8_t canBuf[8];
    CAN_TxHeaderTypeDef txHeader;

public:
    static const uint32_t CTRL_CIRCLE_COUNT;

    CtrlStepMotor(CAN_HandleTypeDef* _hcan, uint8_t _id, bool _inverse = false, uint8_t _reduction = 1,
                  float _minAngle = -180, float _maxAngle = 180);

    uint8_t nodeID;
    bool inverseDirection;
    uint8_t reduction;
    float currentAngle;
    float maxAngle;
    float minAngle;

    void SetMaxAngle(float _angle);
    void SetMinAngle(float _angle);
    void SetReduction(uint8_t _val);

    void SetAngle(float _angle);
    void SetAngleWithTime(float _angle, float _time);

    // CAN Command
    void SetEnable(bool _enable);
    void DoCalibration();
    void SetCurrentSetPoint(float _val);
    void SetSpeedSetPoint(float _val);
    void SetPositionSetPoint(float _val);
    void SetPositionWithTime(float _pos, float _time);
    void SetNodeID(uint32_t _id);
    void SetMaxCurrent(float _val);
    void SetAcceleration(float _val);
    void SetHomePosition();
    void SetAutoEnable(bool _enable);
    void Reboot();
    void EraseConfigs();

    void UpdateAngle();
    void UpdateAngleCallback(float _step);

    // Communication protocol definitions
    auto MakeProtocolDefinitions()
    {
        return make_protocol_member_list(
            make_protocol_function("reboot", *this, &CtrlStepMotor::Reboot),
            make_protocol_function("erase_configs", *this, &CtrlStepMotor::EraseConfigs),
            make_protocol_function("set_enable", *this, &CtrlStepMotor::SetEnable, "enable"),
            make_protocol_function("set_position_with_time", *this, &CtrlStepMotor::SetPositionWithTime,
                                   "pos", "time"),
            make_protocol_function("set_position", *this, &CtrlStepMotor::SetPositionSetPoint, "pos"),
            make_protocol_function("set_velocity", *this, &CtrlStepMotor::SetSpeedSetPoint, "vel"),
            make_protocol_function("set_current", *this, &CtrlStepMotor::SetCurrentSetPoint, "current"),
            make_protocol_function("set_current_limit", *this, &CtrlStepMotor::SetMaxCurrent, "current"),
            make_protocol_function("set_node_id", *this, &CtrlStepMotor::SetNodeID, "id"),
            make_protocol_function("set_acceleration", *this, &CtrlStepMotor::SetAcceleration, "acc"),
            make_protocol_function("apply_offset", *this, &CtrlStepMotor::SetHomePosition),
            make_protocol_function("do_calibration", *this, &CtrlStepMotor::DoCalibration),
            make_protocol_function("set_auto_enable", *this, &CtrlStepMotor::SetAutoEnable, "enable"),
            make_protocol_function("goto_angle", *this, &CtrlStepMotor::SetAngle, "angle"),
            make_protocol_function("goto_angle_with_time", *this, &CtrlStepMotor::SetAngleWithTime, "angle",
                                   "time"),
            make_protocol_function("update_angle", *this, &CtrlStepMotor::UpdateAngle)
        );
    }

};

#endif //DUMMY_CORE_FW_CTRL_STEP_HPP
