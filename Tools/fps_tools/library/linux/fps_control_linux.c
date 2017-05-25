#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include "debug.h"
#include "fps_control.h"
#include "fps_control_linux.h"


// Record how many devices are opened
static int device_opened = 0;


////////////////////////////////////////////////////////////////////////////////
//
// Sensor Open/Close/Reset/Init
//

fps_handle_t*
fps_open_sensor(char *file)
{
    int fd;

    fd = open(file, O_RDWR);
    if (fd < 0) {
        return NULL;
    }

    return fps_attach_sensor(fd);
}

int
fps_close_sensor(fps_handle_t **handle)
{
    close((*handle)->fd);

    return fps_detach_sensor(handle);
}

int
fps_reset_sensor(fps_handle_t *handle,
                 int          state)
{
    int status = 0;

    struct fps_ioc_transfer tr = {
        .len    = (__u32) state,
        .opcode = (__u8)  FPS_IOC_RESET_SENSOR,
    };

    status = ioctl(handle->fd, FPS_IOC_MESSAGE(1), &tr);
    if (status < 0) {
        LOG_ERROR("Calling ioctl() failed! status = %0d\n", status);
    }

    return status;
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

    struct fps_ioc_transfer tr = {
        .len    = (__u32) speed_hz,
        .opcode = (__u8)  FPS_IOC_SET_CLKRATE,
    };

    status = ioctl(handle->fd, FPS_IOC_MESSAGE(1), &tr);
    if (status < 0) {
        LOG_ERROR("Calling ioctl() failed! status = %0d\n", status);
    }

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
    int     status = 0;
    uint8_t rx[2];

    if (chip_id == NULL) {
        return status = -1;
    }

    struct fps_ioc_transfer tr = {
        .rx_buf = (unsigned long) rx,
        .opcode = FPS_IOC_READ_CHIP_ID,
    };

    status = ioctl(handle->fd, FPS_IOC_MESSAGE(1), &tr);
    if (status < 0) {
        LOG_ERROR("Calling ioctl() failed! status = %0d\n", status);
        return status;
    }

    *chip_id = ((int) rx[0] << 8) | ((int) rx[1] << 0);

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
    int      status = 0;
    uint8_t *tx;
    uint8_t *rx;
    int      i;

    tx = (uint8_t *) malloc(length);
    if (tx == NULL) {
        LOG_ERROR("malloc() failed!\n");
        goto fps_multiple_read_end;
    }

    rx = (uint8_t *) malloc(length);
    if (rx == NULL) {
        LOG_ERROR("malloc() failed!\n");
        goto fps_multiple_read_end;
    }

    for (i = 0; i < length; i++) {
        tx[i] = addr[i];
        rx[i] = 0x00;
    }

    struct fps_ioc_transfer tr = {
        .tx_buf = (unsigned long) tx,
        .rx_buf = (unsigned long) rx,
        .len    = (__u32) length,
        .opcode = (__u8)  FPS_IOC_REGISTER_MASS_READ,
    };

    status = ioctl(handle->fd, FPS_IOC_MESSAGE(1), &tr);
    if (status < 0) {
        LOG_ERROR("Calling ioctl() failed! status = %0d\n", status);
        goto fps_multiple_read_end;
    }

    for (i = 0; i < length; i++) {
        data[i] = rx[i];
        LOG_DETAIL("addr = 0x%02X, data = 0x%02X\n", addr[i], data[i]);
    }

fps_multiple_read_end :

    if (tx != NULL) {
        free(tx);
    }

    if (rx != NULL) {
        free(rx);
    }

    return status;
}

int
fps_multiple_write(fps_handle_t *handle,
                   uint8_t      *addr,
                   uint8_t      *data,
                   size_t       length)
{
    int      status = 0;
    uint8_t *tx;
    int      i;

    tx = (uint8_t *) malloc(length * 2);
    if (tx == NULL) {
        LOG_ERROR("malloc() failed!\n");
        goto fps_multiple_write_end;
    }

    for (i = 0; i < length; i++) {
        tx[(i * 2) + 0] = addr[i];
        tx[(i * 2) + 1] = data[i];
    }

    struct fps_ioc_transfer tr = {
        .tx_buf = (unsigned long) tx,
        .len    = (__u32) (length * 2),
        .opcode = (__u8)  FPS_IOC_REGISTER_MASS_WRITE,
    };

    status = ioctl(handle->fd, FPS_IOC_MESSAGE(1), &tr);
    if (status < 0) {
        LOG_ERROR("Calling ioctl() failed! status = %0d\n", status);
        goto fps_multiple_write_end;
    }

    for (i = 0; i < length; i++) {
        LOG_DETAIL("addr = 0x%02X, data = 0x%02X\n", addr[i], data[i]);
    }

fps_multiple_write_end :

    if (tx != NULL) {
        free(tx);
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
    uint8_t tx[6];

    img_size = img_width * img_height + handle->latency;

    tx[0] = img_width;
    tx[1] = img_height;
    tx[2] = handle->latency;
    tx[3] = 0x00;
    tx[4] = 0x00;
    tx[5] = 0x00;

    struct fps_ioc_transfer tr = {
        .tx_buf = (unsigned long) tx,
        .rx_buf = (unsigned long) img_buf,
        .len    = (__u32) img_size,
        .opcode = (__u8)  FPS_IOC_GET_ONE_IMG
    };

    status = ioctl(handle->fd, FPS_IOC_MESSAGE(1), &tr);
    if (status < 0) {
        LOG_ERROR("Calling ioctl() failed! status = %0d\n", status);
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
    int           status = 0;
    int           timeout;
    struct pollfd poll_fps;

    timeout = (sleep_us < 0) ? -1 : (int) (sleep_us / 1000);

    poll_fps.fd      = handle->fd;
    poll_fps.events  = POLLIN;
    poll_fps.revents = 0;

    status = poll(&poll_fps, 1, timeout);
    if ((status < 0) && (errno != EINTR)) {
        LOG_ERROR("Calling poll() failed! status = %0d\n", status);
        return status;
    }
    LOG_DETAIL("poll_fps.revents = %0d\n", poll_fps.revents);

    return poll_fps.revents;
}

void
fps_sleep(double sleep_us)
{
    usleep((int) sleep_us);
}
