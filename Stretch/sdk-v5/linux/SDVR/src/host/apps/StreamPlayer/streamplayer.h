// streamplayer.h

#ifndef __STREAMPLAYER_H__
#define __STREAMPLAYER_H__

#define AUDIO_QUEUE_SIZE    16
#define VIDEO_QUEUE_SIZE    16

typedef enum {
	PLAYER_STATE_STOP = 0,
	PLAYER_STATE_PLAY,
	PLAYER_STATE_PAUSE,
} enumPlayerState;

class CFrameBuffer;
class CRenderer;
class CSystemTime;
class CStretchPlayer;

typedef CStretchQueue<CFrameBuffer> FrameQueue;

class CStreamPlayer
{
public:
	~CStreamPlayer();
	CStreamPlayer();

	sdvr_player_err_e Open(CStretchPlayer *pPlayer, track_type_e type, 
			   enumCodecId eCodecId, CSystemTime *pSysTime);
	sdvr_player_err_e Close();

	track_type_e GetType() {return m_eType;}
	FrameQueue& GetFrameQueue() {return m_frameQueue;}
    void SendFileEndMessage();

    sdvr_player_err_e Play(HWND hWnd);
    sdvr_player_err_e Stop();
    sdvr_player_err_e Pause();

    sdvr_player_err_e FlushQueue();

	// system time
	void CalibrateSystemTime(double dRefTime);
	double GetSystemTime();
	double GetSystemTimeFromTS(double dTime);

	// audio
	sdvr_player_err_e GetVolume(LONG *plVolume);
	sdvr_player_err_e SetVolume(LONG lVolume);

	sx_bool IsMaster() {return m_bMaster;}
	void SetMaster(sx_bool bMaster) {m_bMaster = bMaster;}

	sx_bool IsEnabled() {return m_bEnabled;}
	void Enable(sx_bool bEnabled) {m_bEnabled = bEnabled;}

private:
	CStretchPlayer		*m_pPlayer;		// player pointer
	track_type_e		m_eType;		// track type
	enumPlayerState		m_eState;		// state
	FrameQueue			m_frameQueue;	// frame queue
	HWND				m_hWnd;			// window handle for display
	CRenderer			*m_pRenderer;	// renderer pointer
	CSystemTime			*m_pSysTime;	// system time
	sx_bool				m_bMaster;		// flag of master track
	sx_bool				m_bEnabled;		// if is enabled
};

#endif