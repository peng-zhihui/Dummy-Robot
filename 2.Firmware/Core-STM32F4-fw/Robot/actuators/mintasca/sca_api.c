/**
  ******************************************************************************
  * @文	件 ： SCA_API.c
  * @作	者 ： INNFOS Software Team
  * @版	本 ： V1.5.3
  * @日	期 ： 2019.09.10
  * @摘	要 ： SCA 控制接口层
  ******************************************************************************/
/* Update log --------------------------------------------------------------------*/
//V1.1.0 2019.08.05 所有API调用接口改为ID，与PC SDK保持一致，增加所有参数的读写API
//V1.5.0 2019.08.16 更改数据接收方式（中断接收），加入非阻塞通信功能，适应数据返回慢的
//					情况。加入获取上次关机状态的API，优化开机流程。
//V1.5.1 2019.09.10 增加轮询功能
//V1.5.3 2019.11.15 优化开关机流程

/* Includes ----------------------------------------------------------------------*/
#include "sca_api.h"
/* Variable defines --------------------------------------------------------------*/

/* 每个SCA都需要一个句柄来保存对应的信息，根据实际使用数量进行定义 SCA_NUM_USE */
SCA_Handler_t SCA_Handler_List[SCA_NUM_USE];

/* Funcation declaration ---------------------------------------------------------*/
extern void warnBitAnaly(SCA_Handler_t* pSCA);

/* Funcation defines -------------------------------------------------------------*/

/****************************控制相关*******************************/

/**
  * @功	能	在CAN总线上查找存在的SCA，并打印找到的ID
  * @参	数	canPort：需要轮询的总线
  * @返	回	无
  * @注	意	每台执行器都有自己的ID，若初次使用不知道
  *			对应的ID，可用此函数查找
  */
void lookupActuators(CAN_Handler_t* canPort)
{
    uint16_t ID;
    uint8_t Found = 0;
    SCA_Handler_t temp;

    /* 保存列表项的原始内容 */
    temp = SCA_Handler_List[0];

    /* 使用一个列表项进行查询 */
    SCA_Handler_List[0].Can = canPort;

    for(ID = 1; ID <= 0xFF; ID++)
    {
        /* 装载新的ID */
        SCA_Handler_List[0].ID = ID;

        /* 收到该ID的心跳，则该ID存在 */
        if(isOnline(ID,Block) == SCA_NoError)
        {
            /* 记录找到的个数，打印找到的ID */
            Found++;
            SCA_Debug("Found ID %d in canPort %d\r\n",ID,canPort->CanPort);
        }
    }
    /* 恢复更改的内容 */
    SCA_Handler_List[0] = temp;

    /* 输出提示信息 */
    SCA_Debug("canPort %d polling done ! Found %d Actuators altogether!\r\n\r\n",canPort->CanPort,Found);
}

/**
  * @功	能	初始化控制器，用于ID和CAN端口信息
  * @参	数	id：初始化执行器的ID
  *			pCan：使用的CAN端口地址
  * @返	回	无
  * @注	意	定义次数不要超过SCA_NUM_USE
  */
void setupActuators(uint8_t id, CAN_Handler_t* pCan)
{
    static uint32_t i = 0;

    /* 定义数量超过使用数量 */
    if(i >= SCA_NUM_USE)	return;

    /* 句柄绑定信息 */
    SCA_Handler_List[i].ID = id;
    SCA_Handler_List[i].Can = pCan;

    /* 列表项增加 */
    i++;
}

/**
  * @功	能	复位控制器，用于SCA因错误导致的死机重启
  * @参	数	id：0表示全部复位，不为0时则复位指定ID的控制器
  * @返	回	无
  * @注	意	如果出现红灯或蓝灯状态死机的SCA，请先将
  *			SCA重新上电，恢复至黄灯状态然后执行此函
  *			数,再执行开机函数即可完成死机重启
  */
void resetController(uint8_t id)
{
    uint8_t i,id_temp;
    CAN_Handler_t* pCan_temp = NULL;

    if(id == 0)
    {
        /* 清空所有信息句柄 */
        for(i = 0; i < SCA_NUM_USE; i++)
        {
            /* 保留ID与CAN端口地址 */
            id_temp = SCA_Handler_List[i].ID;
            pCan_temp = SCA_Handler_List[i].Can;

            /* 结构体清零 */
            memset(&SCA_Handler_List[i], 0, sizeof(SCA_Handler_List[i]));

            /* 恢复ID与CAN端口地址 */
            SCA_Handler_List[i].ID = id_temp;
            SCA_Handler_List[i].Can = pCan_temp;
        }
    }else
    {
        /* 获取该ID的信息句柄 */
        SCA_Handler_t* pSCA = getInstance(id);
        if(pSCA == NULL)	return;

        /* 保留CAN端口地址 */
        pCan_temp = pSCA->Can;

        /* 结构体清零 */
        memset(pSCA, 0, sizeof(SCA_Handler_List[0]));

        /* 恢复ID与CAN端口地址 */
        pSCA->ID = id;
        pSCA->Can = pCan_temp;
    }
}

/**
  * @功	能	获取指定ID的SCA信息句柄
  * @参	数	id ：要获取信息的执行器ID
  * @返	回	NULL：未查找到该ID的信息句柄
  *			其他：查找到的信息句柄
  */
SCA_Handler_t* getInstance(uint8_t id)
{
    uint8_t i;

    for(i = 0; i < SCA_NUM_USE; i++)
        if(SCA_Handler_List[i].ID == id)
            return &SCA_Handler_List[i];

    return NULL;
}

/**
  * @功	能	检查执行器的心跳（在线）状态
  * @参	数	id ：要检查的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：该执行器在线
  *			SCA_OverTime：该执行器离线
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t isOnline(uint8_t id, uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = getInstance(id);

    if(pSCA == NULL)	return SCA_UnknownID;

    /* 先清空在线状态 */
    pSCA->Online_State = Actr_Disable;

    /* 调用读取命令与SCA通信，结果放入对应的SCA句柄中 */
    Error = SCA_Read(pSCA, R1_Heartbeat);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 阻塞式通信 */
    while((pSCA->Online_State != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	检查执行器的使能状态
  * @参	数	id ：要检查的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	Actr_Enable：该执行器已使能
  *			Actr_Disable：该执行器未使能
  *
  */
uint8_t isEnable(uint8_t id, uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 先清空读取标志位 */
    pSCA->paraCache.R_Power_State = Actr_Disable;

    /* 调用读取命令与SCA通信，结果放入对应的SCA句柄中 */
    Error = SCA_Read(pSCA, R1_PowerState);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 阻塞式通信 */
    while((pSCA->paraCache.R_Power_State != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	检查执行器的参数更新状态
  * @参	数	id ：要检查的执行器id
  * @返	回	Actr_Enable：有参数更新
  *			Actr_Disable：没有参数更新
  */
uint8_t isUpdate(uint8_t id)
{
    uint8_t State;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 保存更新状态，并复位 */
    State = pSCA->Update_State;
    pSCA->Update_State = Actr_Disable;

    return State;
}

/**
  * @功	能	使能所有执行器，阻塞式
  * @参	数	无
  * @返	回	无
  */
void enableAllActuators()
{
    uint8_t i;

    for(i = 0; i < SCA_NUM_USE; i++)
        enableActuator(SCA_Handler_List[i].ID);
}

/**
  * @功	能	失能所有执行器，阻塞式
  * @参	数	无
  * @返	回	无
  */
void disableAllActuators()
{
    uint8_t i;

    for(i = 0; i < SCA_NUM_USE; i++)
        disableActuator(SCA_Handler_List[i].ID);
}

/**
  * @功	能	执行器使能,阻塞式
  * @参	数	id：要使能的执行器ID
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t enableActuator(uint8_t id)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 查询一次当前的使能状态 */
    Error = isEnable(id, Block);
    if(Error)	return Error;

    /* 若当前已经处于目标状态，直接返回成功 */
    if(pSCA->Power_State == Actr_Enable)	goto PowerOn;

    /* 目标参数写入缓存待更新 */
    pSCA->paraCache.Power_State = Actr_Enable;

    /* 执行开机命令 */
    Error = SCA_Write_1(pSCA, W1_PowerState, Actr_Enable);
    if(Error)	return Error;

    /* 等待开机成功，更新句柄信息 */
    while((pSCA->Power_State != Actr_Enable) && (waitime++ < CanPowertime));
    if(waitime >= CanPowertime)	return SCA_OperationFailed;

    PowerOn:
    /* 更新在线状态 */
    pSCA->Online_State = Actr_Enable;

    /* 读出设备序列号，更改ID用 */
    getActuatorSerialNumber(id,Block);

    /* 读一次上次关机的异常状态 */
    getActuatorLastState(id,Block);
    if(pSCA->Last_State == 0)		//提示上次关机状态异常
        SCA_Debug("ID:%d Last_State Error\r\n",pSCA->ID);

    /*  读出执行器的满量程电流值，在读写电流环参数时使用，
        不同型号的SCA该值不同，也可以手动更新到句柄信息中
        该参数值是必须获取的。*/
    getCurrentRange(id,Block);
    if(pSCA->Current_Max == 0)	//未获取到电流满量程值，无法写入电流值
        SCA_Debug("ID:%d Current_Max Error\r\n",pSCA->ID);

    /* 更新一次所有参数到句柄中，为缩短开机时间采用非阻塞 */
    regainAttrbute(id,Unblock);

    return Error;
}

/**
  * @功	能	执行器失能,阻塞
  * @参	数	id：要失能的执行器ID
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t disableActuator(uint8_t id)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 查询一次当前的使能状态 */
    Error = isEnable(id, Block);
    if(Error)	return Error;

    /* 若当前已经处于目标状态，直接返回成功 */
    if(pSCA->Power_State == Actr_Disable)	return SCA_NoError;

    /* 目标参数写入缓存待更新 */
    pSCA->paraCache.Power_State = Actr_Disable;

    /* 执行关机命令 */
    Error = SCA_Write_1(pSCA, W1_PowerState, Actr_Disable);
    if(Error)	return Error;

    /* 等待关机成功 */
    while((pSCA->Power_State != Actr_Disable) && (waitime++ < CanPowertime));
    if(waitime >= CanPowertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	执行器切换操作模式
  * @参	数	id：要操作的执行器id
  *			mode：操作模式，详见 SCA_Protocol.h
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t activateActuatorMode(uint8_t id, uint8_t ActuatorMode, uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 若当前已经处于目标状态，直接返回成功 */
    if(pSCA->Mode == ActuatorMode)	return SCA_NoError;

    /* 目标参数写入缓存待更新 */
    pSCA->paraCache.Mode = ActuatorMode;

    /* 执行模式切换命令 */
    Error = SCA_Write_1(pSCA, W1_Mode, ActuatorMode);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Mode != ActuatorMode) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	执行器读取当前操作模式
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getActuatorMode(uint8_t id, uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 先清空读取等待标志位 */
    pSCA->paraCache.R_Mode = Actr_Disable;

    /* 封装读取函数，读出值直接保存到句柄中 */
    Error = SCA_Read(pSCA, R1_Mode);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Mode != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	执行器读取报警信息，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getErrorCode(uint8_t id, uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 先清空读取等待标志位 */
    pSCA->paraCache.R_Error_Code = Actr_Disable;

    /* 执行读取错误信息命令 */
    Error = SCA_Read(pSCA, R2_Error);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Error_Code != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	执行器清除报警信息
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t clearError(uint8_t id, uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 若当前无错误，则无需淸错 */
    if(pSCA->SCA_Warn.Error_Code == 0)	return SCA_NoError;

    /* 执行淸错命令 */
    Error = SCA_Write_4(pSCA, W4_ClearError);

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->SCA_Warn.Error_Code != 0) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	执行器获取当前所有参数
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	无
  */
void regainAttrbute(uint8_t id,uint8_t isBlock)
{
    getErrorCode(id,isBlock);
    requestCVPValue(id,isBlock);
    getActuatorMode(id,isBlock);
    getPositionKp(id,isBlock);
    getPositionKi(id,isBlock);
    getPositionUmax(id,isBlock);
    getPositionUmin(id,isBlock);
    getPositionOffset(id,isBlock);
    getMaximumPosition(id,isBlock);
    getMinimumPosition(id,isBlock);
    isPositionLimitEnable(id,isBlock);
    isPositionFilterEnable(id,isBlock);
    getPositionCutoffFrequency(id,isBlock);
    getProfilePositionAcceleration(id,isBlock);
    getProfilePositionDeceleration(id,isBlock);
    getProfilePositionMaxVelocity(id,isBlock);
    getVelocityKp(id,isBlock);
    getVelocityKi(id,isBlock);
    getVelocityUmax(id,isBlock);
    getVelocityUmin(id,isBlock);
    isVelocityFilterEnable(id,isBlock);
    getVelocityCutoffFrequency(id,isBlock);
    getVelocityLimit(id,isBlock);
    getProfileVelocityAcceleration(id,isBlock);
    getProfileVelocityDeceleration(id,isBlock);
    getProfileVelocityMaxVelocity(id,isBlock);
    getCurrentKp(id,isBlock);
    getCurrentKi(id,isBlock);
    isCurrentFilterEnable(id,isBlock);
    getCurrentCutoffFrequency(id,isBlock);
    getCurrentLimit(id,isBlock);
    getVoltage(id,isBlock);
    getLockEnergy(id,isBlock);
    getMotorTemperature(id,isBlock);
    getInverterTemperature(id,isBlock);
    getMotorProtectedTemperature(id,isBlock);
    getMotorRecoveryTemperature(id,isBlock);
    getInverterProtectedTemperature(id,isBlock);
    getInverterRecoveryTemperature(id,isBlock);
}
/**
  * @功	能	执行器保存当前所有参数
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t saveAllParams(uint8_t id, uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空存储状态位 */
    pSCA->Save_State = Actr_Disable;

    Error = SCA_Write_4(pSCA, W4_Save);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行成功 */
    while((pSCA->Save_State != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanPowertime)	return SCA_OperationFailed;

    return Error;
}


/****************************位置相关*******************************/

/**
  * @功	能	执行器设置当前位置值
  * @参	数	id：要操作的执行器id
  *			pos：目标位置值，实际值，范围 -127.0R ~ +127.0R
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setPosition(uint8_t id, float pos)
{
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    return SCA_Write_3(pSCA, W3_Position, pos);
}

/**
  * @功	能	执行器设置当前位置值，快速
  * @参	数	pSCA：要操作的执行器句柄指针或地址
  *			pos：目标位置值，实际值，范围 -127.0R ~ +127.0R
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setPositionFast(SCA_Handler_t* pSCA, float pos)
{
    return SCA_Write_3(pSCA, W3_Position, pos);
}

/**
  * @功	能	执行器读取当前位置值,更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getPosition(uint8_t id, uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Position_Real = Actr_Disable;

    Error = SCA_Read(pSCA, R3_Position);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行成功 */
    while((pSCA->paraCache.R_Position_Real != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanPowertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	执行器读取当前位置值,更新至句柄中，快速
  * @参	数	pSCA：要操作的执行器句柄地址或指针
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getPositionFast(SCA_Handler_t* pSCA, uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;

    /* 清空状态位 */
    pSCA->paraCache.R_Position_Real = Actr_Disable;

    Error = SCA_Read(pSCA, R3_Position);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行成功 */
    while((pSCA->paraCache.R_Position_Real != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanPowertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器位置环 Kp值
  * @参	数	id：要操作的执行器id
  *			Kp：目标位置环 Kp值，实际值
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setPositionKp(uint8_t id,float Kp, uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Position_Filter_P = Kp;

    Error = SCA_Write_3(pSCA, W3_PositionFilterP, Kp);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Position_Filter_P != Kp) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器位置环 Kp值，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getPositionKp(uint8_t id, uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Position_Filter_P = Actr_Disable;

    Error = SCA_Read(pSCA, R3_PositionFilterP);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Position_Filter_P != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器位置环 Ki值
  * @参	数	id：要操作的执行器id
  *			Ki：目标位置环 Ki值，实际值
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setPositionKi(uint8_t id,float Ki, uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Position_Filter_I = Ki;

    Error = SCA_Write_3(pSCA, W3_PositionFilterI, Ki);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Position_Filter_I != Ki) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器位置环 Ki值，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getPositionKi(uint8_t id, uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Position_Filter_I = Actr_Disable;

    Error = SCA_Read(pSCA, R3_PositionFilterI);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Position_Filter_I != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器位置环输出上限值
  * @参	数	id：要操作的执行器id
  *			max：目标位置环输出上限值，实际值
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setPositionUmax(uint8_t id,float max,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Position_Filter_Limit_H = max;

    Error = SCA_Write_3(pSCA, W3_PositionFilterLimitH, max);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Position_Filter_Limit_H != max) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器位置环输出上限值，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getPositionUmax(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Position_Filter_Limit_H = Actr_Disable;

    Error = SCA_Read(pSCA, R3_PositionFilterLimitH);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Position_Filter_Limit_H != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器位置环输出下限值
  * @参	数	id：要操作的执行器id
  *			min：目标位置环输出下限值，实际值
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setPositionUmin(uint8_t id,float min,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Position_Filter_Limit_L = min;

    Error = SCA_Write_3(pSCA, W3_PositionFilterLimitL, min);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Position_Filter_Limit_L != min) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器位置环输出下限值，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getPositionUmin(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Position_Filter_Limit_L = Actr_Disable;

    Error = SCA_Read(pSCA, R3_PositionFilterLimitL);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Position_Filter_Limit_L != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器位置偏置值
  * @参	数	id：要操作的执行器id
  *			offset：目标位置偏置值，实际值
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setPositionOffset(uint8_t id, float offset,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Position_Offset = offset;

    Error = SCA_Write_3(pSCA, W3_PositionOffset, offset);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Position_Offset != offset) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器位置偏置值，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getPositionOffset(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Position_Offset = Actr_Disable;

    Error = SCA_Read(pSCA, R3_PositionOffset);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Position_Offset != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器位置最大值
  * @参	数	id：要操作的执行器id
  *			maxPos：目标位置最大值，实际值
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setMaximumPosition(uint8_t id,float maxPos,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Position_Limit_H = maxPos;

    Error = SCA_Write_3(pSCA, W3_PositionLimitH, maxPos);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Position_Limit_H != maxPos) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器位置最大值，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getMaximumPosition(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Position_Limit_H = Actr_Disable;

    Error = SCA_Read(pSCA, R3_PositionLimitH);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Position_Limit_H != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器位置最小值
  * @参	数	id：要操作的执行器id
  *			minPos：目标位置最小值，实际值
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setMinimumPosition(uint8_t id,float minPos,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Position_Limit_L = minPos;

    Error = SCA_Write_3(pSCA, W3_PositionLimitL, minPos);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Position_Limit_L != minPos) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器位置最小值，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getMinimumPosition(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Position_Limit_L = Actr_Disable;

    Error = SCA_Read(pSCA, R3_PositionLimitL);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Position_Limit_L != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	使能或失能执行器位置限位
  * @参	数	id：要操作的执行器id
  *			enable：使能状态，Actr_Enable使能，Actr_Disable失能
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t enablePositionLimit(uint8_t id, uint8_t enable,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Position_Limit_State = enable;

    Error = SCA_Write_1(pSCA, W1_PositionLimitState, enable);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Position_Limit_State != enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器位置限位使能状态，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t isPositionLimitEnable(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Position_Limit_State = Actr_Disable;

    Error = SCA_Read(pSCA, R1_PositionLimitState);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Position_Limit_State != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器零点位置，重新计算左右限位
  * @参	数	id：要操作的执行器id
  *			homingPos：零点位置，实际值，单位 R
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setHomingPosition(uint8_t id,float homingPos,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Homing_Value = homingPos;

    Error = SCA_Write_3(pSCA, W3_HomingValue, homingPos);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Homing_Value != homingPos) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	使能执行器位置环滤波器
  * @参	数	id：要操作的执行器id
  *			enable：使能状态，Actr_Enable使能，Actr_Disable失能
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t enablePositionFilter(uint8_t id,uint8_t enable,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Position_Filter_State = enable;

    Error = SCA_Write_1(pSCA, W1_PositionFilterState, enable);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Position_Filter_State != enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器位置环滤波器使能状态，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t isPositionFilterEnable(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Position_Filter_State = Actr_Disable;

    Error = SCA_Read(pSCA, R1_PositionFilterState);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Position_Filter_State != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器位置环滤波器带宽
  * @参	数	id：要操作的执行器id
  *			frequency：滤波器带宽，实际值，单位 hz
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setPositionCutoffFrequency(uint8_t id, float frequency,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Position_Filter_Value = frequency;

    Error = SCA_Write_2(pSCA, W2_PositionFilterValue, frequency);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Position_Filter_Value != frequency) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器位置环滤波器带宽，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getPositionCutoffFrequency(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Position_Filter_Value = Actr_Disable;

    Error = SCA_Read(pSCA, R2_PositionFilterValue);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Position_Filter_Value != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	清除homing信息，包括左右极限和0位，待定
  * @参	数	id：执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t clearHomingInfo(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.W_ClearHome = Actr_Disable;

    Error = SCA_Write_4(pSCA, W4_ClearHome);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.W_ClearHome != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器梯形位置环最大加速度
  * @参	数	id：要操作的执行器id
  *			acceleration：最大加速度，实际值，单位 RPM/S^2
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setProfilePositionAcceleration(uint8_t id, float acceleration,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.PP_Max_Acceleration = acceleration;

    /*  梯形加速度传输值是真实值的IQ20倍，第三类读写接口是以
        IQ24格式传输的，需要做IQ4的倍数处理。另外，改数值的
        单位是RPM，需将该数值缩放60变成RPM单位。
        最终缩放值 = 2^4 * 60 = 960
    */
    acceleration /= Profile_Scal;

    Error = SCA_Write_3(pSCA, W3_PPMaxAcceleration, acceleration);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->PP_Max_Acceleration != acceleration) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器梯形位置环最大加速度，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getProfilePositionAcceleration(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_PP_Max_Acceleration = Actr_Disable;

    Error = SCA_Read(pSCA, R3_PPMaxAcceleration);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_PP_Max_Acceleration != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器梯形位置环最大减速度
  * @参	数	id：要操作的执行器id
  *			deceleration：最大减速度，实际值，单位 RPM/S^2
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setProfilePositionDeceleration(uint8_t id, float deceleration,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.PP_Max_Deceleration = deceleration;

    deceleration /= Profile_Scal;

    Error = SCA_Write_3(pSCA, W3_PPMaxDeceleration, deceleration);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->PP_Max_Deceleration != deceleration) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器梯形位置环最大减速度，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getProfilePositionDeceleration(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_PP_Max_Deceleration = Actr_Disable;

    Error = SCA_Read(pSCA, R3_PPMaxDeceleration);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_PP_Max_Deceleration != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器梯形位置环最大速度
  * @参	数	id：要操作的执行器id
  *			maxVelocity：最大速度，实际值，单位 RPM
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setProfilePositionMaxVelocity(uint8_t id, float maxVelocity,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.PP_Max_Velocity = maxVelocity;

    maxVelocity /= Profile_Scal;

    Error = SCA_Write_3(pSCA, W3_PPMaxVelocity, maxVelocity);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->PP_Max_Velocity != maxVelocity) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器梯形位置环最大速度，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getProfilePositionMaxVelocity(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_PP_Max_Velocity = Actr_Disable;

    Error = SCA_Read(pSCA, R3_PPMaxVelocity);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_PP_Max_Velocity != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}


/****************************速度相关*******************************/

/**
  * @功	能	设置执行器当前速度值
  * @参	数	id：要操作的执行器id
  *			vel：目标速度，实际值，单位 RPM
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setVelocity(uint8_t id,float vel)
{
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    return SCA_Write_3(pSCA, W3_Velocity, vel);
}

/**
  * @功	能	设置执行器当前速度值,快速
  * @参	数	pSCA：要操作的执行器句柄指针或地址
  *			vel：目标速度，实际值，单位 RPM
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setVelocityFast(SCA_Handler_t* pSCA,float vel)
{
    return SCA_Write_3(pSCA, W3_Velocity, vel);
}


/**
  * @功	能	获取执行器当前速度，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getVelocity(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Velocity_Real = Actr_Disable;

    Error = SCA_Read(pSCA, R3_Velocity);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Velocity_Real != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器当前速度，更新至句柄中,快速
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getVelocityFast(SCA_Handler_t* pSCA,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;

    /* 清空状态位 */
    pSCA->paraCache.R_Velocity_Real = Actr_Disable;

    Error = SCA_Read(pSCA, R3_Velocity);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Velocity_Real != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器速度环比例，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getVelocityKp(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Velocity_Filter_P = Actr_Disable;

    Error = SCA_Read(pSCA, R3_VelocityFilterP);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Velocity_Filter_P != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器速度环比例
  * @参	数	id：要操作的执行器id
  *			Kp：速度环比例，实际值
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setVelocityKp(uint8_t id,float Kp,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Velocity_Filter_P = Kp;

    Error = SCA_Write_3(pSCA, W3_VelocityFilterP, Kp);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Velocity_Filter_P != Kp) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器速度环积分，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getVelocityKi(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Velocity_Filter_I = Actr_Disable;

    Error = SCA_Read(pSCA, R3_VelocityFilterI);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Velocity_Filter_I != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器速度环积分
  * @参	数	id：要操作的执行器id
  *			Ki：速度环积分，实际值
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setVelocityKi(uint8_t id, float Ki,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Velocity_Filter_I = Ki;

    Error = SCA_Write_3(pSCA, W3_VelocityFilterI, Ki);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Velocity_Filter_I != Ki) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器速度环最大输出限幅，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getVelocityUmax(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Velocity_Filter_Limit_H = Actr_Disable;

    Error = SCA_Read(pSCA, R3_VelocityFilterLimitH);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Velocity_Filter_Limit_H != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器速度环最大输出限幅
  * @参	数	id：要操作的执行器id
  *			max：最大输出限幅，实际值
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setVelocityUmax(uint8_t id, float max,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Velocity_Filter_Limit_H = max;

    Error = SCA_Write_3(pSCA, W3_VelocityFilterLimitH, max);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Velocity_Filter_Limit_H != max) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器速度环最小输出限幅，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getVelocityUmin(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Velocity_Filter_Limit_L = Actr_Disable;

    Error = SCA_Read(pSCA, R3_VelocityFilterLimitL);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Velocity_Filter_Limit_L != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器速度环最小输出限幅
  * @参	数	id：要操作的执行器id
  *			min：最小输出限幅，实际值
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setVelocityUmin(uint8_t id, float min,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Velocity_Filter_Limit_L = min;

    Error = SCA_Write_3(pSCA, W3_VelocityFilterLimitL, min);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Velocity_Filter_Limit_L != min) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器速度环速度量程
  * @参	数	id：要操作的执行器id
  * @返	回	速度环速度量程，实际值
  */
float getVelocityRange(uint8_t id)
{
    return Velocity_Max;
}

/**
  * @功	能	使能执行器速度环滤波器
  * @参	数	id：要操作的执行器id
  *			enable：使能状态，Actr_Enable使能，Actr_Disable失能
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t enableVelocityFilter(uint8_t id,uint8_t enable,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Velocity_Filter_State = enable;

    Error = SCA_Write_1(pSCA, W1_VelocityFilterState, enable);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Velocity_Filter_State != enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器速度环滤波器使能状态，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t isVelocityFilterEnable(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Velocity_Filter_State = Actr_Disable;

    Error = SCA_Read(pSCA, R1_VelocityFilterState);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Velocity_Filter_State != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器速度环滤波器带宽，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getVelocityCutoffFrequency(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Velocity_Filter_Value = Actr_Disable;

    Error = SCA_Read(pSCA, R2_VelocityFilterValue);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Velocity_Filter_Value != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器速度环滤波器带宽
  * @参	数	id：要操作的执行器id
  *			frequency：滤波器带宽，实际值，单位 hz
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setVelocityCutoffFrequency(uint8_t id, float frequency,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Velocity_Filter_Value = frequency;

    Error = SCA_Write_2(pSCA, W2_VelocityFilterValue, frequency);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Velocity_Filter_Value != frequency) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器速度环输入限幅
  * @参	数	id：要操作的执行器id
  *			limit：输入限幅
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setVelocityLimit(uint8_t id,float limit,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Velocity_Limit = limit;

    Error = SCA_Write_3(pSCA, W3_VelocityLimit, limit);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Velocity_Limit != limit) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器速度环输入限幅，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getVelocityLimit(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Velocity_Limit = Actr_Disable;

    Error = SCA_Read(pSCA, R3_VelocityLimit);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Velocity_Limit != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器梯形速度环加速度
  * @参	数	id：要操作的执行器id
  *			acceleration：加速度，实际值
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setProfileVelocityAcceleration(uint8_t id,float acceleration,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.PV_Max_Acceleration = acceleration;

    acceleration /= Profile_Scal;

    Error = SCA_Write_3(pSCA, W3_PVMaxAcceleration, acceleration);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->PV_Max_Acceleration != acceleration) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器梯形速度环加速度，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getProfileVelocityAcceleration(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_PV_Max_Acceleration = Actr_Disable;

    Error = SCA_Read(pSCA, R3_PVMaxAcceleration);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_PV_Max_Acceleration != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器梯形速度环减速度
  * @参	数	id：要操作的执行器id
  *			deceleration：减速度，实际值
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setProfileVelocityDeceleration(uint8_t id,float deceleration,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.PV_Max_Deceleration = deceleration;

    deceleration /= Profile_Scal;

    Error = SCA_Write_3(pSCA, W3_PVMaxDeceleration, deceleration);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->PV_Max_Deceleration != deceleration) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器梯形速度环减速度，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getProfileVelocityDeceleration(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_PV_Max_Deceleration = Actr_Disable;

    Error = SCA_Read(pSCA, R3_PVMaxDeceleration);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_PV_Max_Deceleration != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器梯形速度环最大速度
  * @参	数	id：要操作的执行器id
  *			maxVelocity：最大速度，实际值，单位 RPM
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setProfileVelocityMaxVelocity(uint8_t id, float maxVelocity,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.PV_Max_Velocity = maxVelocity;

    maxVelocity /= Profile_Scal;

    Error = SCA_Write_3(pSCA, W3_PVMaxVelocity, maxVelocity);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->PV_Max_Velocity != maxVelocity) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器梯形速度环最大速度，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getProfileVelocityMaxVelocity(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_PV_Max_Velocity = Actr_Disable;

    Error = SCA_Read(pSCA, R3_PVMaxVelocity);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_PV_Max_Velocity != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}


/****************************电流相关*******************************/

/**
  * @功	能	设置执行器当前电流值
  * @参	数	id：要操作的执行器id
  *			current：当前电流值，实际值，单位 A
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setCurrent(uint8_t id,float current)
{
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    return SCA_Write_3(pSCA, W3_Current, current);
}

/**
  * @功	能	设置执行器当前电流值，快速
  * @参	数	pSCA：要操作的执行器句柄指针或地址
  *			current：当前电流值，实际值，单位 A
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setCurrentFast(SCA_Handler_t* pSCA,float current)
{
    return SCA_Write_3(pSCA, W3_Current, current);
}

/**
  * @功	能	获取执行器当前电流值，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getCurrent(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Current_Real = Actr_Disable;

    Error = SCA_Read(pSCA, R3_Current);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Current_Real != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器当前电流值，更新至句柄中,快速
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getCurrentFast(SCA_Handler_t* pSCA,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;

    /* 清空状态位 */
    pSCA->paraCache.R_Current_Real = Actr_Disable;

    Error = SCA_Read(pSCA, R3_Current);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Current_Real != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器电流环比例值，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getCurrentKp(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 清空状态位 */
    pSCA->paraCache.R_Current_Filter_P = Actr_Disable;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    Error = SCA_Read(pSCA, R3_CurrentFilterP);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Current_Filter_P != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器电流环积分，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getCurrentKi(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Current_Filter_I = Actr_Disable;

    Error = SCA_Read(pSCA, R3_CurrentFilterI);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Current_Filter_I != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;

}

/**
  * @功	能	获取执行器电流量程，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getCurrentRange(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Current_Max = Actr_Disable;

    Error = SCA_Read(pSCA, R2_Current_Max);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Current_Max != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;

}

/**
  * @功	能	使能执行器电流环滤波器
  * @参	数	id：要操作的执行器id
  *			enable：使能状态，Actr_Enable使能，Actr_Disable失能
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t enableCurrentFilter(uint8_t id,uint8_t enable,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Current_Filter_State = enable;

    Error = SCA_Write_1(pSCA, W1_CurrentFilterState, enable);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Current_Filter_State != enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器电流环滤波器使能状态，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t isCurrentFilterEnable(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Current_Filter_State = Actr_Disable;

    Error = SCA_Read(pSCA, R1_CurrentFilterState);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Current_Filter_State != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;

}

/**
  * @功	能	获取执行器电流环滤波器带宽，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getCurrentCutoffFrequency(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Current_Filter_Value = Actr_Disable;

    Error = SCA_Read(pSCA, R2_CurrentFilterValue);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Current_Filter_Value != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;

}

/**
  * @功	能	设置执行器电流环滤波器带宽
  * @参	数	id：要操作的执行器id
  *			frequency：目标截止频率，单位 hz
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setCurrentCutoffFrequency(uint8_t id, float frequency,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Current_Filter_Value = frequency;

    Error = SCA_Write_2(pSCA, W2_CurrentFilterValue, frequency);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Current_Filter_Value != frequency) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器电流环输入限幅
  * @参	数	id：要操作的执行器id
  *			limit：输入限幅
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setCurrentLimit(uint8_t id,float limit,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Current_Limit = limit;

    Error = SCA_Write_3(pSCA, W3_CurrentLimit, limit);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Current_Limit != limit) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器电流环输入限幅，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getCurrentLimit(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Current_Limit = Actr_Disable;

    Error = SCA_Read(pSCA, R3_CurrentLimit);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Current_Limit != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;

}

/****************************其他参数*******************************/

/**
  * @功	能	获取执行器电压，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getVoltage(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Voltage = Actr_Disable;

    Error = SCA_Read(pSCA, R2_Voltage);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Voltage != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器堵转能量，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getLockEnergy(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Blocked_Energy = Actr_Disable;

    Error = SCA_Read(pSCA, R3_BlockEngy);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Blocked_Energy != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;

}

/**
  * @功	能	设置执行器堵转能量值
  * @参	数	id：要操作的执行器id
  *			energy：堵转能量值，实际值，单位 J
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setLockEnergy(uint8_t id,float energy,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Blocked_Energy = energy;

    Error = SCA_Write_3(pSCA, W3_BlockEngy, energy);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Blocked_Energy != energy) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器电机温度值，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getMotorTemperature(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Motor_Temp = Actr_Disable;

    Error = SCA_Read(pSCA, R2_MotorTemp);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Motor_Temp != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;

}

/**
  * @功	能	获取执行器逆变器温度值，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getInverterTemperature(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Inverter_Temp = Actr_Disable;

    Error = SCA_Read(pSCA, R2_InverterTemp);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Inverter_Temp != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;

}

/**
  * @功	能	获取执行器电机保护温度值，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getMotorProtectedTemperature(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Inverter_Protect_Temp = Actr_Disable;

    Error = SCA_Read(pSCA, R2_MotorProtectTemp);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Inverter_Protect_Temp != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;

}

/**
  * @功	能	设置执行器电机保护温度值
  * @参	数	id：要操作的执行器id
  *			temp：电机保护温度值，实际值，单位 摄氏度
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setMotorProtectedTemperature(uint8_t id,float temp,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Motor_Protect_Temp = temp;

    Error = SCA_Write_2(pSCA, W2_MotorProtectTemp, temp);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Motor_Protect_Temp != temp) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器电机恢复温度值，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getMotorRecoveryTemperature(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Motor_Recover_Temp = Actr_Disable;

    Error = SCA_Read(pSCA, R2_MotorRecoverTemp);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Motor_Recover_Temp != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器电机恢复温度值
  * @参	数	id：要操作的执行器id
  *			temp：电机恢复温度值，实际值，单位 摄氏度
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setMotorRecoveryTemperature(uint8_t id,float temp,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Motor_Recover_Temp = temp;

    Error = SCA_Write_2(pSCA, W2_MotorRecoverTemp, temp);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Motor_Recover_Temp != temp) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器逆变器保护温度值，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getInverterProtectedTemperature(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Inverter_Protect_Temp = Actr_Disable;

    Error = SCA_Read(pSCA, R2_InverterProtectTemp);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Inverter_Protect_Temp != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;

}

/**
  * @功	能	设置执行器逆变器保护温度值
  * @参	数	id：要操作的执行器id
  *			temp：逆变器保护温度值，实际值，单位 摄氏度
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setInverterProtectedTemperature(uint8_t id,float temp,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Inverter_Protect_Temp = temp;

    Error = SCA_Write_2(pSCA, W2_InverterProtectTemp, temp);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Inverter_Protect_Temp != temp) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器逆变器恢复温度值，更新至句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getInverterRecoveryTemperature(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Inverter_Recover_Temp = Actr_Disable;

    Error = SCA_Read(pSCA, R2_InverterRecoverTemp);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Inverter_Recover_Temp != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;

}

/**
  * @功	能	设置执行器逆变器恢复温度值
  * @参	数	id：要操作的执行器id
  *			temp：逆变器恢复温度值，实际值，单位 摄氏度
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setInverterRecoveryTemperature(uint8_t id,float temp,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.Inverter_Recover_Temp = temp;

    Error = SCA_Write_2(pSCA, W2_InverterRecoverTemp, temp);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->Inverter_Recover_Temp != temp) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	设置执行器的id
  * @参	数	newID：新id
  *			currentID：当前id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t setActuatorID(uint8_t currentID, uint8_t newID,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 检查目标ID是否已存在 */
    pSCA = getInstance(newID);
    if(pSCA != NULL)	return SCA_OperationFailed;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(currentID);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 目标参数写入缓存，等待更新 */
    pSCA->paraCache.ID = newID;

    Error = SCA_Write_5(pSCA, W5_ChangeID, newID);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->ID != newID) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
  * @功	能	获取执行器的序列号，保存到句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getActuatorSerialNumber(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Serial_Num = Actr_Disable;

    Error = SCA_Read(pSCA, R5_ShakeHands);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Serial_Num != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;

}

/**
  * @功	能	获取执行器上次的关机状态，保存到句柄中
  * @参	数	id：要操作的执行器id
  *			isBlock：Block为阻塞式，Unblock为非阻塞式
  * @返	回	SCA_NoError：操作成功
  *			其他通信错误参见 SCA_Error 错误列表
  */
uint8_t getActuatorLastState(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_Last_State = Actr_Disable;

    Error = SCA_Read(pSCA, R1_LastState);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_Last_State != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;

}

/**
 * @功	能	获取电流速度位置的值，更新至句柄中，效率高
 * @参	数	id：要操作的执行器id
 *			isBlock：Block为阻塞式，Unblock为非阻塞式
 * @返	回	SCA_NoError：操作成功
 *			其他通信错误参见 SCA_Error 错误列表
 */
uint8_t requestCVPValue(uint8_t id,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;
    SCA_Handler_t* pSCA = NULL;

    /* 获取该ID的信息句柄 */
    pSCA = getInstance(id);
    if(pSCA == NULL)	return SCA_UnknownID;

    /* 清空状态位 */
    pSCA->paraCache.R_CVP = Actr_Disable;

    Error = SCA_Read(pSCA, R4_CVP);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_CVP != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}

/**
 * @功	能	获取电流速度位置的值，更新至句柄中，效率高，快速
 * @参	数	pSCA：要操作的执行器句柄指针或地址
 *			isBlock：Block为阻塞式，Unblock为非阻塞式
 * @返	回	SCA_NoError：操作成功
 *			其他通信错误参见 SCA_Error 错误列表
 */
uint8_t requestCVPValueFast(SCA_Handler_t* pSCA,uint8_t isBlock)
{
    uint8_t Error;
    uint32_t waitime = 0;

    /* 清空状态位 */
    pSCA->paraCache.R_CVP = Actr_Disable;

    Error = SCA_Read(pSCA, R4_CVP);
    if(Error)	return Error;

    /* 非阻塞 */
    if(isBlock == Unblock)
    {
        /* 非阻塞发送后延时处理，防止总线过载 */
        SCA_Delay(SendInterval);
        return Error;
    }

    /* 等待执行结果 */
    while((pSCA->paraCache.R_CVP != Actr_Enable) && (waitime++ < CanOvertime));
    if(waitime >= CanOvertime)	return SCA_OperationFailed;

    return Error;
}
