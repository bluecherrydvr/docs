// PlayerManager_JPEG.cpp: implementation of the CPlayerManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "solo6010app.h"
#include "PlayerManager.h"
#include "WndPlayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BOOL Decode_JPEG (CPlayerManager *pPlayer);

#define SOI_OFFSET			0
#define SOI_LEN				2
const BYTE JPEG_SOI[SOI_LEN] = {
	0xff, 0xd8
};

#define SetSOI(pHdrJPEG)	memcpy ((BYTE *)(pHdrJPEG) +SOI_OFFSET, JPEG_SOI, SOI_LEN)

#define DQT_OFFSET			(SOI_OFFSET +SOI_LEN)
#define DQT_LEN				138

const BYTE JPEG_DQT[NUM_JPEG_IMG_QUALITY][DQT_LEN] =
{
	0xff,0xdb,0x00,0x43,0x00,	// low quality
	0x10 *2,0x0b *2,0x0c *2,0x0e *2,0x0c *2,0x0a *2,0x10 *2,0x0e *2,
	0x0d *2,0x0e *2,0x12 *2,0x11 *2,0x10 *2,0x13 *2,0x18 *2,0x28 *2,
	0x1a *2,0x18 *2,0x16 *2,0x16 *2,0x18 *2,0x31 *2,0x23 *2,0x25 *2,
	0x1d *2,0x28 *2,0x3a *2,0x33 *2,0x3d *2,0x3c *2,0x39 *2,0x33 *2,
	0x38 *2,0x37 *2,0x40 *2,0x48 *2,0x5c *2,0x4e *2,0x40 *2,0x44 *2,
	0x57 *2,0x45 *2,0x37 *2,0x38 *2,0x50 *2,0x6d *2,0x51 *2,0x57 *2,
	0x5f *2,0x62 *2,0x67 *2,0x68 *2,0x67 *2,0x3e *2,0x4d *2,0x71 *2,
	0x79 *2,0x70 *2,0x64 *2,0x78 *2,0x5c *2,0x65 *2,0x67 *2,0x63 *2,
	0xff,0xdb,0x00,0x43,0x01,
	0x11 *2,0x12 *2,0x12 *2,0x18 *2,0x15 *2,0x18 *2,0x2f *2,0x1a *2,
	0x1a *2,0x2f *2,0x63 *2,0x42 *2,0x38 *2,0x42 *2,0x63 *2,0x63 *2,
	0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,
	0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,
	0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,
	0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,
	0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,
	0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,0x63 *2,

	0xff,0xdb,0x00,0x43,0x00,	// medium quality
	0x10,0x0b,0x0c,0x0e,0x0c,0x0a,0x10,0x0e,
	0x0d,0x0e,0x12,0x11,0x10,0x13,0x18,0x28,
	0x1a,0x18,0x16,0x16,0x18,0x31,0x23,0x25,
	0x1d,0x28,0x3a,0x33,0x3d,0x3c,0x39,0x33,
	0x38,0x37,0x40,0x48,0x5c,0x4e,0x40,0x44,
	0x57,0x45,0x37,0x38,0x50,0x6d,0x51,0x57,
	0x5f,0x62,0x67,0x68,0x67,0x3e,0x4d,0x71,
	0x79,0x70,0x64,0x78,0x5c,0x65,0x67,0x63,
	0xff,0xdb,0x00,0x43,0x01,
	0x11,0x12,0x12,0x18,0x15,0x18,0x2f,0x1a,
	0x1a,0x2f,0x63,0x42,0x38,0x42,0x63,0x63,
	0x63,0x63,0x63,0x63,0x63,0x63,0x63,0x63,
	0x63,0x63,0x63,0x63,0x63,0x63,0x63,0x63,
	0x63,0x63,0x63,0x63,0x63,0x63,0x63,0x63,
	0x63,0x63,0x63,0x63,0x63,0x63,0x63,0x63,
	0x63,0x63,0x63,0x63,0x63,0x63,0x63,0x63,
	0x63,0x63,0x63,0x63,0x63,0x63,0x63,0x63,

	0xff,0xdb,0x00,0x43,0x00,	// high quality
	0x10 /2,0x0b /2,0x0c /2,0x0e /2,0x0c /2,0x0a /2,0x10 /2,0x0e /2,
	0x0d /2,0x0e /2,0x12 /2,0x11 /2,0x10 /2,0x13 /2,0x18 /2,0x28 /2,
	0x1a /2,0x18 /2,0x16 /2,0x16 /2,0x18 /2,0x31 /2,0x23 /2,0x25 /2,
	0x1d /2,0x28 /2,0x3a /2,0x33 /2,0x3d /2,0x3c /2,0x39 /2,0x33 /2,
	0x38 /2,0x37 /2,0x40 /2,0x48 /2,0x5c /2,0x4e /2,0x40 /2,0x44 /2,
	0x57 /2,0x45 /2,0x37 /2,0x38 /2,0x50 /2,0x6d /2,0x51 /2,0x57 /2,
	0x5f /2,0x62 /2,0x67 /2,0x68 /2,0x67 /2,0x3e /2,0x4d /2,0x71 /2,
	0x79 /2,0x70 /2,0x64 /2,0x78 /2,0x5c /2,0x65 /2,0x67 /2,0x63 /2,
	0xff,0xdb,0x00,0x43,0x01,
	0x11 /2,0x12 /2,0x12 /2,0x18 /2,0x15 /2,0x18 /2,0x2f /2,0x1a /2,
	0x1a /2,0x2f /2,0x63 /2,0x42 /2,0x38 /2,0x42 /2,0x63 /2,0x63 /2,
	0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,
	0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,
	0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,
	0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,
	0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,
	0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,0x63 /2,
};

#define SetDQT(pHdrJPEG, idxQP)		memcpy ((BYTE *)(pHdrJPEG) +DQT_OFFSET, JPEG_DQT[idxQP], DQT_LEN)

#define DHT_OFFSET			(DQT_OFFSET +DQT_LEN)
#define DHT_LEN				420
const BYTE JPEG_DHT[DHT_LEN] = {
	0xff,0xc4,0x01,0xa2,0x00,0x00,0x01,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,
	0x0b,0x10,0x00,0x02,0x01,0x03,0x03,0x02,0x04,0x03,0x05,0x05,0x04,0x04,0x00,0x00,
	0x01,0x7d,0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,
	0x61,0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xa1,0x08,0x23,0x42,0xb1,0xc1,0x15,0x52,
	0xd1,0xf0,0x24,0x33,0x62,0x72,0x82,0x09,0x0a,0x16,0x17,0x18,0x19,0x1a,0x25,0x26,
	0x27,0x28,0x29,0x2a,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,0x47,
	0x48,0x49,0x4a,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x63,0x64,0x65,0x66,0x67,
	0x68,0x69,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x83,0x84,0x85,0x86,0x87,
	0x88,0x89,0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,
	0xa6,0xa7,0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xc2,0xc3,
	0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,
	0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,
	0xf7,0xf8,0xf9,0xfa,0x01,0x00,0x03,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,
	0x0b,0x11,0x00,0x02,0x01,0x02,0x04,0x04,0x03,0x04,0x07,0x05,0x04,0x04,0x00,0x01,
	0x02,0x77,0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,
	0x61,0x71,0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,0xa1,0xb1,0xc1,0x09,0x23,0x33,
	0x52,0xf0,0x15,0x62,0x72,0xd1,0x0a,0x16,0x24,0x34,0xe1,0x25,0xf1,0x17,0x18,0x19,
	0x1a,0x26,0x27,0x28,0x29,0x2a,0x35,0x36,0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,
	0x47,0x48,0x49,0x4a,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x63,0x64,0x65,0x66,
	0x67,0x68,0x69,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x82,0x83,0x84,0x85,
	0x86,0x87,0x88,0x89,0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0xa2,0xa3,
	0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,
	0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,
	0xd9,0xda,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf2,0xf3,0xf4,0xf5,0xf6,
	0xf7,0xf8,0xf9,0xfa
};

#define SetDHT(pHdrJPEG)	memcpy ((BYTE *)(pHdrJPEG) +DHT_OFFSET, JPEG_DHT, DHT_LEN)

#define SOF_OFFSET			(DHT_OFFSET +DHT_LEN)
#define SOF_LEN				19
const BYTE JPEG_SOF[SOF_LEN] = {
	0xff,0xc0,0x00,0x11,0x08,0x00,0xf0,0x02,0xc0,0x03,0x01,0x22,0x00,0x02,0x11,0x01,
	0x03,0x11,0x01
};

static inline void SetSOF (BYTE *pHdrJPEG, DWORD szH, DWORD szV)
{
	memcpy (pHdrJPEG +SOF_OFFSET, JPEG_SOF, SOF_LEN);
	pHdrJPEG[SOF_OFFSET +5] = BYTE(0xff &(szV >>8));
	pHdrJPEG[SOF_OFFSET +6] = BYTE(0xff &(szV >>0));
	pHdrJPEG[SOF_OFFSET +7] = BYTE(0xff &(szH >>8));
	pHdrJPEG[SOF_OFFSET +8] = BYTE(0xff &(szH >>0));
}

#define SOS_OFFSET			(SOF_OFFSET +SOF_LEN)
#define SOS_LEN				14
const BYTE JPEG_SOS[SOS_LEN] = {
	0xff,0xda,0x00,0x0c,0x03,0x01,0x00,0x02,0x11,0x03,0x11,0x00,0x3f,0x00
};
#define SetSOS(pHdrJPEG)	memcpy ((BYTE *)(pHdrJPEG) +SOS_OFFSET, JPEG_SOS, SOS_LEN)

#define EOI_LEN				2
const unsigned char JPEG_EOI[EOI_LEN] = {
	0xff, 0xd9
};
#define SetEOI(pBufFrm)		memcpy ((unsigned char *)pBufFrm, JPEG_EOI, EOI_LEN)

#define SZ_JPEG_HEADER		(SOS_OFFSET +SOS_LEN)
#define SZ_JPEG_PADDING		(SZ_JPEG_HEADER +EOI_LEN)

static BYTE glBufHeaderJPEG[NUM_JPEG_IMG_QUALITY][SZ_JPEG_HEADER];



void CPlayerManager::InitDec_JPEG ()
{
	ijlInit (&m_stJpegCoreProp);

	int i;
	for (i=0; i<NUM_JPEG_IMG_QUALITY; i++)
	{
		SetSOI (glBufHeaderJPEG[i]);
		SetDHT (glBufHeaderJPEG[i]);
		SetSOS (glBufHeaderJPEG[i]);
		SetDQT (glBufHeaderJPEG[i], i);
	}
}

void CPlayerManager::DestroyDec_JPEG ()
{
	ijlFree (&m_stJpegCoreProp);
}

BOOL CPlayerManager::OpenFile_JPEG ()
{
	DWORD i, numRead, idxCh;
	LARGE_INTEGER liPosFile;

	m_bufIdxStreamFormat[PM_FILE_VIDEO] = PM_STREAM_FORMAT_JPEG;

	m_bufNumTotFrm[PM_FILE_VIDEO] = m_bufFileHeader[PM_FILE_VIDEO].JPEG.numFrm;
	m_posVidFileIndex = m_bufFileHeader[PM_FILE_VIDEO].JPEG.posIndex;

	FindAndSetOptimalCS ();

	DecodeVideo = Decode_JPEG;

	// Make index[S]
	if (m_bufVideoIndex != NULL)
	{
		delete[] m_bufVideoIndex;
	}
	ITEM_VID_REC_FILE_INDEX *bufTmpFileIndex = new ITEM_VID_REC_FILE_INDEX[m_bufNumTotFrm[PM_FILE_VIDEO]];
	m_bufVideoIndex = new ITEM_PM_VIDEO_INDEX[m_bufNumTotFrm[PM_FILE_VIDEO] +1];
	if (bufTmpFileIndex == NULL || m_bufVideoIndex == NULL)
	{
		AddReport ("PLAYER: OpenFile_JPEG (): Index buffer allocation fail");
		return FALSE;
	}

	liPosFile.QuadPart = m_posVidFileIndex;
	SetFilePointer (m_bufHFile[PM_FILE_VIDEO], liPosFile.LowPart, &liPosFile.HighPart, FILE_BEGIN);
	ReadFile (m_bufHFile[PM_FILE_VIDEO], bufTmpFileIndex,
		sizeof(ITEM_VID_REC_FILE_INDEX) *m_bufNumTotFrm[PM_FILE_VIDEO], &numRead, NULL);

	memset (m_bufNumVidFrm, 0, sizeof(DWORD) *NUM_VID_TOTAL_ENC_CH);

	for (i=0; i<m_bufNumTotFrm[PM_FILE_VIDEO]; i++)
	{
		idxCh = bufTmpFileIndex[i].idxCh;

		m_bufVideoIndex[i].idxCh = (BYTE)idxCh;
		m_bufVideoIndex[i].isKeyFrm = bufTmpFileIndex[i].isKeyFrm;
		m_bufVideoIndex[i].timeMSec = bufTmpFileIndex[i].timeSec *1000 +bufTmpFileIndex[i].timeUSec /1000;
		m_bufVideoIndex[i].posFrm = bufTmpFileIndex[i].posFrm +sizeof(HEADER_JPEG_FRM);

		if (m_bufVideoIndex[i].posFrm -m_bufVideoIndex[i -1].posFrm > 2 *1024 *1024)
		{
			AddReport ("ERROR: OpenFile_JPEG(): invalid size: [%d, Old Pos:%d, Cur Pos:%d (Diff:%d)]",
				i, m_bufVideoIndex[i -1].posFrm, m_bufVideoIndex[i].posFrm, m_bufVideoIndex[i].posFrm -m_bufVideoIndex[i -1].posFrm);

			m_bufNumTotFrm[PM_FILE_VIDEO] = i;
			break;
		}

		m_bufVideoIndex[i].idxFrmInCh = m_bufNumVidFrm[idxCh];
		m_bufNumVidFrm[idxCh]++;

		m_bufVideoIndex[i].reserved1 = 0;
		m_bufVideoIndex[i].reserved2 = 0;
	}
	m_bufVideoIndex[i].posFrm = m_posVidFileIndex;	// for size of frame checking

	memset (m_bufNumVidFrm, 0, sizeof(DWORD) *NUM_VID_TOTAL_ENC_CH);
	for (i=0; i<m_bufNumTotFrm[PM_FILE_VIDEO]; i++)
	{
		m_bufNumVidFrm[m_bufVideoIndex[i].idxCh]++;
	}

	delete[] bufTmpFileIndex;
	// Make index[E]

	return TRUE;
}

void CPlayerManager::CloseFile_JPEG ()
{
}

BOOL Decode_JPEG (CPlayerManager *pPlayer)
{
	int i, idxCh;
	BYTE *bufDecImg;
	DWORD szImgH, szImgV, typeVideo, idxImgSz, nPitch, idxFrm, szFrm;
	CWndPlayer *pWndPlayer;
	ITEM_PM_VIDEO_INDEX *pIndex;

	pWndPlayer = pPlayer->m_pWndPlayer;
	typeVideo = pPlayer->m_bufFileHeader[PM_FILE_VIDEO].JPEG.typeVideo;
	for (i=0; i<GL_NUM_DISP_FROM_DISP_MODE[pPlayer->m_idxSplitMode]; i++)
	{
		idxFrm = pPlayer->iBuffStat[PM_FILE_VIDEO].idxFrmDecRead;

		// end of stream check[S]
		if (idxFrm >= pPlayer->m_bufNumTotFrm[PM_FILE_VIDEO])
		{
			SetEvent (pPlayer->m_bufHEvt[PM_EVT_REQ_PAUSE]);
			break;
		}
		// end of stream check[E]

		pIndex = &pPlayer->m_bufVideoIndex[idxFrm];
		idxCh = pIndex->idxCh;

		if (pPlayer->m_bufBDec[idxCh] == FALSE)
		{
			i--;
			goto SKIP_DECODING;
		}

		// buffering buffer empty check[S]
		if (idxFrm == pPlayer->iBuffStat[PM_FILE_VIDEO].idxFrmFileWrite)
		{
			break;
		}
		// buffering buffer empty check[E]

		// time check[S]
		if (pIndex->timeMSec -pPlayer->m_timeRef > (pPlayer->m_timeElapsed_x16 >>4))
		{
			break;
		}
		// time check[E]

		szFrm = DWORD((pIndex +1)->posFrm -pIndex->posFrm);

		// buffering buffer position overlap update[S]
		if (pPlayer->iBuffStat[PM_FILE_VIDEO].posDecRead +szFrm +SZ_JPEG_PADDING
			> pPlayer->iBuffStat[PM_FILE_VIDEO].szBufBuffering)
		{
			pPlayer->iBuffStat[PM_FILE_VIDEO].posDecRead = 0;
		}

		// decoding[S]
		idxImgSz = pPlayer->m_bufFileHeader[PM_FILE_VIDEO].JPEG.bufIdxImgSz[idxCh];
		szImgH = IMG_SZ_H_FROM_INDEX_IMG_SZ[typeVideo][idxImgSz];
		szImgV = IMG_SZ_V_FROM_INDEX_IMG_SZ[typeVideo][idxImgSz];

		bufDecImg = pWndPlayer->LockDecImgBuf (nPitch);

		if (bufDecImg != NULL)
		{
			if (pPlayer->Dec1Frame_JPEG (
				&pPlayer->iBuffStat[PM_FILE_VIDEO].bufBuffering[pPlayer->iBuffStat[PM_FILE_VIDEO].posDecRead],
				szFrm +SZ_JPEG_PADDING,
				bufDecImg, nPitch) == TRUE)	// decoding
			{
				pWndPlayer->UnlockAndBltDecImgToBackSurface (
					pPlayer->m_bufWndFromCh[idxCh], szImgH, szImgV,
					idxFrm, &pPlayer->m_iImgPostFilter);	// copy decoded image to window
			}
			else
			{
				pWndPlayer->UnlockDecImgBuf ();
			}
		}
		else
		{
			AddReport ("ERROR: Decode_JPEG() pWndPlayer->LockDecImgBuf () return NULL");
		}

		pPlayer->iBuffStat[PM_FILE_VIDEO].posDecRead += szFrm +SZ_JPEG_PADDING;
		// decoding[E]

SKIP_DECODING:
		pPlayer->m_bufPosCurFrm[PM_FILE_VIDEO] = pPlayer->iBuffStat[PM_FILE_VIDEO].idxFrmDecRead;
		pPlayer->iBuffStat[PM_FILE_VIDEO].idxFrmDecRead++;
	}

	if (i != 0)
	{
		return TRUE;
	}

	return FALSE;
}

void CPlayerManager::Buffering_JPEG ()
{
	DWORD idxCh, idxFrm, numRead, szImgH, szImgV, idxQuality, typeVideo, idxImgSz, szFrm;
	BYTE *pBufBuffering;
	LARGE_INTEGER liPosFile;

	typeVideo = m_bufFileHeader[PM_FILE_VIDEO].JPEG.typeVideo;
	pBufBuffering = &iBuffStat[PM_FILE_VIDEO].bufBuffering[iBuffStat[PM_FILE_VIDEO].posFileWrite];

	while (1)
	{
		idxFrm = iBuffStat[PM_FILE_VIDEO].idxFrmFileWrite;

		if (idxFrm >= m_bufNumTotFrm[PM_FILE_VIDEO])
		{	// end of stream
			break;
		}

		idxCh = m_bufVideoIndex[idxFrm].idxCh;
		szFrm = DWORD(m_bufVideoIndex[idxFrm +1].posFrm -m_bufVideoIndex[idxFrm].posFrm);

		if (m_bufBDec[idxCh] == TRUE)
		{	// need buffering
			if (iBuffStat[PM_FILE_VIDEO].posFileWrite +szFrm +SZ_JPEG_PADDING
				> iBuffStat[PM_FILE_VIDEO].szBufBuffering)	// buffering buffer overflow check
			{
				if (iBuffStat[PM_FILE_VIDEO].posFileWrite < iBuffStat[PM_FILE_VIDEO].posDecRead ||
					(iBuffStat[PM_FILE_VIDEO].posFileWrite +szFrm +SZ_JPEG_PADDING) %iBuffStat[PM_FILE_VIDEO].szBufBuffering
					>= iBuffStat[PM_FILE_VIDEO].posDecRead)
				{
					break;
				}
				else
				{
					iBuffStat[PM_FILE_VIDEO].posFileWrite = 0;
				}
			}

			if (iBuffStat[PM_FILE_VIDEO].posFileWrite < iBuffStat[PM_FILE_VIDEO].posDecRead &&
				iBuffStat[PM_FILE_VIDEO].posFileWrite +szFrm +SZ_JPEG_PADDING >= iBuffStat[PM_FILE_VIDEO].posDecRead)	// buffering buffer fullness check
			{
				break;
			}

			// copy JPEG header to buffering buffer[S]
			idxImgSz = m_bufFileHeader[PM_FILE_VIDEO].JPEG.bufIdxImgSz[idxCh];
//			idxQuality = m_bufFileHeader[PM_FILE_VIDEO].JPEG.bufSetRec[idxCh].idxQuality;
			idxQuality = JPEG_IMG_QUALITY_LOW;	// JPEG quality is always Low. (2007-08-14)
			szImgH = IMG_SZ_H_FROM_INDEX_IMG_SZ[typeVideo][idxImgSz];
			szImgV = IMG_SZ_V_FROM_INDEX_IMG_SZ[typeVideo][idxImgSz];

			SetSOF (glBufHeaderJPEG[idxQuality], szImgH, szImgV);

			memcpy (&iBuffStat[PM_FILE_VIDEO].bufBuffering[iBuffStat[PM_FILE_VIDEO].posFileWrite],
				glBufHeaderJPEG[idxQuality], SZ_JPEG_HEADER);
			iBuffStat[PM_FILE_VIDEO].posFileWrite += SZ_JPEG_HEADER;
			// copy JPEG header to buffering buffer[E]

			// copy frame data to buffering buffer[S]
			liPosFile.QuadPart = m_bufVideoIndex[idxFrm].posFrm;
			SetFilePointer (m_bufHFile[PM_FILE_VIDEO], liPosFile.LowPart, &liPosFile.HighPart, FILE_BEGIN);
			ReadFile (m_bufHFile[PM_FILE_VIDEO],
				&iBuffStat[PM_FILE_VIDEO].bufBuffering[iBuffStat[PM_FILE_VIDEO].posFileWrite],
				szFrm, &numRead, NULL);
			iBuffStat[PM_FILE_VIDEO].posFileWrite += szFrm;
			// copy frame data to buffering buffer[E]

			SetEOI(&iBuffStat[PM_FILE_VIDEO].bufBuffering[iBuffStat[PM_FILE_VIDEO].posFileWrite]);
			iBuffStat[PM_FILE_VIDEO].posFileWrite += EOI_LEN;
		}

		iBuffStat[PM_FILE_VIDEO].idxFrmFileWrite++;
	}
}

BOOL CPlayerManager::Dec1Frame_JPEG (BYTE *bufStream, DWORD szStream, BYTE *bufDecImg, DWORD nPitch)
{
#ifdef __JPEG_OPT_FOR_DECODING__
	m_stJpegCoreProp.JPGFile = NULL;
	m_stJpegCoreProp.JPGBytes = bufStream;
	m_stJpegCoreProp.JPGSizeBytes = szStream;
	IJLERR nErr;
	nErr = ijlRead (&m_stJpegCoreProp, IJL_JBUFF_READPARAMS);

	if (nErr != IJL_OK)
	{
		AddReport ("ERROR: JPEG decdoing (IJL_JBUFF_READPARAMS)");
		return FALSE;
	}

	m_stJpegCoreProp.JPGColor = IJL_YCBCR;

	m_stJpegCoreProp.DIBColor = IJL_RGBA_FPX;
	m_stJpegCoreProp.DIBChannels = 4;

	m_stJpegCoreProp.DIBWidth = m_stJpegCoreProp.JPGWidth;
	m_stJpegCoreProp.DIBHeight = m_stJpegCoreProp.JPGHeight;

	m_stJpegCoreProp.DIBPadBytes = 
		nPitch -(m_stJpegCoreProp.DIBWidth *m_stJpegCoreProp.DIBChannels);
	m_stJpegCoreProp.DIBBytes = bufDecImg;

	nErr = ijlRead (&m_stJpegCoreProp, IJL_JBUFF_READWHOLEIMAGE);

	if (nErr != IJL_OK)
	{
		AddReport ("ERROR: JPEG decdoing (IJL_JBUFF_READWHOLEIMAGE) [%d]", nErr);
		return FALSE;
	}
#else
	m_stJpegCoreProp.JPGFile = NULL;
	m_stJpegCoreProp.JPGBytes = bufStream;
	m_stJpegCoreProp.JPGSizeBytes = szStream;
	IJLERR nErr;
	nErr = ijlRead (&m_stJpegCoreProp, IJL_JBUFF_READPARAMS);

	if (nErr != IJL_OK)
	{
		AddReport ("ERROR: JPEG decdoing (IJL_JBUFF_READPARAMS)");
		return FALSE;
	}

	BYTE *bufTmp = new BYTE[m_stJpegCoreProp.JPGWidth *m_stJpegCoreProp.JPGHeight *4];

	m_stJpegCoreProp.JPGColor = IJL_YCBCR;

	m_stJpegCoreProp.DIBColor = IJL_RGBA_FPX;
	m_stJpegCoreProp.DIBChannels = 4;

	m_stJpegCoreProp.DIBWidth = m_stJpegCoreProp.JPGWidth;
	m_stJpegCoreProp.DIBHeight = m_stJpegCoreProp.JPGHeight;
/*
	m_stJpegCoreProp.DIBPadBytes = 
		nPitch -(m_stJpegCoreProp.DIBWidth *m_stJpegCoreProp.DIBChannels);
	m_stJpegCoreProp.DIBBytes = bufDecImg;
*/

	m_stJpegCoreProp.DIBPadBytes = 0;
	m_stJpegCoreProp.DIBBytes = bufTmp;

	nErr = ijlRead (&m_stJpegCoreProp, IJL_JBUFF_READWHOLEIMAGE);

	if (nErr != IJL_OK)
	{
		AddReport ("ERROR: JPEG decdoing (IJL_JBUFF_READWHOLEIMAGE) [%d]", nErr);

		delete[] bufTmp;
		
		return FALSE;
	}

	int i, j;

	for (i=0; i<m_stJpegCoreProp.JPGHeight; i++)
	{
		for (j=0; j<m_stJpegCoreProp.JPGWidth; j++)
		{
			bufDecImg[i *nPitch +4 *j +0] = bufTmp[i *m_stJpegCoreProp.JPGWidth *4 +4 *j +2];
			bufDecImg[i *nPitch +4 *j +1] = bufTmp[i *m_stJpegCoreProp.JPGWidth *4 +4 *j +1];
			bufDecImg[i *nPitch +4 *j +2] = bufTmp[i *m_stJpegCoreProp.JPGWidth *4 +4 *j +0];
		}
	}

/*	int i, nAddSrc;
	BYTE *pSrc, *pDest;

	pSrc = bufTmp;
	pDest = bufDecImg;
	nAddSrc = m_stJpegCoreProp.JPGWidth *4;

	for (i=0; i<m_stJpegCoreProp.JPGHeight /8; i++)
	{
		FastMemcpy (pDest, pSrc, nAddSrc); pSrc += nAddSrc; pDest += nPitch;
		FastMemcpy (pDest, pSrc, nAddSrc); pSrc += nAddSrc; pDest += nPitch;
		FastMemcpy (pDest, pSrc, nAddSrc); pSrc += nAddSrc; pDest += nPitch;
		FastMemcpy (pDest, pSrc, nAddSrc); pSrc += nAddSrc; pDest += nPitch;
		FastMemcpy (pDest, pSrc, nAddSrc); pSrc += nAddSrc; pDest += nPitch;
		FastMemcpy (pDest, pSrc, nAddSrc); pSrc += nAddSrc; pDest += nPitch;
		FastMemcpy (pDest, pSrc, nAddSrc); pSrc += nAddSrc; pDest += nPitch;
		FastMemcpy (pDest, pSrc, nAddSrc); pSrc += nAddSrc; pDest += nPitch;
	}
*/
	delete[] bufTmp;
#endif

	return TRUE;
}

void CPlayerManager::SetFramePos_JPEG (DWORD posFrm)
{
	int i, posDecStart, posDecEnd;

	posDecStart = 0;
	posDecEnd = 0;

	// get posDecEnd[S]
	if (posFrm >= m_bufNumTotFrm[PM_FILE_VIDEO])
	{
		posFrm = m_bufNumTotFrm[PM_FILE_VIDEO] -1;
	}

	for (i=posFrm; i>=0; i--)
	{
		if (m_bufBDec[m_bufVideoIndex[i].idxCh] == TRUE)
		{
			posDecEnd = i;
			break;
		}
	}
	// get posDecEnd[E]

	BOOL bufTmpBDec[NUM_VID_TOTAL_ENC_CH];
	int numDispFrm, idxBufCurDispFrm;

	numDispFrm = 0;
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		if (m_bufBDec[i] == TRUE)
		{
			numDispFrm++;
		}
	}

	// get posDecStart[S]
	idxBufCurDispFrm = 0;
	memset (bufTmpBDec, 0, sizeof(BOOL) *NUM_VID_TOTAL_ENC_CH);
	for (i=posDecEnd; i>=0; i--)
	{
		if (bufTmpBDec[m_bufVideoIndex[i].idxCh] == FALSE &&
			m_bufBDec[m_bufVideoIndex[i].idxCh] == TRUE)
		{
			bufTmpBDec[m_bufVideoIndex[i].idxCh] = TRUE;
			idxBufCurDispFrm++;

			if (idxBufCurDispFrm >= numDispFrm)
			{
				posDecStart = i;
				break;
			}
		}
	}
	// get posDecStart[E]

	if (i == -1)	// incorrect posDecEnd
	{
		posDecStart = 0;
		posDecEnd = m_bufNumTotFrm[PM_FILE_VIDEO] -1;

		// re-get posDecEnd[S]
		idxBufCurDispFrm = 0;
		memset (bufTmpBDec, 0, sizeof(BOOL) *NUM_VID_TOTAL_ENC_CH);
		for (i=0; i<int(m_bufNumTotFrm[PM_FILE_VIDEO]); i++)
		{
			if (bufTmpBDec[m_bufVideoIndex[i].idxCh] == FALSE &&
				m_bufBDec[m_bufVideoIndex[i].idxCh] == TRUE)
			{
				bufTmpBDec[m_bufVideoIndex[i].idxCh] = TRUE;
				idxBufCurDispFrm++;

				if (idxBufCurDispFrm >= numDispFrm)
				{
					posDecEnd = i;
					break;
				}
			}
		}
		// re-get posDecEnd[E]
	}

	DWORD bufIdxDispFrm[PM_MAX_NUM_DISP_WND], idxCh;

	// get last display frame index[S]
	idxBufCurDispFrm = 0;
	memset (bufTmpBDec, 0, sizeof(DWORD) *NUM_VID_TOTAL_ENC_CH);
	if (posDecEnd > int(posFrm))
	{
		for (i=posFrm; i>=posDecStart; i--)
		{
			idxCh = m_bufVideoIndex[i].idxCh;
			if (bufTmpBDec[idxCh] == FALSE && m_bufBDec[idxCh] == TRUE)
			{
				bufIdxDispFrm[numDispFrm -1 -idxBufCurDispFrm] = i;

				bufTmpBDec[idxCh] = TRUE;
				idxBufCurDispFrm++;
			}
		}
		for (i=0; i<int(idxBufCurDispFrm); i++)
		{
			bufIdxDispFrm[i] = bufIdxDispFrm[numDispFrm -idxBufCurDispFrm +i];
		}
		for (i=posFrm +1; i<=posDecEnd; i++)
		{
			idxCh = m_bufVideoIndex[i].idxCh;
			if (bufTmpBDec[idxCh] == FALSE && m_bufBDec[idxCh] == TRUE)
			{
				bufIdxDispFrm[idxBufCurDispFrm] = i;

				bufTmpBDec[idxCh] = TRUE;
				idxBufCurDispFrm++;

				if (idxBufCurDispFrm >= numDispFrm)
				{
					break;
				}
			}
		}
	}
	else
	{
		for (i=posDecEnd; i>=posDecStart; i--)
		{
			idxCh = m_bufVideoIndex[i].idxCh;
			if (bufTmpBDec[idxCh] == FALSE && m_bufBDec[idxCh] == TRUE)
			{
				bufIdxDispFrm[numDispFrm -1 -idxBufCurDispFrm] = i;

				bufTmpBDec[idxCh] = TRUE;
				idxBufCurDispFrm++;

				if (idxBufCurDispFrm >= numDispFrm)
				{
					break;
				}
			}
		}
	}
	if (i < posDecStart)
	{
		goto SKIP_SET_POS;
	}
	// get last display frame index[E]

	DWORD numRead, szImgH, szImgV, idxQuality, typeVideo, idxImgSz, nPitch, idxFrm, szFrm;
	BYTE *bufDecImg;
	LARGE_INTEGER liPosFile;

	// get frame data & decoding & copy decoded image to window[S]
	typeVideo = m_bufFileHeader[PM_FILE_VIDEO].JPEG.typeVideo;
	for (i=0; i<int(numDispFrm); i++)
	{
		idxFrm = bufIdxDispFrm[i];

		if (m_bufBDec[m_bufVideoIndex[idxFrm].idxCh] == TRUE)
		{
			// copy JPEG header to buffering buffer[S]
			idxImgSz = m_bufFileHeader[PM_FILE_VIDEO].JPEG.bufIdxImgSz[m_bufVideoIndex[idxFrm].idxCh];
//			idxQuality = m_bufFileHeader[PM_FILE_VIDEO].JPEG.bufSetRec[m_bufVideoIndex[idxFrm].idxCh].idxQuality;
			idxQuality = JPEG_IMG_QUALITY_LOW;	// JPEG quality is always Low. (2007-08-14)
			szImgH = IMG_SZ_H_FROM_INDEX_IMG_SZ[typeVideo][idxImgSz];
			szImgV = IMG_SZ_V_FROM_INDEX_IMG_SZ[typeVideo][idxImgSz];
			szFrm = DWORD(m_bufVideoIndex[idxFrm +1].posFrm -m_bufVideoIndex[idxFrm].posFrm);

			SetSOF (glBufHeaderJPEG[idxQuality], szImgH, szImgV);

			memcpy (&iBuffStat[PM_FILE_VIDEO].bufBuffering[0],
				glBufHeaderJPEG[idxQuality], SZ_JPEG_HEADER);
			// copy JPEG header to buffering buffer[E]

			// copy frame data to buffering buffer[S]
			liPosFile.QuadPart = m_bufVideoIndex[idxFrm].posFrm;
			SetFilePointer (m_bufHFile[PM_FILE_VIDEO], liPosFile.LowPart, &liPosFile.HighPart, FILE_BEGIN);
			ReadFile (m_bufHFile[PM_FILE_VIDEO],
				&iBuffStat[PM_FILE_VIDEO].bufBuffering[SZ_JPEG_HEADER],
				szFrm, &numRead, NULL);
			// copy frame data to buffering buffer[E]

			SetEOI(&iBuffStat[PM_FILE_VIDEO].bufBuffering[SZ_JPEG_HEADER +szFrm]);

			bufDecImg = m_pWndPlayer->LockDecImgBuf (nPitch);

			if (bufDecImg != NULL)
			{
				if (Dec1Frame_JPEG (&iBuffStat[PM_FILE_VIDEO].bufBuffering[0],
					szFrm +SZ_JPEG_PADDING, bufDecImg, nPitch) == TRUE)	// decoding
				{
					m_pWndPlayer->UnlockAndBltDecImgToBackSurface (
						m_bufWndFromCh[m_bufVideoIndex[idxFrm].idxCh], szImgH, szImgV,
						idxFrm, &m_iImgPostFilter);	// copy decoded image to window
				}
				else
				{
					m_pWndPlayer->UnlockDecImgBuf ();
				}
			}
			else
			{
				AddReport ("ERROR: m_pWndPlayer->LockDecImgBuf () return NULL");
			}
		}
	}
	// get frame data & decoding & copy decoded image to window[E]

SKIP_SET_POS:
	iBuffStat[PM_FILE_VIDEO].posDecRead = 0;
	iBuffStat[PM_FILE_VIDEO].posFileWrite = 0;
	iBuffStat[PM_FILE_VIDEO].idxFrmDecRead = posFrm +1;
	iBuffStat[PM_FILE_VIDEO].idxFrmFileWrite = posFrm +1;

	m_bufPosCurFrm[PM_FILE_VIDEO] = posFrm;
}
