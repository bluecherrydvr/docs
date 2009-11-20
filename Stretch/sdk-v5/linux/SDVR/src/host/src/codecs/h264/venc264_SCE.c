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
#include "sdvr_sdk.h"

/*****************************************************************************
    Function:	Start code emulation

    Parameters:	ibuf - Input buffer
                isize - size of input buffer
                obuf - Output buffer
 
    Return:		size of the output buffer.

    Note: the size of the output buffer should be 1.5x of the input buffer.
    The actual size used is gauranteed to be smaller than that.
******************************************************************************/
sx_uint32 venc264_start_code_emulation(unsigned char *ibuf, int isize, unsigned char *obuf)
{
    unsigned char *dst = obuf;
    unsigned char *src = ibuf;
    unsigned char *end = src + isize;
    int count = 0;
    int i;

    /* copy the initial start code */
    for (i = 0; i < 5; i++) {
        *dst++ = *src++;
    }

    while( src < end ) {
        if( count == 2 && *src <= 0x03 ) {
            *dst++ = 0x03;
            count = 0;
        }
        if( *src == 0 ) {
            count++;
        } else {
            count = 0;
        }
        *dst++ = *src++;
    }

    return (sx_uint32)(dst - obuf);
}
