/*
 *******************************************************************************
 * $Header: /RD_1/Project/VNDP/MainProfile/DataBroker/unpacketizer/src/unpacketizer.h 4     06/09/15 6:00p Bruce $
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
 * $History: unpacketizer.h $
 * 
 * *****************  Version 4  *****************
 * User: Bruce        Date: 06/09/15   Time: 6:00p
 * Updated in $/RD_1/Project/VNDP/MainProfile/DataBroker/unpacketizer/src
 * 
 * *****************  Version 3  *****************
 * User: Steven       Date: 06/02/20   Time: 3:07p
 * Updated in $/RD_1/Project/VNDP/MainProfile/DataBroker/unpacketizer/src
 * 
 * *****************  Version 2  *****************
 * User: Bruce        Date: 05-11-25   Time: 14:17
 * Updated in $/RD_1/Project/VNDP/MainProfile/DataBroker/unpacketizer/src
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 05/07/26   Time: 2:26p
 * Created in $/RD_1/Project/VNDP/MainProfile/DataBroker/unpacketizer/src
 * 
 * *****************  Version 5  *****************
 * User: Perkins      Date: 05/05/24   Time: 5:32p
 * Updated in $/RD_1/Project/VNDP/MainProfile/DataBroker/unpacketizer/src
 * let RTSP extension and http mode could work
 * 
 * *****************  Version 4  *****************
 * User: Shengfu      Date: 05/04/15   Time: 2:40p
 * Updated in $/RD_1/Project/VNDP/MainProfile/DataBroker/unpacketizer/src
 * let AAC and GAMR both work for RTSP server
 * 
 * *****************  Version 3  *****************
 * User: Perkins      Date: 05/03/07   Time: 6:49p
 * Updated in $/RD_1/Project/VNDP/MainProfile/DataBroker/unpacketizer/src
 * add log to this module
 * 
 * *****************  Version 2  *****************
 * User: Yun          Date: 04/12/29   Time: 3:16p
 * Updated in $/RD_1/Project/VNDP/MainProfile/DataBroker/unpacketizer/src
 * 1. Remove MediaUnpacket_SetOptions function
 * 2. Add MediaUnpacket_SetOption function
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 04/09/17   Time: 8:44a
 * Created in $/RD_1/Project/VNDP/MainProfile/DataBroker/unpacketizer/src
 * 
 * *****************  Version 21  *****************
 * User: Yun          Date: 04/05/28   Time: 1:15p
 * Updated in $/rd_1/System/unpacketizer/src
 * Use new H263Trans.lib to sync the version of GBengine
 * 
 * *****************  Version 20  *****************
 * User: Yun          Date: 04/05/20   Time: 2:28p
 * Updated in $/rd_1/System/unpacketizer/src
 * Support real standard h263
 * 
 * *****************  Version 19  *****************
 * User: Yun          Date: 04/05/06   Time: 5:01p
 * Updated in $/rd_1/System/unpacketizer/src
 * 1. Fix the bug of missing start code when the start code left size =2
 * 2. Add SetOptions let it can output standard h263 bitstream
 * 
 * *****************  Version 18  *****************
 * User: Yun          Date: 04/05/06   Time: 1:13p
 * Updated in $/rd_1/System/unpacketizer/src
 * Fix the bug of not fill the NoVideoSignal field in Data Packet.
 * 
 * *****************  Version 17  *****************
 * User: Yun          Date: 04/04/01   Time: 4:58p
 * Updated in $/rd_1/System/unpacketizer/src
 * Fix the bug of not support variable length of DataPacket header for
 * JPEG
 * 
 * *****************  Version 16  *****************
 * User: Yun          Date: 04/03/31   Time: 10:11a
 * Updated in $/rd_1/System/unpacketizer/src
 *  Fix the buf of not handle the condition when HeaderLeftSize !=0, but
 * parse header failed. (due to packet loss)
 * 
 * *****************  Version 15  *****************
 * User: Yun          Date: 04/03/26   Time: 11:56a
 * Updated in $/rd_1/System/unpacketizer/src
 * 1. Add VideoSignal callback function
 * 2. Support new bitstream format of JPEG
 * 
 * *****************  Version 14  *****************
 * User: Yun          Date: 04/01/29   Time: 2:32p
 * Updated in $/rd_1/System/unpacketizer/src
 * Search start code first then search application header
 * 
 * *****************  Version 13  *****************
 * User: Yun          Date: 03/12/13   Time: 2:05p
 * Updated in $/rd_1/System/unpacketizer/src
 * Call DataPacket_InputVideoUserData when the codec type is JPEG.
 * 
 * *****************  Version 12  *****************
 * User: Yun          Date: 03/11/19   Time: 3:06p
 * Updated in $/rd_1/System/unpacketizer/src
 * Update composedatapacket library to verion 4.0.0.1
 * 
 * *****************  Version 11  *****************
 * User: Yun          Date: 03/08/26   Time: 4:19p
 * Updated in $/rd_1/System/unpacketizer/src
 * 1. Fix the bug of timezone
 * 2. Fix the bug without calling DataPacket_InputFrameSizeType
 * 
 * *****************  Version 10  *****************
 * User: Yun          Date: 03/08/21   Time: 5:13p
 * Updated in $/rd_1/System/unpacketizer/src
 * Fix the bug of timezone
 * 
 * *****************  Version 9  *****************
 * User: Weicheng     Date: 03/07/30   Time: 6:11p
 * Updated in $/rd_1/System/unpacketizer/src
 * add DO and modify interface for DO
 * 
 * *****************  Version 8  *****************
 * User: Yun          Date: 03/07/09   Time: 2:18p
 * Updated in $/rd_1/System/unpacketizer/src
 * Remove "dwDIMask" field in data structure TMediaDataPacketInfo.
 * 
 * *****************  Version 7  *****************
 * User: Yun          Date: 03/04/11   Time: 10:53a
 * Updated in $/rd_1/System/unpacketizer/src
 * Fix the bug of incorrect frame size of G.729A Data Packet
 * 
 * *****************  Version 6  *****************
 * User: Yun          Date: 03/03/28   Time: 7:03p
 * Updated in $/rd_1/System/unpacketizer/src
 * Add get version info function
 * 
 * *****************  Version 5  *****************
 * User: Yun          Date: 03/03/06   Time: 5:29p
 * Updated in $/rd_1/System/unpacketizer/src
 * Callback function add __stdcall
 * 
 * *****************  Version 4  *****************
 * User: Yun          Date: 03/02/20   Time: 10:19a
 * Updated in $/rd_1/System/unpacketizer/src
 * Add released buffer mode, setting output buffer offset
 * 
 * *****************  Version 3  *****************
 * User: Yun          Date: 03/02/15   Time: 6:21p
 * Updated in $/rd_1/System/unpacketizer/src
 * add checking buffer fullness & follow SOP
 * 
 * *****************  Version 2  *****************
 * User: Yun          Date: 03/02/12   Time: 9:47a
 * Updated in $/rd_1/System/unpacketizer/src
 * Modify error code and macro
 * 
 * *****************  Version 1  *****************
 * User: Yun          Date: 03/01/14   Time: 3:29p
 * Created in $/rd_1/System/unpacketizer/src
 * Support VNDP 2000/3000
 *******************************************************************************
 */

/*!
 *******************************************************************************
 * Copyright 2000-2002 Vivotek, Inc. All rights reserved.
 *
 * \file
 * unpacketizer.h
 *
 * \brief
 * Include file for media unpacketizer.
 *
 * \date
 * 2002/12/23
 *
 * \author
 * Mei-Yun Hsu
 *
 *
 *******************************************************************************
 */

#ifndef __UNPAKCETIZER_H__
#define __UNPAKCETIZER_H__

#include "typedef.h"
#include "common.h"
#include "errordef.h"
#include "MediaBuffer.h"
#include "networkpacketdef.h"

#define MEDIAUNPACKET_VERSION   MAKEFOURCC(7, 0, 0, 2)

/*! This enumeration indicates the operation mode of media unpacketizer, connecting to which
 kind of product. */
typedef enum {
	/*! connect to the present product using HTTP and the media type is JPEG */
	MEDIAUNPACKETOPMODE_HTTPJPEG	= 0,
	/*! connect to the present product using HTTP and the media type is H.263 */
	MEDIAUNPACKETOPMODE_HTTPH263	= 1,
	/*! connect to the present product using TCP or UDP and the media type is H.263 */
	MEDIAUNPACKETOPMODE_TCPUDPH263	= 2,
	/*! connect to the present product using TCP or UDP and the media type is G.722.1 */
	MEDIAUNPACKETOPMODE_TCPUDPG7221	= 3,
	/*! connect to the present product using TCP or UDP and the media type is G.729A */
	MEDIAUNPACKETOPMODE_TCPUDPG729A	= 4,
	/*! connect to new product with version 0 */
	MEDIAUNPACKETOPMODE_VER0 = 5,
	/*! connect to the 7k product using UDP */
	MEDIAUNPACKETOPMODE_RTSP	= 6,
	//MEDIAUNPACKETOPMODE_RTSPAUDIO	= 7,
	//ShengFu AAC/AMR
	MEDIAUNPACKETOPMODE_RTSPAUDIO_AMR = 7,
	MEDIAUNPACKETOPMODE_RTSPAUDIO_AAC = 8,	
	//Bruce Add for SAMR  
	MEDIAUNPACKETOPMODE_RTSPAUDIO_SAMR = 9,
	//Bruce add for Vivotek codec
   MEDIAUNPACKETOPMODE_RTSP_VIVOTEK = 10,
} TMediaUnpacketOPMode;

/*! This enumeration indicates the mode of buffer released. */
typedef enum {
	/*! The data in buffer released is to be continued. */
	MEDIAUNPACKETRBM_CONTINUED = 0,
	/*! The data in buffer released is finished. */
	MEDIAUNPACKETRBM_FINISHED  = 1,
}EMediaUnpacketReleaseBufMode;

typedef enum {
	/*! the video signal is restored */
	MEDIAUNPACKETVSC_RESTORE = 0,
	/*! the video signal is lost */
	MEDIAUNPACKETVSC_LOSS = 1,
}EMediaUnpacketVideoSignalChange;

typedef enum {
	// to set use standard h263 or not, dwParam1 is the value of enable (BOOL)
	MEDIAUNPACKETOPT_USESTANDARD263 = 0,
	// to set use dido from telnet, dwParam1 is the value of enable (BOOL)
	MEDIAUNPACKETOPT_TELNETDIDO		= 1,
	// to set DI and DO status, dwParam1 is the value of DI (DWORD), dwParam2 is the value of DO (DWORD)
	MEDIAUNPACKETOPT_SETDIDO		= 2,
	//ShengFu AAC/AMR
	MEDIAUNPACKETOPT_SET_AUDIOCODEC	= 3,
	MEDIAUNPACKETOPT_SET_VIDEOCODEC = 4,
}EMediaUnpacketOptionType;

/*!
 *******************************************************************************
 * \brief
 * This callback function is used to output a Data Packet.
 *
 * \param dwInstance
 * \a (i) the instance of callback handler.
 *
 * \param dwDataPacketSize
 * \a (i) the size of Data Packet if the mode is MEDIAUNPACKETRBM_FINISHED,
 *        the size of output buffer if the mode is MEDIAUNPACKETRBM_CONTINUED
 *
 * \param pbyOutBuf
 * \a (i) the pointer of output buffer.
 *
 * \param dwLostPacketNum
 * \a (i) the number of packets lost
 *
 * \param muMode
 * \a (i) the mode of release buffer
 *
 * \see MediaUnpacket_SetReleaseBufCallback
 *
 **************************************************************************** */
typedef SCODE (__stdcall *TOnMediaUnpacketReleaseBuf) (
	DWORD dwInstance,
	DWORD dwDataPacketSize,
	BYTE *pbyOutBuf,
	DWORD dwLostPacketNum,
	DWORD dwStartOffset,
	EMediaUnpacketReleaseBufMode muMode
);

/*!
 *******************************************************************************
 * \brief
 * This callback function is used to request a buffer to put one Data Packet.
 * It returns the pointer of buffer.
 *
 * \param dwInstance
 * \a (i) the instance of callback handler.
 * 
 * \param mpMediaType
 * \a (i) the mode of media type to request buffer
 *
 * \param pdwBufSize
 * \a (i/o) the pointer of buffer size, if the media type is mpmtAudio, it
 * indicates the buffer size it required, if the media type is mpmtVideo, it
 * indicates the buffer size the application provided.
 *
 * \param pdwBufOffset
 * \a (o) the pointer of buffer offset, the starting point to put the data
 *
 * \see MediaUnpacket_SetRequestBufCallback
 *
 **************************************************************************** */
typedef void* (__stdcall *TOnMediaUnpacketRequestBuf) (DWORD dwInstance,
											  EMediaPacketMediaType mpMediaType,
											  DWORD *pdwBufSize,
											  DWORD *pdwBufOffset);

/*!
 *******************************************************************************
 * \brief
 * This callback function is used to output the string of location when it changed.
 * It is used when connecting to the present products.
 *
 * \param dwInstance
 * \a (i) the instance of callback handler.
 *
 * \param szLocation
 * \a (i) the location string
 * 
 * \see MediaUnpacket_SetOutputLocationCallback
 *
 **************************************************************************** */
typedef SCODE (__stdcall *TOnMediaUnpacketOutputLocation) (DWORD dwInstance, CHAR* szLocation);

/*!
 *******************************************************************************
 * \brief
 * This callback function is used to output the digital input alert and digital output 
 * when connecting to the present product.
 *
 * \param dwInstance
 * \a (i) the instance of callback handler.
 *
 * \param dwChange
 * \a (i) Each bit is used to indicate the change of DI alert and DO, DI alert is the lower 16 bits, 
 * DO is the higher 16 bits. The LSB indicates the first one. It supports four digital input sources 
 * and two digital outputs at most in the present. 
 *
 * \param dwValue
 * \a (i) Each bit is used to indicate the value (H/L) of DI alert and DO, DI alert is the lower 16 bits, 
 * DO is the higher 16 bits. The LSB indicates the first one. It supports four digital input sources 
 * and two digital outputs at most in the present.
 * 
 * \see MediaUnpacket_SetDIDOCallback
 *
 **************************************************************************** */
typedef SCODE (__stdcall *TOnMediaUnpacketDIDO) (DWORD dwInstance, DWORD dwChange, DWORD dwValue);

/*!
 *******************************************************************************
 * \brief
 * This callback function is used to output the video signal changed
 * when connecting to the present product.
 *
 * \param dwInstance
 * \a (i) the instance of callback handler.
 *
 * \param dwValue
 * \a (i) indicate the change condition of video signal 
 *		  (EMediaUnpacketVideoSignalChange).
 * 
 * \see MediaUnpacket_SetVideoSignalCallback
 *
 **************************************************************************** */
typedef SCODE (__stdcall *TOnMediaUnpacketVideoSignal) (DWORD dwInstance, DWORD dwValue);

typedef SCODE (__stdcall *TOnMediaUnpacketIntelligentVideo) (DWORD dwInstance, BYTE *pbyIVData, DWORD dwDataLength);

typedef SCODE (__stdcall *TOnMediaUnpacketFieldMode) (DWORD dwInstance, BOOL dwFieldMode, BOOL dwInterlace);

// Add by Yun, 2004/05/19
typedef struct {
	DWORD	dwActualWidth;
	DWORD	dwActualHeight;
	DWORD	dwTransWidth;
	DWORD	dwTransHeight;
	DWORD	dwTopPaddingHeight;
	DWORD	dwBottomPaddingHeight;
	DWORD	dwRightPaddingWidth;
	DWORD	dwLeftPaddingWidth;
} TMediaUnpacketImageSize;

typedef SCODE (__stdcall *TOnMediaUnpacketImageSize) (DWORD dwInstance, TMediaUnpacketImageSize *ptImageSize);

/*! A data structure used in function MediaUnpacket_Initial to set the initial options. */
typedef struct {

    /*! The is a version control parameter. Set this value to
     * \b MEDIAUNPACKET_VERSION. */
	DWORD dwVersion;

    /*! This parameter can be assigned if you want to allocate memory for object
     * outside. If it is NULL, the object will allocate memory for itself. Using
     * \b MediaUnpacket_QueryMemorySize function to get the memory needed. */
    void *pObjectMem;

} TMediaUnpacketInitOptions;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *******************************************************************************
 * \brief
 * Create an instance of media unpacketizer and initialize it.
 *
 * \param phObject
 * \a (o) pointer to receive the instance's handle of this object.
 *
 * \param ptInitOptions
 * \a (i) the pointer of data structure \b TMediaUnpacketInitOptions for setting
 * the initialization parameters of the media unpacketizer instance. If you did not
 * set some parameters which are needed, the default value will be set.
 *
 * \retval S_OK
 * Initialize media unpacketizer OK.
 * \retval S_FAIL
 * Initialize media unpacketizer failed.
 *
 * \remark
 * This function should be called before using this instance. If you want to
 * allocate memory outside this instance, you can call \b MediaUnpacket_QueryMemorySize
 * function to get the memory size needed for this object.
 *
 * \see MediaUnpacket_Release
 * \see MediaUnpacket_QueryMemorySize
 *
 **************************************************************************** */
SCODE DLLAPI MediaUnpacket_Initial (HANDLE *phObject, TMediaUnpacketInitOptions *ptInitOptions);

/*!
 *******************************************************************************
 * \brief
 * Delete an instance of the media unpacketizer object.
 *
 * \param phObject
 * \a (i/o) the pointer to the handle of the media unpacketizer instance.
 *
 * \retval S_OK
 * Release media unpacketizer OK.
 * \retval S_FAIL
 * Release media unpacketizer failed.
 *
 * \remark
 * After the instance is released, the handle of this instance will be set to
 * zero.
 *
 * \see MediaUnpacket_Initial
 *
 **************************************************************************** */
SCODE DLLAPI MediaUnpacket_Release (HANDLE *phObject);

/*!
 *******************************************************************************
 * \brief
 * Query the memory size needed to initialize media unpacketizer instance.
 *
 * \return Memory size needed for this instance with given options in bytes. If
 * it is zero, it means that it can not estimate the size, you should let this
 * instance to allocate memory itself. And you should set field \a pObjectMem in
 * \b TMediaUnpacket1InitOptions to zero when call \b MediaUnpacket_Initial.
 *
 * \remark
 * Call this function when you need to allocate memory outside the instance.
 *
 * \see MediaUnpacket_Initial
 *
 **************************************************************************** */
DWORD DLLAPI MediaUnpacket_QueryMemorySize ();

/*!
 *******************************************************************************
 * \brief
 * Input stream from network client to the media unpacketizer. 
 * This function is used in streaming mode when you set the mode of media unpacketizer 
 * to MEDIAUNPACKETOPMODE_HTTPJPEG, MEDIAUNPACKETOPMODE_HTTPTCPH263 or 
 * MEDIAUNPACKETOPMODE_UDPH263. The unpacketizer will request a buffer to put the Data
 * Packet first. When the unpacketizer detected a complete Data Packet, the output Data
 * Packet callback function is called and the buffer of Data Packet is released.
 *
 * \param hObject
 * \a (i) a handle of media unpacketizer instance.
 *
 * \param pMediaBuffer
 * \a (i) the pointer of protocol media buffer.
 *
 * \retval S_OK
 * Input the network stream OK.
 * \retval S_FAIL
 * Input the network stream failed.
 *
 * \remark
 * Please refer to the document of protocol modules for the detail description of
 * \b PROTOCOL_MEDIABUFFER structure.
 *
 * \see MediaUnpacket_InputPacket
 *
 **************************************************************************** */
SCODE DLLAPI MediaUnpacket_Input (HANDLE hObject, PROTOCOL_MEDIABUFFER *pMediaBuffer);

/*!
 *******************************************************************************
 * \brief
 * Input packet from network client. This function is used in packet mode when you
 * set the mode of media unpacketizer to MEDIAUNPACKETOPMODE_TCPUDPG7221, 
 * MEDIAUNPACKETOPMODE_TCPUDPG729A or MEDIAUNPACKETOPMODE_VER0. The unpacketizer 
 * will request a buffer to put the Data Packet first. When the unpacketizer detected
 * a complete Data Packet, the output Data Packet callback function is called and 
 * the buffer of Data Packet is released.
 *
 * \param hObject
 * \a (i) a handle of media unpacketizer instance.
 *
 * \param pMediaBuffer
 * \a (i) the pointer of protocol media buffer.
 *
 * \retval S_OK
 * Input the network packet OK.
 * \retval S_FAIL
 * Input the network packet failed.
 *
 * \remark
 * Please refer to the document of protocol modules for the detail description of
 * \b PROTOCOL_MEDIABUFFER structure.
 *
 * \see MediaUnpacket_Input
 *
 **************************************************************************** */
SCODE DLLAPI MediaUnpacket_InputPacket (HANDLE hObject, PROTOCOL_MEDIABUFFER *pMediaBuffer);

/*!
 *******************************************************************************
 * \brief
 * Set the callback function of request buffer for media unpacketizer.
 *
 * \param hObject
 * \a (i) a handle of media unpacketizer instance.
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
SCODE DLLAPI MediaUnpacket_SetRequestBufCallback (HANDLE hObject,
												  void *pfnCallback,
												  DWORD dwInstance);

/*!
 *******************************************************************************
 * \brief
 * Set the callback function of release buffer for media unpacketizer.
 *
 * \param hObject
 * \a (i) a handle of media unpacketizer instance.
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
SCODE DLLAPI MediaUnpacket_SetReleaseBufCallback (HANDLE hObject,
												  void *pfnCallback,
												  DWORD dwInstance);

/*!
 *******************************************************************************
 * \brief
 * Set the callback function for media unpacketizer when detecting the change of 
 * location connecting to the present product.
 *
 * \param hObject
 * \a (i) a handle of media unpacketizer instance.
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
SCODE DLLAPI MediaUnpacket_SetOutputLocationCallback (HANDLE hObject,
													  void *pfnCallback,
													  DWORD dwInstance);

/*!
 *******************************************************************************
 * \brief
 * Set the callback function for media unpacketizer when receiving the digital input alert.
 *
 * \param hObject
 * \a (i) a handle of media unpacketizer instance.
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
SCODE DLLAPI MediaUnpacket_SetDIDOCallback (HANDLE hObject,
											   void *pfnCallback,
											   DWORD dwInstance);

/*!
 *******************************************************************************
 * \brief
 * Set the callback function for media unpacketizer when detecting video signal changed.
 *
 * \param hObject
 * \a (i) a handle of media unpacketizer instance.
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
SCODE DLLAPI MediaUnpacket_SetVideoSignalCallback (HANDLE hObject,
													void *pfnCallback,
													DWORD dwInstance);

/*!
 *******************************************************************************
 * \brief
 * Reset the media unpacketizer object and set the new operation mode.
 *
 * \param hObject
 * \a (i) a handle of media unpacketizer instance.
 *
 * \param tOPMode
 * \a (i) the operation mode of media unpackerizer.
 *
 * \retval S_OK
 * Set callback function OK.
 * \retval S_FAIL
 * Set callback function failed.
 *
 **************************************************************************** */
SCODE DLLAPI MediaUnpacket_Reset (HANDLE hObject, TMediaUnpacketOPMode tOPMode);

/*!
 *******************************************************************************
 * \brief
 * Get version information of media unpacketizer
 *
 * \param pbyMajor
 * \a (o) major field
 *
 * \param pbyMinor
 * \a (o) minor field
 *
 * \param pbyBuild
 * \a (o) build field
 *
 * \param pbyRevision
 * \a (o) revision field
 *
 * \retval S_OK
 * Get version information successfully.
 * \retval S_FAIL
 * Get version information failed.
 *
 * \remark NONE
 *
 * \see NONE
 *
 **************************************************************************** */
SCODE DLLAPI MediaUnpacket_GetVersionInfo(BYTE *pbyMajor, BYTE *pbyMinor, 
										  BYTE *pbyBuild, BYTE *pbyRevision);

// Add by Yun, 2004/05/06, removed 2004/12/29
//SCODE DLLAPI MediaUnpacket_SetOptions(HANDLE hObject, TMediaUnpacketOptions *ptMUOpt);
// add by Yun, 2004/12/29
SCODE DLLAPI MediaUnpacket_SetOption(HANDLE hObject, EMediaUnpacketOptionType eOptionType, DWORD dwParam1, DWORD dwParam2);
// Add by Yun, 2004/05/19
SCODE DLLAPI MediaUnpacket_SetImageSizeCallback (HANDLE hObject,
													void *pfnCallback,
													DWORD dwInstance);

SCODE DLLAPI MediaUnpacket_SetIVCallback(HANDLE hObject,
											void *pfnCallback,
											DWORD dwInstance);

SCODE DLLAPI MediaUnpacket_SetFieldModeCallback(HANDLE hObject,
													void *pfnCallback,
													DWORD dwInstance);
// added by perkins 2004/06/16
SCODE DLLAPI MediaUnpacket_UseStandard263Trans ();
SCODE DLLAPI MediaUnpacket_Use2K ();
SCODE DLLAPI MediaUnpacket_Use3K ();
SCODE DLLAPI MediaUnpacket_Use5K ();
SCODE DLLAPI MediaUnpacket_Use7K ();

void DLLAPI MediaUnpacket_LogInit(void *pvLogTarget);

#ifdef __cplusplus
}
#endif

#endif // __UNPAKCETIZER_H__
