#include <stdio.h>
#include <stdlib.h>

#include "isil_enc_data_mgt_cent.h"
#include "isil_enc_sdk.h"
#include "isil_net_config.h"
#include "isil_debug.h"
#include "config.h"

static struct ENC_DATA_REG_T enc_data_reg_array[TOTAL_CHIPS_NUM][PER_CHIP_MAX_CHAN_NUM][FUNCPROSIZE];


static int enc_reg_data_rec_cb( void *av_packet , void *pContext)
{
    struct ENC_DATA_REG_T *enc_data_reg;

    struct NET_NOTIFY_REG_T *nty_reg_p;

    if( !av_packet || !pContext) {
        return -1;
    }
    
    enc_data_reg = (struct ENC_DATA_REG_T *)pContext;
    nty_reg_p = &enc_data_reg->nty_reg;
    nty_reg_p->notify_all(av_packet , nty_reg_p);

    return 0;
    
}

static int set_enc_reg_func(struct ENC_DATA_REG_T *enc_data_reg)
{
    unsigned int chip_num;
    unsigned int chan_num ;
    enum ECHANFUNCPRO chanpro ;
    int ret;

    chip_num = enc_data_reg->chip_id;
    chan_num = enc_data_reg->chan_id;
    chanpro  = enc_data_reg->chanpro;

    if(chip_num >= TOTAL_CHIPS_NUM) {
        DEBUG_FUNCTION();
        return -1;
    }

    if(chan_num >= PER_CHIP_MAX_CHAN_NUM) {
        DEBUG_FUNCTION();
        return -1;
    }

    if(chanpro >= FUNCPROSIZE) {
        DEBUG_FUNCTION();
        return -1;
    }

    if( enc_data_reg->reg_stat == ENC_DATA_HAVE_REG ) {
        DEBUG_FUNCTION();
        return 0;
    }

    
    switch (chanpro) {
    case IS_H264_MAIN_STREAM:
        DEBUG_FUNCTION();
        ret = ISIL_MediaSDK_RegH264MainCallback(chip_num,
                                                chan_num,
                                                enc_reg_data_rec_cb ,
                                                (void *)enc_data_reg);
        if( ret < 0) {
            return -1;
        }

        ret = ISIL_MediaSDK_StartH264MainChannel(chip_num ,chan_num);
        if( ret < 0) {
            return -1;
        }
        DEBUG_FUNCTION();

        break;
    case IS_H264_SUB_STREAM:
        DEBUG_FUNCTION();

        ret = ISIL_MediaSDK_RegH264SubCallback(chip_num,
                                                chan_num,
                                                enc_reg_data_rec_cb ,
                                                (void *)enc_data_reg);
        if( ret < 0) {
            return -1;
        }

        ret = ISIL_MediaSDK_StartH264SubChannel(chip_num ,chan_num);
        if( ret < 0) {
            return -1;
        }
        DEBUG_FUNCTION();
        break;
    case IS_MJPEG_STREAM:
        ret = ISIL_MediaSDK_RegMJpegCallback(chip_num,
                                                chan_num,
                                                enc_reg_data_rec_cb ,
                                                (void *)enc_data_reg);
        if( ret < 0) {
            return -1;
        }

        ret = ISIL_MediaSDK_StartCapMjpeg(chip_num ,chan_num);
        if( ret < 0) {
            return -1;
        }

        break;
    case IS_AUDIO_STREAM:

        ret = ISIL_MediaSDK_RegAudioCallback(chip_num,
                                                chan_num,
                                                enc_reg_data_rec_cb ,
                                                (void *)enc_data_reg);
        if( ret < 0) {
            return -1;
        }

        ret = ISIL_MediaSDK_StartAudioChannel(chip_num ,chan_num);
        if( ret < 0) {
            return -1;
        }

        break;
    default:
        return -1;
    }

    enc_data_reg->reg_stat = ENC_DATA_HAVE_REG;

    return 0;
}


static int set_enc_unreg_func(struct ENC_DATA_REG_T *enc_data_reg)
{
    unsigned int chip_num;
    unsigned int chan_num ;
    enum ECHANFUNCPRO chanpro ;
    int ret;

    chip_num = enc_data_reg->chip_id;
    chan_num = enc_data_reg->chan_id;
    chanpro  = enc_data_reg->chanpro;


    DEBUG_FUNCTION();

    if(chip_num >= TOTAL_CHIPS_NUM) {
        DEBUG_FUNCTION();
        return -1;
    }

    if(chan_num >= PER_CHIP_MAX_CHAN_NUM) {
        DEBUG_FUNCTION();
        return -1;
    }

    if(chanpro >= FUNCPROSIZE) {
        DEBUG_FUNCTION();
        return -1;
    }

    if( enc_data_reg->reg_stat == ENC_DATA_NONE_REG ) {

        DEBUG_FUNCTION();
        return 0;
    }


    switch (chanpro) {
    case IS_H264_MAIN_STREAM:
        
        DEBUG_FUNCTION();
        ret = ISIL_MediaSDK_StopH264MainChannel(chip_num ,chan_num);
        if( ret < 0) {
            DEBUG_FUNCTION();
            return -1;
        }
        
        DEBUG_FUNCTION();
        break;
    case IS_H264_SUB_STREAM:

        DEBUG_FUNCTION();

        ret = ISIL_MediaSDK_StopH264SubChannel(chip_num ,chan_num);
        if( ret < 0) {
            DEBUG_FUNCTION();
            return -1;
            DEBUG_FUNCTION();
        }
        DEBUG_FUNCTION();
        break;
    case IS_MJPEG_STREAM:
        

        ret = ISIL_MediaSDK_StopCapMjpeg(chip_num ,chan_num);
        if( ret < 0) {
            return -1;
        }

        break;
    case IS_AUDIO_STREAM:

        
        ret = ISIL_MediaSDK_StopAudioChannel(chip_num ,chan_num);
        if( ret < 0) {
            return -1;
        }

        break;
    default:
        return -1;
    }

    enc_data_reg->reg_stat = ENC_DATA_NONE_REG;

    return 0;
    
}


static int package_set_enc_reg_func( void *param)
{
    struct ENC_DATA_REG_T *enc_data_reg = (struct ENC_DATA_REG_T *)param;
    return set_enc_reg_func(enc_data_reg);
}

static int package_set_enc_unreg_func( void *param)
{
    struct ENC_DATA_REG_T *enc_data_reg = (struct ENC_DATA_REG_T *)param;
    return set_enc_unreg_func(enc_data_reg);
}


static void init_s_enc_data_reg_t( struct ENC_DATA_REG_T *enc_data_reg)
{
    if(enc_data_reg) {

        net_notify_reg_sinit(&enc_data_reg->nty_reg);
        enc_data_reg->nty_reg.root = (void *)enc_data_reg;
        enc_data_reg->reg = set_enc_reg_func;
        enc_data_reg->unreg = set_enc_unreg_func;

        enc_data_reg->nty_reg.root_reg = package_set_enc_reg_func;
        enc_data_reg->nty_reg.root_unreg = package_set_enc_unreg_func;

    }
}


void init_enc_data_reg_array( void )
{

    int i, j , k;

    struct ENC_DATA_REG_T *data_reg_p;

    for(i = 0 ; i < TOTAL_CHIPS_NUM ; i++) {

        for(j = 0 ; j < PER_CHIP_MAX_CHAN_NUM ; j++) {

            for(k = 0 ; k < FUNCPROSIZE ; k++) {

                data_reg_p = &enc_data_reg_array[i][j][k];

                data_reg_p->chip_id = i;
                data_reg_p->chan_id = j;
                data_reg_p->chanpro = k;

                init_s_enc_data_reg_t(data_reg_p);

            }

        }

    }

}


struct ENC_DATA_REG_T *get_enc_data_reg_by_inf(unsigned int chip_num,
                                               unsigned int chan_num ,
                                               enum ECHANFUNCPRO chanpro )
{

    if(chip_num >= TOTAL_CHIPS_NUM) {
        return NULL;
    }

    if(chan_num >= PER_CHIP_MAX_CHAN_NUM) {
        return NULL;
    }

    if(chanpro >= FUNCPROSIZE) {
        return NULL;
    }

    return &enc_data_reg_array[chip_num][chan_num][chanpro];

}



















