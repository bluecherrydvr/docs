

#include "stdafx.h"
#include "md5.h"
#include "NetRTSP.h"
#include "NetRTP.h"
#include "NetRTCP.h"

#include "RTSPSession.h"
#include "SEQChecker.h"

void CRTSPSession::InitRTSPConf( const char *rtspservaddr, const int portnum, const char * szFilePath,
	const char * szUserID, const char * szPassWD,
	const char * localhostaddr )
{
	int i = 0;
	RTSP_HOST_INFO *Info = &m_RtspConf.Info;
	RTSP_MEDIA *Media = m_RtspConf.Info.Media;

	strcpy( m_RtspConf.SrvIP , rtspservaddr);	/* RTSP Server's IP address */
	strcpy( m_RtspConf.HostIP, localhostaddr);	/* Host's local IP address */

	memset( m_RtspConf.SrvMcastIP, '\0', 16);
	m_RtspConf.SrvPort = portnum;
	m_RtspConf.ConnTime = 1;      /* 1 sec timer for connection establishment */
	m_RtspConf.RespTime = 100;    /* 100msec for waiting Server's reply timer */

	sprintf( m_RtspConf.Name, "%s", szUserID);
	sprintf( m_RtspConf.Pwd, "%s", szPassWD);

	sprintf( m_RtspConf.FilePath, "%s", szFilePath);

	m_RtspConf.Encoder = RTSP_ENCODER_DIGEST;

	memset( (char *)&Info->Digest, '\0', sizeof(RTSP_DIGEST));

	/* setup URI base on the account and password */
	sprintf( Info->Digest.Uri, "rtsp://%s:%s@%s:%d%s", \
		m_RtspConf.Name, m_RtspConf.Pwd,
		m_RtspConf.SrvIP, m_RtspConf.SrvPort,
		m_RtspConf.FilePath);

	/* initialize the Midea Database */
	for(i = 0 ; i < 2 ; i ++)
	{
		memset( Media[i].Config, '\0', 128);
		Media[i].ConfigLen = 0;
		Media[i].Type = 0;
		Media[i].RTPSrvPort   = 0;
		Media[i].RTPHostPort  = 0;
		Media[i].RTCPSrvPort  = 0;
		Media[i].RTCPHostPort = 0;
		memset( Media[i].CtrlPath, '\0', 32);
	}
	return;
	
}

void CRTSPSession::RTSPDescribeReqBuildDigest(char *buf, int *len )
{

	if( strlen( m_RtspConf.Info.Digest.Nonce) )
	{
		/* there is the Nonce string from RTSP server before, Generate the
		 * chellenge string */
		sprintf( buf, "DESCRIBE rtsp://%s:%s@%s:%d%s RTSP/1.0\r\n"
					 "CSeq: %d\r\n"
					 "Accept: application/sdp\r\n"
					 "%s username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"\r\n"
					 "User-Agent: %s \r\n\r\n",
					 m_RtspConf.Name, m_RtspConf.Pwd, m_RtspConf.SrvIP, m_RtspConf.SrvPort, m_RtspConf.FilePath,
					 m_RtspConf.Info.CSeq,
					 "Authorization: Digest", m_RtspConf.Name, m_RtspConf.Info.Digest.Realm,
					 m_RtspConf.Info.Digest.Nonce, m_RtspConf.Info.Digest.Uri, m_RtspConf.Info.Digest.Resp,
					 THE_RTP_PLAYER);
		*len = (int)strlen( buf);
		return;
	}
	/* build the none encrypt describe packet */
	sprintf(buf, "DESCRIBE rtsp://%s:%s@%s:%d%s RTSP/1.0\r\n" 
				 "CSeq: %d\r\n" 
				 "Accept: application/sdp\r\n" 
				 "User-Agent: %s \r\n\r\n", 
				 m_RtspConf.Name, m_RtspConf.Pwd, m_RtspConf.SrvIP, m_RtspConf.SrvPort, m_RtspConf.FilePath,
				 m_RtspConf.Info.CSeq, 
				 THE_RTP_PLAYER);
	*len = (int)strlen(buf);
	return;
}

void CRTSPSession::RTSPPlayReqBuildDigest(char *buf, int *len )
{

	/* build the challenge string for this PLAY command */
	RTSPDigestRespStrBuild( "PLAY", &m_RtspConf.Info.Digest);
	sprintf(buf, "PLAY rtsp://%s:%s@%s:%d%s RTSP/1.0\r\n"
				 "CSeq: %d\r\n"
				 "Session: %s\r\n"
				 "Range: %s\r\n"
				 "%s username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"\r\n"
				 "User-Agent: %s \r\n\r\n", \
				 m_RtspConf.Name, m_RtspConf.Pwd, m_RtspConf.SrvIP, m_RtspConf.SrvPort, m_RtspConf.FilePath,
				 m_RtspConf.Info.CSeq,
				 m_RtspConf.Info.Session,
				 "ntp=0.0000-",
				 "Authorization: Digest", m_RtspConf.Name, m_RtspConf.Info.Digest.Realm,
				 m_RtspConf.Info.Digest.Nonce, m_RtspConf.Info.Digest.Uri, m_RtspConf.Info.Digest.Resp,
				 THE_RTP_PLAYER);
	*len = (int)strlen(buf);
	return;
}

void CRTSPSession::RTSPSetupReqBuildDigest(char *buf, int *len, RTSP_MEDIA *Media)
{
	/* build the challenge string for this SETUP command */
	RTSPDigestRespStrBuild( "SETUP", &m_RtspConf.Info.Digest);
	m_RtspConf.Info.CSeq += 1;
	if( strlen( m_RtspConf.Info.Session) == 0 )
	{
		sprintf( buf, "SETUP rtsp://%s:%s@%s:%d%s/%s RTSP/1.0\r\n" 
					 "CSeq: %d\r\n" 
					 "Transport: RTP/AVP;%s;client_port=%d-%d\r\n" 
					 "%s username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"\r\n" 
					 "User-Agent: %s\r\n\r\n", 
					 m_RtspConf.Name, m_RtspConf.Pwd, m_RtspConf.SrvIP, m_RtspConf.SrvPort, m_RtspConf.FilePath,
					 Media->CtrlPath, \
					 m_RtspConf.Info.CSeq, \
					 (m_RtspConf.RTPProt==RTSP_RTP_UDP)?"unicast":"multicast", 
					 Media->RTPHostPort, Media->RTCPHostPort, 
					 "Authorization: Digest", m_RtspConf.Name, m_RtspConf.Info.Digest.Realm, 
					 m_RtspConf.Info.Digest.Nonce,m_RtspConf.Info.Digest.Uri, m_RtspConf.Info.Digest.Resp, 
					 THE_RTP_PLAYER);
	}
	else
	{
		sprintf(buf, "SETUP rtsp://%s:%s@%s:%d%s/%s RTSP/1.0\r\n" 
				     "CSeq: %d\r\n" 
					 "Transport: RTP/AVP;%s;client_port=%d-%d\r\n" 
					 "Session: %s\r\n" 
					 "%s username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"\r\n" 
					 "User-Agent: %s\r\n\r\n", 
					 m_RtspConf.Name, m_RtspConf.Pwd, m_RtspConf.SrvIP,m_RtspConf.SrvPort, m_RtspConf.FilePath,
					 Media->CtrlPath, 
					 m_RtspConf.Info.CSeq, \
					 (m_RtspConf.RTPProt==RTSP_RTP_UDP)?"unicast":"multicast", \
					 Media->RTPHostPort, Media->RTCPHostPort, \
					 m_RtspConf.Info.Session, \
					 "Authorization: Digest", m_RtspConf.Name, m_RtspConf.Info.Digest.Realm, \
					 m_RtspConf.Info.Digest.Nonce, m_RtspConf.Info.Digest.Uri, m_RtspConf.Info.Digest.Resp, \
					 THE_RTP_PLAYER);
	}
	*len = (int)strlen(buf);
	return;
}

void CRTSPSession::RTSPTeardownReqBuildDigest(char *buf, int *len )
{
	/* build the challenge string for this SETUP command */
	RTSPDigestRespStrBuild( "TEARDOWN", &m_RtspConf.Info.Digest);
	sprintf(buf, "TEARDOWN rtsp://%s:%s@%s:%d%s RTSP/1.0\r\n" 
			 	 "CSeq: %d\r\n" 
				 "Session: %s\r\n" 
				 "%s username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"\r\n" 
				 "User-Agent: %s\r\n\r\n", 
				 m_RtspConf.Name, m_RtspConf.Pwd, m_RtspConf.SrvIP, m_RtspConf.SrvPort, m_RtspConf.FilePath,
				 m_RtspConf.Info.CSeq, 
				 m_RtspConf.Info.Session, 
				 "Authorization: Digest", m_RtspConf.Name, m_RtspConf.Info.Digest.Realm, 
				 m_RtspConf.Info.Digest.Nonce, m_RtspConf.Info.Digest.Uri, m_RtspConf.Info.Digest.Resp, 
				 THE_RTP_PLAYER);
	*len = (int)strlen(buf);
	return;
}

int CRTSPSession::RTSPHostOptionGet()
{
	char pkt[1024];
	int  len = 0;

	/* build the OPTION request packet to send to RTSP server */
	sprintf( pkt, "OPTIONS rtsp://%s:%s@%s:%d%s RTSP/1.0\r\n" 
				"CSeq: %d\r\n" 
				"User-Agent: %s \r\n\r\n", 
				m_RtspConf.Name, m_RtspConf.Pwd, m_RtspConf.SrvIP, m_RtspConf.SrvPort, m_RtspConf.FilePath,
				m_RtspConf.Info.CSeq, 
				THE_RTP_PLAYER);
	len = (int)strlen( pkt);
	/* Send this RTSP Option Req packet to Server now */
	if( WriteSockTCP( m_RtspConf.Sock, pkt, len, RTSP_TIMER_TX) == len )
	{
		/* success in sending the RTSP OPTION REQ packet, wait for server's
		* reply */
		len = sizeof( pkt);
		if(ReadSockTCP( m_RtspConf.Sock, pkt, &len, m_RtspConf.RespTime) == NET_SUCCESS)
		{
			/* reviced server's reply, get the server's command capabilities */
			return RTSPOptionReplyHandle( pkt, len, &m_RtspConf.Info);
		}
	}
	return RTSP_ERR;
}

int CRTSPSession::IsRTSPSrvAlive()
{
	char pkt[1024];
	int  len = 0;
		
	m_RtspConf.Info.CSeq += 1;
	/* build the OPTION request packet to send to RTSP server */
	sprintf(pkt, "OPTIONS rtsp://%s:%s@%s:%d%s RTSP/1.0\r\nCSeq: %d\r\nUser-Agent: %s \r\n\r\n",
		m_RtspConf.Name, m_RtspConf.Pwd, m_RtspConf.SrvIP, 
		m_RtspConf.SrvPort, m_RtspConf.FilePath, m_RtspConf.Info.CSeq, THE_RTP_PLAYER);

	len = (int)strlen( pkt);
	/* Send this RTSP Option Req packet to Server now */
	if(WriteSockTCP( m_RtspConf.Sock, pkt, len, RTSP_TIMER_TX) == len)
	{
		/* success in sending the RTSP OPTION REQ packet, wait for server's
		 * reply */
		len = sizeof(pkt);
		if(ReadSockTCP( m_RtspConf.Sock, pkt, &len, m_RtspConf.RespTime) == NET_SUCCESS)
		{
			return RTSP_OK;
		}
	}
	return RTSP_ERR;
}

int CRTSPSession::ConnectToRTSPSrv()
{
	int Sock = 0;
	struct sockaddr_in Addr;
	RTSP_HOST_INFO     *Info;

	/* create a TCP socket for RTSP session */
	if( (Sock=(int)CreateTCPSocket()) != INVALID_SOCKET )
	{
		/* Success in creating a TCP socket, set some socket option before
		 * I go further. In here, I ignore the error in setting socket option
		 * because it does not affect to RTSP signalling */
		(void)SockOptReusedAddrSet(Sock);
	   	/* connect to the server now, Set the Server address now  */
		memset((char *)&Addr, 0, sizeof(struct sockaddr_in));
		Addr.sin_family = AF_INET;
		Addr.sin_addr.s_addr = inet_addr( m_RtspConf.SrvIP);
		Addr.sin_port = htons( m_RtspConf.SrvPort);

		if( connect( Sock, (struct sockaddr *) &Addr, sizeof(struct sockaddr_in)) < 0 )
		{
			/* fail to connect to the server, leave now with error code */
			m_RtspConf.ConnState = RTSP_CONN_ERR_CONNECT;
			return SOCKET_ERROR;
		}
		/* success in connection the socket. Record this Socket */
		m_RtspConf.Sock = Sock;
		/* send the RTSP option command to get the RTSP Server's supported 
		 * RTSP commands */
		/* setup the RTSP sequence number, I start it from 1 */
		m_RtspConf.Info.CSeq = 1;
		if( RTSPHostOptionGet() == RTSP_ERR)
		{
			/* fail to connect to the server, leave now with error code */
			closesocket( m_RtspConf.Sock);
			m_RtspConf.Sock = INVALID_SOCKET;
			m_RtspConf.ConnState = RTSP_CONN_ERR_SIGNAL;
			return SOCKET_ERROR;
		}
		/* success in getting Server's Commands */
		/* check the server's supported commands, I need the DESCRIBE, SETUP, PLAY and 
		 * TERADOWN commands at least */
		Info = &m_RtspConf.Info;
		if( (Info->SrvRTSPCmdCap&RTSP_CMD_CAP_DESCRIBE) && (Info->SrvRTSPCmdCap&RTSP_CMD_CAP_SETUP) && 
			(Info->SrvRTSPCmdCap&RTSP_CMD_CAP_PLAY) && (Info->SrvRTSPCmdCap&RTSP_CMD_CAP_TEARDOWN))
		{

			if( RTSPHostDescribeHandle() == RTSP_ERR)
			{
				/* fail to handle the Describe */
				closesocket( m_RtspConf.Sock);
				m_RtspConf.Sock = INVALID_SOCKET;
				return SOCKET_ERROR;
			}
			/* success in Describe authentication, go to setup session */
			if( RTSPHostSetupHandle() == RTSP_ERR )
			{
				/* fail to handle the Describe */
				closesocket( m_RtspConf.Sock);
				m_RtspConf.Sock = INVALID_SOCKET;
				m_RtspConf.ConnState = RTSP_CONN_ERR_SIGNAL;
				return SOCKET_ERROR;
			}
			return 0;
		}

		closesocket( m_RtspConf.Sock);
		m_RtspConf.Sock = INVALID_SOCKET;
		m_RtspConf.ConnState = RTSP_CONN_ERR_SIGNAL;
		return SOCKET_ERROR;
	}
	/* Fail to create the TCP socket, return error */
	m_RtspConf.ConnState = RTSP_CONN_ERR_SOCK;
	return SOCKET_ERROR;
}

int CRTSPSession::RTSPDescribeReplyHandle(char *buf, int len )
{
	int index = 0;
	int ret   = 0;
	int Cseq  = 0;

	/* check if it is a valid RTSP packet (\r\n\r\n ending packet) */
	if( IsValidRTSPFrame(buf, len) == RTSP_OK)  {
		/* received a valid RTSP packet, get the RTSP return code */
		ret = RTSPRetCodeCseq(buf, len, &index, &Cseq);
		if( Cseq != m_RtspConf.Info.CSeq) return RTSP_ERR;
		if( ret == RTSP_RET_OK) return RTSP_RET_OK; /* authentication success */
		if( m_RtspConf.Encoder == RTSP_ENCODER_DIGEST )
		{
			if(strlen( m_RtspConf.Info.Digest.Nonce ) == 0)
			{
				/* fail of the 1st Authentication, The server gave me the 
				 * Nonce and Realm. I could used them to generate the 
				 * response string (challenge string). So, get the Nonce and
				 * Realm form this packet now */
				if( GetRTSPDigestStrings( &buf[index], (len-index), &m_RtspConf.Info.Digest) == (GET_DIGETS_NONCE|GET_DIGETS_REALM) )
				{
					return RTSP_OK;
				}
			}
		}
	}

	return RTSP_ERR;
}

void CRTSPSession::RTSPDigestRespStrBuild( char *Cmd, RTSP_DIGEST *Digest)
{
	int  len = 0;
	char buf[256];
	char data1[33];
	char data2[33];

	len = (int)strlen( m_RtspConf.Name) + 1 + (int)strlen(Digest->Realm) + 1 + (int)strlen( m_RtspConf.Pwd);
	if( len < 0 || len >= sizeof( buf) )
	{
		return;
	}
	sprintf( buf, "%s:%s:%s", m_RtspConf.Name, Digest->Realm, m_RtspConf.Pwd);
	buf[len] = '\0';
	md5_generate( buf, data1, len);

	len = (int)strlen( Cmd) + 1 + (int)strlen( Digest->Uri);
	sprintf(buf, "%s:%s", Cmd, Digest->Uri);
	buf[len] = '\0';
	md5_generate(buf, data2, len);
	
	len = 32 + 1 + (int)strlen(Digest->Nonce) + 1 + 32;
	sprintf(buf, "%s:%s:%s", data1, Digest->Nonce, data2);
	buf[len] = '\0';
	md5_generate(buf, Digest->Resp, len);
	return;
}

int CRTSPSession::RTSPMediaDescribeGet(char *buf, int len )
{
	int i   = 0;
	int pos = 0;
	int ret = 0;

	/* get the video description first */
	if( RTSPFindString( buf, len, &pos, "m=video") )
	{
		char * next = strstr( &buf[pos], "m=");
		if( next )
		{
			char * ptr = &buf[pos];
			ret = RTSPMediaDescribeVideoGet( &buf[pos], (int)(next-ptr) );
		}
		else
		{
			ret = RTSPMediaDescribeVideoGet( &buf[pos], (len-pos) );
		}
		if(ret == 0)
		{
			return RTSP_ERR;
		}
		pos += (ret-1);
	}
	else
	{
		return RTSP_ERR;
	}

	/* get the audio description, it might not have audio description if the video
	 * server doesn't have audio */
	if( RTSPFindString( &buf[pos], (len-pos), &i, "m=audio") )
	{
		pos += i;
		if( RTSPMediaDescribeAudioGet( &buf[pos], (len-pos) ) == 0)
		{
			;
		}
	}
	return RTSP_OK;
}

int CRTSPSession::RTSPMediaDescribeAudioGet(char *buf, int len )
{
	int i    = 0;
	int k    = 0;

	/* get the RTP protocol and the RTP/RTCP ports if the RTP protocol is UDP
	 * over Multicast */
	i = RTSPMediaDescribeProtocolGet( buf, len, &m_RtspConf.RTPProt, &m_RtspConf.Info.Media[1]);
	if(i == 0) return 0;

	/* find the audio media type */
	k = RTSPMediaDescribeTypeGet( &buf[i], (len-i), &m_RtspConf.Info.Media[1].Type, &m_RtspConf.Info.Media[1].DataType);
	if(k == 0) return 0;

	/* find the control path */
	i = RTSPMediaDescribeCtrlGet( &buf[k], (len-k), m_RtspConf.Info.Media[1].CtrlPath);
	if(i == 0) return 0;
									
	return i;
}

int CRTSPSession::RTSPMediaDescribeConfigGet(char *buf, int len, RTSP_MEDIA *Media)
{
	int i = 0;
	int k = 0;
	int m = 0;

	Media->ConfigLen = 0;
	if(RTSPFindString(buf, len, &i, "config="))
	{

#if 1 //andy fix
		for(k = i ; k < len ; k +=2)
		{
			if((buf[k] == 0x0d)&&(buf[k+1] == 0x0a))
			{
				Media->ConfigLen = m;
				return (k+2);
			}
			Media->Config[m] = ((Char2Hex(buf[k])<<4)&0xff) | (Char2Hex(buf[k+1])&0xf);
			m ++;
			if(m == 128) return 0;
		}
#else
		for(k = i ; k < len ; k ++)
		{
			if((buf[k] == 0x0d)&&(buf[k+1] == 0x0a))
			{
				Media->ConfigLen = m;
				return (k+2);
			}
			Media->Config[m] = Char2Hex(buf[k]);
			m ++;
			if(m == 128) return 0;
		}
#endif
	}
	return 0;
}

int CRTSPSession::RTSPMediaDescribeCtrlGet(char *buf, int len, char *CtrlPath)
{
	int i = 0;
	int k = 0;
	int m = 0;

	CtrlPath[0] = '\0';
	if(RTSPFindString(buf, len, &i, "control:"))
	{
		for(k = i ; k < len ; k ++)
		{
			if((buf[k] == 0x0d)&&(buf[k+1] == 0x0a))
			{
				CtrlPath[m] = '\0';
				return (k+2);
			}
			CtrlPath[m] = buf[k];
			m ++;
			if(m == 32)
			{
				memset(CtrlPath, '\0', 32);
				return 0;
			}
		}
	}
	return 0;
}

int CRTSPSession::RTSPMediaDescribeMcastIPGet(char *buf, int len, char *McastIP)
{
	int i = 0;
	int k = 0;

	for(i = 0 ; i < (len-3) ; i ++)
	{
		if((buf[i]=='I')&&(buf[i+1]=='P')&&(buf[i+2]=='4'))
		{
			/* skip all space characters */
			for(k = (i+3) ; k < len ; k ++)
			{
				if(buf[k] != ' ') break;
			}
			if(k == len)
				return 0;
			for(i = k ; i < len ; i ++)
			{
				if((buf[i] == 0x0d)&&(buf[i+1] == 0x0a))
				{
					buf[i] = '\0';
					memcpy(McastIP, &buf[k], (i-k));
					return (i+2);
				}
				else if(buf[i] == '/')
				{
					buf[i] = '\0';
					memcpy(McastIP, &buf[k], (i-k));
					return (i+1);
				}
				if((i-k) == 15)
					break;
			}
		}
	}
	return 0;
}

int CRTSPSession::RTSPMediaDescribeProtocolGet(char *buf, int len, int *RTPProt, RTSP_MEDIA *Media)
{
	int i    = 0;
	int k    = 0;
	int port = 0;

	/* skip all space characters */
	for(i = 0 ; i < len ; i ++)
	{
		if(buf[i] != ' ')
			break;
	}
	if(i == len) return 0;
	/* find next space characters */
	for(k = i ; i < len ; k ++)
	{
		if(buf[k] == ' ')
		{
			buf[k] = '\0';
			port = atoi(&buf[i]);
			if(port == 0)
			{
				/* unicast RTP */
				*RTPProt = RTSP_RTP_UDP;
			}
			else
			{
				*RTPProt = RTSP_RTP_MULTICAST;
				/* set the RTP/RTCP ports */
				Media->RTPSrvPort   = port;
				Media->RTPHostPort  = port;
				Media->RTCPSrvPort  = port+1;
				Media->RTCPHostPort = port+1;
			}
			return (k+1);
		}
	}
	/*printf("<RTSPMediaDescribeProtocolGet>: Unknown\n");*/
	return (k+1);
}

int CRTSPSession::RTSPMediaDescribeTypeGet(char *buf, int len, int *Type, int *DataType)
{
	int i   = 0;
	int k   = 0;
	int num = 0;

	char buffer1[32], buffer2[64];
	if(RTSPFindString(buf, len, &i, "rtpmap:"))
	{
		sscanf( &buf[i], "%s %s", buffer1, buffer2);
		num = atoi( buffer1);
	}
	else
	{
		sscanf( buf, "%s %s", buffer1, buffer2);
		num = atoi( buffer2);
	}



	{
		switch( num )
		{
		case RTSP_MEDIA_JPEG_ES90000:
			{
				*DataType = 4;
				*Type = RTSP_MEDIA_JPEG_ES90000;
				return (k+1);
			}
			break;
		case RTSP_MEDIA_DYNAMIC:
			{ /* MP4V-ES/9000 */
				*Type = RTSP_MEDIA_DYNAMIC;
				/*printf("<RTSPMediaDescribeTypeGet>:  MP4V-ES/9000\n");*/
				if( !strncmp( buffer2, "MP4V-ES", strlen("MP4V-ES")) )
				{
					*DataType = 1;	// MPEG4
				}
				else if( !strncmp( buffer2, "H264", strlen("H264")) )
				{
					*DataType = 5;	// H264
				}
				else
				{
					*DataType = 0;
				}
				return (k+1);
			}
			break;
		case RTSP_MEDIA_PCM16_8KHZ:
			{
				/* PCM, 16bit,8KHz */
				*DataType = 2;
				*Type = RTSP_MEDIA_PCM16_8KHZ;
				/*printf("<RTSPMediaDescribeTypeGet>:  PCM16/8000\n");*/
				return (k+1);
			}
			break;
		default:
			break;
		}
		return 0;
	}

	return 0;
}

int CRTSPSession::RTSPMediaDescribeVideoGet( char *buf, int len )
{
	int i    = 0;
	int k    = 0;
	
	/* get the RTP protocol and the RTP/RTCP ports if the RTP protocol is UDP
	 * over Multicast */
	i = RTSPMediaDescribeProtocolGet( buf, len, &m_RtspConf.RTPProt, &m_RtspConf.Info.Media[0]);
	if(i == 0) return 0;
	
	/* find the Multicast IP address if it is multicast RTP */
	if( m_RtspConf.RTPProt == RTSP_RTP_MULTICAST)
	{
		k = RTSPMediaDescribeMcastIPGet( &buf[i], (len-i), m_RtspConf.SrvMcastIP);
		if(k == 0) return 0;
		i += k;
	}
	
	/* find the video media type */
	k = RTSPMediaDescribeTypeGet( &buf[i], (len-i), &m_RtspConf.Info.Media[0].Type, &m_RtspConf.Info.Media[0].DataType);
	if(k == 0) return 0;
	i += k;
	
	/* find the config string */
	k = RTSPMediaDescribeConfigGet( &buf[i], (len-i), &m_RtspConf.Info.Media[0]);
//	if(k == 0)
//		return 0;
#ifdef _DEBUG
	{
		CSEQChecker tmp;
		BYTE seqhead[21];
		memset( seqhead, 0, sizeof( seqhead));
		tmp.GetSequenceHeader( m_RtspConf.Info.Media[0].Config, m_RtspConf.Info.Media[0].ConfigLen, seqhead);
		MP4_STREAM_ATTR *mp4_addr = tmp.GetSEQInfo( seqhead);
	}
#endif
	i += k;
	/* find the control path */
	k = RTSPMediaDescribeCtrlGet( &buf[i], (len-i), m_RtspConf.Info.Media[0].CtrlPath);
	if(k == 0) return 0;
	i += k;

	return i;	
}

int CRTSPSession::RTSPOptionReplyHandle( char *buf, int len, RTSP_HOST_INFO *Info)
{
	int  index = 0;
	int  i     = 0;
	int  Cseq  = 0;
	char cmd[32];

	Info->SrvRTSPCmdCap = 0;
	/* check if it is a valid RTSP packet (\r\n\r\n ending packet) */
	if( IsValidRTSPFrame( buf, len) == RTSP_OK)
	{
		/* received a valid RTSP packet, get the RTSP return code */
		if( RTSPRetCodeCseq( buf, len, &index, &Cseq) == RTSP_RET_OK)
		{
			if(Cseq != Info->CSeq)
			{
				;
				return RTSP_ERR;
			}
			/* get the OK reply from server, read the command capability now */
			if( RTSPFindString( &buf[index], (len-index), &i, "Public:"))
			{
				index += i;
				i = 0;
				do
				{
					cmd[i] = buf[index];
					if((cmd[i] == ',') || (cmd[i] == 0x0d))
					{
						cmd[i] = '\0';
						i = 0;
						if(strcmp(cmd, "OPTIONS") == 0)
						{
							Info->SrvRTSPCmdCap |= RTSP_CMD_CAP_OPTION; 
						}
						else if(strcmp(cmd, "DESCRIBE") == 0)
						{
							Info->SrvRTSPCmdCap |= RTSP_CMD_CAP_DESCRIBE;
						}
						else if(strcmp(cmd, "SETUP") == 0)
						{
							Info->SrvRTSPCmdCap |= RTSP_CMD_CAP_SETUP;
						}
						else if(strcmp(cmd, "SET_PARAMETER") == 0)
						{
							Info->SrvRTSPCmdCap |= RTSP_CMD_CAP_SETPARAM;
						}
						else if(strcmp(cmd, "TEARDOWN") == 0)
						{
							Info->SrvRTSPCmdCap |= RTSP_CMD_CAP_TEARDOWN;
						}
						else if(strcmp(cmd, "PLAY") == 0)
						{
							Info->SrvRTSPCmdCap |= RTSP_CMD_CAP_PLAY;
						}
						else if(strcmp(cmd, "PAUSE") == 0)
						{
							Info->SrvRTSPCmdCap |= RTSP_CMD_CAP_PAUSE;
						}
						else
						{
							/*return RTSP_ERR;*/
						}
					}
					else if(cmd[i] != ' ')
					{
						i ++;
					}
					if( (buf[index] == 0x0d) && (buf[index+1] == 0x0a) )
					{
						return RTSP_OK;
					}
					index ++;
				} while( index < len);
			}
		}
	}
	return RTSP_ERR;
}

int CRTSPSession::RTSPRetCodeCseq( char *buf, int len, int *index, int *Cseq)
{
	int i    = 0;
	int k    = 0;
	int find = 0;
	int ret  = 0;

	for(i = 0 ; i < (len-4) ; i=i+4)
	{
		if( ((buf[i]=='r') && (buf[i+1]=='t') && (buf[i+2]=='s') && (buf[i+3]=='p')) || 
			((buf[i]=='R') && (buf[i+1]=='T') && (buf[i+2]=='S') && (buf[i+3]=='P')))
		{
			/* find the RTSP string, find the first space */
			find = 1;
			while(buf[++i] != ' ');
			i ++;
			break;
		}
	}
	if(find)
	{
		/* find the return code */
		k = i;
		/* find the next space */
		while(buf[++k] != ' ');
		buf[k] = '\0';
		k ++;
		ret = atoi(&buf[i]);
	}
	else
	{
		return RTSP_ERR;
	}
	/* found the return code. try to get the CSeq now */
	if(RTSPFindString(&buf[k], (len-k), &i, "CSeq:"))
	{
		k += i;
		/* find the CSeq: string. skip all space characters */
		for(i = k ; i < len ; i ++)
		{
			if(buf[i] != ' ')
			{
				break;
			}
		}
		k = i;
		while((buf[i]!=0x0d)&&(buf[i+1]!=0x0a)) i++;
		buf[i]='\0';
		*Cseq = atoi(&buf[k]);
		*index = i + 2;
		return ret;
	}
	return RTSP_ERR;
}

int CRTSPSession::RTSPSessionIDGet(char *src, int len, int *index, char * dst )
{
	int i = 0;
	int k = 0;

	if( RTSPFindString( src, len, &i, "Session:") )
	{
		/* find the Session: Tag, skip the empty characters */
		for( k=i; k<len; k++)
		{
			if( src[k] != ' ')
			{
				/* find the 0x0d, 0x0a */
				i = k;
				while( (src[i]!=0x0d) && (src[i+1]!=0x0a) ) i++;
				src[i]='\0';
				*index = i + 2;
				sprintf( dst, "%s", &src[k]);
				//return atoi(&buf[k]);
				return (int)strlen( dst);
			}
		}
	}
	return -1;
}

int CRTSPSession::RTSPSetupReplyHandle(char *buf, int len, RTSP_MEDIA *Media)
{
	int  i      = 0;
	int  k      = 0;
	int  index  = 0;
	int  Cseq   = 0;

	/* check if it is a valid RTSP packet (\r\n\r\n ending packet) */
	if(IsValidRTSPFrame(buf, len) == RTSP_OK)
	{
		/* received a valid RTSP packet, get the RTSP return code */
		if(RTSPRetCodeCseq( buf, len, &index, &Cseq) == RTSP_RET_OK)
		{
			if(Cseq != m_RtspConf.Info.CSeq)
			{
				return RTSP_ERR;
			}
			/* success in the setup process !! record the server ports */
			if( m_RtspConf.RTPProt == RTSP_RTP_UDP)
			{
				if(RTSPFindString(&buf[index], len, &k, "server_port="))
				{
					index += k;
					/* find the server-port string. get the server ports */
					for(i = index ; i < (index+6) ; i ++)
					{
						if(buf[i] == '-')
						{
							buf[i] = '\0';
							Media->RTPSrvPort = atoi( &buf[index]);
							index = i+1;	
							break;
						}
					}
					
					if(i == (index+6))
					{
						return RTSP_ERR;
					}

					for(i = index ; i < (index+6) ; i ++)
					{
						if((buf[i] == 0x0d) && (buf[i+1] == 0x0a))
						{
							buf[i] = '\0';
							Media->RTCPSrvPort = atoi( &buf[index]);
							index = i+2;			
		
							break;
						}
					}
					if(i == (index+6)) return RTSP_ERR;
				}
				else
				{
					return RTSP_ERR;
				}
			}
			else
			{
				if(RTSPFindString(&buf[index], len, &k, "port="))
				{
					index += k;
					for(i = index ; i < (index+6) ; i ++)
					{
						if((buf[i]==';')||((buf[i]==0x0d)&&(buf[i+1]==0x0a)))
						{
							buf[i] = '\0';
							Media->RTPSrvPort  = atoi(&buf[index]);
							Media->RTCPSrvPort = Media->RTPSrvPort+1;
							index = i+1;
							break;
						}
					}
					if(i == (index+6))
					{
						return RTSP_ERR;
					}
				}
				else
				{
					return RTSP_ERR;
				}
			}
			/* get the Session number */
			if( RTSPFindString( &buf[index], len, &k, "Session:") )
			{
				index += k;
				for(i = index ; i < len ; i ++)
				{
					if(buf[i] != ' ') break;
				}
				if(i == len) return RTSP_ERR;
				index = i;
				for(i = index ; i < len ; i ++)
				{
					if((buf[i] == 0x0d) && (buf[i+1] == 0x0a))
					{
						buf[i] = '\0';
						//m_RtspConf.Info.Session = atoi( &buf[index]);
						sprintf( m_RtspConf.Info.Session, "%s", &buf[index]);
						return RTSP_OK;
					}
				}
			}
		}
	}

    return RTSP_ERR;
}

void CRTSPSession::RTSPSetupReqBuildBase64(char *buf, int *len, RTSP_MEDIA * Media)
{
	/* TODO!! not implemented */
	return;
}

void CRTSPSession::RTSPTeardownReqBuildBase64(char *buf, int *len)
{
	;
	return;
}

int CRTSPSession::GetRTSPDigestStrings(char *buf, int len, RTSP_DIGEST *Digest)
{
	int i   = 0;
	int k   = 0;
	int ret = 0;

	/* try to get the realm */
	for(i = 0 ; i < (len-5) ; i ++)
	{
		if( (buf[i]=='r') && (buf[i+1]=='e') && (buf[i+2]=='a') && 
			(buf[i+3]=='l') && (buf[i+4]=='m') && (buf[i+5]=='='))
		{
			/* get the realm head */
			k = i+5;
			if(RTSPDigestParser(&buf[k], (len-k), Digest->Realm) == RTSP_OK)
			{
				ret |= GET_DIGETS_REALM;
			}
			else
			{
				return 0;
			}
		}
		if( (buf[i]=='n')&&(buf[i+1]=='o')&&(buf[i+2]=='n')&&\
			(buf[i+3]=='c')&&(buf[i+4]=='e')&&(buf[i+5]=='='))
		{
			/* get the nonce head */
			k = i+5;
			if(RTSPDigestParser(&buf[k], (len-k), Digest->Nonce) == RTSP_OK)
			{
				ret |= GET_DIGETS_NONCE;
			}
			else
			{
				return 0;
			}
		}
		if((buf[i]=='u')&&(buf[i+1]=='r')&&(buf[i+2]=='i')&&(buf[i+3]=='='))
		{
			/* get the nonce head */
			k = i+3;
			if(RTSPDigestParser(&buf[k], (len-k), Digest->Uri) == RTSP_OK)
			{
				ret |= GET_DIGETS_URI;
			}
			else
			{
				return 0;
			}
		}
		if(	(buf[i]=='r') && (buf[i+1]=='e') && (buf[i+2]=='s') &&
			(buf[i+3]=='p') && (buf[i+4]=='o') && (buf[i+5]=='n') &&
			(buf[i+6]=='s') && (buf[i+7]=='e') && (buf[i+8]=='=') )
		{
			/* get the nonce head */
			k = i+8;
			if(RTSPDigestParser(&buf[k], (len-k), Digest->Resp) == RTSP_OK)
			{
				ret |= GET_DIGETS_RESP;
			}
			else
			{
				return 0;
			}
		}
	}
	return ret;
}

int CRTSPSession::IsValidRTSPFrame(char *buf, int len)
{
	int i = 0;

	for(i = 0 ; i < len-2 ; i=i+2)
	{
		if((buf[i] == 0xd) && (buf[i+1] == 0xa))
		{
			if(buf[i+2] == 0xd)
			{
				/* I got a complete RTSP message */
				return RTSP_OK;
			}
		}
		else if((buf[i] == 0xa) && (buf[i+1] == 0xd))
		{
			if(buf[i+2] == 0xa)
			{
				/* I got a complete RTSP message */
				return RTSP_OK;
			}
		}
	}
	return RTSP_ERR;
}

int CRTSPSession::RTSPDigestParser(char *buf, int len, char *RetStr)
{
	int i = 0;
	int k = 0;
		
	/* find the first " */
	for(i = 0;  i < len ; i ++)
	{
		if(buf[i] == '\"')
			break;
	}
	if(i == len)
		return RTSP_ERR;
	/* find the next " */
	i ++;
	for(k = i ; k < RTSP_DIGEST_LEN ; k ++)
	{
		if(buf[k] == '\"') break;
	}
	if(k == RTSP_DIGEST_LEN) return RTSP_ERR;
	memcpy(RetStr, &buf[i], (k-i));
	return RTSP_OK;		
}

int CRTSPSession::RTSPFindString(char *buf, int len, int *index, char *Str)
{
	int i = 0;
	int k = 0;
	int slen = (int)strlen(Str);

	for(i = 0 ; i < len ; i ++)
	{
		if(buf[i] != Str[k]) k = 0;
		else k++;
		if(k == slen)
		{
			*index = i+1;
			return 1;
		}
	}
	return 0;
}

int CRTSPSession::RTSPHostDescribeHandle()
{
	char pkt[1024];
	int  len = 0;
	int  ret = RTSP_ERR;
	int  WaitTime = m_RtspConf.RespTime;
		
	m_RtspConf.Info.CSeq += 1;
	/* build the 1st Describe request packet to send to RTSP server */
	if( m_RtspConf.Encoder == RTSP_ENCODER_BASE64)
	{
		RTSPDescribeReqBuildBase64( pkt, &len);
	}
	else
	{
		RTSPDescribeReqBuildDigest( pkt, &len);
	}
	/* Send this RTSP Option Req packet to Server now */
	if(WriteSockTCP( m_RtspConf.Sock, pkt, len, RTSP_TIMER_TX) == len)
	{
		/* success in sending the RTSP Describe REQ packet, wait for server's
		* reply */
		len = sizeof(pkt);
		if( ReadSockTCP( m_RtspConf.Sock, pkt, &len, WaitTime) == NET_SUCCESS)
		{
			/* reviced server's reply, get the server's command capabilities */
			ret = RTSPDescribeReplyHandle( pkt, len);
			if(ret == RTSP_RET_OK)
				return RTSP_OK;
			else if(ret == RTSP_OK)
			{
				/* this is the digest authentication process!! go for the second
				* round. the nonce and realm were received from server in the 1st
				* round */
				/* The Response string is 
				*    SET_1 = MD5(UserName:Realm:PWD)
				*    SET_2 = MD5(RTSP_CMD:URI)
				*    Challenge String = MD5(SET_1:Nonce:SET_2) */
				RTSPDigestRespStrBuild( "DESCRIBE", &m_RtspConf.Info.Digest);
				m_RtspConf.Info.CSeq += 1;
				RTSPDescribeReqBuildDigest(pkt, &len);
				if( WriteSockTCP( m_RtspConf.Sock, pkt, len, RTSP_TIMER_TX) == len )
				{
					len = sizeof(pkt);
					if( WaitTime < 1000) WaitTime = 1000;
					if( ReadSockTCP( m_RtspConf.Sock, pkt, &len, WaitTime) == NET_SUCCESS )
					{
						if( RTSPDescribeReplyHandle( pkt, len ) == RTSP_RET_OK )
						{
							return RTSPMediaDescribeGet( pkt, len );
						}

					}
				}
			}
		}
	}
	return RTSP_ERR;
}

int CRTSPSession::RTSPHostSetupHandle()
{
	char pkt[1024];
	int  len  = 0;
	int  port = 0;

	m_RtspConf.Info.CSeq += 1;
	/* generate the host RTP/RTCP ports if it is RTP over UDP */
	if( m_RtspConf.RTPProt == RTSP_RTP_UDP)
	{
		port = RTSPGetRandomNum()&0xfffe; /* even port */
		if(port <= 1024) port += 1024;
		m_RtspConf.Info.Media[0].RTPHostPort  = port;
		m_RtspConf.Info.Media[0].RTCPHostPort = port+1;
		m_RtspConf.Info.Media[1].RTPHostPort  = port+2;
		m_RtspConf.Info.Media[1].RTCPHostPort = port+3;
	}
	/* initialize the session ID */
	//m_RtspConf.Info.Session = 0;
	sprintf( m_RtspConf.Info.Session, "");

	/* build the 1st Describe request packet to send to RTSP server */
	if( m_RtspConf.Encoder == RTSP_ENCODER_BASE64)
	{
		/* send the video setup command first */
		RTSPSetupReqBuildBase64( pkt, &len, &m_RtspConf.Info.Media[0]);
	}
	else
	{
		/* send the video setup command first */
		RTSPSetupReqBuildDigest( pkt, &len, &m_RtspConf.Info.Media[0]);
	}
	/* send the video setup cmd */
	/* Send this RTSP Option Req packet to Server now */
	if(WriteSockTCP( m_RtspConf.Sock, pkt, len, RTSP_TIMER_TX) == len)
	{
		/* success in sending the RTSP SETUP Video REQ packet, 
		* wait for server's reply */
		len = sizeof(pkt);
		if(ReadSockTCP( m_RtspConf.Sock, pkt, &len, m_RtspConf.RespTime) == NET_SUCCESS)
		{
			/* reviced server's reply, get the server's command capabilities */
			if(RTSPSetupReplyHandle(pkt, len, &m_RtspConf.Info.Media[0]) == RTSP_ERR) 
				return RTSP_ERR;
		}
		else
		{
			return RTSP_ERR;
		}
	}
	else
	{
		return RTSP_ERR;
	}

	/* check if there is audio media */
	if( m_RtspConf.Info.Media[1].Type )
	{
		/* Setup the Audio Media */
		m_RtspConf.Info.CSeq += 1;
		/* build the 1st Describe request packet to send to RTSP server */
		if( m_RtspConf.Encoder == RTSP_ENCODER_BASE64)
		{
			/* send the video setup command first */
			RTSPSetupReqBuildBase64( pkt, &len, &m_RtspConf.Info.Media[1]);
		}
		else
		{
			/* send the video setup command first */
			RTSPSetupReqBuildDigest( pkt, &len, &m_RtspConf.Info.Media[1]);
		}
		if( WriteSockTCP( m_RtspConf.Sock, pkt, len, RTSP_TIMER_TX) == len )
		{
			/* success in sending the RTSP SETUP Audio REQ packet,
			* wait for server's reply */
			len = sizeof(pkt);
			if(ReadSockTCP( m_RtspConf.Sock, pkt, &len, m_RtspConf.RespTime) == NET_SUCCESS)
			{
				/* reviced server's reply, get the server's command capabilities */
				if( RTSPSetupReplyHandle( pkt, len, &m_RtspConf.Info.Media[1]) == RTSP_ERR)
					return RTSP_ERR;
			}
			else
			{
				return RTSP_ERR;
			}
			//return RTSP_OK;
		}
	}
	else
	{
		;//return RTSP_ERR;
	}
	return RTSP_OK;
}

int CRTSPSession::RTSPHostPlayHandle()
{
	/* send the RTSP PLAY Command for Track 1 (Video) first */
	if( RTSPPlayCmdSend() == RTSP_OK )
	{
		/* wait for server's reply */
		if( RTSPPlayReplyHandle() == RTSP_OK )
		{
			/* update the server's Sequence Numbers configurations */
			return RTSP_OK;
		}
	}
	else
	{
		;
	}
	m_RtspConf.ConnState = RTSP_CONN_ERR_SIGNAL;
	return RTSP_ERR;
}

int CRTSPSession::RTSPPlayCmdSend()
{
	char pkt[1024];
	int  len  = 0;

	/* build the RTSP PLAY Cmd packet */
	m_RtspConf.Info.CSeq += 1;
	if( m_RtspConf.Encoder == RTSP_ENCODER_BASE64)
	{
		RTSPPlayReqBuildBase64( pkt, &len);
	}
	else
	{
		RTSPPlayReqBuildDigest( pkt, &len);
	}
	/* send this PLAY Req packet to the server now */
	if(WriteSockTCP( m_RtspConf.Sock, pkt, len, RTSP_TIMER_TX) == len)
		return RTSP_OK;
	return RTSP_ERR;
}

int CRTSPSession::RTSPPlayReplyHandle()
{
	char buf[1024];
	int  len   = sizeof(buf);
	int  Cseq  = 0;
	int  index = 0;
	int  ret   = 0;
	int  i     = 0;

	if( ReadSockTCP( m_RtspConf.Sock, buf, &len, m_RtspConf.RespTime) == NET_SUCCESS)
	{
		/* check if it is a valid RTSP packet (\r\n\r\n ending packet) */
		if(IsValidRTSPFrame( buf, len) == RTSP_OK)
		{
			/* received a valid RTSP packet, get the RTSP return code */
			ret = RTSPRetCodeCseq( buf, len, &index, &Cseq);
			if(Cseq != m_RtspConf.Info.CSeq)
			{
				return RTSP_ERR;
			}
			if(ret != RTSP_RET_OK)
			{
				return RTSP_ERR;
			}
			/* Check the Session */
			char szSession[256];

			ret = RTSPSessionIDGet( &buf[index], (len-index), &i, szSession);
			if( ret && !strcmp( szSession, m_RtspConf.Info.Session) )
			{
				/* the session ID is correct, get the sequence number per media track */
				index += i;
				if( RTSPFindString(&buf[index], (len-index), &i, "track1;seq="))
				{
					index += i;
					for( i = index ; i < len ; i ++)
					{
						if(buf[i] == ',')
						{
							buf[i] = '\0';
							m_RtspConf.Info.Media[0].Seq = atoi(&buf[index]);
							index = i+1;

							break;
						}
						else if((buf[i]==0x0d)&&(buf[i+1]==0x0a))
						{
							buf[i] = '\0';
							m_RtspConf.Info.Media[0].Seq = atoi( &buf[index]);
							index = i+2;
							//return RTSP_OK;
							break;
						}
					}
					if(i == len)
					{
						return RTSP_ERR;
					}
				}
				else
				{
					//return RTSP_ERR;
				}
				/* Success in getting the sequence number in track 1 and there is track2
				 * appended with this reply commmand, read the sequence number of track2 */
				if( RTSPFindString(&buf[index], (len-index), &i, "track2;seq=") )
				{
					index += i;
					for(i = index ; i < len ; i ++)
					{
						if((buf[i] == ',') || ((buf[i]==0x0d)&&(buf[i+1]==0x0a)))
						{
							buf[i] = '\0';
							m_RtspConf.Info.Media[1].Seq = atoi(&buf[index]);
							index += (i+1);
							return RTSP_OK;
						}
					}
					if( i==len )
					{
						return RTSP_ERR;
					}
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
		}
		else
		{
			;
		}
	}
	//return RTSP_ERR;
	return RTSP_OK;
}

int CRTSPSession::RTSPTeardownCmdSend()
{
	char pkt[1024];
	int  len  = 0;

	if( m_RtspConf.Sock == INVALID_SOCKET )
	{
		return RTSP_ERR;
	}
	/* build the RTSP Teardown Cmd packet */
	m_RtspConf.Info.CSeq += 1;
	if( m_RtspConf.Encoder == RTSP_ENCODER_BASE64 )
	{
		RTSPTeardownReqBuildBase64( pkt, &len);
	}
	else
	{
		RTSPTeardownReqBuildDigest( pkt, &len);
	}

	/* send this PLAY Req packet to the server now */
	if( WriteSockTCP( m_RtspConf.Sock, pkt, len, RTSP_TIMER_TX ) == len)
		return RTSP_OK;
	return RTSP_ERR;
}

