#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "../common.h"
#include "com.h"
#include "eim.h"

/***********************************************
* Command Register
*-----------------------------------------------
* Bit 15       0: Read FPGA Register, 1: Write FPGA Register
* Bit 14:8     Reserved
* Bit  7:0     FPGA Register Address
************************************************/
#define FPGA_REG_ADDR_MASK 0x00FF
#define FPGA_REG_RW_MASK   0x8000

#define FPGA_REG_WRITE 0x8000
#define FPGA_REG_READ  0x4000

/***********************************************

 ***********************************************/

#define DELAY(n) { int i = (n); while(i--); }
#define DELAY_DEFUALT() DELAY(5)

uint16_t reg_read(uint16_t reg)
{
    uint16_t cmdval = ((reg & FPGA_REG_ADDR_MASK) | FPGA_REG_READ);
    eim_write_command(cmdval);
    DELAY_DEFUALT();
    return eim_read_result();
}

void reg_write(uint8_t reg, uint16_t value)
{
    uint16_t cmdval;

    eim_write_data(value);
    DELAY_DEFUALT();

    cmdval = ((reg & FPGA_REG_ADDR_MASK) | FPGA_REG_WRITE);
    eim_write_command(cmdval);
    DELAY_DEFUALT();
}

//if you want to ARM to control the FPGA registers, the controller must be set to ARM
#define REG_GLOBAL_CONTROL 0x10
#define REG_GLOBAL_CONTROL_MASK_CONTROLLER 0x8000
void reg_set_controller(enum fpga_reg_controller controller)
{
    uint16_t data = reg_read(REG_GLOBAL_CONTROL);
    if (controller == FPGA_REG_CONTROLLER_ARM)
    {
        data |= REG_GLOBAL_CONTROL_MASK_CONTROLLER;
    }
    else
    {
        data &= (~REG_GLOBAL_CONTROL_MASK_CONTROLLER);
    }
    reg_write(REG_GLOBAL_CONTROL, data);
    //log_debug("read back the 0x10: %04x\n", reg_read(REG_GLOBAL_CONTROL));
}

int reg_set_source_format(enum CameraId camera, enum CapMode mode)
{
    uint16_t fpgamode;
    uint8_t  bufcnt = 8; //default is 8 frames
    uint8_t  outputcnt = 1;

    //Toggle bit11 from 1 to 0 is to reset CSI controller
    reg_write(0x11, 0x800);

    switch(mode)
    {
        case CAP_MODE_GREY_8BIT:
        case CAP_MODE_COLOR_8BIT:
        case CAP_MODE_OV5640:
            fpgamode = (camera == CAMERA_LEFT ? 0x202c : 0x2024);
            bufcnt = 0x01;
            outputcnt = bufcnt;
            break;
        case CAP_MODE_DUAL_CAMERA_COLOR:
            fpgamode = 0x6024;
            bufcnt = 0x01;
            outputcnt = bufcnt;
            break;
        case CAP_MODE_BACKGROUND_SUB:
        case CAP_MODE_GREY_1BIT:
            fpgamode = (camera == CAMERA_LEFT ? 0x9050 : 0x9150);
            break;
        case CAP_MODE_OPEN:
            fpgamode = (camera == CAMERA_LEFT ? 0x9090 : 0x9190);
            break;
        case CAP_MODE_CONTOUR:
            fpgamode = 0x9000;
            break;
        default:
            log_error("unrecongized mode for fpga source format: %d\n", mode);
            return -1;
    }

    reg_write(0x12, fpgamode);

    uint16_t readdata = reg_read(0x12);
    if (fpgamode != readdata)
    {
        log_error("config source format fails, read data (0x%04x) doesn't match with write one (0x%04x)\n", readdata, fpgamode);
        return -1;
    }

    reg_write(0x13, outputcnt); //output cnt
    reg_write(0x11, bufcnt); //set bufcnt and also reset FGPA csi

    log_info("Set FPGA Source: 0x%04x success!\n", fpgamode);
    return 0;
}

/***********************************************
 Built in self test
 ***********************************************/
static void reg_read_test()
{
    uint16_t addr = 0x10;
    uint16_t rdata;
    while(1)
    {
        rdata = reg_read(addr);
        log_debug("RDATA=0x%04X\n", rdata);
        sleep(1);
    }
}

static void reg_readwrite_test()
{
    uint8_t  addr = 0x10;
    uint16_t wdata = 1;
    uint16_t rdata;

    for (wdata = 1; wdata <= 0xFFEE; wdata++)
    {
        if (wdata == 0xFFEE)
            wdata = 1;

        log_debug("Write 0x%04x = 0x%04x ... ", addr, wdata);

        reg_write(addr, wdata);
        rdata = reg_read(addr);
        if (rdata == wdata)
        {
            log_debug("Success\n");
        }
        else
        {
            log_debug("Fail. RDATA=0x%04X\n", rdata);
        }

        usleep(1000);
    }
}

void reg_test()
{
    reg_readwrite_test();
}


