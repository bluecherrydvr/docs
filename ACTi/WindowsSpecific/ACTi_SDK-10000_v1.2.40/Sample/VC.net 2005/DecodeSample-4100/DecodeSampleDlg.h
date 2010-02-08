// DecodeSampleDlg.h : 標頭檔 
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "CapChannel.h"

#define MAX_CONTROL_CHANNEL 4

// CDecodeSampleDlg 對話方塊
class CDecodeSampleDlg : public CDialog
{
// 建構
public:
	CDecodeSampleDlg(CWnd* pParent = NULL);	// 標準建構函式

// 對話方塊資料
	enum { IDD = IDD_DECODESAMPLE_DIALOG };

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
private:
	CButton m_cBtnStartStop;
	CStatic m_cCtrlVW;

    // class of SDK-10000 with PCI-4100
    CCapChannel m_oCapChannel[MAX_CONTROL_CHANNEL];
    bool m_bStartPreview[MAX_CONTROL_CHANNEL];
    bool m_bStartRecord[MAX_CONTROL_CHANNEL];
    bool m_bStartStream[MAX_CONTROL_CHANNEL];


public:



    CEdit m_ctrlChlNo;

	afx_msg void OnBnClickedOk();


    afx_msg void OnBnClickedCancel();
    afx_msg void OnStnClickedStaticVw();

	// Function to handle video data
	void VideoDataHandler(BYTE* buf, DWORD len);
	// Function to handle audio data
	void AudioDataHandler(BYTE* buf, DWORD len);
	// Function to draw video data
	void VideoDisply();

	
//	afx_msg void OnBnClickedButton1();
//	afx_msg void OnBnClickedCheck2();
	
    CStatic m_cCtrlVW2;
    afx_msg void OnBnClickedOk2();
    CStatic m_cCtrlVW3;
    CStatic m_cCtrlVW4;
    afx_msg void OnBnClickedOk3();
    afx_msg void OnBnClickedOk5();
    CButton m_cBtnStartStreamStartStop;
    CButton m_cBtnStartRecordStartStop;
    afx_msg void OnBnClickedOk6();
    afx_msg void OnBnClickedOk8();
    afx_msg void OnBnClickedOk7();
    afx_msg void OnBnClickedOk9();
    afx_msg void OnBnClickedOk11();
    afx_msg void OnBnClickedOk10();
    afx_msg void OnBnClickedOk12();
    afx_msg void OnBnClickedOk14();
    afx_msg void OnBnClickedOk13();
    CButton m_cBtnStartStop2;
    CButton m_cBtnStartStop3;
    CButton m_cBtnStartStop4;
    CButton m_cBtnStartStreamStartStop2;
    CButton m_cBtnStartStreamStartStop3;
    CButton m_cBtnStartStreamStartStop4;
    CButton m_cBtnStartRecordStartStop2;
    CButton m_cBtnStartRecordStartStop3;
    CButton m_cBtnStartRecordStartStop4;
};
