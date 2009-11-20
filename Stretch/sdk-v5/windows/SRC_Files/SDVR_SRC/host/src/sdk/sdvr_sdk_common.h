/************************************************************************
 *
 * Copyright C 2007 Stretch, Inc. All rights reserved.  Stretch products are
 * protected under numerous U.S. and foreign patents, maskwork rights,
 * copyrights and other intellectual property laws.  
 *
 * This source code and the related tools, software code and documentation, and
 * your use thereof, are subject to and governed by the terms and conditions of
 * the applicable Stretch IDE or SDK and RDK License Agreement
 * (either as agreed by you or found at www.stretchinc.com).  By using these
 * items, you indicate your acceptance of such terms and conditions between you
 * and Stretch, Inc.  In the event that you do not agree with such terms and
 * conditions, you may not use any of these items and must immediately destroy
 * any copies you have made.
 *
 ************************************************************************/

#ifndef STRETCH_SDK_COMMON_H
#define STRETCH_SDK_COMMON_H
//#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
//#include <crtdbg.h>
#include <memory.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
#ifdef WIN32
#include <windows.h>
#endif

#ifdef DVR_SDK_TEST
#include "dvrfw_test.h"
#endif
#include <dvr_common.h>
#include <sct.h>
#include "sdvr_sdk.h"
#include "sdk.h"
#include "sdvr_debug.h"
#include "sdk_utils.h"

#if defined(__cplusplus)
   #define EXTERN_SDK_COMMON extern "C"
#else
   #define EXTERN_SDK_COMMON extern
#endif

/**********************************************************************
  host side encoder APIs
***********************************************************************/
EXTERN_SDK_COMMON sx_uint32 venc264_start_code_emulation(unsigned char *ibuf, int isize, unsigned char *obuf);


#endif // STRETCH_SDK_COMMON_H


