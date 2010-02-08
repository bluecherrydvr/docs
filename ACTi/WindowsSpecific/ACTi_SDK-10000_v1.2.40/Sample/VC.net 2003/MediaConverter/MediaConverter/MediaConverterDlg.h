// MediaConverter v1.5.05Dlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "AVIWriter.h"
#include "JPEGWriter.h"
#include "MP4Writer.h"
#include "FolderDlg.h"
 
typedef enum {
	FILE_EXT_NONE = 0,
	FILE_EXT_RAW = 1,
	FILE_EXT_JPG,
	FILE_EXT_AVI,
	FILE_EXT_MP4,
	FILE_EXT_MAX,
}FILE_EXT_TYPE;

typedef enum {
	FILE_SRC_NOE,
	FILE_SRC_RAW = FILE_EXT_RAW,
	FILE_SRC_JPG,
	FILE_SRC_AVI,
	FILE_SRC_MP4,
	FILE_SRC_MAX,
}FILE_SRC_TYPE;


// CMediaConverterDlg dialog
class CMediaConverterDlg : public CDialog
{
// Construction
public:
	CMediaConverterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MEDIACONVERTERV1505_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedRadio11();
	afx_msg void OnBnClickedRadio12();
	afx_msg void OnBnClickedRadio10();

	afx_msg void OnBnClickedRadio3();
	afx_msg void OnBnClickedRadio4();
	afx_msg void OnBnClickedRadio5();
	afx_msg void OnBnClickedRadio6();
	afx_msg void OnBnClickedRadio7();
	afx_msg void OnBnClickedRadio8();
	afx_msg void OnBnClickedRadio9();
    

	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck3();
	
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();

	afx_msg void RunProcess();
	afx_msg bool ConvertRawToSingleRawFile( void );
	afx_msg bool ConvertRawToSingleAVIFile( const time_t tSpecifyBeginDateTime = 0, const time_t tSpecifyEndDateTime = 0);
	afx_msg bool ConvertRawToMultiJPGFile( const time_t tSpecifyBeginDateTime = 0, const time_t tSpecifyEndDateTime = 0);
	afx_msg bool ConvertMP4ToMultiJPGFile();
	
	afx_msg void OnPrintInfo(DWORD arg1, DWORD arg2);
	
	afx_msg void OnPrintMP4Info();
	afx_msg void OnFileClose();
	afx_msg void OnFileCloseTimer();
	afx_msg LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//void CmdLineProcess() ;
	
	CButton m_OSD;

	CButton m_OSD1;
	CButton m_OSD1_1;
	CButton m_OSD1_2_1;
	CEdit	m_OSD1_2_2;
	CStatic m_OSD1_2_3;

	CButton m_OSD2;
	CButton m_OSD2_1;
	CButton m_OSD2_2;
	CButton m_OSD2_3;
	
	CListBox m_Message;
	CString  m_FileName;
	CString  m_OutFileName;
   	CString  m_PathName;
	CString  m_FileExt ;
	CString  m_FolderName;
   

	CString  m_WriteFolderName;
    RawFileTail_t	m_RawFileTail;

	CString m_csOSDText;
	bool m_bConvertFile;
	bool m_bConvertFolder;
	bool m_bConvertEnd;
	// File ext. type
	int m_nFileSrcType;
	int m_nFileExtType;
   

	//int m_nOSDTimeType;
	
	CAVIWriter  m_cAVIWriter;
    CJPEGWriter m_cJPGWriter;
	CMP4Writer  m_cMP4Writer;
	HANDLE	m_hThread;
	DWORD m_dwThreadID;
	DWORD m_nConvertedFile;
	DWORD m_nIFrame;
	DWORD m_nPFrame;
	DWORD m_nAFrame;
	afx_msg void OnBnClickedButton4();
	CButton m_SRC1;
	CButton m_SRC2;
	
		
	
	CEdit m_edtWriteDstPath;
	afx_msg void OnBnClickedButton6();

	CEdit m_edtUSh;
	CEdit m_edtUSm;
	CEdit m_edtUSs;
	CEdit m_edtUSendh;
	CEdit m_edtUSendm;
	CEdit m_edtUSends;

	CEdit m_edtFileBeginDateTime;
	CEdit m_edtFileEndDateTime;
    
	time_t m_tFileEndDateTime;
	time_t m_tFileBeginDateTime;
	time_t m_tFileDateTimeZone;

	time_t m_tSpecifyBeginDateTime;
	time_t m_tSpecifyEndDateTime;

	

	
protected:
	void DisplayFileDateTime( CMpeg4Reader * pFileReader );
	void HandleUserSpecifyDateTime( void);
	BOOL CheckSpecifyDateTime();


public:
	CButton m_chkRADIO10;

	afx_msg void OnBnClickedRadio13();
	CButton m_Rcmp4tojpg;
	CButton m_chkRADIO11;
	CButton m_chkRADIO12;
};
