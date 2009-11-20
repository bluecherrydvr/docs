/************************************************************************
 *
 * Copyright 2008 Stretch, Inc. All rights reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 * STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 * THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.
 *
 ************************************************************************/


#pragma once

#include "ChildView.h"

// user message
#define WM_PLAYER_EVENT		(WM_USER + 100)
#define WM_READER_UI		(WM_USER + 201)
#define WM_READER_TEST		(WM_USER + 202)

const int STREAM_BUF_SIZE = 1024 * 1024;
const unsigned short MAX_VOLUME = 10000;
const unsigned short MIN_VOLUME = 0;

class CZoomWindow : public CFrameWnd
{
public:
	CZoomWindow(CWnd *pParent, CString title, int iPort, int iRegion)
	{
		m_pParent = pParent;
		m_iPort = iPort;
		m_iRegion = iRegion;
		Create(NULL, title,
			   WS_OVERLAPPEDWINDOW,
			   CRect(100, 100, 500, 400), pParent, NULL,
			   WS_EX_TOOLWINDOW);
	}
	~CZoomWindow()
	{
		splayer_set_display_region(m_iPort, m_iRegion, NULL, NULL, false);
	}

	CWnd	*m_pParent;
	int		m_iPort;
	int		m_iRegion;

// Generated message map functions
protected:
	afx_msg void OnClose();

	DECLARE_MESSAGE_MAP()
};

class CChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	enum Mode {UNKNOWN, FILE_MODE, STREAM_MODE, DECODE_MODE};
	enum StreamPlayState {IDLE, PREBUFFER, PLAY, PAUSE};

	CChildFrame();

// Attributes
public:
	void SetPortNumber(int nPort) {m_nPort = nPort;}
	int  GetBufferUsage();
	Mode GetMode() {return m_eMode;}
	float GetPlaySpeed();

// Operations
public:
	bool OpenFile(LPCSTR lpFileName);
	bool OpenStream(LPCSTR lpFileName);
	void Close();
	void CloseStream();
	bool Play();
	bool Stop();
	bool Pause();
	bool FastForward();
	bool FastRewind();
	bool StepForward();
	bool StepBackward();
	bool CaptureFrame();
	bool GetPlayPos(float *pfRelativePos);
	bool SetPlayPos(float fRelativePos);
	bool GetFileDuration(int *pDuration);
	bool GetTotalFrames(int *pFrames);
	bool GetCurrentFrameIndex(int *pIndex);
	bool Zoom();

	bool Mute(bool bMute);
	bool UpdateUI();
	void RefreshDisplay();
	int GetPortNumber() {return m_nPort;}
	void ToggleDecode();

	void HandleDecodedFrame(splayer_frame_t *frame);

// Overrides
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Implementation
public:
	// view for the client area of the frame.
	CChildView m_wndView;
	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// reader thread
	friend DWORD WINAPI ReaderThread( LPVOID lpParam );

private:
	int			m_nPort;
	// for streaming
	Mode			m_eMode;
	StreamPlayState	m_eState;
	FILE		*m_pInputFile;		// input file pointer
	HANDLE		m_hThread;			// thread handle
	DWORD		m_dwThreadId;		// thread id
	bool		m_bAbort;			// abort flag
	int			m_iBufUsage;		// buffer usage in percentage
	FILE		*m_pfVOut;			// video output file
	FILE		*m_pfAOut;			// audio output file
	int			m_iFrameIndex;		// frame index
	CZoomWindow *m_pZoomWindow;

	int ReadInput();

// Generated message map functions
protected:
	afx_msg void OnFileClose();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg LRESULT OnPlayerEvent(WPARAM, LPARAM);
	afx_msg LRESULT OnCloseZoomWindow(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
};
