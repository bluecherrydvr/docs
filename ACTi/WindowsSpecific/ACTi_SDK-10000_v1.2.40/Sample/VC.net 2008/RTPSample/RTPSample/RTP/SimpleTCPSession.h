#pragma once
#include "NetAccess.h"


class CSimpleTCPSession
{
private:
public:
	SOCKET CreateTCPSocket( void)
	{
		SOCKET sock;
		if( (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		{
			return INVALID_SOCKET;
		}
		return sock;
	}

	int ReadSockTCP( SOCKET sock, char * buf, int *max_len, const unsigned long poll_time)
	{
		int len = 0;

		len = recv(sock, (char *)buf, *max_len, 0);
		if(len <= 0)
		{
			return NET_ERROR;
		}
		else
		{
			*max_len = len;
			return NET_SUCCESS;
		}
		return  NET_ERROR;
	}

	int WriteSockTCP( SOCKET sock, char *buf, int max_len, const unsigned long poll_time)
	{
		int len = 0;
		len = send(sock, (char *)buf, max_len, 0);
		if(len < 0)
		{
			return NET_ERROR;
		}
		return len;
	}

	int SockOptReusedAddrSet( SOCKET sock)
	{
		int opt = 1;

		if( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == -1)
		{
			return NET_ERROR;
		}
		return NET_SUCCESS;
	}
};