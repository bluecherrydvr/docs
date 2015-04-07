#include <stdio.h>

#include "isil_sdk_center_mgt.h"
#include "isil_enc_sdk.h"
#include "isil_sdk_main.h"
#include "isil_media_sdk_ev.h"
#include "isil_frame.h"
#include "isil_notify.h"
#include "isil_data_stream.h"
#include "isil_media_config.h"
#include "isil_debug.h"
#include "isil_ev_thr_mgt.h"


ISILERR_CODE ISIL_MediaSDK_Init( void )
{
    int ret;
    

    ret = get_chips_tree_by_sys_inf();
    if( ret < 0 ) {
        return ISIL_ERR_INITERROR;
    }

    ret = isil_init_sdk();
    if( ret < 0) {
        return ISIL_ERR_INITERROR;
    }

    //Maybe not need
    
    glb_enc_thr_mgt_init();

    

    return ISIL_ERR_SUCCESS;

}

ISILERR_CODE ISIL_MediaSDK_Run( void )
{
    int ret;

    ret = isil_sdk_run();
    if( ret < 0) {
        return ISIL_ERR_INITERROR;
    }

    return ISIL_ERR_SUCCESS;
}


ISILERR_CODE ISIL_MediaSDK_Cleanup( void )
{
    ISILERR_CODE ret = ISIL_ERR_SUCCESS;
    
    //Maybe not need
    DEBUG_FUNCTION();
    
    //glb_enc_thr_mgt_release();
    
    
    isil_sdk_release();

    DEBUG_FUNCTION();

    release_chips_mgt_tree();
    DEBUG_FUNCTION();

    return ret;
}

ISILERR_CODE ISIL_MediaSDK_Exit( void ){
    
    DEBUG_FUNCTION();


    sdk_thr_exit();
    DEBUG_FUNCTION();

    return ISIL_ERR_SUCCESS;
}


ISILERR_CODE ISIL_MediaSDK_GetChipCount( unsigned long *lpCount)
{
    int ret;

    if( !lpCount ) {
        return ISIL_ERR_FAILURE;
    }

    ret = isil_get_chips_count();

    *lpCount = ret;

    if( ret == 0 ) {
        return ISIL_ERR_FAILURE;
    }

    return ISIL_ERR_SUCCESS;

}


ISILERR_CODE ISIL_MediaSDK_GetChannelCount(unsigned long *lpCount)
{
    int ret;

    if( !lpCount ) {
        return ISIL_ERR_FAILURE;
    }

    ret = get_total_enc_count();

    *lpCount = ret;

    if( ret == 0 ) {
        return ISIL_ERR_FAILURE;
    }

    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE ISIL_MediaSDK_OpenChannel( unsigned int nChipId ,unsigned int nChnnel)
{

    S_ISIL_CHAN_MGT *chan_mgt;

    chan_mgt =  get_chan_mgt_by_chipid_chn(nChipId, nChnnel , CHAN_IS_ENC );
    if( !chan_mgt ) {
        return ISIL_ERR_FAILURE;
    }

    return ISIL_ERR_SUCCESS;

}


ISILERR_CODE ISIL_MediaSDK_CloseChannel( unsigned int nChipId ,unsigned int nChnnel)
{

    S_ISIL_CHAN_MGT *chan_mgt;
    S_ISIL_SINGLE_CHAN_MGT *s_chan_mgt;
    int echnpro;

    struct ISIL_EV_TASK *task;


    chan_mgt =  get_chan_mgt_by_chipid_chn(nChipId, nChnnel , CHAN_IS_ENC );
    if( !chan_mgt ) {
        return ISIL_ERR_FAILURE;
    }

    for( echnpro = IS_H264_MAIN_STREAM ; echnpro <  FUNCPROSIZE ; echnpro++) {
        s_chan_mgt = get_single_chan_mgt_by_chan_mgt(chan_mgt ,echnpro);

        if(s_chan_mgt) {

            //TODO: close channnel

            task = (struct ISIL_EV_TASK *)s_chan_mgt->priv;
            if( task ) {
                task->release(task);
            }
            s_chan_mgt->priv = NULL;

            //MUST be that;
            s_chan_mgt->release(s_chan_mgt);

        }

    }
    

    
    return ISIL_ERR_SUCCESS;
}


static ISILERR_CODE ISIL_MediaSDK_StartChannel( unsigned int nChipId ,unsigned int nChnnel ,enum ECHANFUNCPRO echnpro)
{
    
    int ret;

    ret = reg_date_rec_handle(nChipId , nChnnel ,echnpro , 1);
    if( ret < 0 ) {
        return ISIL_ERR_FAILURE;
    }

    return ISIL_ERR_SUCCESS;

}




ISILERR_CODE ISIL_MediaSDK_StartH264MainChannel(unsigned int nChipId ,unsigned int nChnnel)
{
    return  ISIL_MediaSDK_StartChannel(nChipId ,nChnnel ,IS_H264_MAIN_STREAM);
}


ISILERR_CODE ISIL_MediaSDK_StartH264SubChannel(unsigned int nChipId ,unsigned int nChnnel)
{
    return  ISIL_MediaSDK_StartChannel(nChipId ,nChnnel ,IS_H264_SUB_STREAM);
}


ISILERR_CODE ISIL_MediaSDK_StartAudioChannel(unsigned int nChipId ,unsigned int nChnnel)
{
    return  ISIL_MediaSDK_StartChannel(nChipId ,nChnnel ,IS_AUDIO_STREAM);
}

ISILERR_CODE ISIL_MediaSDK_StartCapMjpeg(unsigned int nChipId ,unsigned int nChnnel)
{
    return  ISIL_MediaSDK_StartChannel(nChipId ,nChnnel ,IS_MJPEG_STREAM);
}


static ISILERR_CODE ISIL_MediaSDK_StopChannel( unsigned int nChipId ,unsigned int nChnnel ,enum ECHANFUNCPRO echnpro)
{

    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;

    sin_chan_mgt = isil_open_enc_chn(nChipId ,nChnnel ,echnpro , 0);
    if( !sin_chan_mgt  ) {
        return ISIL_ERR_FAILURE;
    }

    return ISIL_ERR_SUCCESS;
}


ISILERR_CODE ISIL_MediaSDK_StopH264MainChannel(unsigned int nChipId ,unsigned int nChnnel)
{
    return  ISIL_MediaSDK_StopChannel(nChipId ,nChnnel ,IS_H264_MAIN_STREAM);
}

ISILERR_CODE ISIL_MediaSDK_StopH264SubChannel(unsigned int nChipId ,unsigned int nChnnel)
{
    return  ISIL_MediaSDK_StopChannel(nChipId ,nChnnel ,IS_H264_SUB_STREAM);
}


ISILERR_CODE ISIL_MediaSDK_StopAudioChannel(unsigned int nChipId ,unsigned int nChnnel)
{
    return  ISIL_MediaSDK_StopChannel(nChipId ,nChnnel ,IS_AUDIO_STREAM);
}

ISILERR_CODE ISIL_MediaSDK_StopCapMjpeg(unsigned int nChipId ,unsigned int nChnnel)
{
    return  ISIL_MediaSDK_StopChannel(nChipId ,nChnnel ,IS_MJPEG_STREAM);
}



ISILERR_CODE ISIL_MediaSDK_SetBufferPool(unsigned int num , unsigned int size)
{

    struct ISIL_FramePool  *glb_frame_pool ;
    
    if( num == 0 || size == 0 ) {
        return ISIL_ERR_FAILURE;
    }

    if( size < MINFRAMEBUFFSIZE) {
        return ISIL_ERR_FAILURE;
    }

    glb_frame_pool = get_glb_frame_pool();

    if(glb_frame_pool->is_init) {
        return ISIL_ERR_FAILURE;
    }

    glb_frame_pool->max_buff_num = num ;
    glb_frame_pool->max_buff_size  = size;

    return ISIL_ERR_SUCCESS;
}


ISILERR_CODE ISIL_MediaSDK_RegPreviewCallback(unsigned int nChipId,
                                            unsigned int nChnnel,
                                            MediaSDKPreviewCallBack *funcCallBack,
                                            void *pContext)
{
    return ISIL_ERR_SUCCESS;
}


ISILERR_CODE ISIL_MediaSDK_RegEncCallback(unsigned int nChipId,
                                            unsigned int nChnnel,
                                            enum ECHANFUNCPRO pro,
                                            MediaSDKCallBack *funcCallBack,
                                            void *pContext)
{
    int ret;

    S_ISIL_SINGLE_CHAN_MGT *s_chan_mgt;

    struct ISIL_EV_TASK *task;

    struct ISIL_NOTIFY_BLK *notify_blk;

    struct ISIL_NOTIFY_LST *notify_lst;

    
    DEBUG_FUNCTION();

    ret  = reg_date_rec_handle(nChipId ,nChnnel ,pro , 0);
    if( ret < 0  ) {
        return ISIL_ERR_FAILURE;
    }
    
    DEBUG_FUNCTION();
    s_chan_mgt = get_sin_chn_mgt_by_chip_chn_type(nChipId , nChnnel , CHAN_IS_ENC ,pro);
    if( !s_chan_mgt ) {
        return ISIL_ERR_FAILURE;
    }

    

    if( !s_chan_mgt->priv ) {
        return ISIL_ERR_FAILURE;
    }

    

    task = (struct ISIL_EV_TASK *)s_chan_mgt->priv;

    notify_lst = &task->notify_list;
    if( notify_lst->notify_num ) {
        fprintf( stderr,"notify_lst have reg,no need .\n");
        return ISIL_ERR_SUCCESS;
    }

    

    notify_blk  = notify_blk_init();
    if( !notify_blk ) {
        return ISIL_ERR_FAILURE;
    }

    

    SET_NOTIFY_BLK_NOTIFY_TYPE(notify_blk ,ISIL_NOTIFY_FOREVER);

    reg_notify_blk_callback(notify_blk, pContext , funcCallBack);

    
    DEBUG_FUNCTION();
    notify_lst->put_notify_blk(notify_blk , notify_lst );
    DEBUG_FUNCTION();

    return ISIL_ERR_SUCCESS;

}


ISILERR_CODE ISIL_MediaSDK_RegH264MainCallback(unsigned int nChipId,
                                            unsigned int nChnnel,
                                            MediaSDKH264CallBack *funcCallBack,
                                            void *pContext)
{
    

        return ISIL_MediaSDK_RegEncCallback(nChipId ,
                                            nChnnel,
                                            IS_H264_MAIN_STREAM,
                                            funcCallBack,
                                            pContext);
        
}


ISILERR_CODE ISIL_MediaSDK_RegH264SubCallback(unsigned int nChipId,
                                                                                        unsigned int nChnnel,
                                                                                        MediaSDKH264CallBack *funcCallBack,
                                                                                        void *pContext)
{
    

        return ISIL_MediaSDK_RegEncCallback(nChipId ,
                                            nChnnel,
                                            IS_H264_SUB_STREAM,
                                            funcCallBack,
                                            pContext);
    
}



ISILERR_CODE ISIL_MediaSDK_RegAudioCallback(unsigned int nChipId,
                                                                                      unsigned int nChnnel,                                                                                     
                                                                                        MediaAudioCallBack *funcCallBack,
                                                                                        void *pContext)
{
        
        return ISIL_MediaSDK_RegEncCallback(nChipId ,
                                            nChnnel,
                                            IS_AUDIO_STREAM,
                                            funcCallBack,
                                            pContext);
        
}


ISILERR_CODE ISIL_MediaSDK_RegMJpegCallback(unsigned int nChipId,
                                                    unsigned int nChnnel,                                                                                        
                                                    MediaMJpegCallBack *funcCallBack,
                                                    void *pContext)
{
    

        return ISIL_MediaSDK_RegEncCallback(nChipId ,
                                            nChnnel,
                                            IS_MJPEG_STREAM,
                                            funcCallBack,
                                            pContext);
    
}


ISILERR_CODE ISIL_MediaSDK_RegAlarmCallback(unsigned int nChipId,
                                            MediaSDKAlarmCallBack *funcCallBack,
                                            void *pContext)
{
    int ret;

    IO_NODE_MGT *io_mgt;

    struct ISIL_EV_TASK *task;

    struct ISIL_NOTIFY_BLK *notify_blk;

    struct ISIL_NOTIFY_LST *notify_lst;

    if( !funcCallBack ) {
        return ISIL_ERR_FAILURE;
    }
    
    ret = create_alarm_ev(nChipId);
    if( ret < 0 ) {
        fprintf( stderr," create_alarm_ev failed .\n");
        return ISIL_ERR_FAILURE;
    }
    
    io_mgt  = get_io_node_mgt_by_type_id(nChipId , NODE_TYPE_VI);
    if( !io_mgt ) {
        fprintf( stderr," get_io_node_mgt_by_type_id failed .\n");
        return ISIL_ERR_FAILURE;
    }

    task = (struct ISIL_EV_TASK *)io_mgt->priv;
    if( !task ) {
        fprintf( stderr," get alarm task failed .\n");
        return ISIL_ERR_FAILURE;
    }

    notify_lst = &task->notify_list;
    if( notify_lst->notify_num ) {
        fprintf( stderr,"notify_lst have reg,no need .\n");
        return ISIL_ERR_SUCCESS;
    }

    DEBUG_FUNCTION();

    notify_blk  = notify_blk_init();
    if( !notify_blk ) {
        return ISIL_ERR_FAILURE;
    }

    DEBUG_FUNCTION();

    SET_NOTIFY_BLK_NOTIFY_TYPE(notify_blk ,ISIL_NOTIFY_FOREVER);

    reg_notify_blk_callback(notify_blk, pContext , funcCallBack);
    

    notify_lst->put_notify_blk(notify_blk , notify_lst );



    return ISIL_ERR_SUCCESS;
}


ISILERR_CODE ISIL_MediaSDK_StopAlarm(unsigned int nChipId )
{
    return ISIL_ERR_SUCCESS;
}

ISILERR_CODE ISIL_MediaSDK_StartAlarm(unsigned int nChipId )
{
    
    int ret;

    IO_NODE_MGT *io_mgt;
    struct ISIL_EV_TASK *task;

    io_mgt  = get_io_node_mgt_by_type_id(nChipId , NODE_TYPE_VI);
    if( !io_mgt ) {
        
        return ISIL_ERR_SUCCESS;
    }

    if( !io_mgt->priv ) {
        return ISIL_ERR_SUCCESS;
    }

    task = (struct ISIL_EV_TASK *)(io_mgt->priv );
    

    return ISIL_ERR_SUCCESS;
}








