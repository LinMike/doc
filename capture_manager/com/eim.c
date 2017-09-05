#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "../common.h"
#include "eim.h"

/***********************************************
 * EIM Base
 ***********************************************/
#define BASE_ADDR       0x8000000
#define MEM_SIZE        0x10000
#define MEM_MIN_BYTES   (2)

//#define ADDR(a) (BASE_ADDR | ((a) << 13))
#define ADDR(a)                ((a) << 14)


/***********************************************
 * EIM_AD[15:13]   Description
 * ---------------------------------------------
 *     0b001       Command Register
 *     0b010       Data Register (Write)
 *     0b011       Result Register (Read)
 ***********************************************/
#define ADDR_COMMAND    ADDR(1)
#define ADDR_DATA       ADDR(2)
#define ADDR_RESULT     ADDR(3)

static volatile uint8_t* fpga_mem = NULL;
static volatile uint16_t* g_command = NULL;
static volatile uint16_t* g_data = NULL;
static volatile uint16_t* g_result = NULL;
static int fd;

int eim_open()
{
    int fd;

    fd = open("/dev/mem", O_RDWR);
    if (fd < 0) {
        log_error("Cannot open /dev/mem.\n");
        return -1;
    }

    log_debug("open /dev/mem success!\n");

    fpga_mem = (volatile uint8_t*)mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, BASE_ADDR);
    if (fpga_mem < 0) {
        log_error("eim mmap failed!\n");
        return -1;
    }

    log_debug("eim mmap succeed, BaseAddr=0x%08X\n", BASE_ADDR);

    g_command = (uint16_t*)(fpga_mem + ADDR_COMMAND);
    g_data = (uint16_t*)(fpga_mem + ADDR_DATA);
    g_result = (uint16_t*)(fpga_mem + ADDR_RESULT);
    return 0;
}

int eim_close()
{
    munmap((void*)fpga_mem, MEM_SIZE);
    close(fd);
    return 0;
}

uint16_t eim_read_command()
{
    return *g_command;
}

void eim_write_command(uint16_t data)
{
    *g_command = data;
}

uint16_t eim_read_data()
{
    return *g_data;
}

void eim_write_data(uint16_t data)
{
    *g_data = data;
}

uint16_t eim_read_result()
{
    return *g_result;
}

void eim_write_result(uint16_t data)
{
    *g_result = data;
}

static void eim_command_test()
{
    char str[64];
    uint16_t value;
    uint16_t rdata;
    while(1)
    {
        scanf("%s", str);
        if (str[0] == 'w')
        {
            value = (uint16_t)strtol(str+1, NULL, 16);
            log_debug("Write EIM Command: 0x%04X\n", value);
            *g_command = value;
            rdata = *g_command;
            log_debug("EIM Command Read-Back: 0x%04X,      %s\n", rdata, rdata == value ? "SUCCESS" : "FAIL");
        }
        else if (str[0] == 'r')
        {
            log_debug("EIM Command Read-Back: 0x%04X\n", *g_command);
        }
    }
}

static void eim_data_test()
{
    char str[64];
    uint16_t value;
    uint16_t rdata;
    while(1)
    {
        scanf("%s", str);
        if (str[0] == 'w')
        {
            value = (uint16_t)strtol(str+1, NULL, 16);
            log_debug("Write EIM Data: 0x%04X\n", value);
            *g_data = value;
            rdata = *g_data;
            log_debug("EIM Data Read-Back: 0x%04X,      %s\n", rdata, rdata == value ? "SUCCESS" : "FAIL");
        }
        else if (str[0] == 'r')
        {
            log_debug("EIM Data Read-Back: 0x%04X\n", *g_data);
        }
    }
}

static void eim_result_test()
{
    char str[64];
    while(1)
    {
        scanf("%s", str);
        if (str[0] == 'r')
        {
            log_debug("EIM Result Read-Back: 0x%04X\n", *g_result);
        }
    }
}
