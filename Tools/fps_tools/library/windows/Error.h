#ifndef __Error_h__
#define __Error_h__


#include "common.h"
#include "Debug.h"


class Error {

public:

    enum ErrorBase {
        ERROR_BASE_GENERAL  = 0,
        ERROR_BASE_FILE     = -10,
        ERROR_BASE_PROTOCOL = -20,
        ERROR_BASE_SENSOR   = -30,
        ERROR_BASE_CUSTOM   = -100,
    };

    enum ErrorCode {
        ERROR_OK                = ERROR_BASE_GENERAL - 0,
        ERROR_UNKNOWN           = ERROR_BASE_GENERAL - 1,
        ERROR_NOT_IMPLEMENTED   = ERROR_BASE_GENERAL - 2,
        ERROR_INVALID_ARGUMENTS = ERROR_BASE_GENERAL - 3,
        ERROR_WAIT_TIMEOUT      = ERROR_BASE_GENERAL - 4,
        ERROR_MEMORY_ALLOC      = ERROR_BASE_GENERAL - 5,
        ERROR_RETURNED          = ERROR_BASE_GENERAL - 6,
		ERROR_ABORTED           = ERROR_BASE_GENERAL - 7,

        ERROR_OPEN_FILE         = ERROR_BASE_FILE - 0,
        ERROR_CLOSE_FILE        = ERROR_BASE_FILE - 1,
        ERROR_CONFIG_FILE       = ERROR_BASE_FILE - 2,
        ERROR_READ_FILE         = ERROR_BASE_FILE - 3,
        ERROR_WRITE_FILE        = ERROR_BASE_FILE - 4,
                               
        ERROR_INVALID_PACKET    = ERROR_BASE_PROTOCOL - 0,
        ERROR_UNKNOWN_COMMAND   = ERROR_BASE_PROTOCOL - 1,
        ERROR_UNKNOWN_RESPONSE  = ERROR_BASE_PROTOCOL - 2,
        ERROR_CHECKSUM          = ERROR_BASE_PROTOCOL - 3,
        ERROR_LENGTH            = ERROR_BASE_PROTOCOL - 4,

        ERROR_NO_MORE_SETTINGS  = ERROR_BASE_SENSOR - 0,
        ERROR_BAD_QUALITY       = ERROR_BASE_SENSOR - 1,
    };

    static inline bool_t IsPassed(__INPUT ErrorCode error)
    {
        return (error >= ERROR_OK);
    }

    static inline bool_t IsFailed(__INPUT ErrorCode error)
    {
        return (error < ERROR_OK);
    }

    static char_t* const DecodeError(__INPUT ErrorCode error)
    {
        switch (error) {
            case ERROR_OK                : return "Success!";
            case ERROR_UNKNOWN           : return "Unknown error(s)!";
            case ERROR_NOT_IMPLEMENTED   : return "Request(s) not implemented!";
            case ERROR_INVALID_ARGUMENTS : return "Invalid argument(s)!";
            case ERROR_WAIT_TIMEOUT      : return "Execution timeout!";
            case ERROR_MEMORY_ALLOC      : return "Memory allocation failed!";
            case ERROR_RETURNED          : return "Error response(s) returned!";
			case ERROR_ABORTED           : return "Aborted by caller!";
                                        
            case ERROR_OPEN_FILE         : return "Opening a file failed!";
            case ERROR_CLOSE_FILE        : return "Closing a file failed!";
            case ERROR_CONFIG_FILE       : return "Configuring a file failed!";
            case ERROR_READ_FILE         : return "Reading a file failed!";
            case ERROR_WRITE_FILE        : return "Writing a file failed!";
                                        
            case ERROR_INVALID_PACKET    : return "Invalid packet(s)!";
            case ERROR_UNKNOWN_COMMAND   : return "Unknown command(s)!";
            case ERROR_UNKNOWN_RESPONSE  : return "Unknown response(s)!";
            case ERROR_CHECKSUM          : return "Comparing checksum failed!";
            case ERROR_LENGTH            : return "Packet length error!";

            case ERROR_NO_MORE_SETTINGS  : return "No more settings!";
            case ERROR_BAD_QUALITY       : return "Bad sensor quality!";
        }

        return 0;
    }
};

typedef Error::ErrorCode err_t;

#if defined(__DEBUG__)
    #define PRINT_ERROR_CODE(_x_) LOG_ERROR("%s\n", Error::DecodeError((_x_)));
#else
    #define PRINT_ERROR_CODE(_x_) (_x_)
#endif


#endif // __Error_h__
