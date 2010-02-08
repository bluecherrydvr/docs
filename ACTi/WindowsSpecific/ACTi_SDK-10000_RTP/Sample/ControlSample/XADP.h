#pragma once
#include "SDKMessageDefine.h"
#include "TCPConnection.h"
#include "VideoControl_2_0.h"
#include "VideoStreaming_2_0.h"
#include "HTTPConnection.h"

typedef void ( WINAPI *CONTROL_DATA_CALLBACK	)( DWORD UserParam, BYTE* buf, DWORD len );

#define CONTACT_TYPE_UNICAST_WOC_PREVIEW		1
#define CONTACT_TYPE_MULTICAST_WOC_PREVIEW		2
#define CONTACT_TYPE_RTSP_PREVIEW				3
#define CONTACT_TYPE_CONTROL_ONLY				4
#define CONTACT_TYPE_UNICAST_PREVIEW			5
#define CONTACT_TYPE_MULTICAST_PREVIEW			6
#define CONTACT_TYPE_PLAYBACK					7
#define CONTACT_TYPE_CARD_PREVIEW				8

#define NET_TVNTSC								0
#define NET_TVPAL								1

#define DEBUG_LEVEL_NO_DEBUG_MESSAGE			0
#define DEBUG_LEVEL_FUNCTION_TRACE				1
#define DEBUG_LEVEL_DETAIL						2

typedef struct structural_MEDIA_CONNECTION_CONFIG
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
	unsigned long	RTSPPort;
	unsigned long	VideoRTPOverMCastPort;
	unsigned long	AudioRTPOverMCastPort;
	int				ConnectTimeOut;
}MEDIA_CONNECTION_CONFIG;

typedef struct structural_MEDIA_VIDEO_CONFIG
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

typedef struct structural_MEDIA_COMMAND
{
	DWORD dwCommandType;
	char* pCommand;
	int	  nCommandLength;
	char* pResult;
	int   nResultLength;
} MEDIA_COMMAND;

typedef struct structural_MEDIA_MOTION_INFO
{
	DWORD dwEnable;
	DWORD dwRangeCount;
	DWORD dwRange[3][4];
	DWORD dwSensitive[3];
} MEDIA_MOTION_INFO;

class XADP
{
public:
	XADP(void);
	~XADP(void);

private:
	CVideoControl_2_0	*							m_ControlPort;
	CVideoStreaming_2_0	*							m_StreamingPort;
	CHTTPConnection		*							m_HTTP;
	char				*							m_TempBuf;

	VIDEO_2_0_VIDEO_STREAMING_CONNECTION_CONFIG		m_VSConfig;
	VIDEO_2_0_VIDEO_CONTROL_CONNECTION_CONFIG		m_VCConfig;
	HTTP_CONNECTION_CONFIG							m_HTTPConfig;
	
	int												m_SessionID;
public:
	MEDIA_CONNECTION_CONFIG							m_MediaConfig;
	MEDIA_VIDEO_CONFIG								m_VideoConfig;
	char											m_UrlTarget[128];
	int												m_nDebugLevel;

	void	SetMediaConfig( MEDIA_CONNECTION_CONFIG* pMediaConfig );
	void	GetVideoConfig( MEDIA_VIDEO_CONFIG* pVideoConfig );
	void	SetVideoConfig( MEDIA_VIDEO_CONFIG* pVideoConfig );

	void	SetMotionInfo( MEDIA_MOTION_INFO* Motioninfo );
	void	GetMotionInfo( MEDIA_MOTION_INFO* Motioninfo );

	DWORD	GetBeginTime();
	DWORD	GetEndTime();

	bool	Connect();
	void	Disconnect();
	bool	StartStreaming();
	void	StopStreaming();
	bool	SendCommand( MEDIA_COMMAND* pMediaCommand );
	void	ExchangHTTPResultToVideoConfig( char* result );
	int		GetNextFrame( int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
	int		GetNextIFrame( BYTE* pFrameBuffer, int nBufferSize );
	void	ShowDebugMessage( int nDebugLevel, TCHAR *pFormat, ...);
	void	SetDebugMessageLevel( int nDebugLevel );
	DWORD	HexToDWORD( char* hex );

	// callback functions & params
	CONTROL_DATA_CALLBACK	m_fnControlCB;
	DWORD					m_ParamControlCB;
	
	void	SetControlDataCallBack( DWORD UserParam, CONTROL_DATA_CALLBACK fnCallBack );
};
