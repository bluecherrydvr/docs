#ifndef  _ISIL_WIN_NET_HEADER_H
#define _ISIL_WIN_NET_HEADER_H


#ifdef __cplusplus
extern "C" {
#endif



#include "isil_data_stream.h"
#include "isil_net_config.h"


#define	ISIL_FRAME_HEAD_FLAG	0x71010000

typedef struct _tagNET_PACKET_HEAD sNet_packet_head;

typedef struct _tagISIL_FRAME_HEAD sISIL_FRAME_HEAD;

typedef struct _tagISIL_NAL_HEAD comISIL_NAL_HEAD;

struct stInfo 
{
  unsigned char  cFlag;          
  unsigned char  cFrameType;     
}__attribute__((__packed__));


struct stFlag 
{
#if defined(__LITTLE_ENDIAN_BITFIELD)
  unsigned short nType:1;          
  short          nErrFlag:15;      
#else
  short          nErrFlag:15;      
  unsigned short nType:1;           
#endif
}__attribute__((__packed__));

struct _tagISIL_FRAME_HEAD
{
	unsigned long	lFrameFlag;			
	unsigned short	nFrameType;			
	unsigned short  nHeadSize;    
    unsigned long	lFrameSize;	    
    unsigned long   lTimeStamp;			
	unsigned long   lEncodeType;
	unsigned long   lReserve;

};

struct _tagNET_PACKET_HEAD  
{
	unsigned long  lFrameID;       
	unsigned long  lFrameSize;     
	unsigned short nDataLen;      
    unsigned short nPacketCount;   
    unsigned short nPacketNum;    

    union
	{
		struct stInfo xInfo;
        struct stFlag xFlag;
        
    }__attribute__((__packed__));

}__attribute__((__packed__));



struct _tagISIL_NAL_HEAD
{

#if defined(__LITTLE_ENDIAN_BITFIELD)
	unsigned long cNalType:8;    
	unsigned long lNalSize:24;

#else

	unsigned long lNalSize:24;
	unsigned long cNalType:8;    
#endif

};


enum _eISIL_FRAME_TYPE
{
	ISIL_FRAME_TYPE_A      = 0x01,  //audio
	ISIL_FRAME_TYPE_PIC    = 0x11,  //mjpeg
	ISIL_FRAME_TYPE_MAIN_I = 0x21,
	ISIL_FRAME_TYPE_MAIN_P = 0x22,
	ISIL_FRAME_TYPE_MAIN_B = 0x23,
	ISIL_FRAME_TYPE_SUB_I  = 0x31,  //sub video
	ISIL_FRAME_TYPE_SUB_P  = 0x32,
	ISIL_FRAME_TYPE_SUB_B  = 0x33,
    ISIL_FRAME_TYPE_USER_A = 0x50,  //
	ISIL_FRAME_TYPE_USER_B = 0x60,
};


enum  _eISIL_AUDIO_ENCODE_TYPE
{
	ISIL_AENCODE_NONE   = 0x00,
	ISIL_AENCODE_PCM    = 0xC0,
	ISIL_AENCODE_ALAW   = 0xC1,
	ISIL_AENCODE_ULAW   = 0xC2,
	ISIL_AENCODE_ADPCM  = 0xC3,
	ISIL_AENCODE_IMAADPCM  = 0xC4,
};


enum  _eISIL_VIDEO_ENCODE_TYPE
{
	ISIL_VENCODE_NONE   = 0x00,
	ISIL_VENCODE_H264   = 0x01,
};


enum eISIL_NAL_TYPE
{
	ISIL_NAL_UNKNOWN      = 0,
	ISIL_NAL_MV           = 0x50,
	ISIL_NAL_MJPEG        = 0x80,
	ISIL_NAL_BMP          = 0x81,
	ISIL_NAL_SPS          = 0xA0,
	ISIL_NAL_PPS          = 0xA1,
	ISIL_NAL_SLICE        = 0xB0,
	ISIL_NAL_SLICE_IDR    = 0xB1,
	ISIL_NAL_SLICE_DPA    = 0xB2,
	ISIL_NAL_SLICE_DPB    = 0xB3,
	ISIL_NAL_SLICE_DPC    = 0xB4,
	ISIL_NAL_SLICE_P      = 0xB5,
	ISIL_NAL_SLICE_I      = 0xB6,
	ISIL_NAL_AUDIO_PCM    = 0xC0,
	ISIL_NAL_AUDIO_ALAW   = 0xC1,
	ISIL_NAL_AUDIO_ULAW   = 0xC2,
	ISIL_NAL_AUDIO_ADPCM  = 0xC3,
	ISIL_NAL_AUDIO_IMAADPCM  = 0xC4,
};



enum  _eISIL_PIC_ENCODE_TYPE
{
	ISIL_PENCODE_NONE   = 0x00,
	ISIL_PENCODE_MJPEG  = 0x80,
	ISIL_PENCODE_BMP    = 0x81,
};


enum FRAME_STREAM_E{
    REALSTREAM = 0,
    FILESTREAM = 1,
};


enum{
    FRAMEDATA = 0,
    FILEDATA = 1,
};




#define IOVMAX   8
#define NALMAXUNIT (3)
#define NETVALIADSIZE  (1440)





extern void net_swap_net_packet_head(sNet_packet_head *net_packet_head);

extern void net_swap_isil_frame_head(sISIL_FRAME_HEAD *isil_frame_head);

extern void net_swap_isil_nal_head( comISIL_NAL_HEAD * isil_nal_head);


extern int net_handle_5864_IDR_frame(int fd ,
                             void* net_addr,
                             ISIL_AV_PACKET *av_header,
                             int realorpb,
                             short nErrFlag,
                             void *sps_buff,
                             int sps_len,
                             void *pps_buff,
                             int pps_len);

extern int net_handle_5864_audio_frame(int fd ,
										void* net_addr,
										ISIL_AV_PACKET *av_header,
										int realorpb,
										short nErrFlag );

extern int net_handle_5864_MJPEG_frame(int fd ,
                                void* net_addr,
                                ISIL_AV_PACKET *av_header);



extern int net_handle_5864_I_P_frame(int fd ,
									void* net_addr,
									ISIL_AV_PACKET *av_header,
									 int realorpb,
									short nErrFlag );




#ifdef __cplusplus
}
#endif //__cplusplus

#endif


