/************************************************************************
 *
 * Copyright 2008 Stretch, Inc. All rights reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 * STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 * THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.
 *
 ************************************************************************/

#include "stdafx.h"
#include "TestPlayers.h"

#include "ChildFrm.h"
#include "MainFrm.h"
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BE32(a, b, c, d) (((a) << 24) + ((b) << 16) + ((c) << 8) + (d))
#define LE32(a, b, c, d) (((d) << 24) + ((c) << 16) + ((b) << 8) + (a))
#define BE2LE32(a) ((((a) & 0xff) << 24) + (((a) & 0xff00) << 8) + (((a) & 0xff0000) >> 8) + (((a) & 0xff000000) >> 24))

#define WM_CLOSE_ZOOM_WINDOW	(WM_USER + 2001)


// CZoomWindow

BEGIN_MESSAGE_MAP(CZoomWindow, CFrameWnd)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

void CZoomWindow::OnClose()
{
	m_pParent->SendMessage(WM_CLOSE_ZOOM_WINDOW);
}


// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_WM_SETFOCUS()
	ON_WM_CREATE()
	ON_WM_MDIACTIVATE()
	ON_MESSAGE(WM_PLAYER_EVENT, OnPlayerEvent)
	ON_MESSAGE(WM_CLOSE_ZOOM_WINDOW, OnCloseZoomWindow)
END_MESSAGE_MAP()


// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	m_eMode = UNKNOWN;
	m_nPort = -1;
	m_pInputFile = NULL;
	m_iBufUsage = 0;
	m_hThread = NULL;
	m_pfVOut = NULL;
	m_pfAOut = NULL;
	m_pZoomWindow = NULL;
}

CChildFrame::~CChildFrame()
{
	if (m_nPort != -1)
		splayer_close_file(m_nPort);
}


BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// Modify the Window class or styles here by modifying the CREATESTRUCT cs
	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	cs.style &= ~WS_SYSMENU;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.style &= ~(LONG)FWS_ADDTOTITLE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}


// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG


// CChildFrame message handlers
void CChildFrame::OnFileClose() 
{
	Close();
	// To close the frame, just send a WM_CLOSE, which is the equivalent
	// choosing close from the system menu.
	SendMessage(WM_CLOSE);
}

int CChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, 
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	UpdateUI();
	return 0;
}

void CChildFrame::OnSetFocus(CWnd* pOldWnd) 
{
	CMDIChildWnd::OnSetFocus(pOldWnd);
	m_wndView.SetFocus();
}

BOOL CChildFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;
	
	// otherwise, do default handling
	return CMDIChildWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);
	if (m_nPort < 0)
		return;

	if (bActivate) {
		splayer_play_sound_share(m_nPort);
		UpdateUI();
	}
	else
		splayer_stop_sound_share(m_nPort);
}

bool CChildFrame::OpenFile(LPCSTR lpFileName)
{
	m_iFrameIndex = 0;
	if (splayer_open_file(m_nPort, (char *)lpFileName) != SPLAYER_NOERROR)
		return false;
	if (splayer_register_file_end_msg(m_nPort, m_hWnd, WM_PLAYER_EVENT) != SPLAYER_NOERROR)
		return false;

	m_eMode = FILE_MODE;
	SetWindowText(lpFileName);
	UpdateUI();
	return true;
}

void CChildFrame::Close() 
{
	if (m_nPort != -1) {
		Stop();
		splayer_close_file(m_nPort);
	}

	if (m_eMode == STREAM_MODE) {
		CloseStream();
	}
	else if (m_eMode == DECODE_MODE) {
		if (m_pfVOut != NULL) {
			fclose(m_pfVOut);
			m_pfVOut = NULL;
		}
		if (m_pfAOut != NULL) {
			fclose(m_pfAOut);
			m_pfAOut = NULL;
		}
	}
}

afx_msg LRESULT CChildFrame::OnPlayerEvent(WPARAM, LPARAM)
{
	DWORD ret = splayer_stop(m_nPort);
	ASSERT(ret == SPLAYER_NOERROR);
	UpdateUI();
	return 0;
}

bool CChildFrame::UpdateUI()
{
	CMainFrame *pFr = (CMainFrame *)GetMDIFrame();
	splayer_state_e state;
	if (splayer_get_state(m_nPort, &state) == SPLAYER_NOERROR)
		pFr->UpdateUIState(state);
	else
		pFr->UpdateUIState(SPLAYER_EMPTY);
	return true;
}

bool CChildFrame::Mute(bool bMute)
{
	if (m_nPort < 0)
		return false;
	if (bMute)
		splayer_stop_sound_share(m_nPort);
	else
		splayer_play_sound_share(m_nPort);
	return true;
}

bool CChildFrame::Play()
{
	if (m_eMode == STREAM_MODE) {
		m_eState = PREBUFFER;
	}

	if (m_eMode == FILE_MODE)
		splayer_play(m_nPort, m_wndView);
	else if (m_eMode == DECODE_MODE) {
		errno_t errno;
		if ((errno = fopen_s(&m_pfVOut, "vout.yuv", "wb")) != 0)
			m_pfVOut = NULL;
		if ((errno = fopen_s(&m_pfAOut, "aout.au", "wb")) != 0)
			m_pfAOut = NULL;

		if (m_pfAOut != NULL) {
			// write au header
			sx_int32 au_magic, au_offset, au_size, au_encoding, au_srate, au_channels;
			au_magic = LE32('.', 's', 'n', 'd');
			au_offset = BE2LE32(24);
			au_size = BE2LE32((sx_uint32)-1);
			au_encoding = BE2LE32(3);
			au_srate = BE2LE32(8000); // audio sampling rate
			au_channels = BE2LE32(2); // audio channels
			fwrite(&au_magic, 1, 4, m_pfAOut);
			fwrite(&au_offset, 1, 4, m_pfAOut);
			fwrite(&au_size, 1, 4, m_pfAOut);
			fwrite(&au_encoding, 1, 4, m_pfAOut);
			fwrite(&au_srate, 1, 4, m_pfAOut);
			fwrite(&au_channels, 1, 4, m_pfAOut);
		}

		splayer_play(m_nPort, m_wndView);
	}

	UpdateUI();
	return true;
}

bool CChildFrame::Stop()
{
	if (splayer_stop(m_nPort) == SPLAYER_NOERROR)
		UpdateUI();

	if (m_eMode == STREAM_MODE) {
		m_eState = IDLE;
		fseek(m_pInputFile, 0, SEEK_SET);
	}
	else if (m_eMode == DECODE_MODE) {
		if (m_pfVOut != NULL) {
			fclose(m_pfVOut);
			m_pfVOut = NULL;
		}
		if (m_pfAOut != NULL) {
			fclose(m_pfAOut);
			m_pfAOut = NULL;
		}
	}
	return true;
}

bool CChildFrame::Pause()
{
	if (m_eMode == STREAM_MODE) {
		m_eState = PAUSE;
	}

	if (splayer_pause(m_nPort, true) == SPLAYER_NOERROR)
		UpdateUI();
	return true;
}

bool CChildFrame::FastForward()
{
	splayer_double_speed(m_nPort);
	return true;
}

bool CChildFrame::FastRewind()
{
	splayer_halve_speed(m_nPort);
	return true;
}

bool CChildFrame::StepForward()
{
	splayer_next_frame(m_nPort);
	return true;
}

bool CChildFrame::StepBackward()
{
	splayer_prev_frame(m_nPort);
	return true;
}

static void _CaptureFrame(sx_int32 port, splayer_frame_t *frame)
{
	if (port >= 0 && frame != NULL) {
		// open output file
		errno_t errno;
		FILE *pf;
		CString text;
		if (frame->type == SPLAYER_FRAME_TYPE_YV12) {
			text.Format("port%02d_fr%06d.yuv", port, frame->id);
			if ((errno = fopen_s(&pf, text, "wb" )) == 0) {
				fwrite(frame->buf, 1, frame->size, pf);
				fclose(pf);
			}
		}
	}
	splayer_set_display_callback(port, NULL);
}

bool CChildFrame::CaptureFrame()
{
	splayer_err_e err = SPLAYER_NOERROR;
#if 0
	err = splayer_set_display_callback(m_nPort, _CaptureFrame);
#else
	splayer_frame_t frame;
	frame.alloc_size = 1024;
	frame.buf = new sx_uint8[frame.alloc_size];
	if (frame.buf != NULL) {
		err = splayer_take_video_snapshot(m_nPort, &frame);
		if (err == SPLAYER_SNAPSHOT_BUFFER_SIZE && frame.size > frame.alloc_size) {
			delete frame.buf;
			frame.buf = new sx_uint8[frame.size];
			if (frame.buf == NULL)
				return true;
			frame.alloc_size = frame.size;
			err = splayer_take_video_snapshot(m_nPort, &frame);
		}
		if (err == SPLAYER_NOERROR) {
			// open output file
			errno_t errno;
			FILE *pf;
			CString text;
			if (frame.type == SPLAYER_FRAME_TYPE_YV12) {
				text.Format("port%02d_fr%06d.yuv", m_nPort, frame.id);
				if ((errno = fopen_s(&pf, text, "wb" )) == 0) {
					fwrite(frame.buf, 1, frame.size, pf);
					fclose(pf);
				}
			}
		}
		delete frame.buf;
	}
	else
		return false;
#endif
	return true;
}

bool CChildFrame::GetFileDuration(int *pDuration)
{
	splayer_get_file_time(m_nPort, pDuration);
	return true;
}

bool CChildFrame::GetTotalFrames(int *pFrames)
{
	splayer_get_total_frames(m_nPort, pFrames);
	return true;
}

bool CChildFrame::GetCurrentFrameIndex(int *pIndex)
{
	if (m_eMode == DECODE_MODE)
		*pIndex = m_iFrameIndex;
	else
		splayer_get_played_frames(m_nPort, pIndex);
	return true;
}

bool CChildFrame::GetPlayPos(float *pfRelativePos)
{
	if (m_eMode == FILE_MODE && m_nPort >= 0) {
		if (splayer_get_play_pos(m_nPort, pfRelativePos) != SPLAYER_NOERROR)
			*pfRelativePos = 0.0;
	}
	else 
		*pfRelativePos = 0.0;
	return true;
}

bool CChildFrame::SetPlayPos(float fRelativePos)
{
	if (m_eMode == FILE_MODE && m_nPort >= 0)
		splayer_set_play_pos(m_nPort, fRelativePos);
	return true;
}

float CChildFrame::GetPlaySpeed()
{
	if (m_nPort >= 0) {
		float fSpeed;
		splayer_get_play_speed(m_nPort, &fSpeed);
		return fSpeed;
	}
	else
		return 0.0;
}

void CChildFrame::RefreshDisplay()
{
	if (m_nPort >= 0) {
		splayer_refresh_play(m_nPort);
	}
}


bool CChildFrame::OpenStream(LPCSTR lpFileName)
{
	CloseStream();
	m_iFrameIndex = 0;

	// open input file
	errno_t errno;
	if ((errno = fopen_s(&m_pInputFile, lpFileName, "rb" )) != 0)
		return false;

	// open stream player
	if (splayer_open_stream(m_nPort, STREAM_BUF_SIZE) != SPLAYER_NOERROR)
		return false;

	m_eMode = STREAM_MODE;
	m_eState = IDLE;

    // create thread
    m_hThread = CreateThread(NULL,		// security descriptor
                             0,			// stack size
                             ReaderThread,		// thread function
                             this,		// thread parameter
                             0,			// start thread immediately
                             &m_dwThreadId);	// thread id
    if (m_hThread == NULL) {
	    return false;
    }

	SetWindowText(lpFileName);
	UpdateUI();
	return true;
}

void CChildFrame::HandleDecodedFrame(splayer_frame_t *frame)
{
	if (frame->type == SPLAYER_FRAME_TYPE_YV12 && m_pfVOut != NULL) {
		fwrite(frame->buf, 1, frame->size, m_pfVOut);
		m_iFrameIndex = frame->id;
	}
	else if (frame->type == SPLAYER_FRAME_TYPE_AUDIO_S16LE && m_pfAOut != NULL) {
		for (int i=0; i<frame->size; i+=2) {
			fwrite(&frame->buf[i+1], 1, 1, m_pfAOut);
			fwrite(&frame->buf[i], 1, 1, m_pfAOut);
		}
	}
}

void CChildFrame::CloseStream()
{
	if (m_pInputFile != NULL) {
		fclose(m_pInputFile);
		m_pInputFile = NULL;
	}

	if (m_hThread != NULL) {
		m_bAbort = true;
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

int CChildFrame::ReadInput()
{
	splayer_err_e err;
	unsigned int size, size1 = 0;
	const int BUF_SIZE = 8192;
	unsigned char buf[BUF_SIZE];

	if (m_eState == IDLE)
		return 0;

	err = splayer_get_num_bytes_in_buffer(m_nPort, &size); // get used size
	if (err == SPLAYER_NOERROR && size < (STREAM_BUF_SIZE - BUF_SIZE)) {
		size1 = (int)fread(buf, 1, BUF_SIZE, m_pInputFile);
		err = splayer_input_data(m_nPort, buf, size1);
		assert(err == SPLAYER_NOERROR);
		m_iBufUsage = (size + size1) * 100 / STREAM_BUF_SIZE;
	}

	if (m_eState == PREBUFFER && size > STREAM_BUF_SIZE * 3 / 4) {
		splayer_play(m_nPort, m_wndView);
		m_eState = PLAY;
	}
	else if (m_eState == PLAY && size < STREAM_BUF_SIZE / 10) {
		splayer_pause(m_nPort, true);
		m_eState = PREBUFFER;
	}

	return size1;
}

int CChildFrame::GetBufferUsage() 
{
	if (m_eMode != STREAM_MODE || m_eState == IDLE)
		return 0;
	else
		return m_iBufUsage;
}

// player thread
DWORD WINAPI ReaderThread( LPVOID lpParam )
{
	CChildFrame &child = *((CChildFrame *)lpParam);
	child.m_bAbort = false;

	while (!child.m_bAbort) {
		// do reading
		int bytes = child.ReadInput();
		if (bytes > 0) {
			::Sleep(30);
		}
		else {
			::Sleep(100);
		}
	}

	return 0;
}

extern void DecodeFrameCallback(sx_int32 port, splayer_frame_t *frame);
void CChildFrame::ToggleDecode()
{
	if (m_eMode == FILE_MODE) {
		splayer_set_decode_callback(m_nPort, DecodeFrameCallback);
		m_eMode = DECODE_MODE;
	}
	else if (m_eMode == DECODE_MODE) {
		splayer_set_decode_callback(m_nPort, NULL);
		m_eMode = FILE_MODE;
	}
	UpdateUI();
}

bool CChildFrame::Zoom()
{
	CZoomDialog dlg;
	int width = 0, height = 0, win_id;
	dlg.SetLeft(0);
	dlg.SetTop(0);
	if (splayer_get_picture_size(m_nPort, &width, &height) == SPLAYER_NOERROR) {
		dlg.SetWidth(width);
		dlg.SetHeight(height);
	}
	if (dlg.DoModal() == IDOK) {
		sdvr_ui_region_t region;
		win_id = dlg.GetWindowId();
		region.top_left_x = dlg.GetLeft();
		region.top_left_y = dlg.GetTop();
		region.width = dlg.GetWidth();
		region.height = dlg.GetHeight();
		if (win_id == 0) {
			splayer_set_display_region(m_nPort, win_id, &region, NULL, false);
		}
		else {
			CString title;
			GetWindowText(title);
			if (m_pZoomWindow != NULL)
				delete m_pZoomWindow;
			m_pZoomWindow = new CZoomWindow(this, title, m_nPort, win_id);
			if (m_pZoomWindow != NULL) {
				m_pZoomWindow->ShowWindow(SW_NORMAL);
				splayer_set_display_region(m_nPort, win_id, &region, m_pZoomWindow->m_hWnd, true);
			}
		}
	}
	return true;
}

LRESULT CChildFrame::OnCloseZoomWindow(WPARAM, LPARAM)
{
	if (m_pZoomWindow != NULL) {
		delete m_pZoomWindow;
		m_pZoomWindow = NULL;
	}
	return 0;
}
