#ifndef __SNDPLAYER_H__
#define __SNDPLAYER_H__

#include <windows.h>
#include <dsound.h>
#include "g711.h"
#include "renderer.h"

class CSoundPlayer : public CRenderer
{
public:
	CSoundPlayer();
    ~CSoundPlayer();

	sdvr_player_err_e Open(CStreamPlayer *pPlayer);
	sdvr_player_err_e Close();
    sdvr_player_err_e Play(HWND hWnd);
    sdvr_player_err_e Stop();
    sdvr_player_err_e Pause();
	sx_bool IsMaster();

	// sound buffer control
	sdvr_player_err_e GetVolume(LONG *plVolume);
	sdvr_player_err_e SetVolume(LONG lVolume);
	sdvr_player_err_e GetFrequency(DWORD *pdwFreq);
	sdvr_player_err_e SetFrequency(DWORD dwFrequency);

protected:
	sx_int32			m_iChannels;		// number of channels
	sx_int32			m_iSamplingRate;	// sampling rate
    LPDIRECTSOUND8		m_pDS;				// DirectSound object
    WAVEFORMATEX		m_wfx;				// wave format

	// sound buffer
	LPDIRECTSOUNDBUFFER m_pDSBuffer;		// DirectSound buffer
    DWORD               m_dwDSBufferSize;	// buffer size
    DWORD				m_dwLastPlayPos;	// last play position
	LONG				m_lVolume;			// buffer volume
	LONG				m_lFrequency;		// buffer frequency
	sx_int64			m_iLastTS;			// last timestamp

	// audio decoder
	g711_t				*m_pG711Dec;
	sx_int16			*m_pDecBuf;			// decoding buffer
	sx_int16			*m_pDecPtr;			// pointer of remaining unplayed decoded data
	DWORD				m_dwDecAllocSamples; // allocated samples of decoding buffer
	DWORD				m_dwRemainSamples;	// remaining unplayed decoded audio samples

	friend DWORD WINAPI AudioThread(LPVOID lpParam);
    sdvr_player_err_e Initialize(HWND hWnd, sx_bool bShare = false);
    sdvr_player_err_e RestoreBuffer(LPDIRECTSOUNDBUFFER pDSB, BOOL* pbWasRestored);
    sdvr_player_err_e FillSoundBuffer();
    sdvr_player_err_e UpdateSoundBuffer();
	sdvr_player_err_e FillBuffer(BYTE* pBuffer, DWORD dwSize);

	sdvr_player_err_e UIHandler(MSG &msg);
	sdvr_player_err_e _Play(HWND hWnd);
	sdvr_player_err_e _Stop();
	sdvr_player_err_e _Pause();
};

#endif // __SNDPLAYER_H__
