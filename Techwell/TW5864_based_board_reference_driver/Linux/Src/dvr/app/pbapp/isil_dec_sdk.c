/*
 * isil_dec_sdk.c
 *
 *  Created on: 2011-6-28
 *      Author: junbinwang
 */

#include "isil_dec_sdk.h"
#include "isil_decoder.h"
#include "isil_debug.h"
#include "isil_decode_api.h"





ISILERR_CODE  ISIL_MediaSDK_CreateDecChan(unsigned int nChipId,
                                          unsigned int nChnnel, 
                                          enum ECHANFUNCPRO type)
{

    S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;
	int enable = 1;
    
    sin_chan_mgt = isil_open_dec_chn( (int)nChipId,(int)nChnnel ,type , enable);
    if( !sin_chan_mgt ) {

        return ISIL_ERR_FAILURE;
    }
    
	return ISIL_ERR_SUCCESS;
}



ISILERR_CODE  ISIL_MediaSDK_ReleaseDecChan(unsigned int nChipId,
		unsigned int nChnnel,  enum ECHANFUNCPRO eStreamType)
{
	S_ISIL_SINGLE_CHAN_MGT *sin_chan_mgt;
	sin_chan_mgt = get_sin_chn_mgt_by_chip_chn_type( (int)nChipId ,
												(int)nChnnel ,
											    CHAN_IS_DEC ,
												eStreamType);
	if( !sin_chan_mgt ) {
		fprintf(stderr,"get_sin_chn_mgt_by_chip_chn_type failed .\n");
		return ISIL_ERR_FAILURE;
	}

	sin_chan_mgt->release(sin_chan_mgt);
	
	


	return ISIL_ERR_SUCCESS;
}



ISILERR_CODE  ISIL_MediaSDK_StartDecode(unsigned int nChipId,
                                        unsigned int nChnnel,
                                        enum ECHANFUNCPRO type)
{

	return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_StopDecode(unsigned int nChipId,
		unsigned int nChnnel, enum ECHANFUNCPRO type)
{
	return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetDecConfig(unsigned int nChipId,
		unsigned int nChnnel, unsigned int nType,  void* pCfg)
{
	return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_GetDecConfig(unsigned int nChipId,
		unsigned int nChnnel,  unsigned int nType, void* pCfg)
{
	return ISIL_ERR_SUCCESS;
}



ISILERR_CODE  ISIL_MediaSDK_GetDecChfd(unsigned int nChipId,
		unsigned int nChnnel,  unsigned int nType, int* pfd)
{
	return ISIL_ERR_SUCCESS;
}


ISILERR_CODE  ISIL_MediaSDK_FlushDecCh(unsigned int nChipId,
		unsigned int nChnnel, unsigned int nType)
{
	return ISIL_ERR_SUCCESS;
}


ISILERR_CODE  ISIL_MediaSDK_StartAVSync(unsigned int nChipId, DEC_AV_SYNC* pAVSync)
{
	return ISIL_ERR_SUCCESS;
}


ISILERR_CODE  ISIL_MediaSDK_ResetAVSync(unsigned int nChipId, DEC_AV_SYNC* pAVSync)
{
	return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_StopAVSync(unsigned int nChipId, DEC_AV_SYNC* pAVSync)
{
	return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_BindVO(unsigned int nChipId, BIND_DEC2VO *pD2VO)
{
	return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_UnbindVO(unsigned int nChipId, BIND_DEC2VO *pD2VO)
{
	return ISIL_ERR_SUCCESS;
}

ISILERR_CODE  ISIL_MediaSDK_SetPbMode(unsigned int nChipId,
		unsigned int nChnnel, unsigned int nType,  VDEC_CH_MODE *pMode)
{
	return ISIL_ERR_SUCCESS;
}


ISILERR_CODE  ISIL_MediaSDK_SendDecdata(unsigned int nChipId,
		unsigned int nChnnel, unsigned int nType, ISIL_AV_PACKET* pData)
{
	return ISIL_ERR_SUCCESS;
}


