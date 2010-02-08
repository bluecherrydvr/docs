#pragma once
#include "SDKMessageDefine.h"
#include ".\tcpconnection.h"

#define _VIDEO_CONTROL_UNKNOW_ERROR					300
#define _VIDEO_CONTROL_NOT_CONNECTED				301
#define _VIDEO_CONTROL_BAD_IP_OR_PORT				302
#define _VIDEO_CONTROL_BAD_ID_OR_PAWWSORD			303
#define _VIDEO_CONTROL_CONTROL_STARTED_ALREADY		304

typedef struct structural_VIDEO_2_0_VIDEO_CONTROL_CONNECTION_CONFIG
{
	char			IP[16];
	unsigned short	VideoControlPort;
	unsigned short  StreamID;

	// int ConnectTimeOut resize to unsigned short,
	// add unsigned short EncryptionType
	unsigned short	ConnectTimeOut;
	unsigned short	EncryptionType;

	char			UserID[64];
	char			Password[64];
}VIDEO_2_0_VIDEO_CONTROL_CONNECTION_CONFIG;

typedef	void (WINAPI *VIDEO_CONTROL_DATA_CALLBACK)( DWORD UserParam, void* pData, int nDataLength );

class CVideoControl_2_0
{
public:
	CVideoControl_2_0(void);
	~CVideoControl_2_0(void);

	enum CONTROL_STATUS{ St_Idle = 0, St_Connected, St_IDNotPassed, St_IDPassed, St_InControl };

private:
	int											m_LastErrorCode;
	int											m_ConnectionStatus;
	VIDEO_2_0_VIDEO_CONTROL_CONNECTION_CONFIG	m_ConnectionConfig;

	int											m_AliveTimeout;
	HANDLE										m_ControlThread;
	DWORD										m_ControlThreadID;
	bool										m_bDoControl;
	HANDLE										m_ControlThreadEndingEvent;
	char*										m_ControlBuffer;

	VIDEO_CONTROL_DATA_CALLBACK					m_fnVCDCallBack;
	DWORD										m_fnVCDParam;

public:
	CTCPConnection								m_VideoControlTCPConnection;
	void		SetConfig( VIDEO_2_0_VIDEO_CONTROL_CONNECTION_CONFIG* st );
	bool		Connect();
	void		Disconnect();
	bool		ISConnected();
	int			CheckIDPassword( char* ID, char* Password );
	bool		StartVideoControl();
	void		StopVideoControl();
	void		WaitForStoping( DWORD nTimeout = 0 );
	void		ControlProcess();
	void		LiveCheck();
	void		SetControlCallBack( DWORD UserParam, VIDEO_CONTROL_DATA_CALLBACK fnCallBack );
	bool		SendCommamd( DWORD dwCommand, char* pData, DWORD dwDataLen );
	void		Test();
	
	int			GetLastError() { return m_LastErrorCode; }
	int			GetConnectionStatus() { return m_ConnectionStatus; }
};
