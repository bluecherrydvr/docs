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


typedef struct structural_MEDIA_CONNECTION_CONFIG
{		
	int	ContactType;

	unsigned char	ChannelNumber;	// For URL Command CHANNEL tag, when set it to 0, URL command won't bring CHANNEL tag, or the URL command will bring CHANNEL=ChannelNumber tag  )
	unsigned char	TCPVideoStreamID;	// 0 based to specify video track, value 0 to 255 for 1 to 256 video track
	unsigned char	RTPVideoTrackNumber;	// set it to 0, ARTP will use 1st video track,  1 to 255 is for specify video track
	unsigned char	RTPAudioTrackNumber;	// set it to 0, ARTP will use 1st audio track,  1 to 255 is for specify audio track

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
	unsigned long	RTSPPort;
	unsigned long	Reserved1;
	unsigned long	Reserved2;

	unsigned short	ConnectTimeOut;
	unsigned short	EncryptionType;
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