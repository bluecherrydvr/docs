#ifndef	DVM_INTERFACE_H
#define	DVM_INTERFACE_H

#ifdef __cplusplus
extern "C"
{
#endif







    //codec type define
#define	ISIL_VIDEO_H264_CODEC_TYPE		(0)
#define	ISIL_VIDEO_MJPEG_CODEC_TYPE		(1)
#define	ISIL_AUDIO_CODEC_TYPE			(2)

    //stream type define
#define	ISIL_MASTER_BITSTREAM		(0)
#define	ISIL_SUB_BITSTREAM		(1)

    //frame type define
#define	H264_FRAME_TYPE_IDR		(0)
#define	H264_FRAME_TYPE_I		(1)
#define	H264_FRAME_TYPE_P		(2)
#define	H264_FRAME_TYPE_B		(3)
#define	H264_FRAME_TYPE_SI		(4)
#define	H264_FRAME_TYPE_SP		(5)
#define	H264_MV_FRAME_TYPE		(6)
#define	MJPEG_FRAME_TYPE		(7)
#define	AUDIO_FRAME_TYPE		(8)


    //#ifndef NEW_HEADER
    struct isil_frame_header {
	unsigned int    codecType;			/*codec的类型*/
	unsigned int    streamType;			/*主码流或子码流*/
	unsigned int    frameType;			/*该帧的帧类型*/
	unsigned int    frameSerial;		/*该帧的帧号*/
	unsigned int    timeStamp;			/*该帧的时间戳*/
	unsigned int    payload_offset;		/*该帧的充域头相对于帧头的 起始地址的偏移值*/
	unsigned int    payloadLen;			/*该帧净载荷的大小，包括填充域头的大小，如果没有填充头就只是NAL载荷的大小*/
	char            pad[0];
    };


    /*针对idr的填充头*/
    struct isil_h264_idr_frame_pad	{
	unsigned short  fps;
	unsigned short  init_qp;
	unsigned char   log2MaxFrameNumNinus4;
	unsigned char   mb_width_minus1;
	unsigned char   mb_height_minus1;
	unsigned char   i_curr_qp;
	unsigned int    sps_frame_offset;	/*sps的NAL码流相对于本填充域首地址的起始偏移*/	
	unsigned int    sps_frame_size;		/*sps载荷的大小*/
	unsigned int    pps_frame_offset;	/*pps的NAL码流相对于本填充域首地址的起始偏移*/
	unsigned int    pps_frame_size;		/*pps载荷的大小*/
	unsigned int    idr_frame_offset;	/*idr的NAL码流相对于本填充域首地址的起始偏移*/
	unsigned int    idr_frame_size;		/*idr载荷的大小*/
	char            nal[0];
    };


    /*针对motion的填充头*/
    struct isil_video_motion_vector_pad {
	unsigned int	mvFlags_offset;		/*mvFlags的NAL码流相对于本填充域首地址的起始偏移*/	
	unsigned int	mvFlags_size;		/*mvFlags载荷的大小*/
	unsigned int	motionVector_offset;/*mv的NAL码流相对于本填充域首地址的起始偏移*/	
	unsigned int	motionVector_size;	/*mv载荷的大小*/
	char			nal[0];
    };


    /*针对audio的填充头*/
    struct isil_audio_frame_pad {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32	bus_id:4;
	u32 chip_id:4;
	u32 chan_id:4;
	u32 type:4;
	u32 bit_wide:4;
	u32 sample_rate:4;
	u32 bit_rate:8;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 bit_rate:8;
	u32 sample_rate:4;
	u32 bit_wide:4;
	u32 type:4;
	u32 chan_id:4;
	u32 chip_id:4;
	u32	bus_id:4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	u32		frame_offset;				/*audio的NAL码流相对于本填充域首地址的起始偏移*/	
	u32		frame_size;					/*audio载荷的大小*/
	char	nal[0];
    };




    //#else



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
    }ISIL_STREAM_TYPE_E;

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
    }isil_transparent_msg_header_t;

typedef enum{
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
        ISIL_VIDEO_MSG_ALARM_GROUP = 0x00060000,
    ISIL_VIDEO_LOST_DETECTION_MSG = 0x00060010,
    ISIL_VIDEO_NIGHT_DETECTION_MSG,
    ISIL_VIDEO_BLIND_DETECTION_MSG,
    ISIL_VIDEO_STANDARD_CHANGE_DETECTION_MSG,

    ISIL_INFO_EXT_MSG = 0x00070010,
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

    typedef struct isis_h264_encode_frame_type_msg{
	unsigned int	msg_type;           //the value comes from ISIL_MSG_TYPE_E(eg:ISIL_H264_ENCODE_FRAME_TYPE)
	unsigned int	msg_len;            //the length of isis_h264_encode_frame_type_msg
	unsigned int	frame_type;         //the value comes from ISIL_H264_FRAME_TYPE(eg:ISIL_IDR_FRAME)
	unsigned short  i_mb_x;             //the mb count in wide
	unsigned short  i_mb_y;             //the mb count in height
    }isil_h264_encode_msg_frame_type_t;




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
    }isil_h264_decode_bit_info_msg_t;




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
}isil_jpeg_param_msg_t;





    //audio staff
    /********************************************************************************/
    enum ISIL_AUDIO_SAMPLE_RATE{
	ISIL_AUDIO_8000 = 0,
	ISIL_AUDIO_16000,
	ISIL_AUDIO_32000,
	ISIL_AUDIO_44100,
	ISIL_AUDIO_48000,
	ISIL_AUDIO_RESERVED,
    };

    enum ISIL_AUDIO_BIT{
	ISIL_AUDIO_16BIT = 0,
	ISIL_AUDIO_8BIT,
    };

    enum ISIL_AUDIO_TYPE{
	ISIL_AUDIO_PCM = 0,
	ISIL_AUDIO_ALAW,
	ISIL_AUDIO_ULAW,
	ISIL_AUDIO_ADPCM_32K,
	ISIL_AUDIO_ADPCM_16K,
	ISIL_AUDIO_ADPCM_48K,
    };

    typedef struct isil_audio_param_msg{
	unsigned int	msg_type;
	unsigned int	msg_len;
	unsigned char	audio_type;
	unsigned char	audio_sample_rate;
	unsigned char	audio_bit_wide;
	unsigned char	reserve;
    }isil_audio_param_msg_t;

    //#endif


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


#ifdef __cplusplus
}
#endif

#endif	//DVM4000I_IOCTL_H

