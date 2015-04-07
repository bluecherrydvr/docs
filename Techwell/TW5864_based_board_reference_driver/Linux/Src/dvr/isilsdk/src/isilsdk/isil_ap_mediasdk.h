#ifndef _ISIL_AP_MEDIASDK_H_
#define _ISIL_AP_MEDIASDK_H_

#include "isil_venc_param.h"
#include "ISIL_SDKError_Def.h"
#include "isil_data_stream.h"
#include "isil_codec_chip_ctl.h"
#include "isil_interface.h"
#include "isil_sdk_center_mgt.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _eISIL_STREAM_TYPE
{
	ISIL_STREAM_TYPE_NONE   = 0,
	ISIL_STREAM_TYPE_VIDEO  = 1,
	ISIL_STREAM_TYPE_AUDIO  = 2,
	ISIL_STREAM_TYPE_AVSYNC = 3,

}ISIL_STREAM_TYPE_E;

#if 0
typedef enum _eISIL_VIDEO_STANDARD
{
	/*ISIL_VIDEO_STANDARD_NONE   = 0,
	ISIL_VIDEO_STANDARD_NTSC   = 1,
	ISIL_VIDEO_STANDARD_PAL    = 2,
	ISIL_VIDEO_STANDARD_SECAM  = 4
	*/
	ISILMSDK_VIDEO_STANDARD_PAL,
	ISILMSDK_VIDEO_STANDARD_NTSC,
	ISILMSDK_VIDEO_STANDARD_USER_DEFINE,

}ISIL_VIDEO_STANDARD;
#endif

#if 1
typedef enum _eISIL_VIDEO_SIZE_TYPE
{
	/*
	ISILMSDK_VIDEO_SIZE_QQCIF,
	ISILMSDK_VIDEO_SIZE_QCIF,
	ISILMSDK_VIDEO_SIZE_HALF_CIF,
	ISILMSDK_VIDEO_SIZE_CIF,
	ISILMSDK_VIDEO_SIZE_2CIF,
	ISILMSDK_VIDEO_SIZE_HALF_D1,
	ISILMSDK_VIDEO_SIZE_4CIF,
	ISILMSDK_VIDEO_SIZE_D1,
	*/
	ISIL_VIDEO_SIZE_QCIF_E = 0,
    ISIL_VIDEO_SIZE_QHALF_D1_E,
    ISIL_VIDEO_SIZE_CIF_E,
    ISIL_VIDEO_SIZE_HALF_D1_E,
    ISIL_VIDEO_SIZE_D1_E,
    ISIL_VIDEO_SIZE_HCIF_E,
    ISIL_VIDEO_SIZE_2CIF_E,
    ISIL_VIDEO_SIZE_4CIF_E,
    ISIL_VIDEO_SIZE_VGA_E,
    ISIL_VIDEO_SIZE_SVGA_E,
    ISIL_VIDEO_SIZE_XGA_E,
    ISIL_VIDEO_SIZE_720P_E,
    ISIL_VIDEO_SIZE_1080P_E,
    ISIL_VIDEO_SIZE_USER_E,

}ISIL_VIDEO_SIZE_TYPE;
#endif

#if 0
typedef enum _eISIL_AUDIO_TYPE
{
    /*
	ISIL_AUDIO_PCM,
	ISIL_AUDIO_ALAW,
	ISIL_AUDIO_ULAW,
	ISIL_AUDIO_ADPCM,
	*/

	ISIL_AUDIO_PCM_E = 0,
	ISIL_AUDIO_ALAW_E,
	ISIL_AUDIO_ULAW_E,
	ISIL_AUDIO_ADPCM_32K_E,
	ISIL_AUDIO_ADPCM_16K_E,
	ISIL_AUDIO_ADPCM_48K_E,


}ISIL_MEDIASDK_AUDIO_TYPE;
#endif

typedef enum _eISIL_STREAM_BITRATE_TYPE
{
    #if 0
	ISIL_STREAM_BITRATE_TYPE_CBR = 0,     //
	ISIL_STREAM_BITRATE_TYPE_VBR = 1,     //
	ISIL_STREAM_BITRATE_TYPE_CQP = 2,     // fix QP
	#endif
    ISIL_STREAM_BITRATE_H264_NO_RC,
	ISIL_STREAM_BITRATE_H264_CBR,
	ISIL_STREAM_BITRATE_H264_VBR,

}ISIL_STREAM_BITRATE_TYPE;


struct __tagQP                //when CQP£¬good to bad[0,51]
{
		   unsigned char cIQP;       //I frame QP[0,51]
		   unsigned char cPQP;       //P frame QP[0,51]
		   unsigned char cBQP;       //B frame QP[0,51]
		   unsigned char cReserve;
}tQP;


typedef struct _tagISIL_BITRATE_VALUE
{
	union
	{
	   unsigned long lImageQuality;  //when VBR£¬from bad to good[0,5]
	   struct __tagQP tQP;
	};


}ISIL_BITRATE_VALUE;


typedef enum _eISIL_STREAM_BITRATE_MODE
{
    #if 0
    ISIL_STREAM_BITRATE_MODE_DEFAULT = 0,
	ISIL_STREAM_BITRATE_MODE_QUALITY = 1,  //quality
	ISIL_STREAM_BITRATE_MODE_FLUX    = 2   //flux
	#endif
    ISIL_STREAM_BITRATE_RC_IMAGE_QUALITY_FIRST = 0,
    ISIL_STREAM_BITRATE_RC_IMAGE_SMOOTH_FIRST,

}ISIL_STREAM_BITRATE_MODE;

enum eISIL_OSD_ATTRIBUTE
{
	ISIL_OSD_ATTRIBTE_GB_16_16 = 0, //font size 16*16
	ISIL_OSD_ATTRIBTE_GB_24_24,
	ISIL_OSD_ATTRIBTE_GB_32_32,
	ISIL_OSD_ATTRIBTE_GB_12_12,
	ISIL_OSD_ATTRIBTE_YUV420,       //
	ISIL_OSD_ATTRIBTE_FORE_COLOR,   //
	ISIL_OSD_ATTRIBTE_BACK_COLOR    //
};


struct stVideo
		{
			unsigned long  lColorSpace;     //video type 0:YV12,1:YUY2,2:RGB32
			unsigned short nVideoWidth;		//video width
			unsigned short nVideoHeight;	//video height
		};
		
		
struct stAudio
		{
			unsigned long  lAudioSamples;	//sample
			unsigned char  cAudioChannels;	//(i.e. 1:mono, 2:stereo, etc.)
			unsigned char  cAudioBits;		//bit	 (8,16)
			unsigned short nFormatTag;      //audio type i.e. WAVE_FORMAT_PCM
		};	

typedef struct _tagISIL_PREVIEW_DATA
{
	unsigned long lDataType;       //0:video,1:audio
	unsigned long lTimeStamp;
	union
	{
		struct stVideo xVideo;
		struct stAudio xAudio;
	};
	unsigned long  lDataSize;
	unsigned char* pDataBuffer;
}ISIL_PREVIEW_DATA;


typedef enum _eISIL_MJPEG_DATA_TYPE
{
	ISIL_MJPEG_DATA_TYPE_DATA  = 0,    //0: pDataBuffer point Mjpeg data, eg. MJPEG_DATA
	ISIL_MJPEG_DATA_TYPE_FRAME = 1,    //1: pDataBuffer point ISIL_FRAME_HEAD
	                                  //   eg. ISIL_FRAME_HEAD + ISIL_NAL_HEAD + MJPEG_DATA

}ISIL_MJPEG_DATA_TYPE;

#if 0
typedef struct _tagISIL_MJPEG_DATA
{
	unsigned long  lDataType;       //0: pDataBuffer point Mjpeg data, eg. MJPEG_DATA
	                                //1: pDataBuffer point ISIL_FRAME_HEAD
	                                //   eg. ISIL_FRAME_HEAD + ISIL_NAL_HEAD + MJPEG_DATA
	unsigned long  lTimeStamp;
	unsigned long  lDataSize;
	unsigned char* pDataBuffer;

}ISIL_MJPEG_DATA;

#endif

typedef struct _tagISIL_MVFLAG_DATA
{
	unsigned long lTimeStamp;
	unsigned char odddata[32];
	unsigned char evendata[32];

}ISIL_MVFLAG_DATA;


typedef struct _tagISIL_MV_INFO
{
	int nWidth;
	int nHeight;
	int nFrameNum;
	unsigned long  lTimeStamp;
	unsigned long  lMVBufSize;
	void* pMVBuffer;

}ISIL_MV_INFO;


enum _eISIL_DATA_TYPE
{
	ISIL_DATA_TYPE_A      = 0x01,  //audio£¬
	ISIL_DATA_TYPE_MAIN_I = 0x21,  //main video
	ISIL_DATA_TYPE_MAIN_P = 0x22,
	ISIL_DATA_TYPE_MAIN_B = 0x23,
	ISIL_DATA_TYPE_MAIN_SPS = 0x24,
	ISIL_DATA_TYPE_MAIN_PPS = 0x25,
	ISIL_DATA_TYPE_SUB_I  = 0x31,  //sub video
	ISIL_DATA_TYPE_SUB_P  = 0x32,
	ISIL_DATA_TYPE_SUB_B  = 0x33,
	ISIL_DATA_TYPE_SUB_SPS  = 0x34,
	ISIL_DATA_TYPE_SUB_PPS  = 0x35,
};

/*
 *cDataType:I,P,B,SPS,PPS and audio
 *when cDataType==ISIL_DATA_TYPE_A£¬cEncodeType: 0xC0--PCM,0xC4--IMAADPCM
 *when cDataType== other£¬          cEncodeType: 0x01--H264
 */
typedef struct _tagISIL_ENCODE_DATA
{
	unsigned char  cDataType;        //_eISIL_DATA_TYPE
	unsigned char  cEncodeType;
	unsigned long  lTimeStamp;
	unsigned long  lBufSize;
	void          *pBuffer;

}ISIL_ENCODE_DATA;

typedef struct _tagISIL_ACCESS_GPIO_ITEM
{
	unsigned int nIOnumber;    //IO number [0,7]
	unsigned int nReadOrWrite; //0:read,1:write
	unsigned int nValue;       //value[0,1]

}ISIL_ACCESS_GPIO_ITEM;

typedef struct _tagISIL_ACCESS_GPIO
{
	unsigned int          nCount;       //max 8
	ISIL_ACCESS_GPIO_ITEM GPIOParam[8];

}ISIL_ACCESS_GPIO;


enum ISIL_CARD_PREVIEW_PARAM_CHANGED_MASK
{
	ISILCARD_PREVIEW_PARAM_ENABLE_CHANGED       = 0x01,
	ISILCARD_PREVIEW_PARAM_PREVIEW_SIZE_CHANGED = 0x02,
	ISILCARD_PREVIEW_PARAM_STREAM_TYPE_CHANGED  = 0x04,
	ISILCARD_PREVIEW_PARAM_FPS_CHANGED          = 0x08,
};

struct ISIL_CARD_PREVIEW_PARAM
{
	//HANDLE hChannel;
	int    changedMask;
	int    enable;
	unsigned int videoSize;
	unsigned int streamType;
	unsigned int fps;
};

enum ISILCARD_ENCODE_PARAM_CHANGED_MASK
{
	ISILCARD_ENCODE_PARAM_ENABLE_CHANGED = 0x01,
	ISILCARD_ENCODE_PARAM_ENCODE_SIZE_CHANGED = 0x02,
	ISILCARD_ENCODE_PARAM_BITRATE_CHANGED = 0x04,
	ISILCARD_ENCODE_PARAM_QPI_CHANGED = 0x08,
	ISILCARD_ENCODE_PARAM_QPP_CHANGED = 0x10,
	ISILCARD_ENCODE_PARAM_QPB_CHANGED = 0x20,
	ISILCARD_ENCODE_PARAM_KEYFRAME_INTERVALS_CHANGED = 0x40,
	ISILCARD_ENCODE_PARAM_SPS_PPS_INTERVALS_CHANGED = 0x80,
	ISILCARD_ENCODE_PARAM_FPS_CHANGED = 0x100,
	ISILCARD_ENCODE_PARAM_VBR_CHANGED = 0x200,
	ISILCARD_ENCODE_PARAM_STREAM_TYPE_CHANGED = 0x400,
	ISILCARD_ENCODE_PARAM_AUDIO_ENCODE_TYPE_CHANGED = 0x800,
	ISILCARD_ENCODE_PARAM_ENCODE_PMODE_CHANGED = 0x1000,
	ISILCARD_ENCODE_PARAM_ENCODE_HFLIP_CHANGED = 0x2000,
	ISILCARD_ENCODE_PARAM_ENCODE_VFLIP_CHANGED = 0x4000,

};

enum ISILCARD_ENCODE_PARAM_ENCODE_PMODE_CONST
{
	ISILCARD_ENCODE_PARAM_ENCODE_PMODE_SKIP = 0x01,
	ISILCARD_ENCODE_PARAM_ENCODE_PMODE_COMBO = 0x02,
};


struct ISIL_CARD_ENCODE_PARAM
{
	//HANDLE       hChannel;
	int          changedMask;
	int          enable;
	unsigned int encodeSize;
	int          btType;
	int          vbrquality;
	unsigned int bitrate;
	unsigned int qpi;
	unsigned int qpp;
	unsigned int qpb;
	unsigned int keyFrameIntervals;
	unsigned int SPS_PPS_Intervals;
	unsigned int fps;
	unsigned int streamType;
	unsigned int audioEncodeType;
	int          encodeMode;
	char         hflip;
	char         vflip;
};


typedef struct _tagISIL_CARD_LOCATION_INFO
{
	int nType;        //1:PCI,2:PCIe
	int nSlot;        //slot position
	int nBus;         //bus
	int nDevice;      //device
	int nFunction;    //function

	int nCardIndex;   //card, if get card location, invalid
	int nChannel;     //channel, if get card location, invalid


}ISIL_CARD_LOCATION_INFO;

typedef struct
{void *p;}COLORREF;

typedef struct
{void *p;}ISIL_MediaSDK_PREVIEW_PARAM;

typedef struct
{
    void *p;
}ISIL_MediaSDK_ENCODE_PARAM;

typedef struct
{
    //void *p;
   long					left;
   long					top;
   long					right;
   long					bottom;
}ISIL_MediaSDK_RECT;


typedef struct {
    unsigned int chipid;
    unsigned int chanid;
}CHIP_CHN_PHY_ID;



/*ChipSDK Base*/
extern void ISIL_MediaSDK_GetSdkVersion();


extern ISILERR_CODE  ISIL_MediaSDK_SetVideoStandard(unsigned int nChipIndex,
											 unsigned int nChnId,
											enum CHIP_VI_STANDARD eVideoStandard);

extern ISILERR_CODE  ISIL_MediaSDK_GetVideoStandard(unsigned int nChipIndex,
    enum ISIL_VIDEO_STANDARD *lpeVideoStandard);

/*ChipSDK MV*/
extern ISILERR_CODE  ISIL_MediaSDK_StartMVInfo(unsigned int nChipId, unsigned int nChannel);

extern ISILERR_CODE  ISIL_MediaSDK_StopMVInfo(unsigned int nChipId, unsigned int nChannel);

/*ChipSDK MV Flag*/
extern ISILERR_CODE  ISIL_MediaSDK_StartMVFlag(unsigned int nChipId, unsigned int nChannel);

extern ISILERR_CODE  ISIL_MediaSDK_StopMVFlag(unsigned int nChipId, unsigned int nChannel);

extern ISILERR_CODE  ISIL_MediaSDK_SetNoMVFlagScope(unsigned int nChipId, unsigned int nChannel,
    ISIL_MediaSDK_RECT *pRectList,int nRectSize);



/*ChipSDK MJPEG*/
extern ISILERR_CODE  ISIL_MediaSDK_StartMJpegCapture(unsigned int nChipId, unsigned int nChannel);

extern ISILERR_CODE  ISIL_MediaSDK_StopMJpegCapture(unsigned int nChipId, unsigned int nChannel);

extern ISILERR_CODE  ISIL_MediaSDK_SetMJpegFormat(unsigned int nChipId, unsigned int nChannel,
    ISIL_VIDEO_SIZE_TYPE ePicSize);

extern ISILERR_CODE  ISIL_MediaSDK_SetMJpegFrameRate(unsigned int nChipId, unsigned int nChannel,
    int nFps);

extern ISILERR_CODE  ISIL_MediaSDK_SetMJpegStreamType(unsigned int nChipId, unsigned int nChannel,
    ISIL_STREAM_TYPE_E eType);

extern ISILERR_CODE  ISIL_MediaSDK_SetMJpegQP(unsigned int nChipId, unsigned int nChannel, int nQP);      //nQP: good to bad[10,51]

extern ISILERR_CODE  ISIL_MediaSDK_SetMJpegDataType(unsigned int nChipId, unsigned int nChannel,
    ISIL_MJPEG_DATA_TYPE eType);
extern ISILERR_CODE  ISIL_MediaSDK_SetMJpegCfgParm(unsigned int nChipId, unsigned int nChannel,
    VENC_MJPG_CFG* cfg);

/*ChipSDK encode*/

extern ISILERR_CODE  ISIL_MediaSDK_SetAudioType(unsigned int nChipId, unsigned int nChannel,
    AUDIO_TYPE_E eAudioType);
extern ISILERR_CODE  ISIL_MediaSDK_SetAudioCfgAll(unsigned int nChipId, unsigned int nChannel,
 		VENC_AUD_CFG* cfg);
extern ISILERR_CODE  ISIL_MediaSDK_SetAudioParm(unsigned int nChipId, unsigned int nChannel,
    unsigned int bit_wide, unsigned int sample_rate);

extern ISILERR_CODE  ISIL_MediaSDK_SetVideoFlip(unsigned int nChipId, unsigned int nChannel,
    unsigned int bHFlip,unsigned int bVFlip);

extern ISILERR_CODE  ISIL_MediaSDK_SetVideoPara(unsigned int nChipId, unsigned int nChannel,
    int  nBrightness, int  nContrast, int  nSaturation, int  nHue, int nSharpness);

extern ISILERR_CODE  ISIL_MediaSDK_GetVideoPara(unsigned int nChipId, unsigned int nChannel,
    int *lpBrightness,int *lpContrast,int *lpSaturation,int *lpHue,int *lpSharpness);

extern ISILERR_CODE ISIL_MediaSDK_ClacSetVd(unsigned int nChipIndex, VD_CONFIG_REALTIME* pRealtime);
extern ISILERR_CODE  ISIL_MediaSDK_BindVIVO(unsigned int nChipId, BIND_VI2VO *pVi2VO);
extern ISILERR_CODE  ISIL_MediaSDK_UnbindVIVO(unsigned int nChipId, BIND_VI2VO *pVi2VO);
/*
 *if eType==ISIL_STREAM_BITRATE_TYPE_CBR, tValue invalid£¬                        call ISIL_MediaSDK_SetupBitrateControl to set BPS
 *if eType==ISIL_STREAM_BITRATE_TYPE_VBR, tValue.lImageQuality£ºbad to good[0,5]£¬call ISIL_MediaSDK_SetupBitrateControl to set BPS
 *if eType==ISIL_STREAM_BITRATE_TYPE_CQP, tValue.tQP£º   cIQP,cPQP,cBQP [0,51]
 */
extern ISILERR_CODE  ISIL_MediaSDK_SetBitrateControlType(unsigned int nChipId, unsigned int nChannel,
ISIL_STREAM_BITRATE_TYPE eType, ISIL_BITRATE_VALUE tValue, unsigned int bSub);

extern ISILERR_CODE  ISIL_MediaSDK_SetBitrateControlMode(unsigned int nChipId, unsigned int nChannel,
    ISIL_STREAM_BITRATE_MODE eMode, unsigned int bSub);

extern ISILERR_CODE  ISIL_MediaSDK_SetupBitrateControl(unsigned int nChipId, unsigned int nChannel,
    unsigned long lMaxBps, unsigned int bSub);


extern ISILERR_CODE  ISIL_MediaSDK_ForceIFrameEncode(unsigned int nChipId, unsigned int nChannel,
    unsigned int bSub);

extern ISILERR_CODE  ISIL_MediaSDK_SetIBPMode(unsigned int nChipId, unsigned int nChannel,
    int nKeyFrameIntervals,int nBFrames,int nPFrames,
    int fFrameRate, unsigned int bSub);

extern ISILERR_CODE  ISIL_MediaSDK_SetEncoderVideoSize(unsigned int nChipId, unsigned int nChannel,
    ISIL_VIDEO_SIZE_TYPE eVideoSize, unsigned int bSub);

extern ISILERR_CODE  ISIL_MediaSDK_SetEncoderVideoSizeByWH(unsigned int nChipId, unsigned int nChannel,
    unsigned int w, unsigned int h,unsigned int bSub);

extern ISILERR_CODE  ISIL_MediaSDK_SetH264Feature(unsigned int nChipId, unsigned int nChannel,
     VENC_H264_FEATURE* cfg,  unsigned int bSub);

extern ISILERR_CODE  ISIL_MediaSDK_SetH264CfgAll(unsigned int nChipId, unsigned int nChannel,
 		VENC_H264_CFG* cfg, unsigned int bSub);
extern ISILERR_CODE  ISIL_MediaSDK_SetRcAll(unsigned int nChipId, unsigned int nChannel,
 		VENC_H264_RC* cfg, unsigned int bSub);

/*ChipSDK special*/
extern ISILERR_CODE  ISIL_MediaSDK_ReadReg(unsigned int nChipIndex, unsigned long lStartAddr,
     unsigned long *lpBuffer, unsigned long lCount);

extern ISILERR_CODE  ISIL_MediaSDK_WriteReg(unsigned int nChipIndex, unsigned long lStartAddr,
											unsigned long *lpBuffer, unsigned long lCount);

extern ISILERR_CODE  ISIL_MediaSDK_ReadMPB(unsigned int nChipIndex, unsigned long lStartAddr,
										   unsigned char *lpBuffer, unsigned long lCount);

extern ISILERR_CODE  ISIL_MediaSDK_WriteMPB(unsigned int nChipIndex, unsigned long lStartAddr,
											unsigned char *lpBuffer, unsigned long lCount);

extern ISILERR_CODE  ISIL_MediaSDK_GPIO(unsigned int nChipIndex, ISIL_ACCESS_GPIO *pGPIO);

extern ISILERR_CODE  ISIL_MediaSDK_ReadI2C(unsigned int nChipIndex, unsigned long lBusAddr ,
										   unsigned long lOffsetAddr , unsigned char *pValue, unsigned long lCount,unsigned char GPIOorIIC);

extern ISILERR_CODE  ISIL_MediaSDK_WriteI2C(unsigned int nChipIndex, unsigned long lBusAddr ,
											unsigned long lOffsetAddr , unsigned char *pValue, unsigned long lCount,unsigned char GPIOorIIC);

extern ISILERR_CODE  ISIL_MediaSDK_ReadDDR(unsigned int nChipIndex, unsigned long lStartAddr,
										   unsigned long *lpBuffer, unsigned long lCount);
extern ISILERR_CODE  ISIL_MediaSDK_WriteDDR(unsigned int nChipIndex, unsigned long lStartAddr,
											unsigned long *lpBuffer, unsigned long lCount);

extern ISILERR_CODE  ISIL_MediaSDK_SetPreviewMultiParam(ISIL_MediaSDK_PREVIEW_PARAM *previewParm,
														int dwCount, void *pVoid);

extern ISILERR_CODE  ISIL_MediaSDK_SetEncodeMultiParam(ISIL_MediaSDK_ENCODE_PARAM *encodeParm,
													   int dwCount);

extern ISILERR_CODE  ISIL_MediaSDK_SetSubEncodeMultiParam(ISIL_MediaSDK_ENCODE_PARAM* encodeParm,
														  int dwCount);

extern ISILERR_CODE ISIL_MediaSDK_ClacSetVd(unsigned int nChipIndex, VD_CONFIG_REALTIME* pRealtime);

extern ISILERR_CODE  ISIL_MediaSDK_SetOsdCfg(unsigned int nChipId, unsigned int nChannel,
											 ISIL_OSD_CFG* pOSDCfg, unsigned int bSub);
extern ISILERR_CODE ISIL_MediaSDK_SetChannelAlarmType(unsigned int nChipIndex, unsigned int ch, unsigned int alarmType);

extern ISILERR_CODE ISIL_MediaSDK_ClearChannelAlarmType(unsigned int nChipIndex, unsigned int ch, unsigned int alarmType);

extern int get_chip_chn_id_by_logic_id(unsigned int log_id , CHIP_CHN_PHY_ID *phy_id_ptr);

extern int get_logic_id_by_chip_chn_id(unsigned int *log_id , CHIP_CHN_PHY_ID *phy_id_ptr);

extern int dec_get_chip_chn_id_by_logic_id(unsigned int log_id , CHIP_CHN_PHY_ID *phy_id_ptr);

extern int dec_get_logic_id_by_chip_chn_id(unsigned int *log_id , CHIP_CHN_PHY_ID *phy_id_ptr);


#ifdef __cplusplus
}
#endif

#endif
