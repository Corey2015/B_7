#include "common.h"
#include "debug.h"
#include "Error.h"
#include "Array.h"
#include "SerialPort.h"
#include "Board_Arduino_USBCDC.h"


Board_Arduino_USBCDC::Board_Arduino_USBCDC(__INPUT Device &serialDev,
                                           __INPUT u08_t  pinLDOEn,
                                           __INPUT u08_t  pinResetN,
                                           __INPUT u08_t  pinIntr,
                                           __INPUT u08_t  pinSPICSB) :
	m_SerialPort(new SerialPort(serialDev)),
    m_PinLDOEn(pinLDOEn),
    m_PinResetN(pinResetN),
    m_PinIntr(pinIntr),
    m_PinSPICSB(pinSPICSB)
{
}

Board_Arduino_USBCDC::~Board_Arduino_USBCDC()
{
    Close();
	delete m_SerialPort;
}

err_t
Board_Arduino_USBCDC::Open()
{
    err_t error = Error::ERROR_OK;

    try {
        if (m_SerialPort == NULL) {
            throw Error::ERROR_MEMORY_ALLOC;
        }

        error = m_SerialPort->Open(FALSE,
                                   115200,
                                   8,
                                   SerialPort::PARITY_BIT_NONE,
                                   SerialPort::STOP_BIT_ONE);
        if (Error::IsFailed(error)) {
            throw error;
        }
    }
    catch (const err_t code) {
        PRINT_ERROR_CODE(error = code);
    }

    return error;
}

err_t
Board_Arduino_USBCDC::Close()
{
    m_SerialPort->Close();
    return Error::ERROR_OK;
}

err_t
Board_Arduino_USBCDC::ReadSPIConfig(__OUTPUT SPIMode     *mode,
                                    __OUTPUT u32_t       *speed,
                                    __OUTPUT u32_t       *delay,
                                    __OUTPUT BitSequence *bitSeq)
{
    err_t error = Error::ERROR_OK;

    try {
        Array<u08_t> cmdBuf(2);

        cmdBuf.At(0) = CMD_READ_SPI_CONFIG;
        cmdBuf.At(1) = m_PinSPICSB;

        error = TransmitCommand(cmdBuf.GetLength(), cmdBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        Array<u08_t> rspBuf(5);

        error = ReceiveResponse(rspBuf.GetLength(), rspBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        if (rspBuf.At(0) != RSP_SUCCESS) {
            throw Error::ERROR_RETURNED;
        }

        if (mode != NULL) {
            switch (rspBuf.At(1)) {
                case 0 : *mode = SPI_MODE_0; break;
                case 1 : *mode = SPI_MODE_1; break;
                case 2 : *mode = SPI_MODE_2; break;
                case 3 : *mode = SPI_MODE_3; break;
            }
        }

        if (speed != NULL) *speed = rspBuf.At(2);
        if (delay != NULL) *delay = rspBuf.At(3);

        if (bitSeq != NULL) {
            switch (rspBuf.At(4)) {
                case 0 : *bitSeq = BIT_SEQ_MSB_FIRST; break;
                case 1 : *bitSeq = BIT_SEQ_LSB_FIRST; break;
            }
        }
    }
    catch (const err_t code) {
        PRINT_ERROR_CODE(error = code);
    }

    return error;
}

err_t
Board_Arduino_USBCDC::WriteSPIConfig(__INPUT SPIMode     mode,
                                     __INPUT u32_t       speed,
                                     __INPUT u32_t       delay,
                                     __INPUT BitSequence bitSeq)
{
    err_t error = Error::ERROR_OK;

    try {
        Array<u08_t> cmdBuf(6);

        cmdBuf.At(0) = CMD_WRITE_SPI_CONFIG;
        cmdBuf.At(1) = m_PinSPICSB;

        switch (mode) {
            case SPI_MODE_0 : cmdBuf.At(2) = 0; break;
            case SPI_MODE_1 : cmdBuf.At(2) = 1; break;
            case SPI_MODE_2 : cmdBuf.At(2) = 2; break;
            case SPI_MODE_3 : cmdBuf.At(2) = 3; break;
        }

        cmdBuf.At(3) = speed;
        cmdBuf.At(4) = delay;

        switch (bitSeq) {
            case BIT_SEQ_MSB_FIRST : cmdBuf.At(5) = 0; break;
            case BIT_SEQ_LSB_FIRST : cmdBuf.At(5) = 1; break;
        }

        error = TransmitCommand(cmdBuf.GetLength(), cmdBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        Array<u08_t> rspBuf(1);

        error = ReceiveResponse(rspBuf.GetLength(), rspBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        if (rspBuf.At(0) != RSP_SUCCESS) {
            throw Error::ERROR_RETURNED;
        }
    }
    catch (const err_t code) {
        PRINT_ERROR_CODE(error = code);
    }

    return error;
}

err_t
Board_Arduino_USBCDC::DoSPITransfer(__INPUT  u32_t txLen,
                                    __INPUT  u08_t *txData,
                                    __OUTPUT u32_t *rxLen,
                                    __OUTPUT u08_t *rxData)
{
    err_t error = Error::ERROR_OK;

    try {
        Array<u08_t> cmdBuf(txLen + 4);

        cmdBuf.At(0) = CMD_DO_SPI_TRANSFER;
        cmdBuf.At(1) = m_PinSPICSB;
        cmdBuf.At(2) = (u08_t) ((txLen & 0xFF00) >> 8);
        cmdBuf.At(3) = (u08_t) ((txLen & 0x00FF) >> 0);

        if (txData != NULL) {
            for (u32_t i = 0; i < txLen; i++) {
                cmdBuf.At(i + 4) = txData[i];
            }
        }

        error = TransmitCommand(cmdBuf.GetLength(), cmdBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        Array<u08_t> rspBuf(txLen + 1);

        error = ReceiveResponse(rspBuf.GetLength(), rspBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        if (rspBuf.At(0) != RSP_SUCCESS) {
            throw Error::ERROR_RETURNED;
        }

        if (rxLen != NULL) {
            *rxLen = rspBuf.GetLength() - 1;
        }

        if (rxData != NULL) {
            for (u32_t i = 1; i < rspBuf.GetLength(); i++) {
                rxData[i - 1] = rspBuf.At(i);
            }
        }
    }
    catch (const err_t code) {
        PRINT_ERROR_CODE(error = code);
    }

    return error;
}

err_t
Board_Arduino_USBCDC::ReadGPIOConfig(__INPUT  u08_t         pin,
                                     __OUTPUT GPIODirection *dir,
                                     __OUTPUT GPIOPull      *pull)
{
    err_t error = Error::ERROR_OK;

    try {
        Array<u08_t> cmdBuf(2);

        cmdBuf.At(0) = CMD_READ_GPIO_CONFIG;
        cmdBuf.At(1) = pin;

        error = TransmitCommand(cmdBuf.GetLength(), cmdBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        Array<u08_t> rspBuf(3);

        error = ReceiveResponse(rspBuf.GetLength(), rspBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        if (rspBuf.At(0) != RSP_SUCCESS) {
            throw Error::ERROR_RETURNED;
        }

        if (dir != NULL) {
            switch (rspBuf.At(1)) {
                case 0 : *dir = GPIO_DIR_IN;    break;
                case 1 : *dir = GPIO_DIR_OUT;   break;
                case 2 : *dir = GPIO_DIR_INOUT; break;
            }
        }

        if (pull != NULL) {
            switch (rspBuf.At(2)) {
                case 0 : *pull = GPIO_PULL_NONE; break;
                case 1 : *pull = GPIO_PULL_UP;   break;
                case 2 : *pull = GPIO_PULL_DOWN; break;
                case 3 : *pull = GPIO_PULL_KEEP; break;
            }
        }
    }
    catch (const err_t code) {
        PRINT_ERROR_CODE(error = code);
    }

    return error;
}

err_t
Board_Arduino_USBCDC::WriteGPIOConfig(__INPUT u08_t         pin,
                                      __INPUT GPIODirection dir,
                                      __INPUT GPIOPull      pull)
{
    err_t error = Error::ERROR_OK;

    try {
        Array<u08_t> cmdBuf(4);

        cmdBuf.At(0) = CMD_WRITE_GPIO_CONFIG;
        cmdBuf.At(1) = pin;

        switch (dir) {
            case GPIO_DIR_IN    : cmdBuf.At(2) = 0; break;
            case GPIO_DIR_OUT   : cmdBuf.At(2) = 1; break;
            case GPIO_DIR_INOUT : cmdBuf.At(2) = 2; break;
        }

        switch (pull) {
            case GPIO_PULL_NONE : cmdBuf.At(3) = 0; break;
            case GPIO_PULL_UP   : cmdBuf.At(3) = 1; break;
            case GPIO_PULL_DOWN : cmdBuf.At(3) = 2; break;
            case GPIO_PULL_KEEP : cmdBuf.At(3) = 3; break;
        }

        error = TransmitCommand(cmdBuf.GetLength(), cmdBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        Array<u08_t> rspBuf(1);

        error = ReceiveResponse(1, rspBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        if (rspBuf.At(0) != RSP_SUCCESS) {
            throw Error::ERROR_RETURNED;
        }
    }
    catch (const err_t code) {
        PRINT_ERROR_CODE(error = code);
    }

    return error;
}

err_t
Board_Arduino_USBCDC::ReadGPIOData(__INPUT  u08_t pin,
                                   __OUTPUT bit_t *data)
{
    err_t error = Error::ERROR_OK;

    try {
        Array<u08_t> cmdBuf(2);
        
        cmdBuf.At(0) = CMD_READ_GPIO_DATA;
        cmdBuf.At(1) = pin;

        error = TransmitCommand(cmdBuf.GetLength(), cmdBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        Array<u08_t> rspBuf(2);

        error = ReceiveResponse(rspBuf.GetLength(), rspBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        if (rspBuf.At(0) != RSP_SUCCESS) {
            throw Error::ERROR_RETURNED;
        }

        *data = (bit_t) rspBuf.At(1);
    }
    catch (const err_t code) {
        PRINT_ERROR_CODE(error = code);
    }

    return error;
}

err_t
Board_Arduino_USBCDC::WriteGPIOData(__INPUT u08_t pin,
                                    __INPUT bit_t data)
{
    err_t error = Error::ERROR_OK;

    try {
        Array<u08_t> cmdBuf(3);

        cmdBuf.At(0) = CMD_WRITE_GPIO_DATA;
        cmdBuf.At(1) = pin;
        cmdBuf.At(2) = data;

        error = TransmitCommand(cmdBuf.GetLength(), cmdBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        Array<u08_t> rspBuf(1);

        error = ReceiveResponse(1, rspBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        if (rspBuf.At(0) != RSP_SUCCESS) {
            throw Error::ERROR_RETURNED;
        }
    }
    catch (const err_t code) {
        PRINT_ERROR_CODE(error = code);
    }

    return error;
}

err_t
Board_Arduino_USBCDC::GetFirmwareVersion(__INPUT  u32_t dataLen,
                                         __OUTPUT u08_t *data)
{
    err_t error = Error::ERROR_OK;

    try {
        if (data == NULL) {
            return Error::ERROR_INVALID_ARGUMENTS;
        }

        u08_t cmdBuf = CMD_READ_FIRMWARE_VERSION;

        error = TransmitCommand(1, &cmdBuf);
        if (Error::IsFailed(error)) {
            throw error;
        }

        Array<u08_t> rspBuf(21);

        error = ReceiveResponse(rspBuf.GetLength(), rspBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        if (rspBuf.At(0) != RSP_SUCCESS) {
            throw Error::ERROR_RETURNED;
        }

        u32_t retLen = MIN(dataLen, (rspBuf.GetLength() - 1));
        for (u32_t i = 0; i < retLen; i++) {
            data[i] = rspBuf.At(i + 1);
        }
    }
    catch (const err_t code) {
        PRINT_ERROR_CODE(error = code);
    }

    return error;
}

err_t
Board_Arduino_USBCDC::ReadADC(__INPUT  u08_t pin,
                              __OUTPUT u16_t *data)
{
    err_t error = Error::ERROR_OK;

    try {
        if (data == NULL) {
            return Error::ERROR_INVALID_ARGUMENTS;
        }

        Array<u08_t> cmdBuf(2);

        cmdBuf.At(0) = CMD_READ_ADC;
        cmdBuf.At(1) = pin;
       
        error = TransmitCommand(cmdBuf.GetLength(), cmdBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }
       
        Array<u08_t> rspBuf(3);
       
        error = ReceiveResponse(rspBuf.GetLength(), rspBuf.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }
       
        if (rspBuf.At(0) != RSP_SUCCESS) {
            throw Error::ERROR_RETURNED;
        }
       
        *data =  (((u16_t) rspBuf.At(1)) << 8) |
                 (((u16_t) rspBuf.At(2)) << 0);
    }
    catch (const err_t code) {
        PRINT_ERROR_CODE(error = code);
    }

    return error;
}


////////////////////////////////////////////////////////////////////////////////
//
// Transmitting Commands and Receiving Responses
//

err_t
Board_Arduino_USBCDC::TransmitCommand(__INPUT u32_t dataLen,
                                      __INPUT u08_t *data)
{
    err_t error = Error::ERROR_OK;

    try {
        Array<u08_t> packet(dataLen + m_PacketOverhead);

        error = PackCommand(dataLen, data, packet.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        error = m_SerialPort->Write(packet.GetLength(), packet.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        if (((u32_t) error) != packet.GetLength()) {
            throw Error::ERROR_LENGTH;
        }
    }
    catch (const err_t code) {
        PRINT_ERROR_CODE(error = code);
    }

    return error;
}

err_t
Board_Arduino_USBCDC::ReceiveResponse(__INPUT  u32_t dataLen,
                                      __OUTPUT u08_t *data)
{
    err_t error = Error::ERROR_OK;

    try {
        Array<u08_t> packet(dataLen + m_PacketOverhead);

        error = m_SerialPort->Read(packet.GetLength(), packet.GetData());
        if (Error::IsFailed(error)) {
            throw error;
        }

        if (((u32_t) error) != packet.GetLength()) {
            throw Error::ERROR_LENGTH;
        }

        error = UnpackResponse(packet.GetLength(), packet.GetData(), data);
        if (Error::IsFailed(error)) {
            throw error;
        }
    }
    catch (const err_t code) {
        PRINT_ERROR_CODE(error = code);
    }

    return error;
}
