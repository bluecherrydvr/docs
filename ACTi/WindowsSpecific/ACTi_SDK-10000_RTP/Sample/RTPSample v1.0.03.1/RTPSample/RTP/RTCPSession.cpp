#include "stdafx.h"
#include "md5.h"
#include "NetRTSP.h"
#include "NetRTP.h"
#include "NetRTCP.h"

#include "RTCPSession.h"


void CRTCPSession::InitRTCPHostConf(RTSP_HOST_CONF *Rtsp )
{
	int i = 0;

	/* Initialize the RTCP host state */
	m_RtcpConf.State = RTCP_HOST_STATE_INIT;
	/* set the RTPProt (RTP streaming protocol */
	m_RtcpConf.RTPProt = Rtsp->RTPProt;
	/* set the Server IP */
	if( m_RtcpConf.RTPProt == RTSP_RTP_UDP ) strcpy( m_RtcpConf.SrvIP, Rtsp->SrvIP);
	else strcpy( m_RtcpConf.SrvIP, Rtsp->SrvMcastIP);
	strcpy( m_RtcpConf.HostIP, Rtsp->HostIP);
	/* setup the Media configuration in RTCP */
	for(i = 0 ; i < 2 ; i ++)
	{
		m_RtcpConf.Media[i].Sock = 0;
		/* RTCP Ports */
		m_RtcpConf.Media[i].SrvPort  = Rtsp->Info.Media[i].RTCPSrvPort;
		m_RtcpConf.Media[i].HostPort = Rtsp->Info.Media[i].RTCPHostPort;
		/* SSRC */
		m_RtcpConf.Media[i].SrvSSRC  = 0;
		m_RtcpConf.Media[i].HostSSRC = (unsigned int)RTSPGetRandomNum();
		/* Timers */
		m_RtcpConf.Media[i].Data.SRTimer    = (time_t) 0;
		m_RtcpConf.Media[i].Data.SRInterval = MIN_SR_INTERVAL;
		m_RtcpConf.Media[i].Data.RRTimer    = (time_t) 0;
		m_RtcpConf.Media[i].Data.RRInterval = MIN_RR_INTERVAL/2;
		/* Statics */
		m_RtcpConf.Media[i].Data.SRPkts  = 0;
		m_RtcpConf.Media[i].Data.SRBytes = 0;
		m_RtcpConf.Media[i].Data.RRPkts  = 0;
		m_RtcpConf.Media[i].Data.RRBytes = 0;
	}
	return;
}

int CRTCPSession::RTSPGetRandomNum(void)
{
	time_t seed;

	time(&seed);
	srand((unsigned int)seed);
	return rand();
}

int CRTCPSession::CreateRTCPHostSockets()
{
	RTCP_MEDIA         *Media;
	int                i = 0;

	for(i = 0 ; i < 2 ; i ++)
	{
		Media = &m_RtcpConf.Media[i];
		/* Create the RTCP Host sockets */
		if(Media->SrvPort && Media->HostPort)
		{
			if( ( Media->Sock=(int)CreateUDPSocket( NULL, Media->HostPort ) ) != INVALID_SOCKET )
			{
				/* set the reuse address socket option to speed up 
				 * re-openning these ports */
				if( SetSockOptReusedAddr( Media->Sock) == NET_ERROR )
				{
					/* fail of set the Reused Address socket option */
					CloseSession();
					return SOCKET_ERROR;
				}
				if( m_RtcpConf.RTPProt == RTSP_RTP_MULTICAST)
				{
					/* disable the loopback function */
					if(SetSockOptLoopback( Media->Sock, NET_SOCK_LOOP_OFF ) == NET_ERROR)
					{
						/* fail of disabling loopback socket option */
						CloseSession();
						return SOCKET_ERROR;
					}
					/* add RTP Server's Multicast address to socket's 
					 * multicast membership */
					if( AddSockOptMembership( Media->Sock, m_RtcpConf.HostIP, m_RtcpConf.SrvIP) == NET_ERROR)
					{
						/* fail of adding server's Mcast IP to socket membership */
						CloseSession();
						return SOCKET_ERROR;
					}
				}
			}
			else
			{ 
				/* fail to create a UDP socket, close all RTCP sockets and 
				 * return error */
				CloseSession();
				return SOCKET_ERROR;
			}
		} /* else => This media is not available */
	}
	return 0;
}

int CRTCPSession::RTCPHostHandler()
{
	struct timeval Tout;
	time_t         Tnow;
	fd_set         RxFd;
	int            MaxFd = 0;
	int            n     = 0;
	int            i     = 0;

	/* Setup the polling timer to probe the Socket buffer to see if there
	 * is RTCP packets from server */
	Tout.tv_sec  = 0;
	Tout.tv_usec = 500*1000; /* 500 msec */
	FD_ZERO(&RxFd);
	MaxFd = 0;
	/* register the valid RTCP socket to rx_fd */
	for(i = 0 ; i < 2 ; i ++)
	{
		if( m_RtcpConf.Media[i].Sock)
		{
			if( m_RtcpConf.Media[i].Sock > MaxFd) MaxFd = m_RtcpConf.Media[i].Sock;
			FD_SET( m_RtcpConf.Media[i].Sock, &RxFd);
		}
	}	
	n = select( MaxFd+1, &RxFd, (fd_set *)0, (fd_set *)0, &Tout);
	if(n > 0)
	{
		for(i = 0 ; i < 2 ; i ++)
		{
			if( (m_RtcpConf.Media[i].Sock) && (FD_ISSET( m_RtcpConf.Media[i].Sock, &RxFd)))
			{
				/* there is a packet in the socket buffer, process it now */
				RTCPHostPktHandler( &m_RtcpConf.Media[i]);
			}
		}
	}
	else if(n < 0)
		return RTCP_ERR;

	/* check if I need to send the RTCP RR packet to the derver or not */
	time(&Tnow);
	for(i = 0 ; i < 2 ; i ++)
	{
		if( IsRRTimerExpired( &m_RtcpConf.Media[i].Data, Tnow) ) 
			RTCPSendRRPkt( &m_RtcpConf.Media[i]);
	}
	return RTCP_OK;	
}

void CRTCPSession::RTCPSendByePkt()
{
	RTCP_BYE_PKT Pkt;
	int Len = sizeof(Pkt);
	struct sockaddr_in Addr;
	int i = 0;
	RTCP_MEDIA *Media;

	for(i = 0 ; i < 2 ; i ++)
	{
		Media = &m_RtcpConf.Media[i];
		/* build the RTCP BYE Packet */
		if(Media->SrvSSRC)
		{
			/* RR Packet */
			Pkt.Head.Cnt     = 1;
			Pkt.Head.Pad     = 0;
			Pkt.Head.Ver     = 2;
			Pkt.Head.PT      = RTCP_PT_RR;
			Pkt.Head.Len     = htons(7);
			Pkt.SSRC         = htonl( Media->HostSSRC);
			Pkt.RR.SSRC      = htonl( Media->SrvSSRC);
			Pkt.RR.TotalLost = 0;
			Pkt.RR.FractLost = 0;
			Pkt.RR.LastSeq   = htons(Media->Seq);
			Pkt.RR.Jitter    = 0;
			Pkt.RR.LSR       = 0;
			Pkt.RR.DLSR      = 0;
			/* Bye packet */
			Pkt.ByeHead.Cnt = 1;
			Pkt.ByeHead.Pad = 0;
			Pkt.ByeHead.Ver = 2;
			Pkt.ByeHead.PT  = RTCP_PT_BYE;
			Pkt.ByeHead.Len = htons(1);
			Pkt.SSRCBye     = htonl( Media->HostSSRC);
			/* Setup the Address to send */
			memset( (char *)&Addr, 0, sizeof(struct sockaddr_in));
			Addr.sin_family = AF_INET;
			Addr.sin_addr.s_addr = inet_addr( m_RtcpConf.SrvIP);
			Addr.sin_port        = htons( Media->SrvPort);
			/* send the packet out */
			if(sendto(  Media->Sock, (char *)&Pkt.Head, Len, 0,
				(struct sockaddr *) &Addr, sizeof(struct sockaddr_in)) != Len)
			{
			}
		}
	}
	return;
}

void CRTCPSession::RTCPSeqNumUpdate(int Seq1, int Seq2, RTCP_HOST_CONF *Rtcp)
{

	Rtcp->Media[0].Seq = Seq1;
	Rtcp->Media[1].Seq = Seq2;
	return;
}

void CRTCPSession::RTCPHostPktHandler( RTCP_MEDIA *Media)
{
	char buf[2000];	/* the RTCP packet could not be fragmented */
	RTCP_HEADER *p;
	struct sockaddr_in Addr;
	socklen_t AddrLen = sizeof(struct sockaddr_in);
	int Len           = 0;
	int Total         = 0;

	Len = recvfrom(	Media->Sock, buf, sizeof(buf), 0, (struct sockaddr *)&Addr, &AddrLen);
	if(Len > 0)
	{
		/* check if it is a valid RTCP packet */
		if( IsValidRTCPPkt( buf, Len) == RTCP_OK )
		{
			/* Yes!! It is a valid RTCP packet, process it now */
			do
			{
				p = (RTCP_HEADER *) &buf[Total];
				/* Only Sender Report needs to be processed */
				switch(p->Comm.PT)
				{
				case RTCP_PT_SR:
					{
						/* Sender Report Packet */
						RTCPSRHandler(p, Media);
					}
					break;
				case RTCP_PT_RR:
					{
						/* drop it */
					}
					break;
				case RTCP_PT_SDES:
					break;
				case RTCP_PT_BYE:
					break;
				case RTCP_PT_APP:
					break;
				}
				Total += (ntohs(p->Comm.Len) + 1) * 4;
			} while(Total < Len);
		}
	}
	return;
}

int CRTCPSession::IsRRTimerExpired(RTCP_DATA *Data, time_t tnow)
{
	int Interval = 0;

	/* Get the Current time */
	if(Data->RRTimer)
	{
		if(tnow >= Data->RRTimer)
		{
			Interval = (int) (tnow - Data->RRTimer);
		}
		else
		{ /* over-round */
			Interval = Data->RRInterval;
		}
		if(Interval >= Data->RRInterval)
		{
			Data->RRTimer = tnow;
			return 1;
		}
	}
	else
	{
		Data->RRTimer = tnow;
		return 0;
	}
	return 0;
}

void CRTCPSession::RTCPSendRRPkt( RTCP_MEDIA *Media)
{
	RTCP_RR_PKT Pkt;
	int Len = sizeof(Pkt);
	struct sockaddr_in Addr;

	if( Media == NULL ) return;

	/* build the RTCP_RR Packet */
	if(Media->SrvSSRC)
	{
		/* RR Packet */
		Pkt.Head.Cnt     = 1;
		Pkt.Head.Pad     = 0;
		Pkt.Head.Ver     = 2;
		Pkt.Head.PT      = RTCP_PT_RR;
		Pkt.Head.Len     = htons(7);
		Pkt.SSRC         = htonl(Media->HostSSRC);
		Pkt.RR.SSRC      = htonl(Media->SrvSSRC);
		Pkt.RR.TotalLost = 0;
		Pkt.RR.FractLost = 0;
		Pkt.RR.LastSeq   = htons(Media->Seq);
		Pkt.RR.Jitter    = 0;
		Pkt.RR.LSR       = 0;
		Pkt.RR.DLSR      = 0;
		/* Sdes Packet */
		Pkt.SDESHead.Cnt = 1;
		Pkt.SDESHead.Pad = 0;
		Pkt.SDESHead.Ver = 2;
		Pkt.SDESHead.PT  = RTCP_PT_SDES;
		Pkt.SDESHead.Len = htons(2);
		Pkt.SSRCSdes     = htonl(Media->HostSSRC);
		Pkt.SdesType     = RTCP_SDES_CNAME;
		Pkt.SdesLen      = 1;
		Pkt.SdesData[0]  = 0x00;
		Pkt.SdesData[1]  = 0x00;
		
		/* Setup the Address to send */
		memset((char *)&Addr, 0, sizeof(struct sockaddr_in));
		Addr.sin_family = AF_INET;
		Addr.sin_addr.s_addr = inet_addr( m_RtcpConf.SrvIP);
		Addr.sin_port = htons( Media->SrvPort);
		/* send the packet out */
		if(sendto(	Media->Sock, (char *)&Pkt.Head, Len, 0,
			(struct sockaddr *) &Addr, sizeof(struct sockaddr_in)) != Len)
		{
		}
		else
		{
			/* update the RRInterval */
			if(Media->Data.RRInterval*2 <= MIN_RR_INTERVAL)
				Media->Data.RRInterval *= 2;

		}
	}
	return;
}

int CRTCPSession::IsValidRTCPPkt( char *buf, int Len)
{
	RTCP_HEADER *p;
	int         Total = 0;

	p = (RTCP_HEADER *) buf;
	/* All RTCP packets must be compound packets (RFC1889, section 6.1) */
	if(((ntohs(p->Comm.Len)+1)*4) == Len)
	{
		/* Not a compound packet */
		/*printf("<IsValidRTCPPkt>: error! not a compound packet\n");*/
		return RTCP_ERR;
	}
	/* Check the RTCP version, payload type and padding of the first in  */
	if( (p->Comm.PT != RTCP_PT_SR) &&
		(p->Comm.PT != RTCP_PT_RR))
	{
		printf("<IsValidRTCPPkt>: header payload type error, %d\n", p->Comm.PT);
		return RTCP_ERR;
	}
	/* Check all following parts of the compund RTCP packet. The RTP version
	 * number must be 2, and the padding bit must be zero on all apart from
	 * the last packet. */
	do {
		if(p->Comm.Ver != 2)
		{
			printf("<IsValidRTCPPkt>: header version error %d\n", p->Comm.Ver);
			return RTCP_ERR;
		}
		if(p->Comm.Pad != 0)
		{
			printf("<IsValidRTCPPkt>: header pad error %d\n", p->Comm.Pad);
			return RTCP_ERR;
		}
		Total += (ntohs(p->Comm.Len) + 1) * 4;
		p  = (RTCP_HEADER *) &buf[Total];
	} while(Total < Len);

	if(Total != Len)
	{
		printf("<IsValidRTCPPkt>: header length error %d!=%d\n", Total, Len);
		return RTCP_ERR;
	}
	return RTCP_OK;
}

void CRTCPSession::RTCPSRHandler( RTCP_HEADER *p, RTCP_MEDIA *Media)
{
	RTCP_SR *SRPkt;
	time_t  tnow;
	int     Interval = 0;

	SRPkt = &p->Head.SR.SRPkt;
	if((Media->SrvSSRC) && (Media->SrvSSRC == ntohl(SRPkt->SSRC)))
	{
		/* this is the RTCP SR to me!! update the SRTimer and SRInterval,
		 * get the currnet time in sec */
		time(&tnow);
		if(Media->Data.SRTimer)
		{
			if(tnow > Media->Data.SRTimer)
			{
				Interval = (int)(tnow - Media->Data.SRTimer);
			}
			else
			{
				/* over-round */
				Media->Data.SRTimer = tnow;
				Interval = 0;
			}
			if(Interval > Media->Data.SRInterval) Media->Data.SRInterval = Interval;
		}
		else
		{
			Media->Data.SRTimer = tnow;
		}
		/* update the server statistics */
		Media->Data.SRPkts  = SRPkt->Pkts;
		Media->Data.SRBytes = SRPkt->Bytes;

	}
	else
	{
		;
	}
	return;
}

