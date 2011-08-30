// PlayerManager.cpp: implementation of the CPlayerManager class.
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

static BOOL Decode_NULL (CPlayerManager *pPlayer);
extern BOOL Decode_MP4 (CPlayerManager *pPlayer);
extern BOOL Decode_JPEG (CPlayerManager *pPlayer);
static BOOL Decode_G723 (CPlayerManager *pPlayer);

UINT WINAPI ThFn_PlayerMain (LPVOID pParam)
{
	CPlayerManager *pPlayer = (CPlayerManager *)pParam;

	while (1)
	{
		WaitForSingleObject (pPlayer->m_bufHEvt[PM_EVT_PLAYER_TIMER], INFINITE);
		ResetEvent (pPlayer->m_bufHEvt[PM_EVT_PLAYER_TIMER]);

		if (pPlayer->m_bufBEndTh[PM_TH_PLAYER_MAIN] == TRUE)
		{
			break;
		}

		pPlayer->OnPlay ();
	}

	pPlayer->m_bufBEndTh[PM_TH_PLAYER_MAIN] = FALSE;

	ExitThread (0);
	return 0;
}

UINT WINAPI ThFn_VideoBufferingExec (LPVOID pParam)
{
	CPlayerManager *pPlayer = (CPlayerManager *)pParam;

	while (1)
	{
		WaitForSingleObject (pPlayer->m_bufHEvt[PM_EVT_VIDEO_BUFFERING_EXEC], INFINITE);

		if (pPlayer->m_bufBEndTh[PM_TH_VIDEO_BUFFERING_EXEC] == TRUE)
		{
			break;
		}

		pPlayer->ExecVideoBuffering ();

		ResetEvent (pPlayer->m_bufHEvt[PM_EVT_VIDEO_BUFFERING_EXEC]);
	}

	pPlayer->m_bufBEndTh[PM_TH_VIDEO_BUFFERING_EXEC] = FALSE;

	ExitThread (0);
	return 0;
}

UINT WINAPI ThFn_AudioBufferingExec (LPVOID pParam)
{
	CPlayerManager *pPlayer = (CPlayerManager *)pParam;

	while (1)
	{
		WaitForSingleObject (pPlayer->m_bufHEvt[PM_EVT_AUDIO_BUFFERING_EXEC], INFINITE);

		if (pPlayer->m_bufBEndTh[PM_TH_AUDIO_BUFFERING_EXEC] == TRUE)
		{
			break;
		}

		pPlayer->ExecAudioBuffering ();

		ResetEvent (pPlayer->m_bufHEvt[PM_EVT_AUDIO_BUFFERING_EXEC]);
	}

	pPlayer->m_bufBEndTh[PM_TH_AUDIO_BUFFERING_EXEC] = FALSE;

	ExitThread (0);
	return 0;
}

UINT WINAPI ThFn_HandleReqPause (LPVOID pParam)
{
	CPlayerManager *pPlayer = (CPlayerManager *)pParam;

	while (1)
	{
		WaitForSingleObject (pPlayer->m_bufHEvt[PM_EVT_REQ_PAUSE], INFINITE);
		ResetEvent (pPlayer->m_bufHEvt[PM_EVT_REQ_PAUSE]);

		if (pPlayer->m_bufBEndTh[PM_TH_REQ_PAUSE] == TRUE)
		{
			break;
		}

		pPlayer->Pause ();
	}

	pPlayer->m_bufBEndTh[PM_TH_REQ_PAUSE] = FALSE;

	ExitThread (0);
	return 0;
}

void CALLBACK CbFn_PlayerTimerProc (UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	CPlayerManager *pPlayer = (CPlayerManager *)dwUser;

	if (pPlayer->m_curPlayerMode != PM_MODE_PLAYING)
	{
		return;
	}

	pPlayer->m_timeElapsed_x16 += pPlayer->m_nTimerIncrement_x16;

	SetEvent (pPlayer->m_bufHEvt[PM_EVT_PLAYER_TIMER]);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerManager::CPlayerManager()
{
	m_pWndPlayer = NULL;
	m_bufVideoIndex = NULL;
	DecodeVideo = Decode_NULL;
	DecodeAudio = Decode_NULL;
	m_idxCS = PM_CS_BGRA;
	m_bUseDefaultCS = FALSE;
	m_curPlayerMode = PM_MODE_IDLE;
	m_idxSelWnd = 0;
	m_nMMTimerID = NULL;
	m_cbProc_UpdatePlayerStat = NULL;
	m_bNowExecOnPlay = FALSE;

	int i;

	// Init variables[S]
	for (i=0; i<PM_NUM_FILE; i++)
	{
		m_bufHFile[i] = NULL;
	}

	m_idxSplitMode = LIVE_SPLIT_MODE_1;
	InitSplitModeParam ();

	for (i=0; i<PM_NUM_FILE; i++)
	{
		m_bufIdxStreamFormat[i] = PM_STREAM_FORMAT_NULL;
	}
	// Init variables[E]

	// Buffering buffer init[S]
	iBuffStat[PM_FILE_VIDEO].szBufBuffering = SZ_BUFFERING_BUF_VIDEO;
	iBuffStat[PM_FILE_AUDIO].szBufBuffering = SZ_BUFFERING_BUF_AUDIO;
	for (i=0; i<PM_NUM_FILE; i++)
	{
		iBuffStat[i].bufBuffering = new BYTE[iBuffStat[i].szBufBuffering];
	}
	// Buffering buffer init[E]

	// Event create[S]
	for (i=0; i<PM_NUM_EVENT; i++)
	{
		m_bufHEvt[i] = CreateEvent (NULL, TRUE, FALSE, NULL);
		if (m_bufHEvt[i] == NULL)
		{
			AddReport ("PLAYER: Event create fail");
		}
		ResetEvent (m_bufHEvt[i]);
	}
	// Event create[E]

	// Thread create[S]
	DWORD idThread;

	for (i=0; i<PM_NUM_THREAD; i++)
	{
		m_bufBEndTh[i] = FALSE;
	}
	
	m_bufHThread[PM_TH_PLAYER_MAIN] = CreateThread (NULL, 0,
		(LPTHREAD_START_ROUTINE)ThFn_PlayerMain, this, 0, &idThread);
	m_bufHThread[PM_TH_VIDEO_BUFFERING_EXEC] = CreateThread (NULL, 0,
		(LPTHREAD_START_ROUTINE)ThFn_VideoBufferingExec, this, 0, &idThread);
	m_bufHThread[PM_TH_AUDIO_BUFFERING_EXEC] = CreateThread (NULL, 0,
		(LPTHREAD_START_ROUTINE)ThFn_AudioBufferingExec, this, 0, &idThread);
	m_bufHThread[PM_TH_REQ_PAUSE] = CreateThread (NULL, 0,
		(LPTHREAD_START_ROUTINE)ThFn_HandleReqPause, this, 0, &idThread);
  	for (i=0; i<PM_NUM_THREAD; i++)
	{
		if (m_bufHThread[i] == NULL)
		{
			AddReport ("PLAYER: Thread create fail (%d)\n", i);
		}
	}
	// Thread create[E]

	InitDec_MP4 ();
	InitDec_JPEG ();
	InitDec_G723 ();
}

CPlayerManager::~CPlayerManager()
{
	int i;

	CloseFile ();

	DestroyDec_MP4 ();
	DestroyDec_JPEG ();
	DestroyDec_G723 ();

	DestroyMMTimer ();

	// buffering buffer destroy[S]
	for (i=0; i<PM_NUM_FILE; i++)
	{
		delete[] iBuffStat[i].bufBuffering;
	}
	// buffering buffer destroy[E]

	// index buffer destroy[S]
	if (m_bufVideoIndex != NULL)
	{
		delete[] m_bufVideoIndex;
	}
	// index buffer destroy[E]

	// stop thread[S]
	for (i=0; i<PM_NUM_THREAD; i++)
	{
		m_bufBEndTh[i] = TRUE;
	}
	for (i=0; i<PM_NUM_EVENT; i++)
	{
		SetEvent (m_bufHEvt[i]);
	}
	for (i=0; i<PM_NUM_EVENT; i++)
	{
		while (m_bufBEndTh[i])
		{
			Sleep (0);
		}
		CloseHandle (m_bufHEvt[i]);
	}
	for (i=0; i<PM_NUM_THREAD; i++)
	{
		WaitForSingleObject (m_bufHThread[i], INFINITE);
		CloseHandle (m_bufHThread[i]);
	}
	// stop thread[E]
}

// Public function[S]
BOOL CPlayerManager::InitPlayer (CWndPlayer *pWndPlayer)
{
	InitMMTimer ();
	SetPlaySpeed (PM_PLAY_SPEED_NORMAL);

	m_pWndPlayer = pWndPlayer;
	
	return TRUE;
}

BOOL CPlayerManager::OpenFile (LPCTSTR strVidFilePath, LPCTSTR strAudFilePath)
{
	Pause ();

	DWORD i;

	// Opened file close[S]
	for (i=0; i<PM_NUM_FILE; i++)
	{
		if (m_bufHFile[i] != NULL)
		{
			CloseHandle (m_bufHFile[i]);
			m_bufHFile[i] = NULL;
		}
	}
	// Opened file close[E]

	// File open[S]
	m_bufHFile[PM_FILE_VIDEO] = CreateFile (strVidFilePath,
		GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY |FILE_FLAG_RANDOM_ACCESS, 0);

	if (m_bufHFile[PM_FILE_VIDEO] == INVALID_HANDLE_VALUE)
	{
		AddReport ("PLAYER: Video stream file open fail: %s", strVidFilePath);
		m_bufHFile[PM_FILE_VIDEO] = NULL;
	}
	else
	{
		AddReport ("PLAYER: Video stream file open succeed: %s", strVidFilePath);
	}

	m_bufHFile[PM_FILE_AUDIO] = CreateFile (strAudFilePath,
		GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY |FILE_FLAG_RANDOM_ACCESS, 0);

	if (m_bufHFile[PM_FILE_AUDIO] == INVALID_HANDLE_VALUE)
	{
		AddReport ("PLAYER: Audio stream file open fail: %s", strAudFilePath);
		m_bufHFile[PM_FILE_AUDIO] = NULL;
	}
	else
	{
		AddReport ("PLAYER: Audio stream file open succeed: %s", strAudFilePath);
	}

	if (m_bufHFile[PM_FILE_VIDEO] == NULL &&
		m_bufHFile[PM_FILE_AUDIO] == NULL)
	{	// file open fail
		goto ERROR_EXIT;
	}
	// File open[E]

	DWORD numRead, nRetVal;
	char strTmp[MAX_PATH];

	// File check[S]
	for (i=0; i<PM_NUM_FILE; i++)
	{
		m_bufIdxStreamFormat[i] = PM_STREAM_FORMAT_NULL;

		if (m_bufHFile[i] == NULL)
		{
			continue;
		}
		
		ReadFile (m_bufHFile[i], &m_bufFileHeader[i],
			sizeof(HEADER_RECORD_FILE), &numRead, NULL);
		memcpy (strTmp, m_bufFileHeader[i].strFileInit, SZ_STR_INIT);
		strTmp[SZ_STR_INIT] = 0;

		if (i == PM_FILE_VIDEO)
		{
			if (strcmp (strTmp, STR_MP4_FILE) == 0)
			{	// video MPEG4
				nRetVal = OpenFile_MP4 ();
			}
			else if (strcmp (strTmp, STR_JPEG_FILE) == 0)
			{	// video JPEG
				nRetVal = OpenFile_JPEG ();
			}
			else
			{	// invalid file
				break;
			}
		}
		else if (i == PM_FILE_AUDIO)
		{
			if (strcmp (strTmp, STR_G723_FILE) == 0)
			{	// audio (G723)
				nRetVal = OpenFile_G723 ();
			}
			else
			{	// invalid file
				break;
			}
		}

		if (nRetVal == FALSE)
		{
			break;
		}
	}
	if (i != PM_NUM_FILE)
	{	// invalid file
		goto ERROR_EXIT;
	}
	// File check[E]

	AddReport ("PLAYER: File open succeed: [%d, %d]",
		m_bufNumTotFrm[PM_FILE_VIDEO], m_bufNumTotFrm[PM_FILE_AUDIO]);

	m_curPlayerMode = PM_MODE_PAUSE;
	
	SetSplitMode (LIVE_SPLIT_MODE_1);

	return TRUE;

ERROR_EXIT:
	for (i=0; i<PM_NUM_FILE; i++)
	{
		if (m_bufHFile[i] != NULL)
		{
			CloseHandle (m_bufHFile[i]);
			m_bufHFile[i] = NULL;
		}
	}

	return FALSE;
}

void CPlayerManager::CloseFile ()
{
	Pause ();

	m_curPlayerMode = PM_MODE_IDLE;

	int i;
	
	for (i=0; i<PM_NUM_FILE; i++)
	{
		if (m_bufHFile[i] != NULL)
		{
			CloseHandle (m_bufHFile[i]);
			m_bufHFile[i] = NULL;
		}
	}

	switch (m_bufIdxStreamFormat[PM_FILE_VIDEO])
	{
	case PM_STREAM_FORMAT_MP4:
		CloseFile_MP4 ();
		break;
	case PM_STREAM_FORMAT_JPEG:
		CloseFile_JPEG ();
		break;
	}

	switch (m_bufIdxStreamFormat[PM_FILE_AUDIO])
	{
	case PM_STREAM_FORMAT_G723:
		CloseFile_G723 ();
		break;
	}

	m_bufIdxStreamFormat[PM_FILE_VIDEO] = PM_STREAM_FORMAT_NULL;
	m_bufIdxStreamFormat[PM_FILE_AUDIO] = PM_STREAM_FORMAT_NULL;
}

void CPlayerManager::Play ()
{
	if (m_curPlayerMode == PM_MODE_IDLE ||
		m_curPlayerMode == PM_MODE_PLAYING)
	{
		return;
	}

	ExecVideoBuffering ();
	ExecAudioBuffering ();

	// reset reference time[S]
	// This is OK because current player state is PAUSE.
	m_timeRef = m_bufVideoIndex[iBuffStat[PM_FILE_VIDEO].idxFrmDecRead].timeMSec;
	m_timeElapsed_x16 = 0;
	// reset reference time[E]

	m_curPlayerMode = PM_MODE_PLAYING;

	// audio play[S]
	if (m_bufIdxStreamFormat[PM_FILE_AUDIO] == PM_STREAM_FORMAT_G723)
	{
		::waveOutRestart (m_hWaveOut);
	}
	// audio play[E]
}

void CPlayerManager::Pause ()
{
	if (m_curPlayerMode == PM_MODE_IDLE ||
		m_curPlayerMode == PM_MODE_PAUSE)
	{
		return;
	}

	m_curPlayerMode = PM_MODE_PAUSE;

	// Thread pause check[S]
	while (1)
	{
		if (WaitForSingleObject (m_bufHEvt[PM_EVT_VIDEO_BUFFERING_EXEC], 0) == WAIT_TIMEOUT)
			break;
	}
	while (1)
	{
		if (WaitForSingleObject (m_bufHEvt[PM_EVT_AUDIO_BUFFERING_EXEC], 0) == WAIT_TIMEOUT)
			break;
	}
	while (1)
	{
		if (WaitForSingleObject (m_bufHEvt[PM_EVT_PLAYER_TIMER], 0) == WAIT_TIMEOUT)
			break;
	}
	// Thread pause check[E]

	// OnPlay () function end check[S]
	while (m_bNowExecOnPlay == TRUE)
	{
		Sleep (0);
	}
	// OnPlay () function end check[E]

	// audio stop[S]
	if (m_bufIdxStreamFormat[PM_FILE_AUDIO] == PM_STREAM_FORMAT_G723)
	{
		::waveOutPause (m_hWaveOut);
	}
	// audio stop[E]
}

void CPlayerManager::Stop ()
{
	if (m_curPlayerMode == PM_MODE_IDLE)
	{
		return;
	}

	SetFramePos (0);
}

void CPlayerManager::SetFramePos (DWORD posFrm)
{
	Pause ();

	// video[S]
	if (m_bufIdxStreamFormat[PM_FILE_VIDEO] == PM_STREAM_FORMAT_MP4)
	{
		SetFramePos_MP4 (posFrm);
	}
	else if (m_bufIdxStreamFormat[PM_FILE_VIDEO] == PM_STREAM_FORMAT_JPEG)
	{
		SetFramePos_JPEG (posFrm);
	}

	Display ();
	// video[E]

	// audio[S]
	if (m_bufIdxStreamFormat[PM_FILE_AUDIO] == PM_STREAM_FORMAT_G723)
	{
		SetFramePos_G723 (posFrm);
	}
	// audio[E]

	// update player stat[S]
	if (m_cbProc_UpdatePlayerStat != NULL)
	{
		m_cbProc_UpdatePlayerStat (m_cbCont_UpdatePlayerStat, this, m_bufPosCurFrm[PM_FILE_VIDEO]);
	}
	// update player stat[E]
}

BOOL CPlayerManager::OffsetFramePos (int idxWnd, int nOffset)
{
	int idxCh = m_bufChFromWnd[idxWnd];
	
	if (m_bufBDec[idxCh] == FALSE || nOffset == 0)
	{
		return FALSE;
	}

	int i;

	// get current pos[S]
	for (i=m_bufPosCurFrm[PM_FILE_VIDEO]; i>=0; i--)
	{
		if (m_bufVideoIndex[i].idxCh == idxCh)
		{
			break;
		}
	}
	if (i < 0)
	{
		for (i=m_bufPosCurFrm[PM_FILE_VIDEO]; i<int(m_bufNumTotFrm[PM_FILE_VIDEO]); i++)
		{
			if (m_bufVideoIndex[i].idxCh == idxCh)
			{
				break;
			}
		}
	}
	if (i >= int(m_bufNumTotFrm[PM_FILE_VIDEO]))
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
		for (; i<int(m_bufNumTotFrm[PM_FILE_VIDEO]); i++)
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
		if (i >= int(m_bufNumTotFrm[PM_FILE_VIDEO]))
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

BOOL CPlayerManager::SetPlaySpeed (int idxPlaySpeed)
{
	if (idxPlaySpeed < PM_PLAY_SPEED_DIV16 || idxPlaySpeed > PM_PLAY_SPEED_X16)
	{
		return FALSE;
	}

	const int BUF_PS_TO_TIMER_RES_MUL[PM_NUM_PLAY_SPEED] = {
		1, 2, 4, 8, 16, 32, 64, 128, 256
	};

	BOOL bTmpPlaying = FALSE;
	if (m_curPlayerMode == PM_MODE_PLAYING)
	{
		bTmpPlaying = TRUE;
		
		Pause ();
	}

	m_idxPS = idxPlaySpeed;
	m_nTimerIncrement_x16 = m_nMMTimerRes *BUF_PS_TO_TIMER_RES_MUL[idxPlaySpeed];

	if (bTmpPlaying == TRUE)
	{
		Play ();
	}
	
	return TRUE;
}

void CPlayerManager::SetSplitMode (int idxMode)
{
	InitSplitModeParam ();

	m_idxSplitMode = idxMode;

	int i, j;
	for (i=0, j=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		if (GetVideoFrameNum (i) > 0)
		{
			m_bufBDec[i] = TRUE;
			m_bufChFromWnd[j] = i;
			m_bufWndFromCh[i] = j;
			j++;

			if (j >= GL_NUM_DISP_FROM_DISP_MODE[idxMode])
			{
				break;
			}
		}
	}

	SetSelWnd (0);

	m_pWndPlayer->ChangeSplitMode (idxMode);

	Stop ();	// update changed split mode
}

BOOL CPlayerManager::SetChannel (int idxWnd, int idxCh)
{
	if (GetVideoFrameNum (idxCh) == 0)
	{
		return FALSE;
	}
	if (idxWnd >= GL_NUM_DISP_FROM_DISP_MODE[m_idxSplitMode])
	{
		return FALSE;
	}

	int idxOldWnd = m_bufWndFromCh[idxCh];
	int idxOldCh = m_bufChFromWnd[idxWnd];

	if (idxOldWnd != idxWnd && idxOldWnd != PM_INVALID_WND)
	{	// Currently "idxCh" channel is displayed by "idxOldWnd" window.
		m_bufChFromWnd[idxOldWnd] = idxOldCh;
		m_bufWndFromCh[idxOldCh] = idxOldWnd;
		m_bufBDec[idxOldCh] = TRUE;
	}
	else
	{
		m_bufBDec[idxOldCh] = FALSE;
	}

	m_bufChFromWnd[idxWnd] = idxCh;
	m_bufWndFromCh[idxCh] = idxWnd;
	m_bufBDec[idxCh] = TRUE;

	Stop ();

	return TRUE;
}

BOOL CPlayerManager::SetSelWnd (int idxWnd)
{
	if (idxWnd >= PM_MAX_NUM_DISP_WND)
	{
		return FALSE;
	}

	m_idxSelWnd = idxWnd;

	if (m_bufIdxStreamFormat[PM_FILE_AUDIO] != PM_STREAM_FORMAT_NULL &&
		m_curPlayerMode == PM_MODE_PLAYING)
	{
		Pause ();
		SetFramePos (m_bufPosCurFrm[PM_FILE_VIDEO]);
		Play ();
	}

	return TRUE;
}

void CPlayerManager::SetDefaultColorspace (BOOL bSet)
{
	m_bUseDefaultCS = bSet;

	FindAndSetOptimalCS ();
}

void CPlayerManager::RegisterCB_UpdatePlayerStat (PM_UPDATE_PLAYER_STAT_PROC cbFnUpdatePlayerStat, void *pContext)
{
	m_cbProc_UpdatePlayerStat = cbFnUpdatePlayerStat;
	m_cbCont_UpdatePlayerStat = pContext;
}

void CPlayerManager::UnregisterCB_UpdatePlayerStat ()
{
	m_cbProc_UpdatePlayerStat = NULL;
}
// Public function[E]

// Private function[S]
void CPlayerManager::FindAndSetOptimalCS ()
{
	if (m_pWndPlayer == NULL)
	{
		return;
	}
	
	int i;
	const int *pPriorityTbl;

	if (m_bufIdxStreamFormat[PM_FILE_VIDEO] == PM_STREAM_FORMAT_MP4)
	{
		pPriorityTbl = PM_MP4_CS_PRIORITY;
	}
	else
	{
		pPriorityTbl = PM_JPEG_CS_PRIORITY;
	}

	if (m_bUseDefaultCS == TRUE)
	{
		m_pWndPlayer->SetCS (PM_CS_BGRA);
		
		AddReport ("CS set to default (%d)", PM_CS_BGRA);
		m_idxCS = PM_CS_BGRA;
	}
	else
	{
		for (i=0; i<PM_NUM_CS; i++)
		{
			if (m_pWndPlayer->IsSupportedCS (pPriorityTbl[i]) == TRUE)
			{
				m_pWndPlayer->SetCS (pPriorityTbl[i]);
				break;
			}
		}

		AddReport ("CS changed to %d", pPriorityTbl[i]);
		m_idxCS = pPriorityTbl[i];
	}

	if (m_bufIdxStreamFormat[PM_FILE_VIDEO] == PM_STREAM_FORMAT_MP4)
	{
		switch (m_idxCS)
		{
		case PM_CS_BGRA:
			m_stXvidDecFrame.output.csp = XVID_CSP_BGRA;
			break;
		case PM_CS_I420:
			m_stXvidDecFrame.output.csp = XVID_CSP_I420;
			break;
		case PM_CS_YV12:
			m_stXvidDecFrame.output.csp = XVID_CSP_YV12;
			break;
		case PM_CS_YUY2:
			m_stXvidDecFrame.output.csp = XVID_CSP_YUY2;
			break;
		case PM_CS_UYVY:
			m_stXvidDecFrame.output.csp = XVID_CSP_UYVY;
			break;
		}
	}
}

BOOL CPlayerManager::InitMMTimer ()
{
	if (m_nMMTimerID != NULL)
	{
		DestroyMMTimer ();
	}

	m_nMMTimerRes = CSOLO6010::AllocateTimerRes ();

	m_nMMTimerID = timeSetEvent (m_nMMTimerRes, m_nMMTimerRes,
		CbFn_PlayerTimerProc, (DWORD)this, TIME_PERIODIC);

	return TRUE;
}

void CPlayerManager::DestroyMMTimer ()
{
	if (m_nMMTimerID == NULL)
	{
		return;
	}

	timeKillEvent (m_nMMTimerID);
	m_nMMTimerID = NULL;

	CSOLO6010::ReleaseTimerRes ();
}

void CPlayerManager::OnPlay ()
{
	BOOL bExecDisp;

	m_bNowExecOnPlay = TRUE;

	// audio[S] (Due to sync with video, audio is decoded first.)
	DecodeAudio (this);
	// audio[E]

	// video[S]
	bExecDisp = DecodeVideo (this);
	
	if (bExecDisp == TRUE)
	{
		Display ();
	}
	// video[E]

	// video buffering check[S]
	if ((iBuffStat[PM_FILE_VIDEO].posFileWrite +iBuffStat[PM_FILE_VIDEO].szBufBuffering
		-iBuffStat[PM_FILE_VIDEO].posDecRead) %iBuffStat[PM_FILE_VIDEO].szBufBuffering
		< iBuffStat[PM_FILE_VIDEO].szBufBuffering /4)
	{
		SetEvent (m_bufHEvt[PM_EVT_VIDEO_BUFFERING_EXEC]);
	}
	// video buffering check[E]

	// audio buffering check[S]
	if (iBuffStat[PM_FILE_AUDIO].idxFrmFileWrite -iBuffStat[PM_FILE_AUDIO].idxFrmDecRead
		< NUM_AUDIO_BUFFERING_FRM /2)
	{
		SetEvent (m_bufHEvt[PM_EVT_AUDIO_BUFFERING_EXEC]);
	}
	// audio buffering check[E]

	// update player stat[S]
	if (m_cbProc_UpdatePlayerStat != NULL)
	{
		m_cbProc_UpdatePlayerStat (m_cbCont_UpdatePlayerStat, this, m_bufPosCurFrm[PM_FILE_VIDEO]);
	}
	// update player stat[E]

	m_bNowExecOnPlay = FALSE;
}

BOOL Decode_NULL (CPlayerManager *pPlayer)
{
	return FALSE;
}

void CPlayerManager::ExecVideoBuffering ()
{
	if (m_bufIdxStreamFormat[PM_FILE_VIDEO] == PM_STREAM_FORMAT_MP4)
	{
		Buffering_MP4 ();
	}
	else if (m_bufIdxStreamFormat[PM_FILE_VIDEO] == PM_STREAM_FORMAT_JPEG)
	{
		Buffering_JPEG ();
	}
}

void CPlayerManager::ExecAudioBuffering ()
{
	if (m_bufIdxStreamFormat[PM_FILE_AUDIO] == PM_STREAM_FORMAT_G723)
	{
		Buffering_G723 ();
	}
}

void CPlayerManager::Display (BOOL bUpdateBorder)
{
	m_pWndPlayer->Draw (bUpdateBorder);
}

void CPlayerManager::InitSplitModeParam ()
{
	int i;

	for (i=0; i<PM_MAX_NUM_DISP_WND; i++)
	{
		m_bufChFromWnd[i] = PM_INVALID_CH;
	}
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		m_bufWndFromCh[i] = PM_INVALID_WND;
	}
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		m_bufBDec[i] = FALSE;
	}
}
// Private function[E]
