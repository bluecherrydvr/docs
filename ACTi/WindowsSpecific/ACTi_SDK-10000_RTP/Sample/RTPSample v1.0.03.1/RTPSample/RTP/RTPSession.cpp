
#include "stdafx.h"
#include "RTPSession.h"
#include "Utility.h"
#include "H264.h"

CRTPSession::CRTPSession()
	: CSimpleUDPSession(), m_FrameResolution(0), MAX_USED_MEDIA_TYPE(2), FRAME_MAXSIZE( 1024*1024)
{
	memset( &m_RtpConf, 0, sizeof( RTP_HOST_CONF));
	for( int i=0; i<MAX_USED_MEDIA_TYPE; i++)
	{
		m_RtpConf.Media[i].Sock = INVALID_SOCKET;
	}

	//m_FrameBin.resize( MAX_USED_MEDIA_TYPE, new CFrameBin( FRAME_MAXSIZE ));
	m_FrameBin.clear();

	for( int i=0; i<MAX_USED_MEDIA_TYPE; i++)
	{
		m_FrameBin.push_back( new CFrameBin( FRAME_MAXSIZE ) );
	}

	sequence_header = NULL;
	sequence_header_size = 0;
	sequence_header_len = 0;
}

CRTPSession::~CRTPSession()
{
	CloseMedia();
	CloseSession( &m_RtpConf);
	ReleaseAllFrames();

	if( sequence_header )
	{
		delete [] sequence_header;
		sequence_header = NULL;
	}
	sequence_header_size = 0;
	sequence_header_len = 0;
}

void CRTPSession::InitRTPHostConf( RTSP_HOST_CONF *Rtsp )
{
	int i = 0;

	/* set the RTPProt (RTP streaming protocol */
	m_RtpConf.RTPProt = Rtsp->RTPProt;

	/* set the Server IP */
	if( m_RtpConf.RTPProt == RTSP_RTP_UDP )
		strcpy( m_RtpConf.SrvIP, Rtsp->SrvIP);
	else
		strcpy( m_RtpConf.SrvIP, Rtsp->SrvMcastIP);

	strcpy( m_RtpConf.HostIP, Rtsp->HostIP);

	/* setup the Media configuration in RTCP */
	for(i = 0 ; i < 2 ; i ++)
	{
		m_RtpConf.Media[i].State = RTP_HOST_STATE_INIT;
		m_RtpConf.Media[i].Sock = 0;

		m_RtpConf.Media[i].Type  = Rtsp->Info.Media[i].Type;
		m_RtpConf.Media[i].DataType  = Rtsp->Info.Media[i].DataType;
		
		/* RTCP Ports */
		m_RtpConf.Media[i].Port  = Rtsp->Info.Media[i].RTPHostPort;
		/* SSRC */
		m_RtpConf.Media[i].SrvSSRC      = 0;
		m_RtpConf.Media[i].Seq          = 0;
		m_RtpConf.Media[i].Data.SRPkts  = 0;
		m_RtpConf.Media[i].Data.SRBytes = 0;
		m_RtpConf.Media[i].Data.RRPkts  = 0;
		m_RtpConf.Media[i].Data.RRBytes = 0;

		m_RtpConf.Media[i].Data.ErrRRPkts = 0;
		m_RtpConf.Media[i].Data.RFrms = 0;
		m_RtpConf.Media[i].Data.RTSs = 0;
	}
}

int CRTPSession::CreateRTPHostSockets()
{
	RTP_MEDIA *Media;
	int       i = 0;

	for(i = 0 ; i < 2 ; i ++)
	{
		Media = &m_RtpConf.Media[i];
		/* Create the RTP Host sockets */
		if( Media->Port )
		{
			if( (Media->Sock=(int)CreateUDPSocket( NULL, Media->Port)) != INVALID_SOCKET )
			{

				if( SetSockOptRecvTimeout( Media->Sock, 1000*3)  == NET_ERROR )
				{
					;
				}
				if( SetSockOptRecvBufSize( Media->Sock, 1024*256) == NET_ERROR )
				{
					;
				}
				/* set the reuse address socket option to speed up
				 * re-openning these ports */
				if( SetSockOptReusedAddr( Media->Sock) == NET_ERROR )
				{
					/* fail of set the Reused Address socket option */
					CloseSession();
					return SOCKET_ERROR;
				}
				if( m_RtpConf.RTPProt == RTSP_RTP_MULTICAST )
				{
					/* disable the loopback function */
					if( SetSockOptLoopback( Media->Sock, NET_SOCK_LOOP_OFF) == NET_ERROR)
					{
						/* fail of disabling loopback socket option */
						CloseSession();
						return SOCKET_ERROR;
					}
					/* add RTP Server's Multicast address to socket's
					 * multicast membership */
					if( AddSockOptMembership( Media->Sock, m_RtpConf.HostIP, m_RtpConf.SrvIP) == NET_ERROR)
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

void CRTPSession::RTPSeqNumUpdate(int Seq1, int Seq2 )
{
	
	m_RtpConf.Media[0].Seq = Seq1;
	m_RtpConf.Media[1].Seq = Seq2;

	return;
}

int CRTPSession::RTPVideoFrameHunting( void)
{
	struct sockaddr_in Addr;
	socklen_t AddrLen = sizeof(struct sockaddr_in);
	int Len   = 0;
	RTP_HEADER *RtpHead;
	unsigned char buf[2048];

	if( m_RtpConf.Media[0].Sock == INVALID_SOCKET )
	{
		m_RtpConf.Media[0].State = RTP_HOST_STATE_INIT;
		return RTP_ERR;
	}

	do {
		Len = recvfrom(	m_RtpConf.Media[0].Sock, (char *)buf,
			sizeof( buf), 0, (struct sockaddr *)&Addr,  &AddrLen);
		if( Len <= 0 )
		{
			int err = WSAGetLastError();
			if( err == WSAETIMEDOUT )
			{
				return RTP_ERR;
			}
			else
			{
				if( m_RtpConf.Media[0].Sock != INVALID_SOCKET )
				{
					closesocket( m_RtpConf.Media[0].Sock);
					m_RtpConf.Media[0].Sock = INVALID_SOCKET;
				}
				m_RtpConf.Media[0].State = RTP_HOST_STATE_INIT;
				return RTP_ERR;
			}
		}
		if(Len <= RTP_HEADER_LEN)
		{
			return RTP_ERR;
		}

		/* Received a RTP packet, check the header */
		RtpHead = (RTP_HEADER *) buf;
		if( (RtpHead->Ver == 2) && (RtpHead->PT == RTSP_MEDIA_DYNAMIC) )
		{
			if( RtpHead->Marker )
			{
				if( m_RtpConf.Media[0].Data.RTSs == 0 )
				{
					m_RtpConf.Media[0].Data.RTSs = (unsigned long)time(NULL);
					m_RtpConf.Media[0].Data.RFrms = 0;
				}
				else
				{
					m_RtpConf.Media[0].Data.RFrms++;
				}
				return RTP_OK;
			}
		}
		else
		{
			;
		}
	} while( m_RtpConf.Media[0].State == RTP_HOST_STATE_RUN);

	return RTP_ERR;
}

int CRTPSession::RTPVideoIFrameHunting( RTP_FRAME *Frame, RTCP_HOST_CONF *Rtcp)
{
	struct sockaddr_in Addr;
	socklen_t AddrLen = sizeof(struct sockaddr_in);
	//int MaxFrame = Frame->Len;
	//int Len   = 0;
	//int Total = B2_FRAME_LEN;
	RTP_HEADER *RtpHead;
	//unsigned char buf[2000];

	if( m_RtpConf.Media[0].Sock == INVALID_SOCKET )
	{
		m_RtpConf.Media[0].State = RTP_HOST_STATE_INIT;
		return RTP_ERR;
	}

	Frame->Len = B2_FRAME_LEN;
	do
	{
		//Frame->Len = MaxFrame;
		m_RecvBufferLen = recvfrom(	m_RtpConf.Media[0].Sock, (char *)m_RecvBuffer,
			sizeof( m_RecvBuffer), 0, (struct sockaddr *)&Addr,  &AddrLen);
		if( m_RecvBufferLen <= 0 )
		{
			int err = WSAGetLastError();
			if( err == WSAETIMEDOUT )
			{
				return RTP_ERR;
			}
			else
			{
				if( m_RtpConf.Media[0].Sock != INVALID_SOCKET )
				{
					closesocket( m_RtpConf.Media[0].Sock);
					m_RtpConf.Media[0].Sock = INVALID_SOCKET;
				}
				m_RtpConf.Media[0].State = RTP_HOST_STATE_INIT;
				return RTP_ERR;
			}
		}

		if( m_RecvBufferLen <= RTP_HEADER_LEN )
		{
			return RTP_ERR;
		}

		/* Received a RTP packet, check the header */
		RtpHead = (RTP_HEADER *) m_RecvBuffer;

		DWORD dwDataLen = m_RecvBufferLen-RTP_HEADER_LEN;
		if( (RtpHead->Ver == 2) &&
			(m_RtpConf.Media[0].Type == RtpHead->PT ) &&
			(RtpHead->PT == RTSP_MEDIA_DYNAMIC ||
			RtpHead->PT == RTSP_MEDIA_JPEG_ES90000 ) )
		{
			m_RtpConf.Media[0].Data.RRPkts ++;
			m_RtpConf.Media[0].Data.RRBytes += dwDataLen;

			if( Frame->Len+(int)dwDataLen < Frame->Size )
			{
				switch( m_RtpConf.Media[0].DataType )
				{
				case 1:
					{
						memcpy( Frame->Buf+Frame->Len, &m_RecvBuffer[RTP_HEADER_LEN], dwDataLen);
						Frame->Len += dwDataLen;

						if( RtpHead->Marker )
						{
							Frame->Type = 1; // MPEG4
							if( GetRTPVideoFrameType( &Frame->Buf[Frame->Len-B2_FRAME_LEN]) == RTP_FRAME_B2 )
							{
								/* Last fragment packet, move the Last 44 B2 to the head */
								memcpy( Frame->Buf, &Frame->Buf[Frame->Len-B2_FRAME_LEN], B2_FRAME_LEN);
								Frame->Len -= B2_FRAME_LEN;
							}
							else
							{
								//Frame->Len = Total;
							}

							/* Check the Video Frame Type */
							if( GetRTPVideoFrameType( &Frame->Buf[B2_FRAME_LEN]) == RTP_FRAME_VIDEO_I )
							{
								/* this is the I frame, update the Sequence number and SSRC
								* and Statistics */
								m_RtpConf.Media[0].SrvSSRC = ntohl(RtpHead->SSRC);
								m_RtpConf.Media[0].Seq     = ntohs(RtpHead->Seq);
								/* Sync this settings to RTCP Host Database */
								Rtcp->Media[0].SrvSSRC = m_RtpConf.Media[0].SrvSSRC;
								Rtcp->Media[0].Seq     = m_RtpConf.Media[0].Seq;
								Rtcp->Media[0].Data.RRPkts  = m_RtpConf.Media[0].Data.RRPkts;
								Rtcp->Media[0].Data.RRBytes = m_RtpConf.Media[0].Data.RRBytes;

								if( m_RtpConf.Media[0].Data.RTSs == 0 )
								{
									m_RtpConf.Media[0].Data.RTSs =  (unsigned long)time(NULL);
									m_RtpConf.Media[0].Data.RFrms = 0;
								}
								else
								{
									m_RtpConf.Media[0].Data.RFrms++;
								}
								return RTP_OK;
							}
							/* P frame or Unknown vidoe frame type, drop it and retry to get the
							* I frame */
							Frame->Len = B2_FRAME_LEN;
						}
					}
					break;
				case 2:
					break;
				case 4:
					{
						if( Frame->Len == B2_FRAME_LEN )
						{
							memcpy( Frame->Buf+Frame->Len, &m_RecvBuffer[RTP_HEADER_LEN], dwDataLen);
							Frame->Len += dwDataLen;
						}
						else
						{
							int offset = (sizeof(jpeghdr)+sizeof( jpeghdr_rst));
							int i_len = dwDataLen - offset;
							memcpy( Frame->Buf+Frame->Len, &m_RecvBuffer[RTP_HEADER_LEN+offset], i_len);
							Frame->Len += i_len;
						}

						if( RtpHead->Marker )
						{
							if( Frame->Buf[B2_FRAME_LEN] == 0x00 && Frame->Buf[B2_FRAME_LEN+1] == 0x00 && 
								Frame->Buf[B2_FRAME_LEN+2] == 0x00 && Frame->Buf[B2_FRAME_LEN+3] == 0x00 && 
								Frame->Buf[B2_FRAME_LEN+4] == 0x41 )
							{

								Frame->Type = 4; // MJPEG
								if( GetRTPVideoFrameType( &Frame->Buf[Frame->Len-B2_FRAME_LEN]) == RTP_FRAME_B2 )
								{
									/* Last fragment packet, move the Last 44 B2 to the head */
									memcpy( Frame->Buf, &Frame->Buf[Frame->Len-B2_FRAME_LEN], B2_FRAME_LEN);
									Frame->Len -= B2_FRAME_LEN;
								}
								else
								{
									;
								}
								{
									/* this is the I frame, update the Sequence number and SSRC
									* and Statistics */
									m_RtpConf.Media[0].SrvSSRC = ntohl(RtpHead->SSRC);
									m_RtpConf.Media[0].Seq     = ntohs(RtpHead->Seq);
									/* Sync this settings to RTCP Host Database */
									Rtcp->Media[0].SrvSSRC = m_RtpConf.Media[0].SrvSSRC;
									Rtcp->Media[0].Seq     = m_RtpConf.Media[0].Seq;
									Rtcp->Media[0].Data.RRPkts  = m_RtpConf.Media[0].Data.RRPkts;
									Rtcp->Media[0].Data.RRBytes = m_RtpConf.Media[0].Data.RRBytes;

									if( m_RtpConf.Media[0].Data.RTSs == 0 )
									{
										m_RtpConf.Media[0].Data.RTSs =  (unsigned long)time(NULL);
										m_RtpConf.Media[0].Data.RFrms = 0;
									}
									else
									{
										m_RtpConf.Media[0].Data.RFrms++;
									}
								}
								return RTP_OK;
							}
							else
							{
								Frame->Len = B2_FRAME_LEN;
							}
						}
					}
					break;
				case 5:
					{
							char header[6];
							//RtpHead
							H26L_NAL_UNIT * h26l_nal = (H26L_NAL_UNIT *)&m_RecvBuffer[RTP_HEADER_LEN];
							if( h26l_nal->TYPE >= 1 && h26l_nal->TYPE <=23 )
							{
								if( m_RecvBuffer[RTP_HEADER_LEN] == 0x67 )
								{
									int seqhlen = dwDataLen - 1;
									BYTE * seqptr = &m_RecvBuffer[RTP_HEADER_LEN+1];

									header[0] = 0x00;
									header[1] = 0x00;
									header[2] = 0x00;
									header[3] = 0x01;
									header[4] = 0x67;

									if( GetRTPVideoFrameType( &m_RecvBuffer[m_RecvBufferLen-B2_FRAME_LEN]) == RTP_FRAME_B2 )
									{
										seqhlen -= B2_FRAME_LEN;
									}

									{
										if( sequence_header )
										{
											delete [] sequence_header;
											sequence_header = NULL;
										}

										sequence_header_size = seqhlen+6;
										sequence_header = new unsigned char[seqhlen+6];
										sequence_header_len = 0;

										memcpy( sequence_header+sequence_header_len, header, 5);
										sequence_header_len += 5;
										memcpy( sequence_header+sequence_header_len, seqptr, seqhlen);
										sequence_header_len += seqhlen;
									}
								}
							}
							else if( h26l_nal->TYPE == 28 )
							{
								FU_HAEDER * fuheader = NULL;
								fuheader = (FU_HAEDER *)&m_RecvBuffer[RTP_HEADER_LEN+1];

								if( fuheader->S == 1 )
								{
									if( (unsigned char)m_RecvBuffer[RTP_HEADER_LEN+2] == 0x88 )
									{	// I_Frame
										// Sequence + I Frame
										memcpy( Frame->Buf+Frame->Len, sequence_header, sequence_header_len);
										Frame->Len += sequence_header_len;
									}
									else
									{
										;
									}
									header[0] = 0x00;
									header[1] = 0x00;
									header[2] = 0x00;
									header[3] = 0x01;
									header[4] = (h26l_nal->NRI & 0x03)<<5 | fuheader->TYPE&0x2F;

									memcpy( Frame->Buf+Frame->Len, header, 5);
									Frame->Len += 5;
								}
								memcpy( Frame->Buf+Frame->Len, m_RecvBuffer+(RTP_HEADER_LEN+2), dwDataLen - 2 );
								Frame->Len += (dwDataLen - 2);

								if( RtpHead->Marker == 1 )
								{
									Frame->Type = 5;
									if( GetRTPVideoFrameType( &Frame->Buf[Frame->Len-B2_FRAME_LEN]) == RTP_FRAME_B2 )
									{
										memcpy( Frame->Buf, &Frame->Buf[Frame->Len-B2_FRAME_LEN], B2_FRAME_LEN);
										B2_FRAME * b2 = (B2_FRAME *)Frame->Buf;
										if( b2->Head.Ext_b2_len != 0 )
										{
											Frame->Len = B2_FRAME_LEN;
											return 0;
										}
										Frame->Type = b2->Head.Type;
										Frame->Len = Frame->Len-B2_FRAME_LEN;
									}
									else
									{
										;
									}

									{
										/* this is the I frame, update the Sequence number and SSRC
										* and Statistics */
										m_RtpConf.Media[0].SrvSSRC = ntohl(RtpHead->SSRC);
										m_RtpConf.Media[0].Seq     = ntohs(RtpHead->Seq);
										/* Sync this settings to RTCP Host Database */
										Rtcp->Media[0].SrvSSRC = m_RtpConf.Media[0].SrvSSRC;
										Rtcp->Media[0].Seq     = m_RtpConf.Media[0].Seq;
										Rtcp->Media[0].Data.RRPkts  = m_RtpConf.Media[0].Data.RRPkts;
										Rtcp->Media[0].Data.RRBytes = m_RtpConf.Media[0].Data.RRBytes;

										if( m_RtpConf.Media[0].Data.RTSs == 0 )
										{
											m_RtpConf.Media[0].Data.RTSs =  (unsigned long)time(NULL);
											m_RtpConf.Media[0].Data.RFrms = 0;
										}
										else
										{
											m_RtpConf.Media[0].Data.RFrms++;
										}
									}

									if( Frame->Buf[B2_FRAME_LEN+4] == 0x67)
									{
										return RTP_OK;
									}
									else
									{
										Frame->Len = B2_FRAME_LEN;
									}
								}
								else
								{
									;
								}
							}
							else if( h26l_nal->TYPE == 29 )
							{
								;
							}
							else
							{
								;
							}

						}

					break;
				default:
					break;
				}
			}
			else
			{
				Frame->Len = B2_FRAME_LEN;
			}
		}
		else
		{
			m_RtpConf.Media[0].Data.ErrRRPkts++;
			Frame->Len = B2_FRAME_LEN;
		}
	} while( m_RtpConf.Media[0].State == RTP_HOST_STATE_RUN);
	return RTP_ERR;
}

int CRTPSession::RTPFrameVideoGet(RTP_FRAME *Frame, RTCP_HOST_CONF *Rtcp)
{
	struct sockaddr_in Addr;
	socklen_t AddrLen = sizeof(struct sockaddr_in);
	
	//int Total = B2_FRAME_LEN;

	RTP_HEADER    *RtpHead;
	//unsigned char buf[2048];


	if( m_RtpConf.Media[0].Sock == INVALID_SOCKET )
	{
		m_RtpConf.Media[0].State = RTP_HOST_STATE_INIT;
		return RTP_ERR;
	}

	Frame->Len = B2_FRAME_LEN;
	do
	{
		m_RecvBufferLen = recvfrom( m_RtpConf.Media[0].Sock, (char *)m_RecvBuffer, sizeof(m_RecvBuffer),
			0, (struct sockaddr *)&Addr, &AddrLen);
		if( m_RecvBufferLen <= 0 )
		{
			int err = WSAGetLastError();
			if( err == WSAETIMEDOUT )
			{
				return RTP_ERR;
			}
			else
			{
				if( m_RtpConf.Media[0].Sock != INVALID_SOCKET )
				{
					closesocket( m_RtpConf.Media[0].Sock);
					m_RtpConf.Media[0].Sock = INVALID_SOCKET;
				}
				m_RtpConf.Media[0].State = RTP_HOST_STATE_INIT;
				return RTP_ERR;
			}
		}

		if( m_RecvBufferLen <= (int)RTP_HEADER_LEN)
		{
			return RTP_ERR;
		}

		
		/* Received a RTP packet, check the header */
		RtpHead = (RTP_HEADER *) m_RecvBuffer;
		DWORD dwDataLen = m_RecvBufferLen-RTP_HEADER_LEN;
		int uiSSrc = ntohl(RtpHead->SSRC);
		if( (RtpHead->Ver == 2) 
			&& (m_RtpConf.Media[0].Type == RtpHead->PT )
			 && (RtpHead->PT == RTSP_MEDIA_DYNAMIC || RtpHead->PT == RTSP_MEDIA_JPEG_ES90000 ) &&
			(Rtcp->Media[0].SrvSSRC == uiSSrc) )
		{

			m_RtpConf.Media[0].Seq = ((m_RtpConf.Media[0].Seq + 1) & 0x0ffff);
			int seq = ntohs(RtpHead->Seq);
			if( m_RtpConf.Media[0].Seq == seq )
			{
				/* this is the Video MPEG4 frame!! */
				if( Frame->Len+(int)dwDataLen < Frame->Size)
				{
					switch( m_RtpConf.Media[0].DataType )
					{
					case 1:
						{
							Frame->Type = 1;
							memcpy( Frame->Buf+Frame->Len, &m_RecvBuffer[RTP_HEADER_LEN], dwDataLen);
							Frame->Len += dwDataLen;
							m_RtpConf.Media[0].Data.RRPkts  ++;
							m_RtpConf.Media[0].Data.RRBytes += dwDataLen;
							if(RtpHead->Marker)
							{
								/* Check the Video Frame Type */
								if( GetRTPVideoFrameType( &Frame->Buf[Frame->Len-B2_FRAME_LEN]) == RTP_FRAME_B2 )
								{
									/* Last fragment packet, move the Last 44 B2 to the head */
									memcpy( Frame->Buf, &Frame->Buf[Frame->Len-B2_FRAME_LEN], B2_FRAME_LEN);
									Frame->Len -= B2_FRAME_LEN;
								}
								else
								{
									;
								}

								/* update teh statictics */
								Rtcp->Media[0].Seq = m_RtpConf.Media[0].Seq;
								Rtcp->Media[0].Data.RRPkts  = m_RtpConf.Media[0].Data.RRPkts;
								Rtcp->Media[0].Data.RRBytes = m_RtpConf.Media[0].Data.RRBytes;
								if( m_RtpConf.Media[0].Data.RTSs == 0 )
								{
									m_RtpConf.Media[0].Data.RTSs =  (unsigned long)time(NULL);
									m_RtpConf.Media[0].Data.RFrms = 0;
								}
								else
								{
									m_RtpConf.Media[0].Data.RFrms++;
								}
								return RTP_OK;
							}
						}
						break;
					case 2:
						break;
					case 4:
						{
							if( Frame->Len == B2_FRAME_LEN )
							{
								memcpy( Frame->Buf+Frame->Len, &m_RecvBuffer[RTP_HEADER_LEN], dwDataLen);
								Frame->Len += dwDataLen;
							}
							else
							{
								int offset = (sizeof(jpeghdr)+sizeof( jpeghdr_rst)) ;
								int i_len = dwDataLen - offset;
								memcpy( Frame->Buf+Frame->Len, &m_RecvBuffer[RTP_HEADER_LEN+offset], i_len);
								Frame->Len += i_len;
							}

							if( RtpHead->Marker )
							{
								if( Frame->Buf[B2_FRAME_LEN] == 0x00 && Frame->Buf[B2_FRAME_LEN+1] == 0x00 && 
									Frame->Buf[B2_FRAME_LEN+2] == 0x00 && Frame->Buf[B2_FRAME_LEN+3] == 0x00 && 
									Frame->Buf[B2_FRAME_LEN+4] == 0x41 )
								{

									Frame->Type = 4; // MJPEG
									if( GetRTPVideoFrameType( &Frame->Buf[Frame->Len-B2_FRAME_LEN]) == RTP_FRAME_B2 )
									{
										/* Last fragment packet, move the Last 44 B2 to the head */
										memcpy( Frame->Buf, &Frame->Buf[Frame->Len-B2_FRAME_LEN], B2_FRAME_LEN);
										Frame->Len -= B2_FRAME_LEN;
									}
									else
									{
										;
									}
									{
										/* this is the I frame, update the Sequence number and SSRC
										* and Statistics */
										m_RtpConf.Media[0].SrvSSRC = ntohl(RtpHead->SSRC);
										m_RtpConf.Media[0].Seq     = ntohs(RtpHead->Seq);
										/* Sync this settings to RTCP Host Database */
										Rtcp->Media[0].SrvSSRC = m_RtpConf.Media[0].SrvSSRC;
										Rtcp->Media[0].Seq     = m_RtpConf.Media[0].Seq;
										Rtcp->Media[0].Data.RRPkts  = m_RtpConf.Media[0].Data.RRPkts;
										Rtcp->Media[0].Data.RRBytes = m_RtpConf.Media[0].Data.RRBytes;

										if( m_RtpConf.Media[0].Data.RTSs == 0 )
										{
											m_RtpConf.Media[0].Data.RTSs =  (unsigned long)time(NULL);
											m_RtpConf.Media[0].Data.RFrms = 0;
										}
										else
										{
											m_RtpConf.Media[0].Data.RFrms++;
										}
									}
									return RTP_OK;
								}
								else
								{
									Frame->Len = B2_FRAME_LEN;
								}
							}
						}
						break;
					case 5:
						{
							char header[6];
							//RtpHead
							H26L_NAL_UNIT * h26l_nal = (H26L_NAL_UNIT *)&m_RecvBuffer[RTP_HEADER_LEN];
							if( h26l_nal->TYPE >= 1 && h26l_nal->TYPE <=23 )
							{
								if( m_RecvBuffer[RTP_HEADER_LEN] == 0x67 )
								{
									int seqhlen = dwDataLen - 1;
									BYTE * seqptr = &m_RecvBuffer[RTP_HEADER_LEN+1];

									header[0] = 0x00;
									header[1] = 0x00;
									header[2] = 0x00;
									header[3] = 0x01;
									header[4] = 0x67;

									if( GetRTPVideoFrameType( &m_RecvBuffer[m_RecvBufferLen-B2_FRAME_LEN]) == RTP_FRAME_B2 )
									{
										seqhlen -= B2_FRAME_LEN;
									}

									{
										if( sequence_header )
										{
											delete [] sequence_header;
											sequence_header = NULL;
										}

										sequence_header_size = seqhlen+6;
										sequence_header = new unsigned char[seqhlen+6];
										sequence_header_len = 0;

										memcpy( sequence_header+sequence_header_len, header, 5);
										sequence_header_len += 5;
										memcpy( sequence_header+sequence_header_len, seqptr, seqhlen);
										sequence_header_len += seqhlen;
									}

									//Frame->Len = B2_FRAME_LEN; // all reset
									//memcpy( Frame->Buf+Frame->Len, header, 5);
									//Frame->Len += 5;

									//memcpy( Frame->Buf+Frame->Len, seqptr, seqhlen);
									//Frame->Len += seqhlen;
								}
							}
							else if( h26l_nal->TYPE == 28 )
							{
								FU_HAEDER * fuheader = NULL;
								fuheader = (FU_HAEDER *)&m_RecvBuffer[RTP_HEADER_LEN+1];

								if( fuheader->S == 1 )
								{
									if( (unsigned char)m_RecvBuffer[RTP_HEADER_LEN+2] == 0x88 )
									{	// I_Frame
										// Sequence + I Frame
										//memcpy( Frame->Buf+Frame->Len, media.sequence_header, media.sequence_header_len);
										//Frame->Len += media.sequence_header_len;
										memcpy( Frame->Buf+Frame->Len, sequence_header, sequence_header_len);
										Frame->Len += sequence_header_len;
									}
									else
									{
										//Frame->Len = B2_FRAME_LEN;
									}
									header[0] = 0x00;
									header[1] = 0x00;
									header[2] = 0x00;
									header[3] = 0x01;
									header[4] = (h26l_nal->NRI & 0x03)<<5 | fuheader->TYPE&0x2F;

									memcpy( Frame->Buf+Frame->Len, header, 5);
									Frame->Len += 5;
								}
								memcpy( Frame->Buf+Frame->Len, m_RecvBuffer+(RTP_HEADER_LEN+2), dwDataLen - 2 );
								Frame->Len += (dwDataLen - 2);

								if( RtpHead->Marker == 1 )
								{
									Frame->Type = 5;
									if( GetRTPVideoFrameType( &Frame->Buf[Frame->Len-B2_FRAME_LEN]) == RTP_FRAME_B2 )
									{
										memcpy( Frame->Buf, &Frame->Buf[Frame->Len-B2_FRAME_LEN], B2_FRAME_LEN);
										B2_FRAME * b2 = (B2_FRAME *)Frame->Buf;
										if( b2->Head.Ext_b2_len != 0 )
										{
											Frame->Len = B2_FRAME_LEN;
											return 0;
										}
										Frame->Type = b2->Head.Type;
										Frame->Len = Frame->Len-B2_FRAME_LEN;
									}
									else
									{
									}

									{
										/* this is the I frame, update the Sequence number and SSRC
										* and Statistics */
										m_RtpConf.Media[0].SrvSSRC = ntohl(RtpHead->SSRC);
										m_RtpConf.Media[0].Seq     = ntohs(RtpHead->Seq);
										/* Sync this settings to RTCP Host Database */
										Rtcp->Media[0].SrvSSRC = m_RtpConf.Media[0].SrvSSRC;
										Rtcp->Media[0].Seq     = m_RtpConf.Media[0].Seq;
										Rtcp->Media[0].Data.RRPkts  = m_RtpConf.Media[0].Data.RRPkts;
										Rtcp->Media[0].Data.RRBytes = m_RtpConf.Media[0].Data.RRBytes;

										if( m_RtpConf.Media[0].Data.RTSs == 0 )
										{
											m_RtpConf.Media[0].Data.RTSs =  (unsigned long)time(NULL);
											m_RtpConf.Media[0].Data.RFrms = 0;
										}
										else
										{
											m_RtpConf.Media[0].Data.RFrms++;
										}
									}

									return RTP_OK;
								}
								else
								{
									;
								}
							}
							else if( h26l_nal->TYPE == 29 )
							{
								;
							}
							else
							{
								;
							}

						}
						break;
					default:
						break;
					}
				}
				else
				{
					return RTP_ERR;
				}
			}
			else
			{
				m_RtpConf.Media[0].Data.ErrRRPkts++;
				m_RtpConf.Media[0].Seq = ntohs(RtpHead->Seq);
				return RTP_ERR;
			}
		}
		else
		{
			m_RtpConf.Media[0].Data.ErrRRPkts++;
			return RTP_ERR;
		}
		/* not receive the maker frame, keep going!! */
	} while( m_RtpConf.Media[0].State == RTP_HOST_STATE_RUN );
	return RTP_ERR;
}

int CRTPSession::GetRTPVideoFrameType( const unsigned char *buf, int *Type)
{
	int type = RTP_FRAME_UNKNOWN;
	if((buf[0]==0x00)&&(buf[1]==0x00)&&(buf[2]==0x01)&&(buf[3]==0xb6))
	{
		type = RTP_FRAME_VIDEO_P;
	}
	else if((buf[0]==0x00)&&(buf[1]==0x00)&&(buf[2]==0x01)&&(buf[3]==0xb0))
	{
		type = RTP_FRAME_VIDEO_I;
	}
	else if((buf[0]==0x00)&&(buf[1]==0x00)&&(buf[2]==0x01)&&(buf[3]==0xb2))
	{
		type = RTP_FRAME_B2;
	}
	else
	{
		type = RTP_FRAME_UNKNOWN;
	}

	if( Type ) *Type = type;
	return type;
}

int CRTPSession::RTPAudio1stFrameHunting(RTP_FRAME *Frame, RTCP_HOST_CONF *Rtcp)
{
	struct sockaddr_in Addr;
	socklen_t          AddrLen = sizeof(struct sockaddr_in);
	int                i     = 0;
	int                Len   = 0;
	RTP_HEADER         *RtpHead;
	unsigned char      buf[2000];

	if( m_RtpConf.Media[1].Sock == INVALID_SOCKET )
	{
		m_RtpConf.Media[1].State = RTP_HOST_STATE_INIT;
		return RTP_ERR;
	}
	Len = recvfrom( (SOCKET)m_RtpConf.Media[1].Sock, (char *)buf, 2000, 0, (struct sockaddr *)&Addr, &AddrLen);
	if( Len <= 0 )
	{
		int err = WSAGetLastError();
		if( err == WSAETIMEDOUT )
		{
			return RTP_ERR;
		}
		else
		{
			if( m_RtpConf.Media[1].Sock != INVALID_SOCKET )
			{
				closesocket( m_RtpConf.Media[1].Sock);
				m_RtpConf.Media[1].Sock = INVALID_SOCKET;
			}
			m_RtpConf.Media[1].State = RTP_HOST_STATE_INIT;
			return RTP_ERR;
		}
	}
	if( Len > (int)RTP_HEADER_LEN )
	{
		/* Received a RTP packet, check the header */
		RtpHead = (RTP_HEADER *) buf;
		if( (RtpHead->Ver == 2) && (RtpHead->PT == RTSP_MEDIA_PCM16_8KHZ) )
		{
			/* this is the Audio frame!! */
			Len -= RTP_HEADER_LEN;
			Frame->Len  = Len;
			Frame->Type = RTP_FRAME_AUDIO;
			/* do the byte swap. I assume the audio data is in even length */
			for(i = 0; i < (Len-1) ; i=i+2)
			{
				Frame->Buf[i]   = buf[i+1];
				Frame->Buf[i+1] = buf[i];
			}
			m_RtpConf.Media[1].Data.RRPkts  ++;
			m_RtpConf.Media[1].Data.RRBytes += Len;
			/* update the Sequence number and SSRC and Statistics */
			m_RtpConf.Media[1].SrvSSRC = ntohl(RtpHead->SSRC);
			m_RtpConf.Media[1].Seq     = ntohs(RtpHead->Seq);
			/* Sync this settings to RTCP Host Database */
			Rtcp->Media[1].SrvSSRC = m_RtpConf.Media[1].SrvSSRC;
			Rtcp->Media[1].Seq     = m_RtpConf.Media[1].Seq;
			Rtcp->Media[1].Data.RRPkts  = m_RtpConf.Media[1].Data.RRPkts;
			Rtcp->Media[1].Data.RRBytes = m_RtpConf.Media[1].Data.RRBytes;

			return RTP_OK;
		} 
	}
	else
	{
		;
	}
	return RTP_ERR;
}
		
int CRTPSession::RTPFrameAudioGet(RTP_FRAME *Frame, RTCP_HOST_CONF *Rtcp)
{
	struct sockaddr_in Addr;
	socklen_t          AddrLen = sizeof(struct sockaddr_in);
	int                i     = 0;
	int                Len   = 0;
	RTP_HEADER         *RtpHead;
	unsigned char      buf[2000];

	if( m_RtpConf.Media[1].Sock == INVALID_SOCKET )
	{
		m_RtpConf.Media[1].State = RTP_HOST_STATE_INIT;
		return RTP_ERR;
	}
	Len = recvfrom( (SOCKET)m_RtpConf.Media[1].Sock, (char *)buf, 2000, 0, (struct sockaddr *)&Addr, &AddrLen);
	if( Len <= 0 )
	{
		int err = WSAGetLastError();
		if( err == WSAETIMEDOUT )
		{
			return RTP_ERR;
		}
		else
		{
			if( m_RtpConf.Media[1].Sock != INVALID_SOCKET )
			{
				closesocket( m_RtpConf.Media[1].Sock);
				m_RtpConf.Media[1].Sock = INVALID_SOCKET;
			}
			m_RtpConf.Media[1].State = RTP_HOST_STATE_INIT;
			return RTP_ERR;
		}
	}
	if(Len > (int)RTP_HEADER_LEN)
	{
		/* Received a RTP packet, check the header */
		RtpHead = (RTP_HEADER *) buf;
		if( (RtpHead->Ver == 2) && (RtpHead->PT == RTSP_MEDIA_PCM16_8KHZ) && 
			(Rtcp->Media[1].SrvSSRC == ntohl(RtpHead->SSRC)))
		{
			/* this is the Audio frame!! */
			m_RtpConf.Media[1].Seq = ((m_RtpConf.Media[1].Seq+1)&0x0ffff);
			if( m_RtpConf.Media[1].Seq == ntohs(RtpHead->Seq) )
			{
				Len -= RTP_HEADER_LEN;
				Frame->Len  = Len;
				Frame->Type = RTP_FRAME_AUDIO;
				/* do the byte swap. I assume the audio data is in even length */
				for(i = 0; i < (Len-1) ; i=i+2)
				{
					Frame->Buf[i]   = buf[i+1];
					Frame->Buf[i+1] = buf[i];
				}
				m_RtpConf.Media[1].Data.RRPkts  ++;
				m_RtpConf.Media[1].Data.RRBytes += Len;
				/* update teh statictics */
				Rtcp->Media[1].Seq          = m_RtpConf.Media[1].Seq;
				Rtcp->Media[1].Data.RRPkts  = m_RtpConf.Media[1].Data.RRPkts;
				Rtcp->Media[1].Data.RRBytes = m_RtpConf.Media[1].Data.RRBytes;
				return RTP_OK;
			}

			m_RtpConf.Media[1].Seq = ntohs(RtpHead->Seq);
		}
		else
		{
			;
		}
	}
	else
	{
		;
	}
	return RTP_ERR;
}

