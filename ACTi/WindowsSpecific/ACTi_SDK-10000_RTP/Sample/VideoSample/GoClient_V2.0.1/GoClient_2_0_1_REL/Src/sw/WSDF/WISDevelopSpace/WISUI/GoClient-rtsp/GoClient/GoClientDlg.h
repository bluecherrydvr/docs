// GoClientDlg.h : header file
//

#if !defined(AFX_GOCLIENTDLG_H__8ED87205_B754_47D2_8A07_DFE18BBD1755__INCLUDED_)
#define AFX_GOCLIENTDLG_H__8ED87205_B754_47D2_8A07_DFE18BBD1755__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VideoRender.h" 

#include "UnicastChannel.h" 
#include "MulticastChannel.h" 
#include "MulticastAddrDlg.h"
#include "SoundDevice.h"

struct SFrameBufItem
{
	unsigned char *pBuf;
	int Length;
	SFrameBufItem *pNext;
};


/////////////////////////////////////////////////////////////////////////////
// CGoClientDlg dialog

class CGoClientDlg : public CDialog
{
// Construction
public:
	CGoClientDlg(CWnd* pParent = NULL);	// standard constructor

	void DoInAReceiveThread();//audio receive thread
	void ProcessAPacket(unsigned char *pPacket, int Len);
	void DoInVReceiveThread();//video receive thread
	void ProcessVPacket(unsigned char *pPacket, int Len);
	CMulticastChannel Channel;
	CUnicastChannel UChannel;

	int			AReceiveThreadState;
	CWinThread	*pAReceiveThread;
	int			VReceiveThreadState;
	CWinThread	*pVReceiveThread;

	unsigned char *pFrameBuf;
	int FrameBufPtr;
	int TotalVideoPacket;
	int TotalAudioPacket;
    unsigned char *pSoundBuf;
    int SoundBufPtr;
    CVideoRender *p_vr;
    CSoundDevice SoundDevice;
    //video
	int VideoFrameGot;
	int VideoByteGot;
    //audio
	int AudioFrameGot;
	int AudioByteGot;
    FILE *fout;

	CMulticastAddrDlg dlg;

// Dialog Data
	//{{AFX_DATA(CGoClientDlg)
	enum { IDD = IDD_GOCLIENT_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGoClientDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CGoClientDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    BOOL m_Mute;
    afx_msg void OnBnClickedCheck1();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GOCLIENTDLG_H__8ED87205_B754_47D2_8A07_DFE18BBD1755__INCLUDED_)
