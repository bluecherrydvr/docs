#ifndef _ISIL_FM_LOG_H_

#define _ISIL_FM_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_fm_utils.h"
//*********************************config*********************************/

#define LOG_PATH_MAX_LEN (32)

/*日志文件名*/
#define ISIL_LOG_NAME "isillog.bin"

/*定义支持的log条数*/
#define ISIL_FM_LOG_MAXSUM (1000)

/*日志文件是否可以覆盖，默认覆盖*/
#define ISIL_FM_LOG_OVERWRITE (1)



/*定义日志类型*/
typedef enum
{
    LOG_TYPE_INVALID,
    LOG_TYPE_SYSTEM_OPT,       //系统操作，开机，关机等
    LOG_TYPE_CFG_OPT,          //配置操作，如配置视频参数等
    LOG_TYPE_DATA_MANAGE,      //数据管理
    LOG_TYPE_ALARM_EVENT,      //报警事件
    LOG_TYPE_REC_OPT,          //录像操作，如开始停止，录像等
    LOG_TYPE_USER_MANAGE,     //用户管理,登录,登出
    LOG_TYPE_FILE_OPT,         //文件操作,播放等
    LOG_TYPE_LOG_CLEAR,        //日志清除
}ISIL_FM_LOG_MAIN_TYPE;

/*日志搜索条件*/
typedef struct
{
    S_REC_DATE_TIME StartDT;//开始日期时间
    S_REC_DATE_TIME StopDT;//结束日期时间
    unsigned char   u8Type;//主类型
    char            cReserved[3];
}MSG_FM_LOG_SEARCH;

/*日志文件记录内容结构*/
typedef struct
{
    S_REC_DATE_TIME stDT;//日期时间
    unsigned char   u8Type;//主类型
    unsigned char   u8Channel;//通道
    char 			strUsrName[8];//用户名
    unsigned short  strId;//事件字符串ID
}ISIL_FM_LOG_CONTEXT;

/*日志搜索结果主动返回*/
typedef struct
{
    int                 s32MsgSum;//返回的msg总数
    ISIL_FM_LOG_CONTEXT  stMsg[0];//msg具体内容
}MSG_FM_LOG_SEARCH_RESULT;

/*********************************API*********************************/
extern int ISIL_FM_LOG_GetFileSize();
extern int ISIL_FM_FILE_Log_SetDir(char *Dir);
extern int ISIL_FM_LOG_Init();
extern int ISIL_FM_LOG_Write(unsigned char u8Type, unsigned char u8Channel, char strUsrName[8], unsigned short strId);
extern int ISIL_FM_LOG_Search(MSG_FM_LOG_SEARCH stLogSearch, MSG_FM_LOG_SEARCH_RESULT *LogBuff);//Type 为0,则表示搜索全部类型
extern int ISIL_FM_LOG_Clear();



#ifdef __cplusplus
}
#endif

#endif

