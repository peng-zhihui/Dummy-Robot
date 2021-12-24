/******
	************************************************************************
	******
	** @versions : 1.1.4
	** @time     : 2020/09/15
	******
	************************************************************************
	******
	** @project : XDrive_Step
	** @brief   : 具有多功能接口和闭环功能的步进电机
	** @author  : unlir (知不知啊)
	******
	** @address : https://github.com/unlir/XDrive
	******
	** @issuer  : IVES ( 艾维斯 实验室) (QQ: 557214000)   (master)
	** @issuer  : REIN (  知驭  实验室) (QQ: 857046846)   (master)
	******
	************************************************************************
	******
	** {Stepper motor with multi-function interface and closed Main function.}
	** Copyright (c) {2020}  {unlir(知不知啊)}
	** 
	** This program is free software: you can redistribute it and/or modify
	** it under the terms of the GNU General Public License as published by
	** the Free Software Foundation, either version 3 of the License, or
	** (at your option) any later version.
	** 
	** This program is distributed in the hope that it will be useful,
	** but WITHOUT ANY WARRANTY; without even the implied warranty of
	** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	** GNU General Public License for more details.
	** 
	** You should have received a copy of the GNU General Public License
	** along with this program.  If not, see <http://www.gnu.org/licenses/>.
	******
	************************************************************************
******/

/*****
  ** @file     : stockpile_config.c/h
  ** @brief    : 存储配置
  ** @versions : newest
  ** @time     : newest
  ** @reviser  : unli (HeFei China)
  ** @explain  : null
*****/

/*************************************************************** Stockpile_Start ***************************************************************/
/*************************************************************** Stockpile_Start ***************************************************************/
/*************************************************************** Stockpile_Start ***************************************************************/
/*********************STM32F103xx*************************/
//主储存块容量
//Flash Size(bytes)/RAM size(bytes)
// 大容量   1M / 96K                                     RG               VG           ZG
// 大容量 768K / 96K                                     RF               VF           ZF
// 大容量 512K / 64K                                     RE               VE           ZE
// 大容量 384K / 64K                                     RD               VD           ZD
// 大容量 256K / 48K                                     RC               VC           ZC
// 中容量 128K / 20K      TB           CB                RB               VB
// 中容量  64K / 20K      T8           C8                R8               V8
// 小容量  32K / 10K      T6           C6                R6
// 小容量  16K /  6K      T4           C4                R4
//        						 36pin-QFN	48pin-LQFP/QFN	64pin-BGA/CSP/LQFP  100pin-LQFP  144pin-BGA/LQFP  
/*************************************************************** Stockpile_End ***************************************************************/
/*************************************************************** Stockpile_End ***************************************************************/
/*************************************************************** Stockpile_End ***************************************************************/

#ifndef STOCKPILE_CONFIG_H
#define STOCKPILE_CONFIG_H

/* ROM sizes */
/* ROM sizes */
/* ROM sizes */

//DAPLINK_ROM_BL
#define		DAPLINK_ROM_BL_START						(0x08000000)		//起始地址
#define		DAPLINK_ROM_BL_SIZE							(0x0000BC00)		//Flash容量    47K		DAPLink_BL(DAPLINK_ROM_BL)
//DAPLINK_ROM_CONFIG_ADMIN
#define		DAPLINK_ROM_CONFIG_ADMIN_START	(0x0800BC00)		//起始地址
#define		DAPLINK_ROM_CONFIG_ADMIN_SIZE		(0x00000400)		//Flash容量     1K		DAPLink_BL(DAPLINK_ROM_CONFIG_ADMIN)
//APP_FIRMWARE
#define		STOCKPILE_APP_FIRMWARE_ADDR			(0x08000000) //(0x0800C000)		//起始地址
#define		STOCKPILE_APP_FIRMWARE_SIZE			(0x0000BC00)		//Flash容量    47K    XDrive(APP_FIRMWARE)
//APP_CALI
#define		STOCKPILE_APP_CALI_ADDR					(0x08017C00)		//起始地址
#define		STOCKPILE_APP_CALI_SIZE					(0x00008000)		//Flash容量    32K    XDrive(APP_CALI)(可容纳16K-2byte校准数据-即最大支持14位编码器的校准数据)
//APP_DATA
#define		STOCKPILE_APP_DATA_ADDR					(0x0801FC00)		//起始地址
#define		STOCKPILE_APP_DATA_SIZE					(0x00000400)		//Flash容量     1K    XDrive(APP_DATA)

/* RAM sizes */
/* RAM sizes */
/* RAM sizes */

#define STOCKPILE_RAM_APP_START           (0x20000000)
#define STOCKPILE_RAM_APP_SIZE            (0x00004F00)		//19K768字节

#define STOCKPILE_RAM_SHARED_START        (0x20004F00)
#define STOCKPILE_RAM_SHARED_SIZE         (0x00000100)		//256字节

#endif
