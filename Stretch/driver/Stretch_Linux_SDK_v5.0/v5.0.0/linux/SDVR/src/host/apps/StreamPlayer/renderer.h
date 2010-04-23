// renderer.h

#ifndef __RENDERER_H__
#define __RENDERER_H__

typedef enum {
	UNINITIALIZED = 0,
	STOP,
	PLAY,
	PAUSE,
} enumRendererState;

class CStreamPlayer;

class CRenderer
{
public:
	virtual sdvr_player_err_e Open(CStreamPlayer *pPlayer) = 0;
	virtual sdvr_player_err_e Close() = 0;
	virtual sdvr_player_err_e Play(HWND hWnd) = 0;
	virtual sdvr_player_err_e Stop() = 0;
	virtual sdvr_player_err_e Pause() = 0;

	virtual sx_bool IsMaster() = 0;

	enumRendererState	m_eState;
	CStreamPlayer		*m_pPlayer;			// player pointer
    HWND				m_hWnd;		        // window handle
	// for thread
	HANDLE				m_hThread;			// thread handle
	DWORD				m_dwThreadId;		// thread id
	HANDLE				m_UIEventId;		// UI event id
};

#endif
