#ifndef	__ISIL_CORE_BUF_H__
#define  __ISIL_CORE_BUF_H__	


#ifdef __cplusplus
extern "C"
{
#endif


#define TC_BUFFER_FOR_66

#define QUEUE_MAX_FRAME		10
#define SCATTRT_MAX_ENTRY	128
    
#define MIN_COMMON_HEAD_LEN	(sizeof(struct tc_buffer) + sizeof(struct tc_frame_header))
	
#define MIN_HEADER_LEN_AUDIO		(MIN_COMMON_HEAD_LEN + sizeof(struct tc_audio_frame_pad))
#define MIN_HEADER_LEN_VIDEO		MIN_COMMON_HEAD_LEN
#define MIN_HEADER_LEN_VIDEO_IDR	(MIN_COMMON_HEAD_LEN + sizeof(struct tc_h264_idr_frame_pad))
#define MIN_HEADER_LEN_JPEG		MIN_COMMON_HEAD_LEN

	//#define VIDEO_DECODER_USEMAP
	
	
	enum tc_mem_type 
	{
	    MEM_MMAP = 0,	
	};
    
	enum tc_buf_type 
	{
	    BUF_ENCODER_MAP = 0,            //encoder map
	    BUF_ENCODER_SYNC,		//encoder syn
	    BUF_DECODER_MAP,		//decoder map
	    BUF_DECODER_SYNC,		//decoder sync
	};


    enum tc_buf_satate
    {
	STATE_NEEDINIT = 0,	
	STATE_FREE,
	STATE_READY,  		/*the data can be used by user*/
	STATE_BUSY, 		/*the user is processing,driver, device framework is untouchable right now!*/
	STATE_OBSOLETE,		/*the data can be free, of cause, we just notify low level driver to do the actully free works */
    };


    struct tc_buf_despt
    {
	__u32  			count;
	enum tc_buf_type	btype;
	enum tc_mem_type	mtype;
    };
    
	struct tc_scatter
	{
	    __u32   *addr;
		__u32   len;
		void    *carrier;
	};

    /*DS = decoder sync*/
#define  TC_BUFFER_TYPE_DS_H264         0x0001
#define  TC_BUFFER_TYPE_DS_AUDIO        0x0002
#define  TC_BUFFER_TYPE_DS_MJPEG        0x0003
	//others to be defined......
	
	
	
	
	
	struct tc_time
	{
	    unsigned char		hour;
		unsigned char		min;
		unsigned char           second;	
		unsigned char           resvered;             
	};
    struct tc_date
    {
	unsigned short		year;
	    unsigned char		month;
	    unsigned char		day;
    };
    struct tc_xtime
    {
	struct tc_date		date;
	    struct tc_time		time;
    };
    
	struct tc_buffer_h264_dspt
	{
	    unsigned char            is_display;	        /*weather this frame should be displayed */
		unsigned char            is_end;	        /*weather this frame is end of one set frames*/
		unsigned char            is_file_end;          /*weather this frame is end of file*/
		unsigned char            is_gop_end;           /*weather this frame is end of one gop*/
	};
    
	struct tc_buffer // total 32 bytes
	{
	    /*common part  4 bytes*/
#if defined(__LITTLE_ENDIAN_BITFIELD)        
		unsigned int            index:8;                /*max 256*/
		unsigned int            type:4;			/*indicate used by decoder or encode*/
		unsigned int            flag:4;                 /**/                  	
		unsigned int            memory:3;               /*max 8 types mapmethod*/
		unsigned int            rev:13;
#elif defined (__BIG_ENDIAN_BITFIELD)
		unsigned int            rev:13;
		unsigned int            memory:3;               /*max 8 types mapmethod*/
		unsigned int            flag:4;                 /**/                  	
		unsigned int            type:4;			/*indicate used by decoder or encode*/
		unsigned int            index:8;                /*max 256*/
#else
#error	"Please fix <asm/byteorder.h>"
#endif
		
		/*union common part*/
		union {
		    /*for decoderuse*/
			struct  {
			    /*common part of the decoder sync 20 bytes*/
				struct tc_xtime	        abs_time;		/*absolutely time of the frame(UTC)*/
				unsigned int            pay_off;		/*payload offset, from this structure begin */
				unsigned int            pay_len;		/*payload length*/
				unsigned char           ds_type;                /*the type for each service decoder*/
				unsigned char           is_xtime_valid;          /*indicate the xtime is valid or not*/
				unsigned short          private_len;            /*max 64K private data len*/         
				unsigned char           private[0];
			}ds;

		    /*for mem map use*/
			struct {
			    enum tc_buf_satate      state;			/*free, ready ,or busy*/
				unsigned long           nr_frame;		/*how many frames are ready */
				int                     isalign; 		/*is the address of 'start' page aligned */
				unsigned long           off_to_page;	        /*offset to 4kb aligned page start address*/
				__u32                   *start;		        /*actually data address */
				__u32                   size;			/*actually data length */
				
				unsigned short          private_len;            /*max 64K private data len*/ 
				unsigned char           private[0];
			}m;
		    
			// others to be added here...
			
		}codec_info;

	    char            tc_frame_head[0];                       /*this field can not access directly,*/                             
	};
    
	
	
	/*macros can be easy use*/
#define __tc__state		codec_info.m.state
#define __tc__nr_frame		codec_info.m.nr_frame
#define __tc__isalign		codec_info.m.isalign
#define __tc__off_to_page	codec_info.m.off_to_page
#define __tc__start		codec_info.m.start
#define __tc__size		codec_info.m.size
	
	/*decoder sync common part*/
#define __tc__abs_time          codec_info.ds.abs_time
#define __tc__pay_off		codec_info.ds.pay_off
#define __tc__pay_len		codec_info.ds.pay_len
#define __tc__ds_type		codec_info.ds.ds_type
#define __tc__is_abs_time_valid codec_info.ds.is_xtime_valid
#define __tc__ds_privlen	codec_info.ds.private_len
	
	
	
#define INVALID_TIME            0xAF
	
	

	/*bused by twell core in kernel*/
	struct tc_buffer_kern
	{
	    __u32			index;
	    enum tc_buf_type	type;
	    enum tc_buf_satate	state;
	    __u32			flag;
	    struct timeval		timestap;
	    __u32                   crc;    
		struct tc_queue		*tq;
	    wait_queue_head_t       done;
	    enum tc_mem_type	memory;
	    struct tc_header_buf    *thb;                           /*the tc_header_buf pointer*/
		__u32                   last_prepare;                   /*user buffer is small,it records the last ioctl we prepared data size, 
									 *when user give us sufficent buffer ,so we can go on*/
	    void                    *holder;
		/*staff for signal map (typical for audio)*/
		int			isalign; 			/* is the address of 'start' page aligned */
	    unsigned long 		off_to_page;			/* the offset to 4kb aligned page start address*/
	    __u32			*start;				/* actully data address */
	    __u32			size;				/* actully data length */
	    
		/*staff for scatter map (typical for video...)*/
		struct tc_scatter       sg[SCATTRT_MAX_ENTRY];
		char			tc_frame_head[0];
	};


    struct tc_frame_header {
	unsigned int    codecType;			
	unsigned int    streamType;			
	unsigned int    frameType;			
	unsigned int    frameSerial;		
	unsigned int    timeStamp;
	unsigned int    payload_offset;
	unsigned int    payloadLen;
	char             pad[0];
    };

    struct tc_audio_frame_pad {
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
	u32	frame_offset;	
	u32	frame_size;
	char	nal[0];
    };

    struct tc_h264_idr_frame_pad{
	unsigned short  fps;
	unsigned short  init_qp;
	unsigned char   log2MaxFrameNumNinus4;
	unsigned char   mb_width_minus1;
	unsigned char   mb_height_minus1;
	unsigned char   i_curr_qp;
	unsigned int    sps_frame_offset;
	unsigned int    sps_frame_size;
	unsigned int    pps_frame_offset;
	unsigned int    pps_frame_size;
	unsigned int    idr_frame_offset;
	unsigned int    idr_frame_size;
	char            nal[0];
    };

    struct tc_queue_ops
    {
	/*will be different while for different driver*/
	int  (*buf_setup)(struct tc_queue * tq, unsigned long *outval);				/*calc the size and the buffer count*/
	int  (*buf_prepare)(struct tc_queue * tq, void *priv,struct tc_buffer __user *tc);     		/* get actully buffer from low level driver */
	int  (*buf_release)(struct tc_queue * tq, void *priv,struct tc_buffer_kern *tc); 	/* return buffer to driver buffer pool */	
	void (*buf_to_ready)(struct tc_queue * tq, struct tc_buffer_kern *tc); 	/* put tc_buffer to our ready array */
	void (*buf_to_obsolete)(struct tc_queue * tq, struct tc_buffer_kern *tc);	/* put tc_buffer to our ready array */
    };



    struct tc_queue {
	struct mutex               lock;
	void			   *dev; 	/* on pci, points to struct pci_dev */

	enum tc_buf_type           type;
	unsigned int               unit_size;  	/*for each unit frame size*/
	unsigned long 		   buf_nr;	/*hwo many tc_buffer_kern this queue have*/	
	struct tc_buffer_kern	   *bufs[QUEUE_MAX_FRAME];
	unsigned long 		   wp;		/*write pointer*/
	unsigned long 		   rp;		/*read pointer*/
	wait_queue_head_t          done;

	struct tc_queue_ops  	   *ops;
	//unsigned int               streaming;
	/* for read/write.... */

	/* driver private data */
	void                       *priv_data;
    };







    static inline int queue_full(struct tc_queue * tq)
    {
	return (tq->rp == (tq->wp + 1)%(tq->buf_nr));
    }

    static inline int queue_empty(struct tc_queue * tq)
    {
	return  tq->wp == tq->rp;
    }


    /*these function should be called with tc_queue.lock holeded*/
    static inline void queue_inc_wp(struct tc_queue * tq)
    {
	tq->wp++;
	tq->wp = tq->wp % tq->buf_nr;
    }

    static inline void queue_inc_rp(struct tc_queue * tq)
    {
	tq->rp++;
	tq->rp = tq->rp % tq->buf_nr;	
    }


    static inline void set_tck_state(struct tc_buffer_kern *tck, enum tc_buf_satate new_state)
    {
	tck->state = new_state;
    }

    static inline enum tc_buf_satate   get_tck_state(struct tc_buffer_kern *tck)
    {
	return tck->state;
    }






    void set_tq_state_busy(struct tc_queue *tq);
	void tc_buffer_set_audio_data(struct tc_buffer *tc, struct tc_buffer_kern *tck,isil_audio_packet_section_t *taps,unsigned long *off);
    //void tc_buffer_set_h264_data(struct tc_buffer *tc, struct tc_buffer_kern *tck, isil_video_frame_tcb_t *vf);
    void tc_buffer_set_h264_data(int is_master,struct tc_buffer __user *tc, struct tc_buffer_kern *tck, isil_video_frame_tcb_t *vf,unsigned long *off);
    void tc_buffer_set_jpeg_data(struct tc_buffer __user *tc, struct tc_buffer_kern *tck, struct isil_vb_frame_tcb *tvf, unsigned long *off);
    int tc_queue_init(struct tc_queue *q, struct tc_queue_ops *ops, void * dev,unsigned long usize, unsigned long buf_count,void *priv);
    void  tc_queue_reset(struct tc_queue *q);
    void tc_queue_release(struct tc_queue *q);






#ifdef __cplusplus
}
#endif

#endif	
