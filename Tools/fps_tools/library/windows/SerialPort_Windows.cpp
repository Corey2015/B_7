#include "common.h"
#include "debug.h"
#include "Error.h"
#include "Device.h"
#include "SerialPort_Windows.h"


SerialPort_Windows::SerialPort_Windows(__INPUT Device &serialDev) :
    SerialPort_Base(serialDev),
    m_DevInfo(INVALID_HANDLE_VALUE),
    m_DevNum(0),
    m_GUID(NULL),
    m_DeviceHandle(INVALID_HANDLE_VALUE),
    m_Timeout(-1),
    m_RxBufSize(1024),
    m_TxBufSize(1024),
    m_Overlapped(FALSE),
    m_RxEvent(INVALID_HANDLE_VALUE),
    m_TxEvent(INVALID_HANDLE_VALUE),
    m_RxMutex(INVALID_HANDLE_VALUE),
    m_TxMutex(INVALID_HANDLE_VALUE)
{
}

SerialPort_Windows::~SerialPort_Windows()
{
    Close();
    Cleanup();
}

Device
SerialPort_Windows::SearchBegin()
{
    if (m_DevInfo != INVALID_HANDLE_VALUE) {
        Cleanup();
    }

    DWORD size = 0;
    SetupDiClassGuidsFromName("Ports", 0, 0, &size);
    if (size < 1) {
        return Error();
    }

    m_GUID = new GUID[size];
    if (m_GUID == NULL) {
        return Error();
    }

    if (SetupDiClassGuidsFromName("Ports",
                                  m_GUID,
                                  (size * sizeof(GUID)),
                                  &size) == FALSE) {
        return Error();
    }

    m_DevInfo = SetupDiGetClassDevs(m_GUID, NULL, NULL, DIGCF_PRESENT);
    if (m_DevInfo == INVALID_HANDLE_VALUE) {
        return Error();
    }

    return SearchNext();
}

Device
SerialPort_Windows::SearchNext()
{
    Device dev;

	memset(dev.Name, 0, sizeof(dev.Name));
	memset(dev.Path, 0, sizeof(dev.Path));

    if (m_DevInfo == INVALID_HANDLE_VALUE) {
        return SearchEnd();
    }

    SP_DEVINFO_DATA devData;
    TCHAR           comName[MAX_STRING_LENGTH] = {0};

    while (TRUE) {
        devData.cbSize = sizeof(SP_DEVINFO_DATA);
        if (SetupDiEnumDeviceInfo(m_DevInfo, m_DevNum, &devData) == FALSE) {
            return Error();
        }

        HKEY devKey = SetupDiOpenDevRegKey(m_DevInfo,
                                           &devData,
                                           DICS_FLAG_GLOBAL,
                                           0,
                                           DIREG_DEV,
                                           KEY_READ);
        if (devKey == NULL) {
            return Error();
        }

        DWORD length = sizeof(comName);
        int   retVal = RegQueryValueEx(devKey,
                                       "portname",
                                       NULL,
                                       NULL,
                                       (LPBYTE) comName,
                                       &length);
        RegCloseKey(devKey);
        if (retVal != ERROR_SUCCESS) {
            return Error();
        }

        m_DevNum++;

        if (strncmp("COM", comName, 3) == STRING_MATCHED) {
            break;
        }
    }

    DWORD length;
    SetupDiGetDeviceRegistryProperty(m_DevInfo,
                                     &devData,
                                     SPDRP_FRIENDLYNAME,
                                     NULL,
                                     NULL,
                                     0,
                                     &length);
    if (length > 0) {
        SetupDiGetDeviceRegistryProperty(m_DevInfo,
                                         &devData,
                                         SPDRP_FRIENDLYNAME,
                                         NULL,
                                         (LPBYTE) dev.Name,
                                         length,
                                         NULL);
        dev.Name[strlen(dev.Name)] = 0;
		sprintf(dev.Path, "\\\\.\\%s", comName);
    }

    return dev;
}

Device
SerialPort_Windows::SearchEnd()
{
	Device nullDev;

	memset(nullDev.Name, 0, sizeof(nullDev.Name));
	memset(nullDev.Path, 0, sizeof(nullDev.Path));

    return nullDev;
}

void
SerialPort_Windows::Cleanup()
{
    m_DevNum = 0;

    if (m_DevInfo != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(m_DevInfo);
        m_DevInfo = INVALID_HANDLE_VALUE;
    }

    if (m_GUID != NULL) {
        delete[] m_GUID;
        m_GUID = NULL;
    }
}

Device
SerialPort_Windows::Error()
{
    Cleanup();
    return SearchEnd();
}

err_t
SerialPort_Windows::Open(__INPUT bool_t    overlapped,
                         __INPUT u32_t     baudRate,
                         __INPUT u32_t     dataBits,
                         __INPUT ParityBit parity,
                         __INPUT StopBit   stopBits)
{
    err_t error = Error::ERROR_OK;

    try {
        if (m_RxEvent == INVALID_HANDLE_VALUE) {
            m_RxEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
            m_TxEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
            m_RxMutex = CreateMutex(NULL, FALSE, NULL);
            m_TxMutex = CreateMutex(NULL, FALSE, NULL);
        }

        if (m_DeviceHandle != INVALID_HANDLE_VALUE) {
            LOG_DEBUG("Device is already opended!\n");
            return Error::ERROR_OK;
        }

        m_Overlapped = overlapped;
        DWORD fileAttribute = (m_Overlapped == TRUE) ? FILE_FLAG_OVERLAPPED : FILE_ATTRIBUTE_NORMAL;

        m_DeviceHandle = CreateFile(m_Device.Path,                  // _In_     LPCTSTR               lpFileName
                                    (GENERIC_READ | GENERIC_WRITE), // _In_     DWORD                 dwDesiredAccess
                                    0,                              // _In_     DWORD                 dwShareMode
                                    NULL,                           // _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes
                                    OPEN_EXISTING,                  // _In_     DWORD                 dwCreationDisposition
                                    fileAttribute,                  // _In_     DWORD                 dwFlagsAndAttributes
                                    NULL);                          // _In_opt_ HANDLE                hTemplateFile
        if (m_DeviceHandle == INVALID_HANDLE_VALUE) {
            throw Error::ERROR_OPEN_FILE;
        }

        PurgeComm(m_DeviceHandle,
                  (PURGE_TXABORT |
                   PURGE_RXABORT |
                   PURGE_TXCLEAR |
                   PURGE_RXCLEAR));

        // Set COM Port Queue Size
        SetupComm(m_DeviceHandle, m_RxBufSize, m_TxBufSize);

        DCB dcbSerialParams;
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

        if (GetCommState(m_DeviceHandle, &dcbSerialParams) == FALSE) {
            throw Error::ERROR_CONFIG_FILE;
        }

        dcbSerialParams.BaudRate    = baudRate;
        dcbSerialParams.ByteSize    = dataBits;
        dcbSerialParams.fDtrControl = 1;

        switch (parity) {
            case PARITY_BIT_NONE : dcbSerialParams.Parity = NOPARITY;   break;
            case PARITY_BIT_ODD  : dcbSerialParams.Parity = ODDPARITY;  break;
            case PARITY_BIT_EVEN : dcbSerialParams.Parity = EVENPARITY; break;
            default              : throw Error::ERROR_INVALID_ARGUMENTS;
        }

        switch (stopBits) {
            case STOP_BIT_ONE      : dcbSerialParams.StopBits = ONESTOPBIT;   break;
            case STOP_BIT_ONE_HALF : dcbSerialParams.StopBits = ONE5STOPBITS; break;
            case STOP_BIT_TWO      : dcbSerialParams.StopBits = TWOSTOPBITS;  break;
            default                : throw Error::ERROR_INVALID_ARGUMENTS;
        }

        if (SetCommState(m_DeviceHandle, &dcbSerialParams) == FALSE) {
            throw Error::ERROR_CONFIG_FILE;
        }

        error = SetTimeout(1000);
        if (Error::IsFailed(error)) {
            throw error;
        }
    }
    catch (const err_t code) {
        LOG_DETAIL("%s\n", Error::DecodeError(error = code));

        if (m_DeviceHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_DeviceHandle);
        }
    }

    return error;
}

void
SerialPort_Windows::Close()
{
    if (m_DeviceHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_DeviceHandle);
        m_DeviceHandle = INVALID_HANDLE_VALUE;
    }

    if (m_RxEvent != INVALID_HANDLE_VALUE) {
        CloseHandle(m_RxEvent);
        m_RxEvent = INVALID_HANDLE_VALUE;
    }

    if (m_TxEvent != INVALID_HANDLE_VALUE) {
        CloseHandle(m_TxEvent);
        m_TxEvent = INVALID_HANDLE_VALUE;
    }

    if (m_RxMutex != INVALID_HANDLE_VALUE) {
        CloseHandle(m_RxMutex);
        m_RxMutex = INVALID_HANDLE_VALUE;
    }

    if (m_TxMutex != INVALID_HANDLE_VALUE) {
        CloseHandle(m_TxMutex);
        m_TxMutex = INVALID_HANDLE_VALUE;
    }
}

err_t
SerialPort_Windows::SetTimeout(__INPUT u32_t timeoutMsec)
{
    err_t error = Error::ERROR_OK;

    try {
        if (m_DeviceHandle == INVALID_HANDLE_VALUE) {
            throw Error::ERROR_OPEN_FILE;
        }

        COMMTIMEOUTS timeOut;

        timeOut.ReadIntervalTimeout        = MAXDWORD;
        timeOut.ReadTotalTimeoutConstant   = timeoutMsec;
        timeOut.ReadTotalTimeoutMultiplier = 0;

        timeOut.WriteTotalTimeoutConstant   = MAXDWORD;
        timeOut.WriteTotalTimeoutMultiplier = 0;

        if (SetCommTimeouts(m_DeviceHandle, &timeOut) == FALSE) {
            throw Error::ERROR_CONFIG_FILE;
        }
    }
    catch (const err_t code) {
        LOG_DETAIL("%s\n", Error::DecodeError(error = code));
    }

    return error;
}

err_t
SerialPort_Windows::SetQueueSize(__INPUT u32_t rxBufSize,
                                 __INPUT u32_t txBufSize)
{
    err_t error = Error::ERROR_OK;

    try {
        if (m_DeviceHandle == INVALID_HANDLE_VALUE) {
            throw Error::ERROR_OPEN_FILE;
        }

        if (SetupComm(m_DeviceHandle, rxBufSize, txBufSize) == FALSE) {
            throw Error::ERROR_CONFIG_FILE;
        }
    }
    catch (const err_t code) {
        LOG_DETAIL("%s\n", Error::DecodeError(error = code));
    }

    return error;
}

err_t
SerialPort_Windows::Read(__INPUT  u32_t size,
                         __OUTPUT u08_t *data)
{
    err_t error = Error::ERROR_OK;

    try {
        if (m_DeviceHandle == INVALID_HANDLE_VALUE) {
            throw Error::ERROR_OPEN_FILE;
        }

        DWORD retVal = WaitForSingleObject(m_RxMutex, m_Timeout);
        if (retVal == WAIT_TIMEOUT) {
            throw Error::ERROR_WAIT_TIMEOUT;
        }
        if (retVal != WAIT_OBJECT_0) {
            throw Error::ERROR_RETURNED;
        }

        DWORD totalSize  = 0;
        DWORD remainSize = size;
        BYTE  *dataPos   = data;
        DWORD bytesRead  = 0;

        while (remainSize > 0) {
            // Overlapped transfer
            if (m_Overlapped == TRUE) {
                ResetEvent(&m_RxEvent);

                OVERLAPPED ovlap;
                ZeroMemory(&ovlap, sizeof(ovlap));
                ovlap.hEvent = m_RxEvent;

                if (ReadFile(m_DeviceHandle,                // _In_        HANDLE       hFile
                             dataPos,                       // _Out_       LPVOID       lpBuffer
                             remainSize,                    // _In_        DWORD        nNumberOfBytesToRead
                             NULL,                          // _Out_opt_   LPDWORD      lpNumberOfBytesRead
                             &ovlap) == FALSE) {            // _Inout_opt_ LPOVERLAPPED lpOverlapped
                    if (GetLastError() != ERROR_IO_PENDING) {
                        throw Error::ERROR_READ_FILE;
                    }

                    DWORD retVal = WaitForSingleObject(m_RxEvent, m_Timeout);
                    if (retVal == WAIT_TIMEOUT) {
                        throw Error::ERROR_WAIT_TIMEOUT;
                    }
                    if (retVal != WAIT_OBJECT_0) {
                        throw Error::ERROR_RETURNED;
                    }
                }

                if (GetOverlappedResult(m_DeviceHandle,     // _In_  HANDLE       hFile,
                                        &ovlap,             // _In_  LPOVERLAPPED lpOverlapped,
                                        &bytesRead,         // _Out_ LPDWORD      lpNumberOfBytesTransferred,
                                        FALSE) == FALSE) {  // _In_  BOOL         bWait
                    throw Error::ERROR_RETURNED;
                }

            // Non-overlapped transfer
            } else {
                if (ReadFile(m_DeviceHandle,                // _In_        HANDLE       hFile
                             dataPos,                       // _Out_       LPVOID       lpBuffer
                             remainSize,                    // _In_        DWORD        nNumberOfBytesToRead
                             &bytesRead,                    // _Out_opt_   LPDWORD      lpNumberOfBytesRead
                             NULL) == FALSE) {              // _Inout_opt_ LPOVERLAPPED lpOverlapped
                    throw Error::ERROR_READ_FILE;
                }
            }

            if (bytesRead == 0) {
                break;
            }

            remainSize -= bytesRead;
            dataPos    += bytesRead;
            totalSize  += bytesRead;
        }

        PurgeComm(m_DeviceHandle,
                  (PURGE_TXABORT |
                   PURGE_RXABORT |
                   PURGE_TXCLEAR |
                   PURGE_RXCLEAR));

        error = (err_t) totalSize;
    }
    catch (const err_t code) {
        LOG_DETAIL("%s\n", Error::DecodeError(error = code));
    }

    return error;
}

err_t
SerialPort_Windows::Write(__INPUT u32_t size,
                          __INPUT u08_t *data)
{
    err_t error = Error::ERROR_OK;

    try {
        if (m_DeviceHandle == INVALID_HANDLE_VALUE) {
            throw Error::ERROR_OPEN_FILE;
        }

        DWORD retVal = WaitForSingleObject(m_TxMutex, m_Timeout);
        if (retVal == WAIT_TIMEOUT) {
            throw Error::ERROR_WAIT_TIMEOUT;
        }
        if (retVal != WAIT_OBJECT_0) {
            throw Error::ERROR_RETURNED;
        }

        DWORD errFlag;
        ClearCommError(m_DeviceHandle, &errFlag, NULL);

        DWORD totalSize    = 0;
        DWORD remainSize   = size;
        BYTE  *dataPos     = (BYTE *) data;
        DWORD bytesWritten = 0;

        while (remainSize > 0) {
            // Overlapped transfer
            if (m_Overlapped == TRUE) {
                ResetEvent(&m_TxEvent);
           
                OVERLAPPED ovlap;
                ZeroMemory(&ovlap, sizeof(ovlap));
                ovlap.hEvent = m_TxEvent;
           
                if (WriteFile(m_DeviceHandle,               // _In_        HANDLE       hFile,
                              data,                         // _In_        LPCVOID      lpBuffer,
                              size,                         // _In_        DWORD        nNumberOfBytesToWrite,
                              NULL,                         // _Out_opt_   LPDWORD      lpNumberOfBytesWritten,
                              &ovlap) == FALSE) {           // _Inout_opt_ LPOVERLAPPED lpOverlapped
                    if (GetLastError() != ERROR_IO_PENDING) {
                        throw Error::ERROR_WRITE_FILE;
                    }
           
                    DWORD retVal = WaitForSingleObject(m_TxEvent, m_Timeout);
                    if (retVal == WAIT_TIMEOUT) {
                        throw Error::ERROR_WAIT_TIMEOUT;
                    }
                    if (retVal != WAIT_OBJECT_0) {
                        throw Error::ERROR_RETURNED;
                    }
                }
           
                if (GetOverlappedResult(m_DeviceHandle,     // _In_  HANDLE       hFile,
                                        &ovlap,             // _In_  LPOVERLAPPED lpOverlapped,
                                        &bytesWritten,      // _Out_ LPDWORD      lpNumberOfBytesTransferred,
                                        FALSE) == FALSE) {  // _In_  BOOL         bWait
                    throw Error::ERROR_RETURNED;
                }
           
            // Non-overlapped transfer
            } else {
                if (WriteFile(m_DeviceHandle,               // _In_        HANDLE       hFile,
                              data,                         // _In_        LPCVOID      lpBuffer,
                              size,                         // _In_        DWORD        nNumberOfBytesToWrite,
                              &bytesWritten,                // _Out_opt_   LPDWORD      lpNumberOfBytesWritten,
                              NULL) == FALSE) {             // _Inout_opt_ LPOVERLAPPED lpOverlapped
                    throw Error::ERROR_WRITE_FILE;
                }
            }

            if (bytesWritten == 0) {
                break;
            }

            remainSize -= bytesWritten;
            dataPos    += bytesWritten;
            totalSize  += bytesWritten;
        }

        error = (err_t) totalSize;
    }
    catch (const err_t code) {
        LOG_DETAIL("%s\n", Error::DecodeError(error = code));
    }

    return error;
}

err_t
SerialPort_Windows::GetChar(__OUTPUT u08_t *c)
{
    err_t error = Error::ERROR_OK;

    try {
        if (m_DeviceHandle == INVALID_HANDLE_VALUE) {
            throw Error::ERROR_OPEN_FILE;
        }

        u08_t value;

        error = Read(1, &value);
        if (Error::IsFailed(error)) {
            throw error;
        }

        *c = value;
        error = (err_t) value;
    }
    catch (const err_t code) {
        LOG_DETAIL("%s\n", Error::DecodeError(error = code));
    }

    return error;
}

err_t
SerialPort_Windows::PutChar(__INPUT u08_t c)
{
    err_t error = Error::ERROR_OK;

    try {
        if (m_DeviceHandle == INVALID_HANDLE_VALUE) {
            throw Error::ERROR_OPEN_FILE;
        }

        u08_t value = c;

        error = Write(1, &value);
        if (Error::IsFailed(error)) {
            throw error;
        }
    }
    catch (const err_t code) {
        LOG_DETAIL("%s\n", Error::DecodeError(error = code));
    }

    return error;
}

void
SerialPort_Windows::Flush()
{
    Sleep(1);
}
