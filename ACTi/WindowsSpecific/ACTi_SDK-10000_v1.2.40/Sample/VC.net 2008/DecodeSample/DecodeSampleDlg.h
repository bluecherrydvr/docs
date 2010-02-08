// DecodeSampleDlg.h : 標頭檔 
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "xvid.h"
#include "SoundPlayer.h"
#include <Winsock2.h>


typedef struct
{
	unsigned char b2h[4]; /* 00 00 01 B2 */
	unsigned char msg_type;
	unsigned char stream_id; /* video streaming id */
	unsigned char ext_b2_len; /* 1: length of the ext. b2 private data appended
							  to B2 frame */
	unsigned char rsvd;
	unsigned int len;
} B2_HEADER_V2;

typedef struct 
{
	time_t date_time;
	unsigned char time_zone; /* 0:-12, ..., 24:+13 */
	unsigned char video_loss; /* 0: video loss, 1 : video ok */
	unsigned char motion; /* 0x02: Motion 1 is active, 0x04: Motion 2 is
						  active, 0x08 Motion 3 is active */

	unsigned char dio; /* for DIs, 0: DI triggered. 1: no triggered */
	unsigned int count; /* frame counter */
	unsigned char resolution; /* 0:N720x480, ... */
	unsigned char bitrate; /* 0:28K, ... */
	unsigned char fps_mode; /* 0:MODE1(constant), 1:0:MODE2 */
	unsigned char fps_number; /* In constant FPS mode, it indicates the video
							  server's constant FPS number.
							  In variable FPS mode, in indicates the variable
							  FPS number which was requested by the TCP
							  host. If it is not in TCP, it indicates the variable
							  FPS number */
	struct timeval timestamp;
	unsigned short md_actives[3]; /* # of active microblocks in motion region */
	unsigned char reserved[2];
} PRIVATE_DATA_B2;

typedef struct 
{
	B2_HEADER_V2 header;
	PRIVATE_DATA_B2 prdata;
} VIDEO_B2_FRAME;

typedef struct 
{
	B2_HEADER_V2 header;
	struct timeval timestamp;
	unsigned char reserved[8];
} AUDIO_B2_V2;



#define DECODER_IH264			_T("IH264CODEC")

// CODEC ADAPTER
typedef int				(* XCInit			)();
typedef void			(* XCExit			)( int );
typedef LPBITMAPINFO	(* XDecode			)( int, int, int, BYTE*, LONG, HWND, PRECT, bool );
typedef bool			(* XSetChannel		)( int, int );
typedef	int				(* XWriteAudio		)( int, BYTE*, LONG );
typedef int				(* XReadAudio		)( int, BYTE*, LONG& );
typedef int				(* XPlayAudio		)( int, bool );

// CDecodeSampleDlg 對話方塊
class CDecodeSampleDlg : public CDialog
{
// 建構
public:
	CDecodeSampleDlg(CWnd* pParent = NULL);	// 標準建構函式

// 對話方塊資料
	enum { IDD = IDD_DECODESAMPLE_DIALOG };

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
private:
	// Connection type control
	CComboBox m_ccbConnTypeCtrl;
	// User type control
	CComboBox m_ccbUserTypeCtrl;
	// IP address control
	CIPAddressCtrl m_cIPACtrl;
	// User name
	CString m_cStrUserName;
	// Password
	CString m_cStrPwd;
	// Path to save bmp file
	CString m_cStrBMPSavePath;
	// Save BMP Button 
	CButton m_cBtnSave;
	bool m_bSaveBMP;
	BITMAPINFO* m_Bmi;
	// SDK Handle
	HANDLE hSDK;
	// Program running flag
	bool m_bStart;
	// Start button
	CButton m_cBtnStartStop;
	// For raw data call back
	char* m_pInBuf;
	char* m_pOutBuf;
	char* m_pAudioBuf;
	DWORD m_dwBufLen;
	// XVID
	xvid_dec_create_t   m_xvidDecHandle;
	DWORD   m_dwWidth;
	DWORD m_dwHeight;
	// Control view window
	CStatic m_cCtrlVW;
	// DC handle
	HDC		m_hDrawDC;
	// Display debug message
	bool m_bDebugMessageShow;
	// Decode I Frame only
	BOOL m_bIOnly;
	// Sound player
	CSoundPlayer* m_pSound;



public:
	afx_msg void OnBnClickedOk();

	// Check frame type.
	DWORD CheckIorP( char* pRawData, int Datalen );
	// Function to handle video data
	void VideoDataHandler(BYTE* buf, DWORD len);
	// Function to handle audio data
	void AudioDataHandler(BYTE* buf, DWORD len);
	// Function to draw video data
	void VideoDisply();
	bool mf_PrepareDibBuffer(LPBITMAPINFO *lplpbi, DWORD dwX, DWORD dwY);
	// Convert IP from DWROD to String or String to DWORD
	bool TransformIp(char* addr,DWORD &dwIP,bool flag);

	void InitXvid( DWORD dwWidth, DWORD dwHeight );

	bool InitH264();
	void ReleaseH264();
	//Draw H.264 
	void H264VideoDisply(LPBITMAPINFO lpBMP);


	DWORD ReverseIP(DWORD dwIP)
	{
		DWORD dwTarget;
		BYTE *src = (BYTE *)&dwIP;
		BYTE *dst = (BYTE *)&dwTarget;
		for (int ip = 0; ip < sizeof(DWORD); ip ++) {
			dst[ip] = src[sizeof(DWORD)-ip-1];
		}
		return dwTarget;
	}


	// H.264 decode Interface
	HINSTANCE				m_H264_hinstDecoderAdapter;

	XCInit					m_H264_lpfnCInit;
	XCExit					m_H264_lpfnCExit;
	XDecode					m_H264_lpfnCDecode;
	XSetChannel				m_H264_lpfnXSetChannel;
	XWriteAudio				m_H264_lpfnXWriteAudio;
	XReadAudio				m_H264_lpfnXReadAudio;
	XPlayAudio				m_H264_lpfnXPlayAudio;

	//
	int m_H264_nDecoderHandle;


	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedCheck2();
	
};
