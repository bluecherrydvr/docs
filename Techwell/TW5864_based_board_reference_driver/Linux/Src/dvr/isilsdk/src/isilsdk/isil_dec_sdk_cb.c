/*
 * isil_dec_sdk_cb.c
 *
 *  Created on: 2011-6-30
 *      Author: junbinwang
 */

#if 0
#include "isil_dec_sdk_cb.h"

int isil_dec_creat_channel_cb(DEC_CHANNEL_TYPE* chType)
{
	int ret;
	CODEC_HANDLE DecHandle;
	S_ISIL_SINGLE_CHAN_MGT *sin_chn_mgt;

	if(NULL == chType)
	{
		printf("%s parm is null\n");
		return -1;
	}
	sin_chn_mgt = get_sin_chn_mgt_by_chip_chn_type(chType->chip_id,
													chType->chn ,
	                                                CHAN_IS_DEC,
	                                                chType->dectype);
	if( !sin_chn_mgt ) {
		fprintf(stderr,"isil_dec_creat_channel failed .\n");
		return -1;
	}
	ret = ISIL_CODEC_DEC_GetChHandle(sin_chn_mgt->codec_channel_t, &DecHandle);
	if(ret < 0)
	{
		fprintf(stderr,"isil_dec_creat_channel get handle failed .\n");
		return -1;
	}

	ret = ISIL_CODEC_DEC_CreateCh(sin_chn_mgt->codec_channel_t, &DecHandle);

	if(ret < 0)
	{
		fprintf(stderr,"isil_dec_creat_channel creat channel failed .\n");
		return -1;
	}

	return 0;
}

int isil_dec_release_channel_cb(DEC_CHANNEL_TYPE* chType)
{
	int ret;
	CODEC_HANDLE DecHandle;
	S_ISIL_SINGLE_CHAN_MGT *sin_chn_mgt;

	if(NULL == chType)
	{
		printf("%s parm is null\n");
		return -1;
	}
	sin_chn_mgt = get_sin_chn_mgt_by_chip_chn_type(chType->chip_id,
													chType->chn ,
	                                                CHAN_IS_DEC,
	                                                chType->dectype);
	if( !sin_chn_mgt ) {
		fprintf(stderr,"isil_dec_release_channel failed .\n");
		return -1;
	}
	ret = ISIL_CODEC_DEC_GetChHandle(sin_chn_mgt->codec_channel_t, &DecHandle);
	if(ret < 0)
	{
		fprintf(stderr,"isil_dec_release_channel get handle failed .\n");
		return -1;
	}

	ret = ISIL_CODEC_DEC_ReleaseCh(sin_chn_mgt->codec_channel_t, &DecHandle);

	if(ret < 0)
	{
		fprintf(stderr,"isil_dec_release_channel relese channel failed .\n");
		return -1;
	}

	return 0;
}
int isil_dec_creat_channel(DEC_CHANNEL_TYPE *cfg)
{
    return isil_msg_set_param((void*)cfg, isil_dec_creat_channel_cb, 0);
}

int isil_dec_release_channel(DEC_CHANNEL_TYPE *cfg)
{
    return isil_msg_set_param((void*)cfg, isil_dec_release_channel_cb, 0);
}

#endif

