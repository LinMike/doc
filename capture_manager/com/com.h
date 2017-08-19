#ifdef __cplusplus
extern "C" {
#endif

#include "../common.h"

#ifndef __COM_H__
#define __COM_H__

//Communication
int com_open();
int com_close();

int com_request_access();
int com_release_access();

//FPGA Registers
uint16_t    reg_read(uint16_t reg);
void        reg_write(uint8_t reg, uint16_t value);
void        reg_test();

//The whole list of FPGA Register
enum fpga_reg_controller {
    FPGA_REG_CONTROLLER_ARM = 0x01,
    FPGA_REG_CONTROLLER_USB = 0x02
};
void reg_set_controller(enum fpga_reg_controller controller);

int reg_set_source_format(enum CameraId camera, enum CapMode mode);


//I2C Communication for ISP (Image Signal Processor)

//I2C Communication for Camera Chip


//BIST (Built-in-self-test)

void isp_test();
void camera_i2c_test();
int i2c_write(uint8_t i2c_addr, enum CameraId camera_id, uint16_t dev_reg, uint8_t data);
int i2c_read(uint8_t i2c_addr, enum CameraId camera_id, uint16_t dev_reg, uint8_t *data);
int isp_read(enum CameraId camera_id, uint16_t dev_reg, uint8_t *data);
int isp_write(enum CameraId camera_id, uint16_t dev_reg, uint8_t data);
int camera_read(enum CameraId camera_id, uint16_t dev_reg, uint8_t *data);
int camera_write(enum CameraId camera_id, uint16_t dev_reg, uint8_t data);


#endif

#ifdef __cplusplus
}
#endif
