#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <assert.h>
#include <pthread.h>


#include "isil_media_config.h"
#include "isil_encoder.h"
#include "isil_decoder.h"
#include "isil_vi.h"
#include "isil_ai.h"
#include "isil_codec_chip_ctl.h"
#include "isil_sys_config.h"
#include "isil_vo.h"
#include "isil_ao.h"
#include "isil_dev_file_mgt.h"
#include "isil_dev_file_st.h" 



static S_MEDIA_CHIPS_MGT glb_media_chips_mgt;




/**************************************************
  *                 RELEASE OPT FUNC
  *************************************************/

static void enc_release_single_chan_mgt(S_ISIL_SINGLE_CHAN_MGT *single_chan_mgt)
{
    int chip_fd;
    if(single_chan_mgt) {
        fprintf(stderr, "[%s %d] singnle chn ptr %p", __FUNCTION__, __LINE__, single_chan_mgt);
        if(single_chan_mgt->is_enable != CHAN_INVALID_MODE ) {
          
            if(single_chan_mgt->parent) {
               
                chip_fd = get_chip_fd(single_chan_mgt->parent);
                if( chip_fd > 0 ) {
                   
                    fprintf( stderr,"eCodecType[%d] , eStreamType[%d] , u32Ch[%d] , u32ChipID[%d] .\n",
                     single_chan_mgt->codec_channel_t.eCodecType,
                     single_chan_mgt->codec_channel_t.eStreamType,
                     single_chan_mgt->codec_channel_t.u32Ch,
                     single_chan_mgt->codec_channel_t.u32ChipID);

              
                    if(ISIL_CODEC_ENC_ReleaseCh(&(single_chan_mgt->codec_channel_t), &chip_fd) != CODEC_ERR_OK ) {
                        fprintf(stderr,"ISIL_CODEC_ENC_ReleaseCh err .\n");
                        
                    }
                    
                }
                unreg_single_chan_mgt(single_chan_mgt);
            }
        }

        free_single_chan_mgt(single_chan_mgt);
    }
}



static void dec_release_single_chan_mgt(S_ISIL_SINGLE_CHAN_MGT *single_chan_mgt)
{


    int chip_fd;

    if(single_chan_mgt) {
        if(single_chan_mgt->is_enable != CHAN_INVALID_MODE ) {
            if(single_chan_mgt->parent) {
                chip_fd = get_chip_fd(single_chan_mgt->parent);
                if( chip_fd > 0 ) {
                   DEBUG_FUNCTION();
                   if(ISIL_CODEC_DEC_ReleaseCh(&single_chan_mgt->codec_channel_t, &chip_fd) != CODEC_ERR_OK ) {
                        assert(0);
                    }
                   DEBUG_FUNCTION();
                }
                unreg_single_chan_mgt(single_chan_mgt);
            }
        }
        free_single_chan_mgt(single_chan_mgt);
    }

}


static void release_media_chips_mgt_enc_list(S_MEDIA_CHIPS_MGT *chips_mgt_p)
{
    S_ISIL_CHAN_MGT  *chan_mgt;
    pthread_mutex_lock( &chips_mgt_p->enc_list_lock);

     while( chips_mgt_p->enc_num ) {
         chan_mgt = list_entry(chips_mgt_p->encode_list.next,S_ISIL_CHAN_MGT,logic_num_entry);
         if( chan_mgt ) {
             list_del_init(&chan_mgt->logic_num_entry);
             chan_mgt->logic_chips_root = NULL;
             chips_mgt_p->enc_num--;
            
             if(chan_mgt->release) {
                 chan_mgt->release(chan_mgt);
             }
         }
     }
     pthread_mutex_unlock( &chips_mgt_p->enc_list_lock);
     pthread_mutex_destroy(&chips_mgt_p->enc_list_lock);
}


static void release_media_chips_mgt_dec_list(S_MEDIA_CHIPS_MGT *chips_mgt_p)
{
    S_ISIL_CHAN_MGT  *chan_mgt;
     pthread_mutex_lock( &chips_mgt_p->dec_list_lock);

     while( chips_mgt_p->dec_num ) {
         chan_mgt = list_entry(chips_mgt_p->decode_list.next,S_ISIL_CHAN_MGT,logic_num_entry);
         if( chan_mgt ) {
             list_del_init(&chan_mgt->logic_num_entry);
             chan_mgt->logic_chips_root = NULL;
             chips_mgt_p->dec_num--;

             if(chan_mgt->release) {
                 chan_mgt->release(chan_mgt);
             }
         }
     }
     pthread_mutex_unlock( &chips_mgt_p->dec_list_lock);
     pthread_mutex_destroy(&chips_mgt_p->dec_list_lock);

}

static void release_media_chips_mgt_t(S_MEDIA_CHIPS_MGT *media_chips_mgt)
{
    int i;

    S_SINGLE_CHIP_MGT * chip_mgt;


    if( !media_chips_mgt ) {
        return ;
    }

    for( i = 0 ; i < media_chips_mgt->total_chip_num ; i++ ) {
        chip_mgt= media_chips_mgt->per_chip_mgt_array[i];
        if( !chip_mgt ) {
            continue;
        }

        


        if( chip_mgt->release ) {
            chip_mgt->release(chip_mgt);
        }
    }

    //release_media_chips_mgt_enc_list(media_chips_mgt);
    //release_media_chips_mgt_dec_list(media_chips_mgt);

    if(media_chips_mgt->per_chip_mgt_array) {

        free(media_chips_mgt->per_chip_mgt_array);
        media_chips_mgt->per_chip_mgt_array = NULL;
    }
}


static void release_single_chan_enc_lst(S_SINGLE_CHIP_MGT * chip_mgt)
{
	int i;
    S_ISIL_CHAN_MGT  *chan_mgt;

    if( !chip_mgt ) {
        return ;
    }

	if( !chip_mgt->enc_chan_mgt_arr ) {
		return ;
	}

	for(i = 0 ; i < chip_mgt->enc_max_chan_num ; i++) {
		chan_mgt = chip_mgt->enc_chan_mgt_arr[i];
		if(chan_mgt) {

            //fprintf( stderr,"release chan_mgt[%p].\n",chan_mgt);
			
			if( chan_mgt->release ) {
                
                chan_mgt->release(chan_mgt);
                
            }
            chan_mgt->single_chip_root = NULL;
			chip_mgt->enc_chan_nums--;
		}
	}

	free(chip_mgt->enc_chan_mgt_arr);
	chip_mgt->enc_chan_mgt_arr = NULL;
    
}


static void release_single_chan_dec_lst(S_SINGLE_CHIP_MGT * chip_mgt)
{
	int i;
    S_ISIL_CHAN_MGT  *chan_mgt;
	
	if( !chip_mgt->dec_chan_mgt_arr ) {
		return ;
	}

	for(i = 0 ; i < chip_mgt->dec_max_chan_num ; i++) {
		chan_mgt = chip_mgt->dec_chan_mgt_arr[i];
		if(chan_mgt) {
			chan_mgt->single_chip_root = NULL;
			if( chan_mgt->release ) {
                chan_mgt->release(chan_mgt);
            }
			chip_mgt->dec_chan_nums--;
		}
	}

	free(chip_mgt->dec_chan_mgt_arr);
	chip_mgt->dec_chan_mgt_arr = NULL;
}


static void free_single_chip_mgt( S_SINGLE_CHIP_MGT * chip_mgt  )
{
    if( chip_mgt ) {
		if(chip_mgt->chip_node) {
			free(chip_mgt->chip_node);
			chip_mgt->chip_node = NULL;
		}

        free(chip_mgt);
        chip_mgt = NULL;

    }
}


static void close_single_chip_io(S_SINGLE_CHIP_MGT * chip_mgt)
{
    IO_NODE_MGT *io_node_ptr;
    if( !chip_mgt ) {
        return ;
    }

    if(chip_mgt->chip_pro & CHIP_CAN_VI ) {
        
        io_node_ptr = &chip_mgt->io_node_mgt_arr[NODE_TYPE_VI];
        if( io_node_ptr->is_enable != -1 ) {
            DEBUG_FUNCTION();
            ISIL_VI_CTL_CloseChip(chip_mgt->cur_chip_id);
            DEBUG_FUNCTION();
            io_node_ptr->is_enable = -1;
        }

    }


    if(chip_mgt->chip_pro & CHIP_CAN_AI ) {
        
        io_node_ptr = &chip_mgt->io_node_mgt_arr[NODE_TYPE_AI];
        if( io_node_ptr->is_enable != -1 ) {
            DEBUG_FUNCTION();
            ISIL_AI_CTL_CloseChip(chip_mgt->cur_chip_id);
            DEBUG_FUNCTION();
            io_node_ptr->is_enable = -1;
        }

    }

    if(chip_mgt->chip_pro & CHIP_CAN_VO ) {
        
        io_node_ptr = &chip_mgt->io_node_mgt_arr[NODE_TYPE_VO];
        if( io_node_ptr->is_enable != -1 ) {
            DEBUG_FUNCTION();
            ISIL_VO_CTL_CloseChip(chip_mgt->cur_chip_id);
            DEBUG_FUNCTION();
            io_node_ptr->is_enable = -1;
        }

    }

    if(chip_mgt->chip_pro & CHIP_CAN_AO ) {
        
        io_node_ptr = &chip_mgt->io_node_mgt_arr[NODE_TYPE_AO];
        if( io_node_ptr->is_enable != -1 ) {
            DEBUG_FUNCTION();
            ISIL_AO_CTL_CloseChip(chip_mgt->cur_chip_id);
            DEBUG_FUNCTION();
            io_node_ptr->is_enable = -1;
        }

    }

    if(chip_mgt->chip_pro & CHIP_CAN_CODEC ) {
        DEBUG_FUNCTION();
        ISIL_CODEC_CTL_CloseChip(chip_mgt->cur_chip_id);
        DEBUG_FUNCTION();

    }

    return ;


}

static void release_single_chip_mgt( S_SINGLE_CHIP_MGT * chip_mgt )
{
    DEBUG_FUNCTION();
    if( !chip_mgt ) {
        return ;
    }
    DEBUG_FUNCTION();
    release_single_chan_enc_lst(chip_mgt);
    DEBUG_FUNCTION();
    release_single_chan_dec_lst(chip_mgt);
    DEBUG_FUNCTION();
    close_single_chip_io(chip_mgt);
    DEBUG_FUNCTION();
    
    free_single_chip_mgt(chip_mgt);
    DEBUG_FUNCTION();
}

static void try_release_chan_mgt_from_lgc_enc_list(S_ISIL_CHAN_MGT *chan_mgt_p)
{
    int ret;
    if(chan_mgt_p->logic_chips_root) {
        ret = pthread_mutex_trylock(&chan_mgt_p->logic_chips_root->enc_list_lock);
        if( ret != 0 ) {
            fprintf(stderr,"Get lock err .\n");
            return;
        }

        list_del_init(&chan_mgt_p->logic_num_entry);
        --chan_mgt_p->logic_chips_root->enc_num;
        pthread_mutex_unlock(&chan_mgt_p->logic_chips_root->enc_list_lock);
        chan_mgt_p->logic_chips_root = NULL;
    }

}


static void try_release_chan_mgt_from_lgc_dec_list(S_ISIL_CHAN_MGT *chan_mgt_p)
{
    int ret;
    if(chan_mgt_p->logic_chips_root) {
        ret = pthread_mutex_trylock(&chan_mgt_p->logic_chips_root->dec_list_lock);
        if( ret != 0 ) {
            fprintf(stderr,"Get lock err .\n");
            return;
        }

        list_del_init(&chan_mgt_p->logic_num_entry);
        --chan_mgt_p->logic_chips_root->dec_num;
        pthread_mutex_unlock(&chan_mgt_p->logic_chips_root->dec_list_lock);
        chan_mgt_p->logic_chips_root = NULL;
    }

}


static void free_chan_mgt(S_ISIL_CHAN_MGT *chan_mgt_p)
{
    if( chan_mgt_p ) {
        if(chan_mgt_p->single_chan_mgt_array) {
            free(chan_mgt_p->single_chan_mgt_array);
            chan_mgt_p->single_chan_mgt_array = NULL;
        }
        free(chan_mgt_p);
        chan_mgt_p = NULL;
    }
}

static void release_chan_mgt(S_ISIL_CHAN_MGT *chan_mgt_p)
{
    int i;
    S_ISIL_SINGLE_CHAN_MGT *single_chan_mgt;

    if( !chan_mgt_p ) {
        return;
    }

    if(!chan_mgt_p->single_chan_mgt_array) {
        return;
    }
    
    //fprintf( stderr,"chan_mgt_p[%p] cur_chip_id[%d],chip_chan_num[%d] .\n",chan_mgt_p,chan_mgt_p->cur_chip_id,chan_mgt_p->chip_chan_num);
    for(i = 0 ; i < FUNCPROSIZE; i++) {

        

        single_chan_mgt = chan_mgt_p->single_chan_mgt_array[i];
        if( !single_chan_mgt ) {
            continue ;
        }

 

        if(single_chan_mgt->release) {
            
            single_chan_mgt->release(single_chan_mgt);
            
        }

  

    }

    if(chan_mgt_p->logic_chips_root) {
        if(chan_mgt_p->enc_or_dec == CHAN_IS_ENC) {
            try_release_chan_mgt_from_lgc_enc_list(chan_mgt_p);
            
        }
        else if(chan_mgt_p->enc_or_dec == CHAN_IS_DEC){
            try_release_chan_mgt_from_lgc_dec_list(chan_mgt_p);
            
        }
        else{
            assert(0);
        }
    }

    free_chan_mgt(chan_mgt_p);


}

/***********************************************
 *
 *       END RELEASE OPT FUNC
 *
 ***********************************************/



/***********************************************
 *
 *       INIT OPT FUNC
 *
 ***********************************************/

S_MEDIA_CHIPS_MGT * get_glb_media_chips_mgt_ptr( void )
{
    return &glb_media_chips_mgt;
}


S_ISIL_SINGLE_CHAN_MGT * init_malloc_single_chan_mgt( enum ECHAN_PROPERTY isEnc )
{
    S_ISIL_SINGLE_CHAN_MGT *single_chan_ptr;

    if(isEnc != CHAN_IS_ENC && isEnc != CHAN_IS_DEC) {
        return NULL;
    }
    single_chan_ptr = (S_ISIL_SINGLE_CHAN_MGT *)calloc(1 ,sizeof(S_ISIL_SINGLE_CHAN_MGT));
	if( !single_chan_ptr ) {
		exit(-1);
	}
    
    
    single_chan_ptr->is_enable = CHAN_INVALID_MODE;
    single_chan_ptr->log_pro    = -1;
    if(isEnc == CHAN_IS_ENC) {
        single_chan_ptr->release = enc_release_single_chan_mgt;
    }
    else if( isEnc == CHAN_IS_DEC ) {
        single_chan_ptr->release = dec_release_single_chan_mgt;
    }
    fprintf(stderr, "[%s %d] singnle chn ptr %p", __FUNCTION__, __LINE__, single_chan_ptr);
    return single_chan_ptr;
}

S_SINGLE_CHIP_MGT *init_malloc_single_chip_mgt( unsigned char chip_id )
{
    S_SINGLE_CHIP_MGT *chip_mgt_ptr;

    IO_NODE_MGT *io_node;

    int i;

    chip_mgt_ptr = (S_SINGLE_CHIP_MGT *)calloc(1 ,sizeof(S_SINGLE_CHIP_MGT));
    if(!chip_mgt_ptr) {
        return NULL;
    }
    
    chip_mgt_ptr->cur_chip_id = chip_id ;
    
    
   
    
    for( i = NODE_TYPE_VI ; i < NODE_TYPE_BUBBON ; i++) {
        io_node = &chip_mgt_ptr->io_node_mgt_arr[i];
        io_node->is_enable = -1;
    }

	if( !chip_mgt_ptr->chip_node) {
		chip_mgt_ptr->chip_node = (CHIP_NODE_CFG *)calloc( 1, sizeof(CHIP_NODE_CFG));
		if(!chip_mgt_ptr->chip_node) {
			free(chip_mgt_ptr);
            chip_mgt_ptr = NULL;
			return NULL;
		}
    }

	chip_mgt_ptr->release = release_single_chip_mgt;

    return chip_mgt_ptr;
}

S_ISIL_CHAN_MGT *malloc_init_isil_chan_mgt( void )
{
    S_ISIL_CHAN_MGT *chan_mgt_ptr;
    chan_mgt_ptr = (S_ISIL_CHAN_MGT *)calloc(1 ,sizeof(S_ISIL_CHAN_MGT));
    if( !chan_mgt_ptr ) {
        return NULL;
    }

    if( !chan_mgt_ptr->single_chan_mgt_array ) {
        chan_mgt_ptr->single_chan_mgt_array = (S_ISIL_SINGLE_CHAN_MGT **)calloc( FUNCPROSIZE ,sizeof(S_ISIL_SINGLE_CHAN_MGT *));
        if( !chan_mgt_ptr->single_chan_mgt_array ) {
            free(chan_mgt_ptr);
            chan_mgt_ptr = NULL ;
            return NULL;
        }
    }
    
    INIT_LIST_HEAD(&chan_mgt_ptr->chip_entry);
    INIT_LIST_HEAD(&chan_mgt_ptr->logic_num_entry);

    chan_mgt_ptr->release = release_chan_mgt;

    return chan_mgt_ptr;
}


void init_glb_media_chips_mgt(int chipNum)
{
    S_MEDIA_CHIPS_MGT *glb_media_chips_mgt_ptr = get_glb_media_chips_mgt_ptr();

    INIT_LIST_HEAD(&glb_media_chips_mgt_ptr->encode_list);
    INIT_LIST_HEAD(&glb_media_chips_mgt_ptr->encode_list);

    pthread_mutex_init(&glb_media_chips_mgt_ptr->enc_list_lock,NULL);
    pthread_mutex_init(&glb_media_chips_mgt_ptr->dec_list_lock,NULL);

    glb_media_chips_mgt_ptr->dec_num =  0;
    glb_media_chips_mgt_ptr->enc_num =  0;

    glb_media_chips_mgt_ptr->total_chip_num = chipNum;
    glb_media_chips_mgt_ptr->per_chip_mgt_array = (S_SINGLE_CHIP_MGT **)calloc( chipNum ,sizeof(S_SINGLE_CHIP_MGT * ));
    if( !glb_media_chips_mgt_ptr->per_chip_mgt_array ) {
        fprintf(stderr,"Malloc per_chip_mgt_array failed .\n");
        exit(-1);
    }
    glb_media_chips_mgt_ptr->release = release_media_chips_mgt_t;
}

/***********************************************
 *
 *       END INIT OPT FUNC
 *
 ***********************************************/


/**************************************************
  *                 LOGIC ENC LST OPT FUNC
  *************************************************/

S_ISIL_CHAN_MGT *enc_get_sameid_chan_mgt_from_logic_lst( int id)
{
    int middle_value;
    S_ISIL_CHAN_MGT  *com_ptr;

    S_MEDIA_CHIPS_MGT *media_chips_mgt_ptr  = get_glb_media_chips_mgt_ptr();
    middle_value = (media_chips_mgt_ptr->max_enc_num >> 1) +1;
    if( list_empty(&media_chips_mgt_ptr->encode_list) ) {
        DEBUG_FUNCTION();
        return NULL;
    }
    fprintf(stderr,"chan num [%d] .\n",id);
    pthread_mutex_lock( &media_chips_mgt_ptr->enc_list_lock);

    if(id < middle_value) {
        list_for_each_entry(com_ptr,&media_chips_mgt_ptr->encode_list,logic_num_entry){
            if(com_ptr) {
                DEBUG_FUNCTION();
                if(com_ptr->logic_num == id) {
                    pthread_mutex_unlock( &media_chips_mgt_ptr->enc_list_lock);
                    return com_ptr;
                }
            }
        }
    }
    else{
        list_for_each_entry_reverse(com_ptr,&media_chips_mgt_ptr->encode_list,logic_num_entry){
            if(com_ptr) {
                if(com_ptr->logic_num == id) {
                    DEBUG_FUNCTION();
                    pthread_mutex_unlock( &media_chips_mgt_ptr->enc_list_lock);
                    return com_ptr;
                }
            }
        }
    }
    pthread_mutex_unlock( &media_chips_mgt_ptr->enc_list_lock);
    return NULL;
}

void enc_put_chan_mgt_to_logic_lst(S_ISIL_CHAN_MGT  *chan_mgt)
{
    unsigned short midValue;
    S_MEDIA_CHIPS_MGT *media_chips_mgt_ptr;
    S_ISIL_CHAN_MGT *comp_ptr;
    if( !chan_mgt ) {
        return;
    }
    media_chips_mgt_ptr = get_glb_media_chips_mgt_ptr();

    if( list_empty(&media_chips_mgt_ptr->encode_list) ) {
        list_add(&chan_mgt->logic_num_entry,&media_chips_mgt_ptr->encode_list);
        ++media_chips_mgt_ptr->enc_num;
        chan_mgt->logic_chips_root = media_chips_mgt_ptr;
        media_chips_mgt_ptr->max_enc_num = chan_mgt->logic_num;
        return;
    }

    midValue = (media_chips_mgt_ptr->max_enc_num >>1) +1;
    pthread_mutex_lock( &media_chips_mgt_ptr->enc_list_lock);

    if(chan_mgt->logic_num < midValue ) {
        list_for_each_entry(comp_ptr,&media_chips_mgt_ptr->encode_list,logic_num_entry){
            if( chan_mgt->logic_num < comp_ptr->logic_num ) {
                list_add_tail( &chan_mgt->logic_num_entry , &comp_ptr->logic_num_entry);
                ++media_chips_mgt_ptr->enc_num;
                if(MAX(media_chips_mgt_ptr->max_enc_num ,chan_mgt->logic_num) == chan_mgt->logic_num) {
                    media_chips_mgt_ptr->max_enc_num = chan_mgt->logic_num;
                }
                chan_mgt->logic_chips_root = media_chips_mgt_ptr;

                pthread_mutex_unlock( &media_chips_mgt_ptr->enc_list_lock);
                return;
            }
        }
    }
    else{

        list_for_each_entry_reverse(comp_ptr,&media_chips_mgt_ptr->encode_list,logic_num_entry){

            if( chan_mgt->logic_num > comp_ptr->logic_num ) {
                list_add( &chan_mgt->logic_num_entry , &comp_ptr->logic_num_entry);
                ++media_chips_mgt_ptr->enc_num;
                if(MAX(media_chips_mgt_ptr->max_enc_num ,chan_mgt->logic_num) == chan_mgt->logic_num) {
                    media_chips_mgt_ptr->max_enc_num = chan_mgt->logic_num;
                }
                chan_mgt->logic_chips_root = media_chips_mgt_ptr;

                pthread_mutex_unlock( &media_chips_mgt_ptr->enc_list_lock);
                return;
            }
        }
    }
    DEBUG_FUNCTION();
    pthread_mutex_unlock( &media_chips_mgt_ptr->enc_list_lock);

    assert(0);
    return;
}


/**************************************************
  *                 END LOGIC ENC LST OPT FUNC
  *************************************************/




/**************************************************
  *                 LOGIC DEC LST OPT FUNC
  *************************************************/


S_ISIL_CHAN_MGT *dec_get_sameid_chan_mgt_from_logic_lst( int id)
{
    int middle_value;
    S_ISIL_CHAN_MGT  *com_ptr;
    S_MEDIA_CHIPS_MGT *media_chips_mgt_ptr;

    media_chips_mgt_ptr = get_glb_media_chips_mgt_ptr();

    middle_value = (media_chips_mgt_ptr->dec_num >> 1);
    if( list_empty(&media_chips_mgt_ptr->decode_list) ) {
        return NULL;
    }

    pthread_mutex_lock( &media_chips_mgt_ptr->dec_list_lock);

    if(id <= middle_value) {
        list_for_each_entry(com_ptr,&media_chips_mgt_ptr->decode_list,logic_num_entry){
            if(com_ptr->logic_num == id) {
                pthread_mutex_unlock( &media_chips_mgt_ptr->dec_list_lock);
                return com_ptr;
            }
        }
    }
    else{
        list_for_each_entry_reverse(com_ptr,&media_chips_mgt_ptr->decode_list,logic_num_entry){
            if(com_ptr->logic_num == id) {
                pthread_mutex_unlock( &media_chips_mgt_ptr->dec_list_lock);
                return com_ptr;
            }
        }
    }
    pthread_mutex_unlock( &media_chips_mgt_ptr->dec_list_lock);
    return NULL;
}

void dec_put_chan_mgt_to_logic_lst(S_ISIL_CHAN_MGT  *chan_mgt)
{
    unsigned short midValue;
    S_MEDIA_CHIPS_MGT *media_chips_mgt_ptr;
    S_ISIL_CHAN_MGT *comp_ptr;
    if( !chan_mgt ) {
        return;
    }
    media_chips_mgt_ptr = get_glb_media_chips_mgt_ptr();

    if( list_empty(&media_chips_mgt_ptr->decode_list) ) {
        list_add_tail(&chan_mgt->logic_num_entry,&media_chips_mgt_ptr->decode_list);
        ++media_chips_mgt_ptr->dec_num;
        chan_mgt->logic_chips_root = media_chips_mgt_ptr;
        media_chips_mgt_ptr->max_enc_num = chan_mgt->logic_num;
        return;
    }

    midValue = (media_chips_mgt_ptr->max_dec_num >>1)+1;
    pthread_mutex_lock( &media_chips_mgt_ptr->dec_list_lock);

    if(chan_mgt->logic_num < midValue ) {
        list_for_each_entry(comp_ptr,&media_chips_mgt_ptr->decode_list,logic_num_entry){
            if( chan_mgt->logic_num < comp_ptr->logic_num ) {
                list_add_tail( &chan_mgt->logic_num_entry , &comp_ptr->logic_num_entry);
                ++media_chips_mgt_ptr->dec_num;
                if(MAX(media_chips_mgt_ptr->max_dec_num ,chan_mgt->logic_num) == chan_mgt->logic_num) {
                    media_chips_mgt_ptr->max_dec_num = chan_mgt->logic_num;
                }
                chan_mgt->logic_chips_root = media_chips_mgt_ptr;
                pthread_mutex_unlock( &media_chips_mgt_ptr->dec_list_lock);
                return;
            }
        }
    }
    else{

        list_for_each_entry_reverse(comp_ptr,&media_chips_mgt_ptr->decode_list,logic_num_entry){

            if( chan_mgt->logic_num > comp_ptr->logic_num ) {
                list_add( &chan_mgt->logic_num_entry , &comp_ptr->logic_num_entry);
                ++media_chips_mgt_ptr->dec_num;
                if(MAX(media_chips_mgt_ptr->max_dec_num ,chan_mgt->logic_num) == chan_mgt->logic_num) {
                    media_chips_mgt_ptr->max_dec_num = chan_mgt->logic_num;
                }
                chan_mgt->logic_chips_root = media_chips_mgt_ptr;
                pthread_mutex_unlock( &media_chips_mgt_ptr->dec_list_lock);
                return;
            }
        }
    }
    pthread_mutex_unlock( &media_chips_mgt_ptr->dec_list_lock);

    assert(0);
    return;
}

/**************************************************
  *                 END LOGIC DEC LST OPT FUNC
  *************************************************/




/**************************************************
  *                CHIP ENC LIST OPT FUNC
  *************************************************/

void enc_put_chan_mgt_to_chip_lst(S_ISIL_CHAN_MGT  *chan_mgt, S_SINGLE_CHIP_MGT *single_chip_mgt)
{
	int i;
	    
    if( !chan_mgt || !single_chip_mgt) {
		fprintf(stderr,"enc_put_chan_mgt_to_chip_lst err 1.\n");
        return;
    }

	i = chan_mgt->chip_chan_num;

	if(i >= single_chip_mgt->enc_max_chan_num) {
		fprintf(stderr,"enc_put_chan_mgt_to_chip_lst err 2.\n");
		return;
	}

    if(single_chip_mgt->enc_chan_mgt_arr[i]) {
		fprintf(stderr,"enc_put_chan_mgt_to_chip_lst err 3.\n");
		return;
	}
    
	single_chip_mgt->enc_chan_mgt_arr[i] = chan_mgt;
    ++single_chip_mgt->enc_chan_nums;
    chan_mgt->single_chip_root = single_chip_mgt ;
}




S_ISIL_CHAN_MGT  *enc_get_chip_chn_chan_mgt_from_chip_lst(unsigned short chip_chn ,S_SINGLE_CHIP_MGT *single_chip_mgt)
{
	
	
	if(!single_chip_mgt) {
		return NULL;
	}

	if( !single_chip_mgt->enc_chan_mgt_arr) {
		fprintf(stderr,"%s err 1 .\n",__FUNCTION__);
		return NULL;
	}

	if(chip_chn >= single_chip_mgt->enc_max_chan_num) {
		fprintf(stderr,"%s err 2 .\n",__FUNCTION__);
		return NULL;
	}

		
    return single_chip_mgt->enc_chan_mgt_arr[chip_chn];
}


/**************************************************
  *                END CHIP ENC LIST OPT FUNC
  *************************************************/



/**************************************************
  *                 CHIP DEC LIST OPT FUNC
  *************************************************/

void dec_put_chan_mgt_to_chip_lst(S_ISIL_CHAN_MGT  *chan_mgt, S_SINGLE_CHIP_MGT *single_chip_mgt)
{
	int i;
    if( !chan_mgt || !single_chip_mgt) {
		fprintf(stderr,"dec_put_chan_mgt_to_chip_lst err 1.\n");
        return;
    }

	i = chan_mgt->chip_chan_num;

	if(i >= single_chip_mgt->dec_max_chan_num) {
		fprintf(stderr,"dec_put_chan_mgt_to_chip_lst err 2.\n");
		return;
	}

    if(single_chip_mgt->dec_chan_mgt_arr[i]) {
		fprintf(stderr,"dec_put_chan_mgt_to_chip_lst err 3.\n");
		return;
	}
    
	single_chip_mgt->dec_chan_mgt_arr[i] = chan_mgt;
    ++single_chip_mgt->dec_chan_nums;
    chan_mgt->single_chip_root = single_chip_mgt ;
	
}


S_ISIL_CHAN_MGT  *dec_get_chip_chn_chan_mgt_from_chip_lst(unsigned short chip_chn ,S_SINGLE_CHIP_MGT *single_chip_mgt)
{
    

	if(!single_chip_mgt) {
		return NULL;
	}

	if( !single_chip_mgt->dec_chan_mgt_arr) {
		fprintf(stderr,"%s err 1 .\n",__FUNCTION__);
		return NULL;
	}

	if(chip_chn >= single_chip_mgt->dec_max_chan_num) {
		fprintf(stderr,"%s err 2 .\n",__FUNCTION__);
		return NULL;
	}

		
    return single_chip_mgt->dec_chan_mgt_arr[chip_chn];
}


/**************************************************
  *                 END CHIP DEC LIST OPT FUNC
  *************************************************/


S_ISIL_SINGLE_CHAN_MGT *get_single_chan_mgt_by_chan_mgt( S_ISIL_CHAN_MGT * chan_mgt ,int type)
{
    if( !chan_mgt ) {
        return NULL;
    }
    if( !chan_mgt->single_chan_mgt_array ) {
        return NULL;
    }
    if( type >= FUNCPROSIZE || type < IS_H264_MAIN_STREAM) {
        return NULL;
    }
    return chan_mgt->single_chan_mgt_array[type];
}


void free_single_chan_mgt(S_ISIL_SINGLE_CHAN_MGT *single_chan_mgt)
{
    if(single_chan_mgt) {
		
        free(single_chan_mgt);
        single_chan_mgt = NULL;
    }
}


int reg_single_chan_mgt(S_ISIL_SINGLE_CHAN_MGT *single_chan_mgt ,S_ISIL_CHAN_MGT *chan_mgt)
{
    int echpro;
    echpro = GET_SINGLE_CHAN_LOG_PRO(single_chan_mgt);

    if( !chan_mgt->single_chan_mgt_array ) {
        return -1;
    }

    if(echpro >= FUNCPROSIZE || echpro < IS_H264_MAIN_STREAM ) {
        return -1;
    }

    if( chan_mgt->single_chan_mgt_array[echpro] ) {
        return -1;
    }

    single_chan_mgt->parent = chan_mgt;

    chan_mgt->single_chan_mgt_array[echpro] = single_chan_mgt;
    return 0;
}

void unreg_single_chan_mgt(S_ISIL_SINGLE_CHAN_MGT *single_chan_mgt)
{
    int eChanPro;
    S_ISIL_CHAN_MGT *chan_mgt_ptr;

    if(single_chan_mgt) {

        eChanPro = single_chan_mgt->log_pro;
        chan_mgt_ptr = single_chan_mgt->parent;
        if( eChanPro >= FUNCPROSIZE || eChanPro < IS_H264_MAIN_STREAM ) {
            return;
        }
        if(!chan_mgt_ptr) {
            return;
        }
        eChanPro = single_chan_mgt->log_pro;
        if(chan_mgt_ptr->single_chan_mgt_array) {
            chan_mgt_ptr->single_chan_mgt_array[eChanPro] = NULL;
        }
        single_chan_mgt->parent = NULL;
    }
}


int get_chip_num_by_chan_mgt(S_ISIL_CHAN_MGT *chan_mgt)
{
    if( !chan_mgt || !chan_mgt->single_chip_root) {
        return -1;
    }
    return chan_mgt->cur_chip_id;
}



/**************************************************
  *                VD  CONFIG FUNC
  *************************************************/

int init_chip_vdconfig( int chip )
{
    int i;
    ENC_CTL_VD_CFG enc_ctl_vd_cfg;

    memset(&enc_ctl_vd_cfg ,0x00 ,sizeof(ENC_CTL_VD_CFG));
    enc_ctl_vd_cfg.i_vd_chan_number = PER_CHIP_MAX_CHAN_NUM;

    for( i = 0 ; i < PER_CHIP_MAX_CHAN_NUM ;i++) {
        enc_ctl_vd_cfg.vd_param[i].i_phy_chan_id = i;
        enc_ctl_vd_cfg.vd_param[i].i_phy_chan_enable = 1;
        enc_ctl_vd_cfg.vd_param[i].i_phy_chan_fps       = 25;
        enc_ctl_vd_cfg.vd_param[i].i_phy_video_width_mb_size = 352>>4;
        enc_ctl_vd_cfg.vd_param[i].i_phy_video_height_mb_size = 288>>4;
    }

    if ( ISIL_ENC_CTL_SetVDCfg(chip, &enc_ctl_vd_cfg) != CODEC_ERR_OK){
        fprintf(stderr,"Set vdconfig failed .\n");
        return -1;
    }
    return 0;
}




/**************************************************
  *                END VD  CONFIG FUNC
  *************************************************/

int get_media_max_chan_number( void )
{
    S_MEDIA_CHIPS_MGT *media_chips_mgt = get_glb_media_chips_mgt_ptr();
    return media_chips_mgt->max_enc_num;
}






void release_chips_mgt_tree( void )
{

    S_MEDIA_CHIPS_MGT *media_chips_mgt;

    media_chips_mgt = get_glb_media_chips_mgt_ptr();
    if( media_chips_mgt->release ) {
        media_chips_mgt->release(media_chips_mgt);
    }
}

/**************************************************
  *                 END CHIPS FILE CONFIG
  *************************************************/



/**************************************************
  *                 DEBUG FUNC
  *************************************************/


void debug_single_chan_mgt( S_ISIL_SINGLE_CHAN_MGT *single_chan_mgt )
{
    if( !single_chan_mgt ) {
        return ;
    }
    fprintf( stderr,"---------is_enable[%d] ,log_id[%d],log_pro[%d],fd[%d] -----------.\n",
             single_chan_mgt->is_enable,
             single_chan_mgt->log_id,
             single_chan_mgt->log_pro,
             single_chan_mgt->fd);

    fprintf( stderr,"---------eCodecType[%d] ,eStreamType[%d],u32ChipID[%d],u32ChipID[%d] -----------.\n",
             single_chan_mgt->codec_channel_t.eCodecType,
             single_chan_mgt->codec_channel_t.eStreamType,
             single_chan_mgt->codec_channel_t.u32ChipID,
             single_chan_mgt->codec_channel_t.u32ChipID);

}


void debug_chan_mgt(S_ISIL_CHAN_MGT  *chan_mgt)
{
    if( !chan_mgt ) {
        return ;
    }
    fprintf( stderr,"---------enc_or_dec[%d] ,chip_chan_num[%d],cur_chip_id[%d],logic_num[%d] -----------.\n",
             chan_mgt->enc_or_dec,
             chan_mgt->chip_chan_num,
             chan_mgt->cur_chip_id,
             chan_mgt->logic_num);

    int i;
    S_ISIL_SINGLE_CHAN_MGT *single_chan_mgt;
    if( !chan_mgt->single_chan_mgt_array) {
        return;
    }
    for( i = 0 ; i < FUNCPROSIZE ; i++ ) {
        
        single_chan_mgt = chan_mgt->single_chan_mgt_array[i];
        if(single_chan_mgt) {
            debug_single_chan_mgt(single_chan_mgt);
        }
        
    }
}


void debug_sin_chip_enc_lst(S_SINGLE_CHIP_MGT *sin_chip_mgt)
{
	int i;
    S_ISIL_CHAN_MGT  *chan_mgt;

    if( !sin_chip_mgt ) {
        return;
    }
    if(!sin_chip_mgt->enc_chan_nums) {
        return;
    }

	if(!sin_chip_mgt->enc_chan_mgt_arr) {
		return;
	}

	for( i = 0 ; i < sin_chip_mgt->enc_max_chan_num; i++) {
		chan_mgt = sin_chip_mgt->enc_chan_mgt_arr[i];
		if(chan_mgt) {
            debug_chan_mgt(chan_mgt);
        }
	}
    
    return;
}

void debug_sin_chip_dec_lst(S_SINGLE_CHIP_MGT *sin_chip_mgt)
{
	int i;
    S_ISIL_CHAN_MGT  *chan_mgt;

    if( !sin_chip_mgt ) {
        return ;
    }
    if(!sin_chip_mgt->dec_chan_nums) {
        return;
    }

	if(!sin_chip_mgt->dec_chan_mgt_arr) {
		return;
	}

	for( i = 0 ; i < sin_chip_mgt->dec_max_chan_num; i++) {
		chan_mgt = sin_chip_mgt->dec_chan_mgt_arr[i];
		if(chan_mgt) {
            debug_chan_mgt(chan_mgt);
        }
	}

    
}


void debug_sin_chip_mgt( S_SINGLE_CHIP_MGT *sin_chip_mgt)
{

    if( !sin_chip_mgt ) {
        return;
    }
    fprintf( stderr,"---------chip_fd[%d] ,chip_pro[%d],cur_chip_id[%d] -----------.\n",
             sin_chip_mgt->chip_fd,
             sin_chip_mgt->chip_pro,
             sin_chip_mgt->cur_chip_id);

    fprintf( stderr,"---------enc_chan_nums[%d] ,dec_chan_nums[%d]-----------.\n",
             sin_chip_mgt->enc_chan_nums,
             sin_chip_mgt->dec_chan_nums);
    debug_sin_chip_enc_lst(sin_chip_mgt);
    debug_sin_chip_dec_lst(sin_chip_mgt);

}


void debug_chips_lgc_enc_list( void )
{
    S_MEDIA_CHIPS_MGT *chips_mgt;
    S_ISIL_CHAN_MGT  *chan_mgt;

    chips_mgt = get_glb_media_chips_mgt_ptr();
    if(list_empty(&chips_mgt->encode_list)) {
        return;
    }

    list_for_each_entry(chan_mgt,&chips_mgt->encode_list,logic_num_entry){
        if(chan_mgt) {
            debug_chan_mgt(chan_mgt);
        }
    }

    return ;
}



void debug_chips_lgc_dec_list( void )
{
    S_MEDIA_CHIPS_MGT *chips_mgt;
    S_ISIL_CHAN_MGT  *chan_mgt;

    DEBUG_FUNCTION();
    chips_mgt = get_glb_media_chips_mgt_ptr();
    if(!chips_mgt->dec_num) {
        return;
    }
    if(list_empty(&chips_mgt->decode_list)) {
        return;
    }
    DEBUG_FUNCTION();
    fprintf(stderr,"dec num= %d.\n",chips_mgt->dec_num);
    list_for_each_entry(chan_mgt,&chips_mgt->decode_list,logic_num_entry){
        if(chan_mgt) {
            debug_chan_mgt(chan_mgt);
        }
    }

    return ;
}

void debug_chips_tree( void )
{
    S_MEDIA_CHIPS_MGT *chips_mgt;
    chips_mgt = get_glb_media_chips_mgt_ptr();

    fprintf( stderr,"---------chips num[%d] ,enc_num[%d],dec_num[%d] -----------.\n",
             chips_mgt->total_chip_num,
             chips_mgt->enc_num,
             chips_mgt->dec_num);

    fprintf( stderr,"---------max_enc_num[%d] ,max_dec_num[%d] -----------.\n",
             chips_mgt->max_enc_num,
             chips_mgt->max_dec_num);
    debug_chips_lgc_enc_list();

    debug_chips_lgc_dec_list();


    fprintf( stderr,"-----------------------------------------\n");

    fprintf( stderr,"          FOR CHIP TARGET        \n");

    fprintf( stderr,"-----------------------------------------\n");

    if( !chips_mgt->per_chip_mgt_array ) {
        return ;
    }

    int i;

    S_SINGLE_CHIP_MGT *sin_chip_mgt;

    for( i = 0 ; i < chips_mgt->total_chip_num ; i++ ) {
        sin_chip_mgt = chips_mgt->per_chip_mgt_array[i];
        if(sin_chip_mgt) {
            debug_sin_chip_mgt(sin_chip_mgt);
        }
    }

    return;
}

void debug_codec_channel_t(CODEC_CHANNEL *codec_channel_t)
{
    if(codec_channel_t) {
        fprintf(stderr,"eCodecType[%d],eStreamType[%d],u32Ch[%d],u32ChipID[%d] .\n",
                codec_channel_t->eCodecType,
                codec_channel_t->eStreamType,
                codec_channel_t->u32Ch,
                codec_channel_t->u32ChipID);
    }
}




/**************************************************
  *                 END DEBUG FUNC
  *************************************************/









static int open_chip_io_port(S_SINGLE_CHIP_MGT * chip_mgt)
{
    
    int i ,ret;

    

    if( !chip_mgt ) {
        fprintf( stderr,"chip_mgt null.\n");
        return -1;
    }

    i = chip_mgt->cur_chip_id;

    if(chip_mgt->chip_pro & CHIP_CAN_CODEC) {

        if(!strlen(chip_mgt->chip_node->codec_dir)) {
            fprintf( stderr,"chip_mgt->chip_node->codec_dir err.\n");
            return -1;
        }

        ret = ISIL_CODEC_CTL_OpenChip(chip_mgt->chip_node->codec_dir, i);
        if( ret != CODEC_ERR_OK) {
            fprintf(stderr,"Open chip err.\n");
            chip_mgt->release(chip_mgt);
            exit(-1);
        }

        ret = ISIL_CODEC_CTL_GetChipHandle(i, &chip_mgt->chip_fd);
        if( ret != CODEC_ERR_OK) {
            fprintf(stderr,"GetChipHandle err.\n");
            chip_mgt->release(chip_mgt);
            exit(-1);
        }

    }
    
    if(chip_mgt->chip_pro & CHIP_CAN_VI){

        ret = isil_open_av_io_package(chip_mgt->chip_node->vi_dir , i, NODE_TYPE_VI);
        if( ret < 0 ) {
            fprintf(stderr,"isil_open_av_io_package err.\n");
            chip_mgt->release(chip_mgt);
            exit(-1);
        }
    }

    if(chip_mgt->chip_pro & CHIP_CAN_AI) {
        ret = isil_open_av_io_package(chip_mgt->chip_node->ai_dir , i, NODE_TYPE_AI);
        if( ret < 0 ) {
            fprintf(stderr,"isil_open_av_io_package err.\n");
            chip_mgt->release(chip_mgt);
            exit(-1);
        }
    }

    if(chip_mgt->chip_pro & CHIP_CAN_VO) {
        ret = isil_open_av_io_package(chip_mgt->chip_node->vo_dir , i, NODE_TYPE_VO);
        if( ret < 0 ) {
            fprintf(stderr,"isil_open_av_io_package err.\n");
            chip_mgt->release(chip_mgt);
            exit(-1);
        }
    }

    if(chip_mgt->chip_pro & CHIP_CAN_AO) {
        ret = isil_open_av_io_package(chip_mgt->chip_node->ao_dir , i, NODE_TYPE_AO);
        if( ret < 0 ) {
            fprintf(stderr,"isil_open_av_io_package err.\n");
            chip_mgt->release(chip_mgt);
            exit(-1);
        }
    }



    return 0;
}


static int chip_create_enc_node(S_SINGLE_CHIP_MGT * chip_mgt)
{
    int i;
    S_ISIL_CHAN_MGT  *chan_mgt;
    if( !chip_mgt ) {
        return -1;
    }

    if((chip_mgt->chip_pro & CHIP_CAN_ENC) && chip_mgt->enc_max_chan_num ) {

		chip_mgt->enc_chan_mgt_arr = (S_ISIL_CHAN_MGT **)calloc(chip_mgt->enc_max_chan_num ,sizeof(S_ISIL_CHAN_MGT *));

		if( !chip_mgt->enc_chan_mgt_arr ) {
			fprintf( stderr,"Calloc enc_chan_mgt_arr err .\n");
			return -1;
		}

        for( i = 0 ; i < chip_mgt->enc_max_chan_num ; i++) {

            chan_mgt = malloc_init_isil_chan_mgt();
            if( !chan_mgt ) {
                fprintf( stderr,"malloc_init_isil_chan_mgt err .\n");
                exit(-1);
            }

            fprintf( stderr,"chan_mgt[%p].\n",chan_mgt);
            chan_mgt->cur_chip_id = chip_mgt->cur_chip_id;
            chan_mgt->chip_chan_num = i ;
            chan_mgt->enc_or_dec = CHAN_IS_ENC;
            //chan_mgt->logic_num = enc_log_id++;

            //enc_put_chan_mgt_to_logic_lst(chan_mgt);
            enc_put_chan_mgt_to_chip_lst(chan_mgt,chip_mgt); 
        }
               
    }

    return 0;
}


static int chip_create_dec_node(S_SINGLE_CHIP_MGT * chip_mgt)
{
    int i;
    S_ISIL_CHAN_MGT  *chan_mgt;
    if( !chip_mgt ) {
        return -1;
    }

    if((chip_mgt->chip_pro & CHIP_CAN_DEC) && chip_mgt->dec_max_chan_num ) {

		chip_mgt->dec_chan_mgt_arr = (S_ISIL_CHAN_MGT **)calloc(chip_mgt->dec_max_chan_num ,sizeof(S_ISIL_CHAN_MGT *));

		if( !chip_mgt->dec_chan_mgt_arr ) {
			fprintf( stderr,"Calloc dec_chan_mgt_arr err .\n");
			return -1;
		}

        for( i = 0 ; i < chip_mgt->dec_max_chan_num ; i++) {

            chan_mgt = malloc_init_isil_chan_mgt();
            if( !chan_mgt ) {
                fprintf( stderr,"malloc_init_isil_chan_mgt err .\n");
                exit(-1);
            }
            chan_mgt->cur_chip_id = chip_mgt->cur_chip_id;
            chan_mgt->chip_chan_num = i ;
            chan_mgt->enc_or_dec = CHAN_IS_DEC;
            //chan_mgt->logic_num = enc_log_id++;

            //enc_put_chan_mgt_to_logic_lst(chan_mgt);
            dec_put_chan_mgt_to_chip_lst(chan_mgt,chip_mgt); 
        }
               
    }

    return 0;
}


static int create_dev_root_dir( void )
{
    int ret ,value ;

    char buff[256];

    
    memset(buff,0x00,sizeof(buff));
    sprintf(buff,"rm -fr %s" ,CHIP_DEV_ROOT);
    ret = system(buff);
    value = WEXITSTATUS(ret);
    if( value ) {
        fprintf(stderr,"Can't del dir [%s] .\n",buff);
        exit(-1);
    }

    memset(buff,0x00,sizeof(buff));
    sprintf(buff,"mkdir -p %s" ,CHIP_DEV_ROOT);

    ret = system(buff);
    value = WEXITSTATUS(ret);
    if( value ) {
        fprintf(stderr,"Can't mkdir dir [%s] .\n",buff);
        exit(-1);
    }

    return 0;
}


static int create_nod_by_dev_tree(struct dev_list_t *dev_root_list ,S_SINGLE_CHIP_MGT * chip_mgt_ptr)
{
    int chip_id ,ret ,value ,i;
    file_list *dev_file_list;

    char buff[256];
    

    if( !dev_root_list || !chip_mgt_ptr) {
        return -1;
    }

    chip_id = chip_mgt_ptr->cur_chip_id;
    if(chip_id >= dev_root_list->chipNum) {
        return -1;
    }

    memset(buff,0x00,sizeof(buff));
    sprintf(buff,"mkdir -p %s/%i" ,CHIP_DEV_ROOT,chip_id);
    ret = system(buff);
    value = WEXITSTATUS(ret);
    if( value ) {
        fprintf(stderr,"Can't mkdir dir [%s] .\n",buff);
        exit(-1);
    }
    
    memset(buff,0x00,sizeof(buff));
    sprintf(buff,"mkdir -p %s/%i/%s" ,CHIP_DEV_ROOT,chip_id,CHIP_CODEC_DEV_DIR);
    ret = system(buff);
    value = WEXITSTATUS(ret);
    if( value ) {
        fprintf(stderr,"Can't mkdir dir [%s] .\n",buff);
        exit(-1);
    }

    memset(buff,0x00,sizeof(buff));
    sprintf(buff,"mkdir -p %s/%i/%s" ,CHIP_DEV_ROOT,chip_id,CHIP_VI_DEV_DIR);
    ret = system(buff);
    value = WEXITSTATUS(ret);
    if( value ) {
        fprintf(stderr,"Can't mkdir dir [%s] .\n",buff);
        exit(-1);
    }


    memset(buff,0x00,sizeof(buff));
    sprintf(buff,"mkdir -p %s/%i/%s" ,CHIP_DEV_ROOT,chip_id,CHIP_VO_DEV_DIR);
    ret = system(buff);
    value = WEXITSTATUS(ret);
    if( value ) {
        fprintf(stderr,"Can't mkdir dir [%s] .\n",buff);
        exit(-1);
    }

    memset(buff,0x00,sizeof(buff));
    sprintf(buff,"mkdir -p %s/%i/%s" ,CHIP_DEV_ROOT,chip_id,CHIP_AI_DEV_DIR);
    ret = system(buff);
    value = WEXITSTATUS(ret);
    if( value ) {
        fprintf(stderr,"Can't mkdir dir [%s] .\n",buff);
        exit(-1);
    }

    memset(buff,0x00,sizeof(buff));
    sprintf(buff,"mkdir -p %s/%i/%s" ,CHIP_DEV_ROOT,chip_id,CHIP_AO_DEV_DIR);
    ret = system(buff);
    value = WEXITSTATUS(ret);
    if( value ) {
        fprintf(stderr,"Can't mkdir dir [%s] .\n",buff);
        exit(-1);
    }

    
    sprintf(chip_mgt_ptr->chip_node->ch_dir,"%s/%i/%s" ,CHIP_DEV_ROOT,chip_id,CHIP_CODEC_DEV_DIR);
        

    for( i = 0 ; i < dev_root_list->num ;i++){
         
        dev_file_list = dev_root_list->dev_file_list[i];
        if(dev_file_list) {
            fprintf(stderr,"dev_file_list->num_magic [%d],chip_id[%d] .\n",dev_file_list->num_magic,chip_id);

            if(dev_file_list->num_magic != chip_id ) {
                
                continue;

            }


            memset(buff,0x00,sizeof(buff));

            if(dev_file_list->type == IS_AUDIO_IN_NODE) {

                if(!(chip_mgt_ptr->chip_pro & CHIP_CAN_AI)) {
    
                    chip_mgt_ptr->chip_pro |= CHIP_CAN_AI;
                    
                    
                    sprintf(buff,"mknod  %s/%i/%s/%s  c %d %d",
                            CHIP_DEV_ROOT ,
                            chip_id,
                            CHIP_AI_DEV_DIR,
                            DEVNODNAME,
                            dev_file_list->dev_major ,
                            dev_file_list->dev_minor);
                    ret = system(buff);
                    value = WEXITSTATUS(ret);
                    if( value ) {
                        fprintf(stderr,"Can't create dir [%s] .\n",buff);
                        exit(-1);
                    }
                    sprintf(chip_mgt_ptr->chip_node->ai_dir,"%s/%i/%s/%s",
                            CHIP_DEV_ROOT ,
                            chip_id,
                            CHIP_AI_DEV_DIR,
                            DEVNODNAME);
                    
                }
                else{
                    fprintf( stderr,"chip ai nod exist .\n");
                    continue;
                }
            }
            else if(dev_file_list->type == IS_AUDIO_OUT_NODE) {
                
                if(!(chip_mgt_ptr->chip_pro & CHIP_CAN_AO)) {
    
                    chip_mgt_ptr->chip_pro |= CHIP_CAN_AO;
                    
                    
                    sprintf(buff,"mknod  %s/%i/%s/%s  c %d %d",
                            CHIP_DEV_ROOT ,
                            chip_id,
                            CHIP_AO_DEV_DIR,
                            DEVNODNAME,
                            dev_file_list->dev_major ,
                            dev_file_list->dev_minor);
                    ret = system(buff);
                    value = WEXITSTATUS(ret);
                    if( value ) {
                        fprintf(stderr,"Can't create dir [%s] .\n",buff);
                        exit(-1);
                    }

                    sprintf(chip_mgt_ptr->chip_node->ao_dir,"%s/%i/%s/%s",
                            CHIP_DEV_ROOT ,
                            chip_id,
                            CHIP_AO_DEV_DIR,
                            DEVNODNAME);

                }
                else{
                    fprintf( stderr,"chip ao nod exist .\n");
                    continue;
                }

            }
            else if(dev_file_list->type == IS_VIDEO_IN_NODE) {
                
                if(!(chip_mgt_ptr->chip_pro & CHIP_CAN_VI)) {
    
                    chip_mgt_ptr->chip_pro |= CHIP_CAN_VI;
                    chip_mgt_ptr->chip_pro |= CHIP_CAN_ENC;
                    chip_mgt_ptr->enc_max_chan_num = MAX_ENC_NUM_PER_CHIP;
                    sprintf(buff,"mknod  %s/%i/%s/%s  c %d %d",
                            CHIP_DEV_ROOT ,
                            chip_id,
                            CHIP_VI_DEV_DIR,
                            DEVNODNAME,
                            dev_file_list->dev_major ,
                            dev_file_list->dev_minor);

                    ret = system(buff);
                    value = WEXITSTATUS(ret);
                    if( value ) {
                        fprintf(stderr,"Can't create dir [%s] .\n",buff);
                        exit(-1);
                    }

                    sprintf(chip_mgt_ptr->chip_node->vi_dir,"%s/%i/%s/%s",
                            CHIP_DEV_ROOT ,
                            chip_id,
                            CHIP_VI_DEV_DIR,
                            DEVNODNAME);

                }
                else{
                    fprintf( stderr,"chip vi nod exist .\n");
                    continue;
                }

            }
            else if(dev_file_list->type == IS_VIDEO_OUT_NODE) {
                
                if(!(chip_mgt_ptr->chip_pro & CHIP_CAN_VO)) {
    
                    chip_mgt_ptr->chip_pro |= CHIP_CAN_VO;
                    chip_mgt_ptr->chip_pro |= CHIP_CAN_DEC;
                    chip_mgt_ptr->dec_max_chan_num = MAX_DEC_NUM_PER_CHIP;
                    
                    sprintf(buff,"mknod  %s/%i/%s/%s  c %d %d",
                            CHIP_DEV_ROOT ,
                            chip_id,
                            CHIP_VO_DEV_DIR,
                            DEVNODNAME,
                            dev_file_list->dev_major ,
                            dev_file_list->dev_minor);

                    ret = system(buff);
                    value = WEXITSTATUS(ret);
                    if( value ) {
                        fprintf(stderr,"Can't create dir [%s] .\n",buff);
                        exit(-1);
                    }

                    sprintf(chip_mgt_ptr->chip_node->vo_dir,"%s/%i/%s/%s",
                            CHIP_DEV_ROOT ,
                            chip_id,
                            CHIP_VO_DEV_DIR,
                            DEVNODNAME);

                }
                else{
                    fprintf( stderr,"chip vo nod exist .\n");
                    continue;
                }
            }
            else if(dev_file_list->type == IS_CODEC_NODE) {
                

                if(!(chip_mgt_ptr->chip_pro & CHIP_CAN_CODEC)) {
    
                    chip_mgt_ptr->chip_pro |= CHIP_CAN_CODEC;
                    

                    sprintf(buff,"mknod  %s/%i/%s/%s  c %d %d",
                            CHIP_DEV_ROOT ,
                            chip_id,
                            CHIP_CODEC_DEV_DIR,
                            DEVNODNAME,
                            dev_file_list->dev_major ,
                            dev_file_list->dev_minor);

                    ret = system(buff);
                    value = WEXITSTATUS(ret);
                    if( value ) {
                        fprintf(stderr,"Can't create dir [%s] .\n",buff);
                        exit(-1);
                    }

                    sprintf(chip_mgt_ptr->chip_node->codec_dir,"%s/%i/%s/%s",
                            CHIP_DEV_ROOT ,
                            chip_id,
                            CHIP_CODEC_DEV_DIR,
                            DEVNODNAME);


                }
                else{
                    fprintf( stderr,"chip vi nod exist .\n");
                    continue;
                }

            }
            else{
                fprintf( stderr,"parse dev nod err .\n");
                continue;
            }


        }
    }

    
    return 0;


}


int get_chips_tree_by_sys_inf( void )
{
    int ret ,i ;
    S_MEDIA_CHIPS_MGT *media_chips_mgt;
    S_SINGLE_CHIP_MGT * chip_mgt;
    

    struct dev_list_t *dev_root_list;

    int totalChipNum = 0;

    if(init_sys_dev_inf_tree() < 0) {

        fprintf( stderr,"init_sys_dev_inf_tree err .\n");
        exit (-1);
    }

    dev_root_list = get_glb_dev_list_t();

    totalChipNum = dev_root_list->chipNum;

    if( totalChipNum < 1 ) {
        fprintf( stderr,"Chip num < 1 ,maybe uninitial driver ? exit .\n");
        exit(-1);
    }

    ret = create_dev_root_dir();
    if( ret < 0) {
        fprintf( stderr,"create_dev_root_dir failed  .\n");
        exit(-1);
    }

    init_glb_media_chips_mgt(totalChipNum);

    media_chips_mgt = get_glb_media_chips_mgt_ptr();

    fprintf( stderr,"totalChipNum[%d].\n",totalChipNum);

    for( i = 0 ; i < totalChipNum ; i++) {

        chip_mgt = init_malloc_single_chip_mgt((unsigned char)i);
        if( !chip_mgt ) {
            fprintf( stderr,"init_malloc_single_chip_mgt failed .\n");
            exit (-1);
        }

        media_chips_mgt->per_chip_mgt_array[i] = chip_mgt;
        
       

        /*create chn dir*/
        ret = create_nod_by_dev_tree(dev_root_list,chip_mgt);
        if( ret < 0 ) {
            fprintf( stderr,"create_nod_by_dev_tree failed .\n");
            exit(-1);
        }

        ret =  open_chip_io_port(chip_mgt);
        if( ret < 0 ) {
            fprintf( stderr,"open_chip_io_port failed .\n");
            exit(-1);
        }

        if( init_chip_vdconfig(i) < 0){
            fprintf( stderr,"init_chip_vdconfig failed .\n");
            chip_mgt->release(chip_mgt);
            exit(-1);
        }


        if(chip_mgt->chip_pro & CHIP_CAN_DEC) {

            ret = chip_create_dec_node(chip_mgt);
            if( ret < 0 ) {
                fprintf( stderr,"chip_create_dec_node failed .\n");
                chip_mgt->release(chip_mgt);
                exit(-1);
            }

            media_chips_mgt->max_dec_num += chip_mgt->dec_max_chan_num;
        }

        if(chip_mgt->chip_pro & CHIP_CAN_ENC) {

            ret = chip_create_enc_node(chip_mgt);
            if( ret < 0 ) {
                fprintf( stderr,"chip_create_enc_node failed .\n");
                chip_mgt->release(chip_mgt);
                exit(-1);
            }
            media_chips_mgt->max_enc_num += chip_mgt->enc_max_chan_num;
            fprintf( stderr,"media_chips_mgt->max_enc_num[%d] .\n",media_chips_mgt->max_enc_num);
        }

        


    }

    release_sys_root_file_tree();
    release_dev_list_t(dev_root_list);

    return 0;


}



S_ISIL_CHAN_MGT *get_chan_mgt_by_chipid_chn(int chipid,int chn ,enum ECHAN_PROPERTY pro)
{
    S_MEDIA_CHIPS_MGT * chips_mgt = NULL;
    S_SINGLE_CHIP_MGT *chip_mgt_ptr;
    S_ISIL_CHAN_MGT     *isil_chan_mgt;

    chips_mgt= get_glb_media_chips_mgt_ptr();

    if(chipid >= chips_mgt->total_chip_num) {
        fprintf(stderr,"chipid is too big .\n");
        return NULL;
    }

    chip_mgt_ptr = chips_mgt->per_chip_mgt_array[chipid];
    if(!chip_mgt_ptr) {
        fprintf(stderr,"no found chip[%d] node,maybe not initialed .\n" ,chipid);
        return NULL;
    }

    if(chn >= chip_mgt_ptr->enc_max_chan_num) {
        fprintf(stderr," chip[%d],chn[%d] too big,maybe not initialed .\n" ,chipid,chn);
        return NULL;
    }
    
    if(pro == CHAN_IS_ENC) {
        isil_chan_mgt = enc_get_chip_chn_chan_mgt_from_chip_lst(chn,chip_mgt_ptr);
        if( !isil_chan_mgt ) {
            fprintf(stderr,"chip[%d] chn[%d] chan_mgt node not exist,maybe not initialed .\n" ,chipid,chn);
            return NULL;
        }
    }
    else if(pro == CHAN_IS_DEC) {
        isil_chan_mgt = dec_get_chip_chn_chan_mgt_from_chip_lst(chn,chip_mgt_ptr);
            
        if( !isil_chan_mgt ) {
            fprintf(stderr,"chip[%d] chn[%d] chan_mgt node not exist,maybe not initialed .\n" ,chipid,chn);
            return NULL;
        }
    }

    return isil_chan_mgt;

}


S_ISIL_SINGLE_CHAN_MGT *get_sin_chn_mgt_by_chip_chn_type(int chipid,
                                                         int chn ,
                                                         enum ECHAN_PROPERTY pro,
                                                         enum ECHANFUNCPRO eStreamType)
{

    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;
    S_ISIL_CHAN_MGT *isil_chan_mgt;
    int chip_fd ,chan_fd;
    int ret;


    if(pro >= CHAN_PRO_BUTTON) {
        return NULL;
    }

    

    isil_chan_mgt = get_chan_mgt_by_chipid_chn(chipid ,chn,pro);

    if( !isil_chan_mgt ) {
        return NULL;
    }

    
    
    sin_chan_mgt = get_single_chan_mgt_by_chan_mgt(isil_chan_mgt ,eStreamType);
    if(!sin_chan_mgt) {
        //TODO :build it
        
        sin_chan_mgt = init_malloc_single_chan_mgt( pro );
        if(!sin_chan_mgt) {
            fprintf(stderr,"malloc sin chan mgt failed .\n");
            return NULL;
        }

        fprintf( stderr,"malloc ---sin_chan_mgt[%p], codec_channel_t[%p].\n",
                 sin_chan_mgt , &sin_chan_mgt->codec_channel_t);
        
        sin_chan_mgt->codec_channel_t.u32ChipID = chipid;
        sin_chan_mgt->codec_channel_t.u32Ch        =  chn;

        if(eStreamType == IS_H264_MAIN_STREAM) {
            sin_chan_mgt->codec_channel_t.eCodecType = CODEC_VIDEO_H264;
            sin_chan_mgt->codec_channel_t.eStreamType = STREAM_TYPE_MAJOR;
            sin_chan_mgt->log_pro = IS_H264_MAIN_STREAM;
        }
        else if(eStreamType == IS_H264_SUB_STREAM) {
            sin_chan_mgt->codec_channel_t.eCodecType = CODEC_VIDEO_H264;
            sin_chan_mgt->codec_channel_t.eStreamType = STREAM_TYPE_MINOR;
            sin_chan_mgt->log_pro = IS_H264_SUB_STREAM;
        }
        else if( eStreamType == IS_MJPEG_STREAM) {
            sin_chan_mgt->codec_channel_t.eCodecType = CODEC_VIDEO_MJPG;
            sin_chan_mgt->codec_channel_t.eStreamType = STREAM_TYPE_MAJOR;
            sin_chan_mgt->log_pro = IS_MJPEG_STREAM;
        }
        else if(eStreamType == IS_AUDIO_STREAM ) {
            sin_chan_mgt->codec_channel_t.eCodecType = CODEC_AUDIO_ADPCM;
            sin_chan_mgt->codec_channel_t.eStreamType = STREAM_TYPE_AUDIO;
            sin_chan_mgt->log_pro = IS_AUDIO_STREAM;
        }
        else {
            fprintf(stderr,"eStreamType failed .\n");
            return NULL;
        }
        reg_single_chan_mgt(sin_chan_mgt,isil_chan_mgt);

    }


    if(GET_SINGLE_CHAN_ENABLE_PRO(sin_chan_mgt) == CHAN_INVALID_MODE) {

        

        assert(sin_chan_mgt->parent);

        chip_fd = get_chip_fd(sin_chan_mgt->parent);
        if( chip_fd < 0 ) {
            fprintf(stderr,"chip fd err .\n");
            return NULL;
        }

        
        //TODO: open chn

        if(pro == CHAN_IS_ENC) {

            
            if(ISIL_CODEC_ENC_CreateCh( &(sin_chan_mgt->codec_channel_t) , &chip_fd,sin_chan_mgt->parent->single_chip_root->chip_node->ch_dir) < 0){
                fprintf(stderr, "------ISIL_CODEC_ENC_CreateCh fail-----\n");
                sin_chan_mgt->release(sin_chan_mgt);
                return NULL;
            }
    
            
            ret = ISIL_CODEC_ENC_GetChHandle( &sin_chan_mgt->codec_channel_t ,&chan_fd );
            if( CODEC_ERR_OK != ret ) {
                    fprintf( stderr, "ISIL_CODEC_ENC_GetChHandle err.\n");
                    sin_chan_mgt->release(sin_chan_mgt);
                    return NULL;
            }
            
        }
        else{

            if(ISIL_CODEC_DEC_CreateCh( &sin_chan_mgt->codec_channel_t , &chip_fd,sin_chan_mgt->parent->single_chip_root->chip_node->ch_dir) < 0){
                fprintf(stderr, "------ISIL_CODEC_DEC_CreateCh fail-----\n");
                sin_chan_mgt->release(sin_chan_mgt);
                return NULL;
            }
    
            
            ret = ISIL_CODEC_DEC_GetChHandle( &sin_chan_mgt->codec_channel_t ,&chan_fd );
            if( CODEC_ERR_OK != ret ) {
                    fprintf( stderr, "ISIL_CODEC_DEC_GetChHandle err.\n");
                    sin_chan_mgt->release(sin_chan_mgt);
                    return NULL;
            }
        }

        

        SET_SINGLE_CHAN_FD(sin_chan_mgt ,chan_fd);

        SET_SINGLE_CHAN_ENABLE_PRO(sin_chan_mgt,0);
        
    }

    else if(GET_SINGLE_CHAN_ENABLE_PRO(sin_chan_mgt) == CHAN_DISABLE_MODE) {

        

    }

    else{

       

    }

  
    
    return sin_chan_mgt;

}


int isil_get_chips_count( void )
{
    

    S_MEDIA_CHIPS_MGT *glb_chips_mgt_ptr = get_glb_media_chips_mgt_ptr();

    return glb_chips_mgt_ptr->total_chip_num;
}

int get_total_enc_count(void)
{
    S_MEDIA_CHIPS_MGT *glb_chips_mgt_ptr = get_glb_media_chips_mgt_ptr();
    return glb_chips_mgt_ptr->max_enc_num;
}

int get_total_dec_count(void)
{
    S_MEDIA_CHIPS_MGT *glb_chips_mgt_ptr = get_glb_media_chips_mgt_ptr();
    return glb_chips_mgt_ptr->max_dec_num;
}


S_SINGLE_CHIP_MGT *get_single_chip_mgt_by_id( unsigned int chip_id)
{
    unsigned int total_chip_num;
    S_MEDIA_CHIPS_MGT * media_chips_mgt ;
    S_SINGLE_CHIP_MGT * s_single_chip_mgt;
    media_chips_mgt = get_glb_media_chips_mgt_ptr();

    total_chip_num = isil_get_chips_count();

    if( chip_id  >= total_chip_num ) {
        fprintf(stderr," chip_id failed .\n");
        return NULL;
    }

	if( !media_chips_mgt->per_chip_mgt_array ) {
		fprintf(stderr," per_chip_mgt_array NULL .\n");
        return NULL;
	}
   
        
    return media_chips_mgt->per_chip_mgt_array[chip_id];
}

#if 0

IO_NODE_MGT *malloc_init_io_node_mgt( void )
{
    IO_NODE_MGT *io_mgt;
    io_mgt = (IO_NODE_MGT *)calloc(1 ,sizeof(IO_NODE_MGT));
    if( ! io_mgt ) {
        fprintf(stderr,"Calloc IO_NODE_MGT failed .\n");
        return NULL;
    }

    io_mgt->is_enable = -1;

    return io_mgt;
}

#endif


static IO_NODE_MGT *open_av_io( char *path,unsigned int chip_id ,EIONODETYPE eionodetype)
{
    int ret ,fd;
    S_SINGLE_CHIP_MGT * s_single_chip_mgt;
    IO_NODE_MGT *io_mgt = NULL;
    s_single_chip_mgt = get_single_chip_mgt_by_id(chip_id);

    if( !s_single_chip_mgt ) {
       
        return NULL;
    }

    fprintf( stderr,"open s_single_chip_mgt addr[%p].\n",s_single_chip_mgt);

    if(!path) {
        
        return NULL;
    }

    if( !strlen(path) ) {
        
        return NULL;
    }

    if(eionodetype >= NODE_TYPE_BUBBON ) {
        
        return NULL;
    }
    
    

    
    io_mgt = &s_single_chip_mgt->io_node_mgt_arr[eionodetype];

#if 0
    if(io_mgt->is_enable != -1) {
        fprintf( stderr,"io_mgt  have open.\n");
        return io_mgt;
    }

#endif

    
    switch(eionodetype) {
    case NODE_TYPE_VI:{

        
        ret = ISIL_VI_CTL_OpenChip(path , chip_id);
        if(ret != CODEC_ERR_OK ) {
            fprintf( stderr,"ISIL_VI_CTL_OpenChip err .\n");
            return NULL;
        }

       
        
        ret = ISIL_VI_CTL_GetChipHandle(chip_id ,&fd);
        if( ret != CODEC_ERR_OK ) {
            fprintf( stderr,"ISIL_VI_CTL_OpenChip err .\n");
            return NULL;
        }

        

        io_mgt->fd = fd;

        io_mgt->is_enable = 1;

        fprintf(stderr,"io_mgt->fd[%d],%p.\n",io_mgt->fd,&s_single_chip_mgt->io_node_mgt_arr[eionodetype]);
    }
        break;
    case NODE_TYPE_AI:{
        ret = ISIL_AI_CTL_OpenChip(path , chip_id);
        if(ret < 0 ) {
            fprintf( stderr,"ISIL_AI_CTL_OpenChip err .\n");
            return NULL;
        }

        ret = ISIL_AI_CTL_GetChipHandle(chip_id ,&fd);
        if( ret < 0 ) {
            fprintf( stderr,"ISIL_AI_CTL_GetChipHandle err .\n");
            return NULL;
        }

        io_mgt->fd = fd;

        io_mgt->is_enable = 1;
    }
        break;
    case NODE_TYPE_VO:{
        
        ret = ISIL_VO_CTL_OpenChip(path , chip_id);
        if(ret < 0 ) {
            fprintf( stderr,"ISIL_VO_CTL_OpenChip err .\n");
            return NULL;
        }

        ret = ISIL_VO_CTL_GetChipHandle(chip_id ,&fd);
        if( ret < 0 ) {
            fprintf( stderr,"ISIL_VO_CTL_GetChipHandle err .\n");
            return NULL;
        }

        io_mgt->fd = fd;


        io_mgt->is_enable = 1;
    }
        break;
    case NODE_TYPE_AO:{
        
        ret = ISIL_AO_CTL_OpenChip(path , chip_id);
        if(ret < 0 ) {
            fprintf( stderr,"ISIL_AO_CTL_OpenChip err .\n");
            return NULL;
        }

        ret = ISIL_AO_CTL_GetChipHandle(chip_id ,&fd);
        if( ret < 0 ) {
            fprintf( stderr,"ISIL_AO_CTL_GetChipHandle err .\n");
            return NULL;
        }

        io_mgt->fd = fd;

        io_mgt->is_enable = 1;
    }
        break;

    default:
        return NULL;
    
    }
    

    return io_mgt;
}


int isil_open_av_io_package(char *path,unsigned int chip_id ,EIONODETYPE eionodetype)
{
    
    IO_NODE_MGT *io_mgt;
    
    io_mgt = open_av_io(path,chip_id,eionodetype);
    if( !io_mgt ) {
        return -1;
    }

    return 0;
}


IO_NODE_MGT *get_io_node_mgt_by_type_id(unsigned int chip_id,EIONODETYPE eionodetype)
{
    S_SINGLE_CHIP_MGT * s_single_chip_mgt;
    IO_NODE_MGT *io_mgt = NULL;

    
    s_single_chip_mgt = get_single_chip_mgt_by_id(chip_id);

    

    if( !s_single_chip_mgt ) {
        DEBUG_FUNCTION();
        return NULL;
    }

    if(eionodetype >= NODE_TYPE_BUBBON ) {
        DEBUG_FUNCTION();
        return NULL;
    }

    io_mgt = &s_single_chip_mgt->io_node_mgt_arr[eionodetype];

 

    return io_mgt;

}


void init_chip_enc_status_inf(CHIP_ENC_STATUS_INF *chip_enc_status)
{
	
	if(chip_enc_status) {
		memset(&chip_enc_status->h264_main_stream_status_arr,-1,sizeof(chip_enc_status->h264_main_stream_status_arr));
		memset(&chip_enc_status->h264_sub_stream_status_arr,-1,sizeof(chip_enc_status->h264_sub_stream_status_arr));
	}
}


int get_enc_chan_status_by_chip(unsigned int chip,CHIP_ENC_STATUS_INF *chip_enc_status)
{
	int i;
    S_ISIL_CHAN_MGT  *chan_mgt;
    S_SINGLE_CHIP_MGT * chip_mgt;
	S_ISIL_SINGLE_CHAN_MGT *single_chan;
    

    chip_mgt = get_single_chip_mgt_by_id(chip);
    if(!chip_mgt) {
        fprintf( stderr,"get_single_chip_mgt_by_id err.\n");
        return -1;
    }

    if( !chip_enc_status) {
        fprintf( stderr,"get_single_chip_mgt_by_id err.\n");
        return -1;
    }

	if(!chip_mgt->enc_chan_mgt_arr) {
		fprintf( stderr,"chip_mgt enc_chan_mgt_arr NULL.\n");
        return -1;
	}
	
	init_chip_enc_status_inf(chip_enc_status);
	chip_enc_status->chip_id = chip;

	for( i = 0 ; i < chip_mgt->enc_max_chan_num ; i++) {
		chan_mgt = chip_mgt->enc_chan_mgt_arr[i];
		if(chan_mgt) {
			single_chan = get_single_chan_mgt_by_chan_mgt(chan_mgt,IS_H264_MAIN_STREAM);
			if(single_chan) {
				chip_enc_status->h264_main_stream_status_arr[i] = single_chan->is_enable;
			}

			single_chan = get_single_chan_mgt_by_chan_mgt(chan_mgt,IS_H264_SUB_STREAM);
			if(single_chan) {
				chip_enc_status->h264_sub_stream_status_arr[i] = single_chan->is_enable;
			}
		}
	}

	return 0;

}





    
    


    

       








    









