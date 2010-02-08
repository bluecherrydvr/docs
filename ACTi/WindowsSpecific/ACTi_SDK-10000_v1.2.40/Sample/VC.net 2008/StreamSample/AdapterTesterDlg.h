// AdapterTesterDlg.h : 標頭檔 
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "SDK10000.h"
#include "colorstatic.h"
#include "eptzdlg.h"

enum QuadDeviceMode
{
	QUAD_MODE = 0,
	SINGLE_MODE,
	SEQUENTIAL_MODE,
	AUTO_DETECT	//only for quad device
};

// CAdapterTesterDlg 對話方塊
class CAdapterTesterDlg : public CDialog
{
// 建構
public:
	CAdapterTesterDlg(CWnd* pParent = NULL);	// 標準建構函式

// 對話方塊資料
	enum { IDD = IDD_ADAPTERTESTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支援


	bool RunOpenURL (char *SessionName, char *szCommand, bool bCheckStatus, char* szResBuffer, int nBufSize, int *pLen);
	int CheckDeviceType (HANDLE sdk);
	int AutoDetectQuadMode ();
// 程式碼實作
protected:
	HICON m_hIcon;

	// 產生的訊息對應函式
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	void ShowDebugMessage( TCHAR *pFormat, ...);
	afx_msg void OnBnClickedOk();
	CStatic m_pc;
	afx_msg void OnClose();
	afx_msg void OnBnClickedCancel();
	HANDLE h;
	CComboBox m_contacttype;
	CIPAddressCtrl m_UIP;
	DWORD m_RPort;
	DWORD m_CPort;
	DWORD m_VPort;
	DWORD m_HPort;
	CIPAddressCtrl m_MIP;
	DWORD m_channel;
	CString m_UID;
	CString m_PWD;
	DWORD m_MPort;
	DWORD m_RTSPPort;
	DWORD m_RTPVideoTrack;
	DWORD m_RTPAudioTrack;
	DWORD m_connecttimeout;
	afx_msg void OnBnClickedButton1();
	CString m_filename;
	CSliderCtrl m_volume;
	afx_msg void OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedOk2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedCancel3();
	afx_msg void OnBnClickedCancel4();
	afx_msg void OnBnClickedCancel2();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnBnClickedRadio4();
	afx_msg void OnBnClickedRadio5();
	afx_msg void OnBnClickedRadio6();
	afx_msg void OnBnClickedRadio7();
	afx_msg void OnBnClickedRadio8();
	CString m_recordfilename;
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	CComboBox m_PlayRate;
	afx_msg void OnCbnSelchangeCombo2();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedCancel5();
	afx_msg void OnBnClickedCancel6();
	
	afx_msg void OnBnClickedButton11();
	DWORD m_Brightness;
	afx_msg void OnBnClickedButton13();
	DWORD m_Contrast;
	DWORD m_Saturation;
	DWORD m_Hue;
	DWORD m_Resolution;
	DWORD m_FPS;
	DWORD m_Bitrate;
	afx_msg void OnBnClickedButton15();
	afx_msg void OnBnClickedButton17();
	afx_msg void OnBnClickedButton19();
	afx_msg void OnBnClickedButton21();
	afx_msg void OnBnClickedButton23();
	afx_msg void OnBnClickedButton25();
	afx_msg void OnBnClickedButton10();
	afx_msg void OnBnClickedButton27();
	afx_msg void OnBnClickedButton28();
	DWORD m_VariableFPS;
	afx_msg void OnBnClickedButton14();
	afx_msg void OnBnClickedButton12();
	DWORD m_M1_X;
	DWORD m_M1_Y;
	DWORD m_M1_W;
	DWORD m_M1_H;
	bool  m_bMotion1Enable;
	DWORD m_dwTime1;
	DWORD m_dwThreshold1;
	DWORD m_bEnable1;

	DWORD m_M2_X;
	DWORD m_M2_Y;
	DWORD m_M2_W;
	DWORD m_M2_H;
	bool  m_bMotion2Enable;
	DWORD m_dwTime2;
	DWORD m_dwThreshold2;
	DWORD m_bEnable2;

	DWORD m_M3_X;
	DWORD m_M3_Y;
	DWORD m_M3_W;
	DWORD m_M3_H;
	bool  m_bMotion3Enable;
	DWORD m_dwTime3;
	DWORD m_dwThreshold3;
	DWORD m_bEnable3;

	DWORD m_M4_X;
	DWORD m_M4_Y;
	DWORD m_M4_W;
	DWORD m_M4_H;
	bool  m_bMotion4Enable;
	DWORD m_dwTime4;
	DWORD m_dwThreshold4;
	DWORD m_bEnable4;

	DWORD m_bMotionEnable;
	CColorStatic m_Color_M1;
	CColorStatic m_Color_M2;
	CColorStatic m_Color_M3;
	CColorStatic m_Color_M4;
	DWORD m_M1_S;
	DWORD m_M2_S;
	DWORD m_M3_S;
	DWORD m_M4_S;

	CColorStatic m_Color_D1;
	CColorStatic m_Color_D2;
	bool  m_bDI1Enable;
	bool  m_bDI2Enable;

	afx_msg void OnBnClickedRadio10();
	afx_msg void OnBnClickedRadio11();
	CComboBox m_DIDefault;
	afx_msg void OnCbnSelchangeCombo3();
	afx_msg void OnBnClickedButton16();
	afx_msg void OnBnClickedButton18();
	afx_msg void OnBnClickedButton20();
	afx_msg void OnBnClickedButton22();
	CSliderCtrl m_CurrectPos;
	DWORD m_dwCurrentPos;
	afx_msg void OnNMReleasedcaptureSlider2(NMHDR *pNMHDR, LRESULT *pResult);
	CString m_NTSCPAL;
	CComboBox m_TCPType;
	afx_msg void OnBnClickedRadio15();
	//int m_51CH;
	void ResolutionChange( int nR );
	afx_msg void OnBnClickedRadio23();
	afx_msg void OnBnClickedRadio24();
	afx_msg void OnBnClickedRadio53();
	afx_msg void OnBnClickedRadio54();
	afx_msg void OnBnClickedRadio55();
	afx_msg void OnBnClickedRadio56();
	afx_msg void OnBnClickedButton24();
	CComboBox m_usingse;
	afx_msg void OnCbnSelchangeCombo5();
	afx_msg void OnTimer(UINT nIDEvent);
	CString m_ver;

	CEPTZDlg * m_oPTZDlg;
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	bool	m_bDraging;
	
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	DWORD m_SID;
	int m_ContactTypeIndex;
	BOOL m_IsQuadDevice;
	int  m_QuadDeviceMode;
	VIDEO_TRANSFORM_CONFIG m_VTC;
};
