
#ifndef _SRVDEPRESOURCE_H_
#define _SRVDEPRESOURCE_H_

/*
#ifdef _USRDLL
#define SRVDEP_API __declspec(dllexport)
#else
#define SRVDEP_API
#endif
*/
#if defined (_WIN32_WCE)
#include <windows.h>
#endif // _WIN32_WCE
#include "typedef.h"
#include "common.h"
#include "errordef.h"
#include "SrvTypeDef.h"


#define SRVDEPRESOURCE_VERSION	MAKEFOURCC(4,1,0,7)

// the flag for various item enable or not
#define	SRV_FLAG_PTZ_SPEED			1
#define	SRV_FLAG_PTZ_ADV			2	// for focus
#define	SRV_FLAG_PT					4	// for focus


#define	SDS_MAX_MODEL_LEN			20
#define	SDS_MAX_HOSTNAME_LEN		32
#define	SDS_MAX_LOCATION_LEN		32
#define	SDS_MAX_CHANNEL_NUM			16
#define	SDS_MAX_PRESET_NUM			20
#define	SDS_MAX_PRESET_LEN			32
#define	SDS_MAX_RTSP_ACC_NAME		256

typedef struct
{
	TCHAR				zServerName[20];
    DWORD				dwCamNum;
    DWORD				dwComNum;
    DWORD				dwDiNum;
	DWORD				dwDoNum;
    DWORD				dwVideoCodec;
    DWORD				dwAudioCodec; 
    TCHAR				zVideoUrl[40];

	// sylvia(2003/10/31)
	TCHAR				zRxUrl[40];
	TCHAR				zTxUrl[40];

	TCHAR				zQuadUrl[40];
    TCHAR				zPtzUrl[40];
	TCHAR				zRecallUrl[40];
	TCHAR				zPresetUrl[40];
    TCHAR				zGetDiUrl[40];
	TCHAR				zSetDoUrl[40];
    TCHAR				zUartUrl[40];
    DWORD				dwMDWinNum;
	TCHAR				zDefaultVSize[10]; // only for 2K series
	DWORD				dwDefaultQuality;
	DWORD				dwDefaultProtocol;
	DWORD				dwDefaultMediaType;
	DWORD				dwFlag;
} TSrvDepResource_ServerInfo;

typedef enum 
{
	ptzCapLensBuiltIn		= 0x00000001,
	ptzCapPan				= 0x00000002,
	ptzCapTilt				= 0x00000004,
	ptzCapZoom				= 0x00000008,
	ptzCapFocus				= 0x00000010,
	ptzCapIris				= 0x00000020,
} ESysPTZCapability;

typedef enum 
{
	eventCapVideoEvents		= 0x00000001,
	eventCapSysinfo			= 0x00000002,
} ESysCapEvent;

typedef enum 
{
	streamCapHttpSingleJpeg		= 0x00000001,
	streamCapHttpMJpeg			= 0x00000002,
	streamCap3K_Http			= 0x00000004,
	streamCap3K_TCP				= 0x00000008,
	streamCap3K_UDP				= 0x00000010,
	streamCap6K_HTTP			= 0x00000020,
	streamCap6K_UDP				= 0x00000040,
	streamCap6K_HTTP_TALK		= 0x00000080,
	streamCapRTSP_RTP_UDP		= 0x00000100,
	streamCapRTSP_RTP_TCP		= 0x00000200,
	streamCapRTSP_RTP_HTTP		= 0x00000400,
	streamCapRTSP_RTP_MCAST		= 0x00000800,
	streamCapRTP_MCAST			= 0x00001000,
} ESysCapStreaming;

// this structure is the parsed data returned by sysinfo.cgi
typedef struct
{
	// these string is all in char mode, but we declare it as byte here to avoid unicode problem
	// the parsing function will append null terminate at end of string
	BYTE				sModel[SDS_MAX_MODEL_LEN + 1];
	BYTE				szHostName[SDS_MAX_HOSTNAME_LEN + 1];
	BYTE				aszLocation[SDS_MAX_CHANNEL_NUM][SDS_MAX_LOCATION_LEN + 1];
	BYTE				aaszPreset[SDS_MAX_CHANNEL_NUM][SDS_MAX_PRESET_NUM][SDS_MAX_PRESET_LEN];
	BYTE				szRTSPAccessName[SDS_MAX_RTSP_ACC_NAME + 1];
	DWORD				adwPTZCap[SDS_MAX_CHANNEL_NUM];	// 0xFFFFFFFF if lack
	DWORD				dwCapVersion;	// 0xFFFFFFFF if lack, if present, HIWORD is major, LOWORD is minor
	DWORD				dwEvent;		// 0xFFFFFFFF if lack
	DWORD				dwRTSPPort;		// 0 if lack
	DWORD				dwCamNum;		// default is 1 if not present
	DWORD				dwUartNum;		// default 0
	DWORD				dwDINum;		// default 0
	DWORD				dwDONum;		// default 0
	DWORD				dwVideoCodec;	// ORed value for supported video codec
	DWORD				dwAudioCodec;	// ORed value for supported audio codec
	DWORD				dwMotionMethod;	// 3 or 1 windows
	DWORD				dwStreamingProtocol;	// ORed value for supported streaming protocol
} TSrvDepResource_SysInfo;


typedef enum 
{
	eoptszHostName				= 0x00000001,
	eoptLocation				= 0x00000002,
	eoptPreset					= 0x00000004,
	eoptRTSPAccessName			= 0x00000008,
	eoptPTZCap					= 0x00000010,
	eoptCapVersion				= 0x00000020,
	eoptEvent					= 0x00000040,
	eoptRTSPPort				= 0x00000080,
	eoptCamNum					= 0x00000100,
	eoptUartNum					= 0x00000200,
	eoptDINum					= 0x00000400,
	eoptDONum					= 0x00000800,
	eoptVideoCodec				= 0x00001000,
	eoptAudioCodec				= 0x00002000,
	eoptMotionMethod			= 0x00004000,
	eoptStreamingProtocol		= 0x00008000,
} EOptSysInfo;



#ifdef __cplusplus
extern "C" {
#endif

SCODE DLLAPI SrvDepResource_GetParamForServerByFriendlyName(TCHAR* pzFriendlyName, TSrvDepResource_ServerInfo* ptServerInfo);
SCODE DLLAPI SrvDepResource_GetParamForServer(TCHAR* pzServerType, TSrvDepResource_ServerInfo* ptServerInfo);
SCODE DLLAPI SrvDepResource_GetMappingFriendlyName(TCHAR* pzServerType, TCHAR* pzFriendlyName);
SCODE DLLAPI SrvDepResource_EnumerateSupportingMode(TCHAR*** pFriendlyNameList, DWORD *dwServerNum);
SCODE DLLAPI SrvDepResource_FreeFriendlyNameList(TCHAR*** pFriendlyNameList);

// parse the sysinfo received from server, the pbySysinfoData is actually a char string,
// and it needs to contain the '\0' at end of string, we define BYTE here to avoid
// unicode issue, under windows CE, it must be a char too. (the sysinfo is sent by camera, so it
// is always non-unicode string)
SCODE DLLAPI SrvDepResource_ParseSysInfo(BYTE *pbySysinfoData, TSrvDepResource_SysInfo *ptTarget);

SCODE DLLAPI SrvDepResource_GetVersionInfo(BYTE *byMajor, BYTE *byMinor, BYTE *byBuild, BYTE *byRevision);
SCODE DLLAPI SrvDepResource_SetDataFilePath(TCHAR *tXMLDataPath);


#ifdef __cplusplus
}
#endif

#endif
