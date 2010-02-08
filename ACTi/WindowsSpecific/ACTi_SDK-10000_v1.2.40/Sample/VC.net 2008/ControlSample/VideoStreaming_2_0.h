#pragma once
#include "SDKMessageDefine.h"
#include ".\tcpconnection.h"


#define STREAM_BUFFER_SIZE			256000
#define STREAM_BUFFER_POOL_SIZE		10

#define _VIDEO_STREAMING_UNKNOW_ERROR				200
#define _VIDEO_STREAMING_NOT_CONNECTED				201
#define _VIDEO_STREAMING_BAD_IP_OR_PORT				202
#define _VIDEO_STREAMING_BAD_ID_OR_PAWWSORD			203
#define _VIDEO_STREAMING_STREAMING_STARTED_ALREADY	204

typedef struct 
{
	char szUserName[32];
	char rsvd[28];
	unsigned short StreamID;
	unsigned short nEncryptionType;
	char szPassword[64];
}TCP_STREAM_AUTHEN_REQ_V2;

typedef struct structural_VIDEO_2_0_VIDEO_STREAMING_CONNECTION_CONFIG
{
	char			IP[16];
	unsigned short	VideoStreamingPort;
	unsigned short  StreamID;

	// int ConnectTimeOut resize to unsigned short,
	// add unsigned short EncryptionType
	unsigned short	ConnectTimeOut;
	unsigned short	EncryptionType;

	char			UserID[64];
	char			Password[64];
	int				AliveTimeout;
}VIDEO_2_0_VIDEO_STREAMING_CONNECTION_CONFIG;

class CVideoStreaming_2_0
{
public:
	CVideoStreaming_2_0(void);
	~CVideoStreaming_2_0(void);

	enum STREAMING_STATUS{ St_Idle = 0, St_Connected, St_IDNotPassed, St_IDPassed, St_Streaming };

private:
	int											m_LastErrorCode;
	int											m_ConnectionStatus;
	VIDEO_2_0_VIDEO_STREAMING_CONNECTION_CONFIG	m_ConnectionConfig;

	char*										m_StreamBuffer;//[STREAM_BUFFER_SIZE];

	int			ReadTCP20Data();
	int			CheckIDPassword( char* ID, char* Password );
	void		DebugMessage( TCHAR *pFormat, ...);
	

public:
	CTCPConnection								m_VideoStreamingTCPConnection;
	//	Functions
	void		SetConfig( VIDEO_2_0_VIDEO_STREAMING_CONNECTION_CONFIG* st );
	bool		Connect();
	void		Disconnect();
	bool		StartVideoStreaming( int& SessionID );
	void		StopVideoStreaming();
	bool		SetRecvBufferSize( long size );
	bool		SetSendBufferSize( long size );
	
	bool		SendCommamd( DWORD dwCommand, char* pData, DWORD dwDataLen );
	//	Functions for request private data
	int			GetLastError() { return m_LastErrorCode; }
	int			GetConnectionStatus() { return m_ConnectionStatus; }
	int			GetOneFrame( char* pBuffer, int nBufferSize );
};
