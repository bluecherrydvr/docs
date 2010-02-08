#pragma once
#include ".\multicastconnection.h"


typedef struct  {
	char	szHostName[24];		// [OUT] Host Name 		: ASCII Z STRING
	char	szProductID[8];		// [OUT] ProductID 		: ASCII Z STRING
	char	szWanIp[16];		// [OUT] WAN IP    		: ASCII Z STRING
	char	szLanIp[16];		// [OUT] LAN IP   		: ASCII Z STRING
	char	szMultiCastIp[16];	// [OUT] MULTICAST IP  	: ASCII Z STRING
	char	szMac[32];			// [OUT] MAC 			: ASCII Z STRING
	char	cType;				// [OUT] Bit0~3 		: 1: Composite, 2: S-Video
	// [OUT] Bit4~7         : 1: Video Server, 2: IPCam
	char    dummy1;
	char    dummy2;
	char    dummy3;
	char    Version[32];
	WORD	wHPort;  
	WORD	wSPortC2S;			// [IN]  Search   Port (Client to Server)
	WORD	wSPortS2C;			// [IN]  Search   Port (Server to Client)
	WORD	wRPort;				// [IN]  Register Port
	WORD	wCPort;				// [IN]  Control  Port
	WORD	wVPort;				// [IN]  Video    Port
	WORD	wMPort;				// [IN]  MultiCastPort
	WORD    dummy4;

} NET_SEARCHSERVER_6006;

typedef struct
{
	NET_SEARCHSERVER_6006  info;
	char     ModelName[16];
	char     SerialNumber[24];
	char     System; //E:Embedded P:PC System
	char     Type; //A:Encoder B:Decoder
	char     Channel; // 1~16
	char     Multiplexing; //X:None Q:Quad R:Rack Mount B:Blade
	char     AudioWay; //0:No Audio, 1: 1 way audio, 2: 2 way audio
	char     AudioType; //0:PCM 1:ADPCM
	char     MotionType; //0:WIS 1:QUAD
	char     ProtocolType; //0: Version 1.0, 1:Version 2.0;
	char     ProtocolMethod; 
	char     AnalogVideo; //N:NTSC P:PAL
	char     Resolution; //0:720x480, 1:352x240, 2:160x112, 3:720x576, 4:352x288, 5:176x144
	char     Bitrate; //0:28Kbps ~ 12:3Mbps
	char     FrameRateMode; //C:Constant V:Variable
	char     FrameRateNum; 
	char     Reserved[38];
}NET_SEARCHSERVER_6005;

#define DATA_LEN_FROM_6005		sizeof( NET_SEARCHSERVER_6005)
#define DATA_LEN_FROM_6006		sizeof( NET_SEARCHSERVER_6006)


typedef struct tagSearchServer {
	char	szPort[4];
	char	szHostName[24];		// [OUT] Host Name 		: ASCII Z STRING
	char	szProductID[8];		// [OUT] ProductID 		: ASCII Z STRING
	char	szWanIp[16];		// [OUT] WAN IP    		: ASCII Z STRING
	char	szLanIp[16];		// [OUT] LAN IP   		: ASCII Z STRING
	char	szMultiCastIp[16];	// [OUT] MULTICAST IP  	: ASCII Z STRING
	char	szMac[32];			// [OUT] MAC 			: ASCII Z STRING
	char	szVideoType;		
	char    szSearchVersion;
	char    szReserved[2];
	char    szVersion[32];
	WORD	wHPort;  
	WORD	wSPortC2S;			// [IN]  Search   Port (Client to Server)
	WORD	wSPortS2C;			// [IN]  Search   Port (Server to Client)
	WORD	wRPort;				// [IN]  Register Port
	WORD	wCPort;				// [IN]  Control  Port
	WORD	wVPort;				// [IN]  Video    Port
	WORD	wMPort;				// [IN]  MultiCastPort
	WORD    wReserved;
	char    szModelName[16];
	char    szSerialNumber[24];
	char    szSystem; //E:Embedded P:PC System
	char    szType; //A:Encoder B:Decoder
	char    szChannel; // 1~16
	char    szMultiplexing; //X:None Q:Quad R:Rack Mount B:Blade
	char    szAudioWay; //0:No Audio, 1: 1 way audio, 2: 2 way audio
	char    szAudioType; //0:PCM 1:ADPCM
	char    szMotionType; //0:WIS 1:QUAD
	char    szProtocolType; //0: Version 1.0, 1:Version 2.0;
	char    szProtocolMethod; 
	char    szAnalogVideo; //N:NTSC P:PAL
	char    szResolution; //0:720x480, 1:352x240, 2:160x112, 3:720x576, 4:352x288, 5:176x144
	char    szBitrate; //0:28Kbps ~ 12:3Mbps
	char    szFrameRateMode; //C:Constant V:Variable
	char    szFrameRateNum; 
	char    szNoUsed[38];
} NET_SEARCHSERVER_V2;

CMulticastConnection h6006;
CMulticastConnection h6005;

HANDLE h600x[2];

char *socketbuffer6006 = NULL;
int nTotal6006 = 0;
char *socketbuffer6005 = NULL;
int nTotal6005 = 0;

DWORD WINAPI Recv6005( LPVOID lp )
{
	nTotal6005 = 0;
	int nRecv = 0;
	while( (nRecv = h6005.RecvData( &socketbuffer6005[nTotal6005], 12 + sizeof(NET_SEARCHSERVER_6005) ) ) > 0 ) 
	{
		if( memcmp( &socketbuffer6005[nTotal6005], "80503736", 8 ) != 0 )
			nTotal6005 += nRecv;
	}

	return 0;
}

DWORD WINAPI Recv6006( LPVOID lp )
{
	nTotal6006 = 0;
	int nRecv = 0;

	while( (nRecv = h6006.RecvData( &socketbuffer6006[nTotal6006], 12 + sizeof(NET_SEARCHSERVER_6006) ) ) > 0 ) 
	{
		nTotal6006 += nRecv;
	}

	return 0;
}


DWORD netSearchServer_V2(char* rBuf, DWORD* totalnum)
{
	socketbuffer6006 = NULL;
	nTotal6006 = 0;
	socketbuffer6005 = NULL;
	nTotal6005 = 0;

	h6005.Connect( "", 6005 );
	h6006.Connect( "", 6006 );

	int n = 0;
	n =  1000 * DATA_LEN_FROM_6006;
	socketbuffer6006 = new char[ n ];
	memset( socketbuffer6006, 0, n );
	n =  1000 * DATA_LEN_FROM_6005;
	socketbuffer6005 = new char[ n ];
	memset( socketbuffer6005, 0, n );

	

	DWORD dwID = 0;
//	h600x[0]  = CreateThread( NULL, 0, Recv6005, NULL, 0, &dwID );
//	h600x[1]  = CreateThread( NULL, 0, Recv6006, NULL, 0, &dwID );

	SOCKET sSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );//, NULL, 0, WSA_FLAG_OVERLAPPED );

	BOOL	 ReuseAddress = TRUE;
	struct	 sockaddr_in sock_in;
	setsockopt( sSocket, SOL_SOCKET ,SO_BROADCAST,(char *)&ReuseAddress,sizeof(ReuseAddress));

	char buffer[100];

	sock_in.sin_family				= 2;
	sock_in.sin_addr.S_un.S_addr	= -1;
	sock_in.sin_port				= htons(6005);
	sprintf( buffer, "80503736\0" );

	int buflen = (int) strlen(buffer);
	int sinlen = sizeof(struct sockaddr_in);

	//WaitForMultipleObjects( 2, h600x, TRUE, INFINITE );

	h600x[0]  = CreateThread( NULL, 0, Recv6005, NULL, 0, &dwID );
	sendto( sSocket, buffer, buflen, 0, (struct sockaddr *)&sock_in, sinlen );
	WaitForSingleObject( h600x[0], INFINITE );
	CloseHandle( h600x[0] );

	h600x[1]  = CreateThread( NULL, 0, Recv6006, NULL, 0, &dwID );
	sendto( sSocket, buffer, buflen, 0, (struct sockaddr *)&sock_in, sinlen );
	WaitForSingleObject( h600x[1], INFINITE );
	CloseHandle( h600x[1] );

	shutdown( sSocket, 2 );
	closesocket( sSocket );
	
	

	int err = 0;
	sinlen = 0;
	int rsinlen = 0;
	int n6005Cnt = 0;
	int n6006Cnt = 0;

	for( int r = 0; r < nTotal6005; r++ ) 
	{
		err = memcmp( &socketbuffer6005[r], "\x41\x43\x54\x69", 4 );
		if( err == 0 )
		{ 
			sinlen = *((DWORD*)(&socketbuffer6005[r+8]));
			if( sinlen == sizeof(NET_SEARCHSERVER_6005) )
			{
				r += 4;
				memcpy( &rBuf[rsinlen], "6005", 4 );
				memcpy( &rBuf[rsinlen+4], &socketbuffer6005[r+8], sinlen );
				r += 8;
				r += sinlen - 1;
				rsinlen += sinlen + 4;
				n6005Cnt++;
			}
		}
	}

	memset( buffer, 0, 100 );
	for( int r = 0; r < nTotal6006; r++ ) 
	{
		err = memcmp( &socketbuffer6006[r], "\x41\x43\x54\x69", 4 );
		if( err == 0 )
		{ 
			sinlen = *((DWORD*)(&socketbuffer6006[r+8]));
			if( sinlen == sizeof(NET_SEARCHSERVER_6006) )
			{
				memcpy( &rBuf[rsinlen], "6006", 4 );
				rsinlen += 4;
				memcpy( &rBuf[rsinlen], &socketbuffer6006[r+12], sinlen );
				rsinlen += sinlen;
				memset( buffer, 0, 100 );
				memcpy( &rBuf[rsinlen], buffer, 92 );
				rsinlen += 92;

				r += 12 + sinlen - 1;

				NET_SEARCHSERVER_V2 pp;
				memcpy( &pp, &rBuf[rsinlen-sinlen-4-92], sizeof(NET_SEARCHSERVER_V2) );
				n6006Cnt++;
			}
		}
	}
/*
		NET_SEARCHSERVER_V2 *p;
		NET_SEARCHSERVER_V2 p2;

		p = new NET_SEARCHSERVER_V2[n6006Cnt];


		memcpy( p, rBuf, rsinlen );

		for( int ii = 0; ii < n6006Cnt; ii++ )
		{
			memcpy( &p2, &p[ii], sizeof(NET_SEARCHSERVER_V2) );//(NET_SEARCHSERVER_V2*)&p[ii * sizeof(NET_SEARCHSERVER_V2)];
			if( memcmp( &p2.szProductID[1], "T597F", 5 ) == 0 )
			{
				char pout[12];
				sprintf( pout, "%d\n", p2.szChannel );
				OutputDebugString( pout );			
			}
		}
		delete [] p;
*/
	delete [] socketbuffer6006;
	delete [] socketbuffer6005;
	h6005.Disconnect();
	h6006.Disconnect();
	*totalnum = n6005Cnt + n6006Cnt;
	return n6005Cnt + n6006Cnt;
}
