// stdafx.cpp : source file that includes just the standard includes
//	S6010StreamTester.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// MPEG4 Stream Help function[S]...
void MakeMP4SeqHdr (int szH, int szV, int bInterlace)
{
	glBufStrh[23] = 0x0a +(szH >>12);
	glBufStrh[24] = ((szH >>4) &0xff);
	glBufStrh[25] = (((szH >>0) &0xf) <<4) +0x8
		+((szV >>10) &0x7);
	glBufStrh[26] = ((szV >>2) &0xff);
	if (bInterlace)
	{
		glBufStrh[27] = (((szV >>0) &0x3) <<6) +0x38;
	}
	else
	{
		glBufStrh[27] = (((szV >>0) &0x3) <<6) +0x28;
	}
}
// MPEG4 Stream Help function[E]...

