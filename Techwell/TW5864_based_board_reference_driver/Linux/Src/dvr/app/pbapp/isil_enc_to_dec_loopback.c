#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <assert.h>


#include "isil_enc_to_dec_loopback.h"
#include "net_thread_pool.h"
#include "isil_pb_mgt.h"
#include "net_task.h"
#include "isil_data_stream.h"
#include "isil_io_utils.h"
#include "isil_ap_mediasdk.h"
#include "isil_media_config.h"
#include "isil_dec_sdk.h"
#include "isil_decode_api.h"
#include "isil_enc_data_reg.h"
#include "isil_decoder.h"
#include "isil_frame.h"
#include "isil_messagestruct.h"
#include "isil_preview_session.h"
#include "isil_channel_map.h"

#define DEC_SELECT_TIMEOUT (40000)



static void single_pb_mgt_loopback_release(single_pb_mgt *s_pb_mgt)
{
    
    
    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;
    struct NET_TASK *ev_task;

    if( !s_pb_mgt ) {
        return ;
    }

    ev_task = s_pb_mgt->ev_task;

    if( ev_task ) {

        if(ev_task->release) {
            ev_task->release(ev_task);
        }
    }

    s_pb_mgt->ev_task = NULL;

    if( s_pb_mgt->sin_chan_mgt_array[DEC_MGT_VIDEO] ) {
        sin_chan_mgt = s_pb_mgt->sin_chan_mgt_array[DEC_MGT_VIDEO];
        sin_chan_mgt->release(sin_chan_mgt);
        s_pb_mgt->sin_chan_mgt_array[DEC_MGT_VIDEO] = NULL;
    }
    
     

}

static void loopback_dec_thr_cb( void *arg)
{
    int ret,maxFd ;
    struct timeval tv;
    fd_set writeset;

    H264_INF_T *h264_inf_ptr;
    frame_t *frame;
    
    
    struct NET_TASK *ev_task;

    single_pb_mgt *s_pb_mgt;

    struct NET_DATA_MGT_T *data_mgt;

    struct NET_DATA_NODE_T *data_node;

    ISIL_AV_PACKET *av_pck;

    s_pb_mgt = (single_pb_mgt *)arg;

    assert(s_pb_mgt);

    

    ev_task = s_pb_mgt->ev_task;
    assert(ev_task);

    data_mgt = ev_task->net_data_mgt;
    assert(data_mgt);

    
    s_pb_mgt->is_pb_thr_run = 1;

    maxFd = s_pb_mgt->video_fd +1;

    fprintf( stderr,"video fd [%d].\n",s_pb_mgt->video_fd );

    

    while(s_pb_mgt->is_pb_thr_run ){

#if 0
        FD_ZERO(&writeset);

        tv.tv_sec = 0;
        tv.tv_usec = DEC_SELECT_TIMEOUT;

        maxFd = s_pb_mgt->video_fd +1;

        ret = select(maxFd,NULL,&writeset,NULL,&tv);
        if( ret == 0 ) {
            if( errno == ETIMEDOUT ) {
                fprintf(stderr, "---select timeout--\n");
            }
            else{
                fprintf(stderr, "other result %d--\n", errno);
            }
            
            continue;
        }
#endif

        data_node = data_mgt->get_net_data_node(data_mgt);
        if( !data_node ) {
            usleep(40000);
            continue;
        }

        av_pck = (ISIL_AV_PACKET *)data_node->arg;
        if( !av_pck ) {
            fprintf(stderr," Why : not av_pck ?.\n");
            data_node->release(data_node);
            break;
        }

        if( !av_pck->buff || !av_pck->date_len) {
            fprintf(stderr," av_pck buff or len err.\n");
            break;
        }

        if( !av_pck->priv ) {
            fprintf(stderr," av_pck priv NULL.\n");
            break;
        }

        if( av_pck->frm_type != ISIL_H264_DATA ) {
            fprintf( stderr,"frm_type is not ISIL_H264_DATA ,is [%d].\n",av_pck->frm_type);
            break;
        }

        if(  !s_pb_mgt->video_sync ) {
            h264_inf_ptr = &(av_pck->data_type_u.h264_inf);
            if( h264_inf_ptr->nalu_type != H264_NALU_IDRSLICE  ) {
                fprintf( stderr,"nal is not H264_NALU_IDRSLICE ,or H264_NALU_ISLICE.\n");
                goto LOOPBACK_RELEASE;
            }
            else{
                s_pb_mgt->video_sync = 1;
            }
        }

        frame = (frame_t *)(av_pck->priv);


        //isil_av_packet_debug(av_pck);

        
        //fprintf( stderr,"len[%d],fd[%d] .\n",av_pck->date_len,s_pb_mgt->video_fd );
        ret = r_write(s_pb_mgt->video_fd ,(char *)frame->buff,frame->len);
        if( ret < 0) {
            fprintf(stderr,"loop back write  data err .\n");
            data_node->release(data_node);
            break;
        }

LOOPBACK_RELEASE:
        //DEBUG_FUNCTION();
        data_node->release(data_node);
                
    }



    fprintf(stderr,"Loopback err ,exit .\n");
    
    if( s_pb_mgt->release ) {
        s_pb_mgt->release(s_pb_mgt);
    }

    return ;
}


static int loopback_set_net_task(single_pb_mgt *s_pb_mgt ,
                                 unsigned int enc_chip_id,
                                 unsigned int enc_chan_id,
                                 enum ECHANFUNCPRO chanpro)
{
    int ret;
    struct NET_TASK *ev_task;

    if( !s_pb_mgt ) {
    	fprintf(stderr,"loopback_set_net_task ERR\n");
        return -1;
    }

    if(chanpro != IS_H264_MAIN_STREAM && chanpro != IS_H264_SUB_STREAM ) {
    	fprintf(stderr,"IS_H264_MAIN_STREAM failed .\n");
        return -1;
    }

    ev_task = net_ev_task_minit();
    if( !ev_task ) {
        fprintf(stderr,"net_ev_task_minit failed .\n");
        return -1;
    }

    s_pb_mgt->ev_task = ev_task;
    ev_task->phy_chip_id = enc_chip_id;
    ev_task->phy_chan_id = enc_chan_id;

    if( ev_task->enc_param ) {
        fprintf( stderr,"ev_task->enc_param exist .\n");
        ev_task->release(ev_task);
        return -1;
        
    }

//    alloc_enc_param_t_to_net_task(ev_task);

    ret = net_task_reg_enc(ev_task,enc_chip_id,enc_chan_id,chanpro);
    if( ret < 0 ) {
        fprintf(stderr,"net_task_reg_enc failed .\n");
        ev_task->release(ev_task);
        return -1;
    }


    return 0;

}

int loopback_start( unsigned int chipid, unsigned int enc_chan_id,  void* win)
{

    int ret;
    VDEC_CH_HEADER vdec_ch_header_e; 
    CHIP_CHN_PHY_ID s_chip_chn_phy_id;

    

    single_pb_mgt *s_pb_mgt;

    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;
    ISIL_SUB_WINDOWS_BIND *tmpwin;
    #ifndef ISIL_USE_SYS_CFG_FILE
    ret = get_chip_chn_id_by_logic_id(enc_chan_id,&s_chip_chn_phy_id);
    #else
    //ret = ISIL_GetChipChnIDByLogicID(enc_chan_id,&s_chip_chn_phy_id);
    ret = ISIL_GetLogicChnIDByPhyChnID(chipid, enc_chan_id);
    s_chip_chn_phy_id.chipid = chipid;
    s_chip_chn_phy_id.chanid = ret;
    #endif
    if( ret < 0 ) {
        fprintf( stderr,"get_chip_chn_id_by_logic_id err .\n");
        return -1;
    }
    



    /*create dec chan ,start thread*/

    sin_chan_mgt = isil_open_dec_chn(s_chip_chn_phy_id.chipid,
                            s_chip_chn_phy_id.chanid ,
                            IS_H264_MAIN_STREAM ,
                            1);
    if( !sin_chan_mgt ) {
        fprintf( stderr,"isil_open_dec_chn err .\n");
        return -1;
    }


    
    vdec_ch_header_e = STREAM_WITH_HEADER;

    ret = ISIL_CODEC_DEC_SetStreamType(&sin_chan_mgt->codec_channel_t,
                                 &vdec_ch_header_e);
    if( ret != 0 ) {
        fprintf( stderr,"ISIL_CODEC_DEC_SetStreamType err.\n");
        return -1;
    }
    
    tmpwin = (ISIL_SUB_WINDOWS_BIND*)win;
    ret = loopback_bind_win(chipid, win);
    if( ret < 0 ) {
        fprintf(stderr,"loopback_bind_win err .\n");
        return -1;
    }
    
    s_pb_mgt = malloc_and_init_single_pb_mgt();
    if( !s_pb_mgt ) {
        fprintf( stderr,"malloc_and_init_single_pb_mgt err .\n");
        return -1;
    }

    s_pb_mgt->have_video = 1 ;
    s_pb_mgt->video_fd = GET_SINGLE_CHAN_FD(sin_chan_mgt);

    fprintf( stderr,"fd[%d] .\n",s_pb_mgt->video_fd );
    s_pb_mgt->sin_chan_mgt_array[DEC_MGT_VIDEO] = sin_chan_mgt;
    sin_chan_mgt->priv = (void *)s_pb_mgt;
    s_pb_mgt->release = single_pb_mgt_loopback_release;
    
    

    ret = loopback_set_net_task(s_pb_mgt,
                          s_chip_chn_phy_id.chipid ,
                          s_chip_chn_phy_id.chanid,
                          IS_H264_MAIN_STREAM);
    if(ret < 0 ) {
        fprintf( stderr,"loopback_set_net_task err .\n");
        s_pb_mgt->release(s_pb_mgt);
        /* TODO: release s_pb_mgt*/
        return -1;
    }
    
    
    

    ret = net_glb_thread_dispatch(loopback_dec_thr_cb , (void *)s_pb_mgt);
    if( ret < 0) {
        fprintf( stderr, "thread_dispatch loopback_dec_thr_cb failed .\n");
        s_pb_mgt->release(s_pb_mgt);
        return -1;
    }


    return 0;


}


int loopback_stop( unsigned int enc_chan_id )
{
    int ret;

    single_pb_mgt *s_pb_mgt;
    
#if 1
    CHIP_CHN_PHY_ID s_chip_chn_phy_id;
    #ifndef ISIL_USE_SYS_CFG_FILE
    ret = get_chip_chn_id_by_logic_id(enc_chan_id,&s_chip_chn_phy_id);
    #else
    ret = ISIL_GetChipChnIDByLogicID(enc_chan_id,&s_chip_chn_phy_id);
    #endif
    if( ret < 0 ) {
        fprintf( stderr,"get_chip_chn_id_by_logic_id err .\n");
        return -1;
    }

    /*TODO :wait for close thread*/
    
#endif

    s_pb_mgt  = get_s_pb_mgt_by_enc_id(s_chip_chn_phy_id.chipid,s_chip_chn_phy_id.chanid);
    if( !s_pb_mgt ) {
        fprintf( stderr,"get_s_pb_mgt_by_enc_id err .\n");
        return -1;
    }

    s_pb_mgt->is_pb_thr_run = 0;
    
    return 0;

}




