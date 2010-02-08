#include "StdAfx.h"
#include ".\tcpconnection.h"

/*********************************************************************

File:         Mpeeg4Adapter.cpp

Description:  SDK 10000 - KMpeg4.dll main code

Modified:     Ken Chan

Author:       Ken Chan

Prototyping:

Created:      [Ken Chan]

Modified:


*********************************************************************/

CTCPConnection::CTCPConnection(void)
{
	m_lRecvBufferSize	= 0; // using default
	m_lSendBufferSize	= 0; // using default
	m_soMySocket		= INVALID_SOCKET;
	memset( m_cSockErrorMessage, 0, sizeof(m_cSockErrorMessage) );
}

CTCPConnection::~CTCPConnection(void)
{
	if( IsConnected() )
		Disconnect();

	if( m_soMySocket != INVALID_SOCKET )
		closesocket( m_soMySocket );
	
	m_soMySocket = INVALID_SOCKET;
}

int CTCPConnection::GetLastError()
{
	return WSAGetLastError();
}


bool CTCPConnection::Connect( char* cIP, unsigned long ulPortNum, unsigned long ulConnectTimeout )
{
	int				addr_len = 0;
	fd_set			rx_fd;
	int				selectret;
	struct timeval	tout;
	int				retrytimes;

	if(INVALID_SOCKET != m_soMySocket)
	{
		Disconnect();
	}

	m_soMySocket = socket( AF_INET, SOCK_STREAM, 0 );
	if( m_soMySocket == INVALID_SOCKET ) 
		return false;

	addr_len = sizeof( sockaddr_in );
	memset( &m_stTarget_addr, 0, addr_len );

	m_stTarget_addr.sin_family		= AF_INET;
	m_stTarget_addr.sin_addr.s_addr = inet_addr( cIP );
	m_stTarget_addr.sin_port		= htons( (unsigned short)ulPortNum );

	if( !SetSocketBlockMode( SOCKET_NON_BLOCK_MODE ) )
	{
		closesocket( m_soMySocket );
		m_soMySocket = INVALID_SOCKET;
		return false;
	}

	LINGER lin;
	lin.l_onoff = 1;
	lin.l_linger = 1;
	setsockopt( m_soMySocket, SOL_SOCKET, SO_LINGER, (const char*)&lin, sizeof(lin) );
	retrytimes = ulConnectTimeout;

	if( connect( m_soMySocket, (struct sockaddr *)&m_stTarget_addr, addr_len ) < 0 ) 
	{
		if( WSAEWOULDBLOCK != WSAGetLastError() )
		{
			closesocket( m_soMySocket );
			m_soMySocket = INVALID_SOCKET;
			return false;
		}
		do
		{
			FD_ZERO( &rx_fd );
			FD_SET( m_soMySocket, &rx_fd );
			tout.tv_sec = 1;
			tout.tv_usec = 0;
			selectret = select( (int)(m_soMySocket + 1), NULL, &rx_fd, NULL, &tout );

			if( selectret > 0 ) 
			{
				/* receive something in this socket, check the socket state */
				if( CheckSocketError() == 0 )
				{
					/* restore this socket to block mode */
					if( SetSocketBlockMode( SOCKET_BLOCK_MODE ) )
					{
						if( SetRecvBufferSize( m_lRecvBufferSize ) )
						{
							if( SetSendBufferSize( m_lSendBufferSize ) )
								return true;
						}
					}

					closesocket( m_soMySocket );
					m_soMySocket = INVALID_SOCKET;
					return false;
				}
			}
		}while( retrytimes-- > 1 );
	}
	return false;
}

SOCKET CTCPConnection::GetSocket()
{
	return m_soMySocket;
}

bool CTCPConnection::SetSocketBlockMode( int nMode )
{
	unsigned long nonblocking;
	nonblocking = nMode;

	if( m_soMySocket != INVALID_SOCKET )
	{
		if( ioctlsocket( m_soMySocket, FIONBIO, &nonblocking ) == 0 )
			return true;
	}
	return false;
}

int CTCPConnection::CheckSocketError() 
{
	int	len;
	len = sizeof(m_cSockErrorMessage);

	if( m_soMySocket != INVALID_SOCKET )
	{
		if( getsockopt( m_soMySocket, SOL_SOCKET, SO_ERROR, m_cSockErrorMessage, &len) < 0 )
			return -1;

		else
			return (int)strlen( m_cSockErrorMessage );
	}
	return -1;
}

bool CTCPConnection::SendData( char* cData, int nLength )
{
	int nLen;
	nLen = nLength;

	if( m_soMySocket != INVALID_SOCKET )
	{
		if( nLen == send( m_soMySocket, cData, nLength, 0 ) )
			return true;
	}
	return false;
}

bool CTCPConnection::SetRecvBufferSize( int size )
{
	int nRecvBuf;
	nRecvBuf = size;

	if( m_soMySocket != INVALID_SOCKET )
	{
		if( nRecvBuf > 0 )
		{
			if( setsockopt( m_soMySocket, SOL_SOCKET,SO_RCVBUF, (const char*)&nRecvBuf, sizeof(long) ) == 0 )
				return true;
		}

		else if( nRecvBuf == 0 ) // using default
			return true;
	}
	return false;
}

bool CTCPConnection::SetSendBufferSize( int size )
{
	int nSendBuf;
	nSendBuf = size;

	if( m_soMySocket != INVALID_SOCKET )
	{
		if( nSendBuf > 0 )
		{
			if( setsockopt( m_soMySocket, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(long) ) == 0 )
				return true;
		}

		else if( nSendBuf == 0 ) // using default
			return true;
	}
	return false;
}
int	CTCPConnection::RecvDataEx( char* cData, int nLength , int nTimeout )
{
	int i = 0;
	fd_set			rx_fd;
	struct timeval	tout;
	try
	{
		if( m_soMySocket != INVALID_SOCKET )
		{
			FD_ZERO( &rx_fd );
			FD_SET( m_soMySocket, &rx_fd );
			tout.tv_sec = nTimeout / 1000;
			tout.tv_usec = 0;
			if( select( (int)(m_soMySocket + 1), NULL, &rx_fd, NULL, &tout ) > 0 )
			{
				i = recv( m_soMySocket, cData, nLength, 0 );

				if( WSAGetLastError() != WSAETIMEDOUT )
				{
					Disconnect();
					return -1;
				}
				return i;
			}
		}
	}

	catch(...)
	{
	}
	return SOCKET_ERROR;
}

int CTCPConnection::RecvData( char* cData, int nLength )
{
	int i = 0;

	try
	{
		if( m_soMySocket != INVALID_SOCKET )
		{
			i = recv( m_soMySocket, cData, nLength, 0 );

			if( i <= 0 )//== -1 )
			{
				if( WSAGetLastError() != WSAETIMEDOUT )
				{
					Disconnect();
					return -1;
				}

				else
					return -2;
			}

			return i;
		}
	}

	catch(...)
	{
	}
	return SOCKET_ERROR;
}

void CTCPConnection::Disconnect()
{
	SOCKET s = m_soMySocket;
	if( m_soMySocket != INVALID_SOCKET )
	{
		m_soMySocket = INVALID_SOCKET;
		shutdown( s, 0x02 ); //RECEIVE 0x00, SEND 0x01, BOTH 0x02
		closesocket( s );
	}
}

bool CTCPConnection::IsConnected()
{
	return (m_soMySocket != INVALID_SOCKET ? true : false);
}

bool CTCPConnection::SetRecvTimeout( long milliseconds )
{
	int nTimeout;
	nTimeout = milliseconds;
	if( setsockopt( m_soMySocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&nTimeout, sizeof(int) ) == 0 )
		return true;

	return false;
}