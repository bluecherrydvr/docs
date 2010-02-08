/*
 *******************************************************************************
 * $Header: /RD_1/Project/VNDP/MainProfile/common/src/datapacketdef.h 1     05/07/26 1:48p Perkins $
 *
 *  Copyright (c) 2000-2002 Vivotek Inc. All rights reserved.
 *
 *  +-----------------------------------------------------------------+
 *  | THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY ONLY BE USED |
 *  | AND COPIED IN ACCORDANCE WITH THE TERMS AND CONDITIONS OF SUCH  |
 *  | A LICENSE AND WITH THE INCLUSION OF THE THIS COPY RIGHT NOTICE. |
 *  | THIS SOFTWARE OR ANY OTHER COPIES OF THIS SOFTWARE MAY NOT BE   |
 *  | PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY OTHER PERSON. THE   |
 *  | OWNERSHIP AND TITLE OF THIS SOFTWARE IS NOT TRANSFERRED.        |
 *  |                                                                 |
 *  | THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT   |
 *  | ANY PRIOR NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY |
 *  | VIVOTEK INC.                                                    |
 *  +-----------------------------------------------------------------+
 *
 * $History: datapacketdef.h $
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 05/07/26   Time: 1:48p
 * Created in $/RD_1/Project/VNDP/MainProfile/common/src
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 04/09/16   Time: 9:26p
 * Created in $/RD_1/Project/VNDP/MainProfile/common/src
 * 
 * *****************  Version 6  *****************
 * User: Perkins      Date: 04/03/26   Time: 20:52
 * Updated in $/rd_1/Application/VNDP/Windows/MainProfile/Src/DataBroker/include
 * new 2K/3K sync module
 * 
 * *****************  Version 4  *****************
 * User: Yun          Date: 04/03/15   Time: 10:24a
 * Updated in $/rd_1/Project/TM1300_PSOS/FarSeer/datapacket/src
 * Add bNoVideoSignal field
 * 
 * *****************  Version 3  *****************
 * User: Yun          Date: 03/11/18   Time: 1:50p
 * Updated in $/rd_1/Project/TM1300_PSOS/FarSeer/datapacket/src
 * Add timemodified field and audioDI field
 * 
 * *****************  Version 2  *****************
 * User: Yun          Date: 03/08/11   Time: 9:35a
 * Updated in $/rd_1/Project/TM1300_PSOS/FarSeer/datapacket/src
 * Move media codec type and frame type to mediatypedef.h
 * 
 * *****************  Version 1  *****************
 * User: Yun          Date: 03/07/28   Time: 3:44p
 * Created in $/rd_1/Project/TM1300_PSOS/FarSeer/datapacket/src
 *
 *******************************************************************************
 */

/*!
 *******************************************************************************
 * Copyright 2000-2002 Vivotek, Inc. All rights reserved.
 *
 * \file
 * datapacketdef.h
 *
 * \brief
 * header file of data packet structure
 *
 * \date
 * 2003/01/09
 *
 * \author
 * May Hsu
 *
 *******************************************************************************
 */
/* =========================================================================================== */
#ifndef __DATAPACKETDEF_H__
#define __DATAPACKETDEF_H__

#include "typedef.h"
#include "mediatypedef.h"

typedef struct
{
 	/*! the flags of motion detection */
	BOOL		bMotionDetection[3];
	/*! the flags of motion detection alert */
	BOOL		bMotionDetectionAlertFlag[3];
	/*! the percentage of motion detection */
	BYTE		byMotionDetectionPercent[3];
 	/*! the window axis of motion detection */
	WORD		wMotionDetectionAxis[3][4];
} TMotionTriggerInfo;
/*!
* A structure provides the information in the Data Packet
*/
typedef struct
{	
	/*! the pointer of buffer containing this Data Packet */
	BYTE		*pbyBuff;
	/*! the offset of buffer refer to the media bitstream, 32-bits align */
	DWORD		dwOffset;
	/*! the size of media bitstream */
	DWORD		dwBitstreamSize;
	/*! the type of stream in the Data Packet */
	DWORD		dwStreamType;
	/*! the number of frame in the Data Packet */
	DWORD		dwFrameNumber;
	/*! the type of frame */
	TMediaDBFrameType	tFrameType;
	/*! the second of first frame */
	DWORD		dwFirstUnitSecond;
 	/*! the millisecond of first frame */
	DWORD		dwFirstUnitMilliSecond;
 	/*! the flag indicates the frame size is fixed */
	BOOL		bFixedFrameSize;
 	/*! audio sampling frequency */
	DWORD		dwAudioSamplingFreq;
	/*! the channel number of audio */
	BYTE		byAudioChannelNum;
	/*! the digital input alert, each bit presents a digital input source. Bit 0 is for DI 0, bit 1 is for DI 1, ¡K */
	DWORD		dwDIAlert;
	/*! the digital output, each bit presents a digital output. Bit 0 is for DO 0, bit 1 is for DO 1 */
	DWORD		dwDO;
 	/*! the flags of motion detection */
	BOOL		bMotionDetection[3];
	/*! the flags of motion detection alert */
	BOOL		bMotionDetectionAlertFlag[3];
	/*! the percentage of motion detection */
	BYTE		byMotionDetectionPercent[3];
 	/*! the window axis of motion detection */
	WORD		wMotionDetectionAxis[3][4];
	/*! the flag indicates the time is modified according to timezone */
	BOOL        bTimeModified;
	/*! the flag indicates audio packets take the DI Alert information */
	BOOL        bAudioDI;
	/*! the flag indicates the loss of video signal */
	BOOL		bNoVideoSignal;
} TMediaDataPacketInfo;

struct _TMediaDataPacketInfoV3;

typedef SCODE (__stdcall *PFMediaPacketAddRef)(struct _TMediaDataPacketInfoV3 *ptMediaDataPacket);
typedef SCODE (__stdcall *PFMediaPacketRelease)(struct _TMediaDataPacketInfoV3 *ptMediaDataPacket);

typedef struct 
{   
	PFMediaPacketAddRef		pfAddRef;	
	PFMediaPacketRelease	pfRelease;

} TMediaDataPacketFuntionTable;

typedef struct
{
	BYTE	*pbyTLVExt;		// the pointer to the tag/length/data extension of a frame
							// the pointer would point to the location after media data in pbyBuff
	DWORD	dwTLVExtLen;	// the length of the tag length
	DWORD	dwStructSize;		// the size of the structure. If this is 0, it means the packet is Ex only, else this maps the size of the overall packet structure
	
	TMediaDataPacketFuntionTable* ptFunctionTable;  
} TMediaPacketFirstReserved;


typedef union
{
	TMediaPacketFirstReserved	tExt;
	void *						apvReserved[4];	// because we do not release 64 bit library yet, so the change will cause no harm
} UMeidaPktReserved1;

typedef struct
{
	BOOL	bTimeZone;			// if the packet contains time zone information, if no, the following fields are from client machine
	long	lDLSaving;			// the daylight saving time in seconds (if 0 it means no day-light saving), -3600 for most case
	long	lOffsetSeconds;		// offset of seconds from GMT time; for example taipei will be 8 * 3600 = 28000
} TMediaPacketTimeZoneInfo;

/// the new structure used in AV callback
/// when AV callback, please cast the TMediaDataPacketInfo * to TMediaDataPacketInfoEx *
typedef struct
{   ///The handle of DataBroker instance
	TMediaDataPacketInfo tInfo;
	///Reserved for future use
	UMeidaPktReserved1 tRv1;

	/// the with of the video frame if it's video (includes padding if any)
	DWORD	dwWidth;	
	/// the height of the video frame if it's video (includes padding if any)
	DWORD	dwHeight;	
	/// padded width, usually this is 0
	DWORD	dwWidthPadLeft;	
	/// padded width, usually this is 0
	DWORD	dwWidthPadRight;
	/// padded height on top, usually this is 0
	DWORD	dwHeightPadTop;	
	/// padded height on bottom, usually this is 0
	DWORD	dwHeightPadBottom; 
	/// Reserved for future use
	//char	szReserved2[16];
} TMediaDataPacketInfoEx;

typedef struct
{
	BOOL	bWithInfo;
	WORD	wCapW;
	WORD	wCapH;
	WORD	wOffX;
	WORD	wOffY;
	WORD	wCropW;
	WORD	wCropH;
} TCaptureWinInfo;

typedef struct
{
	DWORD						dwPIR;				// the PIR status, this field is not set yet in current stage, waiting for
													// HIGH word means the PIR enabled flag, LOW word contains the values
	DWORD						dwWLLed;			// the status for white light LED in some model
	TCaptureWinInfo				tCapWinInfo;		// it contains the capture window info, current not used yet
	DWORD						dwTamperingAlert;	// it contains the tamperingalert info, current not used yet
	TMotionTriggerInfo			*ptMTI;				// point to the data in TMediaDataPacketInfo
} TVUExtInfo;


typedef struct _TMediaDataPacketInfoV3
{   
	///The handle of DataBroker instance
	TMediaDataPacketInfoEx tIfEx;

	TMediaPacketTimeZoneInfo	tTZ;
	DWORD						dwUTCTime;			// this is a redundant filed, it could be got from dwFristUnitSecond and time zone information

	TVUExtInfo					tVUExt;

	BYTE						*pbyVExtBuf;		// video further extension, it points to some point in pbyBuff, if null, means no such extension
	DWORD						dwVExtLen;
	
	// if there are any new member, it goes here
	BOOL						bTemperatureAlert;	//the information about temperature alter
} TMediaDataPacketInfoV3;


typedef struct
{
	DWORD			dwTag;
	DWORD			dwLen;
	DWORD			dwUnused;
	DWORD			dwConsumed;
	BYTE			byD;		// internal used
	BYTE			*pbyP;
} TTLVParseUnit;

#endif //__DATAPACKETDEF_H__
