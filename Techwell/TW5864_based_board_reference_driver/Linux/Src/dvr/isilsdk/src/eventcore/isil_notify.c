#include <stdlib.h>
#include <stdio.h>

#include "isil_notify.h"
#include "isil_debug.h"


static void notify_blk_release(struct ISIL_NOTIFY_BLK *blk)
{

    if( blk ) {

            minus_notify_blk_ref(blk);

            if( !blk->ref ) {
                pthread_mutex_destroy(&blk->entry_lock);
                pthread_cond_destroy(&blk->entry_cond);
                free(blk);
                blk = NULL;
            }
            
    }
      
}

void notify_blk_wait(struct ISIL_NOTIFY_BLK *blk)
{
    if(blk) {
        
        pthread_mutex_lock(&blk->entry_lock);
        // not need ,blk->is_ready = 0 ;
        while( !blk->is_ready ) {
            pthread_cond_wait( &blk->entry_cond ,&blk->entry_lock );
        }
        pthread_mutex_unlock(&blk->entry_lock);
    }
}

void notify_blk_weekup(struct ISIL_NOTIFY_BLK *blk)
{
    if(blk) {
        
        pthread_mutex_lock(&blk->entry_lock);
        blk->is_ready = 1 ;
        pthread_cond_signal(&blk->entry_cond);
        pthread_mutex_unlock(&blk->entry_lock);
    }
}


static void put_blk_to_notify_lst(struct ISIL_NOTIFY_BLK *blk , struct ISIL_NOTIFY_LST *lst)
{
    if( blk && lst) {
//        pthread_mutex_lock(&lst->notify_ev_lock);
        list_add_tail(&blk->entry, &lst->notify_list);
        ++lst->notify_num;
        add_notify_blk_ref(blk);
//        pthread_mutex_unlock(&lst->notify_ev_lock);
    }
}

static struct ISIL_NOTIFY_BLK* get_blk_from_notify_lst_head(struct ISIL_NOTIFY_LST *lst)
{
    struct ISIL_NOTIFY_BLK *notify_blk = NULL;

    if(lst) {
        
        pthread_mutex_lock(&lst->notify_ev_lock);

        if(!list_empty(&lst->notify_list)) {
            
            notify_blk = list_entry(lst->notify_list.next,struct ISIL_NOTIFY_BLK,entry);
            list_del_init(&notify_blk->entry);
            --lst->notify_num;
            minus_notify_blk_ref(notify_blk);
            

        }

        pthread_mutex_unlock(&lst->notify_ev_lock);
    }

    return notify_blk;
}

static void notify_lst_lst_release(struct ISIL_NOTIFY_LST *lst)
{
    struct ISIL_NOTIFY_BLK *notify_blk = NULL ;

    struct list_head *ln1, *ln2;

    if( lst ) {

        pthread_mutex_lock(&lst->notify_ev_lock);
        
        if( !list_empty(&lst->notify_list)){

            list_for_each_safe(ln1, ln2, &lst->notify_list ){

                DEBUG_FUNCTION();

                notify_blk = list_entry(ln1,struct ISIL_NOTIFY_BLK ,entry);

            
                if( notify_blk ) {
                    list_del_init(&notify_blk->entry);
                    --lst->notify_num;
                    
                    //TODO: need free?
                    notify_blk->release(notify_blk);
                    
                }

                DEBUG_FUNCTION();
            }
            
            
        }

        pthread_mutex_unlock(&lst->notify_ev_lock);

        pthread_mutex_destroy(&lst->notify_ev_lock);

        if( lst->alloc_type == ISIL_DYNAMIC_ALLOC_TYPE) {
            free(lst);
            lst = NULL;
        }

        DEBUG_FUNCTION();
        
    }

}


static void notify_reg_event(struct ISIL_NOTIFY_LST *lst ,void *product)
{
    int ret;
    struct list_head *ln1, *ln2;

    struct ISIL_NOTIFY_BLK *notify_blk;

    if(!product) {
        return;
    }

    pthread_mutex_lock(&lst->notify_ev_lock);
    if(list_empty(&lst->notify_list)) {
        pthread_mutex_unlock(&lst->notify_ev_lock);
        return ;
    }

    list_for_each_safe(ln1, ln2, &lst->notify_list){
        notify_blk = list_entry(ln1,struct ISIL_NOTIFY_BLK ,entry);
        if( notify_blk ) {
            //TODO: need modify ?
                
            if(notify_blk->hook) {
                ret = notify_blk->hook(product,notify_blk->arg);
            }

            if(notify_blk->weekup) {
                notify_blk->weekup(notify_blk);
            }

            if(GET_NOTIFY_BLK_NOTIFY_TYPE(notify_blk) == ISIL_NOTIFY_ONCE ) {
                DEBUG_FUNCTION();
                list_del_init(&notify_blk->entry);
                --lst->notify_num;
                notify_blk->release(notify_blk);
            }
            //TODO : handle weekup?

            

        }
    }
    pthread_mutex_unlock(&lst->notify_ev_lock);
    
}


struct ISIL_NOTIFY_BLK *notify_blk_init( void )
{
    struct ISIL_NOTIFY_BLK *notify_blk =(struct ISIL_NOTIFY_BLK *)calloc( 1 , sizeof(struct ISIL_NOTIFY_BLK));
    if(notify_blk) {
        pthread_mutex_init(&notify_blk->entry_lock,NULL);
        INIT_LIST_HEAD(&notify_blk->entry);
        pthread_cond_init(&notify_blk->entry_cond,NULL);
        notify_blk->release = notify_blk_release;
        add_notify_blk_ref(notify_blk);
    }
    return notify_blk;
}


struct ISIL_NOTIFY_LST *notify_lst_minit( void )
{
    struct ISIL_NOTIFY_LST *notify_lst =(struct ISIL_NOTIFY_LST *)calloc( 1 , sizeof(struct ISIL_NOTIFY_LST));
    if(notify_lst) {

        pthread_mutex_init(&notify_lst->notify_ev_lock,NULL);
        INIT_LIST_HEAD(&notify_lst->notify_list);
        notify_lst->notify_num = 0 ;
        notify_lst->alloc_type = ISIL_DYNAMIC_ALLOC_TYPE;

        notify_lst->get_blk_from_head = get_blk_from_notify_lst_head;
        notify_lst->put_notify_blk         = put_blk_to_notify_lst;
        notify_lst->release                    = notify_lst_lst_release;
        notify_lst->notify = notify_reg_event;
        
    }
    return notify_lst;
}


void notify_lst_sinit( struct ISIL_NOTIFY_LST *notify_lst )
{

    if(notify_lst) {
        pthread_mutex_init(&notify_lst->notify_ev_lock,NULL);
        INIT_LIST_HEAD(&notify_lst->notify_list);
        
        notify_lst->alloc_type = ISIL_STATIC_ALLOC_TYPE;

        notify_lst->get_blk_from_head = get_blk_from_notify_lst_head;
        notify_lst->put_notify_blk         = put_blk_to_notify_lst;
        notify_lst->release                    = notify_lst_lst_release;
        notify_lst->notify = notify_reg_event;
    }
    
}














