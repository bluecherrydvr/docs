// HWDecManager.cpp: implementation of the CHWDecManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "solo6010app.h"
#include "HWDecManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define HD_G723_INVALID_CH			-1

UINT WINAPI ThFn_HD_PlayerMain (LPVOID pParam)
{
	CHWDecManager *pPlayer = (CHWDecManager *)pParam;

	while (1)
	{
		WaitForSingleObject (pPlayer->m_bufHEvt[HD_EVT_PLAYER_TIMER], INFINITE);
		ResetEvent (pPlayer->m_bufHEvt[HD_EVT_PLAYER_TIMER]);

		if (pPlayer->m_bufBEndTh[HD_TH_PLAYER_MAIN] == TRUE)
		{
			break;
		}

		if (pPlayer->m_curPlayerMode == HD_MODE_PLAYING)
		{
			pPlayer->OnPlay ();
		}
		else if (pPlayer->m_curPlayerMode == HD_MODE_REVERSE_PLAYING)
		{
			pPlayer->OnReversePlay ();
		}
	}

	pPlayer->m_bufBEndTh[HD_TH_PLAYER_MAIN] = FALSE;

	ExitThread (0);
	return 0;
}

UINT WINAPI ThFn_HD_VideoBufferingExec (LPVOID pParam)
{
	CHWDecManager *pPlayer = (CHWDecManager *)pParam;

	while (1)
	{
		WaitForSingleObject (pPlayer->m_bufHEvt[HD_EVT_VIDEO_BUFFERING_EXEC], INFINITE);

		if (pPlayer->m_bufBEndTh[HD_TH_VIDEO_BUFFERING_EXEC] == TRUE)
		{
			break;
		}

		if (pPlayer->m_curPlayerMode == HD_MODE_PLAYING || pPlayer->m_curPlayerMode == HD_MODE_REVERSE_PLAYING)
		{
			pPlayer->ExecMP4Buffering (pPlayer->m_curPlayerMode);
		}

		ResetEvent (pPlayer->m_bufHEvt[HD_EVT_VIDEO_BUFFERING_EXEC]);
	}

	pPlayer->m_bufBEndTh[HD_TH_VIDEO_BUFFERING_EXEC] = FALSE;

	ExitThread (0);
	return 0;
}

UINT WINAPI ThFn_HD_AudioBufferingExec (LPVOID pParam)
{
	CHWDecManager *pPlayer = (CHWDecManager *)pParam;

	while (1)
	{
		WaitForSingleObject (pPlayer->m_bufHEvt[HD_EVT_AUDIO_BUFFERING_EXEC], INFINITE);

		if (pPlayer->m_bufBEndTh[HD_TH_AUDIO_BUFFERING_EXEC] == TRUE)
		{
			break;
		}

		pPlayer->ExecG723Buffering (FALSE);

		ResetEvent (pPlayer->m_bufHEvt[HD_EVT_AUDIO_BUFFERING_EXEC]);
	}

	pPlayer->m_bufBEndTh[HD_TH_AUDIO_BUFFERING_EXEC] = FALSE;

	ExitThread (0);
	return 0;
}

UINT WINAPI ThFn_HD_HandleReqPause (LPVOID pParam)
{
	CHWDecManager *pPlayer = (CHWDecManager *)pParam;

	while (1)
	{
		WaitForSingleObject (pPlayer->m_bufHEvt[HD_EVT_REQ_PAUSE], INFINITE);
		ResetEvent (pPlayer->m_bufHEvt[HD_EVT_REQ_PAUSE]);

		if (pPlayer->m_bufBEndTh[HD_TH_REQ_PAUSE] == TRUE)
		{
			break;
		}

		pPlayer->Pause ();
	}

	pPlayer->m_bufBEndTh[HD_TH_REQ_PAUSE] = FALSE;

	ExitThread (0);
	return 0;
}

void CALLBACK CbFn_HD_PlayerTimerProc (UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	CHWDecManager *pPlayer = (CHWDecManager *)dwUser;

	if (pPlayer->m_curPlayerMode < HD_MODE_PLAYING)
	{
		return;
	}

	pPlayer->m_timeElapsed_x16 += pPlayer->m_nTimerIncrement_x16;

	SetEvent (pPlayer->m_bufHEvt[HD_EVT_PLAYER_TIMER]);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHWDecManager::CHWDecManager()
{
	m_pS6010 = NULL;
	m_pIMP4DecStat = NULL;

	m_bufVideoIndex = NULL;
	m_curPlayerMode = HD_MODE_IDLE;
	m_nMMTimerID = NULL;
	m_cbProc_UpdatePlayerStat = NULL;

	int i;

	// Init variables[S]
	for (i=0; i<HD_NUM_FILE; i++)
	{
		m_bufHFile[i] = NULL;
	}
	// Init variables[E]

	// Event create[S]
	for (i=0; i<HD_NUM_EVENT; i++)
	{
		m_bufHEvt[i] = CreateEvent (NULL, TRUE, FALSE, NULL);
		if (m_bufHEvt[i] == NULL)
		{
			AddReport ("HW PLAYER: Event create fail");
		}
		ResetEvent (m_bufHEvt[i]);
	}
	// Event create[E]

	// Thread create[S]
	DWORD idThread;

	for (i=0; i<HD_NUM_THREAD; i++)
	{
		m_bufBEndTh[i] = FALSE;
	}
	
	m_bufHThread[HD_TH_PLAYER_MAIN] = CreateThread (NULL, 0,
		(LPTHREAD_START_ROUTINE)ThFn_HD_PlayerMain, this, 0, &idThread);
	m_bufHThread[HD_TH_VIDEO_BUFFERING_EXEC] = CreateThread (NULL, 0,
		(LPTHREAD_START_ROUTINE)ThFn_HD_VideoBufferingExec, this, 0, &idThread);
	m_bufHThread[HD_TH_AUDIO_BUFFERING_EXEC] = CreateThread (NULL, 0,
		(LPTHREAD_START_ROUTINE)ThFn_HD_AudioBufferingExec, this, 0, &idThread);
	m_bufHThread[HD_TH_REQ_PAUSE] = CreateThread (NULL, 0,
		(LPTHREAD_START_ROUTINE)ThFn_HD_HandleReqPause, this, 0, &idThread);
  	for (i=0; i<HD_NUM_THREAD; i++)
	{
		if (m_bufHThread[i] == NULL)
		{
			AddReport ("HW PLAYER: Thread create fail (%d)\n", i);
		}
	}
	// Thread create[E]

	// MP4 decoding[S]
	m_bUseS6010DI = FALSE;
	m_bBufferingOnlyIFrm = FALSE;
	// MP4 decoding[E]
}

CHWDecManager::~CHWDecManager()
{
	int i;

	CloseFile ();
	DestroyMMTimer ();

	// index buffer destroy[S]
	if (m_bufVideoIndex != NULL)
	{
		delete[] m_bufVideoIndex;
	}
	// index buffer destroy[E]

	// stop thread[S]
	for (i=0; i<HD_NUM_THREAD; i++)
	{
		m_bufBEndTh[i] = TRUE;
	}
	for (i=0; i<HD_NUM_EVENT; i++)
	{
		SetEvent (m_bufHEvt[i]);
	}
	for (i=0; i<HD_NUM_EVENT; i++)
	{
		while (m_bufBEndTh[i])
		{
			Sleep (0);
		}
		CloseHandle (m_bufHEvt[i]);
	}
	for (i=0; i<HD_NUM_THREAD; i++)
	{
		WaitForSingleObject (m_bufHThread[i], INFINITE);
		CloseHandle (m_bufHThread[i]);
	}
	// stop thread[E]
}

void CHWDecManager::InitPlayer (CSOLO6010 *pS6010)
{
	InitMMTimer ();
	SetPlaySpeed (HD_PLAY_SPEED_NORMAL);

	m_pS6010 = pS6010;
}

void CHWDecManager::DestroyPlayer ()
{
	CloseFile ();

	m_pS6010 = NULL;
}

BOOL CHWDecManager::OpenFile (LPCTSTR strVidFilePath, LPCTSTR strAudFilePath)
{
	if (m_bufHFile[HD_FILE_VIDEO] != NULL || m_bufHFile[HD_FILE_AUDIO] != NULL)
	{
		CloseFile ();
	}

	DWORD i;

	// File open[S]
	m_bufHFile[HD_FILE_VIDEO] = CreateFile (strVidFilePath,
		GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY |FILE_FLAG_RANDOM_ACCESS, 0);

	if (m_bufHFile[HD_FILE_VIDEO] == INVALID_HANDLE_VALUE)
	{
		AddReport ("HW PLAYER: Video stream file open fail: %s", strVidFilePath);
		m_bufHFile[HD_FILE_VIDEO] = NULL;
	}
	else
	{
		AddReport ("HW PLAYER: Video stream file open succeed: %s", strVidFilePath);
	}

	m_bufHFile[HD_FILE_AUDIO] = CreateFile (strAudFilePath,
		GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY |FILE_FLAG_RANDOM_ACCESS, 0);

	if (m_bufHFile[HD_FILE_AUDIO] == INVALID_HANDLE_VALUE)
	{
		AddReport ("HW PLAYER: Audio stream file open fail: %s", strAudFilePath);
		m_bufHFile[HD_FILE_AUDIO] = NULL;
	}
	else
	{
		AddReport ("HW PLAYER: Audio stream file open succeed: %s", strAudFilePath);
	}

	if (m_bufHFile[HD_FILE_VIDEO] == NULL &&
		m_bufHFile[HD_FILE_AUDIO] == NULL)
	{	// file open fail
		goto ERROR_EXIT;
	}
	// File open[E]

	DWORD numRead, nRetVal;
	char strTmp[MAX_PATH];

	// File check[S]
	for (i=0; i<HD_NUM_FILE; i++)
	{
		if (m_bufHFile[i] == NULL)
		{
			continue;
		}

		ReadFile (m_bufHFile[i], &m_bufFileHeader[i],
			sizeof(HEADER_RECORD_FILE), &numRead, NULL);
		memcpy (strTmp, m_bufFileHeader[i].strFileInit, SZ_STR_INIT);
		strTmp[SZ_STR_INIT] = 0;

		if (i == HD_FILE_VIDEO && strcmp (strTmp, STR_MP4_FILE) == 0)
		{
			nRetVal = OpenFile_MP4 ();
		}
		else if (i == HD_FILE_AUDIO && strcmp (strTmp, STR_G723_FILE) == 0)
		{
			nRetVal = OpenFile_G723 ();
		}
		else
		{
			nRetVal = FALSE;
		}

		if (nRetVal == FALSE)
		{
			break;
		}
	}
	if (i != HD_NUM_FILE)
	{	// invalid file
		goto ERROR_EXIT;
	}
	// File check[E]

	AddReport ("HW PLAYER: File open succeed: [%d, %d]",
		m_bufNumTotFrm[HD_FILE_VIDEO], m_bufNumTotFrm[HD_FILE_AUDIO]);

	m_curPlayerMode = HD_MODE_PAUSE;
	m_pIMP4DecStat = m_pS6010->MP4D_Open ();
	m_pIG723DecStat = m_pS6010->G723D_Open (m_bufFileHeader[HD_FILE_AUDIO].G723.setRec.idxSampRate);
	
	return TRUE;

ERROR_EXIT:
	for (i=0; i<HD_NUM_FILE; i++)
	{
		if (m_bufHFile[i] != NULL)
		{
			CloseHandle (m_bufHFile[i]);
			m_bufHFile[i] = NULL;
		}
	}

	return FALSE;
}
void CHWDecManager::CloseFile ()
{
	Pause ();

	if (m_pS6010 != NULL)
	{
		m_pS6010->MP4D_Close ();
		m_pS6010->G723D_Close ();
	}

	m_curPlayerMode = HD_MODE_IDLE;

	int i;
	
	for (i=0; i<HD_NUM_FILE; i++)
	{
		if (m_bufHFile[i] != NULL)
		{
			CloseHandle (m_bufHFile[i]);
			m_bufHFile[i] = NULL;

			if (i == HD_FILE_VIDEO)
			{
				CloseFile_MP4 ();
			}
			else if (i == HD_FILE_AUDIO)
			{
				CloseFile_G723 ();
			}
		}
	}
}

BOOL CHWDecManager::IsWndStreamCIF ()
{
	if (m_bufHFile[HD_FILE_VIDEO] == NULL ||
		m_pS6010->LIVE_GetSplitMode () != LIVE_SPLIT_MODE_1 ||
		glPCurSelS6010->LIVE_IsWndLiveOrDec (0) == TRUE ||
		glPCurSelS6010->LIVE_GetDecChFromIdxWnd (0) == INVALID_CH_IDX ||
		m_bufFileHeader[HD_FILE_VIDEO].MP4.bufSetRec[glPCurSelS6010->LIVE_GetDecChFromIdxWnd (0)].idxImgSz != IMG_SZ_352X240_352X288)
	{
		return FALSE;
	}
		
	return TRUE;
}

void CHWDecManager::Play ()
{
	if (m_curPlayerMode == HD_MODE_IDLE || m_curPlayerMode == HD_MODE_PLAYING)
	{
		return;
	}
	if (m_curPlayerMode == HD_MODE_REVERSE_PLAYING)
	{
		Pause ();
	}

	// Check only I frame buffering[S]
	int i;
	DWORD numCRU = 0;
	for (i=0; i<GL_NUM_DISP_FROM_DISP_MODE[m_pS6010->LIVE_GetSplitMode ()]; i++)
	{
		if (m_pS6010->LIVE_GetDecChFromIdxWnd (i) != INVALID_CH_IDX)
		{
			numCRU += m_bufFileHeader[HD_FILE_VIDEO].MP4.bufCRU[m_pS6010->LIVE_GetDecChFromIdxWnd (i)];
		}
	}
	switch (m_idxPS)
	{
	case HD_PLAY_SPEED_DIV16:
		numCRU /= 16;
		break;
	case HD_PLAY_SPEED_DIV8:
		numCRU /= 8;
		break;
	case HD_PLAY_SPEED_DIV4:
		numCRU /= 4;
		break;
	case HD_PLAY_SPEED_DIV2:
		numCRU /= 2;
		break;
	case HD_PLAY_SPEED_X2:
		numCRU *= 2;
		break;
	case HD_PLAY_SPEED_X4:
		numCRU *= 4;
		break;
	case HD_PLAY_SPEED_X8:
		numCRU *= 8;
		break;
	case HD_PLAY_SPEED_X16:
		numCRU *= 16;
		break;
	}
	if (numCRU > MAX_NUM_CRU_MP4_ENC)
	{
		m_bBufferingOnlyIFrm = TRUE;
	}
	else
	{
		m_bBufferingOnlyIFrm = FALSE;
	}
	// Check only I frame buffering[E]

	ExecMP4Buffering (HD_MODE_PLAYING);
	ExecG723Buffering (TRUE);

	// Reset reference time[S]
	m_timeRef = m_bufVideoIndex[m_pIMP4DecStat->idxCurFrm].timeMSec;
	m_timeElapsed_x16 = 0;
	// Reset reference time[E]

	if (m_bufHFile[HD_FILE_AUDIO] != NULL)
	{
		m_pS6010->G723D_Play ();
	}

	m_curPlayerMode = HD_MODE_PLAYING;
}

void CHWDecManager::Pause (BOOL bSetFrmPos)
{
	if (m_curPlayerMode == HD_MODE_IDLE ||
		m_curPlayerMode == HD_MODE_PAUSE)
	{
		return;
	}

	m_curPlayerMode = HD_MODE_PAUSE;

	if (m_bufHFile[HD_FILE_VIDEO] != NULL)
	{
		m_pS6010->MP4D_PauseDecoding ();	// Pause SOLO6010 decoding
	}
	if (m_bufHFile[HD_FILE_AUDIO] != NULL)
	{
		m_pS6010->G723D_Stop ();
	}

	// Thread pause check[S]
	while (1)
	{
		if (WaitForSingleObject (m_bufHEvt[HD_EVT_VIDEO_BUFFERING_EXEC], 0) == WAIT_TIMEOUT)
			break;
	}
	while (1)
	{
		if (WaitForSingleObject (m_bufHEvt[HD_EVT_AUDIO_BUFFERING_EXEC], 0) == WAIT_TIMEOUT)
			break;
	}
	while (1)
	{
		if (WaitForSingleObject (m_bufHEvt[HD_EVT_PLAYER_TIMER], 0) == WAIT_TIMEOUT)
			break;
	}
	// Thread pause check[E]

	if (bSetFrmPos == TRUE)
	{
		SetFramePos (GetFramePos ());
	}
}

void CHWDecManager::Stop ()
{
	if (m_curPlayerMode == HD_MODE_IDLE)
	{
		return;
	}

	SetFramePos (0);
}

void CHWDecManager::SetFramePos (DWORD posFrm)
{
	Pause (FALSE);

	// video[S]
	if (m_bufHFile[HD_FILE_VIDEO] != NULL)
	{
		SetFramePos_MP4 (posFrm);
	}
	// video[E]

	// audio[S]
	if (m_bufHFile[HD_FILE_AUDIO] != NULL)
	{
		SetFramePos_G723 (posFrm);
	}
	// audio[E]

	// update player stat[S]
	if (m_cbProc_UpdatePlayerStat != NULL && m_pIMP4DecStat != NULL)
	{
		m_cbProc_UpdatePlayerStat (m_cbCont_UpdatePlayerStat, this, m_pIMP4DecStat->idxCurFrm);
	}
	// update player stat[E]
}

BOOL CHWDecManager::OffsetFramePos (int idxWnd, int nOffset)
{
	int idxCh = m_pS6010->LIVE_GetDecChFromIdxWnd (idxWnd);
	
	if (m_pS6010->LIVE_GetIdxWndFromDecCh (idxCh) == INVALID_WND_IDX || nOffset == 0)
	{
		return FALSE;
	}

	int i;

	// get current pos[S]
	for (i=m_pIMP4DecStat->idxCurFrm; i>=0; i--)
	{
		if (m_bufVideoIndex[i].idxCh == idxCh)
		{
			break;
		}
	}
	if (i < 0)
	{
		for (i=m_pIMP4DecStat->idxCurFrm; i<int(m_bufNumTotFrm[HD_FILE_VIDEO]); i++)
		{
			if (m_bufVideoIndex[i].idxCh == idxCh)
			{
				break;
			}
		}
	}
	if (i >= int(m_bufNumTotFrm[HD_FILE_VIDEO]))
	{
		return FALSE;
	}
	// get current pos[S]

	int curStep, idxLastPos;

	// get offset pos[S]
	curStep = 0;
	if (nOffset > 0)
	{
		i++;
		for (; i<int(m_bufNumTotFrm[HD_FILE_VIDEO]); i++)
		{
			if (m_bufVideoIndex[i].idxCh == idxCh)
			{
				idxLastPos = i;
				curStep++;
				if (curStep >= nOffset)
				{
					break;
				}
			}
		}
		if (i >= int(m_bufNumTotFrm[HD_FILE_VIDEO]))
		{
			if (curStep == 0)
			{
				return FALSE;
			}
			else
			{
				i = idxLastPos;
			}
		}
	}
	else
	{
		nOffset = -nOffset;
		i--;
		for (; i>=0; i--)
		{
			if (m_bufVideoIndex[i].idxCh == idxCh)
			{
				idxLastPos = i;
				curStep++;
				if (curStep >= nOffset)
				{
					break;
				}
			}
		}
		if (i < 0)
		{
			if (curStep == 0)
			{
				return FALSE;
			}
			else
			{
				i = idxLastPos;
			}
		}
	}
	// get offset pos[E]

	SetFramePos (i);

	return TRUE;
}

void CHWDecManager::ReversePlay ()
{
	if (m_curPlayerMode == HD_MODE_IDLE || m_curPlayerMode == HD_MODE_REVERSE_PLAYING)
	{
		return;
	}
	if (m_curPlayerMode == HD_MODE_PLAYING)
	{
		Pause ();
	}

	ExecMP4Buffering (HD_MODE_REVERSE_PLAYING);
	ExecG723Buffering (TRUE);

	// Reset reference time[S]
	m_timeRef = m_bufVideoIndex[m_pIMP4DecStat->idxCurFrm].timeMSec;
	m_timeElapsed_x16 = 0;
	// Reset reference time[E]

	m_curPlayerMode = HD_MODE_REVERSE_PLAYING;
}

BOOL CHWDecManager::SetPlaySpeed (int idxPlaySpeed)
{
	if (idxPlaySpeed < HD_PLAY_SPEED_DIV16 ||
		idxPlaySpeed > HD_PLAY_SPEED_X16)
	{
		return FALSE;
	}

	const int BUF_PS_TO_TIMER_RES_MUL[HD_NUM_PLAY_SPEED] = {
		1, 2, 4, 8, 16, 32, 64, 128, 256
	};

	int idxOldPlayerMode = m_curPlayerMode;
	if (m_curPlayerMode == HD_MODE_PLAYING || m_curPlayerMode == HD_MODE_REVERSE_PLAYING)
	{
		Pause ();
	}

	m_idxPS = idxPlaySpeed;
	m_nTimerIncrement_x16 = m_nMMTimerRes *BUF_PS_TO_TIMER_RES_MUL[idxPlaySpeed];

	if (idxOldPlayerMode == HD_MODE_PLAYING)
	{
		Play ();
	}
	else if (idxOldPlayerMode == HD_MODE_REVERSE_PLAYING)
	{
		ReversePlay ();
	}

	return TRUE;
}

void CHWDecManager::RegisterCB_UpdatePlayerStat (HD_UPDATE_PLAYER_STAT_PROC cbFnUpdatePlayerStat, void *pContext)
{
	m_cbProc_UpdatePlayerStat = cbFnUpdatePlayerStat;
	m_cbCont_UpdatePlayerStat = pContext;
}

void CHWDecManager::UnregisterCB_UpdatePlayerStat ()
{
	m_cbProc_UpdatePlayerStat = NULL;
}

void CHWDecManager::EnableS6010DeinterlaceFilter (BOOL bEnable)
{
	m_bUseS6010DI = bEnable;
}

BOOL CHWDecManager::OpenFile_MP4 ()
{
	DWORD i, numRead, idxCh;
	LARGE_INTEGER liPosFile;

	m_bufNumTotFrm[HD_FILE_VIDEO] = m_bufFileHeader[HD_FILE_VIDEO].MP4.numFrm;
	if (m_bufNumTotFrm[HD_FILE_VIDEO] == 0)
	{
		AddReport ("HW PLAYER: OpenFile_MP4(): Number of MP4 frame is 0.");
		return FALSE;
	}

	m_posVidFileIndex = m_bufFileHeader[HD_FILE_VIDEO].MP4.posIndex;

	// Make index[S]
	if (m_bufVideoIndex != NULL)
	{
		delete[] m_bufVideoIndex;
	}
	ITEM_VID_REC_FILE_INDEX *bufTmpFileIndex = new ITEM_VID_REC_FILE_INDEX[m_bufNumTotFrm[HD_FILE_VIDEO]];
	m_bufVideoIndex = new ITEM_HD_VIDEO_INDEX[m_bufNumTotFrm[HD_FILE_VIDEO] +1];
	if (bufTmpFileIndex == NULL || m_bufVideoIndex == NULL)
	{
		AddReport ("HW PLAYER: OpenFile_MP4(): Index buffer allocation fail");
		return FALSE;
	}

	liPosFile.QuadPart = m_posVidFileIndex;
	SetFilePointer (m_bufHFile[HD_FILE_VIDEO], liPosFile.LowPart, &liPosFile.HighPart, FILE_BEGIN);
	ReadFile (m_bufHFile[HD_FILE_VIDEO], bufTmpFileIndex,
		sizeof(ITEM_VID_REC_FILE_INDEX) *m_bufNumTotFrm[HD_FILE_VIDEO], &numRead, NULL);

	memset (m_bufNumVidFrm, 0, sizeof(DWORD) *NUM_VID_TOTAL_ENC_CH);

	for (i=0; i<m_bufNumTotFrm[HD_FILE_VIDEO]; i++)
	{
		idxCh = bufTmpFileIndex[i].idxCh;
		m_bufVideoIndex[i].idxCh = (BYTE)idxCh;
		m_bufVideoIndex[i].isKeyFrm = bufTmpFileIndex[i].isKeyFrm;
		m_bufVideoIndex[i].timeMSec = bufTmpFileIndex[i].timeSec *1000 +bufTmpFileIndex[i].timeUSec /1000;
		
		m_bufVideoIndex[i].posFrm = bufTmpFileIndex[i].posFrm;

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

BOOL CHWDecManager::OpenFile_G723 ()
{
	int i;

	m_posAudFileIndex = m_bufFileHeader[HD_FILE_AUDIO].G723.posIndex;

	m_szG723LineBuf = 0;
	for (i=0; i<NUM_G723_CH; i++)
	{
		m_bufOffsetG723ChDataInLineBuf[i] = HD_G723_INVALID_CH;
		if (m_bufFileHeader[HD_FILE_AUDIO].G723.setRec.bufBEnc[i] == TRUE)
		{
			m_bufOffsetG723ChDataInLineBuf[i] = SZ_G723_FRM_HEADER +m_szG723LineBuf *SZ_G723_1_FRM;

			m_szG723LineBuf++;
		}
	}
	m_szG723LineBuf *= SZ_G723_1_FRM;
	m_szG723LineBuf += SZ_G723_FRM_HEADER;
	if (m_szG723LineBuf == SZ_G723_FRM_HEADER)
	{
		return FALSE;
	}
	m_bufNumTotFrm[HD_FILE_AUDIO] = DWORD((m_bufFileHeader[HD_FILE_AUDIO].G723.posIndex -sizeof(HEADER_RECORD_FILE)) /m_szG723LineBuf);

	// Make index[S]
	// Make index[S]

	return TRUE;
}

void CHWDecManager::CloseFile_MP4 ()
{
}

void CHWDecManager::CloseFile_G723 ()
{
}

BOOL CHWDecManager::InitMMTimer ()
{
	if (m_nMMTimerID != NULL)
	{
		DestroyMMTimer ();
	}

	m_nMMTimerRes = CSOLO6010::AllocateTimerRes ();

	m_nMMTimerID = timeSetEvent (m_nMMTimerRes, m_nMMTimerRes,
		CbFn_HD_PlayerTimerProc, (DWORD)this, TIME_PERIODIC);

	return TRUE;
}

void CHWDecManager::DestroyMMTimer ()
{
	if (m_nMMTimerID == NULL)
	{
		return;
	}

	timeKillEvent (m_nMMTimerID);
	m_nMMTimerID = NULL;

	CSOLO6010::ReleaseTimerRes ();
}

void CHWDecManager::SetFramePos_MP4 (DWORD posFrm)
{
	int i, j, posDecStart, posDecEnd;

	posDecStart = 0;
	posDecEnd = 0;

	if (posFrm >= m_bufNumTotFrm[HD_FILE_VIDEO])
	{
		posFrm = m_bufNumTotFrm[HD_FILE_VIDEO] -1;
	}

	// get posDecEnd[S]
	for (i=posFrm; i>=0; i--)
	{
		if (m_pS6010->LIVE_GetIdxWndFromDecCh (m_bufVideoIndex[i].idxCh) != INVALID_WND_IDX)
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
		if (m_pS6010->LIVE_GetIdxWndFromDecCh (i) != INVALID_WND_IDX)
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
			m_pS6010->LIVE_GetIdxWndFromDecCh (m_bufVideoIndex[i].idxCh) != INVALID_WND_IDX &&
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
		posDecEnd = m_bufNumTotFrm[HD_FILE_VIDEO] -1;

		// re-get posDecEnd[S]
		idxBufCurDispFrm = 0;
		memset (bufTmpBDec, 0, sizeof(BOOL) *NUM_VID_TOTAL_ENC_CH);
		for (i=0; i<int(m_bufNumTotFrm[HD_FILE_VIDEO]); i++)
		{
			if (bufTmpBDec[m_bufVideoIndex[i].idxCh] == FALSE &&
				m_pS6010->LIVE_GetIdxWndFromDecCh (m_bufVideoIndex[i].idxCh) != INVALID_WND_IDX &&
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

	int posTmpDecStart, posTmpDecEnd, numDecFrm;
	DWORD numRead, *bufPosFrm, *bufIdxFrm, posBufferingStart, posBufferingEnd, szBufBuffering, szBufQ, szFrm;
	LARGE_INTEGER liPosFile;
	BYTE *pBufBuffering, *bufIdxDecMode, *bufBFrmIntp;
	DWORD bufIdxUpdateFrm[NUM_VID_TOTAL_ENC_CH], numUpdateFrm;

	numUpdateFrm = 0;
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		if (m_pS6010->LIVE_GetIdxWndFromDecCh (i) == INVALID_WND_IDX)
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

		// get frame data & decoding[S]
		bufPosFrm = new DWORD[posTmpDecEnd -posTmpDecStart +1];
		bufIdxFrm = new DWORD[posTmpDecEnd -posTmpDecStart +1];
		bufIdxDecMode = new BYTE[posTmpDecEnd -posTmpDecStart +1];
		bufBFrmIntp = new BYTE[posTmpDecEnd -posTmpDecStart +1];
		pBufBuffering = m_pS6010->MP4D_GetFrameBuffer (posBufferingStart, posBufferingEnd, szBufBuffering, szBufQ);

		// Save last frame (for update)[S]
		for (j=posTmpDecEnd; j>=posTmpDecStart; j--)
		{
			if (m_bufVideoIndex[j].idxCh == i)
			{
				bufIdxUpdateFrm[numUpdateFrm] = j;
				posTmpDecEnd = j -1;
				break;
			}
		}
		numUpdateFrm++;
		// Save last frame (for update)[E]

		for (j=posTmpDecStart, numDecFrm=0; j<=posTmpDecEnd; j++)
		{
			if (m_bufVideoIndex[j].idxCh != i)
			{
				continue;
			}

			szFrm = DWORD(m_bufVideoIndex[j +1].posFrm -m_bufVideoIndex[j].posFrm);

			// copy frame data to buffering buffer[S]
			liPosFile.QuadPart = m_bufVideoIndex[j].posFrm;
			SetFilePointer (m_bufHFile[HD_FILE_VIDEO], liPosFile.LowPart, &liPosFile.HighPart, FILE_BEGIN);
			if (posBufferingStart +szFrm >= szBufBuffering)
			{
				posBufferingStart = 0;
			}
			bufPosFrm[numDecFrm] = posBufferingStart;
			bufIdxFrm[numDecFrm] = j;
			bufIdxDecMode[numDecFrm] = MP4D_IDX_DEC_ONLY;
			bufBFrmIntp[numDecFrm] = 0;
			ReadFile (m_bufHFile[HD_FILE_VIDEO], &pBufBuffering[posBufferingStart], szFrm, &numRead, NULL);
			// copy frame data to buffering buffer[E]

			posBufferingStart += szFrm;
			numDecFrm++;
		}
		if (numDecFrm > 0)
		{
			numDecFrm = m_pS6010->MP4D_UpdateFrameBufferStat (numDecFrm, bufPosFrm, bufIdxFrm, bufIdxFrm, bufIdxDecMode, bufBFrmIntp);
			m_pS6010->MP4D_DecodeNFrm (numDecFrm);

			while (m_pIMP4DecStat->idxQDecOK != m_pIMP4DecStat->idxQDecNeed)
			{
				Sleep (0);
			}
		}

		delete[] bufPosFrm;
		delete[] bufIdxFrm;
		delete[] bufIdxDecMode;
		delete[] bufBFrmIntp;
		// get frame data & decoding[E]
	}

	// Update last frame[S]
	// For eliminating update interval, all update frame (last frame) is updated from here
	bufPosFrm = new DWORD[numUpdateFrm];
	bufIdxFrm = new DWORD[numUpdateFrm];
	bufIdxDecMode = new BYTE[numUpdateFrm];
	bufBFrmIntp = new BYTE[numUpdateFrm];
	pBufBuffering = m_pS6010->MP4D_GetFrameBuffer (posBufferingStart, posBufferingEnd, szBufBuffering, szBufQ);

	for (i=0; i<int(numUpdateFrm); i++)
	{
		szFrm = DWORD(m_bufVideoIndex[bufIdxUpdateFrm[i] +1].posFrm -m_bufVideoIndex[bufIdxUpdateFrm[i]].posFrm);

		// copy frame data to buffering buffer[S]
		liPosFile.QuadPart = m_bufVideoIndex[bufIdxUpdateFrm[i]].posFrm;
		SetFilePointer (m_bufHFile[HD_FILE_VIDEO], liPosFile.LowPart, &liPosFile.HighPart, FILE_BEGIN);
		if (posBufferingStart +szFrm >= szBufBuffering)
		{
			posBufferingStart = 0;
		}
		bufPosFrm[i] = posBufferingStart;
		bufIdxFrm[i] = bufIdxUpdateFrm[i];
		bufIdxDecMode[i] = MP4D_IDX_DEC_AND_DISP;
		if (IS_INTERLACE_FROM_INDEX_IMG_SZ[m_bufFileHeader[HD_FILE_VIDEO].MP4.bufSetRec[m_bufVideoIndex[bufIdxUpdateFrm[i]].idxCh].idxImgSz] == 1)
		{
			bufBFrmIntp[i] = 1;
		}
		else
		{
			bufBFrmIntp[i] = 0;
		}
		ReadFile (m_bufHFile[HD_FILE_VIDEO], &pBufBuffering[posBufferingStart], szFrm, &numRead, NULL);
		// copy frame data to buffering buffer[E]

		posBufferingStart += szFrm;
	}
	if (i > 0)
	{
		numDecFrm = m_pS6010->MP4D_UpdateFrameBufferStat (i, bufPosFrm, bufIdxFrm, bufIdxFrm, bufIdxDecMode, bufBFrmIntp);
		m_pS6010->MP4D_DecodeNFrm (numDecFrm);

		while (m_pIMP4DecStat->idxQDecOK != m_pIMP4DecStat->idxQDecNeed)
		{
			Sleep (0);
		}
	}

	delete[] bufPosFrm;
	delete[] bufIdxFrm;
	delete[] bufIdxDecMode;
	delete[] bufBFrmIntp;
	// Update last frame[E]

	m_pIMP4DecStat->idxBufferingNextFrm = posFrm +1;
	m_pIMP4DecStat->idxCurFrm = posFrm;
}

void CHWDecManager::SetFramePos_G723 (DWORD posFrm)
{
	m_pS6010->G723D_Stop ();

	DWORD timeTotal, timeCur;

	timeTotal = m_bufVideoIndex[GetTotalVideoFrameNum () -1].timeMSec -m_bufVideoIndex[0].timeMSec;
	timeCur = GetVideoElapsedTime (posFrm);
	
	m_pIG723DecStat->idxCurFrm = DWORD(double(timeCur) *m_bufNumTotFrm[HD_FILE_AUDIO] /timeTotal);

	SetFilePointer (m_bufHFile[HD_FILE_AUDIO],
		sizeof(HEADER_RECORD_FILE) +m_pIG723DecStat->idxCurFrm *m_szG723LineBuf,
		NULL, FILE_BEGIN);
}

void CHWDecManager::ExecMP4Buffering (int idxModePlay)
{
	if (m_bufHFile[HD_FILE_VIDEO] == NULL)
	{
		return;
	}

	int idxFrm;
	DWORD idxCh, numRead, posBufferingStart, posBufferingEnd, szBufBuffering, numBuffFrm, maxNumBuffFrm, szFrm;
	static DWORD bufPosFrm[SZ_MP4_DEC_QUEUE_ITEM], bufIdxFrm[SZ_MP4_DEC_QUEUE_ITEM], bufTimeMS[SZ_MP4_DEC_QUEUE_ITEM];
	BYTE *pBufBuffering;
	static BYTE bufIdxDecMode[SZ_MP4_DEC_QUEUE_ITEM], bufBFrmIntp[SZ_MP4_DEC_QUEUE_ITEM];
	LARGE_INTEGER liPosFile;
	
	pBufBuffering = m_pS6010->MP4D_GetFrameBuffer (posBufferingStart, posBufferingEnd, szBufBuffering, maxNumBuffFrm);
	if (maxNumBuffFrm <= 0)
	{	// buffering queue fullness
		return;
	}

	idxFrm = m_pIMP4DecStat->idxBufferingNextFrm;

	if (idxModePlay == HD_MODE_PLAYING)
	{
		if (idxFrm >= int(m_bufNumTotFrm[HD_FILE_VIDEO]))
		{	// end of stream
			return;
		}
		
		numBuffFrm = 0;
		while (1)
		{
			if (idxFrm >= int(m_bufNumTotFrm[HD_FILE_VIDEO]))
			{	// end of stream
				break;
			}

			idxCh = m_bufVideoIndex[idxFrm].idxCh;
			szFrm = DWORD(m_bufVideoIndex[idxFrm +1].posFrm -m_bufVideoIndex[idxFrm].posFrm);

			if (m_pS6010->LIVE_GetIdxWndFromDecCh (BYTE(idxCh)) != INVALID_WND_IDX &&
				(m_bBufferingOnlyIFrm == FALSE || m_bufVideoIndex[idxFrm].isKeyFrm == 1))
			{	// need buffering
				if (posBufferingStart +szFrm >= szBufBuffering)
				{
					if (posBufferingStart < posBufferingEnd)
					{	// buffering buffer fullness
						break;
					}
					else
					{
						posBufferingStart = 0;
					}
				}
				if ((posBufferingEnd -posBufferingStart +szBufBuffering) %szBufBuffering <= szFrm)
				{	// buffering buffer fullness
					break;
				}
				if (numBuffFrm >= maxNumBuffFrm)
				{	// buffering queue fullness
					break;
				}

				// copy frame data to buffering buffer[S]
				liPosFile.QuadPart = m_bufVideoIndex[idxFrm].posFrm;
				SetFilePointer (m_bufHFile[HD_FILE_VIDEO], liPosFile.LowPart, &liPosFile.HighPart, FILE_BEGIN);

				bufPosFrm[numBuffFrm] = posBufferingStart;
				bufIdxFrm[numBuffFrm] = idxFrm;
				bufTimeMS[numBuffFrm] = m_bufVideoIndex[idxFrm].timeMSec;
				bufIdxDecMode[numBuffFrm] = MP4D_IDX_DEC_AND_DISP;
				if (IS_INTERLACE_FROM_INDEX_IMG_SZ[m_bufFileHeader[HD_FILE_VIDEO].MP4.bufSetRec[idxCh].idxImgSz] &m_bUseS6010DI)
				{
					bufBFrmIntp[numBuffFrm] = 1;
				}
				else
				{
					bufBFrmIntp[numBuffFrm] = 0;
				}
				
				ReadFile (m_bufHFile[HD_FILE_VIDEO], &pBufBuffering[posBufferingStart], szFrm, &numRead, NULL);
				// copy frame data to buffering buffer[E]

				posBufferingStart = posBufferingStart +szFrm;	// not need %szBufBuffering
				numBuffFrm++;
			}

			idxFrm++;
		}
	}
	else if (idxModePlay == HD_MODE_REVERSE_PLAYING)
	{
		if (idxFrm < 0)
		{	// end of stream
			return;
		}
		
		numBuffFrm = 0;
		while (1)
		{
			if (idxFrm < 0)
			{	// end of stream
				break;
			}

			idxCh = m_bufVideoIndex[idxFrm].idxCh;
			szFrm = DWORD(m_bufVideoIndex[idxFrm +1].posFrm -m_bufVideoIndex[idxFrm].posFrm);

			if (m_pS6010->LIVE_GetIdxWndFromDecCh (BYTE(idxCh)) != INVALID_WND_IDX &&
				m_bufVideoIndex[idxFrm].isKeyFrm == 1)
			{	// need buffering
				if (posBufferingStart +szFrm >= szBufBuffering)
				{
					if (posBufferingStart < posBufferingEnd)
					{	// buffering buffer fullness
						break;
					}
					else
					{
						posBufferingStart = 0;
					}
				}
				if ((posBufferingEnd -posBufferingStart +szBufBuffering) %szBufBuffering <= szFrm)
				{	// buffering buffer fullness
					break;
				}
				if (numBuffFrm >= maxNumBuffFrm)
				{	// buffering queue fullness
					break;
				}

				// copy frame data to buffering buffer[S]
				liPosFile.QuadPart = m_bufVideoIndex[idxFrm].posFrm;
				SetFilePointer (m_bufHFile[HD_FILE_VIDEO], liPosFile.LowPart, &liPosFile.HighPart, FILE_BEGIN);

				bufPosFrm[numBuffFrm] = posBufferingStart;
				bufIdxFrm[numBuffFrm] = idxFrm;
				bufTimeMS[numBuffFrm] = m_bufVideoIndex[idxFrm].timeMSec;
				bufIdxDecMode[numBuffFrm] = MP4D_IDX_DEC_AND_DISP;
				if (IS_INTERLACE_FROM_INDEX_IMG_SZ[m_bufFileHeader[HD_FILE_VIDEO].MP4.bufSetRec[idxCh].idxImgSz] &m_bUseS6010DI)
				{
					bufBFrmIntp[numBuffFrm] = 1;
				}
				else
				{
					bufBFrmIntp[numBuffFrm] = 0;
				}
				
				ReadFile (m_bufHFile[HD_FILE_VIDEO], &pBufBuffering[posBufferingStart], szFrm, &numRead, NULL);
				// copy frame data to buffering buffer[E]

				posBufferingStart = posBufferingStart +szFrm;	// not need %szBufBuffering
				numBuffFrm++;
			}

			idxFrm--;
		}
	}

	numBuffFrm = m_pS6010->MP4D_UpdateFrameBufferStat (numBuffFrm, bufPosFrm, bufIdxFrm, bufTimeMS, bufIdxDecMode, bufBFrmIntp);
	m_pIMP4DecStat->idxBufferingNextFrm = idxFrm;
}

void CHWDecManager::ExecG723Buffering (BOOL bStartPlay)
{
	if (m_bufHFile[HD_FILE_AUDIO] == NULL)
	{
		return;
	}

	DWORD curFrmPos = SetFilePointer (m_bufHFile[HD_FILE_AUDIO], 0, NULL, FILE_CURRENT);
	if (curFrmPos >= DWORD(m_posAudFileIndex))
	{
		return;
	}

	int idxCh = m_pS6010->LIVE_GetDecChFromIdxWnd (glPCurGInfo->idxLiveSelWnd);

	if (m_bufOffsetG723ChDataInLineBuf[idxCh] == HD_G723_INVALID_CH)
	{	// invalid channel
		return;
	}

	int i, numCopyFrm;
	DWORD numRead, nTmp;

	if (bStartPlay == TRUE)
	{
		m_pIG723DecStat->idxQDrvWrite = 0;
		m_pIG723DecStat->idxQAppWrite = 0;
	}
	
	numCopyFrm = (m_pIG723DecStat->idxQDrvWrite -m_pIG723DecStat->idxQAppWrite -1 +SZ_G723_DEC_QUEUE_ITEM) %SZ_G723_DEC_QUEUE_ITEM;

	BYTE *bufTmp = new BYTE[numCopyFrm *m_szG723LineBuf];

	ReadFile (m_bufHFile[HD_FILE_AUDIO], bufTmp, numCopyFrm *m_szG723LineBuf, &numRead, NULL);
	if (curFrmPos +numRead >= DWORD(m_posAudFileIndex))
	{
		numRead = DWORD(m_posAudFileIndex) -curFrmPos;
	}

	numCopyFrm = numRead /m_szG723LineBuf;
	for (i=0; i<numCopyFrm; i++)
	{
		nTmp = ((m_pIG723DecStat->idxQAppWrite +i) %SZ_G723_DEC_QUEUE_ITEM) *SZ_G723_1_ITEM;

		memcpy (&m_pIG723DecStat->pBufG723Dec[nTmp +IDX_G723_DEC_CH *SZ_G723_1_FRM],
			&bufTmp[i *m_szG723LineBuf +m_bufOffsetG723ChDataInLineBuf[idxCh]], SZ_G723_1_FRM);
	}
	
	m_pIG723DecStat->idxQAppWrite = (m_pIG723DecStat->idxQAppWrite +numCopyFrm) %SZ_G723_DEC_QUEUE_ITEM;

	delete[] bufTmp;
}

void CHWDecManager::OnPlay ()
{
	int i, numDecFrm, idxQDrvWrite;

	// audio decoding[S] (HW decoder doesn't need operation for decoding audio.)
	// audio decoding[E]

	// video decoding[S]
	idxQDrvWrite = m_pIMP4DecStat->idxQDrvWrite;
	for (i=0, numDecFrm=0; i<GL_NUM_DISP_FROM_DISP_MODE[m_pS6010->LIVE_GetSplitMode ()]; i++)
	{
		if (m_pIMP4DecStat->idxQDecNeed == idxQDrvWrite)
		{	// no buffered frame
			if (m_bufVideoIndex[m_pIMP4DecStat->idxCurFrm].idxFrmInCh >= 
				GetVideoFrameNum (m_bufVideoIndex[m_pIMP4DecStat->idxCurFrm].idxCh) -1)
			{
				SetEvent (m_bufHEvt[HD_EVT_REQ_PAUSE]);
				return;
			}
			break;
		}

		if (m_pIMP4DecStat->iQueue[m_pIMP4DecStat->idxQDecNeed].timeMS -m_timeRef > (m_timeElapsed_x16 >>4))
		{
			break;
		}
		
		m_pIMP4DecStat->idxQDecNeed = (m_pIMP4DecStat->idxQDecNeed +1) &(SZ_MP4_DEC_QUEUE_ITEM -1);
	}
	// video decoding[E]

	// video buffering check[S]
	DWORD posBufferingStart, posBufferingEnd, szBufBuffering, maxNumBuffFrm;
	BYTE *pBufBuffering;
	
	pBufBuffering = m_pS6010->MP4D_GetFrameBuffer (posBufferingStart, posBufferingEnd, szBufBuffering, maxNumBuffFrm);

	if ((posBufferingEnd -posBufferingStart +szBufBuffering) %szBufBuffering > (szBufBuffering >>1) &&
		maxNumBuffFrm > 32)
	{
		SetEvent (m_bufHEvt[HD_EVT_VIDEO_BUFFERING_EXEC]);
	}
	// video buffering check[E]

	// audio buffering check[S]
	if ((m_pIG723DecStat->idxQAppWrite -m_pIG723DecStat->idxQDrvWrite +SZ_G723_DEC_QUEUE_ITEM)
		%SZ_G723_DEC_QUEUE_ITEM < (SZ_G723_DEC_QUEUE_ITEM >>1))
	{
		SetEvent (m_bufHEvt[HD_EVT_AUDIO_BUFFERING_EXEC]);
	}
	// audio buffering check[E]

	// update player stat[S]
	if (m_cbProc_UpdatePlayerStat != NULL && m_pIMP4DecStat != NULL)
	{
		m_cbProc_UpdatePlayerStat (m_cbCont_UpdatePlayerStat, this, m_pIMP4DecStat->idxCurFrm);
	}
	// update player stat[E]
}

void CHWDecManager::OnReversePlay ()
{
	int i, numDecFrm, idxQDrvWrite;

	// video decoding[S]
	idxQDrvWrite = m_pIMP4DecStat->idxQDrvWrite;
	for (i=0, numDecFrm=0; i<GL_NUM_DISP_FROM_DISP_MODE[m_pS6010->LIVE_GetSplitMode ()]; i++)
	{
		if (m_pIMP4DecStat->idxQDecNeed == idxQDrvWrite)
		{	// no buffered frame
			if (m_bufVideoIndex[m_pIMP4DecStat->idxCurFrm].idxFrmInCh <= 0)
			{
				SetEvent (m_bufHEvt[HD_EVT_REQ_PAUSE]);
				return;
			}
			break;
		}

		if (m_timeRef -m_pIMP4DecStat->iQueue[m_pIMP4DecStat->idxQDecNeed].timeMS > (m_timeElapsed_x16 >>4))
		{
			break;
		}
		
		m_pIMP4DecStat->idxQDecNeed = (m_pIMP4DecStat->idxQDecNeed +1) &(SZ_MP4_DEC_QUEUE_ITEM -1);
	}
	// video decoding[E]

	// video buffering check[S]
	DWORD posBufferingStart, posBufferingEnd, szBufBuffering, maxNumBuffFrm;
	BYTE *pBufBuffering;
	
	pBufBuffering = m_pS6010->MP4D_GetFrameBuffer (posBufferingStart, posBufferingEnd, szBufBuffering, maxNumBuffFrm);

	if ((posBufferingEnd -posBufferingStart +szBufBuffering) %szBufBuffering > (szBufBuffering >>1) &&
		maxNumBuffFrm > 32)
	{
		SetEvent (m_bufHEvt[HD_EVT_VIDEO_BUFFERING_EXEC]);
	}
	// video buffering check[E]

	// update player stat[S]
	if (m_cbProc_UpdatePlayerStat != NULL && m_pIMP4DecStat != NULL)
	{
		m_cbProc_UpdatePlayerStat (m_cbCont_UpdatePlayerStat, this, m_pIMP4DecStat->idxCurFrm);
	}
	// update player stat[E]
}
