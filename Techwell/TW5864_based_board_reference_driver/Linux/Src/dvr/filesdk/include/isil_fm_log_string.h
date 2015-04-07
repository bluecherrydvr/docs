#ifndef _ISIL_FM_LOG_STRING_H_

#define _ISIL_FM_LOG_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

/*定义字串ID*/
typedef enum
{
    //系统操作相关
    STRID_SYS_BOOT,//开机
    STRID_SYS_SHUT_DOWN,//关机

    //配置操作
    STRID_CFG_AD,//配置AD
    STRID_CFG_DA,
    STRID_CFG_DEVICE,
    STRID_CFG_ENCODE,
    STRID_CFG_DECODE,
    STRID_CFG_PTZ,
    STRID_CFG_NETWORKING,

    //报警事件
    STRID_ALARM_VIDEO_LOST,
    STRID_ALARM_VIDEO_SHELTER,
    STRID_ALARM_MD,

    //录像
    STRID_REC_MANUAL,
    STRID_REC_TIMING,
    STRID_REC_ALARM,
    STRID_REC_STOP,

    //用户管理
    STRID_USER_LOGIN,
    STRID_USER_LOGOUT,
    STRID_USER_DEL,

    //文件操作
    STRID_FILE_PLAY,
    //日志清除
    STRID_LOG_CLEAR
}STRID;

extern char *STRING[];

#ifdef __cplusplus
}
#endif

#endif



