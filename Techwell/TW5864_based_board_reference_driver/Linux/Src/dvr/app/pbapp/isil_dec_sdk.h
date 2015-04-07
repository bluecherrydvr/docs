/*
 * isil_dec_sdk.h
 *
 *  Created on: 2011-6-28
 *      Author: junbinwang
 */

#ifndef ISIL_DEC_SDK_H_
#define ISIL_DEC_SDK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ISIL_SDKError_Def.h"
#include "isil_codec_chip_ctl.h"
#include "isil_decoder.h"
#include "isil_data_stream.h"
#include "isil_messagestruct.h"
#include "isil_media_config.h"

extern ISILERR_CODE  ISIL_MediaSDK_CreateDecChan(unsigned int nChipId,
		unsigned int nChnnel, unsigned int nType);

extern ISILERR_CODE  ISIL_MediaSDK_ReleaseDecChan(unsigned int nChipId,
		unsigned int nChnnel,  enum ECHANFUNCPRO eStreamType);



extern ISILERR_CODE  ISIL_MediaSDK_StartDecode(unsigned int nChipId,
		unsigned int nChnnel,unsigned int nType);

extern ISILERR_CODE  ISIL_MediaSDK_StopDecode(unsigned int nChipId,
		unsigned int nChnnel, unsigned int nType);

extern ISILERR_CODE  ISIL_MediaSDK_SetDecConfig(unsigned int nChipId,
		unsigned int nChnnel, unsigned int nType,  void* pCfg);

extern ISILERR_CODE  ISIL_MediaSDK_GetDecConfig(unsigned int nChipId,
		unsigned int nChnnel,  unsigned int nType, void* pCfg);



extern ISILERR_CODE  ISIL_MediaSDK_GetDecChfd(unsigned int nChipId,
		unsigned int nChnnel,  unsigned int nType, int* pfd);


extern ISILERR_CODE  ISIL_MediaSDK_FlushDecCh(unsigned int nChipId,
		unsigned int nChnnel, unsigned int nType);


extern ISILERR_CODE  ISIL_MediaSDK_StartAVSync(unsigned int nChipId, DEC_AV_SYNC* pAVSync);


extern ISILERR_CODE  ISIL_MediaSDK_ResetAVSync(unsigned int nChipId, DEC_AV_SYNC* pAVSync);

extern ISILERR_CODE  ISIL_MediaSDK_StopAVSync(unsigned int nChipId, DEC_AV_SYNC* pAVSync);

extern ISILERR_CODE  ISIL_MediaSDK_BindVO(unsigned int nChipId, BIND_DEC2VO *pD2VO);

extern ISILERR_CODE  ISIL_MediaSDK_UnbindVO(unsigned int nChipId, BIND_DEC2VO *pD2VO);

extern ISILERR_CODE  ISIL_MediaSDK_SetPbMode(unsigned int nChipId,
		unsigned int nChnnel, unsigned int nType, VDEC_CH_MODE *pMode);


extern ISILERR_CODE  ISIL_MediaSDK_SendDecdata(unsigned int nChipId,
		unsigned int nChnnel, unsigned int nType, ISIL_AV_PACKET* pData);


extern ISILERR_CODE ISIL_MediaSDK_Display_Window(unsigned int nChipId ,
                                          unsigned int nChanId ,
                                          ISIL_SUB_WINDOWS *cfg);


extern ISILERR_CODE ISIL_MediaSDK_Set_Sub_Window(unsigned int nChipId ,
                                          unsigned int nChanId ,
                                          ISIL_SUB_WINDOWS *cfg);

#ifdef __cplusplus
}
#endif


#endif /* ISIL_DEC_SDK_H_ */
