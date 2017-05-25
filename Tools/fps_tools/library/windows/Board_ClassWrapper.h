#ifndef __Board_ClassWrapper_h__
#define __Board_ClassWrapper_h__


#include "common.h"


#if defined(__cplusplus)
extern "C" {
#endif

enum DefaultPinList {
    PIN_LDO_EN  = 5,  // Pin # to turn on LDO
    PIN_RSTN    = 4,  // Pin # to sensor reset
    PIN_INTR    = 7,  // Pin # to sensor interrupt
    PIN_SPI_CSB = 10, // Pin # to sensor SPI chip-select
};

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

typedef void* board_t;

// Open a board
board_t board_open(char *file,
		           int  pin_ldo_en,
		           int  pin_rst_n,
		           int  pin_intr,
		           int  pin_spi_csb);

// Close a board
void board_close(board_t *board);

// Set communication time-out
int board_set_timeout(board_t board,
                      int     timeout_ms);

// Set communication queue size
int board_set_queue_size(board_t board,
                         int     rx_buf_size,
                         int     tx_buf_size);

// Read configuration of a SPI port
int board_read_spi_config(board_t board,
                          int     *mode,
                          int     *speed,
                          int     *delay,
                          int     *bit_seq);

// Write configuration to a SPI port
int board_write_spi_config(board_t board,
                           int     mode,
                           int     speed,
                           int     delay,
                           int     bit_seq);

// Do a low-level SPI transfer
int board_do_spi_transfer(board_t board,
                          int     tx_len,
                          uint8_t *tx_data,
                          int     *rx_len,
                          uint8_t *rx_data);

// Read configuration of a GPIO pin
int board_read_gpio_config(board_t board,
                           int     pin,
                           int     *dir,
                           int     *pull);

// Write configuration to a GPIO pin
int board_write_gpio_config(board_t board,
                            int     pin,
                            int     dir,
                            int     pull);

// Read data from a GPIO pin
int board_read_gpio_data(board_t board,
                         int     pin,
                         int     *data);

// Write data to a GPIO pin
int board_write_gpio_data(board_t board,
                          int     pin,
                          int     data);

// Get firmware version
int board_get_firmware_version(board_t board,
                               int     data_len,
                               uint8_t *data);

// Read ADC value
int board_read_adc_data(board_t board,
                        int     pin,
                        int     *data);

// Transmit commands to the board
int board_transmit_command(board_t board,
                           int     data_len,
                           uint8_t *data);

// Receive responses from the board
int board_receive_response(board_t board,
                           int     data_len,
                           uint8_t *data);


#if defined(__cplusplus)
}
#endif


#endif // __Board_ClassWrapper_h__
