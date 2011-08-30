// DooMPEG4ToAVI.cpp: implementation of the CDooMPEG4ToAVI class.
//
//////////////////////////////////////////////////////////////////////

#include "DooMPEG4ToAVI.h"


#include <vfw.h>
#include <AVIRIFF.h>

#define STR_STREAM_FILE_NAME		".\\DOO_TMP_STREAM_FILE.doo"
#define STR_IDX_FILE_NAME			".\\DOO_TMP_IDX_FILE.doo"

DWORD FCC_RIFF = mmioFOURCC('R', 'I', 'F', 'F');
DWORD FCC_AVI  = mmioFOURCC('A', 'V', 'I', ' ');
DWORD FCC_hdrl = mmioFOURCC('h', 'd', 'r', 'l');
DWORD FCC_strl = mmioFOURCC('s', 't', 'r', 'l');
DWORD FCC_strf = mmioFOURCC('s', 't', 'r', 'f');
DWORD FCC_divx = mmioFOURCC('d', 'i', 'v', 'x');
DWORD FCC_DX50 = mmioFOURCC('D', 'X', '5', '0');
DWORD FCC_mp4s = mmioFOURCC('M', 'P', '4', 'S');
DWORD FCC_MP4S = mmioFOURCC('M', 'P', '4', 'S');
DWORD FCC_xvid = mmioFOURCC('x', 'v', 'i', 'd');
DWORD FCC_XVID = mmioFOURCC('X', 'V', 'I', 'D');
DWORD FCC_LIST = mmioFOURCC('L', 'I', 'S', 'T');
DWORD FCC_JUNK = mmioFOURCC('J', 'U', 'N', 'K');
DWORD FCC_odml = mmioFOURCC('o', 'd', 'm', 'l');
DWORD FCC_dmlh = mmioFOURCC('d', 'm', 'l', 'h');
DWORD FCC_00DC = mmioFOURCC('0', '0', 'd', 'c');
DWORD FCC_movi = mmioFOURCC('m', 'o', 'v', 'i');
DWORD FCC_idx1 = mmioFOURCC('i', 'd', 'x', '1');

#define DOO_AVI_IDX_KEYFRM			0x00000010
#define DOO_AVI_IDX_NOT_KEYFRM		0x00000000

#define GET_WORD_SIZE(x)			((x +1) /2 *2)
#define GET_REAL_FRM_SZ(buf, sz)	((buf[sz -1]!=0)?(sz):\
	((buf[sz -2]!=0)?(sz -1):((buf[sz -3]!=0)?(sz -2):(sz -3))))

struct DOO_AVI_IDX
{
	DWORD   dwChunkId;
	DWORD   dwFlags;
	DWORD   dwOffset;
	DWORD   dwSize;
};

#define SIZE_TMP_BUF		0x400000

const char strDooInfo[] = "This Movie File was made by DOO in SoftLogic...";

const unsigned char BUF_DUMMY_FRM[] = {
	0x00, 0x00, 0x01, 0xb6, 0x50, 0x00, 0x13
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDooMPEG4ToAVI::CDooMPEG4ToAVI()
{
	m_numFrm = 0;
	m_szH = 0;
	m_szV = 0;
	m_numMSPF = 33333;

	m_szFileStream = 0;

	m_hStreamFile = NULL;
	m_hIdxFile = NULL;
}

CDooMPEG4ToAVI::~CDooMPEG4ToAVI()
{
}

int CDooMPEG4ToAVI::StartConvert (unsigned int szH, unsigned int szV, unsigned char *bufStartFrm, unsigned int szStartFrm)
{
	DWORD nWritten;

	m_hStreamFile = CreateFile (STR_STREAM_FILE_NAME, GENERIC_READ |GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);
	m_hIdxFile = CreateFile (STR_IDX_FILE_NAME, GENERIC_READ |GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (m_hStreamFile == NULL || m_hIdxFile == NULL)
	{
		return 0;
	}

	m_szH = szH;
	m_szV = szV;

	m_szFileStream = GET_WORD_SIZE(szStartFrm);
	m_numFrm = 1;

	DOO_AVI_IDX infoIdx;
	infoIdx.dwChunkId = FCC_00DC;
	infoIdx.dwFlags = DOO_AVI_IDX_KEYFRM;
	infoIdx.dwOffset = 0x04;
	infoIdx.dwSize = GET_REAL_FRM_SZ(bufStartFrm, szStartFrm);

	m_idxNextOffset = infoIdx.dwOffset +GET_WORD_SIZE(szStartFrm) +8;

	BYTE *bufNULLHeader = new BYTE[0x180c];
	memset (bufNULLHeader, 0, 0x180c);

	WriteFile (m_hStreamFile, bufNULLHeader, 0x180c, &nWritten, NULL);

	delete[] bufNULLHeader;

	WriteFile (m_hStreamFile, &FCC_00DC, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &m_szFileStream, 4, &nWritten, NULL);

	m_szFileStream += 8;

	WriteFile (m_hStreamFile, bufStartFrm, szStartFrm, &nWritten, NULL);

	if (szStartFrm %2 == 1)
	{
		BYTE dummy = 0;
		WriteFile (m_hStreamFile, &dummy, 1, &nWritten, NULL);
	}

	WriteFile (m_hIdxFile, &infoIdx, sizeof(DOO_AVI_IDX), &nWritten, NULL);

	return 1;
}

int CDooMPEG4ToAVI::AddFrame (unsigned char *bufFrm, unsigned int szFrm)
{
	DWORD nWritten;

	m_szFileStream += GET_WORD_SIZE(8 +szFrm);
	m_numFrm++;

	DOO_AVI_IDX infoIdx;
	infoIdx.dwChunkId = FCC_00DC;
	if (bufFrm[4] >>6 == 0x00)
	{
		infoIdx.dwFlags = DOO_AVI_IDX_KEYFRM;
	}
	else
	{
		infoIdx.dwFlags = DOO_AVI_IDX_NOT_KEYFRM;
	}
	infoIdx.dwOffset = m_idxNextOffset;
	infoIdx.dwSize = GET_REAL_FRM_SZ(bufFrm, szFrm);

	m_idxNextOffset = infoIdx.dwOffset +GET_WORD_SIZE(szFrm) +8;

	WriteFile (m_hStreamFile, &FCC_00DC, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &szFrm, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, bufFrm, szFrm, &nWritten, NULL);
	if (szFrm %2 == 1)
	{
		BYTE dummy = 0;
		WriteFile (m_hStreamFile, &dummy, 1, &nWritten, NULL);
	}

	WriteFile (m_hIdxFile, &infoIdx, sizeof(DOO_AVI_IDX), &nWritten, NULL);

	return 1;
}

int CDooMPEG4ToAVI::AddDummyFrame ()
{
	DWORD nWritten, szFrm;

	szFrm = sizeof(BUF_DUMMY_FRM);
	m_szFileStream += GET_WORD_SIZE(8 +szFrm);
	m_numFrm++;

	DOO_AVI_IDX infoIdx;
	infoIdx.dwChunkId = FCC_00DC;
	infoIdx.dwFlags = DOO_AVI_IDX_NOT_KEYFRM;
	infoIdx.dwOffset = m_idxNextOffset;
	infoIdx.dwSize = szFrm;

	m_idxNextOffset = infoIdx.dwOffset +GET_WORD_SIZE(szFrm) +8;

	WriteFile (m_hStreamFile, &FCC_00DC, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &szFrm, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, BUF_DUMMY_FRM, szFrm, &nWritten, NULL);
	if (szFrm %2 == 1)
	{
		BYTE dummy = 0;
		WriteFile (m_hStreamFile, &dummy, 1, &nWritten, NULL);
	}

	WriteFile (m_hIdxFile, &infoIdx, sizeof(DOO_AVI_IDX), &nWritten, NULL);

	return 1;
}

int CDooMPEG4ToAVI::EndConvertFPSx100 (const char *strDestFileName, unsigned long numFPSx100)
{
	return EndConvertMSPF (strDestFileName, 1000000 *100 /numFPSx100);
}
int CDooMPEG4ToAVI::EndConvertMSPF (const char *strDestFileName, unsigned long numMSPF)
{
	DWORD nWritten, nRead;

	m_numMSPF = numMSPF;

	AddHeader ();

	DWORD szListMovi = m_szFileStream +4;
	DWORD szIndex = m_numFrm *0x10;
	DWORD nRemain;
	BYTE *bufTmp = new BYTE[SIZE_TMP_BUF];

	// Write LIST movi[S]...
	WriteFile (m_hStreamFile, &FCC_LIST, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &szListMovi, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &FCC_movi, 4, &nWritten, NULL);
	// Write LIST movi[E]...

	// Write Index Data[S]...
	SetFilePointer (m_hStreamFile, 0, NULL, FILE_END);
	SetFilePointer (m_hIdxFile, 0, NULL, FILE_BEGIN);

	WriteFile (m_hStreamFile, &FCC_idx1, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &szIndex, 4, &nWritten, NULL);

	nRemain = szIndex;
	while (nRemain > SIZE_TMP_BUF)
	{
		ReadFile (m_hIdxFile, bufTmp, SIZE_TMP_BUF, &nRead, NULL);
		WriteFile (m_hStreamFile, bufTmp, SIZE_TMP_BUF, &nWritten, NULL);
		nRemain -= SIZE_TMP_BUF;
	}
	ReadFile (m_hIdxFile, bufTmp, nRemain, &nRead, NULL);
	WriteFile (m_hStreamFile, bufTmp, nRemain, &nWritten, NULL);
	// Write Index Data[E]...

	CloseHandle (m_hStreamFile);
	CloseHandle (m_hIdxFile);
	DeleteFile (STR_IDX_FILE_NAME);

	DeleteFile (strDestFileName);
	if (MoveFile (STR_STREAM_FILE_NAME, strDestFileName) == FALSE)
	{
		return 0;
	}

	return 1;
}

void CDooMPEG4ToAVI::AddHeader ()
{
	DWORD nWritten;

	SetFilePointer (m_hStreamFile, 0, NULL, FILE_BEGIN);

	DWORD szRIFF = 0x1800 -8 +(8 +m_szFileStream +4) +(8 +m_numFrm *0x10);
	DWORD szListHdrl = 0x000011ec;
	DWORD szListStrl = 0x00001094;
	DWORD szStrlStrf = 0x00000028;
	DWORD szListOdml = 0x00000104;
	DWORD szOdmlDmlh = 0x000000f8;
	DWORD szJUNK;

	BYTE *bufJUNK = new BYTE[0xffff];
	memset (bufJUNK, 0, 0xffff);
	memcpy (bufJUNK, strDooInfo, sizeof(strDooInfo));

	BYTE *buf_dmlh = new BYTE[0xf8];
	memset (buf_dmlh, 0, 0xf8);
	buf_dmlh[0] = 1;

	WriteFile (m_hStreamFile, &FCC_RIFF, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &szRIFF, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &FCC_AVI, 4, &nWritten, NULL);

	WriteFile (m_hStreamFile, &FCC_LIST, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &szListHdrl, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &FCC_hdrl, 4, &nWritten, NULL);

	// AVIH[S]...
	AVIMAINHEADER AVI_MAIN_HEADER;
	AVI_MAIN_HEADER.fcc = ckidAVIMAINHDR;
	AVI_MAIN_HEADER.cb = 0x00000038;
	AVI_MAIN_HEADER.dwMicroSecPerFrame = m_numMSPF;
	AVI_MAIN_HEADER.dwMaxBytesPerSec = 0;
	AVI_MAIN_HEADER.dwPaddingGranularity = 0;
	AVI_MAIN_HEADER.dwFlags = AVIF_HASINDEX;
	AVI_MAIN_HEADER.dwTotalFrames = m_numFrm;
	AVI_MAIN_HEADER.dwInitialFrames = 0;
	AVI_MAIN_HEADER.dwStreams = 1;
	AVI_MAIN_HEADER.dwSuggestedBufferSize = 0;
	AVI_MAIN_HEADER.dwWidth = m_szH;
	AVI_MAIN_HEADER.dwHeight = m_szV;
	AVI_MAIN_HEADER.dwReserved[0] = 0;
	AVI_MAIN_HEADER.dwReserved[1] = 0;
	AVI_MAIN_HEADER.dwReserved[2] = 0;
	AVI_MAIN_HEADER.dwReserved[3] = 0;
	WriteFile (m_hStreamFile, &AVI_MAIN_HEADER, sizeof(AVIMAINHEADER), &nWritten, NULL);
	// AVIH[E]...

	WriteFile (m_hStreamFile, &FCC_LIST, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &szListStrl, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &FCC_strl, 4, &nWritten, NULL);

	// STRH[S]...
	AVISTREAMHEADER AVI_STREAM_HEADER;
	AVI_STREAM_HEADER.fcc = ckidSTREAMHEADER;
	AVI_STREAM_HEADER.cb = 0x00000038;
	AVI_STREAM_HEADER.fccType = streamtypeVIDEO;
//	AVI_STREAM_HEADER.fccHandler = FCC_mp4s;
//	AVI_STREAM_HEADER.fccHandler = FCC_divx;
	AVI_STREAM_HEADER.fccHandler = FCC_xvid;
	AVI_STREAM_HEADER.dwFlags = 0;
	AVI_STREAM_HEADER.wPriority = 0;
	AVI_STREAM_HEADER.wLanguage = 0;
	AVI_STREAM_HEADER.dwInitialFrames = 0;
	AVI_STREAM_HEADER.dwScale = 1000;
	AVI_STREAM_HEADER.dwRate = (1000000 /m_numMSPF) *1000;
	AVI_STREAM_HEADER.dwStart = 0;
	AVI_STREAM_HEADER.dwLength = m_numFrm;
	AVI_STREAM_HEADER.dwSuggestedBufferSize = 0xffff;
	AVI_STREAM_HEADER.dwQuality = 10000;
	AVI_STREAM_HEADER.dwSampleSize = 0;
	AVI_STREAM_HEADER.rcFrame.left = 0;
	AVI_STREAM_HEADER.rcFrame.top = 0;
	AVI_STREAM_HEADER.rcFrame.right = m_szH;
	AVI_STREAM_HEADER.rcFrame.bottom = m_szV;
	WriteFile (m_hStreamFile, &AVI_STREAM_HEADER, sizeof(AVI_STREAM_HEADER), &nWritten, NULL);
	// STRH[E]...

	// STRF[S]...
	WriteFile (m_hStreamFile, &FCC_strf, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &szStrlStrf, 4, &nWritten, NULL);

	BITMAPINFOHEADER STRF_HEADER;
	STRF_HEADER.biSize = sizeof(BITMAPINFOHEADER);
	STRF_HEADER.biWidth = m_szH;
	STRF_HEADER.biHeight = m_szV;
	STRF_HEADER.biPlanes = 1;
	STRF_HEADER.biBitCount = 24;
//	STRF_HEADER.biCompression = FCC_MP4S;
//	STRF_HEADER.biCompression = FCC_DX50;
	STRF_HEADER.biCompression = FCC_XVID;
	STRF_HEADER.biSizeImage = m_szH *m_szV *6;
	STRF_HEADER.biXPelsPerMeter = 0;
	STRF_HEADER.biYPelsPerMeter = 0;
	STRF_HEADER.biClrUsed = 0;
	STRF_HEADER.biClrImportant = 0;
	WriteFile (m_hStreamFile, &STRF_HEADER, sizeof(STRF_HEADER), &nWritten, NULL);
	// STRF[E]...

	// JUNK[S]...
	WriteFile (m_hStreamFile, &FCC_JUNK, 4, &nWritten, NULL);
	szJUNK = 0x1018;
	WriteFile (m_hStreamFile, &szJUNK, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, bufJUNK, szJUNK, &nWritten, NULL);
	// JUNK[E]...

	WriteFile (m_hStreamFile, &FCC_LIST, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &szListOdml, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &FCC_odml, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &FCC_dmlh, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, &szOdmlDmlh, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, buf_dmlh, 0xf8, &nWritten, NULL);

	// JUNK[S]...
	WriteFile (m_hStreamFile, &FCC_JUNK, 4, &nWritten, NULL);
	szJUNK = 0x05f8;
	WriteFile (m_hStreamFile, &szJUNK, 4, &nWritten, NULL);
	WriteFile (m_hStreamFile, bufJUNK, szJUNK, &nWritten, NULL);
	// JUNK[E]...

	delete[] bufJUNK;
}
