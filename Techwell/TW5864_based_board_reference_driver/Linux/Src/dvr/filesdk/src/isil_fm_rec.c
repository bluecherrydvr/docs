#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <netinet/in.h>
#include <pthread.h>
#include <strings.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "isil_fm_rec.h"
#include "isil_fm_utils.h"
#include "isil_fm_file_allocate.h"
#include "isil_fm_utils.h"


#define REC_DEBUG_ENABLE
#ifdef REC_DEBUG_ENABLE
#define ISIL_REC_DEBUG(format, args...) \
    do{ \
        fprintf(stderr, "%s, %d, ", __FUNCTION__, __LINE__);\
        fprintf(stderr, format, ## args); \
        fprintf(stderr, "\n"); \
    }while(0)
#else
	#define ISIL_REC_DEBUG(format, args...)
#endif

static isil_rec_mgt REC_MGT_TBL[MAX_REC_CHANNEL];


int ISIL_FM_REC_OpenFile(isil_rec_mgt *rec_mgt, unsigned int MainType, unsigned int SubType)
{
	int s32Ret = 0;
    int s32FileIndexFd = 0;
    int s32DataFd = 0;
    int s32RecIndexFd = 0;
    int s32DataFileID = 0;
    char strPath[64];
    int s32RecIndexID = 0;
    unsigned int s32RecOffset = 0;
    stH264File *h264_file;
	int s32Midxfd = 0;
	int channelnum = 0;

	if(!rec_mgt){
		return -1;
	}

	channelnum = rec_mgt->chan;

    if(channelnum > MAX_REC_CHANNEL){
        ISIL_REC_DEBUG("chan input error");
    }

	rec_mgt->main_type |= MainType;
	rec_mgt->sub_type |= SubType;

    if(rec_mgt->state == recstate_write){//add index
    	ISIL_FM_FILE_ALLOCATE_AddIndexRecLog(rec_mgt->h264file.s32FileIndexFd, rec_mgt->chan,\
    															MainType, SubType,\
    		                                                    rec_mgt->h264file.s32RecOffset);
    }else if(rec_mgt->state == recstate_init){

		h264_file = &(rec_mgt->h264file);
		h264_file->channel = channelnum;
	
		h264_file->nalcnt = 1;
	
		if(IS_MJPG_TYPE(MainType)){
			fprintf(stderr, "-----is mjpg-------\n");
			h264_file->u8MidxMainType = MainType;
			h264_file->u8MidxSubType  = SubType;
			//ISIL_GetSysTime(&h264_file->MidxDT);
		}
	
		//fprintf(stderr, "-----is mjpg-after------\n");
	

		sprintf(strPath, "%s%s%s%s", HDD_REC_DIR, ISIL_INDEX_DIR, ISIL_FILE_INDEX_DIR, ISIL_FILE_INDEX_NAME);
		s32FileIndexFd = ISIL_FM_FILE_ALLOCATE_OpenFile(strPath);
		if(s32FileIndexFd < 0)
		{
			fprintf(stderr, "ISIL_FM_FILE_ALLOCATE_OpenFile fail\n");
			return -1;
		}
		h264_file->s32FileIndexFd = s32FileIndexFd;
	
		s32DataFileID = ISIL_FM_FILE_ALLOCATE_GetDataFile(s32FileIndexFd, channelnum);
		if((s32DataFileID == -1) || (s32DataFileID == -2))
		{
			return s32DataFileID;
		}
	
		if(s32DataFileID & 0x80000000)
		{
			ISIL_REC_DEBUG("--------get a not full file------\n");
			s32DataFileID = s32DataFileID & 0x7fffffff;
	
			ISIL_ALLOCATE_FILE_USING_INFO stUsingInfo;
			int i = 0;
			memset(&stUsingInfo, 0x00, sizeof(ISIL_ALLOCATE_FILE_USING_INFO));
	
			ISIL_FM_FILE_ALLOCATE_LockIndex();

			if(ISIL_FM_FILE_ALLOCATE_ReadIndexUsing(s32FileIndexFd, channelnum, &stUsingInfo) < 0)
			{
				ISIL_REC_DEBUG("ISIL_FM_FILE_ALLOCATE_ReadIndexUsing fail\n");
				ISIL_FM_FILE_ALLOCATE_UnlockIndex();
				return -1;
			}

			ISIL_ALLOCATE_FILE_REC_INFO stRecLog;
			for(i = 0; i < stUsingInfo.u16LogSum; i++)
			{
				memset(&stRecLog, 0x00, sizeof(ISIL_ALLOCATE_FILE_REC_INFO));
				if(ISIL_FM_FILE_ALLOCATE_ReadIndexRec(s32FileIndexFd, s32DataFileID, i, &stRecLog) < 0)
				{
					return -1;
				}
	
				if(s32RecOffset < stRecLog.u32StopOffset)
				{
					s32RecOffset = stRecLog.u32StopOffset;
				}
			}
			ISIL_FM_FILE_ALLOCATE_UnlockIndex();
		}
	
		h264_file->s32RecOffset = s32RecOffset;
		ISIL_REC_DEBUG("--------h264_file->s32RecOffset: %x------\n", h264_file->s32RecOffset);
	
		sprintf(strPath, "%s%sisil%d.264", HDD_REC_DIR, ISIL_DATA_DIR, s32DataFileID);
		s32DataFd = ISIL_FM_FILE_ALLOCATE_OpenFile(strPath);
		if(s32DataFd < 0)
		{
			ISIL_REC_DEBUG("----open data file fail----\n");
			return -1;
		}

		h264_file->GopNal.write_fd = s32DataFd;
	
		s32Midxfd = ISIL_FILE_ALLOCATE_OpenMidx(s32DataFileID);
		if(s32Midxfd < 0){
			ISIL_REC_DEBUG("----open Midx fail----\n");
			return -1;
		}
	
		h264_file->mjpg_idx_fd = s32Midxfd;
	
		//s32RecIndexID = ISIL_FM_FILE_ALLOCATE_GetRecIndexFile(s32FileIndexFd);
		s32RecIndexID = s32DataFileID;
		if(s32RecIndexID < 0)
		{
			return -1;
		}
	
		sprintf(strPath, "%s%s%s%d.bin", HDD_REC_DIR, ISIL_INDEX_DIR, ISIL_REC_INDEX_DIR, s32RecIndexID);
		s32RecIndexFd = ISIL_FM_FILE_ALLOCATE_OpenFile(strPath);
		printf("%s Line %d open recindexfd %d, file [%d]\n", __FUNCTION__, __LINE__, s32RecIndexFd, s32RecIndexID);
		if(s32RecIndexFd < 0)
		{
			return -1;
		}
		h264_file->FileIdxNal.write_file_fd = s32RecIndexFd;
		h264_file->FileIdxNal.s32IndexOffset = sizeof(ISIL_FILE_INDEX_NAL);
	
		/*file index*/
		ISIL_ALLOCATE_FILE_REC_INFO stRec;
		memset(&stRec, 0x00, sizeof(ISIL_ALLOCATE_FILE_REC_INFO));
	
		//fprintf(stderr, "input rectype: %x, ch: %x\n", u16RecType, channelnum);
		stRec.u8RecType = MainType;
		stRec.u8SubType = SubType;
		stRec.u8Channel = channelnum;
		ISIL_GetSysTime(&stRec.stStartDT);
		//stRec.u32RecIndexId = s32RecIndexID;
		stRec.u32StartOffset = h264_file->s32RecOffset;
		stRec.u32StopOffset  = stRec.u32StartOffset;
	
		//fprintf(stderr, "write type:%x: %x, ch: %x\n", stRec.u8RecType, stRec.u8SubType, stRec.s16Channel);
		s32Ret = ISIL_FM_FILE_ALLOCATE_UpdateFileIndex(s32FileIndexFd, s32DataFileID, stRec);
		if(s32Ret < 0)
		{
			return -1;
		}

		rec_mgt->state = recstate_start;
	}

	return 0;
}


static void ISIL_FM_REC_SetFileGopFlag(stH264File *h264_file, unsigned char flag)
{
	h264_file->GopNal.have_gop_nal = flag;
}

static int ISIL_FM_REC_GetFileGopFlag(stH264File *h264_file)
{
	return (h264_file->GopNal.have_gop_nal);
}

static void ISIL_FM_REC_SetFileIndexFlag(stH264File *h264_file, unsigned char flag)
{
	h264_file->FileIdxNal.have_file_index_nal = flag;
}

static int ISIL_FM_REC_GetFileIndexFlag(stH264File *h264_file)
{
	return(h264_file->FileIdxNal.have_file_index_nal);
}

static void ISIL_FM_REC_SetFileTailFlag(unsigned int chan, unsigned char flag)
{
    stH264File *h264_file = &(REC_MGT_TBL[chan].h264file);
	h264_file->FileTailNal.have_file_tail_nal = flag;
}

static int  ISIL_FM_REC_GetFileTailFlag(stH264File *h264_file)
{
	return(h264_file->FileTailNal.have_file_tail_nal);
}



static int ISIL_FM_REC_WriteTail(stH264File *h264_file)
{
    int ret = 0;
    #ifdef BIG_BYTES_ORDER
    unsigned int tmp_val = 0;
	//unsigned short int tmp_val_2B = 0;
    #endif

    h264_file->KeyFrameCnt = 0;

	#ifdef BIG_BYTES_ORDER
	//fprintf(stderr, "......................................write tail start................................\n");
	tmp_val = h264_file->FileTailNal.file_tail_nal.lIndexSize;
	swap_three_bytes(&tmp_val);
	h264_file->FileTailNal.file_tail_nal.lIndexSize = tmp_val;

	tmp_val = h264_file->FileTailNal.file_tail_nal.lIndexOffset;
	swap_four_bytes(&tmp_val);
	h264_file->FileTailNal.file_tail_nal.lIndexOffset = tmp_val;

	tmp_val = h264_file->FileTailNal.file_tail_nal.lAllNalCount;
	swap_four_bytes(&tmp_val);
	h264_file->FileTailNal.file_tail_nal.lAllNalCount = tmp_val;

	tmp_val = h264_file->FileTailNal.file_tail_nal.lTimeBegin;
	swap_four_bytes(&tmp_val);
	h264_file->FileTailNal.file_tail_nal.lTimeBegin = tmp_val;

	tmp_val = h264_file->FileTailNal.file_tail_nal.lTimeEnd;
	swap_four_bytes(&tmp_val);
	h264_file->FileTailNal.file_tail_nal.lTimeEnd = tmp_val;

	tmp_val = h264_file->FileTailNal.file_tail_nal.lMaxNalSize;
	swap_four_bytes(&tmp_val);
	h264_file->FileTailNal.file_tail_nal.lMaxNalSize = tmp_val;

	tmp_val = h264_file->FileTailNal.file_tail_nal.lMaxGopNalSize;
	swap_four_bytes(&tmp_val);
	h264_file->FileTailNal.file_tail_nal.lMaxGopNalSize = tmp_val;

	//fprintf(stderr, "before write file tail lMaxKeyFrameInterval = %x\n", h264_file->FileTailNal.file_tail_nal.lMaxKeyFrameInterval);
	tmp_val = h264_file->FileTailNal.file_tail_nal.lMaxKeyFrameInterval;
	swap_four_bytes(&tmp_val);
	h264_file->FileTailNal.file_tail_nal.lMaxKeyFrameInterval = tmp_val;
	//fprintf(stderr, "after write file tail lMaxKeyFrameInterval = %x\n", h264_file->FileTailNal.file_tail_nal.lMaxKeyFrameInterval);
	//fprintf(stderr, "......................................write tail end................................\n");
	#endif

	ret = lseek(h264_file->GopNal.write_fd, h264_file->s32RecOffset, SEEK_SET);
    if(ret < 0){
        perror("Seek:");
		return -1;
    }
	ret = write_r(h264_file->GopNal.write_fd, (unsigned char *)&(h264_file->FileTailNal.file_tail_nal), sizeof(h264_file->FileTailNal.file_tail_nal));
	if(ret < 0)
	{
		fprintf(stderr, "write file tail fail\n");
		return -1;
	}
    h264_file->s32RecOffset += sizeof(h264_file->FileTailNal.file_tail_nal);
	ISIL_FM_REC_SetFileTailFlag(h264_file->channel, 0);
    return 0;
}


static int ISIL_FM_REC_WriteMIdx(stH264File* h264_file, stDriverFrame *driver_data)
{
	//unsigned int offset = 0;
	ISIL_MJPG_INDEX_HEADER stMidxHeader;
	ISIL_MJPG_INDEX stMidx;
	ISIL_MJPG_INDEX stLastMidx;

	if(!h264_file || !driver_data){
		return -1;
	}

	fprintf(stderr, " enter write Midx \n");

	memset(&stLastMidx, 0x00, sizeof(ISIL_MJPG_INDEX));
	memset(&stMidxHeader, 0x00, sizeof(ISIL_MJPG_INDEX_HEADER));
	if(ISIL_FM_FILE_ALLOCATE_ReadMIdxHeader(h264_file->mjpg_idx_fd, &stMidxHeader) < 0){
		return -1;
	}

	if(stMidxHeader.u32IndexCurNum){
		if(ISIL_FM_FILE_ALLOCATE_ReadMIdx(h264_file->mjpg_idx_fd, stMidxHeader.u32IndexCurNum, &stLastMidx) < 0){
			return -1;
		}
	}

	stMidxHeader.u32Channel =  h264_file->channel;
	stMidxHeader.u32CurSize += sizeof(ISIL_MJPG_INDEX);
	stMidxHeader.u32IndexCurNum++;

	/*update mjpg index content*/
	memset(&stMidx, 0x00, sizeof(ISIL_MJPG_INDEX));
	ISIL_GetSysTime(&stMidx.stDT);

	if(!memcmp(&stLastMidx.stDT, &stMidx.stDT, sizeof(S_REC_DATE_TIME)-1)){
		fprintf(stderr, " Midx DT same \n");
        stMidx.stDT.Time.Reserve = stLastMidx.stDT.Time.Reserve;
		stMidx.stDT.Time.Reserve++;
	}

	stMidx.u8RecType = h264_file->u8MidxMainType;
	stMidx.u8SubType = h264_file->u8MidxSubType;
	stMidx.u32offset = h264_file->s32RecOffset;
	stMidx.u32Len    = driver_data->len;

	if(ISIL_FM_FILE_ALLOCATE_WriteMIdx(h264_file->mjpg_idx_fd, stMidxHeader.u32IndexCurNum, &stMidx) < 0){
		return -1;
	}

	/*update mjpg index header*/
	if(ISIL_FM_FILE_ALLOCATE_WriteMIdxHeader(h264_file->mjpg_idx_fd, &stMidxHeader) < 0){
		return -1;
	}

	return 0;
}

static int ISIL_FM_SRV_Write_RemainSpace(int data_fd, unsigned int offset_start, unsigned int data_len)
{
    char buff[1024] = {};
    int loop = data_len/1024;
    int i = 0;
    unsigned int offset_cur = offset_start;
    unsigned int remain = data_len - loop*1024;
    int ret = 0;

    //fprintf(stderr, "cur:%x,re:%x, loop:%x\n", offset_cur, remain, loop);

    if(loop >0 )
    {
        for(i = 0; i < loop; i++)
        {
            if(lseek(data_fd, offset_cur, SEEK_SET) != offset_cur)
            {
                fprintf(stderr, "%s,%d fail\n", __FUNCTION__, __LINE__);
                return -1;
            }

            ret = write(data_fd, buff, 1024);
            if(ret != 1024)
            {
                fprintf(stderr, "%s,%d fail\n", __FUNCTION__, __LINE__);
                return -1;
            }
            offset_cur += ret;
        }
    }

    if(remain > 0)
    {
        //fprintf(stderr, "---offset_cur: %x, %x---\n", offset_cur, remain);
        if(lseek(data_fd, offset_cur, SEEK_SET) != offset_cur)
        {
            fprintf(stderr, "%s,%d fail\n", __FUNCTION__, __LINE__);
            return -1;
        }

        ret = write(data_fd, buff, remain);
        if(ret != remain)
        {
            perror("why: ");
            fprintf(stderr, "%s,%d fail\n", __FUNCTION__, __LINE__);
            return -1;
        }
        offset_cur += ret;
    }

    return 0;
}

static int ISIL_FM_CheckRecIndexFileIsEnd(int curlen)
{
   if(ISIL_FM_FILE_ALLOCATE_GetRecIndexSize() > (curlen + 1024)){
       return 0;
   }else{
       return 1;
   }
}

static int ISIL_FM_REC_Write(stDriverFrame *driver_data, isil_rec_mgt *rec_mgt)
{
#ifdef BIG_BYTES_ORDER
    unsigned int tmp_val = 0;
	unsigned short int tmp_val_2B = 0;
#endif
	int ret = 0;
    int s32Offset = 0;
    stH264File* h264_file = &(rec_mgt->h264file);
	ISIL_GOP_NAL *gop_nal_header = (ISIL_GOP_NAL *)h264_file->GopNal.gop_nal_buff;
	int gop_nal_header_size = sizeof(ISIL_GOP_NAL);
	int nal_index_size = sizeof(ISIL_NAL_INDEX);
	ISIL_NAL_INDEX *nal_index = NULL;
	unsigned int nal_type = E_H264_FRAME_TYPE_NULL;
	ISIL_FILE_INDEX_NAL *file_index_nal = (ISIL_FILE_INDEX_NAL *)h264_file->FileIdxNal.file_index_nal_buff;
	int file_index_nal_len = sizeof(ISIL_FILE_INDEX_NAL);
	int file_gop_index_len = sizeof(ISIL_GOP_INDEX);
	ISIL_GOP_INDEX *file_gop_index = (ISIL_GOP_INDEX *)(h264_file->FileIdxNal.file_index_nal_buff + file_index_nal_len + h264_file->FileIdxNal.u16IndexGopCnt * file_gop_index_len);
	
	if((rec_mgt->state != recstate_start) && (rec_mgt->state != recstate_write)){
		return -1;
	}

	/*process mjpg frame alonely*/
	if(driver_data){
		if(driver_data->len && (driver_data->type == E_H264_MJPEG_FRAME_TYPE)){

			ISIL_REC_DEBUG(" process mjpg frame\n");
			ret = lseek(h264_file->GopNal.write_fd, h264_file->s32RecOffset, SEEK_SET);
			if(ret < 0){
				perror("Seek:");
				return -1;
			}

			ret = write_r(h264_file->GopNal.write_fd, driver_data->payload, driver_data->len);
			if(ret < 0)
			{
				ISIL_REC_DEBUG( "write mjpg data fail\n");
				return -1;
			}

			/*write mjpg index*/
			if(ISIL_FM_REC_WriteMIdx(h264_file, driver_data) < 0){
				return -1;
			}

			h264_file->s32RecOffset += driver_data->len;
			ISIL_FM_FILE_ALLOCATE_UpdateIndexRecLog(h264_file->s32FileIndexFd, h264_file->channel, h264_file->s32RecOffset);
			return 0;
		}

		/*other frame, audio, video*/

		if(!h264_file->h264vsync && ((driver_data->type > E_FRAME_TYPE_MAIN)
		   && (driver_data->type < E_FRAME_TYPE_SUB_END))){//h264 video frame not find IDR, can't write into file

			if((driver_data->type != E_H264_FRAME_TYPE_MAIN_SPS)
			   && (driver_data->type != E_H264_FRAME_TYPE_SUB_SPS)){
				fprintf(stderr, " rec file not find video SPS\n");
				return 0;
			}else{
				h264_file->h264vsync = 1;
			}
		}

		if(driver_data->len > h264_file->FileTailNal.file_tail_nal.lMaxNalSize)
		{
			h264_file->FileTailNal.file_tail_nal.lMaxNalSize = driver_data->len;
		}


		if(driver_data->len && h264_file->start_write)
		{
			if( (driver_data->type != E_FRAME_TYPE_MJPG)//exclude mjpg timestamp
			   && ((driver_data->type != E_H264_FRAME_TYPE_MAIN_SPS) && (driver_data->type != E_H264_FRAME_TYPE_MAIN_PPS))//main stream
			   && ((driver_data->type != E_H264_FRAME_TYPE_SUB_SPS) && (driver_data->type != E_H264_FRAME_TYPE_SUB_PPS)))//sub stream
			{
				if(driver_data->timestamp >= (h264_file->start_time + 1000*h264_file->nalcnt))
				{
					ISIL_FM_REC_SetFileGopFlag(h264_file, 1);
					h264_file->nalcnt++;
				}
				else
				{
					if(h264_file->start_time >= (driver_data->timestamp + 1000*h264_file->nalcnt))
					{
						ISIL_FM_REC_SetFileGopFlag(h264_file, 1);
						h264_file->nalcnt++;
					}
				}
			}
		}
	}

	if(ISIL_FM_REC_GetFileTailFlag(h264_file) == 1)
	{
		ISIL_FM_REC_SetFileGopFlag(h264_file, 1);
	}

    if(ISIL_FM_REC_GetFileGopFlag(h264_file))
    {
        ISIL_FM_REC_SetFileIndexFlag(h264_file, 1);
    }

    if(driver_data){
		if(driver_data->len)//frame parsing
		{
			if(!h264_file->start_write)//first frame
			{
				h264_file->start_write = 1;
				h264_file->FileTailNal.file_tail_nal.lTimeBegin = driver_data->timestamp;
				h264_file->start_time = driver_data->timestamp;
			}

			h264_file->FileTailNal.file_tail_nal.lTimeEnd = driver_data->timestamp;

			//fprintf(stderr, "line %d--seek: len: %x ,fd[%d]--\n", __LINE__, driver_data->len ,h264_file->GopNal.write_fd);

			ret = lseek(h264_file->GopNal.write_fd, h264_file->s32RecOffset, SEEK_SET);
			if(ret < 0){
				ISIL_REC_DEBUG("lseek fail");
				perror("Seek:");
				return -1;
			}
			//fprintf(stderr, "--seek: %x, len: %x--\n", ret, driver_data->len);
			ret = write_r(h264_file->GopNal.write_fd, driver_data->payload, driver_data->len);
			if(ret < 0)
			{
				fprintf(stderr, "write video data fail\n");
				return -1;
			}

			h264_file->s32RecOffset += driver_data->len;
			nal_index = (ISIL_NAL_INDEX *)(h264_file->GopNal.gop_nal_buff + gop_nal_header_size + gop_nal_header->nNalCount*nal_index_size);


			gop_nal_header->nNalCount++;

			h264_file->KeyFrameCnt++;
			nal_type = driver_data->type;

			if((nal_type == E_H264_FRAME_TYPE_MAIN_IDR) || (nal_type == E_H264_FRAME_TYPE_SUB_IDR))
			{
				h264_file->FileIdxNal.IFrame_count_in_gop++;

				if(h264_file->FileTailNal.file_tail_nal.lMaxKeyFrameInterval < h264_file->KeyFrameCnt)
				{
					h264_file->FileTailNal.file_tail_nal.lMaxKeyFrameInterval = h264_file->KeyFrameCnt;
				}
				h264_file->KeyFrameCnt = 0;
			}

			nal_index->cNalType = nal_type;
			nal_index->lNalSize  = driver_data->len;
			nal_index->lTimeStamp = driver_data->timestamp;
			nal_index->lNalOffset = h264_file->GopNal.current_offset;
			h264_file->GopNal.current_offset += nal_index->lNalSize;

			//fprintf(stderr, "---video time: %x,len:%x----\n", driver_data->video_timestamp, driver_data->video_len);

			#ifdef BIG_BYTES_ORDER
			tmp_val = nal_index->lNalSize;
			swap_three_bytes(&tmp_val);
			nal_index->lNalSize = tmp_val;

			tmp_val = nal_index->lNalOffset;
			swap_four_bytes(&tmp_val);
			nal_index->lNalOffset = tmp_val;

			tmp_val = nal_index->lTimeStamp;
			swap_four_bytes(&tmp_val);
			nal_index->lTimeStamp = tmp_val;
			#endif
		}
    }


	/*write GOP NAL*/
	if(ISIL_FM_REC_GetFileGopFlag(h264_file))
	{

		gop_nal_header->lGopFlag = htonl(0x00000001);
		gop_nal_header->cGopFlag = 0x78;
		gop_nal_header->nGopID = h264_file->GopNal.gop_count;
		gop_nal_header->lLastGopSize = h264_file->GopNal.last_gop_nal_size;
		gop_nal_header->lLastGopOffset =  h264_file->GopNal.last_gop_offset;
		h264_file->GopNal.gop_nal_buff_len = gop_nal_header_size + gop_nal_header->nNalCount * nal_index_size;//GOP size

		#ifdef BIG_BYTES_ORDER
		tmp_val = gop_nal_header->lLastGopSize;
		swap_three_bytes(&tmp_val);
		gop_nal_header->lLastGopSize = tmp_val;

		tmp_val = gop_nal_header->lLastGopOffset;
		swap_four_bytes(&tmp_val);
		gop_nal_header->lLastGopOffset = tmp_val;

		tmp_val_2B = gop_nal_header->nGopID;
		swap_two_bytes(&tmp_val_2B);
		gop_nal_header->nGopID = tmp_val_2B;

		tmp_val_2B = gop_nal_header->nNalCount;
		swap_two_bytes(&tmp_val_2B);
		gop_nal_header->nNalCount = tmp_val_2B;
		#endif
		//fprintf(stderr, "line %d--seek: fd[%d]--\n", __LINE__, h264_file->GopNal.write_fd);
        ret = lseek(h264_file->GopNal.write_fd, h264_file->s32RecOffset, SEEK_SET);
        if(ret < 0){
            perror("Seek:");
			return -1;
        }


		ret = write_r(h264_file->GopNal.write_fd, h264_file->GopNal.gop_nal_buff, h264_file->GopNal.gop_nal_buff_len);
		if(ret < 0)
		{
			ISIL_REC_DEBUG("write video data fail\n");
			return -1;
		}
        h264_file->s32RecOffset += h264_file->GopNal.gop_nal_buff_len;
		#ifdef BIG_BYTES_ORDER
		tmp_val_2B = gop_nal_header->nNalCount;
		swap_two_bytes(&tmp_val_2B);
		gop_nal_header->nNalCount = tmp_val_2B;
		#endif

		//h264_file->FileIdxNal.file_gop_count++;

		file_index_nal->lIndexFlag = htonl(0x00000001);
		file_index_nal->cIndexFlag = 0x79;

		#ifdef BIG_BYTES_ORDER
		tmp_val = file_index_nal->lGopCount;
		swap_three_bytes(&tmp_val);
		file_index_nal->lGopCount = tmp_val;
		#endif

		file_index_nal->lGopCount++;

		/*clean*/
		ISIL_FM_REC_SetFileGopFlag(h264_file, 0);
		h264_file->GopNal.last_gop_nal_size = h264_file->GopNal.gop_nal_buff_len;
		h264_file->GopNal.last_gop_offset = h264_file->GopNal.current_offset;
		h264_file->GopNal.current_offset += h264_file->GopNal.gop_nal_buff_len;


		file_gop_index ->lGopOffset = h264_file->GopNal.last_gop_offset;
		file_gop_index->nGopSize = h264_file->GopNal.gop_nal_buff_len;
		file_gop_index->nIFrameCount = h264_file->FileIdxNal.IFrame_count_in_gop;
        h264_file->FileIdxNal.u16IndexGopCnt++;

        //fprintf(stderr, "rec: offset: %x,size: %x,I cnt: %x, gopCnt: %x\n", file_gop_index ->lGopOffset, file_gop_index->nGopSize, file_gop_index->nIFrameCount, h264_file->FileIdxNal.u16IndexGopCnt);

		h264_file->FileTailNal.file_tail_nal.lTailFlag = htonl(0x00000001);
		h264_file->FileTailNal.file_tail_nal.cTailFlag = 0x7A;
		h264_file->FileTailNal.file_tail_nal.lIndexSize = file_gop_index ->nGopSize;
		h264_file->FileTailNal.file_tail_nal.lIndexOffset = file_gop_index ->lGopOffset ;
		h264_file->FileTailNal.file_tail_nal.lAllNalCount += (gop_nal_header->nNalCount);
		if(file_gop_index->nGopSize > h264_file->FileTailNal.file_tail_nal.lMaxGopNalSize)
		{
			h264_file->FileTailNal.file_tail_nal.lMaxGopNalSize = file_gop_index->nGopSize;
		}
		/*clean gop buff*/
		memset(h264_file->GopNal.gop_nal_buff, 0x00, GOP_NAL_BUFF_SIZE);
		h264_file->GopNal.gop_count++;
		h264_file->FileIdxNal.IFrame_count_in_gop = 0;
	}

    /*write file tail*/
	if(ISIL_FM_REC_GetFileTailFlag(h264_file) == 1)
	{
      ret = ISIL_FM_REC_WriteTail(h264_file);
      if(ret < 0)
      {
    	  ISIL_REC_DEBUG("write file gop indexfail\n");
    	  return -1;
      }
	}

    /*write file index*/
    if(ISIL_FM_REC_GetFileIndexFlag(h264_file) == 1)
	{
		#ifdef BIG_BYTES_ORDER
		tmp_val = file_index_nal->lGopCount;
		swap_three_bytes(&tmp_val);
		file_index_nal->lGopCount = tmp_val;
		#endif

		//write file_index_nal
		s32Offset = ISIL_FM_FILE_ALLOCATE_GetRecIndexStartOffset(h264_file->s32FileIndexFd, h264_file->channel);
        if(s32Offset < 0)
        {
        	ISIL_REC_DEBUG("ISIL_FM_FILE_ALLOCATE_GetRecIndexStartOffset fail\n");
		    return -1;
        }
        //fprintf(stderr, "line %d--seek: fd[%d]--\n", __LINE__, h264_file->FileIdxNal.write_file_fd);
		ret = lseek(h264_file->FileIdxNal.write_file_fd, s32Offset, SEEK_SET);
        if(ret < 0){
        	ISIL_REC_DEBUG("lseek fail");
            perror("Seek:");
			return -1;
        }
		ret = write_r(h264_file->FileIdxNal.write_file_fd, (unsigned char *)file_index_nal, file_index_nal_len);
		if(ret < 0)
		{
			ISIL_REC_DEBUG("write file_index_nal\n");
			return -1;
		}
		#ifdef BIG_BYTES_ORDER
		tmp_val = file_gop_index->lGopOffset;
		swap_four_bytes(&tmp_val);
		file_gop_index->lGopOffset = tmp_val;

		tmp_val_2B = file_gop_index->nGopSize;
		swap_two_bytes((unsigned short int *)&tmp_val_2B);
		file_gop_index->nGopSize = tmp_val_2B;

		tmp_val_2B = file_gop_index->nIFrameCount;
		swap_two_bytes((unsigned short int *)&tmp_val_2B);
		file_gop_index->nIFrameCount = tmp_val_2B;
		#endif

		//write gop index
		//fprintf(stderr, "line %d--seek: fd[%d]--\n", __LINE__, h264_file->FileIdxNal.write_file_fd);
		ret = lseek(h264_file->FileIdxNal.write_file_fd, s32Offset + h264_file->FileIdxNal.s32IndexOffset, SEEK_SET);
        if(ret < 0){
        	ISIL_REC_DEBUG("lseek fail");
            perror("Seek:");
			return -1;
        }
		ret = write_r(h264_file->FileIdxNal.write_file_fd, (unsigned char *)(h264_file->FileIdxNal.file_index_nal_buff + file_index_nal_len), h264_file->FileIdxNal.u16IndexGopCnt * file_gop_index_len);
		if(ret < 0)
		{
			ISIL_REC_DEBUG("write file gop indexfail\n");
			return -1;
		}
        h264_file->FileIdxNal.s32IndexOffset += (h264_file->FileIdxNal.u16IndexGopCnt * file_gop_index_len);
        h264_file->FileIdxNal.u16IndexGopCnt = 0;
		ISIL_FM_REC_SetFileIndexFlag(h264_file, 0);
		memset((h264_file->FileIdxNal.file_index_nal_buff +file_index_nal_len), 0x00, (FILE_INDEX_BUFF_SIZE - file_index_nal_len));
	}

	ISIL_FM_FILE_ALLOCATE_UpdateIndexRecLog(h264_file->s32FileIndexFd, rec_mgt->chan, h264_file->s32RecOffset);
    //fprintf(stderr, "----rec--s32RecOffset: %x------\n", h264_file->s32RecOffset);

	return ret;
}


static int ISIL_FM_REC_close(isil_rec_mgt *rec_mgt)
{
    stH264File *h264_file = &(rec_mgt->h264file);

	close(h264_file->GopNal.write_fd);

	close(h264_file->FileIdxNal.write_file_fd);

	close(h264_file->s32FileIndexFd);

    close(h264_file->mjpg_idx_fd);

	return 0;
}

static int ISIL_FM_REC_SwitchFile(isil_rec_mgt *rec_mgt, unsigned int datalen)
{
	int iRet = 0;
	unsigned int totle_len = 0;

	if(!rec_mgt){
		ISIL_REC_DEBUG("fail");
		return -1;
	}

	totle_len = rec_mgt->h264file.s32RecOffset + datalen + rec_mgt->h264file.GopNal.gop_nal_buff_len + sizeof(ISIL_GOP_INDEX) + sizeof(ISIL_FILE_TAIL_NAL);

	if((totle_len > ISIL_FM_FILE_ALLOCATE_GetFileSize())
       || ISIL_FM_CheckRecIndexFileIsEnd(rec_mgt->h264file.FileIdxNal.s32IndexOffset))
    {
		ISIL_REC_DEBUG("  start");
		ISIL_REC_DEBUG("---off:%x, datalen: %x, goplen: %x---\n", rec_mgt->h264file.s32RecOffset, datalen, rec_mgt->h264file.GopNal.gop_nal_buff_len);
        /*write file tail*/
        ISIL_FM_REC_SetFileTailFlag(rec_mgt->chan, 1);

        if(ISIL_FM_REC_Write(NULL, rec_mgt) < 0){
        	printf("ISIL_FM_REC_WriteTail fail\n");
        		return -1;
        	}

        /*write 0 for file total len*/
        iRet = ISIL_FM_SRV_Write_RemainSpace(rec_mgt->h264file.GopNal.write_fd, rec_mgt->h264file.s32RecOffset,
											  ISIL_FM_FILE_ALLOCATE_GetFileSize()-rec_mgt->h264file.s32RecOffset);
        if(iRet < 0)
        {
            printf("ISIL_FM_SRV_Write_RemainSpace fail\n");
        }

        /*update full*/
        if(ISIL_FM_FILE_ALLOCATE_UpdateFull(rec_mgt->h264file.s32FileIndexFd, rec_mgt->chan) == -1)
        {
            fprintf(stderr, "ISIL_FM_FILE_ALLOCATE_UpdateFull fail\n");
        }

        if(ISIL_FM_REC_SyncFile(rec_mgt) < 0){
        	ISIL_REC_DEBUG("fail");
        	return -1;
        }

        if(ISIL_FM_REC_close(rec_mgt) < 0){
        	ISIL_REC_DEBUG("fail");
        	return -1;
        }

        rec_mgt->state = recstate_close;

        /*open new file*/
		if(ISIL_FM_REC_Init(rec_mgt->chan) < 0){
			ISIL_REC_DEBUG("fail");
			return -1;
		}

		if(ISIL_FM_REC_OpenFile(rec_mgt, rec_mgt->main_type, rec_mgt->sub_type) < 0){
			ISIL_REC_DEBUG("fail");
			return -1;
		}

		ISIL_REC_DEBUG(" end");
    }

	return 0;
}

int	ISIL_FM_REC_WriteFile(stDriverFrame *driver_data, isil_rec_mgt *rec_mgt)
{
	if(!rec_mgt){
		return -1;
	}

	if((rec_mgt->state != recstate_start) && (rec_mgt->state != recstate_write)){
		return -1;
	}

	if(driver_data){
		if(ISIL_FM_REC_SwitchFile(rec_mgt, driver_data->len) < 0){
			ISIL_REC_DEBUG("fail");
			return -1;
		}
	}

	if(ISIL_FM_REC_Write(driver_data, rec_mgt) < 0){
		ISIL_REC_DEBUG("fail");
		return -1;
	}

	rec_mgt->state = recstate_write;
	return 0;
}



int ISIL_FM_REC_SyncFile(isil_rec_mgt *rec_mgt)
{
	if(!rec_mgt){
		return -1;
	}

    stH264File *h264_file = &(rec_mgt->h264file);

    //ISIL_REC_DEBUG("---sync start----\n");

#if 0
	fsync(h264_file->GopNal.write_fd);

	fsync(h264_file->FileIdxNal.write_file_fd);

    fsync(h264_file->s32FileIndexFd);

    fsync(h264_file->mjpg_idx_fd);
#endif
    //fprintf(stderr, "---- sync end----\n");
	return 0;
}



int ISIL_FM_REC_CloseFile(isil_rec_mgt *rec_mgt)
{
	if(!rec_mgt){
    	ISIL_REC_DEBUG("fail");
		return -1;
	}

	if(rec_mgt->state == recstate_close){
		return 0;
	}

    ISIL_FM_REC_SetFileTailFlag(rec_mgt->chan, 1);

    //ISIL_FM_REC_WriteFile(NULL, rec_mgt);
    if(ISIL_FM_REC_Write(NULL, rec_mgt) < 0){
    	ISIL_REC_DEBUG("fail");
    	return -1;
    }

    if(ISIL_FM_REC_SyncFile(rec_mgt) < 0){
    	ISIL_REC_DEBUG("fail");
    	return -1;
    }

    if(ISIL_FM_REC_close(rec_mgt) < 0){
    	ISIL_REC_DEBUG("fail");
    	return -1;
    }

    rec_mgt->state = recstate_close;

	return 0;
}

unsigned long ISIL_GetTimeStamp(void)
{
    //unsigned long tmp = 0;
    struct timeval cur;
    memset(&cur, 0x00, sizeof(struct timeval));
    gettimeofday(&cur, NULL);

	return(cur.tv_sec * 1000 + cur.tv_usec/1000);
}

/*allocate and init rec resource*/
isil_rec_mgt* ISIL_FM_REC_Init(unsigned int chan)
{
	if(chan >= MAX_REC_CHANNEL){
		ISIL_REC_DEBUG("---input chan[%d]>MaxCh[%d]----\n", chan, MAX_REC_CHANNEL);
		return NULL;
	}
	
	if((REC_MGT_TBL[chan].state == recstate_invalid) || REC_MGT_TBL[chan].state == recstate_close){
		memset(&REC_MGT_TBL[chan], 0x00, sizeof(isil_rec_mgt));
	}

	REC_MGT_TBL[chan].chan = chan;
	REC_MGT_TBL[chan].h264file.channel = chan;
    REC_MGT_TBL[chan].state = recstate_init;

    return &REC_MGT_TBL[chan];
}




