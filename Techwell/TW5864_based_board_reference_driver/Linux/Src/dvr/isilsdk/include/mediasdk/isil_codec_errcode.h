#ifndef _ISIL_CODEC_ERRCODE_H_
#define _ISIL_CODEC_ERRCODE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CODEC_ERRCODE_MASK (-8000)

typedef enum
{
    CODEC_ERR_OK    = 0,//sucess
    CODEC_ERR_FAIL  = CODEC_ERRCODE_MASK,//fail
    CODEC_ERR_EXIST,//something had exist
    CODEC_ERR_NOT_EXIST,
    CODEC_ERR_BUSY,//busy
    CODEC_ERR_INVAL,//parament is invalid
}CODEC_ERR_CODE;


#ifdef __cplusplus
}
#endif

#endif



