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

#include "isil_fm_play.h"
#include "isil_fm_play_error.h"
#include "isil_fm_utils.h"

//#include "tw_interface.h"

ISIL_FM_PLAY_PLAYER  Player[MAX_PLAYER_NUM];
char PLAY_ROOT_DIR[PLAY_ROOT_DIR_MAX_LEN + 1];

#ifdef BIG_BYTES_ORDER

static void Swap_FileIndexNal(ISIL_FILE_INDEX_NAL* stFileIndexNal)
{
	unsigned int tmp;

	if (NULL == stFileIndexNal)
		return;
	//printf("%s %d gop count: %d---",__FUNCTION__, __LINE__, stFileIndexNal->lGopCount);
	tmp = stFileIndexNal->lGopCount;
	swap_three_bytes(&tmp);
	stFileIndexNal->lGopCount = tmp;
#if 0
	int i;
	unsigned char *tmpChar;
	tmpChar =(unsigned char *)stFileIndexNal;
	printf("[%s %d]file index nal:", __FUNCTION__, __LINE__);
	for ( i = 0 ; i < sizeof(ISIL_FILE_INDEX_NAL) ; i++)
		printf("0x%x ",tmpChar[i]);
	printf("%s %d gop count: %d---",__FUNCTION__, __LINE__, stFileIndexNal->lGopCount);

	printf("\n");
#endif
	return;
}

static void Swap_GopIndex(ISIL_GOP_INDEX* stGopIndex)
{
	if (NULL == stGopIndex)
		return;

	swap_four_bytes((unsigned int *)&stGopIndex->lGopOffset);
	swap_two_bytes(&stGopIndex->nGopSize);
	swap_two_bytes(&stGopIndex->nIFrameCount);
#if 0
	int i;
	unsigned char *tmpChar;
	tmpChar =(unsigned char *) stGopIndex;
	printf("[%s %d]gop index nal:", __FUNCTION__, __LINE__);
	for ( i = 0 ; i < sizeof(ISIL_GOP_INDEX) ; i++)
		printf("0x%x ",tmpChar[i]);

	printf("\n");
#endif
	return;
}

static void Swap_GopNal(ISIL_GOP_NAL* stGopNal)
{
	unsigned int tmp;

	if (NULL == stGopNal)
		return;
	tmp = stGopNal->lLastGopSize;
	swap_three_bytes(&tmp);
	stGopNal->lLastGopSize = tmp;
	swap_four_bytes((unsigned int *)&stGopNal->lLastGopOffset);
	swap_two_bytes(&stGopNal->nGopID);
	swap_two_bytes(&stGopNal->nNalCount);
#if 0
	int i;
	unsigned char *tmpChar;
	tmpChar =(unsigned char *) stGopNal;
	printf("[%s %d]gop nal:", __FUNCTION__, __LINE__);
	for ( i = 0 ; i < sizeof(ISIL_GOP_NAL) ; i++)
		printf("0x%x ",tmpChar[i]);

	printf("\n");
#endif
	return;
}

static void Swap_NALIndex(ISIL_NAL_INDEX* stNalIndex)
{
	unsigned int tmp;

	if (NULL == stNalIndex)
		return;

	tmp = stNalIndex->lNalSize;
	swap_three_bytes(&tmp);
	stNalIndex->lNalSize = tmp;
	swap_four_bytes((unsigned int *)&stNalIndex->lNalOffset);
	swap_four_bytes((unsigned int *)&stNalIndex->lTimeStamp);

#if 0
	int i;
	unsigned char *tmpChar;
	tmpChar =(unsigned char *) stNalIndex;
	printf("[%s %d]nal index nal:", __FUNCTION__, __LINE__);
	for ( i = 0 ; i < sizeof(ISIL_NAL_INDEX) ; i++)
		printf("0x%x ",tmpChar[i]);

	printf("\n");
#endif
	return;
}


static void Swap_NalTail(ISIL_FILE_TAIL_NAL* NalTail)
{
	unsigned int tmp;

	if (NULL == NalTail)
		return;

	tmp = NalTail->lIndexSize;
	swap_three_bytes(&tmp);
	NalTail->lIndexSize = tmp;

	swap_four_bytes((unsigned int *)&NalTail->lIndexOffset);
	swap_four_bytes((unsigned int *)&NalTail->lAllNalCount);
	swap_four_bytes((unsigned int *)&NalTail->lTimeBegin);
	swap_four_bytes((unsigned int *)&NalTail->lTimeEnd);
	swap_four_bytes((unsigned int *)&NalTail->lMaxNalSize);
	swap_four_bytes((unsigned int *)&NalTail->lMaxGopNalSize);
	swap_four_bytes((unsigned int *)&NalTail->lMaxKeyFrameInterval);
	return;
}


#endif

int ISIL_FM_SEACH_SearchFile(MSG_FM_SEARCH* pSearch, void *buff)
{
	int ret = 0;
	int s32IndexFd = 0;
	char search_path[PATH_MAX];
	unsigned int  u32StartFile = 0;
	unsigned int  u32EndFile = 0;
	unsigned int  u32CurFile = 0;
	unsigned int  u32Channel = 0;
	unsigned int  u32EndTmp = 0;
	unsigned int  u32ReadFull = 0;
	int s32Log = 0;
	int start_ch,end_ch = 0;
	ISIL_ALLOCATE_FILE_BASEINFO stBase;
	ISIL_ALLOCATE_FILE_USING_INFO stUsing;
	ISIL_ALLOCATE_FILE_FULL_INFO  stFull;
	ISIL_ALLOCATE_FILE_REC_INFO   stRecLog;

	MSG_FM_SEARCH_RESULT* search_msg =(MSG_FM_SEARCH_RESULT*)buff;
	memset(search_msg, 0x00, sizeof(MSG_FM_SEARCH_RESULT));

	//fprintf(stderr, "------search add: %p, %p, %d-----\n", buff, search_msg, search_msg->s32MsgNum);

	if ((pSearch->ucChannel != 0xff) && (pSearch->ucChannel >= PLAY_MAX_CHANNEL)) {
		fprintf(stderr, "----search channel num is over %d-----\n", PLAY_MAX_CHANNEL);
		return ISIL_ERR_FAILURE;
	}

	fprintf(stderr, "search, ch:%x, type: %x, %s\n", pSearch->ucChannel, pSearch->ucType,pSearch->ucDeviceName);
	fprintf(stderr, "search, start time,%d-%d-%d %d:%d:%d\n", pSearch->stStart.Date.Year, pSearch->stStart.Date.Month, pSearch->stStart.Date.day,
			pSearch->stStart.Time.Hour,pSearch->stStart.Time.Minute,pSearch->stStart.Time.Second
		   );

	fprintf(stderr, "search, end time,%d-%d-%d %d:%d:%d\n", pSearch->stEnd.Date.Year, pSearch->stEnd.Date.Month, pSearch->stEnd.Date.day,
			pSearch->stEnd.Time.Hour,pSearch->stEnd.Time.Minute,pSearch->stEnd.Time.Second
		   );

	/*open and read index00.bin*/
	ISIL_FM_FILE_ALLOCATE_LockIndex();

	sprintf(search_path, "%s%s%s%s", PLAY_ROOT_DIR, ISIL_INDEX_DIR, ISIL_FILE_INDEX_DIR, ISIL_FILE_INDEX_NAME);
	s32IndexFd = ISIL_FM_FILE_ALLOCATE_OpenFile(search_path);
	if (s32IndexFd < 0) {
		fprintf(stderr, "-----ISIL_FM_FILE_ALLOCATE_OpenFile fail------\n");
		ISIL_FM_FILE_ALLOCATE_UnlockIndex();
		return ISIL_ERR_FAILURE;
	}

	ret = ISIL_FM_FILE_ALLOCATE_ReadIndexBase(s32IndexFd, &stBase);
	if (ret < 0) {
		fprintf(stderr, "-----ISIL_FM_FILE_ALLOCATE_ReadIndexBase fail------\n");
		ISIL_FM_FILE_ALLOCATE_UnlockIndex();
		return ISIL_ERR_FAILURE;
	}

	if (stBase.u32CoveredFileSum != 0xffffffff) {
		u32StartFile = stBase.u32CoveredFileSum;
		u32EndFile = stBase.u32FullFileSum - (stBase.u32FileTotle - stBase.u32CoveredFileSum) - 1;
		/*
		if(stBase.u32CoveredFileSum > (stBase.u32UsedFileSum - stBase.u32FullFileSum))
		{
			u32EndFile = stBase.u32CoveredFileSum - (stBase.u32UsedFileSum - stBase.u32FullFileSum) - 1;
		}
		else
		{
			u32EndFile = stBase.u32FileTotle - (stBase.u32UsedFileSum - stBase.u32FullFileSum) + stBase.u32CoveredFileSum -1 ;
		}
		*/
		u32ReadFull = 1;
	} else if (stBase.u32FullFileSum != 0xffffffff) {
		u32StartFile = 0x00;
		u32EndFile = stBase.u32FullFileSum - 1;
		u32ReadFull = 1;
	} else {
		u32ReadFull = 0;
	}

	if (pSearch->ucChannel == 0xff) {//search all channel
		start_ch = 0;
		end_ch = (PLAY_MAX_CHANNEL - 1);
	} else {
		start_ch = end_ch = pSearch->ucChannel;
	}

	for (; start_ch <= end_ch; start_ch++) {
		u32Channel = start_ch;

		if (u32ReadFull) {
			if (u32StartFile > u32EndFile) {
				u32EndTmp = stBase.u32FileTotle + u32EndFile - 1;
			} else {
				u32EndTmp = u32EndFile;
			}

			fprintf(stderr, "----read full: start: %d,end: %d,tmp: %d----\n", u32StartFile, u32EndFile, u32EndTmp);
			for (u32CurFile = u32StartFile; u32CurFile <= u32EndTmp; u32CurFile++) {
				ret = ISIL_FM_FILE_ALLOCATE_ReadIndexFull(s32IndexFd, u32CurFile, &stFull);
				if (ret < 0) {
					fprintf(stderr, "-----ISIL_FM_FILE_ALLOCATE_ReadIndexFull fail------\n");
					ISIL_FM_FILE_ALLOCATE_UnlockIndex();
					return ISIL_ERR_FAILURE;
				}
				if ((u32CurFile + 1) == stBase.u32FileTotle) {
					u32CurFile = -1;
					u32EndTmp = u32EndFile;
				}


				if (stFull.u32Channel != u32Channel) {//channel not match
					fprintf(stderr, "-----search 111------\n");
					continue;
				}
				fprintf(stderr, "----ch is same,u32CurFile: %d----\n", u32CurFile);
				/*read rec log*/
				for (s32Log = 0; s32Log < stFull.u16LogSum; s32Log++) {
					ret = ISIL_FM_FILE_ALLOCATE_ReadIndexRec(s32IndexFd, stFull.u32FileID, s32Log, &stRecLog);
					if (ret < 0) {
						fprintf(stderr, "-----ISIL_FM_FILE_ALLOCATE_ReadIndexRec fail------\n");
						ISIL_FM_FILE_ALLOCATE_UnlockIndex();
						return ISIL_ERR_FAILURE;
					}
					/*parsing reclog*/
					/*first,compare date*/
					//fprintf(stderr, "-----search start date:%d-%d-%d------\n", pSearch->stStart.Date.Year, pSearch->stStart.Date.Month, pSearch->stStart.Date.day);
					//fprintf(stderr, "-----search end date:%d-%d-%d------\n", pSearch->stEnd.Date.Year, pSearch->stEnd.Date.Month, pSearch->stEnd.Date.day);
					//fprintf(stderr, "-----reclog date:%d-%d-%d------\n", stRecLog.stStartDT.Date.Year, stRecLog.stStartDT.Date.Month, stRecLog.stStartDT.Date.day);
					
					/*exclude mjpg file,now*/
					if(IS_MJPG_TYPE(stRecLog.u8RecType)){
						fprintf(stderr, "exclude mjpg file\n");
						continue;
					}
					
					if ((memcmp(&stRecLog.stStartDT.Date, &pSearch->stStart.Date, sizeof(S_REC_DATE)) >= 0)
						&& (memcmp(&pSearch->stEnd.Date, &stRecLog.stStartDT.Date, sizeof(S_REC_DATE)) >= 0)
					   ) {
						/*compare time, not compare second*/
						if (((pSearch->stStart.Time.Hour*60 + pSearch->stStart.Time.Minute) <= (stRecLog.stStartDT.Time.Hour*60 + stRecLog.stStartDT.Time.Minute))
							&& ((pSearch->stEnd.Time.Hour*60 + pSearch->stEnd.Time.Minute) >= (stRecLog.stStartDT.Time.Hour*60 + stRecLog.stStartDT.Time.Minute))
						   ) {//time
							/*compare rec type*/
							if (pSearch->ucType) {
								if ((stRecLog.u8RecType & pSearch->ucType) == 0) {
									fprintf(stderr, "-----search 222------\n");
									continue;
								}
							}

							fprintf(stderr, "----search: find the file,stFull.u32FileID=%d--\n", stFull.u32FileID);
							
							MSG_FM_SEARCH_RESULT_1* result_msg =(MSG_FM_SEARCH_RESULT_1*)((unsigned char*)buff + sizeof(MSG_FM_SEARCH_RESULT) + search_msg->s32MsgNum*sizeof(MSG_FM_SEARCH_RESULT_1)) ;
							memset(result_msg, 0x00, sizeof(MSG_FM_SEARCH_RESULT_1));
							result_msg->lFlag = 0x0FFEABCD;
							result_msg->ucType      = stRecLog.u8RecType;
							result_msg->ucChNumber  = stRecLog.u8Channel;
							memcpy(result_msg->cURL, "192.168.0.183", 30);
							memcpy(result_msg->cDeviceName, "ISILNVS", 32);
							memcpy(result_msg->cChannelName, "video", 32);
							memcpy(&(result_msg->stStart), &(stRecLog.stStartDT.Time), sizeof(S_REC_TIME));
							memcpy((char *)(result_msg->ucFileExName), H264_FILE_SUFFIX, 4);


							/*pathname: ch_ymd_hms*/
							/*year-month-day*/
							sprintf(result_msg->cFilePath, "%d_%d", stFull.u32FileID, stRecLog.u8Channel);
							sprintf(result_msg->cFilePath, "%s_%d-%d-%d", result_msg->cFilePath, stRecLog.stStartDT.Date.Year, stRecLog.stStartDT.Date.Month, stRecLog.stStartDT.Date.day);
							/*hour-min-second*/
							sprintf(result_msg->cFilePath, "%s_%d-%d-%d", result_msg->cFilePath, stRecLog.stStartDT.Time.Hour, stRecLog.stStartDT.Time.Minute, stRecLog.stStartDT.Time.Second);
							/*.264*/
							sprintf(result_msg->cFilePath, "%s.%s", result_msg->cFilePath, H264_FILE_SUFFIX);

							/*output file name*/
							fprintf(stderr, "the file complete name is: %s\n", result_msg->cFilePath);
							search_msg->s32MsgNum++;
						} else {
							fprintf(stderr, "-----search 333------\n");
							continue;
						}
					} else {
						fprintf(stderr, "-----search 444------\n");
						continue;
					}
				}
			}
		}

		/*read using file*/
		ret = ISIL_FM_FILE_ALLOCATE_ReadIndexUsing(s32IndexFd, u32Channel, &stUsing);
		if (ret < 0) {
			fprintf(stderr, "-----ISIL_FM_FILE_ALLOCATE_ReadIndexUsing fail------\n");
			ISIL_FM_FILE_ALLOCATE_UnlockIndex();
			return ISIL_ERR_FAILURE;
		}

		if (stUsing.u32UsingFileID != 0xffffffff) {
			fprintf(stderr, "----search read using: file id: %d----\n", stUsing.u32UsingFileID);
			/*read rec log*/
			for (s32Log = 0; s32Log < stUsing.u16LogSum; s32Log++) {
				ret = ISIL_FM_FILE_ALLOCATE_ReadIndexRec(s32IndexFd, stUsing.u32UsingFileID, s32Log, &stRecLog);
				if (ret < 0) {
					fprintf(stderr, "-----ISIL_FM_FILE_ALLOCATE_ReadIndexRec fail------\n");
					ISIL_FM_FILE_ALLOCATE_UnlockIndex();
					return ISIL_ERR_FAILURE;
				}
				/*parsing reclog*/
				if(IS_MJPG_TYPE(stRecLog.u8RecType)){
						fprintf(stderr, "exclude mjpg file\n");
						continue;
					}
				/*first,compare date*/
				fprintf(stderr, "search, reclog time,%d-%d-%d %d:%d:%d\n", stRecLog.stStartDT.Date.Year, stRecLog.stStartDT.Date.Month, stRecLog.stStartDT.Date.day,
						stRecLog.stStartDT.Time.Hour,stRecLog.stStartDT.Time.Minute,stRecLog.stStartDT.Time.Second
					   );
				if ((memcmp(&stRecLog.stStartDT.Date, &pSearch->stStart.Date, sizeof(S_REC_DATE)) >= 0)
					&& (memcmp(&pSearch->stEnd.Date, &stRecLog.stStartDT.Date, sizeof(S_REC_DATE)) >= 0)
				   ) {
					fprintf(stderr, "search type: %x, rec type: %x\n", pSearch->ucType, stRecLog.u8RecType);
					/*compare time, not compare second*/
					if (((pSearch->stStart.Time.Hour*60 + pSearch->stStart.Time.Minute) <= (stRecLog.stStartDT.Time.Hour*60 + stRecLog.stStartDT.Time.Minute))
						&& ((pSearch->stEnd.Time.Hour*60 + pSearch->stEnd.Time.Minute) >= (stRecLog.stStartDT.Time.Hour*60 + stRecLog.stStartDT.Time.Minute))
					   ) {//time
						/*compare rec type*/
						if (pSearch->ucType) {
							if ((stRecLog.u8RecType & pSearch->ucType) == 0) {
								fprintf(stderr, "-----search 555------\n");
								continue;
							}
						}

						fprintf(stderr, "----search: find the file--\n");
						MSG_FM_SEARCH_RESULT_1* result_msg =(MSG_FM_SEARCH_RESULT_1*)((unsigned char*)buff + sizeof(MSG_FM_SEARCH_RESULT) +search_msg->s32MsgNum*sizeof(MSG_FM_SEARCH_RESULT_1)) ;
						memset(result_msg, 0x00, sizeof(MSG_FM_SEARCH_RESULT_1));
						//fprintf(stderr, "----search buff: %p, %p, %d--\n", buff, result_msg, search_msg->s32MsgNum);
						result_msg->lFlag = 0x0FFEABCD;

						result_msg->ucType      = stRecLog.u8RecType;
						result_msg->ucChNumber  = stRecLog.u8Channel;

						memcpy(result_msg->cURL, "192.168.0.183", 30);
						memcpy(result_msg->cDeviceName, "ISILNVS", 32);
						memcpy(result_msg->cChannelName, "video", 32);
						memcpy(&(result_msg->stStart), &(stRecLog.stStartDT.Time), sizeof(S_REC_TIME));
						memcpy((char *)(result_msg->ucFileExName), H264_FILE_SUFFIX, 4);
						/*pathname: ch_ymd_hms*/
						/*year-month-day*/
						sprintf(result_msg->cFilePath, "%d_%d", stUsing.u32UsingFileID, stRecLog.u8Channel);
						sprintf(result_msg->cFilePath, "%s_%d-%d-%d", result_msg->cFilePath, stRecLog.stStartDT.Date.Year, stRecLog.stStartDT.Date.Month, stRecLog.stStartDT.Date.day);
						/*hour-min-second*/
						sprintf(result_msg->cFilePath, "%s_%d-%d-%d", result_msg->cFilePath, stRecLog.stStartDT.Time.Hour, stRecLog.stStartDT.Time.Minute, stRecLog.stStartDT.Time.Second);
						/*.264*/
						sprintf(result_msg->cFilePath, "%s.%s", result_msg->cFilePath, H264_FILE_SUFFIX);

						/*output file name*/
						fprintf(stderr, "the file complete name is: %s\n", result_msg->cFilePath);
						search_msg->s32MsgNum++;
					} else {
						fprintf(stderr, "-----search 666------\n");
						continue;
					}
				} else {
					fprintf(stderr, "-----search 777------\n");
					continue;
				}
			}
		}
	}
	ISIL_FM_FILE_ALLOCATE_UnlockIndex();
	fprintf(stderr, "------search result: %d------\n", search_msg->s32MsgNum);
	return ISIL_ERR_SUCCESS;
}

static int ISIL_FM_PLAY_ConvertPath(char *strPath, ISIL_FM_PLAY_CONVERT_PATH *pOpen)
{
	int s32flag = 0;
	char strTmp[32];
	int i = 0;
	int j = 0;

	if (strPath == NULL) {
		fprintf(stderr, "input open path is null\n");
		return -1;
	}

	while (*strPath) {
		if (!s32flag) {//file id
			if (*strPath == '_') {
				s32flag = 1;
				i = 0;
				pOpen->u32FileID = atoi(strTmp);
				//fprintf(stderr, "file id : %d\n", pOpen->u32FileID);
				memset(strTmp, 0x00, 32);
			} else {
				strTmp[i++] = *strPath;
			}
		} else if (s32flag == 1) {//ch id
			if (*strPath == '_') {
				s32flag = 2;
				i = 0;
				pOpen->u32Channel = atoi(strTmp);
				//fprintf(stderr, "ch id : %d\n", pOpen->u32Channel);
				memset(strTmp, 0x00, 32);
			} else {
				strTmp[i++] = *strPath;
			}
		} else if (s32flag == 2) {//date
			if (*strPath == '_') {
				s32flag = 3;
				i = 0;
				j = 0;
				pOpen->stStartDT.Date.day = atoi(strTmp);
				//fprintf(stderr, "day: %d\n", pOpen->stStartDT.Date.day);
				memset(strTmp, 0x00, 32);
			} else {
				if (*strPath == '-') {
					i = 0;
					if (!j) {
						pOpen->stStartDT.Date.Year = atoi(strTmp);
						//fprintf(stderr, "year: %d\n", pOpen->stStartDT.Date.Year);
						memset(strTmp, 0x00, 32);
					} else if (j == 1) {
						pOpen->stStartDT.Date.Month = atoi(strTmp);
						//fprintf(stderr, "month: %d\n", pOpen->stStartDT.Date.Month);
						memset(strTmp, 0x00, 32);
					}
					j++;
				} else {
					strTmp[i++] = *strPath;
				}
			}
		} else if (s32flag == 3) {
			if (*strPath == '.') {
				s32flag = 4;
				i = 0;
				j = 0;
				pOpen->stStartDT.Time.Second = atoi(strTmp);
				//fprintf(stderr, "sec: %d\n", pOpen->stStartDT.Time.Second);
				memset(strTmp, 0x00, 32);
			} else {
				if (*strPath == '-') {
					i = 0;
					if (!j) {
						pOpen->stStartDT.Time.Hour = atoi(strTmp);
						//fprintf(stderr, "hour: %d\n", pOpen->stStartDT.Time.Hour);
						memset(strTmp, 0x00, 32);
					} else if (j == 1) {
						pOpen->stStartDT.Time.Minute = atoi(strTmp);
						//fprintf(stderr, "min: %d\n", pOpen->stStartDT.Time.Minute);
						memset(strTmp, 0x00, 32);
					}
					j++;
				} else {
					strTmp[i++] = *strPath;
				}
			}
		}

		if (s32flag >= 4) {
			break;
		}

		strPath++;
	}

	return 0;
}

int ISIL_FM_PLAY_Open(unsigned int u32id, MSG_FM_REMOTE_OPEN stOpen, MSG_FM_REMOTE_FILEINFO *stFileInfo)
{
	int i = 0;
	int ret = 0;
	int read_fd = 0;
	//struct stat sbuf;
	ISIL_FILE_TAIL_NAL   stTailBuf;
	ISIL_FILE_INDEX_NAL  stIndexBuf;
	ISIL_GOP_INDEX       stGopIndexBuf;
	ISIL_GOP_NAL         stGopNal;
	ISIL_NAL_INDEX       stNalIndex;
	char IndexPath[260];
	//int  IndexPathLen = 0;
	char cIsIndex = 0;

	ISIL_ALLOCATE_FILE_USING_INFO stUsing;
	ISIL_ALLOCATE_FILE_FULL_INFO  stFull;
	ISIL_ALLOCATE_FILE_REC_INFO   stRecLog;
	ISIL_ALLOCATE_FILE_REC_INFO   TmpRecLog;
	ISIL_FM_PLAY_CONVERT_PATH stOpenData;
	int u32FileID = 0;
	int s32RecId = 0;
	unsigned int u32DataOffset = 0;
	unsigned int u32StartOffset = 0;
	//int TmpIndexNum = 0;

	int index = ISIL_FM_PLAY_GetPlayerIndex(u32id);
	if (index < 0) {
		return ISIL_ERR_FILE_OPEN;
	}
	u32id = index+FILE_FD_BASE;

	memset(stFileInfo, 0x00, sizeof(MSG_FM_REMOTE_FILEINFO));
	Player[index].u32PlayerFd = u32id;

	//fprintf(stderr, "the open file info :\n");
	//fprintf(stderr, "cnt: %d, name: %s\n", stOpen.lFileCount, stOpen.cFileNames[stOpen.lFileCount-1]);
	if ((stOpen.lFileCount > 10) || (stOpen.lFileCount <= 0)) {
		fprintf(stderr, "----fun:%s,line:%d fail----\n", __FUNCTION__, __LINE__);
		return ISIL_ERR_FILE_OPEN;
	}

	Player[index].stOpenInfo.lFileCount = stOpen.lFileCount;

	sprintf(IndexPath, "%s%s%s%s", PLAY_ROOT_DIR, ISIL_INDEX_DIR, ISIL_FILE_INDEX_DIR, ISIL_FILE_INDEX_NAME);
	read_fd = open(IndexPath, O_RDONLY);
	if (read_fd < 0) {
		fprintf(stderr, "----fun:%s,line:%d fail----\n", __FUNCTION__, __LINE__);
		return ISIL_ERR_FILE_OPEN;
	}

	for (i = 0; i < stOpen.lFileCount; i++) {
		if(!strlen(stOpen.cFileNames[i])){
			fprintf(stderr, "----fun:%s,line:%d fail----\n", __FUNCTION__, __LINE__);
			return ISIL_ERR_FILE_OPEN;
		}

		/*init variable*/
		cIsIndex = 0;

		memset(&stOpenData, 0x00, sizeof(stOpenData));
		ret = ISIL_FM_PLAY_ConvertPath(stOpen.cFileNames[i], &stOpenData);
		if (ret < 0) {
			return ISIL_ERR_FILE_OPEN;
		}

		/*parsing index00.bin,find the rec file info*/
		ISIL_FM_FILE_ALLOCATE_LockIndex();
		if (ISIL_FM_FILE_ALLOCATE_ReadIndexUsing(read_fd, stOpenData.u32Channel, &stUsing) < 0) {
			close(read_fd);
			ISIL_FM_FILE_ALLOCATE_UnlockIndex();
			return ISIL_ERR_FILE_OPEN;
		}

		if(stUsing.u32UsingFileID != stOpenData.u32FileID) {
			if (ISIL_FM_FILE_ALLOCATE_ReadIndexFull(read_fd, stOpenData.u32FileID, &stFull) < 0) {
				close(read_fd);
				ISIL_FM_FILE_ALLOCATE_UnlockIndex();
				return ISIL_ERR_FILE_OPEN;
			}

			if (stFull.u32FileID != stOpenData.u32FileID) {
				if (i == (stOpen.lFileCount - 1)) {
					fprintf(stderr, "not find the open file id\n");
					close(read_fd);
					ISIL_FM_FILE_ALLOCATE_UnlockIndex();
					return ISIL_ERR_FILE_OPEN;
				}
				continue;
			}
			u32FileID = stFull.u32FileID;
			s32RecId = stFull.u16LogSum;
		} else {
			u32FileID = stUsing.u32UsingFileID;
			s32RecId = stUsing.u16LogSum;
		}
		/*read rec log*/
		int loop = 0;
		for (loop = 0; loop < s32RecId; loop++) {
			if (ISIL_FM_FILE_ALLOCATE_ReadIndexRec(read_fd, u32FileID, loop, &stRecLog) < 0) {
				close(read_fd);
				ISIL_FM_FILE_ALLOCATE_UnlockIndex();
				return ISIL_ERR_FILE_OPEN;
			}

			if ((memcmp(&stRecLog.stStartDT.Date, &stOpenData.stStartDT.Date, sizeof(S_REC_DATE)) != 0)
				|| ((stRecLog.stStartDT.Time.Hour*3600+stRecLog.stStartDT.Time.Minute*60+stRecLog.stStartDT.Time.Second)
					!= (stOpenData.stStartDT.Time.Hour*3600+stOpenData.stStartDT.Time.Minute*60+stOpenData.stStartDT.Time.Second))
			   ) {
				if (loop == (s32RecId - 1)) {
					fprintf(stderr, "not find the open file DT\n");
					close(read_fd);
					ISIL_FM_FILE_ALLOCATE_UnlockIndex();
					return ISIL_ERR_FILE_OPEN;
				}
				continue;
			}

			fprintf(stderr, "find the open file rec log: %x, %x, %x\n", stRecLog.u32StartOffset, stRecLog.u32StopOffset, stRecLog.u32RecIndexId);
			u32StartOffset = stRecLog.u32StartOffset;
			u32DataOffset = u32StartOffset;
#if 1
			int tmp = 0;
			for (tmp = loop; tmp >= 0; tmp--) {
				if (ISIL_FM_FILE_ALLOCATE_ReadIndexRec(read_fd, u32FileID, tmp, &TmpRecLog) < 0) {
					close(read_fd);
					ISIL_FM_FILE_ALLOCATE_UnlockIndex();
					return ISIL_ERR_FILE_READ;
				}

				if (TmpRecLog.u8RecFileID !=  stRecLog.u8RecFileID) {
					break;
				}
				if (u32DataOffset > TmpRecLog.u32StartOffset) {
					u32DataOffset = TmpRecLog.u32StartOffset;
				}
			}
#endif
			break;
		}
		ISIL_FM_FILE_ALLOCATE_UnlockIndex();

		/*get timelength*/
		int s32DataFd = 0;
		memset(IndexPath, 0x00, 260);
		sprintf(IndexPath, "%s%sisil%d.264", PLAY_ROOT_DIR, ISIL_DATA_DIR, u32FileID);
		s32DataFd = open(IndexPath, O_RDONLY);
		if (s32DataFd < 0) {
			return ISIL_ERR_FILE_OPEN;
		}
		
		//parsing file
		fprintf(stderr, "---not find tail in file---\n");
		int s32RecIndexFd = 0;
		int s32Offset = 0;
		memset(IndexPath, 0x00, 260);
		sprintf(IndexPath, "%s%s%s%d.bin",PLAY_ROOT_DIR, ISIL_INDEX_DIR, ISIL_REC_INDEX_DIR, u32FileID);
		s32RecIndexFd = open(IndexPath, O_RDONLY);
		if (s32RecIndexFd < 0) {
			return ISIL_ERR_FILE_OPEN;
		}

		s32Offset = ISIL_FM_FILE_ALLOCATE_GetRecIndexStartOffsetById(stRecLog.u32RecIndexId);
		if (s32Offset < 0) {
			close(s32RecIndexFd);
			return ISIL_ERR_FILE_OPEN;
		}

		if (lseek(s32RecIndexFd, s32Offset, SEEK_SET) < 0) {
			return ISIL_ERR_FILE_OPEN;
		}

		ret = read(s32RecIndexFd, &stIndexBuf, sizeof(ISIL_FILE_INDEX_NAL));
		if (ret < 0) {
			fprintf(stderr, "----fun:%s,line:%d fail----\n", __FUNCTION__, __LINE__);
			close(s32RecIndexFd);
			close(read_fd);
			return ISIL_ERR_FILE_OPEN;
		}

		#ifdef BIG_BYTES_ORDER
		Swap_FileIndexNal(&stIndexBuf);
		#endif

		if ((stIndexBuf.lIndexFlag == htonl(0x00000001)) && (stIndexBuf.cIndexFlag == 0x79)) {
			cIsIndex = 1;//find index file
			fprintf(stderr, "---find the index file,cnt: %x ---\n", stIndexBuf.lGopCount);
		}

		if (cIsIndex) {
			unsigned long ulTimeBegin = 0, ulTimeEnd = 0;
			int IndexNum = 0;

			memset(&stGopIndexBuf, 0x00, sizeof(ISIL_GOP_INDEX));
			for (IndexNum = 0; u32StartOffset > (stGopIndexBuf.lGopOffset + u32DataOffset); IndexNum++) {
				ret = lseek(s32RecIndexFd, s32Offset + sizeof(stIndexBuf)+IndexNum*sizeof(ISIL_GOP_INDEX), SEEK_SET);
				if (ret < 0) {
					fprintf(stderr, "----fun:%s,line:%d fail----\n", __FUNCTION__, __LINE__);
					//close(s32RecIndexFd);
					//close(read_fd);
					//close(s32DataFd);
					//return ISIL_ERR_FILE_OPEN;
					continue;
				}

				ret = read(s32RecIndexFd, &stGopIndexBuf, sizeof(stGopIndexBuf));
				if (ret < 0) {
					fprintf(stderr, "----fun:%s,line:%d fail----\n", __FUNCTION__, __LINE__);
					//close(s32RecIndexFd);
					//close(read_fd);
					//close(s32DataFd);
					//return ISIL_ERR_FILE_OPEN;
					continue;
				}

				#ifdef BIG_BYTES_ORDER
				Swap_GopIndex(&stGopIndexBuf);
				#endif

				if (u32StartOffset <= (stGopIndexBuf.lGopOffset + u32DataOffset)) {
					if(IndexNum){
						IndexNum--;
					}
					//TmpIndexNum = 1;
					break;
				}

			}
			/*
			if (u32StartOffset == (stGopIndexBuf.lGopOffset + u32DataOffset)) {
				TmpIndexNum = 1;
			} 
			*/ 
			fprintf(stderr, "----IndexNum: %x, gopcnt: %x----\n", IndexNum, stIndexBuf.lGopCount);
			int j = 0;
			for(j = 0; j < 2; j++)
			{
				if(!i && !j){
					fprintf(stderr, "--save file indexgop--\n");
					Player[index].PlayerCfg.iGopNum = IndexNum;
				}

				if(!j){
					Player[index].stOpenInfo.startGopNum[i] = IndexNum;
				}

				ret = lseek(s32RecIndexFd, s32Offset + sizeof(stIndexBuf)+IndexNum*sizeof(ISIL_GOP_INDEX), SEEK_SET);
				if (ret < 0) {
					fprintf(stderr, "----fun:%s,line:%d fail----\n", __FUNCTION__, __LINE__);
					//close(s32RecIndexFd);
					//close(read_fd);
					//close(s32DataFd);
					//return ISIL_ERR_FILE_OPEN;
					continue;
				}

				ret = read(s32RecIndexFd, &stGopIndexBuf, sizeof(stGopIndexBuf));
				if (ret < 0) {
					fprintf(stderr, "----fun:%s,line:%d fail----\n", __FUNCTION__, __LINE__);
					//close(s32RecIndexFd);
					//close(read_fd);
					//close(s32DataFd);
					//return ISIL_ERR_FILE_OPEN;
					continue;
				}

				#ifdef BIG_BYTES_ORDER
				Swap_GopIndex(&stGopIndexBuf);
				#endif

				ret = lseek(s32DataFd,  (u32DataOffset + stGopIndexBuf.lGopOffset), SEEK_SET);//find the first gop nal
				if (ret < 0) {
					fprintf(stderr, "----fun:%s,line:%d fail----\n", __FUNCTION__, __LINE__);
					//close(s32RecIndexFd);
					//close(read_fd);
					//close(s32DataFd);
					//return ISIL_ERR_FILE_OPEN;
					continue;
				}

				ret = read(s32DataFd, &stGopNal, sizeof(stGopNal));//read the first gop nal
				if (ret < 0) {
					fprintf(stderr, "----fun:%s,line:%d fail----\n", __FUNCTION__, __LINE__);
					//close(s32RecIndexFd);
					//close(read_fd);
					//close(s32DataFd);
					//return ISIL_ERR_FILE_OPEN;
					continue;
				}
				
				#ifdef BIG_BYTES_ORDER
				Swap_GopNal(&stGopNal);
				#endif

				int NalNum = 0;

				if(j == 0){
					NalNum = 0;
				}else{
					NalNum = stGopNal.nNalCount-1;
					fprintf(stderr, "----here, %x, nalcnt: %x----\n", NalNum, stGopNal.nNalCount);
				}

				ret = lseek(s32DataFd, u32DataOffset + stGopIndexBuf.lGopOffset + sizeof(ISIL_GOP_NAL) + NalNum*sizeof(ISIL_NAL_INDEX), SEEK_SET);//find the first nal index
				if (ret < 0) {
					fprintf(stderr, "----fun:%s,line:%d fail----\n", __FUNCTION__, __LINE__);
					//close(s32RecIndexFd);
					//close(read_fd);
					//close(s32DataFd);
					//return ISIL_ERR_FILE_OPEN;
					continue;
				}

				ret = read(s32DataFd, &stNalIndex,  sizeof(ISIL_NAL_INDEX));
				if (ret < 0) {
					fprintf(stderr, "----fun:%s,line:%d fail----\n", __FUNCTION__, __LINE__);
					//close(s32RecIndexFd);
					//close(read_fd);
					//close(s32DataFd);
					//return ISIL_ERR_FILE_OPEN;
					continue;
				}

				#ifdef BIG_BYTES_ORDER
				Swap_NALIndex(&stNalIndex);
				#endif

				if (j == 0) {
					ulTimeBegin = stNalIndex.lTimeStamp;
					Player[index].stOpenInfo.ulTimeBegin[i] = ulTimeBegin;
				}
				
				ulTimeEnd = stNalIndex.lTimeStamp;
				fprintf(stderr, "BegTime: %lx, EndTime=%lx\n", ulTimeBegin, ulTimeEnd);
				Player[index].stOpenInfo.ulTimeEnd[i] = ulTimeEnd;
				stFileInfo->ulTimeLength = (ulTimeEnd - ulTimeBegin);
				stFileInfo->ulMaxKeyFrameInterval = 100;

				if(j == 0){
					fprintf(stderr, "----here 111:%x----\n", stIndexBuf.lGopCount);
					IndexNum = stIndexBuf.lGopCount - 1;
				}
			}
		}
		//close(s32RecIndexFd);
		Player[index].stOpenInfo.dataindexfd[i] = s32RecIndexFd;

		/*get file size*/
		stFileInfo->ulFileSize += (stRecLog.u32StopOffset - stRecLog.u32StartOffset);
		//fprintf(stderr, "open file: size=%x\n", (stRecLog.u32StopOffset - stRecLog.u32StartOffset));
		//close(s32DataFd);
		Player[index].stOpenInfo.datafd[i] = s32DataFd;
		Player[index].stOpenInfo.datastart[i] = u32DataOffset;
		Player[index].stOpenInfo.recindexstart[i] = s32Offset;
	}

	//close(read_fd);
	Player[index].stOpenInfo.fileindexfd = read_fd;
	stFileInfo->ulFileFd = u32id;
	stFileInfo->ulBeginTimeStamp = Player[index].stOpenInfo.ulTimeBegin[0];
	stFileInfo->ulEndTimeStamp   = Player[index].stOpenInfo.ulTimeEnd[stOpen.lFileCount - 1];
	Player[index].PlayerCfg.lTimePosition = stFileInfo->ulBeginTimeStamp;

	memcpy(&(Player[index].stFileInfo), stFileInfo, sizeof(MSG_FM_REMOTE_FILEINFO));

	fprintf(stderr, "---open size:0x%lx,timelen:0x%lx,start:0x%lx, end: 0x%lx,fd: %lx---\n", 
					stFileInfo->ulFileSize, stFileInfo->ulTimeLength,
					stFileInfo->ulBeginTimeStamp, stFileInfo->ulEndTimeStamp,
					stFileInfo->ulFileFd);

	return ISIL_ERR_SUCCESS;
}


int ISIL_FM_PLAY_InitPlayer(int s32Num)
{
	int i = 0;

	for(i = 0; i < Player[s32Num].stOpenInfo.lFileCount; i++){

		if(i >= 10) break;

		if(Player[s32Num].stOpenInfo.datafd[i]){
			close(Player[s32Num].stOpenInfo.datafd[i]);
		}

		if(Player[s32Num].stOpenInfo.dataindexfd[i]){
			close(Player[s32Num].stOpenInfo.dataindexfd[i]);
		}
	}

	if(Player[s32Num].stOpenInfo.fileindexfd){
		close(Player[s32Num].stOpenInfo.fileindexfd);
	}

	memset(&Player[s32Num], 0x00, sizeof(ISIL_FM_PLAY_PLAYER));

	Player[s32Num].u32PlayerFd = 0xffffffff;
	Player[s32Num].PlayerCfg.cDirection = 1;
	Player[s32Num].PlayerCfg.iNalNum = -1;
	Player[s32Num].PlayerStatus.cDirection = 1;
	Player[s32Num].PlayerCfg.cLoop = 1;
	return 0;
}

int ISIL_FM_PLAY_InitAllPlayer()
{
	int i = 0;

	for (i = 0; i < MAX_PLAYER_NUM; i++) {
		ISIL_FM_PLAY_InitPlayer(i);
	}
	return 0;
}

int ISIL_FM_PLAY_SearchPlayerByFd(unsigned int u32id)
{
	int i = 0;

	for (i = 0; i < MAX_PLAYER_NUM; i++) {
		if (Player[i].u32PlayerFd == u32id) {
			//fprintf(stderr, "----%s, find the player fd, i = %d, fd: %x----\n", __FUNCTION__,i,s32Fd);
			return i;
		}
	}

	fprintf(stderr, "----%s, not find the player fd[%x]----\n", __FUNCTION__, u32id);
	return -1;
}

int ISIL_FM_PLAY_GetPlayerIndex(unsigned int u32id)
{
	int index = -1;

	index = ISIL_FM_PLAY_SearchPlayerByFd(u32id);
	if (index != -1) {//find
		// fprintf(stderr, "---%s,a exit index:%d-\n", __FUNCTION__, index);
		return index;
	} else {//not find
		int i = 0;
		for (i = 0; i < MAX_PLAYER_NUM; i++) {
			if (Player[i].u32PlayerFd == 0xffffffff) {
				// fprintf(stderr, "---%s,new index:%d-\n", __FUNCTION__, i);
				return i;
			}
		}

		fprintf(stderr, "---%s, %d, fail--\n", __FUNCTION__, __LINE__);
		return -1;
	}
}


int ISIL_FM_PLAY_ClearPlayerByFd(unsigned int u32id)
{
	int s32Index = 0;

	s32Index = ISIL_FM_PLAY_SearchPlayerByFd(u32id);
	if (s32Index == -1) {
		return -1;
	}
	ISIL_FM_PLAY_InitPlayer(s32Index);
	fprintf(stderr, "----set close = %x-----\n",u32id);
	return 0;
}

MSG_FM_REMOTE_FILEINFO* ISIL_FM_PLAY_GetFileInfo(unsigned int u32id)
{
	int s32Index = 0;

	s32Index = ISIL_FM_PLAY_SearchPlayerByFd(u32id);
	if (s32Index == -1) {
		return NULL;
	}

	return &(Player[s32Index].stFileInfo);
}


static int ISIL_FM_PLAY_SearchIndexByTSLen(unsigned int u32id, unsigned long ts)
{
	int index = 0, i = 0;
	
	index = ISIL_FM_PLAY_GetPlayerIndex(u32id);
	if (index == -1) {
		return -1;
	}

	unsigned long tslen = 0; 
	for(i = 0; i < Player[index].stOpenInfo.lFileCount; i++){
		tslen += Player[index].stOpenInfo.ulTimeEnd[i]-Player[index].stOpenInfo.ulTimeBegin[i];
		if(tslen >= ts){
			Player[index].PlayerStatus.iPlayingID = i;
			unsigned long tmp_ts = 0;
			tmp_ts = tslen - (Player[index].stOpenInfo.ulTimeEnd[i]-Player[index].stOpenInfo.ulTimeBegin[i]);
			Player[index].PlayerCfg.lTimePosition = Player[index].stOpenInfo.ulTimeBegin[i] + tmp_ts;
			tmp_ts = tmp_ts/1000;
			if(tmp_ts){
				tmp_ts--;
			}
			Player[index].PlayerCfg.iGopNum = tmp_ts;
			break;
		}
	}
	
	fprintf(stderr, "--search ts-gopnum: %x, ts: %lx---\n", Player[index].PlayerCfg.iGopNum,
								Player[index].PlayerCfg.lTimePosition);
	return 0;

}


int ISIL_FM_PLAY_SetTS(unsigned int u32id, unsigned long ts)
{
	int index = 0, fileid = 0;

	index = ISIL_FM_PLAY_GetPlayerIndex(u32id);
	if (index == -1) {
		return -1;
	}
	
	fileid = Player[index].PlayerStatus.iPlayingID;

	Player[index].PlayerCfg.iGopNum = (ts - Player[index].stOpenInfo.ulTimeBegin[fileid])/1000 - 2;

	Player[index].PlayerCfg.lTimePosition = ts;
	Player[index].PlayerCfg.ulOffset = 0;
	Player[index].PlayerCfg.iNalNum = 0;
	Player[index].PlayerStatus.cFrameBack = 0;
	fprintf(stderr, "----ts start is= %lx-----\n",Player[index].stOpenInfo.ulTimeBegin[fileid]);
	fprintf(stderr, "----set ts is= %lx, gopno:%x-----\n",Player[index].PlayerCfg.lTimePosition,
												Player[index].PlayerCfg.iGopNum);
	return 0;
}


int ISIL_FM_PLAY_SetTSLen(unsigned int u32id, MSG_FM_REMOTE_OPERATOR FileOperate)
{
	int index = 0;

	index = ISIL_FM_PLAY_GetPlayerIndex(u32id);
	if (index == -1) {
		return -1;
	}

	if (FileOperate.FM_PLAY_CTL.lTimePosition > Player[index].stFileInfo.ulTimeLength) {
		fprintf(stderr, "fun:%s,line:%d, set timeposition > open file totle time\n", __FUNCTION__, __LINE__);
		return -1;
	}

	ISIL_FM_PLAY_SearchIndexByTSLen(u32id, FileOperate.FM_PLAY_CTL.lTimePosition);
	Player[index].PlayerCfg.ulOffset = 0;
	Player[index].PlayerCfg.iNalNum = 0;
	Player[index].PlayerStatus.cFrameBack = 0;
	fprintf(stderr, "----set ts len is= %ld-----\n",Player[index].PlayerCfg.lTimePosition);
	return 0;
}


long ISIL_FM_PLAY_GetTSLen(unsigned int u32id)
{
	int s32Index = 0;
	unsigned long ts_len = 0;

	s32Index = ISIL_FM_PLAY_SearchPlayerByFd(u32id);
	if (s32Index == -1) {
		return -1;
	}

	if (Player[s32Index].PlayerStatus.lTimePosition > Player[s32Index].stFileInfo.ulTimeLength) {
		fprintf(stderr, "fun:%s,line:%d, get timeposition > open file totle time\n", __FUNCTION__, __LINE__);
		return -1;
	}

	int i = 0;
	for(i = 0; i < Player[s32Index].PlayerStatus.iPlayingID; i++){
		ts_len += (Player[s32Index].stOpenInfo.ulTimeEnd[i]-Player[s32Index].stOpenInfo.ulTimeBegin[i]);
	}

	ts_len += (Player[s32Index].PlayerStatus.lTimePosition - 
		Player[s32Index].stOpenInfo.ulTimeBegin[Player[s32Index].PlayerStatus.iPlayingID]);
	return ts_len;
}

int ISIL_FM_PLAY_SetPause(unsigned int u32id)
{
	int index = 0;

	index = ISIL_FM_PLAY_GetPlayerIndex(u32id);
	if (index == -1) {
		return -1;
	}

	Player[index].PlayerCfg.cPause = 1;
	fprintf(stderr, "----set pause = %x-----\n",u32id);
	return 0;

}

int ISIL_FM_PLAY_SetPlay(unsigned int u32id)
{
	int index = 0;

	index = ISIL_FM_PLAY_GetPlayerIndex(u32id);
	if (index == -1) {
		return -1;
	}

	Player[index].PlayerCfg.cPause = 0;
	#ifdef DUMP_FILE
	playcnt++;
	fprintf(stderr, "----set play = %x, playcnt: %x-----\n",u32id, playcnt);
	#else
	fprintf(stderr, "----set play = %x-----\n",u32id);
	#endif
	return 0;
}

int ISIL_FM_PLAY_SetStop(unsigned int u32id)
{
	int index = 0;

	index = ISIL_FM_PLAY_GetPlayerIndex(u32id);
	if (index == -1) {
		return -1;
	}

	Player[index].PlayerCfg.cPause = 1;

	//initial indexgopnum, ts
	if(Player[index].stOpenInfo.lFileCount)
	{
		Player[index].PlayerStatus.iPlayingID = 0;
		Player[index].PlayerCfg.lTimePosition = Player[index].stFileInfo.ulBeginTimeStamp;
		Player[index].PlayerCfg.iGopNum = Player[index].stOpenInfo.startGopNum[0];
		Player[index].PlayerCfg.iNalNum = 0;
		Player[index].PlayerCfg.ulOffset = 0;
		fprintf(stderr, "----set stop = %x-----\n",u32id);
	}

	return 0;
}

int ISIL_FM_PLAY_SetSeek(unsigned int u32id, MSG_FM_REMOTE_OPERATOR FileOperate)
{
	int index = 0;
	unsigned long ts = 0;

	index = ISIL_FM_PLAY_GetPlayerIndex(u32id);
	if (index == -1) {
		return -1;
	}

	if ((FileOperate.FM_PLAY_CTL.fPercent < 0) && (FileOperate.FM_PLAY_CTL.fPercent*10 > 1)) {
		fprintf(stderr, "fun:%s,line:%d, set seek fPercent is overflow\n", __FUNCTION__, __LINE__);
		return -1;
	}

	Player[index].PlayerCfg.fPercent = FileOperate.FM_PLAY_CTL.fPercent;
	ts = FileOperate.FM_PLAY_CTL.fPercent*100*Player[index].stFileInfo.ulTimeLength/100;
	ISIL_FM_PLAY_SearchIndexByTSLen(u32id, ts);
	Player[index].PlayerCfg.ulOffset = 0;
	Player[index].PlayerCfg.iNalNum = 0;
	//Player[index].PlayerStatus.iPlayingID = 0;
	Player[index].PlayerStatus.cFrameBack = 0;

	fprintf(stderr, "----set seek = %f-----\n",Player[index].PlayerCfg.fPercent);
	return 0;
}

int ISIL_FM_PLAY_SetLoop(unsigned int u32id, MSG_FM_REMOTE_OPERATOR FileOperate)
{
	int index = 0;

	index = ISIL_FM_PLAY_GetPlayerIndex(u32id);
	if (index == -1) {
		return -1;
	}

	Player[index].PlayerCfg.cLoop = FileOperate.FM_PLAY_CTL.cLoop;
	fprintf(stderr, "-----set cLoop = %d--------\n",Player[index].PlayerCfg.cLoop);
	return 0;
}

int ISIL_FM_PLAY_SetDirection(unsigned int u32id, MSG_FM_REMOTE_OPERATOR FileOperate)
{
	int index = 0;

	index = ISIL_FM_PLAY_GetPlayerIndex(u32id);
	if (index == -1) {
		return -1;
	}

	Player[index].PlayerCfg.cDirection = FileOperate.FM_PLAY_CTL.cDirection;
	Player[index].PlayerStatus.cFrameBack = 0;
	fprintf(stderr, "-------set cDirection = %d------\n", Player[index].PlayerCfg.cDirection);
	return 0;
}

static int ISIL_FM_PLAY_GetDirection(unsigned int u32id)
{
	int s32Index = 0;
	s32Index = ISIL_FM_PLAY_SearchPlayerByFd(u32id);
	if (s32Index == -1) {
		return -1;
	}

	return Player[s32Index].PlayerStatus.cDirection;
}

int ISIL_FM_PLAY_SetKeyFrame(unsigned int u32id, MSG_FM_REMOTE_OPERATOR FileOperate)
{
	int index = 0;

	index = ISIL_FM_PLAY_GetPlayerIndex(u32id);
	if (index == -1) {
		fprintf(stderr, "----set keyframe, u32id[%x] fail---\n", u32id);
		return -1;
	}

	Player[index].PlayerCfg.cKeyFrame = FileOperate.FM_PLAY_CTL.cKeyFrame;
	Player[index].PlayerStatus.cFrameBack = 0;
	//RecPlayingCfg.lTimePosition = 0;
	//RecPlayingCfg.fPercent = 0;

	fprintf(stderr, "------set cKeyFrame = %d------\n", Player[index].PlayerCfg.cKeyFrame);
	return 0;
}

int ISIL_FM_PLAY_SetFrameCount(unsigned int u32id, MSG_FM_REMOTE_OPERATOR FileOperate)
{
	int index = 0;

	index = ISIL_FM_PLAY_GetPlayerIndex(u32id);
	if (index == -1) {
		return -1;
	}

	Player[index].PlayerCfg.lFrameCount += FileOperate.FM_PLAY_CTL.xFrameParam.lFrameCount;
	Player[index].PlayerCfg.lFrameDelay = FileOperate.FM_PLAY_CTL.xFrameParam.lFrameDelay;
	Player[index].PlayerStatus.lFrameDelay = Player[index].PlayerCfg.lFrameDelay;
	return 0;
}

int ISIL_FM_PLAY_GetFrameDelay(unsigned int u32id)
{
	int s32Index = 0;
	s32Index = ISIL_FM_PLAY_SearchPlayerByFd(u32id);
	if (s32Index == -1) {
		return -1;
	}

	return Player[s32Index].PlayerStatus.lFrameDelay;
}

int ISIL_FM_PLAY_GetPlayedFrameCnt(unsigned int u32id)
{
	int s32Index = 0;
	s32Index = ISIL_FM_PLAY_SearchPlayerByFd(u32id);
	if (s32Index == -1) {
		return -1;
	}

	return Player[s32Index].PlayerStatus.lFrameCount;
}

int ISIL_FM_PLAY_ClearPlayedFrameCnt(unsigned int u32id)
{
	int s32Index = 0;
	s32Index = ISIL_FM_PLAY_SearchPlayerByFd(u32id);
	if (s32Index == -1) {
		return -1;
	}

	Player[s32Index].PlayerStatus.lFrameCount = 0;
	return 0;
}

int ISIL_FM_PLAY_GetCfgFrameCnt(unsigned int u32id)
{
	int s32Index = 0;
	s32Index = ISIL_FM_PLAY_SearchPlayerByFd(u32id);
	if (s32Index == -1) {
		return -1;
	}

	return Player[s32Index].PlayerCfg.lFrameCount;
}

int ISIL_FM_PLAY_ClearCfgFrameCnt(unsigned int u32id)
{
	int s32Index = 0;
	s32Index = ISIL_FM_PLAY_SearchPlayerByFd(u32id);
	if (s32Index == -1) {
		return -1;
	}
	Player[s32Index].PlayerCfg.lFrameCount = 0;
	return 0;
}


int ISIL_FM_PLAY_ReadFile(void* argv, unsigned int u32id, int *CallBackRetValue)
{
	int s32Ret = 0;

	argv = argv;
	u32id = u32id;
	CallBackRetValue = CallBackRetValue;
	return s32Ret;
}

int ISIL_FM_PLAY_SetPlayDir(char *RootDir)
{
	if (RootDir == NULL) {
		fprintf(stderr, "---RootDiris null---\n");
		return -1;
	}

	memset(PLAY_ROOT_DIR, 0x00, PLAY_ROOT_DIR_MAX_LEN);
	memcpy(PLAY_ROOT_DIR, RootDir, PLAY_ROOT_DIR_MAX_LEN);
	sprintf(PLAY_ROOT_DIR, "%s/", PLAY_ROOT_DIR);

	fprintf(stderr, "---PLAY_ROOT_DIR: %s---\n", PLAY_ROOT_DIR);
	return 0;
}

/*add new get file data API for pb
*ISIL_FM_PLAY_ParsingFile: parsing file, get data offset in file
*ISIL_FM_PLAY_GetFileData: only read data from file
*/
#if 0//def NEW_PB_API
static int ISIL_FM_PLAY_ParsingFrameData(unsigned int u32id, PB_FRAME_INFO* frameinfo)
{
	int  iFileNum = 0;
	//int  iFileNumTmp = 0;
	int iGopNum = 0;
	int iNalNum = 0;
	int  iReadFd = 0;
	int  iReadIndexFd = 0;
	int  iRet = 0;
	ISIL_FILE_INDEX_NAL stFileIndexNal;
	ISIL_GOP_INDEX      stGopIndex;
	ISIL_GOP_NAL        stGopNal;
	ISIL_NAL_INDEX      stNalIndex;

	int s32IndexFd = 0;
	unsigned int u32FileID = 0;
	int s32RecId = 0;
	unsigned int u32DataOffset = 0;//this complement file 's first frame offset
	int s32RecIndexOffset = 0;
	int s32Index = 0;//player index

	//fprintf(stderr, "Enter: %s\n", __FUNCTION__);

	// fprintf(stderr, "---send data s32Fd: %d, %x---\n", s32Fd, s32Fd);

	s32Index = ISIL_FM_PLAY_SearchPlayerByFd(u32id);
	if (s32Index == -1) {
		fprintf(stderr, "---%d--\n", __LINE__);
		return ISIL_ERR_FAILURE;
	}

	if(Player[s32Index].PlayerCfg.cPause){
		frameinfo->nalcnt = 0;
		frameinfo->len = 0;
		//fprintf(stderr, "---%d--\n", __LINE__);
		return ISIL_ERR_SUCCESS;
	}

	if (Player[s32Index].PlayerStatus.cFrameBack) {
		//fprintf(stderr, "---%d--\n", __LINE__);
		return ISIL_ERR_FILE_REMAIN_FRAME;
	}

	if (!Player[s32Index].PlayerCfg.cDirection && !Player[s32Index].PlayerCfg.cKeyFrame) {
		
		fprintf(stderr, "---frame back mode---\n");
		Player[s32Index].PlayerStatus.cFrameBack = 1;
		if (Player[s32Index].PlayerStatus.cDirection) {
			Player[s32Index].PlayerCfg.iNalNum -= 2;
		}
	}

	Player[s32Index].PlayerStatus.cDirection = Player[s32Index].PlayerCfg.cDirection;


	while (Player[s32Index].PlayerStatus.iPlayingID < Player[s32Index].stOpenInfo.lFileCount) {
		iFileNum = Player[s32Index].PlayerStatus.iPlayingID;

		s32IndexFd = Player[s32Index].stOpenInfo.fileindexfd;
		iReadFd = Player[s32Index].stOpenInfo.datafd[iFileNum];
		iReadIndexFd = Player[s32Index].stOpenInfo.dataindexfd[iFileNum];

		u32DataOffset = Player[s32Index].stOpenInfo.datastart[iFileNum];
		s32RecIndexOffset = Player[s32Index].stOpenInfo.recindexstart[iFileNum];

		if(lseek(iReadIndexFd, s32RecIndexOffset, SEEK_SET) != s32RecIndexOffset)
	    {
	        close(iReadIndexFd);
	        close(iReadFd);
	        close(s32IndexFd);
			fprintf(stderr, "---%d--\n", __LINE__);
	        return ISIL_ERR_FILE_READ;
	    }

	    iRet = read(iReadIndexFd, &stFileIndexNal, sizeof(stFileIndexNal));
	    if(iRet < 0)
	    {
	        fprintf(stderr, "fun:%s,line:%d,read index file fail\n", __FUNCTION__, __LINE__);
	        close(iReadFd);
	        close(iReadIndexFd);
	        close(s32IndexFd);
			fprintf(stderr, "---%d--\n", __LINE__);
	        return ISIL_ERR_FILE_READ;
	    }
#ifdef BIG_BYTES_ORDER
        Swap_FileIndexNal(&stFileIndexNal);
#endif
	    /*�ж�flag�Ƿ���ȷ*/
	    if((stFileIndexNal.lIndexFlag != htonl(0x00000001)) || (stFileIndexNal.cIndexFlag != 0x79))
	    {
	        fprintf(stderr, "fun:%s,line:%d,%x,%x, index file flag error\n", __FUNCTION__, __LINE__, 
					stFileIndexNal.lIndexFlag, stFileIndexNal.cIndexFlag);
	        close(iReadFd);
	        close(iReadIndexFd);
	        close(s32IndexFd);
	        return ISIL_ERR_FILE_READ;
	    }


		if (!Player[s32Index].PlayerCfg.cDirection) {
			if (Player[s32Index].PlayerCfg.iGopNum < 0) {
				Player[s32Index].PlayerCfg.iGopNum = stFileIndexNal.lGopCount - 1;
			}

			if (Player[s32Index].PlayerStatus.iGopNum < 0) {
				Player[s32Index].PlayerStatus.iGopNum = stFileIndexNal.lGopCount - 1;
			}
		}

		iGopNum = Player[s32Index].PlayerCfg.iGopNum;

		memset(&stGopIndex, 0x00, sizeof(ISIL_GOP_INDEX));

		while (iGopNum < stFileIndexNal.lGopCount) {
			iRet = lseek(iReadIndexFd, s32RecIndexOffset + sizeof(stFileIndexNal) + iGopNum * sizeof(stGopIndex), SEEK_SET);
			if (iRet < 0) {
				fprintf(stderr, "fun:%s,line:%d,lseek index fail\n", __FUNCTION__, __LINE__);
				close(iReadFd);
				close(iReadIndexFd);
				close(s32IndexFd);
				return ISIL_ERR_FILE_READ;
			}

			iRet = read(iReadIndexFd, &stGopIndex, sizeof(stGopIndex));
			if (iRet < 0) {
				fprintf(stderr, "fun:%s,line:%d,read gop index fail\n", __FUNCTION__, __LINE__);
				close(iReadFd);
				close(iReadIndexFd);
				close(s32IndexFd);
				return ISIL_ERR_FILE_READ;
			}

#ifdef BIG_BYTES_ORDER
			//DEBUG_FUNCTION();
			Swap_GopIndex(&stGopIndex);
#endif

			iRet = lseek(iReadFd, u32DataOffset + stGopIndex.lGopOffset, SEEK_SET);
			if (iRet < 0) {
				fprintf(stderr, "fun:%s,line:%d,lseek gop nal fail\n", __FUNCTION__, __LINE__);
				close(iReadFd);
				close(iReadIndexFd);
				close(s32IndexFd);
				return ISIL_ERR_FILE_READ;
			}

			iRet = read(iReadFd, &stGopNal, sizeof(stGopNal));
			if (iRet < 0) {
				fprintf(stderr, "fun:%s,line:%d,read gop nal fail\n", __FUNCTION__, __LINE__);
				close(iReadFd);
				close(iReadIndexFd);
				close(s32IndexFd);
				return ISIL_ERR_FILE_READ;
			}


#ifdef BIG_BYTES_ORDER
			Swap_GopNal(&stGopNal);
#endif

			if ((stGopNal.lGopFlag != htonl(0x00000001)) || (stGopNal.cGopFlag != 0x78)) {
				fprintf(stderr, "fun:%s,line:%d,%x,%x,  gop nal flag error\n", __FUNCTION__, __LINE__, 
						stGopNal.lGopFlag, stGopNal.cGopFlag);
				close(iReadFd);
				close(iReadIndexFd);
				close(s32IndexFd);
				return ISIL_ERR_FILE_READ;
			}

			/*i??A?A?i??i??A?i??i??i??A?A?i??i??i??A?Nal index*/
			if (!Player[s32Index].PlayerCfg.cDirection) {
				if (Player[s32Index].PlayerCfg.iNalNum < 0) {
					Player[s32Index].PlayerCfg.iNalNum = stGopNal.nNalCount - 1;
				}

				if (Player[s32Index].PlayerStatus.iNalNum < 0) {
					Player[s32Index].PlayerStatus.iNalNum = stGopNal.nNalCount;
				}
			}

			iNalNum = Player[s32Index].PlayerCfg.iNalNum;
			
			while (iNalNum < stGopNal.nNalCount) {
				READSPS:    iRet = lseek(iReadFd, u32DataOffset + stGopIndex.lGopOffset + sizeof(stGopNal) + iNalNum * sizeof(stNalIndex), SEEK_SET);
				if (iRet < 0) {
					fprintf(stderr, "fun:%s,line:%d,lseek nal index fail\n", __FUNCTION__, __LINE__);
					close(iReadFd);
					close(iReadIndexFd);
					close(s32IndexFd);
					return ISIL_ERR_FILE_READ;
				}

				iRet = read(iReadFd, &stNalIndex, sizeof(stNalIndex));
				if (iRet < 0) {
					fprintf(stderr, "fun:%s,line:%d,read nal index fail\n", __FUNCTION__, __LINE__);
					close(iReadFd);
					close(iReadIndexFd);
					close(s32IndexFd);
					return ISIL_ERR_FILE_READ;
				}


#ifdef BIG_BYTES_ORDER
				Swap_NALIndex(&stNalIndex);
#endif

				char cFindData = 0;
				if ( Player[s32Index].PlayerCfg.cDirection   \
					 &&(Player[s32Index].PlayerCfg.lTimePosition <= (stNalIndex.lTimeStamp + 100))\
					 &&(Player[s32Index].PlayerCfg.ulOffset <= stNalIndex.lNalOffset)
				   ) {
					cFindData = 1;
				}

				if ( !Player[s32Index].PlayerCfg.cDirection  \
					 &&(stNalIndex.lTimeStamp <= (Player[s32Index].PlayerCfg.lTimePosition + 100)) \
					 &&(stNalIndex.lNalOffset < Player[s32Index].PlayerCfg.ulOffset)
				   ) {
					cFindData = 1;
				}

				if(cFindData)
	            {
	                if((stNalIndex.cNalType == E_H264_FRAME_TYPE_MAIN_PPS)
                       || (stNalIndex.cNalType == E_H264_FRAME_TYPE_SUB_PPS))
	                {
	                    iNalNum -= 1;
	                    goto READSPS;
	                }

	                if((stNalIndex.cNalType == E_H264_FRAME_TYPE_MAIN_IDR)
                       || (stNalIndex.cNalType == E_H264_FRAME_TYPE_SUB_IDR))
	                {
	                    iNalNum -= 2;
	                    goto READSPS;
	                }

					if(!Player[s32Index].PlayerStatus.cFrameBack)
					{
						Player[s32Index].PlayerStatus.iGopNum = iGopNum;
						Player[s32Index].PlayerStatus.iNalNum = iNalNum;
					}

	                int iNalCnt = 0;
	                int i = 0;

	                if(Player[s32Index].PlayerCfg.cKeyFrame || Player[s32Index].PlayerStatus.cFrameBack)
	                {
	                    if((stNalIndex.cNalType != E_H264_FRAME_TYPE_MAIN_SPS)
                           && (stNalIndex.cNalType != E_H264_FRAME_TYPE_SUB_SPS))
	                    {
							if(Player[s32Index].PlayerCfg.cDirection)
							{
	                            iNalNum++;
							}
							else
							{
								iNalNum--;
								if(iNalNum < 0)
								{
									iNalNum = stGopNal.nNalCount;
								}
							}
	                        continue;
	                    }else{
	                    	if(Player[s32Index].PlayerStatus.cFrameBack)
	                    	{
								if((iGopNum == Player[s32Index].PlayerStatus.iGopNum)
								   && (iNalNum == Player[s32Index].PlayerStatus.iNalNum)){
									iNalNum--;
									if(iNalNum < 0)
									{
										iNalNum = stGopNal.nNalCount;
									}
									continue;
								}
								else{
									Player[s32Index].PlayerCfg.iGopNum = iGopNum;
									Player[s32Index].PlayerCfg.iNalNum = iNalNum;
									Player[s32Index].PlayerCfg.lTimePosition = stNalIndex.lTimeStamp;
									Player[s32Index].PlayerCfg.ulOffset = 0;
									//close(iReadFd);
									//close(iReadIndexFd);
									//close(s32IndexFd);
									fprintf(stderr, "---%d, gop:%x, nal:%x, %x, %x--\n",
											__LINE__, iGopNum, iNalNum,
											Player[s32Index].PlayerStatus.iGopNum,
											Player[s32Index].PlayerStatus.iNalNum);
									return ISIL_ERR_FILE_REMAIN_FRAME;
								}
							}
	                    }
	                }

	                if((stNalIndex.cNalType == E_H264_FRAME_TYPE_MAIN_SPS)
                       ||(stNalIndex.cNalType == E_H264_FRAME_TYPE_SUB_SPS))//SPS,PPS,IO��Ad��cEI
	                {
	                    iNalCnt = 3;
	                }
	                else
	                {
	                    iNalCnt = 1;
	                }

                    frameinfo->nalcnt = iNalCnt;
                    frameinfo->fd = iReadFd;
                    frameinfo->offset = u32DataOffset + stNalIndex.lNalOffset;
                    frameinfo->ts = stNalIndex.lTimeStamp;
                    frameinfo->len = 0;

	                for(i = 0; i < iNalCnt; i++)
	                {
                        switch(stNalIndex.cNalType)
                        {
                            case E_H264_FRAME_TYPE_MAIN_SPS:
                            case E_H264_FRAME_TYPE_SUB_SPS:
                            {
                                frameinfo->nal[0].naltype = stNalIndex.cNalType;
                                frameinfo->nal[0].nallen = stNalIndex.lNalSize;
                            }
                            break;

                        	case E_H264_FRAME_TYPE_MAIN_PPS:
                            case E_H264_FRAME_TYPE_SUB_PPS:
                            {
                                frameinfo->nal[1].naltype = stNalIndex.cNalType;
                                frameinfo->nal[1].nallen = stNalIndex.lNalSize;
                            }
                            break;

                        	case E_H264_FRAME_TYPE_MAIN_IDR:
                            case E_H264_FRAME_TYPE_SUB_IDR:
                            {
                               frameinfo->nal[2].naltype = stNalIndex.cNalType;
                               frameinfo->nal[2].nallen = stNalIndex.lNalSize;
                            }
                            break;

                            default:
                            {
                                frameinfo->nal[0].naltype = stNalIndex.cNalType;
                                frameinfo->nal[0].nallen = stNalIndex.lNalSize;
                            }
                            break;
                        }
                        //fprintf(stderr, "----NalSize: %d-----\n", stNalIndex.lNalSize);
						if(Player[s32Index].PlayerCfg.cDirection
                            && (stNalIndex.cNalType != E_H264_FRAME_TYPE_MAIN_SPS)
                            && (stNalIndex.cNalType != E_H264_FRAME_TYPE_MAIN_PPS)
                            && (stNalIndex.cNalType != E_H264_FRAME_TYPE_SUB_SPS)
                            && (stNalIndex.cNalType != E_H264_FRAME_TYPE_SUB_PPS))
						{
							Player[s32Index].PlayerStatus.ulOffset 		= stNalIndex.lNalOffset;
							Player[s32Index].PlayerCfg.ulOffset     	= (stNalIndex.lNalOffset + stNalIndex.lNalSize);
                            if((stNalIndex.cNalType < E_FRAME_TYPE_AUDIO) 
                               ||(stNalIndex.cNalType > E_FRAME_TYPE_AUDIO_END))
                            {
							    Player[s32Index].PlayerCfg.lTimePosition 	= stNalIndex.lTimeStamp;
							    Player[s32Index].PlayerStatus.lTimePosition = stNalIndex.lTimeStamp;
                            }
							Player[s32Index].PlayerCfg.iGopNum 		= iGopNum;
							Player[s32Index].PlayerCfg.iNalNum 		= iNalNum;//iNalNum + 1;
						}
						else if(!Player[s32Index].PlayerCfg.cDirection 
                               &&(stNalIndex.cNalType != E_H264_FRAME_TYPE_MAIN_PPS) 
                               &&(stNalIndex.cNalType != E_H264_FRAME_TYPE_MAIN_IDR)
                               &&(stNalIndex.cNalType != E_H264_FRAME_TYPE_SUB_PPS) 
                               &&(stNalIndex.cNalType != E_H264_FRAME_TYPE_SUB_IDR))
						{
							Player[s32Index].PlayerStatus.ulOffset 		= stNalIndex.lNalOffset;
							Player[s32Index].PlayerCfg.ulOffset     	= (stNalIndex.lNalOffset - 1);
                            if((stNalIndex.cNalType < E_FRAME_TYPE_AUDIO) 
                               ||(stNalIndex.cNalType > E_FRAME_TYPE_AUDIO_END))//video
                            {
							    Player[s32Index].PlayerCfg.lTimePosition 	= stNalIndex.lTimeStamp;
							    Player[s32Index].PlayerStatus.lTimePosition = stNalIndex.lTimeStamp;
                            }
							Player[s32Index].PlayerCfg.iNalNum 		= iNalNum - 1;
							if(Player[s32Index].PlayerCfg.iNalNum == -1)
							{
								iGopNum--;
								if(iGopNum < 0)
								{
									iGopNum = stFileIndexNal.lGopCount;
								}
							}
							Player[s32Index].PlayerCfg.iGopNum 		= iGopNum;
						}

                        frameinfo->len += stNalIndex.lNalSize;
                        if((iNalCnt == 3) && (i < 2))
                        {
                            iNalNum++;
                            iRet = lseek(iReadFd, u32DataOffset + stGopIndex.lGopOffset + sizeof(stGopNal) + iNalNum * sizeof(stNalIndex), SEEK_SET);
                            if(iRet < 0)
                            {
                                fprintf(stderr, "fun:%s,line:%d,lseek nal index fail\n", __FUNCTION__, __LINE__);
                                close(iReadFd);
                                close(iReadIndexFd);
                                close(s32IndexFd);
                                return ISIL_ERR_FILE_READ;
                            }

                            iRet = read(iReadFd, &stNalIndex, sizeof(stNalIndex));
                            if(iRet < 0)
                            {
                                fprintf(stderr, "fun:%s,line:%d,read nal index fail\n", __FUNCTION__, __LINE__);
                                close(iReadFd);
                                close(iReadIndexFd);
                                close(s32IndexFd);
                                return ISIL_ERR_FILE_READ;
                            }
                            #ifdef BIG_BYTES_ORDER
                            Swap_NALIndex(&stNalIndex);
                            #endif
                        }
	                }

                    Player[s32Index].PlayerStatus.lFrameCount++; 
					
					#if 0
					fprintf(stderr, "cfg:G[%x] N[%x],state:G[%x] N[%x]\n",
							Player[s32Index].PlayerCfg.iGopNum, Player[s32Index].PlayerCfg.iNalNum,
							Player[s32Index].PlayerStatus.iGopNum, Player[s32Index].PlayerStatus.iNalNum);                  
					#endif
                    return ISIL_ERR_SUCCESS;;
	            }

                if(Player[s32Index].PlayerCfg.cDirection)
                {
				    iNalNum++;
                }
                else
                {
                    iNalNum--;
                    if(iNalNum < 0)
                    {
                        iNalNum = stGopNal.nNalCount;
                    }
                }
	        }

            if(Player[s32Index].PlayerCfg.cDirection)
            {
			    iGopNum++;
				Player[s32Index].PlayerCfg.iNalNum = 0;
            }
            else
            {
                iGopNum--;
                if(iGopNum < 0)
                {
                    iGopNum = stFileIndexNal.lGopCount;
                }
				Player[s32Index].PlayerCfg.iNalNum = -1;
            }
			Player[s32Index].PlayerCfg.iGopNum = iGopNum;
	    }

		if(iGopNum >= stFileIndexNal.lGopCount)
		{
			//fprintf(stderr, "**************next file*****************************************\n");

            if(Player[s32Index].PlayerCfg.cDirection)
            {
			    Player[s32Index].PlayerStatus.iPlayingID++;
            }
            else
            {
                Player[s32Index].PlayerStatus.iPlayingID--;
                if(Player[s32Index].PlayerStatus.iPlayingID < 0)
                {
                    Player[s32Index].PlayerStatus.iPlayingID = Player[s32Index].stOpenInfo.lFileCount;
                }
            }
		}

		if(Player[s32Index].PlayerStatus.iPlayingID == Player[s32Index].stOpenInfo.lFileCount)
	    {
            if(Player[s32Index].PlayerCfg.cLoop)
            {
	            fprintf(stderr, "**start loop:%x,%x******\n", 
						Player[s32Index].PlayerStatus.iPlayingID,
						Player[s32Index].stOpenInfo.lFileCount);

                if(Player[s32Index].PlayerCfg.cDirection)
                {
                    Player[s32Index].PlayerStatus.iPlayingID = 0;
					Player[s32Index].PlayerCfg.lTimePosition = Player[s32Index].stFileInfo.ulBeginTimeStamp;
					Player[s32Index].PlayerStatus.ulOffset = 0;
					Player[s32Index].PlayerCfg.ulOffset = 0;
					Player[s32Index].PlayerCfg.iGopNum = Player[s32Index].stOpenInfo.startGopNum;
					Player[s32Index].PlayerCfg.iNalNum = 0;
                }
                else
                {
                	Player[s32Index].PlayerStatus.iPlayingID = Player[s32Index].stOpenInfo.lFileCount - 1;
					Player[s32Index].PlayerCfg.lTimePosition = 0xffffffff;
					Player[s32Index].PlayerCfg.ulOffset = 0xffffffff;
					Player[s32Index].PlayerStatus.ulOffset = 0;
					Player[s32Index].PlayerCfg.iGopNum = -1;
					Player[s32Index].PlayerCfg.iNalNum = -1;
                }
            }
		   	else
			{
			    if(Player[s32Index].PlayerCfg.cDirection)
                {
					fprintf(stderr, "---%d--\n", __LINE__);
                    return ISIL_ERR_FILE_END;
                }
				else
                {
					fprintf(stderr, "---%d--\n", __LINE__);
                    return ISIL_ERR_FILE_BEGIN;
                }
			}
		}
	}

    fprintf(stderr, "---being read fileId is overflow---\n");

	if(Player[s32Index].PlayerCfg.cDirection)
    {
		fprintf(stderr, "---%d--\n", __LINE__);
        return ISIL_ERR_FILE_END;
    }
	else
    {
		fprintf(stderr, "---%d--\n", __LINE__);
        return ISIL_ERR_FILE_BEGIN;
    }
}

static int ISIL_FM_PLAY_ParsingFrameBackData(unsigned int u32id, PB_FRAME_INFO* frameinfo)
{
	int  iFileNum = 0;
	//int  iFileNumTmp = 0;
	int iGopNum = 0;
	int iNalNum = 0;
	int  iReadFd = 0;
	int  iReadIndexFd = 0;
	int  iRet = 0;
	ISIL_FILE_INDEX_NAL stFileIndexNal;
	ISIL_GOP_INDEX      stGopIndex;
	ISIL_GOP_NAL        stGopNal;
	ISIL_NAL_INDEX      stNalIndex;

	int s32IndexFd = 0;
	unsigned int u32DataOffset = 0;
	int s32RecIndexOffset = 0;
	int s32Index = 0;//player index

	//fprintf(stderr, "Enter: %s\n", __FUNCTION__);

	s32Index = ISIL_FM_PLAY_SearchPlayerByFd(u32id);
	if (s32Index == -1) {
		return ISIL_ERR_FAILURE;
	}

	if(Player[s32Index].PlayerCfg.cPause){
		frameinfo->nalcnt = 0;
		frameinfo->len = 0;
		return ISIL_ERR_SUCCESS;
	}

	while (Player[s32Index].PlayerStatus.iPlayingID < Player[s32Index].stOpenInfo.lFileCount) {
		iFileNum = Player[s32Index].PlayerStatus.iPlayingID;

		s32IndexFd = Player[s32Index].stOpenInfo.fileindexfd;
		iReadFd = Player[s32Index].stOpenInfo.datafd[iFileNum];
		iReadIndexFd = Player[s32Index].stOpenInfo.dataindexfd[iFileNum];

		u32DataOffset = Player[s32Index].stOpenInfo.datastart[iFileNum];
		s32RecIndexOffset = Player[s32Index].stOpenInfo.recindexstart[iFileNum];
		
		if(lseek(iReadIndexFd, s32RecIndexOffset, SEEK_SET) != s32RecIndexOffset)
		{
			fprintf(stderr, "--%s, %d fail--\n", __FUNCTION__, __LINE__);
			close(iReadIndexFd);
			close(iReadFd);
			close(s32IndexFd);
			return ISIL_ERR_FILE_READ;
		}
		
		iRet = read(iReadIndexFd, &stFileIndexNal, sizeof(stFileIndexNal));
		if(iRet < 0)
		{
			fprintf(stderr, "fun:%s,line:%d,read index file fail\n", __FUNCTION__, __LINE__);
			close(iReadFd);
			close(iReadIndexFd);
			close(s32IndexFd);
			return ISIL_ERR_FILE_READ;
		}
		#ifdef BIG_BYTES_ORDER
		Swap_FileIndexNal(&stFileIndexNal);
		#endif
		/*�ж�flag�Ƿ���ȷ*/
		if((stFileIndexNal.lIndexFlag != htonl(0x00000001)) || (stFileIndexNal.cIndexFlag != 0x79))
		{
			fprintf(stderr, "fun:%s,line:%d,%x,%x, index file flag error\n", __FUNCTION__, __LINE__, 
					stFileIndexNal.lIndexFlag, stFileIndexNal.cIndexFlag);
			close(iReadFd);
			close(iReadIndexFd);
			close(s32IndexFd);
			close(s32IndexFd);
			return ISIL_ERR_FILE_READ;
		}

		
		iGopNum = Player[s32Index].PlayerCfg.iGopNum;
		memset(&stGopIndex, 0x00, sizeof(ISIL_GOP_INDEX));

		while (iGopNum < stFileIndexNal.lGopCount) {

			iRet = lseek(iReadIndexFd, s32RecIndexOffset + sizeof(stFileIndexNal) + iGopNum * sizeof(stGopIndex), SEEK_SET);
			if (iRet < 0) {
				fprintf(stderr, "fun:%s,line:%d,lseek index fail\n", __FUNCTION__, __LINE__);
				close(iReadFd);
				close(iReadIndexFd);
				close(s32IndexFd);
				return ISIL_ERR_FILE_READ;
			}


			iRet = read(iReadIndexFd, &stGopIndex, sizeof(stGopIndex));
			if (iRet < 0) {
				fprintf(stderr, "fun:%s,line:%d,read gop index fail\n", __FUNCTION__, __LINE__);
				close(iReadFd);
				close(iReadIndexFd);
				close(s32IndexFd);
				return ISIL_ERR_FILE_READ;
			}

			#ifdef BIG_BYTES_ORDER
			//DEBUG_FUNCTION();
			Swap_GopIndex(&stGopIndex);
			#endif


			iRet = lseek(iReadFd, u32DataOffset + stGopIndex.lGopOffset, SEEK_SET);
			if (iRet < 0) {
				fprintf(stderr, "fun:%s,line:%d,lseek gop nal fail\n", __FUNCTION__, __LINE__);
				close(iReadFd);
				close(iReadIndexFd);
				close(s32IndexFd);
				return ISIL_ERR_FILE_READ;
			}

			iRet = read(iReadFd, &stGopNal, sizeof(stGopNal));
			if (iRet < 0) {
				fprintf(stderr, "fun:%s,line:%d,read gop nal fail\n", __FUNCTION__, __LINE__);
				close(iReadFd);
				close(iReadIndexFd);
				close(s32IndexFd);
				return ISIL_ERR_FILE_READ;
			}

			#ifdef BIG_BYTES_ORDER
			Swap_GopNal(&stGopNal);
			#endif

			//fprintf(stderr,"stGopNal.lGopFlag = 0x%lux stGopNal.cGopFlag = %d .\n",stGopNal.lGopFlag , stGopNal.cGopFlag);

			if ((stGopNal.lGopFlag != htonl(0x00000001)) || (stGopNal.cGopFlag != 0x78)) {
				fprintf(stderr, "fun:%s,line:%d,%x, %x, gop nal flag error\n", __FUNCTION__, __LINE__, 
						stGopNal.lGopFlag, stGopNal.cGopFlag);
				close(iReadFd);
				close(iReadIndexFd);
				close(s32IndexFd);
				return ISIL_ERR_FILE_READ;
			}


			if (!Player[s32Index].PlayerCfg.cDirection) {
				if (Player[s32Index].PlayerCfg.iNalNum < 0) {
					Player[s32Index].PlayerCfg.iNalNum = stGopNal.nNalCount - 1;
				}
			}

			iNalNum = Player[s32Index].PlayerCfg.iNalNum;
			while (iNalNum < stGopNal.nNalCount) {
				READSPS:    iRet = lseek(iReadFd, u32DataOffset + stGopIndex.lGopOffset + sizeof(stGopNal) + iNalNum * sizeof(stNalIndex), SEEK_SET);
				if (iRet < 0) {
					fprintf(stderr, "fun:%s,line:%d,lseek nal index fail\n", __FUNCTION__, __LINE__);
					close(iReadFd);
					close(iReadIndexFd);
					close(s32IndexFd);
					return ISIL_ERR_FILE_READ;
				}

				iRet = read(iReadFd, &stNalIndex, sizeof(stNalIndex));
				if ( iRet < 0) {
					fprintf(stderr, "fun:%s,line:%d,read nal index fail\n", __FUNCTION__, __LINE__);
					close(iReadFd);
					close(iReadIndexFd);
					close(s32IndexFd);
					return ISIL_ERR_FILE_READ;
				}

				#ifdef BIG_BYTES_ORDER
				Swap_NALIndex(&stNalIndex);
				#endif

				//fprintf(stderr, "fb,type:%x,N:%x,G:%x ", stNalIndex.cNalType, 
				//										iNalNum, iGopNum);
				char cFindData = 0;
				
				if ( Player[s32Index].PlayerStatus.cFrameBack\
					 &&(Player[s32Index].PlayerCfg.lTimePosition <= (stNalIndex.lTimeStamp + 100))\
					 &&(Player[s32Index].PlayerCfg.ulOffset <= stNalIndex.lNalOffset)
				   ) {
					cFindData = 1;
				}

				if(cFindData)
	            {
	                if((stNalIndex.cNalType == E_H264_FRAME_TYPE_MAIN_PPS)
                       ||(stNalIndex.cNalType == E_H264_FRAME_TYPE_SUB_PPS))
	                {
	                    iNalNum -= 1;
	                    goto READSPS;
	                }

	                if((stNalIndex.cNalType == E_H264_FRAME_TYPE_MAIN_IDR)
                       ||(stNalIndex.cNalType == E_H264_FRAME_TYPE_SUB_IDR))
	                {
	                    iNalNum -= 2;
	                    goto READSPS;
	                }

					char cNalEqual = 0;
					if(Player[s32Index].PlayerStatus.iNalNum == (iNalNum + 1))
					{
						cNalEqual = 1;
					}
					else if(((stNalIndex.cNalType == E_H264_FRAME_TYPE_MAIN_SPS) 
                             || (stNalIndex.cNalType == E_H264_FRAME_TYPE_SUB_SPS)) 
                            && (Player[s32Index].PlayerStatus.iNalNum == (iNalNum + 1 + 2)))
					{
						cNalEqual = 1;
					}

					if((Player[s32Index].PlayerStatus.iGopNum == iGopNum) && cNalEqual)
	                {
						Player[s32Index].PlayerStatus.cFrameBack = 0;
						Player[s32Index].PlayerCfg.iGopNum = Player[s32Index].PlayerStatus.iGopNum;
						Player[s32Index].PlayerCfg.iNalNum = (iNalNum - 1);

						Player[s32Index].PlayerStatus.iNalNum = iNalNum;

						if(Player[s32Index].PlayerCfg.iNalNum < 0)
    	                {
							Player[s32Index].PlayerCfg.iGopNum--;
							Player[s32Index].PlayerCfg.iNalNum = -1;

							Player[s32Index].PlayerStatus.iGopNum = Player[s32Index].PlayerCfg.iGopNum;
							Player[s32Index].PlayerStatus.iNalNum = -1;
    					}
					}

	                int iNalCnt = 0;
	                int i = 0;

	                if((stNalIndex.cNalType == E_H264_FRAME_TYPE_MAIN_SPS)
                        ||(stNalIndex.cNalType == E_H264_FRAME_TYPE_SUB_SPS))
	                {
	                    iNalCnt = 3;
	                }
	                else
	                {
	                    iNalCnt = 1;
	                }

                    frameinfo->nalcnt = iNalCnt;
                    frameinfo->fd = iReadFd;
                    frameinfo->offset = u32DataOffset + stNalIndex.lNalOffset;
                    frameinfo->ts = stNalIndex.lTimeStamp;
                    frameinfo->len = 0;

	                for(i = 0; i < iNalCnt; i++)
	                {
                        switch(stNalIndex.cNalType)
                        {
                            case E_H264_FRAME_TYPE_MAIN_SPS:
                            case E_H264_FRAME_TYPE_SUB_SPS:
                            {
                                frameinfo->nal[0].naltype = stNalIndex.cNalType;
                                frameinfo->nal[0].nallen = stNalIndex.lNalSize;
                            }
                            break;

                        	case E_H264_FRAME_TYPE_MAIN_PPS:
                            case E_H264_FRAME_TYPE_SUB_PPS:
                            {
                                frameinfo->nal[1].naltype = stNalIndex.cNalType;
                                frameinfo->nal[1].nallen = stNalIndex.lNalSize;
                            }
                            break;

                        	case E_H264_FRAME_TYPE_MAIN_IDR:
                            case E_H264_FRAME_TYPE_SUB_IDR:
                            {
                               frameinfo->nal[2].naltype = stNalIndex.cNalType;
                               frameinfo->nal[2].nallen = stNalIndex.lNalSize;
                            }
                            break;

                            default:
                            {
                                frameinfo->nal[0].naltype = stNalIndex.cNalType;
                                frameinfo->nal[0].nallen = stNalIndex.lNalSize;
                            }
                            break;
                        }

						if((stNalIndex.cNalType != E_H264_FRAME_TYPE_MAIN_PPS) 
                           && (stNalIndex.cNalType != E_H264_FRAME_TYPE_MAIN_SPS)
                           && (stNalIndex.cNalType != E_H264_FRAME_TYPE_SUB_SPS)
                           && (stNalIndex.cNalType != E_H264_FRAME_TYPE_SUB_SPS))
						{
							Player[s32Index].PlayerStatus.ulOffset 		= stNalIndex.lNalOffset;
							Player[s32Index].PlayerCfg.ulOffset     	= (stNalIndex.lNalOffset + stNalIndex.lNalSize);

                            if((stNalIndex.cNalType < E_FRAME_TYPE_AUDIO)
                               ||(stNalIndex.cNalType > E_FRAME_TYPE_AUDIO_END))
                            {
							    Player[s32Index].PlayerCfg.lTimePosition 	= stNalIndex.lTimeStamp;
							    Player[s32Index].PlayerStatus.lTimePosition = stNalIndex.lTimeStamp;
                            }
							if(Player[s32Index].PlayerStatus.cFrameBack)
							{
								Player[s32Index].PlayerCfg.iGopNum 		= iGopNum;
								Player[s32Index].PlayerCfg.iNalNum 		= iNalNum + 1;
							}
						}

                        frameinfo->len += stNalIndex.lNalSize;
                        if((iNalCnt == 3) && (i < 2))
                        {
                            iNalNum++;
                            iRet = lseek(iReadFd, u32DataOffset + stGopIndex.lGopOffset + sizeof(stGopNal) + iNalNum * sizeof(stNalIndex), SEEK_SET);
                            if(iRet < 0)
                            {
                                fprintf(stderr, "fun:%s,line:%d,lseek nal index fail\n", __FUNCTION__, __LINE__);
                                close(iReadFd);
                                close(iReadIndexFd);
                                close(s32IndexFd);
                                return ISIL_ERR_FILE_READ;
                            }

                            iRet = read(iReadFd, &stNalIndex, sizeof(stNalIndex));
                            if(iRet < 0)
                            {
                                fprintf(stderr, "fun:%s,line:%d,read nal index fail\n", __FUNCTION__, __LINE__);
                                close(iReadFd);
                                close(iReadIndexFd);
                                close(s32IndexFd);
                                return ISIL_ERR_FILE_READ;
                            }

							#ifdef BIG_BYTES_ORDER
                            Swap_NALIndex(&stNalIndex);
                            #endif
                        }
	                }

					if(Player[s32Index].PlayerStatus.cFrameBack)
                	{
						fprintf(stderr, "---return fb---\n");
						return ISIL_ERR_FILE_REMAIN_FRAME;
                	}
                	else
                	{
                        Player[s32Index].PlayerStatus.lFrameCount++;
						fprintf(stderr, "---fb ok,gop:%x, nal:%x, %x, %x--\n",
											iGopNum, iNalNum,
											Player[s32Index].PlayerStatus.iGopNum,
											Player[s32Index].PlayerStatus.iNalNum);
						return ISIL_ERR_SUCCESS;
                    }
                }

				iNalNum++;
	        }

		    iGopNum++;
			Player[s32Index].PlayerCfg.iNalNum = 0;
			Player[s32Index].PlayerCfg.iGopNum = iGopNum;
	    }

		if(iGopNum >= stFileIndexNal.lGopCount)
		{
			Player[s32Index].PlayerStatus.iPlayingID++;
		}

		if(Player[s32Index].PlayerStatus.iPlayingID == Player[s32Index].stOpenInfo.lFileCount)
	    {
            if(Player[s32Index].PlayerCfg.cLoop)
            {
                fprintf(stderr, "*****start loop******\n");
                Player[s32Index].PlayerStatus.iPlayingID = 0;
                Player[s32Index].PlayerCfg.lTimePosition = Player[s32Index].stFileInfo.ulBeginTimeStamp;
                Player[s32Index].PlayerStatus.ulOffset = 0;
                Player[s32Index].PlayerCfg.ulOffset = 0;
				Player[s32Index].PlayerCfg.iGopNum = Player[s32Index].stOpenInfo.startGopNum;
                Player[s32Index].PlayerCfg.iNalNum = 0;
            }
            else
            {
                return ISIL_ERR_FILE_BEGIN;
            }

		}
	}

	fprintf(stderr, "---being read fileId is overflow---\n");
	return ISIL_ERR_FILE_BEGIN;
}

int ISIL_FM_PLAY_ParsingFile(unsigned int winid, PB_FRAME_INFO* frameinfo)
{
	int s32Ret = 0;
	s32Ret = ISIL_FM_PLAY_ParsingFrameData(winid, frameinfo);
	if (s32Ret == ISIL_ERR_FILE_REMAIN_FRAME) {
		fprintf(stderr, "enter mdoe: frameback\n");
		s32Ret = ISIL_FM_PLAY_ParsingFrameBackData(winid, frameinfo);
	}
	
	//fprintf(stderr, "--cnt[%x],ts[%x]--\n", frameinfo->nalcnt, frameinfo->ts);

	return s32Ret;
}

int ISIL_FM_PLAY_GetFileData(int filefd, size_t offset, size_t datalen, void* buff)
{
	int ret;
	unsigned int off = 0;
	if (!buff) {
		fprintf(stderr, "---buff is null--\n");
		return -1;
	}

	off = lseek(filefd, offset, SEEK_SET);
	//fprintf(stderr,"lseek off = 0x%x .\n",off);
	if ( off != offset) {
		fprintf(stderr, "---seek fail---\n");
		return -1;
	}

	ret = r_read(filefd, buff, datalen);
	return ret;
}
#endif


enum play_state{
	PLAY_UNKOWN = 0x00,//unkown
	PLAY_FW 	= 0x01,//play x forward
	PLAY_KFW 	= 0x02,//key frame forward
	PLAY_BW  	= 0x03,//play x backward 
	PLAY_KBW	= 0x04,//key frame backward
	PLAY_FF		= 0x05,//frame forward
	PLAY_FB		= 0x06,//frame backward
	PLAY_PAUSE  = 0x07,//pause
};


static int ISIL_FM_PLAY_GetState(unsigned int u32id, unsigned int *pState)
{
	int idx = 0;
	unsigned int state = PLAY_UNKOWN;

	idx = ISIL_FM_PLAY_SearchPlayerByFd(u32id);
	if (idx == -1) {
		fprintf(stderr, "---%s,%d--\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if(Player[idx].PlayerCfg.cPause){
		state = PLAY_PAUSE;
		goto EXIT;
	}

	if(Player[idx].PlayerCfg.cDirection){
		if(Player[idx].PlayerCfg.cKeyFrame){
			state = PLAY_KFW;
		}else{
			state = PLAY_FW;
		}
	}else{
		if(Player[idx].PlayerCfg.cKeyFrame){
			state = PLAY_KBW;
		}else{
			state = PLAY_BW;
		}
	}
	
EXIT:		
	*pState = state;
	//fprintf(stderr, "--play state: %x--\n",*pState);
	return 0;
}

static int ISIL_FM_PLAY_ParsingFrameData(unsigned int u32id, PB_FRAME_INFO* frameinfo)
{
	int iFileNum = 0;
	int iRet = 0;
	int iGopNum = 0;
	int iNalNum = 0;

	int iReadFd = 0;
	int iReadIndexFd = 0;
	int s32IndexFd = 0;
	unsigned int u32DataOffset = 0;//this complement file 's first frame offset
	int s32RecIndexOffset = 0;

	ISIL_FILE_INDEX_NAL stFileIndexNal;
	ISIL_GOP_INDEX      stGopIndex;
	ISIL_GOP_NAL        stGopNal;
	ISIL_NAL_INDEX      stNalIndex;
	
	int s32Index = 0;//player index
	enum play_state estate = PLAY_UNKOWN;
	char isloop = 0;

	s32Index = ISIL_FM_PLAY_SearchPlayerByFd(u32id);
	if (s32Index == -1) {
		fprintf(stderr, "---%d--\n", __LINE__);
		iRet = ISIL_ERR_FAILURE;
		goto EXIT;
	}

	isloop = Player[s32Index].PlayerCfg.cLoop;

	if(ISIL_FM_PLAY_GetState(u32id, (unsigned int *)&estate) < 0){
		fprintf(stderr, "---%d--\n", __LINE__);
		iRet = ISIL_ERR_FAILURE;
		goto EXIT;
	}

	if(estate == PLAY_PAUSE){
		frameinfo->nalcnt = 0;
		frameinfo->len = 0;
		iRet = ISIL_ERR_SUCCESS;
		goto EXIT;
	}

	iFileNum = Player[s32Index].PlayerStatus.iPlayingID;

	if((estate == PLAY_FW) || (estate == PLAY_KFW)){
		iGopNum = Player[s32Index].PlayerCfg.iGopNum;
		iNalNum = Player[s32Index].PlayerCfg.iNalNum + 1;
	}else if((estate == PLAY_BW) || (estate == PLAY_KBW)){
		iGopNum = Player[s32Index].PlayerStatus.iGopNum;
		iNalNum = Player[s32Index].PlayerStatus.iNalNum - 1;
		if(iNalNum < 0){
			iGopNum--;
			if((iGopNum < 0) && !isloop){
				iRet = ISIL_ERR_FILE_BEGIN;
				goto EXIT;
			}
		}
	}

	while (iFileNum < Player[s32Index].stOpenInfo.lFileCount) {

		s32IndexFd = Player[s32Index].stOpenInfo.fileindexfd;
		iReadFd = Player[s32Index].stOpenInfo.datafd[iFileNum];
		iReadIndexFd = Player[s32Index].stOpenInfo.dataindexfd[iFileNum];

		u32DataOffset = Player[s32Index].stOpenInfo.datastart[iFileNum];
		s32RecIndexOffset = Player[s32Index].stOpenInfo.recindexstart[iFileNum];

		if(lseek(iReadIndexFd, s32RecIndexOffset, SEEK_SET) != s32RecIndexOffset)
	    {
			fprintf(stderr, "---%d--\n", __LINE__);
	        iRet = ISIL_ERR_FILE_READ;
			goto EXIT;
	    }

	    iRet = read(iReadIndexFd, &stFileIndexNal, sizeof(stFileIndexNal));
	    if(iRet < 0)
	    {
	        fprintf(stderr, "fun:%s,line:%d,read index file fail\n", __FUNCTION__, __LINE__);
	        iRet = ISIL_ERR_FILE_READ;
			goto EXIT;
	    }

		#ifdef BIG_BYTES_ORDER
        Swap_FileIndexNal(&stFileIndexNal);
		#endif

	    /*�ж�flag�Ƿ���ȷ*/
	    if((stFileIndexNal.lIndexFlag != htonl(0x00000001)) || (stFileIndexNal.cIndexFlag != 0x79))
	    {
	        fprintf(stderr, "fun:%s,line:%d,%lx,%x, index file flag error\n", __FUNCTION__, __LINE__, 
					stFileIndexNal.lIndexFlag, stFileIndexNal.cIndexFlag);
	        iRet = ISIL_ERR_FILE_READ;
			goto EXIT;
	    }

		if(iGopNum < 0){
			iGopNum = stFileIndexNal.lGopCount - 1;
		}

		while (iGopNum < stFileIndexNal.lGopCount) {

			if(iGopNum < 0){
				fprintf(stderr, "---%d--\n", __LINE__);
				break;
			}
			
			iRet = lseek(iReadIndexFd, s32RecIndexOffset + sizeof(stFileIndexNal) + iGopNum * sizeof(stGopIndex), SEEK_SET);
			if (iRet < 0) {
				fprintf(stderr, "fun:%s,line:%d,lseek index fail\n", __FUNCTION__, __LINE__);
				iRet = ISIL_ERR_FILE_READ;
				goto EXIT;
			}

			iRet = read(iReadIndexFd, &stGopIndex, sizeof(stGopIndex));
			if (iRet < 0) {
				fprintf(stderr, "fun:%s,line:%d,read gop index fail\n", __FUNCTION__, __LINE__);
				iRet = ISIL_ERR_FILE_READ;
				goto EXIT;
			}

			#ifdef BIG_BYTES_ORDER
			//DEBUG_FUNCTION();
			Swap_GopIndex(&stGopIndex);
			#endif

			iRet = lseek(iReadFd, u32DataOffset + stGopIndex.lGopOffset, SEEK_SET);
			if (iRet < 0) {
				fprintf(stderr, "fun:%s,line:%d,lseek gop nal fail\n", __FUNCTION__, __LINE__);
				iRet = ISIL_ERR_FILE_READ;
				goto EXIT;
			}

			iRet = read(iReadFd, &stGopNal, sizeof(stGopNal));
			if (iRet < 0) {
				fprintf(stderr, "fun:%s,line:%d,read gop nal fail\n", __FUNCTION__, __LINE__);
				iRet = ISIL_ERR_FILE_READ;
				goto EXIT;
			}

			#ifdef BIG_BYTES_ORDER
			Swap_GopNal(&stGopNal);
			#endif

			if ((stGopNal.lGopFlag != htonl(0x00000001)) || (stGopNal.cGopFlag != 0x78)) {
				fprintf(stderr, "fun:%s,line:%d,%lx,%x,  gop nal flag error\n", __FUNCTION__, __LINE__, 
						stGopNal.lGopFlag, stGopNal.cGopFlag);
				iRet = ISIL_ERR_FILE_READ;
				goto EXIT;
			}

			if(iNalNum < 0){
				iNalNum = stGopNal.nNalCount - 1;
			}

			while (iNalNum < stGopNal.nNalCount) {
				
				if(iNalNum < 0){
					fprintf(stderr, "---%d--\n", __LINE__);
					break;
				}
				
				iRet = lseek(iReadFd, u32DataOffset + stGopIndex.lGopOffset + sizeof(stGopNal) + iNalNum * sizeof(stNalIndex), SEEK_SET);
				if (iRet < 0) {
					fprintf(stderr, "fun:%s,line:%d,lseek nal index fail\n", __FUNCTION__, __LINE__);
					iRet = ISIL_ERR_FILE_READ;
					goto EXIT;
				}

				iRet = read(iReadFd, &stNalIndex, sizeof(stNalIndex));
				if (iRet < 0) {
					fprintf(stderr, "fun:%s,line:%d,read nal index fail\n", __FUNCTION__, __LINE__);
					iRet = ISIL_ERR_FILE_READ;
					goto EXIT;
				}

				#ifdef BIG_BYTES_ORDER
				Swap_NALIndex(&stNalIndex);
				#endif

                if((stNalIndex.cNalType == E_H264_FRAME_TYPE_MAIN_PPS)
                   || (stNalIndex.cNalType == E_H264_FRAME_TYPE_SUB_PPS))
                {
                    iNalNum -= 1;
                    continue;
                }

                if((stNalIndex.cNalType == E_H264_FRAME_TYPE_MAIN_IDR)
                   || (stNalIndex.cNalType == E_H264_FRAME_TYPE_SUB_IDR))
                {
                    iNalNum -= 2;
                    continue;
                }	

                if((estate == PLAY_KFW) || (estate == PLAY_KBW))
                {
                    if((stNalIndex.cNalType != E_H264_FRAME_TYPE_MAIN_SPS)
                       && (stNalIndex.cNalType != E_H264_FRAME_TYPE_SUB_SPS))
                    {
						if(estate == PLAY_KFW){
                            iNalNum++;
						}
						else{
							iNalNum--;
						}
                        continue;
                    }
                }

				Player[s32Index].PlayerStatus.iGopNum = iGopNum;
				Player[s32Index].PlayerStatus.iNalNum = iNalNum;

                int iNalCnt = 0;
                int i = 0;
				
                if((stNalIndex.cNalType == E_H264_FRAME_TYPE_MAIN_SPS)
                   ||(stNalIndex.cNalType == E_H264_FRAME_TYPE_SUB_SPS))
                {
                    iNalCnt = 3;
                }
                else
                {
                    iNalCnt = 1;
                }

                frameinfo->nalcnt = iNalCnt;
                frameinfo->fd = iReadFd;
                frameinfo->offset = u32DataOffset + stNalIndex.lNalOffset;
                frameinfo->ts = stNalIndex.lTimeStamp;
                frameinfo->len = 0;

                for(i = 0; i < iNalCnt; i++)
                {
                    switch(stNalIndex.cNalType)
                    {
                        case E_H264_FRAME_TYPE_MAIN_SPS:
                        case E_H264_FRAME_TYPE_SUB_SPS:
                        {
                            frameinfo->nal[0].naltype = (eframe_type)stNalIndex.cNalType;
                            frameinfo->nal[0].nallen = stNalIndex.lNalSize;
                        }
                        break;

                    	case E_H264_FRAME_TYPE_MAIN_PPS:
                        case E_H264_FRAME_TYPE_SUB_PPS:
                        {
                            frameinfo->nal[1].naltype = (eframe_type)stNalIndex.cNalType;
                            frameinfo->nal[1].nallen = stNalIndex.lNalSize;
                        }
                        break;

                    	case E_H264_FRAME_TYPE_MAIN_IDR:
                        case E_H264_FRAME_TYPE_SUB_IDR:
                        {
                           frameinfo->nal[2].naltype = (eframe_type)stNalIndex.cNalType;
                           frameinfo->nal[2].nallen = stNalIndex.lNalSize;
                        }
                        break;

                        default:
                        {
                            frameinfo->nal[0].naltype = (eframe_type)stNalIndex.cNalType;
                            frameinfo->nal[0].nallen = stNalIndex.lNalSize;
                        }
                        break;
                    }

                    frameinfo->len += stNalIndex.lNalSize;
                    if((iNalCnt == 3) && (i < 2))
                    {
                        iNalNum++;
                        iRet = lseek(iReadFd, u32DataOffset + stGopIndex.lGopOffset + sizeof(stGopNal) + iNalNum * sizeof(stNalIndex), SEEK_SET);
                        if(iRet < 0)
                        {
                            fprintf(stderr, "fun:%s,line:%d,lseek nal index fail\n", __FUNCTION__, __LINE__);
                            iRet = ISIL_ERR_FILE_READ;
							goto EXIT;
                        }

                        iRet = read(iReadFd, &stNalIndex, sizeof(stNalIndex));
                        if(iRet < 0)
                        {
                            fprintf(stderr, "fun:%s,line:%d,read nal index fail\n", __FUNCTION__, __LINE__);
                            iRet = ISIL_ERR_FILE_READ;
							goto EXIT;
                        }
                        #ifdef BIG_BYTES_ORDER
                        Swap_NALIndex(&stNalIndex);
                        #endif
                    }
                }

                Player[s32Index].PlayerStatus.lFrameCount++; 
				Player[s32Index].PlayerCfg.iGopNum = iGopNum;
				Player[s32Index].PlayerCfg.iNalNum = iNalNum;

				#if 0
				fprintf(stderr, "--cnt[%x],ts[%x]--\n", frameinfo->nalcnt, frameinfo->ts);

				fprintf(stderr, "cf:G[%x]N[%x],st:G[%x]N[%x]\n",
						Player[s32Index].PlayerCfg.iGopNum, Player[s32Index].PlayerCfg.iNalNum,
						Player[s32Index].PlayerStatus.iGopNum, Player[s32Index].PlayerStatus.iNalNum);
				#endif
				iRet = ISIL_ERR_SUCCESS;
				goto EXIT;
	        }

			if(iNalNum >= stGopNal.nNalCount){
				iNalNum = 0;
				iGopNum++; 
			}else if(iNalNum < 0){
				iGopNum--;
				
				if(iGopNum < 0){
					fprintf(stderr, "---%d--\n", __LINE__);
					break;
				}
			}	
	    }

		if(iGopNum < 0)//backward
		{
            iFileNum--;
			if(iFileNum < 0){
				if(isloop){
					fprintf(stderr, "---backward loop--\n");
					iFileNum = Player[s32Index].stOpenInfo.lFileCount - 1;
				}else{
					break;
				}
			}
		}else{
			iFileNum++;
			if(iFileNum >= Player[s32Index].stOpenInfo.lFileCount){
				if(isloop){
					fprintf(stderr, "---forward loop--\n");
					iFileNum = 0;
				}else break;
			}
			iGopNum = Player[s32Index].stOpenInfo.startGopNum[iFileNum];
		}
	}

    fprintf(stderr, "---being read fileId is overflow,%d, %d---\n",iFileNum, Player[s32Index].stOpenInfo.lFileCount);

	if(iFileNum < 0){
		fprintf(stderr, "---%d--\n", __LINE__);
	    iRet = ISIL_ERR_FILE_BEGIN;
		goto EXIT;
    }else{
		fprintf(stderr, "---%d--\n", __LINE__);
        iRet = ISIL_ERR_FILE_END;
		goto EXIT;
    }

EXIT:
	if((iRet == ISIL_ERR_FILE_BEGIN) || (iRet == ISIL_ERR_FILE_END)){
		ISIL_FM_PLAY_SetStop(u32id);
	}
	
	Player[s32Index].PlayerStatus.iPlayingID = iFileNum;
	return iRet;
}


int ISIL_FM_PLAY_ParsingFile(unsigned int winid, PB_FRAME_INFO* frameinfo)
{
	int s32Ret = 0;
	s32Ret = ISIL_FM_PLAY_ParsingFrameData(winid, frameinfo);
	//fprintf(stderr, "--cnt[%x],ts[%x]--\n", frameinfo->nalcnt, frameinfo->ts);

	return s32Ret;
}

int ISIL_FM_PLAY_GetFileData(int filefd, size_t offset, size_t datalen, void* buff)
{
	int ret;
	unsigned int off = 0;
	if (!buff) {
		fprintf(stderr, "---buff is null--\n");
		return -1;
	}

	off = lseek(filefd, offset, SEEK_SET);
	//fprintf(stderr,"lseek off = 0x%x .\n",off);
	if ( off != offset) {
		fprintf(stderr, "---seek fail---\n");
		return -1;
	}

	ret = read(filefd, (void *)buff, datalen);
	return ret;
}



