/*
 * $Header: /rd_2/misc/SizeDetect/C/SizeDetect/SizeDetect_Lib/SizeDetect.h 4     06/10/11 8:18p Sor $
 *
 * Copyright 2005 Vivotek, Inc.  All rights reserved.
 *
 * Description:
 *
 * $History: SizeDetect.h $
 * 
 * *****************  Version 4  *****************
 * User: Sor          Date: 06/10/11   Time: 8:18p
 * Updated in $/rd_2/misc/SizeDetect/C/SizeDetect/SizeDetect_Lib
 * FEATURE: Use PVOID data type for all cross platform variables - DONE.
 * 
 * *****************  Version 3  *****************
 * User: Sor          Date: 06/10/11   Time: 11:21a
 * Updated in $/rd_2/misc/SizeDetect/C/SizeDetect/SizeDetect_Lib
 * UPDATE: Update version number from 1.0.0.0 to 2.0.0.0 - DONE.
 * 
 * *****************  Version 2  *****************
 * User: Sor          Date: 06/10/11   Time: 11:19a
 * Updated in $/rd_2/misc/SizeDetect/C/SizeDetect/SizeDetect_Lib
 * FEATURE: Synchronize with GBEngine_Lib version 5.0.0.0 - DONE.
 * FEATURE: Remove all warnings about 64-bit portability issue - DONE.
 * 
 * *****************  Version 1  *****************
 * User: Sor          Date: 05/07/01   Time: 4:25p
 * Created in $/rd_2/misc/SizeDetect/C/SizeDetect/SizeDetect_Lib
 * Video and Image Bitstream Size Detector Pure C Version 1.0.0.0.
 * 
 */

/* ============================================================================================== */
#ifndef __SIZEDETECT_H__
#define __SIZEDETECT_H__

/* ============================================================================================== */
#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "errordef.h"
#include "vivo_codec.h"

/* ============================================================================================== */
#define SIZEDETECT_VERSION MAKEFOURCC(2, 0, 0, 0)

/* ============================================================================================== */
typedef enum codec_type_flags
{
	CODEC_TYPE_JPEG = 1,
	CODEC_TYPE_H263 = 2,
	CODEC_TYPE_VIVOTEK_H263 = 3,
	CODEC_TYPE_MPEG4 = 4,
	CODEC_TYPE_H264 = 5
} ECodecTypeFlags;

/* ============================================================================================== */
typedef struct size_detect_init_options
{
	DWORD dwVersion;

	ECodecTypeFlags eCodecTypeFlag;

	void *pObjectMem;
} TSizeDetectInitOptions;

typedef struct size_detect_state
{
	DWORD dwWidth;
	DWORD dwHeight;
} TSizeDetectState;

typedef struct size_detect_options
{
	EVideoOptionFlags eOptionFlags;
	PVOID apvUserData[3];
} TSizeDetectOptions;

/* ============================================================================================== */
#ifdef __cplusplus
extern "C" {
#endif

SCODE DLLAPI SizeDetect_Initial(HANDLE *phObject, TSizeDetectInitOptions *ptInitOptions);
SCODE DLLAPI SizeDetect_OneFrame(HANDLE hObject, TSizeDetectState *ptState);
SCODE DLLAPI SizeDetect_Release(HANDLE *phObject);

SCODE DLLAPI SizeDetect_SetOptions(HANDLE hObject, TSizeDetectOptions *ptOptions);
DWORD DLLAPI SizeDetect_QueryMemSize(TSizeDetectInitOptions *ptInitOptions);

SCODE DLLAPI SizeDetect_GetVersionInfo(BYTE *pbyMajor, BYTE *pbyMinor, BYTE *pbyBuild, BYTE *pbyRevision);

#ifdef __cplusplus
}
#endif

/* ============================================================================================== */
#endif  //__SIZEDETECT_H__

