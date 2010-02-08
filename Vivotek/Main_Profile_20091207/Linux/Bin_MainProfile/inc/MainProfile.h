#ifndef _MAINPROFILE_H_
#define _MAINPROFILE_H_

#include "common.h"

typedef struct 
{
	DWORD			dwOption;
}TMainProfileStaticOptopn;

typedef enum {
	eDataBroker_Use2K	= 0x0001,
	eDataBroker_Use3K	= 0x0002,
	eDataBroker_Use5K	= 0x0004,
	eDataBroker_Use7K	= 0x0008,
}EMainProfleFlag;


#ifdef __cplusplus
extern "C" {
#endif

SCODE DLLAPI MainProfileStatic_Initial(TMainProfileStaticOptopn tMPOptions);
SCODE DLLAPI MainProfileStatic_Release();

#ifdef __cplusplus
}
#endif

#endif	//_MAINPROFILE_H_