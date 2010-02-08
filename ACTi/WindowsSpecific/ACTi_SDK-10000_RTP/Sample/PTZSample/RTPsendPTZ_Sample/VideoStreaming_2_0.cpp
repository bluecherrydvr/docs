#include "StdAfx.h"
#include ".\videostreaming_2_0.h"

CVideoStreaming_2_0::CVideoStreaming_2_0(void)
{
	m_ConnectionStatus = STREAMING_STATUS::St_Idle;
	m_LastErrorCode = 0;
	m_StreamBuffer = new char[STREAM_BUFFER_SIZE];
	memset( &m_ConnectionConfig, 0, sizeof(VIDEO_2_0_VIDEO_STREAMING_CONNECTION_CONFIG) );
}

CVideoStreaming_2_0::~CVideoStreaming_2_0(void)
{
	if( m_StreamBuffer )
		delete [] m_StreamBuffer;
}

void CVideoStreaming_2_0::SetConfig( VIDEO_2_0_VIDEO_STREAMING_CONNECTION_CONFIG* st )
{
	if( st )
		memcpy( &m_ConnectionConfig, st, sizeof( VIDEO_2_0_VIDEO_STREAMING_CONNECTION_CONFIG ) );
}

bool CVideoStreaming_2_0::Connect()
{
	BOOL bRet;

	bRet = m_VideoStreamingTCPConnection.Connect( m_ConnectionConfig.IP, m_ConnectionConfig.VideoStreamingPort );

	if( bRet )
	{
		m_ConnectionStatus = STREAMING_STATUS::St_Connected;
		m_LastErrorCode = 0;
		m_VideoStreamingTCPConnection.SetRecvTimeout( m_ConnectionConfig.AliveTimeout );
		return true;
	}

	else
	{
		m_ConnectionStatus = STREAMING_STATUS::St_Idle;
		m_LastErrorCode = _VIDEO_STREAMING_BAD_IP_OR_PORT;
		return false;
	}

	m_LastErrorCode = _VIDEO_STREAMING_UNKNOW_ERROR;
	return false;
}

bool CVideoStreaming_2_0::StartVideoStreaming( int& SessionID )
{
	int nret;

	if( m_ConnectionStatus == STREAMING_STATUS::St_Connected )
	{
		if( (nret = CheckIDPassword( m_ConnectionConfig.UserID, m_ConnectionConfig.Password )) != 0 )
		{
			//OutputDebugString( "StartVideoStreaming on fine connection Success!!!\n\0" );
			SessionID = nret;
			m_ConnectionStatus = STREAMING_STATUS::St_IDPassed;
			m_ConnectionStatus = STREAMING_STATUS::St_Streaming;
			m_LastErrorCode = 0;
			return true;
		}

		else
		{
			if( nret == 0 )
			{
				//OutputDebugString( "StartVideoStreaming on fine connection Fail( no Session ID )!!!\n\0" );
				m_ConnectionStatus = STREAMING_STATUS::St_IDNotPassed;
				m_LastErrorCode = _VIDEO_STREAMING_BAD_ID_OR_PAWWSORD;
				return false;
			}

			else
			{
				//OutputDebugString( "StartVideoStreaming on fine connection Fail( ??? )!!!\n\0" );
				// error? receive nothing while check id & password
			}
		}
	}

	else
	{
		//OutputDebugString( "StartVideoStreaming on bad connection\n\0" );
		if( m_ConnectionStatus < STREAMING_STATUS::St_Connected )
		{
			m_LastErrorCode = _VIDEO_STREAMING_NOT_CONNECTED;
			return false;
		}

		else
		{
			m_LastErrorCode = _VIDEO_STREAMING_STREAMING_STARTED_ALREADY;
			return false;
		}
	}
	m_LastErrorCode = _VIDEO_STREAMING_UNKNOW_ERROR;
	return false;
}

void CVideoStreaming_2_0::StopVideoStreaming()
{
	Disconnect();
}

int CVideoStreaming_2_0::ReadTCP20Data()
{
	int RawDataLength;
	int VideoOrAudio;
	int nGet;
	int nTmpLength;

	try
	{
		memset( m_StreamBuffer, 0, 256000 );
		if( !m_VideoStreamingTCPConnection.IsConnected() )
			return -1;
		nGet = 0;
		nTmpLength = 0;
		while( nGet != 12 && m_VideoStreamingTCPConnection.IsConnected() )
		{
			nTmpLength = m_VideoStreamingTCPConnection.RecvData( &m_StreamBuffer[nGet], 12 - nTmpLength );
			if( nTmpLength >= 0 )
				nGet += nTmpLength;

			else
			{
				//if( nTmpLength == -2 ) // WSAETIMEDOUT
				//	return 0;

				Disconnect();
				return nTmpLength;
			}
		}
		if( !m_VideoStreamingTCPConnection.IsConnected() )
			return -1;

		if( *((DWORD*)m_StreamBuffer) == 0xB2010000 )
		{
			memcpy( (BYTE*)&VideoOrAudio, &m_StreamBuffer[4], 4 );
			memcpy( (BYTE*)&RawDataLength, &m_StreamBuffer[8], 4 );
			
			if( !m_VideoStreamingTCPConnection.IsConnected() )
				return -1;

			nTmpLength = 0;
			while( nGet != ( RawDataLength + 12 ) && m_VideoStreamingTCPConnection.IsConnected() )
			{
				nTmpLength = m_VideoStreamingTCPConnection.RecvData( &m_StreamBuffer[nGet], RawDataLength + 12 - nGet );

				if( nTmpLength >= 0 )
					nGet += nTmpLength;

				else
				{
					//if( WSAGetLastError() == WSAETIMEDOUT )
					//if( nTmpLength == -2 ) // WSAETIMEDOUT
					//	return 0;
					Disconnect();
					return nTmpLength;
				}			
			}
			if( !m_VideoStreamingTCPConnection.IsConnected() ) 
				return -1;
			return nGet;
		}
	}
	catch(...)
	{
	}
	return -1;
}

bool CVideoStreaming_2_0::SetRecvBufferSize( long size )
{
	return m_VideoStreamingTCPConnection.SetRecvBufferSize( size );
}
bool CVideoStreaming_2_0::SetSendBufferSize( long size )
{
	return m_VideoStreamingTCPConnection.SetSendBufferSize( size );
}

void CVideoStreaming_2_0::Disconnect()
{
	if( m_VideoStreamingTCPConnection.IsConnected() )
		m_VideoStreamingTCPConnection.Disconnect();

	m_ConnectionStatus = STREAMING_STATUS::St_Idle;
}

int CVideoStreaming_2_0::CheckIDPassword( char* ID, char* Password )
{
	char	tmpBuffer[128];
	int		nget = 0;
	int		nRecv = 0;
	int		nSession = 0;

	memset( tmpBuffer, 0, 128 );
	memcpy( tmpBuffer, m_ConnectionConfig.UserID, 64 );
	memcpy( &tmpBuffer[64], m_ConnectionConfig.Password, 64 );

	if( m_VideoStreamingTCPConnection.SendData( tmpBuffer, 128 ) )
	{
		nget = 0;
		nRecv = 0;
		while( nget != 128 )
		{
			nRecv = m_VideoStreamingTCPConnection.RecvData( &tmpBuffer[nget], 128 - nget );
			if( nRecv > 0 )
				nget += nRecv;

			else
				return 0;
		}

		if( tmpBuffer[0] == 0 )
		{
			nSession = *(int*)(tmpBuffer + 4);
			return nSession;
		}
	}

	return 0;
}

void CVideoStreaming_2_0::DebugMessage( TCHAR *pFormat, ...)
{
#ifdef _ACTI_DEBUG_
	va_list ArgList;
	TCHAR tBuffer[1024];
	TCHAR tBuffer2[2048];
	SYSTEMTIME	lst;

	GetLocalTime( &lst );
	va_start( ArgList, pFormat );
	(void)wvsprintf( tBuffer, pFormat, ArgList );
	va_end( ArgList );
	wsprintf( tBuffer2, "%04d%02d%02d %02d%02d%02d %s\n\0", lst.wYear, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, tBuffer );
	OutputDebugString( tBuffer2 );
#endif
}

bool CVideoStreaming_2_0::SendCommamd( DWORD dwCommand, char* pData, DWORD dwDataLen )
{	
	int	BytesSent = 0;
	char	cCommand[ 4096 + CMD_HEADER_SIZE ];

	memcpy( cCommand, "ACTi", 4 );
	memcpy( &cCommand[4], &dwCommand, 4 );
	memcpy( &cCommand[8], &dwDataLen, 4 );

	if( dwDataLen > 0 && pData )
	{
		memcpy( &cCommand[12], pData, dwDataLen );
		BytesSent = CMD_HEADER_SIZE + dwDataLen;
	}

	else
		BytesSent = CMD_HEADER_SIZE;
	
	return m_VideoStreamingTCPConnection.SendData( cCommand, BytesSent );
}

int CVideoStreaming_2_0::GetOneFrame( char* pBuffer, int nBufferSize )
{
	int retLength = 0;
	retLength = ReadTCP20Data();

	if( retLength > 0 )
		memcpy( pBuffer, m_StreamBuffer, retLength );
	return retLength;
}