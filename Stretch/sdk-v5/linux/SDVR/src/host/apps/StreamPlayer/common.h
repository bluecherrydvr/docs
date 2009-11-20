// common.h
#ifndef _COMMON_H_
#define _COMMON_H_

#include "sdvr_player.h"

#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MIN(a, b) ((a) <= (b) ? (a) : (b))

// flag macros
#define CHK_FLAGS(flag, mask)           ((flag) & (mask))
#define SET_FLAGS(flag, bits)           ((flag) |= (bits))
#define CLR_FLAGS(flag, bits)           ((flag) &= (~(bits)))

// support 64-bit file size
typedef sx_int64    offset_t;

//-----------------------------------------------------------------------------
// Miscellaneous helper functions
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

// codec id
typedef enum {
    CODEC_ID_NONE = 0,
    // video codecs
    CODEC_ID_MPEG1VIDEO,
    CODEC_ID_MPEG2VIDEO,
    CODEC_ID_H261,
    CODEC_ID_H263,
    CODEC_ID_JPEG,
    CODEC_ID_MJPEG,
    CODEC_ID_MPEG4,
    CODEC_ID_RAWVIDEO,
    CODEC_ID_WMV1,
    CODEC_ID_WMV2,
    CODEC_ID_H263P,
    CODEC_ID_H263I,
    CODEC_ID_H264,
    CODEC_ID_VC1,
    CODEC_ID_WMV3,
    CODEC_ID_JPEG2000,
    CODEC_ID_YV12_VIDEO,
    // audio codecs 
    CODEC_ID_PCM_S16LE,
    CODEC_ID_PCM_S16BE,
    CODEC_ID_PCM_U16LE,
    CODEC_ID_PCM_U16BE,
    CODEC_ID_PCM_S8,
    CODEC_ID_PCM_U8,
    CODEC_ID_PCM_MULAW,
    CODEC_ID_PCM_ALAW,
    CODEC_ID_ADPCM_IMA_QT,
    CODEC_ID_ADPCM_MS,
    CODEC_ID_ADPCM_G723,
    CODEC_ID_ADPCM_G726,
} enumCodecId;

#endif //#ifndef _COMMON_H_
