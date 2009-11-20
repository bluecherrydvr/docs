//-----------------------------------------------------------------------------
// File: DSUtil.h
//
// Desc: 
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef DSUTIL_H
#define DSUTIL_H

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>


//-----------------------------------------------------------------------------
// Classes used by this header
//-----------------------------------------------------------------------------
class CSoundManager;
class CSound;
class CStreamingSound;


#define NUM_PLAY_NOTIFICATIONS  16

//-----------------------------------------------------------------------------
// Typing macros 
//-----------------------------------------------------------------------------
#define WAVEFILE_READ   1
#define WAVEFILE_WRITE  2

//-----------------------------------------------------------------------------
// Name: class CSoundManager
// Desc: 
//-----------------------------------------------------------------------------
class CSoundManager
{
protected:
    LPDIRECTSOUND8 m_pDS;
    WAVEFORMATEX   *m_pwfx;        // WAVEFORMATEX structure

	// for thread
	HANDLE			m_hThread;		// thread handle
	DWORD			m_dwThreadId;	// thread id

public:
	HANDLE			m_hSoundEvent;
	CStreamingSound *m_pStrmSound;

    CSoundManager();
    ~CSoundManager();

    HRESULT Initialize();
    inline  LPDIRECTSOUND8 GetDirectSound() { return m_pDS; }
    HRESULT SetPrimaryBufferFormat( DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate );

    HRESULT CreateStreamingFromMemory( CStreamingSound** ppStreamingSound, 
										BYTE* pbData, 
										ULONG ulDataSize, 
										LPWAVEFORMATEX pwfx, 
									   DWORD dwCreationFlags, 
									   GUID guid3DAlgorithm, 
									   DWORD dwNotifyCount, 
									   DWORD dwNotifySize, 
									   HANDLE hNotifyEvent );
};




//-----------------------------------------------------------------------------
// Name: class CSound
// Desc: Encapsulates functionality of a DirectSound buffer.
//-----------------------------------------------------------------------------
class CSound
{
};


//-----------------------------------------------------------------------------
// Name: class CStreamingSound
// Desc: Encapsulates functionality to play a wave file with DirectSound.  
//       The Create() method loads a chunk of wave file into the buffer, 
//       and as sound plays more is written to the buffer by calling 
//       HandleWaveStreamNotification() whenever hNotifyEvent is signaled.
//-----------------------------------------------------------------------------
class CStreamingSound
{
protected:
    LPDIRECTSOUNDBUFFER* m_apDSBuffer;
    DWORD                m_dwDSBufferSize;
    DWORD                m_dwNumBuffers;
    DWORD                m_dwCreationFlags;
    WAVEFORMATEX* m_pwfx;        // Pointer to WAVEFORMATEX structure
    DWORD m_dwLastPlayPos;
    DWORD m_dwPlayProgress;
    DWORD m_dwNotifySize;
    DWORD m_dwNextWriteOffset;

    HRESULT RestoreBuffer( LPDIRECTSOUNDBUFFER pDSB, BOOL* pbWasRestored );
    HRESULT FillBufferWithSound( LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger );

public:
    CStreamingSound( LPDIRECTSOUNDBUFFER pDSBuffer, DWORD dwDSBufferSize, DWORD dwNotifySize );
    ~CStreamingSound();

    HRESULT HandleWaveStreamNotification( unsigned char *pBuf, int size );
};


#endif // DSUTIL_H
