#pragma once

class CDvrFCDDlg;


// CBoardSettingDlg

class CBoardSettingDlg : public CDialog
{
	DECLARE_DYNAMIC(CBoardSettingDlg)

public:
	CBoardSettingDlg(CWnd* pParent = NULL);
	virtual ~CBoardSettingDlg();


	enum { IDD = IDD_DIALOG_BOARD_SETTING };

	CDvrFCDDlg* m_pMainDlg;
	CButton		 m_BtnVideoStds[MAX_BOARD_SUPPORT_VIDEO_STANDARD];
	CButton      m_BtnEnableCustomFontTable;
	int			 m_iFont_Start_Code;
	int		     m_iFont_End_Code;
	int			 m_iFont_Y_Color;
	int			 m_iFont_U_Color;
	int			 m_iFont_V_Color;

	sdvr_font_table_t m_font_table;
	CString      m_strFontTable;

	int      m_iBtnVideoStandard;
	int		m_iBoardIndex;	
	int      m_iPCITimeout;	

	void    ConfigInitBoardData();


protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

public:		
	afx_msg void OnBnClickedSetBoard();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedCheckCustomFontTable();
};
