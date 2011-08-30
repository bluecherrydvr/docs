#pragma once

#include "Solo6010_ExRecFileInfo.h"

#define RM_DEF_TIMER_INTERVAL		500

class CRecManager  
{
public:
	CRecManager();
	virtual ~CRecManager();

	void Init (CSOLO6010 *pS6010,
		BYTE modeRecMP4, BYTE modeRecJPEG, BYTE modeRecG723,
		BYTE nNotifyBufLevelMP4, BYTE nNotifyBufLevelJPEG, BYTE nNotifyBufLevelG723);
	BOOL StartRec (SETTING_REC *pSetRec);
	BOOL StopRec ();
	BOOL EndRec ();

private:
	static BOOL m_staBInitCrtFileAcc;
	static CRITICAL_SECTION m_staCrtFileAcc;

	CSOLO6010 *m_pS6010;
	BOOL m_bRec;

	HANDLE m_bufHFileMP4[NUM_VID_TOTAL_ENC_CH];
	HANDLE m_bufHFileJPEG[NUM_VID_TOTAL_ENC_CH];
	HANDLE m_bufHFileG723[NUM_G723_CH];

	BYTE m_modeRecMP4;
	BYTE m_modeRecJPEG;
	BYTE m_modeRecG723;

	UINT m_nMMTimerID;
	INFO_DATE m_dateRecStop;

	void OpenMP4File (INFO_DATE *pInfoDate, SETTING_REC *pSetRec, int idxModeRec);
	void CloseMP4File (INFO_DATE *pInfoDate);

	void OpenJPEGFile (INFO_DATE *pInfoDate, SETTING_REC *pSetRec, int idxModeRec);
	void CloseJPEGFile (INFO_DATE *pInfoDate);

	void OpenG723File (INFO_DATE *pInfoDate, SETTING_REC *pSetRec, int idxModeRec);
	void CloseG723File (INFO_DATE *pInfoDate);


	// MP4[S]
	// MP4[E]

	// JPEG[S]

	// JPEG[E]

	// G723[S]
	SETTING_G723_REC m_settingG723Rec;
	// G723[E]

	// Encoder OSD[S]
	BOOL m_bufBRecCh[NUM_VID_TOTAL_ENC_CH];

	void UpdateEncoderOSD ();
	// Encoder OSD[E]

	// Friend[S]
	friend void CALLBACK CbFn_MP4CaptureRawData (void *pContext, BYTE *bufMP4, DWORD posStart, DWORD posEnd, DWORD szBuf, DWORD flagError);
	friend void CALLBACK CbFn_JPEGCaptureRawData (void *pContext, BYTE *bufJPEG, DWORD posStart, DWORD posEnd, DWORD szBuf, DWORD flagError);
	friend void CALLBACK CbFn_G723CaptureRawData (void *pContext, int idxCh, BYTE *bufG723, DWORD szBuf, DWORD flagError);
	friend void CALLBACK CbFn_MP4CaptureFrame (void *pContext, BYTE *bufMP4, DWORD posStart, DWORD posEnd, DWORD szBuf, DWORD flagError);
	friend void CALLBACK CbFn_JPEGCaptureFrame (void *pContext, BYTE *bufJPEG, DWORD posStart, DWORD posEnd, DWORD szBuf, DWORD flagError);
	friend void CALLBACK CbFn_G723CaptureFrame (void *pContext, int idxCh, BYTE *bufG723, DWORD szBuf, DWORD flagError);
	friend void CALLBACK CbFn_RecManagerTimerProc (UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
	// Friend[S]
};
