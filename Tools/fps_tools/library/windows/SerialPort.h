#ifndef __SerialPort_h__
#define __SerialPort_h__


#include "common.h"
#include "Error.h"
#include "Device.h"


class SerialPort_Base {

public:

    enum ParityBit {
        PARITY_BIT_NONE = 0,
        PARITY_BIT_ODD  = 1,
        PARITY_BIT_EVEN = 2
    };

    enum StopBit {
        STOP_BIT_ONE      = 0,
        STOP_BIT_ONE_HALF = 1,
        STOP_BIT_TWO      = 2
    };

	typedef SerialPort_Base* PortPtr;

    SerialPort_Base(__INPUT Device &serialDev) :
        m_Device(serialDev) {}
    virtual ~SerialPort_Base() {}

    // Search a serial port
    virtual Device SearchBegin() = 0;
    virtual Device SearchNext()  = 0;
    virtual Device SearchEnd()   = 0;

    // Open a serial port
    virtual err_t Open(__INPUT bool_t    overlapped = FALSE,
                       __INPUT u32_t     baudRate   = 115200,
                       __INPUT u32_t     dataBits   = 8,
                       __INPUT ParityBit parity     = PARITY_BIT_NONE,
                       __INPUT StopBit   stopBits   = STOP_BIT_ONE) = 0;

    // Close serial port
    virtual void Close() = 0;

    // Set communication time-out
    virtual err_t SetTimeout(__INPUT u32_t timeoutMsec) = 0;

    // Set serial port queue size
    virtual err_t SetQueueSize(__INPUT u32_t rxBufSize,
                               __INPUT u32_t txBufSize) = 0;

    // Read data from the serial port
    virtual err_t Read(__INPUT  u32_t size,
                       __OUTPUT u08_t *data) = 0;

    // Write data to the serial port
    virtual err_t Write(__INPUT u32_t size,
                        __INPUT u08_t *data) = 0;

    // Get one character
    virtual err_t GetChar(__OUTPUT u08_t *c) = 0;

    // Put one character
    virtual err_t PutChar(__INPUT u08_t c) = 0;

    // Flush the buffer
    virtual void Flush() = 0;

    // Get the device
    virtual PortPtr GetDevice() {
        return this;
    }

protected:

    Device m_Device;

};

#if defined(__WINDOWS__)
    #include "SerialPort_Windows.h"
    class SerialPort_Windows;
    typedef SerialPort_Windows SerialPort;
#else
    #error "[ ERROR ] Platform is not supported!"
#endif


#endif // __SerialPort_h__
