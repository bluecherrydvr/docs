#pragma once

#include "NetRTSP.h"
#include "NetRTP.h"
#include "NetRTCP.h"

#include "NetAccess.h"

#include "SimpleUDPSession.h"

class CRTCPSession : public CSimpleUDPSession
{
private:
	RTCP_HOST_CONF m_RtcpConf;
public:
	const int MAX_USED_MEDIA_TYPE;

	CRTCPSession()
		: CSimpleUDPSession(), MAX_USED_MEDIA_TYPE(2)
	{
		memset( &m_RtcpConf, 0, sizeof( RTCP_HOST_CONF));

		for( int i=0; i<MAX_USED_MEDIA_TYPE; i++)
		{
			m_RtcpConf.Media[i].Sock = INVALID_SOCKET;
		}
	}

	virtual ~CRTCPSession()
	{
		CloseSession();
	}

	RTCP_HOST_CONF & GetRtcpConf( void)
	{
		return m_RtcpConf;
	}

	void InitRTCPHostConf( RTSP_HOST_CONF *Rtsp );
	int RTSPGetRandomNum( void);
	int CreateRTCPHostSockets();
	int RTCPHostHandler();
	void RTCPSendByePkt();
	void RTCPSeqNumUpdate( int Seq1, int Seq2, RTCP_HOST_CONF *Rtcp);
	void RTCPHostPktHandler( RTCP_MEDIA *Media);
	int IsRRTimerExpired( RTCP_DATA *Data, time_t tnow);
	void RTCPSendRRPkt( RTCP_MEDIA *Media);

	int IsValidRTCPPkt( char *buf, int Len);
	void RTCPSRHandler( RTCP_HEADER *p, RTCP_MEDIA *Media);

	void CloseMedia( void)
	{
		m_RtcpConf.State = RTCP_HOST_STATE_INIT;
	}

	void CloseSession( void)
	{
		CloseSession( &m_RtcpConf);
	}

	void CloseSession( RTCP_HOST_CONF *rtcp )
	{
		int i = 0;

		for(i = 0 ; i < 2 ; i ++)
		{
			if( rtcp->Media[i].Sock)
			{
				closesocket( rtcp->Media[i].Sock);
				rtcp->Media[i].Sock = INVALID_SOCKET;
			}
		}
	}
};

