/*
 * isil_test_sdk.c
 *
 *  Created on: 2011-6-30
 *      Author: junbinwang
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "isil_test_sdk.h"
#include "isil_codec_chip_ctl.h"

int isil_chip_read_reg(unsigned int u32ChipID, struct reg_info *info)
{
	int ret;
	CODEC_HANDLE ChipHandle;

	if(NULL == info)
	{
		fprintf(stderr, "%s parm null\n", __FUNCTION__);
		return -1;
	}
	ret = ISIL_CODEC_CTL_GetChipHandle(u32ChipID, &ChipHandle);
	if(ret < 0)
	{
		fprintf(stderr, "%s get chip handle fail\n", __FUNCTION__);
		return -1;
	}

	if(ioctl(ChipHandle, ISIL_CHIP_REG_READ, info) < 0)
	{
		fprintf(stderr, "%s read reg fail\n", __FUNCTION__);
		return -1;
	}
	return 0;
}
int isil_chip_write_reg(unsigned int u32ChipID, struct reg_info *info)
{

	int ret;
	CODEC_HANDLE ChipHandle;

	if(NULL == info)
	{
		fprintf(stderr, "%s parm null\n", __FUNCTION__);
		return -1;
	}
	ret = ISIL_CODEC_CTL_GetChipHandle(u32ChipID, &ChipHandle);
	if(ret < 0)
	{
		fprintf(stderr, "%s get chip handle fail\n", __FUNCTION__);
		return -1;
	}

	if(ioctl(ChipHandle, ISIL_CHIP_REG_WRITE, info) < 0)
	{
		fprintf(stderr, "%s write reg fail\n", __FUNCTION__);
		return -1;
	}
	return 0;
}

int isil_chip_read_mpb(unsigned int u32ChipID, struct mpb_info* info)
{
	int ret;
	CODEC_HANDLE ChipHandle;

	if(NULL == info)
	{
		fprintf(stderr, "%s parm null\n", __FUNCTION__);
		return -1;
	}
	ret = ISIL_CODEC_CTL_GetChipHandle(u32ChipID, &ChipHandle);
	if(ret < 0)
	{
		fprintf(stderr, "%s get chip handle fail\n", __FUNCTION__);
		return -1;
	}

	if(ioctl(ChipHandle, ISIL_CHIP_MPB_READ, info) < 0)
	{
		fprintf(stderr, "%s read mpb fail\n", __FUNCTION__);
		return -1;
	}
	return 0;
}
int isil_chip_write_mpb(unsigned int u32ChipID, struct mpb_info* info)
{
	int ret;
	CODEC_HANDLE ChipHandle;

	if(NULL == info)
	{
		fprintf(stderr, "%s parm null\n", __FUNCTION__);
		return -1;
	}
	ret = ISIL_CODEC_CTL_GetChipHandle(u32ChipID, &ChipHandle);
	if(ret < 0)
	{
		fprintf(stderr, "%s get chip handle fail\n", __FUNCTION__);
		return -1;
	}

	if(ioctl(ChipHandle, ISIL_CHIP_MPB_WRITE, info) < 0)
	{
		fprintf(stderr, "%s write mpb fail\n", __FUNCTION__);
		return -1;
	}
	return 0;
}

int isil_chip_read_i2c(unsigned int u32ChipID, struct i2c_info* info)
{
	int ret;
	CODEC_HANDLE ChipHandle;

	if(NULL == info)
	{
		fprintf(stderr, "%s parm null\n", __FUNCTION__);
		return -1;
	}
	ret = ISIL_CODEC_CTL_GetChipHandle(u32ChipID, &ChipHandle);
	if(ret < 0)
	{
		fprintf(stderr, "%s get chip handle fail\n", __FUNCTION__);
		return -1;
	}

	if(ioctl(ChipHandle, ISIL_CHIP_I2C_READ, info) < 0)
	{
		fprintf(stderr, "%s read I2C fail\n", __FUNCTION__);
		return -1;
	}
	return 0;
}
int isil_chip_write_i2c(unsigned int u32ChipID, struct i2c_info* info)
{
	int ret;
	CODEC_HANDLE ChipHandle;

	if(NULL == info)
	{
		fprintf(stderr, "%s parm null\n", __FUNCTION__);
		return -1;
	}
	ret = ISIL_CODEC_CTL_GetChipHandle(u32ChipID, &ChipHandle);
	if(ret < 0)
	{
		fprintf(stderr, "%s get chip handle fail\n", __FUNCTION__);
		return -1;
	}

	if(ioctl(ChipHandle, ISIL_CHIP_I2C_WRITE, info) < 0)
	{
		fprintf(stderr, "%s write i2c fail\n", __FUNCTION__);
		return -1;
	}
	return 0;
}

int isil_chip_read_ddr(unsigned int u32ChipID, struct ddr_info* info)
{
	int ret;
	CODEC_HANDLE ChipHandle;

	if(NULL == info)
	{
		fprintf(stderr, "%s parm null\n", __FUNCTION__);
		return -1;
	}
	ret = ISIL_CODEC_CTL_GetChipHandle(u32ChipID, &ChipHandle);
	if(ret < 0)
	{
		fprintf(stderr, "%s get chip handle fail\n", __FUNCTION__);
		return -1;
	}

	if(ioctl(ChipHandle, ISIL_CHIP_DDR_READ, info) < 0)
	{
		fprintf(stderr, "%s read ddr fail\n", __FUNCTION__);
		return -1;
	}
	return 0;
}
int isil_chip_write_ddr(unsigned int u32ChipID, struct ddr_info* info)
{
	int ret;
	CODEC_HANDLE ChipHandle;

	if(NULL == info)
	{
		fprintf(stderr, "%s parm null\n", __FUNCTION__);
		return -1;
	}
	ret = ISIL_CODEC_CTL_GetChipHandle(u32ChipID, &ChipHandle);
	if(ret < 0)
	{
		fprintf(stderr, "%s get chip handle fail\n", __FUNCTION__);
		return -1;
	}

	if(ioctl(ChipHandle, ISIL_CHIP_DDR_WRITE, info) < 0)
	{
		fprintf(stderr, "%s write ddr fail\n", __FUNCTION__);
		return -1;
	}
	return 0;
}

int isil_chip_gpio_read(unsigned int u32ChipID, struct gpio_info* info)
{
	int ret;
	CODEC_HANDLE ChipHandle;

	if(NULL == info)
	{
		fprintf(stderr, "%s parm null\n", __FUNCTION__);
		return -1;
	}
	ret = ISIL_CODEC_CTL_GetChipHandle(u32ChipID, &ChipHandle);
	if(ret < 0)
	{
		fprintf(stderr, "%s get chip handle fail\n", __FUNCTION__);
		return -1;
	}

	if(ioctl(ChipHandle, ISIL_CHIP_GPIO_READ, info) < 0)
	{
		fprintf(stderr, "%s read gpio fail\n", __FUNCTION__);
		return -1;
	}
	return 0;
}
int isil_chip_gpio_write(unsigned int u32ChipID, struct gpio_info* info)
{
	int ret;
	CODEC_HANDLE ChipHandle;

	if(NULL == info)
	{
		fprintf(stderr, "%s parm null\n", __FUNCTION__);
		return -1;
	}
	ret = ISIL_CODEC_CTL_GetChipHandle(u32ChipID, &ChipHandle);
	if(ret < 0)
	{
		fprintf(stderr, "%s get chip handle fail\n", __FUNCTION__);
		return -1;
	}

	if(ioctl(ChipHandle, ISIL_CHIP_GPIO_WRITE, &info) < 0)
	{
		fprintf(stderr, "%s write gpio fail\n", __FUNCTION__);
		return -1;
	}
	return 0;
}
