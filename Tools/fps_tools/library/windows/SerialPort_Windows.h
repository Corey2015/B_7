#ifndef __SerialPort_Windows_h__
#define __SerialPort_Windows_h__


#include <setupapi.h>
#include "common.h"
#include "Device.h"
#include "SerialPort.h"


class SerialPort_Windows : public SerialPort_Base {

public:

    SerialPort_Windows(__INPUT Device &serialDev);
    virtual ~SerialPort_Windows();

    // Search a serial port
    Device SearchBegin();
    Device SearchNext();
    Device SearchEnd();

    // Open a serial port
    err_t Open(__INPUT bool_t    overlapped = FALSE,
               __INPUT u32_t     baudRate   = 115200,
               __INPUT u32_t     dataBits   = 8,
               __INPUT ParityBit parity     = PARITY_BIT_NONE,
               __INPUT StopBit   stopBits   = STOP_BIT_ONE);

    // Close a serial port
    void Close();

    // Set communication time-out
    err_t SetTimeout(__INPUT u32_t timeoutMsec);

    // Set serial port queue size
    err_t SetQueueSize(__INPUT u32_t rxBufSize,
                       __INPUT u32_t txBufSize);

    // Read data from the serial port
    err_t Read(__INPUT  u32_t size,
               __OUTPUT u08_t *data);

    // Write data to the serial port
    err_t Write(__INPUT u32_t size,
                __INPUT u08_t *data);

    // Get one character
    err_t GetChar(__OUTPUT u08_t *c);

    // Put one character
    err_t PutChar(__INPUT u08_t c);

    // Flush the buffer
    void Flush();

private:

    // For searching a serial port
    HDEVINFO m_DevInfo;
    u32_t    m_DevNum;
    GUID     *m_GUID;

    void   Cleanup();
    Device Error();

    // Device handle after the device file created
    HANDLE m_DeviceHandle;

    // Timeout
    s32_t m_Timeout;

    // Queue size
    u32_t m_RxBufSize;
    u32_t m_TxBufSize;

    // For overlapped access
	bool_t m_Overlapped;
    HANDLE m_RxEvent;
    HANDLE m_TxEvent;

    // Synchronization objects
    HANDLE m_RxMutex;
    HANDLE m_TxMutex;

};


#endif // __SerialPort_Windows_h__
