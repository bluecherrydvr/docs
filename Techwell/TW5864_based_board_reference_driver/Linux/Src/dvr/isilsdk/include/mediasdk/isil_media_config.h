#ifndef _ISIL_MEDIA_CONFIG_H
#define _ISIL_MEDIA_CONFIG_H



#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include "isil_codec_common.h"
#include "config.h"
#include "linux_list.h"

#define MAX_PATH_LEN (128)

typedef enum{
	NODE_TYPE_VI = 0,
	NODE_TYPE_AI ,
	NODE_TYPE_VO ,
	NODE_TYPE_AO ,
	NODE_TYPE_BUBBON
}EIONODETYPE;

typedef enum{
    CHAN_INVALID_MODE = -1,
    CHAN_DISABLE_MODE = 0,
    CHAN_ENABLE_MODE
}ECHNMODE;

enum ECHIP_PROPERTY{
    CHIP_CAN_ENC = 0x01,
    CHIP_CAN_DEC = 0x02,
	CHIP_CAN_VI  = 0x04,
	CHIP_CAN_AI  = 0x08,
	CHIP_CAN_VO  = 0x10,
	CHIP_CAN_AO  = 0x20,
	CHIP_CAN_CODEC = 0x40
};

enum ECHAN_PROPERTY{
    CHAN_IS_ENC,
    CHAN_IS_DEC,
    CHAN_PRO_BUTTON
};

enum ECHANFUNCPRO{
    IS_H264_MAIN_STREAM,
    IS_H264_SUB_STREAM,
    IS_MJPEG_STREAM,
    IS_AUDIO_STREAM,
    FUNCPROSIZE
};


    
#define MAX_ENC_NUM_PER_CHIP (16)
    
#define MAX_DEC_NUM_PER_CHIP (16)


#define CHIP_DEV_ROOT "/dev/isil"
#define CHIP_VI_DEV_DIR "vi"
#define CHIP_VO_DEV_DIR "vo"
#define CHIP_AI_DEV_DIR "ai"
#define CHIP_AO_DEV_DIR "ao"
#define CHIP_CODEC_DEV_DIR "codec"
#define DEVNODNAME "dev"



typedef struct MEDIA_CHIPS_MGT_T S_MEDIA_CHIPS_MGT;
//typedef struct LOGIC_TO_DRV_CHAN_MAP_T  S_LOGIC_TO_DRV_CHAN_MAP;
typedef struct SINGLE_CHIP_MGT_T  S_SINGLE_CHIP_MGT;
typedef struct ISIL_CHAN_MGT_T      S_ISIL_CHAN_MGT;
typedef struct ISIL_SINGLE_CHAN_MGT_T  S_ISIL_SINGLE_CHAN_MGT;



typedef struct ISIL_IO_NODE_MGT{
	volatile int       is_enable;  // -1: no work ,0 :disable, 1:enable
	int fd;
	void * priv;
	S_SINGLE_CHIP_MGT *sin_chip_mgt_ptr;

	void (*release)(struct ISIL_IO_NODE_MGT);
}__attribute__ ((packed)) IO_NODE_MGT;


typedef struct
{
	char   ch_dir[MAX_PATH_LEN];//root dir,all enc/dec channel is here
	char   vi_dir[MAX_PATH_LEN];//vi chip node dir
	char   vo_dir[MAX_PATH_LEN];//vo chip node dir
	char   ai_dir[MAX_PATH_LEN];//ai chip node dir
	char   ao_dir[MAX_PATH_LEN];//ao chip node dir
	char   codec_dir[MAX_PATH_LEN];//encoding/decoding chip node dir
}CHIP_NODE_CFG;

struct ISIL_SINGLE_CHAN_MGT_T{
    
    volatile int       is_enable;  // -1: no work ,0 :disable, 1:enable
    char                  log_pro;
    char                  log_reserv;
    unsigned short    log_id;
    int fd;
    CODEC_CHANNEL  codec_channel_t;
    void *priv;
    S_ISIL_CHAN_MGT *parent;
    void (*release)(S_ISIL_SINGLE_CHAN_MGT *single_chan_mgt);
}__attribute__ ((packed));

struct ISIL_CHAN_MGT_T{
     struct list_head      chip_entry;
     struct list_head      logic_num_entry;
     unsigned short logic_num;
     unsigned short chip_chan_num;
     S_SINGLE_CHIP_MGT   *single_chip_root;
     S_MEDIA_CHIPS_MGT  *logic_chips_root;
     unsigned char eff_array_num;
     unsigned char cur_chip_id;
     unsigned char enc_or_dec; //ECHIP_PROPERTY
     unsigned char reserv;
     S_ISIL_SINGLE_CHAN_MGT **single_chan_mgt_array;
     void (*release)(S_ISIL_CHAN_MGT *chans_mgt);
}__attribute__ ((packed));


struct SINGLE_CHIP_MGT_T {

	S_ISIL_CHAN_MGT **enc_chan_mgt_arr; //chan  list by chan num; 
    
    int enc_chan_nums;
    S_ISIL_CHAN_MGT **dec_chan_mgt_arr; //chan  list by chan num; 
    
    int dec_chan_nums;
    int            chip_fd ;  //chip_fd
    unsigned char cur_chip_id;
    unsigned char enc_max_chan_num;
    unsigned char chip_pro; //CHIP_CAN_ENC , CHIP_CAN_DEC
    unsigned char dec_max_chan_num;
    
	IO_NODE_MGT io_node_mgt_arr[NODE_TYPE_BUBBON];
    CHIP_NODE_CFG *chip_node;
    void (*release)(S_SINGLE_CHIP_MGT *chip_mgt);
}__attribute__((__packed__));

#if 0
struct LOGIC_TO_DRV_CHAN_MAP_T{
    struct list_head entry;
    unsigned short logic_id;
    volatile unsigned short ref;
    struct list_head same_id_chan_mgt_list;
    int same_id_chan_mgt_num;
    pthread_mutex_t  list_lock;
};
#endif


struct MEDIA_CHIPS_MGT_T{
    int total_chip_num;
    struct list_head encode_list;
    pthread_mutex_t  enc_list_lock;
    unsigned short enc_num;
    unsigned short max_enc_num;
    
    struct list_head decode_list;
    pthread_mutex_t  dec_list_lock;
    unsigned short dec_num;
    unsigned short max_dec_num;
    S_SINGLE_CHIP_MGT **per_chip_mgt_array;
    void (*release)(S_MEDIA_CHIPS_MGT *s_media_chips_mgt);
}__attribute__((__packed__));



typedef struct _CHIP_ENC_STATUS_INF_T{
	unsigned int chip_id;
	int h264_main_stream_status_arr[MAX_ENC_NUM_PER_CHIP];
	int h264_sub_stream_status_arr[MAX_ENC_NUM_PER_CHIP];
}CHIP_ENC_STATUS_INF;
	 




#define  SET_SINGLE_CHAN_LOG_ID(x ,value)  ( ((x)->log_id) = value)
#define  GET_SINGLE_CHAN_LOG_ID(x)  ( ((x)->log_id )

#define SET_SINGLE_CHAN_ENABLE_PRO(x,value) ( ((x)->is_enable) = value)
#define GET_SINGLE_CHAN_ENABLE_PRO(x)  ((x)->is_enable) 

#define SET_SINGLE_CHAN_LOG_PRO(x,value) ( ((x)->log_pro) = value)
#define GET_SINGLE_CHAN_LOG_PRO(x) ( ((x)->log_pro) )

#define SET_SINGLE_CHAN_FD(x,value) ( ((x)->fd) = value)
#define GET_SINGLE_CHAN_FD(x) ( (x)->fd )

#define SET_CHAN_MGT_CHIP_CHAN_NUM(x,value) ( ((x)->chip_chan_num) = value)
#define GET_CHAN_MGT_CHIP_CHAN_NUM(x) ((x)->chip_chan_num)



static inline CODEC_CHANNEL *get_codec_channel_by_chan_mgt(S_ISIL_SINGLE_CHAN_MGT *single_chan_mgt)
{
    return &single_chan_mgt->codec_channel_t;
}

static inline  void set_single_chan_mgt_codec_pro(S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt ,CODEC_CHANNEL *codec_channel)
{
    sin_chan_mgt->codec_channel_t = (*codec_channel);
}


static inline int get_chip_fd(S_ISIL_CHAN_MGT *chan_mgt)
{
    return ( (chan_mgt->single_chip_root == NULL)? -1:chan_mgt->single_chip_root->chip_fd);
}



extern void init_glb_media_chips_mgt(int chipNum);

extern S_MEDIA_CHIPS_MGT * get_glb_media_chips_mgt_ptr( void );

extern int isil_get_chips_count( void );

extern int get_total_enc_count(void);

extern int get_total_dec_count(void);
extern void release_chips_mgt_tree( void );


extern S_ISIL_CHAN_MGT * enc_get_sameid_chan_mgt_from_logic_lst( int id);

extern void enc_put_chan_mgt_to_logic_lst(S_ISIL_CHAN_MGT  *chan_mgt);

/*LOGIC DEC LST OPT*/
extern S_ISIL_CHAN_MGT * dec_get_sameid_chan_mgt_from_logic_lst( int id);

extern void dec_put_chan_mgt_to_logic_lst(S_ISIL_CHAN_MGT  *chan_mgt);

extern S_SINGLE_CHIP_MGT *get_single_chip_mgt_by_id( unsigned int chip_id);


/*CHIP ENC LIST OPT*/


extern S_ISIL_CHAN_MGT  * enc_get_chip_chn_chan_mgt_from_chip_lst(unsigned short chip_chn ,S_SINGLE_CHIP_MGT *single_chip_mgt);

extern void enc_put_chan_mgt_to_chip_lst(S_ISIL_CHAN_MGT  *chan_mgt, S_SINGLE_CHIP_MGT *single_chip_mgt);

/*CHIP DEC LIST OPT*/

extern void dec_put_chan_mgt_to_chip_lst(S_ISIL_CHAN_MGT  *chan_mgt, S_SINGLE_CHIP_MGT *single_chip_mgt);


extern S_ISIL_CHAN_MGT  * dec_get_chip_chn_chan_mgt_from_chip_lst(unsigned short chip_chn ,S_SINGLE_CHIP_MGT *single_chip_mgt);

//extern S_LOGIC_TO_DRV_CHAN_MAP *get_or_malloc_new_logic_drv_map_t(unsigned short chanNum);

extern S_SINGLE_CHIP_MGT *init_malloc_single_chip_mgt( unsigned char chip_id );

extern S_ISIL_CHAN_MGT *malloc_init_isil_chan_mgt( void );

extern S_ISIL_SINGLE_CHAN_MGT *init_malloc_single_chan_mgt( enum ECHAN_PROPERTY isEnc );

extern S_ISIL_SINGLE_CHAN_MGT *get_single_chan_mgt_by_chan_mgt( S_ISIL_CHAN_MGT * chan_mgt ,int type);

extern void free_single_chan_mgt(S_ISIL_SINGLE_CHAN_MGT *single_chan_mgt);

extern int reg_single_chan_mgt(S_ISIL_SINGLE_CHAN_MGT *single_chan_mgt ,S_ISIL_CHAN_MGT *chan_mgt);

extern void unreg_single_chan_mgt(S_ISIL_SINGLE_CHAN_MGT *single_chan_mgt);

extern int get_chip_num_by_chan_mgt(S_ISIL_CHAN_MGT *chan_mgt);

extern S_ISIL_SINGLE_CHAN_MGT *get_sin_chn_mgt_by_chip_chn_type(int chipid,
                                                         int chn ,
                                                         enum ECHAN_PROPERTY pro,
                                                         enum ECHANFUNCPRO eStreamType);



extern int isil_open_av_io_package(char *path,unsigned int chip_id ,EIONODETYPE eionodetype);

extern IO_NODE_MGT *get_io_node_mgt_by_type_id(unsigned int chip_id,EIONODETYPE eionodetype);

/************************************************** 
  *                VD  CONFIG FUNC
  *************************************************/

extern int init_chip_vdconfig( int chip );


extern int get_media_max_chan_number( void );

extern void init_chip_enc_status_inf(CHIP_ENC_STATUS_INF *chip_enc_status);






/**********************************************
*                                                                     
*  CHIP_NODE_CFG CONFIG relation
*  
 **********************************************/




/************************************************** 
  *                 CHIPS FILE CONFIG OPT
  *************************************************/


/************************************************** 
  *                 DEBUG FUNC
  *************************************************/

extern void debug_chips_tree( void );

extern void debug_single_chan_mgt( S_ISIL_SINGLE_CHAN_MGT *single_chan_mgt );

extern void debug_chan_mgt(S_ISIL_CHAN_MGT  *chan_mgt);

extern void debug_sin_chip_enc_lst(S_SINGLE_CHIP_MGT *sin_chip_mgt);

extern void debug_sin_chip_dec_lst(S_SINGLE_CHIP_MGT *sin_chip_mgt);

extern void debug_sin_chip_mgt( S_SINGLE_CHIP_MGT *sin_chip_mgt);

extern void debug_chips_lgc_enc_list( void );

extern void debug_codec_channel_t(CODEC_CHANNEL *codec_channel_t);



extern int get_chips_tree_by_sys_inf( void );

extern S_ISIL_CHAN_MGT * get_chan_mgt_by_chipid_chn(int chipid,int chn ,enum ECHAN_PROPERTY pro);
extern int get_enc_chan_status_by_chip(unsigned int chip,CHIP_ENC_STATUS_INF *chip_enc_status);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
