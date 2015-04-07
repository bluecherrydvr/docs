#include <stdlib.h>
#include <assert.h>

#include "isil_frame.h"
#include "isil_sdk_config.h"
#include "isil_debug.h"

static struct ISIL_FramePool glbFramePool;


static void frame_reset(frame_t *frame)
{
    frame->len = 0;
    frame->ref = 0;
    frame->chanId = 0;
    frame->Type = 0;
}



static frame_t * try_get_frame_from_pool(FramePool* frame_pool)
{
    frame_t *frame = NULL;
    pthread_mutex_lock(&frame_pool->lock);

    if(!list_empty(&frame_pool->framesList)) {

        frame = list_entry(frame_pool->framesList.next,frame_t,entry);
        list_del_init(&frame->entry);
        add_frame_ref(frame);
        --frame_pool->framesNum;
    }

    pthread_mutex_unlock(&frame_pool->lock);
    return frame;
}


static frame_t * get_frame_from_pool(FramePool* frame_pool)
{


    frame_t *frame = NULL;
    
    pthread_mutex_lock(&frame_pool->lock);
     while( frame_pool->framesNum == 0 ) {
         pthread_cond_wait(&frame_pool->cond,&frame_pool->lock);
     }
     
     frame = list_entry(frame_pool->framesList.next,frame_t,entry);
     list_del_init(&frame->entry);
     add_frame_ref(frame);
     --frame_pool->framesNum;
     pthread_mutex_unlock(&frame_pool->lock);
     return frame;
}


static frame_t * get_hd_frame_from_pool(FramePool* frame_pool)
{


    frame_t *frame = NULL;

    if( !frame_pool->hd_max_buff_num ) {
        return NULL;
    }
    
    pthread_mutex_lock(&frame_pool->hdLock);
     while( frame_pool->hdFramesNum == 0 ) {
         pthread_cond_wait(&frame_pool->hdCond,&frame_pool->hdLock);
     }
     
     frame = list_entry(frame_pool->hdFramesList.next,frame_t,entry);
     list_del_init(&frame->entry);
     add_frame_ref(frame);
     --frame_pool->hdFramesNum;
     pthread_mutex_unlock(&frame_pool->hdLock);
     return frame;
}


static frame_t * try_get_hd_frame_from_pool(FramePool* frame_pool)
{
    frame_t *frame = NULL;

    pthread_mutex_lock(&frame_pool->hdLock);

    if(!list_empty(&frame_pool->hdFramesList)) {

        frame = list_entry(frame_pool->hdFramesList.next,frame_t,entry);
        list_del_init(&frame->entry);
        add_frame_ref(frame);
        --frame_pool->hdFramesNum;
    }

    pthread_mutex_unlock(&frame_pool->hdLock);

    return frame;
}


static void put_frame_to_pool(FramePool* frame_pool,frame_t *frame)
{
    //DEBUG_FUNCTION();
    pthread_mutex_lock(&frame_pool->lock);
    
    frame_reset(frame);
    list_add_tail(&frame->entry, &frame_pool->framesList);
    ++frame_pool->framesNum;
    //DEBUG_FUNCTION();
    if(frame_pool->framesNum == 1) {

        //DEBUG_FUNCTION();
        pthread_cond_signal(&frame_pool->cond);
       // DEBUG_FUNCTION();
    }

    pthread_mutex_unlock(&frame_pool->lock);

}


static void put_hd_frame_to_pool(FramePool* frame_pool,frame_t *frame)
{
    

    pthread_mutex_lock(&frame_pool->hdLock);
    frame_reset(frame);
    list_add_tail(&frame->entry, &frame_pool->hdFramesList);
    ++frame_pool->hdFramesNum;
    
    if(frame_pool->hdFramesNum == 1) {
       
        pthread_cond_signal(&frame_pool->hdCond);
       
    }

    pthread_mutex_unlock(&frame_pool->hdLock);

}

static void frame_pool_release(FramePool* frame_pool)
{
    frame_t* frame;
    struct list_head *ln1, *ln2;

    
    DEBUG_FUNCTION();
    pthread_mutex_lock(&frame_pool->lock);
    DEBUG_FUNCTION();
    
    if(frame_pool->max_buff_num) {
        while( frame_pool->framesNum != frame_pool->max_buff_num ) {
             
             fprintf( stderr,"frame_pool framesNum[%d],max_buff_num[%d].\n",frame_pool->framesNum , frame_pool->max_buff_num);
             pthread_cond_wait(&frame_pool->cond,&frame_pool->lock);
             
        }
    
       
        //fprintf( stderr,"frame_pool framesNum[%d],max_buff_num[%d].\n",frame_pool->framesNum , frame_pool->max_buff_num);
        DEBUG_FUNCTION();
        list_for_each_safe(ln1, ln2, &frame_pool->framesList){
           // DEBUG_FUNCTION();
            frame = list_entry(ln1,frame_t ,entry);
            if( frame ) {
    
                frame_reset(frame);
                frame->root = NULL;
                INIT_LIST_HEAD(&frame->entry);
                pthread_mutex_destroy(&frame->ref_lock);
                free(frame);
                frame = NULL;
    
            }
    
        }
    
    
    
    
        //DEBUG_FUNCTION();
        pthread_mutex_unlock(&frame_pool->lock);
    }

    
    if(frame_pool->hd_max_buff_num) {
        pthread_mutex_lock(&frame_pool->hdLock);
        DEBUG_FUNCTION();
    #if 1
        while( frame_pool->hdFramesNum != frame_pool->hd_max_buff_num ) {
             
             pthread_cond_wait(&frame_pool->hdCond,&frame_pool->hdLock);
             
        }
    #endif
       
        //fprintf( stderr,"frame_pool framesNum[%d],max_buff_num[%d].\n",frame_pool->framesNum , frame_pool->max_buff_num);
        DEBUG_FUNCTION();
        list_for_each_safe(ln1, ln2, &frame_pool->hdFramesList){
            DEBUG_FUNCTION();
            frame = list_entry(ln1,frame_t ,entry);
            if( frame ) {
    
                frame_reset(frame);
                frame->root = NULL;
                INIT_LIST_HEAD(&frame->entry);
                pthread_mutex_destroy(&frame->ref_lock);
                free(frame);
                frame = NULL;
    
            }
    
        }
    
    
        //DEBUG_FUNCTION();
        pthread_mutex_unlock(&frame_pool->hdLock);
    }



    //DEBUG_FUNCTION();
    
    if(frame_pool->max_buff_num) {
        pthread_mutex_destroy(&frame_pool->lock);
        pthread_cond_destroy(&frame_pool->cond);
        INIT_LIST_HEAD(&frame_pool->framesList);
    }

    if(frame_pool->hd_max_buff_num) {

        pthread_mutex_destroy(&frame_pool->hdLock);
        pthread_cond_destroy(&frame_pool->hdCond);
        INIT_LIST_HEAD(&frame_pool->hdFramesList);
    }


    frame_pool->framesNum = 0;
    frame_pool->hdFramesNum = 0;

    if(frame_pool->frames_buff) {
        free(frame_pool->frames_buff);
        frame_pool->frames_buff = NULL;
    }

    if( frame_pool->hd_frame_buff ) {
        free(frame_pool->hd_frame_buff);
        frame_pool->hd_frame_buff = NULL;
    }

}



static void release_frame(frame_t *frame)
{
    //DEBUG_FUNCTION();
    if( !frame ) {
        return;
    }
    pthread_mutex_lock(&frame->ref_lock);
    //DEBUG_FUNCTION();
    if( !frame->ref ) {
        pthread_mutex_unlock(&frame->ref_lock);
        assert(0);
        return;
    }
    
    --frame->ref;
   // DEBUG_FUNCTION();
    if(frame->ref == 0) {
        if(frame->root) {
     //       DEBUG_FUNCTION();
            put_frame_to_pool( frame->root ,frame);
     //       DEBUG_FUNCTION();
            
        }
        else{
            assert(0);
        }
    }
   // DEBUG_FUNCTION();
    pthread_mutex_unlock(&frame->ref_lock);
    
   // DEBUG_FUNCTION();
}



static void release_hd_frame(frame_t *frame)
{
    
    if( !frame ) {
        return;
    }
    pthread_mutex_lock(&frame->ref_lock);
    
    if( !frame->ref ) {
        pthread_mutex_unlock(&frame->ref_lock);
        assert(0);
        return;
    }
    
    --frame->ref;
   
    if(frame->ref == 0) {
        if(frame->root) {
            put_hd_frame_to_pool( frame->root ,frame);
        }
        else{
            assert(0);
        }
    }
   
    pthread_mutex_unlock(&frame->ref_lock);
    
  
}


int init_hd_frame_pool(FramePool* frame_pool , unsigned int frame_num , unsigned int frame_size)
{
    
    unsigned int i ;
    frame_t *frame;

    if( !frame_pool ) {
        return -1;
    }

    if( !frame_num || !frame_size) {
    
        return -1;
    }

    

    if( frame_pool->max_buff_size ) {
        if( frame_size <=  frame_pool->max_buff_size ) {
            return -1;
        }
    }
    
    frame_pool->hd_max_buff_num = frame_num;
    pthread_mutex_init(&frame_pool->hdLock,NULL);
    INIT_LIST_HEAD(&frame_pool->hdFramesList);
    pthread_cond_init(&frame_pool->hdCond,NULL);

    frame_pool->hd_frame_buff = (unsigned char *)malloc(frame_num *frame_size);
    if( !frame_pool->hd_frame_buff ) {
         return -1;
    }

    for( i = 0 ; i < frame_num ; i++ ) {
        frame = (frame_t *)malloc(sizeof( *frame ));
        if( !frame ) {

            free(frame_pool->frames_buff );
            frame_pool->frames_buff = NULL;
            return -1;

        }

        frame->root = frame_pool;
        frame->release = release_hd_frame;
        frame->ref = 0;


        pthread_mutex_init(&frame->ref_lock,NULL);
        INIT_LIST_HEAD(&frame->entry);
        frame_reset(frame);
        frame->buff = (frame_pool->hd_frame_buff + i *frame_size);

        put_hd_frame_to_pool(frame_pool,frame);

    }


    return 0;
}


int init_frame_pool(FramePool* frame_pool , unsigned int frame_num , unsigned int frame_size)
{
  //  DEBUG_FUNCTION();
    unsigned int i ;
    frame_t *frame;

//    unsigned char *tmp;
    

    if( !frame_num || !frame_size) {
        /*TODO:notify user set num and frame_size*/
   //     DEBUG_FUNCTION();
        return -1;
    }
  //  DEBUG_FUNCTION();
    if( frame_size < MINFRAMEBUFFSIZE) {
        return -1;
    }

    frame_pool->max_buff_num = frame_num ;
    frame_pool->max_buff_size  = frame_size;

    frame_pool->frames_buff = NULL;
    
 //   DEBUG_FUNCTION(); 
    pthread_mutex_init(&frame_pool->lock,NULL);
	INIT_LIST_HEAD(&frame_pool->framesList);
    pthread_cond_init(&frame_pool->cond,NULL);
    
    frame_pool->framesNum = 0;
    
    frame_pool->release = frame_pool_release;

    
    
    frame_pool->frames_buff = (unsigned char *)malloc(frame_num *frame_size);
    if( !frame_pool->frames_buff ) {
        //TODO:notify user buff not enough
 //       DEBUG_FUNCTION();
        return -1;
    }
//    DEBUG_FUNCTION();
    for(i = 0 ; i < frame_num ; i++) {
        frame = (frame_t *)malloc(sizeof( *frame ));
  //      DEBUG_FUNCTION();
        if( !frame ) {
    //        DEBUG_FUNCTION();
            free(frame_pool->frames_buff );
            frame_pool->frames_buff = NULL;
      //      DEBUG_FUNCTION();
            return -1;
        }

        frame->root = frame_pool;
        frame->release = release_frame;
        frame->ref = 0;

        pthread_mutex_init(&frame->ref_lock,NULL);
        INIT_LIST_HEAD(&frame->entry);
        frame_reset(frame);
        frame->buff = (frame_pool->frames_buff + i *frame_size);
//        add_frame_ref(frame);
      //  DEBUG_FUNCTION();
        put_frame_to_pool(frame_pool,frame);
      //  DEBUG_FUNCTION();
    }

    //debug_glb_frame_pool();

    return 0;
}





int init_glb_frame_pool( unsigned int frame_num , unsigned int frame_size)
{
   
    int ret = 0;

    if( !glbFramePool.is_init ) {

        ret = init_frame_pool(&glbFramePool,frame_num,frame_size);
        if( ret == 0) {
            glbFramePool.is_init  = 1;
        }
    }
    return ret;
}

int init_glb_frame_pool_ex( void )
{
    int ret = 0;

    if( !glbFramePool.is_init ){
        
        if( glbFramePool.max_buff_num && glbFramePool.max_buff_size) {
          //  DEBUG_FUNCTION();
            ret = init_frame_pool(&glbFramePool,glbFramePool.max_buff_num,glbFramePool.max_buff_size);
        }
        else{
          //  DEBUG_FUNCTION();
            ret = init_frame_pool(&glbFramePool,DEFAULT_FRAME_NUMS,DEFAULT_FRAME_BUFF_SIZE);
        }

        if( ret == 0 ) {
            glbFramePool.is_init  = 1;
        }

    }
  //  DEBUG_FUNCTION();
    return ret;
}






struct ISIL_FramePool * get_glb_frame_pool( void )
{
    return &glbFramePool;
}



static frame_t* get_frame_from_date_mgt_header(struct ISIL_DATE_MGT *date_mgt)
{
    frame_t *frame;
    pthread_mutex_lock(&date_mgt->lock);

    while(!date_mgt->framesNum ) {

         pthread_cond_wait(&date_mgt->cond,&date_mgt->lock);
    }

     frame = list_entry(date_mgt->framesList.next,frame_t,entry);
     list_del_init(&frame->entry);
//     minus_frame_ref(frame);
     --date_mgt->framesNum;
     pthread_mutex_unlock(&date_mgt->lock);
     return frame;
}


static frame_t* try_get_frame_from_date_mgt_header(struct ISIL_DATE_MGT *date_mgt)
{
    frame_t *frame;
    pthread_mutex_lock(&date_mgt->lock);

    if(list_empty(&date_mgt->framesList)) {

        pthread_mutex_unlock(&date_mgt->lock);
        return NULL;

    }

     frame = list_entry(date_mgt->framesList.next,frame_t,entry);
     list_del_init(&frame->entry);
//     minus_frame_ref(frame);
     --date_mgt->framesNum;
     pthread_mutex_unlock(&date_mgt->lock);
     return frame;
}





static void put_frame_to_date_mgt_tail(struct ISIL_DATE_MGT *date_mgt ,frame_t *frame)
{
    if(date_mgt && frame) {
        
        
    
        pthread_mutex_lock(&date_mgt->lock);
        
        
        list_add_tail(&frame->entry, &date_mgt->framesList);
        ++date_mgt->framesNum;
//        add_frame_ref(frame);
            
        pthread_mutex_unlock(&date_mgt->lock);
    }
}

static void release_date_mgt(struct ISIL_DATE_MGT *date_mgt)
{

    frame_t *frame;
    
    struct list_head *ln1, *ln2;
    
    if( !date_mgt ) {
        return;
    }
    
    pthread_mutex_lock(&date_mgt->lock);

     if( !list_empty(&date_mgt->framesList)) {

        list_for_each_safe(ln1, ln2, &date_mgt->framesList){

            frame = list_entry(ln1,frame_t ,entry);
            if( frame ) {
                frame->release(frame);
            }
 

        }
     }


    pthread_mutex_unlock(&date_mgt->lock);


    pthread_mutex_destroy(&date_mgt->lock);
    pthread_cond_destroy(&date_mgt->cond);
    INIT_LIST_HEAD(&date_mgt->framesList);

    if(date_mgt->alloc_type == ISIL_DYNAMIC_ALLOC_TYPE) {

        free(date_mgt);
        date_mgt = NULL;
    }
}



static void date_mgt_init(struct ISIL_DATE_MGT *date_mgt)
{

    if(date_mgt) {

        pthread_mutex_init(&date_mgt->lock,NULL);
        INIT_LIST_HEAD(&date_mgt->framesList);
        pthread_cond_init(&date_mgt->cond,NULL);
        date_mgt->framesNum = 0;
        date_mgt->get_frame_header = get_frame_from_date_mgt_header;
        date_mgt->try_get_frame_header = try_get_frame_from_date_mgt_header;
        date_mgt->put_frame_tail   = put_frame_to_date_mgt_tail;
        date_mgt->release          = release_date_mgt;
    }
}

void date_mgt_sinit(struct ISIL_DATE_MGT *date_mgt)
{
    date_mgt_init(date_mgt);
    date_mgt->alloc_type = ISIL_STATIC_ALLOC_TYPE;

}


struct ISIL_DATE_MGT* date_mgt_minit( void )
{
    struct ISIL_DATE_MGT *date_mgt;
    date_mgt = (struct ISIL_DATE_MGT *)calloc(1,sizeof(struct ISIL_DATE_MGT));
    if(!date_mgt) {
        return NULL;
    }

    date_mgt_init(date_mgt);
    date_mgt->alloc_type = ISIL_DYNAMIC_ALLOC_TYPE;

    return date_mgt;
}


frame_t* get_frame_from_glb_pool(void)
{
    return get_frame_from_pool(&glbFramePool);
}




frame_t* try_get_frame_from_glb_pool(void)
{
    return try_get_frame_from_pool(&glbFramePool);
}



frame_t* try_get_hd_frame_from_glb_pool(void)
{
    return try_get_hd_frame_from_pool(&glbFramePool);
}


frame_t* get_hd_frame_from_glb_pool(void)
{
    return get_hd_frame_from_pool(&glbFramePool);
}


int init_glb_hd_frame_pool( unsigned int frame_num , unsigned int frame_size)
{
   
    if( !glbFramePool.is_init ) {

        return -1;
    }

    return init_hd_frame_pool(&glbFramePool,frame_num,frame_size);
    
}




void debug_glb_frame_pool( void )
{
    frame_t *frame;
    FramePool* frame_pool;

    frame_pool = get_glb_frame_pool();
    
    struct list_head *ln1, *ln2;

    if(list_empty(&frame_pool->framesList)){
        return ;
    }

    list_for_each_safe(ln1, ln2, &frame_pool->framesList){
       // DEBUG_FUNCTION();
        frame = list_entry(ln1,frame_t ,entry);
        if( frame ) {

            fprintf( stderr ,"frame addr[%p] ,buff addr[%p].\n",frame, frame->buff);

        }

    }

}

