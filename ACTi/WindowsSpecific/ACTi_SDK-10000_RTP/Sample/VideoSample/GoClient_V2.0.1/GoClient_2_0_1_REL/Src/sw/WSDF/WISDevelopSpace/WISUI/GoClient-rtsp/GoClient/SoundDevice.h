// SoundDevice.h: interface for the CSoundDevice class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOUNDDEVICE_H__15485383_A750_4698_BCBF_78A66E9C38EC__INCLUDED_)
#define AFX_SOUNDDEVICE_H__15485383_A750_4698_BCBF_78A66E9C38EC__INCLUDED_

#include <mmsystem.h>

#include "PtsClock.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define RECORDING_BUFFER_NUM 1000
#define PLAYING_BUFFER_NUM   1000

class CSoundDevice  
{
public:
	CSoundDevice();
	virtual     ~CSoundDevice();

	//int         InitializeDevice(int noChannel,int samplerate);
	int		InitializeDevice(int noChannel,int sampleRate, BOOL bAvSyncEnabled = FALSE);
	void        UninitializeDevice();
    void        Mute(BOOL m_Mute);
	void        DoPlayingThread();
	void		DoMonThread();
    unsigned int dwVolume;
    int         SampleRate;
	int         Write(unsigned char *pBuf, int Len);
	int         Write(unsigned char *pBuf, int Len, UINT64 pts);
	void 	SetInputClock(input_clock_t *pClock);

//public:
protected:

	int		    WritingBufferIndex;
	int		    PlayingThreadState;
	HANDLE      hPlayingThread;

    HANDLE      hPlayingEventDone;
    HWAVEOUT    hWaveOut;
	WAVEHDR     PlayingWaveHdr[PLAYING_BUFFER_NUM];
	int		    PlayingBufferIndex;
	UINT64		AudioPts[PLAYING_BUFFER_NUM];

	UINT64	m_SendCount;
	UINT64	m_DoneCount;
	BOOL	m_bAvSyncEnabled;
	input_clock_t *m_pInputClock;
};

#endif // !defined(AFX_SOUNDDEVICE_H__15485383_A750_4698_BCBF_78A66E9C38EC__INCLUDED_)
