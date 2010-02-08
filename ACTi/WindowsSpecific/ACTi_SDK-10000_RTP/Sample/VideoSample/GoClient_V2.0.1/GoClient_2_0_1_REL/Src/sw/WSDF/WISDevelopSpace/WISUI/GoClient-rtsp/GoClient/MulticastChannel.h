// MulticastChannel.h: interface for the CMulticastChannel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MULTICASTCHANNEL_H__81455AC3_6ED4_49A8_8EBC_0DCDD4E3227B__INCLUDED_)
#define AFX_MULTICASTCHANNEL_H__81455AC3_6ED4_49A8_8EBC_0DCDD4E3227B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Channel.h"

class CMulticastChannel : public CChannel  
{
public:
	CMulticastChannel();
	virtual ~CMulticastChannel();

	int Initialize(char *addr_str, int local_rtp_port,int local_Artp_port);
	void Uninitialize();
	int ReadVideo(unsigned char *pBuf, int& Size);
	int ReadAudio(unsigned char *pBuf, int& Size);

private:

	struct sockaddr_in addrV;
	struct sockaddr_in sin_addrV;
	struct sockaddr_in addrA;
	struct sockaddr_in sin_addrA;
	SOCKET SocketA;
	SOCKET SocketV;
	char hostname[128];
};

#endif // !defined(AFX_MULTICASTCHANNEL_H__81455AC3_6ED4_49A8_8EBC_0DCDD4E3227B__INCLUDED_)
