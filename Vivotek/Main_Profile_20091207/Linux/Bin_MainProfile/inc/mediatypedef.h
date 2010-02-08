/*
 *******************************************************************************
 * $Header: /RD_1/Project/VNDP/MainProfile/common/src/mediatypedef.h 4     06/10/18 2:00p Bruce $
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
 * $History: mediatypedef.h $
 * 
 * *****************  Version 4  *****************
 * User: Bruce        Date: 06/10/18   Time: 2:00p
 * Updated in $/RD_1/Project/VNDP/MainProfile/common/src
 * 
 * *****************  Version 3  *****************
 * User: Bruce        Date: 05-11-25   Time: 15:37
 * Updated in $/RD_1/Project/VNDP/MainProfile/common/src
 * 
 * *****************  Version 2  *****************
 * User: Bruce        Date: 05-11-25   Time: 14:37
 * Updated in $/RD_1/Project/VNDP/MainProfile/common/src
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 05/07/26   Time: 1:48p
 * Created in $/RD_1/Project/VNDP/MainProfile/common/src
 * 
 * *****************  Version 3  *****************
 * User: Bill         Date: 04/10/29   Time: 7:35p
 * Updated in $/RD_1/Project/VNDP/MainProfile/common/src
 * 
 * *****************  Version 2  *****************
 * User: Bill         Date: 04/10/26   Time: 7:17p
 * Updated in $/RD_1/Project/VNDP/MainProfile/common/src
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 04/09/16   Time: 9:26p
 * Created in $/RD_1/Project/VNDP/MainProfile/common/src
 * 
 * *****************  Version 2  *****************
 * User: Yun          Date: 04/01/27   Time: 3:18p
 * Updated in $/rd_1/Project/TM1300_PSOS/FarSeer/common/src
 * Add AAC codec type
 * 
 * *****************  Version 1  *****************
 * User: Yun          Date: 03/08/07   Time: 4:31p
 * Created in $/rd_1/Project/TM1300_PSOS/FarSeer/common/src
 * Move codec type and frame type from datapacketdef.h to this file
 * 
 *******************************************************************************
 */

/*!
 *******************************************************************************
 * Copyright 2000-2002 Vivotek, Inc. All rights reserved.
 *
 * \file
 * mediatypedef.h
 *
 * \brief
 * header file of media type related
 *
 * \date
 * 2003/08/07
 *
 * \author
 * May Hsu
 *
 *******************************************************************************
 */

#ifndef __MEDIATYPEDEF_H__
#define __MEDIATYPEDEF_H__

/*! The media codec type */
typedef enum {
	/*! the codec type is JPEG (image, video) */ 
	mctJPEG   = 0x0001,
	
	/*! the codec type is H263 (video) */
	mctH263   = 0x0002,
	
	/*! the codec type is MPEG-4 video (video) */
	mctMP4V   = 0x0004,

	/*! the codec type is H264 video (video) */
	mctH264   = 0x0008,

	
   /*! the codec type is DMYV video dummy (video) */ //bruce add for video dummy codec 20061013
	mctDMYV   = 0x00FF,
	

	/*! the codec type is G.722.1 (audio) */
	mctG7221  = 0x0100,
	
	/*! the codec type is G.729A (audio) */
	mctG729A  = 0x0200,
	
	/*! the codec type is AAC (audio) */
	mctAAC	  = 0x0400,
	
	/*! the codec type is AMR (audio) */
	mctGAMR	  = 0x0800,

	mctSAMR	  = 0x1000,

	mctG711 = 0x2000,

	mctG711A = 0x4000,
   
	/*! the codec type is DMYA audio dummy (audio) */ //bruce add for audio dummy codec 20061013
	mctDMYA	  = 0xFF00,
	/*! the codec type is IVA (intelligent video) */
	mctIVA1   = 0x010000,
	
	mctALLCODEC = 0xFFFF,

} EMediaCodecType;

/*! the type of frame */
typedef enum
{	
	/*! the frame is intra */
	MEDIADB_FRAME_INTRA = 0,
	/*! the frame is prediction */
	MEDIADB_FRAME_PRED = 1,	
	/*! the frame is bi-direction prediction */
	MEDIADB_FRAME_BIPRED = 2,
} TMediaDBFrameType;

typedef enum
{
	VIVOUDTTAG_PIR					= 0x00,
	VIVOUDTTAG_TEMPERATURE_INFO		= 0x01,
	VIVOUDTTAG_CAPWIN_INFO			= 0x10,
	VIVOUDTTAG_MOTIONW_INNFO		= 0x11,
	VIVOUDTTAG_TEMPERING_INFO		= 0x18,
} TVivoUsrDataExtTag;

#endif // __MEDIATYPEDEF_H__
