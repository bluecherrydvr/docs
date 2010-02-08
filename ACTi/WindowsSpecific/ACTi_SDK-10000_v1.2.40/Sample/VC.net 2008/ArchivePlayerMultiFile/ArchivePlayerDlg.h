// ArchivePlayerDlg.h : 標頭檔 
//

#pragma once


#include "afxcmn.h"
#include "afxwin.h"
#include "SDK10000.h"

#define FILE_EXT_NONE	0
#define FILE_EXT_RAW	1
#define FILE_EXT_MP4	2
#define FILE_EXT_MRAW   4

// CArchivePlayerDlg 對話方塊
class CArchivePlayerDlg : public CDialog
{
// 建構
public:
	CArchivePlayerDlg(CWnd* pParent = NULL);	// 標準建構函式

// 對話方塊資料
	enum { IDD = IDD_ARCHIVEPLAYER_DIALOG };

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
	afx_msg void OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSlider2(NMHDR *pNMHDR, LRESULT *pResult);
	// Picture control for display
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton4();
	void GetTimeToStr(time_t time, char *buf );
	void OnTimeCodeCB(DWORD TimeCode);
	void OnFilePlayCompleteCB();	
	void OnImageCB(BYTE* pBuf, DWORD nBufLen, DWORD dwWidth, DWORD dwHeight);
	void OnAfterRenderCB();
	void ResetAll();
	void CArchivePlayerDlg::OnBnEnable(bool bEnable);
	void SetRender();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedButton6();
	void GetVideoResolution( DWORD dwResolutionIndex, DWORD* dwWidth, DWORD* dwHeight );
	int StringToken(LPTSTR str, LPCTSTR szToken, char** szArray);
	
	bool mf_PrepareDibBuffer(LPBITMAPINFO *lplpbi, DWORD dwX, DWORD dwY);
	
	void onPlayMultiRaw();
	
	bool FileNameCheck();
	void RefreshDisplayArea();
	void GetDisplayInfo(int nResolution, RECT& rect, int& nFontSize);
	void SaveImage();

private:
	// KMpeg4 Handle
	HANDLE m_hKMpeg4;
	// Slide for time
	//CProSliderCtrl m_cSCTime;
	CSliderCtrl m_cSCTime;
	// Play rate slide
	CSliderCtrl m_cSCPlayRate;
	// Display control
	CStatic m_cDisplay;
	// Direction backward
	CButton m_cBnBack;
	// Direction forward
	CButton m_cBnForward;
	// Operation Stop
	CButton m_cBnStop;
	// Operation Pause
	CButton m_cBnPause;
	// File name for play
	//CString m_sFileName;
	// File path for exec file
	char m_szFilePath[4096];
	// Render Info
	MEDIA_RENDER_INFO m_mri;
	// File complete
	bool m_bFileComplete;
	// Connection config setting
	MEDIA_CONNECTION_CONFIG m_mcc;
	// Begin time
	DWORD m_dwBeginTime;
	// End time
	DWORD m_dwEndTime;
	// Current play rate
	DWORD m_dwPlayRate;
	// Take a shoot or not
	bool m_bSnapShoot;
	// Save file type
	int m_nFileType;
	// String for begin time
	CString m_sBeginTime;
	// End time for play file
	CString m_sEndTime;
	// Current time of playing file
	CString m_sCurrentTime;
	// Duration
	DWORD m_dwDuration;
	// Direction
	bool m_bForward;
	// Position on Slide
	DWORD m_dwSlidePos;
	// Frame by Frame
	bool m_bFBF;
	// Save picture button
	CButton m_cBnPicSave;
	// Font Size
	int m_nFontSize;
	// File ext. type
	int m_nFileExtType;
	// Flag for time code change
	bool m_bTimeCodeChange;
	// Background display brush
	HBRUSH m_hBKBrush;
	HPEN m_hPen;
	BYTE *m_bSaveBuffer;
	DWORD m_dwSaveBufferLen;


	//11092007 add by Marty for multiple Raw files playback
	CString m_sRawFile1Name;
	CString m_sRawFile2Name;
	CString m_sRawFile3Name;

public:
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnStnClickedStatic4();
};
