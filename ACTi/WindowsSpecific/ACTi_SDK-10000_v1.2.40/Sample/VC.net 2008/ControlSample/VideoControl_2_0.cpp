#include "StdAfx.h"
#include ".\videocontrol_2_0.h"

DWORD WINAPI ControlThreadEnter_2_0( LPVOID p )
{
	CVideoControl_2_0* pVC = (CVideoControl_2_0*)p;
	pVC->ControlProcess();
	return 0;
}
CVideoControl_2_0::CVideoControl_2_0(void)
{
	m_fnVCDCallBack = NULL;
	m_ControlThreadEndingEvent = NULL;
	m_ControlThread = NULL;
	m_bDoControl = false;
	memset( &m_ConnectionConfig, 0, sizeof(VIDEO_2_0_VIDEO_CONTROL_CONNECTION_CONFIG) );
	m_ControlBuffer = new char[ 4096 + CMD_HEADER_SIZE ];
}

CVideoControl_2_0::~CVideoControl_2_0(void)
{
	if( m_ControlBuffer )
		delete [] m_ControlBuffer;
	m_ControlBuffer = NULL;
}

bool CVideoControl_2_0::Connect()
{
	bool bRet;

	bRet = m_VideoControlTCPConnection.Connect( m_ConnectionConfig.IP, m_ConnectionConfig.VideoControlPort, m_ConnectionConfig.ConnectTimeOut );

	if( bRet )
	{
		m_ConnectionStatus = CONTROL_STATUS::St_Connected;
		m_LastErrorCode = 0;
		m_VideoControlTCPConnection.SetRecvTimeout( 1000 );
		return true;
	}

	else
	{
		m_ConnectionStatus = CONTROL_STATUS::St_Idle;
		m_LastErrorCode = _VIDEO_CONTROL_BAD_IP_OR_PORT;
		return false;
	}

	m_LastErrorCode = _VIDEO_CONTROL_UNKNOW_ERROR;
	return false;
}

void CVideoControl_2_0::Disconnect()
{
	SendCommamd( NET_REQ_CONTROL_EXIT, NULL, 0 );
	Sleep( 50 );
	if( m_VideoControlTCPConnection.IsConnected() )
		m_VideoControlTCPConnection.Disconnect();
	m_ConnectionStatus = CONTROL_STATUS::St_Idle;
}

void CVideoControl_2_0::SetConfig( VIDEO_2_0_VIDEO_CONTROL_CONNECTION_CONFIG* st )
{
	if( st )
		memcpy( &m_ConnectionConfig, st, sizeof( VIDEO_2_0_VIDEO_CONTROL_CONNECTION_CONFIG ) );
}


bool CVideoControl_2_0::StartVideoControl()
{
	int nret;
	if( m_ConnectionStatus == CONTROL_STATUS::St_Connected )
	{
		if( m_ControlThread == NULL )
		{
			if( (nret = CheckIDPassword( m_ConnectionConfig.UserID, m_ConnectionConfig.Password )) == 1 )
			{
				m_ConnectionStatus = CONTROL_STATUS::St_IDPassed;
				m_ControlThread = CreateThread( NULL, 0, ControlThreadEnter_2_0, this, 0, (LPDWORD)&m_ControlThreadID );
				m_LastErrorCode = 0;
				return true;
			}

			else
			{
				if( nret == 0 )
				{
					m_ConnectionStatus = CONTROL_STATUS::St_IDNotPassed;
					m_LastErrorCode = _VIDEO_CONTROL_BAD_IP_OR_PORT;
					return false;
				}

				else
				{
					m_LastErrorCode = WSAGetLastError();
					// error? receive nothing while check id & password
				}
			}
		}

		else
			m_LastErrorCode = _VIDEO_CONTROL_CONTROL_STARTED_ALREADY;
	}

	else
	{
		if( m_ConnectionStatus < CONTROL_STATUS::St_Connected )
		{
			m_LastErrorCode = _VIDEO_CONTROL_NOT_CONNECTED;
			return false;
		}

		else
		{
			m_LastErrorCode = _VIDEO_CONTROL_CONTROL_STARTED_ALREADY;
			return false;
		}
	}
	m_LastErrorCode = _VIDEO_CONTROL_UNKNOW_ERROR;
	return false;
}

void CVideoControl_2_0::StopVideoControl()
{
	m_bDoControl = false;		
}

void CVideoControl_2_0::WaitForStoping( DWORD nTimeout )
{
	if( nTimeout == 0 )
	{
		if( m_ControlThreadEndingEvent )
			WaitForSingleObject( m_ControlThreadEndingEvent, INFINITE );
	}

	else
	{
		if( m_ControlThreadEndingEvent )
			WaitForSingleObject( m_ControlThreadEndingEvent, nTimeout );
	}

	if( m_ControlThreadEndingEvent )
		CloseHandle( m_ControlThreadEndingEvent );
	m_ControlThreadEndingEvent = NULL;

	if( m_ControlThread )
		CloseHandle( m_ControlThread );
	m_ControlThread = NULL;	
}

int CVideoControl_2_0::CheckIDPassword( char* ID, char* Password )
{
	char	tmpBuffer[128];
	int		nget = 0;

	int		nRlen = 0;
	memset( tmpBuffer, 0, 128 );
	memcpy( tmpBuffer, m_ConnectionConfig.UserID, 32 );
	memcpy( &tmpBuffer[64], m_ConnectionConfig.Password, 64 );

	*(DWORD *)(tmpBuffer+32) = NET_REQ_TOKEN_CONTROL;

	if( m_VideoControlTCPConnection.SendData( tmpBuffer, 128 ) )
	{
		nget = 0;
		while( nget < 128 )
		{
			nRlen = m_VideoControlTCPConnection.RecvData( &tmpBuffer[nget], 128 - nget );
			if( nRlen >= 0 )
				nget += nRlen;

			else
			{
				m_LastErrorCode = WSAGetLastError();
				return nRlen;
			}
		}

		if(( nget == 128 )&&(*(DWORD *)(tmpBuffer) == NET_RSP_TOKEN_SUCCESS) )
			return 1;
	}

	return 0;
}

void CVideoControl_2_0::ControlProcess()
{
	int nDatalen;
	int nGet;
	int	nTmpLength;
	DWORD dwTimecount = 0;
	DWORD dwOldTimecount = 0;
	DWORD dwLiveTime = 0;
	DWORD dwRSP = 0;
	int nRecvTimeOutCnt = 0;

	m_bDoControl = true;
	m_ConnectionStatus = CONTROL_STATUS::St_InControl;

	if( m_ControlThreadEndingEvent ) CloseHandle( m_ControlThreadEndingEvent );
	m_ControlThreadEndingEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	ResetEvent( m_ControlThreadEndingEvent );

	SendCommamd( NET_REQ_CONTROL_DIOSTATUS, NULL, 0 );
	
	while( m_bDoControl && m_VideoControlTCPConnection.IsConnected() )
	{
		nGet = 0;
		nRecvTimeOutCnt = 0;
		while( nGet != 12 && ( m_bDoControl && m_VideoControlTCPConnection.IsConnected() ) )
		{
			nTmpLength = m_VideoControlTCPConnection.RecvData( &m_ControlBuffer[nGet], 12 - nGet );
		
			if( nTmpLength > 0 )
				nGet += nTmpLength;

			else
			if( nTmpLength == - 2 )
			{
				nRecvTimeOutCnt++;
				if( nRecvTimeOutCnt > 2 )
					break;
			}

			else
			{
				m_bDoControl = false;
				m_LastErrorCode = WSAGetLastError();

				DWORD dwNLoss = NET_CONTROL_ONLY_NETWORK_LOSS;
				if( m_fnVCDCallBack )
					m_fnVCDCallBack( m_fnVCDParam, &dwNLoss, 4 );

				if( m_ControlThreadEndingEvent )
					SetEvent( m_ControlThreadEndingEvent );
				return;
			}
		}

		nRecvTimeOutCnt = 0;
		if( memcmp( m_ControlBuffer, "ACTi", 4 ) == 0 && nTmpLength != -2 )
		{
			nDatalen = *(int*)&m_ControlBuffer[8];

			if( nDatalen <= 4096 )
			{
				while( nGet != ( nDatalen + 12 ) && ( m_bDoControl && m_VideoControlTCPConnection.IsConnected() ))
				{
					nTmpLength = m_VideoControlTCPConnection.RecvData( &m_ControlBuffer[nGet], ( nDatalen + 12 ) - nGet );

					if( nTmpLength > 0 )
						nGet += nTmpLength;

					else
					if( nTmpLength == -2 )
					{
						nRecvTimeOutCnt++;
						if( nRecvTimeOutCnt > 2 )
							break;
					}

					else
					{
						m_bDoControl = false;
						m_LastErrorCode = WSAGetLastError();

						DWORD dwNLoss = NET_CONTROL_ONLY_NETWORK_LOSS;
						if( m_fnVCDCallBack )
							m_fnVCDCallBack( m_fnVCDParam, &dwNLoss, 4 );

						if( m_ControlThreadEndingEvent )
							SetEvent( m_ControlThreadEndingEvent );
						return;
					}
				}

				if( (*(int*)&m_ControlBuffer[8] + 12) == nGet )
				{
					memcpy( &dwRSP, &m_ControlBuffer[4], 4 );
					if( m_fnVCDCallBack )
						m_fnVCDCallBack( m_fnVCDParam, &m_ControlBuffer[4], nGet - 4 );
				}

			}
		}
		dwTimecount = GetTickCount();
		if ( dwTimecount >= dwOldTimecount )
			dwLiveTime = ( dwTimecount - dwOldTimecount )/1000;
		
		else
			dwLiveTime = ( dwTimecount + 0xffffffff - dwOldTimecount + 1 ) / 1000;

		if( dwLiveTime > 10 )
		{
			dwOldTimecount = dwTimecount;
			LiveCheck();
		}
	}

	if( m_ControlThreadEndingEvent )
		SetEvent( m_ControlThreadEndingEvent );
}

bool  CVideoControl_2_0::SendCommamd( DWORD dwCommand, char* pData, DWORD dwDataLen )
{
	DWORD	BytesSent = 0;
	bool	bret;
	char	cCommand[ 4096 + CMD_HEADER_SIZE ];

	memcpy( cCommand, "ACTi", 4 );
	memcpy( &cCommand[4], (char*)&dwCommand, 4 );
	memcpy( &cCommand[8], (char*)&dwDataLen, 4 );
	BytesSent = 12;

	if( dwDataLen > 0 && pData )
	{
		memcpy( &cCommand[12], pData, dwDataLen );
		BytesSent += dwDataLen;
	}

	else
		BytesSent = CMD_HEADER_SIZE;
	
	bret = m_VideoControlTCPConnection.SendData( cCommand, BytesSent );
	return bret;
}

void CVideoControl_2_0::LiveCheck()
{
	if( m_VideoControlTCPConnection.IsConnected() )
		SendCommamd( NET_REQ_CONTROL_LIVE, NULL, 0 );
}

void CVideoControl_2_0::Test()
{
	SendCommamd( NET_REQ_CONTROL_DIOSTATUS, NULL, 0 );
}

void CVideoControl_2_0::SetControlCallBack( DWORD UserParam, VIDEO_CONTROL_DATA_CALLBACK fnCallBack )
{
	m_fnVCDCallBack = fnCallBack;
	m_fnVCDParam = UserParam;
}

bool CVideoControl_2_0::ISConnected()
{
	return m_VideoControlTCPConnection.IsConnected();
}