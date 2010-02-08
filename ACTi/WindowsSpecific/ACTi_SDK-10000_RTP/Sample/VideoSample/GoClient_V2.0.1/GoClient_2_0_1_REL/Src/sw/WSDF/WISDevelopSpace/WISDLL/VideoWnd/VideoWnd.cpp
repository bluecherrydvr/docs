// VideoWnd.cpp : implementation file
//

#include "stdafx.h"
#include "VideoWndApp.h"
#include "VideoWnd.h"
#include "DebugPrint.h"

#include "AboutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DWORD FourCCSupported[]={	
							MAKEFOURCC('U','Y','V','Y'),
							MAKEFOURCC('Y','U','Y','2'),
						};
int FourCCSupportedNum = sizeof(FourCCSupported)/sizeof(FourCCSupported[0]);

#pragma comment (lib,"ddraw.lib")

const GUID FAR IID_IDirectDraw7=
{0x15e65ec0,0x3b9c,0x11d2,0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b};

/////////////////////////////////////////////////////////////////////////////
// CVideoWnd

CVideoWnd::CVideoWnd()
{
	m_ontop = FALSE;
	m_cmdCallback = NULL;
	m_cmdParam = NULL;
	m_ppLevel = 0;
	m_bShowPopupMenu = TRUE;
	m_bInited = FALSE;
}

CVideoWnd::~CVideoWnd()
{
}


BEGIN_MESSAGE_MAP(CVideoWnd, CWnd)
	//{{AFX_MSG_MAP(CVideoWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_MOVE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_FULLSCREEN, OnFullscreen)
	ON_COMMAND(ID_ZOOM_HALF, OnZoomHalf)
	ON_COMMAND(ID_ZOOM_DOUBLE, OnZoomDouble)
	ON_COMMAND(ID_ZOOM_ORIGINAL, OnZoomOriginal)
	ON_COMMAND(ID_RATIO_KEEP, OnRatioKeep)
	ON_COMMAND(ID_RATIO_43, OnRatio43)
	ON_COMMAND(ID_RATIO_WIDE, OnRatioWide)
	ON_COMMAND(ID_RATIO_FREE, OnRatioFree)
	ON_COMMAND(ID_ALWAYSONTOP, OnAlwaysontop)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_KEYUP()
	ON_COMMAND(ID_RATIO_THIN, OnRatioThin)
	ON_COMMAND(ID_VW_ABOUT, OnVwAbout)
	ON_COMMAND(ID_PP_NONE, OnPpNone)
	ON_COMMAND(ID_PP_SLIGHT, OnPpSlight)
	ON_COMMAND(ID_PP_MEDIUM, OnPpMedium)
	ON_COMMAND(ID_PP_HEAVY, OnPpHeavy)
	ON_COMMAND(ID_PP_SUPERHEAVY, OnPpSuperheavy)
	ON_COMMAND(ID_SNAPSHOT, OnSnapshot)
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CVideoWnd message handlers

void CVideoWnd::OnPaint() 
{
	CPaintDC dc(this);
	RECT r;
	GetClientRect(&r);

	dc.FillSolidRect(&r, RGB(10, 0, 10)); //DD_OVERLAY_COLORREF); 
//	dc.FillSolidRect(&r,RGB(0xC0,0xC0,0xC0));	

	// Do not call CWnd::OnPaint() for painting messages
}

void CVideoWnd::OnMove(int x, int y) 
{
	CWnd::OnMove(x, y);
	
	RECT r;
	GetClientRect(&r);
	m_render.Display(r.left, r.top, r.right - r.left, r.bottom - r.top);
}

void CVideoWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	if(nType != SIZE_MINIMIZED)
	{
		RECT r;
		GetClientRect(&r);
		m_render.Display(r.left, r.top, r.right - r.left, r.bottom - r.top);
	}
}

void CVideoWnd::CreateWnd(CWnd *pParentWnd, int windowStyle)
{
	m_pParentWnd = pParentWnd;

	m_BorderSizeX = GetSystemMetrics(SM_CXBORDER) + GetSystemMetrics(SM_CXEDGE);
	m_BorderSizeY = GetSystemMetrics(SM_CYBORDER) + GetSystemMetrics(SM_CYEDGE);
	m_BorderSizeX *= 2; m_BorderSizeY *= 2;
	if (windowStyle & (WS_CAPTION|WS_SYSMENU))
		m_CaptionHeight = GetSystemMetrics(SM_CYCAPTION);
	else
		m_CaptionHeight = 0;
	m_windowStyle = windowStyle;
	RECT rect={	0,
				0,
				100 + m_BorderSizeX,
				100 + m_BorderSizeY + m_CaptionHeight};

// 	CreateEx(0, AfxRegisterWndClass(CS_NOCLOSE|CS_HREDRAW | CS_VREDRAW | CS_PARENTDC), "VideoWindow",
//	 m_windowStyle, rect, pParentWnd, 0, NULL);
	if(windowStyle & WS_CHILD)
		CreateEx(0, AfxRegisterWndClass(CS_NOCLOSE|CS_HREDRAW | CS_VREDRAW), "VideoWindow",
			m_windowStyle, CRect(26, 40, 746, 540), pParentWnd, NULL);
	else
		CreateEx(0, AfxRegisterWndClass(CS_NOCLOSE|CS_HREDRAW | CS_VREDRAW | CS_PARENTDC), "VideoWindow",
			m_windowStyle, rect, pParentWnd, 0, NULL);
}

void CVideoWnd::CreateRender(int VideoWidth, int VideoHeight, int preferredMode)
{
	m_VideoWidth = VideoWidth;
	m_VideoHeight = VideoHeight;
	m_videoMode = preferredMode;

	RECT cr, wr;
	GetClientRect(&cr);
	GetWindowRect(&wr);
	m_BorderSizeX = (wr.right - wr.left) - (cr.right - cr.left);
	m_BorderSizeY = (wr.bottom - wr.top) - (cr.bottom - cr.top);
	wr.right = wr.left + m_BorderSizeX + VideoWidth;
	wr.bottom = wr.top + m_BorderSizeY + VideoHeight;
	MoveWindow(&wr);
	if((m_windowStyle & WS_CHILD) == 0)
	ShowWindow(SW_SHOW);

	if (m_render.Init(this->m_hWnd, m_VideoWidth, m_VideoHeight, m_videoMode))
	{
		m_bInited = TRUE;
		m_videoMode = m_render.GetVideoMode();
		m_bIsSurface = ((m_videoMode == DDRAW_DIB24 || m_videoMode == DDRAW_DIB32) ? FALSE : TRUE);
		m_pDDBuf = m_render.BeginDDPaint(m_Pitch); 
		int i;
		switch(m_videoMode)
		{
		case DDRAW_UYVY: // UYVY
			for(i = 0; i < m_VideoWidth; i++)
			{
				m_pDDBuf[2 * i] = 128;
				m_pDDBuf[2 * i + 1] = 0;
			}
			for(i = 1; i < m_VideoHeight; i++)
				memcpy(m_pDDBuf + i * m_Pitch, m_pDDBuf, m_VideoWidth * 2);

			break;

		case DDRAW_YUY2: // YUY2
			for(i = 0; i < m_VideoWidth; i++)
			{
				m_pDDBuf[2 * i] = 0;
				m_pDDBuf[2 * i + 1] = 128;
			}
			for(i = 1; i < m_VideoHeight; i++)
				memcpy(m_pDDBuf + i * m_Pitch, m_pDDBuf, m_VideoWidth * 2);

			break;
		default:
			memset(m_pDDBuf, 0, m_Pitch * m_VideoHeight);
			break;
		}

		m_render.EndDDPaint(1);

		GetClientRect(&cr);
		m_render.Display(cr.left, cr.top, cr.right - cr.left, cr.bottom - cr.top);
	}

	CString s;
	s.Format("VideoWindow -- %d x %d (", m_VideoWidth, m_VideoHeight);

	if(m_videoMode == DDRAW_NULL) 
		s += "DIB24";
	else if(m_videoMode == DDRAW_DIB24) 
		s += "DIB24";
	else if(m_videoMode == DDRAW_DIB32) 
		s += "DIB32";
	else if(m_videoMode == DDRAW_UYVY)
		s += "UYVY";
	else if(m_videoMode == DDRAW_YUY2)
		s += "YUY2";
	else if(m_videoMode == DDRAW_RGB24)
		s += "RGB24";
	else if(m_videoMode == DDRAW_RGB32)
		s += "RGB32";

	s += ")";

	SetWindowText(s);
}

DWORD CVideoWnd::GetVideoMode()
{ 
	return m_videoMode; 
}

int CVideoWnd::GetPitch()
{
	return m_Pitch; 
};


int CVideoWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;


	return 0;
}

unsigned char *CVideoWnd::BeginDraw()
{
	m_pDDBuf = m_render.BeginDDPaint(m_Pitch); 
//	if(m_pDDBuf == NULL)
	{
//		m_render.ShowUpdate();
//		return NULL;
	}
	
	return m_pDDBuf;
}

void CVideoWnd::EndDraw(BOOL draw)
{
	m_render.EndDDPaint(draw);
}

void CVideoWnd::OnDestroy() 
{
	CWnd::OnDestroy();
	
	Close();
}

void CVideoWnd::Close()
{
	m_render.Close();
	m_bInited = FALSE;
}

void CVideoWnd::SetFullScreen(BOOL bFullScreen)
{
	if (!IsFullScreen())
		GetWindowRect(&m_rect);

	if (m_render.GetFullScreenMode() == bFullScreen) 
		return;

	if (bFullScreen)
	{
		if (m_render.InitFullscreen(this->m_hWnd, m_VideoWidth, m_VideoHeight, m_videoMode) == FALSE)
		{
			PrintString("Initilize full screen failed");
			if (m_render.Init(this->m_hWnd, m_VideoWidth, m_VideoHeight, m_videoMode))
				m_videoMode = m_render.GetVideoMode();
			else
				m_videoMode = DDRAW_NULL;
		}
		else 
			m_videoMode = m_render.GetVideoMode();
 
		m_windowStyle = ::GetWindowLong(this->m_hWnd, GWL_STYLE);
		::SetWindowLong(this->m_hWnd, GWL_STYLE, WS_VISIBLE|WS_POPUP|WS_SIZEBOX);
		::SetWindowPos(this->m_hWnd, (HWND)NULL,  m_rect.left, m_rect.top, 
			m_rect.right - m_rect.left, m_rect.bottom - m_rect.top, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		int screenX = GetSystemMetrics(SM_CXSCREEN);
		int screenY = GetSystemMetrics(SM_CYSCREEN);
		::MoveWindow(this->m_hWnd, -5, -5, screenX + 20, screenY + 50, TRUE);
		PrintString("Set Full screen: End");
	}
	else
	{
		if (m_render.Init(this->m_hWnd, m_VideoWidth, m_VideoHeight, m_videoMode))
			m_videoMode = m_render.GetVideoMode();
		else
			m_videoMode = DDRAW_NULL;

		::SetWindowLong(this->m_hWnd, GWL_STYLE, m_windowStyle);
		::SetWindowPos(this->m_hWnd, NULL,  m_rect.left, m_rect.top, 
			m_rect.right - m_rect.left, m_rect.bottom - m_rect.top, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		::MoveWindow(this->m_hWnd, m_rect.left, m_rect.top, m_rect.right - m_rect.left, m_rect.bottom - m_rect.top, TRUE);
		PrintString("Restore to normal mode");
	}
}

BOOL CVideoWnd::IsFullScreen()
{
	return m_render.GetFullScreenMode();
}

void CVideoWnd::Zoom(double zoomX, double zoomY)
{
	if (!IsFullScreen())
	{
		GetWindowRect(&m_rect);
		int w = (int)(m_VideoWidth * zoomX) + m_BorderSizeX;
		int h = (int)(m_VideoHeight * zoomY) + m_BorderSizeY; // + m_CaptionHeight;
		m_rect.right = m_rect.left + w;
		m_rect.bottom = m_rect.top + h;
		::MoveWindow(this->m_hWnd, m_rect.left, m_rect.top, w, h, TRUE);
	}
}

void CVideoWnd::SetCallback(int callback, int param)
{
	m_cmdCallback = (COMMANDCALLBACK)callback;
	m_cmdParam = param;
}

void CVideoWnd::SetWindowStyle(int style)
{
	RECT r;
	GetWindowRect(&r);

	::SetWindowLong(this->m_hWnd, GWL_STYLE, style);
	::SetWindowPos(this->m_hWnd, NULL, r.left, r.top, 
			r.right - r.left, r.bottom - r.top, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	m_windowStyle = style;
}


void CVideoWnd::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	// TODO: Add your message handler code here and/or call default
	
//	lpMMI->ptMaxPosition.x = -50;
//	lpMMI->ptMaxPosition.y = -50;
//	lpMMI->ptMaxSize.x += 50;
//	lpMMI->ptMaxSize.y += 50;
//	lpMMI->ptMaxTrackSize.x  += 50;
//	lpMMI->ptMaxTrackSize.y += 50;
	CWnd::OnGetMinMaxInfo(lpMMI);
}

void CVideoWnd::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CWnd::OnRButtonUp(nFlags, point);

	if (!m_bShowPopupMenu || !m_bInited)
		return;
	CMenu	m_menu;
	m_menu.LoadMenu(IDR_MENU_VIDEOWINDOW);

	CMenu *pmenuPopup = m_menu.GetSubMenu(0);

	ClientToScreen(&point);

	MENUITEMINFO item;

	item.cbSize = sizeof(MENUITEMINFO);
	item.fMask = MIIM_TYPE;;
	item.fType = MFT_STRING;
	if (IsFullScreen())
		item.dwTypeData = "Normal mode";
	else
		item.dwTypeData = "Full screen mode";
	SetMenuItemInfo(pmenuPopup->m_hMenu, ID_FULLSCREEN, FALSE, &item);

	item.fMask = MIIM_STATE;
	item.fState =  m_ontop ? MFS_CHECKED : MFS_UNCHECKED;
	SetMenuItemInfo(pmenuPopup->m_hMenu, ID_ALWAYSONTOP, FALSE, &item);
	
	int id = 0, aspectX = 0, aspectY = 0;
	m_render.GetAspectRatio(aspectX, aspectY);
	if (aspectX == 0)
		id = ID_RATIO_FREE;
	else if (aspectX == m_VideoWidth && aspectY == m_VideoHeight)
		id = ID_RATIO_KEEP;
	else if (aspectX == 4 && aspectY == 3)
		id = ID_RATIO_43;
	else if (aspectX == 16 && aspectY == 9)
		id = ID_RATIO_WIDE;
	else if (aspectX == 9 && aspectY == 16)
		id = ID_RATIO_THIN;
	item.fState = MFS_CHECKED;
	if (id != 0)
		SetMenuItemInfo(pmenuPopup->m_hMenu, id, FALSE, &item);
	if (!IsFullScreen())
	{
		RECT r;
		GetClientRect(&r);
		char ssss[128];
		sprintf(ssss, "ClientRect: %d, %d, %d, %d", r.left, r.top, r.right, r.bottom);
		OutputDebugString(ssss);
		if (ABS(r.right - m_VideoWidth) < 3 && ABS(r.bottom - m_VideoHeight) < 3)
			id = ID_ZOOM_ORIGINAL;
		else if (ABS(r.right - m_VideoWidth * 2) < 3 && ABS(r.bottom - m_VideoHeight * 2) < 3)
			id = ID_ZOOM_DOUBLE;
		else if (ABS(r.right * 2 - m_VideoWidth) < 5 && ABS(r.bottom * 2 - m_VideoHeight) < 5)
			id = ID_ZOOM_HALF;
		else id = 0;
		if (id != 0)
			SetMenuItemInfo(pmenuPopup->m_hMenu, id, FALSE, &item);
	}

	if (m_cmdCallback)
	{
		if (m_ppLevel == DPP_NULL)
			id = ID_PP_NONE;
		else if (m_ppLevel == DPP_DBLKLUM)
			id = ID_PP_SLIGHT;
		else if (m_ppLevel == DPP_DBLK)
			id = ID_PP_MEDIUM;
		else if (m_ppLevel == DPP_DBLKDRNGLUM)
			id = ID_PP_HEAVY;
		else if (m_ppLevel == DPP_DBLKDRNG)
			id = ID_PP_SUPERHEAVY;
		else id = 0;
		if (id != 0)
			SetMenuItemInfo(pmenuPopup->m_hMenu, id, FALSE, &item);
	}
	else
	{
		pmenuPopup->DeleteMenu(6, MF_BYPOSITION);
		pmenuPopup->DeleteMenu(5, MF_BYPOSITION);
	}

    pmenuPopup->TrackPopupMenu( (TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RIGHTBUTTON), point.x, point.y, this);

}

void CVideoWnd::OnFullscreen() 
{
	// TODO: Add your command handler code here
	SetFullScreen(!IsFullScreen());
}

void CVideoWnd::OnZoomHalf() 
{
	// TODO: Add your command handler code here
	Zoom(0.5, 0.5);
}

void CVideoWnd::OnZoomDouble() 
{
	// TODO: Add your command handler code here
	Zoom(2., 2.);
}

void CVideoWnd::OnZoomOriginal() 
{
	// TODO: Add your command handler code here
	Zoom(1., 1.);
}

void CVideoWnd::OnRatioKeep() 
{
	// TODO: Add your command handler code here
	m_render.SetAspectRatio(m_VideoWidth, m_VideoHeight);
}

void CVideoWnd::OnRatio43() 
{
	// TODO: Add your command handler code here
	m_render.SetAspectRatio(4, 3);
}

void CVideoWnd::OnRatioWide() 
{
	// TODO: Add your command handler code here
	m_render.SetAspectRatio(16, 9);
}

void CVideoWnd::OnRatioThin() 
{
	// TODO: Add your command handler code here
	m_render.SetAspectRatio(9, 16);
}

void CVideoWnd::OnRatioFree() 
{
	// TODO: Add your command handler code here
	m_render.SetAspectRatio(0, 0);	
}

void CVideoWnd::OnVwAbout() 
{
	// TODO: Add your command handler code here
	CAboutDlg dlg;
	dlg.DoModal();
}

void CVideoWnd::OnAlwaysontop() 
{
	// TODO: Add your command handler code here
	if (!IsFullScreen())
	{
		m_ontop = !m_ontop;
		GetWindowRect(&m_rect);

		::SetWindowPos(this->m_hWnd, m_ontop ? HWND_TOPMOST : HWND_BOTTOM,  m_rect.left, m_rect.top, m_rect.right - m_rect.left, m_rect.bottom - m_rect.top, TRUE);
		if (!m_ontop)
			::SetWindowPos(this->m_hWnd, HWND_TOP,  m_rect.left, m_rect.top, m_rect.right - m_rect.left, m_rect.bottom - m_rect.top, TRUE);
	}
	
}

void CVideoWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CWnd::OnMouseMove(nFlags, point);

	if (IsFullScreen()) return;
	if (nFlags & MK_LBUTTON)
	{
		GetWindowRect(&m_rect);
		ClientToScreen(&point);
		::MoveWindow(this->m_hWnd, point.x - m_moveX, point.y - m_moveY, 
						m_rect.right - m_rect.left, m_rect.bottom - m_rect.top, TRUE);
	}
}

void CVideoWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CWnd::OnLButtonDown(nFlags, point);

	m_moveX = point.x;
	m_moveY = point.y;
	SetCapture();
}

void CVideoWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	ReleaseCapture();
	CWnd::OnLButtonUp(nFlags, point);
}

BOOL CVideoWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}


void CVideoWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if (nChar == VK_ESCAPE)
	{
		if (IsFullScreen())
		{
			SetFullScreen(FALSE);
		}
	}

	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}


void CVideoWnd::OnPpNone() 
{
	// TODO: Add your command handler code here
	if (m_cmdCallback)
	{
		m_cmdCallback(m_cmdParam, ID_PP_NONE); 
		m_ppLevel = DPP_NULL;
	}
}

void CVideoWnd::OnPpSlight() 
{
	// TODO: Add your command handler code here
	if (m_cmdCallback)
	{
		m_cmdCallback(m_cmdParam, ID_PP_SLIGHT); 
		m_ppLevel = DPP_DBLKLUM;
	}
}

void CVideoWnd::OnPpMedium() 
{
	// TODO: Add your command handler code here
	if (m_cmdCallback)
	{
		m_cmdCallback(m_cmdParam, ID_PP_MEDIUM); 
		m_ppLevel = DPP_DBLK;
	}
}

void CVideoWnd::OnPpHeavy() 
{
	// TODO: Add your command handler code here
	if (m_cmdCallback)
	{
		m_cmdCallback(m_cmdParam, ID_PP_HEAVY); 
		m_ppLevel = DPP_DBLKDRNGLUM;
	}
}

void CVideoWnd::OnPpSuperheavy() 
{
	// TODO: Add your command handler code here
	if (m_cmdCallback)
	{
		m_cmdCallback(m_cmdParam, ID_PP_SUPERHEAVY); 
		m_ppLevel = DPP_DBLKDRNG;
	}
}

void CVideoWnd::OnSnapshot() 
{
	// TODO: Add your command handler code here
	if (m_cmdCallback)
	{
		m_cmdCallback(m_cmdParam, ID_SNAPSHOT); 
	}
}

void CVideoWnd::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CWnd::OnLButtonDblClk(nFlags, point);
	PrintString("Doubleclick to take a snapshot");
	if (m_cmdCallback)
	{
		m_cmdCallback(m_cmdParam, ID_SNAPSHOT); 
	}
}
