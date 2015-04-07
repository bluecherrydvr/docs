#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


#include "isil_file_record_session.h"
#include "isil_recapp.h"
#include "isil_ap_mediasdk.h"
#include "isil_messagestruct.h"
#include "isil_codec_common.h"
#include "isil_file_rec_mgt.h"
#include "isil_media_config.h"
#include "isil_enc_data_reg.h"
#include "isil_fm_file_allocate.h"
#include "isil_fm_fdisk.h"
#include "isil_fm_log.h"
#include "isil_fm_play.h"
#include "isil_channel_map.h"



static void task_release_file_rec_cb(struct NET_TASK *ev_task)
{
    struct FILE_REC_TASK_T *file_rec_sess;
    if(ev_task) {
        if(ev_task->priv) {
            file_rec_sess = (struct FILE_REC_TASK_T *)ev_task->priv;
            file_rec_sess->task = NULL;
            ev_task->priv = NULL;
        }
    }
}


static int file_task_write_cb(struct NET_TASK *file_task ,void *src)
{
    int ret;
    
    

    if( !file_task ) {
        fprintf(stderr,"file_task NULL .\n");
        return -1;
    }

    if( !file_task->priv ) {
        fprintf(stderr,"file_task->priv NULL .\n");
        return -1;
    }

    

    ret = isil_recapp_write(file_task->priv);
    if( ret < 0 ) {

        struct FILE_REC_TASK_T *file_rec_sess;
        RECAPP_COMM s_recapp_comm;
        file_rec_sess = (struct FILE_REC_TASK_T *)file_task->priv;
        s_recapp_comm.chan = file_rec_sess->logic_chan_id;
        isil_recapp_close( (void *) &s_recapp_comm);

        fprintf(stderr,"isil_recapp_write err .\n");
        return -1;
    }

    return 0;

}


static int new_file_task_write_cb(struct NET_TASK *file_task ,void *src)
{
    int ret;
    
    

    if( !file_task ) {
        fprintf(stderr,"file_task NULL .\n");
        return -1;
    }

    if( !file_task->priv ) {
        fprintf(stderr,"file_task->priv NULL .\n");
        return -1;
    }

    if( !src ) {
        return -1;
    }

    

    ret = new_isil_recapp_write(file_task->priv , src);
    if( ret < 0 ) {

        struct FILE_REC_TASK_T *file_rec_sess;
        RECAPP_COMM s_recapp_comm;
        file_rec_sess = (struct FILE_REC_TASK_T *)file_task->priv;
        s_recapp_comm.chan = file_rec_sess->logic_chan_id;
        isil_recapp_close( (void *) &s_recapp_comm);

        fprintf(stderr,"isil_recapp_write err .\n");
        return -1;
    }

    return 0;

}

int start_record_file(void *p_fm_start_rec)
{
    int ret ,chan;

    CHIP_CHN_PHY_ID s_chip_chn_phy_id;
    RECAPP_COMM s_recapp_comm;
    RECAPP_START recapp_start_t;
    MSG_FM_START_REC *fm_start_rec_ptr;
    struct FILE_REC_TASK_T *file_rec_sess;
    struct NET_TASK *file_task;

    if( !p_fm_start_rec ) {
        return -1;
    }

    fm_start_rec_ptr = (MSG_FM_START_REC *)p_fm_start_rec;

    memset(&recapp_start_t ,0x00 ,sizeof(RECAPP_START));
    
    recapp_start_t.chan = fm_start_rec_ptr->lChannel;

    recapp_start_t.maintype = fm_start_rec_ptr->ucRecType;
    recapp_start_t.subtype = fm_start_rec_ptr->ucRecSubType;

    chan = recapp_start_t.chan;

    s_recapp_comm.chan = chan;
    #ifndef ISIL_USE_SYS_CFG_FILE
    ret = get_chip_chn_id_by_logic_id(chan,&s_chip_chn_phy_id);
    #else
    ret = ISIL_GetChipChnIDByLogicID(chan,&s_chip_chn_phy_id);
    #endif
    if( ret < 0 ) {
        //isil_recapp_close( (void *)&s_recapp_comm);
        return -1;
    }

    file_rec_sess = get_file_rec_task_by_inf(s_chip_chn_phy_id.chipid,s_chip_chn_phy_id.chanid);
    if( !file_rec_sess ) {
        return -1;
    }

    file_rec_sess->logic_chan_id = chan;

    if(file_rec_sess->task) {
        fprintf(stderr,"file_rec_sess->task exist,file have start.\n");
        return 0;
    }

    file_task = net_ev_task_minit();
    if( !file_task ) {
        fprintf(stderr,"net_ev_task_minit failed .\n");
        return -1;
    }

    file_rec_sess->task = file_task;

    net_task_reg_priv_release_cb(file_task, (void *)file_rec_sess ,task_release_file_rec_cb);
    
    



    ret = isil_recapp_startrec( (void *)&recapp_start_t);
    if(ret < 0 ) {
        fprintf(stderr,"isil_recapp_startrec failed .\n");
        file_task->release(file_task);
        return -1;
    }

#ifdef USE_NEW_DATA_DISPATCH_POLICY

    reg_net_task_write_callback(file_task , new_file_task_write_cb);

#else

    reg_net_task_write_callback(file_task , file_task_write_cb);

#endif

    ret = file_rec_thr_alloc_task(file_task);
    if( ret < 0 ) {
        fprintf(stderr,"file_rec_thr_alloc_task failed .\n");
        file_task->release(file_task);
        return -1;
    }
    
    if(fm_start_rec_ptr->u8Stream & REC_MAIN_VIDEO){
        ret = net_task_reg_enc(file_task,s_chip_chn_phy_id.chipid,s_chip_chn_phy_id.chanid,IS_H264_MAIN_STREAM);
        if( ret < 0 ) {
            fprintf(stderr,"net_task_reg_enc failed .\n");
            isil_recapp_close( (void *) &s_recapp_comm);
            file_task->release(file_task);
            return -1;
        }
    }

    if(fm_start_rec_ptr->u8Stream & REC_SUB_VIDEO){
        ret = net_task_reg_enc(file_task,s_chip_chn_phy_id.chipid,s_chip_chn_phy_id.chanid,IS_H264_SUB_STREAM);
        if( ret < 0 ) {
            fprintf(stderr,"net_task_reg_enc failed .\n");
            isil_recapp_close( (void *) &s_recapp_comm);
            file_task->release(file_task);
            return -1;
        }
    }

    if(fm_start_rec_ptr->u8Stream & REC_ONLY_AUDIO){
        ret = net_task_reg_enc(file_task,s_chip_chn_phy_id.chipid,s_chip_chn_phy_id.chanid,IS_AUDIO_STREAM);
        if( ret < 0 ) {
            fprintf(stderr,"net_task_reg_enc failed .\n");
            isil_recapp_close( (void *) &s_recapp_comm);
            file_task->release(file_task);
            return -1;
        }
    }

    if(fm_start_rec_ptr->u8Stream & REC_ONLY_MJPG){
        ret = net_task_reg_enc(file_task,s_chip_chn_phy_id.chipid,s_chip_chn_phy_id.chanid,IS_MJPEG_STREAM);
        if( ret < 0 ) {
            fprintf(stderr,"net_task_reg_enc failed .\n");
            isil_recapp_close( (void *) &s_recapp_comm);
            file_task->release(file_task);
            return -1;
        }
    }

    return 0;
        
}


int stop_file_record( int chan)
{
    
    int ret ;

    CHIP_CHN_PHY_ID s_chip_chn_phy_id;
    RECAPP_COMM s_recapp_comm;
    
    
    struct FILE_REC_TASK_T *file_rec_sess;
    struct NET_TASK *file_task;


    s_recapp_comm.chan = chan;

    isil_recapp_close( (void *) &s_recapp_comm);
    #ifndef ISIL_USE_SYS_CFG_FILE
    ret = get_chip_chn_id_by_logic_id(chan,&s_chip_chn_phy_id);
    #else
    ret = ISIL_GetChipChnIDByLogicID(chan,&s_chip_chn_phy_id);
    #endif
    if( ret < 0 ) {
        //isil_recapp_close( (void *)&s_recapp_comm);
        return -1;
    }

    file_rec_sess = get_file_rec_task_by_inf(s_chip_chn_phy_id.chipid,s_chip_chn_phy_id.chanid);
    if( !file_rec_sess  ) {
        return -1;
    }

    if(!file_rec_sess->task) {
        return 0;
    }

    if(file_rec_sess->logic_chan_id != (unsigned int)chan) {
        fprintf(stderr,"file_rec_sess->logic_chan_id err .\n");
        return -1;
    }

    file_task = file_rec_sess->task;
    file_task->release(file_task);

    return 0;
}



static int ISIL_FM_SRV_InitDisk()
{
    int ret ,value;

    char buff[64];

    ret = ISIL_FM_FILE_ALLOCATE_ReadAllocParam();
    if(ret < 0)
    {

        fprintf(stderr, "disk has changed, need format\n");
        return 0;

        memset(buff,0x00,sizeof(buff));
        sprintf(buff,"rm -fr %s" ,FILE_REC_MOUNT_NODE);


        ret = system(buff);
        value = WEXITSTATUS(ret);
        if( value ) {
            fprintf(stderr,"Can't del dir [%s] .\n",buff);
            
        }
        
        
        #if 0
        //分区格式化
        if(ISIL_FM_DISK_Format() < 0)
        {
            fprintf(stderr, "ISIL_FM_DISK_Format fail\n");
            return -1;
        }
        #endif
        //建立自己的目录结构
        if(ISIL_FM_FILE_ALLOCATE_Format() < 0)
        {
            fprintf(stderr, "ISIL_FM_FILE_ALLOCATE_Format fail\n");
            return -1;
        }
    }

    return 0;
}


/*设置相关路径,包括:磁盘格式化时设备节点/磁盘挂接点,读写文件目录等*/
static int ISIL_FM_SRV_SetPaths(char *Node, char *Mnt)
{
    /*设置写文件目录*/
    if(ISIL_FM_FILE_ALLOCATE_SetNodePath(Mnt) < 0)
    {
        return -1;
    }

    /*设置格式化设备节点等信息*/
    if(ISIL_FM_DISK_SetNodePath(Node, Mnt) < 0)
    {
        return -1;
    }

    /*设置播放根目录*/
    if(ISIL_FM_PLAY_SetPlayDir(Mnt) < 0)
    {
        return -1;
    }

    /*设置Log dir*/
    if(ISIL_FM_FILE_Log_SetDir(Mnt) < 0)
    {
        return -1;
    }
    return 0;
}




int file_record_init( void )
{
    int ret;

    init_glb_file_rec_task_array();
    glb_file_rec_thr_mgt_init();

    ret = ISIL_FM_SRV_SetPaths(FILE_REC_DEVICE_NODE ,FILE_REC_MOUNT_NODE);
    if(ret < 0 ) {
        fprintf(stderr,"!!! ISIL_FM_SRV_SetPaths failed,can't record .\n");
        return 0;
    }

    ret = ISIL_FM_SRV_InitDisk();
    if(ret < 0 ) {
        fprintf(stderr,"!!! ISIL_FM_SRV_InitDisk failed,can't record .\n");
        return 0;
    }
    
    
    return 0; 


}



