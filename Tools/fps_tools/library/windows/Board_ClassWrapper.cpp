#include "common.h"
#include "Device.h"
#include "Error.h"
#include "SerialPort.h"
#include "Board.h"
#include "Board_ClassWrapper.h"


board_t
board_open(char *file,
		   int  pin_ldo_en,
		   int  pin_rst_n,
		   int  pin_intr,
		   int  pin_spi_csb)
{
    try {
        Device commDev;

		strncpy(commDev.Path, file, MAX_STRING_LENGTH);

		Board *board_ptr = new Board(commDev,
			                         pin_ldo_en,
									 pin_rst_n,
		                             pin_intr,
									 pin_spi_csb);
		if (board_ptr == NULL) {
            throw Error::ERROR_MEMORY_ALLOC;
        }

        err_t error = board_ptr->Open();
        if (Error::IsFailed(error)) {
            throw error;
        }

        return static_cast<board_t>(board_ptr);
    }
    catch (const err_t code) {
        PRINT_ERROR_CODE(code);
        return NULL;
    }
}

void
board_close(board_t *board)
{
    try {
        Board *board_ptr = static_cast<Board *>(*board);

        err_t error = board_ptr->Close();
        if (Error::IsFailed(error)) {
            throw error;
        }

        delete board_ptr;
        *board = NULL;
    }
    catch (const err_t code) {
        PRINT_ERROR_CODE(code);
    }
}

int
board_set_timeout(board_t board,
                  int     timeout_ms)
{
    Board *board_ptr = static_cast<Board *>(board);

    err_t error = board_ptr->SetTimeout((u32_t) timeout_ms);
    if (Error::IsFailed(error)) {
        return -1;
    }

    return 0;
}

int
board_set_queue_size(board_t board,
                     int     rx_buf_size,
                     int     tx_buf_size)
{
    Board *board_ptr = static_cast<Board *>(board);

    err_t error = board_ptr->SetQueueSize((u32_t) rx_buf_size,
                                          (u32_t) tx_buf_size);
    if (Error::IsFailed(error)) {
        return -1;
    }

    return 0;
}

int
board_read_spi_config(board_t board,
                      int     *mode,
                      int     *speed,
                      int     *delay,
                      int     *bit_seq)
{
    Board *board_ptr = static_cast<Board *>(board);

    err_t error = board_ptr->ReadSPIConfig((Board::SPIMode *)     mode,
                                           (u32_t *)              speed,
                                           (u32_t *)              delay,
                                           (Board::BitSequence *) bit_seq);
    if (Error::IsFailed(error)) {
        return -1;
    }

    return 0;
}

int
board_write_spi_config(board_t board,
                       int     mode,
                       int     speed,
                       int     delay,
                       int     bit_seq)
{
    Board *board_ptr = static_cast<Board *>(board);

    err_t error = board_ptr->WriteSPIConfig((Board::SPIMode)     mode,
                                            (u32_t)              speed,
                                            (u32_t)              delay,
                                            (Board::BitSequence) bit_seq);
    if (Error::IsFailed(error)) {
        return -1;
    }

    return 0;
}

int
board_do_spi_transfer(board_t board,
                      int     tx_len,
                      uint8_t *tx_data,
                      int     *rx_len,
                      uint8_t *rx_data)
{
    Board *board_ptr = static_cast<Board *>(board);

    err_t error = board_ptr->DoSPITransfer((u32_t)   tx_len,
                                           (u08_t *) tx_data,
                                           (u32_t *) rx_len,
                                           (u08_t *) rx_data);
    if (Error::IsFailed(error)) {
        return -1;
    }

    return 0;
}

int
board_read_gpio_config(board_t board,
                       int     pin,
                       int     *dir,
                       int     *pull)
{
    Board *board_ptr = static_cast<Board *>(board);

    err_t error = board_ptr->ReadGPIOConfig((u08_t)                  pin,
		                                    (Board::GPIODirection *) dir,
                                            (Board::GPIOPull *)      pull);
    if (Error::IsFailed(error)) {
        return -1;
    }

    return 0;
}

int
board_write_gpio_config(board_t board,
                        int     pin,
                        int     dir,
                        int     pull)
{
    Board *board_ptr = static_cast<Board *>(board);

    err_t error = board_ptr->WriteGPIOConfig((u08_t)                pin,
                                             (Board::GPIODirection) dir,
                                             (Board::GPIOPull)      pull);
    if (Error::IsFailed(error)) {
        return -1;
    }

    return 0;
}

int
board_read_gpio_data(board_t board,
                     int     pin,
                     int     *data)
{
    Board *board_ptr = static_cast<Board *>(board);

    err_t error = board_ptr->ReadGPIOData((u08_t)   pin,
                                          (bit_t *) data);
    if (Error::IsFailed(error)) {
        return -1;
    }

    return 0;
}

int
board_write_gpio_data(board_t board,
                      int     pin,
                      int     data)
{
    Board *board_ptr = static_cast<Board *>(board);

    err_t error = board_ptr->WriteGPIOData((u08_t) pin,
                                           (bit_t) data);
    if (Error::IsFailed(error)) {
        return -1;
    }

    return 0;
}

int
board_get_firmware_version(board_t board,
                           int     data_len,
                           uint8_t *data)
{
    Board *board_ptr = static_cast<Board *>(board);

    err_t error = board_ptr->GetFirmwareVersion((u32_t)   data_len,
                                                (u08_t *) data);
    if (Error::IsFailed(error)) {
        return -1;
    }

    return 0;
}

int
board_read_adc_data(board_t board,
                    int     pin,
                    int     *data)
{
    Board *board_ptr = static_cast<Board *>(board);

    err_t error = board_ptr->ReadADC((u08_t)   pin,
                                     (u16_t *) data);
    if (Error::IsFailed(error)) {
        return -1;
    }

    return 0;
}

int
board_transmit_command(board_t board,
                       int     data_len,
                       uint8_t *data)
{
    Board *board_ptr = static_cast<Board *>(board);

    err_t error = board_ptr->TransmitCommand((u32_t)   data_len,
                                             (u08_t *) data);
    if (Error::IsFailed(error)) {
        return -1;
    }

    return 0;
}

int
board_receive_response(board_t board,
                       int     data_len,
                       uint8_t *data)
{
    Board *board_ptr = static_cast<Board *>(board);

    err_t error = board_ptr->ReceiveResponse((u32_t)   data_len,
                                             (u08_t *) data);
    if (Error::IsFailed(error)) {
        return -1;
    }

    return 0;
}
