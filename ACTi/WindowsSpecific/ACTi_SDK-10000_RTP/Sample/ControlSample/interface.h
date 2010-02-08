#pragma once

typedef void ( WINAPI *RS232_DATA_CALLBACK	)( DWORD UserParam, BYTE* buf, DWORD len );
typedef	void ( WINAPI *RAW_DATA_CALLBACK	)( DWORD UserParam, BYTE* pBuffer, int BufferLength );

#define CONTACT_TYPE_UNICAST_WOC_PREVIEW		1
#define CONTACT_TYPE_MULTICAST_WOC_PREVIEW		2
#define CONTACT_TYPE_RTSP_PREVIEW				3
#define CONTACT_TYPE_CONTROL_ONLY				4
#define CONTACT_TYPE_UNICAST_PREVIEW			5
#define CONTACT_TYPE_MULTICAST_PREVIEW			6
#define CONTACT_TYPE_PLAYBACK					7
#define CONTACT_TYPE_CARD_PREVIEW				8

#define NET_TVNTSC	0
#define NET_TVPAL	1

typedef struct
{
	int				ContactType;
	int				ChannelNumber;
	char			UniCastIP[16];
	char			MultiCastIP[16];
	char			PlayFileName[256];
	char			UserID[64];
	char			Password[64];
	unsigned long	RegisterPort;
	unsigned long	StreamingPort;
	unsigned long	ControlPort;
	unsigned long	MultiCastPort;
	unsigned long	SearchPortC2S;
	unsigned long	SearchPortS2C;
	unsigned long	HTTPPort;
	int				ConnectTimeOut;
}MEDIA_CONNECTION_CONFIG;

typedef struct
{
	DWORD dwTvStander;			// 0:NTSC 1:PAL
	DWORD dwVideoResolution;	// See the definition above
	DWORD dwBitsRate;			// See the definition above
	DWORD dwVideoBrightness;	// 0 ~ 100 : Low ~ High
	DWORD dwVideoContrast;		// 0 ~ 100 : Low ~ High
	DWORD dwVideoSaturation;	// 0 ~ 100 : Low ~ High
	DWORD dwVideoHue;			// 0 ~ 100 : Low ~ High
	DWORD dwFps;				// 0 ~ 30 frame pre second
} MEDIA_VIDEO_CONFIG;

typedef struct
{
	DWORD dwCommandType;
	char* pCommand;
	int	  nCommandLength;
	char* pResult;
	int   nResultLength;
} MEDIA_COMMAND;

extern "C" __declspec(dllexport) HANDLE XInit();
extern "C" __declspec(dllexport) void XSetMediaConfig( HANDLE h, MEDIA_CONNECTION_CONFIG* pMediaCfg );
extern "C" __declspec(dllexport) bool XConnect( HANDLE h );
extern "C" __declspec(dllexport) void XSetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
extern "C" __declspec(dllexport) void XGetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
extern "C" __declspec(dllexport) bool XStartStreaming( HANDLE h );
extern "C" __declspec(dllexport) void XStopStreaming( HANDLE h );
extern "C" __declspec(dllexport) int XGetNextFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
extern "C" __declspec(dllexport) int XGetNextIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
// for playback( ARAW )
extern "C" __declspec(dllexport) int XGetPrevFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
// for playback( ARAW )
extern "C" __declspec(dllexport) int XGetPrevIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
extern "C" __declspec(dllexport) void XDisconnect( HANDLE h );
extern "C" __declspec(dllexport) bool XSendCommand( HANDLE h, MEDIA_COMMAND* pMediaCommand );
extern "C" __declspec(dllexport) void XExit( HANDLE h );
extern "C" __declspec(dllexport) void XSetRawDataCallBack( HANDLE h, DWORD UserParam, RAW_DATA_CALLBACK fnCallBack );
extern "C" __declspec(dllexport) void XSetRS232DataCallBack( HANDLE h, DWORD UserParam, RS232_DATA_CALLBACK fnCallBack );