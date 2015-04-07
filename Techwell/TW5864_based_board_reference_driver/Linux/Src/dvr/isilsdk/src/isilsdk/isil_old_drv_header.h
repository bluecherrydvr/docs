#ifndef _ISIL_OLD_DRV_HEADER_H

#define _ISIL_OLD_DRV_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#define	ISIL_VIDEO_H264_CODEC_TYPE		(0)
#define	ISIL_VIDEO_MJPEG_CODEC_TYPE		(1)
#define	ISIL_AUDIO_CODEC_TYPE			(2)

#define	ISIL_MASTER_BITSTREAM		(0)
#define	ISIL_SUB_BITSTREAM		(1)

#define	H264_FRAME_TYPE_IDR		(0)
#define	H264_FRAME_TYPE_I		(1)
#define	H264_FRAME_TYPE_P		(2)
#define	H264_FRAME_TYPE_B		(3)
#define	H264_FRAME_TYPE_SI		(4)
#define	H264_FRAME_TYPE_SP		(5)
#define	H264_MV_FRAME_TYPE		(6)
#define	MJPEG_FRAME_TYPE		(7)
#define	AUDIO_FRAME_TYPE		(8)


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


/*针对audio的填充头*/
#define __BIG_ENDIAN_BITFIELD
struct isil_audio_frame_pad {
	unsigned int bit_rate:8;
	unsigned int sample_rate:4;
	unsigned int bit_wide:4;
	unsigned int type:4;
	unsigned int chan_id:4;
	unsigned int chip_id:4;
	unsigned int	bus_id:4;
	unsigned int		frame_offset;				/*audio的NAL码流相对于本填充域首地址的起始偏移*/	
	unsigned int		frame_size;					/*audio载荷的大小*/
	char	nal[0];
};


#ifdef __cplusplus
}
#endif

#endif


