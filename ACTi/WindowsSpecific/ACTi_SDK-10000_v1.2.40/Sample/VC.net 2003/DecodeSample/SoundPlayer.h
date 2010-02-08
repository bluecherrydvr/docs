#pragma once
#include <mmsystem.h>

#define	AUDIO_BUFFER_MAX_LENGTH		(8000 * 4) / 25
#define PLAYING_BUFFER_NUM   180

class CSoundPlayer  
{
public:
	CSoundPlayer();
	~CSoundPlayer();
	int				InitializeDevice( int noChannel, int sampleRate, bool bAvSyncEnabled = false );
	void			UninitializeDevice();
	void			Mute( bool bMute );
	void			DoPlayingThread();
	unsigned int	dwVolume;
	int				SampleRate;
	int				Write(unsigned char *pBuf, int Len);
	void 			Sync();
	bool 			SetVolume( int nLeft , int nRight );
	bool 			GetVolume( int &nLeft , int &nRight );
	//public:
protected:

	HANDLE      hPlayingThread;

	HANDLE      hPlayingEventDone;
	bool		m_bPlay;
	bool		m_bPlaying;
	HWAVEOUT    hWaveOut;
	WAVEHDR     PlayingWaveHdr[PLAYING_BUFFER_NUM];
	int		    PlayingBufferIndex;

	UINT64		m_SendCount;
	UINT64		m_DoneCount;
	bool		m_bAvSyncEnabled;
	bool		m_bMute;
	int			m_nUsing;
};
