#ifdef __cplusplus
extern "C" {
#endif

#include "../common.h"

#ifndef __EIM_H__
#define __EIM_H__

int eim_open();
int eim_close();

uint16_t eim_read_command();
void eim_write_command(uint16_t data);

uint16_t eim_read_data();
void eim_write_data(uint16_t data);

uint16_t eim_read_result();
void eim_write_result(uint16_t data);

#endif

#ifdef __cplusplus
}
#endif
