#pragma once
#include "NetAccess.h"

class CSimpleUDPSession
{
private:

protected:
	SOCKET BindSocket( const SOCKET sock, const char *IP = NULL , const int port = 0 )
	{
		struct sockaddr_in addr;

		if( sock == INVALID_SOCKET ) return SOCKET_ERROR;
		memset((char *)&addr, 0, sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		if(IP) addr.sin_addr.s_addr = inet_addr(IP);
		else addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(port);

		if( bind( sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) < 0 )
		{
			return SOCKET_ERROR;
		}
		return sock;
	}
public:
	SOCKET CreateUDPSocket( const char *IP = NULL , const int port = 0 )
	{
		SOCKET sock = INVALID_SOCKET;
		sock = (int)socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if( sock == INVALID_SOCKET )
		{
			return INVALID_SOCKET;
		}

		if( BindSocket( sock, IP, port ) == SOCKET_ERROR )
		{
			closesocket( sock);
			return INVALID_SOCKET;
		}
		return sock;
	}

	int SetSockOptRecvTimeout( const SOCKET sock, long milliseconds )
	{
		long nTimeOut = milliseconds;

		if( setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, ( const char *)&nTimeOut, sizeof( long)) != 0 )
		{
			return NET_ERROR;
		}
		return NET_SUCCESS;
	}

	int SetSockOptRecvBufSize( const SOCKET sock, unsigned long bufsize)
	{
		int opt = bufsize;

		if(setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&opt, sizeof(opt)) == -1)
		{
			return NET_ERROR;
		}
		return NET_SUCCESS;
	}

	int SetSockOptReusedAddr( const SOCKET sock)
	{
		int opt = 1;

		if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == -1)
		{
			return NET_ERROR;
		}
		return NET_SUCCESS;
	}


	int AddSockOptMembership( const SOCKET sock, const char *LocalIP, const char *McastIP)
	{
		struct ip_mreq mreq;

		memset((char *)&mreq, 0, sizeof(mreq));
		mreq.imr_multiaddr.s_addr = inet_addr(McastIP);
		if(LocalIP)
			mreq.imr_interface.s_addr = inet_addr(LocalIP);
		else
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		
		if( setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq, sizeof(mreq)))
		{
			return NET_ERROR;
		}
		return NET_SUCCESS;
	}

	int SetSockOptLoopback( const SOCKET sock, const unsigned char loop)
	{
		if( setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (const char *)&loop, sizeof(loop)) == -1)
		{
			return NET_ERROR;
		}
		return NET_SUCCESS;
	}

};