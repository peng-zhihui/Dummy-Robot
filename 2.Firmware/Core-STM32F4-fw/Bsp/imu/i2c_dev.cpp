// I2Cdev library collection - Main I2C device class header file
// Abstracts bit and byte I2C R/W functions into a convenient class
// 7/26/2018 by Saeed Bazargan <Bazargan0241@hotmail.com>
// ported to STM32 HAL library from Arduino code

#include "i2c_dev.hpp"

// Hold pointer to inited HAL I2C device
static I2C_HandleTypeDef *I2Cdev_hi2c;

/** Default timeout value for read operations.
 * Set this to 0 to disable timeout detection.
 */
uint16_t I2Cdev_readTimeout = I2CDEV_DEFAULT_READ_TIMEOUT;

/** Sets device handle to use for communications
 * You can call this function and set any other device at any moment
 */
void I2Cdev_init(I2C_HandleTypeDef *hi2c)
{
    I2Cdev_hi2c = hi2c;
}

/** Read a single bit from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitNum Bit position to read (0-7)
 * @param data Container for single bit value
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev_readTimeout)
 * @return Status of read operation (true = success)
 */
uint8_t I2Cdev_readBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data, uint16_t timeout)
{
    uint8_t b;
    uint8_t count = I2Cdev_readByte(devAddr, regAddr, &b, timeout);
    *data = b & (1 << bitNum);
    return count;
}

/** Read a single bit from a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitNum Bit position to read (0-15)
 * @param data Container for single bit value
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev_readTimeout)
 * @return Status of read operation (true = success)
 */
uint8_t I2Cdev_readBitW(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t *data, uint16_t timeout)
{
    uint16_t b;
    uint8_t count = I2Cdev_readWord(devAddr, regAddr, &b, timeout);
    *data = b & (1 << bitNum);
    return count;
}

/** Read multiple bits from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitStart First bit position to read (0-7)
 * @param length Number of bits to read (not more than 8)
 * @param data Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev_readTimeout)
 * @return Status of read operation (true = success)
 */
uint8_t
I2Cdev_readBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data, uint16_t timeout)
{
    // 01101001 read byte
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    //    010   masked
    //   -> 010 shifted
    uint8_t count, b;
    if ((count = I2Cdev_readByte(devAddr, regAddr, &b, timeout)) != 0)
    {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        b &= mask;
        b >>= (bitStart - length + 1);
        *data = b;
    }
    return count;
}

/** Read multiple bits from a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitStart First bit position to read (0-15)
 * @param length Number of bits to read (not more than 16)
 * @param data Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev_readTimeout)
 * @return Status of read operation (1 = success, 0 = failure, -1 = timeout)
 */
uint8_t
I2Cdev_readBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint16_t *data, uint16_t timeout)
{
    // 1101011001101001 read byte
    // fedcba9876543210 bit numbers
    //    xxx           args: bitStart=12, length=3
    //    010           masked
    //           -> 010 shifted
    uint8_t count;
    uint16_t w;
    if ((count = I2Cdev_readWord(devAddr, regAddr, &w, timeout)) != 0)
    {
        uint16_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        w &= mask;
        w >>= (bitStart - length + 1);
        *data = w;
    }
    return count;
}

/** Read single byte from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param data Container for byte value read from device
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev_readTimeout)
 * @return Status of read operation (true = success)
 */
uint8_t I2Cdev_readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint16_t timeout)
{
    return I2Cdev_readBytes(devAddr, regAddr, 1, data, timeout);
}

/** Read single word from a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param data Container for word value read from device
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev_readTimeout)
 * @return Status of read operation (true = success)
 */
uint8_t I2Cdev_readWord(uint8_t devAddr, uint8_t regAddr, uint16_t *data, uint16_t timeout)
{
    return I2Cdev_readWords(devAddr, regAddr, 1, data, timeout);
}

/** Read multiple bytes from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register regAddr to read from
 * @param length Number of bytes to read
 * @param data Buffer to store read data in
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev_readTimeout)
 * @return Number of bytes read (-1 indicates failure)
 */
uint8_t I2Cdev_readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, uint16_t timeout)
{
    uint16_t tout = timeout > 0 ? timeout : I2CDEV_DEFAULT_READ_TIMEOUT;

    HAL_I2C_Master_Transmit(I2Cdev_hi2c, devAddr << 1, &regAddr, 1, tout);
    if (HAL_I2C_Master_Receive(I2Cdev_hi2c, devAddr << 1, data, length, tout) == HAL_OK) return length;
    return -1;
}

/** Read multiple words from a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register regAddr to read from
 * @param length Number of words to read
 * @param data Buffer to store read data in
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value in I2Cdev_readTimeout)
 * @return Number of words read (-1 indicates failure)
 */
uint8_t I2Cdev_readWords(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data, uint16_t timeout)
{
    uint16_t tout = timeout > 0 ? timeout : I2CDEV_DEFAULT_READ_TIMEOUT;

    HAL_I2C_Master_Transmit(I2Cdev_hi2c, devAddr << 1, &regAddr, 1, tout);
    if (HAL_I2C_Master_Receive(I2Cdev_hi2c, devAddr << 1, (uint8_t *) data, length * 2, tout) == HAL_OK)
        return length;
    else
        return -1;
}

/** write a single bit in an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitNum Bit position to write (0-7)
 * @param value New bit value to write
 * @return Status of operation (true = success)
 */
uint16_t I2Cdev_writeBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data)
{
    uint8_t b;
    I2Cdev_readByte(devAddr, regAddr, &b, I2Cdev_readTimeout);
    b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
    return I2Cdev_writeByte(devAddr, regAddr, b);
}

/** write a single bit in a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitNum Bit position to write (0-15)
 * @param value New bit value to write
 * @return Status of operation (true = success)
 */
uint16_t I2Cdev_writeBitW(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t data)
{
    uint16_t w;
    I2Cdev_readWord(devAddr, regAddr, &w, 100);
    w = (data != 0) ? (w | (1 << bitNum)) : (w & ~(1 << bitNum));
    return I2Cdev_writeWord(devAddr, regAddr, w);
}

/** Write multiple bits in an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitStart First bit position to write (0-7)
 * @param length Number of bits to write (not more than 8)
 * @param data Right-aligned value to write
 * @return Status of operation (true = success)
 */
uint16_t I2Cdev_writeBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data)
{
    //      010 value to write
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    // 00011100 mask byte
    // 10101111 original value (sample)
    // 10100011 original & ~mask
    // 10101011 masked | value
    uint8_t b;
    if (I2Cdev_readByte(devAddr, regAddr, &b, 100) != 0)
    {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        data <<= (bitStart - length + 1); // shift data into correct position
        data &= mask; // zero all non-important bits in data
        b &= ~(mask); // zero all important bits in existing byte
        b |= data; // combine data with existing byte
        return I2Cdev_writeByte(devAddr, regAddr, b);
    } else
    {
        return 0;
    }
}

/** Write multiple bits in a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitStart First bit position to write (0-15)
 * @param length Number of bits to write (not more than 16)
 * @param data Right-aligned value to write
 * @return Status of operation (true = success)
 */
uint16_t I2Cdev_writeBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint16_t data)
{
    //              010 value to write
    // fedcba9876543210 bit numbers
    //    xxx           args: bitStart=12, length=3
    // 0001110000000000 mask word
    // 1010111110010110 original value (sample)
    // 1010001110010110 original & ~mask
    // 1010101110010110 masked | value
    uint16_t w;
    if (I2Cdev_readWord(devAddr, regAddr, &w, 100) != 0)
    {
        uint16_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        data <<= (bitStart - length + 1); // shift data into correct position
        data &= mask; // zero all non-important bits in data
        w &= ~(mask); // zero all important bits in existing word
        w |= data; // combine data with existing word
        return I2Cdev_writeWord(devAddr, regAddr, w);
    } else
    {
        return 0;
    }
}

/** Write single byte to an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register address to write to
 * @param data New byte value to write
 * @return Status of operation (true = success)
 */
uint16_t I2Cdev_writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data)
{
    return I2Cdev_writeBytes(devAddr, regAddr, 1, &data);
}

/** Write single word to a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register address to write to
 * @param data New word value to write
 * @return Status of operation (true = success)
 */
uint16_t I2Cdev_writeWord(uint8_t devAddr, uint8_t regAddr, uint16_t data)
{
    return I2Cdev_writeWords(devAddr, regAddr, 1, &data);
}

/** Write multiple bytes to an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register address to write to
 * @param length Number of bytes to write
 * @param data Buffer to copy new data from
 * @return Status of operation (true = success)
 */
uint16_t I2Cdev_writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t Size, uint8_t *pData)
{
    // Creating dynamic array to store regAddr + data in one buffer
    uint8_t *dynBuffer;
    dynBuffer = (uint8_t *) malloc(sizeof(uint8_t) * (Size + 1));
    dynBuffer[0] = regAddr;

    // copy array
    memcpy(dynBuffer + 1, pData, sizeof(uint8_t) * Size);

    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(I2Cdev_hi2c, devAddr << 1, dynBuffer, Size + 1, 1000);
    free(dynBuffer);
    return status == HAL_OK;
}

/** Write multiple words to a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register address to write to
 * @param length Number of words to write
 * @param data Buffer to copy new data from
 * @return Status of operation (true = success)
 */
uint16_t I2Cdev_writeWords(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data)
{
    // Creating dynamic array to store regAddr + data in one buffer
    uint8_t *dynBuffer;
    dynBuffer = (uint8_t *) malloc(sizeof(uint8_t) + sizeof(uint16_t) * length);
    dynBuffer[0] = regAddr;

    // copy array
    memcpy(dynBuffer + 1, data, sizeof(uint16_t) * length);
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(I2Cdev_hi2c, devAddr << 1, dynBuffer,
                                                       sizeof(uint8_t) + sizeof(uint16_t) * length, 1000);
    free(dynBuffer);
    return status == HAL_OK;
}

