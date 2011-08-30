// InfoS6010StreamIdx.cpp: implementation of the CInfoS6010StreamIdx class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "S6010StreamTester.h"
#include "InfoS6010StreamIdx.h"
#include "Solo6010.h"
#include "Solo6010_ExRecFileInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CInfoS6010StreamIdx *glInfoStreamIdx;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInfoS6010StreamIdx::CInfoS6010StreamIdx()
{
	glInfoStreamIdx = this;
	
	Init ();
}

void CInfoS6010StreamIdx::Init ()
{
	int i;
	for (i=0; i<MAX_ENC_CHANNEL; i++)
	{
		m_bufFileIdx[i] = NULL;
		m_nNumTotFrame[i] = 0;
		m_nNumValidFrame[i] = 0;
	}
}

CInfoS6010StreamIdx::~CInfoS6010StreamIdx()
{
	Free ();
}

void CInfoS6010StreamIdx::Free ()
{
	int i;
	for (i=0; i<MAX_ENC_CHANNEL; i++)
	{
		if (m_bufFileIdx[i] != NULL)
		{
			delete[] m_bufFileIdx[i];
			m_bufFileIdx[i] = NULL;
		}
		
		m_nNumTotFrame[i] = 0;
		m_nNumValidFrame[i] = 0;
	}
}

#define SZ_TMP_BUF	(2 *1024 *1024)
#define SZ_IDX_BUF	(1024 *1024)
BOOL CInfoS6010StreamIdx::MakeIndex (CString strMPEG4FileName)
{
	Free ();

	CFile fp;
	if (fp.Open (strMPEG4FileName, CFile::modeRead) == FALSE)
	{
		return FALSE;
	}

	DWORD i, szRead, curStreamPos, idxFrm, idxCh, szMP4Code, posVOPH, szFrm;
	int idxMaxDec, numFrmMaxDec, numErrVOP;
	BOOL bOK, bSzFrmInc, bLastRead;
	DWORD bufTotSzFrm[MAX_ENC_CHANNEL];

	BYTE *bufTmp = new BYTE[SZ_TMP_BUF];
	ST_MPEG4_FILE_IDX *tmpBufFileIdx = new ST_MPEG4_FILE_IDX[SZ_IDX_BUF];
	HEADER_MP4_FRM hdrMP4RecFrm;

	const BYTE GL_B_MOTION[8] = {0, 0, 0, 0, 1, 1, 1, 1};
	const BYTE GL_N_ALIGN[8] = {8, 16, 32, 64, 8, 16, 32, 64};

	// S6010 Stream Init String Check[S]
	BYTE bufInitStr[SZ_STR_INIT];
	BOOL bMP4HdrExist;
	fp.Read (bufInitStr, SZ_STR_INIT);
	if (memcmp (bufInitStr, STR_MP4_FILE, SZ_STR_INIT) == 0)
	{
		HEADER_RECORD_FILE hdrMP4;
		fp.Read (&hdrMP4.dateStart, sizeof(HEADER_RECORD_FILE) -SZ_STR_INIT);

		for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
		{
			if (hdrMP4.MP4.bufCRU[i] > 0)
			{
				m_bufIdxVideo[i] = hdrMP4.MP4.typeVideo;
			}
			else
			{
				m_bufIdxVideo[i] = IDX_DEF_VIDEO;
			}
			AddReport ("CH:%d [B%d, S%d, CRU:%d]",
				i +1, hdrMP4.MP4.bufSetRec[i].bSet, hdrMP4.MP4.bufSetRec[i].idxImgSz, hdrMP4.MP4.bufCRU[i]);
		}

		if (hdrMP4.MP4.bUseIndex == 1)
		{
			fp.Close ();
			delete[] tmpBufFileIdx;
			delete[] bufTmp;
			return MakeIndexFromIndexBuf (strMPEG4FileName);
		}

		bMP4HdrExist = TRUE;
	}
	else
	{
		fp.Seek (0, SEEK_SET);

		for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
		{
			m_bufIdxVideo[i] = IDX_DEF_VIDEO;
		}
		bMP4HdrExist = FALSE;
	}
	// S6010 Stream Init String Check[E]

	idxMaxDec = 0;
	numFrmMaxDec = 0;
	bLastRead = FALSE;
	for (i=0; i<16; i++)
	{
START_READ:
		bOK = TRUE;
		idxFrm = 0;
		curStreamPos = 0;
		m_bMotionEx = GL_B_MOTION[i %8];
		m_nAlign = GL_N_ALIGN[i %8];
		bSzFrmInc = i /8;
		numErrVOP = 0;
		memset (bufTotSzFrm, 0, sizeof(DWORD) *MAX_ENC_CHANNEL);
		
		if (bMP4HdrExist == TRUE)
		{
			fp.Seek (sizeof(HEADER_RECORD_FILE), CFile::begin);
			curStreamPos = sizeof(HEADER_RECORD_FILE);
		}
		else
		{
			fp.Seek (0, CFile::begin);
		}

		while (1)
		{
			if (bSzFrmInc == 1 && numErrVOP >= 30)
			{
				bOK = FALSE;
				AddReport ("BAD Stream2 :%0x", curStreamPos);
				break;
			}
			
			if (bSzFrmInc == 1)
			{
				curStreamPos += fp.Read (&szFrm, sizeof(DWORD));
			}
			szRead = fp.Read (&hdrMP4RecFrm, sizeof(HEADER_MP4_FRM));
			curStreamPos += szRead;
			if (szRead < sizeof(HEADER_MP4_FRM))
			{
				AddReport ("File End 1");
				break;
			}

			szMP4Code = GetAlignedSzFrame (hdrMP4RecFrm.nStat[10], m_nAlign) +(256 *m_bMotionEx);
			posVOPH = 256 *m_bMotionEx;

			if (bSzFrmInc == 1)
			{
				if (szFrm > SZ_TMP_BUF)
				{
					AddReport ("Size Frm Err: %d", szFrm);
					break;
				}
				else if (szMP4Code +sizeof(HEADER_MP4_FRM) != szFrm)
				{
					AddReport ("Frame Err: %d (%d)", szMP4Code, szFrm);

					bOK = FALSE;
					numErrVOP++;
					if (szFrm -sizeof(HEADER_MP4_FRM) > 0)
					{
						fp.Read (bufTmp, szFrm -sizeof(HEADER_MP4_FRM));
					}
					continue;
				}
			}

			memset (bufTmp, 0, 256 +4);	// VOL Hdr Clear...
			szRead = fp.Read (bufTmp, szMP4Code);
			curStreamPos += szRead;

			// Stream Verify[S]...
			if (szRead > posVOPH +5
				&&
				(bufTmp[posVOPH +0] != 0x00 ||
				bufTmp[posVOPH +1] != 0x00 ||
				bufTmp[posVOPH +2] != 0x01 ||
				bufTmp[posVOPH +3] != 0xb6 ||
				(bufTmp[posVOPH +4] > 0x40 && ((hdrMP4RecFrm.nStat[0] >>22) &0x3) == 0) ||
				(bufTmp[posVOPH +4] < 0x40 && ((hdrMP4RecFrm.nStat[0] >>22) &0x3) == 1))
				)
			{
				curStreamPos -= (szRead +sizeof(HEADER_MP4_FRM));

				bOK = FALSE;
				if (bSzFrmInc == 1)
				{
					AddReport ("VOP ERROR :%0x", curStreamPos);
					numErrVOP++;
					continue;
				}
				else
				{
					AddReport ("BAD Stream :%0x", curStreamPos);
					break;
				}
			}
			// Stream Verify[E]...

			if (szRead < szMP4Code)	// Unexpected End...
			{
				AddReport ("File End 2");
				break;
			}

			tmpBufFileIdx[idxFrm].isKeyFrame = (bufTmp[posVOPH +4] > 0x40) ? 0 : 1;	// isKeyFrame
			tmpBufFileIdx[idxFrm].nScale = (hdrMP4RecFrm.nStat[1] >>28) &0xf;	// Scale...
			tmpBufFileIdx[idxFrm].idxCh = hdrMP4RecFrm.nStat[9] &0x1f;	// Channel...
			tmpBufFileIdx[idxFrm].bMotionExist = m_bMotionEx;	// is Motion Exist...
			tmpBufFileIdx[idxFrm].nInterval = (hdrMP4RecFrm.nStat[4] >>20) &0x3ff;

			bufTotSzFrm[tmpBufFileIdx[idxFrm].idxCh] += szMP4Code;	// Total Size Read (for Debug)

			// idxFrame[S]...
			tmpBufFileIdx[idxFrm].idxFramePos = curStreamPos -szRead +(256 *m_bMotionEx);
			// idxFrame[E]...

			tmpBufFileIdx[idxFrm].szFrame = szMP4Code;	// szFrame...

			// Time Code[S]...
			if ((bufTmp[posVOPH +4] &0x20) == 0x20)	// modulo 1...
			{
				tmpBufFileIdx[idxFrm].nTimeCode = ((bufTmp[posVOPH +4] &0x7) <<12)
					+((bufTmp[posVOPH +5]) <<4) +(bufTmp[posVOPH +6] >>4);
			}
			else	// modulo 0...
			{
				tmpBufFileIdx[idxFrm].nTimeCode = ((bufTmp[posVOPH +4] &0xf) <<11)
					+((bufTmp[posVOPH +5]) <<3) +(bufTmp[posVOPH +6] >>5);
			}
			// Time Code[E]...

			AddReport ("Ch:%d, idxFrm:%d, Time:%d, Scale:%d, idxFrmPos:%d, szFrm:%d, IsKey:%d, TS:%d, TUS:%d",
				tmpBufFileIdx[idxFrm].idxCh, idxFrm, tmpBufFileIdx[idxFrm].nTimeCode,
				tmpBufFileIdx[idxFrm].nScale, tmpBufFileIdx[idxFrm].idxFramePos,
				tmpBufFileIdx[idxFrm].szFrame, tmpBufFileIdx[idxFrm].isKeyFrame,
				hdrMP4RecFrm.nStat[5], hdrMP4RecFrm.nStat[6]);

			idxFrm++;
		}
		if (bOK == TRUE)
		{
			break;
		}
		if (idxFrm > numFrmMaxDec)
		{
			idxMaxDec = i;
			numFrmMaxDec = idxFrm;
		}
		if (bLastRead == TRUE)
		{
			break;
		}
	}

	if (i == 16)
	{
		m_bMotionEx = GL_B_MOTION[idxMaxDec %8];
		m_nAlign = GL_N_ALIGN[idxMaxDec %8];
		idxFrm = numFrmMaxDec;

		if (numFrmMaxDec == 0)
		{
			delete[] bufTmp;
			return FALSE;
		}
		else
		{
			bLastRead = TRUE;
			goto START_READ;
		}
	}

	delete[] bufTmp;
	fp.Close ();

	for (i=0; i<idxFrm; i++)
	{
		m_nNumTotFrame[tmpBufFileIdx[i].idxCh]++;
	}
	for (i=0; i<MAX_ENC_CHANNEL; i++)
	{
		if (m_nNumTotFrame[i] != 0)
		{
			m_bufFileIdx[i] = new ST_MPEG4_FILE_IDX[m_nNumTotFrame[i]];
		}
	}

	BOOL bufBStartIFrm[MAX_ENC_CHANNEL];
	memset (bufBStartIFrm, 0, sizeof(BOOL) *MAX_ENC_CHANNEL);

	for (i=0; i<idxFrm; i++)
	{
		idxCh = tmpBufFileIdx[i].idxCh;

		if (bufBStartIFrm[idxCh] == FALSE)
		{
			if (tmpBufFileIdx[i].isKeyFrame == TRUE)
			{
				bufBStartIFrm[idxCh] = TRUE;
			}
			else
			{
				continue;
			}
		}

		memcpy (&m_bufFileIdx[idxCh][m_nNumValidFrame[idxCh]],
			&tmpBufFileIdx[i], sizeof(ST_MPEG4_FILE_IDX));
		m_nNumValidFrame[idxCh]++;
	}

	delete[] tmpBufFileIdx;

	AddReport ("SZ_FRM");
	for (i=0; i<MAX_ENC_CHANNEL; i++)
	{
		if (m_nNumTotFrame[i] != 0)
		{
			AddReport ("%02d: %8d, %8d/frm", i, bufTotSzFrm[i], bufTotSzFrm[i] /m_nNumTotFrame[i]);
		}
	}

	return TRUE;
}

DWORD CInfoS6010StreamIdx::GetAlignedSzFrame (DWORD szFrameRAW, int nAlign)
{
	DWORD szMP4Code;

	switch (nAlign)
	{
	case 8:
		szMP4Code = (szFrameRAW +1 +0x7) &0xffff8;
		break;
	case 16:
		szMP4Code = (szFrameRAW +1 +0xf) &0xffff0;
		break;
	case 32:
		szMP4Code = (szFrameRAW +1 +0x1f) &0xfffe0;
		break;
	case 64:
		szMP4Code = (szFrameRAW +1 +0x3f) &0xfffc0;
		break;
	}

	return szMP4Code;
}

BOOL CInfoS6010StreamIdx::MakeIndexFromIndexBuf (CString strMPEG4FileName)
{
	AddReport ("MakeIndexFromIndexBuf ()");
	Free ();

	CFile fp;
	if (fp.Open (strMPEG4FileName, CFile::modeRead) == FALSE)
	{
		return FALSE;
	}

	DWORD i, idxFrm, idxCh, szMP4Code, posVOPH;
	DWORD bufTotSzFrm[MAX_ENC_CHANNEL];

	// Read HEADER_RECORD_FILE[S]
	HEADER_RECORD_FILE hdrMP4File;
	fp.Read (&hdrMP4File, sizeof(HEADER_RECORD_FILE));

	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		if (hdrMP4File.MP4.bufCRU[i] > 0)
		{
			m_bufIdxVideo[i] = hdrMP4File.MP4.typeVideo;
		}
		else
		{
			m_bufIdxVideo[i] = IDX_DEF_VIDEO;
		}
		AddReport ("CH:%d [B%d, S%d, CRU:%d]",
			i +1, hdrMP4File.MP4.bufSetRec[i].bSet, hdrMP4File.MP4.bufSetRec[i].idxImgSz, hdrMP4File.MP4.bufCRU[i]);
	}
	AddReport ("NumFrm:%d, posIndex:%d", hdrMP4File.MP4.numFrm, hdrMP4File.MP4.posIndex);
	// Read HEADER_RECORD_FILE[E]

	// Make ITEM_VID_REC_FILE_INDEX[S]
	ITEM_VID_REC_FILE_INDEX *bufMP4Idx = new ITEM_VID_REC_FILE_INDEX[hdrMP4File.MP4.numFrm];
	fp.Seek (hdrMP4File.MP4.posIndex, CFile::begin);
	fp.Read (bufMP4Idx, sizeof(ITEM_VID_REC_FILE_INDEX) *hdrMP4File.MP4.numFrm);
	// Make ITEM_VID_REC_FILE_INDEX[E]

	BYTE *bufTmp = new BYTE[SZ_TMP_BUF];
	ST_MPEG4_FILE_IDX *tmpBufFileIdx = new ST_MPEG4_FILE_IDX[hdrMP4File.MP4.numFrm];
	HEADER_MP4_FRM hdrMP4RecFrm;

	idxFrm = 0;
	m_bMotionEx = hdrMP4File.MP4.bExistMotionData;
	m_nAlign = 32;
	memset (bufTotSzFrm, 0, sizeof(DWORD) *MAX_ENC_CHANNEL);
	
	while (1)
	{
		fp.Seek (bufMP4Idx[idxFrm].posFrm, CFile::begin);
		fp.Read (&hdrMP4RecFrm, sizeof(HEADER_MP4_FRM));
		fp.Read (bufTmp, 1024);
		posVOPH = 256 *m_bMotionEx;

		tmpBufFileIdx[idxFrm].isKeyFrame = 
			(((hdrMP4RecFrm.nStat[0] >>22) &0x01) == 1) ? 0 : 1;	// isKeyFrame
		tmpBufFileIdx[idxFrm].nScale = (hdrMP4RecFrm.nStat[1] >>28) &0xf;	// Scale
		tmpBufFileIdx[idxFrm].idxCh = hdrMP4RecFrm.nStat[9] &0x1f;	// Channel
		tmpBufFileIdx[idxFrm].bMotionExist = m_bMotionEx;	// is Motion Exist
		tmpBufFileIdx[idxFrm].nInterval = (hdrMP4RecFrm.nStat[4] >>20) &0x3ff;

		szMP4Code = GetAlignedSzFrame (hdrMP4RecFrm.nStat[10], m_nAlign) +256 *m_bMotionEx;
		bufTotSzFrm[tmpBufFileIdx[idxFrm].idxCh] += szMP4Code;	// Total Size Read (for Debug)

		// Pos, Size[S]
		tmpBufFileIdx[idxFrm].idxFramePos = bufMP4Idx[idxFrm].posFrm +sizeof(HEADER_MP4_FRM)
			+m_bMotionEx *256;
		tmpBufFileIdx[idxFrm].szFrame = szMP4Code;	// szFrame
		// Pos, Size[E]

		// Time Code[S]
		if ((bufTmp[posVOPH +4] &0x20) == 0x20)	// modulo 1
		{
			tmpBufFileIdx[idxFrm].nTimeCode = ((bufTmp[posVOPH +4] &0x7) <<12)
				+((bufTmp[posVOPH +5]) <<4) +(bufTmp[posVOPH +6] >>4);
		}
		else	// modulo 0
		{
			tmpBufFileIdx[idxFrm].nTimeCode = ((bufTmp[posVOPH +4] &0xf) <<11)
				+((bufTmp[posVOPH +5]) <<3) +(bufTmp[posVOPH +6] >>5);
		}
		// Time Code[E]

		AddReport ("Ch:%d, idxFrm:%d, Time:%d, Scale:%d, idxFrmPos:%d, szFrm:%d, IsKey:%d, TS:%d, TUS:%d. ID_POS:%d",
			tmpBufFileIdx[idxFrm].idxCh, idxFrm, tmpBufFileIdx[idxFrm].nTimeCode,
			tmpBufFileIdx[idxFrm].nScale, tmpBufFileIdx[idxFrm].idxFramePos,
			tmpBufFileIdx[idxFrm].szFrame, tmpBufFileIdx[idxFrm].isKeyFrame,
			hdrMP4RecFrm.nStat[5], hdrMP4RecFrm.nStat[6],
			bufMP4Idx[idxFrm].posFrm);

		m_nNumTotFrame[bufMP4Idx[idxFrm].idxCh]++;

		idxFrm++;
		if (idxFrm >= hdrMP4File.MP4.numFrm)
		{
			break;
		}
	}

	delete[] bufTmp;
	fp.Close ();

	for (i=0; i<MAX_ENC_CHANNEL; i++)
	{
		if (m_nNumTotFrame[i] != 0)
		{
			m_bufFileIdx[i] = new ST_MPEG4_FILE_IDX[m_nNumTotFrame[i]];
		}
		AddReport ("Ch:%d, NumFrm:%d", i +1, m_nNumTotFrame[i]);
	}

	BOOL bufBStartIFrm[MAX_ENC_CHANNEL];
	memset (bufBStartIFrm, 0, sizeof(BOOL) *MAX_ENC_CHANNEL);

	for (i=0; i<idxFrm; i++)
	{
		idxCh = tmpBufFileIdx[i].idxCh;

		if (m_nNumTotFrame[idxCh] == 0)
		{
			continue;
		}
		if (bufBStartIFrm[idxCh] == FALSE)
		{
			if (tmpBufFileIdx[i].isKeyFrame == TRUE)
			{
				bufBStartIFrm[idxCh] = TRUE;
			}
			else
			{
				continue;
			}
		}

		memcpy (&m_bufFileIdx[idxCh][m_nNumValidFrame[idxCh]],
			&tmpBufFileIdx[i], sizeof(ST_MPEG4_FILE_IDX));
		m_nNumValidFrame[idxCh]++;
	}

	delete[] tmpBufFileIdx;
	delete[] bufMP4Idx;

	AddReport ("SZ_FRM");
	for (i=0; i<MAX_ENC_CHANNEL; i++)
	{
		if (m_nNumTotFrame[i] != 0)
		{
			AddReport ("%02d: %8d, %8d/frm", i, bufTotSzFrm[i], bufTotSzFrm[i] /m_nNumTotFrame[i]);
		}
	}

	return TRUE;
}
