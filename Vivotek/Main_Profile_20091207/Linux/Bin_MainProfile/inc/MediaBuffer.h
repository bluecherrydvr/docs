/* *  Copyright (c) 2002 Vivotek Inc. All rights reserved. *
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
 *  Module name        :   Media Buffer module
 *  File name          :   MediaBufferAPI.h 
 *  File description   :   a media buffer structure for streaming                           
 *  Author             :   ShengFu
 *  Created at         :   2002/8/22 
 *  Note               :   
 */
 
/*!
***********************************************************
* Copyright (c) 2002 Vivotek Inc. All rights reserved.
*
* \file
* MediaBufferAPI.h
* 
* \brief
* a media buffer structure for either streaming or general purpose 
*
* \date
* 2002/09/18
* 
* \author
* ShengFu Cheng
* 
*************************************************************
*/
#ifndef _MEDIABUFFERAPI_H
#define _MEDIABUFFERAPI_H


#include "typedef.h"

/*!
* the structure define the protocol media buffer   
*/
typedef struct tagPROTOCOL_MEDIABUFFER 
{
    /*!
	* a pointer point to the address of the media data ( this value might be 
	* modified according to server or client behavior, please reference 
	* programming guide  
	*/
	BYTE	* pbDataStart;
	/*!
	* a pointer point to the address of the data  ( header + media data )
	*/
	BYTE	* pbBufferStart;
	/*!
	* a pointer point to the address of the parsed header info data ]
	* (this buffer can cast to user define structure ) 
	*/
	BYTE    * pbHeaderInfoStart; 
    /*!
	* a length of header size + max media data size  
	*/
	DWORD	dwBufferLength;	
	/*!
	* a length of used media data size  
	* (this field is also adjustable according to the behavior of
	* client or server
	*/
	DWORD	dwBytesUsed;	
	/*!
	* a length of header size  ( ie. RTP header is 12 bytes)
	*/
	DWORD	dwHeaderSize;
	/*!
	* a length of max media data size ( ususaly used to store
	* encoded media data
	*/
	DWORD   dwDataBufferSize;
	/*!
	* a length of header information size ( user define )  
	* this buffer can be casted as a user define structure  
	* with member such as sequence number and timestamp. The  
	* information parsed from the header data can be stored 
	* here for further usage 
	*/
	DWORD   dwHeaderInfoSize;


}PROTOCOL_MEDIABUFFER;

// data structure for audio information
/*! length of fmtp config*/
#define AAC_Config_Len 256
#define MP4_HEADER_LEN 100

typedef struct  
{
	char acConfig[AAC_Config_Len];
	unsigned short usIndexDeltaLength;
	unsigned short usIndexLength;
	unsigned short usSizeLength;
	unsigned short usCTSDeltaLength;
	unsigned short usDTSDeltaLength;
	unsigned long  ulAudioSampleRate;
	unsigned short usAudioChannel;

} TAAC_HBR_INFO;

typedef struct
{
	 unsigned long ulAudioRate;
	 unsigned short usAudioChannel;
	 BYTE bOctetAlign;
	 BYTE bCRC;
	 BYTE bRobustSorting;
	 unsigned long ulDecodeBuf;
} TAMR_INFO;

typedef struct
{
	char	acBuffer[MP4_HEADER_LEN];
	unsigned int	uiLen;

} TMP4_INFO;

typedef struct
{
	int				iMarker;
	unsigned long	ulTimeStamp;
	unsigned short  usSeqNumber;
	int				iExtension;
	unsigned long	ulTimeInSeconds;
	unsigned long	ulTimeInMilliSeconds;
	int				iUserDataLen;
}RTPHEADERINFO;  

#ifdef __cplusplus
extern "C" {
#endif

/*!
***************************************************************************************************
* \brief
* Create media buffer  
*
* \param ppMediaBuffer
* \a (io) pointer to the pointer to the structure of PROTOCOL_MEDIABUFFER
*
* \param uiHeaderInfoSize
* \a (io) header information size
*
* \param uiBufferSize
* \a (i) buffer size
*
* \param usHeaderSize
* \a (i) header size
*
* \retval 0
* OK
*
* \retval -1 
* Failed
*
* \retval -2 
*  the first input parameter is NULL
****************************************************************************************************
*/
int MediaBuffer_NewMediaBuffer(PROTOCOL_MEDIABUFFER** ppMediaBuffer,unsigned int uiHeaderInfoSize,unsigned int uiBufferSize,unsigned int uiHeaderSize);

/*!
***************************************************************************************************
* \brief
* free media buffer  
*
* \param ppMediaBuffer
* \a (i) pointer to the pointer to the media buffer
*
* \retval 0
*  free OK
*
* \retval -1
*  input pointer is NULL already
*
****************************************************************************************************
*/
int MediaBuffer_DeleteMediaBuffer(PROTOCOL_MEDIABUFFER** ppMediaBuffer);

#ifdef __cplusplus
}
#endif

#endif // MEDIABUFFERAPI_H

