// RTPSampleDlg.h : 標頭檔 
//

#pragma once
#include "version.h"
#include "RTPSession.h"
#include "RTSPSession.h"
#include "RTCPSession.h"

#include "SoundPlayer.h"
#include "IPPCODEC.h"
#include "afxwin.h"

// CRTPSampleDlg 對話方塊
class CRTPSampleDlg : public CDialog
{
// 建構
public:
	CRTPSampleDlg(CWnd* pParent = NULL);	// 標準建構函式

// 對話方塊資料
	enum { IDD = IDD_RTPSAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支援


// 程式碼實作
protected:
	HICON m_hIcon;

	// 產生的訊息對應函式
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	typedef struct _SYSCONF {
		unsigned char sRtspAddr[4];
		unsigned int nPortNum;
		unsigned int nVwSel;
	}SYSCONF;
protected:
	// RTP/RTCP/RTSP
	CRTSPSession m_RTSPSession;
	CRTCPSession m_RTCPSession;
	CRTPSession m_RTPSession;

	HANDLE m_hRtcpSessionThread;
	HANDLE m_hRtpVSessionThread;
	HANDLE m_hRtpVSessionThread2;
	HANDLE m_hRtpASessionThread;
	HANDLE m_hVedioViewThread;

	SYSCONF m_SysConf;
	CSoundPlayer* m_pSound;


protected:

	//CIPAddressCtrl m_ctlTargetAddr;
	CEdit m_ctlPortNum;
	CStatic m_ctlPicture;
	CButton m_ctlRTPStart;
	
public:
	CRTSPSession * GetRTSPSession( void)
	{
		return &m_RTSPSession;
	}
	CRTCPSession * GetRTCPSession( void)
	{
		return &m_RTCPSession;
	}
	CRTPSession * GetRTPSession( void)
	{
		return  &m_RTPSession;
	}

	CSoundPlayer* GetSound()
	{
		return m_pSound;
	}

protected:
	int m_nDecodeID;

	BITMAPINFO* m_Bmi;
	unsigned char * m_BitmapData;
	unsigned char m_FrameResolution;
	int m_VIEW_WIDTH;
	int m_VIEW_HEIGHT;
	BOOL PrepareDibBuffer( LPBITMAPINFO *lplpbi, DWORD dwX, DWORD dwY);

public:
	void Display( BYTE * lpFrameBuffer, int FrameBufferLen, int w = 640, int h = 480);
	void VideoDisply( BITMAPINFO* Bmi, BYTE * pInBuf, DWORD nInBufLen, unsigned char * BitmapData );

	afx_msg void OnBnClickedButton1();
	
	void CloseRTPSession( void);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnTimer(UINT nIDEvent);
	CEdit m_ctlSsysInfo;

	afx_msg void OnBnClickedCheck1();
	CButton m_ctlBeOnlyRTPTest;
	CEdit m_edtMrl;
};


