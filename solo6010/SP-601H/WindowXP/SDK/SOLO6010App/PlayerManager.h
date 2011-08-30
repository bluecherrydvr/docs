// PlayerManager.h: interface for the CPlayerManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYERMANAGER_H__88AFDD34_7BA9_48DE_ABDB_6C1F46C7C4FB__INCLUDED_)
#define AFX_PLAYERMANAGER_H__88AFDD34_7BA9_48DE_ABDB_6C1F46C7C4FB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Solo6010_ExRecFileInfo.h"
#include "MPEG4\\xvid.h"
#include "JPEG\\ijl.h"
#include "MPEG4\\xvid.h"
#include "G723\\G723Coder.h"
#include <Mmsystem.h>

enum
{
	PM_FILE_VIDEO,
	PM_FILE_AUDIO,
	PM_NUM_FILE
};

enum
{
	PM_STREAM_FORMAT_NULL,
	PM_STREAM_FORMAT_MP4,
	PM_STREAM_FORMAT_JPEG,
	PM_STREAM_FORMAT_G723,
	PM_STREAM_FORMAT_G723_NO_PLAY,
};

enum
{
	PM_TH_PLAYER_MAIN,
	PM_TH_VIDEO_BUFFERING_EXEC,
	PM_TH_AUDIO_BUFFERING_EXEC,
	PM_TH_REQ_PAUSE,
	PM_NUM_THREAD
};

enum
{
	PM_EVT_PLAYER_TIMER,
	PM_EVT_VIDEO_BUFFERING_EXEC,
	PM_EVT_AUDIO_BUFFERING_EXEC,
	PM_EVT_REQ_PAUSE,
	PM_NUM_EVENT
};

enum
{
	PM_MODE_IDLE,
	PM_MODE_PAUSE,
	PM_MODE_PLAYING,
};

enum
{
	PM_PLAY_SPEED_DIV16,
	PM_PLAY_SPEED_DIV8,
	PM_PLAY_SPEED_DIV4,
	PM_PLAY_SPEED_DIV2,
	PM_PLAY_SPEED_NORMAL,
	PM_PLAY_SPEED_X2,
	PM_PLAY_SPEED_X4,
	PM_PLAY_SPEED_X8,
	PM_PLAY_SPEED_X16,
	PM_NUM_PLAY_SPEED
};

#define PM_MAX_NUM_DISP_WND	16
#define PM_INVALID_CH	-1
#define PM_INVALID_WND	-1

enum
{
	PM_CS_BGRA,		// BGRA (32Bit)
	PM_CS_I420,		// YUV Planar (YUV)
	PM_CS_YV12,		// YUV Planar (YVU)
	PM_CS_YUY2,		// YUV Packed (YUYV)
	PM_CS_UYVY,		// YUV Packed (UYVY)
	PM_NUM_CS
};

const DWORD PM_FOURCC_CODE_FROM_IDX_CS[PM_NUM_CS] = {
	0,
	MAKEFOURCC('I','4','2','0'),
	MAKEFOURCC('Y','V','1','2'),
	MAKEFOURCC('Y','U','Y','2'),
	MAKEFOURCC('U','Y','V','Y'),
};

const int PM_MP4_CS_PRIORITY[PM_NUM_CS] = {
	PM_CS_BGRA, PM_CS_YUY2, PM_CS_UYVY, PM_CS_BGRA, PM_CS_BGRA
};
const int PM_JPEG_CS_PRIORITY[PM_NUM_CS] = {
	PM_CS_BGRA, PM_CS_BGRA, PM_CS_BGRA, PM_CS_BGRA, PM_CS_BGRA
};

enum	// MPEG4 decoding post filter mode (function Dec1Frame_MP4 (), variable idxModePostFilter)
{
	MP4DEC_POSTFLT_NOT_USE,
	MP4DEC_POSTFLT_DEBLK_LUM,
	MP4DEC_POSTFLT_DEBLK_LUM_DERING_LUM,
	MP4DEC_POSTFLT_DEBLK_LUM_CHR,
	MP4DEC_POSTFLT_DEBLK_LUM_CHR_DERING_LUM,
	MP4DEC_POSTFLT_DEBLK_LUM_CHR_DERING_LUM_CHR,
	NUM_MP4DEC_POSTFLT_MODE
};

struct INFO_IMG_POST_FILTER
{
	BOOL bUsePostFilter;

	BOOL bUseDeint;
	BOOL bUseUpSc;
	BOOL bUseDnSc;
};

struct ITEM_PM_VIDEO_INDEX
{
	unsigned char idxCh;
	unsigned char isKeyFrm;
	unsigned char reserved1;
	unsigned char reserved2;
	DWORD timeMSec;
	__int64 posFrm;
	DWORD idxFrmInCh;
};

struct INFO_PM_BUFFERING_STAT
{
	DWORD szBufBuffering;
	BYTE *bufBuffering;
	volatile DWORD posDecRead;
	volatile DWORD posFileWrite;
	volatile DWORD idxFrmDecRead;
	volatile DWORD idxFrmFileWrite;
};

class CPlayerManager;
typedef void (CALLBACK *PM_UPDATE_PLAYER_STAT_PROC) (void *pContext, CPlayerManager *pPlayer, DWORD posFrm);

#define SZ_BUFFERING_BUF_VIDEO	(8 *1024 *1024)

#define NUM_AUDIO_BUFFERING_FRM	512
#define SZ_BUFFERING_BUF_AUDIO	(SZ_BUF_IN_BYTE_G723_1_FRM_DEC *NUM_AUDIO_BUFFERING_FRM)

class CWndPlayer;


class CPlayerManager
{
public:
	CPlayerManager();
	virtual ~CPlayerManager();

	// control function[S]
	BOOL InitPlayer (CWndPlayer *pWndPlayer);
	BOOL OpenFile (LPCTSTR strVidFilePath, LPCTSTR strAudFilePath);
	void CloseFile ();
	__inline BOOL IsVideoFileOpen () { return !!m_bufHFile[PM_FILE_VIDEO]; }
	__inline BOOL IsAudioFileOpen () { return !!m_bufHFile[PM_FILE_AUDIO]; }
	__inline DWORD GetTotalVideoFrameNum () { return m_bufNumTotFrm[PM_FILE_VIDEO]; }
	__inline DWORD GetVideoFrameNum (int idxCh) { return m_bufNumVidFrm[idxCh]; }
	__inline DWORD GetVideoStartTime () { return m_bufVideoIndex[0].timeMSec; }
	__inline DWORD GetVideoElapsedTime (DWORD posFrm) { return m_bufVideoIndex[posFrm].timeMSec -m_bufVideoIndex[0].timeMSec; }
	__inline const ITEM_PM_VIDEO_INDEX *const GetVideoFileIndex ()	{ return m_bufVideoIndex; }
	__inline int GetPlayMode () { return m_curPlayerMode; }

	void Play ();
	void Pause ();
	void Stop ();
	void SetFramePos (DWORD posFrm);	// Reference to video frame
	__inline DWORD GetFramePos () { return m_bufPosCurFrm[PM_FILE_VIDEO]; }
	BOOL OffsetFramePos (int idxWnd, int nOffset);

	BOOL SetPlaySpeed (int idxPlaySpeed);
	__inline DWORD GetPlaySpeed () { return m_idxPS; }

	void SetSplitMode (int idxMode);
	__inline int GetSplitMode () { return m_idxSplitMode; }
	BOOL SetChannel (int idxWnd, int idxCh);
	__inline int GetChannel (int idxWnd) { return m_bufChFromWnd[idxWnd]; }
	BOOL SetSelWnd (int idxWnd);
	__inline int GetSelWnd () { return m_idxSelWnd; }

	void SetDefaultColorspace (BOOL bSet);

	void RegisterCB_UpdatePlayerStat (PM_UPDATE_PLAYER_STAT_PROC cbFnUpdatePlayerStat, void *pContext);
	void UnregisterCB_UpdatePlayerStat ();

	void SetMP4PostFilterState (BOOL bUseDeblkY, BOOL bUseDeblkUV, BOOL bUseDeringY, BOOL bUseDeringUV);
	void SetImgPostFilterState (BOOL bUseDeint, BOOL bUseUpSc, BOOL bUseDnSc);
	// control function[E]

private:
	// member variables[S]

	//// general[S]
	HANDLE m_bufHThread[PM_NUM_THREAD];
	volatile BOOL m_bufBEndTh[PM_NUM_THREAD];
	HANDLE m_bufHEvt[PM_NUM_EVENT];
	volatile int m_curPlayerMode;
	HANDLE m_bufHFile[PM_NUM_FILE];
	CWndPlayer *m_pWndPlayer;

	DWORD m_bufNumTotFrm[PM_NUM_FILE];
		// Video : All frame numbers of all channel
		// Audio : All frame numbers of 1 channel
	DWORD m_bufNumVidFrm[NUM_VID_TOTAL_ENC_CH];
	DWORD m_bufPosCurFrm[PM_NUM_FILE];

	DWORD m_timeRef;
	DWORD m_timeElapsed_x16;
	DWORD m_nMMTimerRes;
	UINT m_nMMTimerID;
	DWORD m_nTimerIncrement_x16;
	int m_idxPS;	// play speed

	volatile BOOL m_bNowExecOnPlay;
	//// general[E]

	//// colorspace[S]
	int m_idxCS;
	BOOL m_bUseDefaultCS;
	//// colorspace[E]

	//// buffering[S]
	INFO_PM_BUFFERING_STAT iBuffStat[PM_NUM_FILE];
	//// buffering[E]

	//// split & decoding channel[S]
	int m_idxSplitMode;
	int m_idxSelWnd;
	int m_bufChFromWnd[PM_MAX_NUM_DISP_WND];
	int m_bufWndFromCh[NUM_VID_TOTAL_ENC_CH];
	BOOL m_bufBDec[NUM_VID_TOTAL_ENC_CH];
	//// split & decoding channel[E]

	//// Frame index[S]
	HEADER_RECORD_FILE m_bufFileHeader[PM_NUM_FILE];
	__int64 m_posVidFileIndex;
	ITEM_PM_VIDEO_INDEX *m_bufVideoIndex;
	DWORD m_bufIdxStreamFormat[PM_NUM_FILE];
	//// Frame index[E]
	// member variables[E]

	// MP4 decoding[S]
	HANDLE m_hXvidDec[NUM_VID_TOTAL_ENC_CH];
	xvid_dec_stats_t m_stXvidDecStat;
	xvid_dec_frame_t m_stXvidDecFrame;
	int m_idxMP4PostFilterMode;
	// MP4 decoding[E]

	// JPEG decoding[S]
	JPEG_CORE_PROPERTIES m_stJpegCoreProp;
	// JPEG decoding[E]

	// G723 decoding[S]
	HWAVEOUT m_hWaveOut;
	WAVEHDR m_hdrWave[NUM_AUDIO_BUFFERING_FRM];
	CG723Coder m_decG723;
	DWORD m_szG723LineBuf;
	int m_bufOffsetG723ChDataInLineBuf[NUM_G723_CH];
	// G723 decoding[E]

	// Post filter stat[S]
	INFO_IMG_POST_FILTER m_iImgPostFilter;
	// Post filter stat[E]

	// update player stat[S]
	PM_UPDATE_PLAYER_STAT_PROC m_cbProc_UpdatePlayerStat;
	LPVOID m_cbCont_UpdatePlayerStat;
	// update player stat[E]

	// member functions[S]
//	BOOL OpenFile (LPCTSTR strVidFilePath, LPCTSTR strAudFilePath);
		BOOL OpenFile_MP4 ();
		BOOL OpenFile_JPEG ();
		BOOL OpenFile_G723 ();
	void FindAndSetOptimalCS ();
//	void CloseFile ();
		void CloseFile_MP4 ();
		void CloseFile_JPEG ();
		void CloseFile_G723 ();

	BOOL InitMMTimer ();
	void DestroyMMTimer ();

//	void SetFramePos (DWORD posFrm);	// Reference to video frame
		void SetFramePos_MP4 (DWORD posFrm);
		void SetFramePos_JPEG (DWORD posFrm);
		void SetFramePos_G723 (DWORD posFrm);
	BOOL (*DecodeVideo) (CPlayerManager *);
	BOOL (*DecodeAudio) (CPlayerManager *);
	void ExecVideoBuffering ();
		void Buffering_MP4 ();
		void Buffering_JPEG ();
	void ExecAudioBuffering ();
		void Buffering_G723 ();

	void OnPlay ();
	__inline void Display (BOOL bUpdateBorder = FALSE);

	void InitSplitModeParam ();

	void InitDec_MP4 ();
	void InitDec_JPEG ();
	void InitDec_G723 ();

	void DestroyDec_MP4 ();
	void DestroyDec_JPEG ();
	void DestroyDec_G723 ();

	BOOL Dec1Frame_MP4 (BYTE *bufStream, DWORD szStream, int idxCh, BYTE *bufDecImg, DWORD nPitch, int idxModePostFilter);
	BOOL Dec1Frame_JPEG (BYTE *bufStream, DWORD szStream, BYTE *bufDecImg, DWORD nPitch);
	BOOL Dec1Frame_G723 ();
	// member functions[E]

	// Friend function[S]
	friend UINT WINAPI ThFn_PlayerMain (LPVOID pParam);
	friend UINT WINAPI ThFn_VideoBufferingExec (LPVOID pParam);
	friend UINT WINAPI ThFn_AudioBufferingExec (LPVOID pParam);
	friend UINT WINAPI ThFn_HandleReqPause (LPVOID pParam);
	friend void CALLBACK CbFn_PlayerTimerProc (UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
	friend BOOL Decode_MP4 (CPlayerManager *pPlayer);
	friend BOOL Decode_JPEG (CPlayerManager *pPlayer);
	friend BOOL Decode_G723 (CPlayerManager *pPlayer);
	friend BOOL CALLBACK CbFn_WaveOut (HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
	// Friend function[E]
};

#endif // !defined(AFX_PLAYERMANAGER_H__88AFDD34_7BA9_48DE_ABDB_6C1F46C7C4FB__INCLUDED_)
