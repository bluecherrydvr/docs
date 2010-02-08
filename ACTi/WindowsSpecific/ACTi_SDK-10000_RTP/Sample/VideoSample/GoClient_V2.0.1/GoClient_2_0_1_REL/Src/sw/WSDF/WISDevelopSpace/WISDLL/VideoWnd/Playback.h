/**************************************************************************************
 * Media Play back
 * created by weimin, Aug 8, 2002
 **************************************************************************************/

#ifndef _MEDIA_PLAYBACK_H_
#define _MEDIA_PLAYBACK_H_

#include "VideoRender.h"

#define FULLSCREEN_MODE		0x01000000
#define NORMALDISP_MODE		0x02000000
#define PUSH_MODE			0x10000000
#define PULL_MODE			0x20000000

class CPlayback
{
	CVideoRender	*m_videoRender;
	IMP_UniDecTSC	*m_videoDecoder;
	RECT			*m_videoRect;

	HWND			m_hwndPlayback;
	HANDLE			m_videoThread;

	BOOL			m_hasVideo;
	BOOL			m_hasAudio;

	BOOL			m_playing;
	BOOL			m_paused;
	BOOL			m_fastForward;
	BOOL			m_rewind;


public:

	CPlayback();
	~CPlayback();

public:

	BOOL	HasVideo() { return m_hasVideo; };
	BOOL	HasAudio() { return m_hasAudio; };
	
	BOOL	IsFullScreen();
	BOOL	IsOverlay();
	BOOL	IsRGBSurface();
	BOOL	IsRGBDuplicate();

	BOOL	IsPlaying();
	BOOL	IsPaused();

public:

	BOOL	Init(HWND hwndPlayback, int channels, int displayWidth, int displayHeight, int mode);

	BOOL	Play();
	BOOL	Pause();
	BOOL	Rewind();
	BOOL	FastForward();
	BOOL	Seek(int nPercent);
	BOOL	Next();

	BOOL	Stop();
	BOOL	Close();

	BOOL	SetFullScreen(BOOL bFullScreen);
	BOOL	SetLoopPlay(BOOL bLoop);
	BOOL	SetVolumn(int volumn);

public:
	BYTE	*BeginDraw();
	BOOL	EndDraw(BOOL draw);
};

#endif
