/*****************************************************************************
*  Copyright C 2008 Stretch, Inc. All rights reserved. Stretch products are  *
*  protected under numerous U.S. and foreign patents, maskwork rights,       *
*  copyrights and other intellectual property laws.                          *
*                                                                            *
*  This source code and the related tools, software code and documentation,  *
*  and your use thereof, are subject to and governed by the terms and        *
*  conditions of the applicable Stretch IDE or SDK and RDK License Agreement *
*  (either as agreed by you or found at www.stretchinc.com). By using these  *
*  items, you indicate your acceptance of such terms and conditions between  *
*  you and Stretch, Inc. In the event that you do not agree with such terms  *
*  and conditions, you may not use any of these items and must immediately   *
*  destroy any copies you have made.                                         *
*****************************************************************************/

#ifndef __VENC264_AUTHENTICATE_H__
#define __VENC264_AUTHENTICATE_H__

//#include <sx-types.h>
#ifndef __H_SXTYPES
#define __H_SXTYPES


/****************************************************************************
    8-bit unsigned integer.
****************************************************************************/
typedef unsigned char  sx_uint8;

/****************************************************************************
    16-bit unsigned integer.
****************************************************************************/
typedef unsigned short sx_uint16;

/****************************************************************************
    32-bit unsigned integer.
****************************************************************************/
typedef unsigned int   sx_uint32;

/****************************************************************************
    64-bit unsigned integer.
****************************************************************************/
typedef unsigned long long sx_uint64;

/****************************************************************************
    Boolean value.
****************************************************************************/
typedef unsigned int   sx_bool;

/****************************************************************************
    true is a non-zero value
****************************************************************************/

#ifndef true
#define true 1
#endif

/****************************************************************************
    false is zero value
****************************************************************************/

#ifndef false
#define false 0
#endif

/****************************************************************************
    8-bit signed integer.
****************************************************************************/
typedef signed char               sx_int8;     // 8  bit signed integer

/****************************************************************************
    16-bit signed integer.
****************************************************************************/
typedef  short              sx_int16;    // 16 bit signed integer

/****************************************************************************
    32-bit signed integer.
****************************************************************************/
typedef  int                sx_int32;    // 32 bit signed integer

/****************************************************************************
    64-bit signed integer.
****************************************************************************/
typedef  long long          sx_int64;    // 64 bit signed integer 
#endif

//  This structure holds authentication information for an h.264 frame.  It may be written
//  into the bitstream as unregistered user data, to be used later by the verification program

#define VENC264_AUTHENTICATE_CURRENT_VERSION 1
#define VENC264_AUTHENTICATE_MAX_KEY_LENGTH 64
#define VENC264_AUTHENTICATE_INSIDE_KEY_BASE 0x36
#define VENC264_AUTHENTICATE_OUTSIDE_KEY_BASE 0x5c
 typedef struct
 {
	 unsigned char authenticationVersion;   //  Version of authentication method (filled in by authenticate function)
	 sx_uint16 channelID;					//  DVR channel ID
	 unsigned char reserved[1];            	//  Make the structure 32 bytes
	 sx_uint64 timestamp;					//  Timestamp of frame
	 sx_uint32 frameNumber;					//  Number of frame in sequence
	 unsigned char signature[20];			//  HMAC signature (filled in by authenticate function)
 } venc264_authenticate_t;

sx_uint32 venc264_authenticate(unsigned char *buf, sx_uint32 isize, sx_uint32 maxoutsize, 
							   venc264_authenticate_t *pAuthenticateData, unsigned char *pKey, 
							   sx_uint32 keyLength);
 #endif
