
#ifndef __GOCHIPCONFIG_H__
#define __GOCHIPCONFIG_H__


typedef struct
{
	char	name[64];			// configuration name
	char	desc[256];			// configuration description

	int		tv_standard;		
	int		framerate;			// will devided by 1001, for example 30000 represents 29.97
	int		sensor_h;			// sensor valid pixels in horizontal
	int		sensor_v;			// sensor valid pixels in vertical
	char	format;				// 0 : YUV progressive, 1: YUV interlace, 2 : RGB Bayer
	char	pformat;			// 'MultiMedia.h': EPixelMode, valid only if sensor_format si RGB bayer

	char	sensor_656_mode;	// 1: input format is 656 mode
	char	valid_enable;
	char	valid_polar;
	char	href_polar;			// h reference polar mode, only valid in non-656 mode
	char	vref_polar;			// v reference polar mode
	char	field_id_polar;
	char	sensor_bit_width;	// 0: 8bits, 1: 10bits
	char	hv_resync_enable;	

	int	reserved;

} TCFGSYSTEM;

typedef struct
{
	char	name[64];			// configuration name
	char	desc[256];			// configuration description

	char	compress_mode;		// 'MultiMedia.h': EVideoFormat
	char	sequence;			// 'MultiMedia.h': ESequenceMode

	unsigned char   gop_mode;			// 0: open GOP, 1: closed GOP
	unsigned char   deinterlace_mode;	// 0: use one field, 1: deinterlace, 2: interlace coding
	unsigned char	search_range;		// motion search range, should be 16,32,64 or 128
	unsigned char   gop_head_enable;	// 0: diable, 1: enable
	unsigned char   seq_head_enable;	// 0: diable, 1: enable
	unsigned char   av_sync_enable;		// 0: diable, 1: enable
	unsigned char   iip_enable;			// 0: diable, 1: enable
	unsigned char   vbi_enable;			// 0: diable, 1: enable
	unsigned char   aspect_ratio;		// 1= 1:1, 2= 4:3, 3= 16:9
	unsigned char   DVD_compliant;		// 0: diable, 1: enable
	unsigned char   ivtc_enable;	
	unsigned char   four_channel_enable;		// 0: diable, 1: enable

	unsigned char   h_filter_mode;		// 0: don't do filter, 1: median, 2: low pass       
	unsigned char   v_filter_mode;
	char   filter_nAX;			// weight, only valid in mode 2
	char   filter_nBX;
	char   filter_nCX;
	char   filter_nAY;
	char   filter_nBY;
	char   filter_nCY;

	unsigned int FourCC;
	int	reserved;

} TCFGSTREAM;

typedef struct
{
	char	name[64];			// configuration name
	char	desc[256];			// configuration description

	int	width;				// output stream resolution: horizontal size
	int	height;				// output stream resolution: vertical size

	unsigned char   h_sub_window;
	unsigned char   v_sub_window;
	unsigned int  h_sub_offset;
	unsigned int  v_sub_offset;

	unsigned char   h_scale_enb;
	unsigned char   v_scale_enb;

	unsigned char   sub_sample;

	int	reserved;

} TCFGRESOLUTION;

///**	-------------------------------------------------------------------------------------------

typedef struct
{
	char	name[64];			// configuration name
	char	desc[256];			// configuration description

	int	target_bitrate;		// target bitrate (bps), a non-zero value enables WIS adaptive bitrate control, 0 means no bit rate control
	int	peak_bitrate;		
	int  vbv_buffer;		
	int	drop_frame;			// 0 : keep original, 1: 1/2 original, 2: 1/3 original ..
	unsigned char   converge_speed;		// [0, 100],
	unsigned char	lambda;				// [0, 100], from best picture quality to largest framerate

	int	GOPSize;			// maximum count of pictures in a group of picture

	unsigned int  Q;					// initial quantizer, will devided by 4, for exmaple, 7 represents 1.75
	unsigned char	IQ;					// fixed quantize scale of I frames, only valid for target_rate==0 and Q==0
	unsigned char	PQ;					// fixed quantize scale of P frames
	unsigned char	BQ;					// fixed quantize scale of B frames
	
	int	reserved;

} TCFGBRCTRL;

typedef struct
{
	TCFGSTREAM		strcfg;		// configuration for stream information
	TCFGRESOLUTION	rescfg;		// configuration for resolution information
	TCFGBRCTRL		ctlcfg;		// configuration for bitrate control
} TVIDEOCFG;

typedef struct
{
	TCFGSYSTEM		syscfg;		// configuration for system setting
	TCFGSTREAM		strcfg;		// configuration for stream information
	TCFGRESOLUTION	rescfg;		// configuration for resolution information
	TCFGBRCTRL		ctlcfg;		// configuration for bitrate control
} TVIDEOCFGEX;

/////////////////////////////////////////////////////////////////
// sensor capabilities
/////////////////////////////////////////////////////////////////

enum SENSOR_CAPABILITIES
{
	CAP_SENSOR_VIDEO_SOURCE							= 0x00000001,

    CAP_SENSOR_VIDEO_BRIGHTNESS						= 0x00000004,   
    CAP_SENSOR_VIDEO_BRIGHTNESS_AUTO				= 0x00000008,

    CAP_SENSOR_VIDEO_CONTRAST						= 0x00000010,
    CAP_SENSOR_VIDEO_CONTRAST_AUTO					= 0x00000020,
    
	CAP_SENSOR_VIDEO_HUE							= 0x00000040,
	CAP_SENSOR_VIDEO_HUE_AUTO						= 0x00000080,

    CAP_SENSOR_VIDEO_SATURATION						= 0x00000100,
    CAP_SENSOR_VIDEO_SATURATION_AUTO				= 0x00000200,

    CAP_SENSOR_VIDEO_SHARPNESS						= 0x00000400,
    CAP_SENSOR_VIDEO_SHARPNESS_AUTO					= 0x00000800,

    CAP_SENSOR_VIDEO_GAMMA							= 0x00001000,
    CAP_SENSOR_VIDEO_GAMMA_AUTO						= 0x00002000,

    CAP_SENSOR_VIDEO_WHITEBALANCE					= 0x00004000,
    CAP_SENSOR_VIDEO_WHITEBALANCE_AUTO				= 0x00008000,

    CAP_SENSOR_VIDEO_BACKLIGHT_COMPENSATION			= 0x00010000,
    CAP_SENSOR_VIDEO_BACKLIGHT_COMPENSATION_AUTO	= 0x00020000,

    CAP_SENSOR_VIDEO_COLORENABLE					= 0x00040000,
};


/////////////////////////////////////////////////////////////////
// audio capabilities
/////////////////////////////////////////////////////////////////

enum AUDIO_CAPABILITIES
{
	CAP_AUDIO_FORMAT_PCM		= 0x00000001,
	CAP_AUDIO_FORMAT_ADPCM_MS	= 0x00000002,
	CAP_AUDIO_FORMAT_ADPCM_IMA	= 0x00000004,
	CAP_AUDIO_FORMAT_ALAW		= 0x00000008,
	CAP_AUDIO_FORMAT_ULAW		= 0x00000010,

	CAP_AUDIO_SAMPLERATE_8K		= 0x00000100,
	CAP_AUDIO_SAMPLERATE_11025	= 0x00000200,
	CAP_AUDIO_SAMPLERATE_16K	= 0x00000400,
	CAP_AUDIO_SAMPLERATE_22050	= 0x00000800,
	CAP_AUDIO_SAMPLERATE_32K	= 0x00001000,
	CAP_AUDIO_SAMPLERATE_44100	= 0x00002000,
	CAP_AUDIO_SAMPLERATE_48K	= 0x00004000,

	CAP_AUDIO_CHANNEL_MONO		= 0x00010000,
	CAP_AUDIO_CHANNEL_STEREO	= 0x00020000,

	CAP_AUDIO_SAMPLE_8BIT		= 0x00040000,
	CAP_AUDIO_SAMPLE_16BIT		= 0x00080000,
};

enum AUDIO_FORMAT
{
	AUDIO_FORMAT_PCM,
	AUDIO_FORMAT_ADPCM_MS,
	AUDIO_FORMAT_ADPCM_IMA,
	AUDIO_FORMAT_ALAW,
	AUDIO_FORMAT_ULAW,
};

typedef struct _AUDIO_CONFIG
{
	int Format;
	int SampleRate;
	int channels;
	int SampleBits;
} AUDIO_CONFIG;

//// statistic information from chip

typedef struct _STATISTIC
{
	UINT32  VideoByte;		// total video bytes from starting capture
	UINT32  FrameCount;		// total video frames from starting capture
} STATISTIC;

///////////////////////////////////////////////////////////
// because the interface id should be the same of the class
// id for some reason only kernel streaming guys know.
// and if I put the same iid and clsid, I can not make midl
// compiler to be quiet about it. so I had to make the 
// IWisVideoControl as a dummy interface and put real stuff
// in the IGOChipConfig interface.
MIDL_INTERFACE("5C28C78C-6187-4A32-9BEC-C9698E82B3F2")
IGOChipConfig : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetVideoConfig(TVIDEOCFGEX* pConfig) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetVideoConfig(TVIDEOCFGEX* pConfig) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetAudioConfig(AUDIO_CONFIG* pConfig) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetAudioConfig(AUDIO_CONFIG* pConfig) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetVideoSource(int VideoSource) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetVideoSource(int *pVideoSource) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetAudioCapability(unsigned int* pAudioCap) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetSensorCapability(unsigned int* pSensorCap) = 0;
    
	virtual HRESULT STDMETHODCALLTYPE GetStatisticInfo(STATISTIC *pStatistic) = 0;

	virtual HRESULT STDMETHODCALLTYPE Test(LONG *pTestValue)=0;

};

#endif