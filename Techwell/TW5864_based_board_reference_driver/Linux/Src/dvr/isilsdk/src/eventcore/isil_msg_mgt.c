#include <stdlib.h>
#include <stdio.h>

#include "isil_msg_mgt.h"
#include "isil_debug.h"

static struct ISIL_MSG_MGT glb_msg_mgt;

void  msg_blk_release(struct ISIL_MSG_BLK *blk)
{
   if( blk ) {
            
            if(!blk->ref) {
                DEBUG_FUNCTION();
                return;
            }

            minus_msg_blk_ref(blk);

            if( !blk->ref ) {
                DEBUG_FUNCTION();
                pthread_mutex_destroy(&blk->user_lock);
                pthread_cond_destroy(&blk->user_cond);
                DEBUG_FUNCTION();
                free(blk);
                blk = NULL;
            }
        
    } 
}

void msg_blk_wait(struct ISIL_MSG_BLK *blk)
{
    if(blk) {
        // not need ,blk->is_ok = 0 ;
        pthread_mutex_lock(&blk->user_lock);

        while( !blk->is_ok ) {
            pthread_cond_wait( &blk->user_cond ,&blk->user_lock );
        }
        pthread_mutex_unlock(&blk->user_lock);
    }
}



void msg_blk_weekup(struct ISIL_MSG_BLK *blk)
{
    if(blk) {
        
        pthread_mutex_lock(&blk->user_lock);
        DEBUG_FUNCTION();
        blk->is_ok = 1 ;
        pthread_cond_signal(&blk->user_cond);
        DEBUG_FUNCTION();
        pthread_mutex_unlock(&blk->user_lock);
    }
}


static void msg_blk_init(struct ISIL_MSG_BLK *blk)
{
    if(blk) {
        blk->result = -1;
        pthread_mutex_init(&blk->user_lock,NULL);
        INIT_LIST_HEAD(&blk->entry);
        pthread_cond_init(&blk->user_cond,NULL);
        blk->release = msg_blk_release;
        blk->wait = msg_blk_wait;
        blk->weekup = msg_blk_weekup;
    }
}

struct ISIL_MSG_BLK *msg_blk_minit( void )
{
    struct ISIL_MSG_BLK *blk =(struct ISIL_MSG_BLK *)calloc( 1 , sizeof(struct ISIL_MSG_BLK));
    if(blk) {
        msg_blk_init(blk);
        add_msg_blk_ref(blk);
    }

    return blk;
}

static struct ISIL_MSG_BLK *get_msg_blk_from_list_head(struct ISIL_MSG_MGT *msg_mgt)
{
    struct ISIL_MSG_BLK *blk = NULL;

    if(msg_mgt) {

        if(!list_empty(&msg_mgt->msg_list)) {
            pthread_mutex_lock(&msg_mgt->list_lock);
            blk = list_entry(msg_mgt->msg_list.next,struct ISIL_MSG_BLK,entry);
            list_del_init(&blk->entry);
            minus_msg_blk_ref(blk);
            --msg_mgt->msg_num;
            blk->root = NULL;
            pthread_mutex_unlock(&msg_mgt->list_lock);

        }
    }

    return blk;
}


static void put_blk_to_msg_lst(struct ISIL_MSG_BLK *blk , struct ISIL_MSG_MGT *msg_mgt)
{
    if( blk && msg_mgt) {
        pthread_mutex_lock(&msg_mgt->list_lock);
        DEBUG_FUNCTION();
        list_add_tail(&blk->entry, &msg_mgt->msg_list);
        add_msg_blk_ref(blk);
        ++msg_mgt->msg_num;
        blk->root = msg_mgt;
        pthread_mutex_unlock(&msg_mgt->list_lock);
        DEBUG_FUNCTION();
    }
}

static void msg_mgt_release(struct ISIL_MSG_MGT *msg_mgt)
{
    struct list_head *ln1, *ln2;
    struct ISIL_MSG_BLK *blk = NULL;

    if( msg_mgt ) {
        if(!list_empty(&msg_mgt->msg_list)) {

            pthread_mutex_lock(&msg_mgt->list_lock);


            list_for_each_safe(ln1, ln2, &msg_mgt->msg_list ){
            
                blk = list_entry( ln1 ,struct ISIL_MSG_BLK , entry);
            

                if( blk ) {
                    list_del_init(&blk->entry);
                    --msg_mgt->msg_num;
                    blk->root = NULL;
                    //TODO: need free?or weekup ,release it from outer
                    if( blk->weekup ) {
                        blk->weekup(blk);
                    }
                    blk->release(blk);
                    
                }
            }
            pthread_mutex_unlock(&msg_mgt->list_lock);
        }

        pthread_mutex_destroy(&msg_mgt->list_lock);
        if(msg_mgt->alloc_type == ISIL_DYNAMIC_ALLOC_TYPE) {
            free(msg_mgt);
            msg_mgt = NULL;
        }
    }

}


static void notify_reg_msg(struct ISIL_MSG_MGT *msg_mgt)
{
    int ret;
    struct list_head *ln1, *ln2;

    struct ISIL_MSG_BLK *blk;

    pthread_mutex_lock(&msg_mgt->list_lock);
    if(list_empty(&msg_mgt->msg_list)) {
        pthread_mutex_unlock(&msg_mgt->list_lock);
        return ;
    }

    list_for_each_safe(ln1, ln2, &msg_mgt->msg_list){
        blk = list_entry(ln1,struct ISIL_MSG_BLK ,entry);
        if( blk ) {
            list_del_init(&blk->entry);
            --msg_mgt->msg_num;
            blk->root = NULL;
            if(blk->hook) {
                
                ret = blk->hook(blk->param);
                SET_MSG_BLK_RESULT(blk,ret);
                if(blk->weekup) {
                    blk->weekup(blk);
                }
                blk->release(blk);
                
            }
            //TODO : need release ? no
            
        }
    }
    pthread_mutex_unlock(&msg_mgt->list_lock);

}



struct ISIL_MSG_MGT *msg_mgt_minit( void )
{
    struct ISIL_MSG_MGT *msg_mgt =(struct ISIL_MSG_MGT *)calloc( 1 , sizeof(struct ISIL_MSG_MGT));
    if(msg_mgt) {
        pthread_mutex_init(&msg_mgt->list_lock,NULL);
        INIT_LIST_HEAD(&msg_mgt->msg_list);
        msg_mgt->msg_num = 0 ;
        msg_mgt->alloc_type = ISIL_DYNAMIC_ALLOC_TYPE;
        msg_mgt->get_blk_from_head = get_msg_blk_from_list_head;
        msg_mgt->put_blk         = put_blk_to_msg_lst;
        msg_mgt->release         = msg_mgt_release;
        msg_mgt->notify           =  notify_reg_msg;

        msg_mgt->is_init           = 1 ;
    }

    return msg_mgt;
}


int msg_mgt_sinit( struct ISIL_MSG_MGT *msg_mgt )
{
     
    if(msg_mgt) {
        if(!msg_mgt->is_init) {

            pthread_mutex_init(&msg_mgt->list_lock,NULL);
            INIT_LIST_HEAD(&msg_mgt->msg_list);
            msg_mgt->msg_num = 0 ;
            msg_mgt->alloc_type = ISIL_STATIC_ALLOC_TYPE;
    
            msg_mgt->get_blk_from_head = get_msg_blk_from_list_head;
            msg_mgt->put_blk         = put_blk_to_msg_lst;
            msg_mgt->release         = msg_mgt_release;
            msg_mgt->notify           =  notify_reg_msg;

            msg_mgt->is_init = 1;
        }

        return 0;
    }

    return -1;
}

void glb_msg_mgt_init( void )
{
    msg_mgt_sinit(&glb_msg_mgt);
}

struct ISIL_MSG_MGT *get_glb_msg_mgt( void )
{
    return &glb_msg_mgt;
}
