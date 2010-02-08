/*
 *******************************************************************************
 * $Header: /RD_1/Project/VNDP/MainProfile/DataBroker/packetizer/src/packetizer.h 1     05/07/26 2:03p Perkins $
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
 * $History: packetizer.h $
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 05/07/26   Time: 2:03p
 * Created in $/RD_1/Project/VNDP/MainProfile/DataBroker/packetizer/src
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 04/09/17   Time: 8:46a
 * Created in $/RD_1/Project/VNDP/MainProfile/DataBroker/packetizer/src
 * 
 * *****************  Version 1  *****************
 * User: Sylvia       Date: 03/10/31   Time: 2:20p
 * Created in $/rd_1/Application/VNDP/Windows/MainProfile/Src/DataBroker/include
 * using new version streaming client module
 * 
 * *****************  Version 4  *****************
 * User: Yun          Date: 03/08/20   Time: 11:33a
 * Updated in $/rd_1/System/packetizer/src
 * Add __stdcall in callback function
 * 
 * *****************  Version 3  *****************
 * User: Yun          Date: 03/08/07   Time: 5:50p
 * Updated in $/rd_1/System/packetizer/src
 * Add an output parameter *pdwPacketCount in MediaPacket_Input to report
 * the total network packet count it divides into.
 * 
 * *****************  Version 2  *****************
 * User: Yun          Date: 03/08/06   Time: 11:44a
 * Updated in $/rd_1/System/packetizer/src
 * Use new definition NETWORK_PACKET_SIZE
 * 
 * *****************  Version 1  *****************
 * User: Yun          Date: 03/02/19   Time: 6:35p
 * Created in $/rd_1/System/packetizer/src
 * 5000 Series Ready
 *******************************************************************************
 */

/*!
 *******************************************************************************
 * Copyright 2000-2003 Vivotek, Inc. All rights reserved.
 *
 * \file
 * packetizer.h
 *
 * \brief
 * Include file for media packetizer.
 *
 * \date
 * 2003/02/11
 *
 * \author
 * Mei-Yun Hsu
 *
 *
 *******************************************************************************
 */

#ifndef __PAKCETIZER_H__
#define __PAKCETIZER_H__

#include "typedef.h"
#include "common.h"
#include "errordef.h"
#include "networkpacketdef.h"

#define MEDIAPACKET_VERSION	MAKEFOURCC( 2, 0, 0, 1)

/*!
 *******************************************************************************
 * \brief
 * This callback function is used to output a network packet.
 *
 * \param dwInstance
 * \a (i) the instance of callback handler.
 *
 * \param dwPacketSize
 * \a (i) the size of Data Packet.
 *
 * \param pbyOutBuf
 * \a (i) the pointer of output buffer.
 *
 * \see MediaPacket_SetReleaseBufCallback
 *
 **************************************************************************** */
typedef SCODE (__stdcall * FMediaPacketReleaseBuf) (
	DWORD dwInstance,
	DWORD dwPacketSize,
	BYTE *pbyOutBuf
);

/*!
 *******************************************************************************
 * \brief
 * This callback function is used to request a buffer to put one network packet.
 * It returns the pointer of buffer.
 *
 * \param dwInstance
 * \a (i) the instance of callback handler.
 *
 * \see MediaPacket_SetRequestBufCallback
 *
 **************************************************************************** */
typedef void* (__stdcall * FMediaPacketRequestBuf) (DWORD dwInstance);


/*! A data structure used in function MediaPacket_Initial to set the initial options. */
typedef struct {
	
    /*! The is a version control parameter. Set this value to
     * \b MEDIAPACKETIZER_VERSION. */
    DWORD dwVersion;	

} TMediaPacketInitOptions;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *******************************************************************************
 * \brief
 * Create an instance of media packetizer and initialize it.
 *
 * \param phObject
 * \a (o) pointer to receive the instance's handle of this object.
 *
 * \param ptInitOptions
 * \a (i) the pointer of data structure \b TMediaPacketInitOptions for setting
 * the initialization parameters of the media packetizer instance. If you did not
 * set some parameters which are needed, the default value will be set.
 *
 * \retval S_OK
 * Initialize media packetizer OK.
 * \retval S_FAIL
 * Initialize media packetizer failed.
 *
 * \remark 
 *
 * \see MediaPacket_Release
 *
 **************************************************************************** */
SCODE DLLAPI MediaPacket_Initial (HANDLE *phObject, TMediaPacketInitOptions *ptInitOptions);

/*!
 *******************************************************************************
 * \brief
 * Delete an instance of the media packetizer object.
 *
 * \param phObject
 * \a (i/o) the pointer to the handle of the media packetizer instance.
 *
 * \retval S_OK
 * Release media packetizer OK.
 * \retval S_FAIL
 * Release media packetizer failed.
 *
 * \remark
 * After the instance is released, the handle of this instance will be set to
 * zero.
 *
 * \see MediaPacket_Initial
 *
 **************************************************************************** */
SCODE DLLAPI MediaPacket_Release (HANDLE *phObject);


/*!
 *******************************************************************************
 * \brief
 * Input a Data Packet to the media packetizer. The packetizer will request a 
 * buffer to put a network packet and release it when the network packet is 
 * ready through callback functions.
 *
 * \param hObject
 * \a (i) a handle of media packetizer instance.
 *
 * \param pbyBuf
 * \a (i) the pointer of buffer containing a Data Packet.
 *
 * \param dwBufSize
 * \a (i) the size of a Data Packet.
 *
 * \param pdwPacketCount
 * \a (o) the pointer of total packet count
 *
 * \retval S_OK
 * Input the network stream OK.
 * \retval S_FAIL
 * Input the network stream failed.
 *
 * \remark
 *
 **************************************************************************** */
SCODE DLLAPI MediaPacket_Input (HANDLE hObject, BYTE *pbyBuf, DWORD dwBufSize, DWORD *pdwPacketCount);

/*!
 *******************************************************************************
 * \brief
 * Set the callback function of request buffer for media packetizer.
 *
 * \param hObject
 * \a (i) a handle of media packetizer instance.
 *
 * \param pfnCallback
 * \a (i) the pointer of callback function.
 *
 * \param dwInstance
 * \a (i) the instance used in callback function. 
 *
 * \retval S_OK
 * Set callback function OK.
 * \retval S_FAIL
 * Set callback function failed.
 *
 **************************************************************************** */
SCODE DLLAPI MediaPacket_SetRequestBufCallback (HANDLE hObject, void *pfnCallback, DWORD dwInstance);

/*!
 *******************************************************************************
 * \brief
 * Set the callback function of release buffer for media packetizer.
 *
 * \param hObject
 * \a (i) a handle of media packetizer instance.
 *
 * \param pfnCallback
 * \a (i) the pointer of callback function.
 *
 * \param dwInstance
 * \a (i) the instance used in callback function. 
 *
 * \retval S_OK
 * Set callback function OK.
 * \retval S_FAIL
 * Set callback function failed.
 *
 **************************************************************************** */
SCODE DLLAPI MediaPacket_SetReleaseBufCallback (HANDLE hObject, void *pfnCallback, DWORD dwInstance);

/*!
 *******************************************************************************
 * \brief
 * Set the sequence number of the first network packet. If this function is not
 * called, the sequence number will start from zero.
 *
 * \param hObject
 * \a (i) a handle of media packetizer instance.
 *
 * \param wSequneceNum
 * \a (i) the sequence number of the first network packet.
 *
 * \param eMediaType
 * \a (i) the media type to set sequence number
 *
 * \retval S_OK
 * Reset sequence number OK.
 * \retval S_FAIL
 * Reset sequence number failed.
 *
 **************************************************************************** */
SCODE DLLAPI MediaPacket_ResetSequenceNumber (HANDLE hObject, WORD wSequenceNum, EMediaPacketMediaType eMediaType);

#ifdef __cplusplus
}
#endif

#endif // __PAKCETIZER_H__
