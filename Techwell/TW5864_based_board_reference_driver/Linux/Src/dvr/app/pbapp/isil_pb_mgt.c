#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "isil_pb_mgt.h"
#include "isil_decode_api.h"
#include "config.h"

static pb_list dec_pb_list;


void init_pb_list( pb_list *pblist )
{

    INIT_LIST_HEAD(&pblist->pb_thread_list);
    pthread_mutex_init(&pblist->thread_lock,NULL);
    pthread_mutex_init(&pblist->av_chan_lock,NULL);
    pblist->thread_num = 0;
    pblist->dec_v_chan_occupy_t = 0;
    pblist->dec_a_chan_occupy_t = 0;
    pblist->release_over = 0 ;
    pthread_mutex_init(&pblist->release_lock,NULL);
    pthread_cond_init(&pblist->release_cond,NULL);
    
}


pb_list *get_glb_pb_list( void )
{
    return &dec_pb_list;
}

void init_glb_pb_list( void )
{
    init_pb_list(get_glb_pb_list());
}


void wait_for_release_over( void )
{

    pb_list *pbList = get_glb_pb_list();
    pthread_mutex_lock(&pbList->release_lock);

    while(pbList->release_over == 0) {
        pthread_cond_wait(&pbList->release_cond,&pbList->release_lock);
    }

    pthread_mutex_unlock(&pbList->release_lock);
    
}


void wakeup_release_over_wait(void)
{
    pb_list *pbList = get_glb_pb_list();
    pbList->release_over = 1 ;
    pthread_cond_signal(&pbList->release_cond);
        
}


void set_pb_release_over_value(int i)
{
    pb_list *pbList = get_glb_pb_list();
    pbList->release_over = i;
}


static void release_single_pb_mgt(single_pb_mgt *s_pb_mgt)
{
    int ret;
    struct NET_DATA_MGT_T *data_mgt;
    struct ISIL_MSG_MGT *msg_list;

        if(s_pb_mgt) {

      
            
            ret = close_video_dec_chan(s_pb_mgt);
            if( ret < 0) {
                fprintf( stderr,"close_video_dec_chan err .\n");
            }

            ret = close_audio_dec_chan(s_pb_mgt);
            if( ret < 0) {
                fprintf( stderr,"close_audio_dec_chan err .\n");
            }


            del_single_pb_mgt_from_pb_list(s_pb_mgt);

            if(s_pb_mgt->ev_task) {
                if(s_pb_mgt->ev_task->release) {
                    s_pb_mgt->ev_task->release(s_pb_mgt->ev_task);
                }
                s_pb_mgt->ev_task = NULL;
            }
                            
            if( s_pb_mgt->video_list ) {
                data_mgt = s_pb_mgt->video_list;
                if( data_mgt->release ) {
                    data_mgt->release(data_mgt);
                }
                s_pb_mgt->video_list = NULL;
            }
        
            if( s_pb_mgt->audio_list ) {
                data_mgt = s_pb_mgt->audio_list;
                if( data_mgt->release ) {
                    data_mgt->release(data_mgt);
                }
                s_pb_mgt->audio_list = NULL;
            }

            if(s_pb_mgt->msg_list) {
                msg_list = s_pb_mgt->msg_list;
                msg_list->release(msg_list);
                s_pb_mgt->msg_list = NULL;
            }

            free(s_pb_mgt);
            s_pb_mgt = NULL;
    }
}





single_pb_mgt *malloc_and_init_single_pb_mgt( void )
{

    single_pb_mgt *s_pb_mgt_p;
    s_pb_mgt_p = (single_pb_mgt *)calloc(1 , sizeof(single_pb_mgt));
    if( !s_pb_mgt_p ) {
        fprintf(stderr,"calloc s_pb_mgt_p failed .\n");
        return NULL;
    }

    
    s_pb_mgt_p->a_first_tsp -= 1;
    s_pb_mgt_p->v_first_tsp -= 1;

    s_pb_mgt_p->msg_list = msg_mgt_minit();
    if( !s_pb_mgt_p->msg_list ) {
        return NULL;
    }

    s_pb_mgt_p->video_list = net_data_mgt_minit();
    if( !s_pb_mgt_p->video_list ) {
        return NULL;
    }

    s_pb_mgt_p->audio_list = net_data_mgt_minit();
    if( !s_pb_mgt_p->audio_list ) {

        return NULL;
    }

    s_pb_mgt_p->release = release_single_pb_mgt;
    
    return s_pb_mgt_p;
}


void put_single_pb_mgt_to_pb_list(pb_list *pblist, single_pb_mgt *s_pb_mgt_p)
{
    assert(s_pb_mgt_p != NULL);
    pthread_mutex_lock(&pblist->thread_lock);
    list_add_tail(&s_pb_mgt_p->entry, &pblist->pb_thread_list);
    ++pblist->thread_num;
    s_pb_mgt_p->pb_parent = pblist;
    pthread_mutex_unlock(&pblist->thread_lock);
}


single_pb_mgt *get_s_pb_mgt_from_pb_list( pb_list *pblist )
{
    single_pb_mgt* tmp_entry = NULL;
    pthread_mutex_lock( &pblist->thread_lock );

    if( !list_empty(&pblist->pb_thread_list) ) {
        tmp_entry = list_entry(pblist->pb_thread_list.next,single_pb_mgt,entry);
        list_del_init(&tmp_entry->entry);
        --pblist->thread_num;
        tmp_entry->pb_parent = NULL;
    }

    pthread_mutex_unlock( &pblist->thread_lock );
    return tmp_entry;
}

void del_single_pb_mgt_from_pb_list(single_pb_mgt *s_pb_mgt)
{
    pb_list *pblist;

    if(s_pb_mgt) {
        if(s_pb_mgt->pb_parent) {
            pblist = s_pb_mgt->pb_parent;
            pthread_mutex_lock( &pblist->thread_lock );
            list_del_init(&s_pb_mgt->entry);
            --pblist->thread_num;
            s_pb_mgt->pb_parent = NULL;
            pthread_mutex_unlock( &pblist->thread_lock );

        }
    }
}


void release_pb_thr_list( pb_list *pblist )
{
    single_pb_mgt* tmpNode = NULL;
    if(pblist) {

        while (pblist->thread_num) {
            tmpNode= get_s_pb_mgt_from_pb_list(pblist);
            if( tmpNode ) {
                free( tmpNode );
                tmpNode = NULL;
            }
        }

        pthread_mutex_destroy(&pblist->thread_lock);
//        pblist_process_timer_stop(pblist);
    }
}



int  get_idle_audio_chan_num( pb_list *pbList)
{
    unsigned int i;
    pthread_mutex_lock(&pbList->av_chan_lock);
    for (i = 0 ; i < MAX_DEC_CHANS_NUM ; i++) {
        if(CHECK_BIT_VALUE(pbList->dec_a_chan_occupy_t ,i) == 0){
            pthread_mutex_unlock(&pbList->av_chan_lock);
            return i;
        }
    }
    pthread_mutex_unlock(&pbList->av_chan_lock);
    return -1;
}

void set_audio_chan_num( pb_list *pbList,int chNum)
{
    pthread_mutex_lock(&pbList->av_chan_lock);
    pbList->dec_a_chan_occupy_t = SET_BIT_VALUE(pbList->dec_a_chan_occupy_t,chNum);
    pthread_mutex_unlock(&pbList->av_chan_lock);
}

void clear_audio_chan_num( pb_list *pbList,int chNum)
{
    pthread_mutex_lock(&pbList->av_chan_lock);
    pbList->dec_a_chan_occupy_t = CLEAR_BIT_VALUE(pbList->dec_a_chan_occupy_t,chNum);
    pthread_mutex_unlock(&pbList->av_chan_lock);
}


int  get_idle_video_chan_num( pb_list *pbList)
{
    unsigned int i;

    pthread_mutex_lock(&pbList->av_chan_lock);
    for (i = 0 ; i < MAX_DEC_CHANS_NUM ; ++i) {
        if(CHECK_BIT_VALUE(pbList->dec_v_chan_occupy_t ,i) == 0){
            pthread_mutex_unlock(&pbList->av_chan_lock);
            return i;
        }
    }

    pthread_mutex_unlock(&pbList->av_chan_lock);

    return -1;
}


void set_video_chan_num( pb_list *pbList,int chNum)
{
    pthread_mutex_lock(&pbList->av_chan_lock);
    pbList->dec_v_chan_occupy_t = SET_BIT_VALUE(pbList->dec_v_chan_occupy_t,chNum);
    pthread_mutex_unlock(&pbList->av_chan_lock);
}


void clear_video_chan_num( pb_list *pbList,int chNum)
{
    pthread_mutex_lock(&pbList->av_chan_lock);
    pbList->dec_v_chan_occupy_t = CLEAR_BIT_VALUE(pbList->dec_v_chan_occupy_t,chNum);
    pthread_mutex_unlock(&pbList->av_chan_lock);
}



single_pb_mgt *get_s_pb_mgt_by_file_fd( int fd)
{
    struct list_head *ln1, *ln2;
    single_pb_mgt *s_pb_mgt;
    pb_list *glb_pb = get_glb_pb_list();
    assert(glb_pb);


    pthread_mutex_lock(&glb_pb->thread_lock);

    if(list_empty(&glb_pb->pb_thread_list)) {
        pthread_mutex_unlock(&glb_pb->thread_lock);
        return NULL;
    }

    list_for_each_safe(ln1, ln2, &glb_pb->pb_thread_list){
        s_pb_mgt = list_entry(ln1,single_pb_mgt,entry);
        if(s_pb_mgt) {
            if(get_s_pb_mgt_cur_file_fd(s_pb_mgt) == fd) {
                pthread_mutex_unlock(&glb_pb->thread_lock);
                return s_pb_mgt;
            }
        }
    }

    pthread_mutex_unlock(&glb_pb->thread_lock);

    return NULL;
}


single_pb_mgt *get_s_pb_mgt_by_disply_chan( int chan)
{
    struct list_head *ln1, *ln2;
    single_pb_mgt *s_pb_mgt;
    pb_list *glb_pb = get_glb_pb_list();
    assert(glb_pb);

    pthread_mutex_lock(&glb_pb->thread_lock);

    if(list_empty(&glb_pb->pb_thread_list)) {
        pthread_mutex_unlock(&glb_pb->thread_lock);
        return NULL;
    }

    list_for_each_safe(ln1, ln2, &glb_pb->pb_thread_list){
        s_pb_mgt = list_entry(ln1,single_pb_mgt,entry);
        if(s_pb_mgt) {
            if( s_pb_mgt->disply_ch == chan) {
                pthread_mutex_unlock(&glb_pb->thread_lock);
                return s_pb_mgt;
            }
        }
    }

    pthread_mutex_unlock(&glb_pb->thread_lock);

    return NULL;
}


single_pb_mgt *get_s_pb_mgt_by_chan( int chan)
{
    struct list_head *ln1, *ln2;
    single_pb_mgt *s_pb_mgt;
    pb_list *glb_pb = get_glb_pb_list();
    assert(glb_pb);

    pthread_mutex_lock(&glb_pb->thread_lock);

    if(list_empty(&glb_pb->pb_thread_list)) {
        pthread_mutex_unlock(&glb_pb->thread_lock);
        return NULL;
    }

    list_for_each_safe(ln1, ln2, &glb_pb->pb_thread_list){
        s_pb_mgt = list_entry(ln1,single_pb_mgt,entry);
        if(s_pb_mgt) {
            if( s_pb_mgt->is_video_chan_open) {
                if( s_pb_mgt->video_chan_num == chan) {
                    pthread_mutex_unlock(&glb_pb->thread_lock);
                    return s_pb_mgt;
                }
            }
        }
    }

    pthread_mutex_unlock(&glb_pb->thread_lock);

    return NULL;
}


void reg_s_pb_mgt_to_pb_list(single_pb_mgt *s_pb_mgt ,pb_list *pblist)
{
    put_single_pb_mgt_to_pb_list(pblist,s_pb_mgt);
    
}


single_pb_mgt *get_s_pb_mgt_by_enc_id(unsigned int chip_id, unsigned int chan_id)
{
    struct list_head *ln1, *ln2;
    single_pb_mgt *s_pb_mgt;
    pb_list *glb_pb = get_glb_pb_list();

    pthread_mutex_lock(&glb_pb->thread_lock);

    if(list_empty(&glb_pb->pb_thread_list)) {
        pthread_mutex_unlock(&glb_pb->thread_lock);
        return NULL;
    }

    list_for_each_safe(ln1, ln2, &glb_pb->pb_thread_list){
        s_pb_mgt = list_entry(ln1,single_pb_mgt,entry);
        if(s_pb_mgt) {
            if( s_pb_mgt->ev_task) {
                if( (s_pb_mgt->ev_task->phy_chan_id == chan_id) 
                    && (s_pb_mgt->ev_task->phy_chip_id == chip_id) ) {
                    pthread_mutex_unlock(&glb_pb->thread_lock);
                    return s_pb_mgt;
                }
            }
        }
    }

    pthread_mutex_unlock(&glb_pb->thread_lock);

    return NULL;
}


void put_a_pck_to_pb_mgt_alist(single_pb_mgt *s_pb_mgt ,struct NET_DATA_NODE_T *data_node)
{

    struct NET_DATA_MGT_T *data_mgt;
    if( !s_pb_mgt || !data_node) {
        return;
    }
    
    data_mgt = s_pb_mgt->audio_list;
    if( !data_mgt ) {
        fprintf( stderr,"audio data_mgt err .\n");
        return;
    }


    pthread_mutex_lock(&data_mgt->node_lst_lock);
    
    list_add_tail(&data_node->entry, &data_mgt->data_node_list);
    ++data_mgt->node_num;

    data_node->net_data_mgt = data_mgt;
    net_data_node_clone(data_node);

    pthread_mutex_unlock(&data_mgt->node_lst_lock);


}


void put_v_pck_to_pb_mgt_vlist(single_pb_mgt *s_pb_mgt ,struct NET_DATA_NODE_T *data_node)
{

    struct NET_DATA_MGT_T *data_mgt;
    if( !s_pb_mgt || !data_node) {
        return;
    }
    
    data_mgt = s_pb_mgt->video_list;
    if( !data_mgt ) {
        fprintf( stderr,"video data_mgt err .\n");
        return;
    }


    pthread_mutex_lock(&data_mgt->node_lst_lock);
    
    list_add_tail(&data_node->entry, &data_mgt->data_node_list);
    ++data_mgt->node_num;

    data_node->net_data_mgt = data_mgt;
    net_data_node_clone(data_node);

    pthread_mutex_unlock(&data_mgt->node_lst_lock);

}


struct NET_DATA_NODE_T * get_v_data_node_pb_mgt_vlist(single_pb_mgt *s_pb_mgt )
{

    struct NET_DATA_NODE_T *data_node;
    struct NET_DATA_MGT_T *data_mgt;
    if( !s_pb_mgt ) {
        return NULL;
    }
    
    data_mgt = s_pb_mgt->video_list;
    if( !data_mgt ) {
        fprintf( stderr,"video  data_mgt err .\n");
        return NULL;
    }


    pthread_mutex_lock(&data_mgt->node_lst_lock);

    if(data_mgt->node_num == 0 ) {
        pthread_mutex_unlock(&data_mgt->node_lst_lock);
        return NULL;
    }

    
    data_node = list_entry(data_mgt->data_node_list.next,struct NET_DATA_NODE_T,entry);
    list_del_init(&data_node->entry);
    --data_mgt->node_num;
    data_node->net_data_mgt = NULL;
    net_data_node_minus_ref(data_node);
    pthread_mutex_unlock(&data_mgt->node_lst_lock);

    return data_node;


}


struct NET_DATA_NODE_T * get_a_data_node_pb_mgt_alist(single_pb_mgt *s_pb_mgt )
{

    struct NET_DATA_NODE_T *data_node;
    struct NET_DATA_MGT_T *data_mgt;
    if( !s_pb_mgt ) {
        return NULL;
    }
    
    data_mgt = s_pb_mgt->audio_list;
    if( !data_mgt ) {
        fprintf( stderr,"audio  data_mgt err .\n");
        return NULL;
    }


    pthread_mutex_lock(&data_mgt->node_lst_lock);

    if(data_mgt->node_num == 0 ) {
        pthread_mutex_unlock(&data_mgt->node_lst_lock);
        return NULL;
    }

    
    data_node = list_entry(data_mgt->data_node_list.next,struct NET_DATA_NODE_T,entry);
    list_del_init(&data_node->entry);
    --data_mgt->node_num;
    data_node->net_data_mgt = NULL;
    net_data_node_minus_ref(data_node);
    pthread_mutex_unlock(&data_mgt->node_lst_lock);

    return data_node;


}


void s_pb_mgt_v_list_release( single_pb_mgt *s_pb_mgt )
{
    struct NET_DATA_MGT_T *data_mgt;
    struct NET_DATA_NODE_T *data_node;
    struct list_head *ln1, *ln2;

    if(s_pb_mgt && s_pb_mgt->video_list) {
        
        data_mgt = s_pb_mgt->video_list;
        
        pthread_mutex_lock(&data_mgt->node_lst_lock);


        if(!list_empty(&data_mgt->data_node_list)){
            
            list_for_each_safe(ln1, ln2,&data_mgt->data_node_list){
                data_node = list_entry(ln1,struct NET_DATA_NODE_T ,entry);
                if(data_node) {
                    list_del_init(&data_node->entry);
                    --data_mgt->node_num;
                    data_node->net_data_mgt = NULL;
                    net_data_node_minus_ref(data_node);
                    if(data_node->release) {
                        
                        DEBUG_FUNCTION();
                        data_node->release(data_node);
                        DEBUG_FUNCTION();
                    }
                }

            }
            

        }

        pthread_mutex_unlock(&data_mgt->node_lst_lock);
    }
}



void s_pb_mgt_a_list_release( single_pb_mgt *s_pb_mgt )
{
    struct NET_DATA_MGT_T *data_mgt;
    struct NET_DATA_NODE_T *data_node;
    struct list_head *ln1, *ln2;

    if(s_pb_mgt && s_pb_mgt->audio_list) {
        
        data_mgt = s_pb_mgt->audio_list;
        
        pthread_mutex_lock(&data_mgt->node_lst_lock);


        if(!list_empty(&data_mgt->data_node_list)){
            
            list_for_each_safe(ln1, ln2,&data_mgt->data_node_list){
                data_node = list_entry(ln1,struct NET_DATA_NODE_T ,entry);
                if(data_node) {
                    list_del_init(&data_node->entry);
                    --data_mgt->node_num;
                    data_node->net_data_mgt = NULL;
                    net_data_node_minus_ref(data_node);
                    if(data_node->release) {
                        
                        DEBUG_FUNCTION();
                        data_node->release(data_node);
                        DEBUG_FUNCTION();
                    }
                }

            }
            

        }

        pthread_mutex_unlock(&data_mgt->node_lst_lock);
    }
}


void release_s_pb_mgt_av_buff_list( single_pb_mgt *s_pb_mgt )
{

    s_pb_mgt_v_list_release(s_pb_mgt);
    s_pb_mgt_a_list_release(s_pb_mgt);


}









