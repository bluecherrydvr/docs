#ifndef  _ISIL_PB_MGT_H
#define  _ISIL_PB_MGT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include "linux_list.h"
#include "isil_msg_mgt.h"
#include "isil_net_data_header.h"
#include "isil_data_stream.h"
#include "net_task.h"
#include "isil_media_config.h"


typedef enum{
    DEC_MGT_AUDIO,
    DEC_MGT_VIDEO,
    DEC_MGT_BUTTON
}DEC_MGT_TYPE_E;

typedef enum{
    NORMAL_PB_MODE,
    BACKWARD_PB_MODE,
    FRAMEBACK_PB_MODE,
    FRAMEFORWARD_PB_MODE
}PB_PLAY_MODE_E;

#define FILE_GET_FRAME_IS_V  (1)
#define FILE_GET_FRAME_IS_A  (2)

#define MAX_DEC_CHANS_NUM (16)

typedef struct _pb_list{
    struct list_head pb_thread_list;
    unsigned int thread_num;
    pthread_mutex_t  thread_lock;
    short  dec_a_chan_occupy_t;
    short  dec_v_chan_occupy_t;
    pthread_mutex_t av_chan_lock;
    int release_over;
    pthread_cond_t release_cond;
    pthread_mutex_t release_lock;
}pb_list;



typedef struct _single_pb_mgt{
    struct list_head entry;
    struct ISIL_MSG_MGT *msg_list;
    
    struct NET_DATA_MGT_T *audio_list;
    struct NET_DATA_MGT_T *video_list;
    volatile unsigned int is_pb_thr_run;
	unsigned int chip_id;
    unsigned int final_v_frame_count;
    unsigned int final_a_frame_count;
    unsigned int a_first_tsp;
    unsigned int v_first_tsp;
    unsigned char have_video;
    unsigned char have_audio;
    unsigned char is_video_chan_open;
    unsigned char is_audio_chan_open;
    unsigned char video_chan_num;
    unsigned char audio_chan_num;
    unsigned char is_progress_bar_timecoming;
    volatile unsigned char need_read_file;
    int video_fd;
    int audio_fd;
    int cur_file_fd;
    int cur_net_fd;
    unsigned char cur_mode; //maybe
    unsigned char is_preread;
    unsigned char disply_ch;
    volatile unsigned char stop_dec; //stop dec
    unsigned int start_ts;
    unsigned int end_ts;
	int wait;
	int video_sync;
    
    ISIL_AV_PACKET *cur_pck;
    pb_list *pb_parent;
    
    
    struct NET_TASK *ev_task;

    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt_array[DEC_MGT_BUTTON];
    
    void *arg;
    int  (*read_av_file)(struct _single_pb_mgt *s_pb_mgt ,void *arg);
    void (*avSync)(struct _single_pb_mgt *s_pb_mgt);
    void (*disFramePolicy)(struct _single_pb_mgt *s_pb_mgt);
    
    void (*release)(struct _single_pb_mgt *s_pb_mgt);
    
}single_pb_mgt;


#define SET_S_PB_PREREAD_STAT(x,y) (((x)->is_preread) = (y) )

#define GET_S_PB_PREREAD_STAT(x) (((x)->is_preread))

#define SET_S_PB_CHIP_ID(x,y) (((x)->chip_id) = (y) )

#define GET_S_PB_CHIP_ID(x) (((x)->chip_id))

#define SET_S_PB_END_TS(x ,y) (((x)->end_ts) = (y) )

#define GET_S_PB_END_TS(x ) ((x)->end_ts)

#define SET_S_PB_START_TS(x ,y) (((x)->start_ts) = (y) )

#define GET_S_PB_START_TS(x ) ((x)->start_ts)

#define SET_S_PB_NO_READ_FILE(x) ((x)->need_read_file = 0 )

#define SET_S_PB_READ_FILE(x) ((x)->need_read_file = 1 )

#define GET_S_PB_READ_FILE_VALUE(x) ((x)->need_read_file)


#define GET_S_PB_CUR_MODE(x) ((x)->cur_mode)

#define SET_S_PB_CUR_MODE(x,value) ((x)->cur_mode = value)

#define SET_S_PB_MGT_A_CH_NUM(pb ,x)  (pb->audio_chan_num = x)

#define SET_S_PB_MGT_V_CH_NUM(pb ,x)  (pb->video_chan_num = x)

#define GET_S_PB_MGT_V_CH_NUM(pb )  (pb->video_chan_num)


static inline void add_single_pb_mgt_v_count( single_pb_mgt *s_pb_mgt)
{
    ++s_pb_mgt->final_v_frame_count;
}

static inline void add_single_pb_mgt_a_count( single_pb_mgt *s_pb_mgt)
{
    ++s_pb_mgt->final_a_frame_count;
}

static inline int get_single_pb_mgt_v_count( single_pb_mgt *s_pb_mgt)
{
    return s_pb_mgt->final_v_frame_count;
}

static inline int get_single_pb_mgt_a_count( single_pb_mgt *s_pb_mgt)
{
    return s_pb_mgt->final_a_frame_count;
}

static inline void set_single_pb_mgt_v_tsp(single_pb_mgt *s_pb_mgt,unsigned int v_tsp)
{

    s_pb_mgt->v_first_tsp = (s_pb_mgt->v_first_tsp < v_tsp ? s_pb_mgt->v_first_tsp :v_tsp);
  
}

static inline void set_single_pb_mgt_a_tsp(single_pb_mgt *s_pb_mgt,unsigned int a_tsp)
{


      s_pb_mgt->a_first_tsp = (s_pb_mgt->a_first_tsp < a_tsp ? s_pb_mgt->a_first_tsp :a_tsp);

}

static inline unsigned int get_single_pb_mgt_v_tsp(single_pb_mgt *s_pb_mgt)
{
    return s_pb_mgt->v_first_tsp;
}



static inline unsigned int get_single_pb_mgt_a_tsp(single_pb_mgt *s_pb_mgt)
{
    return s_pb_mgt->a_first_tsp;
}


static inline void reg_disframe_policy(single_pb_mgt *s_pb_mgt ,void (*disFramePolicy)(struct _single_pb_mgt *s_pb_mgt) )
{
    s_pb_mgt->disFramePolicy = disFramePolicy;
}


static inline void set_net_fd_to_s_pb_mgt(int fd,single_pb_mgt *s_pb_mgt )
{
    s_pb_mgt->cur_net_fd = fd;
}

static inline int get_s_pb_mgt_cur_net_fd(single_pb_mgt *s_pb_mgt)
{
    return s_pb_mgt->cur_net_fd;
}

static inline void set_file_fd_to_s_pb_mgt(int fd,single_pb_mgt *s_pb_mgt )
{
    s_pb_mgt->cur_file_fd = fd;
}



static inline int get_s_pb_mgt_cur_file_fd(single_pb_mgt *s_pb_mgt)
{
    return s_pb_mgt->cur_file_fd;
}

static inline void reg_s_pb_mgt_read_av_file(single_pb_mgt *s_pb_mgt , int (*read_av_file)(struct _single_pb_mgt *s_pb_mgt ,void *arg), void * arg)
{
    s_pb_mgt->arg  = arg;
    s_pb_mgt->read_av_file = read_av_file;
}

static inline void only_reg_read_cb_func(single_pb_mgt *s_pb_mgt , int (*read_av_file)(struct _single_pb_mgt *s_pb_mgt ,void *arg))
{
    s_pb_mgt->read_av_file = read_av_file;
}


static inline void unreg_s_pb_mgt_read_cb(single_pb_mgt *s_pb_mgt )
{
    s_pb_mgt->read_av_file = NULL;
}
static inline int get_s_pb_mgt_disply_ch(single_pb_mgt *s_pb_mgt)
{
    return s_pb_mgt->disply_ch;
}

static inline void set_s_pb_mgt_disply_ch(int disply_ch,single_pb_mgt *s_pb_mgt )
{
    s_pb_mgt->disply_ch = disply_ch;
}


extern single_pb_mgt *get_s_pb_mgt_by_file_fd( int fd );

extern single_pb_mgt *get_s_pb_mgt_by_disply_chan( int chan);

extern single_pb_mgt *malloc_and_init_single_pb_mgt( void );


extern pb_list *get_glb_pb_list( void );

extern single_pb_mgt *get_s_pb_mgt_by_chan( int chan);

extern void init_glb_pb_list( void );

extern void wait_for_release_over( void );

extern void wakeup_release_over_wait(void);

extern void set_pb_release_over_value(int i);

extern void put_single_pb_mgt_to_pb_list(pb_list *pblist, single_pb_mgt *s_pb_mgt_p);

extern single_pb_mgt *get_s_pb_mgt_from_pb_list( pb_list *pblist );

extern void release_pb_thr_list( pb_list *pblist );

extern int  get_idle_audio_chan_num( pb_list *pbList);

extern void set_audio_chan_num( pb_list *pbList,int chNum);

extern void clear_audio_chan_num( pb_list *pbList,int chNum);

extern int  get_idle_video_chan_num( pb_list *pbList);

extern int  get_idle_video_chan_num( pb_list *pbList);

extern void set_video_chan_num( pb_list *pbList,int chNum);

extern void clear_video_chan_num( pb_list *pbList,int chNum);

extern void reg_s_pb_mgt_to_pb_list(single_pb_mgt *s_pb_mgt ,pb_list *pblist);

extern single_pb_mgt *get_s_pb_mgt_by_enc_id(unsigned int chip_id, unsigned int chan_id);



extern void put_v_pck_to_pb_mgt_vlist(single_pb_mgt *s_pb_mgt ,struct NET_DATA_NODE_T *data_node);

extern void put_a_pck_to_pb_mgt_alist(single_pb_mgt *s_pb_mgt ,struct NET_DATA_NODE_T *data_node);

extern struct NET_DATA_NODE_T * get_v_data_node_pb_mgt_vlist(single_pb_mgt *s_pb_mgt );

extern struct NET_DATA_NODE_T * get_a_data_node_pb_mgt_alist(single_pb_mgt *s_pb_mgt );

extern void s_pb_mgt_v_list_release( single_pb_mgt *s_pb_mgt );

extern void s_pb_mgt_a_list_release( single_pb_mgt *s_pb_mgt );

extern void release_s_pb_mgt_av_buff_list( single_pb_mgt *s_pb_mgt );

extern void del_single_pb_mgt_from_pb_list(single_pb_mgt *s_pb_mgt);

#ifdef __cplusplus
}
#endif

#endif
