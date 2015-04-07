#ifndef __ISIL_FM_REC_H__
#define __ISIL_FM_REC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h" 
#include "isil_fm_h264.h"

/*max video channel*/
#define MAX_REC_CHANNEL CHANNEL_NUMBER //(1)

/*related buff size*/
#define GOP_NAL_BUFF_SIZE  (500*12+16)

//index buff size
#define FILE_INDEX_BUFF_SIZE  (sizeof(ISIL_FILE_INDEX_NAL) + I_FRAME_INTERVAL * sizeof(ISIL_GOP_INDEX))

/*定义I帧间隔*/
#define I_FRAME_INTERVAL    (512)



enum
{
	REC_LOG_TYPE_INVALID		= 0x00,
	REC_LOG_TYPE_ALARM_REC	    = 0x01,
	REC_LOG_TYPE_TIMING_REC	    = 0x02,
	REC_LOG_TYPE_MANUAL_REC	    = 0x04,
	REC_LOG_TYPE_ALARM_SHOT	    = 0x08,
	REC_LOG_TYPE_TIMING_SHOT	= 0x10,
	REC_LOG_TYPE_MANUAL_SHOT	= 0x20,
	REC_LOG_TYPE_AUTO_SHOT	= 0x40,
}ISIL_REC_LOG_MAIN_TYPE;
#define MJPG_TYPE (REC_LOG_TYPE_ALARM_SHOT|REC_LOG_TYPE_TIMING_SHOT|REC_LOG_TYPE_MANUAL_SHOT)

/*文件记录子类型*/
enum
{
	REC_LOG_SUB_TYPE_INVALID		= 0x00,
	REC_LOG_SUB_TYPE_VIDEO_LOST	    = 0x01,
	REC_LOG_SUB_TYPE_MOTION_DETECT	= 0x02,
	REC_LOG_SUB_TYPE_VIDEO_MASK	    = 0x04,
	REC_LOG_SUB_TYPE_VIDEO_SHELTER  = 0x08
}ISIL_REC_LOG_SUB_TYPE;

typedef struct
{
	int				    write_fd;//写H264文件fd
	unsigned char       have_gop_nal;
	unsigned short  	gop_count;
	char				reserved;
	unsigned int	    last_gop_nal_size;//上一个GOP NAL Size
	unsigned int	    last_gop_offset;//上一个GOP NAL Offset
	unsigned int		current_offset;//目前nal在文件中的offset
	unsigned char		gop_nal_buff[GOP_NAL_BUFF_SIZE];//GOP NAL buff
	unsigned int		gop_nal_buff_len;//gop nal len
}stGOPNal;

typedef struct
{
	unsigned char		have_file_tail_nal;
	char				reserved[3];
	ISIL_FILE_TAIL_NAL	file_tail_nal;
}stFileTailNal;

typedef struct
{
	int				    write_file_fd;
	unsigned char		have_file_index_nal;
	unsigned int		IFrame_count_in_gop;
	unsigned char		file_index_nal_buff[FILE_INDEX_BUFF_SIZE];
	unsigned int		file_index_nal_buff_len;
    int                 s32IndexOffset;
	unsigned short		u16IndexGopCnt;
}stFileIdxNal;

typedef struct
{
	unsigned int		channel;
	unsigned char		start_write;//start write
	char 				h264vsync;//whether discard video frame or not,the writed first v frame must be IDR
	unsigned char	    u8MidxMainType;
	unsigned char	    u8MidxSubType;
	stGOPNal			GopNal;
	stFileTailNal		FileTailNal;
	stFileIdxNal		FileIdxNal;
	unsigned long		start_time;//start timestamp
    int                 s32FileIndexFd;
    unsigned int        s32RecOffset;
	unsigned int		nalcnt;//nal totle count in a rec file
	int                 mjpg_idx_fd;//mjpg index fd
	unsigned int 		KeyFrameCnt;
}stH264File;


typedef enum{
	recstate_invalid = 0x00,
	recstate_init 	 = 0x01,
	recstate_start   = 0x02,
	recstate_write   = 0x04,
	recstate_close   = 0x08,
	recstate_cap   	 = 0x10,
	recstate_stopcap = 0x20,
}recstate;

typedef struct isil_rec_mgt_t{
	unsigned int	 chan;//rec channel, start from 0 
	unsigned int     main_type;//rec main type
	unsigned int	 sub_type;//rec sub type
	//unsigned int     mjpg_main_type;//rec main type
	//unsigned int	 mjpg_sub_type;//rec sub type
	stH264File 		 h264file;
	recstate		 state;
}isil_rec_mgt;


/*h264 api declaration*/

//init, get isil_rec_mgt*
extern isil_rec_mgt* ISIL_FM_REC_Init(unsigned int chan);
//open
extern int ISIL_FM_REC_OpenFile(isil_rec_mgt *rec_mgt, unsigned int MainType, unsigned int SubType);
//write
extern int ISIL_FM_REC_WriteFile(stDriverFrame *driver_data, isil_rec_mgt *rec_mgt);
//sync
extern int ISIL_FM_REC_SyncFile(isil_rec_mgt *rec_mgt);
//close
extern int ISIL_FM_REC_CloseFile(isil_rec_mgt *rec_mgt);






#ifdef __cplusplus
}
#endif

#endif

