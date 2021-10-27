#include "ctrl_step.hpp"
#include "communication.hpp"

 const uint32_t CtrlStepMotor::CTRL_CIRCLE_COUNT = 360 / 1.8 * 256;

CtrlStepMotor::CtrlStepMotor(CAN_HandleTypeDef* _hcan, uint8_t _id, bool _inverse, uint8_t _reduction,
                             float _minAngle, float _maxAngle) :
    nodeID(_id), hcan(_hcan), inverseDirection(_inverse), reduction(_reduction), minAngle(_minAngle),
    maxAngle(_maxAngle)
{
    txHeader =
        {
            .StdId = 0,
            .ExtId = 0,
            .IDE = CAN_ID_STD,
            .RTR = CAN_RTR_DATA,
            .DLC = 8,
            .TransmitGlobalTime = DISABLE
        };
}

void CtrlStepMotor::SetEnable(bool _enable)
{
    uint8_t mode = 0x01;
    txHeader.StdId = nodeID << 7 | mode;

    // Int to Bytes
    uint32_t val = _enable ? 1 : 0;
    auto* b = (unsigned char*) &val;
    for (int i = 0; i < 4; i++)
        canBuf[i] = *(b + i);

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}

void CtrlStepMotor::DoCalibration()
{
    uint8_t mode = 0x02;
    txHeader.StdId = nodeID << 7 | mode;

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}

void CtrlStepMotor::SetCurrentSetPoint(float _val)
{
    uint8_t mode = 0x03;
    txHeader.StdId = nodeID << 7 | mode;

    // Float to Bytes
    auto* b = (unsigned char*) &_val;
    for (int i = 0; i < 4; i++)
        canBuf[i] = *(b + i);

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}

void CtrlStepMotor::SetSpeedSetPoint(float _val)
{
    uint8_t mode = 0x04;
    txHeader.StdId = nodeID << 7 | mode;

    // Float to Bytes
    auto* b = (unsigned char*) &_val;
    for (int i = 0; i < 4; i++)
        canBuf[i] = *(b + i);

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}

void CtrlStepMotor::SetPositionSetPoint(float _val)
{
    uint8_t mode = 0x05;
    txHeader.StdId = nodeID << 7 | mode;

    // Float to Bytes
    auto* b = (unsigned char*) &_val;
    for (int i = 0; i < 4; i++)
        canBuf[i] = *(b + i);

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}

void CtrlStepMotor::SetPositionWithTime(float _pos, float _time)
{
    uint8_t mode = 0x06;
    txHeader.StdId = nodeID << 7 | mode;

    // Float to Bytes
    auto* b = (unsigned char*) &_pos;
    for (int i = 0; i < 4; i++)
        canBuf[i] = *(b + i);

    b = (unsigned char*) &_time;
    for (int i = 4; i < 8; i++)
        canBuf[i] = *(b + i - 4);

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}

void CtrlStepMotor::SetNodeID(uint32_t _id)
{
    uint8_t mode = 0x11;
    txHeader.StdId = nodeID << 7 | mode;

    // Int to Bytes
    auto* b = (unsigned char*) &_id;
    for (int i = 0; i < 4; i++)
        canBuf[i] = *(b + i);

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}

void CtrlStepMotor::SetMaxCurrent(float _val)
{
    uint8_t mode = 0x12;
    txHeader.StdId = nodeID << 7 | mode;

    // Float to Bytes
    auto* b = (unsigned char*) &_val;
    for (int i = 0; i < 4; i++)
        canBuf[i] = *(b + i);

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}

void CtrlStepMotor::SetAcceleration(float _val)
{
    uint8_t mode = 0x13;
    txHeader.StdId = nodeID << 7 | mode;

    // Float to Bytes
    auto* b = (unsigned char*) &_val;
    for (int i = 0; i < 4; i++)
        canBuf[i] = *(b + i);

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}

void CtrlStepMotor::SetHomePosition()
{
    uint8_t mode = 0x14;
    txHeader.StdId = nodeID << 7 | mode;

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}

void CtrlStepMotor::SetAutoEnable(bool _enable)
{
    uint8_t mode = 0x15;
    txHeader.StdId = nodeID << 7 | mode;

    // Int to Bytes
    uint32_t val = _enable ? 1 : 0;
    auto* b = (unsigned char*) &val;
    for (int i = 0; i < 4; i++)
        canBuf[i] = *(b + i);

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}

void CtrlStepMotor::Reboot()
{
    uint8_t mode = 0x7f;
    txHeader.StdId = nodeID << 7 | mode;

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}

void CtrlStepMotor::EraseConfigs()
{
    uint8_t mode = 0x7e;
    txHeader.StdId = nodeID << 7 | mode;

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}

void CtrlStepMotor::SetMaxAngle(float _angle)
{
    maxAngle = _angle;
}

void CtrlStepMotor::SetMinAngle(float _angle)
{
    minAngle = _angle;
}

void CtrlStepMotor::SetReduction(uint8_t _val)
{
    reduction = _val;
}

void CtrlStepMotor::SetAngle(float _angle)
{
    _angle = inverseDirection ? -_angle : _angle;
    if (_angle <= maxAngle && _angle >= minAngle)
    {
        float stepMotorCnt = _angle / 360.0f * reduction * CtrlStepMotor::CTRL_CIRCLE_COUNT;
        SetPositionSetPoint(stepMotorCnt);
    }
}

void CtrlStepMotor::SetAngleWithTime(float _angle, float _time)
{
    _angle = inverseDirection ? -_angle : _angle;
    float stepMotorCnt = _angle / 360.0f * reduction * CtrlStepMotor::CTRL_CIRCLE_COUNT;
    SetPositionWithTime(stepMotorCnt, _time);
}

void CtrlStepMotor::UpdateAngle()
{
    uint8_t mode = 0x23;
    txHeader.StdId = nodeID << 7 | mode;

    CanSendMessage(get_can_ctx(hcan), canBuf, &txHeader);
}

void CtrlStepMotor::UpdateAngleCallback(float _step)
{
    float angle = _step / reduction / CtrlStepMotor::CTRL_CIRCLE_COUNT * 360;
    currentAngle = inverseDirection ? -angle : angle;
}

