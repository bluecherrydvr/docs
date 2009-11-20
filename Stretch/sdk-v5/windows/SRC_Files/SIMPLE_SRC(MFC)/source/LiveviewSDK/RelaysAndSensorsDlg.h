#pragma once

class CDvrFCDDlg;

// CRelaysAndSensorsDlg

class CRelaysAndSensorsDlg : public CDialog
{
	DECLARE_DYNAMIC(CRelaysAndSensorsDlg)

public:
	CRelaysAndSensorsDlg(CWnd* pParent = NULL);   //
	virtual ~CRelaysAndSensorsDlg();

// 對話方塊資料
	enum { IDD = IDD_DIALOG_RELAYS_SENEORS };

	CDvrFCDDlg* m_pMainDlg;
	int			 m_iBoardIndex;

	CButton		 m_BtnRelays[MAX_RELAYS_SENSORS];
	CButton		 m_BtnSensors[MAX_RELAYS_SENSORS];
	CComboBox  m_Cbo_SensorTrigger[MAX_RELAYS_SENSORS];
	bool         m_bIsTrigger;
	CListBox		 m_ListSensor;

	void			 ShowSensorAlarm(int iSensor);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    //

	DECLARE_MESSAGE_MAP()	
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRelaysSensorsSetExit();
	afx_msg void OnBnClickedButtonTrigger();
	afx_msg void OnBnClickedSensorsSet();
};
