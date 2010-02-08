// UnicastChannel.h: interface for the CUnicastChannel class.
//
//////////////////////////////////////////////////////////////////////



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Channel.h"

class CUnicastChannel : public CChannel  
{
public:
	CUnicastChannel();
	virtual ~CUnicastChannel();

	int Initialize(char *addr_str,int local_rtp_port,int local_Artp_port);
	void Uninitialize();
	int ReadVideo(unsigned char *pBuf, int& Size);
	int ReadAudio(unsigned char *pBuf, int& Size);

private:

	struct sockaddr_in addrV;
	struct sockaddr_in sin_addr;
	struct sockaddr_in addrA;
	SOCKET SocketV;
	SOCKET SocketA;
	char hostname[128];
};


