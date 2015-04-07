#ifndef  _ISIL_NET_DATA_HEADER_H
#define  _ISIL_NET_DATA_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include "linux_list.h"
#include "isil_net_config.h"

struct NET_DATA_MGT_T;

struct NET_DATA_NODE_T{

    struct list_head send_entry;
    struct list_head entry;
    void *send_root;
    struct NET_DATA_MGT_T *net_data_mgt;
	int ref;
	pthread_mutex_t lock;
    void *arg;
    void (*release)(struct NET_DATA_NODE_T *net_data_node);
};


struct NET_DATA_MGT_T{
    struct list_head data_node_list;
    unsigned int node_num;
    pthread_mutex_t node_lst_lock;
    
    enum NETALLOCTYPE_E alloc_type;
    void *net_task;
	
    
    struct NET_DATA_NODE_T * (*get_net_data_node)(struct NET_DATA_MGT_T *net_data_mgt );
    void (*put_net_data_node)(struct NET_DATA_MGT_T *net_data_mgt , struct NET_DATA_NODE_T *data_node);
    void (*release)(struct NET_DATA_MGT_T *net_data_mgt);

};


inline static void  net_data_node_clone( struct NET_DATA_NODE_T * data_node)
{
	if(data_node) {
		pthread_mutex_lock( &data_node->lock );
		++data_node->ref;
		pthread_mutex_unlock( &data_node->lock );
	}
}

inline static void  net_data_node_minus_ref(struct NET_DATA_NODE_T * data_node)
{
	if(data_node) {
		pthread_mutex_lock( &data_node->lock );
		if(data_node->ref) {
			--data_node->ref;
		}
		pthread_mutex_unlock( &data_node->lock );
	}
}

extern void net_data_mgt_sinit(struct NET_DATA_MGT_T *net_data_mgt);

extern struct NET_DATA_MGT_T *net_data_mgt_minit( void );

extern struct NET_DATA_NODE_T *net_data_node_minit( void );

extern void del_self_data_node_from_mgt(struct NET_DATA_NODE_T *data_node);


#ifdef __cplusplus
}
#endif

#endif


