#ifndef	isil_interface_H
#define	isil_interface_H



#ifdef __cplusplus
extern "C"
{
#endif

#include "isil_codec_common.h"

#define ISIL_CHIP_IOC_MAGIC      'H'  /*ai,ao,vi,vo,chip control use it*/
#define ISIL_CODEC_IOC_MAGIC     'S'  /*only codec use it*/


/******************************************************************************
    chip control io ctl cmds
******************************************************************************/

#define ISIL_CHIP_VD_WORK_MODE_SET                        _IOW(ISIL_CHIP_IOC_MAGIC, 255, unsigned int)
#define ISIL_CHIP_VD_WORK_MODE_GET                   	  _IOR(ISIL_CHIP_IOC_MAGIC, 254, unsigned int)
#define ISIL_VIDEO_BUS_PARAM_SET                          _IOR(ISIL_CHIP_IOC_MAGIC, 253, unsigned int)
#define ISIL_VIDEO_BUS_PARAM_GET                          _IOR(ISIL_CHIP_IOC_MAGIC, 252, unsigned int)
#define ISIL_CHIP_VD_CONFIG_PARAM_SET                     _IOR(ISIL_CHIP_IOC_MAGIC, 251, unsigned int)
#define ISIL_CHIP_VD_CONFIG_PARAM_GET                     _IOR(ISIL_CHIP_IOC_MAGIC, 250, unsigned int)
#define	ISIL_CHIP_AUDIO_ENCODE_PARAM_SET	              _IOR(ISIL_CHIP_IOC_MAGIC, 249, unsigned int)
#define	ISIL_CHIP_AUDIO_ENCODE_PARAM_GET	              _IOW(ISIL_CHIP_IOC_MAGIC, 248, unsigned int)
#define	ISIL_CHIP_AUDIO_DECODE_PARAM_SET	              _IOR(ISIL_CHIP_IOC_MAGIC, 247, unsigned int)
#define	ISIL_CHIP_AUDIO_DECODE_PARAM_GET	              _IOW(ISIL_CHIP_IOC_MAGIC, 246, unsigned int)
#define ISIL_CHIP_CREATE_CHAN                             _IOW(ISIL_CHIP_IOC_MAGIC, 245, unsigned int)                            
#define ISIL_CHIP_RELEASE_CHAN                            _IOW(ISIL_CHIP_IOC_MAGIC, 244, unsigned int)
#define ISIL_CHIP_ID_GET                          	      _IOR(ISIL_CHIP_IOC_MAGIC, 243, unsigned int)

#define ISIL_CHIP_VD_MAP_PARAM_GET                _IOR(ISIL_CHIP_IOC_MAGIC,  215, unsigned int)
#define ISIL_CHIP_VD_MAP_PARAM_SET                _IOR(ISIL_CHIP_IOC_MAGIC,  216, unsigned int)
#define ISIL_CHIP_VP_MAP_PARAM_GET                _IOR(ISIL_CHIP_IOC_MAGIC,  217, unsigned int)
#define ISIL_CHIP_VP_MAP_PARAM_SET                _IOR(ISIL_CHIP_IOC_MAGIC,  218, unsigned int)



/*
 *  ........
 */

#define ISIL_CHIP_CTL_OP                                  _IOW(ISIL_CHIP_IOC_MAGIC,  200, int)
#define ISIL_CHIP_CONFIG_WRITE_REG                        _IOW(ISIL_CHIP_IOC_MAGIC,  201, int)
#define ISIL_CHIP_CONFIG_READ_REG                         _IOR(ISIL_CHIP_IOC_MAGIC,  202, int)


#define ISIL_VIDEO_BUS_PARAM_ENABLE_CHANGE_VIDEO_STANDARD_MASK        (0x00000001)

#define ISIL_CHIP_AUDIO_ENCODE_PARAM_ENABLE_CHANGE_BIT_WIDE_MASK      (0x00000001)
#define ISIL_CHIP_AUDIO_ENCODE_PARAM_ENABLE_CHANGE_SAMPLE_RATE_MASK   (0x00000002)
#define ISIL_CHIP_AUDIO_ENCODE_PARAM_ENABLE_CHANGE_ENCODE_TYPE_MASK   (0x00000004)

#define ISIL_CHIP_AUDIO_DECODE_PARAM_ENABLE_CHANGE_BIT_WIDE_MASK      (0x00000001)
#define ISIL_CHIP_AUDIO_DECODE_PARAM_ENABLE_CHANGE_SAMPLE_RATE_MASK   (0x00000002)
#define ISIL_CHIP_AUDIO_DECODE_PARAM_ENABLE_CHANGE_ENCODE_TYPE_MASK   (0x00000004)

/******************************************************************************
     codec io ctl cmds
******************************************************************************/
#define	ISIL_CODEC_GET_VIDEO_ENCODER_PARAM	_IOR(ISIL_CODEC_IOC_MAGIC, 0, unsigned int)
#define	ISIL_CODEC_SET_VIDEO_ENCODER_PARAM	_IOW(ISIL_CODEC_IOC_MAGIC, 1, unsigned int)

#define	ISIL_CODEC_SET_VIDEO_TIMESTAMP_BASE	_IOW(ISIL_CODEC_IOC_MAGIC, 2, unsigned int)
#define	ISIL_CODEC_GET_VIDEO_TIMESTAMP_BASE	_IOR(ISIL_CODEC_IOC_MAGIC, 3, unsigned int)

/*motion detection cmd*/
#define	ISIL_MOTION_DECTION_GET_PARAM		_IOR(ISIL_CODEC_IOC_MAGIC, 4, unsigned int)
#define	ISIL_MOTION_DECTION_SET_PARAM		_IOW(ISIL_CODEC_IOC_MAGIC, 5, unsigned int)

/*osd cmd*/
#define	ISIL_OSD_GET_PARAM		            _IOR(ISIL_CODEC_IOC_MAGIC, 6, unsigned int)
#define	ISIL_OSD_SET_PARAM			        _IOW(ISIL_CODEC_IOC_MAGIC, 7, unsigned int)
#define ISIL_H264_ENCODE_CHAN_MAP_GET         _IOR(ISIL_CODEC_IOC_MAGIC, 8, unsigned int)
#define ISIL_LOGIC_CHAN_ENABLE_SET            _IOW(ISIL_CODEC_IOC_MAGIC, 9, unsigned int)
#define ISIL_LOGIC_CHAN_DISABLE_SET           _IOW(ISIL_CODEC_IOC_MAGIC, 10, unsigned int)
#define ISIL_H264_ENCODE_FEATURE_SET          _IOW(ISIL_CODEC_IOC_MAGIC, 11, unsigned int)
#define ISIL_H264_ENCODE_FEATURE_GET          _IOR(ISIL_CODEC_IOC_MAGIC, 12, unsigned int)

#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_DEINTERLACE_MASK      0x00000001
#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_SKIP_MASK             0x00000002
#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_I_4X4_MASK            0x00000004
#define ISIL_H264_ENABLE_CHANGE_HALF_PIXEL_MASK                      0x00000008
#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_QUARTER_PIXEL_MASK    0x00000010
#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_MB_DELAY_MASK         0x00000020

#define ISIL_H264_FEATURE_ON      1
#define ISIL_H264_FEATURE_OFF     0

#define ISIL_H264_ENCODE_RC_SET                   _IOW(ISIL_CODEC_IOC_MAGIC, 13, unsigned int)
#define ISIL_H264_ENCODE_RC_GET                   _IOR(ISIL_CODEC_IOC_MAGIC, 14, unsigned int)
#define ISIL_H264_ENCODE_RC_PARAM_SET             _IOW(ISIL_CODEC_IOC_MAGIC, 15, unsigned int)
#define ISIL_H264_ENCODE_RC_PARAM_GET             _IOR(ISIL_CODEC_IOC_MAGIC, 16, unsigned int)

#define ISIL_H264_ENCODE_RC_ENABLE_CHANGE_RC_TYPE_MASK            0x00000001
#define ISIL_H264_ENCODE_RC_ENABLE_CHANGE_IMAGE_PRIORITY_MASK     0x00000002
#define ISIL_H264_ENCODE_RC_ENABLE_CHANGE_DEFAULT_QP_MASK         0x00000004

/*jpeg cmds*/
#define ISIL_JPEG_IOC_MAGIC      ISIL_CODEC_IOC_MAGIC
#define	ISIL_MJPEG_ENCODE_PARAM_GET       	_IOR(ISIL_JPEG_IOC_MAGIC, 17, unsigned int)
#define	ISIL_MJPEG_ENCODE_PARAM_SET	        _IOW(ISIL_JPEG_IOC_MAGIC, 18, unsigned int)

//...
#define ISIL_CODEC_CHAN_RELEASE_MEM               _IOW(ISIL_CODEC_IOC_MAGIC, 242, unsigned int)
#define ISIL_CODEC_CHAN_REQUEST_MEM               _IOW(ISIL_CODEC_IOC_MAGIC, 243, unsigned int)                            



#ifndef ISIL_CODEC_CHAN_DISCAED_FRAME
#define ISIL_CODEC_CHAN_DISCAED_FRAME		_IOW(ISIL_CODEC_IOC_MAGIC, 244, unsigned int)	
#endif

#define ISIL_CHIP_LOGIC_MAP_TABLE_SET      _IOW(ISIL_CHIP_IOC_MAGIC,  207, int)

#define ISIL_CHIP_LOGIC_MAP_TABLE_GET     _IOW(ISIL_CHIP_IOC_MAGIC,  208, int)


#define ISIL_CHIP_VD_CHNL_CONFIG_GET  _IOR(ISIL_CHIP_IOC_MAGIC,209, unsigned int)
#define ISIL_CHIP_VD_CHNL_CONFIG_SET  _IOR(ISIL_CHIP_IOC_MAGIC,210, unsigned int)


//decoding control
#define ISIL_CODEC_CHAN_FLUSH  _IOW(ISIL_CODEC_IOC_MAGIC, 150, unsigned int)	

#define ISIL_CODEC_CHAN_CHANG_MODE _IOW(ISIL_CODEC_IOC_MAGIC, 151, unsigned int)

#define ISIL_CODEC_CHAN_TS_RESYNC _IOW(ISIL_CODEC_IOC_MAGIC, 152, unsigned int)	

#define ISIL_CODEC_CHAN_SET_STRATEGY _IOW(ISIL_CODEC_IOC_MAGIC, 153, unsigned int)

#define ISIL_CODEC_CHAN_SET_TIME_CONFIG _IOW(ISIL_CODEC_IOC_MAGIC, 154, unsigned int)

#define ISIL_CHIP_START_AVSYNC	_IOW(ISIL_CHIP_IOC_MAGIC, 242, unsigned int)

#define ISIL_CHIP_STOP_AVSYNC	_IOW(ISIL_CHIP_IOC_MAGIC, 241, unsigned int)

#define ISIL_CHIP_BIND_H264D2VO _IOR(ISIL_CHIP_IOC_MAGIC,211, unsigned int)

#define ISIL_CHIP_UNBIND_H264D2VO  _IOR(ISIL_CHIP_IOC_MAGIC, 212, unsigned int)

#define ISIL_CHIP_BIND_VI2VO  _IOR(ISIL_CHIP_IOC_MAGIC,  213, unsigned int)

#define ISIL_CHIP_UNBIND_VI2VO _IOR(ISIL_CHIP_IOC_MAGIC,  214, unsigned int)

#define ISIL_CODEC_CHAN_READY_TO_DIE  _IOW(ISIL_CODEC_IOC_MAGIC, 155, unsigned int)


#define ISIL_CODEC_CHAN_SET_DEC_STREAM_TYPE  _IOW(ISIL_CODEC_IOC_MAGIC, 156, unsigned int)

#define	DATA_SECTION_ALIGN(x)	__attribute__ ((aligned(x)))
#define	STRUCT_PACKET_ALIGN(x)	__attribute__ ((packed))

#ifndef ISIL_CHIP_VIN_CHAN_NUMBER_GET
#define ISIL_CHIP_VIN_CHAN_NUMBER_GET   _IOR(ISIL_CHIP_IOC_MAGIC,  203, int)
#endif

#ifndef ISIL_CHIP_AIN_CHAN_NUMBER_GET
#define ISIL_CHIP_AIN_CHAN_NUMBER_GET    _IOR(ISIL_CHIP_IOC_MAGIC,  205, int)
#endif

/*******************************************************************************
				chip control used structs (to create channel)	
********************************************************************************/

typedef enum isil_service_type
{
    ISIL_CHIP = 0,
    ISIL_ENCODER,
    ISIL_DECODER,
    ISIL_VIDEO_IN,
    ISIL_VIDEO_OUT,
    ISIL_AUDIO_IN,
    ISIL_AUDIO_OUT,
    ISIL_SERVICE_TYPE_NOTHING = 0XFFFF,
}ISIL_SERVICE_TYPE;



struct device_info
{
	int major;
	int minor;
	char path[256];
};


struct cmd_arg
{
	ISIL_SERVICE_TYPE type;
	unsigned long	channel_idx;
	CODEC_TYPE	    algorithm;
	STREAM_TYPE	    stream;
	char 		    dev_info[0];     // this is use for exchange data beISILeen uerland and kernel land. 
};

/*new definition goes from here*/
#define	DATA_SECTION_ALIGN(x)	__attribute__ ((aligned(x)))
#define	STRUCT_PACKET_ALIGN(x)	__attribute__ ((packed))

typedef enum ISIL_STREAM_TYPE{
	ISIL_H264_STERAM = 0x10,
	ISIL_MVFLAG_STERAM,
	ISIL_MVD_STERAM,
	ISIL_MJPEG_STREAM,
	ISIL_IMAGE_STREAM,
	ISIL_AUDIO_STREAM,
	ISIL_INFO_STREAM,
}ISIL_STREAM_TYPE_E_U;

#define	ISIL_COMMON_STREAM_HEADER_HAVE_PADDING_MSG	0x80000000

/*all XX_len is include the structure itself*/
typedef struct isil_common_stream_header{
	unsigned int    stream_type;
	unsigned int    stream_timestamp;
	unsigned int    stream_len;
}STRUCT_PACKET_ALIGN(1) isil_common_stream_header_t;

typedef struct isil_transparent_msg_header{
	unsigned int	isil_transparent_msg_number;
	unsigned int	isil_transparent_msg_total_len;
}STRUCT_PACKET_ALIGN(1) isil_transparent_msg_header_t;



typedef enum ISIL_MSG_TYPE{
	ISIL_H264_ENCODE_FRAME_MSG = 0x00000010,
	ISIL_H264_ENCODE_FRAME_TYPE,

	ISIL_H264_DECODE_FRAME_MSG = 0x00010010,
	ISIL_H264_DECODE_BIT_INFO_MSG,

	ISIL_H264_MVFLAG_FRAME_MSG = 0x00020010,
	ISIL_H264_MVD_FRAME_MSG,

	ISIL_MJPEG_ENCODE_FRAME_MSG = 0x00030010,
	ISIL_MJPEG_ENCODE_PARAM_MSG = 0x00030011,

	ISIL_IMAGE_ENCODE_FRAME_MSG = 0x00040010,

	ISIL_AUDIO_ENCODE_FRAME_MSG = 0x00050010,
	ISIL_AUDIO_ENCODE_PARAM_MSG,
	ISIL_AUDIO_DECODE_FRAME_MSG,
	ISIL_AUDIO_DECODE_PARAM_MSG,

	ISIL_VIDEO_LOST_DETECTION_MSG = 0x00060010, //新增加的video_lost_detection_msg
	ISIL_VIDEO_NIGHT_DETECTION_MSG = 0x00060011,//新增加的video_night_detection_msg
	ISIL_VIDEO_BLIND_DETECTION_MSG = 0x00060012,//新增加的video_blind_detection_msg
	ISIL_VIDEO_STANDARD_CHANGE_DETECTION_MSG = 0x00060013,//新增加的video_standard_change_detection_msg

	ISIL_INFO_xxx_MSG = 0x00070010,        //for extend

}ISIL_MSG_TYPE_E;


/*this is the unified frame struct for video,audio,jpeg....*/
typedef struct isil_frame_msg{
        unsigned int	msg_type;       //the value comes from ISIL_MSG_TYPE_E(egs:ISIL_H264_ENCODE_FRAME_MSG)
        unsigned int	msg_len;        //it's scope is [start of isil_frame_msg_t,end of payload]
        unsigned int	frame_serial;
        unsigned int    frame_timestamp;
        unsigned int    frame_checksam; //it's scope is [start of payload,end of payload] 
        unsigned char	frame_payload[0];
}STRUCT_PACKET_ALIGN(1) isil_frame_msg_t;





/********************************************************************************/
/*all the msgs blew only appears when parameter changed/need further explanation*/
/********************************************************************************/
//h264 encode staff
/********************************************************************************/
typedef enum ISIL_H264_FRAME_TYPE{
    ISIL_SPS_FRAME,
    ISIL_PPS_FRAME,
    ISIL_IDR_FRAME,
    ISIL_I_FRAME,
    ISIL_P_FRAME,
}ISIL_H264_FRAME_TYPE_E;

typedef struct isil_h264_encode_frame_type_msg{
    unsigned int	msg_type;           //the value comes from ISIL_MSG_TYPE_E(eg:ISIL_H264_ENCODE_FRAME_TYPE)
    unsigned int	msg_len;            //the length of isil_h264_encode_frame_type_msg
    unsigned int	frame_type;         //the value comes from ISIL_H264_FRAME_TYPE(eg:ISIL_IDR_FRAME)
    unsigned short  i_mb_x;             //the mb count in wide
    unsigned short  i_mb_y;             //the mb count in height
}STRUCT_PACKET_ALIGN(1) isil_h264_encode_msg_frame_type_t;




//h264 decode staff
/********************************************************************************/
#define  H264_DECODE_BIT_INFO_IS_DISPLAY    0    //set to 1 if the frame should be displayed
#define  H264_DECODE_BIT_INFO_IS_END        1    //set to 1 if the frame is the last one in a set of frames
#define  H264_DECODE_BIT_INFO_IS_FILE_END   2    //set to 1 if the frame is end of file
#define  H264_DECODE_BIT_INFO_IS_GOP_END    3    //set to 1 if the frame is the last one in one GOP

#define  BI_IS_DISPLAY(x)   ((x) & ((1) << H264_DECODE_BIT_INFO_IS_DISPLAY))
#define  BI_IS_END(x)       ((x) & ((1) << H264_DECODE_BIT_INFO_IS_END))
#define  BI_IS_FILE_END(x)  ((x) & ((1) << H264_DECODE_BIT_INFO_IS_FILE_END))
#define  BI_IS_GOP_END(x)   ((x) & ((1) << H264_DECODE_BIT_INFO_IS_GOP_END))

typedef struct isil_h264_decode_bit_info_msg{
	unsigned int	msg_type;
	unsigned int	msg_len;
	unsigned int    bit_info;               
}STRUCT_PACKET_ALIGN(1) isil_h264_decode_bit_info_msg_t;




//jpeg staff
/********************************************************************************/
#define ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_TIMER                       (1 << 31)
#define ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_USER                        (1 << 30)


typedef struct isil_jpeg_param_msg{
        unsigned int	msg_type;
        unsigned int	msg_len;
        unsigned int	capture_type;
        unsigned char	i_mb_x;
        unsigned char	i_mb_y;
        unsigned short	reserve;
}STRUCT_PACKET_ALIGN(1) isil_jpeg_param_msg_t;

//audio staff
/********************************************************************************/
typedef struct isil_audio_param_msg{
        unsigned int	msg_type;
        unsigned int	msg_len;
        unsigned char	audio_type;
        unsigned char	audio_sample_rate;
        unsigned char	audio_bit_wide;
        unsigned char	reserve;
}STRUCT_PACKET_ALIGN(1) isil_audio_param_msg_t;

typedef struct isil_video_lost_detection_msg{
	unsigned int    msg_type;       //the value comes from ISIL_MSG_TYPE_E(egs:ISIL_VIDEO_LOST_DETECTION_MSG)
	unsigned int    msg_len;        //the length of isil_video_lost_detection_msg
	unsigned short  chan_id;               //gen video_lost_msg's chan in chip
	unsigned char   video_lost_valid;             //video signal lost
	unsigned char   video_contect_valid;     //after video lost, the video signal have been connected
}STRUCT_PACKET_ALIGN(1) isil_video_lost_detection_msg_t;

typedef struct isil_video_night_detection_msg{
	unsigned int     msg_type;       //the value comes from ISIL_MSG_TYPE_E(egs:ISIL_VIDEO_NIGHT_DETECTION_MSG)
	unsigned int     msg_len;        //the length of isil_video_night_detection_msg
	unsigned short   chan_id;        //gen video_night_msg's chan in chip
	unsigned char    video_night_valid_from_day_to_night;
	unsigned char    video_night_valid_from_night_to_day;
}STRUCT_PACKET_ALIGN(1) isil_video_night_detection_msg_t;

typedef struct isil_video_blind_detection_msg{
	unsigned int    msg_type;       //the value comes from ISIL_MSG_TYPE_E(egs:ISIL_VIDEO_BLIND_DETECTION_MSG)
	unsigned int    msg_len;        //the length of isil_video_blind_detection_msg
	unsigned short  chan_id;               //gen video_night_msg's chan in chip
	unsigned char   video_blind_add_valid;
	unsigned char   video_blind_remove_valid;
}STRUCT_PACKET_ALIGN(1) isil_video_blind_detection_msg_t;

typedef struct isil_video_standard_change_detection_msg{
	unsigned int     msg_type;       //the value comes from ISIL_MSG_TYPE_E(egs:ISIL_VIDEO_STANDARD_CHANGE_DETECTION_MSG)
	unsigned int     msg_len;        //the length of isil_video_standard_change_detection_msg
	unsigned short   chan_id;               //gen video_night_msg's chan in chip
	unsigned char    latest_video_standard;
	unsigned char    curr_video_standard;
}STRUCT_PACKET_ALIGN(1) isil_video_standard_change_detection_msg_t;



/********************************************************************************************/
//some helper functions
/********************************************************************************************/
static const unsigned short crc16_table[256] =
{
0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78,
};

static inline unsigned short compute_crc(unsigned char *data, unsigned int len)
{
        unsigned short  v = 0;
         
        while (len--)
                v = (v >> 8) ^ crc16_table[(v ^ *data++) & 0xff];

        return v;
} 


static inline unsigned short compute_accumulate_crc(unsigned short init_v, unsigned char *data, unsigned int len)
{
        unsigned short  v;

        v = (0 != init_v)? init_v: 0; 
        while (len--)
                v = (v >> 8) ^ crc16_table[(v ^ *data++) & 0xff];

        return v;
} 


/************************************************* 
 *  
 *      DECODE IOCTL CMD 
 * 
 *************************************************/


#define ISIL_CODEC_IOC_MAGIC     'S'  /*only codec use it*/

#define ISIL_DISPLAY_IOC_MAGIC            ISIL_CODEC_IOC_MAGIC
#define ISIL_DISPLAY_VIDEO_STANDARD_SET   _IOR(ISIL_DISPLAY_IOC_MAGIC, 220, unsigned int)
#define ISIL_DISPLAY_VIDEO_STANDARD_GET   _IOR(ISIL_DISPLAY_IOC_MAGIC, 221, unsigned int)

#define ISIL_DISPLAY_PORT_MODE_SET        _IOW(ISIL_DISPLAY_IOC_MAGIC, 222, unsigned int)
#define ISIL_DISPLAY_PORT_MODE_GET        _IOR(ISIL_DISPLAY_IOC_MAGIC, 223, unsigned int)

#define ISIL_DISPLAY_CHNL_PARAM_SET       _IOW(ISIL_DISPLAY_IOC_MAGIC, 224, unsigned int)
#define ISIL_DISPLAY_CHNL_PARAM_GET       _IOR(ISIL_DISPLAY_IOC_MAGIC, 225, unsigned int)

#define ISIL_DISPLAY_CHNL_CREATE          _IOR(ISIL_DISPLAY_IOC_MAGIC, 226, unsigned int)
#define ISIL_DISPLAY_CHNL_DESTROY         _IOR(ISIL_DISPLAY_IOC_MAGIC, 227, unsigned int)


enum ISIL_DISPLAY_CHANNLE_SOURCE{
     ISIL_DISPLAY_CHANNLE_SOURCE_VI = 0,
     ISIL_DISPLAY_CHANNLE_SOURCE_VDEC,
     ISIL_DISPLAY_CHANNLE_SOURCE_USER,
     ISIL_DISPLAY_CHANNLE_SOURCE_RESERVED,
};


struct ISIL_rect{
     unsigned int x;
     unsigned int y;
     unsigned int width;
     unsigned int height;
};



struct ISIL_display_chnl_param{
     unsigned int i_phy_slot_id;

     unsigned int i_bind_port;
     unsigned int i_bind_phase;

     enum ISIL_DISPLAY_CHANNLE_SOURCE e_video_source;
     struct ISIL_rect rect;
     unsigned int b_enable;
     unsigned int fps;
};

struct ISIL_display_chnl_property{
    unsigned int i_phy_slot_id;
    unsigned int i_bind_port;
    unsigned int i_bind_phase;
    struct ISIL_rect rect;
};




#define TC_NETLINK_GENERIC      17

enum tn_msg_type {
             TN_MSG_REQ_FRAMES = 0,
             TN_MSG_DISCARD_FRAME_REASON,
             TN_MSG_INVAILD = 0x10000,
};

struct tn_msg 
{ 
        unsigned int            rpid;           //who is messages's receiver
        unsigned int            spid;           //who send this messages 
        enum tn_msg_type  msg_type;  //message type
        unsigned int            msg_len;    //actually data len 
        char                         payload[0];// message data starts from here actually
};

struct frame_query
{
    signed int          f_invalid;                //0: start is invalid, 1:end is invalid
    unsigned int  start;
    //the start time stamp, 
    unsigned int  end;
    //the end time stamp
    unsigned int      key_frame; 
      //key frame or not, 0: one by one
    unsigned int  disp_timestamp;
    //feedback to GUI APP to update  progress bar
};

struct  query_info
{
    unsigned int    chan_id;
    struct frame_query f_query;
};




#ifdef __cplusplus
}
#endif

#endif	//DVM4000I_IOCTL_H

