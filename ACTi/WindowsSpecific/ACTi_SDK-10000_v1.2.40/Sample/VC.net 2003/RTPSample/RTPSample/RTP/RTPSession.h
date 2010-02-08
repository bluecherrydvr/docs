#pragma once


#include <iostream>
#include <iterator>
#include <vector>

#include "version.h"
#include "head.h"
#include "NetRTSP.h"
#include "NetRTP.h"
#include "NetRTCP.h"

#include "FrameBin.h"
#include "SimpleUDPSession.h"

using namespace std;

class CRTPSession : public CSimpleUDPSession
{
protected:
	RTP_HOST_CONF  m_RtpConf;
	unsigned char m_FrameResolution;

	BYTE m_RecvBuffer[256*1024];
	DWORD m_RecvBufferLen;

	unsigned char *sequence_header;
	int sequence_header_size;
	int sequence_header_len;

	vector< CFrameBin * > m_FrameBin;
	CCriticalSection m_cCS;
public:
	const unsigned int FRAME_MAXSIZE;
	const int MAX_USED_MEDIA_TYPE;

	CRTPSession();
	virtual ~CRTPSession();

	int GetFrameSize( const int MediaType )
	{
		if( MediaType < 0 || MediaType >= MAX_USED_MEDIA_TYPE ) return 0;
		return (int)m_FrameBin[MediaType]->size();
	}

	RTP_FRAME * NewFrames( const int MediaType, const DWORD FrameSize )
	{
		if( MediaType < 0 || MediaType >= MAX_USED_MEDIA_TYPE )
			return NULL;
		return m_FrameBin[MediaType]->NewFrames( FrameSize);
	}

	BOOL PushNewFrames( const int MediaType, RTP_FRAME * Frame)
	{
		if( Frame == NULL ) return FALSE;
		if( MediaType < 0 || MediaType >= MAX_USED_MEDIA_TYPE ) return FALSE;
		m_FrameBin[MediaType]->PushNewFrames( Frame);
		return TRUE;
	}

	RTP_FRAME * PopFrames( const int MediaType )
	{
		if( MediaType < 0 || MediaType >= MAX_USED_MEDIA_TYPE ) return NULL;
		return m_FrameBin[MediaType]->PopFrames();
	}

	BOOL FreeFrames( const int MediaType, RTP_FRAME * Frame )
	{
		if( MediaType < 0 || MediaType >= MAX_USED_MEDIA_TYPE )
			return FALSE;
		return m_FrameBin[MediaType]->FreeFrames( Frame);
	}

	void ReleaseAllFrames(void)
	{
		vector< CFrameBin * >::iterator rip;
		CFrameBin * tmp;
		while( m_FrameBin.size() > 0 )
		{
			rip = m_FrameBin.begin();
			if( rip == m_FrameBin.end() ) break;
			tmp = (*rip);
			if( tmp != NULL )
			{
				tmp->ReleaseAllFrames();
				delete tmp;
			}
            m_FrameBin.erase( rip);
		}
		m_FrameBin.clear();
	}

	void SetFrameResolution( unsigned char resolution )
	{
		m_FrameResolution = resolution;
	}
	unsigned char GetFrameResolution( void ) const
	{
		return m_FrameResolution;
	}


	RTP_HOST_CONF & GetRtpConf( void)
	{
		return m_RtpConf;
	}

	void InitRTPHostConf( RTSP_HOST_CONF *Rtsp );
	int CreateRTPHostSockets();
	void RTPSeqNumUpdate(int Seq1, int Seq2 );

	int RTPVideoFrameHunting( void); // for test
	int RTPVideoIFrameHunting(RTP_FRAME *Frame, RTCP_HOST_CONF *Rtcp);
	int RTPFrameVideoGet( RTP_FRAME *Frame, RTCP_HOST_CONF *Rtcp);
	static int GetRTPVideoFrameType( const unsigned char *buf, int *Type = NULL );

	int RTPAudio1stFrameHunting(RTP_FRAME *Frame, RTCP_HOST_CONF *Rtcp);
	int RTPFrameAudioGet(RTP_FRAME *Frame, RTCP_HOST_CONF *Rtcp);

	void CloseMedia( void)
	{
		for( int i=0; i<MAX_USED_MEDIA_TYPE; i++)
		{
			m_RtpConf.Media[i].State = RTP_HOST_STATE_INIT;
		}
	}

	void CloseSession( void)
	{
		CloseSession( &m_RtpConf);
	}

	void CloseSession( RTP_HOST_CONF *Conf )
	{
		int i = 0;

		for(i = 0 ; i < 2 ; i ++)
		{
			if(Conf->Media[i].Sock)
			{
				closesocket( Conf->Media[i].Sock);
				Conf->Media[i].Sock = INVALID_SOCKET;
			}
		}
		return;
	}

};



