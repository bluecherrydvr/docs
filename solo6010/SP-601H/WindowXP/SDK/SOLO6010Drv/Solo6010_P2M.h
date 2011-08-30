#ifndef __SOLO6010_P2M_H__
#define __SOLO6010_P2M_H__

#include "DriverDefs.h"
#include "SupportFunc.h"
#include "Solo6010.h"

enum
{
	P2M_FUNC_RET_OK,
	P2M_FUNC_RET_ERROR,
};

#define P2M_TIMEOUT_NORM			200		// ms
#define P2M_TIMEOUT_CODE_DATA		100		// ms

#define P2M_ERROR_RETRY_NORM		2
#define P2M_ERROR_RETRY_LIVE		0

#ifdef __cplusplus
extern "C" {
#endif


int S6010_InitP2M (DEVICE_EXTENSION *pdx);
int S6010_TestP2M (DEVICE_EXTENSION *pdx, unsigned int addrTest);

// idxOP: P2M_READ (EXT -> PC), P2M_WRITE (EXT <- PC)
int S6010_P2M_Copy (DEVICE_EXTENSION *pdx, unsigned int idxP2M, unsigned int idxOP,
	unsigned int nAddrPC, unsigned int nAddrExt, unsigned int szCopy,
	unsigned int numErrorRetry, unsigned int numTimeoutMS);
int S6010_P2M_Copy_Adv (DEVICE_EXTENSION *pdx, unsigned int idxP2M, unsigned int idxOP,
	unsigned int nAddrPC, unsigned int nAddrExt, unsigned int szCopy, unsigned int numRepeat, unsigned int nAddrIncPC, unsigned int nAddrIncExt,
	unsigned int numErrorRetry, unsigned int numTimeoutMS);

#ifdef __USE_ONLY_1_P2M_CHANNEL__

#define S6010_P2M_Copy_Sync		S6010_P2M_Copy
#define S6010_P2M_Copy_Adv_Sync	S6010_P2M_Copy_Adv

#else

int S6010_P2M_Copy_Sync (DEVICE_EXTENSION *pdx, unsigned int idxP2M, unsigned int idxOP,
	unsigned int nAddrPC, unsigned int nAddrExt, unsigned int szCopy,
	unsigned int numErrorRetry, unsigned int numTimeoutMS);
int S6010_P2M_Copy_Adv_Sync (DEVICE_EXTENSION *pdx, unsigned int idxP2M, unsigned int idxOP,
	unsigned int nAddrPC, unsigned int nAddrExt, unsigned int szCopy, unsigned int numRepeat, unsigned int nAddrIncPC, unsigned int nAddrIncExt,
	unsigned int numErrorRetry, unsigned int numTimeoutMS);

#endif


#ifdef __cplusplus
}
#endif

#endif
