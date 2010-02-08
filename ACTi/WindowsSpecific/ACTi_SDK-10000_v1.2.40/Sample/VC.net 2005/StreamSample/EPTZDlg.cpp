// EPTZDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdapterTester.h"
#include ".\eptzdlg.h"

#define _SFT_SIZE 10
// CEPTZDlg dialog

IMPLEMENT_DYNAMIC(CEPTZDlg, CDialog)
CEPTZDlg::CEPTZDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEPTZDlg::IDD, pParent)
{
	_W_SIZE_X = 200;
	m_bResize = false;
	m_bDoResize = false;
	m_LastMoustMove.x = 0;
	m_LastMoustMove.y = 0;
	m_pnPTZWnd.x = 0;
	m_pnPTZWnd.y = 0;
	m_bShow = false;
	m_nImageWidth = 0;
	m_nImageHeight = 0;
	m_nZoomX = 1;
	m_nPTZWndWidth = 160;
	m_nPTZWndHeight = 100;
	m_bDraging = false;
	m_pBitmapBuf = new BYTE[(1920*1024*4)+64];
	memset( m_pBitmapBuf, 0, (1920*1024*4)+64 );
	m_hShowThread = NULL;
	m_bFirstTime = true;
	m_nShowSizeX = 0;
	m_nShowSizeY = 0;
	OFFSET_X = 0;
	OFFSET_Y = 3;
	m_nWndStartX = 300;
	m_nWndStartY = 300;
}

CEPTZDlg::~CEPTZDlg()
{
	BYTE * p = m_pBitmapBuf;
	m_pBitmapBuf = NULL;
	delete [] p;
}

void CEPTZDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PIC, m_pic);
	DDX_Control(pDX, IDC_SLIDER1, m_ZoomSlider);
}


BEGIN_MESSAGE_MAP(CEPTZDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, OnNMReleasedcaptureSlider1)
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_WM_NCLBUTTONDOWN()
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()


// CEPTZDlg message handlers

void CEPTZDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	if( m_bShow == false )
	{
		m_bFirstTime = true;
		m_bShow = true;
		RECT rect;
		rect.top = m_nWndStartY;
		rect.left = m_nWndStartX;
		rect.right = m_nWndStartX + 160 + 12;
		rect.bottom = m_nWndStartY + 160;
		MoveWindow( &rect );
		if( m_hShowThread == NULL )
			m_hShowThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ShowProc, this, 0, NULL );
	}
}

void CEPTZDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if( point.x > m_pnPTZWnd.x && point.y > m_pnPTZWnd.y )
	{
		if( (point.x-OFFSET_X) < (m_pnPTZWnd.x + m_nPTZWndWidth) && (point.y-OFFSET_Y) < (m_pnPTZWnd.y + m_nPTZWndHeight) )
		{
			m_nXRangeToMouse = point.x - m_pnPTZWnd.x;
			m_nYRangeToMouse = point.y - m_pnPTZWnd.y;
			m_bDraging = true;
			m_hChangeCursor = LoadCursor( AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_CURSOR1)) ;
			SetCursor( m_hChangeCursor );
		}
	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CEPTZDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bDraging = false;
	CDialog::OnLButtonUp(nFlags, point);
}

void CEPTZDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if( m_bDraging || m_bResize )
	{
		if( !m_bResize )
		{
			m_hChangeCursor = LoadCursor( AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_CURSOR1)) ;
			SetCursor( m_hChangeCursor );
		}

		if( m_bResize ) 
		{
			m_nXRangeToMouse = 0;
			m_nYRangeToMouse = 0;
			m_pnPTZWnd.x = 0;
			m_pnPTZWnd.y = 0;
			m_bResize = false;
		}
		
		if( m_nShowSizeX - (point.x - m_nXRangeToMouse + m_nPTZWndWidth ) <= 0  )
		{
			m_pnPTZWnd.x = m_nShowSizeX - m_nPTZWndWidth;
		}
		else
		{
			if( point.x - m_nXRangeToMouse > 0 )//&& m_nShowSizeX - (point.x - m_nXRangeToMouse + m_nPTZWndWidth ) > 0  )
				m_pnPTZWnd.x = point.x - m_nXRangeToMouse;
			else m_pnPTZWnd.x = 0;
		}
		if( m_nShowSizeY - (point.y - m_nYRangeToMouse + m_nPTZWndHeight) <= 0 )
		{
			m_pnPTZWnd.y = m_nShowSizeY - m_nPTZWndHeight;
		}
		else	
		{
			if( point.y - m_nYRangeToMouse > 0 )//&& m_nShowSizeY - (point.y - m_nYRangeToMouse + m_nPTZWndHeight) > 0 )
				m_pnPTZWnd.y = point.y - m_nYRangeToMouse;
			else
				m_pnPTZWnd.y = 0;
		}
		if( m_fnEPTZMoving )
			m_fnEPTZMoving( m_dwEPTZFunParam, (m_pnPTZWnd.x*m_nRate), (m_pnPTZWnd.y*m_nRate), (m_nPTZWndWidth*m_nRate), (m_nPTZWndHeight*m_nRate) );
	}
	CDialog::OnMouseMove(nFlags, point);
}

void CEPTZDlg::SetImage( BYTE* buf, DWORD len, DWORD dwWidth, DWORD dwHeight )
{
	memcpy( m_pBitmapBuf, buf, len );
	m_nImageWidth = dwWidth;
	m_nImageHeight = dwHeight;
}

void CEPTZDlg::ShowProc( LPVOID lp )
{
	CEPTZDlg *p = (CEPTZDlg*)lp;

	while( p->m_bShow )
	{
		if( p->m_nImageWidth > 0 && p->m_nImageHeight > 0 )
		{
			if( p->m_bDoResize )
			{
				p->m_bDoResize = false;
				RECT rect;
				switch( p->m_nImageHeight )
				{
				case 1024:
					p->m_nPTZWndWidth = 160 / p->m_nZoomX;
					rect.bottom = 128;
					break;
				case 576:
					rect.bottom = 128;
					p->m_nPTZWndWidth = 160 / p->m_nZoomX;
					break;
				case 960:
					rect.bottom = 120;
					p->m_nPTZWndWidth = 160 / p->m_nZoomX;
					break;
				case 288:
					if( p->m_nImageWidth == 352 )
					{
						p->m_nPTZWndWidth = 176 / p->m_nZoomX;
						rect.bottom = 144;
					}
					break;
				case 240:
					if( p->m_nImageWidth == 320 )
					{
						p->m_nPTZWndWidth = 160 / p->m_nZoomX;
						rect.bottom = 120;
					}
					else if( p->m_nImageWidth == 352 )
					{
						p->m_nPTZWndWidth = 176 / p->m_nZoomX;
						rect.bottom = 120;
					}
					break;
				case 480:
					if( p->m_nImageWidth == 720 )
					{
						p->m_nPTZWndWidth = 180 / p->m_nZoomX;
						rect.bottom = 120;
					}
					else if( p->m_nImageWidth == 640 )
					{
						p->m_nPTZWndWidth = 160 / p->m_nZoomX;
						rect.bottom = 120;
					}
					break;
				case 720:
					p->m_nPTZWndWidth = 160 / p->m_nZoomX;
					rect.bottom = 90;
					break;
				default:
					p->m_nPTZWndWidth = 180 / p->m_nZoomX;
					rect.bottom = 120;
					break;
				}
				
				p->m_nPTZWndHeight = rect.bottom/p->m_nZoomX;
				p->m_bResize = true;
				p->PostMessage( WM_MOUSEMOVE, 0, 0 );
			}

			if( p->m_bFirstTime )
			{
				RECT rect;
				p->m_bFirstTime = false;
				switch( p->m_nImageHeight )
				{
				case 1024:
					{
						p->m_nRate = 8;
						rect.bottom = 128;
						p->m_nPTZWndWidth = 160 / p->m_nZoomX;
						p->m_nShowSizeX = 160;
						//p->_W_SIZE_X = 180;
					}
					break;
				case 576:
					{
						p->m_nRate = 4.5;
						rect.bottom = 128;
						p->m_nPTZWndWidth = 160 / p->m_nZoomX;
						p->m_nShowSizeX = 160;
						//p->_W_SIZE_X = 180;
					}
					break;
				case 960:
					{
						p->m_nRate = 8;
						rect.bottom = 120;
						p->m_nPTZWndWidth = 160 / p->m_nZoomX;
						p->m_nShowSizeX = 160;
						//p->_W_SIZE_X = 180;
					}
					break;
				case 288:
					if( p->m_nImageWidth == 352 )
					{
						p->m_nRate = 2;
						rect.right = 176 + p->OFFSET_X;
						rect.bottom = 144;
						p->m_nPTZWndWidth = 176 / p->m_nZoomX;
						p->m_nShowSizeX = 176;
						//p->_W_SIZE_X = 196;
					}
					break;
				case 240:
					if( p->m_nImageWidth == 320 )
					{
						p->m_nRate = 2;
						rect.bottom = 120;
						p->m_nPTZWndWidth = 160 / p->m_nZoomX;
						p->m_nShowSizeX = 160;
						//p->_W_SIZE_X = 180;
					}
					else if( p->m_nImageWidth == 352 )
					{
						p->m_nRate = 2;
						rect.right = 176 + p->OFFSET_X;
						rect.bottom = 120;
						p->m_nPTZWndWidth = 176 / p->m_nZoomX;
						p->m_nShowSizeX = 176;
						//p->_W_SIZE_X = 196;
					}
					break;
				case 480:
					if( p->m_nImageWidth == 720 )
					{
						rect.right = 180 + p->OFFSET_X;
						p->m_nRate = 4;
						rect.bottom = 120;
						p->m_nPTZWndWidth = 180 / p->m_nZoomX;
						p->m_nShowSizeX = 180;
						//p->_W_SIZE_X = 200;
					}
					else if( p->m_nImageWidth == 640 )
					{
						p->m_nRate = 4;
						rect.bottom = 120;
						p->m_nPTZWndWidth = 160 / p->m_nZoomX;
						p->m_nShowSizeX = 160;
						//p->_W_SIZE_X = 180;
					}
					break;
				case 720:
					{
						p->m_nRate = 8;
						rect.bottom = 90;
						p->m_nPTZWndWidth = 160 / p->m_nZoomX;
						p->m_nShowSizeX = 160;
						//p->_W_SIZE_X = 180;
					}
					break;
				default:
					{
						rect.right = 180 + p->OFFSET_X;
						p->m_nRate = 4;
						rect.bottom = 120;
						p->m_nPTZWndWidth = 180 / p->m_nZoomX;
						p->m_nShowSizeX = 180;
						//p->_W_SIZE_X = 200;
					}
					break;
				}

				p->_W_SIZE_X = p->m_nShowSizeX + _SFT_SIZE;
				p->m_nPTZWndHeight = rect.bottom / p->m_nZoomX;
				p->m_nShowSizeY = rect.bottom;

				p->OFFSET_X = ( p->_W_SIZE_X - ( p->m_nPTZWndWidth * p->m_nZoomX ) )/2;

				RECT rectPic;
				rectPic.top = 0 + p->OFFSET_Y;
				rectPic.left = 0 + p->OFFSET_X;
				rectPic.right = rectPic.left + p->m_nShowSizeX;
				rectPic.bottom = rectPic.top + p->m_nShowSizeY;
				p->m_pic.MoveWindow( &rectPic );

				RECT r2;
				r2.top = rect.bottom + 8;
				r2.bottom = r2.top + 15;
				r2.left = rectPic.left + 15;
				r2.right = rectPic.right - 15;
				p->m_ZoomSlider.MoveWindow( &r2 );

				RECT rXB;

				rXB.top = r2.top;
				rXB.bottom = rXB.top + 15;
				rXB.left = r2.left - 15;
				rXB.right = rXB.left + 12;
				p->GetDlgItem( IDC_BUTTON2 )->MoveWindow( &rXB );

				rXB.top = r2.top;
				rXB.bottom = rXB.top + 15;
				rXB.left = r2.right + 3;
				rXB.right = rXB.left + 12;
				p->GetDlgItem( IDC_BUTTON1 )->MoveWindow( &rXB );
				

				RECT rX;

				rX.top = r2.bottom + 2;
				rX.bottom = rX.top + 15;
				rX.left = r2.left;
				rX.right = rX.left + 15;
				p->GetDlgItem( IDC_STATIC1 )->MoveWindow( &rX );

				rX.left = r2.left + (r2.right-r2.left-10) / 3;
				rX.right = rX.left + 15;
				p->GetDlgItem( IDC_STATIC2 )->MoveWindow( &rX );

				rX.left = r2.left + ((r2.right-r2.left-10) / 3 * 2);
				rX.right = rX.left + 15;
				p->GetDlgItem( IDC_STATIC3 )->MoveWindow( &rX );

				rX.left = r2.left + (r2.right-r2.left-10);
				rX.right = rX.left + 15;
				p->GetDlgItem( IDC_STATIC4 )->MoveWindow( &rX );
				

				rect.top = p->m_nWndStartY;
				rect.left = p->m_nWndStartX;
				rect.right = p->m_nWndStartX + p->_W_SIZE_X + 6;
				rect.bottom = p->m_nWndStartY + rect.bottom + 70;
				p->MoveWindow( &rect );
				p->Invalidate();
			}

			CPen oPTZPen ;
			LOGBRUSH lbPTZ;
			lbPTZ.lbStyle = BS_SOLID;
			lbPTZ.lbColor = RGB( 255, 255, 0 );
			oPTZPen.CreatePen( PS_GEOMETRIC | PS_ENDCAP_ROUND, 1, &lbPTZ );

			HDC RanderDC = ::GetDC( p->m_pic.m_hWnd );
			::SetStretchBltMode( RanderDC, COLORONCOLOR );
			::StretchDIBits( RanderDC, 0, 0, p->m_nShowSizeX, p->m_nShowSizeY, 0, 0, p->m_nImageWidth, p->m_nImageHeight, (const void*)&p->m_pBitmapBuf[sizeof(BITMAPINFO)], (LPBITMAPINFO)p->m_pBitmapBuf, DIB_RGB_COLORS, SRCCOPY );
			::SetBkMode( RanderDC , TRANSPARENT );
			HPEN hPenOld = (HPEN)::SelectObject( RanderDC, oPTZPen.GetSafeHandle() );
			::MoveToEx( RanderDC, p->m_pnPTZWnd.x, p->m_pnPTZWnd.y, NULL );
			::LineTo( RanderDC, p->m_pnPTZWnd.x + p->m_nPTZWndWidth -1, p->m_pnPTZWnd.y );
			::LineTo( RanderDC, p->m_pnPTZWnd.x + p->m_nPTZWndWidth -1, p->m_pnPTZWnd.y + p->m_nPTZWndHeight-1 );
			::LineTo( RanderDC, p->m_pnPTZWnd.x, p->m_pnPTZWnd.y + p->m_nPTZWndHeight -1 );
			::LineTo( RanderDC, p->m_pnPTZWnd.x, p->m_pnPTZWnd.y );
			::SelectObject( RanderDC, hPenOld );
			::ReleaseDC( p->m_pic.m_hWnd, RanderDC );
		}
		Sleep( 40 );
	}
}
void CEPTZDlg::OnDestroy()
{
	CDialog::OnDestroy();

	m_bShow = false;
	WaitForSingleObject( m_hShowThread, INFINITE );
	CloseHandle( m_hShowThread );
	m_hShowThread = NULL;
}

bool CEPTZDlg::SetZoomX( int nZoomX )
{
	if( nZoomX > 0 && nZoomX <= 8 )
	{
		m_nZoomX = nZoomX;
		return true;
	}
	return false;
}

void CEPTZDlg::SetEPTZMoveCallback( DWORD dwUserParam, EPTZMoving fnPoint )
{
	m_dwEPTZFunParam = dwUserParam;
	m_fnEPTZMoving = fnPoint;
}

BOOL CEPTZDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_hCursor = GetCursor();
	int i = 0;
	switch( m_nZoomX )
	{
	case 1:	i = 0;	break;
	case 2:	i = 1;	break;
	case 4:	i = 2;	break;
	case 8:	i = 3;	break;
	default:	i = 0;	break;
	}
	m_ZoomSlider.SetRange( 0, 3 );
	m_ZoomSlider.SetPos( i );
	return TRUE;
}
void CEPTZDlg::OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_bDraging = false;

	int i = m_ZoomSlider.GetPos();
	switch( i )
	{
	case 0:	m_nZoomX = 1;	break;
	case 1:	m_nZoomX = 2;	break;
	case 2:	m_nZoomX = 4;	break;
	case 3:	m_nZoomX = 8;	break;
	default:	m_nZoomX = 1;	break;
	}
	m_bDoResize = true;
	m_bFirstTime = true;
	*pResult = 0;
}

void CEPTZDlg::OnMove(int x, int y)
{
	CDialog::OnMove(x, y);
	m_nWndStartX = x-3;
	m_nWndStartY = y-21;
	m_hChangeCursor = LoadCursor( AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_CURSOR1)) ;
	SetCursor( m_hChangeCursor );
}

void CEPTZDlg::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_bDraging = false;
	m_hChangeCursor = LoadCursor( AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_CURSOR1)) ;
	SetCursor( m_hChangeCursor );
	CDialog::OnNcLButtonDown(nHitTest, point);
}

void CEPTZDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	switch( m_nZoomX ) 
	{
	case 1:
		m_ZoomSlider.SetPos( 1 );
		m_nZoomX = 2;
		m_bDoResize = true;
		break;
	case 2:
		m_nZoomX = 4;
		m_ZoomSlider.SetPos( 2 );
		m_bDoResize = true;
		break;
	case 4:
		m_nZoomX = 8;
		m_ZoomSlider.SetPos( 3 );
		m_bDoResize = true;
		break;
	case 8:
		break;
	}
}

void CEPTZDlg::OnBnClickedButton2()
{
	switch( m_nZoomX ) 
	{
	case 1:
		break;
	case 2:
		m_nZoomX = 1;
		m_ZoomSlider.SetPos( 0 );
		m_bDoResize = true;
		break;
	case 4:
		m_nZoomX = 2;
		m_ZoomSlider.SetPos( 1 );
		m_bDoResize = true;
		break;
	case 8:
		m_nZoomX = 4;
		m_ZoomSlider.SetPos( 2 );
		m_bDoResize = true;
		break;
	}
}

