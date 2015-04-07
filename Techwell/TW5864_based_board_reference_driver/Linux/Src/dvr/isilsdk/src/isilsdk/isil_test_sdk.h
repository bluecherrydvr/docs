/*
 * isil_test_sdk.h
 *
 *  Created on: 2011-6-30
 *      Author: junbinwang
 */

#ifndef ISIL_TEST_SDK_H_
#define ISIL_TEST_SDK_H_

#include "isil_interface.h"
#include "isil_codec_common.h"


struct reg_info {
        unsigned long startaddr;
        unsigned long *buffer;
        unsigned long count;            /* in double word  */
};

struct mpb_info {
        unsigned long startaddr;
        unsigned char *buffer;
        unsigned long count;            /* in byte */
};

struct i2c_info {
        unsigned long busaddr;
        unsigned long offset;
        unsigned char *buffer;
        unsigned long count;            /* in byte */
};

struct ddr_info {
        unsigned long startaddr;
        unsigned long *buffer;          /* pattern */
        unsigned long count;            /* in double word  */
};

struct gpio_info {
        unsigned long addr;
        unsigned long mode;
        unsigned long value;
};
#define ISIL_CHIP_REG_READ                              _IOWR(ISIL_CHIP_IOC_MAGIC, 239, struct reg_info)
#define ISIL_CHIP_REG_WRITE                             _IOW(ISIL_CHIP_IOC_MAGIC, 238, struct reg_info)
#define ISIL_CHIP_MPB_READ                              _IOWR(ISIL_CHIP_IOC_MAGIC, 237, struct mpb_info)
#define ISIL_CHIP_MPB_WRITE                             _IOW(ISIL_CHIP_IOC_MAGIC, 236, struct mpb_info)
#define ISIL_CHIP_I2C_READ                              _IOWR(ISIL_CHIP_IOC_MAGIC, 235, struct i2c_info)
#define ISIL_CHIP_I2C_WRITE                             _IOW(ISIL_CHIP_IOC_MAGIC, 234, struct i2c_info)
#define ISIL_CHIP_DDR_READ                              _IOWR(ISIL_CHIP_IOC_MAGIC, 233, struct ddr_info)
#define ISIL_CHIP_DDR_WRITE                             _IOW(ISIL_CHIP_IOC_MAGIC, 232, struct ddr_info)
#define ISIL_CHIP_GPIO_READ                             _IOWR(ISIL_CHIP_IOC_MAGIC, 231, struct gpio_info)
#define ISIL_CHIP_GPIO_WRITE                            _IOW(ISIL_CHIP_IOC_MAGIC, 230, struct gpio_info)

int isil_chip_read_reg(unsigned int u32ChipID, struct reg_info *info);
int isil_chip_write_reg(unsigned int u32ChipID, struct reg_info *info);

int isil_chip_read_mpb(unsigned int u32ChipID, struct mpb_info* info);
int isil_chip_write_mpb(unsigned int u32ChipID, struct mpb_info* info);

int isil_chip_read_i2c(unsigned int u32ChipID, struct i2c_info* info);
int isil_chip_write_i2c(unsigned int u32ChipID, struct i2c_info* info);

int isil_chip_read_ddr(unsigned int u32ChipID, struct ddr_info* info);
int isil_chip_write_ddr(unsigned int u32ChipID, struct ddr_info* info);

int isil_chip_gpio_read(unsigned int u32ChipID, struct gpio_info* info);
int isil_chip_gpio_write(unsigned int u32ChipID, struct gpio_info* info);


#endif /* ISIL_TEST_SDK_H_ */
