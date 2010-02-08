/*********************************************************************

File:         XADP.cpp

Description:  SDK 10000 - ATCP20.dll main code

Modified:     Ken Chan

Author:       Ken Chan

Prototyping:

Created:      [Ken Chan]

Modified:

Email:		  Ken.Chan@ACTi.com; jjmin32@yahoo.com.tw
MSN:          kennet2@giga.net.tw

*********************************************************************/
#include "StdAfx.h"
#include ".\XADP.h"

XADP::XADP(void)
{
	m_SessionID = 0;
	m_fnControlCB = NULL;
	m_ParamControlCB = 0;

	m_nDebugLevel = 0;
	m_MediaConfig.ChannelNumber = 0;
	m_MediaConfig.HTTPPort = 80;
	m_MediaConfig.ControlPort = 6001;
	m_MediaConfig.StreamingPort = 6002;

	m_TempBuf		= new char[256000];
	m_ControlPort	= new CVideoControl_2_0();
	m_StreamingPort = new CVideoStreaming_2_0();
	m_HTTP			= new CHTTPConnection();
}

XADP::~XADP(void)
{
	m_fnControlCB = NULL;
	m_ParamControlCB = 0;
	
	if( m_HTTP )
		delete m_HTTP;
	m_HTTP = NULL;
	if( m_ControlPort )
		delete m_ControlPort;
	m_ControlPort = NULL;
	if( m_StreamingPort )
		delete m_StreamingPort;
	m_StreamingPort = NULL;
	
	if( m_TempBuf )
		delete [] m_TempBuf;
	m_TempBuf = NULL;
}

void XADP::SetMediaConfig( MEDIA_CONNECTION_CONFIG* pMediaConfig )
{
	ShowDebugMessage( 1, "[ATCP20][SetMediaConfig][ENTER]\n\0" );
	memcpy( &m_MediaConfig, pMediaConfig, sizeof(MEDIA_CONNECTION_CONFIG) );

	// VIDEO_2_0_VIDEO_STREAMING_CONNECTION_CONFIG
	memcpy( m_VSConfig.IP, m_MediaConfig.UniCastIP, strlen(m_MediaConfig.UniCastIP) );
	m_VSConfig.IP[strlen(m_MediaConfig.UniCastIP)] = 0;
	memcpy( m_VSConfig.UserID, m_MediaConfig.UserID, strlen(m_MediaConfig.UserID) );
	m_VSConfig.UserID[strlen(m_MediaConfig.UserID)] = 0;
	memcpy( m_VSConfig.Password, m_MediaConfig.Password, strlen(m_MediaConfig.Password) );
	m_VSConfig.Password[strlen(m_MediaConfig.Password)] = 0;
	m_VSConfig.VideoStreamingPort = m_MediaConfig.StreamingPort;
	m_VSConfig.ConnectTimeOut = m_MediaConfig.ConnectTimeOut;
	m_VSConfig.AliveTimeout = 15000;

	// VIDEO_2_0_VIDEO_CONTROL_CONNECTION_CONFIG
	memcpy( m_VCConfig.IP, m_MediaConfig.UniCastIP, strlen(m_MediaConfig.UniCastIP) );
	m_VCConfig.IP[strlen(m_MediaConfig.UniCastIP)] = 0;
	memcpy( m_VCConfig.UserID, m_MediaConfig.UserID, strlen(m_MediaConfig.UserID) );
	m_VCConfig.UserID[strlen(m_MediaConfig.UserID)] = 0;
	memcpy( m_VCConfig.Password, m_MediaConfig.Password, strlen(m_MediaConfig.Password) );
	m_VCConfig.Password[strlen(m_MediaConfig.Password)] = 0;
	m_VCConfig.VideoControlPort = m_MediaConfig.ControlPort;
	m_VCConfig.ConnectTimeOut = m_MediaConfig.ConnectTimeOut;
	//m_VCConfig.AliveTimeout = 1000;
	sprintf( m_UrlTarget, "http://%s:%d/cgi-bin/\0", m_MediaConfig.UniCastIP, m_MediaConfig.HTTPPort );
	ShowDebugMessage( 1, "[ATCP20][SetMediaConfig][LEAVE]\n\0" );
}

void XADP::GetVideoConfig( MEDIA_VIDEO_CONFIG* pVideoConfig )
{
	ShowDebugMessage( 1, "[ATCP20][GetVideoConfig][ENTER]\n\0" );
	char cRequestConfig[512];
	char cResult[512];

	if( m_HTTP )//m_StreamingPort->GetConnectionStatus() > 0 )
	{
		memset( cResult, 0, 512 );
		if( m_MediaConfig.ChannelNumber == 0 )
			sprintf( cRequestConfig, "%smpeg4?USER=%s&PWD=%s&VIDEO_BITRATE&VIDEO_RESOLUTION&VIDEO_FPS_NUM&VIDEO_BRIGHTNESS&VIDEO_CONTRAST&VIDEO_HUE&VIDEO_SATURATION\0", m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password );
		else
			sprintf( cRequestConfig, "%smpeg4?USER=%s&PWD=%s&CHANNEL=%d&VIDEO_BITRATE&VIDEO_RESOLUTION&VIDEO_FPS_NUM&VIDEO_BRIGHTNESS&VIDEO_CONTRAST&VIDEO_HUE&VIDEO_SATURATION\0", m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, m_MediaConfig.ChannelNumber );

		if( m_HTTP->ExecuteURLCommand( cRequestConfig, cResult, 512 ) )
		{
			ExchangHTTPResultToVideoConfig( cResult );
			if( pVideoConfig )
				memcpy( pVideoConfig, &m_VideoConfig, sizeof(MEDIA_VIDEO_CONFIG) );	
		}
		if( pVideoConfig )
			memcpy( pVideoConfig, &m_VideoConfig, sizeof(MEDIA_VIDEO_CONFIG) );	
	}

	else
	{
		if( pVideoConfig )
			memset( pVideoConfig, 0, sizeof(MEDIA_VIDEO_CONFIG) );
	}
	ShowDebugMessage( 1, "[ATCP20][GetVideoConfig][LEAVE]\n\0" );
}

void XADP::SetVideoConfig( MEDIA_VIDEO_CONFIG* pVideoConfig )
{
	ShowDebugMessage( 1, "[ATCP20][SetVideoConfig][ENTER]\n\0" );
	char cRequestConfig[1024];
	char cResult[1024];
	char resolut[16];
	//char TVStd[8];

	switch( pVideoConfig->dwVideoResolution )
	{
	case 0:
		sprintf( resolut, "N720x480\0" );
		break;
	case 1:
		sprintf( resolut, "N352x240\0" );
		break;
	case 2:
		sprintf( resolut, "N160x112\0" );
		break;
	case 3:
		sprintf( resolut, "P720x576\0" );
		break;
	case 4:
		sprintf( resolut, "P352x288\0" );
		break;
	case 5:
		sprintf( resolut, "P176x144\0" );
		break;
	case 6:
		sprintf( resolut, "N176x120\0" );
		break;
	case 64:
		sprintf( resolut, "N640x480\0" );
		break;
	case 192:
		sprintf( resolut, "P640x480\0" );
		break;
	default:
		sprintf( resolut, "N720x480\0" );
		break;
	}

	if( m_HTTP )
	{
		if( m_MediaConfig.ChannelNumber == 0 ) 
			sprintf( cRequestConfig, "%smpeg4?USER=%s&PWD=%s&VIDEO_BITRATE=%d&VIDEO_RESOLUTION=%s&VIDEO_FPS_NUM=%d&VIDEO_BRIGHTNESS=%d&VIDEO_CONTRAST=%d&VIDEO_HUE=%d&VIDEO_SATURATION=%d\0", 
			m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, pVideoConfig->dwBitsRate, resolut, pVideoConfig->dwFps, pVideoConfig->dwVideoBrightness, pVideoConfig->dwVideoContrast, pVideoConfig->dwVideoHue, pVideoConfig->dwVideoSaturation );
		else
			sprintf( cRequestConfig, "%smpeg4?USER=%s&PWD=%s&CHANNEL=%d&VIDEO_BITRATE=%d&VIDEO_RESOLUTION=%s&VIDEO_FPS_NUM=%d&VIDEO_BRIGHTNESS=%d&VIDEO_CONTRAST=%d&VIDEO_HUE=%d&VIDEO_SATURATION=%d\0", 
			m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, m_MediaConfig.ChannelNumber, pVideoConfig->dwBitsRate, resolut, pVideoConfig->dwFps, pVideoConfig->dwVideoBrightness, pVideoConfig->dwVideoContrast, pVideoConfig->dwVideoHue, pVideoConfig->dwVideoSaturation );

		m_HTTP->ExecuteURLCommand( cRequestConfig, cResult, 1024 );
	}

	ShowDebugMessage( 1, "[ATCP20][SetVideoConfig][LEAVE]\n\0" );
}

bool XADP::Connect()
{
	ShowDebugMessage( 1, "[ATCP20][Connect][ENTER]\n\0" );
	bool bret = false;

	switch( m_MediaConfig.ContactType )
	{
	case CONTACT_TYPE_UNICAST_WOC_PREVIEW:
		m_StreamingPort->SetConfig(&m_VSConfig);

		if( m_StreamingPort->Connect() )
		{
			//OutputDebugString( "Connect OK\n\0" );
			GetVideoConfig( NULL );
			bret = true;
		}
		break;
	case CONTACT_TYPE_CONTROL_ONLY:
		m_ControlPort->SetConfig(&m_VCConfig);
		bret = m_ControlPort->Connect();
		m_ControlPort->StartVideoControl();
		break;
	case CONTACT_TYPE_UNICAST_PREVIEW:
		m_StreamingPort->SetConfig(&m_VSConfig);
		m_ControlPort->SetConfig(&m_VCConfig);
		if( m_ControlPort->Connect() )
		{
			m_ControlPort->StartVideoControl();
			if( m_StreamingPort->Connect() )
			{
				GetVideoConfig( NULL );
				bret = true;
			}
		}
		break;

	default:
		break;
	}

	ShowDebugMessage( 1, "[ATCP20][Connect][LEAVE]\n\0" );
	return bret ;
}

bool XADP::StartStreaming()
{
	ShowDebugMessage( 1, "[ATCP20][StartStreaming][ENTER]\n\0" );
	bool bret = false;

	if( m_MediaConfig.ContactType == CONTACT_TYPE_CONTROL_ONLY )
	{
		bret = true;
		//OutputDebugString( "StartStreaming  CONTACT_TYPE_CONTROL_ONLY\n\0" );
	}

	else
	{
		bret = m_StreamingPort->StartVideoStreaming( m_SessionID );
	}
	ShowDebugMessage( 1, "[ATCP20][StartStreaming][LEAVE]\n\0" );
	return bret;
}

void XADP::StopStreaming()
{
	ShowDebugMessage( 1, "[ATCP20][StopStreaming][ENTER]\n\0" );
	
	if( m_MediaConfig.ContactType == CONTACT_TYPE_CONTROL_ONLY )
	{
		if( m_ControlPort )
		{
			m_ControlPort->StopVideoControl();
			m_ControlPort->WaitForStoping(0);
		}
	}

	else
	{
		if( m_ControlPort )
		{
			m_ControlPort->StopVideoControl();
			m_ControlPort->WaitForStoping(0);
		}
		if( m_StreamingPort )
			m_StreamingPort->StopVideoStreaming();
	}
	ShowDebugMessage( 1, "[ATCP20][StopStreaming][LEAVE]\n\0" );
}

void XADP::Disconnect()
{
	ShowDebugMessage( 1, "[ATCP20][Disconnect][ENTER]\n\0" );
	if( m_ControlPort )
		m_ControlPort->Disconnect();
	if( m_StreamingPort )
		m_StreamingPort->Disconnect();
	ShowDebugMessage( 1, "[ATCP20][Disconnect][LEAVE]\n\0" );
}

bool XADP::SendCommand( MEDIA_COMMAND* pMediaCommand )
{
	ShowDebugMessage( 1, "[ATCP20][SendCommand][ENTER]\n\0" );
	int nget = 0;
	int nrecv = 0;
	char httpcmd[1024];

	BYTE bstatus;
	char s1[32];
	DWORD i2 = 0;
	bool bret = false;

	if( pMediaCommand )
	{
		switch( pMediaCommand->dwCommandType ) 
		{
		case NET_REQ_URL_COMMAND:
			if( m_HTTP )
			{
				if( pMediaCommand->nResultLength > 0 && pMediaCommand->pResult )
				{
					bret = m_HTTP->ExecuteURLCommand( pMediaCommand->pCommand, pMediaCommand->pResult, pMediaCommand->nResultLength );
					if( bret )
						pMediaCommand->nResultLength = strlen( pMediaCommand->pResult );
				}

				else
					bret = m_HTTP->ExecuteURLCommand( pMediaCommand->pCommand, NULL, 0 );
			}
			break;
		case S_ENGINE_REQ_PLAY:
			if( m_StreamingPort )
				bret = m_StreamingPort->SendCommamd( S_ENGINE_REQ_PLAY, pMediaCommand->pCommand, pMediaCommand->nCommandLength );
			break;
		case NET_REQ_DO:

			if( m_ControlPort )
			{
				if( m_ControlPort->ISConnected() )
				{
					bret = m_ControlPort->SendCommamd( NET_REQ_CONTROL_DO, pMediaCommand->pCommand, pMediaCommand->nCommandLength );
					break;
				}
			}

			bstatus = pMediaCommand->pCommand[0];
			sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&DIO_OUTPUT=0x%.2x\0",
				m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, bstatus >> 4 );

			if( m_HTTP )
			{
				if( pMediaCommand->nResultLength > 0 && pMediaCommand->pResult )
					bret = m_HTTP->ExecuteURLCommand( httpcmd, pMediaCommand->pResult, pMediaCommand->nResultLength );

				else
					bret = m_HTTP->ExecuteURLCommand( httpcmd, NULL, 0 );
			}
			break;
		case NET_REQ_RS232:
			if( m_ControlPort )
			{
				if( m_ControlPort->ISConnected() )
					bret = m_ControlPort->SendCommamd( NET_REQ_CONTROL_SERIAL, pMediaCommand->pCommand, pMediaCommand->nCommandLength );
			}
			break;
		case NET_REQ_RS232_SET:

			if( pMediaCommand->pCommand[0] == NET_N81 ) sprintf( s1, "8N1\0" );
			else if( pMediaCommand->pCommand[0] == NET_O81 ) sprintf( s1, "8O1\0" );
			else if( pMediaCommand->pCommand[0] == NET_E81 ) sprintf( s1, "8E1\0" );

			if( pMediaCommand->pCommand[1] == NET_1200BPS ) i2 = 1200;
			else if( pMediaCommand->pCommand[1] == NET_2400BPS ) i2 = 2400;
			else if( pMediaCommand->pCommand[1] == NET_4800BPS ) i2 = 4800;
			else if( pMediaCommand->pCommand[1] == NET_9600BPS ) i2 = 9600;
			else if( pMediaCommand->pCommand[1] == NET_19200BPS ) i2 = 19200;
			else if( pMediaCommand->pCommand[1] == NET_38400BPS ) i2 = 38400;
			else if( pMediaCommand->pCommand[1] == NET_57600BPS ) i2 = 57600;
			else if( pMediaCommand->pCommand[1] == NET_115200BPS ) i2 = 115200;
			else if( pMediaCommand->pCommand[1] == NET_230400BPS ) i2 = 230400;

			sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&SERIAL_SETTING=%s,%d\0",
				m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, s1, i2 );

			if( m_HTTP )
			{
				if( pMediaCommand->nResultLength > 0 && pMediaCommand->pResult )
					bret = m_HTTP->ExecuteURLCommand( httpcmd, pMediaCommand->pResult, pMediaCommand->nResultLength );

				else
					bret = m_HTTP->ExecuteURLCommand( httpcmd, NULL, 0 );
			}
			break;

		case NET_REQ_VARIABLE_FPS:

			memcpy( &i2, pMediaCommand->pCommand, 4 );
			if( m_MediaConfig.ChannelNumber == 0 )
			{
				sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&VIDEO_VARIABLE_FPS=%d,%d\0",
					m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, m_SessionID, i2 );
			}

			else
			{
				sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&CHANNEL=%d&VIDEO_VARIABLE_FPS=%d,%d\0",
					m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, m_MediaConfig.ChannelNumber, m_SessionID, i2 );
			}

			if( m_HTTP )
			{
				if( pMediaCommand->nResultLength > 0 && pMediaCommand->pResult )
					bret = m_HTTP->ExecuteURLCommand( httpcmd, pMediaCommand->pResult, pMediaCommand->nResultLength );

				else
					bret = m_HTTP->ExecuteURLCommand( httpcmd, NULL, 0 );
			}
			break;
		case NET_REQ_BRIGHTNESS:
			memcpy( &i2, pMediaCommand->pCommand, 4 );

			if( m_MediaConfig.ChannelNumber == 0 )
			{
				sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&VIDEO_BRIGHTNESS=%d\0",
					m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, i2 );
			}

			else
			{
				sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&CHANNEL=%d&VIDEO_BRIGHTNESS=%d\0",
					m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, m_MediaConfig.ChannelNumber, i2 );
			}

			if( m_HTTP )
			{
				if( pMediaCommand->nResultLength > 0 && pMediaCommand->pResult )
					bret = m_HTTP->ExecuteURLCommand( httpcmd, pMediaCommand->pResult, pMediaCommand->nResultLength );

				else
					bret = m_HTTP->ExecuteURLCommand( httpcmd, NULL, 0 );
			}
			break;
		case NET_REQ_CONTRAST:
			memcpy( &i2, pMediaCommand->pCommand, 4 );

			if( m_MediaConfig.ChannelNumber == 0 )
			{
				sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&VIDEO_CONTRAST=%d\0",
					m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, i2 );
			}

			else
			{
				sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&CHANNEL=%d&VIDEO_CONTRAST=%d\0",
					m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, m_MediaConfig.ChannelNumber, i2 );
			}

			if( m_HTTP )
			{
				if( pMediaCommand->nResultLength > 0 && pMediaCommand->pResult )
					bret = m_HTTP->ExecuteURLCommand( httpcmd, pMediaCommand->pResult, pMediaCommand->nResultLength );

				else
					bret = m_HTTP->ExecuteURLCommand( httpcmd, NULL, 0 );
			}
			break;
		case NET_REQ_SATURATION:
			memcpy( &i2, pMediaCommand->pCommand, 4 );

			if( m_MediaConfig.ChannelNumber == 0 )
			{
				sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&VIDEO_SATURATION=%d\0",
					m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, i2 );
			}

			else
			{
				sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&CHANNEL=%d&VIDEO_SATURATION=%d\0",
					m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, m_MediaConfig.ChannelNumber, i2 );
			}

			if( m_HTTP )
			{
				if( pMediaCommand->nResultLength > 0 && pMediaCommand->pResult )
					bret = m_HTTP->ExecuteURLCommand( httpcmd, pMediaCommand->pResult, pMediaCommand->nResultLength );

				else
					bret = m_HTTP->ExecuteURLCommand( httpcmd, NULL, 0 );
			}
			break;

		case NET_REQ_HUE:
			memcpy( &i2, pMediaCommand->pCommand, 4 );

			if( m_MediaConfig.ChannelNumber == 0 )
			{
				sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&VIDEO_HUE=%d\0",
					m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, i2 );
			}

			else
			{
				sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&CHANNEL=%d&VIDEO_HUE=%d\0",
					m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, m_MediaConfig.ChannelNumber, i2 );
			}

			if( m_HTTP )
			{
				if( pMediaCommand->nResultLength > 0 && pMediaCommand->pResult )
					bret = m_HTTP->ExecuteURLCommand( httpcmd, pMediaCommand->pResult, pMediaCommand->nResultLength );

				else
					bret = m_HTTP->ExecuteURLCommand( httpcmd, NULL, 0 );
			}
			break;
		case NET_REQ_BITRATE:

			memcpy( &i2, pMediaCommand->pCommand, 4 );

			switch(i2) 
			{
			case 0: // BITRATE_28K
				sprintf( s1, "28K\0" );
				break;
			case 1: // BITRATE_56K
				sprintf( s1, "56K\0" );
				break;
			case 2: // BITRATE_128K
				sprintf( s1, "128K\0" );
				break;
			case 3: // BITRATE_256K
				sprintf( s1, "256K\0" );
				break;
			case 4: // BITRATE_384K
				sprintf( s1, "384K\0" );
				break;
			case 5: // BITRATE_500K
				sprintf( s1, "500K\0" );
				break;
			case 6: // BITRATE_750K
				sprintf( s1, "750K\0" );
				break;
			case 7: // BITRATE_1000K
				sprintf( s1, "1M\0" );
				break;
			case 8: // BITRATE_1200K
				sprintf( s1, "1.2M\0" );
				break;
			case 9: // BITRATE_1500K
				sprintf( s1, "1.5M\0" );
				break;
			case 10: // BITRATE_2000K
				sprintf( s1, "2M\0" );
				break;
			case 11: // BITRATE_2500K
				sprintf( s1, "2.5M\0" );
				break;
			case 12: // BITRATE_3000K
				sprintf( s1, "3M\0" );
				break;
			default:
				break;
			}

			if( m_MediaConfig.ChannelNumber == 0 )
			{
				sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&VIDEO_BITRATE=%s\0",
					m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, s1 );
			}

			else
			{
				sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&CHANNEL=%d&VIDEO_BITRATE=%s\0",
					m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, m_MediaConfig.ChannelNumber, s1 );
			}

			if( m_HTTP )
			{
				if( pMediaCommand->nResultLength > 0 && pMediaCommand->pResult )
					bret = m_HTTP->ExecuteURLCommand( httpcmd, pMediaCommand->pResult, pMediaCommand->nResultLength );

				else
					bret = m_HTTP->ExecuteURLCommand( httpcmd, NULL, 0 );
			}
			break;

		case NET_REQ_RESOLUTION:
			memcpy( &i2, pMediaCommand->pCommand, 4 );

			switch( i2 )
			{
			case 0:
				sprintf( s1, "N720x480\0" );
				break;
			case 1:
				sprintf( s1, "N352x240\0" );
				break;
			case 2:
				sprintf( s1, "N160x112\0" );
				break;
			case 3:
				sprintf( s1, "P720x576\0" );
				break;
			case 4:
				sprintf( s1, "P352x288\0" );
				break;
			case 5:
				sprintf( s1, "P176x144\0" );
				break;
			case 6:
				sprintf( s1, "N176x120\0" );
				break;
			case 64:
				sprintf( s1, "N640x480\0" );
				break;
			case 192:
				sprintf( s1, "P640x480\0" );
				break;
			default:
				sprintf( s1, "N720x480\0" );
				break;
			}

			if( m_MediaConfig.ChannelNumber == 0 )
			{
				sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&VIDEO_RESOLUTION=%s\0",
					m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, s1 );
			}

			else
			{
				sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&CHANNEL=%d&VIDEO_RESOLUTION=%s\0",
					m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, m_MediaConfig.ChannelNumber, s1 );
			}

			if( m_HTTP )
			{
				if( pMediaCommand->nResultLength > 0 && pMediaCommand->pResult )
					bret = m_HTTP->ExecuteURLCommand( httpcmd, pMediaCommand->pResult, pMediaCommand->nResultLength );

				else
					bret = m_HTTP->ExecuteURLCommand( httpcmd, NULL, 0 );
			}
			break;
		case NET_REQ_FPS:

			memcpy( &i2, pMediaCommand->pCommand, 4 );

			if( m_MediaConfig.ChannelNumber == 0 )
			{
				sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&VIDEO_FPS_NUM=%d\0",
					m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, i2 );
			}

			else
			{
				sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&CHANNEL=%d&VIDEO_FPS_NUM=%d\0",
					m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, m_MediaConfig.ChannelNumber, i2 );
			}

			if( m_HTTP )
			{
				if( pMediaCommand->nResultLength > 0 && pMediaCommand->pResult )
					bret = m_HTTP->ExecuteURLCommand( httpcmd, pMediaCommand->pResult, pMediaCommand->nResultLength );

				else
					bret = m_HTTP->ExecuteURLCommand( httpcmd, NULL, 0 );
			}
			break;
		case NET_REQ_SAVE_REBOOT:

			sprintf( httpcmd, "%ssystem?USER=%s&PWD=%s&SAVE_REBOOT\0",
				m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password );

			if( m_HTTP )
			{
				if( pMediaCommand->nResultLength > 0 && pMediaCommand->pResult )
					bret = m_HTTP->ExecuteURLCommand( httpcmd, pMediaCommand->pResult, pMediaCommand->nResultLength );

				else
					bret = m_HTTP->ExecuteURLCommand( httpcmd, NULL, 0 );
			}
			break;
		default:
			if( m_ControlPort->ISConnected() )
				bret = m_ControlPort->SendCommamd( pMediaCommand->dwCommandType, pMediaCommand->pCommand, pMediaCommand->nCommandLength );
			break;
		}
	}
	ShowDebugMessage( 1, "[ATCP20][SendCommand][LEAVE]\n\0" );
	return bret;
}

void XADP::ExchangHTTPResultToVideoConfig( char* result )
{
	ShowDebugMessage( 1, "[ATCP20][ExchangHTTPResultToVideoConfig][ENTER]\n\0" );
	int i, j, k;
	char cResult[512];
	char cTemp[64];
	int len = strlen( result );
	if( len >= 512 )
		len = 511;
	memcpy( cResult, result, len );
	cResult[ len ] = 0;	

	for( i = 0; i < len; i++ )
	{
		if( memcmp( &cResult[i], "VIDEO_BITRATE=", 14 ) == 0 )
		{
			i += 14;
			j = i;
			j++;
			k = 0;
			while( cResult[j] != '\'' && ( j < len ) )
                cTemp[k++] = cResult[j++];
			cTemp[k] = 0;

			if( memcmp( cTemp, "3M", 2 ) == 0 ) m_VideoConfig.dwBitsRate = 12;
			else if( memcmp( cTemp, "2.5M", 4 ) == 0 ) m_VideoConfig.dwBitsRate = 11;
			else if( memcmp( cTemp, "2M", 2 ) == 0 ) m_VideoConfig.dwBitsRate = 10;
			else if( memcmp( cTemp, "1.5M", 4 ) == 0 ) m_VideoConfig.dwBitsRate = 9;
			else if( memcmp( cTemp, "1.2M", 4 ) == 0 ) m_VideoConfig.dwBitsRate = 8;
			else if( memcmp( cTemp, "1M", 2 ) == 0 ) m_VideoConfig.dwBitsRate = 7;
			else if( memcmp( cTemp, "750K", 4 ) == 0 ) m_VideoConfig.dwBitsRate = 6;
			else if( memcmp( cTemp, "500K", 4 ) == 0 ) m_VideoConfig.dwBitsRate = 5;
			else if( memcmp( cTemp, "384K", 4 ) == 0 ) m_VideoConfig.dwBitsRate = 4;
			else if( memcmp( cTemp, "256K", 4 ) == 0 ) m_VideoConfig.dwBitsRate = 3;
			else if( memcmp( cTemp, "128K", 4 ) == 0 ) m_VideoConfig.dwBitsRate = 2;
			else if( memcmp( cTemp, "56K", 3 ) == 0 ) m_VideoConfig.dwBitsRate = 1;
			else if( memcmp( cTemp, "28K", 3 ) == 0 ) m_VideoConfig.dwBitsRate = 0;
			else  m_VideoConfig.dwBitsRate = 7;
		}

		if( memcmp( &cResult[i], "VIDEO_RESOLUTION=", 17 ) == 0 )
		{
			i += 17;
			j = i;
			j++;
			k = 0;
			while( cResult[j] != '\'' && ( j < len ) )
				cTemp[k++] = cResult[j++];
			cTemp[k] = 0;

			if( memcmp( cTemp, "N720x480", 8 ) == 0 ) 
			{
				m_VideoConfig.dwTvStander = NET_TVNTSC;
				m_VideoConfig.dwVideoResolution = 0;
			}
			else if( memcmp( cTemp, "N352x240", 8 ) == 0 ) 
			{
				m_VideoConfig.dwTvStander = NET_TVNTSC;
				m_VideoConfig.dwVideoResolution = 1;
			}

			else if( memcmp( cTemp, "N160x112", 8 ) == 0 ) 
			{
				m_VideoConfig.dwTvStander = NET_TVNTSC;
				m_VideoConfig.dwVideoResolution = 2;
			}

			else if( memcmp( cTemp, "P720x576", 8 ) == 0 ) 
			{
				m_VideoConfig.dwTvStander = NET_TVPAL;
				m_VideoConfig.dwVideoResolution = 3;
			}

			else if( memcmp( cTemp, "P352x288", 8 ) == 0 ) 
			{
				m_VideoConfig.dwTvStander = NET_TVPAL;
				m_VideoConfig.dwVideoResolution = 4;
			}

			else if( memcmp( cTemp, "P176x144", 8 ) == 0 ) 
			{
				m_VideoConfig.dwTvStander = NET_TVPAL;
				m_VideoConfig.dwVideoResolution = 5;
			}

			else if( memcmp( cTemp, "N176x120", 8 ) == 0 ) 
			{
				m_VideoConfig.dwTvStander = NET_TVNTSC;
				m_VideoConfig.dwVideoResolution = 6;
			}

			else if( memcmp( cTemp, "N640x480", 8 ) == 0 ) 
			{
				m_VideoConfig.dwTvStander = NET_TVNTSC;
				m_VideoConfig.dwVideoResolution = 64;
			}

			else if( memcmp( cTemp, "P640x480", 8 ) == 0 ) 
			{
				m_VideoConfig.dwTvStander = NET_TVPAL;
				m_VideoConfig.dwVideoResolution = 192;
			}

			else
			{
				m_VideoConfig.dwTvStander = NET_TVPAL;
				m_VideoConfig.dwVideoResolution = 3;
			}
		}

		if( memcmp( &cResult[i], "VIDEO_FPS_NUM=", 14 ) == 0 )
		{
			i += 14;
			j = i;
			j++;
			k = 0;
			while( cResult[j] != '\'' && ( j < len ) )
				cTemp[k++] = cResult[j++];
			cTemp[k] = 0;

			m_VideoConfig.dwFps = atoi(cTemp);
		}

		if( memcmp( &cResult[i], "VIDEO_BRIGHTNESS=", 17 ) == 0 )
		{
			i += 17;
			j = i;
			j++;
			k = 0;
			while( cResult[j] != '\'' && ( j < len ) )
				cTemp[k++] = cResult[j++];
			cTemp[k] = 0;

			m_VideoConfig.dwVideoBrightness = atoi(cTemp);
		}
		if( memcmp( &cResult[i], "VIDEO_CONTRAST=", 15 ) == 0 )
		{
			i += 15;
			j = i;
			j++;
			k = 0;
			while( cResult[j] != '\'' && ( j < len ) )
				cTemp[k++] = cResult[j++];
			cTemp[k] = 0;

			m_VideoConfig.dwVideoContrast = atoi(cTemp);
		}
		if( memcmp( &cResult[i], "VIDEO_HUE=", 10 ) == 0 )
		{
			i += 10;
			j = i;
			j++;
			k = 0;
			while( cResult[j] != '\'' && ( j < len ) )
				cTemp[k++] = cResult[j++];
			cTemp[k] = 0;

			m_VideoConfig.dwVideoHue = atoi(cTemp);
		}
		if( memcmp( &cResult[i], "VIDEO_SATURATION=", 17 ) == 0 )
		{
			i += 17;
			j = i;
			j++;
			k = 0;
			while( cResult[j] != '\'' && ( j < len ) )
				cTemp[k++] = cResult[j++];
			cTemp[k] = 0;

			m_VideoConfig.dwVideoSaturation = atoi(cTemp);
		}
	}
	ShowDebugMessage( 1, "[ATCP20][ExchangHTTPResultToVideoConfig][LEAVE]\n\0" );
}

int XADP::GetNextFrame( int& nDataType, BYTE* pFrameBuffer, int nBufferSize )
{
	int  buffersize = nBufferSize;
	int  nret = 0;
	
	nret = m_StreamingPort->GetOneFrame( m_TempBuf, buffersize );
	
	if( nret > 0 )
	{
		memcpy( pFrameBuffer, m_TempBuf, nret );

		if( m_TempBuf[4] == 1 )
			nDataType = 1;

		else
		if( m_TempBuf[4] == 2 )
			nDataType = 2;

		else
		{
			nDataType = 0;
			//nret = 0;
		}
	}

	else
		ShowDebugMessage( 1, "[ATCP20][GetNextFrame][error]\n\0" );
	
	return nret;
}

int XADP::GetNextIFrame( BYTE* pFrameBuffer, int nBufferSize )
{
	int  buffersize = 256000;
	int  nret = 0;
	DWORD dwI = 0;

	do 
	{
		nret = m_StreamingPort->GetOneFrame( m_TempBuf, buffersize );
		if( nret > 80 )
			memcpy( &dwI, &m_TempBuf[75], sizeof( DWORD ) );
	} while( dwI != 0xB3010000 );

	return nret;
}

void XADP::SetControlDataCallBack( DWORD UserParam, CONTROL_DATA_CALLBACK fnCallBack )
{
	m_fnControlCB = fnCallBack;
	m_ParamControlCB = UserParam;
	m_ControlPort->SetControlCallBack( m_ParamControlCB, (VIDEO_CONTROL_DATA_CALLBACK)m_fnControlCB );
}

void XADP::ShowDebugMessage( int nDebugLevel, TCHAR *pFormat, ...)
{
	if( m_nDebugLevel == 0 )
		return;

	if( nDebugLevel <= m_nDebugLevel )
	{
		va_list ArgList;
		TCHAR tBuffer[1024];
		TCHAR tBuffer2[2048];
		SYSTEMTIME	lst;

		try
		{
			GetLocalTime( &lst );
			va_start( ArgList, pFormat );
			(void)wvsprintf( tBuffer, pFormat, ArgList );
			va_end( ArgList );
			wsprintf( tBuffer2, "%04d%02d%02d %02d%02d%02d %s \0", lst.wYear, lst.wMonth, lst.wDay, lst.wHour, lst.wMinute, lst.wSecond, tBuffer );
			OutputDebugString( tBuffer2 );
		}

		catch (...) 
		{
			OutputDebugString( "DebugMsg Error\n\0" );
		}
	}
}

void XADP::SetDebugMessageLevel( int nDebugLevel )
{
	ShowDebugMessage( 1, "[ATCP20][SetDebugMessageLevel][ENTER]\n\0" );
	m_nDebugLevel = nDebugLevel;
	ShowDebugMessage( 1, "[ATCP20][SetDebugMessageLevel][LEAVE]\n\0" );
}

void XADP::SetMotionInfo( MEDIA_MOTION_INFO* Motioninfo )
{
	ShowDebugMessage( 1, "[ATCP20][SetMotionInfo][ENTER]\n\0" );
	MEDIA_MOTION_INFO mmi;
	char httpcmd[1024];
	memcpy( &mmi, Motioninfo, sizeof(MEDIA_MOTION_INFO) );

	if( m_MediaConfig.ChannelNumber == 0 )
	{
		if( mmi.dwRangeCount == 1 )
			sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&MOTION_ENABLED=%d&MOTION_SETTING=1,%d,%d,%d,%d,%d\0",
			m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, (mmi.dwEnable == 1 ? 1 : 0),
			mmi.dwRange[0][0] , mmi.dwRange[0][1] , mmi.dwRange[0][2] , mmi.dwRange[0][3], mmi.dwSensitive[0] );

		else
		if( mmi.dwRangeCount == 2 )
			sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&MOTION_ENABLED=%d&MOTION_SETTING=1,%d,%d,%d,%d,%d&MOTION_SETTING=2,%d,%d,%d,%d,%d\0",
			m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, (mmi.dwEnable == 1 ? 1 : 0),
			mmi.dwRange[0][0] , mmi.dwRange[0][1] , mmi.dwRange[0][2] , mmi.dwRange[0][3], mmi.dwSensitive[0],
			mmi.dwRange[1][0] , mmi.dwRange[1][1] , mmi.dwRange[1][2] , mmi.dwRange[1][3], mmi.dwSensitive[1] );

		else
		if( mmi.dwRangeCount == 3 )
			sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&MOTION_ENABLED=%d&MOTION_SETTING=1,%d,%d,%d,%d,%d&MOTION_SETTING=2,%d,%d,%d,%d,%d&MOTION_SETTING=3,%d,%d,%d,%d,%d\0",
								m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, (mmi.dwEnable == 1 ? 1 : 0),
								mmi.dwRange[0][0] , mmi.dwRange[0][1] , mmi.dwRange[0][2] , mmi.dwRange[0][3], mmi.dwSensitive[0],
								mmi.dwRange[1][0] , mmi.dwRange[1][1] , mmi.dwRange[1][2] , mmi.dwRange[1][3], mmi.dwSensitive[1],
								mmi.dwRange[2][0] , mmi.dwRange[2][1] , mmi.dwRange[2][2] , mmi.dwRange[2][3], mmi.dwSensitive[2] );
	}

	else
	{
		if( mmi.dwRangeCount == 1 )
			sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&CHANNEL=%d&&MOTION_ENABLED=%dMOTION_SETTING=1,%d,%d,%d,%d,%d\0",
				m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, m_MediaConfig.ChannelNumber, (mmi.dwEnable == 1 ? 1 : 0),
				mmi.dwRange[0][0] , mmi.dwRange[0][1] , mmi.dwRange[0][2] , mmi.dwRange[0][3], mmi.dwSensitive[0] );

		else
		if( mmi.dwRangeCount == 2 )
			sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&CHANNEL=%d&MOTION_ENABLED=%d&MOTION_SETTING=1,%d,%d,%d,%d,%d&MOTION_SETTING=2,%d,%d,%d,%d,%d\0",
				m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, m_MediaConfig.ChannelNumber, (mmi.dwEnable == 1 ? 1 : 0),
				mmi.dwRange[0][0] , mmi.dwRange[0][1] , mmi.dwRange[0][2] , mmi.dwRange[0][3], mmi.dwSensitive[0],
				mmi.dwRange[1][0] , mmi.dwRange[1][1] , mmi.dwRange[1][2] , mmi.dwRange[1][3], mmi.dwSensitive[1] );

		else
		if( mmi.dwRangeCount == 3 )
			sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&CHANNEL=%d&MOTION_ENABLED=%d&MOTION_SETTING=1,%d,%d,%d,%d,%d&MOTION_SETTING=2,%d,%d,%d,%d,%d&MOTION_SETTING=3,%d,%d,%d,%d,%d\0",
								m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, m_MediaConfig.ChannelNumber, (mmi.dwEnable == 1 ? 1 : 0),
								mmi.dwRange[0][0] , mmi.dwRange[0][1] , mmi.dwRange[0][2] , mmi.dwRange[0][3], mmi.dwSensitive[0],
								mmi.dwRange[1][0] , mmi.dwRange[1][1] , mmi.dwRange[1][2] , mmi.dwRange[1][3], mmi.dwSensitive[1],
								mmi.dwRange[2][0] , mmi.dwRange[2][1] , mmi.dwRange[2][2] , mmi.dwRange[2][3], mmi.dwSensitive[2] );
	}

	m_HTTP->ExecuteURLCommand( httpcmd, NULL, 0 );
	ShowDebugMessage( 1, "[ATCP20][SetMotionInfo][LEAVE]\n\0" );
}

void XADP::GetMotionInfo( MEDIA_MOTION_INFO* Motioninfo )
{
	ShowDebugMessage( 1, "[ATCP20][GetMotionInfo][ENTER]\n\0" );
	MEDIA_MOTION_INFO mmi;
	char httpcmd[1024];
	char httpresult[1024];
	char tmp[5];
	int fc = 0;
	int sc = 0;
	int fieldcounter = 0;
	int objdex = 0;

	if( m_MediaConfig.ChannelNumber == 0 )
	{
		sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&MOTION_ENABLED&MOTION_STATUS\0",
			m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password );
	}

	else
	{
		sprintf( httpcmd, "%smpeg4?USER=%s&PWD=%s&CHANNEL=%d&MOTION_ENABLED&MOTION_STATUS\0",
			m_UrlTarget, m_MediaConfig.UserID, m_MediaConfig.Password, m_MediaConfig.ChannelNumber );
	}

	if( m_HTTP->ExecuteURLCommand( httpcmd, httpresult, 1024 ) )
	{
		memset( &mmi, 0, sizeof(MEDIA_MOTION_INFO) );
		for( int i = 0; i < 1024; i++ )
		{
			if( memcmp( &httpresult[i], "MOTION_ENABLED=", 15 ) == 0 )
			{
				memset( tmp, 0, 5 );
				memcpy( tmp, &httpresult[i+16], 4 );
				tmp[4] = 0;
				mmi.dwEnable = HexToDWORD( tmp );
			}

			if( memcmp( &httpresult[i], "MOTION_STATUS=", 14 ) == 0 )
			{
				mmi.dwRangeCount++;
				fc = 0;
				sc = 0;
				fieldcounter = 0;
				objdex = 0;
				memset( tmp, 0, 5 );
				while( fieldcounter < 6 )
				{
					if( httpresult[ i + 14 + sc ] == ',' || httpresult[ i + 14 + sc ] == 10 )
					{
						tmp[fc] = 0;
						fc = 0;
						switch( fieldcounter ) 
						{
						case 0:
							objdex = atoi( tmp );
							break;
						case 1:
							mmi.dwRange[ objdex-1 ][0] = atoi( tmp );
							break;
						case 2:
							mmi.dwRange[ objdex-1 ][1] = atoi( tmp );
							break;
						case 3:
							mmi.dwRange[ objdex-1 ][2] = atoi( tmp );
							break;
						case 4:
							mmi.dwRange[ objdex-1 ][3] = atoi( tmp );
							break;
						case 5:
							mmi.dwSensitive[ objdex-1 ] = atoi( tmp );
							break;						
						}
						fieldcounter++;
						memset( tmp, 0, 5 );
					}

					else
					{
						tmp[fc++] = httpresult[ i + 14 + sc ];
					}
					sc++;
				}
			}
		}

		memcpy( Motioninfo, &mmi, sizeof(MEDIA_MOTION_INFO) );
	}

	else
	{
		if( Motioninfo )
			memset( Motioninfo, 0, sizeof(MEDIA_MOTION_INFO) );
	}
	ShowDebugMessage( 1, "[ATCP20][GetMotionInfo][LEAVE]\n\0" );
}

DWORD XADP::GetBeginTime()
{
	return 0;
}

DWORD XADP::GetEndTime()
{
	return 0;
}

DWORD XADP::HexToDWORD( char* hex )
{
	ShowDebugMessage( 1, "[ATCP20][HexToDWORD][ENTER]\n\0" );
	char cHex[16];
	char cHexNo0x[16];
	int nLen = strlen(hex);
	DWORD nT = 0;
	DWORD nR = 0;

	memcpy( cHex, hex, nLen );
	cHex[ nLen ] = 0;

	for( int i = 0; i < nLen; i++ )
	{
		if( memcmp( &cHex[i], "0x", 2 ) == 0 )
		{
			memcpy( cHexNo0x, &cHex[i+2], nLen - i - 2 );
			nLen = nLen - i - 2;
			cHexNo0x[ nLen ] = 0;	
			break;
		}

		if( memcmp( &cHex[i], "0X", 2 ) == 0 )
		{
			memcpy( cHexNo0x, &cHex[i+2], nLen - i - 2 );
			nLen = nLen - i - 2;
			cHexNo0x[ nLen ] = 0;			
			break;
		}
	}

	for( int j = 0; j < nLen; j++ )
	{
		switch( cHexNo0x[j] )
		{
		case '0':
			nT = 0;
			break;
		case '1':
			nT = 1;
			break;
		case '2':
			nT = 2;
			break;
		case '3':
			nT = 3;
			break;
		case '4':
			nT = 4;
			break;
		case '5':
			nT = 5;
			break;
		case '6':
			nT = 6;
			break;
		case '7':
			nT = 7;
			break;
		case '8':
			nT = 8;
			break;
		case '9':
			nT = 9;
			break;
		case 'a':
		case 'A':
			nT = 10;
			break;
		case 'b':
		case 'B':
			nT = 11;
			break;
		case 'c':
		case 'C':
			nT = 12;
			break;
		case 'd':
		case 'D':
			nT = 13;
			break;
		case 'e':
		case 'E':
			nT = 14;
			break;
		case 'f':
		case 'F':
			nT = 15;
			break;
		default :
			break;
		}
		nR = nT|(nR<<4);
	}
	ShowDebugMessage( 1, "[ATCP20][HexToDWORD][LEAVE]\n\0" );
	return nR;
}