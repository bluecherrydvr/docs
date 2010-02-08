/*********************************************************************

File:         SoundPlayer.cpp

Description:  SDK 10000 - Sound player

Modified:     Ken Chan

Author:       Ken Chan

Prototyping:

Created:      [Ken Chan]

Modified:

Email:		  Ken.Chan@ACTi.com; jjmin32@yahoo.com.tw
MSN:          kennet2@giga.net.tw

*********************************************************************/

#include "stdafx.h"
#include ".\SoundPlayer.h"

CSoundPlayer::CSoundPlayer()
{	
	hPlayingThread = NULL;
	hPlayingEventDone = NULL;
	hWaveOut = NULL;
	m_nUsing = 0;
	m_bMute = false; 
	m_bPlay = false;
	m_bPlaying = false;
	InitializeDevice( 1, 8000 ); 
}

CSoundPlayer::~CSoundPlayer()
{ 
	if( hPlayingEventDone )
		UninitializeDevice(); 
}

int CSoundPlayer::InitializeDevice( int noChannel, int sampleRate, BOOL bAvSyncEnabled )
{
	int numDevsIn = waveInGetNumDevs();
	if(numDevsIn == 0)
		return 1;

	int numDevsOut = waveOutGetNumDevs();
	if(numDevsOut == 0)
		return 2;

	//hPlayingEventDone = CreateEvent(NULL, FALSE, FALSE, NULL);
	hPlayingEventDone = CreateEvent(NULL, TRUE, FALSE, NULL);

	WAVEFORMATEX format;

	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = noChannel;         
	format.wBitsPerSample = 16; 
	SampleRate = sampleRate;
	format.nSamplesPerSec = sampleRate;//hardcoded for now 48k sample rate 
	format.nBlockAlign = format.wBitsPerSample * format.nChannels / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;//* 2 channel * 2bytes per channel
	format.cbSize = 0;

	DWORD hr = waveOutOpen( &hWaveOut, WAVE_MAPPER, &format, (DWORD_PTR) hPlayingEventDone, 0, CALLBACK_EVENT );

	if( hr != MMSYSERR_NOERROR )
	{
		CloseHandle(hPlayingEventDone);
		hPlayingEventDone = NULL;
		return 3;
	}

	for( int i = 0; i < PLAYING_BUFFER_NUM; i++ )
	{
		PlayingWaveHdr[i].lpData = new char[AUDIO_BUFFER_MAX_LENGTH];
		PlayingWaveHdr[i].dwBufferLength = AUDIO_BUFFER_MAX_LENGTH;//48000*4/30;
		PlayingWaveHdr[i].dwFlags = 0;
		waveOutPrepareHeader(hWaveOut, &(PlayingWaveHdr[i]), sizeof(WAVEHDR));
		PlayingWaveHdr[i].dwBufferLength = 0;
	}	

	m_SendCount = 0;
	m_DoneCount = 0;

	m_bAvSyncEnabled = bAvSyncEnabled;

	//waveOutSetPlaybackRate(hWaveOut,0x00014000);
	waveOutGetVolume(hWaveOut,(LPDWORD)&dwVolume);
	if (!dwVolume)
	{
		dwVolume = 0x85858585;
		waveOutSetVolume(hWaveOut,dwVolume);
	}
	m_bPlay = true;

	return 0;

}

void CSoundPlayer::UninitializeDevice()
{
	m_bPlay = false;
	while( m_bPlaying ) 
		Sleep( 30 );
	if( hWaveOut )
		waveOutReset( hWaveOut );

	if( hPlayingEventDone )
	{
		SetEvent( hPlayingEventDone );
		CloseHandle( hPlayingEventDone );
		hPlayingEventDone = NULL;
	}

	for( int i=0; i < PLAYING_BUFFER_NUM; i++ )
	{
		waveOutUnprepareHeader( hWaveOut, &(PlayingWaveHdr[i]), sizeof(WAVEHDR) );
		delete [] PlayingWaveHdr[i].lpData;
		PlayingWaveHdr[i].lpData = NULL;
	}
	if( hWaveOut )
		waveOutClose( hWaveOut );

	if( hPlayingThread )
		CloseHandle( hPlayingThread );
}


int CSoundPlayer::Write(unsigned char *pBuf, int Len)
{
	if( m_bMute ) 
		return 0;

	if( m_bPlay )
	{
		m_bPlaying = true;
		if( m_nUsing < PLAYING_BUFFER_NUM )
		{
			if( m_nUsing == 0 )
				Sync();
			memcpy(PlayingWaveHdr[m_nUsing].lpData,pBuf,Len);
			PlayingWaveHdr[m_nUsing].dwBufferLength = Len;
			waveOutWrite(hWaveOut, &(PlayingWaveHdr[m_nUsing]), sizeof(WAVEHDR));
			m_nUsing++;
			if( m_nUsing >= PLAYING_BUFFER_NUM )
				m_nUsing = 0;

			//if( hPlayingEventDone )
			//{
			//	if( WaitForSingleObject( hPlayingEventDone, 30 ) != WAIT_TIMEOUT )
			//	{
			//		ResetEvent( hPlayingEventDone );
			//	}
			//}
		}
		m_bPlaying = false;
	}
	return 0;
}


void CSoundPlayer::Mute( bool bMute )
{
	m_bMute = bMute;
}


void CSoundPlayer::Sync()
{
	if( hWaveOut )
		waveOutReset( hWaveOut ); 
}

bool CSoundPlayer::SetVolume( int nLeft , int nRight )
{
	if( hWaveOut )
	{
		MMRESULT nRt;
		nLeft = min( 100 , max( nLeft, 0) );
		nRight = min( 100 , max( nRight, 0) );

		DWORD dwVIn = ((0xffff * nRight / 100) << 16) + (0xffff * nLeft / 100);
		nRt = waveOutSetVolume(hWaveOut,dwVIn);
		if( nRt == MMSYSERR_NOERROR )
		{
			dwVolume = dwVIn;
			return true;
		}
	}
	return false;
}

bool CSoundPlayer::GetVolume( int &nLeft , int &nRight )
{
	if( hWaveOut )
	{
		waveOutGetVolume( hWaveOut, (DWORD*)&dwVolume );
		nRight = ((dwVolume & 0xffff0000) >> 16) * 100 / 0xffff;
		nLeft = (dwVolume & 0x0000ffff) * 100 / 0xffff;
		return true;
	}
	return false;
}
