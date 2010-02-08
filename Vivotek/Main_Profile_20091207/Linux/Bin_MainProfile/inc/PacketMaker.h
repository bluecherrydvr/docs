#ifndef _PACKET_MAKER_H 
#define _PACKET_MAKER_H

#include <windows.h>
#include <time.h>

#include "typedef.h"
#include "common.h"
#include "errordef.h"
#include "mainprofile_error.h"
#include "datapacketdef.h"

#define PACKET_MAKER_VERSION	MAKEFOURCC ( 2, 2, 0, 1 ) 

typedef enum emPMChOption
{
	eChOptAudioCodec = 1,
	eChOptAudioSampleFreq = 2,
	eChOptAudioChannelNum = 3,
	eChOptAudioFramePerPacket = 4,
	eChOptAudioSizePerFrame = 5,
	eChOptAudioMute = 6,
	eChOptBitsPerSample = 7,
	eChOptPacketCallback = 8,	// dwParam1 = functin pointer, dwParam2 = context
	eChOptStatusCallback = 9,	// dwParam1 = functin pointer, dwParam2 = context
} EMPMCHOPTION;

typedef enum emPMUpdateOption
{
	eUpChOptAudioVolume = 1,	//dwParam1 = volume(0 ~ 100)
}EPMUPDATEOPTION;

typedef struct tPmChannelOption
{
	DWORD dwFlag;
	DWORD dwAudioCodec;
	DWORD dwAudioSampleFreq;
	DWORD dwAudioChannelNum;
	DWORD dwAudioFramesPerPacket;
	DWORD dwAudioSizePerFrame;
} TPMCHANNELOPTION;

typedef SCODE (__stdcall * LPACAPSTATUSCALLBACK)(DWORD dwContext, DWORD dwStatusCode, DWORD dwParam1, DWORD dwParam2);
typedef SCODE (__stdcall * LPACAPPACKETCALLBACK)(DWORD dwContext, TMediaDataPacketInfo *ptMediaPacket, DWORD dwDataTimePeriod);

typedef struct tPmACapChannelOption
{
	DWORD dwFlag;
	DWORD dwAudioCodec;
	DWORD dwAudioSampleFreq;
	DWORD dwAudioChannelNum;
	DWORD dwAudioFramesPerPacket;
	DWORD dwAudioSizePerFrame;
	
	DWORD dwBitsPerSample;

	DWORD					dwStatusContext;
	DWORD					dwPacketContext;

	LPACAPSTATUSCALLBACK	pfStatusCB;
	LPACAPPACKETCALLBACK	pfPacketCB;
} TPMACAPCHANNELOPTION;

/*! Date/time information for data structure */
typedef struct {
    //! year in A.D. from 1970 ~ 2034
    WORD    wYear;
    //! month of year. form 1 ~ 12
    WORD    wMonth;
    //! day of month. from 1 ~ 31
    WORD    wMonthDay;
    //! day of week. from 0(Sun) ~ 6(Sat)
    WORD    wWeekDay;
    //! hour of day. from 0 ~ 23
    WORD    wHour;
    //! minutes of hour. from 0 ~ 59
    WORD    wMinute;
    //! second of minute. from 0 ~ 59
    WORD    wSecond;
	//! milliSecond of second
    WORD    wMilliSecond;

} TPacketDateTimeInfo;

#ifdef __cplusplus
extern "C" {
#endif

SCODE DLLAPI PacketMaker_Initial (HANDLE * phPacketMaker, DWORD dwFlag, DWORD dwVersion);
SCODE DLLAPI PacketMaker_Release (HANDLE * phPacketMaker);
SCODE DLLAPI PacketMaker_GetMaxHeader (HANDLE hPacketMaker, DWORD *pdwMaxHeader);

SCODE DLLAPI PacketMaker_CreateChannel (HANDLE hPacketMaker, HANDLE * phChannel, TPMCHANNELOPTION *ptChannelOptions);
SCODE DLLAPI PacketMaker_DeleteChannel (HANDLE * phChannel);

SCODE DLLAPI PacketMaker_SetChannelOption (HANDLE hChannel, DWORD dwOption, DWORD dwParam1, DWORD dwParam2);

SCODE DLLAPI PacketMaker_MakeVideoPacket (HANDLE hChannel, BYTE *pbyBuffer, DWORD dwBuffSize, 
										  DWORD dwDataOffset, TMediaDataPacketInfo *ptMediaPacket);
SCODE DLLAPI PacketMaker_MakeAudioPacket (HANDLE hChannel, BYTE *pbySrcBuffer, DWORD dwBuffSize, 
										  DWORD dwTargetBuffSize,
										  DWORD *pdwUsedSize, TMediaDataPacketInfo *ptMediaPacket);
//bruce add 20051213 
SCODE DLLAPI PacketMaker_SetPacketTime(TMediaDataPacketInfo *ptMediaPacket, TPacketDateTimeInfo tpktTime);
#if defined(_WIN32) || defined(_WIN32_WCE)
SCODE DLLAPI PacketMaker_CreateAudioCaptureChannel (HANDLE hPacketMaker, HANDLE * phChannel, 
													TPMACAPCHANNELOPTION *ptChannelOptions );

SCODE DLLAPI PacketMaker_DeleteAudioCaptureChannel (HANDLE * phChannel);

SCODE DLLAPI PacketMaker_SetAudioCaptureChannelOption (HANDLE hChannel, DWORD dwOption, DWORD dwParam1, DWORD dwParam2);
SCODE DLLAPI PacketMaker_StartAudioCapture (HANDLE hChannel);
SCODE DLLAPI PacketMaker_StopAudioCapture (HANDLE hChannel);
SCODE DLLAPI PacketMaker_UpdateChannelSettings (HANDLE hChannel, DWORD dwOption, DWORD dwParam1, DWORD dwParam2);
#endif // _WIN32 || _WIN32_WCE

#ifdef __cplusplus
}
#endif

#endif // _PACKET_MAKER_H
