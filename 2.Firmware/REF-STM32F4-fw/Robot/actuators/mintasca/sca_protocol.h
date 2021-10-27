/**
  ******************************************************************************
  * @文	件 ： SCA_Protocol.h
  * @作	者 ： INNFOS Software Team
  * @版	本 ： V1.5.2
  * @日	期 ： 2019.08.20
  * @摘	要 ： INNFOS CAN 通信协议层
  ******************************************************************************/

#ifndef __SCA_PROTOCOL_H
#define __SCA_PROTOCOL_H


#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* ！！！以下宏定义信息参数请勿修改！！！ */

//INNFOS CAN 通信协议指令
//第一类读取指令
#define R1_Heartbeat            0x00
#define R1_Mode                    0x55
#define R1_LastState            0xB0
#define R1_CurrentFilterState    0X71
#define R1_VelocityFilterState    0x75
#define R1_PositionFilterState    0x79
#define R1_PositionLimitState    0x8B
#define R1_PowerState            0x2B

//第二类读取指令
#define R2_Voltage                0x45
#define R2_Current_Max            0x53
#define R2_CurrentFilterValue    0x73
#define R2_VelocityFilterValue    0x77
#define R2_PositionFilterValue    0x7B
#define R2_MotorTemp            0x5F
#define R2_InverterTemp            0x60
#define R2_InverterProtectTemp    0x62
#define R2_InverterRecoverTemp    0x64
#define R2_MotorProtectTemp        0x6C
#define R2_MotorRecoverTemp        0x6E
#define R2_Error                0xFF

//第三类读取指令
#define R3_Current                0x04
#define R3_Velocity                0x05
#define R3_Position                0x06
#define R3_CurrentFilterP        0x15
#define R3_CurrentFilterI        0x16
#define R3_VelocityFilterP        0x17
#define R3_VelocityFilterI        0x18
#define R3_PositionFilterP        0x19
#define R3_PositionFilterI        0x1A
#define R3_PositionFilterD        0X1B
#define R3_PPMaxVelocity        0x1C
#define R3_PPMaxAcceleration    0x1D
#define R3_PPMaxDeceleration    0x1E
#define R3_PVMaxVelocity        0x22
#define R3_PVMaxAcceleration    0x23
#define R3_PVMaxDeceleration    0x24
#define R3_CurrentFilterLimitL    0x34
#define R3_CurrentFilterLimitH    0x35
#define R3_VelocityFilterLimitL    0x36
#define R3_VelocityFilterLimitH    0x37
#define R3_PositionFilterLimitL    0x38
#define R3_PositionFilterLimitH    0x39
#define R3_CurrentLimit            0x59
#define R3_VelocityLimit        0x5B
#define R3_Inertia                0x7D
#define R3_PositionLimitH        0x85
#define R3_PositionLimitL        0x86
#define R3_PositionOffset        0x8A
#define R3_HomingCurrentLimitL    0x92
#define R3_HomingCurrentLimitH    0x93
#define R3_BlockEngy            0x7F

//第四类读取指令
#define R4_CVP                    0x94

//第五类读取指令
#define R5_ShakeHands            0x02

//第一类写入命令
#define W1_Mode                    0x07
#define W1_CurrentFilterState    0X70
#define W1_VelocityFilterState    0x74
#define W1_PositionFilterState    0x78
#define W1_PositionLimitState    0x8C
#define W1_PowerState            0x2A

//第二类写入命令
#define W2_CurrentFilterValue    0x72
#define W2_VelocityFilterValue    0x76
#define W2_PositionFilterValue    0x7A
#define W2_InverterProtectTemp    0x61
#define W2_InverterRecoverTemp    0x63
#define W2_MotorProtectTemp        0x6B
#define W2_MotorRecoverTemp        0x6D

//第三类写入命令
#define W3_Current                0x08
#define W3_Velocity                0x09
#define W3_Position                0x0A
#define W3_CurrentFilterP        0x0E
#define W3_CurrentFilterI        0x0F
#define W3_VelocityFilterP        0x10
#define W3_VelocityFilterI        0x11
#define W3_PositionFilterP        0x12
#define W3_PositionFilterI        0x13
#define W3_PositionFilterD        0X14
#define W3_PPMaxVelocity        0x1F
#define W3_PPMaxAcceleration    0x20
#define W3_PPMaxDeceleration    0x21
#define W3_PVMaxVelocity        0x25
#define W3_PVMaxAcceleration    0x26
#define W3_PVMaxDeceleration    0x27
#define W3_CurrentFilterLimitL    0x2E
#define W3_CurrentFilterLimitH    0x2F
#define W3_VelocityFilterLimitL    0x30
#define W3_VelocityFilterLimitH    0x31
#define W3_PositionFilterLimitL    0x32
#define W3_PositionFilterLimitH    0x33
#define W3_CurrentLimit            0x58
#define W3_VelocityLimit        0x5A
#define W3_PositionLimitH        0x83
#define W3_PositionLimitL        0x84
#define W3_HomingValue            0x87
#define W3_PositionOffset        0x89
#define W3_HomingCurrentLimitL    0x90
#define W3_HomingCurrentLimitH    0x91
#define W3_BlockEngy            0x7E

//第四类写入命令
#define W4_ClearError            0xFE
#define W4_ClearHome            0x88
#define W4_Save                    0x0D

//第五类写入命令
#define W5_ChangeID                0x3D

//变量缩放值定义
#define Velocity_Max    6000.0f            //速度最大值，固定为6000RPM（仅作为换算用）
#define BlkEngy_Scal    75.225f            //堵转能量缩放值
#define Profile_Scal    960.0f            //梯形参数缩放值
#define IQ8                256.0f            //2^8
#define IQ10            1024.0f            //2^10
#define IQ24            16777216.0f        //2^24
#define IQ30            1073741824.0f    //2^30

/* ID为CAN发送帧ID，msg为要发送的数据（地址）
   len为发送数据的长度，返回0成功，返回其他失败 */
typedef uint8_t (*Send_t)(uint8_t ID, uint8_t *msg, uint8_t len);

typedef struct                //CAN端口句柄
{
    //SCA 状态信息
    uint8_t CanPort;        //使用的CAN端口号
    uint8_t Retry;            //发送失败时重发次数
    Send_t Send;            //发送函数，格式参见Send_t
} CAN_Handler_t;

typedef struct                        //SCA报警信息
{
    uint16_t Error_Code;            //错误代码

    /* 具体报警信息，0：正常，1：出错 */
    uint8_t WARN_OVER_VOLT;        //过压异常
    uint8_t WARN_UNDER_VOLT;        //欠压异常
    uint8_t WARN_LOCK_ROTOR;        //堵转异常
    uint8_t WARN_OVER_TEMP;        //过热异常
    uint8_t WARN_RW_PARA;            //读写参数异常
    uint8_t WARN_MUL_CIRCLE;        //多圈计数异常
    uint8_t WARN_TEMP_SENSOR_INV;    //逆变器温度传感器异常
    uint8_t WARN_CAN_BUS;            //CAN通讯异常
    uint8_t WARN_TEMP_SENSOR_MTR;    //电机温度传感器异常
    uint8_t WARN_OVER_STEP;            //位置模式阶跃大于1
    uint8_t WARN_DRV_PROTEC;        //DRV保护
    uint8_t WARN_DVICE;            //设备异常

} SCA_Warn_t;

/* 
	SCA变量缓存，用于写入参数时保存目标参数，待成功后再写入句柄中
	读取标志位在阻塞时使用，变量内容可根据项目需要进行裁剪或添加
 */
typedef struct
{
    /* 基础信息 */
    uint8_t ID;                        //SCA 的ID号

    /* 第一类数据变量 */
    uint8_t Mode;                    //当前操作模式
    uint8_t Current_Filter_State;    //电流环滤波器状态
    uint8_t Velocity_Filter_State;    //速度环滤波器状态
    uint8_t Position_Filter_State;    //速度环滤波器状态
    uint8_t Position_Limit_State;    //位置限位状态
    uint8_t Power_State;            //开关机状态
    /* 读取标志位 */
    uint8_t R_Mode;                    //读取数据返回标志位 1为有数据返回
    uint8_t R_Last_State;
    uint8_t R_Current_Filter_State;
    uint8_t R_Velocity_Filter_State;
    uint8_t R_Position_Filter_State;
    uint8_t R_Position_Limit_State;
    uint8_t R_Power_State;

    /* 第二类数据变量 */
    float Current_Filter_Value;        //电流环滤波器带宽
    float Velocity_Filter_Value;    //速度环滤波器带宽
    float Position_Filter_Value;    //位置环滤波器带宽
    float Inverter_Protect_Temp;    //逆变器保护温度
    float Inverter_Recover_Temp;    //逆变器恢复温度
    float Motor_Protect_Temp;        //电机保护温度
    float Motor_Recover_Temp;        //电机恢复温度
    /* 读取标志位 */
    uint8_t R_Current_Filter_Value;
    uint8_t R_Velocity_Filter_Value;
    uint8_t R_Position_Filter_Value;
    uint8_t R_Inverter_Protect_Temp;
    uint8_t R_Inverter_Recover_Temp;
    uint8_t R_Motor_Protect_Temp;
    uint8_t R_Motor_Recover_Temp;
    uint8_t R_Voltage;
    uint8_t R_Current_Max;
    uint8_t R_Motor_Temp;
    uint8_t R_Inverter_Temp;
    uint8_t R_Error_Code;

    /* 第三类数据变量 */
    float Current_Real;                //当前电流（单位：A）
    float Velocity_Real;            //当前速度（单位：RPM）
    float Position_Real;            //当前位置，真实值（单位：R）
    float Current_Filter_P;            //电流环的P值
    float Current_Filter_I;            //电流环的I值
    float Velocity_Filter_P;        //速度环的P值
    float Velocity_Filter_I;        //速度环的I值
    float Position_Filter_P;        //位置环的P值
    float Position_Filter_I;        //位置环的I值
    //float Position_Filter_D;		//位置环的D值
    float PP_Max_Velocity;            //位置梯形速度最大值
    float PP_Max_Acceleration;        //位置梯形加速度最大值
    float PP_Max_Deceleration;        //位置梯形减速度最大值
    float PV_Max_Velocity;            //速度梯形速度最大值
    float PV_Max_Acceleration;        //速度梯形加速度最大值
    float PV_Max_Deceleration;        //速度梯形减速度最大值
    //float Current_Filter_Limit_L;	//电流环输出下限
    //float Current_Filter_Limit_H;	//电流环输出上限
    float Velocity_Filter_Limit_L;    //速度环输出下限
    float Velocity_Filter_Limit_H;    //速度环输出上限
    float Position_Filter_Limit_L;    //位置环输出下限
    float Position_Filter_Limit_H;    //位置环输出上限
    float Position_Limit_H;            //执行器的位置上限
    float Position_Limit_L;            //执行器的位置下限
    float Current_Limit;            //电流输入限幅
    float Velocity_Limit;            //速度输入限幅
    float Homing_Value;                //执行器的Homing值
    float Position_Offset;            //执行器的位置偏置
    float Homing_Current_Limit_L;    //自动归零电流下限
    float Homing_Current_Limit_H;    //自动归零电流上限
    float Blocked_Energy;            //堵转锁定能量
    /* 读取标志位 */
    uint8_t R_Current_Real;
    uint8_t R_Velocity_Real;
    uint8_t R_Position_Real;
    uint8_t R_Current_Filter_P;
    uint8_t R_Current_Filter_I;
    uint8_t R_Velocity_Filter_P;
    uint8_t R_Velocity_Filter_I;
    uint8_t R_Position_Filter_P;
    uint8_t R_Position_Filter_I;
    //uint8_t R_Position_Filter_D;
    uint8_t R_PP_Max_Velocity;
    uint8_t R_PP_Max_Acceleration;
    uint8_t R_PP_Max_Deceleration;
    uint8_t R_PV_Max_Velocity;
    uint8_t R_PV_Max_Acceleration;
    uint8_t R_PV_Max_Deceleration;
    //uint8_t R_Current_Filter_Limit_L;
    //uint8_t R_Current_Filter_Limit_H;
    uint8_t R_Velocity_Filter_Limit_L;
    uint8_t R_Velocity_Filter_Limit_H;
    uint8_t R_Position_Filter_Limit_L;
    uint8_t R_Position_Filter_Limit_H;
    uint8_t R_Position_Limit_H;
    uint8_t R_Position_Limit_L;
    uint8_t R_Current_Limit;
    uint8_t R_Velocity_Limit;
    uint8_t R_Homing_Value;
    uint8_t R_Position_Offset;
    uint8_t R_Homing_Current_Limit_L;
    uint8_t R_Homing_Current_Limit_H;
    uint8_t R_Blocked_Energy;
    uint8_t R_CVP;
    uint8_t R_Serial_Num;
    uint8_t W_ClearHome;

} Para_Cache_t;

/* 
	SCA信息句柄，请勿随意更改其中数值，
	变量内容可根据项目需要进行裁剪或添加
 */
typedef struct
{
    /* 协议数据变量区 */
    uint8_t ID;                        //SCA的ID号
    uint8_t Serial_Num[4];            //序列号
    uint8_t Save_State;                //参数保存状态，1为已保存
    uint8_t Online_State;            //当前在线状态，1为在线
    uint8_t Update_State;            //是否有参数刷新，1为有参数刷新
    CAN_Handler_t *Can;                //所使用的CAN端口
    Para_Cache_t paraCache;            //参数缓存

    /* 用户数据变量区 */

    /* 第一类数据变量 */
    uint8_t Mode;                    //当前操作模式
    uint8_t Last_State;                //上次关机的异常状态，1为正常
    uint8_t Current_Filter_State;    //电流环滤波器状态
    uint8_t Velocity_Filter_State;    //速度环滤波器状态
    uint8_t Position_Filter_State;    //速度环滤波器状态
    uint8_t Position_Limit_State;    //位置限位状态
    uint8_t Power_State;            //开关机状态

    /* 第二类数据变量 */
    float Voltage;                    //当前电压（单位：V）
    float Current_Max;                //最大电流量程
    float Current_Filter_Value;        //电流环滤波器带宽
    float Velocity_Filter_Value;    //速度环滤波器带宽
    float Position_Filter_Value;    //位置环滤波器带宽
    float Motor_Temp;                //电机温度
    float Inverter_Temp;            //逆变器温度
    float Inverter_Protect_Temp;    //逆变器保护温度
    float Inverter_Recover_Temp;    //逆变器恢复温度
    float Motor_Protect_Temp;        //电机保护温度
    float Motor_Recover_Temp;        //电机恢复温度
    SCA_Warn_t SCA_Warn;            //电机报警信息

    /* 第三类数据变量 */
    float Current_Real;                //当前电流（单位：A）
    float Velocity_Real;            //当前速度（单位：RPM）
    float Position_Real;            //当前位置，真实值（单位：R）
    float Current_Filter_P;            //电流环的P值
    float Current_Filter_I;            //电流环的I值
    float Velocity_Filter_P;        //速度环的P值
    float Velocity_Filter_I;        //速度环的I值
    float Position_Filter_P;        //位置环的P值
    float Position_Filter_I;        //位置环的I值
    //float Position_Filter_D;		//位置环的D值
    float PP_Max_Velocity;            //位置梯形速度最大值
    float PP_Max_Acceleration;        //位置梯形加速度最大值
    float PP_Max_Deceleration;        //位置梯形减速度最大值
    float PV_Max_Velocity;            //速度梯形速度最大值
    float PV_Max_Acceleration;        //速度梯形加速度最大值
    float PV_Max_Deceleration;        //速度梯形减速度最大值
    //float Current_Filter_Limit_L;	//电流环输出下限
    //float Current_Filter_Limit_H;	//电流环输出上限
    float Velocity_Filter_Limit_L;    //速度环输出下限
    float Velocity_Filter_Limit_H;    //速度环输出上限
    float Position_Filter_Limit_L;    //位置环输出下限
    float Position_Filter_Limit_H;    //位置环输出上限
    float Position_Limit_H;            //执行器的位置上限
    float Position_Limit_L;            //执行器的位置下限
    float Current_Limit;            //电流输入限幅
    float Velocity_Limit;            //速度输入限幅
    float Homing_Value;                //执行器的Homing值
    float Position_Offset;            //执行器的位置偏置
    float Homing_Current_Limit_L;    //自动归零电流下限
    float Homing_Current_Limit_H;    //自动归零电流上限
    float Blocked_Energy;            //堵转锁定能量

} SCA_Handler_t;

enum SCA_Error                //SCA通信错误类型枚举
{
    SCA_NoError = 0,        //无错误
    SCA_OverTime,            //通信等待超时
    SCA_SendError,            //数据发送失败
    SCA_OperationFailed,    //操作失败
    SCA_UnknownID,            //未找到该ID的执行器句柄
};

/* 数据接收接口，有新的CAN数据包传入时调用
  CanRxMsg 为CAN数据包的接收类型结构体移植时请自行
  根据平台定义CanRxMsg结构类型，此处默认使用STM32
  标准库函数中的接收结构	*/
typedef struct
{
    uint32_t StdId;  /*!< Specifies the standard identifier.
                        This parameter can be a value between 0 to 0x7FF. */

    uint32_t ExtId;  /*!< Specifies the extended identifier.
                        This parameter can be a value between 0 to 0x1FFFFFFF. */

    uint8_t IDE;     /*!< Specifies the type of identifier for the message that
                        will be received. This parameter can be a value of
                        @ref CAN_identifier_type */

    uint8_t RTR;     /*!< Specifies the type of frame for the received message.
                        This parameter can be a value of
                        @ref CAN_remote_transmission_request */

    uint8_t DLC;     /*!< Specifies the length of the frame that will be received.
                        This parameter can be a value between 0 to 8 */

    uint8_t Data[8]; /*!< Contains the data to be received. It ranges from 0 to
                        0xFF. */

    uint8_t FMI;     /*!< Specifies the index of the filter the message stored in
                        the mailbox passes through. This parameter can be a
                        value between 0 to 0xFF */
} CanRxMsg;

void canDispatch(CanRxMsg *RxMsg);

/* 以下函数为API层调用 */

/* 读取命令接口 */
uint8_t SCA_Read(SCA_Handler_t *pSCA, uint8_t cmd);

/* 五类写入命令 */
uint8_t SCA_Write_1(SCA_Handler_t *pSCA, uint8_t cmd, uint8_t TxData);
uint8_t SCA_Write_2(SCA_Handler_t *pSCA, uint8_t cmd, float TxData);
uint8_t SCA_Write_3(SCA_Handler_t *pSCA, uint8_t cmd, float TxData);
uint8_t SCA_Write_4(SCA_Handler_t *pSCA, uint8_t cmd);
uint8_t SCA_Write_5(SCA_Handler_t *pSCA, uint8_t cmd, uint8_t TxData);


#ifdef __cplusplus
}

#endif
#endif
