// sndplayer.cpp
#define STRICT
#include <assert.h>
#include "common.h"
#include "stplayer.h"
#include "systime.h"
#include "streamplayer.h"
#include "sndplayer.h"


// audio thread
DWORD WINAPI AudioThread( LPVOID lpParam )
{
	CSoundPlayer &player = *((CSoundPlayer *)lpParam);
	HRESULT hr;
	MSG msg;

	sx_bool bAbort = false;
	while (!bAbort) {
		// check messages
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			switch (msg.message) {
			case WM_QUIT:
				bAbort = true;
				break;
			case WM_PLAYER_UI: // UI message
				player.UIHandler(msg);
				break;
			case WM_PLAYER_TEST:
				SetEvent(player.m_UIEventId);
				break;
			default:
				DispatchMessage(&msg);
			}
		}

		if (player.m_eState == PLAY)
			hr = player.UpdateSoundBuffer();
		::Sleep(100);
	}
	return 0;
}

CSoundPlayer::CSoundPlayer()
{
	m_hWnd = NULL;
	m_eState = UNINITIALIZED;
	m_pDS = NULL;
	m_pDSBuffer = NULL;
	m_dwDSBufferSize = 0;
	m_dwLastPlayPos = 0;
	m_pG711Dec = NULL;
	m_iChannels = 2;
	m_iSamplingRate = 8000;
	m_pDecBuf = NULL;
	m_pDecPtr = NULL;
	m_dwDecAllocSamples = 0;
	m_dwRemainSamples = 0;
}

CSoundPlayer::~CSoundPlayer()
{
	Close();
}

sdvr_player_err_e CSoundPlayer::Open(CStreamPlayer *pPlayer)
{
	m_pPlayer = pPlayer;

	// create G.711 decoder
	g711_config_t cfg;
	cfg.law = G711_ULAW;
	cfg.num_channels = 1;
	cfg.channel_select = 0;
	g711_open(&cfg, &m_pG711Dec);
	if (m_pG711Dec == NULL)
		return SDVR_PLAYER_AUDIO_DECODER_INIT_ERROR;

	// allocate buffer for audio data
	m_dwDecAllocSamples = 8192;
	m_pDecBuf = new sx_int16[m_dwDecAllocSamples];
	if (m_pDecBuf == NULL)
		return SDVR_PLAYER_ALLOC_MEMORY_ERROR;
	m_pDecPtr = m_pDecBuf;

	// create UI event
	m_UIEventId = CreateEvent(NULL, FALSE, FALSE, NULL); 

	// create thread
	m_hThread = CreateThread(NULL,		// security descriptor
		0,					// stack size
		AudioThread,		// thread function
		this,				// thread parameter
		0,					// start thread immediately
		&m_dwThreadId);     // thread id
	if (m_hThread == NULL)
		return SDVR_PLAYER_THREAD_ERROR;

	// wait until thread is up
	ResetEvent(m_UIEventId);
	bool bWait = true;
	do {
		::PostThreadMessage(m_dwThreadId, WM_PLAYER_TEST, NULL, NULL);
		DWORD res = WaitForSingleObject(m_UIEventId, 100);
		switch (res) {
		case WAIT_OBJECT_0:
			bWait = false;
			break;
		case WAIT_TIMEOUT:
			continue;
		default:
			return SDVR_PLAYER_THREAD_ERROR;
		}
	} while (bWait);

	m_eState = STOP;
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e CSoundPlayer::Close()
{
	if (m_eState != UNINITIALIZED)
	{
		::PostThreadMessage(m_dwThreadId, WM_QUIT, NULL, NULL);
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;

		SAFE_RELEASE(m_pDSBuffer); 
		SAFE_RELEASE(m_pDS); 

		// close G.711 decoder
		if (m_pG711Dec != NULL) {
			g711_close(m_pG711Dec);
			m_pG711Dec = NULL;
		}

		// delete audio buffer
		if (m_pDecBuf != NULL) {
			delete m_pDecBuf;
			m_pDecBuf = NULL;
			m_dwDecAllocSamples = 0;
			m_dwRemainSamples = 0;
		}
	}
	m_eState = UNINITIALIZED;
	return SDVR_PLAYER_NOERROR;
}

// initialization
sdvr_player_err_e CSoundPlayer::Initialize(HWND hWnd, sx_bool bShared)
{
	HRESULT hr;
	SAFE_RELEASE(m_pDSBuffer); 
	SAFE_RELEASE(m_pDS);

	// Create IDirectSound using the primary sound device
	if (FAILED(hr = DirectSoundCreate8(NULL, &m_pDS, NULL)))
		return SDVR_PLAYER_CREATE_DSOUND_ERROR;

	// Set DirectSound coop level 
	if (FAILED(hr = m_pDS->SetCooperativeLevel(hWnd, DSSCL_NORMAL)))
		return SDVR_PLAYER_CREATE_DSOUND_ERROR;   

	// wave format for secondary sound buffer
	memset(&m_wfx, 0, sizeof(WAVEFORMATEX));
	m_wfx.wFormatTag = WAVE_FORMAT_PCM;
	m_wfx.nSamplesPerSec = m_iSamplingRate;
	m_wfx.nChannels = m_iChannels;
	m_wfx.wBitsPerSample = 16;
	m_wfx.nBlockAlign = (WORD)(m_wfx.wBitsPerSample / 8 * m_wfx.nChannels);
	m_wfx.nAvgBytesPerSec = m_wfx.nSamplesPerSec * m_wfx.nBlockAlign;
	m_wfx.cbSize = 0;

	// Figure out how big the DSound buffer should be 
	m_dwDSBufferSize = m_iSamplingRate * m_iChannels; 
	m_dwLastPlayPos = 0;

	// secondary sound buffer descriptor
	DSBUFFERDESC dsbd;
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2;
	dsbd.dwBufferBytes = m_dwDSBufferSize;
	dsbd.guid3DAlgorithm = GUID_NULL;
	dsbd.lpwfxFormat = &m_wfx;

	// create secondary sound buffer
	if (FAILED(hr = m_pDS->CreateSoundBuffer(&dsbd, &m_pDSBuffer, NULL)))
		return SDVR_PLAYER_CREATE_DSOUND_ERROR;

	return SDVR_PLAYER_NOERROR;
}

// UI message handler
sdvr_player_err_e 
CSoundPlayer::UIHandler(MSG &msg)
{
	switch (msg.wParam) {
	case PLAYER_CMD_PLAY:
		_Play((HWND)msg.lParam);
		break;
	case PLAYER_CMD_STOP:
		_Stop();
		break;
	case PLAYER_CMD_PAUSE:
		_Pause();
		break;
	}
	// response by setting event
	SetEvent(m_UIEventId);
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CSoundPlayer::Play(HWND hWnd)
{
	sdvr_player_err_e err = SDVR_PLAYER_NOERROR;

	// initialize
	if (m_hWnd != hWnd) {
		assert(m_pDS == NULL);
		err = Initialize(hWnd, false);
		if (err != SDVR_PLAYER_NOERROR)
			return err;
		m_hWnd = hWnd;
	}

	ResetEvent(m_UIEventId);
	::PostThreadMessage(m_dwThreadId, WM_PLAYER_UI, PLAYER_CMD_PLAY, (LPARAM)hWnd);
	DWORD ret = WaitForSingleObject(m_UIEventId, INFINITE);
	return err;
}

sdvr_player_err_e 
CSoundPlayer::Stop()
{
	ResetEvent(m_UIEventId);
	::PostThreadMessage(m_dwThreadId, WM_PLAYER_UI, PLAYER_CMD_STOP, NULL);
	WaitForSingleObject(m_UIEventId, INFINITE);
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CSoundPlayer::Pause()
{
	ResetEvent(m_UIEventId);
	::PostThreadMessage(m_dwThreadId, WM_PLAYER_UI, PLAYER_CMD_PAUSE, NULL);
	WaitForSingleObject(m_UIEventId, INFINITE);
	return SDVR_PLAYER_NOERROR;
}

// start play
sdvr_player_err_e CSoundPlayer::_Play(HWND hWnd)
{
	HRESULT hr;
	sdvr_player_err_e err;

	switch (m_eState) {
	case STOP:
		if (FAILED(hr = m_pDSBuffer->Play(0, 0, DSBPLAY_LOOPING)))
			return SDVR_PLAYER_PLAY_DSOUND_ERROR;
		m_eState = PLAY;
		break;
	case PLAY:
		break;
	case PAUSE:
		// Restore the buffer if it was lost
		err = RestoreBuffer(m_pDSBuffer, NULL);
		if (err != SDVR_PLAYER_NOERROR)
			return err;
		// play buffer
		if (FAILED(hr = m_pDSBuffer->Play(0, 0, DSBPLAY_LOOPING)))
			return SDVR_PLAYER_PLAY_DSOUND_ERROR;
		m_eState = PLAY;
		break;
	default:
		return SDVR_PLAYER_UNEXPECTED;
	}

	return SDVR_PLAYER_NOERROR;
}

// stop play
sdvr_player_err_e CSoundPlayer::_Stop()
{
	HRESULT hr;
	switch (m_eState) {
	case PLAY:
	case PAUSE:
		hr = m_pDSBuffer->Stop();
		m_eState = STOP;
		break;
	default:
		return SDVR_PLAYER_UNEXPECTED;
	}
	return SDVR_PLAYER_NOERROR;
}

// pause play
sdvr_player_err_e CSoundPlayer::_Pause()
{
	HRESULT hr;
	sdvr_player_err_e err = SDVR_PLAYER_NOERROR;

	switch (m_eState) {
	case PLAY:
		hr = m_pDSBuffer->Stop();
		m_eState = PAUSE;
		break;
	case PAUSE:
	case STOP:
		break;
	default:
		return SDVR_PLAYER_UNEXPECTED;
	}
	return err;
}

// restore sound buffer
sdvr_player_err_e CSoundPlayer::RestoreBuffer(LPDIRECTSOUNDBUFFER pDSB, BOOL* pbWasRestored)
{
	HRESULT hr;

	if (pDSB == NULL)
		return SDVR_PLAYER_DSOUND_NOT_INITIALIZED;

	if (pbWasRestored)
		*pbWasRestored = FALSE;

	DWORD dwStatus;
	if (FAILED(hr = pDSB->GetStatus( &dwStatus )))
		return SDVR_PLAYER_RESTORE_DSOUND_BUFFER_ERROR;

	if (dwStatus & DSBSTATUS_BUFFERLOST) {
		do {
			Sleep(10);
		}
		while ((hr = pDSB->Restore()) == DSERR_BUFFERLOST);

		if (pbWasRestored)
			*pbWasRestored = TRUE;
	}

	return SDVR_PLAYER_NOERROR;
}

// fill sound buffer
sdvr_player_err_e CSoundPlayer::FillSoundBuffer()
{
	HRESULT hr;
	VOID* pDSLockedBuffer = NULL; // Pointer to locked buffer memory
	DWORD dwDSLockedBufferSize = 0; // Size of the locked DirectSound buffer
	DWORD dwDataRead = 0; // Amount of data read 
	sdvr_player_err_e err;

	if (m_pDSBuffer == NULL)
		return SDVR_PLAYER_DSOUND_NOT_INITIALIZED;

	// Make sure we have focus, and we didn't just switch in from
	// an app which had a DirectSound device
	err = RestoreBuffer( m_pDSBuffer, NULL );
	if (err != SDVR_PLAYER_NOERROR)
		return err;

	// Lock the buffer down
	if (FAILED(hr = m_pDSBuffer->Lock(0, m_dwDSBufferSize, &pDSLockedBuffer, 
		&dwDSLockedBufferSize, NULL, NULL, 0L)))
		return SDVR_PLAYER_LOCK_DSOUND_BUFFER_ERROR;
	// Fill the buffer
	FillBuffer((BYTE*)pDSLockedBuffer, dwDSLockedBufferSize);
	// Unlock the buffer
	m_pDSBuffer->Unlock(pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0);

	return SDVR_PLAYER_NOERROR;
}

// update sound buffer
sdvr_player_err_e CSoundPlayer::UpdateSoundBuffer()
{
	HRESULT hr;
	DWORD dwCurrentPlayPos;
	DWORD dwPlayDelta;
	VOID* pDSLockedBuffer = NULL;
	VOID* pDSLockedBuffer2 = NULL;
	DWORD dwDSLockedBufferSize;
	DWORD dwDSLockedBufferSize2;
	sdvr_player_err_e err;

	if (m_pDSBuffer == NULL)
		return SDVR_PLAYER_DSOUND_NOT_INITIALIZED;

	// Restore the buffer if it was lost
	BOOL bRestored;
	err = RestoreBuffer(m_pDSBuffer, &bRestored);
	if (err != SDVR_PLAYER_NOERROR)
		return err;

	if (bRestored) {
		// The buffer was restored, so we need to fill it with new data
		err = FillSoundBuffer();
		return err;
	}

	// get current buffer position
	if (FAILED(hr = m_pDSBuffer->GetCurrentPosition(&dwCurrentPlayPos, NULL)))
		return SDVR_PLAYER_GET_DSOUND_POSITION_ERROR;

	// Check to see if the position counter looped
	if (dwCurrentPlayPos < m_dwLastPlayPos)
		dwPlayDelta = (m_dwDSBufferSize - m_dwLastPlayPos) + dwCurrentPlayPos;
	else
		dwPlayDelta = dwCurrentPlayPos - m_dwLastPlayPos;

	if (dwPlayDelta <= 0)
		return SDVR_PLAYER_NOERROR;

	// calibrate system time if is master
	if (IsMaster()) {
		DWORD samples = (m_dwDSBufferSize - dwPlayDelta) / 2 + m_dwRemainSamples; // remaining unplayed samples
		samples -= 4000; // TODO: don't know why we need this constant to make lip sync
		double dTime = (double)samples / ((double)m_iSamplingRate * (double)m_iChannels);
		m_pPlayer->CalibrateSystemTime((double)m_iLastTS / 100000.0 - dTime);
	}

	// Lock the DirectSound buffer
	if (FAILED(hr = m_pDSBuffer->Lock(m_dwLastPlayPos, dwPlayDelta, 
		&pDSLockedBuffer, &dwDSLockedBufferSize, 
		&pDSLockedBuffer2, &dwDSLockedBufferSize2, 0L)))
		return SDVR_PLAYER_LOCK_DSOUND_BUFFER_ERROR;

	// fill sound buffer
	FillBuffer((BYTE*)pDSLockedBuffer, dwDSLockedBufferSize);
	if (pDSLockedBuffer2 != NULL && dwDSLockedBufferSize2 > 0)
		FillBuffer((BYTE*)pDSLockedBuffer2, dwDSLockedBufferSize2);

	// Unlock the DirectSound buffer
	m_pDSBuffer->Unlock(pDSLockedBuffer, dwDSLockedBufferSize, 
		pDSLockedBuffer2, dwDSLockedBufferSize2);

	m_dwLastPlayPos += dwDSLockedBufferSize + dwDSLockedBufferSize2;
	m_dwLastPlayPos %= m_dwDSBufferSize; // Circular buffer

	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e CSoundPlayer::FillBuffer(BYTE* pBuffer, DWORD dwSize)
{
	short *pOut = (short *)pBuffer;
	sdvr_player_err_e ret = SDVR_PLAYER_NOERROR;
	DWORD dwCopyLen;

	dwSize /= 2; // convert from byte to short

	// check if there are remaining decoded audio data
	if (m_dwRemainSamples > 0) {
		if (m_dwRemainSamples < dwSize) {
			dwCopyLen = m_dwRemainSamples;
			memcpy(pOut, m_pDecPtr, dwCopyLen * sizeof(short));
			pOut += dwCopyLen;
			dwSize -= dwCopyLen;
			m_dwRemainSamples = 0;
			m_pDecPtr = m_pDecBuf;
		}
		else {
			memcpy(pOut, m_pDecPtr, dwSize * sizeof(short));
			m_dwRemainSamples -= dwSize;
			m_pDecPtr += dwSize;
			return ret;
		}
	}

	while (dwSize > 0) {
		// get buffer from audio queue
		CFrameBuffer *frame = m_pPlayer->GetFrameQueue().GetTail();
		if (frame == NULL)
			continue;

		if (frame->type == NULL_FRAME) {
			m_pPlayer->SendFileEndMessage();
			break;
		}

		m_iLastTS = frame->timestamp;

		// check if allocated enough space
		DWORD dwDecLen = frame->size;
		if (dwDecLen > m_dwDecAllocSamples) {
			assert(m_pDecBuf != NULL);
			delete m_pDecBuf;
			m_pDecBuf = new short[dwDecLen];
			assert(m_pDecBuf != NULL);
		}

		// decode audio
		g711_decode(m_pG711Dec, dwDecLen, frame->payload, m_pDecBuf);
		m_pDecPtr = m_pDecBuf;
		m_dwRemainSamples = dwDecLen;

		// copy data to buffer
		dwCopyLen = m_dwRemainSamples < dwSize ? m_dwRemainSamples : dwSize;
		memcpy(pOut, m_pDecPtr, dwCopyLen * sizeof(short));
		pOut += dwCopyLen;
		m_pDecPtr += dwCopyLen;
		dwSize -= dwCopyLen;
		m_dwRemainSamples -= dwCopyLen;

		// commit buffer
		m_pPlayer->GetFrameQueue().CommitTail();
	}

	return ret;
}

sdvr_player_err_e 
CSoundPlayer::GetVolume(LONG *plVolume) 
{
	if (m_pDSBuffer == NULL)
		return SDVR_PLAYER_DSOUND_NOT_INITIALIZED;
	m_pDSBuffer->GetVolume(plVolume);
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CSoundPlayer::SetVolume(LONG lVolume)
{
	if (m_pDSBuffer == NULL)
		return SDVR_PLAYER_DSOUND_NOT_INITIALIZED;
	HRESULT res = m_pDSBuffer->SetVolume(lVolume);
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CSoundPlayer::GetFrequency(DWORD *pdwFreq) 
{
	if (m_pDSBuffer == NULL)
		return SDVR_PLAYER_DSOUND_NOT_INITIALIZED;
	m_pDSBuffer->GetFrequency(pdwFreq);
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CSoundPlayer::SetFrequency(DWORD dwFrequency)
{
	if (m_pDSBuffer == NULL)
		return SDVR_PLAYER_DSOUND_NOT_INITIALIZED;
	m_pDSBuffer->SetFrequency(dwFrequency);
	return SDVR_PLAYER_NOERROR;
}

sx_bool CSoundPlayer::IsMaster() 
{
	return m_pPlayer->IsMaster();
}
