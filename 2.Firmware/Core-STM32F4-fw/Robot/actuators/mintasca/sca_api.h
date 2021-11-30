/**
  ******************************************************************************
  * @文	件 ： SCA_API.h
  * @作	者 ： INNFOS Software Team
  * @版	本 ： V1.5.3
  * @日	期 ： 2019.09.10
  * @摘	要 ： SCA 控制接口层
  ******************************************************************************/

#ifndef __SCA_API_H
#define __SCA_API_H


#ifdef __cplusplus
extern "C" {
#endif

#include "sca_protocol.h"
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include "time_utils.h"

/* 参数配置 */
#define SCA_NUM_USE        2            //当前使用SCA的数量,1-255
#define SCA_DEBUGER        0            //使能调试接口
#define CanOvertime        0xFFFF        //数据阻塞超时（180MHZ）
#define CanPowertime    0xFFFFFF    //开关机阻塞超时（180MHZ）
#define SendInterval    200            //非阻塞时的指令发送间隔
#define SCA_Delay(x)    delayMicroseconds(x)    //延时接口，非阻塞时连续发送需延时


#if SCA_DEBUGER
#define SCA_Debug printf
#else
#define SCA_Debug(s,...)
#endif

/* ！！！以下宏定义信息参数请勿修改！！！ */

//SCA状态定义
#define Actr_Enable        0x01
#define Actr_Disable    0x00

//通信方式定义
#define Block            0x01
#define Unblock            0x00

//SCA操作模式定义
#define SCA_Current_Mode            0x01
#define SCA_Velocity_Mode            0x02
#define SCA_Position_Mode            0x03
#define SCA_Profile_Position_Mode    0X06
#define SCA_Profile_Velocity_Mode    0X07
#define SCA_Homing_Mode                0X08

/* 
FAST类函数使用说明：
	以ID调用API时，会先内部查找ID对应的信息句柄，操作直观但当SCA的使用
	数量较多时，函数的执行效率低。若直接定义指针指向对应的结构体，会省
	去查找句柄的过程，但需防止该指针在使用时对句柄内部数据的意外修改。
	此类指针用于带有Fast型的API，当使用的SCA数量较多或高频读写时，可以
	提高函数的执行效率。其他类型的函数若有需求，也可按照此种方式进行修改。
	
	Example:

		//例如执行器ID是 0x03，对该ID进行快速写位置
		SCA_Handler_t* pSCA_ID3 = NULL;
		pSCA_ID3 = getInstance(0x03);
		if(pSCA_ID3 == NULL)	return;//未找到该ID的信息句柄

		//用定义好的指针直接传入Fast型写位置函数中
		setPositionFast(pSCA_ID3,100);
	
函数阻塞方式使用说明：
	带有参数isBlock的函数，可支持阻塞或非阻塞式的执行方式，可根据实际
	使用情况进行选择。阻塞等待时间在参数配置中可根据CPU速率更改负。其中类
	似开机等函数必须为阻塞式通信（等待执行结果返回），否责会造成数据错乱。
	另外，非阻塞函数的连续使用容易造成总线过载，SCA会出现蓝灯现象，在非阻
	塞执行时函数内部会有保护延时，通过参数配置更改延时时间。
*/

/***************控制相关******************/
void lookupActuators(CAN_Handler_t *canPort);
void setupActuators(uint8_t id, CAN_Handler_t *can);
void resetController(uint8_t id);
void enableAllActuators(void);
void disableAllActuators(void);
void regainAttrbute(uint8_t id, uint8_t isBlock);
uint8_t isOnline(uint8_t id, uint8_t isBlock);
uint8_t isEnable(uint8_t id, uint8_t isBlock);
uint8_t isUpdate(uint8_t id);
uint8_t enableActuator(uint8_t id);
uint8_t disableActuator(uint8_t id);
uint8_t activateActuatorMode(uint8_t id, uint8_t ActuatorMode, uint8_t isBlock);
uint8_t getActuatorMode(uint8_t id, uint8_t isBlock);
uint8_t getErrorCode(uint8_t id, uint8_t isBlock);
uint8_t clearError(uint8_t id, uint8_t isBlock);
uint8_t saveAllParams(uint8_t id, uint8_t isBlock);
SCA_Handler_t *getInstance(uint8_t id);

/***************位置相关******************/
uint8_t setPosition(uint8_t id, float pos);
uint8_t setPositionFast(SCA_Handler_t *pSCA, float pos);
uint8_t getPosition(uint8_t id, uint8_t isBlock);
uint8_t getPositionFast(SCA_Handler_t *pSCA, uint8_t isBlock);
uint8_t setPositionKp(uint8_t id, float Kp, uint8_t isBlock);
uint8_t getPositionKp(uint8_t id, uint8_t isBlock);
uint8_t setPositionKi(uint8_t id, float Ki, uint8_t isBlock);
uint8_t getPositionKi(uint8_t id, uint8_t isBlock);
uint8_t setPositionUmax(uint8_t id, float max, uint8_t isBlock);
uint8_t getPositionUmax(uint8_t id, uint8_t isBlock);
uint8_t setPositionUmin(uint8_t id, float min, uint8_t isBlock);
uint8_t getPositionUmin(uint8_t id, uint8_t isBlock);
uint8_t setPositionOffset(uint8_t id, float offset, uint8_t isBlock);
uint8_t getPositionOffset(uint8_t id, uint8_t isBlock);
uint8_t setMaximumPosition(uint8_t id, float maxPos, uint8_t isBlock);
uint8_t getMaximumPosition(uint8_t id, uint8_t isBlock);
uint8_t setMinimumPosition(uint8_t id, float minPos, uint8_t isBlock);
uint8_t getMinimumPosition(uint8_t id, uint8_t isBlock);
uint8_t enablePositionLimit(uint8_t id, uint8_t enable, uint8_t isBlock);
uint8_t isPositionLimitEnable(uint8_t id, uint8_t isBlock);
uint8_t setHomingPosition(uint8_t id, float homingPos, uint8_t isBlock);
uint8_t enablePositionFilter(uint8_t id, uint8_t enable, uint8_t isBlock);
uint8_t isPositionFilterEnable(uint8_t id, uint8_t isBlock);
uint8_t setPositionCutoffFrequency(uint8_t id, float frequency, uint8_t isBlock);
uint8_t getPositionCutoffFrequency(uint8_t id, uint8_t isBlock);
uint8_t clearHomingInfo(uint8_t id, uint8_t isBlock);
uint8_t setProfilePositionAcceleration(uint8_t id, float acceleration, uint8_t isBlock);
uint8_t getProfilePositionAcceleration(uint8_t id, uint8_t isBlock);
uint8_t setProfilePositionDeceleration(uint8_t id, float deceleration, uint8_t isBlock);
uint8_t getProfilePositionDeceleration(uint8_t id, uint8_t isBlock);
uint8_t setProfilePositionMaxVelocity(uint8_t id, float maxVelocity, uint8_t isBlock);
uint8_t getProfilePositionMaxVelocity(uint8_t id, uint8_t isBlock);

/***************速度相关******************/
uint8_t setVelocity(uint8_t id, float vel);
uint8_t setVelocityFast(SCA_Handler_t *pSCA, float vel);
uint8_t getVelocity(uint8_t id, uint8_t isBlock);
uint8_t getVelocityFast(SCA_Handler_t *pSCA, uint8_t isBlock);
uint8_t getVelocityKp(uint8_t id, uint8_t isBlock);
uint8_t setVelocityKp(uint8_t id, float Kp, uint8_t isBlock);
uint8_t getVelocityKi(uint8_t id, uint8_t isBlock);
uint8_t setVelocityKi(uint8_t id, float Ki, uint8_t isBlock);
uint8_t getVelocityUmax(uint8_t id, uint8_t isBlock);
uint8_t setVelocityUmax(uint8_t id, float max, uint8_t isBlock);
uint8_t getVelocityUmin(uint8_t id, uint8_t isBlock);
uint8_t setVelocityUmin(uint8_t id, float min, uint8_t isBlock);
uint8_t enableVelocityFilter(uint8_t id, uint8_t enable, uint8_t isBlock);
uint8_t isVelocityFilterEnable(uint8_t id, uint8_t isBlock);
uint8_t getVelocityCutoffFrequency(uint8_t id, uint8_t isBlock);
uint8_t setVelocityCutoffFrequency(uint8_t id, float frequency, uint8_t isBlock);
uint8_t setVelocityLimit(uint8_t id, float limit, uint8_t isBlock);
uint8_t getVelocityLimit(uint8_t id, uint8_t isBlock);
uint8_t setProfileVelocityAcceleration(uint8_t id, float acceleration, uint8_t isBlock);
uint8_t getProfileVelocityAcceleration(uint8_t id, uint8_t isBlock);
uint8_t setProfileVelocityDeceleration(uint8_t id, float deceleration, uint8_t isBlock);
uint8_t getProfileVelocityDeceleration(uint8_t id, uint8_t isBlock);
uint8_t setProfileVelocityMaxVelocity(uint8_t id, float maxVelocity, uint8_t isBlock);
uint8_t getProfileVelocityMaxVelocity(uint8_t id, uint8_t isBlock);
float getVelocityRange(uint8_t id);

/***************电流相关******************/
uint8_t setCurrent(uint8_t id, float current);
uint8_t setCurrentFast(SCA_Handler_t *pSCA, float current);
uint8_t getCurrent(uint8_t id, uint8_t isBlock);
uint8_t getCurrentFast(SCA_Handler_t *pSCA, uint8_t isBlock);
uint8_t getCurrentKp(uint8_t id, uint8_t isBlock);
uint8_t getCurrentKi(uint8_t id, uint8_t isBlock);
uint8_t getCurrentRange(uint8_t id, uint8_t isBlock);
uint8_t enableCurrentFilter(uint8_t id, uint8_t enable, uint8_t isBlock);
uint8_t isCurrentFilterEnable(uint8_t id, uint8_t isBlock);
uint8_t getCurrentCutoffFrequency(uint8_t id, uint8_t isBlock);
uint8_t setCurrentCutoffFrequency(uint8_t id, float frequency, uint8_t isBlock);
uint8_t setCurrentLimit(uint8_t id, float limit, uint8_t isBlock);
uint8_t getCurrentLimit(uint8_t id, uint8_t isBlock);

/***************其他参数******************/
uint8_t getVoltage(uint8_t id, uint8_t isBlock);
uint8_t getLockEnergy(uint8_t id, uint8_t isBlock);
uint8_t setLockEnergy(uint8_t id, float energy, uint8_t isBlock);
uint8_t getActuatorSerialNumber(uint8_t id, uint8_t isBlock);
uint8_t getMotorTemperature(uint8_t id, uint8_t isBlock);
uint8_t getInverterTemperature(uint8_t id, uint8_t isBlock);
uint8_t getMotorProtectedTemperature(uint8_t id, uint8_t isBlock);
uint8_t setMotorProtectedTemperature(uint8_t id, float temp, uint8_t isBlock);
uint8_t getMotorRecoveryTemperature(uint8_t id, uint8_t isBlock);
uint8_t setMotorRecoveryTemperature(uint8_t id, float temp, uint8_t isBlock);
uint8_t getInverterProtectedTemperature(uint8_t id, uint8_t isBlock);
uint8_t setInverterProtectedTemperature(uint8_t id, float temp, uint8_t isBlock);
uint8_t getInverterRecoveryTemperature(uint8_t id, uint8_t isBlock);
uint8_t setInverterRecoveryTemperature(uint8_t id, float temp, uint8_t isBlock);
uint8_t setActuatorID(uint8_t currentID, uint8_t newID, uint8_t isBlock);
uint8_t getActuatorLastState(uint8_t id, uint8_t isBlock);
uint8_t requestCVPValue(uint8_t id, uint8_t isBlock);
uint8_t requestCVPValueFast(SCA_Handler_t *pSCA, uint8_t isBlock);


#ifdef __cplusplus
}

#endif
#endif
