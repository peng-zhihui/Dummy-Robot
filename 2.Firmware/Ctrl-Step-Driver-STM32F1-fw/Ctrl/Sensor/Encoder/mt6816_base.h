#ifndef CTRL_STEP_FW_MT6816_H
#define CTRL_STEP_FW_MT6816_H

#include "encoder_base.h"
#include <cstdint>

class MT6816Base : public EncoderBase
{
public:
    explicit MT6816Base(uint16_t* _quickCaliDataPtr) :
        quickCaliDataPtr(_quickCaliDataPtr),
        spiRawData(SpiRawData_t{0})
    {
    }


    bool Init() override;
    uint16_t UpdateAngle() override;  // Get current rawAngle (rad)
    bool IsCalibrated() override;


private:
    typedef struct
    {
        uint16_t rawData;       // SPI raw 16bits data
        uint16_t rawAngle;      // 14bits rawAngle in rawData
        bool noMagFlag;
        bool checksumFlag;
    } SpiRawData_t;


    SpiRawData_t spiRawData;
    uint16_t* quickCaliDataPtr;
    uint16_t dataTx[2];
    uint16_t dataRx[2];
    uint8_t hCount;


    /***** Port Specified Implements *****/
    virtual void SpiInit();

    virtual uint16_t SpiTransmitAndRead16Bits(uint16_t _dataTx);

};

#endif
