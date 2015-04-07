/*
 * isil_decode_api.c
 *
 *  Created on: 2011-6-30
 *      Author: zoucx
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "isil_interface.h"
#include "ISIL_SDKError_Def.h"
#include "isil_decoder.h"
#include "isil_debug.h"
#include "isil_decode_api.h"
#include "net_thread_pool.h"
#include "isil_vo.h"
#include "isil_decode_file_process.h"
#include "isil_hndl_track_mode.h"
#include "isil_ap_mediasdk.h"
#include "isil_fm_play.h"
#include "isil_channel_map.h"




int isil_dec_msg_set_thr_param(int fd, int msg_type,int arg_len,void *arg)
{
  
    struct ISIL_MSG_MGT *msg_mgt;
    single_pb_mgt *s_pb_mgt;
    unsigned char *buff;
    DEC_MSG_T *dec_mgt_ptr;
    int ret;

    
    if( fd <= 0 ) {
        return -1;
    }

    if( !arg_len || !arg ) {
        return -1;
    }

    s_pb_mgt = get_s_pb_mgt_by_file_fd(  fd );
    if( !s_pb_mgt ) {
        return -1;
    }
    DEBUG_FUNCTION();
    msg_mgt = s_pb_mgt->msg_list;

    if( !msg_mgt ) {
        return -1;
         DEBUG_FUNCTION();
    }
    
    struct ISIL_MSG_BLK *msg_blk = msg_blk_minit();
    if( !msg_blk ) {
        return -1;
         DEBUG_FUNCTION();
    }

    buff = (unsigned char *)malloc(sizeof(DEC_MSG_T) + arg_len);
    if( !buff ) {
        msg_blk->release(msg_blk);
         DEBUG_FUNCTION();
        return -1;
    }
     DEBUG_FUNCTION();
    dec_mgt_ptr = (DEC_MSG_T *)buff;
    dec_mgt_ptr->msg_type = msg_type;
    dec_mgt_ptr->msg_len = arg_len;

    memcpy((void*)(buff + sizeof(DEC_MSG_T)) , arg , arg_len);
    msg_blk->param = (void *)buff;
    DEBUG_FUNCTION();
    msg_mgt->put_blk(msg_blk,msg_mgt);
    DEBUG_FUNCTION();

    if(msg_blk->wait) {
        msg_blk->wait(msg_blk);
    }
    DEBUG_FUNCTION();
    ret = GET_MSG_BLK_RESULT(msg_blk);

    if(msg_blk->param ) {
        free(msg_blk->param);
        msg_blk->param = NULL;
    }
    DEBUG_FUNCTION();
    if( msg_blk->release ) {
        msg_blk->release(msg_blk);
    }
    
    return ret;
}



int isil_set_dec_msg_by_single_pb_mgt(single_pb_mgt *s_pb_mgt, int msg_type,int arg_len,void *arg)
{
  
    struct ISIL_MSG_MGT *msg_mgt;
    
    unsigned char *buff;
    DEC_MSG_T *dec_mgt_ptr;
    int ret;

    
    if( !s_pb_mgt ) {
        return -1;
    }

    if( !arg_len || !arg ) {
        return -1;
    }

    
    msg_mgt = s_pb_mgt->msg_list;

    if( !msg_mgt ) {
        return -1;
    }
    
    struct ISIL_MSG_BLK *msg_blk = msg_blk_minit();
    if( !msg_blk ) {
        return -1;
    }

    buff = (unsigned char *)malloc(sizeof(DEC_MSG_T) + arg_len);
    if( !buff ) {
        msg_blk->release(msg_blk);
        return -1;
    }

    dec_mgt_ptr = (DEC_MSG_T *)buff;
    dec_mgt_ptr->msg_type = msg_type;
    dec_mgt_ptr->msg_len = arg_len;

    memcpy((void*)(buff + sizeof(DEC_MSG_T)) , arg , arg_len);
    msg_blk->param = (void *)buff;
   
    msg_mgt->put_blk(msg_blk,msg_mgt);

    if(msg_blk->wait) {
        msg_blk->wait(msg_blk);
    }

    ret = GET_MSG_BLK_RESULT(msg_blk);

    if(msg_blk->param ) {
        
        free(msg_blk->param);
        msg_blk->param = NULL;
    }

    DEBUG_FUNCTION();
    if( msg_blk->release ) {
        msg_blk->release(msg_blk);
    }
    DEBUG_FUNCTION();
    
    return ret;
}
int isil_remote_search_file( void * search_condition, void *buff)
{
    
    int ret ,len;

    MSG_FM_SEARCH *search_ptr;

    if( !search_condition ||  !buff ) {
        return -1;
    }
    
    search_ptr = ( MSG_FM_SEARCH *)search_condition;
    ret = ISIL_FM_SEACH_SearchFile(search_ptr, (void *)buff);
    if( ret < 0 ) {
        fprintf(stderr,"ISIL_FM_SEACH_SearchFile failed .\n");
        return -1;
    }

    MSG_FM_SEARCH_RESULT *pSrhRst = (MSG_FM_SEARCH_RESULT *)buff;

    len = sizeof(MSG_FM_SEARCH_RESULT) + pSrhRst->s32MsgNum*sizeof(MSG_FM_SEARCH_RESULT_1);

    return len;
   

}

int isil_remote_open_file_cb( void * stOpen_p, void* fileinfo)
{
    
    int ret;

    MSG_FM_REMOTE_FILEINFO stOpenInfo;
    MSG_FM_REMOTE_OPEN stOpen;

    if( !stOpen_p  && !fileinfo) {
        fprintf(stderr,"isil_remote_open_file_cb failed .\n");
        return -1;
    }

    memset(&stOpenInfo, 0x00, sizeof(MSG_FM_REMOTE_FILEINFO));
    memset(&stOpen, 0x00, sizeof(MSG_FM_REMOTE_OPEN));
    memcpy(&stOpen, stOpen_p, sizeof(MSG_FM_REMOTE_OPEN));

    ret = ISIL_FM_PLAY_Open(stOpen.reserve, stOpen, &stOpenInfo);
    if(ret < 0){
        fprintf(stderr,"ISIL_FM_PLAY_Open fail\n");
        ISIL_FM_PLAY_ClearPlayerByFd(stOpen.reserve);
        
        return -1;
        
    }
    memcpy(fileinfo, &stOpenInfo, sizeof(MSG_FM_REMOTE_FILEINFO));
    //stOpen.reserve = stOpenInfo.ulFileFd;

    return 0;
    
}



int isil_set_play_mod_param_cb( int fd, void * plyMod )
{
    if( fd <= 0 || !plyMod ) {
        return -1;
    }

    return isil_dec_msg_set_thr_param(fd , G_MSG_SET_PLAY_MOD , sizeof(G_ISIL_PLAY_MOD) , (void *)plyMod);
    
   
}

int isil_fm_remote_close_cb(int fd,void *stClose)
{
    
    if( fd <= 0 || !stClose ) {
        return -1;
    }

    return isil_dec_msg_set_thr_param(fd , MSG_TYPE_FM_REMOTE_CLOSE , sizeof(MSG_FM_REMOTE_OPERATOR) , (void *)stClose);

        
}


int isil_fm_remote_play_cb(int fd, void *stplay)
{
    if( fd <= 0 || !stplay ) {
        return -1;
    }

    return isil_dec_msg_set_thr_param(fd , MSG_TYPE_FM_REMOTE_PLAY , sizeof(MSG_FM_REMOTE_OPERATOR) , (void *)stplay);

}


int isil_fm_remote_pause_cb(int fd,void *stpause)
{
    if( fd <= 0 || !stpause ) {
        return -1;
    }
    
    return isil_dec_msg_set_thr_param(fd , MSG_TYPE_FM_REMOTE_PAUSE , sizeof(MSG_FM_REMOTE_OPERATOR) , (void *)stpause);
    
}


int isil_fm_remote_stop_cb(int fd,void *ststop)
{
    if( fd <= 0 || !ststop ) {
        return -1;
    }

    return isil_dec_msg_set_thr_param(fd , MSG_TYPE_FM_REMOTE_STOP , sizeof(MSG_FM_REMOTE_OPERATOR) , (void *)ststop);

    

}


int isil_fm_remote_get_frame_cb(int fd,void *get_frame_opt)
{
    if( fd <= 0 || !get_frame_opt ) {
        return -1;
    }

    return isil_dec_msg_set_thr_param(fd , MSG_TYPE_FM_REMOTE_GET_FRAME , sizeof(MSG_FM_REMOTE_OPERATOR) , (void *)get_frame_opt);

}

int isil_remote_get_fileinfo_cb(void *opt ,void *stRFileInfo)
{

    //MSG_FM_REMOTE_OPERATOR FileOpt;

    MSG_FM_REMOTE_FILEINFO *tmp_stRFileInfo;
    
    MSG_FM_REMOTE_OPERATOR *tmp_op;

    if( !opt || !stRFileInfo ) {
        return -1;
    }

    tmp_op = (MSG_FM_REMOTE_OPERATOR *)opt;

    tmp_stRFileInfo = ISIL_FM_PLAY_GetFileInfo(tmp_op->uiPlayFd);
    if( !tmp_stRFileInfo ) {
        return -1;
    }

    
    memcpy(&stRFileInfo, tmp_stRFileInfo, sizeof(MSG_FM_REMOTE_FILEINFO));

    return 0;
}

int isil_set_av_sync_cb(int fd ,void *avsync)
{
    if( fd <= 0 || !avsync ) {
        return -1;
    }

    return isil_dec_msg_set_thr_param(fd , G_MSG_SET_AV_SYNC , sizeof(G_FILE_AV_SYNC) , (void *)avsync);

}

int isil_dec_set_diaplay_port_mode_cb(unsigned int chipId ,void* portmode)
{
    if( !portmode ) {
        return -1;
    }

    return isil_set_display_port_mode(chipId ,(ISIL_DIAPLAY_PORT_MODE *)portmode);
    
}


int isil_bind_file_windows_cb(unsigned int chip_id,void *win)
{
    int ret;
    int len;
    single_pb_mgt *s_pb_mgt;

    ISIL_SUB_WINDOWS_BIND *tmp_win;

    if( !win ) {
        return -1;
    }

    tmp_win = (ISIL_SUB_WINDOWS_BIND *)win;


    s_pb_mgt = get_s_pb_mgt_by_file_fd( tmp_win->lVi_chId);
    if( !s_pb_mgt ) {

        s_pb_mgt = malloc_and_init_single_pb_mgt();
        assert(s_pb_mgt);
        set_file_fd_to_s_pb_mgt(tmp_win->lVi_chId,s_pb_mgt);
        set_s_pb_mgt_disply_ch(tmp_win->lDisplayId, s_pb_mgt);
        reg_s_pb_mgt_to_pb_list(s_pb_mgt,get_glb_pb_list());

        ret = net_glb_thread_dispatch(file_decode_thread, (void *)s_pb_mgt);
        if(ret < 0) {
            fprintf(stderr,"Create bind_decode_thread fail\n");

            /*TODO single_pb_mgt release*/

            return -1;        
        }
    }

    fprintf(stderr, "%s subwx %d subwy %d\n", __FUNCTION__, tmp_win->stSubInfo.stSubWindows[0].iSubWin_x, 
                tmp_win->stSubInfo.stSubWindows[0].iSubWin_y);


    len = sizeof(ISIL_SUB_WINDOWS_BIND)+((tmp_win->stSubInfo.cSubWindowsNum)*sizeof(ISIL_LOCCALGUI_RECT));
    return isil_set_dec_msg_by_single_pb_mgt(s_pb_mgt ,
                                             G_MSG_BIND_FILE_WINDOWS ,
                                             len, (void *)tmp_win);
}




int isil_bind_display_window(unsigned int nChipId ,
                             unsigned int nChanId ,
                             ISIL_SUB_WINDOWS *cfg)
{
    
    int i;
    int winCnt;
    int ret;
    int len;
    ISIL_LOCCALGUI_RECT* tmp;
    struct ISIL_display_chnl_property* disply;
    VO_CTL_MSG *pMsg;

    
    char buff[sizeof(VO_CTL_MSG)+sizeof(struct ISIL_display_chnl_property)];
    if(  !cfg ) {
        return -1;
    }

    winCnt = cfg->cSubWindowsNum;
    tmp = cfg->stSubWindows;
    len = sizeof(ISIL_LOCCALGUI_RECT);
    pMsg = (VO_CTL_MSG *)buff;

    pMsg->u32ChipID = nChipId;
    pMsg->u32Cmd = ISIL_DISPLAY_CHNL_CREATE;
    pMsg->u32MsgLen = sizeof(struct ISIL_display_chnl_property);

    disply = (struct ISIL_display_chnl_property *)(pMsg->pMsg);

    disply->i_bind_phase = cfg->iPhase;
    disply->i_bind_port = cfg->iPort;


    //printf("%s chip id %d wincnt %d\n", __FUNCTION__, nChipId,winCnt);
    for(i = 0; i < winCnt; i++){
        disply->i_phy_slot_id = ( winCnt > 1? (unsigned int)i:nChanId);
        memcpy(&disply->rect, tmp, len);
        fprintf(stderr, "bind disply win %d, x %d y %d w %d h %d\n", i, disply->rect.x,
        		disply->rect.y, disply->rect.width, disply->rect.height);
        ret = ISIL_VO_CTL_SendMsg( nChipId , pMsg);
        if( ret < 0 ) {
            fprintf( stderr,"ISIL_VO_CTL_SendMsg failed .\n");
            return -1;
        }
        tmp++;
    }

    return 0;

}


int isil_del_display_window(unsigned int chipId,unsigned int ch)
{
    int ret;

    VO_CTL_MSG* pMsg;

    char buff[sizeof(VO_CTL_MSG)+sizeof(int)];
    
    pMsg = (VO_CTL_MSG *)buff;

    pMsg->u32ChipID = chipId;
    pMsg->u32Cmd = ISIL_DISPLAY_CHNL_DESTROY;
    pMsg->u32MsgLen = sizeof(int);
    *(int*)pMsg->pMsg = ch;
    //printf("%s %d close ch %d\n", __FUNCTION__, __LINE__, ch);
    ret = ISIL_VO_CTL_SendMsg(0, pMsg);
    if(ret != 0)
    {
        fprintf( stderr,"ISIL_VO_CTL_SendMsg failed .\n");
        return -1;
    }
   
    return 0;
}


int isil_set_sub_window(unsigned int nChipId ,
                        unsigned int nChanId ,
                        ISIL_SUB_WINDOWS *cfg)
{
    int i;
    int winCnt;
    int ret;
    int len;
    ISIL_LOCCALGUI_RECT* tmp;
    struct ISIL_display_chnl_param* disply;

    VO_CTL_MSG *pMsg;

    
    char buff[sizeof(VO_CTL_MSG)+sizeof(struct ISIL_display_chnl_param)];

    if( !cfg ) {
        return -1;
    }

    winCnt = cfg->cSubWindowsNum;
    tmp = cfg->stSubWindows;
    len = sizeof(ISIL_LOCCALGUI_RECT);

    pMsg = (VO_CTL_MSG *)buff;
    disply = (struct ISIL_display_chnl_param *)pMsg->pMsg;
    pMsg->u32ChipID = nChipId;
    pMsg->u32Cmd = ISIL_DISPLAY_CHNL_PARAM_SET;
    pMsg->u32MsgLen = sizeof(struct ISIL_display_chnl_param);

    disply->b_enable = 1;
    //fprintf(stderr, "%s image type %d\n", __FUNCTION__, cfg->cImageType);

    if( cfg->cImageType == TYPE_PLAYBACK || cfg->cImageType == TYPE_LOOPBACK ) {
        disply->e_video_source = ISIL_DISPLAY_CHANNLE_SOURCE_VDEC;
    }
    else if( cfg->cImageType == TYPE_PREVIEW ) {
        
        disply->e_video_source = ISIL_DISPLAY_CHANNLE_SOURCE_VI;
    }
    else{
        disply->e_video_source = ISIL_DISPLAY_CHANNLE_SOURCE_VI;
    }

    disply->fps = 25;
    disply->i_bind_phase = cfg->iPhase;
    disply->i_bind_port = cfg->iPort;

    //printf("%s chip id %d wincnt %d\n", __FUNCTION__, nChipId,winCnt);
    for(i = 0; i < winCnt; i++){
        disply->i_phy_slot_id = (winCnt > 1? (unsigned int)i:nChanId);
        memcpy(&disply->rect, tmp, len);
        //fprintf(stderr, "set sub win %d, x %d y %d w %d h %d\n", i, disply->rect.x,
           //     		disply->rect.y, disply->rect.width, disply->rect.height);

        ret = ISIL_VO_CTL_SendMsg(nChipId, pMsg);
        if( ret < 0 ) {
            fprintf( stderr,"ISIL_VO_CTL_SendMsg failed .\n");
            return -1;
        }
        tmp++;
    }

    return 0;

}


int isil_set_video_standard(unsigned int chipId,
                            struct ISIL_display_video_standard* cfg)
{
    int ret;
    VO_CTL_MSG* pMsg;

    char buff[sizeof(VO_CTL_MSG)+sizeof(struct ISIL_display_video_standard)];

    if(NULL == cfg)
        return -1;

    pMsg = (VO_CTL_MSG *)buff;

    pMsg->u32ChipID = chipId;
    pMsg->u32Cmd = ISIL_DISPLAY_VIDEO_STANDARD_SET;
    pMsg->u32MsgLen = sizeof(struct ISIL_display_video_standard);


    memcpy(pMsg->pMsg, cfg, pMsg->u32MsgLen);
    ret = ISIL_VO_CTL_SendMsg(chipId, pMsg);
    if(ret != 0)
    {
        fprintf( stderr,"ISIL_VO_CTL_SendMsg failed .\n");
        return -1;
    }
    
    return 0;
}


int isil_set_display_port_mode(unsigned int chipId ,
                               ISIL_DIAPLAY_PORT_MODE* cfg)
{
    int ret;
    VO_CTL_MSG* pMsg;
    char buff[sizeof(VO_CTL_MSG)+sizeof(ISIL_DIAPLAY_PORT_MODE)];

    if(NULL == cfg)
        return -1;

    pMsg = (VO_CTL_MSG *)buff;

    pMsg->u32ChipID = chipId;
    pMsg->u32Cmd = ISIL_DISPLAY_PORT_MODE_SET;
    pMsg->u32MsgLen = sizeof(ISIL_DIAPLAY_PORT_MODE);


    memcpy(pMsg->pMsg, cfg, pMsg->u32MsgLen);

    ret = ISIL_VO_CTL_SendMsg(chipId, pMsg);
    if(ret != 0){
        fprintf( stderr,"ISIL_VO_CTL_SendMsg failed .\n");
        return -1;
    }
    
    return 0;
}

int isil_blind_privew_window(unsigned int chipId, ISIL_SUB_WINDOWS_BIND* WinCfg)
{
    unsigned int ch;
    int ret;
    BIND_VI2VO vi2vo;

    if(NULL == WinCfg){
        return -1;
    }

#if 0
    printf("%s: chip id 0x%x, loc id 0x%x, image type %d, disply ch %d\n",__FUNCTION__,
        WinCfg->lChipId, WinCfg->lVi_chId,WinCfg->stSubInfo.cImageType,
        WinCfg->lDisplayId);
#endif

    ch = WinCfg->lDisplayId;


    ret = isil_bind_display_window(chipId ,ch, &WinCfg->stSubInfo);
    if( ret < 0 ) {
        fprintf( stderr,"isil_bind_display_window failed .\n");
        return -1;
    }


    ret = isil_set_sub_window(chipId ,ch, &WinCfg->stSubInfo);
    if( ret < 0 ) {
        fprintf( stderr,"isil_set_sub_window failed .\n");
        return -1;
    }

    memset(&vi2vo, 0x00, sizeof(BIND_VI2VO));
    vi2vo.i_display_phy_id = ch;
    vi2vo.i_vi_phy_id = WinCfg->lVi_chId;


    ret = ISIL_DEC_CTL_BindVI2VO(chipId, &vi2vo);
    if( ret < 0) {
        fprintf( stderr,"ISIL_DEC_CTL_BindVI2VO failed .\n");
        return -1;
    }

    return 0;


}

int isil_blind_loopback_window(unsigned int chipId, ISIL_SUB_WINDOWS_BIND* WinCfg)
{
    unsigned int ch;
    int ret;
    BIND_DEC2VO dec2vo;

    if(NULL == WinCfg){
        return -1;
    }

#if 0
    printf("%s: chip id 0x%x, loc id 0x%x, image type %d, disply ch %d\n",__FUNCTION__,
        WinCfg->lChipId, WinCfg->lVi_chId,WinCfg->stSubInfo.cImageType,
        WinCfg->lDisplayId);
#endif

    ch = WinCfg->lDisplayId;


    ret = isil_bind_display_window(chipId ,ch, &WinCfg->stSubInfo);
    if( ret < 0 ) {
        fprintf( stderr,"isil_bind_display_window failed .\n");
        return -1;
    }


    ret = isil_set_sub_window(chipId ,ch, &WinCfg->stSubInfo);
    if( ret < 0 ) {
        fprintf( stderr,"isil_set_sub_window failed .\n");
        return -1;
    }

    memset(&dec2vo, 0x00, sizeof(BIND_VI2VO));
    dec2vo.i_display_phy_id = ch;
    //dec2vo.i_h264d_logic_id = WinCfg->lVi_chId;    
    dec2vo.i_h264d_logic_id = ISIL_GetLogicChnIDByPhyChnID(chipId, WinCfg->lVi_chId);

    ret = ISIL_DEC_CTL_BindDec2VO(chipId, &dec2vo);
    if( ret < 0) {
        fprintf( stderr,"ISIL_DEC_CTL_BindDec2VO failed .\n");
        return -1;
    }

    return 0;


}



static int thr_handle_file_close_sess(single_pb_mgt * s_pb_mgt ,MSG_FM_REMOTE_OPERATOR *stClose)
{
    int ret;

    BIND_DEC2VO  dec2VO;
    unsigned char video_chan_num;

    if( !s_pb_mgt || !stClose ) {
        return -1;
    }

    s_pb_mgt->is_pb_thr_run = 0;
    s_pb_mgt->need_read_file = 0;
    s_pb_mgt->have_audio  = 0;

    ret = ISIL_FM_PLAY_ClearPlayerByFd(get_s_pb_mgt_cur_file_fd(s_pb_mgt));
    if( ret < 0 ) {
        fprintf(stderr,"ISIL_FM_PLAY_ClearPlayerByFd err.\n");
        return -1;
    }

    video_chan_num = GET_S_PB_MGT_V_CH_NUM(s_pb_mgt);
    dec2VO.i_h264d_logic_id = video_chan_num;
    dec2VO.i_display_phy_id = stClose->uiPlayFd;

    /*TODO ISIL_CODEC_DEC_Readydie*/

    ret = ISIL_DEC_CTL_UnbindDec2VO(GET_S_PB_CHIP_ID(s_pb_mgt), &dec2VO);
    if( ret < 0 ) {
        fprintf(stderr,"ISIL_DEC_CTL_UnbindDec2VO err.\n");
        return -1;
    }

    ret = isil_del_display_window(GET_S_PB_CHIP_ID(s_pb_mgt) ,stClose->uiPlayFd);
    if( ret < 0 ) {
        fprintf(stderr,"ISIL_DEC_CTL_UnbindDec2VO err.\n");
        return -1;
    }
    


    return 0;
}


static int thr_handle_bind_file_windows_sess(single_pb_mgt * s_pb_mgt ,ISIL_SUB_WINDOWS_BIND* WinCfg)
{
    
    int ret;

    BIND_DEC2VO pD2VO;

    if( !s_pb_mgt || !WinCfg ) {
        return -1;
    }

    ret = isil_bind_display_window(GET_S_PB_CHIP_ID(s_pb_mgt),
                                   WinCfg->lDisplayId, 
                                   &WinCfg->stSubInfo);

    if( ret < 0 ) {
        fprintf(stderr,"isil_bind_display_window err.\n");
        return -1;
    }

    ret = isil_set_sub_window(GET_S_PB_CHIP_ID(s_pb_mgt),
                              WinCfg->lDisplayId, 
                              &WinCfg->stSubInfo);
    if( ret < 0 ) { 
        fprintf(stderr,"isil_set_sub_window err.\n");
        return -1;
    }

    pD2VO.i_display_phy_id = WinCfg->lDisplayId;
    pD2VO.i_h264d_logic_id = GET_S_PB_MGT_V_CH_NUM(s_pb_mgt);

    ret = ISIL_DEC_CTL_BindDec2VO( GET_S_PB_CHIP_ID(s_pb_mgt), &pD2VO);
    if( ret < 0 ) {
        fprintf(stderr,"ISIL_DEC_CTL_BindDec2VO err.\n");
        return -1;
    }


    return 0;
}


static int pb_set_vdec_mode(single_pb_mgt * s_pb_mgt ,G_ISIL_PLAY_MOD* plyMod)
{
    int ret;
    VDEC_CH_MODE chMod;
    MSG_FM_REMOTE_OPERATOR stmode;
    S_ISIL_SINGLE_CHAN_MGT *s_chan_mgt;

    if( !s_pb_mgt ||  !plyMod ) {
        return -1;
    }

    stmode.uiPlayFd = plyMod->fd;

    chMod.direction = plyMod->mod.direction;
    chMod.is_continue = plyMod->mod.is_continue;
    chMod.key_frame = plyMod->mod.key_frame;
    chMod.loop = plyMod->mod.loop;
    chMod.speed.x_speed_value = plyMod->mod.speed.x_speed_value;
    chMod.speed.mode = plyMod->mod.speed.mode;

    fprintf(stderr, "---l:%x,d:%x,k:%x,c:%x,x:%x---\n", chMod.loop,
             chMod.direction, chMod.key_frame, chMod.is_continue, (unsigned int)chMod.speed.x_speed_value);


    if( !s_pb_mgt->have_video ) {
        fprintf( stderr, "s_pb_mgt no video.\n");
        return -1;
    }

    s_chan_mgt = s_pb_mgt->sin_chan_mgt_array[DEC_MGT_VIDEO];
    if( !s_chan_mgt ) {
        fprintf( stderr, "s_chan_mgt video is null.\n");
        return -1;
    }

    ret = ISIL_CODEC_DEC_SetPBMode(&s_chan_mgt->codec_channel_t,&chMod);
    if( ret < 0 ) {
        fprintf( stderr,"ISIL_CODEC_DEC_SetPBMode err .\n");
        return -1;
    }

    if(chMod.direction == 0 ){

        if(chMod.is_continue) {

            if(GET_S_PB_CUR_MODE(s_pb_mgt) != BACKWARD_PB_MODE){

                SET_S_PB_CUR_MODE(s_pb_mgt,BACKWARD_PB_MODE);
                release_s_pb_mgt_av_buff_list(s_pb_mgt);
                s_pb_mgt->cur_pck = NULL;
            }
        }
        else{

            if(GET_S_PB_CUR_MODE(s_pb_mgt) != FRAMEBACK_PB_MODE){
                SET_S_PB_CUR_MODE(s_pb_mgt,FRAMEBACK_PB_MODE);
                release_s_pb_mgt_av_buff_list(s_pb_mgt);
                s_pb_mgt->cur_pck = NULL;

            }
        }
        
        SET_S_PB_NO_READ_FILE(s_pb_mgt);   
    }
    else{

        /*forwaord*/
        /*set direction*/
         stmode.FM_PLAY_CTL.cDirection = plyMod->mod.direction;
         ISIL_FM_PLAY_SetDirection(plyMod->fd, stmode);

            /*set key frame*/
         stmode.FM_PLAY_CTL.cKeyFrame = plyMod->mod.key_frame;
         ISIL_FM_PLAY_SetKeyFrame(plyMod->fd, stmode);

         if( chMod.is_continue ){
             if(GET_S_PB_CUR_MODE(s_pb_mgt) != NORMAL_PB_MODE) {
                 release_s_pb_mgt_av_buff_list(s_pb_mgt);
                 s_pb_mgt->cur_pck = NULL;
             }

             SET_S_PB_CUR_MODE(s_pb_mgt,NORMAL_PB_MODE);
             only_reg_read_cb_func(s_pb_mgt,dec_normal_read_file);
             SET_S_PB_READ_FILE(s_pb_mgt);

         }
         else{
             /*frame forward mode*/
                //DEBUG_FUNCTION();
                if(GET_S_PB_CUR_MODE(s_pb_mgt) == FRAMEBACK_PB_MODE ||
                   GET_S_PB_CUR_MODE(s_pb_mgt) == BACKWARD_PB_MODE ){
                    release_s_pb_mgt_av_buff_list(s_pb_mgt);
                    s_pb_mgt->cur_pck = NULL;
                }

                SET_S_PB_CUR_MODE(s_pb_mgt,FRAMEFORWARD_PB_MODE);
                //DEBUG_FUNCTION();
                only_reg_read_cb_func(s_pb_mgt,dec_frame_forward_read_file);
                //DEBUG_FUNCTION();
                SET_S_PB_READ_FILE(s_pb_mgt);
         }

    }
    return 0;
}



int parse_dec_msg(single_pb_mgt * s_pb_mgt ,struct ISIL_MSG_BLK *msg_node)
{
    int ret;
    DEC_MSG_T *dec_msg;
    unsigned char *buff;

    if( !msg_node || !msg_node->param) {
        
        fprintf( stderr,"parse_dec_msg err .\n");
        assert(0);
        return -1;
    }

    dec_msg = (DEC_MSG_T *)msg_node->param;
    if(!dec_msg->msg_len) {
        SET_MSG_BLK_RESULT(msg_node , -1);
        goto DEC_MSG_ERR;
    }

    buff = (unsigned char *)((unsigned char *)(msg_node->param) +sizeof(DEC_MSG_T));

    switch (dec_msg->msg_type) {
    case G_MSG_SET_PLAY_MOD:{

        G_ISIL_PLAY_MOD *plyMod = (G_ISIL_PLAY_MOD *)buff;
        MSG_FM_REMOTE_OPERATOR stmode;
        stmode.uiPlayFd = plyMod->fd;
        if(stmode.uiPlayFd != (unsigned int)get_s_pb_mgt_cur_file_fd(s_pb_mgt)) {
      
            SET_MSG_BLK_RESULT(msg_node , -1);
            goto DEC_MSG_ERR;
        }

        stmode.FM_PLAY_CTL.cLoop = 1;//plyMod->mod.loop;
        ret = ISIL_FM_PLAY_SetLoop(plyMod->fd, stmode);
        if( ret < 0 ) {
            
            fprintf(stderr,"ISIL_FM_PLAY_SetLoop err .\n");

            SET_MSG_BLK_RESULT(msg_node , -1);
            goto DEC_MSG_ERR;
        }

        /*TODO :pb_set_vdec_mode*/
#if 1
        if(pb_set_vdec_mode(s_pb_mgt ,plyMod) < 0){
            fprintf( stderr,"pb_set_vdec_mode failed .\n");
            return -1;
        }
#endif

        SET_MSG_BLK_RESULT(msg_node , 0);
    }
        break;
    case G_MSG_SET_AV_SYNC:{

        G_FILE_AV_SYNC* avsync = (G_FILE_AV_SYNC*)buff;

        if(!s_pb_mgt->is_audio_chan_open || !s_pb_mgt->is_video_chan_open){
            fprintf(stderr,"audio no open or video no open ,can,t av sync .\n");

            SET_MSG_BLK_RESULT(msg_node , -1);
            goto DEC_MSG_ERR;
        }

        ret = set_decoder_avsync(GET_S_PB_CHIP_ID(s_pb_mgt) ,s_pb_mgt->video_chan_num, s_pb_mgt->audio_chan_num);
        if( ret < 0 ) {
            fprintf(stderr,"set_decoder_avsync err.\n");
            SET_MSG_BLK_RESULT(msg_node , -1);
            goto DEC_MSG_ERR;
        }

        SET_MSG_BLK_RESULT(msg_node , 0);

    }
    break;

    case MSG_TYPE_FM_REMOTE_CLOSE:{

        ret = thr_handle_file_close_sess(s_pb_mgt,(MSG_FM_REMOTE_OPERATOR *)buff);
        if( ret < 0 ) {
            fprintf(stderr,"ISIL_FM_PLAY_ClearPlayerByFd err.\n");
            SET_MSG_BLK_RESULT(msg_node , -1);
            goto DEC_MSG_ERR;
        }

        SET_MSG_BLK_RESULT(msg_node , 0);

    }
        break;

    case MSG_TYPE_FM_REMOTE_PLAY:{
        
        MSG_FM_REMOTE_OPERATOR *stplay;
        stplay = (MSG_FM_REMOTE_OPERATOR *)buff;
        ret = ISIL_FM_PLAY_SetPlay(stplay->uiPlayFd);
        if( ret < 0 ) {
            fprintf(stderr,"ISIL_FM_PLAY_SetPlay err.\n");
            SET_MSG_BLK_RESULT(msg_node , -1);
            goto DEC_MSG_ERR;
        }

        SET_MSG_BLK_RESULT(msg_node , 0);
    }
        break;

    case MSG_TYPE_FM_REMOTE_PAUSE:{
        MSG_FM_REMOTE_OPERATOR *stpause;
        stpause = (MSG_FM_REMOTE_OPERATOR *)buff;
        ret = ISIL_FM_PLAY_SetPause(stpause->uiPlayFd);
        if( ret < 0 ) {

            fprintf(stderr,"ISIL_FM_PLAY_SetPause err.\n");
            SET_MSG_BLK_RESULT(msg_node , -1);
            goto DEC_MSG_ERR;
        }

        SET_MSG_BLK_RESULT(msg_node , 0);
    }
        break;
        
    case MSG_TYPE_FM_REMOTE_STOP:{

        MSG_FM_REMOTE_OPERATOR *ststop;
        ststop = (MSG_FM_REMOTE_OPERATOR *)buff;

        ret = ISIL_FM_PLAY_SetPause(ststop->uiPlayFd);
        if( ret < 0 ) {

            fprintf(stderr,"ISIL_FM_PLAY_SetPause err.\n");
            SET_MSG_BLK_RESULT(msg_node , -1);
            goto DEC_MSG_ERR;

        }

        SET_MSG_BLK_RESULT(msg_node , 0);
    }
        break;

    case MSG_TYPE_FM_REMOTE_GET_FRAME:{

        MSG_FM_REMOTE_OPERATOR *get_frame_opt;

        get_frame_opt = (MSG_FM_REMOTE_OPERATOR *)buff;
        
        if((unsigned int)get_s_pb_mgt_cur_file_fd(s_pb_mgt) != get_frame_opt->uiPlayFd) {
            fprintf(stderr,"GET_FRAME : fd err,\n");
            SET_MSG_BLK_RESULT(msg_node , -1);
            goto DEC_MSG_ERR;

        }
        ret = ISIL_FM_PLAY_SetPlay(get_frame_opt->uiPlayFd);
        if( ret < 0 ) {

            fprintf(stderr,"ISIL_FM_PLAY_SetPlay err.\n");
            SET_MSG_BLK_RESULT(msg_node , -1);
            goto DEC_MSG_ERR;

        }

        SET_S_PB_READ_FILE(s_pb_mgt);

        reg_s_pb_mgt_read_av_file(s_pb_mgt,dec_normal_read_file,NULL);


        SET_S_PB_CUR_MODE(s_pb_mgt,NORMAL_PB_MODE);

        SET_MSG_BLK_RESULT(msg_node , 0);
        fprintf(stderr, "MSG_TYPE_FM_REMOTE_GET_FRAME---004\n");
    }
        break;

    case NETLINK_REQ_MSG:{

        struct  query_info *pb_netlink_query;
        pb_netlink_query = (struct  query_info *)buff;
        if(pb_netlink_query->chan_id != GET_S_PB_MGT_V_CH_NUM(s_pb_mgt)) {
            SET_MSG_BLK_RESULT(msg_node , -1);
            goto DEC_MSG_ERR;
        }
        ret = dev_drv_need_frame_handle(s_pb_mgt,pb_netlink_query);
        if( ret < 0 ) {

            fprintf(stderr,"dev_drv_need_frame_handle err.\n");
            SET_MSG_BLK_RESULT(msg_node , -1);
            goto DEC_MSG_ERR;

        }

        SET_MSG_BLK_RESULT(msg_node , 0);
    }
        break;

    case G_MSG_BIND_FILE_WINDOWS:{
        ISIL_SUB_WINDOWS_BIND * tmp_win = (ISIL_SUB_WINDOWS_BIND *)buff;
        fprintf(stderr, "%s subwx %d subwy %d\n", __FUNCTION__, tmp_win->stSubInfo.stSubWindows[0].iSubWin_x, 
                tmp_win->stSubInfo.stSubWindows[0].iSubWin_y);
        ret = thr_handle_bind_file_windows_sess(s_pb_mgt ,
                                                (ISIL_SUB_WINDOWS_BIND *)buff);
        if( ret < 0 ) {

            fprintf(stderr,"thr_handle_bind_file_windows_sess err.\n");
            SET_MSG_BLK_RESULT(msg_node , -1);
            goto DEC_MSG_ERR;

        }

        SET_MSG_BLK_RESULT(msg_node , 0);

    }
        break;

    default:
        DEBUG_FUNCTION();
        SET_MSG_BLK_RESULT(msg_node , -1);
        break;
   }

DEC_MSG_ERR:
    
    ret =  GET_MSG_BLK_RESULT(msg_node);
    if(msg_node->weekup) {
        DEBUG_FUNCTION();
        msg_node->weekup(msg_node);
    }
    DEBUG_FUNCTION();

    return 0;
}





int isil_pb_netlink_req_frames_cb(struct  query_info *pb_netlink_query)
{
    struct ISIL_MSG_MGT *msg_mgt;
    single_pb_mgt *s_pb_mgt;
    unsigned char *buff;
    DEC_MSG_T *dec_mgt_ptr;

    if( !pb_netlink_query ) {
        return -1;
    }


    s_pb_mgt = get_s_pb_mgt_by_chan(pb_netlink_query->chan_id);
    if( !s_pb_mgt ) {
        return -1;
    }

    msg_mgt = s_pb_mgt->msg_list;

    if( !msg_mgt ) {
        return -1;
    }
    
    struct ISIL_MSG_BLK *msg_blk = msg_blk_minit();
    if( !msg_blk ) {
        return -1;
    }

    buff = (unsigned char *)malloc(sizeof(DEC_MSG_T) + sizeof(struct  query_info ));
    if( !buff ) {
        msg_blk->release(msg_blk);
        return -1;
    }

    dec_mgt_ptr = (DEC_MSG_T *)buff;
    dec_mgt_ptr->msg_type = NETLINK_REQ_MSG;
    dec_mgt_ptr->msg_len = sizeof(struct  query_info );

    memcpy((void*)(buff + sizeof(DEC_MSG_T)) , pb_netlink_query , sizeof(struct  query_info ));
    msg_blk->param = (void *)buff;
 
    msg_mgt->put_blk(msg_blk,msg_mgt);

    if(msg_blk->weekup) {
        msg_blk->weekup(msg_blk);
    }

    msg_blk->release(msg_blk);
    
    return 0;
}




S_ISIL_SINGLE_CHAN_MGT * isil_open_dec_chn(int chipid,int chn ,enum ECHANFUNCPRO type,int enable)
{


    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;

    DEBUG_FUNCTION();

    sin_chan_mgt = get_sin_chn_mgt_by_chip_chn_type(chipid,chn,CHAN_IS_DEC,type);
    if( !sin_chan_mgt ) {
        fprintf(stderr,"get_sin_chn_mgt_by_chip_chn_type dec err .\n");
        return NULL;
    }


    DEBUG_FUNCTION();

     if( enable ) {

        DEBUG_FUNCTION();
        if( GET_SINGLE_CHAN_ENABLE_PRO(sin_chan_mgt) != 1 ) {
            if( ISIL_CODEC_DEC_EnableCh(&sin_chan_mgt->codec_channel_t) != CODEC_ERR_OK ){
                fprintf(stderr, "ISIL_CODEC_ENC_EnableCh err .\n");
                sin_chan_mgt->release(sin_chan_mgt);
                return NULL;
            }
        }

        DEBUG_FUNCTION();
        SET_SINGLE_CHAN_ENABLE_PRO(sin_chan_mgt,1);

     }
     else{
         DEBUG_FUNCTION();
         if(GET_SINGLE_CHAN_ENABLE_PRO(sin_chan_mgt) != 0) {
             if( ISIL_CODEC_DEC_DisableCh(&sin_chan_mgt->codec_channel_t) != CODEC_ERR_OK ){
                fprintf(stderr, "ISIL_CODEC_ENC_EnableCh err .\n");
                sin_chan_mgt->release(sin_chan_mgt);
                return NULL;
            }
         }

         DEBUG_FUNCTION();
        SET_SINGLE_CHAN_ENABLE_PRO(sin_chan_mgt,0);

     }

     DEBUG_FUNCTION();
     return sin_chan_mgt;

}


int set_decoder_avsync(unsigned int chipId,unsigned short vchan, unsigned short achan)
{
    DEC_AV_SYNC* pAVSync = NULL;
    char buff[sizeof(DEC_AV_SYNC)+sizeof(struct syn_item)*2];
    pAVSync = (DEC_AV_SYNC*)buff;

    pAVSync->nr = 2;
    pAVSync->type = SYNC_METHOD_AV;
    pAVSync->idx_ref = 0;

    int i = 0;
    for(i = 0; i < pAVSync->nr; i++){
        struct syn_item* psyn_item = (struct syn_item*)((char *)(pAVSync->items) + i*sizeof(struct syn_item));
        psyn_item->type = i;

        if(!i){//video channel
            psyn_item->chan_id = vchan;
        }else{//audio channel
            psyn_item->chan_id = achan;
        }
    }

    
    fprintf(stderr, "--av sync,v[%x],a[%x]--\n", vchan, achan);

    if(ISIL_DEC_CTL_StartAVSync(chipId, pAVSync) < 0){
        fprintf(stderr, "--start av sync fail--\n");
        
        return -1;
    }
    
    return 0;
}


int create_video_dec_chan(single_pb_mgt * s_pb_mgt)
{
    int video_chan_num,ret,enable;
    CHIP_CHN_PHY_ID s_chip_chn_phy_id;
    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;
    VDEC_CH_HEADER streamheader = STREAM_WITH_HEADER;
    
    video_chan_num = get_idle_video_chan_num(get_glb_pb_list());
    if( video_chan_num < 0 ) {
        fprintf(stderr,"get_idle_video_chan_num err.\n");
        return -1;
    }

    fprintf(stderr,"Video dec chan = %d .\n",video_chan_num);

    s_pb_mgt->video_chan_num = video_chan_num;

    ret = dec_get_chip_chn_id_by_logic_id(video_chan_num,&s_chip_chn_phy_id);
    if( ret < 0 ) {
        fprintf( stderr,"get_chip_chn_id_by_logic_id err .\n");
        return -1;
    }
    
    if( s_pb_mgt->sin_chan_mgt_array[DEC_MGT_VIDEO] ) {
        fprintf( stderr,"DEC_MGT_VIDEO exit .\n");
        return -1;
    }

    enable = 1;
    sin_chan_mgt = isil_open_dec_chn(s_chip_chn_phy_id.chipid ,
                            s_chip_chn_phy_id.chanid ,
                            IS_H264_MAIN_STREAM ,
                            enable );

    if( !sin_chan_mgt ) {
        fprintf( stderr,"isil_open_dec_chn err .\n");
        return -1;
    }

    ISIL_CODEC_DEC_SetStreamType(&sin_chan_mgt->codec_channel_t, &streamheader);
    s_pb_mgt->sin_chan_mgt_array[DEC_MGT_VIDEO] = sin_chan_mgt;

    s_pb_mgt->video_fd = sin_chan_mgt->fd;
    s_pb_mgt->is_video_chan_open = 1;

    set_video_chan_num(s_pb_mgt->pb_parent, video_chan_num);
    return 0;
    
}


int close_video_dec_chan(single_pb_mgt * s_pb_mgt)
{
    int video_chan_num,ret,enable;
    CHIP_CHN_PHY_ID s_chip_chn_phy_id;
    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;

    if( !s_pb_mgt ) {
        fprintf( stderr,"s_pb_mgt is NULL .\n");
        return -1;
    }

    if( s_pb_mgt->is_video_chan_open ) {
        
        video_chan_num = s_pb_mgt->video_chan_num;
        
    
        ret = dec_get_chip_chn_id_by_logic_id(video_chan_num,&s_chip_chn_phy_id);
        if( ret < 0 ) {
            fprintf( stderr,"get_chip_chn_id_by_logic_id err .\n");
            return -1;
        }
        
        if( !s_pb_mgt->sin_chan_mgt_array[DEC_MGT_VIDEO] ) {
            fprintf( stderr,"DEC_MGT_VIDEO exit .\n");
            return -1;
        }
    
        enable = 0;
        sin_chan_mgt = isil_open_dec_chn(s_chip_chn_phy_id.chipid ,
                                s_chip_chn_phy_id.chanid ,
                                IS_H264_MAIN_STREAM ,
                                enable );
    
        if( !sin_chan_mgt ) {
            fprintf( stderr,"isil_open_dec_chn err .\n");
            return -1;
        }
    
        clear_video_chan_num(get_glb_pb_list(),video_chan_num);
    
        s_pb_mgt->sin_chan_mgt_array[DEC_MGT_VIDEO] = NULL;
    
        s_pb_mgt->video_fd = 0;
        s_pb_mgt->is_video_chan_open = 0;
    }

    return 0;
    
}



int create_audio_dec_chan(single_pb_mgt * s_pb_mgt)
{
    int audio_chan_num,ret,enable;
    CHIP_CHN_PHY_ID s_chip_chn_phy_id;
    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;

    if( !s_pb_mgt ) {
        return -1;
    }
    
    audio_chan_num = get_idle_audio_chan_num(get_glb_pb_list());
    if( audio_chan_num < 0 ) {
        fprintf(stderr,"get_idle_audio_chan_num err.\n");
        return -1;
    }

    fprintf(stderr,"audio_chan_num = %d .\n",audio_chan_num);

    ret = dec_get_chip_chn_id_by_logic_id(audio_chan_num,&s_chip_chn_phy_id);
    if( ret < 0 ) {
        fprintf( stderr,"get_chip_chn_id_by_logic_id err .\n");
        return -1;
    }
    
    if( s_pb_mgt->sin_chan_mgt_array[DEC_MGT_AUDIO] ) {
        fprintf( stderr,"DEC_MGT_AUDIO exit .\n");
        return -1;
    }

    enable = 1;
    sin_chan_mgt = isil_open_dec_chn(s_chip_chn_phy_id.chipid ,
                            s_chip_chn_phy_id.chanid ,
                            IS_AUDIO_STREAM ,
                            enable );

    if( !sin_chan_mgt ) {
        fprintf( stderr,"isil_open_dec_chn err .\n");
        return -1;
    }

    s_pb_mgt->sin_chan_mgt_array[DEC_MGT_AUDIO] = sin_chan_mgt;

    s_pb_mgt->audio_fd = sin_chan_mgt->fd;
    s_pb_mgt->is_audio_chan_open = 1;
    set_audio_chan_num(s_pb_mgt->pb_parent, audio_chan_num);

    return 0;
    
}


int close_audio_dec_chan(single_pb_mgt * s_pb_mgt)
{
    int audio_chan_num,ret,enable;
    CHIP_CHN_PHY_ID s_chip_chn_phy_id;
    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;

    if( !s_pb_mgt ) {
        fprintf( stderr,"s_pb_mgt is NULL .\n");
        return -1;
    }

    if( s_pb_mgt->is_audio_chan_open ) {
        
        audio_chan_num = s_pb_mgt->audio_chan_num;
        
    
        ret = dec_get_chip_chn_id_by_logic_id(audio_chan_num,&s_chip_chn_phy_id);
        if( ret < 0 ) {
            fprintf( stderr,"get_chip_chn_id_by_logic_id err .\n");
            return -1;
        }
        
        if( !s_pb_mgt->sin_chan_mgt_array[DEC_MGT_AUDIO] ) {
            fprintf( stderr,"DEC_MGT_VIDEO exit .\n");
            return -1;
        }
    
        enable = 0;
        sin_chan_mgt = isil_open_dec_chn(s_chip_chn_phy_id.chipid ,
                                s_chip_chn_phy_id.chanid ,
                                IS_AUDIO_STREAM ,
                                enable );
    
        if( !sin_chan_mgt ) {
            fprintf( stderr,"isil_open_dec_chn err .\n");
            return -1;
        }
    
        clear_audio_chan_num(get_glb_pb_list(),audio_chan_num);
    
        s_pb_mgt->sin_chan_mgt_array[DEC_MGT_AUDIO] = NULL;
    
        s_pb_mgt->audio_fd = 0;
        s_pb_mgt->is_audio_chan_open = 0;
    }

    return 0;
    
}





