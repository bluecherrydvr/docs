#ifndef	DVM_INTERFACE_H
#define	DVM_INTERFACE_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum ISIL_STREAM_TYPE{
	ISIL_H264_STERAM,
	ISIL_MVFLAG_STERAM,
	ISIL_MVD_STERAM,
	ISIL_MJPEG_STREAM,
	ISIL_IMAGE_STREAM,
	ISIL_AUDIO_STREAM,
	ISIL_INFO_STREAM,
    }ISIL_STREAM_TYPE_E;

    struct isil_common_stream_header{
	unsigned int    stream_type;
	unsigned int    stream_timestamp;
	unsigned int    stream_checksam;
	unsigned int    payload_len;
	unsigned char   pad[0];
    };

    typedef enum ISIL_CODEC_TYPE{
	ISIL_ENCODE_TYPE,
	ISIL_DECODE_TYPE,
    };

    typedef enum ISIL_H264_FRAME_TYPE{
	ISIL_SPS_FRAME,
	ISIL_PPS_FRAME,
	ISIL_IDR_FRAME,
	ISIL_I_FRAME,
	ISIL_P_FRAME,
    };

    struct isil_h264_codec_pad{
	unsigned char   codec_type;
	unsigned char   frame_type;
	unsigned char   frame_info[0];
    };

    struct isil_h264_encode_frame_info{
	unsigned short  reserve;
	unsigned int    nal_position_offset_from_isil_common_stream_header;
	unsigned int    nal_len;
    };

    struct isil_h264_decode_frame_info{
	unsigned short  bit_info;
	unsigned int    nal_position_offset_from_isil_common_stream_header;
	unsigned int    nal_len;
    };

    struct isil_mjpeg_codec_pad{
	unsigned char   codec_type;
	unsigned char   reserve;
	unsigned short  mjpeg_position_offset_from_isil_common_stream_header;
	unsigned int    mjpeg_payload_len;
    };

    typedef enum ISIL_IMAGE_FORMAT_TYPE{
	YUV420,
	YUV422,
	/*    ...*/
    };

    struct isil_image_frame_pad{
	unsigned char   mb_width_minus1;
	unsigned char   mb_height_minus1;
	unsigned short  image_type;
	unsigned short  image_position_offset_from_isil_common_stream_header;
	unsigned int    image_payload_len;
    };

    struct isil_audio_codec_pad{
	unsigned char   audio_type;
	unsigned char   bit_wide;
	unsigned char   sample_rate;
	unsigned char   bit_rate;
	unsigned short  audio_position_offset_from_isil_common_stream_header;
	unsigned short  audio_payload_len;
    };

    struct isil_mvf_pad{
	unsigned short  mvf_position_offset_from_isil_common_stream_header;
	unsigned short  mvf_payload_len;
    };

    struct isil_mvd_pad{
	unsigned short  mvd_position_offset_from_isil_common_stream_header;
	unsigned short  mvd_payload_len;
    };

    struct isil_info_pad{
	//reference netlink msg define
    };


#ifdef __cplusplus
}
#endif


#endif	//DVM4000I_IOCTL_H


