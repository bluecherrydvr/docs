// DvrFCDDlg.h : header file
//

#pragma once
#include "sdvr_sdk.h"
#include "DVRSystem.h"
#include "DirectDraw.h"
#include "WaveOut.h"
#include "VideoEncoder.h"

#include "../../codecs/avmux/mp4/mp4mux.h"
#include "avi_api.h"

typedef enum
{	
	ENAME_INIT_BOARD = 0,
	ENAME_UNINIT_BOARD,
	ENAME_START_PREVIEW_CH,
	ENAME_STOP_PREVIEW_CH,
	ENAME_START_ENCODE_CH,
	ENAME_STOP_ENCODE_CH,
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
	sx_int32 vtrack;            // Video track handle within mp4 file
	sx_int32 atrack;            // Audio track handle within mp4 file
	mp4_handle FileHandle;      // The file handle to the mp4 file.
	sx_int64 dts;
} mp4File_t_sec;

typedef struct
{
	CWnd* pp;
	int ChannelID;
}PACKET_TRREAD_STRUCT;

class CFlatPopupMenu ;

// CDvrFCDDlg dialog
class CDvrFCDDlg : public CDialog
{
// Construction
public:
	CDvrFCDDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DVRFCD_DIALOG };

	CMenu						*m_pMenu;
	CDvrFCDDlg					*m_pMainDlg;
	CVideoEncoder				*m_pVideoEncoderDlg;
	sdvr_chan_def_t				m_chan_def;
	//CFlatPopupMenu	*m_pMenu;
	char						m_szCurExeDir[_MAX_PATH];
	//////////////////////////////////////////////////////////////////////////
	
	mp4File_t					*m_pMp4SaveFile[MAX_BOARD_CHANNEL];//Save mov File
	mp4File_t_sec				*m_pMp4SaveFile_sec[MAX_BOARD_CHANNEL];

	aviFile_t					*m_pAviSaveFile[MAX_BOARD_CHANNEL];//Save avi File
	aviFile_t_sec				*m_pAviSaveFile_sec[MAX_BOARD_CHANNEL];

	bool						StopRecording(int iCh);
	bool						StartRecording_Primary(int iCh, char szFileName[MAX_PATH],sdvr_venc_e EncoderType = SDVR_VIDEO_ENC_NONE);
	bool						StartRecording_Secondary(int iCh, char szFileName[MAX_PATH],sdvr_venc_e EncoderType = SDVR_VIDEO_ENC_NONE);
	//MPEG4/H.264
	bool						PutPrimaryMp4Packet(int iCh, sdvr_frame_type_e frame_type, sdvr_av_buffer_t *buf);
	bool						PutSecondaryMp4Packet(int iCh, sdvr_frame_type_e frame_type, sdvr_av_buffer_t *buf_sec);
	//G711
	bool						PutEncAudioPacket(int iCh, sdvr_frame_type_e frame_type, sdvr_av_buffer_t *buf);
	//MJPEG
	bool						PutEncPrimaryMJPEGPacket(int iCh, sdvr_frame_type_e frame_type, sdvr_av_buffer_t *buf);
	bool						PutEncSecondaryMJPEGPacket(int iCh, sdvr_frame_type_e frame_type, sdvr_av_buffer_t *buf);
	//////////////////////////////////////////////////////////////////////////
	int							m_iBoardCount;
	int							m_iVdoCount[MAX_SUPPORT_BOARD];
	int							m_iAdoCount[MAX_SUPPORT_BOARD];

	sdvr_video_std_e			m_VideoStandard[MAX_SUPPORT_BOARD];
	CDVRSystem					*m_pDVRSystem[MAX_SUPPORT_BOARD];	

	HANDLE  					m_hUIControlThread;
	static DWORD WINAPI			Thread_UIControl(LPVOID pParam);	
	HANDLE						m_hEventUIControl[MAX_EVENT_UI_CONTROL];

	sdvr_err_e					InitBoard(int iBoardIndex);
	sdvr_err_e					UnInitBoard(int iBoardIndex);		
	
	bool						SetPreviewChannel(int iCh , bool Enable);
	bool						SetEncodeChannel(int iCh , bool bRecord);// Start or Stop Encode channel.
	bool						SetInitDvrMenuState(bool bShow, bool bCheck);
	bool						SetEncodeChMenuState(bool bShow, bool bCheck, int iCh, int iBoard);	// iCh=-1 means set all channel.
	bool 						SetEnableChMenuState(bool bShow, bool bCheck, int iCh, int iBoard);
	bool						SetChannel(bool bConnect, int iCh);
	int							m_iFrameCount[MAX_BOARD_CHANNEL];
	bool						acquireBuffer(int iBoard, int iCh, sdvr_frame_type_e frame_type, sx_bool is_primary, sdvr_chan_handle_t handle);
	int							m_iEventOpenBoard;		// for InitBoard().
	int							m_iEventEncodeCh;
	int							m_iEventConnectCh;
	bool						m_bIsInitDvrBoardOK[MAX_SUPPORT_BOARD];
	bool						m_bIsInitAllDvrBoardOK;
	bool						m_bIsEncodeChOK[MAX_SUPPORT_CHANNEL];
	bool						m_bIsConnectChannelOK[MAX_SUPPORT_CHANNEL];
	bool						m_bIsUnInitBoard ;

	bool						bCheckPrimary[MAX_BOARD_CHANNEL];
	bool						bCheckSecond[MAX_BOARD_CHANNEL];
	bool						bCheckPriH264[MAX_BOARD_CHANNEL];
	bool						bCheckSecH264[MAX_BOARD_CHANNEL];
	bool						bCheckPriMJPEG[MAX_BOARD_CHANNEL];
	bool						bCheckSecMJPEG[MAX_BOARD_CHANNEL];

	bool						m_bIsUseControlAll;		// Avoid to show channel setting dialog: means use default channel setting.	
	int							m_iVdoWidth_Raw[MAX_BOARD_CHANNEL];
	int							m_iVdoHeight_Raw[MAX_BOARD_CHANNEL];
	int							m_iVdoWidth_Enc[MAX_BOARD_CHANNEL];
	int							m_iVdoHeight_Enc[MAX_BOARD_CHANNEL];
	CDirectDraw					*m_pDirectDrawDisplay[MAX_BOARD_CHANNEL];
	BYTE						*m_pbyYUVBuffer[MAX_BOARD_CHANNEL];
	bool						m_bProcessIsClosing;
	BOOL						InitDrawDib(int iCh , int iBoard );
	BOOL						CloseDrawDib(int iCh);
	CWaveOut					*m_pWaveOut;
	int							m_iPlayAudioCh;
	int							m_iVdo_YBufSize[MAX_BOARD_CHANNEL];
	int							m_iVdo_UBufSize[MAX_BOARD_CHANNEL];
	int							m_iVdo_VBufSize[MAX_BOARD_CHANNEL];
	void						DrawVideoYUV(int iCh, char * y, char * u, char *v);
	BOOL						m_bInitDrawOK[MAX_BOARD_CHANNEL];
	BOOL						SetEncodePath();
	BOOL						m_bShowBoard[MAX_SUPPORT_BOARD];

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
	HRESULT	OnModifyChannelSettingDlg(WPARAM wParam, LPARAM lParam);
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedRadioBoard1();
	afx_msg void OnBnClickedRadioBoard2();
	afx_msg void OnBnClickedRadioBoard3();
	afx_msg void OnBnClickedRadioBoard4();
};
