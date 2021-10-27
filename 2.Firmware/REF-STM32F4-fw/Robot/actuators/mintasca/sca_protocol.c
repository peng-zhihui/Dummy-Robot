/**
  ******************************************************************************
  * @文	件 ： SCA_Protocol.c
  * @作	者 ： INNFOS Software Team
  * @版	本 ： V1.5.2
  * @日	期 ： 2019.06.24
  * @摘	要 ： INNFOS CAN 通信协议层
  ******************************************************************************/
/* Update log --------------------------------------------------------------------*/
//V1.1.0 2019.08.05 加入第五类写入命令接口,更改CAN总线的数据等待时间
//V1.5.0 2019.08.16 加入数据接收接口，统一读命令接口。加入各类数据分析接口，加入参数缓存
//V1.5.2 2019.11.04	修复对旧版编译器的兼容性。

/* Includes ----------------------------------------------------------------------*/
#include "sca_api.h"

/* Forward Declaration -----------------------------------------------------------*/
static uint8_t canTransmit(SCA_Handler_t *pSCA, uint8_t *TxBuf, uint8_t TxLen);
static void R1dataProcess(SCA_Handler_t *pSCA, CanRxMsg *RxMsg);
static void R2dataProcess(SCA_Handler_t *pSCA, CanRxMsg *RxMsg);
static void R3dataProcess(SCA_Handler_t *pSCA, CanRxMsg *RxMsg);
static void R4dataProcess(SCA_Handler_t *pSCA, CanRxMsg *RxMsg);
static void WriteDataProcess(SCA_Handler_t *pSCA, CanRxMsg *RxMsg);
void warnBitAnaly(SCA_Handler_t *pSCA);

/* Funcation defines -------------------------------------------------------------*/

/**
  * @功	能	第1类写入命令，发送2byte，接收2byte
  * @参	数	pSCA：要操作的执行器句柄指针或地址
  *			cmd：操作指令
  *			TxData：要发送的数据，可以是以下两种
  *					1.执行器操作模式选择 2.使能（0x01）或失能（0x00）
  * @返	回	SCA_NoError：发送成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t SCA_Write_1(SCA_Handler_t *pSCA, uint8_t cmd, uint8_t TxData)
{
    uint8_t TxBuf[2];

    /* 数据打包格式：
        TxBuf[0]-操作命令 	TxBuf[1]-数据（高位）至 TxBuf[7]-数据（低位） */
    TxBuf[0] = cmd;
    TxBuf[1] = TxData;

    /* 调用底层通信函数传输数据，若出现通信错误则返回错误值 */
    return canTransmit(pSCA, TxBuf, 2);
}

/**
  * @功	能	第2类写入命令，发送3byte，接收2byte
  * @参	数	pSCA：要操作的执行器句柄指针或地址
  *			cmd：操作指令
  *			TxData：要发送的数据，真实值
  * @返	回	SCA_NoError：发送成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t SCA_Write_2(SCA_Handler_t *pSCA, uint8_t cmd, float TxData)
{
    uint8_t TxBuf[3];
    int16_t temp;

    /* 第二类读写命令以IQ8格式进行传输 */
    temp = TxData * IQ8;

    /* 数据打包 */
    TxBuf[0] = cmd;
    TxBuf[1] = (uint8_t) (temp >> 8);
    TxBuf[2] = (uint8_t) (temp >> 0);

    return canTransmit(pSCA, TxBuf, 3);
}

/**
  * @功	能	第3类写入命令，发送5byte，接收2byte
  * @参	数	pSCA：要操作的执行器句柄指针或地址
  *			cmd：操作指令
  *			TxData：发送的数据，真实值
  * @返	回	SCA_NoError：发送成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t SCA_Write_3(SCA_Handler_t *pSCA, uint8_t cmd, float TxData)
{
    uint8_t TxBuf[5];
    int32_t temp;

    /*	速度与电流在设定时，要采用标值，
        即设定值除以该参数的最大值，再转换为IQ24格式	*/
    if ((cmd == W3_Velocity) || (cmd == W3_VelocityLimit))
        temp = TxData / Velocity_Max * IQ24;
    else if ((cmd == W3_Current) || (cmd == W3_CurrentLimit))
        temp = TxData / pSCA->Current_Max * IQ24;
    else if (cmd == W3_BlockEngy)
        temp = TxData * BlkEngy_Scal;    //堵转能量为真实值的75.225倍
    else
        temp = TxData * IQ24;

    TxBuf[0] = cmd;
    TxBuf[1] = (uint8_t) (temp >> 24);
    TxBuf[2] = (uint8_t) (temp >> 16);
    TxBuf[3] = (uint8_t) (temp >> 8);
    TxBuf[4] = (uint8_t) (temp >> 0);

    return canTransmit(pSCA, TxBuf, 5);
}

/**
  * @功	能	第4类写入命令，发送1byte，接收2byte
  * @参	数	pSCA：要操作的执行器句柄指针或地址
  *			cmd：操作指令
  * @返	回	SCA_NoError：发送成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t SCA_Write_4(SCA_Handler_t *pSCA, uint8_t cmd)
{
    uint8_t TxBuf[1];
    TxBuf[0] = cmd;
    return canTransmit(pSCA, TxBuf, 1);
}

/**
  * @功	能	第5类写入命令，发送6byte，接收2byte
  * @参	数	pSCA：要操作的执行器句柄指针或地址
  *			cmd：操作指令
  *			TxData：发送数据
  * @返	回	SCA_NoError：发送成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t SCA_Write_5(SCA_Handler_t *pSCA, uint8_t cmd, uint8_t TxData)
{
    uint8_t TxBuf[6];

    /*
        第五类写入命令数据格式：
        1字节命令+4字节地址（SCA的序列号）+1字节参数（目标数据）
    */
    TxBuf[0] = cmd;
    TxBuf[1] = pSCA->Serial_Num[0];
    TxBuf[2] = pSCA->Serial_Num[1];
    TxBuf[3] = pSCA->Serial_Num[2];
    TxBuf[4] = pSCA->Serial_Num[3];
    TxBuf[5] = TxData;

    return canTransmit(pSCA, TxBuf, 6);
}

/**
  * @功	能	读取命令接口，发送1byte
  * @参	数	pSCA：要操作的执行器句柄指针或地址
  *			cmd：操作指令
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t SCA_Read(SCA_Handler_t *pSCA, uint8_t cmd)
{
    uint8_t TxBuf[1];
    TxBuf[0] = cmd;
    return canTransmit(pSCA, TxBuf, 1);
}

/**
  * @功	能	CAN底层通信函数，发送数据
  * @参	数	ID：要操作的执行器ID
  *			TxBuf：要发送的数据地址
  *			TxLen：要发送的数据长度
  * @返	回	SCA_NoError：操作成功
  *			SCA_SendError：发送失败
  */
static uint8_t canTransmit(SCA_Handler_t *pSCA, uint8_t *TxBuf, uint8_t TxLen)
{
    uint32_t waitime = 0;

    /* 调用CAN1发送指定的数据，若发送失败则进行重发，最多重发Retry次 */
    while (pSCA->Can->Send(pSCA->ID, TxBuf, TxLen) && (waitime < pSCA->Can->Retry)) waitime++;

    /* 发送次数超出设定值，返回发送失败 */
    if (waitime >= pSCA->Can->Retry)
        return SCA_SendError;

    /* 数据发送成功，没有错误产生 */
    return SCA_NoError;
}

/**
  * @功	能	第1类读取命令返回数据解析，发送1byte，接收2byte
  * @参	数	pSCA：目标执行器句柄指针或地址
  *			RxMsg：接收到的数据包
  * @返	回	无
  */
static void R1dataProcess(SCA_Handler_t *pSCA, CanRxMsg *RxMsg)
{
    /* 将读取结果装载到接收地址中 */
    switch (RxMsg->Data[0])
    {
        case R1_Heartbeat:
            pSCA->Online_State = RxMsg->Data[1];
            break;

        case R1_Mode:
            pSCA->Mode = RxMsg->Data[1];
            pSCA->paraCache.R_Mode = Actr_Enable;
            break;

        case R1_LastState:
            pSCA->Last_State = RxMsg->Data[1];
            pSCA->paraCache.R_Last_State = Actr_Enable;
            break;

        case R1_CurrentFilterState:
            pSCA->Current_Filter_State = RxMsg->Data[1];
            pSCA->paraCache.R_Current_Filter_State = Actr_Enable;
            break;

        case R1_VelocityFilterState:
            pSCA->Velocity_Filter_State = RxMsg->Data[1];
            pSCA->paraCache.R_Velocity_Filter_State = Actr_Enable;
            break;

        case R1_PositionFilterState:
            pSCA->Position_Filter_State = RxMsg->Data[1];
            pSCA->paraCache.R_Position_Filter_State = Actr_Enable;
            break;

        case R1_PositionLimitState:
            pSCA->Position_Limit_State = RxMsg->Data[1];
            pSCA->paraCache.R_Position_Limit_State = Actr_Enable;
            break;

        case R1_PowerState:
            pSCA->Power_State = RxMsg->Data[1];
            pSCA->paraCache.R_Power_State = Actr_Enable;
            break;

        default:
            break;
    }
}

/**
  * @功	能	第2类读取命令返回数据解析，发送1byte，接收3byte
  * @参	数	pSCA：目标执行器句柄指针或地址
  *			RxMsg：接收到的数据包
  * @返	回	无
  */
static void R2dataProcess(SCA_Handler_t *pSCA, CanRxMsg *RxMsg)
{
    int16_t temp;
    float RxData;

    /* 第二类读写命令为IQ8格式 */
    temp = ((int16_t) RxMsg->Data[1]) << 8;
    temp |= ((int16_t) RxMsg->Data[2]) << 0;

    /* 在第二类读写命令中，电压数据为IQ10格式 */
    if (RxMsg->Data[0] == R2_Voltage)
        RxData = (float) temp / IQ10;
    else
        RxData = (float) temp / IQ8;

    switch (RxMsg->Data[0])
    {
        case R2_Voltage:
            pSCA->Voltage = RxData;
            pSCA->paraCache.R_Voltage = Actr_Enable;
            break;

        case R2_Current_Max:
            pSCA->Current_Max = RxData;
            pSCA->paraCache.R_Current_Max = Actr_Enable;
            break;

        case R2_CurrentFilterValue:
            pSCA->Current_Filter_Value = RxData;
            pSCA->paraCache.R_Current_Filter_Value = Actr_Enable;
            break;

        case R2_VelocityFilterValue:
            pSCA->Velocity_Filter_Value = RxData;
            pSCA->paraCache.R_Velocity_Filter_Value = Actr_Enable;
            break;

        case R2_PositionFilterValue:
            pSCA->Position_Filter_Value = RxData;
            pSCA->paraCache.R_Position_Filter_Value = Actr_Enable;
            break;

        case R2_MotorTemp:
            pSCA->Motor_Temp = RxData;
            pSCA->paraCache.R_Motor_Temp = Actr_Enable;
            break;

        case R2_InverterTemp:
            pSCA->Inverter_Temp = RxData;
            pSCA->paraCache.R_Inverter_Temp = Actr_Enable;
            break;

        case R2_InverterProtectTemp:
            pSCA->Inverter_Protect_Temp = RxData;
            pSCA->paraCache.R_Inverter_Protect_Temp = Actr_Enable;
            break;

        case R2_InverterRecoverTemp:
            pSCA->Inverter_Recover_Temp = RxData;
            pSCA->paraCache.R_Inverter_Recover_Temp = Actr_Enable;
            break;

        case R2_MotorProtectTemp:
            pSCA->Motor_Protect_Temp = RxData;
            pSCA->paraCache.R_Motor_Protect_Temp = Actr_Enable;
            break;

        case R2_MotorRecoverTemp:
            pSCA->Motor_Recover_Temp = RxData;
            pSCA->paraCache.R_Motor_Recover_Temp = Actr_Enable;
            break;

        case R2_Error:
            pSCA->SCA_Warn.Error_Code = temp;
            warnBitAnaly(pSCA);
            pSCA->paraCache.R_Error_Code = Actr_Enable;
            break;

        default:
            break;
    }
}

/**
  * @功	能	第3类读取命令返回数据解析，发送1byte，接收5byte
  * @参	数	pSCA：目标执行器句柄指针或地址
  *			RxMsg：接收到的数据包
  * @返	回	无
  */
static void R3dataProcess(SCA_Handler_t *pSCA, CanRxMsg *RxMsg)
{
    int32_t temp;
    float RxData;

    /* 第三类读写命令以IQ24格式传输 */
    temp = ((int32_t) RxMsg->Data[1]) << 24;
    temp |= ((int32_t) RxMsg->Data[2]) << 16;
    temp |= ((int32_t) RxMsg->Data[3]) << 8;
    temp |= ((int32_t) RxMsg->Data[4]) << 0;

    /* 速度和电流使用标值，需要将转换值乘以该参数的最大值得到实际值 */
    if ((RxMsg->Data[0] == R3_Velocity) || (RxMsg->Data[0] == R3_VelocityLimit))
        RxData = (float) temp / IQ24 * Velocity_Max;

    else if ((RxMsg->Data[0] == R3_Current) || (RxMsg->Data[0] == R3_CurrentLimit))
        RxData = (float) temp / IQ24 * pSCA->Current_Max;

    else if (RxMsg->Data[0] == R3_BlockEngy)
        RxData = (float) temp / BlkEngy_Scal;    //堵转能量为真实的75.225倍

    else
        RxData = (float) temp / IQ24;

    switch (RxMsg->Data[0])
    {
        case R3_Current:
            pSCA->Current_Real = RxData;
            pSCA->paraCache.R_Current_Real = Actr_Enable;
            break;

        case R3_Velocity:
            pSCA->Velocity_Real = RxData;
            pSCA->paraCache.R_Velocity_Real = Actr_Enable;
            break;

        case R3_Position:
            pSCA->Position_Real = RxData;
            pSCA->paraCache.R_Position_Real = Actr_Enable;
            break;

        case R3_CurrentFilterP:
            pSCA->Current_Filter_P = RxData;
            pSCA->paraCache.R_Current_Filter_P = Actr_Enable;
            break;

        case R3_CurrentFilterI:
            pSCA->Current_Filter_I = RxData;
            pSCA->paraCache.R_Current_Filter_I = Actr_Enable;
            break;

        case R3_VelocityFilterP:
            pSCA->Velocity_Filter_P = RxData;
            pSCA->paraCache.R_Velocity_Filter_P = Actr_Enable;
            break;

        case R3_VelocityFilterI:
            pSCA->Velocity_Filter_I = RxData;
            pSCA->paraCache.R_Velocity_Filter_I = Actr_Enable;
            break;

        case R3_PositionFilterP:
            pSCA->Position_Filter_P = RxData;
            pSCA->paraCache.R_Position_Filter_P = Actr_Enable;
            break;

        case R3_PositionFilterI:
            pSCA->Position_Filter_I = RxData;
            pSCA->paraCache.R_Position_Filter_I = Actr_Enable;
            break;

        case R3_PositionFilterD:
            break;

        case R3_PPMaxVelocity:
            pSCA->PP_Max_Velocity = RxData * Profile_Scal;
            pSCA->paraCache.R_PP_Max_Velocity = Actr_Enable;
            break;

        case R3_PPMaxAcceleration:
            pSCA->PP_Max_Acceleration = RxData * Profile_Scal;
            pSCA->paraCache.R_PP_Max_Acceleration = Actr_Enable;
            break;

        case R3_PPMaxDeceleration:
            pSCA->PP_Max_Deceleration = RxData * Profile_Scal;
            pSCA->paraCache.R_PP_Max_Deceleration = Actr_Enable;
            break;

        case R3_PVMaxVelocity:
            pSCA->PV_Max_Velocity = RxData * Profile_Scal;
            pSCA->paraCache.R_PV_Max_Velocity = Actr_Enable;
            break;

        case R3_PVMaxAcceleration:
            pSCA->PV_Max_Acceleration = RxData * Profile_Scal;
            pSCA->paraCache.R_PV_Max_Acceleration = Actr_Enable;
            break;

        case R3_PVMaxDeceleration:
            pSCA->PV_Max_Deceleration = RxData * Profile_Scal;
            pSCA->paraCache.R_PV_Max_Deceleration = Actr_Enable;
            break;

        case R3_CurrentFilterLimitL:
            break;

        case R3_CurrentFilterLimitH:
            break;

        case R3_VelocityFilterLimitL:
            pSCA->Velocity_Filter_Limit_L = RxData;
            pSCA->paraCache.R_Velocity_Filter_Limit_L = Actr_Enable;
            break;

        case R3_VelocityFilterLimitH:
            pSCA->Velocity_Filter_Limit_H = RxData;
            pSCA->paraCache.R_Velocity_Filter_Limit_H = Actr_Enable;
            break;

        case R3_PositionFilterLimitL:
            pSCA->Position_Filter_Limit_L = RxData;
            pSCA->paraCache.R_Position_Filter_Limit_L = Actr_Enable;
            break;

        case R3_PositionFilterLimitH:
            pSCA->Position_Filter_Limit_H = RxData;
            pSCA->paraCache.R_Position_Filter_Limit_H = Actr_Enable;
            break;

        case R3_CurrentLimit:
            pSCA->Current_Limit = RxData;
            pSCA->paraCache.R_Current_Limit = Actr_Enable;
            break;

        case R3_VelocityLimit:
            pSCA->Velocity_Limit = RxData;
            pSCA->paraCache.R_Velocity_Limit = Actr_Enable;
            break;

        case R3_Inertia:
            break;

        case R3_PositionLimitH:
            pSCA->Position_Limit_H = RxData;
            pSCA->paraCache.R_Position_Limit_H = Actr_Enable;
            break;

        case R3_PositionLimitL:
            pSCA->Position_Limit_L = RxData;
            pSCA->paraCache.R_Position_Limit_L = Actr_Enable;
            break;

        case R3_PositionOffset:
            pSCA->Position_Offset = RxData;
            pSCA->paraCache.R_Position_Offset = Actr_Enable;
            break;

        case R3_HomingCurrentLimitL:
            pSCA->Homing_Current_Limit_L = RxData;
            pSCA->paraCache.R_Homing_Current_Limit_L = Actr_Enable;
            break;

        case R3_HomingCurrentLimitH:
            pSCA->Homing_Current_Limit_H = RxData;
            pSCA->paraCache.R_Homing_Current_Limit_H = Actr_Enable;
            break;

        case R3_BlockEngy:
            pSCA->Blocked_Energy = RxData;
            pSCA->paraCache.R_Blocked_Energy = Actr_Enable;
            break;

        default:
            break;
    }
}

/**
  * @功	能	第4类读取命令返回数据解析，发送1byte，接收8byte
  * @参	数	pSCA：目标执行器句柄指针或地址
  *			RxMsg：接收到的数据包
  * @返	回	无
  */
static void R4dataProcess(SCA_Handler_t *pSCA, CanRxMsg *RxMsg)
{
    int32_t temp;

    /*	在三环读取协议中，为了使速度、电流、位置数据在同一数据帧中表示出
        将电流和速度值以IQ14格式传输，将位置值以IQ16格式传输。为了方便符
        号位的计算，将位置值向左移8位对齐符号位，转而除以IQ24得到真实值；
        同理，将电流和速度值左移16位对齐符号位，转而除以IQ30得到真实值	。	*/

    temp = ((int32_t) RxMsg->Data[1]) << 24;
    temp |= ((int32_t) RxMsg->Data[2]) << 16;
    temp |= ((int32_t) RxMsg->Data[3]) << 8;
    pSCA->Position_Real = (float) temp / IQ24;

    temp = ((int32_t) RxMsg->Data[4]) << 24;
    temp |= ((int32_t) RxMsg->Data[5]) << 16;
    pSCA->Velocity_Real = (float) temp / IQ30 * Velocity_Max;

    temp = ((int32_t) RxMsg->Data[6]) << 24;
    temp |= ((int32_t) RxMsg->Data[7]) << 16;
    pSCA->Current_Real = (float) temp / IQ30 * pSCA->Current_Max;

    /* 标记数据已收到 */
    pSCA->paraCache.R_CVP = Actr_Enable;
}

/**
  * @功	能	第5类读取命令返回数据解析，发送1byte，接收5byte
  *			用于查询指定执行器的序列号
  * @参	数	pSCA：目标执行器句柄指针或地址
  *			RxMsg：接收到的数据包
  * @返	回	无
  */
static void R5dataProcess(SCA_Handler_t *pSCA, CanRxMsg *RxMsg)
{
    /* 装填序列号 */
    pSCA->Serial_Num[0] = RxMsg->Data[1];
    pSCA->Serial_Num[1] = RxMsg->Data[2];
    pSCA->Serial_Num[2] = RxMsg->Data[3];
    pSCA->Serial_Num[3] = RxMsg->Data[4];

    /* 标记数据已收到 */
    pSCA->paraCache.R_Serial_Num = Actr_Enable;
}

/**
  * @功	能	写入命令返回数据解析，将参数缓存中的数据写入句柄中
  *			用于查询指定执行器的序列号
  * @参	数	pSCA：目标执行器句柄指针或地址
  *			RxMsg：接收到的数据包
  * @返	回	无
  */
static void WriteDataProcess(SCA_Handler_t *pSCA, CanRxMsg *RxMsg)
{
    /* 写入成功，将缓存中的参数更新到句柄中 */
    if (RxMsg->Data[1] == Actr_Enable)
    {
        /* 有新数据写入成功，复位存储标志位 */
        pSCA->Save_State = Actr_Disable;

        switch (RxMsg->Data[0])
        {
            case W1_Mode:
                pSCA->Mode = pSCA->paraCache.Mode;
                break;

            case W1_CurrentFilterState:
                pSCA->Current_Filter_State = pSCA->paraCache.Current_Filter_State;
                break;

            case W1_VelocityFilterState:
                pSCA->Velocity_Filter_State = pSCA->paraCache.Velocity_Filter_State;
                break;

            case W1_PositionFilterState:
                pSCA->Position_Filter_State = pSCA->paraCache.Position_Filter_State;
                break;

            case W1_PositionLimitState:
                pSCA->Position_Limit_State = pSCA->paraCache.Position_Limit_State;
                break;

            case W1_PowerState:
                pSCA->Power_State = pSCA->paraCache.Power_State;
                break;

            case W2_CurrentFilterValue:
                pSCA->Current_Filter_Value = pSCA->paraCache.Current_Filter_Value;
                break;

            case W2_VelocityFilterValue:
                pSCA->Velocity_Filter_Value = pSCA->paraCache.Velocity_Filter_Value;
                break;

            case W2_PositionFilterValue:
                pSCA->Position_Filter_Value = pSCA->paraCache.Position_Filter_Value;
                break;

            case W2_InverterProtectTemp:
                pSCA->Inverter_Protect_Temp = pSCA->paraCache.Inverter_Protect_Temp;
                break;

            case W2_InverterRecoverTemp:
                pSCA->Inverter_Recover_Temp = pSCA->paraCache.Inverter_Recover_Temp;
                break;

            case W2_MotorProtectTemp:
                pSCA->Motor_Protect_Temp = pSCA->paraCache.Motor_Protect_Temp;
                break;

            case W2_MotorRecoverTemp:
                pSCA->Motor_Recover_Temp = pSCA->paraCache.Motor_Recover_Temp;
                break;

            case W3_Current:
                pSCA->Current_Real = pSCA->paraCache.Current_Real;
                break;

            case W3_Velocity:
                pSCA->Velocity_Real = pSCA->paraCache.Velocity_Real;
                break;

            case W3_Position:
                pSCA->Position_Real = pSCA->paraCache.Position_Real;
                break;

            case W3_CurrentFilterP:
                pSCA->Current_Filter_P = pSCA->paraCache.Current_Filter_P;
                break;

            case W3_CurrentFilterI:
                pSCA->Current_Filter_I = pSCA->paraCache.Current_Filter_I;
                break;

            case W3_VelocityFilterP:
                pSCA->Velocity_Filter_P = pSCA->paraCache.Velocity_Filter_P;
                break;

            case W3_VelocityFilterI:
                pSCA->Velocity_Filter_I = pSCA->paraCache.Velocity_Filter_I;
                break;

            case W3_PositionFilterP:
                pSCA->Position_Filter_P = pSCA->paraCache.Position_Filter_P;
                break;

            case W3_PositionFilterI:
                pSCA->Position_Filter_I = pSCA->paraCache.Position_Filter_I;
                break;

            case W3_PositionFilterD:
                break;

            case W3_PPMaxVelocity:
                pSCA->PP_Max_Velocity = pSCA->paraCache.PP_Max_Velocity;
                break;

            case W3_PPMaxAcceleration:
                pSCA->PP_Max_Acceleration = pSCA->paraCache.PP_Max_Acceleration;
                break;

            case W3_PPMaxDeceleration:
                pSCA->PP_Max_Deceleration = pSCA->paraCache.PP_Max_Deceleration;
                break;

            case W3_PVMaxVelocity:
                pSCA->PV_Max_Velocity = pSCA->paraCache.PV_Max_Velocity;
                break;

            case W3_PVMaxAcceleration:
                pSCA->PV_Max_Acceleration = pSCA->paraCache.PV_Max_Acceleration;
                break;

            case W3_PVMaxDeceleration:
                pSCA->PV_Max_Deceleration = pSCA->paraCache.PV_Max_Deceleration;
                break;

            case W3_CurrentFilterLimitL:
                break;

            case W3_CurrentFilterLimitH:
                break;

            case W3_VelocityFilterLimitL:
                pSCA->Velocity_Filter_Limit_L = pSCA->paraCache.Velocity_Filter_Limit_L;
                break;

            case W3_VelocityFilterLimitH:
                pSCA->Velocity_Filter_Limit_H = pSCA->paraCache.Velocity_Filter_Limit_H;
                break;

            case W3_PositionFilterLimitL:
                pSCA->Position_Filter_Limit_L = pSCA->paraCache.Position_Filter_Limit_L;
                break;

            case W3_PositionFilterLimitH:
                pSCA->Position_Filter_Limit_H = pSCA->paraCache.Position_Filter_Limit_H;
                break;

            case W3_CurrentLimit:
                pSCA->Current_Limit = pSCA->paraCache.Current_Limit;
                break;

            case W3_VelocityLimit:
                pSCA->Velocity_Limit = pSCA->paraCache.Velocity_Limit;
                break;

            case W3_PositionLimitH:
                pSCA->Position_Limit_H = pSCA->paraCache.Position_Limit_H;
                break;

            case W3_PositionLimitL:
                pSCA->Position_Limit_L = pSCA->paraCache.Position_Limit_L;
                break;

            case W3_HomingValue:
                pSCA->Homing_Value = pSCA->paraCache.Homing_Value;
                break;

            case W3_PositionOffset:
                pSCA->Position_Offset = pSCA->paraCache.Position_Offset;
                break;

            case W3_HomingCurrentLimitL:
                pSCA->Homing_Current_Limit_L = pSCA->paraCache.Homing_Current_Limit_L;
                break;

            case W3_HomingCurrentLimitH:
                pSCA->Homing_Current_Limit_H = pSCA->paraCache.Homing_Current_Limit_H;
                break;

            case W3_BlockEngy:
                pSCA->Blocked_Energy = pSCA->paraCache.Blocked_Energy;
                break;

            case W4_ClearError:
                pSCA->SCA_Warn.Error_Code = 0;
                warnBitAnaly(pSCA);
                break;

            case W4_ClearHome:
                pSCA->Position_Real = 0;
                pSCA->Position_Limit_H = 127.0f;
                pSCA->Position_Limit_L = -127.0f;
                pSCA->paraCache.W_ClearHome = Actr_Enable;
                break;

            case W4_Save:
                pSCA->Save_State = Actr_Enable;
                break;

            case W5_ChangeID:
                pSCA->ID = pSCA->paraCache.ID;

            default:
                break;
        }
    }
}

/**
  * @功	能	CAN接收数据解析，
  * @参	数	RxMessage：接收的数据包
  * @返	回	无
  */
void canDispatch(CanRxMsg *RxMsg)
{
    SCA_Handler_t *pSCA = getInstance((uint8_t) RxMsg->StdId);

    /* 不存在该ID，忽略消息 */
    if (pSCA == NULL) return;

    /* 标记有数据更新 */
    pSCA->Update_State = Actr_Enable;

    /* 命令解析 */
    switch (RxMsg->Data[0])
    {
        case R1_Heartbeat:
        case R1_Mode:
        case R1_LastState:
        case R1_CurrentFilterState:
        case R1_VelocityFilterState:
        case R1_PositionFilterState:
        case R1_PositionLimitState:
        case R1_PowerState:
            R1dataProcess(pSCA, RxMsg);
            break;

        case R2_Voltage:
        case R2_Current_Max:
        case R2_CurrentFilterValue:
        case R2_VelocityFilterValue:
        case R2_PositionFilterValue:
        case R2_MotorTemp:
        case R2_InverterTemp:
        case R2_InverterProtectTemp:
        case R2_InverterRecoverTemp:
        case R2_MotorProtectTemp:
        case R2_MotorRecoverTemp:
        case R2_Error:
            R2dataProcess(pSCA, RxMsg);
            break;

        case R3_Current:
        case R3_Velocity:
        case R3_Position:
        case R3_CurrentFilterP:
        case R3_CurrentFilterI:
        case R3_VelocityFilterP:
        case R3_VelocityFilterI:
        case R3_PositionFilterP:
        case R3_PositionFilterI:
        case R3_PositionFilterD:
        case R3_PPMaxVelocity:
        case R3_PPMaxAcceleration:
        case R3_PPMaxDeceleration:
        case R3_PVMaxVelocity:
        case R3_PVMaxAcceleration:
        case R3_PVMaxDeceleration:
        case R3_CurrentFilterLimitL:
        case R3_CurrentFilterLimitH:
        case R3_VelocityFilterLimitL:
        case R3_VelocityFilterLimitH:
        case R3_PositionFilterLimitL:
        case R3_PositionFilterLimitH:
        case R3_CurrentLimit:
        case R3_VelocityLimit:
        case R3_Inertia:
        case R3_PositionLimitH:
        case R3_PositionLimitL:
        case R3_PositionOffset:
        case R3_HomingCurrentLimitL:
        case R3_HomingCurrentLimitH:
        case R3_BlockEngy:
            R3dataProcess(pSCA, RxMsg);
            break;

        case R4_CVP:
            R4dataProcess(pSCA, RxMsg);
            break;

        case R5_ShakeHands:
            R5dataProcess(pSCA, RxMsg);
            break;

            /* 其余为写入指令，判断写入是否成功，更新句柄 */
        default:
            WriteDataProcess(pSCA, RxMsg);
            break;
    }
}

/**
  * @功	能	识别错误代码中的具体错误信息
  * @参	数	pSCA：要操作的执行器句柄地址或指针
  * @返	回	无
  */
void warnBitAnaly(SCA_Handler_t *pSCA)
{
    if (pSCA->SCA_Warn.Error_Code & 0x0001)
        pSCA->SCA_Warn.WARN_OVER_VOLT = Actr_Enable;
    else
        pSCA->SCA_Warn.WARN_OVER_VOLT = Actr_Disable;

    if (pSCA->SCA_Warn.Error_Code & 0x0002)
        pSCA->SCA_Warn.WARN_UNDER_VOLT = Actr_Enable;
    else
        pSCA->SCA_Warn.WARN_UNDER_VOLT = Actr_Disable;

    if (pSCA->SCA_Warn.Error_Code & 0x0004)
        pSCA->SCA_Warn.WARN_LOCK_ROTOR = Actr_Enable;
    else
        pSCA->SCA_Warn.WARN_LOCK_ROTOR = Actr_Disable;

    if (pSCA->SCA_Warn.Error_Code & 0x0008)
        pSCA->SCA_Warn.WARN_OVER_TEMP = Actr_Enable;
    else
        pSCA->SCA_Warn.WARN_OVER_TEMP = Actr_Disable;

    if (pSCA->SCA_Warn.Error_Code & 0x0010)
        pSCA->SCA_Warn.WARN_RW_PARA = Actr_Enable;
    else
        pSCA->SCA_Warn.WARN_RW_PARA = Actr_Disable;

    if (pSCA->SCA_Warn.Error_Code & 0x0020)
        pSCA->SCA_Warn.WARN_MUL_CIRCLE = Actr_Enable;
    else
        pSCA->SCA_Warn.WARN_MUL_CIRCLE = Actr_Disable;

    if (pSCA->SCA_Warn.Error_Code & 0x0040)
        pSCA->SCA_Warn.WARN_TEMP_SENSOR_INV = Actr_Enable;
    else
        pSCA->SCA_Warn.WARN_TEMP_SENSOR_INV = Actr_Disable;

    if (pSCA->SCA_Warn.Error_Code & 0x0080)
        pSCA->SCA_Warn.WARN_CAN_BUS = Actr_Enable;
    else
        pSCA->SCA_Warn.WARN_CAN_BUS = Actr_Disable;

    if (pSCA->SCA_Warn.Error_Code & 0x0100)
        pSCA->SCA_Warn.WARN_TEMP_SENSOR_MTR = Actr_Enable;
    else
        pSCA->SCA_Warn.WARN_TEMP_SENSOR_MTR = Actr_Disable;

    if (pSCA->SCA_Warn.Error_Code & 0x0200)
        pSCA->SCA_Warn.WARN_OVER_STEP = Actr_Enable;
    else
        pSCA->SCA_Warn.WARN_OVER_STEP = Actr_Disable;

    if (pSCA->SCA_Warn.Error_Code & 0x0400)
        pSCA->SCA_Warn.WARN_DRV_PROTEC = Actr_Enable;
    else
        pSCA->SCA_Warn.WARN_DRV_PROTEC = Actr_Disable;

    if (pSCA->SCA_Warn.Error_Code & 0xF800)
        pSCA->SCA_Warn.WARN_DVICE = Actr_Enable;
    else
        pSCA->SCA_Warn.WARN_DVICE = Actr_Disable;

}

