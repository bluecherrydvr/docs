#ifndef  _ISIL_ENC_SDK_H
#define _ISIL_ENC_SDK_H



#ifdef __cplusplus
extern "C" {
#endif


#include "ISIL_SDKError_Def.h"

#include "isil_media_config.h"

typedef int MediaSDKCallBack( void *av_packet , void *pContext);

typedef int MediaSDKPreviewCallBack( void *av_packet , void *pContext);

typedef int MediaSDKH264CallBack( void *av_packet , void *pContext);

typedef int MediaAudioCallBack( void *av_packet , void *pContext);

typedef int MediaMJpegCallBack( void *av_packet , void *pContext);

typedef int MediaMVCallBack( void *av_packet , void *pContext);

typedef int MediaSDKAlarmCallBack( void *alarm_packet , void *pContext);




extern ISILERR_CODE ISIL_MediaSDK_Init( void );

extern ISILERR_CODE ISIL_MediaSDK_Run( void );

extern ISILERR_CODE ISIL_MediaSDK_Cleanup( void );

extern ISILERR_CODE ISIL_MediaSDK_Exit( void );

extern ISILERR_CODE ISIL_MediaSDK_GetChipCount( unsigned long *lpCount);

extern ISILERR_CODE ISIL_MediaSDK_SetBufferPool(unsigned int num , unsigned int size);


extern ISILERR_CODE ISIL_MediaSDK_StartH264MainChannel(unsigned int nChipId ,unsigned int nChnnel);

extern ISILERR_CODE ISIL_MediaSDK_StartH264SubChannel(unsigned int nChipId ,unsigned int nChnnel);

extern ISILERR_CODE ISIL_MediaSDK_StartAudioChannel(unsigned int nChipId ,unsigned int nChnnel);

extern ISILERR_CODE ISIL_MediaSDK_StopH264MainChannel(unsigned int nChipId ,unsigned int nChnnel);

extern ISILERR_CODE ISIL_MediaSDK_StopH264SubChannel(unsigned int nChipId ,unsigned int nChnnel);

extern ISILERR_CODE ISIL_MediaSDK_StopAudioChannel(unsigned int nChipId ,unsigned int nChnnel);

extern ISILERR_CODE ISIL_MediaSDK_StartCapMjpeg(unsigned int nChipId ,unsigned int nChnnel);

extern ISILERR_CODE ISIL_MediaSDK_StopCapMjpeg(unsigned int nChipId ,unsigned int nChnnel);


extern ISILERR_CODE ISIL_MediaSDK_OpenChannel( unsigned int nChipId ,unsigned int nChnnel);

extern ISILERR_CODE ISIL_MediaSDK_CloseChannel( unsigned int nChipId ,unsigned int nChnnel);


extern ISILERR_CODE ISIL_MediaSDK_RegEncCallback(unsigned int nChipId,
                                            unsigned int nChnnel,
                                            enum ECHANFUNCPRO pro,
                                            MediaSDKCallBack *funcCallBack,
                                            void *pContext);

extern ISILERR_CODE ISIL_MediaSDK_RegPreviewCallback(unsigned int nChipId,
                                                                                          unsigned int nChnnel,
                                                                                          MediaSDKPreviewCallBack,
                                                                                          void *pContext);

extern ISILERR_CODE ISIL_MediaSDK_RegH264MainCallback(unsigned int nChipId,
                                                                                        unsigned int nChnnel,
                                                                                        MediaSDKH264CallBack *funcCallBack,
                                                                                        void *pContext);

extern ISILERR_CODE ISIL_MediaSDK_RegH264SubCallback(unsigned int nChipId,
                                                                                        unsigned int nChnnel,
                                                                                        MediaSDKH264CallBack *funcCallBack,
                                                                                        void *pContext);

extern ISILERR_CODE ISIL_MediaSDK_RegAudioCallback(unsigned int nChipId,
                                                                                        unsigned int nChnnel,                                                                                     
                                                                                        MediaAudioCallBack *funcCallBack,
                                                                                        void *pContext);

extern ISILERR_CODE ISIL_MediaSDK_RegMJpegCallback(unsigned int nChipId,
                                                                                        unsigned int nChnnel,                                                                                        
                                                                                        MediaMJpegCallBack *funcCallBack,
                                                                                        void *pContext);


extern ISILERR_CODE ISIL_MediaSDK_RegAlarmCallback(unsigned int nChipId,
                                            MediaSDKAlarmCallBack *funcCallBack,
                                            void *pContext);

extern ISILERR_CODE ISIL_MediaSDK_StopAlarm(unsigned int nChipId );

extern ISILERR_CODE ISIL_MediaSDK_StartAlarm(unsigned int nChipId );




#ifdef __cplusplus
}
#endif

#endif
