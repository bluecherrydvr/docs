#ifndef __SOLO6010_I2C_H__
#define __SOLO6010_I2C_H__

#include "DriverDefs.h"
#include "SupportFunc.h"
#include "Solo6010.h"

#ifdef __cplusplus
extern "C" {
#endif


void S6010_InitI2C (DEVICE_EXTENSION *pdx);
void S6010_DestroyI2C (DEVICE_EXTENSION *pdx);

int S6010_I2C_Write (DEVICE_EXTENSION *pdx, int nChannel,
	int nAddrSlave, int nAddrSub, int nData);
int S6010_I2C_Read (DEVICE_EXTENSION *pdx, int nChannel,
	int nAddrSlave, int nAddrSlaveRead, int nAddrSub);


#ifdef __cplusplus
}
#endif

#endif
