#include "StdAfx.h"
#include ".\multicastconnection.h"

CMulticastConnection::CMulticastConnection(void)
{
	m_soMySocket		= INVALID_SOCKET;
	memset( m_cSockErrorMessage, 0, sizeof(m_cSockErrorMessage) );
	memset(&m_stTarget_addr, 0, sizeof(struct sockaddr_in));
	memset(&m_imreq, 0, sizeof(struct ip_mreq));
}

CMulticastConnection::~CMulticastConnection(void)
{
	if( IsConnected() )
		Disconnect();

	if( m_soMySocket != INVALID_SOCKET )
		closesocket( m_soMySocket );

	m_soMySocket = INVALID_SOCKET;
}

int CMulticastConnection::GetLastError()
{
	return WSAGetLastError();
}

bool CMulticastConnection::Connect( char* cIP, unsigned long ulPortNum, unsigned long ulConnectTimeout )
{
	int status = 0;
	int socklen = 0;
	BOOL bToReuseAddress = TRUE;

	m_soMySocket = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
//	setsockopt( m_soMySocket,SOL_SOCKET ,SO_REUSEADDR,(char *)&bToReuseAddress,sizeof(bToReuseAddress) );

	m_stTarget_addr.sin_family = PF_INET;
	m_stTarget_addr.sin_port = htons( ulPortNum );
	m_stTarget_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	status = bind(m_soMySocket, (struct sockaddr *)&m_stTarget_addr, sizeof(struct sockaddr_in));

	m_socklen = sizeof(struct sockaddr_in);
	status = getsockname(m_soMySocket, (struct sockaddr *)&m_stTarget_addr, &m_socklen );
	
	SetRecvTimeout( _MCST_RECV_TIMEOUT );
	return true;
}

SOCKET CMulticastConnection::GetSocket()
{
	return m_soMySocket;
}

int CMulticastConnection::CheckSocketError() 
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

int CMulticastConnection::RecvData( char* cData, int nLength )
{
	int i = 0;

	try
	{
		if( m_soMySocket != INVALID_SOCKET )
		{
			i = recvfrom( m_soMySocket, cData, nLength, 0, (struct sockaddr *)&m_stTarget_addr, &m_socklen);

			if( i < 0 )//== -1 )
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

void CMulticastConnection::Disconnect()
{
	SOCKET s = m_soMySocket;
	if( m_soMySocket != INVALID_SOCKET )
	{
		m_soMySocket = INVALID_SOCKET;
		shutdown( s, 0x02 ); //RECEIVE 0x00, SEND 0x01, BOTH 0x02
		closesocket( s );
	}
}

bool CMulticastConnection::IsConnected()
{
	return (m_soMySocket != INVALID_SOCKET ? true : false);
}

bool CMulticastConnection::SetRecvTimeout( long milliseconds )
{
	int nTimeout;
	nTimeout = milliseconds;
	if( setsockopt( m_soMySocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&nTimeout, sizeof(int) ) == 0 )
		return true;

	return false;
}

int	CMulticastConnection::SendData( char* cData, int nLength )
{
	return sendto( m_soMySocket, cData, nLength, 0, (struct sockaddr *)&m_stTarget_addr, m_socklen );
}