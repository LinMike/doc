#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "com.h"
#include "eim.h"
#include "../common.h"

int com_open()
{
    if (eim_open() < 0) {
        log_error("open eim fails\n");
        return -1;
    }

    if (com_request_access() < 0) {
        log_error("request communication access fails\n");
        return -1;
    }
}

int com_close()
{
    com_release_access();
    if (eim_close() < 0) {
        log_error("close eim fails\n");
        return -1;
    }
}

int com_request_access()
{
    reg_set_controller(FPGA_REG_CONTROLLER_ARM);
}

int com_release_access()
{
    reg_set_controller(FPGA_REG_CONTROLLER_USB);
}


