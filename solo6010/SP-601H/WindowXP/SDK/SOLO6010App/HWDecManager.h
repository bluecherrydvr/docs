#pragma once

#include "Solo6010_ExRecFileInfo.h"
#include <Mmsystem.h>

enum
{
	HD_FILE_VIDEO,
	HD_FILE_AUDIO,
	HD_NUM_FILE
};

enum
{
	HD_TH_PLAYER_MAIN,
	HD_TH_VIDEO_BUFFERING_EXEC,
	HD_TH_AUDIO_BUFFERING_EXEC,
	HD_TH_REQ_PAUSE,
	HD_NUM_THREAD
};

enum
{
	HD_EVT_PLAYER_TIMER,
	HD_EVT_VIDEO_BUFFERING_EXEC,
	HD_EVT_AUDIO_BUFFERING_EXEC,
	HD_EVT_REQ_PAUSE,
	HD_NUM_EVENT
};

enum
{
	HD_MODE_IDLE,
	HD_MODE_PAUSE,
	// Above: Pause, Below: Play
	HD_MODE_PLAYING,
	HD_MODE_REVERSE_PLAYING,
};

enum
{
	HD_PLAY_SPEED_DIV16,
	HD_PLAY_SPEED_DIV8,
	HD_PLAY_SPEED_DIV4,
	HD_PLAY_SPEED_DIV2,
	HD_PLAY_SPEED_NORMAL,
	HD_PLAY_SPEED_X2,
	HD_PLAY_SPEED_X4,
	HD_PLAY_SPEED_X8,
	HD_PLAY_SPEED_X16,
	HD_NUM_PLAY_SPEED
};

static const BYTE GL_NUM_PS_FROM_IDX_PS[HD_NUM_PLAY_SPEED] = {
	1, 1, 1, 1, 1, 2, 4, 8, 16
};

struct ITEM_HD_VIDEO_INDEX
{
	unsigned char idxCh;
	unsigned char isKeyFrm;
	unsigned char reserved1;
	unsigned char reserved2;
	DWORD timeMSec;
	__int64 posFrm;
	DWORD idxFrmInCh;
};

class CHWDecManager;
typedef void (CALLBACK *HD_UPDATE_PLAYER_STAT_PROC) (void *pContext, CHWDecManager *pPlayer, DWORD posFrm);


class CHWDecManager  
{
public:
	CHWDecManager();
	virtual ~CHWDecManager();
	void InitPlayer (CSOLO6010 *pS6010);
	void DestroyPlayer ();

	BOOL OpenFile (LPCTSTR strVidFilePath, LPCTSTR strAudFilePath);
	void CloseFile ();

	__inline BOOL IsVideoFileOpen () { return !!m_bufHFile[HD_FILE_VIDEO]; }
	__inline BOOL IsAudioFileOpen () { return !!m_bufHFile[HD_FILE_AUDIO]; }
	__inline DWORD GetTotalVideoFrameNum () { return m_bufNumTotFrm[HD_FILE_VIDEO]; }
	__inline DWORD GetVideoFrameNum (int idxCh) { return m_bufNumVidFrm[idxCh]; }
	__inline DWORD GetVideoStartTime () { return (m_bufVideoIndex == NULL) ? 0 : m_bufVideoIndex[0].timeMSec; }
	__inline DWORD GetVideoElapsedTime (DWORD posFrm) { return (m_bufVideoIndex == NULL) ? 0 : (m_bufVideoIndex[posFrm].timeMSec -m_bufVideoIndex[0].timeMSec); }
	__inline const ITEM_HD_VIDEO_INDEX *const GetVideoFileIndex ()	{ return m_bufVideoIndex; }
	__inline int GetPlayMode () { return m_curPlayerMode; }
	BOOL IsWndStreamCIF ();

	void Play ();
	void Pause (BOOL bSetFrmPos = TRUE);
	void Stop ();
	void SetFramePos (DWORD posFrm);	// Reference to video frame
	__inline DWORD GetFramePos () { return (m_pIMP4DecStat == NULL) ? 0 : m_pIMP4DecStat->idxCurFrm; }
	BOOL OffsetFramePos (int idxWnd, int nOffset);
	void ReversePlay ();

	BOOL SetPlaySpeed (int idxPlaySpeed);
	__inline DWORD GetPlaySpeed () { return m_idxPS; }

	void RegisterCB_UpdatePlayerStat (HD_UPDATE_PLAYER_STAT_PROC cbFnUpdatePlayerStat, void *pContext);
	void UnregisterCB_UpdatePlayerStat ();

	void EnableS6010DeinterlaceFilter (BOOL bEnable);
	// control function[E]

private:
	//// general[S]
	CSOLO6010 *m_pS6010;
	
	HANDLE m_bufHThread[HD_NUM_THREAD];
	volatile BOOL m_bufBEndTh[HD_NUM_THREAD];
	HANDLE m_bufHEvt[HD_NUM_EVENT];
	volatile int m_curPlayerMode;
	HANDLE m_bufHFile[HD_NUM_FILE];

	DWORD m_bufNumTotFrm[HD_NUM_FILE];
		// Video : All frame numbers of all channel
		// Audio : All frame numbers of 1 channel
	DWORD m_bufNumVidFrm[NUM_VID_TOTAL_ENC_CH];

	DWORD m_timeRef;
	DWORD m_timeElapsed_x16;
	DWORD m_nMMTimerRes;
	UINT m_nMMTimerID;
	DWORD m_nTimerIncrement_x16;
	int m_idxPS;	// play speed

	HEADER_RECORD_FILE m_bufFileHeader[HD_NUM_FILE];
	__int64 m_posVidFileIndex;
	ITEM_HD_VIDEO_INDEX *m_bufVideoIndex;
	__int64 m_posAudFileIndex;
	//// general[E]

	// MP4 decoding[S]
	INFO_MP4_DEC_STAT *m_pIMP4DecStat;
	BOOL m_bUseS6010DI;
	BOOL m_bBufferingOnlyIFrm;
	// MP4 decoding[E]

	// G723 decoding[S]
	INFO_G723_DEC_STAT *m_pIG723DecStat;
	DWORD m_szG723LineBuf;
	DWORD m_idxFrmDataOut;
	int m_bufOffsetG723ChDataInLineBuf[NUM_G723_CH];
	// G723 decoding[E]

	// update player stat[S]
	HD_UPDATE_PLAYER_STAT_PROC m_cbProc_UpdatePlayerStat;
	LPVOID m_cbCont_UpdatePlayerStat;
	// update player stat[E]

	// member functions[S]
//	BOOL OpenFile (LPCTSTR strVidFilePath, LPCTSTR strAudFilePath);
		BOOL OpenFile_MP4 ();
		BOOL OpenFile_G723 ();
//	void CloseFile ();
		void CloseFile_MP4 ();
		void CloseFile_G723 ();

	BOOL InitMMTimer ();
	void DestroyMMTimer ();

//	void SetFramePos (DWORD posFrm);	// Reference to video frame
		void SetFramePos_MP4 (DWORD posFrm);
		void SetFramePos_G723 (DWORD posFrm);
	void ExecMP4Buffering (int idxModePlay);
	void ExecG723Buffering (BOOL bStartPlay);

	void OnPlay ();
	void OnReversePlay ();
	// member functions[E]

	// Friend function[S]
	friend UINT WINAPI ThFn_HD_PlayerMain (LPVOID pParam);
	friend UINT WINAPI ThFn_HD_VideoBufferingExec (LPVOID pParam);
	friend UINT WINAPI ThFn_HD_AudioBufferingExec (LPVOID pParam);
	friend UINT WINAPI ThFn_HD_HandleReqPause (LPVOID pParam);
	friend void CALLBACK CbFn_HD_PlayerTimerProc (UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
	// Friend function[E]
};
