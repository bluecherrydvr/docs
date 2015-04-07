#ifndef _ISIL_FM_FILE_ALLOCATE_H_
#define _ISIL_FM_FILE_ALLOCATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include "isil_fm_utils.h"


/*支持的最大通道数目:32*/
#define ISIL_ALLOCATE_MAXCHANNEL  CHANNEL_NUMBER//(1)

/*录像管理相关根目录路径最大长度*/
#define REC_ROOT_DIR_MAX_LEN (32)

#define ISIL_DATA_DIR        "data/"
#define ISIL_INDEX_DIR       "index/"
#define ISIL_REC_INDEX_DIR   "264_index/"
#define ISIL_FILE_INDEX_DIR  "file_index/"
#define ISIL_FILE_INDEX_NAME "index00.bin"

//add for mjpg
#define ISIL_MJPG_INDEX_DIR  "mjpg_index/"

/*写文件的最大码流*/
#define ISIL_ALLOCATE_WRITE_BITRATE_MAX (256)//kB,(2*1024/8)
#define ISIL_ALLOCATE_WRITE_BITRATE_MIN (128)//kB,(1*1024/8)

/*静态文件的大小*/
//#define ISIL_ALLOCATE_FILE_SIZE_2048M (2048*1024)//2G
#define ISIL_ALLOCATE_FILE_SIZE_512M  (512*1024)//512MB//
#define ISIL_ALLOCATE_FILE_SIZE_256M  (256*1024)//256MB//
#define ISIL_ALLOCATE_FILE_SIZE_128M  (128*1024)//128MB
#define ISIL_ALLOCATE_FILE_SIZE_64M   (64*1024)//64MB
#define ISIL_ALLOCATE_FILE_SIZE_32M   (32*1024)//32MB
#define ISIL_ALLOCATE_FILE_SIZE_16M   (16*1024)//16MB
#define ISIL_ALLOCATE_FILE_SIZE_512K  (512)//512kB

/*录像最短记录时间*/
#define ISIL_ALLOCATE_REC_MINTIME   (2)//2s
#define ISIL_ALLOCATE_REC_MINLEN    (2*1024/ISIL_ALLOCATE_REC_MINTIME/8)//512k

/*录像文件保留大小，用于更新最后的gopnal,file tail*/
#define ISIL_ALLOCATE_REC_REMAIN_SIZE  (1024)//1k

/*一张mjpg平均最小大小*/
#define ISIL_MJPG_MINMUM_SIZE (50)//unit in B, 50KB per each mjpg frame minimum size


typedef struct
{
    unsigned int u32RecFileTotle;//录像文件总数
    unsigned int u32Reserved;//保留
    unsigned int u32RecIndexSumEachFile;//每个静态文件分配的二级索引项数
    unsigned int u32RecIndexSize;//每个二级索引项的大小
    //int s32RecIndexTotle;//静态分配的二级索引文件数目
    //int s32RecIndexUsed;//已经使用的二级索引文件数目
    unsigned int u32FileTotle;//静态分配的文件总数
    unsigned int u32UsedFileSum;//使用过的文件数目
    unsigned int u32FullFileSum;//已经写满的文件数目
    unsigned int u32CoveredFileSum;//覆盖的文件数目
}ISIL_ALLOCATE_FILE_BASEINFO;

typedef struct
{
    unsigned int                u32UsingFileID;
    unsigned short              u16RecSum;//该静态文件里的录像文件数目,counting from 1
    unsigned short              u16LogSum;//该静态文件里的录像日志数目，一个录像可能对应多个日志, counting from 1
    S_REC_DATE_TIME             stStartDT;
    unsigned char               u8HaveMjpg;//whether exit mjpg or not,exit :1,  not: 0
    char                        s8Reserved[15];
}ISIL_ALLOCATE_FILE_USING_INFO;


typedef struct
{
    unsigned int                u32FileID;
    unsigned int                u32Channel;
    unsigned short              u16RecSum;//该静态文件里的录像文件数目
    unsigned short              u16LogSum;//该静态文件里的录像日志数目，一个录像可能对应多个日志
    S_REC_DATE_TIME             stStartDT;
    unsigned char               u8HaveMjpg;//whether exit mjpg or not,exit :1,  not: 0
    char                        s8Reserved[11];
}ISIL_ALLOCATE_FILE_FULL_INFO;

typedef struct
{
    unsigned char               u8RecType;
    unsigned char               u8SubType;
    unsigned char               u8RecFileID;//属于第几个录像文件的日志
    unsigned char               u8Channel;
    S_REC_DATE_TIME             stStartDT;
    S_REC_DATE_TIME             stStopDT;
    unsigned int                u32StartOffset;
    unsigned int                u32StopOffset;
    unsigned int                u32RecIndexId;
}ISIL_ALLOCATE_FILE_REC_INFO;

typedef struct
{
    unsigned int IsFormat;//是否已经格式化
    unsigned int FILE_INDEX_RECLOG_SUM;//每个静态文件里录像记录预分配总数
    unsigned int FILE_NUM;//实际分配的静态文件数目
    unsigned int FILE_SIZE;//静态文件大小
    unsigned int REC_INDEX_SIZE;//每个索引文件的长度
    unsigned int ISIL_FM_DISK_OVER_WRITE;//硬盘覆盖,1:覆盖,0:不覆盖
    unsigned int u32RecIndexSumEachFile;//每个静态文件分配的二级索引项数
    unsigned int u32RecIndexSize;//每个二级索引项的大小
    unsigned int u32MaxRecChannel;//最大录像通道数目
}ISIL_SAVE_DISK_INFO;


/*******************以下为存储MJPG文件方式******************************/
/*1 mjpg数据仍然跟h264音视频数据一样混合放在一起，但是gopnal中不存储mjpg信息*/
/*2 mjpg单独增加一个预先静态分配好的文件来实现检索，目录如下:index/mjpg_index/xx.bin*/
/*3 在改文件中，前16个byte记录该文件大小，当前已用大小，mjpg最大索引数，已经用到的索引数*/
/*4 文件大小的计算方法:header_size + 数据文件大小/mjpg平均最小帧大小*  */

typedef struct{
    unsigned int u32FileTotleSize;//该索引文件的大小
    unsigned int u32CurSize;//已占用大小
    unsigned int u32IndexTotleNum;//mjpg索引总数
    unsigned int u32IndexCurNum;//mjpg已用数
    unsigned int u32Channel;//which channel,init value is 0xffffffff
    char         cReserved[12];
}ISIL_MJPG_INDEX_HEADER;


typedef struct{
    S_REC_DATE_TIME stDT;//date and time
	unsigned int    u32offset;//mjpg frame offset in data file
    unsigned int    u32Len;//mjpg frame size
	unsigned char   u8RecType;
    unsigned char   u8SubType;
	char            cReserved[2];
}ISIL_MJPG_INDEX;

extern char HDD_REC_DIR[];

/*API*/
/*设置硬盘是否覆盖*/
extern void ISIL_FM_FILE_ALLOCATE_SetDiskOverWrite(unsigned int u32IsOverWrite);
/*获取覆盖标志位*/
extern int ISIL_FM_FILE_ALLOCATE_GetDiskOverWrite(void);
/*从所分配的静态文件中得到一个可用的h264 data文件*/
extern int ISIL_FM_FILE_ALLOCATE_GetDataFile(int s32IndexFd, unsigned int u32Channel);
/*从所分配的静态索引文件中得到一个可用的录像二级索引文件*/
//extern int ISIL_FM_FILE_ALLOCATE_GetRecIndexFile(int s32IndexFd);
/*打开文件*/
extern int ISIL_FM_FILE_ALLOCATE_OpenFile(char *pPathName);
/*update file index*/
extern int ISIL_FM_FILE_ALLOCATE_UpdateFileIndex(int s32IndexFd, unsigned int u32FileID, ISIL_ALLOCATE_FILE_REC_INFO stIndexRecLog);
/*获取静态文件的大小*/
extern unsigned int ISIL_FM_FILE_ALLOCATE_GetFileSize();
/*仅仅一个文件写满时调用*/
extern int ISIL_FM_FILE_ALLOCATE_UpdateFull(int s32IndexFd, unsigned int u32Channel);
/*update indexRecLog*/
/*主要在文件结束时调用,每2s也需要调用一次*/
extern int ISIL_FM_FILE_ALLOCATE_UpdateIndexRecLog(int s32IndexFd, unsigned int u32Channel, unsigned int u32StopOffset);
/*add a rec log*/
extern int ISIL_FM_FILE_ALLOCATE_AddIndexRecLog(int s32IndexFd, unsigned int u32ChNum, unsigned char u8RecType, unsigned char u8SubType, unsigned int u32StartOffset);

/*读取索引文件各个区域*/
extern int ISIL_FM_FILE_ALLOCATE_ReadIndexBase(int s32Fd, ISIL_ALLOCATE_FILE_BASEINFO *stBaseInfo);
extern int ISIL_FM_FILE_ALLOCATE_ReadIndexUsing(int s32Fd, unsigned int u32Channel, ISIL_ALLOCATE_FILE_USING_INFO *stUsingInfo);
extern int ISIL_FM_FILE_ALLOCATE_ReadIndexFull(int s32Fd, unsigned int u32FileID, ISIL_ALLOCATE_FILE_FULL_INFO *stFullInfo);
extern int ISIL_FM_FILE_ALLOCATE_ReadIndexRec(int s32Fd, unsigned int u32FileID, unsigned int u32RecID, ISIL_ALLOCATE_FILE_REC_INFO *stRecInfo);

extern void ISIL_FM_FILE_ALLOCATE_LockIndex(void);
extern void ISIL_FM_FILE_ALLOCATE_UnlockIndex(void);

/*获取二级索引写的起始位置,成功返回offset,失败返回0*/
extern int ISIL_FM_FILE_ALLOCATE_GetRecIndexStartOffset(int s32IndexFd, unsigned int u32Channel);
extern int ISIL_FM_FILE_ALLOCATE_GetRecIndexStartOffsetById(unsigned int u32RecIndexId);

/*磁盘的初始化，建立自己的专属目录结构*/
extern int ISIL_FM_FILE_ALLOCATE_Format();
/**/
extern int ISIL_FM_FILE_ALLOCATE_ReadAllocParam();

extern int ISIL_FM_FILE_ALLOCATE_SetNodePath(char *NodePath);

extern int ISIL_FM_FILE_ALLOCATE_GetRecIndexSize(void);

extern int ISIL_FILE_ALLOCATE_OpenMidx(unsigned int u32MidxFileID);
extern int ISIL_FM_FILE_ALLOCATE_ReadMIdxHeader(int s32MidxFd, ISIL_MJPG_INDEX_HEADER *pMidxHeader);
extern int ISIL_FM_FILE_ALLOCATE_WriteMIdxHeader(int s32MidxFd, ISIL_MJPG_INDEX_HEADER *pMidxHeader);
extern int ISIL_FM_FILE_ALLOCATE_ReadMIdx(int s32MidxFd, unsigned int u32MidxNum, ISIL_MJPG_INDEX *pMidx);
extern int ISIL_FM_FILE_ALLOCATE_WriteMIdx(int s32MidxFd, unsigned int u32MidxNum, ISIL_MJPG_INDEX *pMidx);
extern int IS_MJPG_TYPE(unsigned char type);

#ifdef __cplusplus
    }
#endif
#endif

