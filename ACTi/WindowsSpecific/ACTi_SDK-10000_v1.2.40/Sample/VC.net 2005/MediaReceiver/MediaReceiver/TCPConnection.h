#pragma once

#define SOCKET_BLOCK_MODE       0
#define	SOCKET_NON_BLOCK_MODE	1
#define	SOCKET_SHUTDOWN_TIMEOUT	10	//	( in sec. )

class CTCPConnection
{
public:
	CTCPConnection(void);
	~CTCPConnection(void);

private:

	//	Variables
	SOCKET				m_soMySocket;
	struct sockaddr_in	m_stTarget_addr;
	char				m_cSockErrorMessage[256];
	long				m_lRecvBufferSize;
	long				m_lSendBufferSize;

	//	Functions
	bool				SetSocketBlockMode( int nMode );
	int					CheckSocketError();

public:

	//	Functions
	SOCKET				GetSocket();
	int					GetLastError();
	bool				Connect( char* cIP, unsigned long ulPortNum, unsigned long ulConnectTimeout = 0/*if 0 won't check*/ );
	bool				SendData( char* cData, int nLength );
	int					RecvData( char* cData, int nLength );
	int					RecvDataEx( char* cData, int nLength , int nTimeout = 15000 );
	bool				SetRecvBufferSize( int size );
	bool				SetRecvTimeout( long milliseconds );
	bool				SetSendBufferSize( int size );
	void				Disconnect();
	bool				IsConnected();
};
