/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Sat Oct 11 13:56:39 2003
 */
/* Compiler settings for C:\wis-src\depot\top\sw\WSDF\WISDevelopSpace\WISDriver\AVStream\WisProxy\WisProxy.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __WisProxy_h__
#define __WisProxy_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IGOChip_FWD_DEFINED__
#define __IGOChip_FWD_DEFINED__
typedef interface IGOChip IGOChip;
#endif 	/* __IGOChip_FWD_DEFINED__ */


#ifndef __IGOChipConfig_FWD_DEFINED__
#define __IGOChipConfig_FWD_DEFINED__
typedef interface IGOChipConfig IGOChipConfig;
#endif 	/* __IGOChipConfig_FWD_DEFINED__ */


#ifndef __AudioControlPropertyPage_FWD_DEFINED__
#define __AudioControlPropertyPage_FWD_DEFINED__

#ifdef __cplusplus
typedef class AudioControlPropertyPage AudioControlPropertyPage;
#else
typedef struct AudioControlPropertyPage AudioControlPropertyPage;
#endif /* __cplusplus */

#endif 	/* __AudioControlPropertyPage_FWD_DEFINED__ */


#ifndef __VideoControlPropertyPage_FWD_DEFINED__
#define __VideoControlPropertyPage_FWD_DEFINED__

#ifdef __cplusplus
typedef class VideoControlPropertyPage VideoControlPropertyPage;
#else
typedef struct VideoControlPropertyPage VideoControlPropertyPage;
#endif /* __cplusplus */

#endif 	/* __VideoControlPropertyPage_FWD_DEFINED__ */


#ifndef __GOChip_FWD_DEFINED__
#define __GOChip_FWD_DEFINED__

#ifdef __cplusplus
typedef class GOChip GOChip;
#else
typedef struct GOChip GOChip;
#endif /* __cplusplus */

#endif 	/* __GOChip_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_WisProxy_0000 */
/* [local] */ 

typedef /* [public] */ 
enum __MIDL___MIDL_itf_WisProxy_0000_0001
    {	MMSF_RAW	= 0,
	MMSF_AVI_WIS	= 1,
	MMSF_AVI_DIVX	= 2,
	MMSF_AVI_SIGMA	= 3,
	MMSF_AVI_MSFT	= 4,
	MMSF_MPEG1	= 5,
	MMSF_MPEG2	= 6,
	MMSF_WMV	= 7,
	MMSF_WMA	= 8,
	MMSF_MP2	= 9,
	MMSF_MP3	= 10,
	MMSF_NONE	= -1
    }	EMMStrFormat;

typedef /* [public] */ 
enum __MIDL___MIDL_itf_WisProxy_0000_0002
    {	AUDIO_MP1	= 0x1,
	AUDIO_MP2	= 0x2,
	AUDIO_MP3	= 0x3,
	VOICE_G7231	= 0x14
    }	EAudioFormat;

typedef /* [public][public][public][public][public][public][public][public][public][public] */ 
enum __MIDL___MIDL_itf_WisProxy_0000_0003
    {	MPEG1	= 0,
	MPEG2	= 0x1,
	H261	= 0x2,
	H263	= 0x3,
	MPEG4	= 0x4,
	MPEG4XGO	= 0x5,
	MPEG2X4	= 0x6,
	MOTIONJPEG	= 0x8,
	DV	= 0x9,
	H26L	= 0x20,
	GO	= 0x40
    }	EVideoFormat;

typedef /* [public] */ 
enum __MIDL___MIDL_itf_WisProxy_0000_0004
    {	UYVYP	= 0,
	UYVYI	= 1,
	BAYER_GB	= 2,
	BAYER_GR	= 3,
	BAYER_BG	= 4,
	BAYER_RG	= 5
    }	EPixelMode;

typedef /* [public] */ 
enum __MIDL___MIDL_itf_WisProxy_0000_0005
    {	DDRAW_YUY2	= 0x1,
	DDRAW_UYVY	= 0x2,
	DDRAW_YV12	= 0x8,
	DDRAW_RGB24	= 0x3,
	DDRAW_RGB32	= 0x4,
	DDRAW_RGB555	= 0x5,
	DDRAW_RGB565	= 0x6,
	DDRAW_DIB24	= 0x13,
	DDRAW_DIB32	= 0x14,
	DDRAW_DIB555	= 0x15,
	DDRAW_DIB565	= 0x16,
	DDRAW_NULL	= 0
    }	EDDrawMode;

typedef /* [public][public][public][public][public][public][public][public][public][public] */ 
enum __MIDL___MIDL_itf_WisProxy_0000_0006
    {	IONLY	= 1,
	IPONLY	= 2,
	IPB	= 3,
	IPBDROP	= 4
    }	ESequenceMode;

typedef /* [public] */ 
enum __MIDL___MIDL_itf_WisProxy_0000_0007
    {	I_FRAME	= 0,
	P_FRAME	= 1,
	B_FRAME	= 2,
	D_FRAME	= 3
    }	EFrameType;

typedef /* [public] */ 
enum __MIDL___MIDL_itf_WisProxy_0000_0008
    {	MB_INTRA	= 0x1,
	MB_PATTERN	= 0x2,
	MB_BACKWARD	= 0x4,
	MB_FORWARD	= 0x8,
	MB_QUANT	= 0x10,
	MB_4V	= 0x20,
	MB_DIRECT	= 0x40
    }	EMBType;

typedef /* [public] */ 
enum __MIDL___MIDL_itf_WisProxy_0000_0009
    {	PIXEL_XI_YI	= 0,
	PIXEL_XH_YI	= 0x1,
	PIXEL_XI_YH	= 0x10,
	PIXEL_XH_YH	= 0x11
    }	EFractionPixel;

typedef /* [public] */ 
enum __MIDL___MIDL_itf_WisProxy_0000_0010
    {	FLHALF	= 1,
	FLQUATER	= 2,
	FLEIGHTH	= 3,
	FLINTEGER	= 0
    }	EFractionLevel;

typedef /* [public] */ 
enum __MIDL___MIDL_itf_WisProxy_0000_0011
    {	NOSEQUENCE_HEADER	= 0x1,
	NOGOP_HEADER	= 0x2,
	NOFRAME_HEADER	= 0x4,
	NOFRAME_TAIL	= 0x8,
	NOSEQUENCE_TAIL	= 0x10,
	NOIFRAME	= 0x20,
	NOPFRAME	= 0x40,
	NOBFRAME	= 0x80
    }	ETranscodeOption;

typedef /* [public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0012
    {
    unsigned char interlace;
    unsigned char mode;
    unsigned char seq;
    int cols;
    int rows;
    double fps;
    unsigned char uvmode;
    unsigned char dqmode;
    unsigned char fpmode;
    unsigned char wismode;
    unsigned char acpred;
    unsigned char userq;
    unsigned char intraq[ 64 ];
    unsigned char interq[ 64 ];
    }	TMP_StrInfo;

typedef /* [public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0013
    {
    double timeStamp;
    int nGOP;
    int nSubGOP;
    int nPicture;
    double fno;
    unsigned char ftype;
    double fq;
    }	TMP_FrmInfo;

typedef /* [public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0014
    {
    int col;
    int row;
    int dc[ 6 ];
    int vx[ 6 ];
    int vy[ 6 ];
    unsigned char mbtype;
    unsigned char Q;
    unsigned char cbp;
    int ach[ 6 ][ 7 ];
    int acv[ 6 ][ 7 ];
    }	TMP_BlkInfo;

typedef /* [public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0015
    {
    unsigned char run[ 6 ][ 64 ];
    int lvl[ 6 ][ 64 ];
    int pair[ 6 ];
    }	TMP_BlkPair;

typedef /* [public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0016
    {
    unsigned char pproc;
    int rcLeft;
    int rcTop;
    int rcWidth;
    int rcHeight;
    }	TMP_PPrcSet;

typedef /* [public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0017
    {
    int SBYT;
    int EBYT;
    unsigned char SBIT;
    unsigned char EBIT;
    unsigned char Q;
    unsigned char I;
    short GOBN;
    short MBA;
    short hmvp00;
    short vmvp00;
    short hmvp10;
    short vmvp10;
    }	TMP_StrMBInfo;

typedef /* [public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0018
    {
    unsigned char name[ 64 ];
    unsigned char desc[ 256 ];
    unsigned long flags;
    unsigned long size;
    }	TCFG_HEADER;

typedef /* [public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public] */ 
enum __MIDL___MIDL_itf_WisProxy_0000_0019
    {	TVStandard_None	= 0,
	TVStandard_NTSC_M	= 0x1,
	TVStandard_NTSC_M_J	= 0x2,
	TVStandard_NTSC_433	= 0x4,
	TVStandard_PAL_B	= 0x10,
	TVStandard_PAL_D	= 0x20,
	TVStandard_PAL_G	= 0x40,
	TVStandard_PAL_H	= 0x80,
	TVStandard_PAL_I	= 0x100,
	TVStandard_PAL_M	= 0x200,
	TVStandard_PAL_N	= 0x400,
	TVStandard_PAL_60	= 0x800,
	TVStandard_SECAM_B	= 0x1000,
	TVStandard_SECAM_D	= 0x2000,
	TVStandard_SECAM_G	= 0x4000,
	TVStandard_SECAM_H	= 0x8000,
	TVStandard_SECAM_K	= 0x10000,
	TVStandard_SECAM_K1	= 0x20000,
	TVStandard_SECAM_L	= 0x40000,
	TVStandard_SECAM_L1	= 0x80000
    }	TV_STANDARD;

typedef /* [public][public][public][public][public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0020
    {
    TCFG_HEADER header;
    TV_STANDARD tv_standard;
    long framerate;
    long sensor_h;
    long sensor_v;
    unsigned char format;
    unsigned char pformat;
    unsigned char sensor_656_mode;
    unsigned char valid_enable;
    unsigned char valid_polar;
    unsigned char href_polar;
    unsigned char vref_polar;
    unsigned char field_id_polar;
    unsigned char sensor_bit_width;
    unsigned char hv_resync_enable;
    long reserved;
    }	TCFGSYSTEM;

typedef /* [public][public][public][public][public][public][public][public][public] */ 
enum __MIDL___MIDL_itf_WisProxy_0000_0021
    {	GO7007SB_MIDIAN	= 1,
	GO7007SB_LOWPASS	= 2,
	GO7007SB_NOFILTER	= 0
    }	FilterMode;


enum MPEG4_MODE
    {	WIS_MPEG4	= 0,
	DIVX_MPEG4	= 1,
	MICROSOFT_MPEG4	= 2,
	XVID_MPEG4	= 3
    };
typedef /* [public][public][public][public][public][public][public][public][public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0022
    {
    TCFG_HEADER header;
    EVideoFormat compress_mode;
    ESequenceMode sequence;
    unsigned char gop_mode;
    unsigned long gop_size;
    unsigned char mpeg4_mode;
    unsigned char DVD_compliant;
    unsigned char deinterlace_mode;
    unsigned char search_range;
    unsigned char gop_head_enable;
    unsigned char seq_head_enable;
    unsigned char aspect_ratio;
    long reserved;
    }	TCFGSTREAM;


enum FLAGS_STREAM
    {	FLAGS_STREAM_COMPRESS_MODE	= 0x1,
	FLAGS_STREAM_SEQUENCE_MODE	= 0x2,
	FLAGS_STREAM_GOP_MODE	= 0x4,
	FLAGS_STREAM_GOP_SIZE	= 0x8,
	FLAGS_STREAM_MPEG4_MODE	= 0x10,
	FLAGS_STREAM_DEINTERLACE_MODE	= 0x20,
	FLAGS_STREAM_SEARCH_RANGE	= 0x40,
	FLAGS_STREAM_GOPHEAD_ENABLE	= 0x80,
	FLAGS_STREAM_SEQHEAD_ENABLE	= 0x100,
	FLAGS_STREAM_ASPECT_RATIO	= 0x200,
	FLAGS_STREAM_DVD_COMPLIANT	= 0x400,
	FLAGS_STREAM_MPEG4_MANDETORY	= FLAGS_STREAM_COMPRESS_MODE + FLAGS_STREAM_MPEG4_MODE
    };
typedef /* [public][public][public][public][public][public][public][public][public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0023
    {
    TCFG_HEADER header;
    TV_STANDARD tv_standard;
    unsigned long frame_rate;
    unsigned long drop_frame;
    unsigned char ivtc_enable;
    long reserved;
    }	TCFGFRAMERATE;


enum FLAGS_FRAMERATE
    {	FLAGS_FRAMERATE_FRAMERATE	= 0x1,
	FLAGS_FRAMERATE_IVTC_ENABLE	= 0x2,
	FLAGS_FRAMERATE_DROP_FRAME	= 0x4,
	FLAGS_FRAMERATE_TVSTANDARD	= 0x8,
	FLAGS_FRAMERATE_MANDETORY	= FLAGS_FRAMERATE_FRAMERATE + FLAGS_FRAMERATE_TVSTANDARD
    };
typedef /* [public][public][public][public][public][public][public][public][public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0024
    {
    TCFG_HEADER header;
    TV_STANDARD tv_standard;
    unsigned long width;
    unsigned long height;
    unsigned char h_sub_window;
    unsigned char v_sub_window;
    unsigned long h_sub_offset;
    unsigned long v_sub_offset;
    unsigned char h_scale_enb;
    unsigned char v_scale_enb;
    unsigned char sub_sample;
    unsigned long max_bitrate;
    unsigned long min_bitrate;
    long reserved;
    }	TCFGRESOLUTION;


enum FLAGS_RESOLUTION
    {	FLAGS_RESOLUTION_WIDTH	= 0x1,
	FLAGS_RESOLUTION_HEIGHT	= 0x2,
	FLAGS_RESOLUTION_H_SUBWINDOW	= 0x4,
	FLAGS_RESOLUTION_V_SUBWINDOW	= 0x8,
	FLAGS_RESOLUTION_SCALE_OFFSET	= 0x10,
	FLAGS_RESOLUTION_SUBSAMPLE	= 0x100,
	FLAGS_RESOLUTION_TVSTANDARD	= 0x200,
	FLAGS_RESOLUTION_MAX_BITRATE	= 0x400,
	FLAGS_RESOLUTION_MIN_BITRATE	= 0x800,
	FLAGS_RESOLUTION_H_SUBOFFSET	= 0x1000,
	FLAGS_RESOLUTION_V_SUBOFFSET	= 0x2000,
	FLAGS_RESOLUTION_H_SCALE_ENABLE	= 0x4000,
	FLAGS_RESOLUTION_V_SCALE_ENABLE	= 0x8000,
	FLAGS_RESOLUTION_MANDETORY	= FLAGS_RESOLUTION_WIDTH + +FLAGS_RESOLUTION_HEIGHT + FLAGS_RESOLUTION_TVSTANDARD + FLAGS_RESOLUTION_MAX_BITRATE + FLAGS_RESOLUTION_MIN_BITRATE
    };
typedef /* [public][public][public][public][public][public][public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0025
    {
    TCFG_HEADER header;
    unsigned long target_bitrate;
    unsigned long peak_bitrate;
    unsigned long vbv_buffer;
    unsigned char converge_speed;
    unsigned char lambda;
    unsigned long Q;
    unsigned char IQ;
    unsigned char PQ;
    unsigned char BQ;
    long reserved;
    }	TCFGBRCTRL;


enum FLAGS_BITRATE
    {	FLAGS_BITRATE_TARGET	= 0x4,
	FLAGS_BITRATE_PEAK	= 0x8,
	FLAGS_BITRATE_VBV_BUFFER	= 0x10,
	FLAGS_BITRATE_CONVERGE_SPEED	= 0x20,
	FLAGS_BITRATE_LAMBDA	= 0x40,
	FLAGS_BITRATE_Q	= 0x80,
	FLAGS_BITRATE_IPBQ	= 0x100,
	FLAGS_BITRATE_IQ	= 0x200,
	FLAGS_BITRATE_PQ	= 0x400,
	FLAGS_BITRATE_BQ	= 0x800,
	FLAGS_BITRATE_MANDETORY	= FLAGS_BITRATE_TARGET + FLAGS_BITRATE_Q
    };
typedef /* [public][public][public][public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0026
    {
    TCFG_HEADER header;
    unsigned char av_sync_enable;
    unsigned char iip_enable;
    unsigned char vbi_enable;
    unsigned char four_channel_enable;
    FilterMode h_filter_mode;
    FilterMode v_filter_mode;
    unsigned char filter_nAX;
    unsigned char filter_nBX;
    unsigned char filter_nCX;
    unsigned char filter_nAY;
    unsigned char filter_nBY;
    unsigned char filter_nCY;
    long reserved;
    }	TCFGMISC;


enum FLAGS_MISC
    {	FLAGS_MISC_AV_SYNC_ENABLE	= 0x1,
	FLAGS_MISC_IIP_ENABLE	= 0x2,
	FLAGS_MISC_VBI_ENABLE	= 0x4,
	FLAGS_MISC_FOUR_CHANNEL_ENABLE	= 0x8,
	FLAGS_MISC_FILTER	= 0x10,
	FLAGS_MISC_MANDETORY	= 0
    };
typedef /* [public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0027
    {
    TCFGMISC misccfg;
    TCFGSTREAM strcfg;
    TCFGRESOLUTION rescfg;
    TCFGFRAMERATE fpscfg;
    TCFGBRCTRL ctlcfg;
    }	TCFGVIDEO;

typedef /* [public][public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0028
    {
    TCFGSYSTEM syscfg;
    TCFGMISC misccfg;
    TCFGSTREAM strcfg;
    TCFGRESOLUTION rescfg;
    TCFGFRAMERATE fpscfg;
    TCFGBRCTRL ctlcfg;
    }	TCFGVIDEOEX;

typedef /* [public][public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0029
    {
    TCFGSTREAM _stream;
    TCFGFRAMERATE _framerate;
    TCFGRESOLUTION _resolution;
    TCFGBRCTRL _bitrate;
    }	TCFG_FORMAT_EXTENSION;

typedef struct  _SENSOR_PARAMETERS
    {
    int Brightness;
    int BrightnessAuto;
    int Contrast;
    int ContrastAuto;
    int Hue;
    int HueAuto;
    int Saturation;
    int SaturationAuto;
    int Sharpness;
    int SharpnessAuto;
    int Gamma;
    int GammaAuto;
    int WhiteBalance;
    int WhiteBalanceAuto;
    int BacklightComp;
    int BacklightCompAuto;
    int ColorEnable;
    int VideoSource;
    }	SENSOR_PARAMETERS;


enum ASSOCIATION_TYPE
    {	TYPE_SYSTEM_CONFIG	= 0,
	TYPE_STREAM_CONFIG	= TYPE_SYSTEM_CONFIG + 1,
	TYPE_RESOLUTION_CONFIG	= TYPE_STREAM_CONFIG + 1,
	TYPE_BITRATE_CONFIG	= TYPE_RESOLUTION_CONFIG + 1,
	TYPE_FRAMERATE_CONFIG	= TYPE_BITRATE_CONFIG + 1
    };

enum ASSOCIATION_INDEX
    {	ASSOCIATION_ALL	= 0xffffffff
    };

enum ASSOCIATION_PERMISSION
    {	ASSOCIATION_TYPE_ALLOW	= 0,
	ASSOCIATION_TYPE_DENY	= 1
    };
typedef /* [public][public][public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0030
    {
    enum ASSOCIATION_TYPE _master_type;
    unsigned long _master_id;
    enum ASSOCIATION_TYPE _slave_type;
    unsigned long _slave_id;
    unsigned char _associate_type;
    }	TCFGASSOCIATION;

typedef /* [public][public][public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0031
    {
    unsigned long stream_index;
    unsigned long resolution_index;
    unsigned long framerate_index;
    }	TVCFG_ENTRY;

typedef /* [public][public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0032
    {
    unsigned long _num_of_system_configs;
    TCFGSYSTEM _system_configs[ 16 ];
    unsigned long _num_of_stream_configs;
    TCFGSTREAM _stream_configs[ 30 ];
    unsigned long _num_of_resolution_configs;
    TCFGRESOLUTION _resolution_configs[ 30 ];
    unsigned long _num_of_framerate_configs;
    TCFGFRAMERATE _framerate_configs[ 30 ];
    unsigned long _num_of_associations;
    TCFGASSOCIATION _associations[ 300 ];
    unsigned long _num_of_configurations;
    TVCFG_ENTRY __RPC_FAR *_configurations;
    }	_VIDEO_CAPABILITIES;

typedef /* [public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0033
    {
    unsigned long _num_of_stream_profiles;
    TCFGSTREAM _stream_profiles[ 30 ];
    unsigned long _stream_profile_cap_index[ 30 ];
    unsigned long _num_of_resolution_profiles;
    TCFGRESOLUTION _resolution_profiles[ 30 ];
    unsigned long _resolution_profile_cap_index[ 30 ];
    unsigned long _num_of_bitrate_profiles;
    TCFGBRCTRL _bitrate_profiles[ 30 ];
    unsigned long _bitrate_profile_cap_index[ 30 ];
    unsigned long _num_of_framerate_profiles;
    TCFGFRAMERATE _framerate_profiles[ 30 ];
    unsigned long _framerate_profile_cap_index[ 30 ];
    }	_VIDEO_PROFILES;

typedef /* [public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0034
    {
    unsigned long TotalDelay;
    unsigned long ChipDelay;
    unsigned long FrameInBuf;
    unsigned long VIPFrameNum;
    unsigned long LastChipTick;
    }	VIDEO_TIME_INFO;

typedef /* [public][public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0035
    {
    unsigned __int64 VideoByte;
    unsigned long FrameCount;
    }	STATISTIC;


enum AUDIO_CAPS
    {	CAP_AUDIO_FORMAT_PCM	= 0x1,
	CAP_AUDIO_FORMAT_ADPCM_MS	= 0x2,
	CAP_AUDIO_FORMAT_ADPCM_IMA	= 0x4,
	CAP_AUDIO_FORMAT_ALAW	= 0x8,
	CAP_AUDIO_FORMAT_ULAW	= 0x10,
	CAP_AUDIO_FORMAT_MP3	= 0x20,
	CAP_AUDIO_SAMPLERATE_8K	= 0x100,
	CAP_AUDIO_SAMPLERATE_11025	= 0x200,
	CAP_AUDIO_SAMPLERATE_16K	= 0x400,
	CAP_AUDIO_SAMPLERATE_22050	= 0x800,
	CAP_AUDIO_SAMPLERATE_32K	= 0x1000,
	CAP_AUDIO_SAMPLERATE_44100	= 0x2000,
	CAP_AUDIO_SAMPLERATE_48K	= 0x4000,
	CAP_AUDIO_CHANNEL_MONO	= 0x10000,
	CAP_AUDIO_CHANNEL_STEREO	= 0x20000,
	CAP_AUDIO_SAMPLE_8BIT	= 0x40000,
	CAP_AUDIO_SAMPLE_16BIT	= 0x80000
    };

enum AUDIO_FORMAT
    {	AUDIO_FORMAT_PCM	= 1,
	AUDIO_FORMAT_ADPCM_MS	= 2,
	AUDIO_FORMAT_ADPCM_IMA	= AUDIO_FORMAT_ADPCM_MS + 1,
	AUDIO_FORMAT_ALAW	= AUDIO_FORMAT_ADPCM_IMA + 1,
	AUDIO_FORMAT_ULAW	= AUDIO_FORMAT_ALAW + 1,
	AUDIO_FORMAT_MP3	= 0x55
    };
typedef struct  _AUDIO_CONFIG
    {
    unsigned long Format;
    unsigned long SampleRate;
    unsigned long Channels;
    unsigned long SampleBits;
    unsigned short BlockAlign;
    unsigned long AvgBytesPerSec;
    unsigned short SamplesPerBlock;
    unsigned short ExtSize;
    }	AUDIO_CONFIG;

typedef /* [public][public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0036
    {
    unsigned long _format_index;
    unsigned long _samplerate_index;
    unsigned long _samplebits_index;
    unsigned long _channel_index;
    unsigned long _extension_size;
    void __RPC_FAR *_extension;
    }	TACFG_ENTRY;

typedef /* [public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0037
    {
    unsigned long _num_of_format_config;
    unsigned long _audio_format_table[ 16 ];
    unsigned long _num_of_samplerate_config;
    unsigned long _audio_samplerate_table[ 16 ];
    unsigned long _num_of_samplebits_config;
    unsigned long _audio_samplebits_table[ 16 ];
    unsigned long _num_of_channel_config;
    unsigned long _audio_channel_table[ 16 ];
    unsigned long _num_of_audio_configuration;
    TACFG_ENTRY _audio_configuration[ 200 ];
    }	_AUDIO_CAPABILITIES;

typedef /* [public] */ struct  __MIDL___MIDL_itf_WisProxy_0000_0038
    {
    int DriverMajor;
    int DriverMinor;
    int BoardRevision;
    unsigned char BoardName[ 64 ];
    int MaxBandWidth;
    }	REVISION_INFO;



extern RPC_IF_HANDLE __MIDL_itf_WisProxy_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_WisProxy_0000_v0_0_s_ifspec;

#ifndef __IGOChip_INTERFACE_DEFINED__
#define __IGOChip_INTERFACE_DEFINED__

/* interface IGOChip */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IGOChip;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E4557E5C-BB64-4186-905E-8FC119DBA5E2")
    IGOChip : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetVideoConfig( 
            TCFG_FORMAT_EXTENSION __RPC_FAR *pConfig,
            unsigned int __RPC_FAR *pError) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGOChipVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGOChip __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGOChip __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGOChip __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetVideoConfig )( 
            IGOChip __RPC_FAR * This,
            TCFG_FORMAT_EXTENSION __RPC_FAR *pConfig,
            unsigned int __RPC_FAR *pError);
        
        END_INTERFACE
    } IGOChipVtbl;

    interface IGOChip
    {
        CONST_VTBL struct IGOChipVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGOChip_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGOChip_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGOChip_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGOChip_SetVideoConfig(This,pConfig,pError)	\
    (This)->lpVtbl -> SetVideoConfig(This,pConfig,pError)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGOChip_SetVideoConfig_Proxy( 
    IGOChip __RPC_FAR * This,
    TCFG_FORMAT_EXTENSION __RPC_FAR *pConfig,
    unsigned int __RPC_FAR *pError);


void __RPC_STUB IGOChip_SetVideoConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGOChip_INTERFACE_DEFINED__ */


#ifndef __IGOChipConfig_INTERFACE_DEFINED__
#define __IGOChipConfig_INTERFACE_DEFINED__

/* interface IGOChipConfig */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IGOChipConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("55F1E004-76CD-412e-9413-68AF75304D73")
    IGOChipConfig : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetVideoConfig( 
            /* [out][in] */ TCFGVIDEOEX __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetVideoSource( 
            /* [out][in] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetVideoSource( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSensorCapability( 
            /* [out][in] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetStatisticInfo( 
            /* [out][in] */ STATISTIC __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetVideoCapabilities( 
            _VIDEO_CAPABILITIES __RPC_FAR *pCaps) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetAudioConfig( 
            /* [out][in] */ AUDIO_CONFIG __RPC_FAR *pConfig) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetAudioConfig( 
            /* [in] */ AUDIO_CONFIG __RPC_FAR *pConfig) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetAudioCapability( 
            /* [out][in] */ unsigned int __RPC_FAR *pAudioCap) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGOChipConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGOChipConfig __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGOChipConfig __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGOChipConfig __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetVideoConfig )( 
            IGOChipConfig __RPC_FAR * This,
            /* [out][in] */ TCFGVIDEOEX __RPC_FAR *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetVideoSource )( 
            IGOChipConfig __RPC_FAR * This,
            /* [out][in] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetVideoSource )( 
            IGOChipConfig __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSensorCapability )( 
            IGOChipConfig __RPC_FAR * This,
            /* [out][in] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStatisticInfo )( 
            IGOChipConfig __RPC_FAR * This,
            /* [out][in] */ STATISTIC __RPC_FAR *pVal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetVideoCapabilities )( 
            IGOChipConfig __RPC_FAR * This,
            _VIDEO_CAPABILITIES __RPC_FAR *pCaps);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAudioConfig )( 
            IGOChipConfig __RPC_FAR * This,
            /* [out][in] */ AUDIO_CONFIG __RPC_FAR *pConfig);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetAudioConfig )( 
            IGOChipConfig __RPC_FAR * This,
            /* [in] */ AUDIO_CONFIG __RPC_FAR *pConfig);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAudioCapability )( 
            IGOChipConfig __RPC_FAR * This,
            /* [out][in] */ unsigned int __RPC_FAR *pAudioCap);
        
        END_INTERFACE
    } IGOChipConfigVtbl;

    interface IGOChipConfig
    {
        CONST_VTBL struct IGOChipConfigVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGOChipConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGOChipConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGOChipConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGOChipConfig_GetVideoConfig(This,pVal)	\
    (This)->lpVtbl -> GetVideoConfig(This,pVal)

#define IGOChipConfig_GetVideoSource(This,pVal)	\
    (This)->lpVtbl -> GetVideoSource(This,pVal)

#define IGOChipConfig_SetVideoSource(This,newVal)	\
    (This)->lpVtbl -> SetVideoSource(This,newVal)

#define IGOChipConfig_GetSensorCapability(This,pVal)	\
    (This)->lpVtbl -> GetSensorCapability(This,pVal)

#define IGOChipConfig_GetStatisticInfo(This,pVal)	\
    (This)->lpVtbl -> GetStatisticInfo(This,pVal)

#define IGOChipConfig_GetVideoCapabilities(This,pCaps)	\
    (This)->lpVtbl -> GetVideoCapabilities(This,pCaps)

#define IGOChipConfig_GetAudioConfig(This,pConfig)	\
    (This)->lpVtbl -> GetAudioConfig(This,pConfig)

#define IGOChipConfig_SetAudioConfig(This,pConfig)	\
    (This)->lpVtbl -> SetAudioConfig(This,pConfig)

#define IGOChipConfig_GetAudioCapability(This,pAudioCap)	\
    (This)->lpVtbl -> GetAudioCapability(This,pAudioCap)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGOChipConfig_GetVideoConfig_Proxy( 
    IGOChipConfig __RPC_FAR * This,
    /* [out][in] */ TCFGVIDEOEX __RPC_FAR *pVal);


void __RPC_STUB IGOChipConfig_GetVideoConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGOChipConfig_GetVideoSource_Proxy( 
    IGOChipConfig __RPC_FAR * This,
    /* [out][in] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IGOChipConfig_GetVideoSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGOChipConfig_SetVideoSource_Proxy( 
    IGOChipConfig __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IGOChipConfig_SetVideoSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGOChipConfig_GetSensorCapability_Proxy( 
    IGOChipConfig __RPC_FAR * This,
    /* [out][in] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IGOChipConfig_GetSensorCapability_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGOChipConfig_GetStatisticInfo_Proxy( 
    IGOChipConfig __RPC_FAR * This,
    /* [out][in] */ STATISTIC __RPC_FAR *pVal);


void __RPC_STUB IGOChipConfig_GetStatisticInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGOChipConfig_GetVideoCapabilities_Proxy( 
    IGOChipConfig __RPC_FAR * This,
    _VIDEO_CAPABILITIES __RPC_FAR *pCaps);


void __RPC_STUB IGOChipConfig_GetVideoCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGOChipConfig_GetAudioConfig_Proxy( 
    IGOChipConfig __RPC_FAR * This,
    /* [out][in] */ AUDIO_CONFIG __RPC_FAR *pConfig);


void __RPC_STUB IGOChipConfig_GetAudioConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGOChipConfig_SetAudioConfig_Proxy( 
    IGOChipConfig __RPC_FAR * This,
    /* [in] */ AUDIO_CONFIG __RPC_FAR *pConfig);


void __RPC_STUB IGOChipConfig_SetAudioConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGOChipConfig_GetAudioCapability_Proxy( 
    IGOChipConfig __RPC_FAR * This,
    /* [out][in] */ unsigned int __RPC_FAR *pAudioCap);


void __RPC_STUB IGOChipConfig_GetAudioCapability_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGOChipConfig_INTERFACE_DEFINED__ */



#ifndef __WISPROXYLib_LIBRARY_DEFINED__
#define __WISPROXYLib_LIBRARY_DEFINED__

/* library WISPROXYLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_WISPROXYLib;

EXTERN_C const CLSID CLSID_AudioControlPropertyPage;

#ifdef __cplusplus

class DECLSPEC_UUID("8ED37ED7-477B-4764-B72E-DFC2D1899C6C")
AudioControlPropertyPage;
#endif

EXTERN_C const CLSID CLSID_VideoControlPropertyPage;

#ifdef __cplusplus

class DECLSPEC_UUID("35D3656A-6C20-46B0-B44A-DC8E861F1205")
VideoControlPropertyPage;
#endif

EXTERN_C const CLSID CLSID_GOChip;

#ifdef __cplusplus

class DECLSPEC_UUID("E65DDCB1-652E-4843-958F-95099EEDEC8D")
GOChip;
#endif
#endif /* __WISPROXYLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
