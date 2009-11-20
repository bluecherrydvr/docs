// demux_raw.cpp

#include <windows.h>
#include "sdvr_player.h"
#include "demux_raw.h"
#include <assert.h>

CDemuxRawFile::~CDemuxRawFile()
{
	Close();
}

CDemuxRawFile::CDemuxRawFile()
{
	CDemuxFile::CDemuxFile();
}

sdvr_player_err_e 
CDemuxRawFile::Open(LPSTR pstrFileName)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT]; 

	_splitpath_s(pstrFileName, drive, dir, fname, ext);
	m_uiIsEs = 0;
	m_uiIsAvc = 0;
	if (strcmp(ext, ".264") == 0) {
		m_uiIsEs = 1;
		m_uiIsAvc = 1;
	}

	// create input stream
    if ((m_pInStrm = new CInputStream) == NULL)
        return SDVR_PLAYER_ALLOC_MEMORY_ERROR;
    if (!m_pInStrm->Open(pstrFileName))
        return SDVR_PLAYER_FILE_NOT_EXIST;
    m_ofstParsedPos = 0; // parse from the beginning

	if (m_uiIsEs) {
		// ASSUME: one video track and one audio track
		// we should parse the file to get tracks' info
		m_iNumTracks = 1;
		m_pTracks[0] = new CTrack(TRACK_VIDEO, CODEC_ID_H264, 720, 480, 1, 1);
		if (m_pTracks[0] == NULL)
			return SDVR_PLAYER_ALLOC_MEMORY_ERROR;
		m_pVideoTrack = m_pTracks[0];
		m_pAudioTrack = 0;
		return ParseEsFile();
	} else {
		// ASSUME: one video track and one audio track
		// we should parse the file to get tracks' info
		m_iNumTracks = 2;
		m_pTracks[0] = new CTrack(TRACK_VIDEO, CODEC_ID_H264, 720, 480, 1, 1);
		if (m_pTracks[0] == NULL)
			return SDVR_PLAYER_ALLOC_MEMORY_ERROR;
		m_pVideoTrack = m_pTracks[0];
		m_pTracks[1] = new CTrack(TRACK_AUDIO, CODEC_ID_PCM_MULAW, 0, 0, 8000, 2);
		if (m_pTracks[1] == NULL)
			return SDVR_PLAYER_ALLOC_MEMORY_ERROR;
		m_pAudioTrack = m_pTracks[1];
		return ParseFile();
	
	}
}


sdvr_player_err_e 
CDemuxRawFile::Close(void)
{
    if (m_pInStrm != NULL) {
        m_pInStrm->Close();
        delete m_pInStrm;
		m_pInStrm = NULL;
    }
	int i;
	for (i=0; i<MAX_NUM_TRACKS; i++) {
		if (m_pTracks[i] != NULL) {
			delete m_pTracks[i];
			m_pTracks[i] = NULL;
		}
	}
	return SDVR_PLAYER_NOERROR;
}


sdvr_player_err_e 
CDemuxRawFile::ParseFile(void)
{
	sdvr_player_err_e err = SDVR_PLAYER_NOERROR;
    sx_bool end_of_file = false;

    // parse the whole file
	m_ofstFileSize = m_pInStrm->GetFileLength();
    m_pInStrm->Seek(m_ofstParsedPos, SEEK_SET);
	while (err == SDVR_PLAYER_NOERROR) {
        end_of_file = m_ofstFileSize - m_ofstParsedPos < sizeof(sdvr_av_buffer_t1);
        if(end_of_file)
            break;
		err = ParsePacket();     
	}

	// update info if the whole file has been parsed
	if (end_of_file) {
		// calculate and update ref clock rate
		sx_int64 ts = m_pAudioTrack->GetTSDuration();
		sx_int32 samples = m_pAudioTrack->GetNumSamples();
		if (samples > 0) {
			// ref clock rate in kHz
			float fClkRate = (float)ts / (float)samples * (float)m_pAudioTrack->GetNumChannels() 
							 * (float)m_pAudioTrack->GetSamplingRate() / (float)1000.0;
			m_pAudioTrack->SetRefClockRate(fClkRate);
			m_pVideoTrack->SetRefClockRate(fClkRate);
		}

		// update file time
		sx_uint32 vtime = m_pVideoTrack->GetDuration();
		sx_uint32 atime = m_pAudioTrack->GetDuration();
		m_uiFileTime = atime > vtime ? atime : vtime;
	}

    return err;
}

sdvr_player_err_e 
CDemuxRawFile::ParseEsFile(void)
{
	sdvr_player_err_e err = SDVR_PLAYER_NOERROR;
    sx_bool end_of_file = false;

    // parse the whole file
	m_ofstFileSize = m_pInStrm->GetFileLength();
	m_uiTimestamp = 0;
	
	m_pDataBuffer = 0;
	m_pDataBuffer = new sx_uint8[m_ofstFileSize];
	if (!m_pDataBuffer) {
		return SDVR_PLAYER_ALLOC_MEMORY_ERROR;
	}

    m_pInStrm->Seek(m_ofstParsedPos, SEEK_SET);
	while (err == SDVR_PLAYER_NOERROR) {
        end_of_file = (m_ofstFileSize - m_ofstParsedPos) < sizeof(sdvr_av_buffer_t1);
        if (end_of_file)
            break;
		err = ParseEsPacket();     
	}
    return err;
}


void 
CDemuxRawFile::SetBsStart(char *start, int size)
{
	m_pBsStart		= (char *)start;
	m_uiRdBytePos	= 0;
	m_uiRdBitPos	= 0;
	m_uiBufferSize	= size;
}

unsigned char
CDemuxRawFile::SeekNextNalHdr()
{
    int         zero_cnt;
    char        header_code = 0, code;         	

    /* Start searching */
    zero_cnt = 0;
    while (1) {
		if (m_uiBufferSize < 8) {
			return 0;
		}
        code = ParseNextByteAligned();
        if (code == 0x00) {
            zero_cnt++;
        } else if (code == 0x01) {
            if (zero_cnt >= 3) {
				if (m_uiBufferSize < 8) {
					return 0;
				}
                header_code = ParseNextByteAligned();
                break;
            } else {
                zero_cnt = 0;
            }
        } else {
            zero_cnt = 0;        
		}
    }	

    return header_code;
}

int 
CDemuxRawFile::ParseNextByteAligned()
{
    int  value = 0, bytepos;
    int  bitpos;

    assert((m_uiRdBitPos == 0));

    bitpos      = m_uiRdBytePos * 8 + m_uiRdBitPos;
    bytepos     = bitpos >> 3;
    value       = m_pBsStart[bytepos];

    m_uiRdBytePos++;
	m_uiBufferSize--;
 
    return value;
}



sdvr_player_err_e 
CDemuxRawFile::ParseEsPacket()
{
	int		start_code, nal_type;
	int		curr_start_code_pos, next_start_code_pos;
	int		nal_size;
	int		err = 0, num_bytes_for_start_code;
	int		start_pos;

    // Read the complete file.
	if (m_ofstParsedPos == 0) {
		m_pInStrm->Read(m_pDataBuffer, m_ofstFileSize);
	}
	start_pos = m_ofstParsedPos;

	SetBsStart((char *)m_pDataBuffer + m_ofstParsedPos, m_ofstFileSize - m_ofstParsedPos - 1);
	
	start_code = SeekNextNalHdr();
	if (start_code == 0) {
		m_ofstParsedPos = m_ofstFileSize;
		return SDVR_PLAYER_NOERROR;
	}
	nal_type  = start_code & 0x1f;

	curr_start_code_pos = m_uiRdBytePos - 5;
	start_code = SeekNextNalHdr();
	if (start_code == 0) {
		m_ofstParsedPos = m_ofstFileSize;
		return SDVR_PLAYER_NOERROR;
	}
	next_start_code_pos = m_uiRdBytePos - 5;
	nal_size = next_start_code_pos - curr_start_code_pos;

    // parse header
	switch (nal_type) {
	case H264_NAL_SLICE_IDR:
		m_uiTimestamp = m_uiFileTime * 90;
		m_pVideoTrack->AddSampleEntry(m_ofstParsedPos, m_uiTimestamp, nal_size, FLAG_KEY_FRAME, 1);
		m_uiFileTime += 5;
		break;
	case H264_NAL_SLICE:
		m_uiTimestamp = m_uiFileTime * 90;
		m_pVideoTrack->AddSampleEntry(m_ofstParsedPos, m_uiTimestamp, nal_size, 0, 1);
		m_uiFileTime += 5;
		break;
	case H264_NAL_SPS:
        m_pVideoTrack->AddSPSEntry(m_ofstParsedPos, nal_size);
        break;
	case H264_NAL_PPS:
        m_pVideoTrack->AddPPSEntry(m_ofstParsedPos, nal_size);
		break;
	default:
        return SDVR_PLAYER_UNKNOWN_FRAME_TYPE;
	}

   	m_ofstParsedPos = next_start_code_pos + start_pos;

    return SDVR_PLAYER_NOERROR;
}


sdvr_player_err_e 
CDemuxRawFile::ParsePacket()
{
	sdvr_av_buffer_t1 pkt_hdr;

    // read packet header
    m_pInStrm->Read((sx_uint8*)&pkt_hdr, sizeof(sdvr_av_buffer_t1));
	m_ofstParsedPos += sizeof(sdvr_av_buffer_t1);

    // parse header
	switch (pkt_hdr.frame_type) {
	case SDVR_FRAME_H264_IDR:
	case SDVR_FRAME_H264_I:
		m_pVideoTrack->AddSampleEntry(m_ofstParsedPos, pkt_hdr.timestamp, 
            pkt_hdr.payload_size, FLAG_KEY_FRAME, 1);
		break;
	case SDVR_FRAME_H264_P:
	case SDVR_FRAME_H264_B:
		m_pVideoTrack->AddSampleEntry(m_ofstParsedPos, pkt_hdr.timestamp, 
            pkt_hdr.payload_size, 0, 1);
		break;
	case SDVR_FRAME_H264_SPS:
        m_pVideoTrack->AddSPSEntry(m_ofstParsedPos, pkt_hdr.payload_size);
        break;
	case SDVR_FRAME_H264_PPS:
        m_pVideoTrack->AddPPSEntry(m_ofstParsedPos, pkt_hdr.payload_size);
		break;
	case SDVR_FRAME_AUDIO_ENCODED:
		//printf("a pkt %I64d, %I64d, %d\n", m_ofstParsedPos, pkt_hdr.timestamp, pkt_hdr.payload_size);
		m_pAudioTrack->AddSampleEntry(m_ofstParsedPos, 
			pkt_hdr.timestamp, pkt_hdr.payload_size, 0, pkt_hdr.payload_size);
		break;
	default:
        return SDVR_PLAYER_UNKNOWN_FRAME_TYPE;
	}

	// skip payload to next packet
    m_pInStrm->Seek(pkt_hdr.payload_size, SEEK_CUR);
   	m_ofstParsedPos += pkt_hdr.payload_size;

    return SDVR_PLAYER_NOERROR;
}

// set play pos by percentage
sdvr_player_err_e 
CDemuxRawFile::SetPlayPos(float fRelativePos)
{
	sdvr_player_err_e err = SDVR_PLAYER_NOERROR;

	if (m_pVideoTrack->IsEnabled()) {
        m_pVideoTrack->SetSeekFlags(TRACK_FLAG_SEEK_KEY_FRAME);
		// seek video by pos
		err = m_pVideoTrack->SeekByPercentage(fRelativePos);
		if (err != SDVR_PLAYER_NOERROR)
			return err;

        m_pVideoTrack->ResetCfgIdx();
		// get current video timestamp
		sx_int64 ts = m_pVideoTrack->GetCurrentTimestamp();

		if (m_pAudioTrack) {
			// seek audio by timestamp
			if (m_pAudioTrack->IsEnabled()) {
				err = m_pAudioTrack->SeekByTimestamp(ts);
			}
		}
	}
	else if (m_pAudioTrack->IsEnabled()) {
		err = m_pAudioTrack->SeekByPercentage(fRelativePos);
	}

    return err;
}

// set current time by milli-second
sdvr_player_err_e 
CDemuxRawFile::SetCurrentTime(DWORD dwTime)
{
	sdvr_player_err_e err;

	if (m_pVideoTrack->IsEnabled()) {

        m_pVideoTrack->SetSeekFlags(TRACK_FLAG_SEEK_KEY_FRAME);

		// seek video by time
		err = m_pVideoTrack->SeekByTime(dwTime);
		if (err != SDVR_PLAYER_NOERROR)
			return err;

        m_pVideoTrack->ResetCfgIdx();

		// get current video timestamp
		sx_int64 ts = m_pVideoTrack->GetCurrentTimestamp();
		// seek audio by timestamp
        if (m_pAudioTrack->IsEnabled()) {
		    err = m_pAudioTrack->SeekByTimestamp(ts);
        }
	}
	else if (m_pAudioTrack->IsEnabled()) {
		err = m_pAudioTrack->SeekByTime(dwTime);
	}

    return err;
}

// set current frame number
sdvr_player_err_e 
CDemuxRawFile::SetCurrentFrameNum(DWORD dwFrameNum)
{
	if (!m_pVideoTrack->IsEnabled())
		return SDVR_PLAYER_FILE_NO_VIDEO;

    // seek video by frame number
    sdvr_player_err_e err = m_pVideoTrack->SeekByFrameNum(dwFrameNum);
    if (err != SDVR_PLAYER_NOERROR)
        return err;
    // get current video timestamp
    sx_int64 ts = m_pVideoTrack->GetCurrentTimestamp();
    // seek audio by timestamp
    err = m_pAudioTrack->SeekByTimestamp(ts);
    return err;
}

void CDemuxRawFile::PrintInfo()
{
	printf("file\n");
	printf("  time = %d\n", m_uiFileTime);
	printf("  ref clk rate = %8.3f Hz\n", m_pAudioTrack->GetRefClockRate() * (float)1000.0);

	printf("video\n");
	printf("  frames = %d\n", m_pVideoTrack->GetNumFrames());
	printf("  key frames = %d\n", m_pVideoTrack->GetNumKeyFrames());
	printf("  duration = %6.1f sec\n", (float)m_pVideoTrack->GetDuration() / 1000.0);
    double dTemp;
    GetAverageTimePerFrame(&dTemp);
	printf("  avg time per frame = %6.1f ms\n", dTemp);
	printf("  config entries = %d\n", m_pVideoTrack->GetNumConfigEntries());

	printf("audio\n");
	printf("  frames = %d\n", m_pAudioTrack->GetNumFrames());
	printf("  samples = %d\n", m_pAudioTrack->GetNumSamples());
	printf("  duration = %6.1f sec\n", (float)m_pAudioTrack->GetDuration() / 1000.0);
}
