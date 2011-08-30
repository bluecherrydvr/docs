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

BOOL Decode_MP4 (CPlayerManager *pPlayer);

#define SZ_MPEG4_VOLH		30

BYTE glBufStrh[SZ_MPEG4_VOLH] = {
	0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x20, 0x02, 0x48, 0x0d, 0xc0, 0x00, 0x40, 0x00, 0x40,
	0x00, 0x40, 0x00, 0x80, 0x00, 0x97, 0x53, 0x0a, 0x2c, 0x08, 0x3c, 0x28, 0x8c, 0x1f
};

__inline static void MakeMP4SeqHdr (int szH, int szV, int bInterlace)
{
	glBufStrh[23] = 0x0a +(szH >>12);
	glBufStrh[24] = ((szH >>4) &0xff);
	glBufStrh[25] = (((szH >>0) &0xf) <<4) +0x8 +((szV >>10) &0x7);
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

void CPlayerManager::SetMP4PostFilterState (BOOL bUseDeblkY, BOOL bUseDeblkUV, BOOL bUseDeringY, BOOL bUseDeringUV)
{
	m_idxMP4PostFilterMode = MP4DEC_POSTFLT_NOT_USE;

	if (bUseDeblkY == TRUE)
	{
		m_idxMP4PostFilterMode = MP4DEC_POSTFLT_DEBLK_LUM;
		if (bUseDeblkUV == TRUE)
		{
			m_idxMP4PostFilterMode = MP4DEC_POSTFLT_DEBLK_LUM_CHR;
			if (bUseDeringY == TRUE)
			{
				m_idxMP4PostFilterMode = MP4DEC_POSTFLT_DEBLK_LUM_CHR_DERING_LUM;
				if (bUseDeringUV == TRUE)
				{
					m_idxMP4PostFilterMode = MP4DEC_POSTFLT_DEBLK_LUM_CHR_DERING_LUM_CHR;
				}
			}
		}
		else
		{
			if (bUseDeringY == TRUE)
			{
				m_idxMP4PostFilterMode = MP4DEC_POSTFLT_DEBLK_LUM_DERING_LUM;
			}
		}
	}
}

void CPlayerManager::SetImgPostFilterState (BOOL bUseDeint, BOOL bUseUpSc, BOOL bUseDnSc)
{
	m_iImgPostFilter.bUseDeint = bUseDeint;
	m_iImgPostFilter.bUseUpSc = bUseUpSc;
	m_iImgPostFilter.bUseDnSc = bUseDnSc;

	if (bUseDeint +bUseUpSc +bUseDnSc > 0)
	{
		m_iImgPostFilter.bUsePostFilter = TRUE;
	}
	else
	{
		m_iImgPostFilter.bUsePostFilter = FALSE;
	}
}

void CPlayerManager::InitDec_MP4 ()
{
	xvid_gbl_init_t xvid_gbl_init;
	xvid_dec_create_t xvid_dec_create;

	xvid_gbl_init.version = XVID_VERSION;
	xvid_gbl_init.cpu_flags = 0;
	xvid_global (NULL, 0, &xvid_gbl_init, NULL);

	xvid_dec_create.version = XVID_VERSION;

	xvid_dec_create.width = 0;
	xvid_dec_create.height = 0;

	int i;
	
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		if (xvid_decore (NULL, XVID_DEC_CREATE, &xvid_dec_create, NULL) < 0)
		{
			AddReport ("ERROR: InitDec_MP4(): XVID creation fail (%d)", i);
			
			xvid_dec_create.handle = NULL;
		}

		m_hXvidDec[i] = xvid_dec_create.handle;
	}

	m_stXvidDecFrame.version = XVID_VERSION;
	m_stXvidDecStat.version = XVID_VERSION;
	m_stXvidDecFrame.general = XVID_LOWDELAY;
	m_stXvidDecFrame.output.csp = XVID_CSP_I420;
	m_stXvidDecFrame.brightness = 0;

	m_idxMP4PostFilterMode = MP4DEC_POSTFLT_NOT_USE;

	m_iImgPostFilter.bUsePostFilter = FALSE;
	m_iImgPostFilter.bUseDeint = FALSE;
	m_iImgPostFilter.bUseUpSc = FALSE;
	m_iImgPostFilter.bUseDnSc = FALSE;
}

void CPlayerManager::DestroyDec_MP4 ()
{
	int i;
	
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		if (m_hXvidDec[i] != NULL)
		{
			xvid_decore (m_hXvidDec[i], XVID_DEC_DESTROY, NULL, NULL);
			
			m_hXvidDec[i] = NULL;
		}
	}
}

BOOL CPlayerManager::OpenFile_MP4 ()
{
	DWORD i, numRead, idxCh;
	LARGE_INTEGER liPosFile;

	m_bufIdxStreamFormat[PM_FILE_VIDEO] = PM_STREAM_FORMAT_MP4;

	m_bufNumTotFrm[PM_FILE_VIDEO] = m_bufFileHeader[PM_FILE_VIDEO].MP4.numFrm;
	if (m_bufNumTotFrm[PM_FILE_VIDEO] == 0)
	{
		AddReport ("PLAYER: OpenFile_MP4(): Number of MP4 frame is 0.");
		return FALSE;
	}

	m_posVidFileIndex = m_bufFileHeader[PM_FILE_VIDEO].MP4.posIndex;

	FindAndSetOptimalCS ();

	DecodeVideo = Decode_MP4;

	// Make index[S]
	if (m_bufVideoIndex != NULL)
	{
		delete[] m_bufVideoIndex;
	}
	ITEM_VID_REC_FILE_INDEX *bufTmpFileIndex = new ITEM_VID_REC_FILE_INDEX[m_bufNumTotFrm[PM_FILE_VIDEO]];
	m_bufVideoIndex = new ITEM_PM_VIDEO_INDEX[m_bufNumTotFrm[PM_FILE_VIDEO] +1];
	if (bufTmpFileIndex == NULL || m_bufVideoIndex == NULL)
	{
		AddReport ("PLAYER: OpenFile_MP4(): Index buffer allocation fail");
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
		
		if (m_bufFileHeader[PM_FILE_VIDEO].MP4.bExistMotionData == 1)
		{
			m_bufVideoIndex[i].posFrm = bufTmpFileIndex[i].posFrm +sizeof(HEADER_MP4_FRM) +256;
		}
		else
		{
			m_bufVideoIndex[i].posFrm = bufTmpFileIndex[i].posFrm +sizeof(HEADER_MP4_FRM);
		}

		m_bufVideoIndex[i].idxFrmInCh = m_bufNumVidFrm[idxCh];
		m_bufNumVidFrm[idxCh]++;

		m_bufVideoIndex[i].reserved1 = 0;
		m_bufVideoIndex[i].reserved2 = 0;
	}
	m_bufVideoIndex[i].posFrm = m_posVidFileIndex;	// for size of frame checking

	delete[] bufTmpFileIndex;
	// Make index[E]

	return TRUE;
}

void CPlayerManager::CloseFile_MP4 ()
{
}

BOOL Decode_MP4 (CPlayerManager *pPlayer)
{
	int i, idxCh;
	BYTE *bufDecImg;
	DWORD szImgH, szImgV, typeVideo, idxImgSz, nPitch, idxFrm, szFrm;
	CWndPlayer *pWndPlayer;
	ITEM_PM_VIDEO_INDEX *pIndex;

	pWndPlayer = pPlayer->m_pWndPlayer;
	typeVideo = pPlayer->m_bufFileHeader[PM_FILE_VIDEO].MP4.typeVideo;
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
		if (pPlayer->iBuffStat[PM_FILE_VIDEO].posDecRead +szFrm > pPlayer->iBuffStat[PM_FILE_VIDEO].szBufBuffering)
		{
			pPlayer->iBuffStat[PM_FILE_VIDEO].posDecRead = 0;
		}

		// decoding[S]
		idxImgSz = pPlayer->m_bufFileHeader[PM_FILE_VIDEO].MP4.bufSetRec[idxCh].idxImgSz;
		szImgH = IMG_SZ_H_FROM_INDEX_IMG_SZ[typeVideo][idxImgSz];
		szImgV = IMG_SZ_V_FROM_INDEX_IMG_SZ[typeVideo][idxImgSz];

		bufDecImg = pWndPlayer->LockDecImgBuf (nPitch);

		if (bufDecImg != NULL)
		{
			if (pPlayer->Dec1Frame_MP4 (
				&pPlayer->iBuffStat[PM_FILE_VIDEO].bufBuffering[pPlayer->iBuffStat[PM_FILE_VIDEO].posDecRead],
				szFrm, idxCh, bufDecImg, nPitch, pPlayer->m_idxMP4PostFilterMode) == TRUE)	// decoding
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
			AddReport ("ERROR: Decode_MP4() pWndPlayer->LockDecImgBuf () return NULL");
		}
		pPlayer->iBuffStat[PM_FILE_VIDEO].posDecRead += szFrm;
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

void CPlayerManager::Buffering_MP4 ()
{
	DWORD idxCh, idxFrm, numRead, szFrm;
	LARGE_INTEGER liPosFile;

	idxFrm = iBuffStat[PM_FILE_VIDEO].idxFrmFileWrite;
	while (1)
	{
		if (idxFrm >= m_bufNumTotFrm[PM_FILE_VIDEO])
		{	// end of stream
			break;
		}

		idxCh = m_bufVideoIndex[idxFrm].idxCh;
		szFrm = DWORD(m_bufVideoIndex[idxFrm +1].posFrm -m_bufVideoIndex[idxFrm].posFrm);

		if (m_bufBDec[idxCh] == TRUE)
		{	// need buffering
			if (iBuffStat[PM_FILE_VIDEO].posFileWrite +szFrm > iBuffStat[PM_FILE_VIDEO].szBufBuffering)	// buffering buffer overflow check
			{
				if (iBuffStat[PM_FILE_VIDEO].posFileWrite < iBuffStat[PM_FILE_VIDEO].posDecRead ||
					(iBuffStat[PM_FILE_VIDEO].posFileWrite +szFrm) %iBuffStat[PM_FILE_VIDEO].szBufBuffering
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
				iBuffStat[PM_FILE_VIDEO].posFileWrite +szFrm >= iBuffStat[PM_FILE_VIDEO].posDecRead)	// buffering buffer fullness check
			{
				break;
			}

			// copy frame data to buffering buffer[S]
/*
			// for debug[S]
			HEADER_MP4_FRM hdrMP4Frm;
			liPosFile.QuadPart = m_bufVideoIndex[idxFrm].posFrm -64;
			SetFilePointer (m_bufHFile[PM_FILE_VIDEO], liPosFile.LowPart, &liPosFile.HighPart, FILE_BEGIN);
			ReadFile (m_bufHFile[PM_FILE_VIDEO], &hdrMP4Frm, 64, &numRead, NULL);
			AddReport ("Ch:%d, Interval:%d", GET_CH(hdrMP4Frm), (hdrMP4Frm.nStat[VE_STATUS_4] >>20) &0xff);
			// for debug[E]
*/
			liPosFile.QuadPart = m_bufVideoIndex[idxFrm].posFrm;
			SetFilePointer (m_bufHFile[PM_FILE_VIDEO], liPosFile.LowPart, &liPosFile.HighPart, FILE_BEGIN);
			ReadFile (m_bufHFile[PM_FILE_VIDEO],
				&iBuffStat[PM_FILE_VIDEO].bufBuffering[iBuffStat[PM_FILE_VIDEO].posFileWrite], szFrm, &numRead, NULL);

			iBuffStat[PM_FILE_VIDEO].posFileWrite += szFrm;
			// copy frame data to buffering buffer[E]
		}

		idxFrm++;
	}

	iBuffStat[PM_FILE_VIDEO].idxFrmFileWrite = idxFrm;
}

BOOL CPlayerManager::Dec1Frame_MP4 (BYTE *bufStream, DWORD szStream, int idxCh, BYTE *bufDecImg, DWORD nPitch, int idxModePostFilter)
{
	const int DEC_POSTFLT_FLAG_FROM_IDX[NUM_MP4DEC_POSTFLT_MODE] = 
	{
		XVID_LOWDELAY,		// MP4DEC_POSTFLT_NOT_USE
		XVID_LOWDELAY |XVID_DEBLOCKY,	// MP4DEC_POSTFLT_DEBLK_LUM
		XVID_LOWDELAY |XVID_DEBLOCKY |XVID_DERINGY,	// MP4DEC_POSTFLT_DEBLK_LUM_DERING_LUM
		XVID_LOWDELAY |XVID_DEBLOCKY |XVID_DEBLOCKUV,	// MP4DEC_POSTFLT_DEBLK_LUM_CHR
		XVID_LOWDELAY |XVID_DEBLOCKY |XVID_DEBLOCKUV |XVID_DERINGY,	// MP4DEC_POSTFLT_DEBLK_LUM_CHR_DERING_LUM
		XVID_LOWDELAY |XVID_DEBLOCKY |XVID_DEBLOCKUV |XVID_DERINGY |XVID_DERINGUV,	// MP4DEC_POSTFLT_DEBLK_LUM_CHR_DERING_LUM_CHR
	};

	BYTE *pBufMP4Stream;
	DWORD nUsedBytes;

	pBufMP4Stream = bufStream;

	m_stXvidDecFrame.bitstream = pBufMP4Stream;
	m_stXvidDecFrame.length = szStream;
	m_stXvidDecFrame.output.plane[0] = bufDecImg;
	m_stXvidDecFrame.output.stride[0] = nPitch;

	m_stXvidDecFrame.general = DEC_POSTFLT_FLAG_FROM_IDX[idxModePostFilter];

	nUsedBytes = xvid_decore (m_hXvidDec[idxCh],
		XVID_DEC_DECODE, &m_stXvidDecFrame, &m_stXvidDecStat);

/*	if(m_stXvidDecStat.type == XVID_TYPE_VOL)
	{
		AddReport ("VOLH is decoded (CH:%d, szH:%d, szV:%d)", idxCh, m_stXvidDecStat.data.vol.width, m_stXvidDecStat.data.vol.height);
	}
*/
	return TRUE;
}

void CPlayerManager::SetFramePos_MP4 (DWORD posFrm)
{
	int i, j, posDecStart, posDecEnd;

	posDecStart = 0;
	posDecEnd = 0;

	if (posFrm >= m_bufNumTotFrm[PM_FILE_VIDEO])
	{
		posFrm = m_bufNumTotFrm[PM_FILE_VIDEO] -1;
	}

	// get posDecEnd[S]
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
			m_bufBDec[m_bufVideoIndex[i].idxCh] == TRUE &&
			m_bufVideoIndex[i].isKeyFrm == 1)
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
				m_bufBDec[m_bufVideoIndex[i].idxCh] == TRUE &&
				m_bufVideoIndex[i].isKeyFrm == 1)
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

	int typeVideo, idxImgSz;

	typeVideo = m_bufFileHeader[PM_FILE_VIDEO].MP4.typeVideo;
	
	// VOLH decode[S]
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		if (m_bufBDec[i] == TRUE)
		{
			idxImgSz = m_bufFileHeader[PM_FILE_VIDEO].MP4.bufSetRec[i].idxImgSz;
			
			MakeMP4SeqHdr (IMG_SZ_H_FROM_INDEX_IMG_SZ[typeVideo][idxImgSz],
				IMG_SZ_V_FROM_INDEX_IMG_SZ[typeVideo][idxImgSz],
				IS_INTERLACE_FROM_INDEX_IMG_SZ[idxImgSz]);
			Dec1Frame_MP4 (glBufStrh, SZ_MPEG4_VOLH, i, NULL, 0, m_idxMP4PostFilterMode);
		}
	}
	// VOLH decode[E]

	int posTmpDecStart, posTmpDecEnd, posTmpLastDec;
	DWORD numRead, szImgH, szImgV, nPitch, szFrm;
	BYTE *bufDecImg;
	LARGE_INTEGER liPosFile;

	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		if (m_bufBDec[i] == FALSE)
		{
			continue;
		}

		// get decoding start & end position of each channel[S]
		if (posDecEnd > int(posFrm))
		{	// abnormal situation: posDecEnd exceed posFrm ( |posDecStart - posFrm - posDecEnd| )
			for (j=posFrm; j>=posDecStart; j--)
			{
				if (m_bufVideoIndex[j].idxCh == i && m_bufVideoIndex[j].isKeyFrm == 1)
				{
					posTmpDecStart = j;
					break;
				}
			}
			if (j < posDecStart)
			{
				for (j=posFrm +1; j<=posDecEnd; j++)
				{
					if (m_bufVideoIndex[j].idxCh == i && m_bufVideoIndex[j].isKeyFrm == 1)
					{
						posTmpDecStart = j;
						posTmpDecEnd = j;
						break;
					}
				}
			}
			else
			{
				for (j=posFrm; j>=posDecStart; j--)
				{
					if (m_bufVideoIndex[j].idxCh == i)
					{
						posTmpDecEnd = j;
						break;
					}
				}
			}
		}
		else
		{	// normal situation ( |posDecStart - posDecEnd - posFrm| )
			for (j=posDecEnd; j>=posDecStart; j--)
			{
				if (m_bufVideoIndex[j].idxCh == i)
				{
					posTmpDecEnd = j;
					break;
				}
			}
			for (; j>=posDecStart; j--)
			{
				if (m_bufVideoIndex[j].idxCh == i && m_bufVideoIndex[j].isKeyFrm == 1)
				{
					posTmpDecStart = j;
					break;
				}
			}
		}
		// get decoding start & end position of each channel[E]

		// get frame data & decoding & copy decoded image to window[S]
		bufDecImg = m_pWndPlayer->LockDecImgBuf (nPitch);
		if (bufDecImg == NULL)
		{
			AddReport ("ERROR: m_pWndPlayer->LockDecImgBuf () return NULL");
			continue;
		}

		for (j=posTmpDecStart; j<=posTmpDecEnd; j++)
		{
			if (m_bufVideoIndex[j].idxCh != i)
			{
				continue;
			}

			posTmpLastDec = j;

			idxImgSz = m_bufFileHeader[PM_FILE_VIDEO].MP4.bufSetRec[m_bufVideoIndex[j].idxCh].idxImgSz;
			szImgH = IMG_SZ_H_FROM_INDEX_IMG_SZ[typeVideo][idxImgSz];
			szImgV = IMG_SZ_V_FROM_INDEX_IMG_SZ[typeVideo][idxImgSz];
			szFrm = DWORD(m_bufVideoIndex[j +1].posFrm -m_bufVideoIndex[j].posFrm);

			// copy frame data to buffering buffer[S]
			liPosFile.QuadPart = m_bufVideoIndex[j].posFrm;
			SetFilePointer (m_bufHFile[PM_FILE_VIDEO], liPosFile.LowPart, &liPosFile.HighPart, FILE_BEGIN);
			ReadFile (m_bufHFile[PM_FILE_VIDEO], &iBuffStat[PM_FILE_VIDEO].bufBuffering[0], szFrm, &numRead, NULL);
			// copy frame data to buffering buffer[E]

			Dec1Frame_MP4 (&iBuffStat[PM_FILE_VIDEO].bufBuffering[0], szFrm, m_bufVideoIndex[j].idxCh, bufDecImg, nPitch, m_idxMP4PostFilterMode);
		}

		m_pWndPlayer->UnlockAndBltDecImgToBackSurface (
			m_bufWndFromCh[m_bufVideoIndex[posTmpLastDec].idxCh], szImgH, szImgV,
			posTmpDecEnd, &m_iImgPostFilter);	// copy decoded image to window
		// get frame data & decoding & copy decoded image to window[E]
	}

	iBuffStat[PM_FILE_VIDEO].posDecRead = 0;
	iBuffStat[PM_FILE_VIDEO].posFileWrite = 0;
	iBuffStat[PM_FILE_VIDEO].idxFrmDecRead = posFrm +1;
	iBuffStat[PM_FILE_VIDEO].idxFrmFileWrite = posFrm +1;

	m_bufPosCurFrm[PM_FILE_VIDEO] = posFrm;
}
