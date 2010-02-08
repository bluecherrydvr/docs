// MulticastChannel.cpp: implementation of the CMulticastChannel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GoClient.h"
#include "MulticastChannel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMulticastChannel::CMulticastChannel()
{

}

CMulticastChannel::~CMulticastChannel()
{

}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


int CMulticastChannel::Initialize(char *addr_str,int local_rtp_port,int local_Artp_port)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 
	wVersionRequested = MAKEWORD( 2, 0 );
 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
	    return -1;
 
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
		    HIBYTE( wsaData.wVersion ) != 0 ) {
	    WSACleanup( );
		return -2; 
	}

	SocketV=WSASocket(AF_INET,SOCK_DGRAM,IPPROTO_UDP,NULL,0,
		WSA_FLAG_OVERLAPPED|WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF); 
	if(SocketV==INVALID_SOCKET)
	{
		ShowErrorMessage();
		return -3;
	}

	BOOL ReuseAddress=TRUE;
	err=setsockopt(SocketV,SOL_SOCKET,SO_REUSEADDR,(char *)&ReuseAddress,sizeof(ReuseAddress));
	if(err!=0)
	{
		ShowErrorMessage();
		return -4;
	}

	gethostname(hostname,128);

	HOSTENT* Hostent = gethostbyname(hostname); 
	if(Hostent == NULL)
	{
		ShowErrorMessage();
		return -5;
	}
		
	CString Address ;
	unsigned char* AddrList = (unsigned char *)(Hostent->h_addr_list[0]) ;
	int len;

	Address.Format("%d.%d.%d.%d",AddrList[0],AddrList[1],AddrList[2],AddrList[3]);
//[pma] bugbug,if host has more than one IP address,it only bind to one of them,which
// maynot receive multicast data
//	Address = "169.254.60.60";
	len=sizeof(addrV);
//	WSAStringToAddress((LPSTR)(LPCSTR)Address,AF_INET,NULL,(LPSOCKADDR)&addr,&len);


	addrV.sin_family = 2;
	addrV.sin_addr.S_un.S_addr = inet_addr((LPSTR)(LPCSTR)Address);
	addrV.sin_port = htons(local_rtp_port);


	err=bind(SocketV,(PSOCKADDR)&addrV,sizeof(addrV));
	if(err!=0)
	{
		ShowErrorMessage();
		return -6;
	}

	len=sizeof(addrV);
	err=getsockname(SocketV,(PSOCKADDR)&addrV,&len);
	if(err!=0)
	{
		ShowErrorMessage();
		return -7;
	}

	int McastTTL=1;
	DWORD cbRet;
	err=WSAIoctl(SocketV,SIO_MULTICAST_SCOPE,&McastTTL,sizeof(McastTTL),NULL,0,&cbRet,NULL,NULL);
	if(err!=0)
	{
		ShowErrorMessage();
		return -8;
	}

	len=sizeof(sin_addrV);

	sin_addrV.sin_family = 2;
	sin_addrV.sin_addr.S_un.S_addr = inet_addr(addr_str);
	sin_addrV.sin_port = htons(local_rtp_port);
//	WSAStringToAddress((LPSTR)(LPCSTR)Address,AF_INET,NULL,(LPSOCKADDR)&sin_addr,&len);

	SOCKET s1=WSAJoinLeaf(SocketV,(LPSOCKADDR)&sin_addrV,sizeof(sin_addrV),NULL,NULL,NULL,NULL,JL_BOTH);
	if(s1==INVALID_SOCKET)
	{
		ShowErrorMessage();
		return -11;
	}
//audio
	SocketA=WSASocket(AF_INET,SOCK_DGRAM,IPPROTO_UDP,NULL,0,
		WSA_FLAG_OVERLAPPED|WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF); 
	if(SocketA==INVALID_SOCKET)
	{
		ShowErrorMessage();
		return -3;
	}

	err=setsockopt(SocketA,SOL_SOCKET,SO_REUSEADDR,(char *)&ReuseAddress,sizeof(ReuseAddress));
	if(err!=0)
	{
		ShowErrorMessage();
		return -4;
	}

	addrA.sin_family = 2;
	addrA.sin_addr.S_un.S_addr = inet_addr((LPSTR)(LPCSTR)Address);
	addrA.sin_port = htons(local_Artp_port);


	err=bind(SocketA,(PSOCKADDR)&addrA,sizeof(addrA));
	if(err!=0)
	{
		ShowErrorMessage();
		return -6;
	}

	len=sizeof(addrA);
	err=getsockname(SocketA,(PSOCKADDR)&addrA,&len);
	if(err!=0)
	{
		ShowErrorMessage();
		return -7;
	}

	err=WSAIoctl(SocketA,SIO_MULTICAST_SCOPE,&McastTTL,sizeof(McastTTL),NULL,0,&cbRet,NULL,NULL);
	if(err!=0)
	{
		ShowErrorMessage();
		return -8;
	}

	len=sizeof(sin_addrA);

	sin_addrA.sin_family = 2;
	sin_addrA.sin_addr.S_un.S_addr = inet_addr(addr_str);
	sin_addrA.sin_port = htons(local_Artp_port);
//	WSAStringToAddress((LPSTR)(LPCSTR)Address,AF_INET,NULL,(LPSOCKADDR)&sin_addr,&len);

	s1=WSAJoinLeaf(SocketA,(LPSOCKADDR)&sin_addrA,sizeof(sin_addrA),NULL,NULL,NULL,NULL,JL_BOTH);
	if(s1==INVALID_SOCKET)
	{
		ShowErrorMessage();
		return -11;
	}
    return 0;
}	
	
void CMulticastChannel::Uninitialize()
{
	shutdown(SocketV, SD_BOTH);
	closesocket(SocketV);
    WSACleanup();
}

int CMulticastChannel::ReadVideo(unsigned char *pBuf, int& Size)
{
	ULONG flags=0;
	struct sockaddr from_addr;
	int fromlen=sizeof(from_addr);
	int rcvd = recvfrom(SocketV,(char *)pBuf,Size,flags, &from_addr,&fromlen);

	if(rcvd<=0)
		return -1;
	else
	{
		Size = rcvd;
		return 0;
	}
}
int CMulticastChannel::ReadAudio(unsigned char *pBuf, int& Size)
{
	ULONG flags=0;
	struct sockaddr from_addr;
	int fromlen=sizeof(from_addr);
	int rcvd = recvfrom(SocketA,(char *)pBuf,Size,flags, &from_addr,&fromlen);

	if(rcvd<=0)
		return -1;
	else
	{
		Size = rcvd;
		return 0;
	}
}
