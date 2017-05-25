#include <stdlib.h>
#include "common.h"
#include "debug.h"
#include "fps.h"
#include "fps_control.h"
#include "fps_register.h"
#include "Board_ClassWrapper.h"


#define BOARD_HANDLE ((board_t) handle->fd)


////////////////////////////////////////////////////////////////////////////////
//
// Sensor Open/Close/Reset/Init
//

fps_handle_t*
fps_open_sensor(char *file)
{
    int     status = 0;
    board_t board;

    board = board_open(file,
                       PIN_LDO_EN,
                       PIN_RSTN,
                       PIN_INTR,
                       PIN_SPI_CSB);
    if (board == NULL) {
        return NULL;
    }

    // Configure LDO pin
    status = board_write_gpio_config(board,
                                     PIN_LDO_EN,
                                     GPIO_DIR_OUT,
                                     GPIO_PULL_NONE);
    if (status < 0) {
        return NULL;
    }

    status = board_write_gpio_data(board,
                                   PIN_LDO_EN,
                                   LOW);
    if (status < 0) {
        return NULL;
    }
    fps_sleep(50.0 * 1000);

    status = board_write_gpio_data(board,
                                   PIN_LDO_EN,
                                   HIGH);
    if (status < 0) {
        return NULL;
    }
    fps_sleep(50.0 * 1000);

    // Configure reset pin
    status = board_write_gpio_config(board,
                                     PIN_RSTN,
                                     GPIO_DIR_OUT,
                                     GPIO_PULL_NONE);
    if (status < 0) {
        return NULL;
    }

    status = board_write_gpio_data(board,
                                   PIN_RSTN,
                                   LOW);
    if (status < 0) {
        return NULL;
    }
    fps_sleep(50.0 * 1000);

    status = board_write_gpio_data(board,
                                   PIN_RSTN,
                                   HIGH);
    if (status < 0) {
        return NULL;
    }
    fps_sleep(50.0 * 1000);

    // Configure interrupt pin
    status = board_write_gpio_config(board,
                                     PIN_INTR,
                                     GPIO_DIR_IN,
                                     GPIO_PULL_UP);
    if (status < 0) {
        return NULL;
    }

    // Configure SPI
    status = board_write_spi_config(board,
                                    SPI_MODE_3,
                                    8 * 1000 * 1000,
                                    0,
                                    BIT_SEQ_MSB_FIRST);
    if (status < 0) {
        return NULL;
    }

    // Create a handle to attach this sensor
    return fps_attach_sensor((int) board);
}

int
fps_close_sensor(fps_handle_t **handle)
{
	int status = 0;

	if (*handle != NULL) {
		board_close((board_t *) &((*handle)->fd));
		status = fps_detach_sensor(handle);
	}

	return status;
}

int
fps_reset_sensor(fps_handle_t *handle,
                 int          state)
{
    int status = 0;

    status = board_write_gpio_data(BOARD_HANDLE,
                                   PIN_RSTN,
                                   (state == 1) ? HIGH : LOW);

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Chip ID
//

int
fps_get_chip_id(fps_handle_t *handle,
				int          *chip_id)
{
	if (chip_id == NULL) {
		return -1;
	}

    return 0;
}


////////////////////////////////////////////////////////////////////////////////
//
// Set Sensor Speed
//

int
fps_set_sensor_speed(fps_handle_t *handle,
                     int          speed_hz)
{
    int status = 0;
    int mode;
    int speed_old;
    int delay;
    int bit_seq;

    status = board_read_spi_config(BOARD_HANDLE,
                                   &mode,
                                   &speed_old,
                                   &delay,
                                   &bit_seq);
    if (status < 0) {
        return status;
    }

    status = board_write_spi_config(BOARD_HANDLE,
                                    mode,
                                    speed_hz,
                                    delay,
                                    bit_seq);

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Register Access
//

int
fps_multiple_read(fps_handle_t *handle,
                  uint8_t      *addr,
                  uint8_t      *data,
                  size_t       length)
{
    int     status = 0;
    uint8_t *cmd_buf;
    uint8_t *rsp_buf;
    size_t  i;

    if ((handle == NULL) || (length == 0) || (addr == NULL) || (data == NULL)) {
        return -1;
    }

    cmd_buf = (uint8_t *) malloc(sizeof(uint8_t) * (length + 3));
    if (cmd_buf == NULL) {
        goto fps_multiple_read_end;
    }

    cmd_buf[0] = CMD_READ_REG;
    cmd_buf[1] = PIN_SPI_CSB;
    cmd_buf[2] = (uint8_t) length;

    for (i = 0; i < length; i++) {
        cmd_buf[i + 3] = (uint8_t) addr[i];
    }

    status = board_transmit_command(BOARD_HANDLE, (length + 3), cmd_buf);
    if (status < 0) {
        goto fps_multiple_read_end;
    }

    rsp_buf = (uint8_t *) malloc(sizeof(uint8_t) * (length + 1));
    if (rsp_buf == NULL) {
        goto fps_multiple_read_end;
    }

    status = board_receive_response(BOARD_HANDLE, (length + 1), rsp_buf);
    if (status < 0) {
        goto fps_multiple_read_end;
    }

    if (rsp_buf[0] != RSP_SUCCESS) {
        goto fps_multiple_read_end;
    }

    for (i = 0; i < length; i++) {
        data[i] = (uint8_t) rsp_buf[i + 1];
    }

fps_multiple_read_end :

    if (cmd_buf != NULL) {
        free(cmd_buf);
    }

    if (rsp_buf != NULL) {
        free(rsp_buf);
    }

    return status;
}

int
fps_multiple_write(fps_handle_t *handle,
                   uint8_t      *addr,
                   uint8_t      *data,
                   size_t       length)
{
    int     status = 0;
    uint8_t *cmd_buf;
    uint8_t rsp;
    size_t  i;

    if ((handle == NULL) || (length == 0) || (addr == NULL) || (data == NULL)) {
        return -1;
    }

    cmd_buf = (uint8_t *) malloc(sizeof(uint8_t) * ((length + 1) * 2 + 3));
    if (cmd_buf == NULL) {
        goto fps_multiple_write_end;
    }

    cmd_buf[0] = CMD_WRITE_REG;
    cmd_buf[1] = PIN_SPI_CSB;
    cmd_buf[2] = (uint8_t) (length + 1);

    for (i = 0; i < length; i++) {
        cmd_buf[(i * 2) + 3] = (uint8_t) addr[i];
        cmd_buf[(i * 2) + 4] = (uint8_t) data[i];
    }
	cmd_buf[(i * 2) + 3] = 0xFF;
	cmd_buf[(i * 2) + 4] = 0xFF;
	
    status = board_transmit_command(BOARD_HANDLE, ((length + 1) * 2 + 3), cmd_buf);
    if (status < 0) {
        goto fps_multiple_write_end;
    }

    status = board_receive_response(BOARD_HANDLE, 1, &rsp);
    if (status < 0) {
        goto fps_multiple_write_end;
    }

    if (rsp != RSP_SUCCESS) {
        goto fps_multiple_write_end;
    }

fps_multiple_write_end :

    if (cmd_buf != NULL) {
        free(cmd_buf);
    }

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Image Mode Operations
//

int
fps_get_raw_image(fps_handle_t *handle,
                  int          img_width,
                  int          img_height,
                  uint8_t      *img_buf)
{
    int     status = 0;
	size_t  img_size;
    size_t  length;
    uint8_t cmd_buf[4];
    uint8_t *rsp_buf;
    size_t  i;

	img_size = img_width * img_height;
    length   = img_size + handle->latency;
    if (length <= 0) {
        return -1;
    }

    cmd_buf[0] = CMD_READ_IMAGE;
    cmd_buf[1] = PIN_SPI_CSB;
    cmd_buf[2] = (uint8_t) ((length & 0xFF00) >> 8);
    cmd_buf[3] = (uint8_t) ((length & 0x00FF) >> 0);

    status = board_transmit_command(BOARD_HANDLE, 4, cmd_buf);
    if (status < 0) {
        return status;
    }

    rsp_buf = (uint8_t *) malloc(sizeof(uint8_t) * (length + 1));
    if (rsp_buf == NULL) {
        return -1;
    }

    status = board_receive_response(BOARD_HANDLE, (length + 1), rsp_buf);
    if (status < 0) {
        goto fps_get_raw_image_end;
    }

    if (rsp_buf[0] != RSP_SUCCESS) {
        goto fps_get_raw_image_end;
    }

    for (i = 0; i < img_size; i++) {
        img_buf[i] = rsp_buf[i + 1];
    }

fps_get_raw_image_end:

    if (rsp_buf != NULL) {
        free(rsp_buf);
    }

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Helpers
//

int
fps_wait_event(fps_handle_t *handle,
               double       sleep_us)
{
    int status = 0;
    int events = 0;

    fps_sleep(sleep_us);

    status = fps_check_interrupt(handle, FPS_ALL_EVENTS);
    if (status < 0) {
        return status;
    }

	return status;
}

void
fps_sleep(double sleep_us)
{
    __int64 elapsed_time;
    __int64 start_time;
    __int64 freq;
    __int64 wait_time;

    QueryPerformanceFrequency((LARGE_INTEGER *) &freq);

    wait_time = (__int64) (((double) freq) * sleep_us / ((double) 1000000.0f) + 0.5);
    
    QueryPerformanceCounter((LARGE_INTEGER *) &start_time);

    elapsed_time = start_time;

    while ((elapsed_time - start_time) < wait_time) {
        QueryPerformanceCounter((LARGE_INTEGER *) &elapsed_time);
    }
}
