/*****
  ** @file     : stockpile_f103cb.c/h
  ** @brief    : Flash存储库
  ** @versions : newest
  ** @time     : newest
  ** @reviser  : unli (WuHu China)
  ** @explain  : null
*****/

//Oneself
#include "stockpile_f103cb.h"

////LL_Driver
//#include "stm32f0xx_ll_flash_ex.h"

////LL库FLASH清除函数
//extern void FLASH_PageErase(uint32_t PageAddress);

/*************************************************************** Flash_Start ***************************************************************/
/*************************************************************** Flash_Start ***************************************************************/
/*************************************************************** Flash_Start ***************************************************************/
//Flash分区表实例
Stockpile_FLASH_Typedef	stockpile_app_firmware	= {STOCKPILE_APP_FIRMWARE_ADDR, STOCKPILE_APP_FIRMWARE_SIZE,	(STOCKPILE_APP_FIRMWARE_SIZE / Stockpile_Page_Size),	0};
Stockpile_FLASH_Typedef	stockpile_quick_cali		= {STOCKPILE_APP_CALI_ADDR, 		STOCKPILE_APP_CALI_SIZE,			(STOCKPILE_APP_CALI_SIZE / Stockpile_Page_Size),			0};
Stockpile_FLASH_Typedef stockpile_data					= {STOCKPILE_APP_DATA_ADDR,			STOCKPILE_APP_DATA_SIZE, 			(STOCKPILE_APP_DATA_SIZE / Stockpile_Page_Size),			0};

/**
  * @brief  Flash数据清空
  * @param  stockpile	Flash分区表实例
  * @retval NULL
**/
void Stockpile_Flash_Data_Empty(Stockpile_FLASH_Typedef *stockpile)
{
	uint32_t count;
	HAL_FLASH_Unlock();	//LL_FLASH_Unlock();
	for(count = 0; count < stockpile->page_num; count++)
	{
		FLASH_EraseInitTypeDef erase_config;
		uint32_t page_error;
		erase_config.TypeErase   = FLASH_TYPEERASE_PAGES;																	//页擦除		
		erase_config.PageAddress = stockpile->begin_add + (count * Stockpile_Page_Size);	//页起始地址
		erase_config.NbPages     = 1;																											//擦除页数量
		HAL_FLASHEx_Erase(&erase_config, &page_error);
		FLASH_WaitForLastOperation(HAL_MAX_DELAY);
		CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
	}
	HAL_FLASH_Lock();	//LL_FLASH_Lock();
}

/**
  * @brief  Flash数据开始写入
  * @param  stockpile	Flash分区表实例
  * @retval NULL
**/
void Stockpile_Flash_Data_Begin(Stockpile_FLASH_Typedef *stockpile)
{
	HAL_FLASH_Unlock();	//LL_FLASH_Unlock();
	stockpile->asce_write_add = stockpile->begin_add;
}

/**
  * @brief  Flash数据结束写入
  * @param  stockpile	Flash分区表实例
  * @retval NULL
**/
void Stockpile_Flash_Data_End(Stockpile_FLASH_Typedef *stockpile)
{
	HAL_FLASH_Lock();	//LL_FLASH_Lock();
}

/**
  * @brief  Flash设置写地址
  * @param  stockpile	Flash分区表实例
  * @param  write_add	写地址
  * @retval NULL
**/
void Stockpile_Flash_Data_Set_Write_Add(Stockpile_FLASH_Typedef *stockpile, uint32_t write_add)
{
	if(write_add < stockpile->begin_add)						return;
	if(write_add > stockpile->begin_add + stockpile->area_size)	return;
	stockpile->asce_write_add = write_add;
}

/**
  * @brief  Flash_16位数据写入
  * @param  stockpile	Flash分区表实例
  * @param  data		半字数据缓冲区
  * @param  num			半字数量
  * @retval NULL
**/
void Stockpile_Flash_Data_Write_Data16(Stockpile_FLASH_Typedef *stockpile, uint16_t *data, uint32_t num)
{
	if(stockpile->asce_write_add < stockpile->begin_add)									return;
	if((stockpile->asce_write_add + num * 2) > stockpile->begin_add + stockpile->area_size)	return;
	
	for(uint32_t i=0; i<num; i++)
	{
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, stockpile->asce_write_add, (uint64_t)data[i]) == HAL_OK)
			stockpile->asce_write_add += 2;
	}
}

/**
  * @brief  Flash_32位数据写入
  * @param  stockpile	Flash分区表实例
  * @param  data		字数据缓冲区
  * @param  num			字数量
  * @retval NULL
**/
void Stockpile_Flash_Data_Write_Data32(Stockpile_FLASH_Typedef *stockpile, uint32_t *data, uint32_t num)
{
	if(stockpile->asce_write_add < stockpile->begin_add)									return;
	if((stockpile->asce_write_add + num * 4) > stockpile->begin_add + stockpile->area_size)	return;
	
	for(uint32_t i=0; i<num; i++)
	{
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, stockpile->asce_write_add, (uint64_t)data[i]) == HAL_OK)
			stockpile->asce_write_add += 4;
	}
}

/**
  * @brief  Flash_64位数据写入
  * @param  stockpile	Flash分区表实例
  * @param  data		双字数据缓冲区
  * @param  num			双字数量
  * @retval NULL
**/
void Stockpile_Flash_Data_Write_Data64(Stockpile_FLASH_Typedef *stockpile, uint64_t *data, uint32_t num)
{
	if(stockpile->asce_write_add < stockpile->begin_add)									return;
	if((stockpile->asce_write_add + num * 8) > stockpile->begin_add + stockpile->area_size)	return;
	
	for(uint32_t i=0; i<num; i++)
	{
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, stockpile->asce_write_add, (uint64_t)data[i]) == HAL_OK)
			stockpile->asce_write_add += 8;
	}
}
/*************************************************************** Flash_End ***************************************************************/
/*************************************************************** Flash_End ***************************************************************/
/*************************************************************** Flash_End ***************************************************************/

