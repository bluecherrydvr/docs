// DvrFCDDlg.h : header file
//

#pragma once
#include "sdvr_sdk.h"
#include "DVRSystem.h"
#include "DirectDraw.h"
#include "ChannelSettingDlg.h"
#include "BoardSettingDlg.h"
#include "WaveOut.h"
#include "RegionSetting.h"
#include "RelaysAndSensorsDlg.h"

typedef enum
{	
	ENAME_INIT_BOARD = 0,
	ENAME_UNINIT_BOARD,
	ENAME_CONNECT_CHANNEL,
	ENAME_DISCONNECT_CHANNEL,
	ENAME_CONNECT_ALL,
	ENAME_DISCONNECT_ALL,
	ENAME_KILL_UI_CONTROL_THREAD,
	MAX_EVENT_UI_CONTROL
}ENUM_EVENT_UI_CONTROL;


// CDvrFCDDlg dialog
class CDvrFCDDlg : public CDialog
{
// Construction
public:
	CDvrFCDDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DVRFCD_DIALOG };

	CMenu*		m_pMenu;	
	
	int m_iBoardCount;
	int m_iVdoCount[MAX_SUPPORT_BOARD];
	int m_iAdoCount[MAX_SUPPORT_BOARD];	
	CDVRSystem* m_pDVRSystem[MAX_SUPPORT_BOARD];	

	HANDLE  				m_hUIControlThread;
	static DWORD WINAPI	Thread_UIControl(LPVOID pParam);	
	HANDLE					m_hEventUIControl[MAX_EVENT_UI_CONTROL];

	sdvr_err_e				InitBoard(int iBoardIndex);
	sdvr_err_e				UnInitBoard(int iBoardIndex);		

	bool					    SetChannel(bool bConnect, int iCh);	// Connect or disconnect channel.
	bool						SetInitDvrMenuState(bool bShow, bool bCheck, int iBoard);
	bool						SetConnectChMenuState(bool bShow, bool bCheck, int iCh, int iBoard);	// iCh=-1 means set all channel.	
	bool						acquireBuffer(int iBoard, int iCh, sdvr_frame_type_e frame_type, sx_bool is_primary, sdvr_chan_handle_t handle);

	// Event action board and channel
	int						m_iEventOpenBoard;		// for InitBoard().
	int                      m_iEventConnectCh;		// for ConnectChannel().
	bool					m_bIsInitAllDvrBoardOK;

	bool						m_bIsInitDvrBoardOK[MAX_SUPPORT_BOARD];
	bool						m_bIsConnectChannelOK[MAX_SUPPORT_BOARD][MAX_BOARD_CHANNEL];	
//	BOOL					m_bShowBoard[MAX_SUPPORT_BOARD];

	int						m_iVdoWidth_Raw[MAX_SUPPORT_BOARD][MAX_BOARD_CHANNEL];
	int						m_iVdoHeight_Raw[MAX_SUPPORT_BOARD][MAX_BOARD_CHANNEL];
	int                      m_iVdo_YBufSize[MAX_SUPPORT_BOARD][MAX_BOARD_CHANNEL];
	int                      m_iVdo_UBufSize[MAX_SUPPORT_BOARD][MAX_BOARD_CHANNEL];
	int                      m_iVdo_VBufSize[MAX_SUPPORT_BOARD][MAX_BOARD_CHANNEL];

	// Draw video function.
	CDirectDraw*					m_pDirectDrawDisplay[MAX_SUPPORT_BOARD][MAX_BOARD_CHANNEL];
	//BOOL						InitDrawDib(int iCh);
	BOOL						InitDrawDib(int iCh , int iBoard );
	BOOL						CloseDrawDib(int iCh, int iBoard );
	void							DrawVideoYUV(int iCh, char * y, char * u, char *v, int iBoard );
	BOOL						m_bInitDrawOK[MAX_SUPPORT_BOARD][MAX_BOARD_CHANNEL];
//	BOOL						m_bShowBoard[MAX_SUPPORT_BOARD];
	int							m_iRadioBtnBoard;

	bool						m_bCanDoCloseProcess;	// if in connect all/disconnect all, can't do close program.
	bool						m_bProcessIsClosing;
	bool						m_bIsUseControlAll;		// Avoid to show channel setting dialog: means use default channel setting.	

	// Wave out raw audio.
	CWaveOut*				m_pWaveOut[MAX_SUPPORT_BOARD][MAX_BOARD_CHANNEL];
	BOOL					OpenWaveOut(int iCh, int iBoard );
	BOOL					CloseWaveOut(int iCh, int iBoard );
	BOOL                   m_bOpenWaveOutOK[MAX_SUPPORT_BOARD][MAX_BOARD_CHANNEL];
	
	
	CChannelSettingDlg*	    m_pChannelSettingDlg;
	BOOL                   m_bCreateAndInitialChannelSettingDlgOK;
	
	// Region Setting.
	CRegionSetting*			m_pRegionSettingDlg;
	BOOL                   m_bCreateAndInitialRegionSettingDlgOK;	
	CRect					m_rcRegionSettingDlgDrawArea;
	sdvr_err_e				ResetMotion(int iCh);		// After disconnect, remove all channel setting and disable motion setting.
	void						DrawMotionAlarm(int iCh);
	void					    RectReverse2MB(int iCh);

	// Video Adjust.
	void						ConfigBrightness(int iCh, BOOL bGet);
	void						ConfigContrast(int iCh, BOOL bGet);
	void						ConfigSaturation(int iCh, BOOL bGet);
	void						ConfigHue(int iCh, BOOL bGet);
	void						ConfigSharpness(int iCh, BOOL bGet);
	sdvr_image_ctrl_t			m_image_ctrl[MAX_SUPPORT_BOARD][MAX_BOARD_CHANNEL];

	// PTZ.
	int		m_iBaudRate;
	int		m_iAddress;	
	int      m_iSpeed;
	bool 	m_bRS485Init[MAX_SUPPORT_BOARD];


	CRelaysAndSensorsDlg* m_pRelaysAndSensorsDlg;

	void GetSensorArrayTrueFlag(int iValue);	

	CButton m_RadioBtnBoard;

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
	afx_msg void OnClose();	
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	HRESULT	 OnCreateBoardSettingDlg(WPARAM wParam, LPARAM lParam);
	HRESULT	 OnCreateChannelSettingDlg(WPARAM wParam, LPARAM lParam);
	HRESULT	 OnModifyChannelSettingDlg(WPARAM wParam, LPARAM lParam);
	HRESULT	 OnCreateReginSettingDlg(WPARAM wParam, LPARAM lParam);
	HRESULT	 OnMD_Modify(WPARAM wParam, LPARAM lParam);
	HRESULT	 OnMD_Enable(WPARAM wParam, LPARAM lParam);
	HRESULT	 OnPTZ_Action(WPARAM wParam, LPARAM lParam);
	HRESULT     OnCreateRSDlg(WPARAM wParam, LPARAM lParam);
	HRESULT     OnConfigSensors(WPARAM wParam, LPARAM lParam);
	HRESULT     OnRelayTrigger(WPARAM wParam, LPARAM lParam);
	HRESULT	 OnGetProtectID(WPARAM wParam, LPARAM lParam);
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);	

public:
	afx_msg void OnBnClickedRadioBoard1();
	afx_msg void OnBnClickedRadioBoard2();
	afx_msg void OnBnClickedRadioBoard3();
	afx_msg void OnBnClickedRadioBoard4();
};
