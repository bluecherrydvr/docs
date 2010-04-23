// stplayer.cpp

#include <windows.h>
#include <basetsd.h>
#include <assert.h>
#include "stplayer.h"
#include <mmreg.h>
#include <dxerr9.h>
#include <math.h>
#include "syncobj.h"
#include "demux_raw.h"
#include "sndplayer.h"
#include "vidplayer.h"
#include "systime.h"
#include "streamplayer.h"
#include "stRtpIO.h"


static void fast_memcpy(void *dst, void *src, int size)
{
	unsigned long *dst_long = (unsigned long *)dst;
	unsigned long *src_long = (unsigned long *)src;

	if ((((int)dst & 0x3) == 0) && (((int)src & 0x3) == 0)) {
		while (size >= 4) {
			*dst_long++ = *src_long++;
			size -= 4;
		}

		memcpy((void *)dst_long, (void *)src_long, size);
	} else
		memcpy((void *)dst_long, (void *)src_long, size);

}

CStretchPlayer::~CStretchPlayer()
{
	if (m_hRtpRcv) {
		stRtpIORcvClose(m_hRtpRcv);
		m_hRtpRcv = NULL;
	}
}

CStretchPlayer::CStretchPlayer() :
	m_eState(SDVR_PLAYER_EMPTY),
	m_hWnd(NULL),
	m_hMsgWnd(NULL),
	m_uiMsg(0),
	m_iNumStreams(0),
	m_hThread(NULL),
	m_dwThreadId(NULL),
	m_pSplitter(NULL),
	m_pSysTime(NULL),
	m_fPlaySpeed(1.0),
	m_hRtpRcv(NULL),
	m_frame(NULL),
	m_uiSpsReceived(0),
	p_dst(0)
{
}

// open file
sdvr_player_err_e 
CStretchPlayer::OpenFile(LPSTR pstrFileName)
{
    sdvr_player_err_e err;

	// create system time
	m_pSysTime = new CSystemTime;
	if (m_pSysTime == NULL)
		return SDVR_PLAYER_ALLOC_MEMORY_ERROR;

	// create splitter and open file
	m_pSplitter = new CDemuxRawFile;
	if (m_pSplitter == NULL)
		return SDVR_PLAYER_ALLOC_MEMORY_ERROR;

	// open and parse file
	err = m_pSplitter->Open(pstrFileName);
    if (err != SDVR_PLAYER_NOERROR)
        return err;

	// check file duration
	sx_int32 time;
	m_pSplitter->GetFileTime(&time);
	if (time <= 0)
		return SDVR_PLAYER_READ_FILE_ERROR;

	// get number of tracks
	m_iNumStreams = m_pSplitter->GetNumTracks();
	if (m_iNumStreams <= 0)
		return SDVR_PLAYER_READ_FILE_ERROR;

	// create stream players according to tracks
	m_pStrmPlayer = new CStreamPlayer[m_iNumStreams];
	if (m_pStrmPlayer == NULL)
		return SDVR_PLAYER_ALLOC_MEMORY_ERROR;

	int i;
	sx_bool bHasMaster = false;
	for (i=0; i<m_iNumStreams; i++) {
		CTrack *trk = m_pSplitter->GetTrack(i);
		// check track length
		if (trk->GetDuration() > 0) {
			m_pStrmPlayer[i].Enable(true);
			m_pStrmPlayer[i].Open(this, trk->GetType(), trk->GetCodecId(), m_pSysTime);
#if 1 // use audio as master
			// set the first audio track as master
			if (!bHasMaster && trk->GetType() == TRACK_AUDIO) {
				m_pStrmPlayer[i].SetMaster(true);
				bHasMaster = true;
			}
#endif
		}
		else
			m_pStrmPlayer[i].Enable(false);
	}

	// if no audio as master, use first video track as master
	if (!bHasMaster) {
		for (i=0; i<m_iNumStreams; i++) {
			if (m_pStrmPlayer[i].GetType() == TRACK_VIDEO 
				&& m_pStrmPlayer[i].IsEnabled()) {
				m_pStrmPlayer[i].SetMaster(true);
				bHasMaster = true;
				break;
			}
		}
	}

	// player initialization
    err = Initialize();
    if (err != SDVR_PLAYER_NOERROR)
        return err;

	m_eState = SDVR_PLAYER_READY;
	return SDVR_PLAYER_NOERROR;
}

// open file
sdvr_player_err_e 
CStretchPlayer::OpenSdpFile(LPSTR pstrFileName)
{
#if 1
    sdvr_player_err_e	err;
	FILE*				sdpFile;
	char				nextchar;
	int					i = 0;

	// create system time
	m_pSysTime = new CSystemTime;
	if (m_pSysTime == NULL)
		return SDVR_PLAYER_ALLOC_MEMORY_ERROR;

	// Initialized the rtp Rcv
	m_pRcvCfg = (void *)malloc(sizeof(stRtpIORcvConfig_t));
	memset(m_pRcvCfg, 0, sizeof(stRtpIORcvConfig_t));		
    sdpFile = fopen(pstrFileName, "r");
    if (sdpFile == NULL) {
		return SDVR_PLAYER_FILE_NOT_EXIST;
    }

	i = 0;
    while ((nextchar = fgetc(sdpFile)) != EOF) {
        ((stRtpIORcvConfig_t *)m_pRcvCfg)->inputSdpDesc[i++] = (char)nextchar;
    }
	((stRtpIORcvConfig_t *)m_pRcvCfg)->bufferCapacity	= 0x100000;
	((stRtpIORcvConfig_t *)m_pRcvCfg)->numBuffers		= 64;
	((stRtpIORcvConfig_t *)m_pRcvCfg)->nonblocking      = 0;

	m_bRtp = true;

	// get number of tracks
	m_iNumStreams = 1;

	// create stream players according to tracks
	m_pStrmPlayer = new CStreamPlayer[1];
	if (m_pStrmPlayer == NULL)
		return SDVR_PLAYER_ALLOC_MEMORY_ERROR;

	m_pStrmPlayer[0].Enable(true);
	m_pStrmPlayer[0].Open(this, TRACK_VIDEO, CODEC_ID_H264, m_pSysTime);
	m_pStrmPlayer[0].SetMaster(true);

	// player initialization
    err = Initialize();
    if (err != SDVR_PLAYER_NOERROR)
        return err;

	m_eState = SDVR_PLAYER_READY;

#endif
	return SDVR_PLAYER_NOERROR;
}

// close file
sdvr_player_err_e 
CStretchPlayer::Close()
{
	if (m_eState == SDVR_PLAYER_EMPTY)
		return SDVR_PLAYER_NOERROR;

	// stop and release stream players
	Stop();
	if (m_pStrmPlayer != NULL) {
		delete[] m_pStrmPlayer;
		m_pStrmPlayer = NULL;
	}

	// quit thread
	if (m_hThread != NULL) {
#if 0
		// send quit message
		::PostThreadMessage(m_dwThreadId, WM_QUIT, NULL, NULL);
		// wait thread to end
		WaitForSingleObject(m_hThread, INFINITE);
#endif
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	// release splitter
	if (m_pSplitter != NULL) {
		m_pSplitter->Close();
		delete m_pSplitter;
		m_pSplitter = NULL;
	}

	// release system time
	if (m_pSysTime != NULL) {
		delete m_pSysTime;
		m_pSysTime = NULL;
	}

	m_eState = SDVR_PLAYER_EMPTY;
	return SDVR_PLAYER_NOERROR;
}

// register file end message
sdvr_player_err_e 
CStretchPlayer::RegisterFileEndMessage(HWND hWnd, sx_uint32 uiMsg)
{
	m_hMsgWnd = hWnd;
	m_uiMsg = uiMsg;
	return SDVR_PLAYER_NOERROR;
}

void 
CStretchPlayer::SendFileEndMessage()
{
	::PostMessage(m_hMsgWnd, m_uiMsg, NULL, NULL);
}

sdvr_player_err_e 
CStretchPlayer::Play(HWND hWnd)
{
	ResetEvent(m_UIEventId);
	::PostThreadMessage(m_dwThreadId, WM_PLAYER_UI, PLAYER_CMD_PLAY, (LPARAM)hWnd);
	DWORD ret = WaitForSingleObject(m_UIEventId, INFINITE);
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CStretchPlayer::Stop()
{
	ResetEvent(m_UIEventId);

#if 0
	::PostThreadMessage(m_dwThreadId, WM_PLAYER_UI, PLAYER_CMD_STOP, NULL);
	WaitForSingleObject(m_UIEventId, INFINITE);
#else
	_Stop();
#endif
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CStretchPlayer::Pause()
{
	ResetEvent(m_UIEventId);
	::PostThreadMessage(m_dwThreadId, WM_PLAYER_UI, PLAYER_CMD_PAUSE, NULL);
	WaitForSingleObject(m_UIEventId, INFINITE);
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CStretchPlayer::SetPlayPos(float fRelativePos) 
{
	ResetEvent(m_UIEventId);
	::PostThreadMessage(m_dwThreadId, WM_PLAYER_UI, PLAYER_CMD_SET_POS, (LPARAM)&fRelativePos);
	WaitForSingleObject(m_UIEventId, INFINITE);
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CStretchPlayer::DoublePlaySpeed() 
{
	m_fPlaySpeed *= 2.0;
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CStretchPlayer::HalvePlaySpeed() 
{
	m_fPlaySpeed /= 2.0;
	return SDVR_PLAYER_NOERROR;
}

// play sound
sdvr_player_err_e 
CStretchPlayer::PlaySound(sx_bool bEnableShare)
{
//	assert(m_hWnd != NULL);
	for (int i=0; i<m_iNumStreams; i++) {
		if (m_pStrmPlayer[i].GetType() == TRACK_AUDIO)
			m_pStrmPlayer[i].Play(m_hWnd);
	}
	return SDVR_PLAYER_NOERROR;
}

// stop sound
sdvr_player_err_e 
CStretchPlayer::StopSound()
{
	return SDVR_PLAYER_NOERROR;
}

// play video
sdvr_player_err_e 
CStretchPlayer::PlayVideo()
{
//	assert(m_hWnd != NULL);
	for (int i=0; i<m_iNumStreams; i++) {
		if (m_pStrmPlayer[i].GetType() == TRACK_VIDEO)
			m_pStrmPlayer[i].Play(m_hWnd);
	}
	return SDVR_PLAYER_NOERROR;
}

// refresh display
sdvr_player_err_e 
CStretchPlayer::Refresh()
{
/*	for (int i=0; i<m_iNumStreams; i++) {
		if (m_pStrmPlayer[i].GetType() == TRACK_VIDEO)
			m_pStrmPlayer[i].Refresh();
	}*/

    return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CStretchPlayer::GetState(sdvr_player_state_e *pState) 
{
	*pState = m_eState;
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CStretchPlayer::GetPlayRate(float *pfPlayRate) 
{
	*pfPlayRate = m_fPlaySpeed;
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CStretchPlayer::GetTotalFrames(sx_int32 *pFrames) 
{
    return m_pSplitter->GetTotalFrames(pFrames);
}

// get average time per frame (video)
sdvr_player_err_e 
CStretchPlayer::GetAverageTimePerFrame(double* pAvgTimePerFrame) 
{
    return m_pSplitter->GetAverageTimePerFrame(pAvgTimePerFrame);
}

sdvr_player_err_e 
CStretchPlayer::GetFileTime(sx_int32* pTime) 
{
    return m_pSplitter->GetFileTime(pTime);
}

sdvr_player_err_e 
CStretchPlayer::GetPlayPos(float* pRelativePos) 
{
    return m_pSplitter->GetPlayPos(pRelativePos);
}

sdvr_player_err_e 
CStretchPlayer::GetVolume(sx_uint16 *pVolume)
{
	for (int i=0; i<m_iNumStreams; i++) {
		if (m_pStrmPlayer[i].GetType() == TRACK_AUDIO) {
			LONG vol;
			sdvr_player_err_e err = m_pStrmPlayer[i].GetVolume(&vol);
			if (err == SDVR_PLAYER_NOERROR)
				*pVolume = (sx_uint16)vol;
			else
				*pVolume = 0;
			return err;
		}
	}
	return SDVR_PLAYER_FILE_NO_AUDIO;
}

sdvr_player_err_e 
CStretchPlayer::SetVolume(sx_uint16 volume)
{
	for (int i=0; i<m_iNumStreams; i++) {
		if (m_pStrmPlayer[i].GetType() == TRACK_AUDIO)
			return m_pStrmPlayer[i].SetVolume((LONG)volume - 10000);
	}
	return SDVR_PLAYER_FILE_NO_AUDIO;
}

// streaming
sdvr_player_err_e 
CStretchPlayer::OpenStream(sx_int32 iBufSize)
{
	return SDVR_PLAYER_THREAD_ERROR;
}

sdvr_player_err_e 
CStretchPlayer::InputData(sx_uint8 *pBuf, sx_int32 iSize)
{
	return SDVR_PLAYER_THREAD_ERROR;
}

sdvr_player_err_e 
CStretchPlayer::GetBufferAvailableSize(sx_uint32 *pSize)
{
	return SDVR_PLAYER_THREAD_ERROR;
}

////////////////////////////
// internal implementations
// player thread
DWORD WINAPI PlayerThread( LPVOID lpParam )
{
	CStretchPlayer &player = *((CStretchPlayer *)lpParam);
	sdvr_player_err_e err;
	sx_int32 iAbort = 0;
	MSG msg;

	while (!iAbort) {
		// check messages
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			switch (msg.message) {
			case WM_QUIT: // quit message
				iAbort = 1;
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
		// splitter handler
		err = player.SplitterHandler();
	}

	return 0;
}

sdvr_player_err_e 
CStretchPlayer::Initialize()
{
	// create UI event
	m_UIEventId = CreateEvent(NULL, FALSE, FALSE, NULL); 

    // create thread
    m_hThread = CreateThread(NULL,		// security descriptor
                             0,			// stack size
                             PlayerThread,		// thread function
                             this,		// thread parameter
                             0,			// start thread immediately
                             &m_dwThreadId);	// thread id
    if (m_hThread == NULL) {
	    return SDVR_PLAYER_THREAD_ERROR;
    }

	// wait until thread is up
	ResetEvent(m_UIEventId);
	do {
		::PostThreadMessage(m_dwThreadId, WM_PLAYER_TEST, NULL, NULL);
		DWORD res = WaitForSingleObject(m_UIEventId, 100);
		switch (res) {
		case WAIT_OBJECT_0:
			return SDVR_PLAYER_NOERROR;
		case WAIT_TIMEOUT:
			continue;
		default:
			return SDVR_PLAYER_THREAD_ERROR;
		}
	} while (1);

	return SDVR_PLAYER_NOERROR;
}

// UI message handler
sdvr_player_err_e 
CStretchPlayer::UIHandler(MSG &msg)
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
	case PLAYER_CMD_SET_POS:
		_SetPlayPos((float*)msg.lParam);
		break;
	}
	// response by setting event
	SetEvent(m_UIEventId);
	return SDVR_PLAYER_NOERROR;
}

#define STREAM_MAX_FRAME_SIZE		0x100000


sdvr_player_err_e 
CStretchPlayer::SplitterHandler()
{
	CDemuxRawFile &spl = *m_pSplitter;
	sdvr_player_err_e err;

	if (m_bRtp) {

		if (m_eState == SDVR_PLAYER_PLAY) {
			stRtpIOErr_t		rtp_err = ST_RTPIO_OK;
			char				*rcv_buf = NULL;
			int					nal_size = 0;
			FrameQueue&			q = m_pStrmPlayer[0].GetFrameQueue();
			int					nal_type;

			if (p_dst == 0) {
				if (!q.IsFull()) {
					m_frame = q.GetHead();
					if (m_frame->alloc_size < STREAM_MAX_FRAME_SIZE) {
						if (m_frame->Realloc(STREAM_MAX_FRAME_SIZE) == 0 )
						return SDVR_PLAYER_ALLOC_MEMORY_ERROR;
					}
					p_dst = (char *)m_frame->payload;
					decode_size = 0;
				} else {
					Sleep(1);
					return SDVR_PLAYER_NOERROR;
				}
			}
			
			if (m_hRtpRcv == NULL) {

				SetPriorityClass(GetCurrentProcess, ABOVE_NORMAL_PRIORITY_CLASS);

				rtp_err = stRtpIORcvCreate(&m_hRtpRcv, ((stRtpIORcvConfig_t *)m_pRcvCfg));
				if (rtp_err) {
					assert(0);
					return	SDVR_PLAYER_RTP_ERROR;
				}

				/* Start RTP snd/rcv channels */
				rtp_err = stRtpIORcvStart(m_hRtpRcv);
				if (rtp_err) {
					assert(0);
					return SDVR_PLAYER_RTP_ERROR;
				}
				
				m_bRtpStarted = true;
				m_uiSpsReceived = 0;
				decode_time = 0;

 			}

			if (m_uiSpsReceived == 0) {

				/* Check for SPS */
				stRtpIORcvGetFullBuf(m_hRtpRcv, &rcv_buf, &nal_size); 
				
				nal_type  = rcv_buf[0] & 0x1f;
				if ((nal_type == H264_NAL_SPS) && ((rcv_buf[4] & 0x80) == 0x80)) {
					m_uiSpsReceived = 1;
				} else {
					stRtpIORcvPutEmptyBuf(m_hRtpRcv, rcv_buf);
					return SDVR_PLAYER_NOERROR;
				} 
				
				/* Copy the sps to the target frame buffer */
				p_dst[0] = 0x00;
				p_dst[1] = 0x00;
				p_dst[2] = 0x00;
				p_dst[3] = 0x01;
				p_dst		+= 4;
				decode_size += 4;

				fast_memcpy(p_dst, rcv_buf, nal_size);
				p_dst		+= nal_size;
				decode_size += nal_size;

				stRtpIORcvPutEmptyBuf(m_hRtpRcv, rcv_buf);
			} 

			/* Get another NAL */
			stRtpIORcvGetFullBuf(m_hRtpRcv, &rcv_buf, &nal_size); 

			p_dst[0] = 0x00;
			p_dst[1] = 0x00;
			p_dst[2] = 0x00;
			p_dst[3] = 0x01;
			p_dst += 4;
			decode_size += 4;

			fast_memcpy(p_dst, rcv_buf, nal_size);
			p_dst += nal_size;
			decode_size += nal_size;

			nal_type  = rcv_buf[0] & 0x1f;

			if (nal_type == H264_NAL_SLICE ||
				nal_type == H264_NAL_SLICE_DPA ||
				nal_type == H264_NAL_SLICE_DPB ||
				nal_type == H264_NAL_SLICE_DPC ||
				nal_type == H264_NAL_SLICE_IDR) {
				
				/* Can forward the frame to decoder after receiving one slice */
				m_frame->type = VIDEO_FRAME;
				m_frame->size = decode_size;
				m_frame->timestamp = (sx_uint64)(decode_time / 100.0);
				q.CommitHead();
	
				m_frame = NULL;
				decode_size = 0;
				p_dst = 0;
				decode_time += 0.005;
			} 
			 	
			stRtpIORcvPutEmptyBuf(m_hRtpRcv, rcv_buf);
		} else {
			Sleep(100);
		}
	} else {
		if (m_eState == SDVR_PLAYER_PLAY) {
			sx_bool bMoreSpace;
			do {
				bMoreSpace = false;
				for (int i=0; i<m_iNumStreams; i++) {
					if (m_pStrmPlayer[i].IsEnabled()) {
						FrameQueue& q = m_pStrmPlayer[i].GetFrameQueue();
						if (!q.IsFull()) {
							bMoreSpace = true;
							CFrameBuffer *frame = q.GetHead();
							// read a frame
							err = spl.GetNextFrame(i, frame);
							if (err == SDVR_PLAYER_NOERROR) {
								q.CommitHead();
							}
						}
					}
				}
			} while (bMoreSpace);
			::Sleep(10);
		}
		else
			::Sleep(500);
	}

	return SDVR_PLAYER_NOERROR;
}


sdvr_player_err_e 
CStretchPlayer::_Play(HWND hWnd)
{
    sdvr_player_err_e err;

    if (m_eState == SDVR_PLAYER_EMPTY)
        return SDVR_PLAYER_NOT_READY;

	m_pSysTime->Reset();

    if (m_eState != SDVR_PLAYER_PLAY) { 
		m_hWnd = hWnd;
		for (int i=0; i<m_iNumStreams; i++) {
			err = m_pStrmPlayer[i].Play(hWnd);
			if (err != SDVR_PLAYER_NOERROR)
				return err;
		}
	    m_eState = SDVR_PLAYER_PLAY;
    }

	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CStretchPlayer::_Stop()
{
    sdvr_player_err_e err;
	if (m_pStrmPlayer != NULL) {
		for (int i=0; i<m_iNumStreams; i++) {
			err = m_pStrmPlayer[i].Stop();
			if (err != SDVR_PLAYER_NOERROR)
				return err;
		}
	}
	m_eState = SDVR_PLAYER_STOP;

	if (m_pSplitter) {
		// reset play pos to 0
		m_pSplitter->SetPlayPos(0.0);
	}

	if (m_hRtpRcv) {
		stRtpIORcvStop(m_hRtpRcv);
		stRtpIORcvClose(m_hRtpRcv);
		m_hRtpRcv = NULL;
	}

	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CStretchPlayer::_Pause()
{
    sdvr_player_err_e err;
	if (m_eState == SDVR_PLAYER_PAUSE)
		return SDVR_PLAYER_NOERROR;
	for (int i=0; i<m_iNumStreams; i++) {
		err = m_pStrmPlayer[i].Pause();
		if (err != SDVR_PLAYER_NOERROR)
			return err;
	}
	m_eState = SDVR_PLAYER_PAUSE;
	return SDVR_PLAYER_NOERROR;
}

sdvr_player_err_e 
CStretchPlayer::_SetPlayPos(float *pfRelativePos) 
{
	for (int i=0; i<m_iNumStreams; i++)
		m_pStrmPlayer[i].FlushQueue();
	m_pSysTime->Reset();
    return m_pSplitter->SetPlayPos(*pfRelativePos);
}
