/************************************************************************
*                                                                       *
*   video_error.h --  error code definitions for video codec module     *
*                                                                       *
*   Copyright (c) Vivotek Corp.  All rights reserved.                   *
*                                                                       *
************************************************************************/

#ifndef _VIDEO_ERROR_
#define _VIDEO_ERROR_

// Severity code 1

//--------------------------------------------------------------------------------
// Video Codec Error Codes
//--------------------------------------------------------------------------------
#define VIDEO_E_FIRST 0x80010000
#define VIDEO_E_LAST  0x8001FFFF
#define VIDEO_S_FIRST 0x00010000
#define VIDEO_S_LAST  0x0001FFFF

/* 2003.01.15 */
#define VIDEO_S_STUFFING			  0x00010001
#define VIDEO_S_MARKER				  0x00010002

#define VIDEO_E_MISSING_MARKER_BIT	  0x80010001
#define VIDEO_E_NO_MARKER			  0x80010002
#define VIDEO_E_ERROR_VP_HEADER		  0x80010003
#define VIDEO_E_ERROR_MB_HEADER		  0x80010004
#define VIDEO_E_ERROR_GB_FLUSH		  0x80010005
#define VIDEO_E_MISSING_MB			  0x80010006
/* Version 2.0.0.0 modification, 2004.08.10 */
#define VIDEO_E_EXCEEDING_ONES_COUNT  0x80010007
#define VIDEO_E_EXCEEDING_ZEROS_COUNT 0x80010008
#define VIDEO_E_OUT_RVLD_TABLE        0x80010009
#define VIDEO_E_OUT_BLK_BOUNDARY      0x8001000A
#define VIDEO_E_ESCAPE_CODE_ERROR     0x8001000B
/* ======================================== */

/* Version 2.0.0.0 modification, 2004.08.10 */
#define VIDEO_E_NO_SUPPORT			  0x8001000C
/* ======================================== */
/* 2004.05.07 */
#define VIDEO_S_CONTROLLER_SUPPORT	  0x00010003
/* ========== */

#endif //_VIDEO_ERROR_
