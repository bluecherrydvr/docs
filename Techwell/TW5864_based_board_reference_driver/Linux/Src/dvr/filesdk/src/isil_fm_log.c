#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "isil_fm_log.h"
//#include "isil_fm_h264.h"
#include "isil_fm_log_string.h"


/*日志文件索引结构*/
/*日志静态分配时的索引结构，仅内部使用*/
typedef struct
{
    int  s32LogTole;//总共可以记录多少条log
    int  s32Used;   //目前用了多少条log，如果写满等于s32LogTole
    int  s32Over;   //覆盖了多少条log
    int  s32Reserved;
}ISIL_FM_LOG_INDEX;

char LogRootDir[LOG_PATH_MAX_LEN + 1];

static pthread_mutex_t ISIL_FM_LOG_LOCK = PTHREAD_MUTEX_INITIALIZER;//LOG LOCK

/*日志文件大小，kB为单位*/
int ISIL_FM_LOG_GetFileSize(void)
{
    int s32LogSize;

    s32LogSize = sizeof(ISIL_FM_LOG_INDEX) + ISIL_FM_LOG_MAXSUM * sizeof(ISIL_FM_LOG_CONTEXT);
    s32LogSize = s32LogSize/1024 + 1;//KB

    fprintf(stderr, "log file size is : %d(KB)\n", s32LogSize);
    return s32LogSize;
}

/*创建文件*/
static int ISIL_FM_LOG_Creat(char *pPathName, int s32Len)
{
    int s32Ret = 0;
    int s32LogFd = 0;
    char buff[1024]= {};//1k
    int i = 0;
    int s32Offset = 0;

    s32LogFd = open(pPathName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(s32LogFd == -1)
    {
        fprintf(stderr, "creat:%s fail\n", pPathName);
        return -1;
    }

    if(s32Len > 0)
    {
        for(i = 0; i < s32Len; i++)
        {
            if(lseek(s32LogFd, s32Offset, SEEK_SET) == -1)
            {
                fprintf(stderr, "%s, %d, fail\n", __FUNCTION__, __LINE__);
                close(s32LogFd);
                return -1;
            }

            s32Ret = write(s32LogFd, buff, 1024);
            if(s32Ret != 1024)
            {
                fprintf(stderr, "%s, %d, fail\n", __FUNCTION__, __LINE__);
                close(s32LogFd);
                return -1;
            }

            s32Offset += s32Ret;
        }

        if(s32Offset != (s32Len*1024))
        {
            fprintf(stderr, "%s, %d, write fail\n", __FUNCTION__, __LINE__);
            close(s32LogFd);
            return -1;
        }
    }

    return s32LogFd;
}

/*打开文件*/
static int ISIL_FM_LOG_Open()
{
   int s32Fd = 0;
   char path[64] = {};

   sprintf(path, "%s%s", LogRootDir, ISIL_LOG_NAME);

   s32Fd = open(path, O_RDWR, S_IRUSR | S_IWUSR);
   if(s32Fd == -1)
   {
        fprintf(stderr, "open:%s fail\n", path);
        return -1;
   }
   return s32Fd;
}

static int ISIL_FM_LOG_Close(int s32LogFd)
{
    fsync(s32LogFd);
    close(s32LogFd);
    return 0;
}

static int ISIL_FM_LOG_WriteIndex(int s32LogFd, ISIL_FM_LOG_INDEX stLogIndex)
{
    int s32Ret = 0;

    s32Ret = lseek(s32LogFd, 0, SEEK_SET);
    if(s32Ret < 0)
    {
        fprintf(stderr, "log write index; lseek fail\n");
        return -1;
    }

    s32Ret = write(s32LogFd, &stLogIndex, sizeof(ISIL_FM_LOG_INDEX));
    if(s32Ret != sizeof(ISIL_FM_LOG_INDEX))
    {
        fprintf(stderr, "log write index: write len fail\n");
        return -1;
    }

    return 0;
}

static int ISIL_FM_LOG_ReadIndex(int s32LogFd, ISIL_FM_LOG_INDEX *stLogIndex)
{
    int s32Ret = 0;

    s32Ret = lseek(s32LogFd, 0, SEEK_SET);
    if(s32Ret < 0)
    {
        fprintf(stderr, "log read index; lseek fail\n");
        return -1;
    }

    s32Ret = read(s32LogFd, stLogIndex, sizeof(ISIL_FM_LOG_INDEX));
    if(s32Ret != sizeof(ISIL_FM_LOG_INDEX))
    {
        fprintf(stderr, "log read index: read len fail\n");
        return -1;
    }

    return 0;
}

static int ISIL_FM_LOG_WriteContent(int s32LogFd, int s32LogId, ISIL_FM_LOG_CONTEXT stLogContext)
{
    int s32Ret = 0;
    int s32Offset = 0;

    s32Offset = sizeof(ISIL_FM_LOG_INDEX) + s32LogId * sizeof(ISIL_FM_LOG_CONTEXT);
    s32Ret = lseek(s32LogFd, s32Offset, SEEK_SET);
    if(s32Ret < 0)
    {
        fprintf(stderr, "log write cotent: lseek fail\n");
        return -1;
    }

    s32Ret = write(s32LogFd, &stLogContext, sizeof(ISIL_FM_LOG_CONTEXT));
    if(s32Ret != sizeof(ISIL_FM_LOG_CONTEXT))
    {
        fprintf(stderr, "log write cotent: write len fail\n");
        return -1;
    }

    return 0;
}

static int ISIL_FM_LOG_ReadContent(int s32LogFd, int s32LogId, ISIL_FM_LOG_CONTEXT *stLogContext)
{
    int s32Ret = 0;
    int s32Offset = 0;

    s32Offset = sizeof(ISIL_FM_LOG_INDEX) + s32LogId * sizeof(ISIL_FM_LOG_CONTEXT);
    s32Ret = lseek(s32LogFd, s32Offset, SEEK_SET);
    if(s32Ret < 0)
    {
        fprintf(stderr, "log read cotent: lseek fail\n");
        return -1;
    }

    s32Ret = read(s32LogFd, stLogContext, sizeof(ISIL_FM_LOG_CONTEXT));
    if(s32Ret != sizeof(ISIL_FM_LOG_CONTEXT))
    {
        fprintf(stderr, "log read cotent: read len fail\n");
        return -1;
    }

    return 0;
}

/*初始化Log file*/
int ISIL_FM_LOG_Init()
{
    int s32LogFd = 0;
    int s32Ret = 0;
    char path[64] = {};

    sprintf(path, "%s%s", LogRootDir, ISIL_LOG_NAME);

    pthread_mutex_lock(&ISIL_FM_LOG_LOCK);
    s32LogFd = ISIL_FM_LOG_Creat(path, ISIL_FM_LOG_GetFileSize());
    if(s32LogFd < 0)
    {
        fprintf(stderr, "creat log fd fail\n");
        return -1;
    }

    /*log index init*/
    ISIL_FM_LOG_INDEX stLogIndex;
    memset(&stLogIndex, 0x00, sizeof(ISIL_FM_LOG_INDEX));
    stLogIndex.s32LogTole = ISIL_FM_LOG_MAXSUM;

    s32Ret = ISIL_FM_LOG_WriteIndex(s32LogFd, stLogIndex);
    if(s32Ret < 0)
    {
        fprintf(stderr, "ISIL_FM_LOG_WriteIndex fail\n");
    }

    /*close*/
    ISIL_FM_LOG_Close(s32LogFd);

    pthread_mutex_unlock(&ISIL_FM_LOG_LOCK);
    return s32Ret;
}


/*写log日志*/
int ISIL_FM_LOG_Write(unsigned char u8Type, unsigned char u8Channel, char strUsrName[8], unsigned short strId)
{
    int s32Ret = 0;
    int s32LogFd = 0;
    int s32LogId = 0;
    ISIL_FM_LOG_CONTEXT LogContext;

    //fprintf(stderr, "Enter FUNC: %s\n", __FUNCTION__);

    pthread_mutex_lock(&ISIL_FM_LOG_LOCK);

    s32LogFd = ISIL_FM_LOG_Open();
    if(s32LogFd < 0)
    {
        fprintf(stderr, "ISIL_FM_LOG_Open fail\n");
        s32Ret = -1;
        goto exit;
    }

    ISIL_FM_LOG_INDEX stLogHead;

    s32Ret = ISIL_FM_LOG_ReadIndex(s32LogFd, &stLogHead);
    if(s32Ret < 0)
    {
        fprintf(stderr, "ISIL_FM_LOG_ReadIndex fail\n");
        goto exit1;
    }

    if(stLogHead.s32LogTole <= 0)
    {
        fprintf(stderr, "log totle < 0 fail\n");
        goto exit1;
    }
    else
    {
        if(stLogHead.s32Used == stLogHead.s32LogTole)//写完，需要覆盖
        {
            #if ISIL_FM_LOG_OVERWRITE//over write
                if(stLogHead.s32Over >= stLogHead.s32LogTole)
                {
                    stLogHead.s32Over = 0;
                }
                s32LogId = stLogHead.s32Over;
            #else
                fprintf(stderr, "log is full\n");
                goto exit1;
            #endif
        }
        else
        {
            s32LogId = stLogHead.s32Used;
        }
    }
    //fprintf(stderr, "Enter FUNC: %s 111\n", __FUNCTION__);
    /*填充数据*/
    memset(&LogContext, 0x00, sizeof(ISIL_FM_LOG_CONTEXT));
    LogContext.u8Type = u8Type;
    LogContext.u8Channel = u8Channel;
    LogContext.strId = strId;
    strcpy(LogContext.strUsrName, strUsrName);
    ISIL_GetSysTime(&LogContext.stDT);

    //fprintf(stderr, "Enter FUNC: %s 222\n", __FUNCTION__);

    s32Ret = ISIL_FM_LOG_WriteContent(s32LogFd, s32LogId, LogContext);
    if(s32Ret < 0)
    {
        fprintf(stderr, "ISIL_FM_LOG_WriteContent fail\n");
        goto exit1;
    }

    /*update index*/
    if(stLogHead.s32Used == stLogHead.s32LogTole)
    {
        #if ISIL_FM_LOG_OVERWRITE//over write
            stLogHead.s32Over++;
        #else
            fprintf(stderr, "log is full\n");
            goto exit1;
        #endif
    }
    else
    {
        stLogHead.s32Used++;
    }

    s32Ret = ISIL_FM_LOG_WriteIndex(s32LogFd, stLogHead);
    if(s32Ret < 0)
    {
        fprintf(stderr, "ISIL_FM_LOG_WriteIndex fail\n");
        goto exit1;
    }


exit1:
    ISIL_FM_LOG_Close(s32LogFd);
exit:
    pthread_mutex_unlock(&ISIL_FM_LOG_LOCK);

    //fprintf(stderr, "Enter FUNC: %s 333\n", __FUNCTION__);
    //fprintf(stderr, "\n###log: %s %s %d###\n", LogContext.strUsrName, STRING[LogContext.strId], LogContext.u8Channel);
    //fprintf(stderr, "Enter FUNC: %s 444\n", __FUNCTION__);
    return s32Ret;
}

/*log search*/
int ISIL_FM_LOG_Search(MSG_FM_LOG_SEARCH stLogSearch, MSG_FM_LOG_SEARCH_RESULT *LogBuff)
{
    int s32Ret = 0;
    int s32LogFd = 0;
    int s32StartID = 0, s32EndID = 0;
    int loop = 0;
    ISIL_FM_LOG_CONTEXT LogContext;
    MSG_FM_LOG_SEARCH_RESULT *SchResult = (MSG_FM_LOG_SEARCH_RESULT *)LogBuff;

    memset(SchResult, 0x00, sizeof(MSG_FM_LOG_SEARCH_RESULT));

    pthread_mutex_lock(&ISIL_FM_LOG_LOCK);

    s32LogFd = ISIL_FM_LOG_Open();
    if(s32LogFd < 0)
    {
        fprintf(stderr, "ISIL_FM_LOG_Open fail\n");
        s32Ret = -1;
        goto LOG_EXIT;
    }

    ISIL_FM_LOG_INDEX stLogHead;

    s32Ret = ISIL_FM_LOG_ReadIndex(s32LogFd, &stLogHead);
    if(s32Ret < 0)
    {
        fprintf(stderr, "ISIL_FM_LOG_ReadIndex fail\n");
        goto LOG_EXIT1;
    }

    if(stLogHead.s32LogTole <= 0)
    {
        fprintf(stderr, "log totle < 0 fail\n");
        goto LOG_EXIT1;
    }
    else//读log
    {
        //确定起点与终点
        if(stLogHead.s32Over == 0)//没有覆盖
        {
            s32StartID = 0;
            s32EndID = stLogHead.s32Used;
        }
        else//有覆盖
        {
            s32StartID = stLogHead.s32Over;
            s32EndID = s32StartID - 1;
            if(s32EndID <= s32StartID)
            {
                s32EndID = s32EndID + stLogHead.s32LogTole;
            }
        }
    }

    
    fprintf(stderr, "startid: %d, endId: %d\n", s32StartID, s32EndID);

    for(loop = s32StartID; loop < s32EndID; )
    {
        s32Ret = ISIL_FM_LOG_ReadContent(s32LogFd, loop, &LogContext);
        if(s32Ret < 0)
        {
            fprintf(stderr, "ISIL_FM_LOG_ReadContent fail\n");
            goto LOG_EXIT1;
        }

        loop++;

        if(loop >= stLogHead.s32LogTole)
        {
            loop = 0;
            s32StartID = 0;
            s32EndID = s32EndID - stLogHead.s32LogTole + 1;
        }

        //比较日期
        if((memcmp(&LogContext.stDT.Date, &stLogSearch.StartDT.Date, sizeof(S_REC_DATE)) >= 0)
            && (memcmp(&stLogSearch.StopDT.Date, &LogContext.stDT.Date, sizeof(S_REC_DATE)) >= 0)
           )
        {
            //fprintf(stderr, "find the log 22222\n");
            //比较时间,not care second
            if(((LogContext.stDT.Time.Hour * 60 + LogContext.stDT.Time.Minute) >= (stLogSearch.StartDT.Time.Hour * 60 + stLogSearch.StartDT.Time.Minute))
                && ((LogContext.stDT.Time.Hour * 60 + LogContext.stDT.Time.Minute) <= (stLogSearch.StopDT.Time.Hour * 60 + stLogSearch.StopDT.Time.Minute))
            )
            {
                if((stLogSearch.u8Type) && (stLogSearch.u8Type != LogContext.u8Type))//0:表示所有类型
                {
                    fprintf(stderr, "find the log 000\n");
                    continue;
                }

                //fprintf(stderr, "find the log 111\n");

                //find the log, save to buff
                ISIL_FM_LOG_CONTEXT *LogMsg = (ISIL_FM_LOG_CONTEXT *)(LogBuff + sizeof(MSG_FM_LOG_SEARCH_RESULT) + SchResult->s32MsgSum * sizeof(ISIL_FM_LOG_CONTEXT));
                memcpy(LogMsg, &LogContext, sizeof(ISIL_FM_LOG_CONTEXT));

                SchResult->s32MsgSum++;
            }
        }
    }

    fprintf(stderr, "find the log sum: %d\n", SchResult->s32MsgSum);
    if(!SchResult->s32MsgSum)
    {
        fprintf(stderr, "not find the log\n");
    }
LOG_EXIT1:
    ISIL_FM_LOG_Close(s32LogFd);
LOG_EXIT:
    pthread_mutex_unlock(&ISIL_FM_LOG_LOCK);
    return s32Ret;
}

int ISIL_FM_LOG_Clear()
{
    if(ISIL_FM_LOG_Init() < 0)
    {
        fprintf(stderr, "----ISIL_FM_LOG_Init fail---\n");
        return -1;
    }

    if(ISIL_FM_LOG_Write(LOG_TYPE_LOG_CLEAR, 0x00, (char *)"admin", STRID_LOG_CLEAR) < 0)
    {
        fprintf(stderr, "----ISIL_FM_LOG_Write fail---\n");
        return -1;
    }

    return 0;
}



int ISIL_FM_FILE_Log_SetDir(char *Dir)
{
    if(Dir == NULL)
    {
        fprintf(stderr, "---LogRootDir is null---\n");
        return -1;
    }

    memset(LogRootDir, 0x00, LOG_PATH_MAX_LEN);
    memcpy(LogRootDir, Dir, LOG_PATH_MAX_LEN);
    sprintf(LogRootDir, "%s/", LogRootDir);

    fprintf(stderr, "---LogRootDir: %s---\n", LogRootDir);
    return 0;
}





