
#pragma once
#include <afxsock.h>		// MFC socket extensions

#define _MCST_RECV_TIMEOUT 3000

class CMulticastConnection
{
public:
	CMulticastConnection(void);
	~CMulticastConnection(void);

private:

	//	Variables
	SOCKET				m_soMySocket;
	struct sockaddr_in	m_stTarget_addr;
	struct ip_mreq		m_imreq;
	char				m_cSockErrorMessage[256];
	int					m_socklen;
	int					CheckSocketError();

	DWORD				m_Resolution;
	DWORD				m_FPS;
	int					m_nTCPType;
	unsigned char		m_SeqHeader[21];
public:

	//	Functions
	SOCKET				GetSocket();
	int					GetLastError();
	bool				Connect( char* cIP, unsigned long ulPortNum, unsigned long ulConnectTimeout = 0/*if 0 won't check*/ );
	int					RecvData( char* cData, int nLength );
	int					SendData( char* cData, int nLength );

	bool				SetRecvTimeout( long milliseconds );
	void				Disconnect();
	bool				IsConnected();
};
