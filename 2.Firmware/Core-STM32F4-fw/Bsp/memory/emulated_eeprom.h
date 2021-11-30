/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_EEPROM_H
#define __STM32_EEPROM_H

/* Includes ------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#include "main.h"
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#if defined(STM32MP1xx)
/* Note for STM32MP1xx devices:
 * Those devices do not have non-volatile memory. The emulation is done
 * in RETRAM. Therefore data will be preserved *only* when VBAT is supplied
 * (e.g. A coin battery is connected to CN3 on STM32MP157A_DK1) and
 * the coprocessor is waken up from STANBY mode.
 * The data won't be preserved from cold boot, even if VBAT is connected.
 * See: https://community.st.com/s/question/0D50X0000B44pHUSQY/doesnt-the-mcu-coprocessor-have-nonvolatile-memory
 */
#define EEPROM_RETRAM_MODE
/* 4kB is the same size as EEPROM size of ATMega2560. */
#ifndef EEPROM_RETRAM_MODE_SIZE
#define EEPROM_RETRAM_MODE_SIZE ((uint32_t)(4*1024))
#endif
/* RETRAM start address is 0x00000000 (retset entry) and end address is
 * 0x00020000 (64kB in total). The by default, ldscript.ld for STM32MP1xx
 * does not define address between 0x00000298 (end of ISR Vector) and 0x00020000.
 * So it is okay to use in this address range. Make sure ldscript.ld does not
 * overrap the following address range.
 */
#ifndef EEPROM_RETRAM_START_ADDRESS
#define EEPROM_RETRAM_START_ADDRESS (0x00000400UL)
#endif
#define E2END (EEPROM_RETRAM_MODE_SIZE - 1)
#else
#ifndef FLASH_PAGE_SIZE
/*
 * FLASH_PAGE_SIZE is not defined for STM32F2xx, STM32F4xx and STM32F7xx
 * Could be redefined in variant.h or using build_opt.h
 * Warning: This is not the sector size, only the size used for EEPROM
 * emulation. Anyway, all the sector size will be erased.
 * So pay attention to not use this sector for other stuff.
 */
#define FLASH_PAGE_SIZE     ((uint32_t)(1*1024)) /* 1kB page */
#endif

#if defined(DATA_EEPROM_BASE) || defined(FLASH_EEPROM_BASE)

#if defined (DATA_EEPROM_END)
#define E2END (DATA_EEPROM_END - DATA_EEPROM_BASE)
#elif defined (DATA_EEPROM_BANK2_END)
/* assuming two contiguous banks */
#define DATA_EEPROM_END DATA_EEPROM_BANK2_END
#define E2END (DATA_EEPROM_BANK2_END - DATA_EEPROM_BASE)
#elif defined (FLASH_EEPROM_END)
#define DATA_EEPROM_BASE FLASH_EEPROM_BASE
#define DATA_EEPROM_END FLASH_EEPROM_END
#define E2END (DATA_EEPROM_END - DATA_EEPROM_BASE)
#endif /* __EEPROM_END */

#else /* _EEPROM_BASE */
#define E2END (FLASH_PAGE_SIZE - 1)
#endif /* _EEPROM_BASE */

#endif


/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

uint8_t eeprom_read_byte(uint32_t pos);
void eeprom_write_byte(uint32_t pos, uint8_t value);

#if !defined(DATA_EEPROM_BASE)
void eeprom_buffer_fill();
void eeprom_buffer_flush();
uint8_t eeprom_buffered_read_byte(uint32_t pos);
void eeprom_buffered_write_byte(uint32_t pos, uint8_t value);
#endif /* ! DATA_EEPROM_BASE */

#ifdef __cplusplus
}


#endif
#endif /* __STM32_EEPROM_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
