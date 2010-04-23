// streamplayer.cpp

#include <windows.h>
#include <basetsd.h>
#include <assert.h>
#include "stplayer.h"
#include <mmreg.h>
#include <dxerr9.h>
#include <math.h>
#include "syncobj.h"
#include "stsplitter.h"
#include "sndplayer.h"
#include "vidplayer.h"
#include "systime.h"
#include "streamplayer.h"

CStreamPlayer::~CStreamPlayer()
{
	Close();
}

CStreamPlayer::CStreamPlayer()
{
	m_eState = PLAYER_STATE_STOP;
	m_pSysTime = NULL;
	m_pRenderer = NULL;
	m_bMaster = false;
	m_bEnabled = true;
}

sdvr_player_err_e 
CStreamPlayer::Open(CStretchPlayer *pPlayer, 
					track_type_e type, 
					enumCodecId eCodecId,
					CSystemTime *pSysTime)
{
	m_pPlayer = pPlayer;
	m_eType = type;
	m_pSysTime = pSysTime;

	if (type == TRACK_VIDEO) {
	    if (m_frameQueue.Create(VIDEO_QUEUE_SIZE) == 0)
            return SDVR_PLAYER_ALLOC_MEMORY_ERROR;

		// initialize video player
		m_pRenderer = new CVideoPlayer();
		if (m_pRenderer == NULL)
			return SDVR_PLAYER_ALLOC_MEMORY_ERROR;
	}
	else if (type == TRACK_AUDIO) {
	    if (m_frameQueue.Create(AUDIO_QUEUE_SIZE) == 0)
            return SDVR_PLAYER_ALLOC_MEMORY_ERROR;

		// initialize audio player
		m_pRenderer = new CSoundPlayer();
		if (m_pRenderer == NULL)
			return SDVR_PLAYER_ALLOC_MEMORY_ERROR;
	}

	return m_pRenderer->Open(this);
}

sdvr_player_err_e CStreamPlayer::Close()
{
	Stop();
	if (m_pRenderer != NULL) {
		m_pRenderer->Close();
		delete m_pRenderer;
		m_pRenderer = NULL;
	}
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e CStreamPlayer::Play(HWND hWnd)
{
	sdvr_player_err_e err;

	if (!IsEnabled())
		return SDVR_PLAYER_NOERROR;

	if (m_eState == PLAYER_STATE_PLAY)
		return SDVR_PLAYER_NOERROR;

	err = m_pRenderer->Play(hWnd);
	if (err == SDVR_PLAYER_NOERROR)
		m_eState = PLAYER_STATE_PLAY;
	return err;
}

sdvr_player_err_e CStreamPlayer::Stop()
{
	if (!IsEnabled())
		return SDVR_PLAYER_NOERROR;

	if (m_eState == PLAYER_STATE_STOP)
		return SDVR_PLAYER_NOERROR;

	// abort queue
	m_pRenderer->Stop();
	m_frameQueue.Abort();
	m_frameQueue.Flush();
	m_eState = PLAYER_STATE_STOP;
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e CStreamPlayer::Pause()
{
	if (!IsEnabled())
		return SDVR_PLAYER_NOERROR;

	m_pRenderer->Pause();
	m_eState = PLAYER_STATE_PAUSE;
	return SDVR_PLAYER_NOERROR;
}

void CStreamPlayer::SendFileEndMessage()
{
	m_pPlayer->SendFileEndMessage();
}

void CStreamPlayer::CalibrateSystemTime(double dRefTime)
{
	assert(m_pSysTime != NULL);
	m_pSysTime->Calibrate(dRefTime);
}

double CStreamPlayer::GetSystemTime()
{
	assert(m_pSysTime != NULL);
	return m_pSysTime->GetTime();
}

double CStreamPlayer::GetSystemTimeFromTS(double dTime)
{
	assert(m_pSysTime != NULL);
	return m_pSysTime->GetTimeFromTimestamp(dTime);
}

sdvr_player_err_e CStreamPlayer::FlushQueue()
{
	m_frameQueue.Flush();
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e CStreamPlayer::GetVolume(LONG *plVolume)
{
	if (m_eType == TRACK_AUDIO) {
		CSoundPlayer *pRenderer = (CSoundPlayer *)m_pRenderer;
		return pRenderer->GetVolume(plVolume);
	}
	else
		return SDVR_PLAYER_FILE_NO_AUDIO;
}

sdvr_player_err_e CStreamPlayer::SetVolume(LONG lVolume)
{
	if (m_eType == TRACK_AUDIO) {
		CSoundPlayer *pRenderer = (CSoundPlayer *)m_pRenderer;
		return pRenderer->SetVolume(lVolume);
	}
	else
		return SDVR_PLAYER_FILE_NO_AUDIO;
}
