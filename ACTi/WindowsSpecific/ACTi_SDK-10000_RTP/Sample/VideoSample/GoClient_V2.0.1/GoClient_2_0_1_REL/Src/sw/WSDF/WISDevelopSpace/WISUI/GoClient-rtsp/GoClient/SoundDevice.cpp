// SoundDevice.cpp: implementation of the CSoundDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SoundDevice.h"

#include "debugprint.h"
//#include "G723\\g723.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//extern int ProcessRecordedData(unsigned char *lpData, int Len);
#define	AUDIO_BUFFER_MAX_LENGTH		7680       //48000*4/25
#define	AUDIO_PTS_MAX_GAP			500000	//0.5s
#define	AUDIO_PTS_MIN_GAP			50000		//	0.05s
#define	AUDIO_PTS_MAX_DELAY			300000		//	0.3s
#define	AUDIO_DEV_MAX_DELAY			8 //25
#define	AUDIO_WAKEUP_TIME			2000		//	0.002s
#define FIRST_PTS_MAX_GAP			120000000   //2min


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSoundDevice::CSoundDevice()
{
}

CSoundDevice::~CSoundDevice()
{
	
}
DWORD WINAPI PlayingThreadProc( LPVOID lpParam )
{
/* while ((((CSoundDevice *)lpParam)->WritingBufferIndex < 300)&&(((CSoundDevice *)lpParam)->PlayingThreadState==1))
{
Sleep(100);
}*/
	((CSoundDevice *)lpParam)->DoPlayingThread();
	return 0;
} 
// 0 : success, >0 : fail

DWORD WINAPI MonThreadProc( LPVOID lpParam )
{
	((CSoundDevice *)lpParam)->DoMonThread();
	return 0;
} 

int CSoundDevice::InitializeDevice(int noChannel,int sampleRate, BOOL bAvSyncEnabled)
{
	int numDevsIn = waveInGetNumDevs();
	if(numDevsIn == 0)
		return 1;
	
	
	
	int numDevsOut = waveOutGetNumDevs();
	if(numDevsOut == 0)
		return 2;
	
	hPlayingEventDone = CreateEvent(NULL, FALSE, FALSE, NULL);
	ResetEvent(hPlayingEventDone);
	
	WAVEFORMATEX format;
	
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = noChannel;         
    format.wBitsPerSample = 16; 
    SampleRate = sampleRate;
    format.nSamplesPerSec = sampleRate;//hardcoded for now 48k sample rate 
    format.nBlockAlign = format.wBitsPerSample * format.nChannels / 8;
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;//* 2 channel * 2bytes per channel
    format.cbSize = 0;
	
    DWORD hr = waveOutOpen(&hWaveOut, WAVE_MAPPER, &format,
		(DWORD)hPlayingEventDone, 0, CALLBACK_EVENT);
	
	if(hr != MMSYSERR_NOERROR)
	{
		CloseHandle(hPlayingEventDone);
		return 3;
	}
	
	for(int i=0;i<PLAYING_BUFFER_NUM;i++)
	{
		PlayingWaveHdr[i].lpData = new char[AUDIO_BUFFER_MAX_LENGTH];
		PlayingWaveHdr[i].dwBufferLength = AUDIO_BUFFER_MAX_LENGTH;//48000*4/30;
		PlayingWaveHdr[i].dwFlags = 0;
		
		AudioPts[i] = 0;
		
		waveOutPrepareHeader(hWaveOut, &(PlayingWaveHdr[i]), sizeof(WAVEHDR));

		PlayingWaveHdr[i].dwBufferLength = 0;
	}	
    PlayingBufferIndex = 0;
    WritingBufferIndex = 0;
	
	m_SendCount = 0;
	m_DoneCount = 0;
	
	PlayingThreadState=1; 
	
	m_bAvSyncEnabled = bAvSyncEnabled;
	
	DWORD ThreadID;
	hPlayingThread = CreateThread(NULL, 0, PlayingThreadProc, (LPVOID)this, 0, &ThreadID);
    waveOutGetVolume(hWaveOut,(LPDWORD)&dwVolume);
    if (!dwVolume)
    {
        dwVolume = 0x85858585;
        waveOutSetVolume(hWaveOut,dwVolume);
    }

	DWORD ThreadID2;
	CreateThread(NULL, 0, MonThreadProc, (LPVOID)this, 0, &ThreadID2);
	
	return 0;
}

void CSoundDevice::UninitializeDevice()
{
	PlayingThreadState=0; 
	WaitForSingleObject(hPlayingThread,INFINITE);
	waveOutClose(hWaveOut);
	CloseHandle(hPlayingEventDone);
	
	for(int i=0;i<PLAYING_BUFFER_NUM;i++)
	{
		waveOutUnprepareHeader(hWaveOut, &(PlayingWaveHdr[i]), sizeof(WAVEHDR));
		delete PlayingWaveHdr[i].lpData;
	}	
}

void CSoundDevice::DoMonThread()
{

	while((PlayingThreadState==1) && (m_bAvSyncEnabled == TRUE))
	{
		if(WAIT_OBJECT_0 == WaitForSingleObjectEx(hPlayingEventDone, 1000, 0))
			m_DoneCount++;			
	}
}

void CSoundDevice::DoPlayingThread()
{
	PrintString("Player Thread Begin");
    int cnt = 0;
    int threshold_rate = 2;
	UINT64 RefPts;
	UINT64 RefTime;
	INT64 PtsDiff;
	INT64 PlayDiff;
	
    if (SampleRate >= 8000)
        threshold_rate = 50;
	while(PlayingThreadState==1)
	{
		if((PlayingBufferIndex != WritingBufferIndex))//&&(cnt < threshold_rate))
		{
			if(m_bAvSyncEnabled == TRUE)
			{				
				GetRefTime(m_pInputClock, &RefPts, &RefTime);
				
				
				PtsDiff = AudioPts[PlayingBufferIndex] - RefPts;
				PlayDiff = mdate() - RefTime;

				if(PtsDiff > PlayDiff)
				{
					if(((PtsDiff - PlayDiff) > AUDIO_PTS_MIN_GAP) && ((PtsDiff - PlayDiff) < AUDIO_PTS_MAX_GAP))
					{
						//wake up before the AUDIO_WAKEUP_TIME to solve the audio delay problem
						Sleep((UINT32)((PtsDiff - PlayDiff- AUDIO_WAKEUP_TIME) / 1000));
					}
					else if((PtsDiff - PlayDiff) >= AUDIO_PTS_MAX_GAP)
					{
						/*do nothing, waiting next reference pts and time*/						
					}
				}

				if((m_SendCount > m_DoneCount) && (m_SendCount - m_DoneCount > AUDIO_DEV_MAX_DELAY))
				{
					/*The Audio device is so busy, reset it*/
					waveOutReset(hWaveOut);
					//OutputDebugString("Audio: Reset Audio output device\n");
					m_SendCount = m_DoneCount = 0;
				}
			}
			
			waveOutWrite(hWaveOut, &(PlayingWaveHdr[PlayingBufferIndex]), sizeof(WAVEHDR));
			if (((PlayingWaveHdr[PlayingBufferIndex].dwFlags&WHDR_DONE) != WHDR_DONE )&&(PlayingThreadState==1))
				Sleep(0);

			if(m_bAvSyncEnabled == TRUE)
			{				
				AudioPts[PlayingBufferIndex] = 0;
				PlayingWaveHdr[PlayingBufferIndex].dwBufferLength = 0;

				m_SendCount++;
			}

			PlayingBufferIndex ++;
			if(PlayingBufferIndex >= PLAYING_BUFFER_NUM)
				PlayingBufferIndex = 0;
            cnt++;
		}
        else
        {
            cnt = 0;
            Sleep(10);//reshedule
        }
	}
	
	PlayingThreadState = 3;
	PrintString("Work Thread End");
}

int CSoundDevice::Write(unsigned char *pBuf, int Len)
{
    memcpy(PlayingWaveHdr[WritingBufferIndex].lpData,pBuf,Len);
    PlayingWaveHdr[WritingBufferIndex].dwBufferLength = Len;
	WritingBufferIndex ++;
    if (WritingBufferIndex == PLAYING_BUFFER_NUM)
        WritingBufferIndex = 0;
	return 0;
}

int CSoundDevice::Write(unsigned char *pBuf, int Len, UINT64 pts)
{
	if(SampleRate == 8000)
	{
		AudioPts[WritingBufferIndex] = pts;
		return Write(pBuf, Len);
	}
	else
	{
	if((PlayingWaveHdr[WritingBufferIndex].dwBufferLength + Len <= AUDIO_BUFFER_MAX_LENGTH)
		&& ((AudioPts[WritingBufferIndex] == 0) ||((AudioPts[WritingBufferIndex] != 0) &&
			((pts >= AudioPts[WritingBufferIndex]) && (pts - AudioPts[WritingBufferIndex] < AUDIO_PTS_MAX_DELAY)))))
	{
		memcpy(PlayingWaveHdr[WritingBufferIndex].lpData + 
			PlayingWaveHdr[WritingBufferIndex].dwBufferLength
			,pBuf,Len);
		PlayingWaveHdr[WritingBufferIndex].dwBufferLength += Len;
		if((AudioPts[WritingBufferIndex] > pts) || (AudioPts[WritingBufferIndex] == 0))
			AudioPts[WritingBufferIndex] = pts;
	}
	else
	{
		WritingBufferIndex ++;
		if (WritingBufferIndex == PLAYING_BUFFER_NUM)
			WritingBufferIndex = 0;
		memcpy(PlayingWaveHdr[WritingBufferIndex].lpData,pBuf,Len);
		PlayingWaveHdr[WritingBufferIndex].dwBufferLength = Len;
		AudioPts[WritingBufferIndex] = pts;
	}
	return 0;
	}
}

void CSoundDevice::Mute(BOOL m_Mute)
{
    if (m_Mute == TRUE)
    {
        waveOutSetVolume(hWaveOut,0x0);
    }
    else 
    {
        waveOutSetVolume(hWaveOut,dwVolume);
    }
}

void CSoundDevice::SetInputClock(input_clock_t *pClock)
{
	m_pInputClock = pClock;
}



