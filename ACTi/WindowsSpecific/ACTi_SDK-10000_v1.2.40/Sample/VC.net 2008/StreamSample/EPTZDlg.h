#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CEPTZDlg dialog
typedef void (*EPTZMoving)( DWORD dwUserParam, int nXSrc, int nYSrc, int nWidth, int nHeight );

class CEPTZDlg : public CDialog
{
	DECLARE_DYNAMIC(CEPTZDlg)

public:
	CEPTZDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEPTZDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG2 };

private:
	CPoint	m_pnPTZWnd;
	int		m_nPTZWndWidth;
	int		m_nPTZWndHeight;
	int		m_nXRangeToMouse;
	int		m_nYRangeToMouse;
	bool	m_bDraging;
	BYTE*	m_pBitmapBuf;
	int		m_nImageWidth;
	int		m_nImageHeight;
	bool	m_bShow;
	HANDLE	m_hShowThread;
	bool	m_bFirstTime;
	int		m_nShowSizeX;
	int		m_nShowSizeY;
	int		m_nZoomX;
	float	m_nRate;
	DWORD	m_dwEPTZFunParam;
	EPTZMoving	m_fnEPTZMoving;
	CPoint	m_LastMoustMove;
	bool	m_bResize;
	bool	m_bDoResize;
	int OFFSET_X;
	int	OFFSET_Y;
	int	m_nWndStartX;
	int	m_nWndStartY;
	int _W_SIZE_X;


public:
	void	SetEPTZMoveCallback( DWORD dwUserParam, EPTZMoving fnPoint );
	bool	SetZoomX( int nZoomX );
	void	SetImage( BYTE* buf, DWORD len, DWORD dwWidth, DWORD dwHeight );
	static	void ShowProc( LPVOID lp );
	HCURSOR m_hCursor;
	HCURSOR m_hChangeCursor;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	CStatic m_pic;
	CSliderCtrl m_ZoomSlider;
	afx_msg void OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};
