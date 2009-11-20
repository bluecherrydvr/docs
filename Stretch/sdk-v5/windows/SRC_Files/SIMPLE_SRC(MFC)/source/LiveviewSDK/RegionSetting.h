#pragma once


class CDvrFCDDlg;

// CRegionSetting

class CRegionSetting : public CDialog
{
	DECLARE_DYNAMIC(CRegionSetting)

public:
	CRegionSetting(CWnd* pParent = NULL);   //
	virtual ~CRegionSetting();

// 對話方塊資料
	enum { IDD = IDD_DIALOG_DRAW_REGION_SETTING };

	CDvrFCDDlg* m_pMainDlg;
	CButton		 m_BtnMD[SDVR_MAX_MD_REGIONS];
	CButton		 m_BtnRemove;
	int			 m_iChannelIndex;	
	
	// Draw video function.
	BOOL                   m_bLBtnDown;
	BOOL					m_bIsDrawArea;
	BOOL                   m_bStopDrawRect;
	CPoint					m_ptStart;
	CPoint					m_ptEnd;
	CRect                   m_rcDrawArea;
	CDirectDraw*				m_pDirectDrawDisplay;	
	int						m_iVdoWidth_Raw;
	int						m_iVdoHeight_Raw;		
	BOOL					InitDrawDib();
	BOOL					CloseDrawDib();
	void						DrawVideoYUV(char * y, char * u, char *v);	
	void                    DrawRegion();
							
	BOOL					FindEmptyRegion();
	int						m_iNextRegionID; // Start from 0.
	int                      m_iMDRectLayer;	// Now select MD Layer(Layer ID).
	CListBox					m_ListRectID;
	void						RefreshListID();
	CChannelParam*			m_pChannelParam;

	// Motion alarm region.	
	BOOL					m_bIsRect;	// means LbtnDown is start at a correct draw area.
	int                      m_iMDThreshold[SDVR_MAX_MD_REGIONS];
	BOOL                   m_bEnableMD;
	int						m_iRegionCount;	// start from 0.
	int						m_iMDValueFrequency;
	
	BOOL					m_bStatusIsAddRegion;
	void						DrawMotionAlarm();	
	int						m_iListCurSelectNum;
	int						m_iCurListPos;

	// PTZ
	CComboBox				m_Cbo_BaudRate;
	CComboBox				m_Cbo_Address;
	CComboBox				m_Cbo_Speed;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRegionSettingOk();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);	
	afx_msg void OnBnClickedRadioMd0();
	afx_msg void OnBnClickedRadioMd1();
	afx_msg void OnBnClickedRadioMd2();
	afx_msg void OnBnClickedRadioMd3();
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonRemoveRegion();
	afx_msg void OnBnClickedButtonAddRegion();
	afx_msg void OnBnClickedButtonSetExit();	
	afx_msg void OnCbnSelchangeComboBaudRate();
	afx_msg void OnCbnSelchangeComboAddress();
	afx_msg void OnCbnSelchangeComboSpeed();
	afx_msg void OnBnClickedButtonInitRs485();
	afx_msg void OnBnClickedButtonUp();
	afx_msg void OnBnClickedButtonLeft();
	afx_msg void OnBnClickedButtonRight();
	afx_msg void OnBnClickedButtonDown();
	afx_msg void OnBnClickedButtonZoomIn();
	afx_msg void OnBnClickedButtonZoomOut();
	afx_msg void OnBnClickedButtonFocusNear();
	afx_msg void OnBnClickedButtonFocusFar();
	afx_msg void OnBnClickedButtonIrisLarge();
	afx_msg void OnBnClickedButtonIrisSmall();
	HRESULT	 OnPTZ_ActionStop(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLbnSelchangeListRectId();		
};
