#ifndef _ISIL_PRIVATE_FUNC_H_

#define _ISIL_PRIVATE_FUNC_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int ISIL_ENC_GetDataBymmap(CODEC_HANDLE* pHdl, char** pMmapAddr, CODEC_STREAM_DATA *stpStream);

extern int ISIL_ENC_GetDataByRead(CODEC_HANDLE* pHdl, CODEC_STREAM_DATA *stpStream);
#ifdef __cplusplus
}
#endif
#endif







