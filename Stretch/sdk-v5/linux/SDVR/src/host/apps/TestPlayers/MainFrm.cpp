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

#include "MainFrm.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int STATUS_INDEX_FRAME = 1;
const int STATUS_INDEX_FILE = 2;
const int STATUS_INDEX_SPEED = 3;

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_HSCROLL()
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnToolbarUpdateOpen)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPENSTREAM, OnToolbarUpdateOpenStream)
	ON_UPDATE_COMMAND_UI(ID_ACTION_PLAY, OnToolbarUpdatePlay)
	ON_UPDATE_COMMAND_UI(ID_ACTION_PAUSE, OnToolbarUpdatePause)
	ON_UPDATE_COMMAND_UI(ID_ACTION_STOP, OnToolbarUpdateStop)
	ON_UPDATE_COMMAND_UI(ID_ACTION_FASTFWD, OnToolbarUpdateFastFwd)
	ON_UPDATE_COMMAND_UI(ID_ACTION_FASTREV, OnToolbarUpdateFastRew)
	ON_UPDATE_COMMAND_UI(ID_ACTION_STEPFWD, OnToolbarUpdateStepFwd)
	ON_UPDATE_COMMAND_UI(ID_ACTION_STEPBWD, OnToolbarUpdateStepBwd)
	ON_UPDATE_COMMAND_UI(ID_ACTION_CAPTURE, OnToolbarUpdateCapture)
	ON_UPDATE_COMMAND_UI(ID_ACTION_SETDECODE, OnToolbarUpdateDecode)
	ON_UPDATE_COMMAND_UI(ID_ACTION_ZOOM, OnToolbarUpdateZoom)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_nTimer = 0;
	m_bEnableOpen = false;
	m_bEnableOpenStream = false;
	m_bEnablePlay = false;
	m_bEnablePause = false;
	m_bEnableStop = false;
	m_bEnableFastFwd = false;
	m_bEnableFastRew = false;
	m_bEnableStepFwd = false;
	m_bEnableStepBwd = false;
	m_bEnableCapture = false;
	m_bEnableDecode = false;
	m_bEnableZoom = false;
}

CMainFrame::~CMainFrame()
{
	if (m_nTimer)
		KillTimer(m_nTimer);
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// create tool bar
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	// create slider
	int index = m_wndToolBar.GetToolBarCtrl().GetButtonCount() - 1;
	m_wndToolBar.SetButtonInfo(index, ID_SLIDER, TBBS_SEPARATOR, 500);

	CRect rect;
	m_wndToolBar.GetItemRect(index, &rect);
	rect.left += 10;
	rect.top = 3;
	rect.bottom = rect.top + 16;
	if (!m_slider.Create(TBS_HORZ | WS_VISIBLE | WS_TABSTOP | TBS_AUTOTICKS,
		rect, this, ID_SLIDER))
	{
		TRACE0("Failed to create combo-box\n");
		return FALSE;
	}
	m_slider.SetRangeMin(0);
	m_slider.SetRangeMax(100);
	m_slider.SetTicFreq(10);

	// Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	// create status bar
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	int cxWidth;
	UINT nID, nStyle;
	m_wndStatusBar.GetPaneInfo(STATUS_INDEX_FRAME, nID, nStyle, cxWidth);
	m_wndStatusBar.SetPaneInfo(STATUS_INDEX_FRAME, nID, nStyle, 100);
	m_wndStatusBar.GetPaneInfo(STATUS_INDEX_FILE, nID, nStyle, cxWidth);
	m_wndStatusBar.SetPaneInfo(STATUS_INDEX_FILE, nID, nStyle, 200);
	m_wndStatusBar.GetPaneInfo(STATUS_INDEX_SPEED, nID, nStyle, cxWidth);
	m_wndStatusBar.SetPaneInfo(STATUS_INDEX_SPEED, nID, nStyle, 100);

	// set UI timer
	m_nTimer = SetTimer(1, 200, 0);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// Modify the Window class or styles here by modifying
	// the CREATESTRUCT cs

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnClose()
{
	CTestPlayersApp* pApp = (CTestPlayersApp *)AfxGetApp();
	pApp->Close();
	CMDIFrameWnd::OnClose();
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIFrameWnd::OnSize(nType, cx, cy);
	MDITile(MDITILE_HORIZONTAL);
}

void CMainFrame::OnTimer(UINT nIDEvent)
{
	CChildFrame* pChild = (CChildFrame *)MDIGetActive();
	if (pChild != NULL) {
		// update slider
		float fPos;
		pChild->GetPlayPos(&fPos);
		m_slider.SetPos((int)(fPos * 100.0));

		// read player state
		pChild->UpdateUI();

		// update toolbar buttons
		CToolBarCtrl& ctl = m_wndToolBar.GetToolBarCtrl();
		ctl.EnableButton(ID_FILE_OPEN, m_bEnableOpen);
		ctl.EnableButton(ID_FILE_OPENSTREAM, m_bEnableOpenStream);
		ctl.EnableButton(ID_ACTION_PLAY, m_bEnablePlay);
		ctl.EnableButton(ID_ACTION_PAUSE, m_bEnablePause);
		ctl.EnableButton(ID_ACTION_STOP, m_bEnableStop);
		ctl.EnableButton(ID_ACTION_PLAY, m_bEnableDecode);

		// update status bar
		CString text;
		if (pChild->GetMode() == CChildFrame::UNKNOWN) {
			text.Format("Not open");
			m_wndStatusBar.SetPaneText(STATUS_INDEX_FILE, text);
			text.Format("Speed = 0.0");
			m_wndStatusBar.SetPaneText(STATUS_INDEX_SPEED, text);
		}
		else if (pChild->GetMode() == CChildFrame::FILE_MODE
			|| pChild->GetMode() == CChildFrame::DECODE_MODE) 
		{
			int time, frames, index;
			pChild->GetCurrentFrameIndex(&index);
			text.Format("Frame: %d", index);
			m_wndStatusBar.SetPaneText(STATUS_INDEX_FRAME, text);
			pChild->GetFileDuration(&time);
			pChild->GetTotalFrames(&frames);
			if (pChild->GetMode() == CChildFrame::FILE_MODE)
				text.Format("File mode, %5.1f sec, %d frames", (double)time / 1000.0, frames);
			else if (pChild->GetMode() == CChildFrame::DECODE_MODE)
				text.Format("Decode mode, %5.1f sec, %d frames", (double)time / 1000.0, frames);
			m_wndStatusBar.SetPaneText(STATUS_INDEX_FILE, text);
			text.Format("Speed = %4.3f", pChild->GetPlaySpeed());
			m_wndStatusBar.SetPaneText(STATUS_INDEX_SPEED, text);
		}
		else if (pChild->GetMode() == CChildFrame::STREAM_MODE) {
			text.Format("Buffer mode: %d%%", pChild->GetBufferUsage());
			m_wndStatusBar.SetPaneText(STATUS_INDEX_FILE, text);
			text.Format("Speed = %3.1f", pChild->GetPlaySpeed());
			m_wndStatusBar.SetPaneText(STATUS_INDEX_SPEED, text);
		}
	}

	CMDIFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::OnToolbarUpdateOpen(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bEnableOpen);
}

void CMainFrame::OnToolbarUpdateOpenStream(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bEnableOpenStream);
}

void CMainFrame::OnToolbarUpdatePlay(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bEnablePlay);
}

void CMainFrame::OnToolbarUpdatePause(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bEnablePause);
}

void CMainFrame::OnToolbarUpdateStop(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bEnableStop);
}

void CMainFrame::OnToolbarUpdateFastFwd(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bEnableFastFwd);
}

void CMainFrame::OnToolbarUpdateFastRew(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bEnableFastRew);
}

void CMainFrame::OnToolbarUpdateStepFwd(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bEnableStepFwd);
}

void CMainFrame::OnToolbarUpdateStepBwd(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bEnableStepBwd);
}

void CMainFrame::OnToolbarUpdateCapture(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bEnableCapture);
}

void CMainFrame::OnToolbarUpdateDecode(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bEnableDecode);
}

void CMainFrame::OnToolbarUpdateZoom(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bEnableZoom);
}

void CMainFrame::OnHScroll(UINT nSBCode, UINT nPos,	CScrollBar* pScrollBar)
{
	CChildFrame* pChild = (CChildFrame *)MDIGetActive();
	if (pChild == NULL)
		return;

	int pos;
	switch (nSBCode) {
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		pos = nPos;
		break;
	case SB_LEFT:
	case SB_RIGHT:
	case SB_LINELEFT:
	case SB_LINERIGHT:
	case SB_PAGELEFT:
	case SB_PAGERIGHT:
		pos = m_slider.GetPos();    
		break;
	default:
		return;
	}

	int min, max;
	m_slider.GetRange(min, max);
	pChild->SetPlayPos((float)pos / float(max - min));
	return;
}

void CMainFrame::UpdateUIState(splayer_state_e eState)
{
	CChildFrame* pChild = (CChildFrame *)MDIGetActive();
	float fSpeed;

	switch (eState) {
	case SPLAYER_EMPTY:
		m_bEnableOpen = true;
		m_bEnableOpenStream = true;
		m_bEnablePlay = false;
		m_bEnablePause = false;
		m_bEnableStop = false;
		m_bEnableFastFwd = false;
		m_bEnableFastRew = false;
		m_bEnableStepFwd = false;
		m_bEnableStepBwd = false;
		m_bEnableCapture = false;
		m_bEnableDecode = false;
		m_bEnableZoom = false;
		break;
	case SPLAYER_READY:
		m_bEnableOpen = true;
		m_bEnableOpenStream = true;
		m_bEnablePlay = true;
		m_bEnablePause = false;
		m_bEnableStop = false;
		m_bEnableFastFwd = false;
		m_bEnableFastRew = false;
		m_bEnableStepFwd = false;
		m_bEnableStepBwd = false;
		m_bEnableCapture = false;
		if (pChild->GetMode() == CChildFrame::FILE_MODE 
			|| pChild->GetMode() == CChildFrame::DECODE_MODE)
			m_bEnableDecode = true;
		m_bEnableZoom = false;
		break;
	case SPLAYER_PLAY:
		fSpeed = pChild->GetPlaySpeed();
		m_bEnableOpen = false;
		m_bEnableOpenStream = false;
		if (fSpeed == 1.0)
			m_bEnablePlay = false;
		else
			m_bEnablePlay = true;
		m_bEnablePause = true;
		m_bEnableStop = true;
		if (pChild->GetMode() == CChildFrame::FILE_MODE) {
			m_bEnableFastFwd = true;
			m_bEnableFastRew = true;
		}
		m_bEnableStepFwd = false;
		m_bEnableStepBwd = false;
		m_bEnableCapture = true;
		m_bEnableDecode = false;
		m_bEnableZoom = true;
		break;
	case SPLAYER_STOP:
		m_bEnableOpen = true;
		m_bEnableOpenStream = true;
		m_bEnablePlay = true;
		m_bEnablePause = false;
		m_bEnableStop = false;
		m_bEnableFastFwd = false;
		m_bEnableFastRew = false;
		m_bEnableStepFwd = false;
		m_bEnableStepBwd = false;
		m_bEnableCapture = false;
		if (pChild->GetMode() == CChildFrame::FILE_MODE 
			|| pChild->GetMode() == CChildFrame::DECODE_MODE)
			m_bEnableDecode = true;
		m_bEnableZoom = false;
		break;
	case SPLAYER_PAUSE:
		m_bEnableOpen = false;
		m_bEnableOpenStream = false;
		m_bEnablePlay = true;
		m_bEnablePause = false;
		m_bEnableStop = true;
		m_bEnableFastFwd = false;
		m_bEnableFastRew = false;
		if (pChild->GetMode() == CChildFrame::FILE_MODE) {
			m_bEnableStepFwd = true;
			m_bEnableStepBwd = true;
		}
		m_bEnableCapture = true;
		m_bEnableDecode = false;
		m_bEnableZoom = true;
		break;
	}
}
