// stplayer.h
#ifndef __STPLAYER_H__
#define __STPLAYER_H__

#include "sdvr_player.h"
#include "stsplitter.h"
#include "vdec264.h"


// user message
#define WM_PLAYER_UI		(WM_USER + 101)
#define WM_PLAYER_TEST		(WM_USER + 102)
#define WM_PLAYER_STAGE1	(WM_USER + 103)
#define WM_PLAYER_STAGE2	(WM_USER + 104)

// user command
#define PLAYER_CMD_PLAY		0
#define PLAYER_CMD_STOP		1
#define PLAYER_CMD_PAUSE	2
#define PLAYER_CMD_SET_POS	3

class CFrameBuffer;
class CDemuxRawFile;
class CSystemTime;
class CStreamPlayer;

// player class
class CStretchPlayer
{
public:
    ~CStretchPlayer();
    CStretchPlayer();

	// open and close
    sdvr_player_err_e OpenFile(LPSTR pstrFileName);
	sdvr_player_err_e OpenSdpFile(LPSTR pstrFileName);
    sdvr_player_err_e Close();

	// EOF message
    sdvr_player_err_e RegisterFileEndMessage(HWND hWnd, sx_uint32 uiMsg);
    void SendFileEndMessage();

    // get player info
	sdvr_player_err_e GetState(sdvr_player_state_e *pState);
	sdvr_player_err_e GetPlayRate(float *pfPlayRate);
    sdvr_player_err_e GetTotalFrames(sx_int32 *pFrames);
	sdvr_player_err_e GetAverageTimePerFrame(double* pAvgTimePerFrame);
	sdvr_player_err_e GetFileTime(sx_int32* pTime);
	sdvr_player_err_e GetPlayPos(float* pRelativePos);

	// UI controls
    sdvr_player_err_e Play(HWND hWnd);
    sdvr_player_err_e Stop();
    sdvr_player_err_e Pause();
    sdvr_player_err_e SetPlayPos(float fRelativePos);
	sdvr_player_err_e DoublePlaySpeed();
	sdvr_player_err_e HalvePlaySpeed();

	// play/stop sound
    sdvr_player_err_e PlaySound(sx_bool bEnableShare = true);
    sdvr_player_err_e StopSound();

	// audio volume/frequency
    sdvr_player_err_e GetVolume(sx_uint16 *pVolume);
    sdvr_player_err_e SetVolume(sx_uint16 volume);
    sdvr_player_err_e GetFrequency(DWORD *pdwFreq);
    sdvr_player_err_e SetFrequency(DWORD dwFreq);

    // refresh display
    sdvr_player_err_e Refresh();

	// streaming
	sdvr_player_err_e OpenStream(sx_int32 iBufSize);
	sdvr_player_err_e InputData(sx_uint8 *pBuf, sx_int32 iSize);
	sdvr_player_err_e GetBufferAvailableSize(sx_uint32 *pSize);

	// player thread
	friend DWORD WINAPI PlayerThread( LPVOID lpParam );

protected:
	sdvr_player_err_e _Play(HWND hWnd);
	sdvr_player_err_e _Stop();
	sdvr_player_err_e _Pause();
    sdvr_player_err_e _SetPlayPos(float *pfRelativePos);

	sdvr_player_err_e Initialize();
	sdvr_player_err_e SplitterHandler();
	sdvr_player_err_e UIHandler(MSG &msg);

	sdvr_player_err_e PlayVideo();

public:
	void				*m_hRtpRcv;
	int					m_uiSpsReceived;
	bool				m_bRtp;
	bool				m_bRtpStarted;
	void				*m_pRcvCfg;
	unsigned int		decode_size;
	double				decode_time;
	CFrameBuffer		*m_frame;
	char				*p_dst;
	int					rcv_cnt;
	int					prev_nal_type;

private:
	sdvr_player_state_e	m_eState;			// state
	HWND				m_hWnd;				// window handle for display
	CDemuxRawFile		*m_pSplitter;		// splitter
	CStreamPlayer		*m_pStrmPlayer;		// stream players
	CSystemTime			*m_pSysTime;		// system time
	HWND				m_hMsgWnd;			// EOF message window handle
	sx_uint32			m_uiMsg;			// EOF message
	sx_int32			m_iNumStreams;		// number of streams
	HANDLE				m_hThread;			// thread handle
	DWORD				m_dwThreadId;		// thread id
	HANDLE				m_UIEventId;		// UI event id
	float				m_fPlaySpeed;		// play speed

};

#endif
