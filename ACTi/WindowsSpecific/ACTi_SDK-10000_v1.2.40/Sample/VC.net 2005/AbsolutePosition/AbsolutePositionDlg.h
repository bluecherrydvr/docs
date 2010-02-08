// AbsolutePositionDlg.h : ���Y�� 
//

#pragma once
#include "afxwin.h"
#include "Absolute.h"
#include "afxcmn.h"

#include <string>
#include "SDK10000.h"
//#include "PTZDefine.h"
#define PTZ_MEDIA_SOURCE_RS		1
#define PTZ_MEDIA_SOURCE_FILE	2

// CAbsolutePositionDlg ��ܤ��
class CAbsolutePositionDlg : public CDialog
{
// �غc
public:
	CAbsolutePositionDlg(CWnd* pParent = NULL);	// �зǫغc�禡

// ��ܤ�����
	enum { IDD = IDD_ABSOLUTEPOSITION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �䴩


// �{���X��@
protected:
	HICON m_hIcon;

	// ���ͪ��T�������禡
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();

private:
	HANDLE m_hKMpeg4;
	
public:
	void Disconnect();
private:
	// Picture control
	CStatic m_pc;
	CAbsolute m_cAbsolute;
	unsigned char m_szRSDCB[256];
	DWORD m_dwRSDLen;
	// Zoom ratio number
	DWORD m_dwRatio;
	// Pan value
	DWORD m_dwPan;
	// Tilt
	DWORD m_dwTilt;
	// Area for process click event. 
	RECT m_rcClick;
	CPoint m_MidPoint;
	// Connect IP address
	CIPAddressCtrl m_IPCtrl;
	DWORD m_dwQuadrant;
	CStatic m_pc2;
	CPoint m_ClickPt;
	float m_fSlope;


	//display caught  p t z units
	int m_ICatchedPan;
	int m_ICatchedTilt;
	int m_ICatchedZoom;

	//load ptz protocol from file
	std::string m_strPTZfileName;
	MEDIA_PTZ_PROTOCOL m_mPTZ;



	// some new interface
	CComboBox m_CmdTypeList;
	float m_fPanDegree;
	float m_fTiltDegree;


	CRITICAL_SECTION m_CriticalSection;

public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnRSDCB(BYTE* buf, DWORD len);
//	afx_msg void OnBnClickedButton4();	
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	
	afx_msg void OnBnClickedButton5();
	afx_msg void GoHome();
	afx_msg bool CheckPTZValue(int nPan, int nTilt, int nRatio);
	
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnEnChangeEdit1();
	
	afx_msg void OnEnChangeEdit4();
	afx_msg void OnEnChangeEdit5();
	afx_msg void OnEnChangeEdit6();
	afx_msg void OnEnChangeEdit7();
	afx_msg void OnBnClickedButton8();
};	
