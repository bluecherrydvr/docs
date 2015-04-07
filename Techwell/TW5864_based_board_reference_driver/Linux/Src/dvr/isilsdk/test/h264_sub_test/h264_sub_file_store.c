#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>


#include "isil_enc_sdk.h"
#include "isil_data_stream.h"
#include "linux_list.h"


#define FILESTORENAME "./test_sub.h264"

/*encode channel  numbers */

static const int channel_num = 0;


#define  STORE_FILE_MAX_LEN (1024000)

#define THR_SLEEP_TIME (40000)


typedef struct _file_store_list_t{
    struct list_head file_list;
    pthread_mutex_t lock;
   
}file_store_list_t;


typedef struct _file_store_node{
    struct list_head entry;
    ISIL_AV_PACKET *pck;
}file_store_node;



static  file_store_list_t cur_file_store_list_t;


static void init_cur_file_store_list_t( void )
{
    INIT_LIST_HEAD(&cur_file_store_list_t.file_list);
    pthread_mutex_init(&cur_file_store_list_t.lock,NULL);
    
}

static file_store_node* get_init_file_node( void )
{
    file_store_node *node = (file_store_node *)calloc( 1 ,sizeof(file_store_node));
    if( !node ) {
        exit(-1);
    }

    INIT_LIST_HEAD( &node->entry );
    return node;
}

static file_store_node* get_file_node_from_list( void )
{
    file_store_node *node = NULL;

    pthread_mutex_lock(&cur_file_store_list_t.lock);

    if( list_empty(&cur_file_store_list_t.file_list)){
        pthread_mutex_unlock(&cur_file_store_list_t.lock);
        return NULL;
    }

    node = list_entry(cur_file_store_list_t.file_list.next,file_store_node,entry);
    list_del_init(&node->entry);
    

    pthread_mutex_unlock(&cur_file_store_list_t.lock);

    return node;
}


static void  put_file_node_to_list( file_store_node *node)
{
    if( !node ) {
        return;
    }
    
    pthread_mutex_lock(&cur_file_store_list_t.lock);
    
    list_add_tail(&node->entry, &cur_file_store_list_t.file_list);

    pthread_mutex_unlock(&cur_file_store_list_t.lock);

}


static void file_list_release( void )
{
    struct list_head *ln1, *ln2;
    file_store_node *node = NULL;

    pthread_mutex_lock(&cur_file_store_list_t.lock);

    if( !list_empty(&cur_file_store_list_t.file_list) ){

        list_for_each_safe(ln1, ln2, &cur_file_store_list_t.file_list ){
            
            node = list_entry(ln1,file_store_node ,entry);
            if( node ) {
    
                list_del_init(&node->entry);
                if( node->pck ) {
                    isil_av_packet_release(node->pck);
                }
    
                free( node );
                node = NULL;
            }
        }

    }

    pthread_mutex_unlock(&cur_file_store_list_t.lock);


    INIT_LIST_HEAD(&cur_file_store_list_t.file_list);

    pthread_mutex_destroy(&cur_file_store_list_t.lock);
}












static  int GetH264CallBack( void *av_packet , void *pContext)
{

    file_store_node *node;

    ISIL_AV_PACKET *pck = (ISIL_AV_PACKET *)av_packet;
    if( !pck ) {
        return -1;
    }
    
    isil_av_packet_clone(pck);

    node = get_init_file_node();
    node->pck = pck ;

    put_file_node_to_list(node);

   //fprintf(stderr,"get  data .\n");

	/*DEBUG stream, please look at isil_data_stream.h */
//    isil_av_packet_debug(pck);
    //isil_av_packet_release(pck);

    return 0;
    
}


static void* file_store_thread( void * arg)
{
    
    int ret ,file_fd ,file_len = 0;

    file_store_node *node = NULL;
    

    init_cur_file_store_list_t();


    file_fd = open( FILESTORENAME ,O_RDWR |O_CREAT|O_TRUNC);
    if( file_fd < 0 ) {
        fprintf( stderr,"Open %s failed .\n",FILESTORENAME);
        exit(-1);
    }


    /*register h264 get frame */
    ret = ISIL_MediaSDK_RegH264SubCallback( 0 ,channel_num , GetH264CallBack , NULL);
    if( ret < 0 ) {
        fprintf(stderr ,"ISIL_MediaSDK_RegH264MainCallback err .\n");
        exit (-1);
    }

    /*start transfer stream*/

    ret = ISIL_MediaSDK_StartH264SubChannel(0 , channel_num );
    if( ret < 0 ) {
        fprintf(stderr ,"ISIL_MediaSDK_StartH264MainChannel err .\n");
        exit (-1);
    }


    while( file_len < STORE_FILE_MAX_LEN ) {
        node = get_file_node_from_list();
        if( !node ) {
            usleep(THR_SLEEP_TIME);
            continue;
        }

        if( !node->pck ) {
            fprintf( stderr,"Err: node->pck ptr null .\n");
            exit (-1);
        }
        
        if( !node->pck->buff || !node->pck->date_len ) {
            fprintf( stderr,"Err: node->pck->buff ptr null or node->pck->date_len zero .\n");
            exit (-1);
        }
        ret = write( file_fd , node->pck->buff , node->pck->date_len);
        if( ret < 0) {
            fprintf( stderr,"Write to file %s err .\n",FILESTORENAME);
            exit (-1);
        }

        

        file_len += ret;

        fprintf( stderr,"Write file len %d .\n" ,file_len);

        isil_av_packet_release(node->pck);

        free(node);
        node = NULL;
    }


    fprintf( stderr,"Write finishied.\n");

    ISIL_MediaSDK_StopH264SubChannel( 0 , channel_num);


    file_list_release();

    close(file_fd);

    ISIL_MediaSDK_Exit();

    pthread_exit(NULL);

}

/* if you want to exit ,Press CTRL + C*/

int main(int argc ,char *argv[])
{
    int ret ,i ;

    pthread_t tid;

    void *status;

    ret = ISIL_MediaSDK_Init();
    if(ret < 0 ) {
        exit (-1);
    }

    ret = ISIL_MediaSDK_Run();

    if(ret < 0 ) {
        exit (-1);
    }

    ret = pthread_create( &tid , NULL , &file_store_thread  , NULL );
    if( ret < 0) {
        fprintf( stderr,"Create thread err .\n");
    }

    ret = pthread_join( tid, (void **)&status);
	if( ret != 0 ) {
		fprintf(stderr, "Join   thread  err!!! \n");
        exit (-1);
	}
    	

    ISIL_MediaSDK_Cleanup();
	
	return 0;
}
