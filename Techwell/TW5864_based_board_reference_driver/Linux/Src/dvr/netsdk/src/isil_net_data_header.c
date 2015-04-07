#include <stdio.h>
#include <stdlib.h>

#include "isil_net_data_header.h"
#include "net_debug.h"



struct NET_DATA_NODE_T *net_data_node_minit( void )
{
    struct NET_DATA_NODE_T *node;
    node = (struct NET_DATA_NODE_T *)calloc(1 ,sizeof(struct NET_DATA_NODE_T));
    if( !node ) {
        return NULL;
    }

    INIT_LIST_HEAD(&node->entry);
    INIT_LIST_HEAD(&node->send_entry);
    pthread_mutex_init( &node->lock , NULL);
    node->send_root = NULL;
    node->net_data_mgt = NULL;
    node->arg = NULL;
    node->release = NULL;
    net_data_node_clone(node);

    return node;

}


void del_self_data_node_from_mgt(struct NET_DATA_NODE_T *data_node)
{
    struct NET_DATA_MGT_T *net_data_mgt;
    if(data_node) {
        if(data_node->net_data_mgt) {

            net_data_mgt = data_node->net_data_mgt;
            pthread_mutex_lock(&net_data_mgt->node_lst_lock);
            list_del_init(&data_node->entry);
            --net_data_mgt->node_num;
            data_node->net_data_mgt = NULL;
            net_data_node_minus_ref(data_node);
            pthread_mutex_unlock(&net_data_mgt->node_lst_lock);

        }
    }
}


static void put_net_data_node_to_mgt(struct NET_DATA_MGT_T *net_data_mgt , struct NET_DATA_NODE_T *data_node)
{
    if( !net_data_mgt || !data_node) {
        return ;
    }

    pthread_mutex_lock(&net_data_mgt->node_lst_lock);
    
    list_add_tail(&data_node->entry, &net_data_mgt->data_node_list);
    ++net_data_mgt->node_num;

    data_node->net_data_mgt = net_data_mgt;
    net_data_node_clone(data_node);

    pthread_mutex_unlock(&net_data_mgt->node_lst_lock);

}


static struct NET_DATA_NODE_T * get_net_data_node_from_mgt(struct NET_DATA_MGT_T *net_data_mgt )
{

    struct NET_DATA_NODE_T *data_node = NULL;

    if( !net_data_mgt ) {
        return NULL;
    }

    pthread_mutex_lock(&net_data_mgt->node_lst_lock);

    if(net_data_mgt->node_num == 0 ) {
        pthread_mutex_unlock(&net_data_mgt->node_lst_lock);
        return NULL;
    }

    
    data_node = list_entry(net_data_mgt->data_node_list.next,struct NET_DATA_NODE_T,entry);
    list_del_init(&data_node->entry);
    --net_data_mgt->node_num;
    data_node->net_data_mgt = NULL;
    net_data_node_minus_ref(data_node);
    pthread_mutex_unlock(&net_data_mgt->node_lst_lock);
    
    return data_node;
}




static void net_data_mgt_release(struct NET_DATA_MGT_T *net_data_mgt)
{
    struct NET_DATA_NODE_T *data_node;
    struct list_head *ln1, *ln2;

    if(net_data_mgt) {

        pthread_mutex_lock(&net_data_mgt->node_lst_lock);

        if(!list_empty(&net_data_mgt->data_node_list)){
            
            list_for_each_safe(ln1, ln2,&net_data_mgt->data_node_list){
                data_node = list_entry(ln1,struct NET_DATA_NODE_T ,entry);
                if(data_node) {
                    list_del_init(&data_node->entry);
                    --net_data_mgt->node_num;
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

        net_data_mgt->net_task = NULL ;
        pthread_mutex_unlock(&net_data_mgt->node_lst_lock);
        
        pthread_mutex_destroy(&net_data_mgt->node_lst_lock);
        INIT_LIST_HEAD(&net_data_mgt->data_node_list);

        if(net_data_mgt->alloc_type == NET_DYNAMIC_ALLOC_TYPE) {
           free(net_data_mgt);
           net_data_mgt = NULL;
        }
        
        
    }
    
}


static void net_data_mgt_t_init(struct NET_DATA_MGT_T *net_data_mgt)
{
    if( net_data_mgt ) {

        pthread_mutex_init(&net_data_mgt->node_lst_lock,NULL);
        INIT_LIST_HEAD(&net_data_mgt->data_node_list);
        net_data_mgt->node_num = 0 ;

        

        net_data_mgt->net_task = NULL ;

        net_data_mgt->put_net_data_node = put_net_data_node_to_mgt;
        net_data_mgt->get_net_data_node = get_net_data_node_from_mgt;
        net_data_mgt->release = net_data_mgt_release;
    }
}

void net_data_mgt_sinit(struct NET_DATA_MGT_T *net_data_mgt)
{
    net_data_mgt_t_init(net_data_mgt);
    net_data_mgt->alloc_type = NET_STATIC_ALLOC_TYPE;
}


struct NET_DATA_MGT_T *net_data_mgt_minit( void )
{
    struct NET_DATA_MGT_T *net_data_mgt;
    net_data_mgt = (struct NET_DATA_MGT_T *)calloc(1 ,sizeof(struct NET_DATA_MGT_T));
    if( !net_data_mgt ) {
        return NULL;
    }
    
    net_data_mgt_t_init(net_data_mgt);
    net_data_mgt->alloc_type = NET_DYNAMIC_ALLOC_TYPE;

    return net_data_mgt;
}








