#pragma once

class CDvrFCDDlg;
// CChannelSettingDlg



class CChannelSettingDlg : public CDialog
{
	DECLARE_DYNAMIC(CChannelSettingDlg)

public:
	CChannelSettingDlg(CWnd* pParent = NULL);   // 
	virtual ~CChannelSettingDlg();
	
	enum { IDD = IDD_DIALOG_CHANNEL_SETTING };

	CDvrFCDDlg* m_pMainDlg;
	int m_iBoardIndex;
	int m_iChannelIndex;
	bool m_bIsModify;

	CButton	m_BtnRes[5];
	CButton m_BtnVpp[2];
	BOOL   m_bBtnEnableOSD[SDVR_MAX_OSD];
	CComboBox m_Cbo_OSD_Pos[SDVR_MAX_OSD];
	CString  m_szOSD_Text[SDVR_MAX_OSD];
	int      m_iOSD_XPos[SDVR_MAX_OSD];
	int      m_iOSD_YPos[SDVR_MAX_OSD];
	CComboBox m_Cbo_OSD_TimeStamp[SDVR_MAX_OSD];
	CSliderCtrl m_Slider_OSD_Translucent[SDVR_MAX_OSD];
	CEdit	 m_Edit_OSD_XPos[SDVR_MAX_OSD];
	CEdit	 m_Edit_OSD_YPos[SDVR_MAX_OSD];
	CEdit	m_edit_OSDText[SDVR_MAX_OSD];	
	CSliderCtrl m_Slider_Brightness;
	CSliderCtrl m_Slider_Contrast;
	CSliderCtrl m_Slider_Saturation;
	CSliderCtrl m_Slider_Hue;
	CSliderCtrl m_Slider_Sharpness;
	
	int      m_iFrameRate;
	BOOL	m_bMute;

	CRect                   m_rcDrawArea;
	CDirectDraw*				m_pDirectDrawDisplay;	
	int						m_iVdoWidth_Raw;
	int						m_iVdoHeight_Raw;		
	BOOL					InitDrawDib();
	BOOL					CloseDrawDib();
	void						DrawVideoYUV(char * y, char * u, char *v);	
	void						DrawMotionAlarm();

	sdvr_osd_config_ex_t      m_OSD_Config_ex[SDVR_MAX_OSD];
	
	void    ConfigInitChannelData();
	void    ReloadData();
	void    CreateDefaultData();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSetChannel();
	virtual BOOL OnInitDialog();				
	afx_msg void OnNMCustomdrawSliderOsd1Translucent(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderOsd2Translucent(NMHDR *pNMHDR, LRESULT *pResult);	
	afx_msg void OnCbnSelchangeComboOsd1Pos();
	afx_msg void OnCbnSelchangeComboOsd2Pos();	
	afx_msg void OnNMCustomdrawSliderBrightness(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderContrast(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderSaturation(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderHue(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderSharpness(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
