#ifndef __Protocol_h__
#define __Protocol_h__


#include "common.h"
#include "debug.h"
#include "Error.h"


class Protocol {

public:

    enum PacketHeader {
        ASCII_STX = 0x02,
        ASCII_ETX = 0x03,
    };

    enum PacketCommand {
        CMD_READ_REG                  = 0x02,         
        CMD_WRITE_REG                 = 0x03,
        CMD_BURST_READ_REG            = 0x04,
        CMD_BURST_WRITE_REG           = 0x05,
        CMD_READ_FINE_TUNE_REG        = 0x06,
        CMD_WRITE_FINE_TUNE_REG       = 0x07,
        CMD_BURST_READ_FINE_TUNE_REG  = 0x08,
        CMD_BURST_WRITE_FINE_TUNE_REG = 0x09,
        CMD_READ_IMAGE                = 0x0A,
        CMD_READ_INTR_CNT             = 0x10,
        CMD_READ_INTR_CONFIG          = 0x12,
        CMD_WRITE_INTR_CONFIG         = 0x13,
        CMD_DETACH_SENSOR_INTR        = 0x14,
        CMD_ATTACH_SENSOR_INTR        = 0x15,
        CMD_READ_REF_VOLTAGE          = 0xB0,
        CMD_WRITE_REF_VOLTAGE         = 0xB1,
        CMD_READ_ADC                  = 0xB2,
        CMD_WRITE_DAC                 = 0xB3,
        CMD_READ_SPI_CS               = 0xD0,
        CMD_WRITE_SPI_CS              = 0xD1,
        CMD_DO_SPI_TRANSFER           = 0xD2,
        CMD_READ_SPI_CONFIG           = 0xD4,
        CMD_WRITE_SPI_CONFIG          = 0xD5,
        CMD_READ_GPIO_DATA            = 0xE0,
        CMD_WRITE_GPIO_DATA           = 0xE1,
        CMD_READ_GPIO_CONFIG          = 0xE2,
        CMD_WRITE_GPIO_CONFIG         = 0xE3,
        CMD_READ_FIRMWARE_VERSION     = 0xF0,
        CMD_READ_BOARD_ID             = 0xF2,
        CMD_READ_TEST_SITE_NUM        = 0xF4,
        CMD_WRITE_TEST_SITE_NUM       = 0xF5,
        CMD_READ_FLASH_STATUS         = 0xF6,
    };

    enum PacketResponse {
        RSP_SUCCESS         = 0x00,
        RSP_INVALID_PACKET  = 0x01,
        RSP_CHECKSUM_FAILED = 0x02,
        RSP_INVALID_COMMAND = 0x03,
        RSP_UNKNWON_ERROR   = 0xFF,
    };

    Protocol(__INPUT u32_t overhead = 3) : m_PacketOverhead(overhead) {}
    virtual ~Protocol() {}

protected:

    const u32_t m_PacketOverhead;

    // Pack a command packet
    err_t PackCommand(__INPUT  u32_t dataLen,
                      __INPUT  u08_t *data,
                      __OUTPUT u08_t *packet)
    {
        err_t error = Error::ERROR_OK;

        try {
            if ((data == NULL) || (packet == NULL) || (dataLen == 0)) {
                throw Error::ERROR_INVALID_ARGUMENTS;
            }

            packet[0] = ASCII_STX;

            u08_t chkSum = 0;
            for (u32_t i = 0; i < dataLen; i++) {
                packet[i + 1]  = data[i];
                chkSum        += data[i];
            }

            packet[++i] = chkSum;
            packet[++i] = ASCII_ETX;
        }
        catch (const err_t code) {
            PRINT_ERROR_CODE(error = code);
        }

        return error;
    }

    // Unpack a response packet
    err_t UnpackResponse(__INPUT  u32_t packetLen,
                         __INPUT  u08_t *packet,
                         __OUTPUT u08_t *data)
    {
        err_t error = Error::ERROR_OK;

        try {
            if ((data == NULL)  || (packet == NULL) || (packetLen <= m_PacketOverhead)) {
                throw Error::ERROR_INVALID_ARGUMENTS;
            }

            if ((packet[0] != ASCII_STX) || (packet[packetLen - 1] != ASCII_ETX)) {
                throw Error::ERROR_INVALID_PACKET;
            }

            u08_t chkSum = 0;
            for (u32_t i = 0; i < (packetLen - m_PacketOverhead); i++) {
                data[i]  = packet[i + 1];
                chkSum  += packet[i + 1];
            }

            if (chkSum != packet[++i]) {
                throw Error::ERROR_CHECKSUM;
            }
        }
        catch (const err_t code) {
            PRINT_ERROR_CODE(error = code);
        }

        return error;
    }

};


#endif // __Protocol_h__
