#include "SOLO6010.h"
#include <winioctl.h>
#include "Solo6010_Ioctl.h"
#include <Mmsystem.h>
#include <stdio.h>
#include <math.h>

#pragma warning( disable : 4244 )

LONG CSOLO6010::m_cntTimerUsed = 0;
UINT CSOLO6010::m_nMMTimerRes = 0;

const BYTE GL_BUF_IDX_SZ_TO_SCALE[NUM_IMG_SZ] = {
	1, 2, 5, 9
};
BYTE GetIdxImgSzFromScale (BYTE nScale)
{
	if (nScale == 1)		return	IMG_SZ_704X240_704X288;
	else if (nScale == 2)	return	IMG_SZ_352X240_352X288;
	else if (nScale == 5)	return	IMG_SZ_176X112_176X144;
	else if (nScale == 9)	return	IMG_SZ_704X480_704X576;
	return IMG_SZ_704X240_704X288;
}

const WORD GL_FPS_TO_FIELD_INTERVAL[NUM_VIDEO_TYPE][NUM_FPS] = 
{
	0, 1, 2, 3, 4, 5, 6, 10, 15, 30, 60, 120, 150, 150,
	0, 1, 2, 3, 4, 5, 6,  8, 12, 25, 50, 100, 125, 125
};
const WORD GL_FPS_TO_FRAME_INTERVAL[NUM_VIDEO_TYPE][NUM_FPS] = 
{
	0, 0, 1, 2, 3, 4, 5, 9, 14, 29, 59, 119, 149, 299,
	0, 0, 1, 2, 3, 4, 5, 7, 11, 24, 49,  99, 124, 249
};
BYTE GetIdxFPSFromInterval (BYTE typeVideo, BYTE nScale, WORD nInterval)
{
	if (typeVideo == VIDEO_TYPE_NTSC)
	{
		if (nScale == 9)
		{	// Frame
			if (nInterval == 0)			return FPS_30;
			else if (nInterval == 1)	return FPS_15;
			else if (nInterval == 2)	return FPS_10;
			else if (nInterval == 3)	return FPS_7_5;
			else if (nInterval == 4)	return FPS_6;
			else if (nInterval == 5)	return FPS_5;
			else if (nInterval == 9)	return FPS_3;
			else if (nInterval == 14)	return FPS_2;
			else if (nInterval == 29)	return FPS_1;
			else if (nInterval == 59)	return FPS_0_5;
			else if (nInterval == 119)	return FPS_0_25;
			else if (nInterval == 149)	return FPS_0_2;
			else if (nInterval == 299)	return FPS_0_1;
		}
		else
		{	// Field
			if (nInterval == 0)			return FPS_60;
			else if (nInterval == 1)	return FPS_30;
			else if (nInterval == 2)	return FPS_15;
			else if (nInterval == 3)	return FPS_10;
			else if (nInterval == 4)	return FPS_7_5;
			else if (nInterval == 5)	return FPS_6;
			else if (nInterval == 6)	return FPS_5;
			else if (nInterval == 10)	return FPS_3;
			else if (nInterval == 15)	return FPS_2;
			else if (nInterval == 30)	return FPS_1;
			else if (nInterval == 60)	return FPS_0_5;
			else if (nInterval == 120)	return FPS_0_25;
			else if (nInterval == 150)	return FPS_0_2;
		}
	}
	else
	{
		if (nScale == 9)
		{	// Frame
			if (nInterval == 0)			return FPS_30;
			else if (nInterval == 1)	return FPS_15;
			else if (nInterval == 2)	return FPS_10;
			else if (nInterval == 3)	return FPS_7_5;
			else if (nInterval == 4)	return FPS_6;
			else if (nInterval == 5)	return FPS_5;
			else if (nInterval == 7)	return FPS_3;
			else if (nInterval == 11)	return FPS_2;
			else if (nInterval == 24)	return FPS_1;
			else if (nInterval == 49)	return FPS_0_5;
			else if (nInterval == 99)	return FPS_0_25;
			else if (nInterval == 124)	return FPS_0_2;
			else if (nInterval == 249)	return FPS_0_1;
		}
		else
		{	// Field
			if (nInterval == 0)			return FPS_60;
			else if (nInterval == 1)	return FPS_30;
			else if (nInterval == 2)	return FPS_15;
			else if (nInterval == 3)	return FPS_10;
			else if (nInterval == 4)	return FPS_7_5;
			else if (nInterval == 5)	return FPS_6;
			else if (nInterval == 6)	return FPS_5;
			else if (nInterval == 8)	return FPS_3;
			else if (nInterval == 12)	return FPS_2;
			else if (nInterval == 25)	return FPS_1;
			else if (nInterval == 50)	return FPS_0_5;
			else if (nInterval == 100)	return FPS_0_25;
			else if (nInterval == 125)	return FPS_0_2;
		}
	}
	
	return FPS_60;
}

// Encoder OSD font[S]
const unsigned int ENC_OSD_FONT[512] = {
	0x00000000,0x0000c0c8,0xccfefe0c,0x08000000,0x00000000,0x10103838,0x7c7cfefe,0x00000000,
	0x00000000,0xfefe7c7c,0x38381010,0x10000000,0x00000000,0x7c82fefe,0xfefefe7c,0x00000000,
	0x00000000,0x00001038,0x10000000,0x00000000,0x00000000,0x0010387c,0xfe7c3810,0x00000000,
	0x00000000,0x00384444,0x44380000,0x00000000,0x00000000,0x38448282,0x82443800,0x00000000,
	0x00000000,0x007c7c7c,0x7c7c0000,0x00000000,0x00000000,0x6c6c6c6c,0x6c6c6c6c,0x00000000,
	0x00000000,0x061e7efe,0xfe7e1e06,0x00000000,0x00000000,0xc0f0fcfe,0xfefcf0c0,0x00000000,
	0x00000000,0xc6cedefe,0xfedecec6,0x00000000,0x00000000,0xc6e6f6fe,0xfef6e6c6,0x00000000,
	0x00000000,0x12367efe,0xfe7e3612,0x00000000,0x00000000,0x90d8fcfe,0xfefcd890,0x00000000,
	0x00000038,0x7cc692ba,0x92c67c38,0x00000000,0x00000038,0x7cc6aa92,0xaac67c38,0x00000000,
	0x00000038,0x7830107c,0xbaa8680c,0x00000000,0x00000038,0x3c18127c,0xb8382c60,0x00000000,
	0x00000044,0xaa6c8254,0x38eec67c,0x00000000,0x00000082,0x44288244,0x38c6827c,0x00000000,
	0x00000038,0x444444fe,0xfeeec6fe,0x00000000,0x00000018,0x78187818,0x3c7e7e3c,0x00000000,
	0x00000000,0x3854929a,0x82443800,0x00000000,0x00000000,0x00c0c8cc,0xfefe0c08,0x00000000,
	0x0000e0a0,0xe040e00e,0x8a0ea40e,0x00000000,0x0000e0a0,0xe040e00e,0x0a8e440e,0x00000000,
	0x0000007c,0x82829292,0x929282fe,0x00000000,0x000000f8,0xfc046494,0x946404fc,0x00000000,
	0x0000003f,0x7f404c52,0x524c407f,0x00000000,0x0000007c,0x82ba82ba,0x82ba82fe,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x183c3c3c,0x18180018,0x18000000,
	0x00000066,0x66240000,0x00000000,0x00000000,0x00000000,0x6c6cfe6c,0x6c6cfe6c,0x6c000000,
	0x00001010,0x7cd6d616,0x7cd0d6d6,0x7c101000,0x00000000,0x0086c660,0x30180cc6,0xc2000000,
	0x00000000,0x386c6c38,0xdc766666,0xdc000000,0x0000000c,0x0c0c0600,0x00000000,0x00000000,
	0x00000000,0x30180c0c,0x0c0c0c18,0x30000000,0x00000000,0x0c183030,0x30303018,0x0c000000,
	0x00000000,0x0000663c,0xff3c6600,0x00000000,0x00000000,0x00001818,0x7e181800,0x00000000,
	0x00000000,0x00000000,0x00000e0e,0x0c060000,0x00000000,0x00000000,0x7e000000,0x00000000,
	0x00000000,0x00000000,0x00000006,0x06000000,0x00000000,0x80c06030,0x180c0602,0x00000000,
	0x0000007c,0xc6e6f6de,0xcec6c67c,0x00000000,0x00000030,0x383c3030,0x303030fc,0x00000000,
	0x0000007c,0xc6c06030,0x180cc6fe,0x00000000,0x0000007c,0xc6c0c07c,0xc0c0c67c,0x00000000,
	0x00000060,0x70786c66,0xfe6060f0,0x00000000,0x000000fe,0x0606067e,0xc0c0c67c,0x00000000,
	0x00000038,0x0c06067e,0xc6c6c67c,0x00000000,0x000000fe,0xc6c06030,0x18181818,0x00000000,
	0x0000007c,0xc6c6c67c,0xc6c6c67c,0x00000000,0x0000007c,0xc6c6c6fc,0xc0c06038,0x00000000,
	0x00000000,0x18180000,0x00181800,0x00000000,0x00000000,0x18180000,0x0018180c,0x00000000,
	0x00000060,0x30180c06,0x0c183060,0x00000000,0x00000000,0x007e0000,0x007e0000,0x00000000,
	0x00000006,0x0c183060,0x30180c06,0x00000000,0x0000007c,0xc6c66030,0x30003030,0x00000000,
	0x0000007c,0xc6f6d6d6,0x7606067c,0x00000000,0x00000010,0x386cc6c6,0xfec6c6c6,0x00000000,
	0x0000007e,0xc6c6c67e,0xc6c6c67e,0x00000000,0x00000078,0xcc060606,0x0606cc78,0x00000000,
	0x0000003e,0x66c6c6c6,0xc6c6663e,0x00000000,0x000000fe,0x0606063e,0x060606fe,0x00000000,
	0x000000fe,0x0606063e,0x06060606,0x00000000,0x00000078,0xcc060606,0xf6c6ccb8,0x00000000,
	0x000000c6,0xc6c6c6fe,0xc6c6c6c6,0x00000000,0x0000003c,0x18181818,0x1818183c,0x00000000,
	0x00000060,0x60606060,0x6066663c,0x00000000,0x000000c6,0xc666361e,0x3666c6c6,0x00000000,
	0x00000006,0x06060606,0x060606fe,0x00000000,0x000000c6,0xeefed6c6,0xc6c6c6c6,0x00000000,
	0x000000c6,0xcedefef6,0xe6c6c6c6,0x00000000,0x00000038,0x6cc6c6c6,0xc6c66c38,0x00000000,
	0x0000007e,0xc6c6c67e,0x06060606,0x00000000,0x00000038,0x6cc6c6c6,0xc6d67c38,0x60000000,
	0x0000007e,0xc6c6c67e,0x66c6c6c6,0x00000000,0x0000007c,0xc6c60c38,0x60c6c67c,0x00000000,
	0x0000007e,0x18181818,0x18181818,0x00000000,0x000000c6,0xc6c6c6c6,0xc6c6c67c,0x00000000,
	0x000000c6,0xc6c6c6c6,0xc66c3810,0x00000000,0x000000c6,0xc6c6c6c6,0xd6d6fe6c,0x00000000,
	0x000000c6,0xc6c66c38,0x6cc6c6c6,0x00000000,0x00000066,0x66666666,0x3c181818,0x00000000,
	0x000000fe,0xc0603018,0x0c0606fe,0x00000000,0x0000003c,0x0c0c0c0c,0x0c0c0c3c,0x00000000,
	0x00000002,0x060c1830,0x60c08000,0x00000000,0x0000003c,0x30303030,0x3030303c,0x00000000,
	0x00001038,0x6cc60000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00fe0000,
	0x00001818,0x30000000,0x00000000,0x00000000,0x00000000,0x00003c60,0x7c66667c,0x00000000,
	0x0000000c,0x0c0c7ccc,0xcccccc7c,0x00000000,0x00000000,0x00007cc6,0x0606c67c,0x00000000,
	0x00000060,0x60607c66,0x6666667c,0x00000000,0x00000000,0x00007cc6,0xfe06c67c,0x00000000,
	0x00000078,0x0c0c0c3e,0x0c0c0c0c,0x00000000,0x00000000,0x00007c66,0x6666667c,0x60603e00,
	0x0000000c,0x0c0c7ccc,0xcccccccc,0x00000000,0x00000030,0x30003830,0x30303078,0x00000000,
	0x00000030,0x30003c30,0x30303030,0x30301f00,0x0000000c,0x0c0ccc6c,0x3c6ccccc,0x00000000,
	0x00000030,0x30303030,0x30303030,0x00000000,0x00000000,0x000066fe,0xd6d6d6d6,0x00000000,
	0x00000000,0x000078cc,0xcccccccc,0x00000000,0x00000000,0x00007cc6,0xc6c6c67c,0x00000000,
	0x00000000,0x00007ccc,0xcccccc7c,0x0c0c0c00,0x00000000,0x00007c66,0x6666667c,0x60606000,
	0x00000000,0x000076dc,0x0c0c0c0c,0x00000000,0x00000000,0x00007cc6,0x1c70c67c,0x00000000,
	0x00000000,0x1818fe18,0x18181870,0x00000000,0x00000000,0x00006666,0x6666663c,0x00000000,
	0x00000000,0x0000c6c6,0xc66c3810,0x00000000,0x00000000,0x0000c6d6,0xd6d6fe6c,0x00000000,
	0x00000000,0x0000c66c,0x38386cc6,0x00000000,0x00000000,0x00006666,0x6666667c,0x60603e00,
	0x00000000,0x0000fe60,0x30180cfe,0x00000000,0x00000070,0x1818180e,0x18181870,0x00000000,
	0x00000018,0x18181800,0x18181818,0x00000000,0x0000000e,0x18181870,0x1818180e,0x00000000,
	0x000000dc,0x76000000,0x00000000,0x00000000,0x00000000,0x0010386c,0xc6c6fe00,0x00000000
};
// Encoder OSD font[E]

UINT WINAPI ThFn_MP4DataGet (LPVOID pParam)
{
	CSOLO6010 *pS6010 = (CSOLO6010 *)pParam;
	unsigned long nTmpAddrEnd;
	BOOL bFlagErrorOn;

	while (1)
	{
		if (pS6010->m_bufBEndTh[S6010_IDX_TH_MP4] == TRUE)
		{
			break;
		}
		WaitForSingleObject (pS6010->m_bufHEvt[COMMON_EVT_MP4], INFINITE);
		ResetEvent (pS6010->m_bufHEvt[COMMON_EVT_MP4]);
		bFlagErrorOn = FALSE;

//		AddReport ("MP4 Event Set");

		if (pS6010->m_bufBEndTh[S6010_IDX_TH_MP4] == TRUE)
		{
			break;
		}

		if (pS6010->m_pIShare == NULL)
		{
			continue;
		}

		nTmpAddrEnd = pS6010->m_pIShare->iMP4Buf.addrEnd;

		if (pS6010->m_bufCbProc[S6010_CB_MP4_CAPTURE] != NULL)
		{
			if (pS6010->m_recModeMP4 == REC_DATA_RCV_BY_FRAME)
			{
				HEADER_MP4_FRM *pHdrMP4RecFrm;
				BYTE *bufTmpFrm, *bufMP4, bufTmpHeader[sizeof(HEADER_MP4_FRM)];
				unsigned long posStart, posStartReal, szBuf, szFrm;
				BOOL bOverlapBuf;

				szBuf = pS6010->m_pIShare->iMP4Buf.szBuf;
				posStart = pS6010->m_pIShare->iMP4Buf.addrStart;

				bOverlapBuf = FALSE;
				if (nTmpAddrEnd < posStart)
				{	// posStart > nTmpAddrEnd => posStart < nTmpAddrEnd
					nTmpAddrEnd += pS6010->m_pIShare->iMP4Buf.szBuf;
					bOverlapBuf = TRUE;
				}

				if (pS6010->m_pIShare->iMP4Buf.flagError != 0)
				{
					bFlagErrorOn = TRUE;
				}

				while (posStart < nTmpAddrEnd)
				{
					posStartReal = posStart %pS6010->m_pIShare->iMP4Buf.szBuf;

					if (pS6010->m_pIShare->iMP4Buf.szBuf -posStart == 32)
					{
						memcpy (&bufTmpHeader[0], &pS6010->m_pIShare->pBufMP4Rec[posStartReal], 32);
						memcpy (&bufTmpHeader[32], &pS6010->m_pIShare->pBufMP4Rec[0], 32);
						pHdrMP4RecFrm = (HEADER_MP4_FRM *)&bufTmpHeader[0];
					}
					else
					{
						pHdrMP4RecFrm = (HEADER_MP4_FRM *)&pS6010->m_pIShare->pBufMP4Rec[posStartReal];
					}
					szFrm = sizeof(HEADER_MP4_FRM) +GET_MP4_CODE_SZ_PT(pHdrMP4RecFrm);

					if (szBuf -posStartReal < szFrm)
					{
						bufTmpFrm = new BYTE[szFrm];
						memcpy (&bufTmpFrm[0], &pS6010->m_pIShare->pBufMP4Rec[posStartReal], szBuf -posStartReal);
						memcpy (&bufTmpFrm[szBuf -posStartReal], &pS6010->m_pIShare->pBufMP4Rec[0], szFrm -(szBuf -posStartReal));

						bufMP4 = bufTmpFrm;
					}
					else
					{
						bufMP4 = &pS6010->m_pIShare->pBufMP4Rec[posStartReal];
					}

					((MP4_CAPTURE_PROC)pS6010->m_bufCbProc[S6010_CB_MP4_CAPTURE])
						(pS6010->m_bufCbContext[S6010_CB_MP4_CAPTURE],
						bufMP4,
						0,
						szFrm,
						szFrm,
						pS6010->m_pIShare->iMP4Buf.flagError);

					if (szBuf -posStartReal < szFrm)
					{
						delete[] bufTmpFrm;
					}
					posStart += szFrm;
				}
				if (bOverlapBuf == TRUE)
				{
					nTmpAddrEnd -= pS6010->m_pIShare->iMP4Buf.szBuf;
				}

				if (bFlagErrorOn == TRUE)
				{
					pS6010->m_pIShare->iMP4Buf.flagError = 0;
				}
			}
			else	// REC_DATA_RCV_BY_RAW_DATA
			{
				if (pS6010->m_pIShare->iMP4Buf.flagError != 0)
				{
					bFlagErrorOn = TRUE;
				}

				((MP4_CAPTURE_PROC)pS6010->m_bufCbProc[S6010_CB_MP4_CAPTURE])
					(pS6010->m_bufCbContext[S6010_CB_MP4_CAPTURE],
					pS6010->m_pIShare->pBufMP4Rec,
					pS6010->m_pIShare->iMP4Buf.addrStart,
					nTmpAddrEnd,
					pS6010->m_pIShare->iMP4Buf.szBuf,
					pS6010->m_pIShare->iMP4Buf.flagError);

				if (bFlagErrorOn == TRUE)
				{
					pS6010->m_pIShare->iMP4Buf.flagError = 0;
				}
			}
		}

		pS6010->m_pIShare->iMP4Buf.addrStart = nTmpAddrEnd;
		pS6010->m_bRecMP4_BufferFlushed = 1;
	}

	pS6010->m_bufBEndTh[S6010_IDX_TH_MP4] = FALSE;

	ExitThread (0);
	return 0;
}

UINT WINAPI ThFn_JPEGDataGet (LPVOID pParam)
{
	CSOLO6010 *pS6010 = (CSOLO6010 *)pParam;
	unsigned long nTmpAddrEnd;
	BOOL bFlagErrorOn;

	while (1)
	{
		if (pS6010->m_bufBEndTh[S6010_IDX_TH_JPEG] == TRUE)
		{
			break;
		}
		WaitForSingleObject (pS6010->m_bufHEvt[COMMON_EVT_JPEG], INFINITE);
		ResetEvent (pS6010->m_bufHEvt[COMMON_EVT_JPEG]);
		bFlagErrorOn = FALSE;

		if (pS6010->m_bufBEndTh[S6010_IDX_TH_JPEG] == TRUE)
		{
			break;
		}

		nTmpAddrEnd = pS6010->m_pIShare->iJPEGBuf.addrEnd;

		if (pS6010->m_bufCbProc[S6010_CB_JPEG_CAPTURE] != NULL)
		{
			if (pS6010->m_recModeJPEG == REC_DATA_RCV_BY_FRAME)
			{
				if (pS6010->m_pIShare->iMP4Buf.flagError != 0)
				{
					bFlagErrorOn = TRUE;
				}
				
				if (bFlagErrorOn == TRUE)
				{
					pS6010->m_pIShare->iMP4Buf.flagError = 0;
				}
			}
			else	// REC_DATA_RCV_BY_RAW_DATA
			{
				if (pS6010->m_pIShare->iJPEGBuf.flagError != 0)
				{
					bFlagErrorOn = TRUE;
				}
				
				((JPEG_CAPTURE_PROC)pS6010->m_bufCbProc[S6010_CB_JPEG_CAPTURE])
					(pS6010->m_bufCbContext[S6010_CB_JPEG_CAPTURE],
					pS6010->m_pIShare->pBufJPEGRec,
					pS6010->m_pIShare->iJPEGBuf.addrStart,
					nTmpAddrEnd,
					pS6010->m_pIShare->iJPEGBuf.szBuf,
					pS6010->m_pIShare->iJPEGBuf.flagError);
				
				if (bFlagErrorOn == TRUE)
				{
					pS6010->m_pIShare->iJPEGBuf.flagError = 0;
				}
			}
		}

		pS6010->m_pIShare->iJPEGBuf.addrStart = nTmpAddrEnd;
		pS6010->m_bRecJPEG_BufferFlushed = 1;
	}

	pS6010->m_bufBEndTh[S6010_IDX_TH_JPEG] = FALSE;

	ExitThread (0);
	return 0;
}

UINT WINAPI ThFn_G723DataGet (LPVOID pParam)
{
	CSOLO6010 *pS6010 = (CSOLO6010 *)pParam;

	unsigned long nTmpAddrEnd, posStart, szBuf, i, j, k, sz1Line, cntCurWriteChPair;
	DWORD szWrite, szWriteEachCh;
	BYTE *bufG723WriteAllCh, *bufG723WriteChA, *bufG723WriteChB, *pBufG723Raw;
	BOOL bFlagErrorOn;

	szBuf = 0;

	while (1)
	{
		if (pS6010->m_bufBEndTh[S6010_IDX_TH_G723] == TRUE)
		{
			break;
		}
		WaitForSingleObject (pS6010->m_bufHEvt[COMMON_EVT_G723], INFINITE);
		ResetEvent (pS6010->m_bufHEvt[COMMON_EVT_G723]);
		bFlagErrorOn = FALSE;

//		AddReport ("G723 Event Set");

		if (pS6010->m_bufBEndTh[S6010_IDX_TH_G723] == TRUE)
		{
			break;
		}

		if (szBuf == 0)
		{
			szBuf = pS6010->m_pIShare->iG723Buf.szBuf;

			bufG723WriteAllCh = new BYTE[szBuf];
			bufG723WriteChA = new BYTE[szBuf /NUM_G723_CH];
			bufG723WriteChB = new BYTE[szBuf /NUM_G723_CH];
			pBufG723Raw = pS6010->m_pIShare->pBufG723Rec;
		}

		posStart = pS6010->m_pIShare->iG723Buf.addrStart;
		nTmpAddrEnd = pS6010->m_pIShare->iG723Buf.addrEnd;

		if (pS6010->m_bufCbProc[S6010_CB_G723_CAPTURE] != NULL)
		{
			szWrite = 0;
			if (nTmpAddrEnd < posStart)
			{
				for (i=posStart; i<szBuf; i+=SZ_G723_1_ITEM)
				{
					// Write time information[S]
					memcpy (&bufG723WriteAllCh[szWrite], &pBufG723Raw[i +SZ_G723_1_FRM *NUM_G723_CH], SZ_G723_TIME_INFO);
					szWrite += SZ_G723_TIME_INFO;
					// Write time information[E]

					for (j=0; j<NUM_G723_CH_PAIR; j++)
					{
						if (pS6010->m_iRec.iG723.bufBEncChPair[j] == TRUE)
						{
							memcpy (&bufG723WriteAllCh[szWrite], &pBufG723Raw[i +j *(SZ_G723_1_FRM *2)], SZ_G723_1_FRM *2);
							szWrite += SZ_G723_1_FRM *2;
						}
					}
				}
				for (i=0; i<nTmpAddrEnd; i+=SZ_G723_1_ITEM)
				{
					// Write time information[S]
					memcpy (&bufG723WriteAllCh[szWrite], &pBufG723Raw[i +SZ_G723_1_FRM *NUM_G723_CH], SZ_G723_TIME_INFO);
					szWrite += SZ_G723_TIME_INFO;
					// Write time information[E]

					for (j=0; j<NUM_G723_CH_PAIR; j++)
					{
						if (pS6010->m_iRec.iG723.bufBEncChPair[j] == TRUE)
						{
							memcpy (&bufG723WriteAllCh[szWrite], &pBufG723Raw[i +j *(SZ_G723_1_FRM *2)], SZ_G723_1_FRM *2);
							szWrite += SZ_G723_1_FRM *2;
						}
					}
				}
			}
			else
			{
				for (i=posStart; i<nTmpAddrEnd; i+=SZ_G723_1_ITEM)
				{
					// Write time information[S]
					memcpy (&bufG723WriteAllCh[szWrite], &pBufG723Raw[i +SZ_G723_1_FRM *NUM_G723_CH], SZ_G723_TIME_INFO);
					szWrite += SZ_G723_TIME_INFO;
					// Write time information[E]
					
					for (j=0; j<NUM_G723_CH_PAIR; j++)
					{
						if (pS6010->m_iRec.iG723.bufBEncChPair[j] == TRUE)
						{
							memcpy (&bufG723WriteAllCh[szWrite], &pBufG723Raw[i +j *(SZ_G723_1_FRM *2)], SZ_G723_1_FRM *2);
							szWrite += SZ_G723_1_FRM *2;
						}
					}
				}
			}

			if (pS6010->m_recModeG723 == REC_DATA_RCV_BY_FRAME)
			{
				sz1Line = SZ_G723_TIME_INFO;
				for (j=0; j<NUM_G723_CH_PAIR; j++)
				{
					if (pS6010->m_iRec.iG723.bufBEncChPair[j] == TRUE)
					{
						sz1Line += SZ_G723_1_FRM *2;
					}
				}

				if (pS6010->m_pIShare->iG723Buf.flagError != 0)
				{
					bFlagErrorOn = TRUE;
				}
				
				for (j=0, cntCurWriteChPair=0; j<NUM_G723_CH_PAIR; j++)
				{
					if (pS6010->m_iRec.iG723.bufBEncChPair[j] == TRUE)
					{
						szWriteEachCh = 0;
						for (k=0; k<szWrite; k+=sz1Line)
						{
							// Copy Time Info[S]
							memcpy (&bufG723WriteChA[szWriteEachCh], &bufG723WriteAllCh[k], SZ_G723_TIME_INFO);
							memcpy (&bufG723WriteChB[szWriteEachCh], &bufG723WriteAllCh[k], SZ_G723_TIME_INFO);
							szWriteEachCh += SZ_G723_TIME_INFO;
							// Copy Time Info[E]

							// Copy G723 Channel Data[S]
							memcpy (&bufG723WriteChA[szWriteEachCh],
								&bufG723WriteAllCh[k +SZ_G723_TIME_INFO +cntCurWriteChPair *SZ_G723_1_FRM *2], SZ_G723_1_FRM);
							memcpy (&bufG723WriteChB[szWriteEachCh],
								&bufG723WriteAllCh[k +SZ_G723_TIME_INFO +cntCurWriteChPair *SZ_G723_1_FRM *2 +SZ_G723_1_FRM], SZ_G723_1_FRM);
							szWriteEachCh += SZ_G723_1_FRM;
							// Copy G723 Channel Data[E]
						}
						((G723_CAPTURE_PROC)pS6010->m_bufCbProc[S6010_CB_G723_CAPTURE])
							(pS6010->m_bufCbContext[S6010_CB_G723_CAPTURE], j *2 +0, bufG723WriteChA, szWriteEachCh, pS6010->m_pIShare->iG723Buf.flagError);
						((G723_CAPTURE_PROC)pS6010->m_bufCbProc[S6010_CB_G723_CAPTURE])
							(pS6010->m_bufCbContext[S6010_CB_G723_CAPTURE], j *2 +1, bufG723WriteChB, szWriteEachCh, pS6010->m_pIShare->iG723Buf.flagError);

						cntCurWriteChPair++;
					}
				}

				if (bFlagErrorOn == TRUE)
				{
					pS6010->m_pIShare->iG723Buf.flagError = 0;
				}
			}
			else	// REC_DATA_RCV_BY_RAW_DATA
			{
				((G723_CAPTURE_PROC)pS6010->m_bufCbProc[S6010_CB_G723_CAPTURE])
					(pS6010->m_bufCbContext[S6010_CB_G723_CAPTURE], 0, bufG723WriteAllCh, szWrite, pS6010->m_pIShare->iG723Buf.flagError);

				pS6010->m_pIShare->iG723Buf.flagError = 0;
			}
		}

		pS6010->m_pIShare->iG723Buf.addrStart = nTmpAddrEnd;
		pS6010->m_bRecG723_BufferFlushed = 1;
	}
	if (szBuf != 0)
	{
		delete[] bufG723WriteAllCh;
		delete[] bufG723WriteChA;
		delete[] bufG723WriteChB;
	}

	pS6010->m_bufBEndTh[S6010_IDX_TH_G723] = FALSE;

	ExitThread (0);
	return 0;
}

UINT WINAPI ThFn_OverlayFlip (LPVOID pParam)
{
	CSOLO6010 *pS6010 = (CSOLO6010 *)pParam;
	CUR_STAT_DDRAW_BUF *pCurDDrawStat;

	int tmpIdxBuf, tmpIdxField;

	SetThreadPriority (pS6010->m_bufHThread[S6010_IDX_TH_OVERLAY_FLIP], THREAD_PRIORITY_TIME_CRITICAL);
	
	while (1)
	{
		WaitForSingleObject (pS6010->m_bufHEvt[COMMON_EVT_OVERLAY_FLIP], INFINITE);
		
		if (pS6010->m_bufBEndTh[S6010_IDX_TH_OVERLAY_FLIP] == TRUE)
		{
			ResetEvent (pS6010->m_bufHEvt[COMMON_EVT_OVERLAY_FLIP]);
			break;
		}

		pCurDDrawStat = &pS6010->m_pIShare->curStatDDrawBuf;

		if (pCurDDrawStat->posApp == pCurDDrawStat->posDrv)
		{
			ResetEvent (pS6010->m_bufHEvt[COMMON_EVT_OVERLAY_FLIP]);
			continue;
		}

		tmpIdxBuf = pCurDDrawStat->posApp;
		tmpIdxField = pCurDDrawStat->bufIdxField[tmpIdxBuf];

		if (pS6010->m_bufCbProc[S6010_CB_OVERLAY_FLIP] != NULL)
		{
			((OVERLAY_FLIP_PROC)pS6010->m_bufCbProc[S6010_CB_OVERLAY_FLIP])
				(pS6010->m_bufCbContext[S6010_CB_OVERLAY_FLIP],
				tmpIdxBuf,
				pCurDDrawStat->numBuf,
				tmpIdxField);
		}

		pCurDDrawStat->posApp = (pCurDDrawStat->posApp +1) %pCurDDrawStat->numBuf;

		ResetEvent (pS6010->m_bufHEvt[COMMON_EVT_OVERLAY_FLIP]);
	}

	pS6010->m_bufBEndTh[S6010_IDX_TH_OVERLAY_FLIP] = FALSE;

	ExitThread (0);
	return 0;
}

UINT WINAPI ThFn_VMotion (LPVOID pParam)
{
	CSOLO6010 *pS6010 = (CSOLO6010 *)pParam;
	DWORD hRet;

	while (1)
	{
		hRet = WaitForSingleObject (pS6010->m_bufHEvt[COMMON_EVT_V_MOTION], INTERVAL_VMOTION_CHECK);

		if (pS6010->m_bufBEndTh[S6010_IDX_TH_V_MOTION] == TRUE)
		{
			break;
		}

		if (pS6010->m_bufCbProc[S6010_CB_V_MOTION_ALARM] != NULL)
		{
			if (hRet != WAIT_TIMEOUT)
			{	// signalled by event (MOTION EXIST)
				ResetEvent (pS6010->m_bufHEvt[COMMON_EVT_V_MOTION]);
				((V_MOTION_ALARM_PROC)pS6010->m_bufCbProc[S6010_CB_V_MOTION_ALARM])
					(pS6010->m_bufCbContext[S6010_CB_V_MOTION_ALARM],
					pS6010->m_pIShare->statCurVMotion);
			}
			else
			{	// timeout (NO MOTION)
				((V_MOTION_ALARM_PROC)pS6010->m_bufCbProc[S6010_CB_V_MOTION_ALARM])
					(pS6010->m_bufCbContext[S6010_CB_V_MOTION_ALARM],
					0);
			}
		}
		else
		{
			ResetEvent (pS6010->m_bufHEvt[COMMON_EVT_V_MOTION]);
		}
	}

	pS6010->m_bufBEndTh[S6010_IDX_TH_V_MOTION] = FALSE;

	ExitThread (0);
	return 0;
}

UINT WINAPI ThFn_CheckCamBlkDet (LPVOID pParam)
{
	CSOLO6010 *pS6010 = (CSOLO6010 *)pParam;
	DWORD hRet, nCheckInterval, szRead, flagChCamBlkDet;
	unsigned short *bufOldWC[NUM_LIVE_DISP_WND], *pCurWC, *pTmpWC1, *pTmpWC2;
	int i, j, k, szBlkH, szBlkV, szBlkAll;
	int bufMeanOld[NUM_LIVE_DISP_WND], bufVarOld[NUM_LIVE_DISP_WND], nMeanCur, nVarCur, nCovariance, nCorrCoeff;
	float bufStdDevOld[NUM_LIVE_DISP_WND], nStdDevCur;
	
	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		bufOldWC[i] = new unsigned short[SZ_BUF_V_MOTION_TH_IN_USHORT];
		memset (bufOldWC[i], 0, SZ_BUF_V_MOTION_TH_IN_BYTE);
	}

	memset (bufMeanOld, 0, sizeof(int) *NUM_LIVE_DISP_WND);
	while (1)
	{
		if (pS6010->m_pIShare == NULL)
		{
			nCheckInterval = MIN_INTERVAL_CAM_BLK_DET;
		}
		else
		{
			nCheckInterval = pS6010->m_iCamBlkDet.timeCheckInterval;
			if (nCheckInterval < MIN_INTERVAL_CAM_BLK_DET)
			{
				nCheckInterval = MIN_INTERVAL_CAM_BLK_DET;
			}
		}
		hRet = WaitForSingleObject (pS6010->m_bufHEvt[S6010_EVT_CAM_BLK_DET],
			nCheckInterval);

		if (pS6010->m_bufBEndTh[S6010_IDX_TH_CAM_BLK_DET] == TRUE)
		{
			break;
		}

		flagChCamBlkDet = 0;
		for (i=0; i<NUM_LIVE_DISP_WND; i++)
		{
			if (pS6010->m_iCamBlkDet.bufBUseDetection[i] == TRUE)
			{
				pS6010->m_iCamBlkDet.idxCh = i;
				DeviceIoControl (pS6010->m_hS6010, S6010_IOCTL_GET_CAM_BLK_STATE, NULL, 0, &pS6010->m_iCamBlkDet,
					sizeof(INFO_CAM_BLK_DETECTION), &szRead, NULL);
				pCurWC = (unsigned short *)pS6010->m_pIShare->pBufCamBlkDet;

				szBlkH = pS6010->m_iS6010Init.szVideoH >>4;
				szBlkV = (pS6010->m_iS6010Init.szVideoV *2) >>4;
				szBlkAll = szBlkH *szBlkV;

				nMeanCur = 0;
				nVarCur = 0;
				for (j=0; j<szBlkV; j++)
				{
					pTmpWC1 = &pCurWC[j *64];
					for (k=0; k<szBlkH; k++)
					{
						*pTmpWC1 = *pTmpWC1 /64;

						nMeanCur += *pTmpWC1;
						nVarCur += *pTmpWC1 **pTmpWC1;
						pTmpWC1++;
					}
				}
				nMeanCur /= szBlkAll;
				nVarCur = nVarCur /szBlkAll -nMeanCur *nMeanCur;
				nStdDevCur = sqrt (nVarCur);

				if (bufMeanOld[i] != 0)
				{	// Check correlation coefficient
					nCovariance = 0;
					for (j=0; j<szBlkV; j++)
					{
						pTmpWC1 = &bufOldWC[i][j *64];
						pTmpWC2 = &pCurWC[j *64];
						for (k=0; k<szBlkH; k++)
						{
							nCovariance += *pTmpWC1 **pTmpWC2;
							pTmpWC1++;
							pTmpWC2++;
						}
					}

					nCovariance = (nCovariance /szBlkAll) -(bufMeanOld[i] *nMeanCur);

					if (nStdDevCur *bufStdDevOld[i] == 0)
					{
						nCorrCoeff = 100;
					}
					else
					{
						nCorrCoeff = int(nCovariance *100 /(nStdDevCur *bufStdDevOld[i]));
						if (nCorrCoeff < 0)
						{
							nCorrCoeff = -nCorrCoeff;
						}
					}

					if (nCorrCoeff < pS6010->m_iCamBlkDet.bufAlarmLevel[i])
					{
						flagChCamBlkDet |= 1 <<i;
					}

//					AddReport ("Ch:%d Var:%d CorrCoeff:%d", i, nVarCur, nCorrCoeff);
				}

				bufMeanOld[i] = nMeanCur;
				bufVarOld[i] = nVarCur;
				bufStdDevOld[i] = nStdDevCur;
				memcpy (bufOldWC[i], pCurWC, SZ_BUF_V_MOTION_TH_IN_BYTE);
			}
		}

		if (pS6010->m_bufCbProc[S6010_CB_CAM_BLK_DET_ALARM] != NULL)
		{
			((CAM_BLK_DET_ALARM_PROC)pS6010->m_bufCbProc[S6010_CB_CAM_BLK_DET_ALARM])
				(pS6010->m_bufCbContext[S6010_CB_CAM_BLK_DET_ALARM], flagChCamBlkDet);
		}
	}

	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		delete[] bufOldWC[i];
	}
	pS6010->m_bufBEndTh[S6010_IDX_TH_CAM_BLK_DET] = FALSE;

	ExitThread (0);
	return 0;
}

UINT WINAPI ThFn_GPIO_IntAlarm (LPVOID pParam)
{
	CSOLO6010 *pS6010 = (CSOLO6010 *)pParam;
	DWORD hRet;

	while (1)
	{
		hRet = WaitForSingleObject (pS6010->m_bufHEvt[COMMON_EVT_GPIO_INT_ALARM], INFINITE);

		if (pS6010->m_bufBEndTh[S6010_IDX_TH_GPIO_INT_ALARM] == TRUE)
		{
			break;
		}

		AddReport ("GPIO Int Alarm:%08x", pS6010->m_pIShare->iGPIO_IntStat.maskIntStat);
		
		pS6010->m_pIShare->iGPIO_IntStat.maskIntStat = 0;
		if (pS6010->m_bufCbProc[S6010_CB_GPIO_INT_ALARM] != NULL)
		{
		}

		ResetEvent (pS6010->m_bufHEvt[COMMON_EVT_GPIO_INT_ALARM]);
	}

	pS6010->m_bufBEndTh[S6010_IDX_TH_GPIO_INT_ALARM] = FALSE;

	ExitThread (0);
	return 0;
}

UINT WINAPI ThFn_AdvMotionData (LPVOID pParam)
{
	CSOLO6010 *pS6010 = (CSOLO6010 *)pParam;
	DWORD hRet;
	BYTE bufMotionMap[SZ_BUF_V_MOTION_TH_IN_USHORT *NUM_LIVE_DISP_WND];
	DWORD flagAdvMotDet;

	while (1)
	{
		hRet = WaitForSingleObject (pS6010->m_bufHEvt[COMMON_EVT_ADV_MOTION_DATA], INTERVAL_ADV_MOT_CHECK);

		if (pS6010->m_bufBEndTh[S6010_IDX_TH_ADV_MOTION_DATA] == TRUE)
		{
			break;
		}

		if (hRet != WAIT_TIMEOUT)
		{
			if (pS6010->m_iAdvMotDet.bUseAdvMotion == 1 && pS6010->m_bufCbProc[S6010_CB_ADV_MOT_DET_ALARM] != NULL)
			{
				memcpy (pS6010->m_iAdvMotDet.bbufCurWCData[pS6010->m_iAdvMotDet.curWCDataPos], pS6010->m_pIShare->pBufAdvVM, SZ_BUF_ADV_MOTION_DATA);
				
				if (pS6010->m_iAdvMotDet.bInitOK == 0)
				{
					pS6010->m_iAdvMotDet.cntNumGetData++;
					
					if (pS6010->m_iAdvMotDet.cntNumGetData >= MAX_MOT_DATA_DISTANCE)
					{
						pS6010->m_iAdvMotDet.bInitOK = 1;
					}
				}
				else
				{
					pS6010->ADV_MOT_CheckAdvMotion (flagAdvMotDet, bufMotionMap);

					if (pS6010->m_bufCbProc[S6010_CB_ADV_MOT_DET_ALARM] != NULL)
					{
						((ADV_MOT_DET_ALARM_PROC)pS6010->m_bufCbProc[S6010_CB_ADV_MOT_DET_ALARM]) 
							(pS6010->m_bufCbContext[S6010_CB_ADV_MOT_DET_ALARM], flagAdvMotDet, bufMotionMap);
					}

				}
				pS6010->m_iAdvMotDet.curWCDataPos = (pS6010->m_iAdvMotDet.curWCDataPos +1) %MAX_MOT_DATA_DISTANCE;
			}

			ResetEvent (pS6010->m_bufHEvt[COMMON_EVT_ADV_MOTION_DATA]);
		}
		else
		{
			if (pS6010->m_bufCbProc[S6010_CB_ADV_MOT_DET_ALARM] != NULL)
			{
				((ADV_MOT_DET_ALARM_PROC)pS6010->m_bufCbProc[S6010_CB_ADV_MOT_DET_ALARM])
					(pS6010->m_bufCbContext[S6010_CB_ADV_MOT_DET_ALARM], 0, NULL);
			}
		}
	}

	pS6010->m_bufBEndTh[S6010_IDX_TH_ADV_MOTION_DATA] = FALSE;

	ExitThread (0);
	return 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

int CSOLO6010::GENERAL_GetNumOfS6010 ()
{
	int i;
	char strTmp[MAX_PATH];
	HANDLE hS6010;

	for (i=0; i<NUM_MAX_S6010; i++)
	{
		sprintf (strTmp, "\\\\.\\%s-%d", SOLO6010_DRIVER_NAME, i);
		hS6010 = CreateFile (strTmp, GENERIC_READ |GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hS6010 == INVALID_HANDLE_VALUE)
		{
			break;
		}
		CloseHandle (hS6010);
	}

	return i;
}

CSOLO6010::CSOLO6010(int idxS6010)
{
	int i;

	m_idxS6010 = idxS6010;

	m_hS6010 = NULL;
	m_pIShare = NULL;

	for (i=0; i<S6010_NUM_EVENT; i++)
	{
		m_bufHEvt[i] = NULL;
	}
	
	m_bInit = FALSE;
	m_bLiveOn = FALSE;
	m_bRecOn = FALSE;

	m_bMP4DecOn = FALSE;

	for (i=0; i<S6010_NUM_CALLBACK; i++)
	{
		m_bufCbProc[i] = NULL;
		m_bufCbContext[i] = NULL;
	}

	// Live info init[S]
	ZeroMemory (&m_iLive, sizeof(INFO_LIVE));
	LIVE_Init ();
	// Live info init[E]

	// Mosaic info init[S]
	ZeroMemory (&m_iMosaic, sizeof(INFO_MOSAIC));
	// Mosaic info init[E]

	// Video motion info init[S]
	ZeroMemory (&m_iVMotion, sizeof(INFO_V_MOTION));
	// Video motion info init[E]

	// Camera block detection init[S]
	ZeroMemory (&m_iCamBlkDet, sizeof(INFO_CAM_BLK_DETECTION));
	
	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		m_iCamBlkDet.bufAlarmLevel[i] = DEF_ALARM_LEVEL;
	}
	m_iCamBlkDet.timeCheckInterval = MIN_INTERVAL_CAM_BLK_DET;
	// Camera block detection init[E]

	// Advanced motion detection init[S]
	ZeroMemory (&m_iAdvMotDet, sizeof(INFO_ADV_MOT_DETECTION));
	// Advanced motion detection init[E]

	// Record info init[S]
	ZeroMemory (&m_iRec, sizeof(INFO_REC));

	REC_SetRecDataRcvMode (REC_DATA_RCV_BY_RAW_DATA, REC_DATA_RCV_BY_RAW_DATA, REC_DATA_RCV_BY_RAW_DATA);
	REC_SetNotifyBufferLevel (DEFAULT_REC_BUF_NOTIFY_LEVEL, DEFAULT_REC_BUF_NOTIFY_LEVEL, DEFAULT_REC_BUF_NOTIFY_LEVEL);
	// Record info init[E]
}

CSOLO6010::~CSOLO6010()
{
	GENERAL_Close ();
}

BOOL CSOLO6010::GENERAL_Open ()
{
	GENERAL_Close ();

	char strTmp[MAX_PATH];
	int i;

	for (i=0; i<S6010_NUM_EVENT; i++)
	{
		sprintf (strTmp, GL_STR_FORMAT_EVT[i], m_idxS6010);
		m_bufHEvt[i] = CreateEvent (NULL, TRUE, FALSE, strTmp);

		if (m_bufHEvt[i] == NULL)
		{
			AddReport ("ERROR: Event Open Fail");
			return FALSE;
		}
		ResetEvent (m_bufHEvt[i]);
	}

	sprintf (strTmp, "\\\\.\\%s-%d", SOLO6010_DRIVER_NAME, m_idxS6010);
	m_hS6010 = CreateFile (strTmp, GENERIC_READ |GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (m_hS6010 == INVALID_HANDLE_VALUE)
	{
		AddReport ("ERROR: SOLO6010 Open Fail");
		m_hS6010 = NULL;
		return FALSE;
	}

	DWORD idThread;
	for (i=0; i<S6010_NUM_THREAD; i++)
	{
		m_bufBEndTh[i] = FALSE;
	}
	m_bufHThread[S6010_IDX_TH_MP4] = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)ThFn_MP4DataGet, this, 0, &idThread);
	m_bufHThread[S6010_IDX_TH_JPEG] = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)ThFn_JPEGDataGet, this, 0, &idThread);
	m_bufHThread[S6010_IDX_TH_G723] = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)ThFn_G723DataGet, this, 0, &idThread);
	m_bufHThread[S6010_IDX_TH_OVERLAY_FLIP] = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)ThFn_OverlayFlip, this, 0, &idThread);
	m_bufHThread[S6010_IDX_TH_V_MOTION] = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)ThFn_VMotion, this, 0, &idThread);
	m_bufHThread[S6010_IDX_TH_CAM_BLK_DET] = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)ThFn_CheckCamBlkDet, this, 0, &idThread);
	m_bufHThread[S6010_IDX_TH_GPIO_INT_ALARM] = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)ThFn_GPIO_IntAlarm, this, 0, &idThread);
	m_bufHThread[S6010_IDX_TH_ADV_MOTION_DATA] = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)ThFn_AdvMotionData, this, 0, &idThread);
 
 	for (i=0; i<S6010_NUM_THREAD; i++)
	{
		if (m_bufHThread[i] == NULL)
		{
			AddReport ("CreateThread Error: %d\n", i);
		}
	}

	return TRUE;
}

void CSOLO6010::GENERAL_Close ()
{
	if (m_bMP4DecOn == TRUE)
	{
		MP4D_Close ();
	}
	if (m_bLiveOn == TRUE)
	{
		LIVE_End ();
	}
	if (m_bRecOn == TRUE)
	{
		REC_End ();
	}

	// Unregister callback[S]
	int i;
	for (i=0; i<S6010_NUM_CALLBACK; i++)
	{
		m_bufCbProc[i] = NULL;
		m_bufCbContext[i] = NULL;
	}
	// Unregister callback[E]

	if (m_hS6010 != NULL)
	{
		int i;
		for (i=0; i<S6010_NUM_THREAD; i++)
		{
			m_bufBEndTh[i] = TRUE;
		}
		for (i=0; i<S6010_NUM_EVENT; i++)
		{
			SetEvent (m_bufHEvt[i]);
		}
		for (i=0; i<S6010_NUM_THREAD; i++)
		{
			while (m_bufBEndTh[i])
			{
				Sleep (0);
			}
		}
		for (i=0; i<S6010_NUM_EVENT; i++)
		{
			CloseHandle (m_bufHEvt[i]);
			m_bufHEvt[i] = NULL;
		}
		for (i=0; i<S6010_NUM_THREAD; i++)
		{
			WaitForSingleObject (m_bufHThread[i], INFINITE);
			CloseHandle (m_bufHThread[i]);
		}

		CloseHandle (m_hS6010);
		m_hS6010 = NULL;
		m_pIShare = NULL;
		m_bInit = FALSE;
	}
}

BOOL CSOLO6010::GENERAL_Init (BYTE typeVideo)
{
	DWORD szRead;
	int i;

	if (m_hS6010 == NULL)
	{
		AddReport ("ERROR: S6010 Init. Fail -> Not Opened");
		return FALSE;
	}

	ZeroMemory (&m_iS6010Init, sizeof(INFO_S6010_INIT));

	m_iS6010Init.idxS6010 = m_idxS6010;
	m_iS6010Init.typeVideo = typeVideo;

	if (m_iS6010Init.typeVideo == VIDEO_TYPE_NTSC)
	{
		m_iS6010Init.szVideoH = VIDEO_IN_H_SIZE;
		m_iS6010Init.szVideoV = 240;

		m_iS6010Init.posVOutH = 6;
		m_iS6010Init.posVOutV = 8;
		m_iS6010Init.posVInH = 8;
		m_iS6010Init.posVInV = 1;
	}
	else
	{
		m_iS6010Init.szVideoH = VIDEO_IN_H_SIZE;
		m_iS6010Init.szVideoV = 288;

		m_iS6010Init.posVOutH = 6;
		m_iS6010Init.posVOutV = 8;
		m_iS6010Init.posVInH = 8;
		m_iS6010Init.posVInV = 1;
	}
		
	for (i=0; i<NUM_COMMON_EVT; i++)
	{
		m_iS6010Init.bufHEvt[i] = m_bufHEvt[i];
	}

	DeviceIoControl (m_hS6010, S6010_IOCTL_OPEN_S6010, NULL, 0,
		(PVOID)&m_iS6010Init, sizeof(INFO_S6010_INIT), &szRead, NULL);

	if (szRead == 0)
	{
		AddReport ("ERROR: S6010 Init. Fail");
		return FALSE;
	}

	m_pIShare = m_iS6010Init.pIShare;
	m_bInit = TRUE;

	return TRUE;
}

UINT CSOLO6010::AllocateTimerRes ()
{
	if (CSOLO6010::m_cntTimerUsed == 0)
	{
		TIMECAPS tc;
		if (timeGetDevCaps (&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
		{	// Multimedia timer open fail
			return FALSE;
		}
		m_nMMTimerRes = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
		timeBeginPeriod (m_nMMTimerRes);
	}

	InterlockedIncrement (&CSOLO6010::m_cntTimerUsed);

	return m_nMMTimerRes;
}

void CSOLO6010::ReleaseTimerRes ()
{
	if (CSOLO6010::m_cntTimerUsed > 0)
	{
		InterlockedDecrement (&CSOLO6010::m_cntTimerUsed);

		if (CSOLO6010::m_cntTimerUsed == 0)
		{
			timeEndPeriod (m_nMMTimerRes);
		}
	}
}

// Register IO[S]
DWORD CSOLO6010::REGISTER_Get (DWORD nAddr)
{
	DWORD szRead;
	INFO_REG_RW iRegRW;

	iRegRW.nAddr = nAddr;
	iRegRW.nVal = 0;

	DeviceIoControl (m_hS6010, S6010_IOCTL_REG_READ, NULL, 0,
		&iRegRW, sizeof(INFO_REG_RW), &szRead, NULL);

	return iRegRW.nVal;
}

void CSOLO6010::REGISTER_Set (DWORD nAddr, DWORD nData)
{
	DWORD szRead;
	INFO_REG_RW iRegRW;

	iRegRW.nAddr = nAddr;
	iRegRW.nVal = nData;

	DeviceIoControl (m_hS6010, S6010_IOCTL_REG_WRITE, NULL, 0,
		&iRegRW, sizeof(INFO_REG_RW), &szRead, NULL);
}
// Register IO[E]

// Record[S]
BOOL CSOLO6010::REC_Start ()
{
	if (m_bRecOn == TRUE)
	{
		return TRUE;
	}
	
	DWORD szRead;

	// Set record property[S]
	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_REC_PROP, NULL, 0,
		(PVOID)&m_iRec, sizeof(INFO_REC), &szRead, NULL);
	if (szRead == 0)
	{
		AddReport ("ERROR: S6010_IOCTL_SET_REC_PROP");
		return FALSE;
	}
	// Set record property[E]

	// Start record[S]
	DeviceIoControl (m_hS6010, S6010_IOCTL_START_REC, NULL, 0, 0, 0, &szRead, NULL);
	if (szRead == 0)
	{
		AddReport ("ERROR: S6010_IOCTL_START_REC");
		return FALSE;
	}
	// Start record[E]

	m_bRecOn = TRUE;

	return TRUE;
}

BOOL CSOLO6010::REC_End ()
{
	if (m_bRecOn == FALSE)
	{
		return TRUE;
	}

	DWORD szRead;
	DeviceIoControl (m_hS6010, S6010_IOCTL_END_REC, NULL, 0, 0, 0, &szRead, NULL);

	if (szRead == 0)
	{
		AddReport ("ERROR: S6010_IOCTL_END_REC");
		return FALSE;
	}

	m_bRecMP4_BufferFlushed = 0;
	m_bRecJPEG_BufferFlushed = 0;
	m_bRecG723_BufferFlushed = 0;
	SetEvent (m_bufHEvt[COMMON_EVT_MP4]);
	SetEvent (m_bufHEvt[COMMON_EVT_JPEG]);
	SetEvent (m_bufHEvt[COMMON_EVT_G723]);

	m_bRecOn = FALSE;

	return TRUE;
}

void CSOLO6010::REC_MP4_RegisterCallback (MP4_CAPTURE_PROC cbFnMP4Capture, void *pContext)
{
	m_bufCbProc[S6010_CB_MP4_CAPTURE] = (LPVOID)cbFnMP4Capture;
	m_bufCbContext[S6010_CB_MP4_CAPTURE] = (LPVOID)pContext;
}

void CSOLO6010::REC_MP4_UnregisterCallback ()
{
	m_bufCbProc[S6010_CB_MP4_CAPTURE] = NULL;
	m_bufCbContext[S6010_CB_MP4_CAPTURE] = NULL;
}

void CSOLO6010::REC_JPEG_RegisterCallback (JPEG_CAPTURE_PROC cbFnJPEGCapture, void *pContext)
{
	m_bufCbProc[S6010_CB_JPEG_CAPTURE] = (LPVOID)cbFnJPEGCapture;
	m_bufCbContext[S6010_CB_JPEG_CAPTURE] = (LPVOID)pContext;
}

void CSOLO6010::REC_JPEG_UnregisterCallback ()
{
	m_bufCbProc[S6010_CB_JPEG_CAPTURE] = NULL;
	m_bufCbContext[S6010_CB_JPEG_CAPTURE] = NULL;
}

void CSOLO6010::REC_G723_RegisterCallback (G723_CAPTURE_PROC cbFnG723Capture, void *pContext)
{
	m_bufCbProc[S6010_CB_G723_CAPTURE] = (LPVOID)cbFnG723Capture;
	m_bufCbContext[S6010_CB_G723_CAPTURE] = (LPVOID)pContext;
}

void CSOLO6010::REC_G723_UnregisterCallback ()
{
	m_bufCbProc[S6010_CB_G723_CAPTURE] = NULL;
	m_bufCbContext[S6010_CB_G723_CAPTURE] = NULL;
}

BOOL CSOLO6010::REC_MP4_RealChSetProp (BYTE idxCh,
	BYTE bRec, BYTE idxImgSz, BYTE nKeyFrmInterval,
	BYTE nQuality, BYTE bIBasedPred, BYTE idxFPS, WORD nMotionTh,
	BYTE bUseCBR, DWORD nBitRate, DWORD szVirtualDecBuf)
{
	if (idxCh >= NUM_VID_REAL_ENC_CH)
	{
		return FALSE;
	}
	if (bRec == 0)
	{
		ZeroMemory (&m_iRec.iMP4[idxCh], sizeof(INFO_MP4_REC));
		return TRUE;
	}
	m_iRec.iMP4[idxCh].bSet = bRec;
	m_iRec.iMP4[idxCh].nScale = GL_BUF_IDX_SZ_TO_SCALE[idxImgSz];
	m_iRec.iMP4[idxCh].nGOP = nKeyFrmInterval;
	m_iRec.iMP4[idxCh].nQP = nQuality +MP4E_MIN_QP;
	m_iRec.iMP4[idxCh].bIBasedPred = bIBasedPred;
	m_iRec.iMP4[idxCh].bUseCBR = bUseCBR;
	if (m_iRec.iMP4[idxCh].nScale == 9)
	{	// Frame
		m_iRec.iMP4[idxCh].nInterval = GL_FPS_TO_FRAME_INTERVAL[m_iS6010Init.typeVideo][idxFPS];
	}
	else
	{	// Field
		m_iRec.iMP4[idxCh].nInterval = GL_FPS_TO_FIELD_INTERVAL[m_iS6010Init.typeVideo][idxFPS];
	}
	m_iRec.iMP4[idxCh].nMotionTh = nMotionTh;
	m_iRec.iMP4[idxCh].nBitrate = nBitRate;
	m_iRec.iMP4[idxCh].szVirtualDecBuf = szVirtualDecBuf;

	return TRUE;
}

void CSOLO6010::REC_MP4_RealChGetProp (BYTE idxCh,
	BYTE &bRec, BYTE &idxImgSz, BYTE &nKeyFrmInterval,
	BYTE &nQuality, BYTE &bIBasedPred, BYTE &idxFPS, WORD &nMotionTh,
	BYTE &bUseCBR, DWORD &nBitRate, DWORD &szVirtualDecBuf)
{
	if (idxCh >= NUM_VID_REAL_ENC_CH)
	{
		bRec = 0;
		idxImgSz = 0;
		nKeyFrmInterval = 0;
		nQuality = 0;
		bIBasedPred = 0;
		bUseCBR = 0;
		idxFPS = 0;
		nMotionTh = 0;
		nBitRate = 0;
		szVirtualDecBuf = 0;
		return;
	}

	bRec = m_iRec.iMP4[idxCh].bSet;
	idxImgSz = GetIdxImgSzFromScale (m_iRec.iMP4[idxCh].nScale);
	nKeyFrmInterval = m_iRec.iMP4[idxCh].nGOP;
	nQuality = m_iRec.iMP4[idxCh].nQP -MP4E_MIN_QP;
	bIBasedPred = m_iRec.iMP4[idxCh].bIBasedPred;
	bUseCBR = m_iRec.iMP4[idxCh].bUseCBR;
	idxFPS = GetIdxFPSFromInterval (m_iS6010Init.typeVideo, m_iRec.iMP4[idxCh].nScale, m_iRec.iMP4[idxCh].nInterval);
	nMotionTh = m_iRec.iMP4[idxCh].nMotionTh;
	nBitRate = m_iRec.iMP4[idxCh].nBitrate;
	szVirtualDecBuf = m_iRec.iMP4[idxCh].szVirtualDecBuf;
}

BOOL CSOLO6010::REC_MP4_VirtChSetProp (BYTE idxCh,
	BYTE bRec, BYTE nKeyFrmInterval,
	BYTE nQuality, BYTE idxFPS,
	BYTE bUseCBR, DWORD nBitRate, DWORD szVirtualDecBuf)
{
	if (idxCh >= NUM_VID_VIRT_ENC_CH ||
		(bRec == 1 && m_iRec.iMP4[idxCh].bSet == 0))
	{
		return FALSE;
	}
	if (bRec == 0)
	{
		ZeroMemory (&m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh], sizeof(INFO_MP4_REC));
		return TRUE;
	}
	m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].bSet = bRec;
	m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].nScale = m_iRec.iMP4[idxCh].nScale;
	m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].nGOP = nKeyFrmInterval;
	m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].nQP = nQuality +MP4E_MIN_QP;
	m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].bIBasedPred = m_iRec.iMP4[idxCh].bIBasedPred;
	m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].bUseCBR = bUseCBR;
	if (m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].nScale == 9)
	{	// Frame
		m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].nInterval = GL_FPS_TO_FRAME_INTERVAL[m_iS6010Init.typeVideo][idxFPS];
	}
	else
	{	// Field
		m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].nInterval = GL_FPS_TO_FIELD_INTERVAL[m_iS6010Init.typeVideo][idxFPS];
	}
	m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].nMotionTh = m_iRec.iMP4[idxCh].nMotionTh;
	m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].nBitrate = nBitRate;
	m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].szVirtualDecBuf = szVirtualDecBuf;
	
	return TRUE;
}

void CSOLO6010::REC_MP4_VirtChGetProp (BYTE idxCh,
	BYTE &bRec, BYTE &idxImgSz, BYTE &nKeyFrmInterval,
	BYTE &nQuality, BYTE &bIBasedPred, BYTE &idxFPS, WORD &nMotionTh,
	BYTE &bUseCBR, DWORD &nBitRate, DWORD &szVirtualDecBuf)
{
	if (idxCh >= NUM_VID_VIRT_ENC_CH)
	{
		bRec = 0;
		idxImgSz = 0;
		nKeyFrmInterval = 0;
		nQuality = 0;
		bIBasedPred = 0;
		bUseCBR = 0;
		idxFPS = 0;
		nMotionTh = 0;
		nBitRate = 0;
		szVirtualDecBuf = 0;
		return;
	}

	bRec = m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].bSet;
	idxImgSz = GetIdxImgSzFromScale (m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].nScale);
	nKeyFrmInterval = m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].nGOP;
	nQuality = m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].nQP -MP4E_MIN_QP;
	bIBasedPred = m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].bIBasedPred;
	bUseCBR = m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].bUseCBR;
	idxFPS = GetIdxFPSFromInterval (m_iS6010Init.typeVideo, m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].nScale,
		m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].nInterval);
	nMotionTh = m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].nMotionTh;
	nBitRate = m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].nBitrate;
	szVirtualDecBuf = m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].szVirtualDecBuf;
}

BOOL CSOLO6010::REC_JPEG_RealChSetProp (BYTE idxCh, BYTE bRec, BYTE idxQuality)
{
	if (m_iRec.iMP4[idxCh].bSet == FALSE)
	{	// TO encode JPEG, Same channel MP4 recording must be set.
		return FALSE;
	}
	if (idxCh >= NUM_VID_REAL_ENC_CH)
	{
		return FALSE;
	}
	if (idxQuality > JPEG_IMG_QUALITY_HIGH)
	{
		return FALSE;
	}

	if (bRec == 0)
	{
		ZeroMemory (&m_iRec.iJPEG[idxCh], sizeof(INFO_JPEG_REC));
		return TRUE;
	}

	m_iRec.iJPEG[idxCh].bRec = bRec;
	m_iRec.iJPEG[idxCh].idxQP = idxQuality;

	return TRUE;
}

void CSOLO6010::REC_JPEG_RealChGetProp (BYTE idxCh, BYTE &bRec, BYTE &idxQuality)
{
	if (idxCh >= NUM_VID_REAL_ENC_CH)
	{
		bRec = 0;
		idxQuality = 0;
		return;
	}

	bRec = m_iRec.iJPEG[idxCh].bRec;
	idxQuality = m_iRec.iJPEG[idxCh].idxQP;
}

BOOL CSOLO6010::REC_JPEG_VirtChSetProp (BYTE idxCh, BYTE bRec, BYTE idxQuality)
{
	if (m_iRec.iMP4[NUM_VID_REAL_ENC_CH +idxCh].bSet == FALSE)
	{	// TO encode JPEG, Same channel MP4 recording must be set.
		return FALSE;
	}
	if (idxCh >= NUM_VID_VIRT_ENC_CH)
	{
		return FALSE;
	}
	if (idxQuality > JPEG_IMG_QUALITY_HIGH)
	{
		return FALSE;
	}

	if (bRec == 0)
	{
		ZeroMemory (&m_iRec.iJPEG[NUM_VID_REAL_ENC_CH +idxCh], sizeof(INFO_JPEG_REC));
		return TRUE;
	}

	m_iRec.iJPEG[NUM_VID_REAL_ENC_CH +idxCh].bRec = bRec;
	m_iRec.iJPEG[NUM_VID_REAL_ENC_CH +idxCh].idxQP = idxQuality;

	return TRUE;
}

void CSOLO6010::REC_JPEG_VirtChGetProp (BYTE idxCh, BYTE &bRec, BYTE &idxQuality)
{
	if (idxCh >= NUM_VID_VIRT_ENC_CH)
	{
		bRec = 0;
		idxQuality = 0;
		return;
	}

	bRec = m_iRec.iJPEG[NUM_VID_REAL_ENC_CH +idxCh].bRec;
	idxQuality = m_iRec.iJPEG[NUM_VID_REAL_ENC_CH +idxCh].idxQP;
}

void CSOLO6010::REC_G723_SetProp (BYTE idxChPair, BYTE bRec, BYTE idxSamplingRate)
{
	m_iRec.iG723.bufBEncChPair[idxChPair] = bRec;
	m_iRec.iG723.idxSampRate = idxSamplingRate;
}

void CSOLO6010::REC_G723_GetProp (BYTE idxChPair, BYTE &bRec, BYTE &idxSamplingRate)
{
}

void CSOLO6010::REC_SetRecDataRcvMode (BYTE modeMP4, BYTE modeJPEG, BYTE modeG723)
{
	m_recModeMP4 = modeMP4;
	m_recModeJPEG = modeJPEG;
	m_recModeG723 = modeG723;
}

void CSOLO6010::REC_GetRecDataRcvMode (BYTE &modeMP4, BYTE &modeJPEG, BYTE &modeG723)
{
	modeMP4 = m_recModeMP4;
	modeJPEG = m_recModeJPEG;
	modeG723 = m_recModeG723;
}

void CSOLO6010::REC_SetNotifyBufferLevel (BYTE levelMP4, BYTE levelJPEG, BYTE levelG723)
{
	m_iRec.nNotifyBufLevelMP4 = levelMP4;
	m_iRec.nNotifyBufLevelJPEG = levelJPEG;
	m_iRec.nNotifyBufLevelG723 = levelG723;
}

void CSOLO6010::REC_GetNotifyBufferLevel (BYTE &levelMP4, BYTE &levelJPEG, BYTE &levelG723)
{
	levelMP4 = m_iRec.nNotifyBufLevelMP4;
	levelJPEG = m_iRec.nNotifyBufLevelJPEG;
	levelG723 = m_iRec.nNotifyBufLevelG723;
}

BOOL CSOLO6010::REC_MP4_RealChChangePropOnRec (BYTE idxCh, BYTE bRec, BYTE idxImgSz, BYTE nKeyFrmInterval,
	BYTE nQuality, BYTE bIBasedPred, BYTE idxFPS, BYTE bUseCBR, DWORD nBitRate, DWORD szVirtualDecBuf)
{
	if (idxCh >= NUM_VID_REAL_ENC_CH)
	{
		return FALSE;
	}
	if (bRec == 0)
	{
		m_iRec.iMP4[idxCh].bSet = 0;
	}
	else
	{
		m_iRec.iMP4[idxCh].bSet = bRec;
		m_iRec.iMP4[idxCh].nScale = GL_BUF_IDX_SZ_TO_SCALE[idxImgSz];
		m_iRec.iMP4[idxCh].nGOP = nKeyFrmInterval;
		m_iRec.iMP4[idxCh].nQP = nQuality +MP4E_MIN_QP;
		m_iRec.iMP4[idxCh].bIBasedPred = bIBasedPred;
		m_iRec.iMP4[idxCh].bUseCBR = bUseCBR;
		if (m_iRec.iMP4[idxCh].nScale == 9)
		{	// Frame
			m_iRec.iMP4[idxCh].nInterval = GL_FPS_TO_FRAME_INTERVAL[m_iS6010Init.typeVideo][idxFPS];
		}
		else
		{	// Field
			m_iRec.iMP4[idxCh].nInterval = GL_FPS_TO_FIELD_INTERVAL[m_iS6010Init.typeVideo][idxFPS];
		}
		m_iRec.iMP4[idxCh].nMotionTh = 0xfff;
		m_iRec.iMP4[idxCh].nBitrate = nBitRate;
		m_iRec.iMP4[idxCh].szVirtualDecBuf = szVirtualDecBuf;
	}
	m_iRec.iMP4[idxCh].bSet += MP4E_ADD_VAL_FOR_CHANGE_REC_SETTINGS;

	DWORD szRead;

	DeviceIoControl (m_hS6010, S6010_IOCTL_CHANGE_REC_PROP, NULL, 0,
		(PVOID)&m_iRec, sizeof(INFO_REC), &szRead, NULL);

	if (szRead == 0)
	{
		AddReport ("ERROR: S6010_IOCTL_CHANGE_REC_PROP");
		return FALSE;
	}

	return TRUE;
}
// Record[E]


// Live display & control[S]
BOOL CSOLO6010::LIVE_Start (INFO_DDRAW_SURF *pIDDrawSurf)
{
	DWORD szRead;
	int i;

	if (m_bLiveOn == FALSE)
	{
		if (pIDDrawSurf->bUse2StepCopyMode == TRUE)
		{
			pIDDrawSurf->numSurf = m_pIShare->szBufLiveImg /(pIDDrawSurf->nPitch *pIDDrawSurf->szV);
			for (i=0; i<pIDDrawSurf->numSurf; i++)
			{
				pIDDrawSurf->lpSurf[i] = m_pIShare->pBufLiveImg +(pIDDrawSurf->nPitch *pIDDrawSurf->szV) *i;
			}
		}

		DeviceIoControl (m_hS6010, S6010_IOCTL_START_LIVE, NULL, 0, pIDDrawSurf,
			sizeof(INFO_DDRAW_SURF), &szRead, NULL);
	}

	m_bLiveOn = TRUE;

	return TRUE;
}

BOOL CSOLO6010::LIVE_End ()
{
	DWORD szRead;

	if (m_bLiveOn == TRUE)
	{
		DeviceIoControl (m_hS6010, S6010_IOCTL_END_LIVE, NULL, 0, 0,
			0, &szRead, NULL);

		m_bLiveOn = FALSE;
	}

	return TRUE;
}

void CSOLO6010::LIVE_SetProp ()
{
	DWORD szRead;
	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_LIVE_PROP, NULL, 0, &m_iLive,
		sizeof(INFO_LIVE), &szRead, NULL);

/*	if (pInfoLive->idxSplitMode == LIVE_SPLIT_MODE_1 &&
		(pInfoLive->flagLiveOrDec &0x1) == 0x1 &&
		m_hdrMP4File.strFileInit[0] != 0 &&
		(m_hdrMP4File.bufSetRec[pInfoLive->bufIdxDecCh[0]].nScale == 2 ||
		m_hdrMP4File.bufSetRec[pInfoLive->bufIdxDecCh[0]].nScale == 3))
	{
//		glPWndLiveDisp->ZoomOverlayDestHSize (2);
	}
	else
	{
//		glPWndLiveDisp->ZoomOverlayDestHSize (1);
	}
*/
}

void CSOLO6010::LIVE_RegisterCallback (OVERLAY_FLIP_PROC cbFnFlipOverlay, void *pContext)
{
	m_bufCbProc[S6010_CB_OVERLAY_FLIP] = (LPVOID)cbFnFlipOverlay;
	m_bufCbContext[S6010_CB_OVERLAY_FLIP] = (LPVOID)pContext;
}

void CSOLO6010::LIVE_UnregisterCallback ()
{
	m_bufCbProc[S6010_CB_OVERLAY_FLIP] = NULL;
	m_bufCbContext[S6010_CB_OVERLAY_FLIP] = NULL;
}

void CSOLO6010::LIVE_Init ()
{
	int i;

	m_iLive.idxSplitMode = LIVE_SPLIT_MODE_1;

	for (i=0; i<NUM_LIVE_DISP_WND; i++)	// Channel Init
	{
		m_iLive.bufIdxCh[i] = i;
	}
	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		m_iLive.bufIdxDecCh[i] = INVALID_CH_IDX;
	}
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		m_iLive.bufIdxWndFromDecCh[i] = INVALID_WND_IDX;
	}
}

void CSOLO6010::LIVE_SetSplitMode (int idxSplitMode)
{
	m_iLive.idxSplitMode = idxSplitMode;

	int i;
	for (i=GL_NUM_DISP_FROM_DISP_MODE[idxSplitMode]; i<NUM_LIVE_DISP_WND; i++)
	{
		if (m_iLive.bufIdxDecCh[i] != INVALID_CH_IDX)
		{
			m_iLive.bufIdxWndFromDecCh[m_iLive.bufIdxDecCh[i]] = INVALID_WND_IDX;
			m_iLive.bufIdxDecCh[i] = INVALID_CH_IDX;
		}
	}
	LIVE_SetProp ();
}

void CSOLO6010::LIVE_SetWndLiveOrDec (int idxWnd, int bLive)
{
	if (bLive == TRUE)
	{	// Change window mode to live
		m_iLive.flagLiveOrDec &= ~(1 <<idxWnd);

		m_iLive.bufIdxWndFromDecCh[m_iLive.bufIdxDecCh[idxWnd]] = INVALID_WND_IDX;
		m_iLive.bufIdxDecCh[idxWnd] = INVALID_CH_IDX;
	}
	else
	{	// Change window mode to MP4 dec
		m_iLive.flagLiveOrDec |= (1 <<idxWnd);
	}
	LIVE_SetProp ();
}

BOOL CSOLO6010::LIVE_IsWndLiveOrDec (int idxWnd)
{
	if (((m_iLive.flagLiveOrDec >>idxWnd) &0x01) == 0x01)
	{
		return FALSE;
	}
	return TRUE;
}

void CSOLO6010::LIVE_ChangeLiveCh (int idxSelWnd, int idxCh)
{
	int i, j;
	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		if (m_iLive.bufIdxCh[i] == idxCh)
		{	// Channel swap
			j = m_iLive.bufIdxCh[idxSelWnd];
			m_iLive.bufIdxCh[idxSelWnd] = m_iLive.bufIdxCh[i];
			m_iLive.bufIdxCh[i] = j;
			break;
		}
	}
	LIVE_SetProp ();
}

void CSOLO6010::LIVE_ChangeDecCh (int idxSelWnd, int idxCh)
{
	int i, j;

	// Change bufIdxDecCh[] [S]
	for (i=0; i<GL_NUM_DISP_FROM_DISP_MODE[m_iLive.idxSplitMode]; i++)
	{
		if (m_iLive.bufIdxDecCh[i] == idxCh)
		{	// Channel swap
			j = m_iLive.bufIdxDecCh[idxSelWnd];
			m_iLive.bufIdxDecCh[idxSelWnd] = idxCh;
			m_iLive.bufIdxDecCh[i] = j;
			break;
		}
	}
	if (i == GL_NUM_DISP_FROM_DISP_MODE[m_iLive.idxSplitMode])
	{
		m_iLive.bufIdxDecCh[idxSelWnd] = idxCh;
	}
	for (i=GL_NUM_DISP_FROM_DISP_MODE[m_iLive.idxSplitMode]; i<NUM_LIVE_DISP_WND; i++)
	{
		m_iLive.bufIdxDecCh[i] = INVALID_CH_IDX;
	}
	// Change bufIdxDecCh[] [E]

	// Change bufIdxWndFromDecCh[] from bufIdxDecCh[] [S]
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		m_iLive.bufIdxWndFromDecCh[i] = INVALID_WND_IDX;
	}
	for (i=0; i<GL_NUM_DISP_FROM_DISP_MODE[m_iLive.idxSplitMode]; i++)
	{
		if (m_iLive.bufIdxDecCh[i] != INVALID_CH_IDX)
		{
			m_iLive.bufIdxWndFromDecCh[m_iLive.bufIdxDecCh[i]] = i;
		}
	}
	// Change bufIdxWndFromDecCh[] from bufIdxDecCh[] [E]
}
// Live display & control[E]


// Color[S]
void CSOLO6010::COLOR_SetHue (BYTE idxCh, BYTE nHue)
{
	DWORD szRead;
	INFO_COLOR infoColor;

	memset (&infoColor, 0, sizeof(INFO_COLOR));

	infoColor.idxCh = m_iVMatrix.bufIdxCamera[idxCh];
	infoColor.bAccHue = 1;
	infoColor.nHue = nHue;

	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_COLOR, NULL, 0, &infoColor,
		sizeof(INFO_COLOR), &szRead, NULL);
}

void CSOLO6010::COLOR_SetSaturation (BYTE idxCh, BYTE nSaturation)
{
	DWORD szRead;
	INFO_COLOR infoColor;

	memset (&infoColor, 0, sizeof(INFO_COLOR));

	infoColor.idxCh = m_iVMatrix.bufIdxCamera[idxCh];
	infoColor.bAccSaturation = 1;
	infoColor.nSaturation = nSaturation;

	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_COLOR, NULL, 0, &infoColor,
		sizeof(INFO_COLOR), &szRead, NULL);
}

void CSOLO6010::COLOR_SetContrast (BYTE idxCh, BYTE nContrast)
{
	DWORD szRead;
	INFO_COLOR infoColor;

	memset (&infoColor, 0, sizeof(INFO_COLOR));

	infoColor.idxCh = m_iVMatrix.bufIdxCamera[idxCh];
	infoColor.bAccContrast = 1;
	infoColor.nContrast = nContrast;

	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_COLOR, NULL, 0, &infoColor,
		sizeof(INFO_COLOR), &szRead, NULL);
}

void CSOLO6010::COLOR_SetBrightness (BYTE idxCh, BYTE nBrightness)
{
	DWORD szRead;
	INFO_COLOR infoColor;

	memset (&infoColor, 0, sizeof(INFO_COLOR));

	infoColor.idxCh = m_iVMatrix.bufIdxCamera[idxCh];
	infoColor.bAccBrightness = 1;
	infoColor.nBrightness = nBrightness;

	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_COLOR, NULL, 0, &infoColor,
		sizeof(INFO_COLOR), &szRead, NULL);
}

void CSOLO6010::COLOR_Get (BYTE idxCh,
	BYTE &nHue, BYTE &nSaturation, BYTE &nContrast, BYTE &nBrightness)
{
	DWORD szRead;
	INFO_COLOR infoColor;

	memset (&infoColor, 0, sizeof(INFO_COLOR));

	infoColor.idxCh = m_iVMatrix.bufIdxCamera[idxCh];
	infoColor.bAccHue = 1;
	infoColor.bAccSaturation = 1;
	infoColor.bAccContrast = 1;
	infoColor.bAccBrightness = 1;

	DeviceIoControl (m_hS6010, S6010_IOCTL_GET_COLOR, NULL, 0, &infoColor,
		sizeof(INFO_COLOR), &szRead, NULL);

	nHue = infoColor.nHue;
	nSaturation = infoColor.nSaturation;
	nContrast = infoColor.nContrast;
	nBrightness = infoColor.nBrightness;
}
// Color[E]

// Mosaic Control[S]
void CSOLO6010::MOSAIC_SetOnOffAndRect (BYTE idxCh, BYTE bSet, RECT *pRcMosaic)
{
	if (idxCh < 0 || idxCh >= NUM_LIVE_DISP_WND)
	{
		return;
	}

	m_iMosaic.bufBSetMosaic[idxCh] = bSet;
	m_iMosaic.bufPosHStart[idxCh] = (unsigned short)pRcMosaic->left;
	m_iMosaic.bufPosHEnd[idxCh] = (unsigned short)pRcMosaic->right;
	m_iMosaic.bufPosVStart[idxCh] = (unsigned short)pRcMosaic->top;
	m_iMosaic.bufPosVEnd[idxCh] = (unsigned short)pRcMosaic->bottom;

	MOSAIC_SetInfo (&m_iMosaic);
}

void CSOLO6010::MOSAIC_GetOnOffAndRect (BYTE idxCh, BYTE *pBSet, RECT *pRcMosaic)
{
	if (idxCh < 0 || idxCh >= NUM_LIVE_DISP_WND)
	{
		return;
	}

	*pBSet = m_iMosaic.bufBSetMosaic[idxCh];
	(*pRcMosaic).left = m_iMosaic.bufPosHStart[idxCh];
	(*pRcMosaic).right = m_iMosaic.bufPosHEnd[idxCh];
	(*pRcMosaic).top = m_iMosaic.bufPosVStart[idxCh];
	(*pRcMosaic).bottom = m_iMosaic.bufPosVEnd[idxCh];
}

void CSOLO6010::MOSAIC_SetStrength (DWORD nStrength)
{
	if (nStrength < MIN_MOSAIC_STRENGTH)
	{
		nStrength = MIN_MOSAIC_STRENGTH;
	}
	if (nStrength > MAX_MOSAIC_STRENGTH)
	{
		nStrength = MAX_MOSAIC_STRENGTH;
	}

	m_iMosaic.nStrength = nStrength;

	MOSAIC_SetInfo (&m_iMosaic);
}

DWORD CSOLO6010::MOSAIC_GetStrength ()
{
	return m_iMosaic.nStrength;
}

void CSOLO6010::MOSAIC_SetInfo (INFO_MOSAIC *pInfoMosaic)
{
	DWORD szRead;
	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_MOSAIC_PROP, NULL, 0, pInfoMosaic,
		sizeof(INFO_MOSAIC), &szRead, NULL);
}
// Mosaic Control[E]

// Video Motion[S]
void CSOLO6010::VMOTION_SetOnOff (BYTE idxCh, BYTE bOn)
{
	DWORD szRead;

	m_iVMotion.bufBUseMotion[idxCh] = bOn;

	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_V_MOTION_PROP, NULL, 0, &m_iVMotion,
		sizeof(INFO_V_MOTION), &szRead, NULL);
}

void CSOLO6010::VMOTION_SetThreshold (BYTE idxCh, WORD *pBufThreshold)
{
	DWORD szRead;

	m_iVMotion.bufBSetMotionTh[idxCh] = 1;
	memcpy (m_iVMotion.bbufMotionTh[idxCh], pBufThreshold, SZ_BUF_V_MOTION_TH_IN_BYTE);

	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_V_MOTION_PROP, NULL, 0, &m_iVMotion,
		sizeof(INFO_V_MOTION), &szRead, NULL);

	m_iVMotion.bufBSetMotionTh[idxCh] = 0;
}

void CSOLO6010::VMOTION_SetMinDetectThreshold (WORD minDetThreshold)
{
	DWORD szRead;

	m_iVMotion.bSetCntMinDet = 1;
	m_iVMotion.cntMinDet = minDetThreshold;

	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_V_MOTION_PROP, NULL, 0, &m_iVMotion,
		sizeof(INFO_V_MOTION), &szRead, NULL);

	m_iVMotion.bSetCntMinDet = 0;
}

void CSOLO6010::VMOTION_SetGridOnOff (BYTE bOn)
{
	DWORD szRead;

	m_iVMotion.bSetShowGrid = 1;
	m_iVMotion.bShowGrid = bOn;

	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_V_MOTION_PROP, NULL, 0, &m_iVMotion,
		sizeof(INFO_V_MOTION), &szRead, NULL);

	m_iVMotion.bSetShowGrid = 0;
}

void CSOLO6010::VMOTION_SetBorderColor (BYTE idxColModeY, BYTE valY,
		BYTE idxColModeCb, BYTE valCb, BYTE idxColModeCr, BYTE valCr)
{
	DWORD szRead;

	m_iVMotion.bSetShowGrid = 1;

	if (idxColModeY != VMOTION_NOT_CHANGE_PROP)
	{
		m_iVMotion.bufModeColorSet[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_Y] = idxColModeY;
	}
	if (valY != VMOTION_NOT_CHANGE_PROP)
	{
		m_iVMotion.bufColor[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_Y] = valY;
	}
	if (idxColModeCb != VMOTION_NOT_CHANGE_PROP)
	{
		m_iVMotion.bufModeColorSet[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CB] = idxColModeCb;
	}
	if (valCb != VMOTION_NOT_CHANGE_PROP)
	{
		m_iVMotion.bufColor[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CB] = valCb;
	}
	if (idxColModeCr != VMOTION_NOT_CHANGE_PROP)
	{
		m_iVMotion.bufModeColorSet[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CR] = idxColModeCr;
	}
	if (valCr != VMOTION_NOT_CHANGE_PROP)
	{
		m_iVMotion.bufColor[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CR] = valCr;
	}

	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_V_MOTION_PROP, NULL, 0, &m_iVMotion,
		sizeof(INFO_V_MOTION), &szRead, NULL);

	m_iVMotion.bSetShowGrid = 0;
}

void CSOLO6010::VMOTION_SetBarColor (BYTE idxColModeY, BYTE valY,
		BYTE idxColModeCb, BYTE valCb, BYTE idxColModeCr, BYTE valCr)
{
	DWORD szRead;

	m_iVMotion.bSetShowGrid = 1;

	if (idxColModeY != VMOTION_NOT_CHANGE_PROP)
	{
		m_iVMotion.bufModeColorSet[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_Y] = idxColModeY;
	}
	if (valY != VMOTION_NOT_CHANGE_PROP)
	{
		m_iVMotion.bufColor[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_Y] = valY;
	}
	if (idxColModeCb != VMOTION_NOT_CHANGE_PROP)
	{
		m_iVMotion.bufModeColorSet[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CB] = idxColModeCb;
	}
	if (valCb != VMOTION_NOT_CHANGE_PROP)
	{
		m_iVMotion.bufColor[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CB] = valCb;
	}
	if (idxColModeCr != VMOTION_NOT_CHANGE_PROP)
	{
		m_iVMotion.bufModeColorSet[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CR] = idxColModeCr;
	}
	if (valCr != VMOTION_NOT_CHANGE_PROP)
	{
		m_iVMotion.bufColor[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CR] = valCr;
	}

	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_V_MOTION_PROP, NULL, 0, &m_iVMotion,
		sizeof(INFO_V_MOTION), &szRead, NULL);

	m_iVMotion.bSetShowGrid = 0;
}

void CSOLO6010::VMOTION_GetBorderColor (BYTE &idxColModeY, BYTE &valY,
		BYTE &idxColModeCb, BYTE &valCb, BYTE &idxColModeCr, BYTE &valCr)
{
	idxColModeY = m_iVMotion.bufModeColorSet[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_Y];
	valY = m_iVMotion.bufColor[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_Y];
	idxColModeCb = m_iVMotion.bufModeColorSet[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CB];
	valCb = m_iVMotion.bufColor[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CB];
	idxColModeCr = m_iVMotion.bufModeColorSet[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CR];
	valCr = m_iVMotion.bufColor[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CR];
}

void CSOLO6010::VMOTION_GetBarColor (BYTE &idxColModeY, BYTE &valY,
		BYTE &idxColModeCb, BYTE &valCb, BYTE &idxColModeCr, BYTE &valCr)
{
	idxColModeY = m_iVMotion.bufModeColorSet[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_Y];
	valY = m_iVMotion.bufColor[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_Y];
	idxColModeCb = m_iVMotion.bufModeColorSet[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CB];
	valCb = m_iVMotion.bufColor[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CB];
	idxColModeCr = m_iVMotion.bufModeColorSet[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CR];
	valCr = m_iVMotion.bufColor[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CR];
}

BYTE *CSOLO6010::VMOTION_GetCurMotionMap ()
{
	DWORD szRead;

	m_iVMotion.bGetCurMotionMap = 1;

	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_V_MOTION_PROP, NULL, 0, &m_iVMotion,
		sizeof(INFO_V_MOTION), &szRead, NULL);

	m_iVMotion.bGetCurMotionMap = 0;

	return &m_iVMotion.bufCurMotionMap[0];
	// bufCurMotionMap is current Video Motion Map for all channel.
	// 512Byte per channel.
	// 1bit per 16x16 pixel block.
	// Horizontal size of Motion Map is 8Byte. (64bit = 1024 pixel)
	// 8Byte *64 vertical block = 512Byte
}

void CSOLO6010::VMOTION_RegisterAlarmCallback (V_MOTION_ALARM_PROC cbFnVMotionAlarm, void *pContext)
{
	m_bufCbProc[S6010_CB_V_MOTION_ALARM] = (LPVOID)cbFnVMotionAlarm;
	m_bufCbContext[S6010_CB_V_MOTION_ALARM] = (LPVOID)pContext;
}

void CSOLO6010::VMOTION_UnregisterAlarmCallback ()
{
	m_bufCbProc[S6010_CB_V_MOTION_ALARM] = NULL;
	m_bufCbContext[S6010_CB_V_MOTION_ALARM] = NULL;
}
// Video Motion[E]

// Camera block detection[S]
void CSOLO6010::CAM_BLK_DET_SetOnOff (BYTE idxCh, BYTE bOn)
{
	m_iCamBlkDet.bufBUseDetection[idxCh] = bOn;
}

void CSOLO6010::CAM_BLK_DET_SetAlarmLevel (BYTE idxCh, BYTE nAlarmLevel)
{
	m_iCamBlkDet.bufAlarmLevel[idxCh] = nAlarmLevel;
}

void CSOLO6010::CAM_BLK_DET_SetCheckInterval (DWORD nCheckInterval)
{
	m_iCamBlkDet.timeCheckInterval = nCheckInterval;
}

void CSOLO6010::CAM_BLK_DET_RegisterAlarmCallback (CAM_BLK_DET_ALARM_PROC cbFnCamBlkDetAlarm, void *pContext)
{
	m_bufCbProc[S6010_CB_CAM_BLK_DET_ALARM] = (LPVOID)cbFnCamBlkDetAlarm;
	m_bufCbContext[S6010_CB_CAM_BLK_DET_ALARM] = (LPVOID)pContext;
}

void CSOLO6010::CAM_BLK_DET_UnregisterAlarmCallback ()
{
	m_bufCbProc[S6010_CB_CAM_BLK_DET_ALARM] = NULL;
	m_bufCbContext[S6010_CB_CAM_BLK_DET_ALARM] = NULL;
}
// Camera block detection[E]

// Advanced motion detection[S]
void CSOLO6010::ADV_MOT_SetOnOff (BYTE idxCh, BYTE bOn)
{
	if (idxCh >= NUM_LIVE_DISP_WND || (bOn != 0 && bOn != 1))
	{
		return;
	}

	m_iAdvMotDet.bufBUseMotion[idxCh] = bOn;

	int i, bAdvMotDetOn;

	bAdvMotDetOn = 0;
	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		if (m_iAdvMotDet.bufBUseMotion[i] == 1)
		{
			bAdvMotDetOn = 1;
		}
	}

	DWORD szRead;
	if (m_iAdvMotDet.bUseAdvMotion == 0 && bAdvMotDetOn == 1)
	{	// OFF -> ON
		DeviceIoControl (m_hS6010, S6010_IOCTL_START_ADV_MOTION_DATA_GET, NULL, 0, NULL, 0, &szRead, NULL);

		m_iAdvMotDet.bInitOK = 0;
		m_iAdvMotDet.cntNumGetData = 0;
		m_iAdvMotDet.bUseAdvMotion = 1;
	}
	else if (m_iAdvMotDet.bUseAdvMotion == 1 && bAdvMotDetOn == 0)
	{	// ON -> OFF
		DeviceIoControl (m_hS6010, S6010_IOCTL_END_ADV_MOTION_DATA_GET, NULL, 0, NULL, 0, &szRead, NULL);

		m_iAdvMotDet.bUseAdvMotion = 0;
	}
}

void CSOLO6010::ADV_MOT_SetNumMotChkItem (BYTE idxCh, BYTE numMotChkItem)
{
	if (idxCh >= NUM_LIVE_DISP_WND || numMotChkItem > MAX_ADV_MOT_CHECK_ITEM)
	{
		return;
	}

	m_iAdvMotDet.bufNumMotionCheck[idxCh] = numMotChkItem;
}

void CSOLO6010::ADV_MOT_SetMotChkDist (BYTE idxCh, BYTE idxChkItem, BYTE numDist)
{
	if (idxCh >= NUM_LIVE_DISP_WND || idxChkItem >= MAX_ADV_MOT_CHECK_ITEM)
	{
		return;
	}

	m_iAdvMotDet.bbufMotDataDist[idxCh][idxChkItem] = numDist;
}

void CSOLO6010::ADV_MOT_SetThreshold (BYTE idxCh, BYTE idxChkItem, WORD *pBufThreshold)
{
	if (idxCh >= NUM_LIVE_DISP_WND || idxChkItem >= MAX_ADV_MOT_CHECK_ITEM)
	{
		return;
	}

	memcpy (m_iAdvMotDet.bbbufMotionTh[idxCh][idxChkItem], pBufThreshold, SZ_BUF_V_MOTION_TH_IN_BYTE);
}

void CSOLO6010::ADV_MOT_SetMinDetectThreshold (BYTE idxCh, WORD minDetThreshold)
{
	if (idxCh >= NUM_LIVE_DISP_WND)
	{
		return;
	}

	m_iAdvMotDet.bbufCntMinDet[idxCh] = minDetThreshold;
}

void CSOLO6010::ADV_MOT_RegisterAlarmCallback (ADV_MOT_DET_ALARM_PROC cbFnAdvMotDetAlarm, void *pContext)
{
	m_bufCbProc[S6010_CB_ADV_MOT_DET_ALARM] = (LPVOID)cbFnAdvMotDetAlarm;
	m_bufCbContext[S6010_CB_ADV_MOT_DET_ALARM] = (LPVOID)pContext;
}

void CSOLO6010::ADV_MOT_UnregisterAlarmCallback ()
{
	m_bufCbProc[S6010_CB_ADV_MOT_DET_ALARM] = NULL;
	m_bufCbContext[S6010_CB_ADV_MOT_DET_ALARM] = NULL;
}

void CSOLO6010::ADV_MOT_CheckAdvMotion (DWORD &flagAdvMotDet, BYTE *bufMotionMap)
{
	int i, j, k, l, nTmp, numMotionCell, flagChk;
	int posCur = m_iAdvMotDet.curWCDataPos;
	int szBlkH = m_iS6010Init.szVideoH >>4;
	int szBlkV = (m_iS6010Init.szVideoV *2) >>4;
	unsigned short *pSrcWC, *pDestWC, *pBufTh;
	BYTE *pBufMotionMap;

	memset (bufMotionMap, 0, SZ_BUF_V_MOTION_TH_IN_USHORT *NUM_LIVE_DISP_WND);	// Clear Motion Map
	flagAdvMotDet = 0;

	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		if (m_iAdvMotDet.bufBUseMotion[i] == 0)
		{
			continue;
		}

		// Get All Motion Map[S]
		for (j=0; j<m_iAdvMotDet.bufNumMotionCheck[i]; j++)
		{
			for (k=0; k<szBlkV; k++)
			{
				pSrcWC = &m_iAdvMotDet.bbufCurWCData[posCur]
					[i *(2 *SZ_BUF_V_MOTION_TH_IN_USHORT) +SZ_BUF_V_MOTION_TH_IN_USHORT +64 *k];
				pDestWC = &m_iAdvMotDet.bbufCurWCData[(posCur -m_iAdvMotDet.bbufMotDataDist[i][j] +MAX_MOT_DATA_DISTANCE) %MAX_MOT_DATA_DISTANCE]
					[i *(2 *SZ_BUF_V_MOTION_TH_IN_USHORT) +SZ_BUF_V_MOTION_TH_IN_USHORT +64 *k];
				pBufTh = &m_iAdvMotDet.bbbufMotionTh[i][j][64 *k];
				pBufMotionMap = &bufMotionMap[SZ_BUF_V_MOTION_TH_IN_USHORT *i +64 *k];

				for (l=0; l<szBlkH; l++)
				{
					nTmp = int(*pSrcWC) -int(*pDestWC);
					if (nTmp < 0)
					{
						nTmp = -nTmp;
					}
					if (nTmp > *pBufTh)
					{
						*pBufMotionMap |= (1 <<j);
					}

					pSrcWC++;
					pDestWC++;
					pBufTh++;
					pBufMotionMap++;
				}
			}
		}
		// Get All Motion Map[E]

		// Exec Motion Detection[S]
		flagChk = (1 <<m_iAdvMotDet.bufNumMotionCheck[i]) -1;

		numMotionCell = 0;
		for (j=0; j<szBlkV; j++)
		{
			pBufMotionMap = &bufMotionMap[SZ_BUF_V_MOTION_TH_IN_USHORT *i +64 *j];

			for (k=0; k<szBlkH; k++)
			{
				if (*pBufMotionMap == flagChk)
				{
					*pBufMotionMap = 0xff;
					numMotionCell++;
				}

				pBufMotionMap++;
			}
		}
		if (numMotionCell > m_iAdvMotDet.bbufCntMinDet[i])
		{
			flagAdvMotDet |= 1 <<i;
		}
		// Exec Motion Detection[E]
	}
}
// Advanced motion detection[E]

// Video Matrix Control[S]
void CSOLO6010::VMATRIX_Set (BYTE *bufCamToCh)
{
	int i;
	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		if (bufCamToCh[i] >= NUM_LIVE_DISP_WND)
		{
			m_iVMatrix.bufIdxCamera[i] = 0;
		}
		else
		{
			m_iVMatrix.bufIdxCamera[i] = bufCamToCh[i];
		}
	}
	VMATRIX_SetInfo (&m_iVMatrix);
}

void CSOLO6010::VMATRIX_Get (BYTE *bufCamToCh)
{
	int i;
	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		bufCamToCh[i] = m_iVMatrix.bufIdxCamera[i];
	}
}

void CSOLO6010::VMATRIX_SetInfo (INFO_VIDEO_MATRIX *pInfoVMatrix)
{
	DWORD szRead;
	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_CAM_TO_WND_PROP, NULL, 0, pInfoVMatrix,
		sizeof(INFO_VIDEO_MATRIX), &szRead, NULL);
}
// Video Matrix Control[E]

DWORD CSOLO6010::GetNumMP4EncCRU (SETTING_MP4_REC *pInfoMP4Enc)
{
	DWORD numFPS, num1FrmCRU;

	if (pInfoMP4Enc->bSet == 0)
	{
		return 0;
	}

	if (pInfoMP4Enc->idxImgSz >= IMG_SZ_704X480_704X576)
	{	// Frame
		numFPS = 30 *20 /(GL_FPS_TO_FRAME_INTERVAL[m_iS6010Init.typeVideo][pInfoMP4Enc->idxFPS] +1);
		num1FrmCRU = NUM_CRU_704_480;
	}
	else
	{	// Field
		if (pInfoMP4Enc->idxFPS == 0)
		{
			numFPS = 60 *20;
		}
		else
		{
			numFPS = 30 *20 /GL_FPS_TO_FIELD_INTERVAL[m_iS6010Init.typeVideo][pInfoMP4Enc->idxFPS];
		}

		switch (pInfoMP4Enc->idxImgSz)
		{
		case IMG_SZ_704X240_704X288:
			num1FrmCRU = NUM_CRU_704_240;
			break;
		case IMG_SZ_352X240_352X288:
			num1FrmCRU = NUM_CRU_352_240;
			break;
		case IMG_SZ_176X112_176X144:
			num1FrmCRU = NUM_CRU_176_112;
			break;
		}
	}

	return (num1FrmCRU *numFPS +19) /20;
}

BOOL CSOLO6010::IsValidInfoRec (SETTING_REC *pSetRec)
{
	int i;
	DWORD nTmpMP4EncCRU = 0;
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		if (pSetRec->setMP4[i].bSet == TRUE)
		{
			nTmpMP4EncCRU += GetNumMP4EncCRU (&pSetRec->setMP4[i]);
		}
	}
	if (nTmpMP4EncCRU > MAX_NUM_CRU_MP4_ENC)
	{
		return FALSE;
	}

	return TRUE;
}


// HW MP4 decoding[S]
INFO_MP4_DEC_STAT *CSOLO6010::MP4D_Open ()
{
	if (m_hS6010 == NULL)
	{
		return NULL;
	}

	DWORD szRead;
	DeviceIoControl (m_hS6010, S6010_IOCTL_OPEN_HW_MP4_DEC, NULL, 0, NULL, 0, &szRead, NULL);

	return &m_pIShare->iMP4DecStat;
}

void CSOLO6010::MP4D_Close ()
{
	if (m_hS6010 == NULL)
	{
		return;
	}

	DWORD szRead;
	DeviceIoControl (m_hS6010, S6010_IOCTL_CLOSE_HW_MP4_DEC, NULL, 0, NULL, 0, &szRead, NULL);
}

BYTE *CSOLO6010::MP4D_GetFrameBuffer (DWORD &posStart, DWORD &posEnd, DWORD &szBufFrm, DWORD &numRemainQ)
{
	int idxQAppWrite, idxQDecOK;

	szBufFrm = m_pIShare->iMP4DecStat.szBufMP4Dec;
	idxQAppWrite = m_pIShare->iMP4DecStat.idxQAppWrite;
	idxQDecOK = m_pIShare->iMP4DecStat.idxQDecOK;

	int idxQDecOKM1 = (idxQDecOK -1 +SZ_MP4_DEC_QUEUE_ITEM) &(SZ_MP4_DEC_QUEUE_ITEM -1);
	int idxQAppWriteM1 = (idxQAppWrite -1 +SZ_MP4_DEC_QUEUE_ITEM) &(SZ_MP4_DEC_QUEUE_ITEM -1);
	int posAppWrite = (m_pIShare->iMP4DecStat.iQueue[idxQAppWriteM1].posFrm +m_pIShare->iMP4DecStat.iQueue[idxQAppWriteM1].szFrm) %szBufFrm;
	int posDecOK = (m_pIShare->iMP4DecStat.iQueue[idxQDecOKM1].posFrm +szBufFrm) %szBufFrm;

	if (((posDecOK -posAppWrite +szBufFrm) %szBufFrm <= (szBufFrm >>2)) ||
		((idxQDecOK -idxQAppWrite +SZ_MP4_DEC_QUEUE_ITEM) &(SZ_MP4_DEC_QUEUE_ITEM -1) <= (SZ_MP4_DEC_QUEUE_ITEM >>2)))
	{
		posStart = posAppWrite;
		posEnd = posStart;
		numRemainQ = 0;
	}
	else
	{
		posStart = posAppWrite;
		posEnd = (posDecOK -(szBufFrm >>2) +szBufFrm) %szBufFrm;
		numRemainQ = (idxQDecOK -idxQAppWrite -(SZ_MP4_DEC_QUEUE_ITEM >>2) +SZ_MP4_DEC_QUEUE_ITEM) &(SZ_MP4_DEC_QUEUE_ITEM -1);
	}

	return m_pIShare->iMP4DecStat.pBufMP4Dec;
}

int CSOLO6010::MP4D_UpdateFrameBufferStat (int numFrm, DWORD *bufPosFrm, DWORD *bufIdxFrm, DWORD *bufTimeMS, BYTE *bufIdxDecMode, BYTE *bufBFrmIntp)
{
	int i, idxCurQueue, idxWnd, numInsFrm;
	HEADER_MP4_FRM *pHdrMP4Frm;
	BYTE *pBufFrm = m_pIShare->iMP4DecStat.pBufMP4Dec;

	idxCurQueue = m_pIShare->iMP4DecStat.idxQAppWrite;
	for (i=0; i<numFrm; i++)
	{
		pHdrMP4Frm = (HEADER_MP4_FRM *)&pBufFrm[bufPosFrm[i]];

		idxWnd = LIVE_GetIdxWndFromDecCh (GET_CH_PT(pHdrMP4Frm));
		if (idxWnd == INVALID_WND_IDX)
		{
			continue;
		}

		m_pIShare->iMP4DecStat.iQueue[idxCurQueue].posFrm = bufPosFrm[i];
		m_pIShare->iMP4DecStat.iQueue[idxCurQueue].szFrm = sizeof(HEADER_MP4_FRM) +GET_MP4_CODE_SZ_PT(pHdrMP4Frm);
		m_pIShare->iMP4DecStat.iQueue[idxCurQueue].idxFrm = bufIdxFrm[i];
		m_pIShare->iMP4DecStat.iQueue[idxCurQueue].timeMS = bufTimeMS[i];

		m_pIShare->iMP4DecStat.iQueue[idxCurQueue].idxWnd = idxWnd;
		m_pIShare->iMP4DecStat.iQueue[idxCurQueue].idxDecMode = bufIdxDecMode[i];
		m_pIShare->iMP4DecStat.iQueue[idxCurQueue].bFrameIntp = bufBFrmIntp[i];
		
		idxCurQueue = (idxCurQueue +1) &(SZ_MP4_DEC_QUEUE_ITEM -1);
	}
	numInsFrm = (idxCurQueue -m_pIShare->iMP4DecStat.idxQAppWrite +SZ_MP4_DEC_QUEUE_ITEM) &(SZ_MP4_DEC_QUEUE_ITEM -1);
	m_pIShare->iMP4DecStat.idxQAppWrite = idxCurQueue;

	SetEvent (m_bufHEvt[COMMON_EVT_MP4D_COPY_DATA]);

	while (m_pIShare->iMP4DecStat.idxQDrvWrite != m_pIShare->iMP4DecStat.idxQAppWrite)
	{
		Sleep (0);
	}

	return numInsFrm;
}

void CSOLO6010::MP4D_DecodeNFrm (int numDec)
{	// Simply, increase "iMP4DecStat.idxDecNeed"
	m_pIShare->iMP4DecStat.idxQDecNeed = (m_pIShare->iMP4DecStat.idxQDecNeed +numDec) &(SZ_MP4_DEC_QUEUE_ITEM -1);
}

void CSOLO6010::MP4D_PauseDecoding ()
{	// Set "iMP4DecStat.idxDecNeed" to "iMP4DecStat.idxDecOK" in driver
	m_pIShare->iMP4DecStat.reqPauseDecoding = 1;
	while (m_pIShare->iMP4DecStat.reqPauseDecoding == 1)
	{
		Sleep (0);
	}

	while (1)
	{
		if (WaitForSingleObject (m_bufHEvt[COMMON_EVT_MP4D_COPY_DATA], 0) == WAIT_TIMEOUT)
			break;
	}

	m_pIShare->iMP4DecStat.idxQDrvWrite = m_pIShare->iMP4DecStat.idxQDecNeed;
	m_pIShare->iMP4DecStat.idxQAppWrite = m_pIShare->iMP4DecStat.idxQDecNeed;
}
// HW MP4 decoding[E]


// HW G723 decoding[S]
INFO_G723_DEC_STAT *CSOLO6010::G723D_Open (int idxSamplingRate)
{
	if (m_hS6010 == NULL)
	{
		return NULL;
	}

	m_pIShare->iG723DecStat.idxSamplingRate = idxSamplingRate;

	DWORD szRead;
	DeviceIoControl (m_hS6010, S6010_IOCTL_OPEN_HW_G723_DEC, NULL, 0, NULL, 0, &szRead, NULL);

	return &m_pIShare->iG723DecStat;
}

void CSOLO6010::G723D_Close ()
{
	if (m_hS6010 == NULL)
	{
		return;
	}

	DWORD szRead;
	DeviceIoControl (m_hS6010, S6010_IOCTL_CLOSE_HW_G723_DEC, NULL, 0, NULL, 0, &szRead, NULL);
}

void CSOLO6010::G723D_Play ()
{
	if (m_hS6010 == NULL)
	{
		return;
	}

	DWORD szRead;
	DeviceIoControl (m_hS6010, S6010_IOCTL_PLAY_HW_G723_DEC, NULL, 0, NULL, 0, &szRead, NULL);
}

void CSOLO6010::G723D_Stop ()
{
	if (m_hS6010 == NULL)
	{
		return;
	}

	DWORD szRead;
	DeviceIoControl (m_hS6010, S6010_IOCTL_STOP_HW_G723_DEC, NULL, 0, NULL, 0, &szRead, NULL);
}
// HW G723 decoding[E]


// GPIO[S]
void CSOLO6010::GPIO_EnableInterrupt (DWORD maskPort, DWORD intCfg)
{
	if (intCfg == GPIO_INT_CFG_DISABLE_INTERRUPT)
	{
		GPIO_Operation (GPIO_OP_SET_DISABLE_INT, maskPort, 0);
	}
	else
	{
		GPIO_Operation (GPIO_OP_SET_ENABLE_INT, maskPort, intCfg);
	}
}

unsigned long CSOLO6010::GPIO_Operation (BYTE idxOp, DWORD maskPort, DWORD nData)
{
	DWORD szRead;
	INFO_GPIO iGPIO;

	memset (&iGPIO, 0, sizeof(INFO_GPIO));
	iGPIO.idxOp = idxOp;
	iGPIO.maskPort = maskPort;
	iGPIO.nData = nData;

	DeviceIoControl (m_hS6010, S6010_IOCTL_GPIO_CTRL, NULL, 0, &iGPIO, sizeof(INFO_GPIO), &szRead, NULL);

	return iGPIO.nData;
}

void CSOLO6010::GPIO_RegisterIntAlarmCallback (GPIO_INT_ALARM_PROC cbFnGPIO_IntAlarm, void *pContext)
{
	m_bufCbProc[S6010_CB_GPIO_INT_ALARM] = (LPVOID)cbFnGPIO_IntAlarm;
	m_bufCbContext[S6010_CB_GPIO_INT_ALARM] = (LPVOID)pContext;
}

void CSOLO6010::GPIO_UnregisterIntAlarmCallback ()
{
	m_bufCbProc[S6010_CB_GPIO_INT_ALARM] = NULL;
	m_bufCbContext[S6010_CB_GPIO_INT_ALARM] = NULL;
}
// GPIO[E]

// Video Loss[S]
DWORD CSOLO6010::VLOSS_GetState ()
{
	DWORD szRead;
	INFO_VLOSS iVLoss;

	DeviceIoControl (m_hS6010, S6010_IOCTL_GET_VLOSS_STATE, NULL, 0, &iVLoss, sizeof(INFO_VLOSS), &szRead, NULL);

	return iVLoss.maskVLoss;
}
// Video Loss[E]

// Encoder OSD[S]
void CSOLO6010::ENC_OSD_Init (BOOL bUseShadowText, BYTE colorY, BYTE colorU, BYTE colorV, BYTE offsetH, BYTE offsetV)
{
	DWORD szRead;

	m_pIShare->iEncOSD.bUseShadowText = bUseShadowText;
	m_pIShare->iEncOSD.clrY = colorY;
	m_pIShare->iEncOSD.clrU = colorU;
	m_pIShare->iEncOSD.clrV = colorV;
	m_pIShare->iEncOSD.offsetH = offsetH;
	m_pIShare->iEncOSD.offsetV = offsetV;

	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_ENC_OSD_PROP, NULL, 0, 0, 0, &szRead, NULL);
}

void CSOLO6010::ENC_OSD_Clear (int idxCh)
{
	DWORD szRead;

	m_pIShare->iEncOSD.idxCh = idxCh;
	m_pIShare->iEncOSD.posY = 0;
	m_pIShare->iEncOSD.szV = 0x10000 /2048;

	memset (m_pIShare->pBufEncOSD, 0, 0x10000);

	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_ENC_OSD, NULL, 0, 0, 0, &szRead, NULL);
}

void CSOLO6010::ENC_OSD_Set (int idxCh, const char*strOSD, DWORD lenStrOSD, int idxStartLine)
{
	DWORD szRead, i, j, idxCol;

	BYTE *pBufOSD = m_pIShare->pBufEncOSD;

	m_pIShare->iEncOSD.idxCh = idxCh;
	m_pIShare->iEncOSD.posY = idxStartLine;

	m_pIShare->iEncOSD.szV = 1;
	idxCol = 0;
	for (i=0; i<lenStrOSD; i++)
	{
		if (strOSD[i] == '\n' || strOSD[i] == '\r')
		{
			pBufOSD += 2048;
			m_pIShare->iEncOSD.szV++;
			idxCol = 0;

			continue;
		}
		for (j=0; j<16; j++)
		{
			pBufOSD[(j *2) +(idxCol %2) +(idxCol /2 *32)] = (ENC_OSD_FONT[(strOSD[i] *4) +(j /4)] >>((3 -(j %4)) *8)) &0xff;
		}
		idxCol++;
	}

	DeviceIoControl (m_hS6010, S6010_IOCTL_SET_ENC_OSD, NULL, 0, 0, 0, &szRead, NULL);
}
// Encoder OSD[E]

// UART[S]
void CSOLO6010::UART_Setup (int idxPort, int idxBaudRate, int idxDataSize, int idxStopBit, int idxParity)
{
	DWORD szRead;
	INFO_UART_SETUP iUART_Setup;

	iUART_Setup.idxPort = idxPort;

	iUART_Setup.idxBaudRate = idxBaudRate;
	iUART_Setup.idxDataSize = idxDataSize;
	iUART_Setup.idxStopBit = idxStopBit;
	iUART_Setup.idxParity = idxParity;

	iUART_Setup.bEnableCTS_RTS = FALSE;
	iUART_Setup.bEnableDataDropWhenParityError = FALSE;
	
	DeviceIoControl (m_hS6010, S6010_IOCTL_SETUP_UART, NULL, 0, &iUART_Setup, sizeof(INFO_UART_SETUP), &szRead, NULL);
}

void CSOLO6010::UART_Read (int idxPort, BYTE *pBufRead, int szBufRead, int *pSzActualRead)
{
	DWORD szRead;
	INFO_UART_RW iUART_RW;

	iUART_RW.idxPort = idxPort;
	iUART_RW.szBufData = szBufRead;
	iUART_RW.pBufData = pBufRead;

	DeviceIoControl (m_hS6010, S6010_IOCTL_READ_UART, NULL, 0, &iUART_RW, sizeof(INFO_UART_RW), &szRead, NULL);

	*pSzActualRead = iUART_RW.szActualRW;
}

void CSOLO6010::UART_Write (int idxPort, BYTE *pBufWrite, int szBufWrite, int *pSzActualWritten)
{
	DWORD szRead;
	INFO_UART_RW iUART_RW;

	iUART_RW.idxPort = idxPort;
	iUART_RW.szBufData = szBufWrite;
	iUART_RW.pBufData = pBufWrite;

	DeviceIoControl (m_hS6010, S6010_IOCTL_WRITE_UART, NULL, 0, &iUART_RW, sizeof(INFO_UART_RW), &szRead, NULL);

	*pSzActualWritten = iUART_RW.szActualRW;
}
// UART[E]

// I2C I/O[S]
int CSOLO6010::I2C_Read (int nChannel, int nAddrSlave, int nAddrSub)
{
	DWORD szRead;
	INFO_I2C_RW iI2C_RW;

	iI2C_RW.nChannel = nChannel;
	iI2C_RW.nAddrSlave = nAddrSlave;
	iI2C_RW.nAddrSub = nAddrSub;

	DeviceIoControl (m_hS6010, S6010_IOCTL_I2C_READ, NULL, 0, &iI2C_RW, sizeof(INFO_I2C_RW), &szRead, NULL);

	return iI2C_RW.nData;
}

int CSOLO6010::I2C_Write (int nChannel, int nAddrSlave, int nAddrSub, int nData)
{
	DWORD szRead;
	INFO_I2C_RW iI2C_RW;

	iI2C_RW.nChannel = nChannel;
	iI2C_RW.nAddrSlave = nAddrSlave;
	iI2C_RW.nAddrSub = nAddrSub;
	iI2C_RW.nData = nData;

	DeviceIoControl (m_hS6010, S6010_IOCTL_I2C_WRITE, NULL, 0, &iI2C_RW, sizeof(INFO_I2C_RW), &szRead, NULL);

	return iI2C_RW.nData;
}
// I2C I/O[E]

