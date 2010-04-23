// stsplitter.cpp

#include "stsplitter.h"

// CDemux implementation
CDemux::~CDemux()
{
}

CDemux::CDemux()
{
}

// CDemuxFile implementation
CDemuxFile::~CDemuxFile()
{
}

CDemuxFile::CDemuxFile() :
	m_pInStrm(NULL),
    m_fPlaySpeed(1.0),
    m_uiFileTime(0)
{
	int i;
	for (i=0; i<MAX_NUM_TRACKS; i++) {
		m_pTracks[i] = NULL;
	}
}

// open/close file
sdvr_player_err_e CDemuxFile::Open(LPSTR pstrFileName)
{
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e CDemuxFile::Close(void)
{
	return SDVR_PLAYER_NOERROR;
}

sx_int32 CDemuxFile::GetNumTracks()
{
	return m_iNumTracks;
}

CTrack* CDemuxFile::GetTrack(sx_int32 index)
{
	return m_pTracks[index];
}

// get/set play speed
sdvr_player_err_e CDemuxFile::SetPlaySpeed(float fSpeed)
{
	m_fPlaySpeed = fSpeed;
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e CDemuxFile::GetPlaySpeed(float* pSpeed)
{
	*pSpeed = m_fPlaySpeed;
	return SDVR_PLAYER_NOERROR;
}

// set play position
sdvr_player_err_e CDemuxFile::SetPlayPos(float fRelativePos)
{
	return SDVR_PLAYER_NOERROR;
}

// get play pos in percentage
sdvr_player_err_e CDemuxFile::GetPlayPos(float* pfRelativePos)
{
	if (m_pVideoTrack->IsEnabled())
		*pfRelativePos = m_pVideoTrack->GetPlayPos();
	else if (m_pAudioTrack->IsEnabled())
		*pfRelativePos = m_pAudioTrack->GetPlayPos();
	else
		*pfRelativePos = 0.0;
    return SDVR_PLAYER_NOERROR;
}

// get file time
sdvr_player_err_e CDemuxFile::GetFileTime(sx_int32* pTime)
{
	*pTime = m_uiFileTime;
	return SDVR_PLAYER_NOERROR;
}

// get total frames (video)
sdvr_player_err_e CDemuxFile::GetTotalFrames(sx_int32 *pFrames)
{
	if (m_pVideoTrack->IsEnabled())
	    *pFrames = m_pVideoTrack->GetNumFrames();
	else if (m_pAudioTrack->IsEnabled())
	    *pFrames = m_pAudioTrack->GetNumFrames();
	else
		*pFrames = 0;
    return SDVR_PLAYER_NOERROR;
}

// get average time per frame (video)
sdvr_player_err_e CDemuxFile::GetAverageTimePerFrame(double* pAvgTimePerFrame)
{
	sx_int32 frames;
	if (m_pVideoTrack->IsEnabled()) {
		frames = m_pVideoTrack->GetNumFrames();
		if (frames > 0)
			*pAvgTimePerFrame = (double)m_pVideoTrack->GetDuration() / frames;
		else
			*pAvgTimePerFrame = 0.0;
	}
	else if (m_pAudioTrack->IsEnabled())	{
		frames = m_pAudioTrack->GetNumFrames();
		if (frames > 0)
			*pAvgTimePerFrame = (double)m_pAudioTrack->GetDuration() / frames;
		else
			*pAvgTimePerFrame = 0.0;
	}
	else
		*pAvgTimePerFrame = 0.0;
    return SDVR_PLAYER_NOERROR;
}

// get played time in milli-second
sdvr_player_err_e CDemuxFile::GetPlayedTime(DWORD* pTime)
{
	if (m_pVideoTrack->IsEnabled())
	    *pTime = m_pVideoTrack->GetPlayedTime();
	else if (m_pAudioTrack->IsEnabled())
	    *pTime = m_pAudioTrack->GetPlayedTime();
	else
		*pTime = 0;
    return SDVR_PLAYER_NOERROR;
}

// get played frames
sdvr_player_err_e CDemuxFile::GetPlayedFrames(DWORD* pFrames)
{
	if (m_pVideoTrack->IsEnabled())
		*pFrames = m_pVideoTrack->GetPlayedFrames();
	else if (m_pAudioTrack->IsEnabled())
		*pFrames = m_pAudioTrack->GetPlayedFrames();
	else
		*pFrames = 0;
    return SDVR_PLAYER_NOERROR;
}

// set current time
sdvr_player_err_e CDemuxFile::SetCurrentTime(DWORD dwTime)
{
	return SDVR_PLAYER_NOERROR;
}

// set current frame number
sdvr_player_err_e CDemuxFile::SetCurrentFrameNum(DWORD dwFrameNum)
{
	return SDVR_PLAYER_NOERROR;
}

// enable/disable track
sdvr_player_err_e CDemuxFile::EnableTrack(sx_int32 iTrackId)
{
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e CDemuxFile::DisableTrack(sx_int32 iTrackId)
{
	return SDVR_PLAYER_NOERROR;
}

// get next frame
sdvr_player_err_e CDemuxFile::GetNextFrame(sx_int32 iTrackId, CFrameBuffer *pFrame)
{
	CTrack *trk = m_pTracks[iTrackId];
	if (trk != NULL)
		return trk->GetNextFrame(m_pInStrm, pFrame);
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e CDemuxFile::ParseFile(void)
{
	return SDVR_PLAYER_NOERROR;
}

// for debugging
void CDemuxFile::PrintInfo()
{
}
