#ifndef	ISIL_AUDIO_EN_H__
#define	ISIL_AUDIO_EN_H__

#ifdef __cplusplus
extern "C"
{
#endif




#define ISIL_CONFIG_ADD_NAL


#ifdef ISIL_CONFIG_ADD_NAL
#define ISIL_TOTAL_AUDIO_FRAMEHEADER_LEN    (sizeof(isil_frame_header_t) + sizeof(isil_audio_frame_pad_t) + sizeof(ext_h264_nal_bitstream_t)) 
#else
#define ISIL_TOTAL_AUDIO_FRAMEHEADER_LEN    (sizeof(isil_frame_header_t) + sizeof(isil_audio_frame_pad_t)) 
#endif

#define TC_BUFF_AND_FRAME_HEADER_LEN	(sizeof(struct tc_buffer) + sizeof(struct tc_frame_header))
#define TDB_FRAME_NR_MAX	64


#define ISIL_PCM_AUDIO_LEN    (498)
#define ISIL_ADPCM_AUDIO_LEN  (384)

#define	ISIL_AUDIO_8K         (0)
#define	ISIL_AUDIO_16K        (1)

#define	ISIL_AUDIO_16BIT      (0)
#define	ISIL_AUDIO_8BIT       (1)

#define	AUDIO_ENABLE        (0x0001)
#define	AUDIO_DISABLE       (0x0000)
#define	AUDIO_MUTE_ENABLE   (0x0001)
#define	AUDIO_MUTE_DISABLE  (0x0000)

#define MS_384BYTE_WITH_128K     23




#define TC_UNINITED            (0x0000)
#define TC_INITED              (0x0001)
#define TC_OPENED              (0x0002)
#define TC_RUNNING             (0x0003)
#define TC_TO_BE_DIE           (0x0004)

    /*
       enum audio_type {
       ISIL_AUDIO_PCM = 0,
       ISIL_AUDIO_ALAW,
       ISIL_AUDIO_ULAW,
       ISIL_AUDIO_ADPCM_32K,
       ISIL_AUDIO_ADPCM_16K,
       ISIL_AUDIO_ADPCM_48K,
       };
       */
    struct isil_audio_en;
    struct pre_read_queue_video;
    struct pre_read_queue_audio;


    typedef enum audio_state{
	NEED_INIT = 0,
	INITED,
	OPENED,
	CLOSED,
    }AUDIO_STATE;




    struct tdb_frame
    {
	struct list_head                list;             //used to link all tdb_bufs in one logic frame
	void                            *f_tcb;
	char 				f_header[TC_BUFF_AND_FRAME_HEADER_LEN];
    };


    struct audio_en_operation
    {
	int (*init)(struct isil_audio_en *ai);
    };

    struct isil_audio_config
    {
	int type;
	int sample_rate;
	int bit_wide;
    };

    struct isil_chip_audio_param_my
    {
	u32		change_mask_flag;
	u8		i_bit_wide;
	u8		i_sample_rate;
	unsigned long	e_audio_type;
    };

    /*
       struct decoder_mode
       {
       unsigned int            loop:1;			//0: not loop,  1: loop
       unsigned int            direction:1;		//0: backward 1: forward
       unsigned int            key_frame:1;		//0: frame one by one 1:only for key frames
       unsigned int            is_continue:1;	        //0: stop after display one frame
    //1: continue after display one frame
    int                     x_speed;		// -1 means nonsense
    };
    */
    struct prq_operation_audio
    {
	int (*get_one)(void *driv, void *dev, const char __user *buff, loff_t *off,unsigned int count, unsigned int flag);
	void (*put_one)(void *data);
	unsigned int (*get_space_free)(void *pool);    	//return the queue have how many space can use
	int  (*is_above_spressure_high)(void *data);
	int  (*is_above_spressure_low)(void *data);

	int  (*init)(struct pre_read_queue_audio *prq);
	void (*destroy)(struct pre_read_queue_audio *prq,void *data);
    };


    struct pre_read_queue_audio
    {
	unsigned int			total_nr;	  //total frames nr
	//unsigned int            	nr_vaild;         //total avaiable frames in pre_read queue
	unsigned int			water_markl;      //water mark low
	unsigned int            	water_markh;      //water mark high
	isil_audio_packet_queue_t         d_queue;	  //
	isil_audio_chan_buf_pool_t        *pool;
	struct prq_operation_audio    	*op;
    };




    struct prq_operation_video
    {
	int (*get_one)(void *driv, void *dev, const char __user *buff, loff_t *off,unsigned int count, unsigned int flag);
	void (*put_one)(void *data);
	unsigned int (*get_space_free)(void *pool);    	//return the queue have how many space can use
	int  (*is_above_spressure_high)(void *data);
	int  (*is_above_spressure_low)(void *data);

	int  (*init)(struct pre_read_queue_video *prq);
	void (*destroy)(struct pre_read_queue_video *prq,void *data);
    };


    struct pre_read_queue_video
    {
	unsigned int			total_nr;	  //total frames nr
	//unsigned int            	nr_vaild;         //total avaiable frames in pre_read queue
	unsigned int			water_markl;      //water mark low
	unsigned int            	water_markh;      //water mark high
	isil_video_frame_tcb_queue_t      d_queue;	  //
	isil_video_chan_buf_pool_t        *pool;
	struct prq_operation_video    	*op;
    };



    /*the top struct of audio encoder*/
    struct isil_audio_en
    {
	struct isil_service_device	tsd;
	struct isil_dev_table_id		tbid;
	char 				*name;
	int 				users;
	AUDIO_STATE			state;
	struct isil_chip_audio_param_my	para;
	struct semaphore		sem;
	wait_queue_head_t		wait_poll;
    struct timer_list   polling_timer;
	int				discard;    //flags to indicate the channel should discard frame.
    };

    /*the top struct of audio decoder*/
    struct isil_audio_de
    {
	struct isil_service_device	tsd;
	struct isil_dev_table_id		tbid;
	char 				*name;
	int 				users;
	AUDIO_STATE			state;
	struct isil_chip_audio_param_my	para;
	struct semaphore		sem;
	wait_queue_head_t		wait_poll;
    struct timer_list   polling_timer;
	struct list_head                resotre_head;
	struct pre_read_queue_audio     prq;
    };

    struct isil_video_en
    {
	struct isil_service_device	tsd;
	struct isil_dev_table_id		tbid;
	char 				*name;
	int 				users;
	unsigned long 			state;
	struct semaphore		sem;
	wait_queue_head_t		wait_poll;
    struct timer_list   polling_timer;
	int				discard;    //flags to indicate the channel should discard frame.
    };


    struct isil_video_de
    {
	struct isil_service_device	tsd;
	struct isil_dev_table_id		tbid;
	char                        	*name;
	int                         	users;
	unsigned long               	state;
	struct semaphore            	sem;
	wait_queue_head_t           	wait_poll;
    struct timer_list               polling_timer;
	unsigned int                  tvf_nr;
	isil_video_frame_tcb_t          *tvf[128];
	struct list_head		resotre_head; //a list to hold the frames
	struct pre_read_queue_video     prq;
#ifdef DEBUG_HELPER
	unsigned int                    i_slice_num_curr;
	unsigned int                    i_slice_num_last;
#endif
	unsigned int                    i_mb_x;
	unsigned int                    i_mb_y;
	void                            *priv;
    };


    struct isil_jpeg_en
    {
	struct isil_service_device	tsd;
	struct isil_dev_table_id		tbid;
	char 				*name;
	int 				users;
	unsigned long 			state;
	struct semaphore		sem;
	wait_queue_head_t		wait_poll;
    struct timer_list   polling_timer;
	int				discard;    //flags to indicate the channel should discard frame.
    };


    struct isil_chip_ai
    {
	struct isil_service_device	tsd;
	struct isil_dev_table_id		tbid;
	char 				*name;
	int 				users;
	unsigned long 			state;
	struct semaphore		sem;
	wait_queue_head_t		wait_poll;
    struct timer_list   polling_timer;
    };

    struct isil_chip_vi
    {
	struct isil_service_device	tsd;
	struct isil_dev_table_id		tbid;
	char 				*name;
	int 				users;
	unsigned long 			state;
	struct semaphore		sem;
	wait_queue_head_t		wait_poll;
    struct timer_list   polling_timer;
    };

    struct isil_chip_vo
    {
	struct isil_service_device	tsd;
	struct isil_dev_table_id		tbid;
	char 				*name;
	int 				users;
	unsigned long 			state;
	struct semaphore		sem;
	wait_queue_head_t		wait_poll;
    struct timer_list   polling_timer;
    };

    static inline void set_tve_state(struct isil_audio_en *tve, AUDIO_STATE new_state)
    {
	tve->state = new_state;
    }


    static inline void set_tae_state(struct isil_audio_en *tae, AUDIO_STATE new_state)
    {
	tae->state = new_state;
    }

    static inline void set_tde_state(struct isil_audio_de *tde, AUDIO_STATE new_state)
    {
	tde->state = new_state;
    }

    static inline void tc_set_audio_param(struct isil_chip_audio_param_my *src,struct isil_chip_audio_param_my *dest)
    {
	/*change_mask_flag how to use???*/
	src->i_bit_wide = dest->i_bit_wide;
	src->i_sample_rate = dest->i_sample_rate;
	src->e_audio_type = dest->e_audio_type;
    }


    static inline unsigned long get_payload_len(struct isil_chip_audio_param_my *para)
    {
	unsigned long ret = U32LONG_MAX;

	switch(para->e_audio_type)
	{
	    case ISIL_AUDIO_PCM:
		ret = ISIL_PCM_AUDIO_LEN;
		break;
	    case ISIL_AUDIO_ADPCM_32K:
	    case ISIL_AUDIO_ADPCM_16K:
	    case ISIL_AUDIO_ADPCM_48K:
		ret = ISIL_ADPCM_AUDIO_LEN;
		break;
	    case ISIL_AUDIO_ALAW:
	    case ISIL_AUDIO_ULAW:
	    default:
		printk("not supported now!\n");
		break;
	}
	return ret;
    }

    /*
       static inline  unsigned long calc_time_stamp_deta(struct isil_audio_frame_pad *para, unsigned long len)
       {
       unsigned long bytes_per_ms = 0;

       switch(para->sample_rate)
       {
       case ISIL_AUDIO_8K:
       if(ISIL_AUDIO_8BIT == para->bit_wide)
       bytes_per_ms =  8;
       else
       bytes_per_ms =  16;
       break;
       case ISIL_AUDIO_16K:
       if(ISIL_AUDIO_8BIT == para->bit_wide)
       bytes_per_ms =  16;
       else
       bytes_per_ms =  32;			
       break;
       }
       return len/bytes_per_ms;
       }
       */











    extern struct isil_chip_audio_param_my 	def_audio_parm;
    int  tc_chip_init(struct isil_chip_device *tcd,unsigned long bus_id,unsigned long chip_id);
    void  tc_chip_exit(struct isil_chip_device *tcd);
    int isil_audio_en_init(void);
    void isil_audio_en_remove(void);

    int isil_audio_de_init(void);
    void isil_audio_de_remove(void);

    int isil_videoh264_en_init(void);
    void isil_videoh264_en_remove(void);

    int isil_videoh264s_en_init(void);
    void isil_videoh264s_en_remove(void);

    int isil_jpeg_en_init(void);
    void isil_jpeg_en_remove(void);

    int isil_audio_de_driver_init(void);
    void isil_audio_de_driver_remove(void);

    int isil_audio_en_driver_init(void);
    void isil_audio_en_driver_remove(void);

    int isil_video_en_driver_init(void);
    void isil_video_en_driver_remove(void);

    int isil_videosub_en_driver_init(void);
    void isil_videosub_en_driver_remove(void);

    int isil_jpeg_en_driver_init(void);
    void isil_jpeg_en_driver_remove(void);

    int isil_chip_vi_init(void);
    void isil_chip_vi_remove(void);


    int isil_chip_ai_init(void);
    void isil_chip_ai_remove(void);


#ifdef __cplusplus
}
#endif

#endif
