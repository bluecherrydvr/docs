/*
 *******************************************************************************
 * $Header: /RD_1/Project/VNDP/MainProfile/networkpacketdef/src/networkpacketdef.h 2     05/09/13 12:18p Perkins $
 *
 *  Copyright (c) 2000-2003 Vivotek Inc. All rights reserved.
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
 * $History: networkpacketdef.h $
 * 
 * *****************  Version 2  *****************
 * User: Perkins      Date: 05/09/13   Time: 12:18p
 * Updated in $/RD_1/Project/VNDP/MainProfile/networkpacketdef/src
 * add new type
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 05/07/26   Time: 1:49p
 * Created in $/RD_1/Project/VNDP/MainProfile/networkpacketdef/src
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 04/09/17   Time: 8:53a
 * Created in $/RD_1/Project/VNDP/MainProfile/networkpacketdef/src
 * 
 * *****************  Version 2  *****************
 * User: Sylvia       Date: 03/10/31   Time: 2:20p
 * Updated in $/rd_1/Application/VNDP/Windows/MainProfile/Src/DataBroker/include
 * using new version streaming client module
 *
 *******************************************************************************
 */

/*!
 *******************************************************************************
 * Copyright 2000-2003 Vivotek, Inc. All rights reserved.
 *
 * \file
 * networkpacketdef.h
 *
 * \brief
 * header file of network packet enumeration
 *
 * \date
 * 2003/02/12
 *
 * \author
 * Mei-Yun Hsu
 *
 *******************************************************************************
 */
/* =========================================================================================== */
#ifndef __NETWORKPACKETDEF_H__
#define __NETWORKPACKETDEF_H__

/*! The media type of network packet */
typedef enum {
	/*! the media is audio */
	mpmtAudio = 1,
	
	/*! the media is video*/
	mpmtVideo = 2,
	
	/*! the media is user data */
	mpmtUserData = 3,

} EMediaPacketMediaType;

/*! The flag type */
typedef enum {
	mpftWholePacket = 0,
	mpftFirstPacket = 1,
	mpftLastPacket = 2,
	mpftOtherPacket = 3
}EMediaPacketFlagType;

typedef struct {
	WORD					wSequenceNum;
	EMediaPacketMediaType	empMediaType;
	EMediaPacketFlagType	empFlagType;
}TMediaPacketHeaderInfo;


#endif //__NETWORKPACKETDEF_H__
