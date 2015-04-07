#include <stdio.h>
#include <stdlib.h>

#include "isil_preview_session.h"
#include "isil_decode_api.h"
#include "isil_codec_chip_ctl.h"
#include "isil_pb_mgt.h"
#include "isil_ap_mediasdk.h"
#include "isil_enc_to_dec_loopback.h"
#include "isil_channel_map.h"

 


int preview_start_cb(unsigned int chipId, void* WinCfg)
{
    int ret;
    ISIL_SUB_WINDOWS_BIND* tmp_wincfg;

    if( !WinCfg ) {
        return -1;
    }

    tmp_wincfg = (ISIL_SUB_WINDOWS_BIND *)WinCfg;

    /*TODO: some works*/

    ret = isil_blind_privew_window(chipId , tmp_wincfg);
    if( ret < 0 ) {
        fprintf( stderr,"isil_blind_privew_window failed .\n");
        return -1;
    }

    return 0;
}

int loopback_bind_win(unsigned int chipId, void* WinCfg)
{
    int ret;
    ISIL_SUB_WINDOWS_BIND* tmp_wincfg;

    if( !WinCfg ) {
        return -1;
    }

    tmp_wincfg = (ISIL_SUB_WINDOWS_BIND *)WinCfg;

    /*TODO: some works*/

    ret = isil_blind_loopback_window(chipId , tmp_wincfg);
    if( ret < 0 ) {
        fprintf( stderr,"isil_blind_loopback_window failed .\n");
        return -1;
    }

    return 0;
}



int preview_stop_cb(unsigned int chipId, void  *stClose )
{
    int ret;

    MSG_FM_REMOTE_OPERATOR  *tmp_stClose;
    BIND_VI2VO  Vi2VO;


    if( !stClose ) {
        return -1;
    }

    tmp_stClose = (MSG_FM_REMOTE_OPERATOR *)stClose;


    Vi2VO.i_display_phy_id = tmp_stClose->uiPlayFd;

    Vi2VO.i_vi_phy_id = tmp_stClose->uiPlayFd;


    ret = ISIL_DEC_CTL_UnbindVI2VO(chipId, &Vi2VO);
    if( ret < 0 ) {
        fprintf( stderr,"ISIL_DEC_CTL_UnbindVI2VO failed .\n");
        return -1;
    }

    ret = isil_del_display_window(chipId ,tmp_stClose->uiPlayFd);
    if( ret < 0 ) {
        fprintf( stderr,"isil_del_display_window failed .\n");
        return -1;
    }

    return 0;

}


int isil_union_remote_close_cb(int fd,void *stClose)
{
    unsigned int tmp_ch ;
    int ret;
    CHIP_CHN_PHY_ID s_chip_chn_phy_id;
    single_pb_mgt *s_pb_mgt = NULL;

    MSG_FM_REMOTE_OPERATOR  *tmp_stclose ;

    if( !stClose ) {
        return -1;
    }

    

    tmp_stclose = (void *)stClose;

    tmp_ch = tmp_stclose->uiPlayFd;

    s_pb_mgt = get_s_pb_mgt_by_disply_chan(tmp_ch);
    if(s_pb_mgt){
        /* TODO: loopback or file dec ,need s_pb_mgt add priority*/

        ret =isil_fm_remote_close_cb(fd ,stClose);
        if( ret < 0 ) {
            return -1;
        }
    }
    else{
        #if 0
        #ifndef ISIL_USE_SYS_CFG_FILE
        ret = get_chip_chn_id_by_logic_id(fd,&s_chip_chn_phy_id);
        #else
        ret = ISIL_GetChipChnIDByLogicID(fd,&s_chip_chn_phy_id);
        #endif
        #endif
        ret = get_chip_chn_id_by_logic_id(fd,&s_chip_chn_phy_id);
        if( ret < 0 ) {
            fprintf(stderr,"get_chip_chn_id_by_logic_id err .\n");
            return -1;
        }

        ret = preview_stop_cb(s_chip_chn_phy_id.chipid ,stClose);
        if( ret < 0 ) {
            return -1;
        }
    }
    
    return 0;

}

int isil_union_bind_windows_cb(unsigned int chip_id,void *win)
{
    int ret;
    ISIL_SUB_WINDOWS_BIND* tmp_win;
    CHIP_CHN_PHY_ID s_chip_chn_phy_id;
    tmp_win = (ISIL_SUB_WINDOWS_BIND *)win;


    if( !tmp_win ) {
        return -1;
    }

    /*TODO :lLogicId is not exist, modify it*/
    if( tmp_win->emViType == TYPE_PLAYBACK) {

         /* TODO: loopback or file dec ,need s_pb_mgt add priority*/
       // fprintf(stderr, "%s subwx %d subwy %d\n", __FUNCTION__, tmp_win->stSubInfo.stSubWindows[0].iSubWin_x, 
         //       tmp_win->stSubInfo.stSubWindows[0].iSubWin_y);
        ret = isil_bind_file_windows_cb(chip_id ,win);
        if( ret < 0 ) {
            return -1;
        }
    }
    else if(tmp_win->emViType == TYPE_PREVIEW ){
        #ifndef ISIL_USE_SYS_CFG_FILE
		ret = get_chip_chn_id_by_logic_id(chip_id,&s_chip_chn_phy_id);
        #else
        ret = ISIL_GetChipChnIDByLogicID(chip_id,&s_chip_chn_phy_id);
        #endif

		if( ret < 0 ) {
			fprintf(stderr,"get_chip_chn_id_by_logic_id err .\n");
			return -1;
		}
        ret = preview_start_cb(s_chip_chn_phy_id.chipid, win);
        if( ret < 0 ) {
            return -1;
        }
    }
    else if(tmp_win->emViType == TYPE_LOOPBACK ){
    	fprintf(stderr, "bind loopback\n");
        
        #ifndef ISIL_USE_SYS_CFG_FILE
    	ret = get_chip_chn_id_by_logic_id(chip_id,&s_chip_chn_phy_id);
        #else
        ret = ISIL_GetChipChnIDByLogicID(chip_id,&s_chip_chn_phy_id);
        #endif
		if( ret < 0 ) {
			fprintf(stderr,"get_chip_chn_id_by_logic_id err .\n");
			return -1;
		}

        ret = loopback_start(s_chip_chn_phy_id.chipid, tmp_win->lVi_chId, win);
		if( ret < 0 ) {
            fprintf(stderr,"loopback_start err .\n");
			return -1;
		}
        /*
        ret = loopback_bind_win(s_chip_chn_phy_id.chipid, win);
		if( ret < 0 ) {
            fprintf(stderr,"loopback_bind_win err .\n");
			return -1;
		}
        */
		

		

    }

    return 0;

}





