#ifndef __STM32_EEPROM_HPP
#define __STM32_EEPROM_HPP

#include <string.h>
#include "emulated_eeprom.h"

#ifdef __cplusplus
extern "C" {

#endif

/* Be able to change FLASH_BANK_NUMBER to use if relevant */
#if !defined(FLASH_BANK_NUMBER) && \
    (defined(STM32F0xx) || defined(STM32F1xx) || defined(STM32G4xx) || \
     defined(STM32H7xx) || defined(STM32L4xx) || defined(STM32L5xx))
/* For STM32F0xx, FLASH_BANK_1 is not defined only FLASH_BANK1_END is defined */
#if defined(STM32F0xx)
#define FLASH_BANK_1 1U
#endif
#if defined(FLASH_BANK_2)
#define FLASH_BANK_NUMBER   FLASH_BANK_2
#else
#define FLASH_BANK_NUMBER   FLASH_BANK_1
#endif /* FLASH_BANK_2 */
#ifndef FLASH_BANK_NUMBER
#error "FLASH_BANK_NUMBER could not be defined"
#endif
#endif /* !FLASH_BANK_NUMBER */

/* Be able to change FLASH_DATA_SECTOR to use if relevant */
#if defined(STM32F2xx) || defined(STM32F4xx) || defined(STM32F7xx) || \
    defined(STM32H7xx)
#if !defined(FLASH_DATA_SECTOR)
#define FLASH_DATA_SECTOR   ((uint32_t)(FLASH_SECTOR_TOTAL - 1))
#else
#ifndef FLASH_BASE_ADDRESS
#error "FLASH_BASE_ADDRESS have to be defined when FLASH_DATA_SECTOR is defined"
#endif
#endif /* !FLASH_DATA_SECTOR */
#endif /* STM32F2xx || STM32F4xx || STM32F7xx */

/* Be able to change FLASH_PAGE_NUMBER to use if relevant */
#if !defined(FLASH_PAGE_NUMBER) && \
    (defined (STM32G0xx) || defined(STM32G4xx) || defined (STM32L4xx) || \
     defined (STM32L5xx) || defined(STM32WBxx))
#define FLASH_PAGE_NUMBER   ((uint32_t)((FLASH_SIZE / FLASH_PAGE_SIZE) - 1))
#endif /* !FLASH_PAGE_NUMBER */

/* Be able to change FLASH_END to use */
#if !defined(FLASH_END)
#if defined (STM32F0xx) || defined (STM32F1xx)
#if defined (FLASH_BANK2_END) && (FLASH_BANK_NUMBER == FLASH_BANK_2)
#define FLASH_END  FLASH_BANK2_END
#elif defined (FLASH_BANK1_END) && (FLASH_BANK_NUMBER == FLASH_BANK_1)
#define FLASH_END  FLASH_BANK1_END
#endif
#elif defined (STM32F3xx)
static inline uint32_t get_flash_end(void)
{
  uint32_t size;
  switch ((*((uint16_t *)FLASH_SIZE_DATA_REGISTER))) {
    case 0x200U:
      size = 0x0807FFFFU;
      break;
    case 0x100U:
      size = 0x0803FFFFU;
      break;
    case 0x80U:
      size = 0x0801FFFFU;
      break;
    case 0x40U:
      size = 0x0800FFFFU;
      break;
    case 0x20U:
      size = 0x08007FFFU;
      break;
    default:
      size = 0x08003FFFU;
      break;
  }
  return size;
}
#define FLASH_END  get_flash_end()
#elif defined(STM32G0xx) || defined(STM32G4xx) || defined (STM32L4xx) || \
      defined (STM32L5xx) || defined(STM32WBxx)
/* If FLASH_PAGE_NUMBER is defined by user, this is not really end of the flash */
#define FLASH_END  ((uint32_t)(FLASH_BASE + (((FLASH_PAGE_NUMBER +1) * FLASH_PAGE_SIZE))-1))
#elif defined(EEPROM_RETRAM_MODE)
#define FLASH_END  ((uint32_t)(EEPROM_RETRAM_START_ADDRESS + EEPROM_RETRAM_MODE_SIZE -1))
#elif defined(DATA_EEPROM_END)
#define FLASH_END DATA_EEPROM_END
#endif
#ifndef FLASH_END
#error "FLASH_END could not be defined"
#endif
#endif /* FLASH_END */

/* Be able to change FLASH_BASE_ADDRESS to use */
#ifndef FLASH_BASE_ADDRESS
/*
 * By default, Use the last page of the flash to store data
 * in order to prevent overwritting
 * program data
 */
#if defined(EEPROM_RETRAM_MODE)
#define FLASH_BASE_ADDRESS  EEPROM_RETRAM_START_ADDRESS
#else
#define FLASH_BASE_ADDRESS  ((uint32_t)((FLASH_END + 1) - FLASH_PAGE_SIZE))
#endif
#ifndef FLASH_BASE_ADDRESS
#error "FLASH_BASE_ADDRESS could not be defined"
#endif
#endif /* FLASH_BASE_ADDRESS */

#if !defined(DATA_EEPROM_BASE)
static uint8_t eeprom_buffer[E2END + 1] __attribute__((aligned(8))) = {0};
#endif

/**
  * @brief  Function reads a byte from emulated eeprom (flash)
  * @param  pos : address to read
  * @retval byte : data read from eeprom
  */
uint8_t eeprom_read_byte(const uint32_t pos)
{
#if defined(DATA_EEPROM_BASE)
    __IO uint8_t data = 0;
    if (pos <= (DATA_EEPROM_END - DATA_EEPROM_BASE)) {
      /* with actual EEPROM, pos is a relative address */
      data = *(__IO uint8_t *)(DATA_EEPROM_BASE + pos);
    }
    return (uint8_t)data;
#else
    eeprom_buffer_fill();
    return eeprom_buffered_read_byte(pos);
#endif /* _EEPROM_BASE */
}

/**
  * @brief  Function writes a byte to emulated eeprom (flash)
  * @param  pos : address to write
  * @param  value : value to write
  * @retval none
  */
void eeprom_write_byte(uint32_t pos, uint8_t value)
{
#if defined(DATA_EEPROM_BASE)
    /* with actual EEPROM, pos is a relative address */
    if (pos <= (DATA_EEPROM_END - DATA_EEPROM_BASE)) {
      if (HAL_FLASHEx_DATAEEPROM_Unlock() == HAL_OK) {
        HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, (pos + DATA_EEPROM_BASE), (uint32_t)value);
        HAL_FLASHEx_DATAEEPROM_Lock();
      }
    }
#else
    eeprom_buffered_write_byte(pos, value);
    eeprom_buffer_flush();
#endif /* _EEPROM_BASE */
}

#if !defined(DATA_EEPROM_BASE)

/**
  * @brief  Function reads a byte from the eeprom buffer
  * @param  pos : address to read
  * @retval byte : data read from eeprom
  */
uint8_t eeprom_buffered_read_byte(const uint32_t pos)
{
    return eeprom_buffer[pos];
}

/**
  * @brief  Function writes a byte to the eeprom buffer
  * @param  pos : address to write
  * @param  value : value to write
  * @retval none
  */
void eeprom_buffered_write_byte(uint32_t pos, uint8_t value)
{
    eeprom_buffer[pos] = value;
}

/**
  * @brief  This function copies the data from flash into the buffer
  * @param  none
  * @retval none
  */
void eeprom_buffer_fill(void)
{
    memcpy(eeprom_buffer, (uint8_t*) (FLASH_BASE_ADDRESS), E2END + 1);
}

#if defined(EEPROM_RETRAM_MODE)

/**
  * @brief  This function writes the buffer content into the flash
  * @param  none
  * @retval none
  */
void eeprom_buffer_flush(void)
{
  memcpy((uint8_t *)(FLASH_BASE_ADDRESS), eeprom_buffer, E2END + 1);
}

#else /* defined(EEPROM_RETRAM_MODE) */

/**
  * @brief  This function writes the buffer content into the flash
  * @param  none
  * @retval none
  */
void eeprom_buffer_flush(void)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t offset = 0;
    uint32_t address = FLASH_BASE_ADDRESS;
    uint32_t address_end = FLASH_BASE_ADDRESS + E2END;
#if defined (STM32F0xx) || defined (STM32F1xx) || defined (STM32F3xx) || \
    defined (STM32G0xx) || defined (STM32G4xx) || \
    defined (STM32L4xx) || defined (STM32L5xx) || defined (STM32WBxx)
    uint32_t pageError = 0;
    uint64_t data = 0;

    /* ERASING page */
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
#if defined (STM32F1xx) || defined (STM32G4xx) || defined (STM32L4xx) || \
    defined (STM32L5xx)
    EraseInitStruct.Banks = FLASH_BANK_NUMBER;
#endif
#if defined (STM32G0xx) || defined (STM32G4xx) || defined (STM32L4xx) || \
    defined (STM32L5xx) || defined (STM32WBxx)
    EraseInitStruct.Page = FLASH_PAGE_NUMBER;
#else
    EraseInitStruct.PageAddress = FLASH_BASE_ADDRESS;
#endif
    EraseInitStruct.NbPages = 1;

    if (HAL_FLASH_Unlock() == HAL_OK)
    {
#if defined (STM32G0xx) || defined (STM32G4xx) || defined (STM32L4xx) || \
      defined (STM32L5xx) || defined (STM32WBxx)
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
#else
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR);
#endif
        if (HAL_FLASHEx_Erase(&EraseInitStruct, &pageError) == HAL_OK)
        {
            while (address <= address_end)
            {

                data = *((uint64_t*) ((uint8_t*) eeprom_buffer + offset));

                if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data) == HAL_OK)
                {
                    address += 8;
                    offset += 8;
                } else
                {
                    address = address_end + 1;
                }
            }
        }
        HAL_FLASH_Lock();
    }
#else
    uint32_t SectorError = 0;
#if defined(STM32H7xx)
    uint64_t data[4] = {0x0000};
#else
    uint32_t data = 0;
#endif

    /* ERASING page */
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
#if defined(STM32H7xx)
    EraseInitStruct.Banks = FLASH_BANK_NUMBER;
#endif
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector = FLASH_DATA_SECTOR;
    EraseInitStruct.NbSectors = 1;

    HAL_FLASH_Unlock();

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) == HAL_OK)
    {
        while (address <= address_end)
        {
#if defined(STM32H7xx)
            /* 256 bits */
            memcpy(&data, eeprom_buffer + offset, 8 * sizeof(uint32_t));
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, (uint32_t)data) == HAL_OK) {
              address += 32;
              offset += 32;
#else
            memcpy(&data, eeprom_buffer + offset, sizeof(uint32_t));
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data) == HAL_OK)
            {
                address += 4;
                offset += 4;
#endif
            } else
            {
                address = address_end + 1;
            }
        }
    }
    HAL_FLASH_Lock();
#endif
}

#endif /* defined(EEPROM_RETRAM_MODE) */

#endif /* ! DATA_EEPROM_BASE */

#ifdef __cplusplus
}
#endif
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
