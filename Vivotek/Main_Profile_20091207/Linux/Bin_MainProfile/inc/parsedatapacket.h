/*
 *******************************************************************************
 * $Header: /RD_1/Project/VNDP/MainProfile/parsedatapacket/src/parsedatapacket.h 4     06/10/18 2:00p Bruce $
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
 * $History: parsedatapacket.h $
 * 
 * *****************  Version 4  *****************
 * User: Bruce        Date: 06/10/18   Time: 2:00p
 * Updated in $/RD_1/Project/VNDP/MainProfile/parsedatapacket/src
 * 
 * *****************  Version 3  *****************
 * User: Bruce        Date: 05-11-25   Time: 15:27
 * Updated in $/RD_1/Project/VNDP/MainProfile/parsedatapacket/src
 * 
 * *****************  Version 2  *****************
 * User: Steven       Date: 05/09/16   Time: 5:37p
 * Updated in $/RD_1/Project/VNDP/MainProfile/parsedatapacket/src
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 05/07/26   Time: 2:04p
 * Created in $/RD_1/Project/VNDP/MainProfile/parsedatapacket/src
 * 
 * *****************  Version 2  *****************
 * User: Allatin      Date: 05/02/03   Time: 10:24p
 * Updated in $/RD_1/Project/VNDP/MainProfile/parsedatapacket/src
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 04/09/16   Time: 9:32p
 * Created in $/RD_1/Project/VNDP/MainProfile/parsedatapacket/src
 * 
 * *****************  Version 13  *****************
 * User: Weicheng     Date: 04/03/15   Time: 2:13p
 * Updated in $/rd_1/System/parsedatapacket/src
 * Add bNoVideoSignal field
 * 
 * *****************  Version 12  *****************
 * User: Weicheng     Date: 04/01/29   Time: 11:09a
 * Updated in $/rd_1/System/parsedatapacket/src
 * 1. Add AAC codec type
 * 2. Move codec type from datapacketdef.h and frame type from common.h to
 * mediatypedef.h
 * 
 * *****************  Version 11  *****************
 * User: Yun          Date: 03/11/18   Time: 6:25p
 * Updated in $/rd_1/System/parsedatapacket/src
 * Port to Trimedia
 * 
 * *****************  Version 10  *****************
 * User: Weicheng     Date: 03/11/10   Time: 2:55p
 * Updated in $/rd_1/System/parsedatapacket/src
 * Variable " bTimeModified " and "bAudioDI" in structure
 * "TMediaDataPacketInfo" are added
 * 
 * *****************  Version 9  *****************
 * User: Weicheng     Date: 03/08/22   Time: 4:14p
 * Updated in $/rd_1/System/parsedatapacket/src
 * get new time format no mater the modified time flag is  0 or 1
 * 
 * *****************  Version 8  *****************
 * User: Weicheng     Date: 03/07/30   Time: 4:53p
 * Updated in $/rd_1/System/parsedatapacket/src
 * Variable "dwDO" in structure "TMediaDataPacketInfo" is added
 * 
 * *****************  Version 7  *****************
 * User: Weicheng     Date: 03/07/11   Time: 2:16p
 * Updated in $/rd_1/System/parsedatapacket/src
 * initialize the structure "TMediaDataPacketInfo" in DataPacket_Parse
 * 
 * *****************  Version 6  *****************
 * User: Weicheng     Date: 03/07/08   Time: 7:31p
 * Updated in $/rd_1/System/parsedatapacket/src
 * Variable "dwDIMask" in structure "TMediaDataPacketInfo" is removed
 * 
 * *****************  Version 5  *****************
 * User: Weicheng     Date: 03/02/14   Time: 6:50p
 * Updated in $/rd_1/System/parsedatapacket/src
 * following the rule of SOP(Standard Operation Procedures)
 * 
 * *****************  Version 4  *****************
 * User: Weicheng     Date: 03/02/13   Time: 3:44p
 * Updated in $/rd_1/System/parsedatapacket/src
 * modified file format
 * 
 * *****************  Version 3  *****************
 * User: Weicheng     Date: 03/01/10   Time: 9:31a
 * Updated in $/rd_1/System/parsedatapacket/src
 * separate the common definitions to datapacketdef.h
 * 
 * *****************  Version 2  *****************
 * User: Weicheng     Date: 03/01/09   Time: 1:10p
 * Updated in $/rd_1/System/parsedatapacket/src
 * added DI alert
 * 
 * *****************  Version 1  *****************
 * User: Weicheng     Date: 02/12/13   Time: 11:44a
 * Created in $/rd_1/System/ParseDataPacket/src
 *
 *******************************************************************************
 */

/*!
 *******************************************************************************
 * Copyright 2000-2002 Vivotek, Inc. All rights reserved.
 *
 * \file
 * parsedatapacket.h
 *
 * \brief
 * header file of data packet parser
 *
 * \date
 * 2002/12/12
 *
 * \author
 * Wei-Cheng Lin
 *
 *
 *******************************************************************************
 */
/* =========================================================================================== */
#ifndef __PARSEDATAPACKET_H__
#define __PARSEDATAPACKET_H__

/* =========================================================================================== */
#include "typedef.h"
#include "errordef.h"
#include "datapacketdef.h"
#include "common.h"

#define PARSEDATAPACKET_VERSION   MAKEFOURCC(5, 1, 0, 5)

#define	IS_PKTINFO_PTR_V3(a)	(((TMediaDataPacketInfoEx *) a)->tRv1.tExt.dwStructSize == sizeof(TMediaDataPacketInfoV3))
#define	IS_PKTINFO_VAL_V3(a)	IS_PKTINFO_PTR_V3(&a)
#define	INIT_PKTINFO_PTR_V3(a)	(a)->tIfEx.tRv1.tExt.dwStructSize = sizeof(TMediaDataPacketInfoV3)
#define	INIT_PKTINFO_VAL_V3(a)	INIT_PKTINFO_PTR_V3(&a)

// Utility to get extension pointer
#define	PKPARSE_V3_GET_EXT_PTR(a)	(a)->tIfEx.tRv1.tExt.pbyTLVExt
#define	PKPARSE_V3_GET_EXT_LEN(a)	(a)->tIfEx.tRv1.tExt.dwTLVExtLen

#define	PKPARSE_V3_GET_EXT_PTR2(a)	(a).tIfEx.tRv1.tExt.pbyTLVExt
#define	PKPARSE_V3_GET_EXT_LEN2(a)	(a).tIfEx.tRv1.tExt.dwTLVExtLen

#define	PKPARSE_EX_GET_EXT_PTR(a)	(a)->tRv1.tExt.pbyTLVExt
#define	PKPARSE_EX_GET_EXT_LEN(a)	(a)->tRv1.tExt.dwTLVExtLen

#define	PKPARSE_EX_GET_EXT_PTR2(a)	(a).tRv1.tExt.pbyTLVExt
#define	PKPARSE_EX_GET_EXT_LEN2(a)	(a).tRv1.tExt.dwTLVExtLen

/*!
* A structure provides the information of audio
*/
typedef struct
{
	/*! the size of audio unit */
	DWORD			dwSize;
} TMediaAudioInfo;

#ifdef __cplusplus
extern "C" {
#endif

/*!
******************************************************************************
* \brief
* This function parse the Data Packet in the input buffer.
*
* \param ptDataPacketInfo
* \a (i/o) the pointer to the data structure storing the information about Data Packet
*
* \retval S_OK
* Parse the Data Packet successful.
*
* \retval S_FAIL
* Parse the Data Packet failed.
*
* \remark
* NONE
*
* \see 
*
******************************************************************************
*/
SCODE DLLAPI DataPacket_Parse(TMediaDataPacketInfo *ptDataPacketInfo);

/*!
******************************************************************************
* \brief
* This function get the size and buffer address of every audio unit in a Data Packet.
*
* \param pbyInBuf
* \a (i) the pointer of buffer containing an audio Data Packet
*
* \param ptAudionfo
* \a (o) the pointer of structure to store the list of audio information, the 
* size must be larger than audionumber * sizeof(TMediaDBAudioInfo)
*
* \retval S_OK
* Get the fields successful.
*
* \retval S_FAIL
* Get the fields failed.
*
* \remark
* NONE
*
* \see 
*
******************************************************************************
*/
SCODE DLLAPI DataPacket_GetEveryAudioInfo(BYTE *pbyInBuf, TMediaAudioInfo *ptAudionfo);

/*!
******************************************************************************
* \brief
* This function parse the extended Data Packet in the buffer, and put the data into TMediaDataPacketInfoV3.
*
* \param ptDataPacketInfoV3
* \a (i/o) the pointer to the data structure storing the information about Data Packet. Before calling this function,
* the pbyBuff of tInfo if ptDataPacketInfoEx must be set to the media data (with extension) and the dwStructSize member
* should be set
*
* \retval S_OK
* Parse the Data Packet successful.
*
* \retval S_FAIL
* Parse the Data Packet failed.
*
* \remark
* NONE
*
* \see 
*
******************************************************************************
*/
SCODE DLLAPI DataPacket_ParseV3(TMediaDataPacketInfoV3 *ptDataPacketInfoV3);

/*!
******************************************************************************
* \brief
* This function parse the extended Data Packet in the buffer, and put the data into TMediaDataPacketInfoV3.
* This function will not check the extra flag which indicates if the time zone is correct
*
* \param ptDataPacketInfoV3
* \a (i/o) the pointer to the data structure storing the information about Data Packet. Before calling this function,
* the pbyBuff of tInfo if ptDataPacketInfoEx must be set to the media data (with extension) and the dwStructSize member
* should be set
*
* \retval S_OK
* Parse the Data Packet successful.
*
* \retval S_FAIL
* Parse the Data Packet failed.
*
* \remark
* NONE
*
* \see 
*
******************************************************************************
*/
SCODE DLLAPI DataPacket_ParseV3_NoCheckTimeZoneCorrect(TMediaDataPacketInfoV3 *ptDataPacketInfoV3);

/*!
******************************************************************************
* \brief
* This function parse the video extended data in traditional extension part
*
* \param pbyData
* \a (i) the pointer to the video extended data
* \param dwLen
* \a (i) the length of the extended data
* \param ptExt
* \a (i) the output of parsed structure
*
* \retval S_OK
* Parse the Data Packet successful.
*
* \retval S_FAIL
* Parse the Data Packet failed.
*
* \remark
* NONE
*
* \see 
*
******************************************************************************
*/
SCODE DLLAPI DataPacket_ParseVUExt(BYTE *pbyData, DWORD dwLen, TVUExtInfo *ptExt);

/*!
******************************************************************************
* \brief
* basic function to retrieve one unit from TLV encoded stream
* you must advance the stream pointer with ptUnit->dwConsumed to get unit of next tag
*
* \param pbyData
* \a (i) the pointer to the TLV data
* \param dwLen
* \a (i) the length of the TLV data
* \param ptUnit
* \a (o) the output of one unit of tag/leng/data
*
* \retval S_OK
* Parse the Data Packet successful.
*
* \retval S_FAIL
* Parse the Data Packet failed.
*
* \remark
* NONE
*
* \see 
*
******************************************************************************
*/
SCODE DataPacket_GetTLV_ExtLen(BYTE *pbyData, DWORD dwLen, TTLVParseUnit *ptUnit);

// the callback function when you use the parsing loop to parsing a TLV stream
typedef SCODE (__stdcall *TDataPacketParserTLVCB)(void *pvContext, TTLVParseUnit *ptUnit);

/*!
******************************************************************************
* \brief
* the parsing loop, you just pass the handling function pointer when a tag/leng/data is retrieved, the loop will
* keep parsing until end of data
*
* \param pbyData
* \a (i) the pointer to the video extended data
* \param dwLen
* \a (i) the length of the extended data
* \param pfCB
* \a (i) the callback function that will be called when a unit of tag/length/data is retrieved
* \param pvContext
* \a (i) the context that will be passed to the callback function when a unit of tag/length/data is retrieved
*
* \retval S_OK
* Parse the Data Packet successful.
*
* \retval S_FAIL
* Parse the Data Packet failed.
*
* \remark
* NONE
*
* \see 
*
******************************************************************************
*/
SCODE DLLAPI DataPacket_ParseLoop(BYTE *pbyData, DWORD dwLen, TDataPacketParserTLVCB pfCB, void *pvContext);

/*!
******************************************************************************
* \brief
* This function parse the temperature alert flag in video extended
*
* \param pbyData
* \a (i) the pointer to the video extended data
* \param dwLen
* \a (i) the length of the extended data
* \param pbTemperatureAlert
* \a (i) the output of parsed flag
*
* \retval S_OK
* Parse the Data Packet successful.
*
* \retval S_FAIL
* Parse the Data Packet failed.
*
* \remark
* NONE
*
* \see 
*
******************************************************************************
*/
SCODE DLLAPI DataPacket_ParseTemperatureAlert(BYTE *pbyData, DWORD dwLen, BOOL *pbTemperatureAlert);

#ifdef __cplusplus
}
#endif

#endif //__PARSEDATAPACKET_H__
