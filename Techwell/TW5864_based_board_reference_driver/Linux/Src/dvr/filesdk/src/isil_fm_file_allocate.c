#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

#include "isil_fm_h264.h"
#include "isil_fm_file_allocate.h"
#include "isil_fm_fdisk.h"
#include "isil_fm_log.h"
#include "isil_fm_rec.h"
#include "isil_fm_utils.h"
//#include "isil_fm_msg.h"

/*index.bin文件中不同区域的起始位置定义*/
#define FILE_INDEX_BASE_OFFSET  (0)
#define FILE_INDEX_USING_OFFSET (sizeof(ISIL_ALLOCATE_FILE_BASEINFO))
#define FILE_INDEX_FULL_OFFSET  (FILE_INDEX_USING_OFFSET + sizeof(ISIL_ALLOCATE_FILE_USING_INFO)*ISIL_ALLOCATE_MAXCHANNEL)

static unsigned int FILE_INDEX_REC_OFFSET;
static unsigned int FILE_INDEX_RECLOG_SUM;//每个静态文件里录像记录预分配总数
static unsigned int FILE_NUM;//实际分配的静态文件数目
static unsigned int FILE_SIZE;//静态文件大小
static unsigned int REC_INDEX_SIZE;//每个索引文件的长度
static unsigned int ISIL_FM_DISK_OVER_WRITE = 1;//硬盘覆盖,1:覆盖,0:不覆盖
static unsigned int u32RecIndexSize;//rec index size each rec file

pthread_mutex_t FILE_INDEX_LOCK = PTHREAD_MUTEX_INITIALIZER;//index00.bin文件锁

static int ISIL_FM_DISK_SaveDiskInfo(ISIL_SAVE_DISK_INFO DiskInfo);
static int ISIL_FM_DISK_GetDiskInfo(ISIL_SAVE_DISK_INFO *DiskInfo);
static int ISIL_FM_FILE_ALLOCATE_SaveOverWrite(unsigned int u32OverWrite);

static void TransFileBaseInfo(ISIL_ALLOCATE_FILE_BASEINFO* info)
{
    if(NULL == info)
        return;

    swap_four_bytes(&(info->u32CoveredFileSum));
    swap_four_bytes(&(info->u32FileTotle));
    swap_four_bytes(&(info->u32FullFileSum));
    swap_four_bytes(&(info->u32RecFileTotle));
    swap_four_bytes(&(info->u32RecIndexSize));
    swap_four_bytes(&(info->u32RecIndexSumEachFile));
    swap_four_bytes(&(info->u32UsedFileSum));

    return;
}

static void TransFileUsingInfo(ISIL_ALLOCATE_FILE_USING_INFO* info)
{
    if(NULL == info)
        return;

    swap_two_bytes(&(info->u16LogSum));
    swap_two_bytes(&(info->u16RecSum));
    swap_four_bytes(&(info->u32UsingFileID));
    swap_two_bytes(&(info->stStartDT.Date.Year));

    return;
}

static void TransFileFullInfo(ISIL_ALLOCATE_FILE_FULL_INFO* info)
{
    if(NULL == info)
        return;

    swap_two_bytes(&(info->u16LogSum));
    swap_two_bytes(&(info->u16RecSum));
    swap_four_bytes(&(info->u32Channel));
    swap_four_bytes(&(info->u32FileID));
    swap_two_bytes(&(info->stStartDT.Date.Year));
    return;
}

static void TransFileRecInfo(ISIL_ALLOCATE_FILE_REC_INFO* info)
{
    if(NULL == info)
        return;

    swap_two_bytes(&(info->stStartDT.Date.Year));
    swap_two_bytes(&(info->stStopDT.Date.Year));
    swap_four_bytes(&(info->u32RecIndexId));
    swap_four_bytes(&(info->u32StartOffset));
    swap_four_bytes(&(info->u32StopOffset));
    return;
}

static void TransMidxHeader(ISIL_MJPG_INDEX_HEADER* info)
{
    if(NULL == info)
        return;

    swap_four_bytes(&(info->u32FileTotleSize));
    swap_four_bytes(&(info->u32CurSize));
    swap_four_bytes(&(info->u32IndexTotleNum));
    swap_four_bytes(&(info->u32IndexCurNum));
    swap_four_bytes(&(info->u32Channel));
    return;
}

static void TransMidx(ISIL_MJPG_INDEX* info)
{
    if(NULL == info)
        return;

    swap_two_bytes(&(info->stDT.Date.Year));
    swap_four_bytes(&(info->u32offset));
    swap_four_bytes(&(info->u32Len));
    return;
}

static void TransSaveDiskInfo(ISIL_SAVE_DISK_INFO* info)
{
    if(NULL == info)
        return;

	swap_four_bytes(&(info->IsFormat));
	swap_four_bytes(&(info->ISIL_FM_DISK_OVER_WRITE));
	swap_four_bytes(&(info->FILE_INDEX_RECLOG_SUM));
	swap_four_bytes(&(info->FILE_NUM));
	swap_four_bytes(&(info->FILE_SIZE));
	swap_four_bytes(&(info->REC_INDEX_SIZE));
	swap_four_bytes(&(info->u32MaxRecChannel));
	swap_four_bytes(&(info->u32RecIndexSize));
	swap_four_bytes(&(info->u32RecIndexSumEachFile));

    return;
}


char HDD_REC_DIR[REC_ROOT_DIR_MAX_LEN + 1];

/*获取静态文件大小*/
unsigned int ISIL_FM_FILE_ALLOCATE_GetFileSize()
{
    return (FILE_SIZE*1024);
}
/*设置硬盘是否覆盖*/
void ISIL_FM_FILE_ALLOCATE_SetDiskOverWrite(unsigned int u32IsOverWrite)
{
    ISIL_FM_DISK_OVER_WRITE = u32IsOverWrite;
    ISIL_FM_FILE_ALLOCATE_SaveOverWrite(u32IsOverWrite);
}

/*获取覆盖标志位*/
int ISIL_FM_FILE_ALLOCATE_GetDiskOverWrite(void)
{
    return (ISIL_FM_DISK_OVER_WRITE);
}


/*获取整个硬盘的容量，以K为单位返回，失败返回-1*/
static unsigned int ISIL_FM_FILE_ALLOCATE_GetHddCapacity(void)
{
    unsigned int s32TotleCapacity = 0;
    s32TotleCapacity = ISIL_FM_DISK_GetDiskCapacity();

    fprintf(stderr, "allocate get totle from fdisk: %d(KB)\n", s32TotleCapacity);
    s32TotleCapacity = (s32TotleCapacity*9)/10;//保留10%空间
    return s32TotleCapacity;
}


/*创建目录*/
static int ISIL_FM_FILE_ALLOCATE_CreatDir()
{
    char path[64];

    memset(path, 0x00, 64);
    sprintf(path, "mkdir -p %s%s", HDD_REC_DIR, ISIL_DATA_DIR);
    system(path);

    memset(path, 0x00, 64);
    sprintf(path, "mkdir -p %s%s", HDD_REC_DIR, ISIL_INDEX_DIR);
    system(path);

    memset(path, 0x00, 64);
    sprintf(path, "mkdir -p %s%s%s", HDD_REC_DIR, ISIL_INDEX_DIR, ISIL_REC_INDEX_DIR);
    system(path);

    memset(path, 0x00, 64);
    sprintf(path, "mkdir -p %s%s%s", HDD_REC_DIR, ISIL_INDEX_DIR, ISIL_FILE_INDEX_DIR);
    system(path);

    memset(path, 0x00, 64);
    sprintf(path, "mkdir -p %s%s%s", HDD_REC_DIR, ISIL_INDEX_DIR, ISIL_MJPG_INDEX_DIR);
    system(path);

    return 0;
}

/*创建文件*/
static int ISIL_FM_FILE_ALLOCATE_CreatFile(char *pPathName)
{
   int s32Ret = 0;

   s32Ret = open(pPathName, O_RDWR | O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
   //fprintf(stderr, "creat:%s, s32ret = %d\n", pPathName, s32Ret);
   if(s32Ret == -1)
   {
        perror("why: ");
        fprintf(stderr, "creat:%s fail\n", pPathName);
        return -1;
   }

   return s32Ret;
}

/*打开文件*/
int ISIL_FM_FILE_ALLOCATE_OpenFile(char *pPathName)
{
   int s32Ret = 0;

   s32Ret = open(pPathName, O_RDWR, S_IRUSR | S_IWUSR);
   if(s32Ret == -1)
   {
        fprintf(stderr, "open:%s fail\n", pPathName);
        return -1;
   }

   return s32Ret;
}

#if 1
static int ISIL_FM_FILE_ALLOCATE_InitData_1M(int s32Fd, int offset, int s32Len)
{
    char buff[1*1024*1024]= {};//1M
    int s32Ret = 0;
    //int i = 0;
    int s32Offset = 0;
    int s32Left = s32Len;

    if(s32Len < 0)
    {
        fprintf(stderr, "%s, %d, fail\n", __FUNCTION__, __LINE__);
        return -1;
    }

    while(s32Offset != s32Len)
    {
        if(lseek(s32Fd, offset + s32Offset, SEEK_SET) == -1)
        {
            fprintf(stderr, "%s, %d, fail\n", __FUNCTION__, __LINE__);
            return -1;
        }

        if(s32Left <= (1*1024*1024))//需要写的所有大小小于1M
        {
            s32Ret = write(s32Fd, buff, s32Left);
            if(s32Ret != s32Left)
            {
                fprintf(stderr, "%s, %d, fail\n", __FUNCTION__, __LINE__);
                return -1;
            }
        }
        else
        {
            s32Ret = write(s32Fd, buff, (1*1024*1024));
            if(s32Ret != (1*1024*1024))
            {
                fprintf(stderr, "%s, %d, fail\n", __FUNCTION__, __LINE__);
                return -1;
            }
        }

        s32Offset += s32Ret;
        s32Left = (s32Len - s32Offset);
    }

    return s32Offset;
}
#endif

#if 1
/*初始化isilxxx.264*/
/*一次写1KB*/
static int ISIL_FM_FILE_ALLOCATE_InitData(int s32Fd, unsigned int s32StartOffset, int s32Len)
{
    char buff[1024]= {};//1k
    int s32Ret = 0;
    int i = 0;
    int s32Offset = 0;

    if(s32Len < 0)
    {
        fprintf(stderr, "%s, %d, fail\n", __FUNCTION__, __LINE__);
        return -1;
    }

    for(i = 0; i < s32Len; i++)
    {
        if(lseek(s32Fd, (s32StartOffset+s32Offset), SEEK_SET) == -1)
        {
            fprintf(stderr, "%s, %d, fail\n", __FUNCTION__, __LINE__);
            return -1;
        }

        s32Ret = write(s32Fd, buff, 1024);
        if(s32Ret != 1024)
        {
            fprintf(stderr, "%s, %d, fail\n", __FUNCTION__, __LINE__);
            return -1;
        }

        s32Offset += s32Ret;
    }

    if(s32Offset != (s32Len*1024))
    {
        fprintf(stderr, "%s, %d, write fail\n", __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

#endif

/*write base info*/
static int ISIL_FM_FILE_ALLOCATE_WriteIndexBase(int s32Fd, ISIL_ALLOCATE_FILE_BASEINFO stBaseInfo)
{
    int s32Ret = 0;

    s32Ret = lseek(s32Fd, 0, SEEK_SET);
    if(s32Ret < 0)
    {
        fprintf(stderr, "write base lseek fail\n");
        return -1;
    }
    TransFileBaseInfo(&stBaseInfo);
    s32Ret = write(s32Fd, &stBaseInfo, sizeof(ISIL_ALLOCATE_FILE_BASEINFO));
    if(s32Ret != sizeof(ISIL_ALLOCATE_FILE_BASEINFO))
    {
        fprintf(stderr, "write base write len fail\n");
        return -1;
    }

    return 0;
}

/*read base info*/
int ISIL_FM_FILE_ALLOCATE_ReadIndexBase(int s32Fd, ISIL_ALLOCATE_FILE_BASEINFO *stBaseInfo)
{
    int s32Ret = 0;

    s32Ret = lseek(s32Fd, 0, SEEK_SET);
    if(s32Ret < 0)
    {
        fprintf(stderr, "read base lseek fail\n");
        return -1;
    }

    s32Ret = read(s32Fd, stBaseInfo, sizeof(ISIL_ALLOCATE_FILE_BASEINFO));

    if(s32Ret != sizeof(ISIL_ALLOCATE_FILE_BASEINFO))
    {
        fprintf(stderr, "read index base len fail\n");
        return -1;
    }
    TransFileBaseInfo(stBaseInfo);
    return 0;
}

/*write using info*/
static int ISIL_FM_FILE_ALLOCATE_WriteIndexUsing(int s32Fd, unsigned int u32Channel, ISIL_ALLOCATE_FILE_USING_INFO stUsingInfo)
{
    int s32Ret = 0;

    s32Ret = lseek(s32Fd, (FILE_INDEX_USING_OFFSET + (u32Channel*sizeof(ISIL_ALLOCATE_FILE_USING_INFO))), SEEK_SET);
    if(s32Ret < 0)
    {
        fprintf(stderr, "write using lseek fail\n");
        return -1;
    }
    TransFileUsingInfo(&stUsingInfo);
    s32Ret = write(s32Fd, &stUsingInfo, sizeof(ISIL_ALLOCATE_FILE_USING_INFO));
    if(s32Ret != sizeof(ISIL_ALLOCATE_FILE_USING_INFO))
    {
        fprintf(stderr, "write using write len fail\n");
        return -1;
    }

    return 0;
}

/*read using info*/
int ISIL_FM_FILE_ALLOCATE_ReadIndexUsing(int s32Fd, unsigned int u32Channel, ISIL_ALLOCATE_FILE_USING_INFO *stUsingInfo)
{
    int s32Ret = 0;

    s32Ret = lseek(s32Fd, (FILE_INDEX_USING_OFFSET + (u32Channel*sizeof(ISIL_ALLOCATE_FILE_USING_INFO))), SEEK_SET);
    //fprintf(stderr, "read index offset: %x\n", s32Ret);
    if(s32Ret < 0)
    {
        fprintf(stderr, "read using lseek fail\n");
        return -1;
    }

    s32Ret = read(s32Fd, stUsingInfo, sizeof(ISIL_ALLOCATE_FILE_USING_INFO));
    if(s32Ret != sizeof(ISIL_ALLOCATE_FILE_USING_INFO))
    {
        fprintf(stderr, "read using write len[%d] fail\n", s32Ret);
        perror("why: ");
        return -1;
    }
    TransFileUsingInfo(stUsingInfo);
    return 0;
}

/*write full info*/
static int ISIL_FM_FILE_ALLOCATE_WriteIndexFull(int s32Fd, unsigned int u32FileID, ISIL_ALLOCATE_FILE_FULL_INFO stFullInfo)
{
    int s32Ret = 0;

    s32Ret = lseek(s32Fd, (FILE_INDEX_FULL_OFFSET + (u32FileID*sizeof(ISIL_ALLOCATE_FILE_FULL_INFO))), SEEK_SET);
    if(s32Ret < 0)
    {
        fprintf(stderr, "write full lseek fail\n");
        return -1;
    }
    TransFileFullInfo(&stFullInfo);
    s32Ret = write(s32Fd, &stFullInfo, sizeof(ISIL_ALLOCATE_FILE_FULL_INFO));
    if(s32Ret != sizeof(ISIL_ALLOCATE_FILE_FULL_INFO))
    {
        fprintf(stderr, "write full write len fail\n");
        return -1;
    }

    return 0;
}

/*read full info*/
int ISIL_FM_FILE_ALLOCATE_ReadIndexFull(int s32Fd, unsigned int u32FileID, ISIL_ALLOCATE_FILE_FULL_INFO *stFullInfo)
{
    int s32Ret = 0;

    s32Ret = lseek(s32Fd, (FILE_INDEX_FULL_OFFSET + (u32FileID*sizeof(ISIL_ALLOCATE_FILE_FULL_INFO))), SEEK_SET);
    if(s32Ret < 0)
    {
        fprintf(stderr, "read full lseek fail\n");
        return -1;
    }

    s32Ret = read(s32Fd, stFullInfo, sizeof(ISIL_ALLOCATE_FILE_FULL_INFO));
    if(s32Ret != sizeof(ISIL_ALLOCATE_FILE_FULL_INFO))
    {
        fprintf(stderr, "read full write len fail\n");
        return -1;
    }
    TransFileFullInfo(stFullInfo);
    return 0;
}

/*write rec log info*/
static int ISIL_FM_FILE_ALLOCATE_WriteIndexRec(int s32Fd, unsigned int u32FileID, unsigned int u32RecID, ISIL_ALLOCATE_FILE_REC_INFO stRecInfo)
{
    int s32Ret = 0;
    int s32Offset = 0;

    //fprintf(stderr, "write rec log lseek: FILE_INDEX_REC_OFFSET:%x,%x\n", FILE_INDEX_REC_OFFSET, FILE_INDEX_RECLOG_SUM);

    s32Offset = FILE_INDEX_REC_OFFSET + u32FileID*sizeof(ISIL_ALLOCATE_FILE_REC_INFO)*FILE_INDEX_RECLOG_SUM + u32RecID*sizeof(ISIL_ALLOCATE_FILE_REC_INFO);

    //fprintf(stderr, "write rec log lseek: %x\n", s32Offset);

    s32Ret = lseek(s32Fd, s32Offset, SEEK_SET);
    if(s32Ret != s32Offset)
    {
        fprintf(stderr, "write rec lseek fail\n");
        return -1;
    }
    TransFileRecInfo(&stRecInfo);
    s32Ret = write(s32Fd, &stRecInfo, sizeof(ISIL_ALLOCATE_FILE_REC_INFO));
    if(s32Ret != sizeof(ISIL_ALLOCATE_FILE_REC_INFO))
    {
        fprintf(stderr, "write rec len fail\n");
        return -1;
    }

    return s32Ret;
}

/*read rec log info*/
int ISIL_FM_FILE_ALLOCATE_ReadIndexRec(int s32Fd, unsigned int u32FileID, unsigned int u32RecID, ISIL_ALLOCATE_FILE_REC_INFO *stRecInfo)
{
    int s32Ret = 0;
    int s32Offset = 0;

    s32Offset = FILE_INDEX_REC_OFFSET + u32FileID*sizeof(ISIL_ALLOCATE_FILE_REC_INFO)*FILE_INDEX_RECLOG_SUM \
               + u32RecID*sizeof(ISIL_ALLOCATE_FILE_REC_INFO);

    s32Ret = lseek(s32Fd, s32Offset, SEEK_SET);
    if(s32Ret != s32Offset)
    {
        fprintf(stderr, "read rec lseek fail\n");
        return -1;
    }

    s32Ret = read(s32Fd, stRecInfo, sizeof(ISIL_ALLOCATE_FILE_REC_INFO));
    if(s32Ret != sizeof(ISIL_ALLOCATE_FILE_REC_INFO))
    {
        fprintf(stderr, "read rec len fail\n");
        return -1;
    }
    TransFileRecInfo(stRecInfo);
    return 0;
}

/*判断磁盘是否满*/
//1:full,0:not full,-1:fail
int ISIL_FM_FILE_ALLOCATE_DiskIsFull(int s32IndexFd)
{
    int s32Ret = 0;
    int s32IsFull = 0;//是否覆盖，1:覆盖
    ISIL_ALLOCATE_FILE_BASEINFO stBase;

    /*update base info*/
    memset(&stBase, 0x00, sizeof(ISIL_ALLOCATE_FILE_BASEINFO));

    s32Ret = ISIL_FM_FILE_ALLOCATE_ReadIndexBase(s32IndexFd, &stBase);
    if(s32Ret < 0)
    {
        return -1;
    }

    if(stBase.u32FullFileSum == stBase.u32FileTotle)
    {
        if(ISIL_FM_FILE_ALLOCATE_GetDiskOverWrite())
        {
            s32IsFull = 0;
        }
        else
        {
            s32IsFull = 1;
        }
    }

    return s32IsFull;
}

/*硬盘格式化*/
int ISIL_FM_FILE_ALLOCATE_Format()
{
    unsigned int u32Capacity = 0;//硬盘容量
    unsigned int u32FileSize = 0;//每个静态文件的大小
    unsigned int u32FileSum = 0;//静态文件的数量
    unsigned int u32FileIndexMaxSize = 0;
    unsigned int u32RecIndexMaxSize = 0;
    unsigned int u32RecIndexMaxSum = 0;
    unsigned int u32RecIndexSum = 0;
    unsigned int u32MaxFileSum = 0;//最大文件个数
    unsigned int u32FileID = 0;
    unsigned int u32MIdxFileSize = 0;//每个mjpg索引文件大小
    unsigned int u32MIdxTotleSize = 0;//所有mjpg索引文件总大小
    unsigned int u32MIdxSum = 0;//每个mjpg索引文件中索引记录总项数，即mjpg总帧数
    char strName[64];
    int s32Ret = 0;
    int s32DataFd = 0;
    int s32IndexFd = 0;

    fprintf(stderr, "---enter %s--\n", __FUNCTION__);
    /*delete all file*/
    sprintf(strName, "rm -rf %s*", HDD_REC_DIR);
    system(strName);

    u32Capacity = ISIL_FM_FILE_ALLOCATE_GetHddCapacity();
    u32Capacity = u32Capacity - ISIL_FM_LOG_GetFileSize();
    fprintf(stderr, "capacity is: %d(kB), %d(MB),\n", u32Capacity, u32Capacity>>10);
    ISIL_FM_FILE_ALLOCATE_CreatDir();

    //根据容量来确定文件大小，至少保证一个通道一个文件
    /*文件分配原则，每个文件尽可能向512M靠拢*/
    #if 0
    if(u32Capacity > (100*1024*1024))//16G,?С?512M
    {
        u32FileSize = ISIL_ALLOCATE_FILE_SIZE_2048M;
    }
    else 
    #endif
    if(u32Capacity > (16*1024*1024))//16G,?С?512M
    {
        u32FileSize = ISIL_ALLOCATE_FILE_SIZE_512M;
    }
    else if(u32Capacity > (ISIL_ALLOCATE_FILE_SIZE_256M*ISIL_ALLOCATE_MAXCHANNEL))
    {
        u32FileSize = ISIL_ALLOCATE_FILE_SIZE_256M;
    }
    else if(u32Capacity > (ISIL_ALLOCATE_FILE_SIZE_128M*ISIL_ALLOCATE_MAXCHANNEL))
    {
        u32FileSize = ISIL_ALLOCATE_FILE_SIZE_128M;
    }
    else if(u32Capacity > (ISIL_ALLOCATE_FILE_SIZE_64M*ISIL_ALLOCATE_MAXCHANNEL))
    {
        u32FileSize = ISIL_ALLOCATE_FILE_SIZE_64M;
    }
    else if(u32Capacity > (ISIL_ALLOCATE_FILE_SIZE_32M*ISIL_ALLOCATE_MAXCHANNEL))
    {
        u32FileSize = ISIL_ALLOCATE_FILE_SIZE_32M;
    }
    else if(u32Capacity > (ISIL_ALLOCATE_FILE_SIZE_16M*ISIL_ALLOCATE_MAXCHANNEL))
    {
        u32FileSize = ISIL_ALLOCATE_FILE_SIZE_16M;
    }
    else
    {
        u32FileSize = ISIL_ALLOCATE_FILE_SIZE_512K;
    }

    FILE_SIZE = u32FileSize;
    u32MaxFileSum = u32Capacity/u32FileSize;
    u32FileIndexMaxSize = sizeof(ISIL_ALLOCATE_FILE_BASEINFO) + ISIL_ALLOCATE_MAXCHANNEL*sizeof(ISIL_ALLOCATE_FILE_USING_INFO)\
        + u32MaxFileSum*sizeof(ISIL_ALLOCATE_FILE_FULL_INFO) + u32MaxFileSum*(sizeof(ISIL_ALLOCATE_FILE_REC_INFO)*(u32FileSize/ISIL_ALLOCATE_FILE_SIZE_512K));
    u32FileIndexMaxSize = u32FileIndexMaxSize/1024 + 1;//kB为单位

    fprintf(stderr, "s32FileIndexMaxSize=%d(kB)\n", u32FileIndexMaxSize);

    /*整个HDD录像文件最大的索引数目*/
    u32RecIndexMaxSum = u32MaxFileSum*(u32FileSize/ISIL_ALLOCATE_FILE_SIZE_512K);
    fprintf(stderr, "s32RecIndexMaxSum=%d\n", u32RecIndexMaxSum);
    /*每个录像文件索引的最大长度*/
    u32RecIndexMaxSize = sizeof(ISIL_FILE_INDEX_NAL) + sizeof(ISIL_GOP_INDEX)*(u32FileSize/ISIL_ALLOCATE_WRITE_BITRATE_MIN);
    u32RecIndexMaxSize = u32RecIndexMaxSize/1024 + 1;//kB
    fprintf(stderr, "u32RecIndexMaxSize=%d\n", u32RecIndexMaxSize);

    //mjpg index totle size
    //mjpg index sum
    u32MIdxSum = FILE_SIZE/ISIL_MJPG_MINMUM_SIZE + 1;
    //each mjpg index file size
    u32MIdxFileSize = sizeof(ISIL_MJPG_INDEX_HEADER) + u32MIdxSum*sizeof(ISIL_MJPG_INDEX);
    u32MIdxFileSize = u32MIdxFileSize/1024 + 1;//kB
    //all mjpg index totle size
    u32MIdxTotleSize = u32MaxFileSum*u32MIdxFileSize;//KB

    fprintf(stderr, "---Midx,sum[%d],size[%d],allsize[%d]---\n", u32MIdxSum, u32MIdxFileSize, u32MIdxTotleSize);

    /*根据以上最大值来确定实际分配的静态文件个数*/
    u32FileSum = (u32Capacity - u32FileIndexMaxSize - u32MIdxTotleSize - u32RecIndexMaxSize*u32RecIndexMaxSum)/u32FileSize;
    FILE_NUM = u32FileSum;
    /*实际需要分配的录像索引文件个数*/
    u32RecIndexSum = u32FileSum*(u32FileSize/ISIL_ALLOCATE_FILE_SIZE_512K);//有待商榷

    if((u32FileSize/ISIL_ALLOCATE_FILE_SIZE_512K) > 100)
    {
        FILE_INDEX_RECLOG_SUM = 100;//s32FileSize/ISIL_ALLOCATE_FILE_SIZE_512K;//有待商榷
    }
    else
    {
        FILE_INDEX_RECLOG_SUM = u32FileSize/ISIL_ALLOCATE_FILE_SIZE_512K;
    }

    /*创建264 index文件信息*/
    REC_INDEX_SIZE = sizeof(ISIL_FILE_INDEX_NAL) + sizeof(ISIL_GOP_INDEX)*(FILE_SIZE/ISIL_ALLOCATE_WRITE_BITRATE_MIN);
    REC_INDEX_SIZE = REC_INDEX_SIZE/1024 + 1;//kB


    fprintf(stderr, "file sum=%d,size=%d \n", u32FileSum, u32FileSize);
    if(u32RecIndexSum <= 0)
    {
        fprintf(stderr, "allocate file  fail\n");
        return -1;
    }

    /*建立data文件，并初始化*/
    for(u32FileID = 0; u32FileID < u32FileSum; u32FileID++)
    {
        sprintf(strName, "%s%sisil%d.264", HDD_REC_DIR, ISIL_DATA_DIR, u32FileID);

        fprintf(stderr, "000 data file: %s,%d, %d\n", strName, u32FileID, u32FileSum);

        s32DataFd = ISIL_FM_FILE_ALLOCATE_CreatFile(strName);
        if(s32DataFd < 0)
        {
            fprintf(stderr, "ISIL_FM_FILE_ALLOCATE_CreatFile fail\n");
            return -1;
        }

        close(s32DataFd);
    }

    /*创建index文件*/
    sprintf(strName, "%s%s%s%s", HDD_REC_DIR, ISIL_INDEX_DIR, ISIL_FILE_INDEX_DIR, ISIL_FILE_INDEX_NAME);
    s32IndexFd = ISIL_FM_FILE_ALLOCATE_CreatFile(strName);
    if(s32IndexFd < 0)
    {
        fprintf(stderr, "ISIL_FM_FILE_ALLOCATE_CreatFile fail\n");
        return -1;
    }

    /*初始化index00.bin文件*/
    ISIL_ALLOCATE_FILE_BASEINFO stBaseInfo;
    memset(&stBaseInfo, 0x00, sizeof(ISIL_ALLOCATE_FILE_BASEINFO));
    stBaseInfo.u32FileTotle = u32FileSum;
    stBaseInfo.u32UsedFileSum = 0xffffffff;
    stBaseInfo.u32FullFileSum = 0xffffffff;
    stBaseInfo.u32CoveredFileSum = 0xffffffff;
    stBaseInfo.u32RecIndexSumEachFile = FILE_INDEX_RECLOG_SUM;
    stBaseInfo.u32RecIndexSize = REC_INDEX_SIZE*1024;
    s32Ret = ISIL_FM_FILE_ALLOCATE_WriteIndexBase(s32IndexFd, stBaseInfo);
    if(s32Ret < 0)
    {
        fprintf(stderr, "ISIL_FM_FILE_ALLOCATE_WriteIndexBase fail\n");
        return -1;
    }

    ISIL_ALLOCATE_FILE_USING_INFO stUsingInfo;
    memset(&stUsingInfo, 0x00, sizeof(ISIL_ALLOCATE_FILE_USING_INFO));
    stUsingInfo.u32UsingFileID = 0xFFFFFFFF;

    int u32Channel = 0;
    for(u32Channel = 0; u32Channel < ISIL_ALLOCATE_MAXCHANNEL; u32Channel++)
    {
        s32Ret = ISIL_FM_FILE_ALLOCATE_WriteIndexUsing(s32IndexFd, u32Channel, stUsingInfo);
        if(s32Ret < 0)
        {
            fprintf(stderr, "ISIL_FM_FILE_ALLOCATE_WriteIndexUsing fail\n");
            return -1;
        }
    }

    ISIL_ALLOCATE_FILE_FULL_INFO stFullInfo;
    memset(&stFullInfo, 0x00, sizeof(ISIL_ALLOCATE_FILE_FULL_INFO));
    stFullInfo.u32Channel = 0xffffffff;

    for(u32FileID = 0; u32FileID < u32FileSum; u32FileID++)
    {
        stFullInfo.u32FileID = u32FileID;
        s32Ret = ISIL_FM_FILE_ALLOCATE_WriteIndexFull(s32IndexFd, u32FileID, stFullInfo);
        if(s32Ret < 0)
        {
            fprintf(stderr, "ISIL_FM_FILE_ALLOCATE_WriteIndexFull fail\n");
            return -1;
        }
    }

    FILE_INDEX_REC_OFFSET = FILE_INDEX_FULL_OFFSET + u32FileSum*sizeof(ISIL_ALLOCATE_FILE_FULL_INFO);
    fprintf(stderr, "index rec offset start: %x\n", FILE_INDEX_REC_OFFSET);
#if 0
    int s32LogID = 0;
    ISIL_ALLOCATE_FILE_REC_INFO stRecLog;
    memset(&stRecLog, 0x00, sizeof(ISIL_ALLOCATE_FILE_REC_INFO));

    for(u32FileID = 0; u32FileID < s32FileSum; u32FileID++)
    {
        for(s32LogID = 0; s32LogID < FILE_INDEX_RECLOG_SUM; s32LogID++)
        {
            s32Ret = ISIL_FM_FILE_ALLOCATE_WriteIndexRec(s32IndexFd, u32FileID, s32LogID, stRecLog);
            if(s32Ret < 0)
            {
                fprintf(stderr, "ISIL_FM_FILE_ALLOCATE_WriteIndexRec fail\n");
                return -1;
            }
            TotleByte += s32Ret;
        }
        }
#else
    if(ISIL_FM_FILE_ALLOCATE_InitData_1M(s32IndexFd, FILE_INDEX_REC_OFFSET, u32FileSum*FILE_INDEX_RECLOG_SUM*sizeof(ISIL_ALLOCATE_FILE_REC_INFO)) < 0)
    {
        fprintf(stderr, "----ISIL_FM_FILE_ALLOCATE_InitData_1M fail---\n");
        return -1;
    }
#endif
    close(s32IndexFd);
    fprintf(stderr, "s32FileSum: %d, FILE_INDEX_RECLOG_SUM=%d\n", u32FileSum, FILE_INDEX_RECLOG_SUM);

    /*创建264 index文件*/
    fprintf(stderr, "REC_INDEX_SIZE=%d\n", REC_INDEX_SIZE);
    //fprintf(stderr, "######################come here 1111 ############\n");
    unsigned int u32RecIndexId = 0;
    int s32RecIndexFd = 0;

    for(u32RecIndexId = 0; u32RecIndexId < FILE_NUM; u32RecIndexId++)
    {
        sprintf(strName, "%s%s%s%d.bin", HDD_REC_DIR, ISIL_INDEX_DIR, ISIL_REC_INDEX_DIR, u32RecIndexId);

        s32RecIndexFd = ISIL_FM_FILE_ALLOCATE_CreatFile(strName);
        if(s32RecIndexFd < 0)
        {
            fprintf(stderr, "ISIL_FM_FILE_ALLOCATE_CreatFile rec index fail\n");
            return -1;
        }

        close(s32RecIndexFd);
    }

    /*creat and init index/mjpg_index/xx.bin*/
    ISIL_MJPG_INDEX_HEADER mjpg_index_header;
    int i = 0;
    memset(&mjpg_index_header, 0x00, sizeof(ISIL_MJPG_INDEX_HEADER));
    mjpg_index_header.u32FileTotleSize = u32MIdxFileSize*1024;
    mjpg_index_header.u32IndexTotleNum = u32MIdxSum;
    mjpg_index_header.u32Channel = 0xffffffff;
    mjpg_index_header.u32CurSize = sizeof(ISIL_MJPG_INDEX_HEADER);

    for(i = 0; i < FILE_NUM; i++)
    {
        int midx_fd = 0;
        sprintf(strName, "%s%s%s%d.bin", HDD_REC_DIR, ISIL_INDEX_DIR, ISIL_MJPG_INDEX_DIR, i);
        midx_fd = ISIL_FM_FILE_ALLOCATE_CreatFile(strName);
        if(midx_fd < 0)
        {
            fprintf(stderr, "ISIL_FM_FILE_ALLOCATE_CreatFile mjpg index fail\n");
            return -1;
        }

        //init mjpg index file
        if(ISIL_FM_FILE_ALLOCATE_InitData(midx_fd, 0, u32MIdxFileSize) < 0){
            fprintf(stderr, "init mjpg index file fail\n");
            return -1;
        }

        //write header into mjpg index file
        if(ISIL_FM_FILE_ALLOCATE_WriteMIdxHeader(midx_fd, &mjpg_index_header) < 0){
            fprintf(stderr, " write mjpg index header fail \n");
            return -1;
        }

        close(midx_fd);
    }

    //write idisk info into device.bin
    ISIL_SAVE_DISK_INFO write_disk;
    write_disk.IsFormat = 1;
    write_disk.FILE_INDEX_RECLOG_SUM = FILE_INDEX_RECLOG_SUM;
    write_disk.FILE_NUM = FILE_NUM;
    write_disk.FILE_SIZE = FILE_SIZE;
    write_disk.REC_INDEX_SIZE = REC_INDEX_SIZE;
    write_disk.ISIL_FM_DISK_OVER_WRITE = ISIL_FM_DISK_OVER_WRITE;
    write_disk.u32RecIndexSumEachFile = stBaseInfo.u32RecIndexSumEachFile;
    write_disk.u32RecIndexSize = stBaseInfo.u32RecIndexSize;
    u32RecIndexSize = write_disk.u32RecIndexSize;
    write_disk.u32MaxRecChannel = ISIL_ALLOCATE_MAXCHANNEL;

    if(ISIL_FM_DISK_SaveDiskInfo(write_disk) < 0)
    {
        fprintf(stderr, "ISIL_FM_DISK_GetDiskInfo fail\n");
        return -1;
    }

    if(ISIL_FM_LOG_Init() < 0 )
    {
        fprintf(stderr, "ISIL_FM_LOG_Init fail\n");
        return -1;
    }
    //fprintf(stderr, "######################come here 5555 ############\n");
    return 0;
}


/*从所分配的静态文件中得到一个可用的文件*/
/*返回文件 ID,错误:-1*/
int ISIL_FM_FILE_ALLOCATE_GetDataFile(int s32IndexFd, unsigned int u32Channel)
{
    int s32Ret = 0;
    int u32FileID = 0;
    ISIL_ALLOCATE_FILE_BASEINFO stBaseInfo;
    ISIL_ALLOCATE_FILE_USING_INFO stUsingInfo;

    pthread_mutex_lock(&FILE_INDEX_LOCK);

    s32Ret = ISIL_FM_FILE_ALLOCATE_ReadIndexBase(s32IndexFd, &stBaseInfo);
    if(s32Ret < 0)
    {
        fprintf(stderr, "ISIL_FM_FILE_ALLOCATE_ReadIndexBase fail \n");
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    s32Ret = ISIL_FM_FILE_ALLOCATE_ReadIndexUsing(s32IndexFd, u32Channel, &stUsingInfo);
    if(s32Ret < 0)
    {
        fprintf(stderr, "ISIL_FM_FILE_ALLOCATE_ReadIndexUsing fail\n");
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    if(stUsingInfo.u32UsingFileID != 0xffffffff)//正在使用该文件，而且还有容量
    {
        u32FileID = stUsingInfo.u32UsingFileID|0x80000000;
    }
    else//分配新的静态文件
    {
        /*初始化后第一次写*/
        if((stBaseInfo.u32UsedFileSum == 0xffffffff) && (stBaseInfo.u32CoveredFileSum == 0xffffffff))//格式化后，第一次rec
        {
            u32FileID = 0x00;
        }
        else if((stBaseInfo.u32FileTotle == stBaseInfo.u32UsedFileSum))//写满
        {
            //fprintf(stderr, "-----*****over write get file id******---\n");
            if(ISIL_FM_FILE_ALLOCATE_GetDiskOverWrite())
            {
                if(stBaseInfo.u32CoveredFileSum == 0xffffffff)//第一次覆盖
                {
                    //fprintf(stderr, "-----*****over write 111*****---\n");
                    u32FileID = 0x00;
                }
                else
                {
                    if(stBaseInfo.u32CoveredFileSum == stBaseInfo.u32FileTotle)
                    {
                        //fprintf(stderr, "-----*****over write 222*****---\n");
                        u32FileID = 0x00;
                    }
                    else
                    {
                        //fprintf(stderr, "-----*****over write 333*****---\n");
                        u32FileID = stBaseInfo.u32CoveredFileSum;
                    }
                }
            }
            else//没有空间
            {
                fprintf(stderr, "%s, hdd write full!!!\n", __FUNCTION__);
                pthread_mutex_unlock(&FILE_INDEX_LOCK);
                return -2;
            }
        }
        else
        {
            //fprintf(stderr, "-----*****not over write 444*****---\n");
            u32FileID = stBaseInfo.u32UsedFileSum;
        }
    }

    pthread_mutex_unlock(&FILE_INDEX_LOCK);
    fprintf(stderr, "get allocate file id : %d\n", (u32FileID & 0x7fffffff));
    return u32FileID;
}

#if 0
/*从所分配的静态索引文件中得到一个可用的索引文件*/
/*返回文件ID*/
int ISIL_FM_FILE_ALLOCATE_GetRecIndexFile(int s32IndexFd)
{
    int s32Ret = 0;
    int s32RecIndexID = 0;
    ISIL_ALLOCATE_FILE_BASEINFO stBaseInfo;

    pthread_mutex_lock(&FILE_INDEX_LOCK);

    s32Ret = ISIL_FM_FILE_ALLOCATE_ReadIndexBase(s32IndexFd, &stBaseInfo);
    if(s32Ret < 0)
    {
        fprintf(stderr, "ISIL_FM_FILE_ALLOCATE_ReadIndexBase fail \n");
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    if(stBaseInfo.s32RecIndexTotle <= 0)
    {
        fprintf(stderr, "get rec index file id : totle <= 0 fail \n");
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    if(stBaseInfo.s32RecIndexUsed == 0xffffffff)
    {
        s32RecIndexID = 0;
    }
    else
    {
        if(stBaseInfo.s32RecIndexUsed == stBaseInfo.s32RecIndexTotle)
        {
            if(ISIL_FM_FILE_ALLOCATE_GetDiskOverWrite())
            {
                s32RecIndexID = 0;
            }
            else
            {
                s32RecIndexID = -1;
            }
        }
        else
        {
            s32RecIndexID = stBaseInfo.s32RecIndexUsed;
        }
    }

    pthread_mutex_unlock(&FILE_INDEX_LOCK);
    fprintf(stderr, "get rec index file id : %d\n", s32RecIndexID);

    return s32RecIndexID;
}
#endif

/*update file index*/
int ISIL_FM_FILE_ALLOCATE_UpdateFileIndex(int s32IndexFd, unsigned int u32FileID, ISIL_ALLOCATE_FILE_REC_INFO stIndexRecLog)
{
    int s32Ret = 0;
    //int s32IsOverlap = 0;//是否覆盖，1:覆盖
    //int u32FileID = 0;
    int s32RecID = 0;
    ISIL_ALLOCATE_FILE_BASEINFO      stBase;
    ISIL_ALLOCATE_FILE_USING_INFO    stUsing;
    ISIL_ALLOCATE_FILE_FULL_INFO     stFull;
    ISIL_ALLOCATE_FILE_REC_INFO      stRecLog;
    char is_mjpg = 0;//will being to rec mjpg frame
    char contiue_mjpg = 0;//in this data file,mjpg not the first rec file
    ISIL_ALLOCATE_FILE_REC_INFO  log;

    is_mjpg = IS_MJPG_TYPE(stIndexRecLog.u8RecType);

    pthread_mutex_lock(&FILE_INDEX_LOCK);

    /*初始化变量*/
    memset(&stBase, 0x00, sizeof(ISIL_ALLOCATE_FILE_BASEINFO));
    memset(&stUsing, 0x00, sizeof(ISIL_ALLOCATE_FILE_USING_INFO));
    memset(&stRecLog, 0x00, sizeof(ISIL_ALLOCATE_FILE_REC_INFO));
    /*读取基本信息*/
    s32Ret = ISIL_FM_FILE_ALLOCATE_ReadIndexBase(s32IndexFd, &stBase);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    if(stBase.u32FileTotle == 0)
    {
        fprintf(stderr, "allocate file totle is 0\n");
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    /*读取正在使用的文件信息*/
    s32Ret = ISIL_FM_FILE_ALLOCATE_ReadIndexUsing(s32IndexFd, stIndexRecLog.u8Channel, &stUsing);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    if(!stUsing.u16LogSum){
        memcpy(&stUsing.stStartDT, &stIndexRecLog.stStartDT, sizeof(S_REC_DATE_TIME));
    }
    /*通过using ID确定正在使用的文件是否写完*/
    if(stUsing.u32UsingFileID == 0xffffffff)//该文件已经写满或第一次写
    {
        if(stBase.u32UsedFileSum == 0xffffffff)//第一次写文件
        {
            stBase.u32UsedFileSum = 1;
            stBase.u32RecFileTotle = 0;

            stUsing.u32UsingFileID = 0;
            stUsing.u16LogSum = 0;
            stUsing.u16RecSum = 0;
            if(is_mjpg){
                stUsing.u16LogSum = 1;
                stUsing.u16RecSum = 0;
                s32RecID = 0;
            }
        }
        else//写满
        {
            if(stBase.u32FileTotle == stBase.u32UsedFileSum)//写满，覆盖
            {
                if(is_mjpg){
                    stUsing.u16LogSum = 1;
                    stUsing.u16RecSum = 0;
                    s32RecID = 0;
                }

                if(stBase.u32CoveredFileSum == 0xffffffff)//first overwrite
                {
                    stBase.u32CoveredFileSum = 0;
                    stUsing.u32UsingFileID = stBase.u32CoveredFileSum;
                }
                else//非第一次覆盖
                {
                    stUsing.u32UsingFileID = stBase.u32CoveredFileSum;
                }

                if(stBase.u32CoveredFileSum++ >= stBase.u32FileTotle)
                {
                    stBase.u32CoveredFileSum = 1;
                    stUsing.u32UsingFileID = 0;
                }

                /*覆盖，除去覆盖掉的文件*/
                memset(&stFull, 0x00, sizeof(ISIL_ALLOCATE_FILE_FULL_INFO));
                s32Ret = ISIL_FM_FILE_ALLOCATE_ReadIndexFull(s32IndexFd, stUsing.u32UsingFileID, &stFull);
                if(s32Ret < 0)
                {
                    pthread_mutex_unlock(&FILE_INDEX_LOCK);
                    return -1;
                }


                //fprintf(stderr, "--000---over write file: file_id= %x,rec_id:%x,log_id:%x-----\n", stFull.u32FileID, stFull.u16RecSum, stFull.u16LogSum);
                //fprintf(stderr, "write rec log lseek: FILE_INDEX_REC_OFFSET:%x,%x\n", FILE_INDEX_REC_OFFSET, FILE_INDEX_RECLOG_SUM);
                /*擦除将要覆盖文件的所有日志*/
                ISIL_ALLOCATE_FILE_REC_INFO  RecLogTmp;
                int log_id = 0;
                memset(&RecLogTmp, 0x00, sizeof(ISIL_ALLOCATE_FILE_REC_INFO));

                for(log_id = 0; log_id < stFull.u16LogSum; log_id++)
                {
                    s32Ret = ISIL_FM_FILE_ALLOCATE_WriteIndexRec(s32IndexFd, stFull.u32FileID, log_id, RecLogTmp);
                    if(s32Ret < 0)
                    {
                        pthread_mutex_unlock(&FILE_INDEX_LOCK);
                        return -1;
                    }
                }
                //fprintf(stderr, "--111---over write file: file_id= %x,rec_id:%x,log_id:%x-----\n", stFull.u32FileID, stFull.u16RecSum, stFull.u16LogSum);
                stBase.u32RecFileTotle -= stFull.u16RecSum;
                stBase.u32UsedFileSum--;
                stBase.u32FullFileSum--;

                //fprintf(stderr, "rectol:%x,used:%x,full:%x\n", stBase.u32RecFileTotle,stBase.u32UsedFileSum,stBase.u32FullFileSum);
            }
            else//未覆盖
            {
                stUsing.u32UsingFileID = stBase.u32UsedFileSum;
                if(is_mjpg){
                    contiue_mjpg = 1;
                }
            }
            stBase.u32UsedFileSum++;
        }

    }
    else//该文件还没有写满，仍然可以继续写相应通道的文件
    {
        stBase.u32UsedFileSum = stBase.u32UsedFileSum;
        stUsing.u32UsingFileID = stUsing.u32UsingFileID;
        if(is_mjpg){
            contiue_mjpg = 1;
        }
    }

    if(contiue_mjpg){
        if(!stUsing.u8HaveMjpg){
            s32RecID = stUsing.u16LogSum + 1;
        }else{
            int i = 0;
            memset(&log, 0x00, sizeof(ISIL_ALLOCATE_FILE_REC_INFO));

            for(i = 0; i < stUsing.u16LogSum; i++)
            {
                s32Ret = ISIL_FM_FILE_ALLOCATE_ReadIndexRec(s32IndexFd, stUsing.u32UsingFileID, i, &log);
                if(s32Ret < 0)
                {
                    pthread_mutex_unlock(&FILE_INDEX_LOCK);
                    return -1;
                }

                if(IS_MJPG_TYPE(log.u8RecType)){
                    s32RecID = i;
                    break;
                }
            }
        }
    }

    //u32FileID = stUsing.u32UsingFileID;

    if(!is_mjpg){
        s32RecID = stUsing.u16LogSum;
        stBase.u32RecFileTotle++;
        stUsing.u16LogSum++;
        stUsing.u16RecSum++;
    }

    /*更新录像log*/
    memset(&stRecLog, 0x00, sizeof(ISIL_ALLOCATE_FILE_REC_INFO));
    memcpy(&stRecLog, &stIndexRecLog, sizeof(ISIL_ALLOCATE_FILE_REC_INFO));

    stRecLog.u32RecIndexId = stUsing.u16RecSum;
    stRecLog.u8RecFileID   = stUsing.u16RecSum;

    if(is_mjpg && stUsing.u8HaveMjpg){
        stRecLog.u32StartOffset = log.u32StartOffset;

        stRecLog.u8RecType = log.u8RecType;
        stRecLog.u8RecType |= stIndexRecLog.u8RecType;

        stRecLog.u8SubType = log.u8SubType;
        stRecLog.u8SubType |= stIndexRecLog.u8SubType;

        memcpy(&stRecLog.stStartDT, &log.stStartDT, sizeof(S_REC_DATE_TIME));
    }

    s32Ret = ISIL_FM_FILE_ALLOCATE_WriteIndexRec(s32IndexFd, u32FileID, s32RecID, stRecLog);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    /*update using*/
    if(is_mjpg){
        stUsing.u8HaveMjpg = 1;
    }

    s32Ret = ISIL_FM_FILE_ALLOCATE_WriteIndexUsing(s32IndexFd, stIndexRecLog.u8Channel, stUsing);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }
    /*update base*/
    s32Ret = ISIL_FM_FILE_ALLOCATE_WriteIndexBase(s32IndexFd, stBase);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    /*更新full*/
#if 0
    memset(&stUsing, 0x00, sizeof(ISIL_ALLOCATE_FILE_USING_INFO));
    s32Ret = ISIL_FM_FILE_ALLOCATE_ReadIndexUsing(s32IndexFd, stIndexRecLog.u8Channel, &stUsing);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }
#endif
    memset(&stFull, 0x00, sizeof(ISIL_ALLOCATE_FILE_FULL_INFO));
    stFull.u32Channel = 0xffffffff;
    stFull.u32FileID = stUsing.u32UsingFileID;
    if(stFull.u32FileID >= stBase.u32FileTotle)
    {
        fprintf(stderr, "************ERROR!!!!!!!!!!!!**************\n");
        fprintf(stderr, "************ERROR!!!!!!!!!!!!**************\n");
        fprintf(stderr, "************ERROR!!!!!!!!!!!!**************\n");
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }
    s32Ret = ISIL_FM_FILE_ALLOCATE_WriteIndexFull(s32IndexFd, stUsing.u32UsingFileID, stFull);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    pthread_mutex_unlock(&FILE_INDEX_LOCK);

    if(!is_mjpg){
        /*根据将要写的文件ID,确定二级索引文件;根据目前该文件中的录像数目来确定二级索引文件ID,将其ID对应的数据全部初始化*/
        char strName[64]= {};
        int s32RecIndexFd = 0;
        sprintf(strName, "%s%s%s%d.bin", HDD_REC_DIR, ISIL_INDEX_DIR, ISIL_REC_INDEX_DIR, stUsing.u32UsingFileID);
        //fprintf(stderr, "the seconds rec index: %s\n", strName);

        s32RecIndexFd = ISIL_FM_FILE_ALLOCATE_OpenFile(strName);
        if(s32RecIndexFd < 0)
        {
            fprintf(stderr, "---open rec index file fail---\n");
            return -1;
        }

        /*写到该文件的哪个位置上*/
        int s32RecIndexOffset = ISIL_FM_FILE_ALLOCATE_GetRecIndexStartOffsetById(stUsing.u16RecSum);
        //fprintf(stderr, "----s32RecIndexOffset: %x----\n", s32RecIndexOffset);

        if(ISIL_FM_FILE_ALLOCATE_InitData(s32RecIndexFd, s32RecIndexOffset, REC_INDEX_SIZE) < 0)
        {
            fprintf(stderr, "----ISIL_FM_FILE_ALLOCATE_InitData fail----\n");
            return -1;
        }
    }

    return 0;
}


/*仅仅一个文件写满时调用*/
int ISIL_FM_FILE_ALLOCATE_UpdateFull(int s32IndexFd, unsigned int u32Channel)
{
    int s32Ret = 0;
    ISIL_ALLOCATE_FILE_BASEINFO      stBase;
    ISIL_ALLOCATE_FILE_USING_INFO    stUsing;
    ISIL_ALLOCATE_FILE_FULL_INFO     stFull;

    pthread_mutex_lock(&FILE_INDEX_LOCK);

    /*update base info*/
    memset(&stBase, 0x00, sizeof(ISIL_ALLOCATE_FILE_BASEINFO));
    s32Ret = ISIL_FM_FILE_ALLOCATE_ReadIndexBase(s32IndexFd, &stBase);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    if(stBase.u32FullFileSum == 0xffffffff)
    {
        stBase.u32FullFileSum = 1;
    }
    else
    {
        stBase.u32FullFileSum++;
    }

    memset(&stUsing, 0x00, sizeof(ISIL_ALLOCATE_FILE_USING_INFO));
    s32Ret = ISIL_FM_FILE_ALLOCATE_ReadIndexUsing(s32IndexFd, u32Channel, &stUsing);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    //fprintf(stderr, "stUsing.u32UsingFileID = %x\n", stUsing.u32UsingFileID);
    //fprintf(stderr, "stUsing.s32RecSum = %x\n", stUsing.s32RecSum);

    /*update full info*/
    memset(&stFull, 0x00, sizeof(ISIL_ALLOCATE_FILE_FULL_INFO));

    stFull.u32FileID = stUsing.u32UsingFileID;
    stFull.u32Channel = u32Channel;
    stFull.u16RecSum = stUsing.u16RecSum;
    stFull.u16LogSum = stUsing.u16LogSum;
    stFull.u8HaveMjpg = stUsing.u8HaveMjpg;
    memcpy(&stFull.stStartDT, &stUsing.stStartDT, sizeof(S_REC_DATE_TIME));
    s32Ret = ISIL_FM_FILE_ALLOCATE_WriteIndexFull(s32IndexFd, stUsing.u32UsingFileID, stFull);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }


    /*update using info*/
    memset(&stUsing, 0x00, sizeof(ISIL_ALLOCATE_FILE_USING_INFO));
    stUsing.u32UsingFileID = 0xffffffff;
    s32Ret = ISIL_FM_FILE_ALLOCATE_WriteIndexUsing(s32IndexFd, u32Channel, stUsing);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    /*update base info*/
    s32Ret = ISIL_FM_FILE_ALLOCATE_WriteIndexBase(s32IndexFd, stBase);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    pthread_mutex_unlock(&FILE_INDEX_LOCK);

 #if 1
    /*当写满一个文件时，根据rec sum确定还有多少二级索引项没有写*/
    char strName[64]= {};
    int s32RecIndexFd = 0;
    sprintf(strName, "%s%s%s%d.bin", HDD_REC_DIR, ISIL_INDEX_DIR, ISIL_REC_INDEX_DIR, stFull.u32FileID);
    fprintf(stderr, "full the seconds rec index: %s\n", strName);

    s32RecIndexFd = ISIL_FM_FILE_ALLOCATE_OpenFile(strName);
    if(s32RecIndexFd < 0)
    {
        fprintf(stderr, "--full-open rec index file fail---\n");
        return -1;
    }

    /*写到该文件的哪个位置上*/
    #if 0
    int i = 0;
    for(i = stFull.u16RecSum; i < FILE_INDEX_RECLOG_SUM; i++)
    {
        int s32RecIndexOffset = ISIL_FM_FILE_ALLOCATE_GetRecIndexStartOffsetById(i+1);
        fprintf(stderr, "----full s32RecIndexOffset: %x----\n", s32RecIndexOffset);

        if(ISIL_FM_FILE_ALLOCATE_InitData(s32RecIndexFd, s32RecIndexOffset, REC_INDEX_SIZE) < 0)
        {
            fprintf(stderr, "--full--ISIL_FM_FILE_ALLOCATE_InitData fail----\n");
            return -1;
        }
    }
    #else
    int s32RecIndexOffset = ISIL_FM_FILE_ALLOCATE_GetRecIndexStartOffsetById(stFull.u16RecSum+1);
    int s32Len = REC_INDEX_SIZE*(FILE_INDEX_RECLOG_SUM - stFull.u16RecSum)*1024;

    //warn_log("----remain:%d, len:%x-----\n",(FILE_INDEX_RECLOG_SUM - stFull.u16RecSum), s32Len);

    if(ISIL_FM_FILE_ALLOCATE_InitData_1M(s32RecIndexFd, s32RecIndexOffset, s32Len) != s32Len)
    {
        fprintf(stderr, "--full--ISIL_FM_FILE_ALLOCATE_InitData_1M fail----\n");
        return -1;
    }
    #endif
#endif

    return 0;
}

/*update indexRecLog*/
/*主要在文件结束时调用,每2s也需要调用一次*/
int ISIL_FM_FILE_ALLOCATE_UpdateIndexRecLog(int s32IndexFd, unsigned int u32Channel, unsigned int u32StopOffset)
{
    int s32Ret = 0;
    int u32FileID = 0;
    //int s32RecId = 0;
    ISIL_ALLOCATE_FILE_USING_INFO    stUsing;;
    ISIL_ALLOCATE_FILE_REC_INFO      stRec;

    pthread_mutex_lock(&FILE_INDEX_LOCK);

    memset(&stUsing, 0x00, sizeof(ISIL_ALLOCATE_FILE_USING_INFO));
    s32Ret = ISIL_FM_FILE_ALLOCATE_ReadIndexUsing(s32IndexFd, u32Channel, &stUsing);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    if(stUsing.u32UsingFileID == 0xffffffff)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return 0;
    }

    u32FileID = stUsing.u32UsingFileID;

    int i = 0;
    for(i = 0; i < stUsing.u16LogSum; i++)
    {
        memset(&stRec, 0x00, sizeof(ISIL_ALLOCATE_FILE_REC_INFO));
        s32Ret = ISIL_FM_FILE_ALLOCATE_ReadIndexRec(s32IndexFd, u32FileID, i, &stRec);
        if(s32Ret < 0)
        {
            pthread_mutex_unlock(&FILE_INDEX_LOCK);
            return -1;
        }

        if((stRec.u8RecFileID != stUsing.u16RecSum) && !IS_MJPG_TYPE(stRec.u8RecType))
        {
            continue;
        }

        stRec.u32StopOffset = u32StopOffset;
        ISIL_GetSysTime(&stRec.stStopDT);
        s32Ret = ISIL_FM_FILE_ALLOCATE_WriteIndexRec(s32IndexFd, u32FileID, i, stRec);
        if(s32Ret < 0)
        {
            pthread_mutex_unlock(&FILE_INDEX_LOCK);
            return -1;
        }
    }

    pthread_mutex_unlock(&FILE_INDEX_LOCK);
    return 0;
}

/*add a rec log*/
int ISIL_FM_FILE_ALLOCATE_AddIndexRecLog(int s32IndexFd, unsigned int u32ChNum, unsigned char u8RecType, unsigned char u8SubType, unsigned int u32StartOffset)
{
    int s32Ret = 0;
    ISIL_ALLOCATE_FILE_USING_INFO stUsing;
    ISIL_ALLOCATE_FILE_REC_INFO stRecLog;

    pthread_mutex_lock(&FILE_INDEX_LOCK);

    s32Ret = ISIL_FM_FILE_ALLOCATE_ReadIndexUsing(s32IndexFd, u32ChNum, &stUsing);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    s32Ret = ISIL_FM_FILE_ALLOCATE_ReadIndexRec(s32IndexFd, stUsing.u32UsingFileID, (stUsing.u16LogSum - 1), &stRecLog);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    /*写入当前类型及时间*/
    stRecLog.u8RecType = u8RecType;
    stRecLog.u8SubType = u8SubType;
    stRecLog.u32StartOffset = u32StartOffset;
    ISIL_GetSysTime(&stRecLog.stStartDT);

    s32Ret = ISIL_FM_FILE_ALLOCATE_WriteIndexRec(s32IndexFd, stUsing.u32UsingFileID, stUsing.u16LogSum, stRecLog);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    /*update using info*/
    stUsing.u16LogSum++;
    if(stUsing.u16LogSum > FILE_INDEX_RECLOG_SUM)
    {
        fprintf(stderr, "add the rec log sum is over totle\n");
        return -1;
    }
    s32Ret = ISIL_FM_FILE_ALLOCATE_WriteIndexUsing(s32IndexFd, u32ChNum, stUsing);
    if(s32Ret < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    pthread_mutex_unlock(&FILE_INDEX_LOCK);
    return 0;
}

void ISIL_FM_FILE_ALLOCATE_LockIndex(void)
{
    pthread_mutex_lock(&FILE_INDEX_LOCK);
}

void ISIL_FM_FILE_ALLOCATE_UnlockIndex(void)
{
     pthread_mutex_unlock(&FILE_INDEX_LOCK);
}


/*获取二级索引写的起始位置,成功返回offset,失败返回0*/
int ISIL_FM_FILE_ALLOCATE_GetRecIndexStartOffset(int s32IndexFd, unsigned int u32Channel)
{
    int s32Ret = 0;
    ISIL_ALLOCATE_FILE_USING_INFO stUsing;

    pthread_mutex_lock(&FILE_INDEX_LOCK);

    if(ISIL_FM_FILE_ALLOCATE_ReadIndexUsing(s32IndexFd, u32Channel, &stUsing) < 0)
    {
        pthread_mutex_unlock(&FILE_INDEX_LOCK);
        return -1;
    }

    s32Ret = (stUsing.u16RecSum - 1) * REC_INDEX_SIZE * 1024;
    pthread_mutex_unlock(&FILE_INDEX_LOCK);

    //fprintf(stderr, "sec index rec sum:%d,size:%d,%x, ret=%x\n", stUsing.u16RecSum, REC_INDEX_SIZE*1024, REC_INDEX_SIZE*1024, s32Ret);
    return (s32Ret);
}

int ISIL_FM_FILE_ALLOCATE_GetRecIndexStartOffsetById(unsigned int u32RecIndexId)
{
    int s32Ret = 0;

    s32Ret = (u32RecIndexId - 1) * REC_INDEX_SIZE * 1024;

    //fprintf(stderr, "sec index offset=%x\n", s32Ret);
    return (s32Ret);
}


static int ISIL_FM_DISK_SaveDiskInfo(ISIL_SAVE_DISK_INFO DiskInfo)
{
    char path[64] = {};
    int dev_fd = 0;

    sprintf(path, "%sdevice.bin", HDD_REC_DIR);

    dev_fd = ISIL_FM_FILE_ALLOCATE_CreatFile(path);
    if(dev_fd < 0)
    {
        return -1;
    }

    lseek(dev_fd, 0, SEEK_SET);
    TransSaveDiskInfo(&DiskInfo);
    if(write(dev_fd, &DiskInfo, sizeof(ISIL_SAVE_DISK_INFO)) != sizeof(ISIL_SAVE_DISK_INFO))
    {
        close(dev_fd);
        return -1;
    }

    fsync(dev_fd);
    close(dev_fd);
    return 0;
}

static int ISIL_FM_DISK_GetDiskInfo(ISIL_SAVE_DISK_INFO *DiskInfo)
{
    char path[64] = {};
    int dev_fd = 0;

    sprintf(path, "%sdevice.bin", HDD_REC_DIR);

    dev_fd = ISIL_FM_FILE_ALLOCATE_OpenFile(path);
    if(dev_fd < 0)
    {
        return -1;
    }

    lseek(dev_fd, 0, SEEK_SET);

    if(read(dev_fd, DiskInfo, sizeof(ISIL_SAVE_DISK_INFO)) != sizeof(ISIL_SAVE_DISK_INFO))
    {
        close(dev_fd);
        return -1;
    }
    TransSaveDiskInfo(DiskInfo);
    close(dev_fd);
    return 0;
}

/*读取device.bin文件，主要是文件静态分配信息等*/
/*返回值:0正确,-1:参数有错,上层调用需要调用格式化函数格式化硬盘*/
int ISIL_FM_FILE_ALLOCATE_ReadAllocParam()
{
    char need_format = 0;
    int indexFd = 0;
    ISIL_ALLOCATE_FILE_BASEINFO stBaseInfo;
    char path[64] = {};
    ISIL_SAVE_DISK_INFO ReadDisk;

    memset(&ReadDisk, 0x00, sizeof(ISIL_SAVE_DISK_INFO));

    if(ISIL_FM_DISK_GetDiskInfo(&ReadDisk) < 0)
    {
        need_format = 1;
        goto FORMAT;
    }

    /*如果读取正确，比较读出的数据*/
    if(!ReadDisk.IsFormat)
    {
        need_format = 1;
        goto FORMAT;
    }

    if(ReadDisk.u32MaxRecChannel <= 0)
	{
        need_format = 1;
		fprintf(stderr, "come here read max ch fail\n");
        goto FORMAT;
    }

    if(ReadDisk.u32MaxRecChannel != ISIL_ALLOCATE_MAXCHANNEL)
	{
        need_format = 1;
		fprintf(stderr, "max channel had changed fail\n");
        goto FORMAT;
    }

    if(!ReadDisk.FILE_INDEX_RECLOG_SUM || !ReadDisk.FILE_NUM || !ReadDisk.FILE_SIZE
        || !ReadDisk.REC_INDEX_SIZE || !ReadDisk.u32RecIndexSumEachFile || !ReadDisk.u32RecIndexSize
        )
    {
        need_format = 1;
        goto FORMAT;
    }

    sprintf(path, "%s%s%s%s", HDD_REC_DIR, ISIL_INDEX_DIR, ISIL_FILE_INDEX_DIR, ISIL_FILE_INDEX_NAME);
    indexFd = ISIL_FM_FILE_ALLOCATE_OpenFile(path);
    if(indexFd < 0)
    {
        need_format = 1;
        goto FORMAT;
    }

    ISIL_FM_FILE_ALLOCATE_LockIndex();
    if(ISIL_FM_FILE_ALLOCATE_ReadIndexBase(indexFd, &stBaseInfo) < 0)
    {
        need_format = 1;
        goto FORMAT;
    }
    ISIL_FM_FILE_ALLOCATE_UnlockIndex();

    /*比较相关参数是否一致*/
    if((stBaseInfo.u32FileTotle != ReadDisk.FILE_NUM)
       ||(stBaseInfo.u32RecIndexSumEachFile != ReadDisk.u32RecIndexSumEachFile)
       ||(stBaseInfo.u32RecIndexSize != ReadDisk.u32RecIndexSize)
    )
    {
        need_format = 1;
        goto FORMAT;
    }

FORMAT:
    if(!need_format)
    {
        FILE_INDEX_RECLOG_SUM = ReadDisk.FILE_INDEX_RECLOG_SUM;
        FILE_NUM = ReadDisk.FILE_NUM;
        FILE_SIZE = ReadDisk.FILE_SIZE;
        REC_INDEX_SIZE = ReadDisk.REC_INDEX_SIZE;
        ISIL_FM_DISK_OVER_WRITE = ReadDisk.ISIL_FM_DISK_OVER_WRITE;
        u32RecIndexSize = ReadDisk.u32RecIndexSize;

        FILE_INDEX_REC_OFFSET = FILE_INDEX_FULL_OFFSET + FILE_NUM*sizeof(ISIL_ALLOCATE_FILE_FULL_INFO);
        return 0;
    }
    else
    {
        return -1;
    }
}

int ISIL_FM_FILE_ALLOCATE_GetRecIndexSize(void)
{
    return u32RecIndexSize;
}


static int ISIL_FM_FILE_ALLOCATE_SaveOverWrite(unsigned int u32OverWrite)
{
    ISIL_SAVE_DISK_INFO DiskInfo;

    if(ISIL_FM_DISK_GetDiskInfo(&DiskInfo) < 0)
    {
        fprintf(stderr, "ISIL_FM_DISK_GetDiskInfo fail\n");
        return -1;
    }

    DiskInfo.ISIL_FM_DISK_OVER_WRITE = u32OverWrite;

    if(ISIL_FM_DISK_SaveDiskInfo(DiskInfo) < 0)
    {
        fprintf(stderr, "ISIL_FM_DISK_SaveDiskInfo fail\n");
        return -1;
    }

    return 0;
}

int ISIL_FM_FILE_ALLOCATE_SetNodePath(char *NodePath)
{
    if(NodePath == NULL)
    {
        fprintf(stderr, "---NodePath is null---\n");
        return -1;
    }

    memset(HDD_REC_DIR, 0x00, REC_ROOT_DIR_MAX_LEN);
    memcpy(HDD_REC_DIR, NodePath, REC_ROOT_DIR_MAX_LEN);
    sprintf(HDD_REC_DIR, "%s/", HDD_REC_DIR);

    fprintf(stderr, "---NodePath: %s---\n", HDD_REC_DIR);
    return 0;
}

int ISIL_FILE_ALLOCATE_OpenMidx(unsigned int u32MidxFileID)
{
    char path[64];

    memset(path, 0x00, 64);
    sprintf(path, "%s%s%s%d.bin", HDD_REC_DIR, ISIL_INDEX_DIR, ISIL_MJPG_INDEX_DIR, u32MidxFileID);
    return ISIL_FM_FILE_ALLOCATE_OpenFile(path);
}

int ISIL_FM_FILE_ALLOCATE_ReadMIdxHeader(int s32MidxFd, ISIL_MJPG_INDEX_HEADER *pMidxHeader)
{
    lseek(s32MidxFd, 0, SEEK_SET);

    if(read_r(s32MidxFd, pMidxHeader, sizeof(ISIL_MJPG_INDEX_HEADER)) < 0){
        fprintf(stderr, "%s, %d, read fail\n", __FUNCTION__, __LINE__);
        return -1;
    }

    TransMidxHeader(pMidxHeader);
    return 0;
}

int ISIL_FM_FILE_ALLOCATE_WriteMIdxHeader(int s32MidxFd, ISIL_MJPG_INDEX_HEADER *pMidxHeader)
{
    TransMidxHeader(pMidxHeader);
    lseek(s32MidxFd, 0, SEEK_SET);

    if(write_r(s32MidxFd, pMidxHeader, sizeof(ISIL_MJPG_INDEX_HEADER)) < 0){
        fprintf(stderr, "%s, %d, write fail\n", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

int ISIL_FM_FILE_ALLOCATE_ReadMIdx(int s32MidxFd, unsigned int u32MidxNum, ISIL_MJPG_INDEX *pMidx)
{
    unsigned int off = 0;

    off = sizeof(ISIL_MJPG_INDEX_HEADER) + (u32MidxNum - 1)*sizeof(ISIL_MJPG_INDEX);
    fprintf(stderr, "%s, %d, off:%d\n", __FUNCTION__, __LINE__, off);
    lseek(s32MidxFd, off, SEEK_SET);

    if(read_r(s32MidxFd, pMidx, sizeof(ISIL_MJPG_INDEX)) < 0){
        fprintf(stderr, "%s, %d, read fail\n", __FUNCTION__, __LINE__);
        return -1;
    }

    TransMidx(pMidx);
    return 0;
}

int ISIL_FM_FILE_ALLOCATE_WriteMIdx(int s32MidxFd, unsigned int u32MidxNum, ISIL_MJPG_INDEX *pMidx)
{
    unsigned int off = 0;
    TransMidx(pMidx);
    off = sizeof(ISIL_MJPG_INDEX_HEADER) + (u32MidxNum - 1)*sizeof(ISIL_MJPG_INDEX);
    fprintf(stderr, "%s, %d, off:%d\n", __FUNCTION__, __LINE__, off);
    lseek(s32MidxFd, off, SEEK_SET);

    if(write_r(s32MidxFd, pMidx, sizeof(ISIL_MJPG_INDEX)) < 0){
        fprintf(stderr, "%s, %d, read fail\n", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;

}

//check whether mjpg file
int IS_MJPG_TYPE(unsigned char type)
{
    if(type&MJPG_TYPE){
        return 1;
    }else{
        return 0;
    }
}


