// track.cpp

#include <windows.h>
#include "sdvr_player.h"
#include "track.h"
#include "instrm.h"
#include "stsplitter.h"
#include <assert.h>

CTrack::~CTrack()
{
}

CTrack::CTrack(track_type_e type, 
				enumCodecId eCodecId,
				sx_int32 iWidth,
				sx_int32 iHeight,
				sx_int32 iSamplingRate,
				sx_int32 iChannels) : 
	m_iTrackId(0),
    m_eType(type),
	m_eCodecId(eCodecId),
    m_uiFlags(0),
	m_uiNumSamples(0),
    m_uiCurSample(0),
    m_iTime(0),
    m_iWidth(iWidth),
    m_iHeight(iHeight),
    m_iSamplingRate(iSamplingRate),
    m_bEnabled(true),
    m_fRefClockRate(100.0),
    m_iCurCfgIdx(-1),
	m_iChannels(iChannels)
{
}

// get number of frames
sx_int32 CTrack::GetNumFrames()
{
	return (sx_int32)m_FrameIndex.size();
}

// get number of key frames
sx_int32 CTrack::GetNumKeyFrames()
{
	return (sx_int32)m_KeyIndex.size();
}

// add a sample entry at the end of array
sdvr_player_err_e
CTrack::AddSampleEntry(sx_int64 pos, 
					   sx_int64 dts, 
					   sx_uint32 size, 
					   sx_uint32 flags, 
					   sx_uint32 samples)
{
	frame_entry_t entry;

	// add entry
	entry.pos = pos;
	entry.dts = dts;
	entry.size = size;
	entry.flags = flags;
	entry.samples = samples;

    if (m_eType == TRACK_VIDEO) {
        // update config index
        entry.cfg_index = (sx_uint32)m_ConfigIndex.size() - 1;
        // update key frame index
        if (CHK_FLAGS(entry.flags, FLAG_KEY_FRAME))
            m_KeyIndex.push_back((sx_uint32)m_FrameIndex.size());
    }
	else if (m_eType == TRACK_AUDIO)
		m_uiNumSamples += samples;

	m_FrameIndex.push_back(entry);
	return SDVR_PLAYER_NOERROR;
}

// add a sps entry
sdvr_player_err_e
CTrack::AddSPSEntry(sx_int64 pos, sx_uint32 size)
{
	config_entry_t entry;

	// add entry
	entry.sps_pos = pos;
	entry.sps_size = size;
    entry.pps_pos = -1;
    entry.pps_size = 0;
	m_ConfigIndex.push_back(entry);
	return SDVR_PLAYER_NOERROR;
}

// add a pps entry
sdvr_player_err_e
CTrack::AddPPSEntry(sx_int64 pos, sx_uint32 size)
{
    // ASSUME: we always get sps/pps pair
    assert(m_ConfigIndex.size() > 0);
	config_entry_t &entry = m_ConfigIndex.back();
    assert(entry.pps_pos == -1 && entry.pps_size == 0);
	entry.pps_pos = pos;
	entry.pps_size = size;
	return SDVR_PLAYER_NOERROR;
}

// get duration in milli-second
sx_int32 CTrack::GetDuration()
{
    if (m_FrameIndex.size() == 0)
        return 0;
    else
	    return (sx_int32)((float)(m_FrameIndex.back().dts - m_FrameIndex.front().dts) / m_fRefClockRate);
}

// get track duration in timestamp
sx_int64 CTrack::GetTSDuration()
{
    if (m_FrameIndex.size() == 0)
        return 0;
    else
	    return m_FrameIndex.back().dts - m_FrameIndex.front().dts;
}

// get play pos in percentage
float CTrack::GetPlayPos()
{
    if (m_FrameIndex.size() < 2)
        return 0.0;
    else {
        sx_int32 duration = GetDuration();
        if (duration == 0)
            return 0.0;
		else {
			unsigned int uiCur = m_uiCurSample >= m_FrameIndex.size() ? 
				(unsigned int)m_FrameIndex.size() - 1 : m_uiCurSample;
            return (float)(m_FrameIndex.at(uiCur).dts - m_FrameIndex.front().dts) 
                / m_fRefClockRate / (float)duration;
		}
    }
}

sx_int32 CTrack::GetNumConfigEntries()
{
    return (sx_int32)m_ConfigIndex.size();
}

// get played time
sx_int32 CTrack::GetPlayedTime()
{
    if (m_FrameIndex.size() == 0)
        return 0;
    else {
        sx_int32 duration = GetDuration();
        if (duration == 0)
            return 0;
		else {
			unsigned int uiCur = m_uiCurSample >= m_FrameIndex.size() ? 
				(unsigned int)m_FrameIndex.size() - 1 : m_uiCurSample;
            return (sx_int32)((float)(m_FrameIndex.at(uiCur).dts - m_FrameIndex.front().dts) / m_fRefClockRate);
		}
    }
}

sx_int32 CTrack::GetPlayedFrames()
{
    return m_uiCurSample;
}

// get current frame's timestamp
sx_int64 CTrack::GetCurrentTimestamp()
{
    if (m_FrameIndex.size() == 0)
        return 0;
    else {
		unsigned int uiCur = m_uiCurSample >= m_FrameIndex.size() ? 
			(unsigned int)m_FrameIndex.size() - 1 : m_uiCurSample;
        return m_FrameIndex.at(uiCur).dts;
    }
}

// set seek flags
void CTrack::SetSeekFlags(sx_uint32 flags)
{
    flags &= (TRACK_FLAG_SEEK_KEY_FRAME);
    SET_FLAGS(m_uiFlags, flags);
}

// seek by timestamp
sdvr_player_err_e
CTrack::SeekByTimestamp(sx_int64 ts)
{
    sx_int32 iFrames = (sx_int32)m_FrameIndex.size();
    sx_int32 a, b, m;
    sx_int64 timestamp;

    a = -1;
    b = iFrames;

    // seek the time stamp
    while (b - a > 1) {
        m = (a + b) >> 1;
        timestamp = m_FrameIndex.at(m).dts;
        if (timestamp >= ts)
            b = m;
        if (timestamp <= ts)
            a = m;
    }
    m = CHK_FLAGS(m_uiFlags, TRACK_FLAG_SEEK_BACKWARD) ? a : b;

    // seek the nearest key frame
    if (CHK_FLAGS(m_uiFlags, TRACK_FLAG_SEEK_KEY_FRAME)) {
        while (m >= 0 && m < iFrames 
            && !CHK_FLAGS(m_FrameIndex.at(m).flags, FLAG_KEY_FRAME)) {
            m += CHK_FLAGS(m_uiFlags, TRACK_FLAG_SEEK_BACKWARD) ? -1 : 1;
        }
    }

    // could not find an entry
    if (m == iFrames)
        m = iFrames - 1;

    // update current sample index and return
    m_uiCurSample = m;
    return SDVR_PLAYER_NOERROR;
}

// seek by pos percentage
sdvr_player_err_e
CTrack::SeekByPercentage(float fPercent)
{
    sx_int64 ts = m_FrameIndex.front().dts;
    // convert percentage to timestamp
    ts += (sx_int64)(fPercent * GetDuration() * m_fRefClockRate);
    // seek by timestamp
    return SeekByTimestamp(ts);
}

// seek by time in milli-second
sdvr_player_err_e
CTrack::SeekByTime(sx_int32 iTime)
{
    sx_int64 ts = m_FrameIndex.front().dts;
    // convert time to timestamp
    ts += (sx_int64)((float)iTime * m_fRefClockRate);
    // seek by timestamp
    return SeekByTimestamp(ts);
}

// seek by frame number
sdvr_player_err_e
CTrack::SeekByFrameNum(sx_int32 iFrameNum)
{
    // check range
    sx_int32 iFrames = (sx_int32)m_FrameIndex.size();
    if (iFrameNum >= iFrames)
        iFrameNum = iFrames - 1;

    if (CHK_FLAGS(m_uiFlags, TRACK_FLAG_SEEK_KEY_FRAME)) {
        // seek the first key frame
        for (sx_int32 i = m_uiCurSample; i < iFrames; i++) {
            if (CHK_FLAGS(m_FrameIndex.at(i).flags, FLAG_KEY_FRAME)) {
                m_uiCurSample = i;
                break;
            }
            // TODO: could not find a key frame, return error code?
        }
    }
    else
        m_uiCurSample = iFrameNum;

    return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e
CTrack::GetNextFrame(CInputStream *pStrm, CFrameBuffer *pFrame)
{
    frame_entry_t entry;

    assert(m_FrameIndex.size() > 0);

    // check if reach the end
    if (m_uiCurSample >= m_FrameIndex.size()) {
        pFrame->type = NULL_FRAME;
        pFrame->size = 0;
        return SDVR_PLAYER_NOERROR;
    }

    if (m_eType == TRACK_AUDIO) {
        // get frame entry
        entry = m_FrameIndex.at(m_uiCurSample);
        if (pFrame->alloc_size < entry.size) {
            if ( pFrame->Realloc(entry.size) == 0 )
                return SDVR_PLAYER_ALLOC_MEMORY_ERROR;
        }

        pFrame->type = AUDIO_FRAME;
        pFrame->size = entry.size;
        pFrame->timestamp = entry.dts;
        pStrm->Seek(entry.pos, SEEK_SET);
        if (pStrm->Read(pFrame->payload, entry.size) < (sx_int32)entry.size)
            return SDVR_PLAYER_READ_FILE_ERROR;
    }
    else if (m_eType == TRACK_VIDEO) {
        // get frame entry
        entry = m_FrameIndex.at(m_uiCurSample);
        sx_uint32 size = entry.size;

        if (entry.cfg_index >= 0 && entry.cfg_index != m_iCurCfgIdx) {
            // get config entry
            config_entry_t config = m_ConfigIndex.at(entry.cfg_index);
            // check size
            size += config.sps_size + config.pps_size;
        }

        if (pFrame->alloc_size < size) {
             if ( pFrame->Realloc(size) == 0 )
                return SDVR_PLAYER_ALLOC_MEMORY_ERROR;
        }
        sx_uint8 *p = pFrame->payload;

        if (entry.cfg_index >= 0 && entry.cfg_index != m_iCurCfgIdx) {
            // get config entry
            config_entry_t config = m_ConfigIndex.at(entry.cfg_index);
            // read sps
            pStrm->Seek(config.sps_pos, SEEK_SET);
            if (pStrm->Read(p, config.sps_size) < (sx_int32)config.sps_size)
                return SDVR_PLAYER_READ_FILE_ERROR;
            p += config.sps_size;
            // read pps
            pStrm->Seek(config.pps_pos, SEEK_SET);
            if (pStrm->Read(p, config.pps_size) < (sx_int32)config.pps_size)
                return SDVR_PLAYER_READ_FILE_ERROR;
            p += config.pps_size;
        } 

        // read frame
        pFrame->type = VIDEO_FRAME;
        pFrame->size = size;
        pFrame->timestamp = entry.dts;
        pStrm->Seek(entry.pos, SEEK_SET);
        if (pStrm->Read(p, entry.size) < (sx_int32)entry.size)
            return SDVR_PLAYER_READ_FILE_ERROR;
        m_iCurCfgIdx = entry.cfg_index;
    }

    // succeed
    m_uiCurSample++;
 
    return SDVR_PLAYER_NOERROR;
}
