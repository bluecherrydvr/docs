#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#include "isil_net_nty.h"



static void del_net_nty_from_reg_list( struct NET_NOTIFY_T *notify_p )
{

    int ret;

    struct NET_NOTIFY_REG_T * net_notify_reg_p;

    if( !notify_p || !notify_p->reg_root) {
        return;
    }

    net_notify_reg_p = notify_p->reg_root;

    if( net_notify_reg_p ) {
            
        pthread_mutex_lock(&net_notify_reg_p->reg_lock);
    
        list_del_init(&notify_p->reg_entry);
    
        --net_notify_reg_p->reg_num;
        
        notify_p->reg_root = NULL;
    
        if( list_empty(&net_notify_reg_p->reg_list) ) {
    
            if( !net_notify_reg_p->root || !net_notify_reg_p->root_unreg) {
                assert(0);
            }
    
            ret = net_notify_reg_p->root_unreg(net_notify_reg_p->root);
            if( ret < 0 ) {
                assert(0);
            }
    
        }
    
        pthread_mutex_unlock(&net_notify_reg_p->reg_lock);
    }
    
}


static void del_net_nty_from_record_list( struct NET_NOTIFY_T *notify_p )
{
    struct NET_NOTIFY_RECORD_T * net_notify_record_p;

    if( !notify_p || !notify_p->record_root) {
        return;
    }

    net_notify_record_p = notify_p->record_root;

    if( net_notify_record_p ) {
            
        pthread_mutex_lock(&net_notify_record_p->record_lock);
    
        list_del_init(&notify_p->record_entry);
    
        --net_notify_record_p->record_num;
    
        notify_p->record_root = NULL;
    
        pthread_mutex_unlock(&net_notify_record_p->record_lock);
    }
    
}


static void net_notify_release( struct NET_NOTIFY_T *notify_p )
{
    if(notify_p) {

        del_net_nty_from_reg_list(notify_p);
        del_net_nty_from_record_list(notify_p);
        free(notify_p);
        notify_p = NULL;
    }
}

struct NET_NOTIFY_T * net_notify_minit( void )
{
    struct NET_NOTIFY_T *notify_p = (struct NET_NOTIFY_T *)calloc(1 ,sizeof(struct NET_NOTIFY_T));
    if( !notify_p ) {
        return NULL;
    }

    INIT_LIST_HEAD(&notify_p->reg_entry);
    INIT_LIST_HEAD(&notify_p->record_entry);
    notify_p->reg_root = NULL;
    notify_p->record_root = NULL;

    notify_p->release = net_notify_release;
    notify_p->param = NULL;
    notify_p->hook    = NULL;

    return notify_p;

}


static void put_net_nty_to_reg_lst(struct NET_NOTIFY_T *notify_p , struct NET_NOTIFY_REG_T * net_notify_reg_p)
{
    int ret;
    if( notify_p && net_notify_reg_p) {

        //pthread_mutex_lock(&net_notify_reg_p->reg_lock);

        if( list_empty(&net_notify_reg_p->reg_list) ) {

            list_add_tail(&notify_p->reg_entry, &net_notify_reg_p->reg_list);
            ++net_notify_reg_p->reg_num;
            notify_p->reg_root = net_notify_reg_p;

            if( !net_notify_reg_p->root || !net_notify_reg_p->root_reg) {
                assert(0);
            }

            ret = net_notify_reg_p->root_reg(net_notify_reg_p->root);
            if( ret < 0 ) {
                assert(0);
            }

        }else{
            list_add_tail(&notify_p->reg_entry, &net_notify_reg_p->reg_list);
            ++net_notify_reg_p->reg_num;
            notify_p->reg_root = net_notify_reg_p;
        }

        
        //pthread_mutex_unlock(&net_notify_reg_p->reg_lock);
    }
}


static void net_nty_reg_notify_all( void *src,struct NET_NOTIFY_REG_T * net_notify_reg_p)
{

    int ret;
    struct list_head *ln1, *ln2;

    struct NET_NOTIFY_T *notify_p;

    if( !net_notify_reg_p || !src ) {
        return;
    }
    
    pthread_mutex_lock(&net_notify_reg_p->reg_lock);

    if( list_empty(&net_notify_reg_p->reg_list) ) {
        pthread_mutex_unlock(&net_notify_reg_p->reg_lock);
        return ;
    }

    list_for_each_safe(ln1, ln2, &net_notify_reg_p->reg_list){
            
        notify_p = list_entry(ln1,struct NET_NOTIFY_T ,reg_entry);
        if( notify_p ) {
            if(notify_p->hook) {

                
#if 1
                ret = notify_p->hook(src ,notify_p);
                if( ret < 0 ) {

                    //TODO: del from list?
                    
                    list_del_init(&notify_p->reg_entry);

                    --net_notify_reg_p->reg_num;
    
                    notify_p->reg_root = NULL;
                    
                    notify_p->release(notify_p);
                }
#endif
                
            }
        }
    }

    pthread_mutex_unlock(&net_notify_reg_p->reg_lock);
    
}




static void net_nty_reg_release(struct NET_NOTIFY_REG_T * net_notify_reg_p)
{

    int ret;

    struct list_head *ln1, *ln2;

    struct NET_NOTIFY_T *notify_p;

    if( !net_notify_reg_p ) {
        return;
    }

    pthread_mutex_lock(&net_notify_reg_p->reg_lock);

    if( list_empty(&net_notify_reg_p->reg_list) ) {
        pthread_mutex_unlock(&net_notify_reg_p->reg_lock);
        return ;
    }

    list_for_each_safe(ln1, ln2, &net_notify_reg_p->reg_list){
            
        notify_p = list_entry(ln1,struct NET_NOTIFY_T ,reg_entry);
        if( notify_p ) {
            list_del_init(&notify_p->reg_entry);
            --net_notify_reg_p->reg_num;
            notify_p->reg_root = NULL;
        }
    }

    if( list_empty(&net_notify_reg_p->reg_list) ) {

        if( !net_notify_reg_p->root || !net_notify_reg_p->root_unreg) {
            assert(0);
        }

        ret = net_notify_reg_p->root_unreg(net_notify_reg_p->root);
        if( ret < 0 ) {
            assert(0);
        }

    }
    


    pthread_mutex_unlock(&net_notify_reg_p->reg_lock);


    pthread_mutex_destroy(&net_notify_reg_p->reg_lock);
    INIT_LIST_HEAD(&net_notify_reg_p->reg_list);

    if( net_notify_reg_p->alloc_type == NET_DYNAMIC_ALLOC_TYPE ) {
        free(net_notify_reg_p);
        net_notify_reg_p = NULL;
    }

}



static void net_notify_reg_init(struct NET_NOTIFY_REG_T *net_notify_reg_p)
{
    if( !net_notify_reg_p ) {
        return ;
    }

    pthread_mutex_init(&net_notify_reg_p->reg_lock,NULL);
    
    INIT_LIST_HEAD(&net_notify_reg_p->reg_list);

    net_notify_reg_p->reg_num = 0;

    net_notify_reg_p->notify_all = net_nty_reg_notify_all;

    net_notify_reg_p->put_net_nty = put_net_nty_to_reg_lst;

    net_notify_reg_p->release = net_nty_reg_release;

}


void net_notify_reg_sinit(struct NET_NOTIFY_REG_T *net_notify_reg_p)
{
    net_notify_reg_init(net_notify_reg_p);
    net_notify_reg_p->alloc_type = NET_STATIC_ALLOC_TYPE;
}


struct NET_NOTIFY_REG_T * net_notify_reg_minit( void )
{
    struct NET_NOTIFY_REG_T * net_notify_reg_p;

    net_notify_reg_p = (struct NET_NOTIFY_REG_T *)calloc(1 ,sizeof(struct NET_NOTIFY_REG_T));
    if( !net_notify_reg_p ) {
        return NULL;
    }

    net_notify_reg_init(net_notify_reg_p);
    net_notify_reg_p->alloc_type = NET_DYNAMIC_ALLOC_TYPE;

    return net_notify_reg_p;

}



static void put_net_nty_to_record_lst(struct NET_NOTIFY_T *notify_p , struct NET_NOTIFY_RECORD_T * net_notify_record_p)
{

    if( notify_p && net_notify_record_p) {

        pthread_mutex_lock(&net_notify_record_p->record_lock);
        list_add_tail(&notify_p->record_entry, &net_notify_record_p->record_list);
        notify_p->record_root = net_notify_record_p;
        ++net_notify_record_p->record_num;
        pthread_mutex_unlock(&net_notify_record_p->record_lock);

    }

}


static void net_nty_record_release(struct NET_NOTIFY_RECORD_T * net_notify_record_p)
{


    struct list_head *ln1, *ln2;

    struct NET_NOTIFY_T *notify_p;

    if( !net_notify_record_p ) {
        return;
    }

    pthread_mutex_lock(&net_notify_record_p->record_lock);

    if( list_empty(&net_notify_record_p->record_list) ) {
        pthread_mutex_unlock(&net_notify_record_p->record_lock);
        return ;
    }

    list_for_each_safe(ln1, ln2, &net_notify_record_p->record_list){
            
        notify_p = list_entry(ln1,struct NET_NOTIFY_T ,record_entry);
        if( notify_p ) {
            list_del_init(&notify_p->record_entry);
            --net_notify_record_p->record_num;
            
            /*TODO :*/
            del_net_nty_from_reg_list(notify_p);
            free(notify_p);
            notify_p = NULL;
        }
    }

    pthread_mutex_unlock(&net_notify_record_p->record_lock);

    pthread_mutex_destroy(&net_notify_record_p->record_lock);

    INIT_LIST_HEAD(&net_notify_record_p->record_list);
    if(net_notify_record_p->alloc_type == NET_DYNAMIC_ALLOC_TYPE) {
        free(net_notify_record_p);
        net_notify_record_p = NULL;
    }

}




static void net_notify_record_init(struct NET_NOTIFY_RECORD_T *net_notify_record_p)
{

    if( !net_notify_record_p ) {
        return ;
    }

    pthread_mutex_init(&net_notify_record_p->record_lock,NULL);
    
    INIT_LIST_HEAD(&net_notify_record_p->record_list);

    net_notify_record_p->record_num = 0;

    net_notify_record_p->put_net_nty = put_net_nty_to_record_lst;
    
    net_notify_record_p->release = net_nty_record_release;
    
}


void net_notify_record_sinit(struct NET_NOTIFY_RECORD_T *net_notify_record_p)
{
    net_notify_record_init(net_notify_record_p);
    net_notify_record_p->alloc_type = NET_STATIC_ALLOC_TYPE;
}


struct NET_NOTIFY_RECORD_T * net_notify_record_minit( void )
{

    struct NET_NOTIFY_RECORD_T *net_notify_record_p;
    net_notify_record_p = (struct NET_NOTIFY_RECORD_T *)calloc(1 ,sizeof(struct NET_NOTIFY_RECORD_T));
    if( !net_notify_record_p ) {
        return NULL;
    }

    net_notify_record_init(net_notify_record_p);

    net_notify_record_p->alloc_type = NET_DYNAMIC_ALLOC_TYPE;

    return net_notify_record_p;

}



























