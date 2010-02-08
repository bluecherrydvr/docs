/*
 *******************************************************************************
 * $Header: /RD_1/Project/VNDP/MainProfile/encryptdatapacket/src/encryptdatapacket.h 1     05/07/26 1:51p Perkins $
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
 * $History: encryptdatapacket.h $
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 05/07/26   Time: 1:51p
 * Created in $/RD_1/Project/VNDP/MainProfile/encryptdatapacket/src
 * 
 * *****************  Version 1  *****************
 * User: Perkins      Date: 04/09/16   Time: 9:34p
 * Created in $/RD_1/Project/VNDP/MainProfile/encryptdatapacket/src
 * 
 * *****************  Version 1  *****************
 * User: Yun          Date: 04/06/24   Time: 4:02p
 * Created in $/rd_1/System/encrypt/src
 * 
 * *****************  Version 1  *****************
 * User: Yun          Date: 03/03/07   Time: 2:42p
 * Created in $/rd_1/System/encrypt/src
 *******************************************************************************
 */

/*!
 *******************************************************************************
 * Copyright 2000-2003 Vivotek, Inc. All rights reserved.
 *
 * \file
 * encrypt.h
 *
 * \brief
 * the header file of encryption and decryption function for Data Packet
 *
 * \date
 * 2003/03/05
 *
 * \author
 * Mei-Yun Hsu
 *
 *
 *******************************************************************************
 */

#ifndef _ENCRYPTDATAPACKET_H_
#define _ENCRYPTDATAPACKET_H_

#include "typedef.h"
#include "common.h"
#include "errordef.h"
#include "datapacketdef.h"

#if 0
SCODE DLLAPI DataPacket_Encrypt(TMediaDataPacketInfo *ptDataPacket);
SCODE DLLAPI DataPacket_Decrypt(TMediaDataPacketInfo *ptDataPacket);
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *******************************************************************************
 * \brief
 * Encrypt the Data Packet
 *
 * \param hObject
 * \a (i) the pointer of Data Packet structure
 *
 * \retval S_OK
 * Encrypt OK.
 * \retval S_FAIL
 * Encrypt failed.
 *
 * \remark
 * The fields in the Data Packet structure should be valid. 
 *
 **************************************************************************** */
SCODE DLLAPI DataPacket_Encrypt(TMediaDataPacketInfo *ptDataPacket);

/*!
 *******************************************************************************
 * \brief
 * Decrypt the Data Packet
 *
 * \param hObject
 * \a (i) the pointer of Data Packet structure
 *
 * \retval S_OK
 * Decrypt OK.
 * \retval S_FAIL
 * Decrypt failed.
 *
 * \remark
 * The fields in the Data Packet structure should be valid. 
 *
 **************************************************************************** */
SCODE DLLAPI DataPacket_Decrypt(TMediaDataPacketInfo *ptDataPacket);

#ifdef __cplusplus
}
#endif

#endif //_ENCRYPTDATAPACKET_H_
