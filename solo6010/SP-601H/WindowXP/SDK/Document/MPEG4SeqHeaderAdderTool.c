//
// These function make MPEG4 stream header for playback.
//
// Basically, the correct stream header of SOLO6010 generating stream is MPEG_Q stream header.
// So, if you have no problem in decoding, use "MakeMP4SeqHdr_MPEG_Q ()" function.
// But, some decoder(that only support MPEG4 Simple Profile)
// cannot decode the stream with MPEG_Q stream header.
// In this case, use "MakeMP4SrqHdr_H264_Q ()" function instead.
//
// 2008.06.03 Doojin Han, SoftLogic
//

#define SZ_MPEG4_VOLH_MPEG_Q		30
#define SZ_MPEG4_VOLH_H263_Q		18

BYTE glBufStrh_MPEG_Q[SZ_MPEG4_VOLH_MPEG_Q] = {
	0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x20, 0x02, 0x48, 0x0d, 0xc0, 0x00, 0x40, 0x00, 0x40,
	0x00, 0x40, 0x00, 0x80, 0x00, 0x97, 0x53, 0x0a, 0x2c, 0x08, 0x3c, 0x28, 0x8c, 0x1f
};

BYTE glBufStrh_H263_Q[SZ_MPEG4_VOLH_H263_Q] = {
	0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x20, 0x02, 0x04, 0x5d, 0x4c, 0x28, 0x58, 0x20, 0xf0,
	0xa3, 0x1f
};

__inline static void MakeMP4SeqHdr_MPEG_Q (int szH, int szV, int bInterlace)
{
	glBufStrh_MPEG_Q[23] = 0x0a +(szH >>12);
	glBufStrh_MPEG_Q[24] = ((szH >>4) &0xff);
	glBufStrh_MPEG_Q[25] = (((szH >>0) &0xf) <<4) +0x8 +((szV >>10) &0x7);
	glBufStrh_MPEG_Q[26] = ((szV >>2) &0xff);
	if (bInterlace)
	{
		glBufStrh_MPEG_Q[27] = (((szV >>0) &0x3) <<6) +0x38;
	}
	else
	{
		glBufStrh_MPEG_Q[27] = (((szV >>0) &0x3) <<6) +0x28;
	}
}

__inline static void MakeMP4SeqHdr_H263_Q (int szH, int szV, int bInterlace)
{
	glBufStrh_H263_Q[12] = 0x28 +(szH >>10);
	glBufStrh_H263_Q[13] = ((szH >>2) &0xff);
	glBufStrh_H263_Q[14] = ((szH &0x3) <<6) +(0x01 <<5) +(szV >>8);
	glBufStrh_H263_Q[15] = szV &0xff;
	if (bInterlace)
	{
		glBufStrh_H263_Q[16] = 0xe3;
	}
	else
	{
		glBufStrh_H263_Q[16] = 0xa3;
	}
}

