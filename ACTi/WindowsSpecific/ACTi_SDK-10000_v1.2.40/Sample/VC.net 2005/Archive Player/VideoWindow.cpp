// VideoWindow.cpp : implementation file
//

#include "stdafx.h"
#include "VideoWindow.h"
#include ".\videowindow.h"


CVideoWindow::CVideoWindow(bool *pbFullScreen)
{
	m_pbFullScreen = pbFullScreen;

	// Register your unique class name that you wish to use
    WNDCLASS wndcls;
    memset(&wndcls, 0, sizeof(WNDCLASS));   // start with NULL
                                            // defaults

	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;

    //you can specify your own window procedure
	wndcls.lpfnWndProc = ::DefWindowProc;
    wndcls.hInstance = AfxGetInstanceHandle();

    // Specify your own class name for using FindWindow later
    wndcls.lpszClassName = _T("VideoWindow");

    // Register the new class and exit if it fails
    if( !AfxRegisterClass( &wndcls ) )
		return ;

	m_XLEN = GetSystemMetrics( SM_CXSCREEN );
	m_YLEN = GetSystemMetrics( SM_CYSCREEN );
	BOOL created = CreateEx( WS_EX_OVERLAPPEDWINDOW,
									wndcls.lpszClassName,
									"CVideoWindow", 
									WS_POPUPWINDOW | WS_CHILDWINDOW,
									0,
									0,
									m_XLEN,
									m_YLEN,
									NULL,
									0 );
}

CVideoWindow::~CVideoWindow()
{
}
BEGIN_MESSAGE_MAP(CVideoWindow, CWnd)
END_MESSAGE_MAP()

LRESULT CVideoWindow::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case WM_KEYDOWN:
		if( wParam != VK_ESCAPE )
			break;
		if( m_pbFullScreen )
			*m_pbFullScreen = false;

		ShowWindow( SW_HIDE );
		return 0;
		break;			
	}
	return CWnd::DefWindowProc(message, wParam, lParam);
}
