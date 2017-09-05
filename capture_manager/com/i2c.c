#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "../common.h"
#include "com.h"
#include "eim.h"

/***********************************************
 * FPGA Register List
 ***********************************************/
#define REG_I2C_COMMAND 0x01
#define REG_I2C_STATUS  0x01
#define REG_DEVICE_REG  0x02
#define REG_I2C_WDATA   0x03
#define REG_I2C_RDATA   0x04
#define REG_I2C_DEVICE_ADDR 0x05

#define ISP_REG_SWITCH      0x020B
#define ISP_SWITCH_ENABLE   0x09
#define ISP_SWITCH_DISABLE  0x08

//The i2c address
#define I2C_ADDR_ISP 0x84
#define I2C_ADDR_CAMERA 0xC0 //The left/right camera is selected in command register


/***********************************************
 * BITS For I2C Command Register
 ***********************************************/
#define I2C_WRITE_MODE  0x0001
#define I2C_READ_MODE   0x0002

//bit 15-14 is to select one of the camera
#define I2C_COMMAND_CAMERA_ID_MASK 0xC000
#define I2C_COMMAND_CAMERA_0 0x0000
#define I2C_COMMAND_CAMERA_1 0x8000

#define I2C_RDATA_READY_MASK    0x0100
#define I2C_BUS_NOT_BUSY_MASK   0x1000

void i2c_cmd_reset()
{
    reg_write(REG_I2C_COMMAND, 0x00);
}

static int wait_i2c_avaiable()
{
    uint16_t data;
    long t = get_time_ms();
    while((get_time_ms() - t) <= 1000)
    {
        data = reg_read(REG_I2C_STATUS);
        if (data & I2C_BUS_NOT_BUSY_MASK)
        {
            return 0;
        }
        usleep(10);
    }
    return -1;
}

int i2c_write(uint8_t i2c_addr, enum CameraId camera_id, uint16_t dev_reg, uint8_t data)
{
    uint16_t val;
    if (wait_i2c_avaiable() < 0)
    {
        log_debug("i2c_write: wait i2c aviable timeout (before writing)!\n");
        return -1;
    }
    i2c_cmd_reset();
    reg_write(REG_DEVICE_REG, dev_reg);
    reg_write(REG_I2C_DEVICE_ADDR, i2c_addr);

    reg_write(REG_I2C_WDATA, data);

    //parse camera id
    if (camera_id == CAMERA_LEFT)
    {
        val = (I2C_WRITE_MODE | (I2C_COMMAND_CAMERA_0 & I2C_COMMAND_CAMERA_ID_MASK));
    }
    else if (camera_id == CAMERA_RIGHT)
    {
        val = (I2C_WRITE_MODE | (I2C_COMMAND_CAMERA_1 & I2C_COMMAND_CAMERA_ID_MASK));
    }
    else
    {
        log_debug("i2c_write: invalid camera_id!\n");
        return -1;
    }

    reg_write(REG_I2C_COMMAND, val);

    if (wait_i2c_avaiable() < 0)
    {
        log_debug("i2c_write: wait i2c aviable timeout (after writing)!\n");
        return -1;
    }
    return 0;
}

#define I2C_READ_TIMEOUT_MS (1000)


int i2c_read(uint8_t i2c_addr, enum CameraId camera_id, uint16_t dev_reg, uint8_t *data)
{
    uint16_t val;
    long t;

    if (wait_i2c_avaiable() < 0)
    {
        log_debug("i2c_read: wait i2c aviable timeout!\n");
        return -1;
    }

    i2c_cmd_reset();

    reg_write(REG_DEVICE_REG, dev_reg);
    reg_write(REG_I2C_DEVICE_ADDR, i2c_addr);

    //parse camera id
    if (camera_id == CAMERA_LEFT)
    {
        val = (I2C_READ_MODE | (I2C_COMMAND_CAMERA_0 & I2C_COMMAND_CAMERA_ID_MASK));
    }
    else if (camera_id == CAMERA_RIGHT)
    {
        val = (I2C_READ_MODE | (I2C_COMMAND_CAMERA_1 & I2C_COMMAND_CAMERA_ID_MASK));
    }
    else
    {
        log_debug("i2c_read: invalid camera_id!\n");
        return -1;
    }
    reg_write(REG_I2C_COMMAND, val);

    t = get_time_ms();
    while((get_time_ms() - t) <= I2C_READ_TIMEOUT_MS)
    {
        val = reg_read(REG_I2C_STATUS);
        if (val & I2C_RDATA_READY_MASK)
        {
            *data = reg_read(REG_I2C_RDATA);
            return 0;
        }
        usleep(10);
    }

    //The ready signal is not ok, this i2c read operation fails
    return -1;
}

int isp_read(enum CameraId camera_id, uint16_t dev_reg, uint8_t *data)
{
    if (i2c_write(I2C_ADDR_ISP, camera_id, ISP_REG_SWITCH, ISP_SWITCH_DISABLE) < 0)
    {
        log_error("disabe isp switch fails!\n");
        return -1;
    }
    return i2c_read(I2C_ADDR_ISP, camera_id, dev_reg, data);
}

int isp_write(enum CameraId camera_id, uint16_t dev_reg, uint8_t data)
{
    if (i2c_write(I2C_ADDR_ISP, camera_id, ISP_REG_SWITCH, ISP_SWITCH_DISABLE) < 0)
    {
        log_error("disabe isp switch fails!\n");
        return -1;
    }
    return i2c_write(I2C_ADDR_ISP, camera_id, dev_reg, data);
}

int camera_read(enum CameraId camera_id, uint16_t dev_reg, uint8_t *data)
{
    int ret = 0;
    ret = i2c_write(I2C_ADDR_ISP, camera_id, ISP_REG_SWITCH, ISP_SWITCH_ENABLE);
    if (ret < 0)
    {
        goto defer;
    }
    ret = i2c_read(I2C_ADDR_CAMERA, camera_id, dev_reg, data);
defer:
    i2c_write(I2C_ADDR_ISP, camera_id, ISP_REG_SWITCH, ISP_SWITCH_DISABLE);
    return ret;
}

int camera_write(enum CameraId camera_id, uint16_t dev_reg, uint8_t data)
{
    int ret = 0;
    ret = i2c_write(I2C_ADDR_ISP, camera_id, ISP_REG_SWITCH, ISP_SWITCH_ENABLE);
    if (ret < 0)
    {
        goto defer;
    }
    ret = i2c_write(I2C_ADDR_CAMERA, camera_id, dev_reg, data);
defer:
    i2c_write(I2C_ADDR_ISP, camera_id, ISP_REG_SWITCH, ISP_SWITCH_DISABLE);
    return ret;
}

static void isp_read_test()
{
    uint16_t reg = 0x281B;
    uint8_t rdata;
    int ret;
    int camera = CAMERA_LEFT;

    while(1)
    {
        log_info("Press key to read data:\n");
        getchar();

        ret = isp_read(camera, reg, &rdata);
        if (ret < 0)
        {
            log_debug("i2c_read fails, ready signal is not ok\n");
        }
        else
        {
            log_debug("i2c_read_data: reg(0x%04X) = 0x%02X\n", reg, rdata);
        }

        //break;

        //sleep(1);
    }
}

static void isp_readwrite_test()
{
    uint16_t reg = 0x281B;
    uint8_t rdata, wdata;
    int ret;
    int camera = CAMERA_LEFT;

    while(1)
    {
        for (wdata = 0x12; wdata <= 0xFE; wdata++)
        {
            if (isp_write(camera, reg, wdata) < 0)
            {
                log_debug("isp_readwrite fails, i2c_write fails, wdata=0x%02X\n", wdata);
                continue;
            }

            ret = isp_read(camera, reg, &rdata);
            if (ret < 0)
            {
                log_debug("isp_readwrite fails, ready signal is not ok\n");
                //return;
            }
            else if (rdata != wdata)
            {
                log_debug("isp_readwrite fail: reg(0x%04X), rdata=0x%02X, wdata=0x%02X\n", reg, rdata, wdata);
            }
            else
            {
                log_debug("isp_readwrite success: reg(0x%04X) = 0x%02X\n", reg, rdata);
            }

            usleep(200);
        }
        //sleep(1);
    }
}

static void camera_readwrite_test()
{
    uint16_t reg = 0x3509;
    uint8_t rdata, wdata;
    int ret;
    int camera = CAMERA_LEFT;

    while(1)
    {
        for (wdata = 0x12; wdata <= 0xFE; wdata++)
        {
            if (camera_write(camera, reg, wdata) < 0)
            {
                log_debug("camera_readwrite_test fails, camera_write fails, wdata=0x%02X\n", wdata);
                continue;
            }

            ret = camera_read(camera, reg, &rdata);
            if (ret < 0)
            {
                log_debug("camera_readwrite_test fails, ready signal is not ok\n");
                //return;
            }
            else if (rdata != wdata)
            {
                log_debug("camera_readwrite_test fail: reg(0x%04X), rdata=0x%02X, wdata=0x%02X\n", reg, rdata, wdata);
            }
            else
            {
                log_debug("camera_readwrite_test success: reg(0x%04X) = 0x%02X\n", reg, rdata);
            }

            usleep(200);
        }
        //sleep(1);
    }
}

void isp_test()
{
    isp_readwrite_test();
}

void camera_i2c_test()
{
    camera_readwrite_test();
}


