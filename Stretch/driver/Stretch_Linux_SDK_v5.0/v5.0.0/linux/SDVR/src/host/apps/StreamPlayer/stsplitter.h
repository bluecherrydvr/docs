// stsplitter.h
#ifndef _STSPLITTER_H_
#define _STSPLITTER_H_

#include "common.h"
#include "instrm.h"
#include "track.h"
#include "st_queue.h"

#define MAX_NUM_TRACKS	16

typedef enum {
	NULL_FRAME = 0,
	VIDEO_FRAME,
	AUDIO_FRAME,
} frame_type_e;

class CFrameBuffer
{
public:
	sx_uint64   timestamp;
	sx_uint32   alloc_size;
	sx_uint32   size;
	frame_type_e type;
	sx_uint8    *payload;
	sx_uint32	*_p;
	CFrameBuffer() {
		int sz = 1024; // must be multiple of 4
		size = 0;
		type = NULL_FRAME;
		_p = new sx_uint32[sz/4];
		if (_p == NULL) {
			alloc_size = 0;
			payload = NULL;
		}
		else {
			alloc_size = sz;
			payload = (sx_uint8 *)_p;
		}
	}
	~CFrameBuffer() {
		if (_p != NULL) {
			delete _p;
			_p = NULL;
			payload = NULL;
		}
	}
	int Realloc(int size) {
		if (size <= 0)
			return 0;
		if (_p != NULL) {
			delete _p;
			payload = NULL;
		}
		if (size % 4)
			size = size + 4 - (size % 4);
		_p = new sx_uint32[size/4];
		if (_p == NULL) {
			payload = NULL;
			alloc_size = 0;
			return 0;
		}
		payload = (sx_uint8 *)_p;
		alloc_size = size;
		return size;
	}
};

class CDemux
{
public:
	~CDemux();
	CDemux();
};

// demux file class
class CDemuxFile : public CDemux
{
public:
	~CDemuxFile();
	CDemuxFile();

	// open/close file
	virtual sdvr_player_err_e Open(LPSTR pstrFileName);
	virtual sdvr_player_err_e Close(void);

	virtual sx_int32 GetNumTracks();
	virtual CTrack *GetTrack(sx_int32 index);

	// get/set play speed
	virtual sdvr_player_err_e SetPlaySpeed(float fSpeed);
	virtual sdvr_player_err_e GetPlaySpeed(float* pSpeed);

	// get/set play position
	virtual sdvr_player_err_e SetPlayPos(float fRelativePos);
	virtual sdvr_player_err_e GetPlayPos(float* pRelativePos);

    // get file time
    virtual sdvr_player_err_e GetFileTime(sx_int32* pTime);

	// get total frames (video)
	virtual sdvr_player_err_e GetTotalFrames(sx_int32 *pFrames);
	// get average time per frame (video)
	virtual sdvr_player_err_e GetAverageTimePerFrame(double* pAvgTimePerFrame);

	// get played time
	virtual sdvr_player_err_e GetPlayedTime(DWORD* pTime);
	// get played frames
	virtual sdvr_player_err_e GetPlayedFrames(DWORD* pFrames);

	// set current time
	virtual sdvr_player_err_e SetCurrentTime(DWORD dwTime);
	// set current frame number
	virtual sdvr_player_err_e SetCurrentFrameNum(DWORD dwFrameNum);

	// enable/disable track
	virtual sdvr_player_err_e EnableTrack(sx_int32 iTrackId);
	virtual sdvr_player_err_e DisableTrack(sx_int32 iTrackId);

	// get next frame
	virtual sdvr_player_err_e GetNextFrame(sx_int32 iTrackId, CFrameBuffer *pFrame);

	virtual sdvr_player_err_e ParseFile(void);

	// for debugging
	virtual void PrintInfo();

protected:
	CInputStream*   m_pInStrm;			// input stream
	sx_int32		m_iNumTracks;		// number of tracks
	CTrack*         m_pTracks[MAX_NUM_TRACKS];	// tracks
	CTrack*         m_pVideoTrack;		// video track
	CTrack*         m_pAudioTrack;		// audio track
	sx_uint32       m_uiFileTime;		// file time in milli-second
	offset_t        m_ofstFileSize;		// file size
	offset_t        m_ofstParsedPos;	// parsed file pos
	float           m_fPlaySpeed;		// playback speed
};

#endif
