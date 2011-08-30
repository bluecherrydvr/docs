#include "stdafx.h"
#include "RecManager.h"
#include <Mmsystem.h>


#define STR_MP4_RAW_FILE_FORMAT			"%sRM4_%d_%02d%02d%02d_%02d%02d%02d.slf"
#define STR_JPEG_RAW_FILE_FORMAT		"%sRJP_%d_%02d%02d%02d_%02d%02d%02d.slf"
#define STR_G723_RAW_FILE_FORMAT		"%sRG7_%d_%02d%02d%02d_%02d%02d%02d.slf"

#define STR_MP4_BY_CH_FILE_FORMAT		"%sRM4_%d_C%02d_%02d%02d%02d_%02d%02d%02d.slf"
#define STR_JPEG_BY_CH_FILE_FORMAT		"%sRJP_%d_C%02d_%02d%02d%02d_%02d%02d%02d.slf"
#define STR_G723_BY_CH_FILE_FORMAT		"%sRG7_%d_C%02d_%02d%02d%02d_%02d%02d%02d.slf"


BOOL CRecManager::m_staBInitCrtFileAcc;
CRITICAL_SECTION CRecManager::m_staCrtFileAcc;


#pragma warning( disable : 4244 )

void CALLBACK CbFn_MP4CaptureRawData (void *pContext, BYTE *bufMP4, DWORD posStart, DWORD posEnd, DWORD szBuf, DWORD flagError)
{
	// Information about argument
	// 
	// pContext = pointer of CRecManager class
	// bufMP4 = MPEG4 App/Drv common whole ring buffer
	// posStart = start address of received raw stream
	// posEnd = end address of received raw stream
	// szBuf = Entire size of bufMP4 (CAUTION)

#ifdef __S6010APP_REC_NO_WRITE__
	return;
#endif

	CRecManager *pRecM = (CRecManager *)pContext;
	DWORD szWritten;

	// Stream Write[S]
	if (pRecM->m_bufHFileMP4[0] == INVALID_HANDLE_VALUE)
	{
		return;
	}

	EnterCriticalSection (&pRecM->m_staCrtFileAcc);
	if (posEnd < posStart)
	{	// ring buffer overlap
		WriteFile (pRecM->m_bufHFileMP4[0], &bufMP4[posStart], szBuf -posStart, &szWritten, NULL);
		WriteFile (pRecM->m_bufHFileMP4[0], &bufMP4[0], posEnd, &szWritten, NULL);
	}
	else
	{
		WriteFile (pRecM->m_bufHFileMP4[0], &bufMP4[posStart], posEnd -posStart, &szWritten, NULL);
	}
	LeaveCriticalSection (&pRecM->m_staCrtFileAcc);
	// Stream Write[E]

	if (flagError != 0)
	{
		if (flagError &REC_BUF_STAT_FLAG_SDRAM_OVF)			AddReport ("MPEG4: SDRAM Overflow");
		else if (flagError &REC_BUF_STAT_FLAG_SHR_MEM_OVF)	AddReport ("MPEG4: Shared Memory Overflow");
		else if (flagError &REC_BUF_STAT_FLAG_P2M_ERROR)	AddReport ("MPEG4: P2M Error");
		else												AddReport ("MPEG4: Unknown Error");
	}
}

void CALLBACK CbFn_MP4CaptureFrame (void *pContext, BYTE *bufMP4, DWORD posStart, DWORD posEnd, DWORD szBuf, DWORD flagError)
{
	// Information about argument
	// 
	// pContext = pointer of CRecManager class
	// bufMP4 = [HEADER_MP4_FRM (64)] +[Encoding motion data (256)] +[MPEG4 VOP data (variable)]
	// posStart = 0;
	// posEnd = size of "MPEG4 VOP data"
	// szBuf = posEnd

#ifdef __S6010APP_REC_NO_WRITE__
	return;
#endif

	CRecManager *pRecM = (CRecManager *)pContext;

	int idxCh = GET_CH_PT(((HEADER_MP4_FRM *)bufMP4));
	DWORD szWritten;

	// Stream Write[S]
	if (pRecM->m_bufHFileMP4[idxCh] == INVALID_HANDLE_VALUE)
	{
		return;
	}

	EnterCriticalSection (&pRecM->m_staCrtFileAcc);
	WriteFile (pRecM->m_bufHFileMP4[idxCh], bufMP4, szBuf, &szWritten, NULL);
	LeaveCriticalSection (&pRecM->m_staCrtFileAcc);
	// Stream Write[E]

	if (flagError != 0)
	{
		if (flagError &REC_BUF_STAT_FLAG_SDRAM_OVF)			AddReport ("MPEG4: SDRAM Overflow");
		else if (flagError &REC_BUF_STAT_FLAG_SHR_MEM_OVF)	AddReport ("MPEG4: Shared Memory Overflow");
		else if (flagError &REC_BUF_STAT_FLAG_P2M_ERROR)	AddReport ("MPEG4: P2M Error");
		else												AddReport ("MPEG4: Unknown Error");
	}
}

void CALLBACK CbFn_JPEGCaptureRawData (void *pContext, BYTE *bufJPEG, DWORD posStart, DWORD posEnd, DWORD szBuf, DWORD flagError)
{
#ifdef __S6010APP_REC_NO_WRITE__
	return;
#endif

	CRecManager *pRecM = (CRecManager *)pContext;
	DWORD szWritten;

	// Stream Write[S]
	if (pRecM->m_bufHFileJPEG[0] == INVALID_HANDLE_VALUE)
	{
		return;
	}

	EnterCriticalSection (&pRecM->m_staCrtFileAcc);
	if (posEnd < posStart)
	{	// ring buffer overlap
		WriteFile (pRecM->m_bufHFileJPEG[0], &bufJPEG[posStart], szBuf -posStart, &szWritten, NULL);
		WriteFile (pRecM->m_bufHFileJPEG[0], &bufJPEG[0], posEnd, &szWritten, NULL);
	}
	else
	{
		WriteFile (pRecM->m_bufHFileJPEG[0], &bufJPEG[posStart], posEnd -posStart, &szWritten, NULL);
	}
	LeaveCriticalSection (&pRecM->m_staCrtFileAcc);
	// Stream Write[E]

	if (flagError != 0)
	{
		if (flagError &REC_BUF_STAT_FLAG_SDRAM_OVF)			AddReport ("JPEG: SDRAM Overflow");
		else if (flagError &REC_BUF_STAT_FLAG_SHR_MEM_OVF)	AddReport ("JPEG: Shared Memory Overflow");
		else if (flagError &REC_BUF_STAT_FLAG_P2M_ERROR)	AddReport ("JPEG: P2M Error");
		else												AddReport ("JPEG: Unknown Error");
	}
}

void CALLBACK CbFn_JPEGCaptureFrame (void *pContext, BYTE *bufJPEG, DWORD posStart, DWORD posEnd, DWORD szBuf, DWORD flagError)
{
#ifdef __S6010APP_REC_NO_WRITE__
	return;
#endif

	if (flagError != 0)
	{
		if (flagError &REC_BUF_STAT_FLAG_SDRAM_OVF)			AddReport ("JPEG: SDRAM Overflow");
		else if (flagError &REC_BUF_STAT_FLAG_SHR_MEM_OVF)	AddReport ("JPEG: Shared Memory Overflow");
		else if (flagError &REC_BUF_STAT_FLAG_P2M_ERROR)	AddReport ("JPEG: P2M Error");
		else												AddReport ("JPEG: Unknown Error");
	}
}

void CALLBACK CbFn_G723CaptureRawData (void *pContext, int idxCh, BYTE *bufG723, DWORD szBuf, DWORD flagError)
{
#ifdef __S6010APP_REC_NO_WRITE__
	return;
#endif

	CRecManager *pRecM = (CRecManager *)pContext;

	unsigned long szWritten;

	if (pRecM->m_bufHFileG723[0] == INVALID_HANDLE_VALUE)
	{
		return;
	}

	// Stream Write[S]
	EnterCriticalSection (&pRecM->m_staCrtFileAcc);
	WriteFile (pRecM->m_bufHFileG723[0], bufG723, szBuf, &szWritten, NULL);
	LeaveCriticalSection (&pRecM->m_staCrtFileAcc);
	// Stream Write[E]

	if (flagError != 0)
	{
		if (flagError &REC_BUF_STAT_FLAG_SDRAM_OVF)			AddReport ("G723: SDRAM Overflow");
		else if (flagError &REC_BUF_STAT_FLAG_SHR_MEM_OVF)	AddReport ("G723: Shared Memory Overflow");
		else if (flagError &REC_BUF_STAT_FLAG_P2M_ERROR)	AddReport ("G723: P2M Error");
		else												AddReport ("G723: Unknown Error");
	}
}

void CALLBACK CbFn_G723CaptureFrame (void *pContext, int idxCh, BYTE *bufG723, DWORD szBuf, DWORD flagError)
{
#ifdef __S6010APP_REC_NO_WRITE__
	return;
#endif

	CRecManager *pRecM = (CRecManager *)pContext;

	DWORD szWritten;

	// Stream Write[S]
	if (pRecM->m_bufHFileG723[idxCh] == INVALID_HANDLE_VALUE)
	{
		return;
	}

	EnterCriticalSection (&pRecM->m_staCrtFileAcc);
	WriteFile (pRecM->m_bufHFileG723[idxCh], bufG723, szBuf, &szWritten, NULL);
	LeaveCriticalSection (&pRecM->m_staCrtFileAcc);
	// Stream Write[E]

	if (flagError != 0)
	{
		if (flagError &REC_BUF_STAT_FLAG_SDRAM_OVF)			AddReport ("G723: SDRAM Overflow");
		else if (flagError &REC_BUF_STAT_FLAG_SHR_MEM_OVF)	AddReport ("G723: Shared Memory Overflow");
		else if (flagError &REC_BUF_STAT_FLAG_P2M_ERROR)	AddReport ("G723: P2M Error");
		else												AddReport ("G723: Unknown Error");
	}
}

void CALLBACK CbFn_RecManagerTimerProc (UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	CRecManager *pRecM = (CRecManager *)dwUser;
	static volatile BOOL bInTimerProc = FALSE;

	if (pRecM->m_bRec == FALSE)
	{
		return;
	}

	if (bInTimerProc == TRUE)
	{
		return;
	}

	bInTimerProc = TRUE;

	pRecM->UpdateEncoderOSD ();

	bInTimerProc = FALSE;
}

#pragma warning( default : 4244 )

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRecManager::CRecManager()
{
	if (CRecManager::m_staBInitCrtFileAcc == FALSE)
	{
		InitializeCriticalSection (&m_staCrtFileAcc);
		CRecManager::m_staBInitCrtFileAcc = TRUE;
	}

	m_pS6010 = NULL;
	m_bRec = FALSE;

	int i;
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		m_bufHFileMP4[i] = INVALID_HANDLE_VALUE;
		m_bufHFileJPEG[i] = INVALID_HANDLE_VALUE;
		m_bufHFileG723[i] = INVALID_HANDLE_VALUE;
	}

	m_nMMTimerID = NULL;
}

CRecManager::~CRecManager()
{
	StopRec ();
	EndRec ();
}

void CRecManager::Init (CSOLO6010 *pS6010,
	BYTE modeRecMP4, BYTE modeRecJPEG, BYTE modeRecG723,
	BYTE nNotifyBufLevelMP4, BYTE nNotifyBufLevelJPEG, BYTE nNotifyBufLevelG723)
{
	m_pS6010 = pS6010;

	switch (modeRecMP4)
	{
	case REC_DATA_RCV_BY_FRAME:
		m_pS6010->REC_MP4_RegisterCallback (CbFn_MP4CaptureFrame, this);
		break;
	case REC_DATA_RCV_BY_RAW_DATA:
		m_pS6010->REC_MP4_RegisterCallback (CbFn_MP4CaptureRawData, this);
		break;
	}
	switch (modeRecJPEG)
	{
	case REC_DATA_RCV_BY_FRAME:
		m_pS6010->REC_JPEG_RegisterCallback (CbFn_JPEGCaptureFrame, this);
		break;
	case REC_DATA_RCV_BY_RAW_DATA:
		m_pS6010->REC_JPEG_RegisterCallback (CbFn_JPEGCaptureRawData, this);
		break;
	}
	switch (modeRecG723)
	{
	case REC_DATA_RCV_BY_FRAME:
		m_pS6010->REC_G723_RegisterCallback (CbFn_G723CaptureFrame, this);
		break;
	case REC_DATA_RCV_BY_RAW_DATA:
		m_pS6010->REC_G723_RegisterCallback (CbFn_G723CaptureRawData, this);
		break;
	}

	m_modeRecMP4 = modeRecMP4;
	m_modeRecJPEG = modeRecJPEG;
	m_modeRecG723 = modeRecG723;

	m_pS6010->REC_SetRecDataRcvMode (modeRecMP4, modeRecJPEG, modeRecG723);
	m_pS6010->REC_SetNotifyBufferLevel (nNotifyBufLevelMP4,
		nNotifyBufLevelJPEG, nNotifyBufLevelG723);
}

BOOL CRecManager::StartRec (SETTING_REC *pSetRec)
{
	if (m_pS6010 == NULL)
	{
		return FALSE;
	}
	if (m_bRec == TRUE)
	{
		return TRUE;
	}
	
	int i;
	int idxS6010 = m_pS6010->GENERAL_GetID ();
	BOOL bEncMP4, bEncJPEG, bEncG723;

	// Set record prop[S]
	//// MPEG4[S]
	bEncMP4 = FALSE;
	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		m_pS6010->REC_MP4_RealChSetProp (i,
			pSetRec->setMP4[i].bSet,
			pSetRec->setMP4[i].idxImgSz,
			pSetRec->setMP4[i].nKeyFrmInterval,
			pSetRec->setMP4[i].nQuality,
			pSetRec->setMP4[i].bIBasedPred,
			pSetRec->setMP4[i].idxFPS,
			pSetRec->setMP4[i].nMotionTh,
			pSetRec->setMP4[i].bUseCBR,
			pSetRec->setMP4[i].nBitrate,
			pSetRec->setMP4[i].szVirtualDecBuf);

		if (pSetRec->setMP4[i].bSet == TRUE)
		{
			bEncMP4 = TRUE;
		}
	}
	for (i=0; i<NUM_VID_VIRT_ENC_CH; i++)
	{
		m_pS6010->REC_MP4_VirtChSetProp (i,
			pSetRec->setMP4[NUM_VID_REAL_ENC_CH +i].bSet,
			pSetRec->setMP4[NUM_VID_REAL_ENC_CH +i].nKeyFrmInterval,
			pSetRec->setMP4[NUM_VID_REAL_ENC_CH +i].nQuality,
			pSetRec->setMP4[NUM_VID_REAL_ENC_CH +i].idxFPS,
			pSetRec->setMP4[NUM_VID_REAL_ENC_CH +i].bUseCBR,
			pSetRec->setMP4[NUM_VID_REAL_ENC_CH +i].nBitrate,
			pSetRec->setMP4[NUM_VID_REAL_ENC_CH +i].szVirtualDecBuf);
	}
	//// MPEG4[E]

	//// JPEG[S]
	bEncJPEG = FALSE;
	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		m_pS6010->REC_JPEG_RealChSetProp (i, pSetRec->setJPEG[i].bRec, JPEG_IMG_QUALITY_LOW);

		if (pSetRec->setMP4[i].bSet == TRUE && pSetRec->setJPEG[i].bRec == TRUE)
		{
			bEncJPEG = TRUE;
		}
	}
	for (i=0; i<NUM_VID_VIRT_ENC_CH; i++)
	{
		m_pS6010->REC_JPEG_VirtChSetProp (i, pSetRec->setJPEG[NUM_VID_REAL_ENC_CH +i].bRec, JPEG_IMG_QUALITY_LOW);
	}
	//// JPEG[E]

	//// G723[S]
	bEncG723 = FALSE;
	for (i=0; i<NUM_G723_CH_PAIR; i++)
	{
		m_pS6010->REC_G723_SetProp (i, pSetRec->setG723.bufBEnc[2 *i], pSetRec->setG723.idxSampRate);

		if (pSetRec->setG723.bufBEnc[2 *i] == TRUE)
		{
			bEncG723 = TRUE;
		}
	}

	memcpy (&m_settingG723Rec, &pSetRec->setG723, sizeof(SETTING_G723_REC));
	//// G723[E]
	// Set record prop[E]

	// Get date[S]
	SYSTEMTIME timeCur;
	GetLocalTime (&timeCur);

	INFO_DATE infoDate;
	infoDate.nYear = timeCur.wYear;
	infoDate.nMonth = (BYTE)timeCur.wMonth;
	infoDate.nDay = (BYTE)timeCur.wDay;
	infoDate.nHour = (BYTE)timeCur.wHour;
	infoDate.nMin = (BYTE)timeCur.wMinute;
	infoDate.nSec = (BYTE)timeCur.wSecond;
	// Get date[E]

	if (bEncMP4 == TRUE)
	{
		OpenMP4File (&infoDate, pSetRec, m_modeRecMP4);
	}
	if (bEncJPEG == TRUE)
	{
		OpenJPEGFile (&infoDate, pSetRec, m_modeRecJPEG);
	}
	if (bEncG723 == TRUE)
	{
		OpenG723File (&infoDate, pSetRec, m_modeRecG723);
	}
	
	// Encoder OSD init[S]
	memset (m_bufBRecCh, 0, sizeof(BOOL) *NUM_VID_TOTAL_ENC_CH);

	m_pS6010->ENC_OSD_Init (glBufGInfo[idxS6010].ENC_OSD_bUseShadowText,
		glBufGInfo[idxS6010].ENC_OSD_colorY, glBufGInfo[idxS6010].ENC_OSD_colorU, glBufGInfo[idxS6010].ENC_OSD_colorV,
		glBufGInfo[idxS6010].ENC_OSD_offsetH, glBufGInfo[idxS6010].ENC_OSD_offsetV);
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		if (pSetRec->setMP4[i].bSet == 1)
		{
			m_bufBRecCh[i] = TRUE;

			m_pS6010->ENC_OSD_Clear (i);
		}
	}

	UpdateEncoderOSD ();
	// Encoder OSD init[E]

	// MM Timer init[S]
	m_nMMTimerID = timeSetEvent (RM_DEF_TIMER_INTERVAL, CSOLO6010::AllocateTimerRes (),
		CbFn_RecManagerTimerProc, (DWORD)this, TIME_PERIODIC);
	// MM Timer init[E]

	// Record start[S]
	if (m_pS6010->REC_Start () == FALSE)
	{
		AddReport ("ERROR: m_pS6010->StartRec () Fail");

		if (bEncMP4 == TRUE)
		{
			CloseMP4File (&infoDate);
		}
		if (bEncJPEG == TRUE)
		{
			CloseJPEGFile (&infoDate);
		}
		if (bEncG723 == TRUE)
		{
			CloseG723File (&infoDate);
		}

		return FALSE;
	}
	// Record start[E]

	m_bRec = TRUE;

	AddReport ("Start Record OK");
	return TRUE;
}

BOOL CRecManager::StopRec ()
{
	if (m_bRec == FALSE)
	{
		return TRUE;
	}

	// MM Timer Destroy[S]
	timeKillEvent (m_nMMTimerID);
	m_nMMTimerID = NULL;

	CSOLO6010::ReleaseTimerRes ();
	// MM Timer Destroy[E]

	if (m_pS6010->REC_End () == FALSE)
	{
		AddReport ("ERROR: m_pS6010->REC_End () Fail");
		return FALSE;
	}

	while (1)
	{
		if (m_pS6010->REC_MP4_IsBufferFlushed () == TRUE) break;
		else Sleep (10);
	}
	while (1)
	{
		if (m_pS6010->REC_JPEG_IsBufferFlushed () == TRUE) break;
		else Sleep (10);
	}
	while (1)
	{
		if (m_pS6010->REC_G723_IsBufferFlushed () == TRUE) break;
		else Sleep (10);
	}

	// Get Date[S]
	SYSTEMTIME timeCur;
	GetLocalTime (&timeCur);

	m_dateRecStop.nYear = timeCur.wYear;
	m_dateRecStop.nMonth = (BYTE)timeCur.wMonth;
	m_dateRecStop.nDay = (BYTE)timeCur.wDay;
	m_dateRecStop.nHour = (BYTE)timeCur.wHour;
	m_dateRecStop.nMin = (BYTE)timeCur.wMinute;
	m_dateRecStop.nSec = (BYTE)timeCur.wSecond;
	// Get Date[E]

	AddReport ("Stop Record OK");
	return TRUE;
}

BOOL CRecManager::EndRec ()
{
	if (m_bRec == FALSE)
	{
		return TRUE;
	}

	CloseMP4File (&m_dateRecStop);
	CloseJPEGFile (&m_dateRecStop);
	CloseG723File (&m_dateRecStop);
	
	m_bRec = FALSE;

	AddReport ("End Record OK");
	return TRUE;
}

void CRecManager::OpenMP4File (INFO_DATE *pInfoDate, SETTING_REC *pSetRec, int idxModeRec)
{
	int i, idxS6010;
	DWORD szWritten;
	HEADER_RECORD_FILE hdrMP4File;
	char strMP4FilePath[MAX_PATH];

	idxS6010 = m_pS6010->GENERAL_GetID ();
	switch (idxModeRec)
	{
	case REC_DATA_RCV_BY_FRAME:
		for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
		{
			if (pSetRec->setMP4[i].bSet == TRUE)
			{
				sprintf (strMP4FilePath, STR_MP4_BY_CH_FILE_FORMAT, glBufGInfo[idxS6010].strRecDir, idxS6010, i,
					pInfoDate->nYear, pInfoDate->nMonth, pInfoDate->nDay, pInfoDate->nHour, pInfoDate->nMin, pInfoDate->nSec);
				m_bufHFileMP4[i] = CreateFile (strMP4FilePath, GENERIC_READ |GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);
				if (m_bufHFileMP4[i] == INVALID_HANDLE_VALUE)
				{
					AddReport ("ERROR: MP4 file open fail: %s", strMP4FilePath);
				}
			}
		}
		for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
		{
			if (m_bufHFileMP4[i] != INVALID_HANDLE_VALUE)
			{
				memset (&hdrMP4File, 0, sizeof(HEADER_RECORD_FILE));

				memcpy (hdrMP4File.strFileInit, STR_MP4_FILE, SZ_STR_INIT);
				memcpy (&hdrMP4File.dateStart, pInfoDate, sizeof(INFO_DATE));
				memcpy (&hdrMP4File.MP4.bufSetRec[i], &pSetRec->setMP4[i], sizeof(SETTING_MP4_REC));
				hdrMP4File.MP4.bufCRU[i] = m_pS6010->GetNumMP4EncCRU (&pSetRec->setMP4[i]);
				hdrMP4File.MP4.typeVideo = glBufGInfo[idxS6010].typeVideo;
				hdrMP4File.MP4.bExistMotionData = 0;
				hdrMP4File.MP4.bUseIndex = 1;

				WriteFile (m_bufHFileMP4[i], &hdrMP4File, sizeof(HEADER_RECORD_FILE), &szWritten, NULL);
			}
		}
		break;
	case REC_DATA_RCV_BY_RAW_DATA:
		sprintf (strMP4FilePath, STR_MP4_RAW_FILE_FORMAT, glBufGInfo[idxS6010].strRecDir, idxS6010,
			pInfoDate->nYear, pInfoDate->nMonth, pInfoDate->nDay, pInfoDate->nHour, pInfoDate->nMin, pInfoDate->nSec);
		m_bufHFileMP4[0] = CreateFile (strMP4FilePath, GENERIC_READ |GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);

		if (m_bufHFileMP4[0] == INVALID_HANDLE_VALUE)
		{
			AddReport ("ERROR: MP4 file open fail: %s", strMP4FilePath);
		}
		else
		{
			memset (&hdrMP4File, 0, sizeof(HEADER_RECORD_FILE));

			memcpy (hdrMP4File.strFileInit, STR_MP4_FILE, SZ_STR_INIT);
			memcpy (&hdrMP4File.dateStart, pInfoDate, sizeof(INFO_DATE));

			memcpy (&hdrMP4File.MP4.bufSetRec[0], &pSetRec->setMP4[0], sizeof(SETTING_MP4_REC) *NUM_VID_TOTAL_ENC_CH);
			for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
			{
				hdrMP4File.MP4.bufCRU[i] = m_pS6010->GetNumMP4EncCRU (&pSetRec->setMP4[i]);
			}
			hdrMP4File.MP4.typeVideo = glBufGInfo[idxS6010].typeVideo;
			hdrMP4File.MP4.bExistMotionData = 0;
			hdrMP4File.MP4.bUseIndex = 1;
			WriteFile (m_bufHFileMP4[0], &hdrMP4File, sizeof(HEADER_RECORD_FILE), &szWritten, NULL);
		}
		break;
	}
}

void CRecManager::CloseMP4File (INFO_DATE *pInfoDate)
{
	int i, szBufIndex, posBufIndex;
	DWORD szRead, szWritten, posHigh, szFrame;
	ITEM_VID_REC_FILE_INDEX *bufIndex;
	HEADER_RECORD_FILE hdrMP4File;
	LARGE_INTEGER liPosFileHandle;
	HEADER_MP4_FRM hdrMP4Frm;
	__int64 posCurFrame;

	szBufIndex = 256 *1024;
	posBufIndex = 0;
	bufIndex = new ITEM_VID_REC_FILE_INDEX[szBufIndex];
	EnterCriticalSection (&m_staCrtFileAcc);

	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		if (m_bufHFileMP4[i] != INVALID_HANDLE_VALUE)
		{
			// Read file header[S]
			liPosFileHandle.QuadPart = 0;
			SetFilePointer (m_bufHFileMP4[i], liPosFileHandle.LowPart, &liPosFileHandle.HighPart, FILE_BEGIN);
			ReadFile (m_bufHFileMP4[i], &hdrMP4File, sizeof(HEADER_RECORD_FILE), &szRead, NULL);
			// Read file header[E]
			
			// Set date, index buffer position[S]
			memcpy (&hdrMP4File.dateEnd, pInfoDate, sizeof(INFO_DATE));

			liPosFileHandle.LowPart = GetFileSize (m_bufHFileMP4[i], &posHigh);
			liPosFileHandle.HighPart = posHigh;
			hdrMP4File.MP4.posIndex = liPosFileHandle.QuadPart;
			// Set date, index buffer position[E]

			hdrMP4File.MP4.numFrm = 0;
			posCurFrame = szRead;
			while (1)
			{
				// Read frame header[S]
				ReadFile (m_bufHFileMP4[i], &hdrMP4Frm, sizeof(HEADER_MP4_FRM), &szRead, NULL);
				
				if (szRead < sizeof(HEADER_MP4_FRM))
				{
					posCurFrame = hdrMP4File.MP4.posIndex;
					goto WRITE_INDEX;
				}

				szFrame = GET_MP4_CODE_SZ(hdrMP4Frm);
				// Read frame header[E]

				// Set index data[S]
				bufIndex[posBufIndex].idxCh = BYTE(GET_CH(hdrMP4Frm));
				bufIndex[posBufIndex].isKeyFrm = IS_KEY_FRM(hdrMP4Frm);
				bufIndex[posBufIndex].timeSec = GET_SEC(hdrMP4Frm);
				bufIndex[posBufIndex].timeUSec = GET_USEC(hdrMP4Frm);
				bufIndex[posBufIndex].posFrm = posCurFrame;

				hdrMP4File.MP4.numFrm++;
				posBufIndex++;
				// Set index data[E]

				posCurFrame += sizeof(HEADER_MP4_FRM) +szFrame;
				
				// Write index data[S]
WRITE_INDEX:
				if (posBufIndex >= szBufIndex ||
					posCurFrame >= hdrMP4File.MP4.posIndex)
				{
					liPosFileHandle.QuadPart = 0;
					SetFilePointer (m_bufHFileMP4[i], liPosFileHandle.LowPart, &liPosFileHandle.HighPart, FILE_END);
					WriteFile (m_bufHFileMP4[i], bufIndex, sizeof(ITEM_VID_REC_FILE_INDEX) *posBufIndex, &szWritten, NULL);

					posBufIndex = 0;
				}
				// Write index data[E]

				if (posCurFrame >= hdrMP4File.MP4.posIndex)
				{
					break;
				}
				
				// Update file position[S]
				liPosFileHandle.QuadPart = posCurFrame;
				SetFilePointer (m_bufHFileMP4[i], liPosFileHandle.LowPart, &liPosFileHandle.HighPart, FILE_BEGIN);
				// Update file position[E]
			}

			// Write file header[S]
			liPosFileHandle.QuadPart = 0;
			SetFilePointer (m_bufHFileMP4[i], liPosFileHandle.LowPart, &liPosFileHandle.HighPart, FILE_BEGIN);
			WriteFile (m_bufHFileMP4[i], &hdrMP4File, sizeof(HEADER_RECORD_FILE), &szWritten, NULL);
			// Write file header[E]

			// file close[S]
			CloseHandle (m_bufHFileMP4[i]);
			m_bufHFileMP4[i] = INVALID_HANDLE_VALUE;
			// file close[E]
		}
	}

	LeaveCriticalSection (&m_staCrtFileAcc);
	delete[] bufIndex;
}

void CRecManager::OpenJPEGFile (INFO_DATE *pInfoDate, SETTING_REC *pSetRec, int idxModeRec)
{
	int i, idxS6010;
	DWORD szWritten;
	HEADER_RECORD_FILE hdrJPEGFile;
	char strJPEGFilePath[MAX_PATH];

	idxS6010 = m_pS6010->GENERAL_GetID ();
	switch (idxModeRec)
	{
	case REC_DATA_RCV_BY_FRAME:
		break;
	case REC_DATA_RCV_BY_RAW_DATA:
		sprintf (strJPEGFilePath, STR_JPEG_RAW_FILE_FORMAT, glBufGInfo[idxS6010].strRecDir, idxS6010,
			pInfoDate->nYear, pInfoDate->nMonth, pInfoDate->nDay, pInfoDate->nHour, pInfoDate->nMin, pInfoDate->nSec);
		m_bufHFileJPEG[0] = CreateFile (strJPEGFilePath, GENERIC_READ |GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);

		if (m_bufHFileJPEG[0] == INVALID_HANDLE_VALUE)
		{
			AddReport ("ERROR: JPEG file open fail: %s", strJPEGFilePath);
		}
		else
		{
			memset (&hdrJPEGFile, 0, sizeof(HEADER_RECORD_FILE));

			memcpy (hdrJPEGFile.strFileInit, STR_JPEG_FILE, SZ_STR_INIT);
			memcpy (&hdrJPEGFile.dateStart, pInfoDate, sizeof(INFO_DATE));

			memcpy (&hdrJPEGFile.JPEG.bufSetRec[0], &pSetRec->setJPEG[0], sizeof(SETTING_JPEG_REC) *NUM_VID_TOTAL_ENC_CH);
			for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
			{
				hdrJPEGFile.JPEG.bufIdxImgSz[i] = pSetRec->setMP4[i].idxImgSz;
			}
			hdrJPEGFile.JPEG.typeVideo = glBufGInfo[idxS6010].typeVideo;
			WriteFile (m_bufHFileJPEG[0], &hdrJPEGFile, sizeof(HEADER_RECORD_FILE), &szWritten, NULL);
		}
		break;
	}
}

void CRecManager::CloseJPEGFile (INFO_DATE *pInfoDate)
{
	int i, szBufIndex, posBufIndex;
	DWORD szRead, szWritten, posHigh, szFrame;
	ITEM_VID_REC_FILE_INDEX *bufIndex;
	HEADER_RECORD_FILE hdrJPEGFile;
	LARGE_INTEGER liPosFileHandle;
	HEADER_JPEG_FRM hdrJPEGFrm;
	__int64 posCurFrame;

	szBufIndex = 256 *1024;
	posBufIndex = 0;
	bufIndex = new ITEM_VID_REC_FILE_INDEX[szBufIndex];
	EnterCriticalSection (&m_staCrtFileAcc);

	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		if (m_bufHFileJPEG[i] != INVALID_HANDLE_VALUE)
		{
			// Read file header[S]
			liPosFileHandle.QuadPart = 0;
			SetFilePointer (m_bufHFileJPEG[i], liPosFileHandle.LowPart, &liPosFileHandle.HighPart, FILE_BEGIN);
			ReadFile (m_bufHFileJPEG[i], &hdrJPEGFile, sizeof(HEADER_RECORD_FILE), &szRead, NULL);
			// Read file header[E]

			
			// Set date, index buffer position[S]
			memcpy (&hdrJPEGFile.dateEnd, pInfoDate, sizeof(INFO_DATE));

			liPosFileHandle.LowPart = GetFileSize (m_bufHFileJPEG[i], &posHigh);
			liPosFileHandle.HighPart = posHigh;
			hdrJPEGFile.JPEG.posIndex = liPosFileHandle.QuadPart;
			// Set date, index buffer position[E]

			hdrJPEGFile.JPEG.numFrm = 0;
			posCurFrame = szRead;
			while (1)
			{
				// Read frame header[S]
				ReadFile (m_bufHFileJPEG[i], &hdrJPEGFrm, sizeof(HEADER_JPEG_FRM), &szRead, NULL);
				
				if (szRead < sizeof(HEADER_JPEG_FRM))
				{
					posCurFrame = hdrJPEGFile.JPEG.posIndex;
					goto WRITE_INDEX;
				}

				szFrame = hdrJPEGFrm.szJPEG;
				// Read frame header[E]

				// Set index data[S]
				bufIndex[posBufIndex].idxCh = BYTE(hdrJPEGFrm.idxCh);
				bufIndex[posBufIndex].isKeyFrm = 1;
				bufIndex[posBufIndex].timeSec = hdrJPEGFrm.timeSec;
				bufIndex[posBufIndex].timeUSec = hdrJPEGFrm.timeUSec;
				bufIndex[posBufIndex].posFrm = posCurFrame;

				hdrJPEGFile.JPEG.numFrm++;
				posBufIndex++;
				// Set index data[E]

				posCurFrame += sizeof(HEADER_JPEG_FRM) +szFrame;
				
				// Write index data[S]
WRITE_INDEX:
				if (posBufIndex >= szBufIndex ||
					posCurFrame >= hdrJPEGFile.JPEG.posIndex)
				{
					liPosFileHandle.QuadPart = 0;
					SetFilePointer (m_bufHFileJPEG[i], liPosFileHandle.LowPart, &liPosFileHandle.HighPart, FILE_END);
					WriteFile (m_bufHFileJPEG[i], bufIndex, sizeof(ITEM_VID_REC_FILE_INDEX) *posBufIndex, &szWritten, NULL);

					posBufIndex = 0;
				}
				// Write index data[E]

				if (posCurFrame >= hdrJPEGFile.JPEG.posIndex)
				{
					break;
				}
				
				// Update file position[S]
				liPosFileHandle.QuadPart = posCurFrame;
				SetFilePointer (m_bufHFileJPEG[i], liPosFileHandle.LowPart, &liPosFileHandle.HighPart, FILE_BEGIN);
				// Update file position[E]
			}

			// Write file header[S]
			liPosFileHandle.QuadPart = 0;
			SetFilePointer (m_bufHFileJPEG[i], liPosFileHandle.LowPart, &liPosFileHandle.HighPart, FILE_BEGIN);
			WriteFile (m_bufHFileJPEG[i], &hdrJPEGFile, sizeof(HEADER_RECORD_FILE), &szWritten, NULL);
			// Write file header[E]

			// file close[S]
			CloseHandle (m_bufHFileJPEG[i]);
			m_bufHFileJPEG[i] = INVALID_HANDLE_VALUE;
			// file close[E]
		}
	}

	LeaveCriticalSection (&m_staCrtFileAcc);
	delete[] bufIndex;
}

void CRecManager::OpenG723File (INFO_DATE *pInfoDate, SETTING_REC *pSetRec, int idxModeRec)
{
	int i, idxS6010;
	DWORD szWritten;
	HEADER_RECORD_FILE hdrG723File;
	char strG723FilePath[MAX_PATH];

	idxS6010 = m_pS6010->GENERAL_GetID ();

	switch (m_modeRecG723)
	{
	case REC_DATA_RCV_BY_FRAME:
		for (i=0; i<NUM_G723_CH; i++)
		{
			if (pSetRec->setG723.bufBEnc[i] == TRUE)
			{
				sprintf (strG723FilePath, STR_G723_BY_CH_FILE_FORMAT, glBufGInfo[idxS6010].strRecDir, idxS6010, i,
					pInfoDate->nYear, pInfoDate->nMonth, pInfoDate->nDay, pInfoDate->nHour, pInfoDate->nMin, pInfoDate->nSec);
				m_bufHFileG723[i] = CreateFile (strG723FilePath, GENERIC_READ |GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);
				if (m_bufHFileG723[i] == INVALID_HANDLE_VALUE)
				{
					AddReport ("ERROR: G723 file open fail: %s", strG723FilePath);
				}
			}
		}
		for (i=0; i<NUM_G723_CH; i++)
		{
			if (m_bufHFileG723[i] != INVALID_HANDLE_VALUE)
			{
				memset (&hdrG723File, 0, sizeof(HEADER_RECORD_FILE));

				memcpy (hdrG723File.strFileInit, STR_G723_FILE, SZ_STR_INIT);
				memcpy (&hdrG723File.dateStart, pInfoDate, sizeof(INFO_DATE));

				hdrG723File.G723.setRec.idxSampRate = pSetRec->setG723.idxSampRate;
				hdrG723File.G723.setRec.bufBEnc[i] = TRUE;
				WriteFile (m_bufHFileG723[i], &hdrG723File, sizeof(HEADER_RECORD_FILE), &szWritten, NULL);
			}
		}
		break;
	case REC_DATA_RCV_BY_RAW_DATA:
		sprintf (strG723FilePath, STR_G723_RAW_FILE_FORMAT, glBufGInfo[idxS6010].strRecDir, idxS6010,
			pInfoDate->nYear, pInfoDate->nMonth, pInfoDate->nDay, pInfoDate->nHour, pInfoDate->nMin, pInfoDate->nSec);
		m_bufHFileG723[0] = CreateFile (strG723FilePath, GENERIC_READ |GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);

		if (m_bufHFileG723[0] == INVALID_HANDLE_VALUE)
		{
			AddReport ("ERROR: G723 file open fail: %s", strG723FilePath);
		}
		else
		{
			memset (&hdrG723File, 0, sizeof(HEADER_RECORD_FILE));

			memcpy (hdrG723File.strFileInit, STR_G723_FILE, SZ_STR_INIT);
			memcpy (&hdrG723File.dateStart, pInfoDate, sizeof(INFO_DATE));

			memcpy (&hdrG723File.G723.setRec, &pSetRec->setG723, sizeof(SETTING_G723_REC));
			WriteFile (m_bufHFileG723[0], &hdrG723File, sizeof(HEADER_RECORD_FILE), &szWritten, NULL);
		}
		break;
	}
}

void CRecManager::CloseG723File (INFO_DATE *pInfoDate)
{
	int i, j, szBufIndex, posBufIndex;
	DWORD szRead, szWritten, posHigh, szFrame, bufTimeFrm[2], posCurFrame;
	ITEM_AUD_REC_FILE_INDEX *bufIndex;
	HEADER_RECORD_FILE hdrG723File;
	LARGE_INTEGER liPosFileHandle;

	szBufIndex = 256 *1024;
	posBufIndex = 0;
	bufIndex = new ITEM_AUD_REC_FILE_INDEX[szBufIndex];

	EnterCriticalSection (&m_staCrtFileAcc);

	for (i=0; i<NUM_G723_CH; i++)
	{
		if (m_bufHFileG723[i] != INVALID_HANDLE_VALUE)
		{
			// Read file header[S]
			liPosFileHandle.QuadPart = 0;
			SetFilePointer (m_bufHFileG723[i], liPosFileHandle.LowPart, &liPosFileHandle.HighPart, FILE_BEGIN);
			ReadFile (m_bufHFileG723[i], &hdrG723File, sizeof(HEADER_RECORD_FILE), &szRead, NULL);

			szFrame = SZ_G723_FRM_HEADER;
			for (j=0; j<NUM_G723_CH; j++)
			{
				if (hdrG723File.G723.setRec.bufBEnc[j] == 1)
				{
					szFrame += SZ_G723_1_FRM;
				}
			}
			// Read file header[E]
			
			// Set date, index buffer position[S]
			memcpy (&hdrG723File.dateEnd, pInfoDate, sizeof(INFO_DATE));

			liPosFileHandle.LowPart = GetFileSize (m_bufHFileG723[i], &posHigh);
			liPosFileHandle.HighPart = posHigh;
			hdrG723File.G723.posIndex = liPosFileHandle.QuadPart;
			// Set date, index buffer position[E]

			hdrG723File.G723.numFrm = 0;
			posCurFrame = szRead;
			while (1)
			{
				// Read frame header[S]
				ReadFile (m_bufHFileG723[i], bufTimeFrm, SZ_G723_FRM_HEADER, &szRead, NULL);
				
				if (szRead < SZ_G723_FRM_HEADER)
				{
					posCurFrame = DWORD(hdrG723File.G723.posIndex);
					goto WRITE_INDEX;
				}
				// Read frame header[E]

				// Set index data[S]
				bufIndex[posBufIndex].timeSec = bufTimeFrm[0];
				bufIndex[posBufIndex].timeUSec = bufTimeFrm[1];
				bufIndex[posBufIndex].posFrm = posCurFrame;

				hdrG723File.G723.numFrm++;
				posBufIndex++;
				// Set index data[E]

				// Check index buffer size[S]
				if (posBufIndex >= szBufIndex)
				{
					ITEM_AUD_REC_FILE_INDEX *pTmpIndex = new ITEM_AUD_REC_FILE_INDEX[szBufIndex *2];
					memcpy (pTmpIndex, bufIndex, szBufIndex);
					delete[] bufIndex;
					bufIndex = pTmpIndex;
					szBufIndex *= 2;
				}
				// Check index buffer size[E]

				posCurFrame += szFrame;
				
				// Write index data[S]
WRITE_INDEX:
				if (posBufIndex >= szBufIndex ||
					posCurFrame >= hdrG723File.G723.posIndex)
				{
					liPosFileHandle.QuadPart = 0;
					SetFilePointer (m_bufHFileG723[i], liPosFileHandle.LowPart, &liPosFileHandle.HighPart, FILE_END);
					WriteFile (m_bufHFileG723[i], bufIndex, sizeof(ITEM_AUD_REC_FILE_INDEX) *posBufIndex, &szWritten, NULL);

					posBufIndex = 0;
				}
				// Write index data[E]

				if (posCurFrame >= hdrG723File.G723.posIndex)
				{
					break;
				}
				
				// Update file position[S]
				liPosFileHandle.QuadPart = posCurFrame;
				SetFilePointer (m_bufHFileG723[i], liPosFileHandle.LowPart, &liPosFileHandle.HighPart, FILE_BEGIN);
				// Update file position[E]
			}

			// Write file header[S]
			liPosFileHandle.QuadPart = 0;
			SetFilePointer (m_bufHFileG723[i], liPosFileHandle.LowPart, &liPosFileHandle.HighPart, FILE_BEGIN);
			WriteFile (m_bufHFileG723[i], &hdrG723File, sizeof(HEADER_RECORD_FILE), &szWritten, NULL);
			// Write file header[E]

			// file close[S]
			CloseHandle (m_bufHFileG723[i]);
			m_bufHFileG723[i] = INVALID_HANDLE_VALUE;
			// file close[E]
		}
	}

	LeaveCriticalSection (&m_staCrtFileAcc);
	delete[] bufIndex;
}

void CRecManager::UpdateEncoderOSD ()
{
	int i;
	char strOSD[(704 /8 +2) *(576 /16)], strTmp[MAX_PATH];
	DWORD lenStrOSD;
	int idxStartLine;
	int idxS6010 = m_pS6010->GENERAL_GetID ();

	SYSTEMTIME timeCur;	
	GetLocalTime (&timeCur);	

	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		if (m_bufBRecCh[i] == TRUE)
		{
			strOSD[0] = '\0';

			if (glBufGInfo[idxS6010].ENC_OSD_bShowCh == TRUE)
			{
				sprintf (strTmp, "%2d ", i +1);
				strcat (strOSD, strTmp);
			}
			if (glBufGInfo[idxS6010].ENC_OSD_bShowDate == TRUE)
			{
				sprintf (strTmp, "%04d-%02d-%02d ", timeCur.wYear, timeCur.wMonth, timeCur.wDay);
				strcat (strOSD, strTmp);
			}
			if (glBufGInfo[idxS6010].ENC_OSD_bShowTime == TRUE)
			{
				sprintf (strTmp, "%02d:%02d:%02d ", timeCur.wHour, timeCur.wMinute, timeCur.wSecond);
				strcat (strOSD, strTmp);
			}

			lenStrOSD = strlen (strOSD);
			idxStartLine = 0;

			m_pS6010->ENC_OSD_Set (i, strOSD, lenStrOSD, idxStartLine);
		}
	}
}
