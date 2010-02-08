#include "stdafx.h"
#include "CFraw.h"
#include "MemoryBitstream.h"




CFraw::CFraw() 
	: m_pFileReader(NULL)
    , m_pFrame(NULL)
    , m_bConvertFolder(false)
	, m_tSpecifyBeginDateTime(0)
	, m_tSpecifyEndDateTime(0)
	, m_dwResolution(0)
	, m_dwFPS(0)
	, m_dwBitrate(0)
	, m_nWidth(0)
	, m_nHeight(0)
	, m_nTVStd(0)
	, m_hRecordHandle( NULL)
	, m_dwIFrameCount(0)
	, m_dwPFrameCount(0)
{
	FFRAW::RAWFILE_HEADER_INFO m_RawFileHeaderInfo;
	FFRAW::RAWFILE_RECORD_INFO m_RawFileRecordInfo;

	memset( &m_RawFileHeaderInfo, 0, sizeof(FFRAW::RAWFILE_HEADER_INFO));
	memset( &m_RawFileRecordInfo, 0, sizeof(FFRAW::RAWFILE_RECORD_INFO));
}

CFraw::~CFraw()
{
	ReleaseFileReader();
	CloseFile();
}

BOOL CFraw::CreateFile( const DWORD dwBiteRate, const DWORD dwFps, const DWORD dwResolution, char * pszFileName, char * pszFilePath )
{
	if( m_hRecordHandle )
	{
		FFRAW::FStopRecord( m_hRecordHandle, &m_RawFileRecordInfo);
		FFRAW::FExit( m_hRecordHandle);
		m_hRecordHandle = NULL;
	}

	m_hRecordHandle = FFRAW::FInit();
	if( m_hRecordHandle )
	{
		m_RawFileHeaderInfo.dwFirstB2 = 0xB2010000;
		m_RawFileHeaderInfo.dwStreamType = 0x00000022;
		m_RawFileHeaderInfo.dwVideoType = 0x00000011;
		m_RawFileHeaderInfo.dwAudioType = 0x00000022;
		m_RawFileHeaderInfo.dwControlType = 0x000000FF;	
		m_RawFileHeaderInfo.dwAudio2Type = 0x000000FF;
		m_RawFileHeaderInfo.dwControl2Type	= 0x000000FF;
		m_RawFileHeaderInfo.dwBiteRate = dwBiteRate;
		m_RawFileHeaderInfo.dwFps = dwFps;
		m_RawFileHeaderInfo.dwResolution = dwResolution;
		m_RawFileHeaderInfo.dwReserve1 = 0x000000FF;
		m_RawFileHeaderInfo.dwReserve2 = 0x000000FF;
		m_RawFileHeaderInfo.dwReserve3 = 0x000000FF;
		m_RawFileHeaderInfo.dwReserve4 = 0x000000FF;
		m_RawFileHeaderInfo.dwReserve5 = 0x000000FF;
		m_RawFileHeaderInfo.dwReserve6 = 0x000000FF;
		

		char szFullFileName[512];
		{
			//char * pszFileName, char * pszFilePath
			if( pszFilePath )
			{
				sprintf( szFullFileName, "%s\\%s", pszFilePath, pszFileName);
			}
			else
			{
				sprintf( szFullFileName, "%s", pszFileName );
			}
		}
		if( FFRAW::FStartRecord( m_hRecordHandle, szFullFileName, &m_RawFileHeaderInfo ) )
		{
		}
		else
		{
			FFRAW::FStopRecord( m_hRecordHandle, &m_RawFileRecordInfo);
			FFRAW::FExit( m_hRecordHandle);
			m_hRecordHandle = NULL;
			return FALSE;
		}
		return TRUE;
	}

	return FALSE;
}

void CFraw::CloseFile( void)
{
	if( m_hRecordHandle )
	{
		FFRAW::FStopRecord( m_hRecordHandle, &m_RawFileRecordInfo);
		FFRAW::FExit( m_hRecordHandle);
		m_hRecordHandle = NULL;
	}
}

BOOL CFraw::InitFileReader( char * pszRawFileName )
{
	if( m_pFileReader )
	{
		ReleaseFileReader();
	}

	if( pszRawFileName == NULL )
	{
		return FALSE;
	}

	if( m_pFrame == NULL )
		m_pFrame = new unsigned char[2073600];
	memset(m_pFrame, 0x00, 2073600);
	m_dwFullFrameLen = 0;
	m_pFileReader = new CMpeg4Reader();
	memset( &m_DataInfo, 0x00, sizeof(DataInfo_t));

	MEDIA_CONNECTION_CONFIG mcc;
	memset( &mcc, 0x00, sizeof(MEDIA_CONNECTION_CONFIG));
	strcpy( mcc.PlayFileName, pszRawFileName );
	mcc.ContactType = CONTACT_TYPE_PLAYBACK;

	m_pFileReader->SetMediaConfig(&mcc);
	if(!m_pFileReader->OpenFile())
	{
		ReleaseFileReader();
		return FALSE;
	}

	if( !m_pFileReader->GetFirstIFrame( m_pFrame, m_DataInfo) )
	{
		ReleaseFileReader();
		return FALSE;
	}
	m_pFileReader->SetFirstIPos();
	m_dwFullFrameLen = m_DataInfo.dw20DataLength + TCP1020_HEADER_SIZE;

	TCP20VideoHeader_t TCP20Header;
	{
		memset( &TCP20Header, 0x00, sizeof(TCP20VideoHeader_t));
		memcpy( &TCP20Header, m_pFrame+TCP1020_HEADER_SIZE, sizeof(TCP20VideoHeader_t));
	}
	{
		m_dwFPS = TCP20Header.bFpsNumber;
		m_dwBitrate = TCP20Header.bBitrate;
		m_dwResolution = TCP20Header.bResolution;
	}

	SetVideoSize( m_dwResolution );

	return TRUE;
}

void CFraw::ReleaseFileReader( void)
{
	if( m_pFrame )
	{
		delete [] m_pFrame;
		m_pFrame = NULL;
	}
	m_dwFullFrameLen = 0;

	if( m_pFileReader )
	{
		delete m_pFileReader;
		m_pFileReader = NULL;
	}
}

BOOL CFraw::WriteFile( DWORD dwMSGType, DWORD dwFrameType, BYTE * pData, DWORD dwDataLen )
{
	if( !m_hRecordHandle )
		return FALSE;

	switch( dwMSGType)
	{
	case 2:
	case 3:
		break;

	case 1:
	case 4:
	case 5:
		{
			switch( dwFrameType)
			{
			case FRAME_TYPE_I:
				{
					m_dwIFrameCount += 1;
					AfxGetApp()->GetMainWnd()->PostMessage(PRINT_INFO, 2, m_dwIFrameCount);
				}
				break;

			case FRAME_TYPE_P:
				{
					m_dwPFrameCount += 1;
					AfxGetApp()->GetMainWnd()->PostMessage(PRINT_INFO, 1, m_dwPFrameCount);
				}
				break;
			}
		}
		break;
	}
	FFRAW::FWriteData( m_hRecordHandle, pData, dwDataLen);
	return TRUE;
}

BOOL  CFraw::CheckPeriodOfTime( DWORD dwTime  )
{
	if( m_tSpecifyBeginDateTime>0 &&  m_tSpecifyEndDateTime>0 )
	{
		if( m_tSpecifyEndDateTime >= m_tSpecifyBeginDateTime )
         {
		   
			if( dwTime >= m_tSpecifyBeginDateTime && dwTime <= m_tSpecifyEndDateTime )
			{
                        return TRUE;
			}
	     }
		return FALSE;
	}
	return TRUE;
}

void CFraw::SetVideoSize( int nResolution )
{
	switch ( nResolution ) 
	{
	case 0:
		{
			m_nWidth = 720;	
			m_nHeight = 480;
			m_nTVStd = NET_TVNTSC;
		}
		break;
	case 1:	// NTSC CIF
		{
			m_nWidth = 352;	
			m_nHeight = 240;
			m_nTVStd = NET_TVNTSC;
		}
		break;
	case 2:	// NTSC QCIF
		{
			m_nWidth = 160;	
			m_nHeight = 112;
			m_nTVStd = NET_TVNTSC;
		}
		break;
	case 70:	// NTSC CIF
		{
			m_nWidth = 320;	
			m_nHeight = 240;
			m_nTVStd = NET_TVNTSC;
		}
		break;
	case 71:	// NTSC QCIF
		{
			m_nWidth = 160;	
			m_nHeight = 120;
			m_nTVStd = NET_TVNTSC;
		}
		break;
	case 3 :	// PAL D1
		{
			m_nWidth = 720;	
			m_nHeight = 576;
			m_nTVStd = NET_TVPAL;
		}
		break;
	case 4 :	// PAL CIF
		{
			m_nWidth = 352;	
			m_nHeight = 288;
			m_nTVStd = NET_TVPAL;
		}
		break;
	case 5:	// PAL QCIF
		{
			m_nWidth = 176;	
			m_nHeight = 144;
			m_nTVStd = NET_TVPAL;
		}
		break;
	case 6: // NTSC QCIF
		{
			m_nWidth = 176;	
			m_nHeight = 120;
			m_nTVStd = NET_TVNTSC;
		}
		break;
	case 64: // VGA
		{
			m_nWidth = 640;	
			m_nHeight = 480;
			m_nTVStd = NET_TVNTSC;
		}
		break;
	case 65: // Mega Pixel
		{
			m_nWidth = 1280;	
			m_nHeight = 720;
			m_nTVStd = NET_TVNTSC;
		}
		break;
	case 66: // Mega Pixel
		{
			m_nWidth = 1280;	
			m_nHeight = 960;
			m_nTVStd = NET_TVNTSC;
		}
		break;
	case 67: // Mega Pixel
		{
			m_nWidth = 1280;	
			m_nHeight = 1024;
			m_nTVStd = NET_TVNTSC;
		}
		break;
	case 68: // Mega Pixel
		{
			m_nWidth = 1920;	
			m_nHeight = 1080;
			m_nTVStd = NET_TVNTSC;
		}
		break;
	case 192: // VGA
		{
			m_nWidth = 640;	
			m_nHeight = 480;
			m_nTVStd = NET_TVPAL;
		}
		break;
	default :				// NTSC D1
		{
			m_nWidth = 720;	
			m_nHeight = 480;
			m_nTVStd = NET_TVNTSC;
		}
		break;
	}
}

BOOL CFraw::ConvertFile( char * pszFileName,  char *pcaRawFileName, char * pszFilePath,
	const time_t tSpecifyBeginDateTime, const time_t tSpecifyEndDateTime )
{
	BOOL bResult = FALSE;
	DWORD dwDataLen = 0;

	m_dwIFrameCount = 0;
	m_dwPFrameCount = 0;

	if( InitFileReader( pcaRawFileName) )
	{
		if( CreateFile( m_dwBitrate, m_dwFPS, m_dwResolution, pszFileName, pszFilePath) )
		{
			m_pFileReader->GetRawTail( m_TailInfo);
			m_dwFileBeginTime  = m_TailInfo.dwBeginTime; 
			m_dwFileEndTime    = m_TailInfo.dwEndTime;

			{
				m_tSpecifyBeginDateTime = tSpecifyBeginDateTime;
				m_tSpecifyEndDateTime = tSpecifyEndDateTime;
			}

			bool bAudioInTimeRage = false;
			time_t dwTime = 0;
			VIDEO_B2_FRAME * b2 = NULL;
			BOOL bHasFirstIFrame = FALSE;

			while( bHasFirstIFrame == FALSE && m_pFileReader->GetNextFrame( m_pFrame, m_DataInfo) )
			{
				if( m_DataInfo.dwFrameType != 1 ) // I Frame
				{
					continue;
				}
					
				dwTime = 0;
				if( m_pFrame[0]==0x00 && m_pFrame[1]==0x00 && m_pFrame[2]==0x01 && m_pFrame[3]==0xB2 )
				{
					switch( m_DataInfo.dwMSGType )
					{
					case 1:
					case 4:
					case 5:
						{
							VIDEO_B2_FRAME * b2 = NULL;
							b2 = (VIDEO_B2_FRAME *)m_pFrame;
							dwTime = b2->prdata.date_time;
						}
						break;
					}
				}

				if( !CheckPeriodOfTime( dwTime) )
					continue;

				m_dwFullFrameLen = m_DataInfo.dw20DataLength + TCP1020_HEADER_SIZE;

				if( WriteFile( m_DataInfo.dwMSGType, m_DataInfo.dwFrameType, m_pFrame, m_dwFullFrameLen ) )
				{
					bHasFirstIFrame = TRUE;
					bResult = TRUE;
					while( m_pFileReader->GetNextFrame( m_pFrame, m_DataInfo) )
					{
						dwTime = 0;
						if( m_pFrame[0]==0x00 && m_pFrame[1]==0x00 && m_pFrame[2]==0x01 && m_pFrame[3]==0xB2 )
						{
							switch( m_DataInfo.dwMSGType )
							{
							case 1:
							case 4:
							case 5:
								{
									VIDEO_B2_FRAME * b2 = NULL;
									b2 = (VIDEO_B2_FRAME *)m_pFrame;
									dwTime = b2->prdata.date_time;
									bAudioInTimeRage = false;
								}
								break;
							}
						}

						if( bAudioInTimeRage == true || CheckPeriodOfTime( dwTime) )
						{
							m_dwFullFrameLen = m_DataInfo.dw20DataLength + TCP1020_HEADER_SIZE;
							if( !WriteFile( m_DataInfo.dwMSGType, m_DataInfo.dwFrameType, m_pFrame, m_dwFullFrameLen ) )
							{
								bResult = FALSE;
								break;
							}
							bAudioInTimeRage = true;
						}
						else
						{
							bAudioInTimeRage = false;
						}
					}
				}
			}
		}
	}

	ReleaseFileReader();
	CloseFile();

	return bResult;
}
