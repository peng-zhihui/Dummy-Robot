#ifndef FlashStorage_STM32_h
#define FlashStorage_STM32_h


#if !(defined(STM32F0) || defined(STM32F1) || defined(STM32F2) || defined(STM32F3) || defined(STM32F4) || defined(STM32F7) || \
       defined(STM32L0) || defined(STM32L1) || defined(STM32L4) || defined(STM32H7) || defined(STM32G0) || defined(STM32G4) || \
       defined(STM32WB) || defined(STM32MP1) || defined(STM32L5))
#error This code is intended to run on STM32F/L/H/G/WB/MP1 platform! Please check your Tools->Board setting.
#endif

#define FLASH_STORAGE_STM32_VERSION     "FlashStorage_STM32 v1.1.0"

// Only use this with emulated EEPROM, without integrated EEPROM
#if !defined(DATA_EEPROM_BASE)

#include "emulated_eeprom.h"

class EEPROMClass
{
public:

    EEPROMClass() : _initialized(false), _dirtyBuffer(false), _commitASAP(true), _validEEPROM(true)
    {}

    /**
     * Read an eeprom cell
     * @param index
     * @return value
     */
    uint8_t read(int address)
    {
        if (!_initialized)
            init();

        return eeprom_buffered_read_byte(address);
    }

    /**
     * Update an eeprom cell
     * @param index
     * @param value
     */
    void update(int address, uint8_t value)
    {
        if (!_initialized)
            init();

        if (eeprom_buffered_read_byte(address) != value)
        {
            _dirtyBuffer = true;
            eeprom_buffered_write_byte(address, value);
        }
    }

    /**
     * Write value to an eeprom cell
     * @param index
     * @param value
     */
    void write(int address, uint8_t value)
    {
        update(address, value);
    }

    /**
     * Update eeprom cells from an object
     * @param index
     * @param value
     */
    //Functionality to 'get' and 'put' objects to and from EEPROM.
    template<typename T>
    T &get(int idx, T &t)
    {
        // Copy the data from the flash to the buffer if not yet
        if (!_initialized)
            init();

        uint16_t offset = idx;
        uint8_t* _pointer = (uint8_t * ) & t;

        for (uint16_t count = sizeof(T); count; --count, ++offset)
        {
            *_pointer++ = eeprom_buffered_read_byte(offset);
        }

        return t;
    }

    template<typename T>
    const T &put(int idx, const T &t)
    {
        // Copy the data from the flash to the buffer if not yet
        if (!_initialized)
            init();

        uint16_t offset = idx;

        const uint8_t* _pointer = (const uint8_t*) &t;

        for (uint16_t count = sizeof(T); count; --count, ++offset)
        {
            eeprom_buffered_write_byte(offset, *_pointer++);
        }

        if (_commitASAP)
        {
            // Save the data from the buffer to the flash right away
            eeprom_buffer_flush();

            _dirtyBuffer = false;
            _validEEPROM = true;
        } else
        {
            // Delay saving the data from the buffer to the flash. Just flag and wait for commit() later
            _dirtyBuffer = true;
        }

        return t;
    }

    /**
     * Check whether the eeprom data is valid
     * @return true, if eeprom data is valid (has been written at least once), false if not
     */
    bool isValid()
    {
        return _validEEPROM;
    }

    /**
     * Write previously made eeprom changes to the underlying flash storage
     * Use this with care: Each and every commit will harm the flash and reduce it's lifetime (like with every flash memory)
     */
    void commit()
    {
        if (!_initialized)
            init();

        if (_dirtyBuffer)
        {
            // Save the data from the buffer to the flash
            eeprom_buffer_flush();

            _dirtyBuffer = false;
            _validEEPROM = true;
        }
    }

    uint16_t length()
    { return E2END + 1; }

    void setCommitASAP(bool value = true)
    { _commitASAP = value; }
    bool getCommitASAP()
    { return _commitASAP; }

private:

    void init()
    {
        // Copy the data from the flash to the buffer
        eeprom_buffer_fill();
        _initialized = true;
    }

    bool _initialized;
    bool _dirtyBuffer;
    bool _commitASAP;
    bool _validEEPROM;
};


#else

#include "EEPROM.h"

#endif    // #if !defined(DATA_EEPROM_BASE)

#endif    //#ifndef FlashAsEEPROM_SAMD_h
