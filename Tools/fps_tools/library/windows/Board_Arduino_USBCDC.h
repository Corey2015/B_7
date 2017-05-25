#ifndef __Board_Arduino_USBCDC_h__
#define __Board_Arduino_USBCDC_h__


#include "common.h"
#include "Error.h"
#include "Protocol.h"
#include "Board.h"
#include "SerialPort.h"


class Board_Arduino_USBCDC : public Board_Base, public Protocol {

public:

    enum DefaultPinList {
        PIN_LDO_EN  = 5,  // Pin # to turn on LDO
        PIN_RSTN    = 4,  // Pin # to sensor reset
        PIN_INTR    = 7,  // Pin # to sensor interrupt
        PIN_SPI_CSB = 10, // Pin # to sensor SPI chip-select
    };

    Board_Arduino_USBCDC(__INPUT Device &serialDev,
                         __INPUT u08_t  pinLDOEn  = PIN_LDO_EN,
                         __INPUT u08_t  pinResetN = PIN_RSTN,
                         __INPUT u08_t  pinIntr   = PIN_INTR,
                         __INPUT u08_t  pinSPICSB = PIN_SPI_CSB);
    virtual ~Board_Arduino_USBCDC();

    // Open a board
    err_t Open();

    // Close a board
    err_t Close();

    // Set communication time-out
    err_t SetTimeout(__INPUT u32_t timeoutMsec)
    {
        return m_SerialPort->SetTimeout(timeoutMsec);
    }

    // Set communication queue size
    err_t SetQueueSize(__INPUT u32_t rxBufSize,
                       __INPUT u32_t txBufSize)
    {
        return m_SerialPort->SetQueueSize(rxBufSize, txBufSize);
    }

    // Read configuration of a SPI port
    err_t ReadSPIConfig(__OUTPUT SPIMode     *mode,
                        __OUTPUT u32_t       *speed,
                        __OUTPUT u32_t       *delay,
                        __OUTPUT BitSequence *bitSeq);

    // Write configuration to a SPI port
    err_t WriteSPIConfig(__INPUT SPIMode     mode   = SPI_MODE_3,
                         __INPUT u32_t       speed  = (8 * 1000 * 1000),
                         __INPUT u32_t       delay  = 0,
                         __INPUT BitSequence bitSeq = BIT_SEQ_MSB_FIRST);

    // Do a low-level SPI transfer
    err_t DoSPITransfer(__INPUT  u32_t txLen,
                        __INPUT  u08_t *txData,
                        __OUTPUT u32_t *rxLen,
                        __OUTPUT u08_t *rxData);

    // Read configuration of a GPIO pin
    err_t ReadGPIOConfig(__INPUT  u08_t         pin,
                         __OUTPUT GPIODirection *dir,
                         __OUTPUT GPIOPull      *pull);

    // Write configuration to a GPIO pin
    err_t WriteGPIOConfig(__INPUT u08_t         pin,
                          __INPUT GPIODirection dir  = GPIO_DIR_OUT,
                          __INPUT GPIOPull      pull = GPIO_PULL_NONE);

    // Read data from a GPIO pin
    err_t ReadGPIOData(__INPUT  u08_t pin,
                       __OUTPUT bit_t *data);

    // Write data to a GPIO pin
    err_t WriteGPIOData(__INPUT u08_t pin,
                        __INPUT bit_t data);

    // Get firmware version
    err_t GetFirmwareVersion(__INPUT  u32_t dataLen,
                             __OUTPUT u08_t *data);

    // Read ADC value
    err_t ReadADC(__INPUT  u08_t pin,
                  __OUTPUT u16_t *data);

    // Transmit commands to the board
    err_t TransmitCommand(__INPUT u32_t dataLen,
                          __INPUT u08_t *data);

    // Receive responses from the board
    err_t ReceiveResponse(__INPUT  u32_t dataLen,
                          __OUTPUT u08_t *data);

private:

	SerialPort *m_SerialPort;
    u08_t      m_PinLDOEn;
    u08_t      m_PinResetN;
    u08_t      m_PinIntr;
    u08_t      m_PinSPICSB;

};


#endif // __Board_Arduino_USBCDC_h__
