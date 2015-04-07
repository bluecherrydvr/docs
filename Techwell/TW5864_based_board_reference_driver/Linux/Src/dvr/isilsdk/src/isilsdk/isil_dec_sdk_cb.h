/*
 * isil_dec_sdk_cb.h
 *
 *  Created on: 2011-6-30
 *      Author: junbinwang
 */

#ifndef ISIL_DEC_SDK_CB_H_
#define ISIL_DEC_SDK_CB_H_

#if 0

typedef struct {
 unsigned short chip_id;
 unsigned short chn;
 enum ECHANFUNCPRO streamType;
 unsigned int dectype;
}DEC_CHANNEL_TYPE;


typedef struct {
 unsigned short chip_id;
 unsigned short chn;
 enum ECHANFUNCPRO streamType;
 VDEC_H264_CFG cfg;
}DEC_H264_CFG;

typedef struct {
 unsigned short chip_id;
 unsigned short chn;
 enum ECHANFUNCPRO streamType;
 VDEC_MJPG_CFG cfg;
}DEC_MJPG_CFG;

typedef struct {
 unsigned short chip_id;
 unsigned short chn;
 enum ECHANFUNCPRO streamType;
 VDEC_AUD_CFG cfg;
}DEC_AUDIO_CFG;

typedef struct {
 unsigned short chip_id;
 unsigned short chn;
 unsigned int decChFd;
}DEC_CHANNEL_FD;

typedef struct {
 unsigned short chip_id;
 enum ECHANFUNCPRO streamType;
 DEC_AV_SYNC av_sync;
}DEC_AV_SYNC_CFG;

#endif


#endif /* ISIL_DEC_SDK_CB_H_ */
