#pragma once

#include "NetRTSP.h"
#include "NetRTP.h"
#include "NetRTCP.h"
#include "SimpleTCPSession.h"

#define GET_DIGETS_NONCE		0x01
#define GET_DIGETS_REALM		0x02
#define GET_DIGETS_URI			0x04
#define GET_DIGETS_RESP			0x08


class CRTSPSession : public CSimpleTCPSession
{
private:
	RTSP_HOST_CONF m_RtspConf;

public:
	CRTSPSession()
		: CSimpleTCPSession()
	{
		memset( &m_RtspConf, 0, sizeof( RTSP_HOST_CONF));
		m_RtspConf.Sock = INVALID_SOCKET;
	}

	virtual ~CRTSPSession()
	{
		CloseSession();
	}

	RTSP_HOST_CONF & GetRtspConf( void)
	{
		return m_RtspConf;
	}

	void InitRTSPConf( const char *rtspservaddr, const int portnum, const char * szFilePath,
		const char * szUserID, const char * szPassWD,
		const char * localhostaddr );

	void RTSPDescribeReqBuildDigest(char *, int *);
	void RTSPPlayReqBuildDigest(char *, int * );
	void RTSPSetupReqBuildDigest(char *, int *, RTSP_MEDIA *);
	void RTSPTeardownReqBuildDigest(char *, int * );
	int RTSPHostOptionGet();
	int IsRTSPSrvAlive();
	int ConnectToRTSPSrv();
	int  RTSPDescribeReplyHandle(char *, int );
	void RTSPDescribeReqBuildBase64(char *, int * )
	{
	}
	void RTSPDigestRespStrBuild( char *, RTSP_DIGEST *);
	int  RTSPMediaDescribeGet( char *, int );
	int  RTSPMediaDescribeAudioGet( char *, int);
	int  RTSPMediaDescribeConfigGet( char *, int, RTSP_MEDIA *);
	int  RTSPMediaDescribeCtrlGet( char *, int, char *);
	int  RTSPMediaDescribeMcastIPGet( char *, int, char *);
	int  RTSPMediaDescribeProtocolGet( char *, int, int *, RTSP_MEDIA *);
	int  RTSPMediaDescribeTypeGet( char *, int, int *, int *);
	int  RTSPMediaDescribeVideoGet( char *, int );
	int  RTSPOptionReplyHandle(char *, int, RTSP_HOST_INFO *);
	void RTSPPlayReqBuildBase64(char *, int *)
	{
	}
	
	int  RTSPRetCodeCseq( char *, int, int *, int *);
	int  RTSPSessionIDGet(char *, int , int *, char * );
	int  RTSPSetupReplyHandle( char *, int, RTSP_MEDIA *);
	void RTSPSetupReqBuildBase64( char *, int *, RTSP_MEDIA *);
	
	void RTSPTeardownReqBuildBase64( char *, int *);
	
	int GetRTSPDigestStrings( char *buf, int len, RTSP_DIGEST *Digest);
	int IsValidRTSPFrame( char *buf, int len);
	int RTSPDigestParser( char *buf, int len, char *RetStr);
	int RTSPFindString( char *buf, int len, int *index, char *Str);
	int RTSPHostDescribeHandle();
	
	int RTSPHostSetupHandle();
	int RTSPHostPlayHandle();
	int RTSPPlayCmdSend();
	int RTSPPlayReplyHandle();
	
	int RTSPTeardownCmdSend();

	int Char2Hex( char c)
	{
		if((c >= '0') && (c <= '9')) return (c-'0');
		if((c >= 'a') && (c <= 'f')) return (10+(c-'a'));
		if((c >= 'A') && (c <= 'F')) return (10+(c-'A'));
		return 0;
	}

	int RTSPGetRandomNum( void)
	{
		time_t seed;

		time(&seed);
		srand((unsigned int)seed);
		return rand();
	}

	void CloseSession( void)
	{
		if( m_RtspConf.Sock != INVALID_SOCKET )
		{
			closesocket( m_RtspConf.Sock);
			m_RtspConf.Sock = INVALID_SOCKET;
		}
	}
};
