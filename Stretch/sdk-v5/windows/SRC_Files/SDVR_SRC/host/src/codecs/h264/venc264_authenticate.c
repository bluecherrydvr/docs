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

#include "venc264_authenticate.h"
#include "sha1.h"

/*****************************************************************************
    Function:	Authentication

	sx_uint32 venc264_authenticate(
		unsigned char *buf, 				- Input/output buffer
		sx_uint32 isize, 					- Size of input buffer
		sx_uint32 maxoutsize, 				- Max allowed buffer size after 
		                                      adding authentication data
		authenticate_data_t *pAuthenticateData, - Pointer to structure 
		                                      containing additional data
                                              used in authentication
		unsigned char *pKey,                - Message authentication key
		sx_uint32 keyLength);               - Length of key
    Parameters:	buf - Input/output buffer
                isize - Size of input buffer
                maxoutsize - Max allowed buffer size after adding authentication data
				pAuthenticateData - Pointer to structure containing additional data
				                    used in authentication
 
    Return:		New size of the input/output buffer.  The input portion of the buffer
	            will be unchanged.

    The input buffer should contain a NAL packet for an h.264 video frame.  
	In general, sps and pps packets don't need authentication, but they can
	be authenticated as long as the calling program has a venc_authenticate_t
	packet for them.  This function fills in the authenticationVersion field
	of the authentication info structure, and computes the signature field.  If
	maxoutsize - isize >= 2*sizeof(venc264_authenticate_t), an additional NAL
	packet will be appended to the frame that includes the data pointed to by
	pAuthenticateData.  To calculate only the version and signature without changing
	the bitstream, call this function with maxoutsize = isize.

	The signature is calculated using the key specified with a hashing message
	authentication code scheme using SHA-1.  Keys longer than 64 bytes are
	truncated.
******************************************************************************/
#include <string.h>

#define NAL_PACKET_START_OFFSET 4
#define NAL_TYPE_OFFSET 5
#define NAL_LENGTH_OFFSET 6
#define NAL_DATA_START_OFFSET 7
#define NAL_SIZE_ABOVE_AUTHENTICATE_LENGTH 9

sx_uint32 venc264_authenticate(unsigned char *buf, sx_uint32 isize, sx_uint32 maxoutsize, 
							   venc264_authenticate_t *pAuthenticateData, unsigned char *pKey, 
							   sx_uint32 keyLength)

{
	SHA1Context innerShaContext, outerShaContext;
	unsigned char insideKey[VENC264_AUTHENTICATE_MAX_KEY_LENGTH];
	unsigned char outsideKey[VENC264_AUTHENTICATE_MAX_KEY_LENGTH];
	unsigned int i, authenticationLength;
    unsigned char lastByte=1, lastLastByte=1;

	//  Generate inside and outside keys
	memset(insideKey, VENC264_AUTHENTICATE_INSIDE_KEY_BASE, sizeof(insideKey));
	memset(outsideKey, VENC264_AUTHENTICATE_OUTSIDE_KEY_BASE, sizeof(outsideKey));
	if(keyLength > VENC264_AUTHENTICATE_MAX_KEY_LENGTH)
		keyLength = VENC264_AUTHENTICATE_MAX_KEY_LENGTH;

	for(i=0; i < keyLength; ++i)
	{
		insideKey[i] ^= pKey[i];
        outsideKey[i] ^= pKey[i];
	}
	

	//  Compute the inner SHA1 value
	SHA1Reset(&innerShaContext);
    SHA1Input(&innerShaContext, insideKey, sizeof(insideKey));
	SHA1Input(&innerShaContext, buf, isize);
	SHA1Input(&innerShaContext, (unsigned char *)&(pAuthenticateData->channelID), sizeof(pAuthenticateData->channelID));
	SHA1Input(&innerShaContext, (unsigned char *)&(pAuthenticateData->frameNumber), sizeof(pAuthenticateData->frameNumber));
	SHA1Input(&innerShaContext, (unsigned char *)&(pAuthenticateData->timestamp), sizeof(pAuthenticateData->timestamp));
	SHA1Result(&innerShaContext);
	//  Compute the outer SHA1 value
	SHA1Reset(&outerShaContext);
    SHA1Input(&outerShaContext, outsideKey, sizeof(outsideKey));
    SHA1Input(&outerShaContext, (unsigned char *)innerShaContext.Message_Digest, sizeof(innerShaContext.Message_Digest));
	SHA1Result(&outerShaContext);
	memcpy(pAuthenticateData->signature, outerShaContext.Message_Digest, sizeof(pAuthenticateData->signature));
	pAuthenticateData->authenticationVersion = VENC264_AUTHENTICATE_CURRENT_VERSION;

	//  Check if we should skip appending data to 
	if(maxoutsize < isize + 2*sizeof(venc264_authenticate_t))
		return isize;

	//  Add a NAL packet after the input data
    buf[isize] = 0;
    buf[isize+1] = 0;
    buf[isize+2] = 0;
    buf[isize+3] = 1;
	buf[isize + NAL_PACKET_START_OFFSET] = 0x66;  //  Forbidden byte=0, nal_ref_idc=3, nal type sei
	buf[isize + NAL_TYPE_OFFSET] = 0x05;  //  Payload type = user data
	
    authenticationLength = 0;
	for(i=0; i < sizeof(venc264_authenticate_t); ++i)
	{
		unsigned char thisByte = *(((unsigned char *)pAuthenticateData) + i);
		if((lastLastByte == 0) && (lastByte == 0) && ((thisByte == 0) || (thisByte == 1) || (thisByte == 3)))
		{
			buf[isize + 7 + authenticationLength++] = 3;
			lastLastByte = lastByte;
			lastByte = 3;
		}
		buf[isize + 7 + authenticationLength++] = thisByte;
        lastLastByte = lastByte;
		lastByte = thisByte;
	}
	//  Finish start of NAL header and mark the end
	buf[isize + NAL_LENGTH_OFFSET] = authenticationLength;
	buf[isize + NAL_DATA_START_OFFSET + authenticationLength + 1] = 0x80;  //  End of SEI packet
	return (isize + NAL_SIZE_ABOVE_AUTHENTICATE_LENGTH + authenticationLength);
}

