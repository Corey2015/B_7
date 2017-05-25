#ifndef __Board_h__
#define __Board_h__


#include "common.h"
#include "Error.h"


class Board_Base {

public:

    enum BitSequence {
        BIT_SEQ_MSB_FIRST = 0,
        BIT_SEQ_LSB_FIRST = 1,
    };

    enum SPIMode {
        SPI_MODE_0 = 0,
        SPI_MODE_1 = 1,
        SPI_MODE_2 = 2,
        SPI_MODE_3 = 3
    };

    enum GPIODirection {
        GPIO_DIR_IN    = 0,
        GPIO_DIR_OUT   = 1,
        GPIO_DIR_INOUT = 2
    };

    enum GPIOPull {
        GPIO_PULL_NONE = 0,
        GPIO_PULL_UP   = 1,
        GPIO_PULL_DOWN = 2,
        GPIO_PULL_KEEP = 3
    };

    Board_Base() {}
    virtual ~Board_Base() {}

    // Open a board
    virtual err_t Open() = 0;

    // Close a board
    virtual err_t Close() = 0;

    // Set communication time-out
    virtual err_t SetTimeout(__INPUT u32_t timeoutMsec) = 0;

    // Set communication queue size
    virtual err_t SetQueueSize(__INPUT u32_t rxBufSize,
                               __INPUT u32_t txBufSize) = 0;

    // Read configuration of a SPI port
    virtual err_t ReadSPIConfig(__INPUT  u08_t       csPin,
                                __OUTPUT SPIMode     *mode,
                                __OUTPUT u32_t       *speed,
                                __OUTPUT u32_t       *delay,
                                __OUTPUT BitSequence *seq)
    {
        return Error::ERROR_NOT_IMPLEMENTED;
    }

    // Write configuration to a SPI port
    virtual err_t WriteSPIConfig(__INPUT u08_t       csPin,
                                 __INPUT SPIMode     mode  = SPI_MODE_3,
                                 __INPUT u32_t       speed = (8 * 1000 * 1000),
                                 __INPUT u32_t       delay = 0,
                                 __INPUT BitSequence seq   = BIT_SEQ_MSB_FIRST)
    {
        return Error::ERROR_NOT_IMPLEMENTED;
    }

    // Do a low-level SPI transfer
    virtual err_t DoSPITransfer(__INPUT  u08_t csPin,
                                __INPUT  u32_t *txLen,
                                __INPUT  u08_t *txData,
                                __OUTPUT u32_t *rxLen,
                                __OUTPUT u08_t *rxData)
    {
        return Error::ERROR_NOT_IMPLEMENTED;
    }

    // Read configuration of a I2C port
    virtual err_t ReadI2CConfig(__INPUT  u08_t       devId,
                                __OUTPUT u32_t       *speed,
                                __OUTPUT BitSequence *seq)
    {
        return Error::ERROR_NOT_IMPLEMENTED;
    }

    // Write configuration to a I2C port
    virtual err_t WriteI2CConfig(__INPUT u08_t       devId,
                                 __INPUT u32_t       speed = (400 * 1000),
                                 __INPUT BitSequence seq   = BIT_SEQ_MSB_FIRST)
    {
        return Error::ERROR_NOT_IMPLEMENTED;
    }

    // Read data from a I2C port
    virtual err_t ReadI2CData(__INPUT  u08_t devId,
                              __INPUT  u32_t dataLen,
                              __OUTPUT u08_t *data)
    {
        return Error::ERROR_NOT_IMPLEMENTED;
    }

    // Write data to an I2C port
    virtual err_t WriteI2CData(__INPUT u08_t devId,
                               __INPUT u32_t dataLen,
                               __INPUT u08_t *data)
    {
        return Error::ERROR_NOT_IMPLEMENTED;
    }

    // Read configuration of a GPIO pin
    virtual err_t ReadGPIOConfig(__INPUT  u08_t         pin,
                                 __OUTPUT GPIODirection *dir,
                                 __OUTPUT GPIOPull      *pull)
    {
        return Error::ERROR_NOT_IMPLEMENTED;
    }

    // Write configuration to a GPIO pin
    virtual err_t WriteGPIOConfig(__INPUT u08_t         pin,
                                  __INPUT GPIODirection dir  = GPIO_DIR_OUT,
                                  __INPUT GPIOPull      pull = GPIO_PULL_NONE)
    {
        return Error::ERROR_NOT_IMPLEMENTED;
    }

    // Read data from a GPIO pin
    virtual err_t ReadGPIOData(__INPUT  u08_t pin,
                               __OUTPUT bit_t *data)
    {
        return Error::ERROR_NOT_IMPLEMENTED;
    }

    // Write data to a GPIO pin
    virtual err_t WriteGPIOData(__INPUT u08_t pin,
                                __INPUT bit_t data)
    {
        return Error::ERROR_NOT_IMPLEMENTED;
    }

    // Get firmware version
    virtual err_t GetFirmwareVersion(__INPUT  u32_t dataLen,
                                     __OUTPUT u08_t *data)
    {
        return Error::ERROR_NOT_IMPLEMENTED;
    }

};

#if defined(__ARDUINO_USBCDC__)
    #include "Board_Arduino_USBCDC.h"
    class Board_Arduino_USBCDC;
    typedef Board_Arduino_USBCDC Board;
#else
    #error " [ ERROR ] Board is not supported! "
#endif


#endif // __Board_h__
