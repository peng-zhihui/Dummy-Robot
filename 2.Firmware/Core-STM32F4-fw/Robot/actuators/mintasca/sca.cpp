#include "sca.hpp"

/*----- NEED TO MODIFY THESE FUNCTIONS TO ADAPT DIFFERENT PLATFORMS ¡ý -----*/
// As well should modify Defines in sca_api.h

uint8_t SCA::Can1SendMsg(uint8_t _id, uint8_t *_msg, uint8_t _len)
{
    SCA::txHeader[0].StdId = _id;
    SCA::txHeader[0].ExtId = _id;
    SCA::txHeader[0].IDE = 0;
    SCA::txHeader[0].RTR = 0;
    SCA::txHeader[0].DLC = _len;
    for (int i = 0; i < _len; i++)
        SCA::data[0][i] = _msg[i];

    HAL_StatusTypeDef re;
    int i = 0xFFF;
    do
        re = HAL_CAN_AddTxMessage(&hcan1, &SCA::txHeader[0], SCA::data[0], nullptr);
    while (re == HAL_ERROR && (i--));

    return 0;
}

uint8_t SCA::Can2SendMsg(uint8_t _id, uint8_t *_msg, uint8_t _len)
{
    SCA::txHeader[1].StdId = _id;
    SCA::txHeader[1].ExtId = _id;
    SCA::txHeader[1].IDE = 0;
    SCA::txHeader[1].RTR = 0;
    SCA::txHeader[1].DLC = _len;
    for (int i = 0; i < _len; i++)
        SCA::data[1][i] = _msg[i];

    HAL_StatusTypeDef re;
    int i = 0xFFF;
    do
        re = HAL_CAN_AddTxMessage(&hcan2, &SCA::txHeader[1], SCA::data[1], nullptr);
    while (re == HAL_ERROR && (i--));

    return 0;
}

void SCA::OnCanReceiveMsg(CAN_RxHeaderTypeDef *_rxHeader, uint8_t *_data)
{
    SCA::RxMsg.DLC = _rxHeader->DLC;
    SCA::RxMsg.StdId = _rxHeader->StdId;
    SCA::RxMsg.ExtId = _rxHeader->ExtId;
    SCA::RxMsg.FMI = _rxHeader->FilterMatchIndex;
    SCA::RxMsg.IDE = _rxHeader->IDE;
    SCA::RxMsg.RTR = _rxHeader->RTR;
    for (int i = 0; i < 8; i++)
        SCA::RxMsg.Data[i] = _data[i];

    // Should be invoked when got CAN responds.
    canDispatch(&SCA::RxMsg);
}

/*------------------------------------------------------------------------------*/

// Static class fields.
uint8_t SCA::data[2][8];
CAN_TxHeaderTypeDef SCA::txHeader[2];
CanRxMsg SCA::RxMsg;

SCA::SCA(CAN_HandleTypeDef *_hcan, uint8_t _id, float _reductionRatio) :
    hcan(_hcan), id(_id), reductionRatio(_reductionRatio)
{
    hcan1.Instance = CAN1;
    hcan2.Instance = CAN2;

    if (hcan->Instance == CAN1)
    {
        canHandler.CanPort = 1;
        canHandler.Send = SCA::Can1SendMsg;
    } else if (hcan->Instance == CAN2)
    {
        canHandler.CanPort = 2;
        canHandler.Send = SCA::Can2SendMsg;
    }

    canHandler.Retry = 2;
}


bool SCA::Init(bool _enableActuator)
{
    SetupActuators();
    scaHandler = getInstance(id);

    if (_enableActuator)
        return EnableActuator() == SCA_NoError;
    else
        return true;
}

void SCA::Homing()
{
    if (scaHandler->Power_State == Actr_Disable)
        return;

    SetMode(SCA_Profile_Position_Mode);
    SetPosition(0);

    do
    {
        GetPosition();
        HAL_Delay(100);
    } while ((scaHandler->Position_Real > 0.1f) || (scaHandler->Position_Real < -0.1f));
}


void SCA::LookupActuators()
{
    lookupActuators(&canHandler);
}

void SCA::SetupActuators()
{
    setupActuators(id, &canHandler);
}

void SCA::ResetController()
{
    resetController(id);
}

void SCA::RegainAttributes()
{
    regainAttrbute(id, Block);
}

bool SCA::IsOnline()
{
    isOnline(id, runCmdBlock);
    return scaHandler->Online_State == Actr_Enable;
}

bool SCA::IsEnable()
{
    isEnable(id, runCmdBlock);
    return scaHandler->Power_State == Actr_Enable;
}

bool SCA::IsUpdate()
{
    return isUpdate(id);
}

bool SCA::EnableActuator()
{
    return enableActuator(id) == SCA_NoError;
}

bool SCA::DisableActuator()
{
    return disableActuator(id) == SCA_NoError;
}

bool SCA::SetMode(uint8_t _actuatorMode)
{
    return activateActuatorMode(id, _actuatorMode, Block) == SCA_NoError;
}

uint8_t SCA::GetMode()
{
    getActuatorMode(id, runCmdBlock);
    return scaHandler->Mode;
}

uint16_t SCA::GetErrorCode()
{
    getErrorCode(id, Block);
    return scaHandler->SCA_Warn.Error_Code;
}

bool SCA::ClearError()
{
    return clearError(id, runCmdBlock) == SCA_NoError;
}

bool SCA::SaveAllParams()
{
    return saveAllParams(id, Block) == SCA_NoError;
}

void SCA::SetPosition(float _pos)
{
    setPositionFast(scaHandler, _pos * reductionRatio);
}

float SCA::GetPosition()
{
    getPositionFast(scaHandler, Block);
    return scaHandler->Position_Real / reductionRatio;
}

void SCA::SetPositionKp(float _kp)
{
    setPositionKp(id, _kp, runCmdBlock);
}

float SCA::GetPositionKp()
{
    getPositionKp(id, Block);
    return scaHandler->Position_Filter_P;
}

void SCA::SetPositionKi(float _ki)
{
    setPositionKi(id, _ki, runCmdBlock);
}

float SCA::GetPositionKi()
{
    getPositionKi(id, Block);
    return scaHandler->Position_Filter_I;
}

void SCA::SetPositionUmax(float _max)
{
    setPositionUmax(id, _max, runCmdBlock);
}

float SCA::GetPositionUmax()
{
    getPositionUmax(id, Block);
    return scaHandler->Position_Filter_Limit_H;
}

void SCA::SetPositionUmin(float _min)
{
    setPositionUmax(id, _min, runCmdBlock);
}

float SCA::GetPositionUmin()
{
    getPositionUmin(id, Block);
    return scaHandler->Position_Filter_Limit_L;
}

void SCA::SetPositionOffSet(float _offSet)
{
    setPositionOffset(id, _offSet, runCmdBlock);
}

float SCA::GetPositionOffSet()
{
    getPositionOffset(id, Block);
    return scaHandler->Position_Offset;
}

void SCA::SetMaximumPosition(float _maxPos)
{
    setMaximumPosition(id, _maxPos, runCmdBlock);
}

float SCA::GetMaximumPosition()
{
    getMaximumPosition(id, Block);
    return scaHandler->Position_Limit_H;
}

void SCA::SetMinimumPosition(float _minPos)
{
    setMinimumPosition(id, _minPos, runCmdBlock);
}

float SCA::GetMinimumPosition()
{
    getMinimumPosition(id, Block);
    return scaHandler->Position_Limit_L;
}

uint8_t SCA::EnablePositionLimit(uint8_t _enable)
{
    return enablePositionLimit(id, _enable, runCmdBlock);
}

uint8_t SCA::IsPositionLimitEnable()
{
    return isPositionLimitEnable(id, runCmdBlock);
}

uint8_t SCA::SetHomingPosition(float _homingPos)
{
    return setHomingPosition(id, _homingPos, runCmdBlock);
}

uint8_t SCA::EnablePositionFilter(uint8_t _enable)
{
    return enablePositionFilter(id, _enable, runCmdBlock);
}

uint8_t SCA::IsPositionFilterEnable()
{
    return isPositionFilterEnable(id, runCmdBlock);
}

void SCA::SetPositionCutoffFrequency(float _frequency)
{
    setPositionCutoffFrequency(id, _frequency, runCmdBlock);
}

float SCA::GetPositionCutoffFrequency()
{
    getPositionCutoffFrequency(id, Block);
    return scaHandler->Position_Filter_Value;
}

uint8_t SCA::ClearHomingInfo()
{
    return clearHomingInfo(id, runCmdBlock);
}

uint8_t SCA::SetProfilePositionAcceleration(float _acceleration)
{
    return setProfilePositionAcceleration(id, _acceleration, runCmdBlock);
}

float SCA::GetProfilePositionAcceleration()
{
    getProfilePositionAcceleration(id, Block);
    return scaHandler->PP_Max_Acceleration;
}

uint8_t SCA::SetProfilePositionDeceleration(float _deceleration)
{
    return setProfilePositionDeceleration(id, _deceleration, runCmdBlock);
}

float SCA::GetProfilePositionDeceleration()
{
    getProfilePositionDeceleration(id, Block);
    return scaHandler->PP_Max_Deceleration;
}

uint8_t SCA::SetProfilePositionMaxVelocity(float _maxVelocity)
{
    return setProfilePositionMaxVelocity(id, _maxVelocity, runCmdBlock);
}

float SCA::GetProfilePositionMaxVelocity()
{
    getProfilePositionMaxVelocity(id, Block);
    return scaHandler->PP_Max_Velocity;
}

void SCA::SetVelocity(float _vel)
{
    setVelocityFast(scaHandler, _vel * reductionRatio);
}

float SCA::GetVelocity()
{
    getVelocityFast(scaHandler, Block);
    return scaHandler->Velocity_Real / reductionRatio;
}

uint8_t SCA::GetVelocityKp()
{
    return getVelocityKp(id, Block);
}

uint8_t SCA::SetVelocityKp(float _kp)
{
    return setVelocityKp(id, _kp, runCmdBlock);
}

uint8_t SCA::GetVelocityKi()
{
    return getVelocityKi(id, Block);
}

uint8_t SCA::SetVelocityKi(float _ki)
{
    return setVelocityKi(id, _ki, runCmdBlock);
}

uint8_t SCA::GetVelocityUmax()
{
    return getVelocityUmax(id, Block);
}

uint8_t SCA::SetVelocityUmax(float _max)
{
    return setVelocityUmax(id, _max, runCmdBlock);
}

uint8_t SCA::GetVelocityUmin()
{
    return getVelocityUmin(id, Block);
}

uint8_t SCA::SetVelocityUmin(float _min)
{
    return setVelocityUmin(id, _min, runCmdBlock);
}

uint8_t SCA::EnableVelocityFilter(uint8_t _enable)
{
    return enableVelocityFilter(id, _enable, runCmdBlock);
}

uint8_t SCA::IsVelocityFilterEnable()
{
    return isVelocityFilterEnable(id, runCmdBlock);
}

uint8_t SCA::GetVelocityCutoffFrequency()
{
    return getVelocityCutoffFrequency(id, Block);
}

uint8_t SCA::SetVelocityCutoffFrequency(float _frequency)
{
    return setVelocityCutoffFrequency(id, _frequency, runCmdBlock);
}

uint8_t SCA::SetVelocityLimit(float _limit)
{
    return setVelocityLimit(id, _limit, runCmdBlock);
}

uint8_t SCA::GetVelocityLimit()
{
    return getVelocityLimit(id, Block);
}

uint8_t SCA::SetProfileVelocityAcceleration(float _acceleration)
{
    return setProfileVelocityAcceleration(id, _acceleration, runCmdBlock);
}

uint8_t SCA::GetProfileVelocityAcceleration()
{
    return getProfileVelocityAcceleration(id, Block);
}

uint8_t SCA::SetProfileVelocityDeceleration(float _deceleration)
{
    return setProfileVelocityDeceleration(id, _deceleration, runCmdBlock);
}

uint8_t SCA::GetProfileVelocityDeceleration()
{
    return getProfileVelocityDeceleration(id, Block);
}

uint8_t SCA::SetProfileVelocityMaxVelocity(float _maxVelocity)
{
    return setProfileVelocityMaxVelocity(id, _maxVelocity, runCmdBlock);
}

uint8_t SCA::GetProfileVelocityMaxVelocity()
{
    return getProfileVelocityMaxVelocity(id, Block);
}

float SCA::GetVelocityRange()
{
    return getVelocityRange(id);
}

uint8_t SCA::SetCurrent(float _current)
{
    return setCurrentFast(scaHandler, _current);
}

uint8_t SCA::GetCurrent()
{
    return getCurrentFast(scaHandler, Block);
}

uint8_t SCA::GetCurrentKp()
{
    return getCurrentKp(id, Block);
}

uint8_t SCA::GetCurrentKi()
{
    return getCurrentKi(id, Block);
}

uint8_t SCA::GetCurrentRange()
{
    return getCurrentRange(id, Block);
}

uint8_t SCA::EnableCurrentFilter(uint8_t _enable)
{
    return enableCurrentFilter(id, _enable, runCmdBlock);
}

uint8_t SCA::IsCurrentFilterEnable()
{
    return isCurrentFilterEnable(id, runCmdBlock);
}

uint8_t SCA::GetCurrentCutoffFrequency()
{
    return getCurrentCutoffFrequency(id, Block);
}

uint8_t SCA::SetCurrentCutoffFrequency(float _frequency)
{
    return setCurrentCutoffFrequency(id, _frequency, runCmdBlock);
}

uint8_t SCA::SetCurrentLimit(float _limit)
{
    return setCurrentLimit(id, _limit, runCmdBlock);
}

uint8_t SCA::GetCurrentLimit()
{
    return getCurrentLimit(id, Block);
}

uint8_t SCA::GetVoltage()
{
    return getVoltage(id, Block);
}

uint8_t SCA::GetLockEnergy()
{
    return getLockEnergy(id, Block);
}

uint8_t SCA::SetLockEnergy(float _energy)
{
    return setLockEnergy(id, _energy, runCmdBlock);
}

uint8_t SCA::GetActuatorSerialNumber()
{
    return getActuatorSerialNumber(id, Block);
}

uint8_t SCA::GetMotorTemperature()
{
    return getMotorTemperature(id, Block);
}

uint8_t SCA::GetInverterTemperature()
{
    return getInverterTemperature(id, Block);
}

uint8_t SCA::GetMotorProtectedTemperature()
{
    return getMotorProtectedTemperature(id, Block);
}

uint8_t SCA::SetMotorProtectedTemperature(float _temp)
{
    return setMotorProtectedTemperature(id, _temp, runCmdBlock);
}

uint8_t SCA::GetMotorRecoveryTemperature()
{
    return getMotorRecoveryTemperature(id, Block);
}

uint8_t SCA::SetMotorRecoveryTemperature(float _temp)
{
    return setMotorRecoveryTemperature(id, _temp, runCmdBlock);
}

uint8_t SCA::GetInverterProtectedTemperature()
{
    return getInverterProtectedTemperature(id, Block);
}

uint8_t SCA::SetInverterProtectedTemperature(float _temp)
{
    return setInverterProtectedTemperature(id, _temp, runCmdBlock);
}

uint8_t SCA::GetInverterRecoveryTemperature()
{
    return getInverterRecoveryTemperature(id, Block);
}

uint8_t SCA::SetInverterRecoveryTemperature(float _temp)
{
    return setInverterRecoveryTemperature(id, _temp, runCmdBlock);
}

uint8_t SCA::SetActuatorID(uint8_t currentID, uint8_t _newId)
{
    return setActuatorID(id, _newId, runCmdBlock);
}

uint8_t SCA::GetActuatorLastState()
{
    return getActuatorLastState(id, Block);
}

uint8_t SCA::RequestCVPValue()
{
    return requestCVPValueFast(scaHandler, runCmdBlock);
}



