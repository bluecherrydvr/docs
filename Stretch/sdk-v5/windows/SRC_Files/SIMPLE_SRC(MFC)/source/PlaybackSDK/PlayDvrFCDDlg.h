// DvrFCDDlg.h : header file
//

#pragma once
#include "sdvr_sdk.h"
#include "DVRSystem.h"
#include "DirectDraw.h"
#include "WaveOut.h"

#include "../../codecs/avmux/mp4/mp4mux.h"
#include "../../codecs/g711/g711.h"

#include "avi_api.h"

typedef enum
{	
	ENAME_INIT_BOARD = 0,
	ENAME_UNINIT_BOARD,
	ENAME_START_DECODE_CH,
	ENAME_STOP_DECODE_CH,

	ENAME_KILL_UI_CONTROL_THREAD,
	MAX_EVENT_UI_CONTROL
}ENUM_EVENT_UI_CONTROL;

typedef struct
{
	sx_int32 vtrack;            // Video track handle within mp4 file
	sx_int32 atrack;            // Audio track handle within mp4 file
	mp4_handle FileHandle;      // The file handle to the mp4 file.
	sx_int64 dts;
} mp4File_t;

typedef struct
{
	CWnd* pp;
	int ChannelID;
	int BoardIndex;
}PACKET_TRREAD_STRUCT;

// CDvrFCDDlg dialog
class CDvrFCDDlg : public CDialog
{
// Construction
public:
	CDvrFCDDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DVRFCD_DIALOG };

	CMenu		*m_pMenu;

	char		m_szCurExeDir[_MAX_PATH];

	//////////////////////////////////////////////////////////////////////////
	//Open mov File
	mp4File_t	*m_pMp4OpenFile[MAX_SUPPORT_CHANNEL];

	bool		m_bOpenMovFileReady[MAX_SUPPORT_CHANNEL];

	sx_uint32	m_PlaySeq[MAX_SUPPORT_CHANNEL];
	int			m_iSendCounter[MAX_SUPPORT_CHANNEL];

	PACKET_TRREAD_STRUCT	m_PacketThreadStructure[MAX_SUPPORT_CHANNEL];

	bool OpenMovFile(int iCh, char szFileName[MAX_PATH]);
	bool CloseMovFile(int iCh);

	bool SendOneAVEncFrame(int iBoard, int iCh);

	HANDLE	m_hKillThreadGetMovPacketData[MAX_SUPPORT_CHANNEL];

	HANDLE	m_hThreadGetMovPacketData[MAX_SUPPORT_CHANNEL];
	static  DWORD WINAPI ThreadGetMovPacketData(LPVOID pParam);
	//////////////////////////////////////////////////////////////////////////

	int m_iBoardCount;
	int m_iVdoCount[MAX_SUPPORT_BOARD];
	int m_iAdoCount[MAX_SUPPORT_BOARD];

	sdvr_video_std_e m_VideoStandard[MAX_SUPPORT_BOARD];
	CDVRSystem* m_pDVRSystem[MAX_SUPPORT_BOARD];	

	HANDLE  				m_hUIControlThread;
	static DWORD WINAPI	Thread_UIControl(LPVOID pParam);	
	HANDLE					m_hEventUIControl[MAX_EVENT_UI_CONTROL];

	sdvr_err_e				InitBoard(int iBoardIndex);
	sdvr_err_e				UnInitBoard(int iBoardIndex);		

	bool	SetDecodeChannel(bool bConnect, int iCh);	// Start or Stop Decode channel.

	bool	DeterminIfDoDecodeAction(int iCh, bool *bIsHwDecodeAction, 
									bool *bIsDecodeFile, int *iSelEncodeCh, char szFileFullPath[_MAX_PATH]);

	bool	SetRecording(bool bRecord, int iCh); //Start or Stop recording

	bool	SetInitDvrMenuState(bool bShow, bool bCheck);
	bool	SetDecodeChMenuState(bool bShow, bool bCheck, int iCh, int iBoard);	// iCh=-1 means set all channel.	

	bool	acquireBuffer(int iBoard, int iCh, sdvr_frame_type_e frame_type, sx_bool is_primary, sdvr_chan_handle_t handle);

	int							m_iEventOpenBoard;		// for InitBoard().

	int							m_iEventEncodeCh;
	int							m_iEventDecodeCh;

	bool						m_bIsInitAllDvrBoardOK;
	bool						m_bIsInitDvrBoardOK[MAX_SUPPORT_BOARD];
	bool						m_bIsEncodeChOK[MAX_SUPPORT_CHANNEL];
	bool						m_bIsDecodeChOK[MAX_SUPPORT_CHANNEL];

	int						m_iVdoWidth_Raw[MAX_BOARD_CHANNEL];
	int						m_iVdoHeight_Raw[MAX_BOARD_CHANNEL];

	BOOL					InitDrawDib(int iCh , int iBoard);
	BOOL					CloseDrawDib(int iCh , int iBoard);
	void						DrawVideo(int iCh, BYTE *pBuffer, int iBoard);

	CWaveOut				*m_pWaveOut;
	int						m_iPlayAudioCh;

	BOOL					m_bShowBoard[MAX_SUPPORT_BOARD];
	BOOL					m_bInitDrawOK[MAX_SUPPORT_BOARD][MAX_BOARD_CHANNEL];
	CDirectDraw				*m_pDirectDrawDisplay[MAX_SUPPORT_BOARD][MAX_BOARD_CHANNEL];
	BYTE					*m_pbyYUVBuffer[MAX_SUPPORT_BOARD][MAX_BOARD_CHANNEL];

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
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedRadioBoard1();
	afx_msg void OnBnClickedRadioBoard2();
	afx_msg void OnBnClickedRadioBoard3();
	afx_msg void OnBnClickedRadioBoard4();
};
