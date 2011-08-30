#ifndef __SOLO6010_EXAMPLE_REC_FILE_INFO_H__
#define __SOLO6010_EXAMPLE_REC_FILE_INFO_H__

struct INFO_DATE
{
	WORD nYear;
	BYTE nMonth;
	BYTE nDay;
	BYTE nHour;
	BYTE nMin;
	BYTE nSec;
};

#define SZ_STR_INIT	12
#define SZ_STR_INIT_COMMON	9
#define STR_MP4_FILE	"S6010BYSLMP4"
#define STR_JPEG_FILE	"S6010BYSLJPG"
#define STR_G723_FILE	"S6010BYSLG72"

#pragma pack(push, 4)

struct HEADER_RECORD_FILE_MP4
{
	SETTING_MP4_REC bufSetRec[NUM_VID_TOTAL_ENC_CH];
	DWORD bufCRU[NUM_VID_TOTAL_ENC_CH];

	BYTE typeVideo;
	BYTE bExistMotionData;
	BYTE bUseIndex;
	DWORD numFrm;
	__int64 posIndex;
};

struct HEADER_RECORD_FILE_JPEG
{
	SETTING_JPEG_REC bufSetRec[NUM_VID_TOTAL_ENC_CH];
	BYTE bufIdxImgSz[NUM_VID_TOTAL_ENC_CH];
	BYTE typeVideo;
	DWORD numFrm;
	__int64 posIndex;
};

struct HEADER_RECORD_FILE_G723
{
	SETTING_G723_REC setRec;
	DWORD numFrm;
	__int64 posIndex;
};

struct HEADER_RECORD_FILE
{
	char strFileInit[SZ_STR_INIT];
	INFO_DATE dateStart;
	INFO_DATE dateEnd;

	union
	{
		HEADER_RECORD_FILE_MP4 MP4;
		HEADER_RECORD_FILE_JPEG JPEG;
		HEADER_RECORD_FILE_G723 G723;
		BYTE bufReserved[2048];
	};
};

#define SZ_G723_FRM_HEADER		SZ_G723_TIME_INFO

struct ITEM_VID_REC_FILE_INDEX
{
	BYTE idxCh;
	BYTE isKeyFrm;
	BYTE reserved1;
	BYTE reserved2;
	DWORD timeSec;
	DWORD timeUSec;
	__int64 posFrm;
};
struct ITEM_AUD_REC_FILE_INDEX
{
	DWORD timeSec;
	DWORD timeUSec;
	DWORD posFrm;
};
#define NUM_INIT_ITEM_INDEX		(256 *1024)

#pragma pack(pop, 4)

#endif
