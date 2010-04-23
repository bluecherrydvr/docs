// track.h
#ifndef _TRACK_H_
#define _TRACK_H_

#include "common.h"
#include <vector>

// frame entry flags
#define FLAG_KEY_FRAME              0x00000001

// track flags
#define TRACK_FLAG_SEEK_KEY_FRAME   0x00000001
#define TRACK_FLAG_SEEK_BACKWARD    0x00000002

// track type
typedef enum {
    TRACK_VIDEO,
    TRACK_AUDIO,
    TRACK_UNKNOWN,
} track_type_e;

// frame index entry
typedef struct {
    sx_int64    pos;        // offset in file
    sx_int64    dts;        // dts
    sx_uint32   size;       // frame size
    sx_uint32   flags;      // flags
    union {
        sx_uint32   cfg_index;  // video: configuration index
        sx_uint32   samples;    // audio: samples in this frame
    };
    sx_uint32   key_index;      // video: key frame index
} frame_entry_t;

// config index entry
typedef struct {
    sx_int64    sps_pos;    // sps offset in file
    sx_int64    pps_pos;    // pps offset in file
    sx_uint32   sps_size;   // sps size
    sx_uint32   pps_size;   // pps size
} config_entry_t;

// arrays
using namespace std;
typedef vector <frame_entry_t> FrameArray;
typedef vector <config_entry_t> ConfigArray;
typedef vector <sx_uint32> Uint32Array;

class CInputStream;
class CFrameBuffer;

// track class
class CTrack {
public:
    ~CTrack();
    CTrack(track_type_e type, 
			enumCodecId eCodecId,
			sx_int32 iWidth,
			sx_int32 iHeight,
			sx_int32 iSamplingRate,
			sx_int32 iChannels);

	track_type_e GetType() {return m_eType;}
	enumCodecId	GetCodecId() {return m_eCodecId;}

    // get track duration in milli-second
    sx_int32 GetDuration();
    // get track duration in timestamp
    sx_int64 GetTSDuration();
    // get number of frames (for video)
    sx_int32 GetNumFrames();
    // get number of key frames (for video)
    sx_int32 GetNumKeyFrames();
    // get number of samples (for audio)
    inline sx_int32 GetNumSamples() {return m_uiNumSamples;}
    // get video width/height
    inline sx_int32 GetWidth() {return m_iWidth;}
    inline sx_int32 GetHeight() {return m_iHeight;}
    // enable/disable track
    inline void Enable() {m_bEnabled = true;}
    inline void Disable() {m_bEnabled = false;}
    inline bool IsEnabled() {return m_bEnabled;}
	// audio parameters
	inline sx_int32 GetNumChannels() {return m_iChannels;}
	inline sx_int32 GetSamplingRate() {return m_iSamplingRate;}

    // get play pos
    float GetPlayPos();
    // get played time
    sx_int32 GetPlayedTime();
    // get played frames
    sx_int32 GetPlayedFrames();

    // get current frame's timestamp
    sx_int64 GetCurrentTimestamp();

    // get next frame
    sdvr_player_err_e GetNextFrame(CInputStream *pStrm, CFrameBuffer *pFrame);

    // set seek flags
    void SetSeekFlags(sx_uint32 flags);
    // seek by timestamp
    sdvr_player_err_e SeekByTimestamp(sx_int64 ts);
    // seek by pos percentage
    sdvr_player_err_e SeekByPercentage(float fPercent);
    // seek by time in milli-second
    sdvr_player_err_e SeekByTime(sx_int32 iTime);
    // seek by frame number
    sdvr_player_err_e SeekByFrameNum(sx_int32 iFrameNum);

    // add a sample entry
    sdvr_player_err_e 
	AddSampleEntry(sx_int64 pos, sx_int64 dts, sx_uint32 size, 
				   sx_uint32 flags, sx_uint32 samples);
    // add a sps entry
    sdvr_player_err_e 
	AddSPSEntry(sx_int64 pos, sx_uint32 size);
    // add a pps entry
    sdvr_player_err_e 
	AddPPSEntry(sx_int64 pos, sx_uint32 size);
    // get number of config entries (for video)
    sx_int32 GetNumConfigEntries();

    // get/set ref clock rate
    float GetRefClockRate() {return m_fRefClockRate;}
    void SetRefClockRate(float fRefClockRate) {m_fRefClockRate = fRefClockRate;}
    void ResetCfgIdx() {m_iCurCfgIdx = -1;}

protected:
	sx_int32		m_iTrackId;			// track id
    track_type_e	m_eType;			// track type
	enumCodecId		m_eCodecId;			// codec id
    sx_uint32		m_uiFlags;          // flags
    sx_uint32		m_uiNumSamples;     // number of samples
    sx_uint32		m_uiCurSample;      // current sample index
    sx_int32		m_iTime;            // media time in milli-second
    sx_int32		m_iWidth;           // video width
    sx_int32		m_iHeight;          // video height
    sx_int32		m_iSamplingRate;    // sampling rate (audio)
    bool			m_bEnabled;         // if is enabled
	FrameArray		m_FrameIndex;       // frame indexes array
    ConfigArray		m_ConfigIndex;      // config indexes array
    Uint32Array		m_KeyIndex;         // key frames array
    float			m_fRefClockRate;    // ref clock in kHz
    sx_int32		m_iCurCfgIdx;       // current config index
	sx_int32		m_iChannels;		// number of channels (audio)
};

#endif //#ifndef _TRACK_H_
