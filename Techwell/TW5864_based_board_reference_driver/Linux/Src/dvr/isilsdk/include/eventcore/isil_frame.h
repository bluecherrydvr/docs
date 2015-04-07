#ifndef _ISIL_FRAME_H
#define _ISIL_FRAME_H



#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include "linux_list.h"
#include "isil_sdk_config.h"



typedef struct ISIL_Frame frame_t;
typedef struct ISIL_FramePool FramePool;


struct ISIL_Frame{
    struct list_head entry;
    pthread_mutex_t ref_lock;
    int ref;
    unsigned int len;
    unsigned short chanId;
    unsigned short Type;
    unsigned char *buff;
    FramePool         *root;
    void (*release)(frame_t *frame);
};




struct ISIL_FramePool{
    struct list_head framesList;
    unsigned int framesNum;
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    int is_init;
    unsigned int max_buff_size;
    unsigned int max_buff_num;


	struct list_head hdFramesList;
    unsigned int hdFramesNum;
    pthread_mutex_t hdLock;
    pthread_cond_t  hdCond;
	unsigned int hd_max_buff_num;

	unsigned char* hd_frame_buff;
    
    unsigned char* frames_buff;
    void     (*release)(FramePool* frame_pool);
};



struct ISIL_DATE_MGT{
  struct list_head framesList;
  unsigned int framesNum;
  pthread_mutex_t lock; 
  pthread_cond_t  cond; 
  
  int alloc_type;

  frame_t* (*get_frame_header)(struct ISIL_DATE_MGT *date_mgt);
  frame_t* (*try_get_frame_header)(struct ISIL_DATE_MGT *date_mgt);
  void (*put_frame_tail)(struct ISIL_DATE_MGT *date_mgt ,frame_t *frame);
  void (*release)(struct ISIL_DATE_MGT *date_mgt);
};


inline static void add_frame_ref(frame_t *frame)
{
    pthread_mutex_lock(&frame->ref_lock);
    ++frame->ref;
    pthread_mutex_unlock(&frame->ref_lock);
}

inline static void minus_frame_ref(frame_t *frame)
{
    pthread_mutex_lock(&frame->ref_lock);
    if( frame->ref > 0 ) {
        --frame->ref;
    }
    pthread_mutex_unlock(&frame->ref_lock);
}


extern int init_frame_pool(FramePool* frame_pool , unsigned int frame_num , unsigned int frame_size);

extern int init_glb_frame_pool( unsigned int frame_num , unsigned int frame_size);

extern int init_glb_frame_pool_ex( void );

extern struct ISIL_FramePool *get_glb_frame_pool( void );

extern void date_mgt_sinit(struct ISIL_DATE_MGT *date_mgt);

extern struct ISIL_DATE_MGT* date_mgt_minit( void );

extern frame_t * get_frame_from_glb_pool(void);

extern frame_t * try_get_frame_from_glb_pool(void);

extern void debug_glb_frame_pool( void );


extern frame_t* get_hd_frame_from_glb_pool(void);

extern frame_t* try_get_hd_frame_from_glb_pool(void);

extern int init_glb_hd_frame_pool( unsigned int frame_num , unsigned int frame_size);


#ifdef __cplusplus
}
#endif

#endif
