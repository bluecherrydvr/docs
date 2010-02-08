#ifndef _DATABROKER_H_
#define _DATABROKER_H_

#include "typedef.h"
#include "errordef.h"
#include "common.h"
//#include "MainProfile/MediaBuffer.h"
#include "datapacketdef.h"
#include "SrvTypeDef.h"
#include "DataBrokerCallbackDef.h"

/*! The databroker version define */
#define DATABROKER_VERSION	MAKEFOURCC(5,0,0,27)

// sylvia(2003/10/315)
/*! The max Video codec number */
#define	MAX_VIDEO_CODEC		5
/*! The max audio codec number */
#define	MAX_AUDIO_CODEC		5
/*! This enumeration lists the options available when calling */
typedef enum
{
	/// to set queue size for connection or input, it is effect before the queue is created
	/// so for input set this before DataBroker_SetInputOptions is called
	/// for connection, before DataBroker_SetConnectionOptions is called
	/// param1 is Video Q size, param2 is Audio Q Size
	eOptQueueSize = 1,
	/// the maximum value for video is 60, and the maximum for Audio is 20
	/// to enable or disable the auto-force I function, if to enable, the dwParam2 is
	/// 0 period in milli-seconds, minimum value is 100 ms
	/// it's only for connection, not input
	eOptAutoForceI = 2,
	/// dwParam1 is true/false, if the DI/DO value is from caller or from packet
	/// only works for 2000 and 3000 server, the caller must take care
	/// for if the firmware alreay send DI/DO, if so, it's not necessary to use
	/// this path to set DI/DO value
	eOptDIDOBySet = 3,
	/// dwParam1 is DI value, dwParam2 is DO value
	eOptSetDIDOValue = 4,
	/// if the 3000 audio packet should also carry DI/DO value, the value will
	/// come from video, no way to set it by caller, dwParam1 = true/false
	eOpt3KAudioDIDO = 5,
	/// the control port for the RTSP server, dwParam1 is the port
	eOptRtspCtrlPort = 6,
	/// the serve info
	eOptServerInfo = 7,
	//bruce 20060904 add for AAC DataBroker Input channe;
	eOptAACInfo = 8,
	//perkins 2006/11/01 For proxy authentication, dwParam1 is the user name, dwParam2 is password
	eOptProxyAuthen = 9,

	//steven 2007/03/22 For dual-stream models, dwParam1 is the index of stream
	eOptStreamIndex = 10,

	// perkins 2007/4/13 for rtsp proxy, this could
	eOptRtspProxyEnable = 11,	// dwParam1 is TRUE for enable, FALSE for disable
	eOptRtspProxyInfo = 12,	// dwParam1 is proxy address, dwParam2 is the proxy port
	eOptProtocolMedia = 13, // dwParam1 is protocol, and dwParam2 is media type. value of 0xFFFFFFFF means do not set it
	eOptProtocolRollNext = 14, // dwParam1 is the next protocol to be tried
	eOptUpdUserNamePwd = 15, // dwParam1 is user name, and dwParam2 is password
	eOptRtspMcastProtocolRollingTimeout = 16, // dwParam1 is timeout value
	eOptRtspUDPProtocolRollingTimeout = 17, // dwParam1 is timeout value

	//steven 2007/09/14 add for talking to camera without getting streaming. 
	//					User should use dwParam1 to specify the server type. 0 for SIP server, 1 for 6k camera																
	eOptTalkWithoutStreaming = 18,

	eOptSSLEnable = 19,		// dwParam1 is TRUE for enable, FALSE for disable, dwParam2 is TRUE for check certification and false for not

	eOptRTSPForward = 20,
	eOptRTSPBackward = 21,
	eOptReqFieldModeInfo = 22, //callback field mode info or not, dwParam1 is true for enable and false for disable
	
	// 2008/12/16: if dwParam1 is TRUE, the connection use the shared thread for connection server, otherwise create its own thread(default behavior)
	eOptUseSharedThread = 23,

	// 2008/12/22: dwParam1 is TDataBrokerAVCallbackV3
	eOptUseV3Callback = 24

} TExtraOptions;
/*! This enumeration indicates the flags that have to be turned on when modify DataBroker handle¡¯s option in TDataBrokerOptions. When the flag is set the corresponding field is checked and taken in function call. Otherwise, the field is ignored. */
typedef enum {
	/*! the flag for bEnableProxy field of TDataBrokerOptions*/
	eOptEnableProxy				= 0x0001,
	/*! the flag for dwProxyPort field of TDataBrokerOptions. */
	eOptProxyPort				= 0x0002,
	/*! the flag for szProxyName field of TDataBrokerOptions. */
	eOptProxyName				= 0x0004,
	/*! the flag for bEnableIPRestrict field of TDataBrokerOptions */
	eOptEnableIPRestrict		= 0x0008,
	/*! the flag for dwIPRestrictNum field of TDataBrokerOptions */
	eOptIPRestrictNum			= 0x0010,
	/*! the flag for pszIPRestrictList field of TDataBrokerOptions */
	eOptIPRestrictList			= 0x0020,
	/*! the flag to indicate that the connection will use Wininet to connect to server. Only works when in Windows 32 or Windows CE platform */
	eOptConnUseIE				= 0x0040,
	/*! the flag to indicate the connection will NOT use Wininet to connect to server. Only works when in Windows 32 or Windows CE platform. If both this and eOptConnUseIE are defined, the not use is taken */
	eOptConnNotUseIE			= 0x0080,	// if both appear, the not used is taken
	/*! the flag for dwConnTimeout field of TDataBrokerOptions */
	eOptConnTimeout				= 0x0100,
	/*! the flag for dwRWTimeout field of TDataBrokerOptions */
	eOptRWTimeout				= 0x0200,
} TDataBrokerOptionFlag;
/*! This enumeration indicates the flags that have to be turned on when you want to specify values to some fields of the TDataBrokerConnectionOptions. When the flag is set the corresponding field is checked and taken in function call. Otherwise, the field is ignored */
typedef enum {
	/*! tthe flag for wCam field of TdataBrokerConnectionOptions */
	eConOptCam					= 0x0001,
	/*! the flag for zVSize field of TDataBrokerConnectionOptions */
	eConOptVSize				= 0x0002,
	/*! the flag for dwQuality field of TDataBrokerConnectionOptions */
	eConOptQuality				= 0x0004,
	/*! the flag for wHttpPort field of TDataBrokerConnectionOptions */
	eConOptHttpPort				= 0x0008,
	/*! the flag for dwProtocolType and dwMediaType fields of TDataBrokerConnectionOptions. You should specify these two values at the same time */
	eConOptProtocolAndMediaType = 0x0010,
	/*! the flag for dwVideoCodec field of TDataBrokerConnectionOptions */
	eConOptVideoCodec			= 0x0020,
	/*! the flag for dwAudioCodec field of TDataBrokerConnectionOptions */
	eConOptAudioCodec			= 0x0040,
	/*! the flag for pfStatus field of TDataBrokerConnectionOptions */
	eConOptStatusCallback		= 0x0080,
	/*! the flag for pfAV field of TDataBrokerConnectionOptions */
	eConOptAVCallback			= 0x0100,
	/*! the flag for dwStatusContext field of TDataBrokerConnectionOptions */
	eConOptStatusContext		= 0x0200,
	/*! the flag for dwAVContext field of TDataBrokerConnectionOptions */
	eConOptAVContext			= 0x0400,

	// sylvia(2003/10/31)
	/*! the flag for pfTx field of TDataBrokerConnectionOptions */
	eConOptTxCallback			= 0x0800,
	/*! the flag for dwTxContext field of TDataBrokerConnectionOptions */
	eConOptTxContext			= 0x1000,
	/*! the flag for dwAudioEnc field of TDataBrokerConnectionOptions */
	eConOptAudioEncCodec		= 0x2000,
	/*! the flag for dwVCodecOrder field of TDataBrokerConnectionOptions */
	eConOptVideoCodecPri		= 0x4000,
	/*! the flag for dwACodecOrder field of TDataBrokerConnectionOptions */
	eConOptAudioCodecPri		= 0x8000,
	/*! the flag for dwAudioSample field of TDataBrokerConnectionOptions */
	eConOptAudioSample			= 0x10000,
	/*! the flag for bEnableProxy field of TDataBrokerOptions*/
	eConOptAudioEncSample		= 0x20000,
	/*! the flag for dwAudioEncSample field of TDataBrokerConnectionOptions */
} TDataBrokerConnOptionFlag;



/*! This enumeration indicates the reason why media type is changed. This reason code is only available when connecting to 4000/5000/6000 series servers */
typedef enum
{	/*! The permission of current user is not allowed to open downstream or upstream audio connection */
	eNoPermission,
	/*! The server¡¯s audio mode is set to talk-only or audio disabled mode so users are not allowed to establish downstream audio connection. Or the server is set to listen-only or disabled mode so users are not allowed to establish upstream audio connection */
	eModeNotSupport,
} TMediaChangeReason;

/*! This enumeration indicates the audio mode currently set on server. The notification of this mode is only available when connecting to 4000/5000/6000 series servers */
typedef enum
{	/*! Server is in full-duplex mode, which enables users to listen and talk simultaneously */
	eFullDuplex = 1,
	/*! Server is in half-duplex mode, which allows users to be either talk or listen. When talking, the downstream will be disabled */
	eHalfDuplex = 2,
	/*! Server is in talk-only mode, which allows users to talk but not listen. Users might get eOnChangeMediaType/ eOnAudioDisabled status callback if selects video/audio or audio only mode when connecting */
	eTalkOnly = 3,
	/*! Server is in listen-only mode, which allows users to listen but not talk */
	eListenOnly = 4,
} TMediaAudioMode;

/*! This enumeration indicates the privilege for a user. This privilege value is only available when connecting to 4000/5000/6000 series servers */
typedef enum
{	/*! Users could set DO and retrieve DI value from server. Note: this flag might be renamed to DO only because the DI is carried in video stream. So there is no reason to limit users to get DI value */
	ePrivilegeDIDO = 1,
	/*! Users could open the downstream audio connection to server. Hence they could listen to the live audio sent via server */
	ePrivilegeLISTEN = 2,
	/*! Users could open the upstream audio connection to server. Hence they could send server audio data. Note: Talk capability is determined by this privilege and the server¡¯s audio mode. Both must be turn on to enable talking */
	ePrivilegeTALK = 4,
	/*! Users could control the camera control. This includes Pan/Tilt/Zoom/Focus. It depends on the server model */
	ePrivilegeCAMCTRL = 8,
	/*! Users could set the configuration of the servers */
	ePrivilegeCONF = 128,
	/*! Users have the full access to the camera */
	ePrivilegeAll = 0xFFFFFFFF,
} TUserPrivilege;

/*! This enumeration indicates the channel types */
typedef enum {
	/*! audio channel */
	eAudioChannel,
	/*! video channel */
	eVideoChannel
} TDataBrokerChannelType;
/*! This enumeration indicates the signal state when callback notify that the video signal changes. The callback would be called whenever the signal state changes, so if the signal lost at certain time, the signal will be called once for signal lost, and would not be called until signal restored */
typedef enum
{	/*! The signal restored */
	eSignalRestored = 0,
	/*! The signal is lost */
	eSignalLost = 1,
} TVideoSignalState;

/// This structure indicates the proxy options for DataBroker.
typedef struct {
	///Enable proxy when connect to server by HTTP.
	BOOL	bEnableProxy;
	/// Proxy port.
	DWORD	dwProxyPort;
	/// IP of proxy server.
	TCHAR	szProxyName[128];
	/// Enable IP restriction check.
	BOOL	bEnableIPRestrict;
	/// The number of IP restriction strings you want to apply
	DWORD	dwIPRestrictNum;
	/// The pointer to the list of IP restriction strings. Proxy will not be applied to the IP starting by these strings
	TCHAR	(*pszIPRestrictList)[128];
	/// Flags for the optional field
	DWORD	dwFlags;
	/// timeout when connect
	DWORD	dwConnTimeout;
	/// timeout when receive or write
	DWORD	dwRWTimeout;
} TDataBrokerOptions;
/// This structure indicates the connection options
typedef struct {
	///the camera index
	WORD						wCam;
	/// the Vsize (For 2K series only)
	TCHAR						zVSize[10];	// only for VS2K
	/// the quality value (For 2K series only)
	DWORD						dwQuality;	// only for VS2K
	/// the pointer to the TDataBrokerStatusCallback  callback function.
	TDataBrokerStatusCallback	pfStatus;
	/// the pointer to the callback function of receiving AV frames
	TDataBrokerAVCallback		pfAV;

	// sylvia(2003/10/31)
	/// the pointer to the callback function of transmitting media
	TDataBrokerTxCallback		pfTx;
	/// the context value for status callback function
	DWORD						dwStatusContext;
	/// the context value for AV callback function
	DWORD						dwAVContext;

	// sylvia(2003/10/31)
	/// the context value for Tx callback function
	DWORD						dwTxContext;
	/// the HTTP port number
	WORD						wHttpPort;
	/// the protocol type
	DWORD						dwProtocolType;
	/// the requested media type
	DWORD						dwMediaType;
	/// the video codec type
	DWORD						dwVideoCodec;
	/// the audio codec type
	DWORD						dwAudioCodec;

	// sylvia(2003/10/31)
	/// audio sample rate, used only for 3135
	DWORD						dwAudioSample;	//
	/// audio encoding codec type
	DWORD						dwAudioEnc;	//
	/// audio sample rate, only for 3135
	DWORD						dwAudioEncSample;	//
	/// Server friendly name. This option must be specified
	TCHAR*						pzServerType;
	/// Remote IP address you want to connect to, the maximum length is 128 bytes. This option must be specified
	TCHAR*						pzIPAddr;
	/// User login ID, the maximum length is 40 bytes. This option must be specified
	TCHAR*						pzUID;
	/// User login password, the maximum length is 40 bytes. This option must be specified
	TCHAR*						pzPWD;

	// sylvia(2003/10/31)
	/// Server supports video codecs following this order. It¡¯s only valid for 4/5/6K server.
	DWORD						adwVCodecOrder[MAX_VIDEO_CODEC];
	/// Server supports audio codecs following this order. It¡¯s only valid for 4/5/6K server
	DWORD						adwACodecOrder[MAX_AUDIO_CODEC];
	/// Flags for the optional fields
	DWORD						dwFlags;
	/// Play speed. It¡¯s only valid for 7K server
	double						dPlaySpeed;
} TDataBrokerConnectionOptions, /*! This pointer to TDataBrokerConnectionOptions */  *PTDataBrokerConnectionOptions;

/* TDataBrokerConnInfo - the info which will be passed to app */
/// This structure indicates the information of a connection
typedef struct
{	/// the width of the image. Note that this value is only for reference because for HTTP mode of 3000 server, the value is not retrieved actually, so some reference value is returned. To get the exact value, please use the decoder callback from AVSynchronizer
	DWORD						dwWidth;
	/// the height of the image. Note that this value is only for reference because for HTTP mode of 3000 server, the value is not retrieved actually, so some reference value is returned. To get the exact value, please use the decoder callback from AVSynchronizer
	DWORD						dwHeight;
	/// the language type of the server
	TCHAR						zLanguage[8];
	/// the audio codec type of the server
	DWORD						dwAudioCodec;
	/// the video codec type of the server
	DWORD						dwVideoCodec;
	/// the media type of the server
	DWORD						dwMediaType;
	/// the actual protocol used by the connection
	DWORD						dwProtocol;
	/// the server side video port used by the connection
	WORD						wVideoPort;
	/// the server side audio port used by the connection
	WORD						wAudioPort;
	/// the server¡¯s friendly name
	TCHAR						szServerType[20];
} TDataBrokerConnInfo, *PTDataBrokerConnInfo;
/// This structure indicates the Input options
typedef struct {
	///the pointer to the TDataBrokerStatusCallback callback function
	TDataBrokerStatusCallback	pfStatus;
	/// the pointer to the TDataBrokerAVCallback AV callback function
	TDataBrokerAVCallback		pfAV;
	/// the context value for the status callback function
	DWORD						dwStatusContext;
	/// the context value for the AV callback function
	DWORD						dwAVContext;
	/// the server friendly name
	TCHAR*						pzServerType;
	/// the audio codec type
	DWORD						dwAudioCodec;
	/// the video codec type
	DWORD						dwVideoCodec;
	/// the protocol type
	DWORD						dwProtocolType;
	/// the Vsize (For 2K series only)
	TCHAR*						zVSize;
} TDataBrokerInputOptions, *PTDataBrokerInputOptions;
//bruce 20060904 add for AAC DataBroker_Input

typedef struct
{
	BYTE *pbyCI;
	DWORD dwCILength;

}TDataBrokerMPEG4Info;

typedef struct
{
	BYTE *pbyCI;
	DWORD dwCILength;

}TDataBrokerMJPEGInfo;

typedef struct
{
	BYTE *pbySPS;
	DWORD dwSPSLength;
	BYTE *pbyPPS;
	DWORD dwPPSLength;

}TDataBrokerH264Info;

typedef struct {

	DWORD dwSamplingFrequency;

	DWORD dwChannelNumber;
}TAACExtInfo, *PTAACExtInfo;

#ifdef __cplusplus
extern "C" {
#endif

	// sylvia(2003/10/31)
	//SCODE DLLAPI DataBroker_Initial(HANDLE *phDataBroker, DWORD dwMaxConn, TDataBrokerStatusCallback pfStatus, TDataBrokerAVCallback pfAV, DWORD dwVersion);
	SCODE DLLAPI DataBroker_Initial(HANDLE *phDataBroker, DWORD dwMaxConn, TDataBrokerStatusCallback pfStatus, TDataBrokerAVCallback pfAV, DWORD dwSupportCodec, DWORD dwFlag, DWORD dwVersion);
	SCODE DLLAPI DataBroker_SetOptions(HANDLE hDataBroker, TDataBrokerOptions* ptOptions);

	// sylvia(2003/10/31)
	// call this function to set the codec using priority for all connections.
	// the priorities are only used for IP3135 server
	// the sizes of the array are MAX_VIDEO_CODEC for video and MAX_AUDIO_CODEC for audio
	SCODE DLLAPI DataBroker_SetCodecPriority(HANDLE hDataBroker, DWORD *pdwVideoCodec, DWORD *pdwAudioCodec);

	SCODE DLLAPI DataBroker_Release(HANDLE *phDataBroker);
	SCODE DLLAPI DataBroker_CreateConnection(HANDLE hDataBroker, HANDLE *phConn);
	SCODE DLLAPI DataBroker_DeleteConnection(HANDLE hDataBroker, HANDLE *phConn);
	SCODE DLLAPI DataBroker_Connect(HANDLE hConn);
	SCODE DLLAPI DataBroker_Disconnect(HANDLE hConn);
	SCODE DLLAPI DataBroker_SetConnectionOptions(HANDLE hConn, TDataBrokerConnectionOptions* ptConnOptions);

	// to set more options for connection
	SCODE DLLAPI DataBroker_SetConnectionExtraOption(HANDLE hConn, DWORD dwOption, DWORD dwParam1, DWORD dwParam2);

	// to be used when connect to IIS solution of 2K server
	SCODE DLLAPI DataBroker_SetConnectionUrlsExtra(HANDLE hConn, const TCHAR *pszVideoUrl, const TCHAR *pszVideoExtra, const TCHAR *pszTxUrl, const TCHAR *pszTxExtra, const TCHAR *pszRxUrl, const TCHAR *pszRxExtra);

	// to assign if the connection would callback for each network packet
	// if the callback funciton pointer is null, the callback will stop. if users only network packet
	// callback, please set bCallbackOnly to true.
	SCODE DLLAPI DataBroker_SetConnectionNetPacketCallback(HANDLE hConn, TDataBrokerNetPacketCallback pfNetPacketCallback, DWORD dwContext, BOOL bCallbackOnly);

	// sylvia(2003/10/31)
	SCODE DLLAPI DataBroker_StartTxConnection(HANDLE hConn);
	SCODE DLLAPI DataBroker_StopTxConnection(HANDLE hConn);
	SCODE DLLAPI DataBroker_InputTxPacket(HANDLE hConn, TMediaDataPacketInfo *ptMediaPacketInfo, DWORD dwDataTimePeriod);

	SCODE DLLAPI DataBroker_CreateInput(HANDLE *phInput, DWORD dwVersion);
	SCODE DLLAPI DataBroker_CreateInputEx(HANDLE hDataBroker, HANDLE *phInput);
	SCODE DLLAPI DataBroker_DeleteInput(HANDLE *phInput);
	SCODE DLLAPI DataBroker_DeleteInputEx(HANDLE hDataBroker, HANDLE *phInput);
	SCODE DLLAPI DataBroker_SetInputOptions(HANDLE hInput, TDataBrokerInputOptions* ptInputOptions);
	SCODE DLLAPI DataBroker_SetInputExtraOption(HANDLE hInput, DWORD dwOption, DWORD dwParam1, DWORD dwParam2);
	SCODE DLLAPI DataBroker_InputPacket(HANDLE hInput, TsdrMediaType dwMediaType, BYTE *pbyData, DWORD dwLength);
	SCODE DLLAPI DataBroker_InputPacketEx(HANDLE hInput, TsdrMediaType dwMediaType, BYTE *pbyData, DWORD dwLength);
	SCODE DLLAPI DataBroker_ForceIFrame(HANDLE hConn);

	SCODE DLLAPI DataBroker_GetVersionInfo(BYTE *byMajor, BYTE *byMinor, BYTE *byBuild, BYTE *byRevision);

	// Bill,2004/09/23
	SCODE DLLAPI DataBroker_PauseMediaStreaming(HANDLE hConn);
	SCODE DLLAPI DataBroker_ResumeMediaStreaming(HANDLE hConn);
	SCODE DLLAPI DataBroker_JumpMediaStreaming(HANDLE hConn, DWORD dwPercentage);
	SCODE DLLAPI DataBroker_CheckIfLive(HANDLE hConn);

	// Demon,2009/03/11
	SCODE DLLAPI DataBroker_GetNextFrame(HANDLE hConn);

	SCODE DLLAPI DataBroker_GetCurrentVideoFrameID(HANDLE hConn,DWORD* pdwVideoFrameID);
	SCODE DLLAPI DataBroker_GetCurrentAudioFrameID(HANDLE hConn,DWORD* pdwAudioFrameID);

	SCODE DLLAPI DataBroker_CreateConnectionEx(HANDLE *phConn);
	SCODE DLLAPI DataBroker_DeleteConnectionEx(HANDLE *phConn);


#ifdef __cplusplus
}
#endif



#endif // _DATABROKER_H_
