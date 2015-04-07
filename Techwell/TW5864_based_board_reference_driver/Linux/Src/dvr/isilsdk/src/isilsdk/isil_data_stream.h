#ifndef   _ISIL_DATA_STREAM_H
#define   _ISIL_DATA_STREAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#define	ISIL_PACKET_ALIGN(x)	__attribute__ ((packed))

typedef enum ISIL_H264_NALU_TYPE_E{
    H264_NALU_PSLICE  = 1,
    H264_NALU_ISLICE  = 5,
    H264_NALU_IDRSLICE = 6,
    H264_NALU_SPS        = 7,
    H264_NALU_PPS        = 8,
    H264_NALU_BUTT
}H264_NALU_TYPE_E;


typedef enum _ISIL_AUDIO_TYPE_E{
    #if 1
    ISIL_AUDIO_PCM_E    ,
	ISIL_AUDIO_ALAW_E  ,
	ISIL_AUDIO_ULAW_E   ,
	ISIL_AUDIO_ADPCM_E ,
    ISIL_AUDIO_ADPCM_32K_E,
    #endif

    AUDIO_TYPE_BUTT
}AUDIO_TYPE_E;

typedef enum _ISIL_MJPEG_TYPE_E{
    
    ISIL_MJPEG_CAPTURE_TYPE_TIMER,
    ISIL_MJPEG_CAPTURE_TYPE_USER,
    
    MJPEG_TYPE_BUTT
}MJPEG_TYPE_E;

typedef enum _ISIL_STREAM_TYPE_E{
    ISIL_MAIN_STREAM_E,
    ISIL_SUB_STREAM_E
}STREAM_TYPE_E;


typedef struct {
    unsigned short x_width;
    unsigned short y_height;
}ISIL_PACKET_ALIGN(1) ISIL_IMAGE_SIZE;

typedef struct ISIL_H264_INF_T{
    H264_NALU_TYPE_E nalu_type;
    STREAM_TYPE_E stream_type;
    ISIL_IMAGE_SIZE image_size;
}ISIL_PACKET_ALIGN(1) H264_INF_T;


typedef struct ISIL_MJPEG_INF_T{
    MJPEG_TYPE_E capture_type;
    ISIL_IMAGE_SIZE image_size;
}ISIL_PACKET_ALIGN(1) MJPEG_INF_T;


typedef struct ISIL_AUDIO_INF_T{
    AUDIO_TYPE_E audio_type_e;
    int sample;
    int bit_rate;
}ISIL_PACKET_ALIGN(1) AUDIO_INF_T;

typedef enum ISIL_DATA_TYPE_E{

    ISIL_H264_DATA,
    ISIL_AUDIO_DATA,
    ISIL_MJPEG_DATA,

    DATA_TYPE_BUTT
}DATA_TYPE_E;

typedef union ISIL_DATA_TYPE_U{
    H264_INF_T  h264_inf;
    AUDIO_INF_T audio_inf;
    MJPEG_INF_T mjpeg_inf;
}ISIL_PACKET_ALIGN(1) DATA_TYPE_U;


typedef struct _ISIL_AV_PACKET{

    unsigned int pts;
    unsigned short chip_id;
    unsigned short chan_id;
    unsigned int frm_seq; //frame number
    int  buff_size;               //buff size
    int  date_len;                //valid date len
    unsigned char *buff;
    DATA_TYPE_E frm_type;
    DATA_TYPE_U data_type_u;
    int ref;
    pthread_mutex_t lock; 
    void *priv;                //inner frame ptr, use for frame release.
//    void (*destroy)(struct _ISIL_AV_PACKET *av_pck);

}ISIL_PACKET_ALIGN(1) ISIL_AV_PACKET;


typedef enum _ISIL_ALARM_TYPE_E{
    VIDEO_LOST_DETECTION_E = 0x01,
    VIDEO_NIGHT_DETECTION_E = 0x02,
    VIDEO_BLIND_DETECTION_E = 0x04,
    VIDEO_STANDARD_MODIFY_DETECTION_E = 0x08
}ISIL_ALARM_TYPE_E;


typedef struct _ISIL_VIDEO_LOST_DETECTION_T{
   
    unsigned short chip_id;
    unsigned short  chan_id;  
    unsigned int   video_lost_valid;
    unsigned int   video_contect_valid;   
      
}ISIL_VIDEO_LOST_DETECTION_T;


typedef struct _ISIL_VIDEO_NIGHT_DETECTION_T{
   
    unsigned short chip_id;
    unsigned short  chan_id;  
    unsigned int   video_night_valid_from_day_to_night;
    unsigned int   video_night_valid_from_night_to_day;   
      
}ISIL_VIDEO_NIGHT_DETECTION_T;



typedef struct _ISIL_VIDEO_BLIND_DETECTION_T{
   
    unsigned short chip_id;
    unsigned short  chan_id;  
    unsigned int   video_blind_add_valid;
    unsigned int   video_blind_remove_valid;   
      
}ISIL_VIDEO_BLIND_DETECTION_T;


typedef struct _ISIL_VIDEO_STANDARD_DETECTION_T{
   
    unsigned short chip_id;
    unsigned short chan_id;  
    unsigned int   latest_video_standard;
    unsigned int   curr_video_standard;   
      
}ISIL_VIDEO_STANDARD_DETECTION_T;



typedef union _ISIL_ALARM_TYPE_U{
    
    ISIL_VIDEO_LOST_DETECTION_T v_lost_detect_inf;
    ISIL_VIDEO_NIGHT_DETECTION_T v_night_detect_inf;
    ISIL_VIDEO_BLIND_DETECTION_T v_blind_detect_inf;
    ISIL_VIDEO_STANDARD_DETECTION_T v_standard_detect_inf;


}ISIL_ALARM_TYPE_U;






typedef struct _ISIL_ALARM_PACKET{

    ISIL_ALARM_TYPE_E alarm_type_e;
    ISIL_ALARM_TYPE_U alarm_type_u;
    volatile int ref;
} ISIL_ALARM_PACKET;


static inline void isil_av_packet_clone(ISIL_AV_PACKET *av_packet)
{

    if(av_packet) {
        pthread_mutex_lock(&av_packet->lock);
        ++av_packet->ref;
        pthread_mutex_unlock(&av_packet->lock);
    }
}


static inline int isil_av_packet_ref_minus(ISIL_AV_PACKET *av_packet)
{
    if( av_packet ) {
        pthread_mutex_lock(&av_packet->lock);
        if(av_packet->ref > 0) {
            --av_packet->ref; 
            pthread_mutex_unlock(&av_packet->lock);
            return 0;
        }
        
        pthread_mutex_unlock(&av_packet->lock);
    }
    return -1;
}





extern void isil_av_packet_release(ISIL_AV_PACKET *av_packet);

extern ISIL_AV_PACKET * isil_av_packet_minit( void );

extern void isil_av_packet_debug(ISIL_AV_PACKET *av_packet);

extern void isil_av_packet_bit_debug(ISIL_AV_PACKET *av_packet);


static inline void isil_alarm_packet_clone(ISIL_ALARM_PACKET *alarm_packet)
{
    if(alarm_packet) {
        ++alarm_packet->ref;
    }
}

static inline void isil_alarm_packet_ref_minus(ISIL_ALARM_PACKET *alarm_packet)
{
    if( alarm_packet ) {
        if(alarm_packet->ref > 0) {
            --alarm_packet->ref; 
        }
    }
}

extern ISIL_ALARM_PACKET *isil_alarm_packet_minit( void );

extern void isil_alarm_packet_release(ISIL_ALARM_PACKET *alarm_packet);

#ifdef __cplusplus
}
#endif

#endif



