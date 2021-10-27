// ported to STM32 HAL library from Arduino code


#ifndef _I2CDEV_H_
#define _I2CDEV_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f4xx_hal.h"
#include "i2c.h"

#define true 1
#define false 0

// 1000ms default read timeout (modify with "I2Cdev::readTimeout = [ms];")
#define I2CDEV_DEFAULT_READ_TIMEOUT     1000

void I2Cdev_init(I2C_HandleTypeDef *hi2c);

uint8_t I2Cdev_readBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data,
                       uint16_t timeout = I2CDEV_DEFAULT_READ_TIMEOUT);
uint8_t I2Cdev_readBitW(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t *data,
                        uint16_t timeout = I2CDEV_DEFAULT_READ_TIMEOUT);
uint8_t
I2Cdev_readBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data,
                uint16_t timeout = I2CDEV_DEFAULT_READ_TIMEOUT);
uint8_t
I2Cdev_readBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint16_t *data,
                 uint16_t timeout = I2CDEV_DEFAULT_READ_TIMEOUT);
uint8_t
I2Cdev_readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint16_t timeout = I2CDEV_DEFAULT_READ_TIMEOUT);
uint8_t
I2Cdev_readWord(uint8_t devAddr, uint8_t regAddr, uint16_t *data, uint16_t timeout = I2CDEV_DEFAULT_READ_TIMEOUT);
uint8_t I2Cdev_readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data,
                         uint16_t timeout = I2CDEV_DEFAULT_READ_TIMEOUT);
uint8_t I2Cdev_readWords(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data,
                         uint16_t timeout = I2CDEV_DEFAULT_READ_TIMEOUT);

uint16_t I2Cdev_writeBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data);
uint16_t I2Cdev_writeBitW(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t data);
uint16_t I2Cdev_writeBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data);
uint16_t I2Cdev_writeBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint16_t data);
uint16_t I2Cdev_writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data);
uint16_t I2Cdev_writeWord(uint8_t devAddr, uint8_t regAddr, uint16_t data);
uint16_t I2Cdev_writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data);
uint16_t I2Cdev_writeWords(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data);

#endif /* _I2CDEV_H_ */

