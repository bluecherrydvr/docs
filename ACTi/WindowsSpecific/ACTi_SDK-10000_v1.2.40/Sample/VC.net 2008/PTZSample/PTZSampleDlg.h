// PTZSampleDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "PTZex.h"
#include "SDK10000.h"

#include <string>

#define PTZ_MEDIA_SOURCE_RS		1
#define PTZ_MEDIA_SOURCE_FILE	2

// CPTZSampleDlg dialog
class CPTZSampleDlg : public CDialog
{
// Construction
public:
	CPTZSampleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PTZSAMPLE_DIALOG };

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
    afx_msg void OnBnClickedOk2();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedButton1();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnBnClickedButton4tiltup();
    afx_msg void OnBnClickedButton4leftdown();
    afx_msg void OnBnClickedButton4panleft();
    afx_msg void OnBnClickedButton4leftup();
    afx_msg void OnBnClickedButton4stop();
    afx_msg void OnBnClickedButton4tiltdown();
    afx_msg void OnBnClickedButton4rightup();
    afx_msg void OnBnClickedButton4panright();
    afx_msg void OnBnClickedButton4rightdown();
    afx_msg void OnBnClickedButton4connect();
    afx_msg void OnBnClickedButton4disconnect();
    afx_msg void OnBnClickedButton4();


	DWORD m_dwPanTiltSpeed;

	//abs PTZ valuable
	float m_fPanDegree;
	float m_fTilteDegree;
	float m_fZoomDegree;
	CComboBox m_ctrlIsCCW;
	DWORD m_dwABSPTZspeed;


    // SDK handle
    HANDLE m_hNet;
    bool m_bConnected;
    CComboBox m_ctrlParity;
    CComboBox m_ctrlBaudrate;
    CStatic m_ctrlVW;

    CString m_csFileName;
    //HANDLE  m_hPTZ;
    //PTZInfo m_sInfo;
	MEDIA_PTZ_PROTOCOL m_mPTZ;
    
    CEdit m_ctrlProtocolFileName;
    CButton m_ctrlSelectProtocolFile;                                    
    CComboBox m_ctrlVendor;
    CComboBox m_ctrlComboProtocol;

    CEdit m_ctrlProtocol;
    CEdit m_ctrlPTZCmdParam1;
    CEdit m_ctrlPTZCmdParam2;
    CEdit m_ctrlPTZURLCmd;
    CComboBox m_ctrlPTZCmdList;


	CComboBox m_EncoderPTZCmdList;
	CComboBox m_EncoderParamStrList;
	CEdit m_ctrlEncoderCmdParam1;
	CEdit m_ctrlEncoderCmdParam2;
	CEdit m_ctrlEncoderCmdParam3;
	CEdit m_ctrlEncoderCmdParam4;
	CEdit m_ctrlEncoderCmd;
	CEdit m_ctrlEncoderReturnVal;

    CEdit m_ctrlAddrID;
    afx_msg void OnBnClickedButton4selefile();
    afx_msg void OnBnClickedRadio4file();
    afx_msg void OnBnClickedRadio4buildin();
    afx_msg void OnEnChangeEdit4addrid();
    afx_msg void OnCbnSelchangeCombo4vendor();
    CComboBox m_ctrlVendorList;
    CComboBox m_ctrlProtocolList;
    CComboBox m_ctrlSendMode;
    int m_iSendMode;
    void SendMoveCommand(char *pcaCommand, int iParam1);

    afx_msg void OnBnClickedButton4zoomin();
    afx_msg void OnBnClickedButton4zoomstop();
    afx_msg void OnBnClickedButton4zoomout();
    CString m_csLastCommand;
    afx_msg void OnBnClickedButton4focusin();
    afx_msg void OnBnClickedButton4focusstop();
    afx_msg void OnBnClickedButton4focusout();
    afx_msg void OnBnClickedButton15();
    afx_msg void OnBnClickedButton16();
    afx_msg void OnBnClickedButton4stop2();
    afx_msg void OnBnClickedButton4up2();
    afx_msg void OnBnClickedButton4left2();
    afx_msg void OnBnClickedButton4right2();
    afx_msg void OnBnClickedButton4down2();
    afx_msg void OnBnClickedButton4osdon();
    afx_msg void OnBnClickedButton4osdoff();
    afx_msg void OnBnClickedButton4osdleave();
    afx_msg void OnBnClickedButton4blcon();
    afx_msg void OnBnClickedButton4blcon4blcoff();
    afx_msg void OnBnClickedButton4blcon4dandnon();
    afx_msg void OnBnClickedButton4blcon4dandnoff();
    afx_msg void OnBnClickedButton4blcon4daynightauto();
    afx_msg void OnBnClickedButton4blcon4daynightautooff();
    afx_msg void OnBnClickedButton14();
    afx_msg void OnBnClickedButton17();
    afx_msg void OnBnClickedButton18();
    afx_msg void OnBnClickedButton19();
    afx_msg void OnBnClickedButton20();
    afx_msg void OnBnClickedButton21();
    afx_msg void OnBnClickedButton22();
    afx_msg void OnBnClickedButton23();
    afx_msg void OnBnClickedButton24();
    afx_msg void OnBnClickedButton25();
    afx_msg void OnBnClickedButton26();
    afx_msg void OnBnClickedButton27();
    afx_msg void OnBnClickedButton28();
    afx_msg void OnBnClickedButton29();
    afx_msg void OnBnClickedButton30();
    afx_msg void OnBnClickedButton31();
    afx_msg void OnBnClickedButton32();
    afx_msg void OnBnClickedButton33();
    afx_msg void OnBnClickedButton34();
    afx_msg void OnBnClickedButton35();
    afx_msg void OnBnClickedButton36();
    afx_msg void OnBnClickedButton37();
    afx_msg void OnBnClickedButton38();
    afx_msg void OnBnClickedButton39();
    afx_msg void OnBnClickedButton40();
    afx_msg void OnBnClickedButton41();
    afx_msg void OnBnClickedButton42();
    afx_msg void OnBnClickedButton43();
    afx_msg void OnBnClickedButton44();
    afx_msg void OnBnClickedButton45();
    afx_msg void OnBnClickedButton46();
    afx_msg void OnBnClickedButton47();
    afx_msg void OnBnClickedButton48();
    afx_msg void OnBnClickedButton49();
    afx_msg void OnBnClickedButton50();
    afx_msg void OnBnClickedButton51();
    afx_msg void OnBnClickedButton52();
    afx_msg void OnBnClickedButton53();
    afx_msg void OnBnClickedButton54();
    afx_msg void OnBnClickedButton55();
    afx_msg void OnBnClickedButton56();
    afx_msg void OnBnClickedButton57();
    afx_msg void OnBnClickedButton58();
    afx_msg void OnBnClickedButton59();
    afx_msg void OnBnClickedButton60();
    afx_msg void OnBnClickedButton61();
    afx_msg void OnBnClickedButton62();
    afx_msg void OnBnClickedButton63();
    afx_msg void OnBnClickedButton3();
    afx_msg void OnBnClickedButton5();
    bool m_bTest;
    void Testing();
    int m_ctrlCommandCount;
    afx_msg void OnBnClickedOk4();
	afx_msg void OnEnChangeEdit4urlcmdoutput();
	afx_msg void OnCbnSelchangeCombo4command();
	afx_msg void OnEnChangeEdit4param1();
	afx_msg void OnEnChangeEdit4param2();
	afx_msg void OnEnChangeEdit10();
	afx_msg void OnEnChangeEdit11();
	afx_msg void OnEnChangeEdit12();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnCbnSelchangeCombo4baudrate();
	afx_msg void OnCbnSelchangeCombo4parity();
	afx_msg void OnCbnSelchangeCombo2();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnCbnSelchangeCombo3();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton10();
};
