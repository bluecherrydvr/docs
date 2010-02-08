#include "stdafx.h"
#include "AVIWriter.h"
//#include "XVIDCODEC.h"
#include "MemoryBitstream.h"
#include "JpegEncoder.h"

#define USEENCODER JPEGENCODER


CAVIWriter::CAVIWriter( const bool bOSD, const char * pszTitleText, const int nOSDTimeType) 
    : CXEncoder()
    , m_pAviAudio(NULL)
    , m_pAviVideo(NULL)
    , m_pAviFile(NULL)
    , m_pcFileReader(NULL)
    , m_bProcessing(false)
    , m_pFrame(NULL)
    , m_bConvertFolder(false)
    , m_nMP4ConvertFormat(1)
    , m_hEncoder( NULL)
//  , m_nDecoder( -1)
    , m_bOSD( bOSD)
	, m_szOSDText( pszTitleText)
	, m_tSpecifyBeginDateTime(0)
	, m_tSpecifyEndDateTime(0)
	, MAX_FRAMESIZE( 1024*1024)
	, m_dAvgFrameDiffTime( 0)
	, m_dwAvgFrameDiffTimeValue( 0)
	, m_nDiffTimeSum( 0)
	, m_dwDiffTimeSumCount( 0)
{
	{
		//Steve0828
		memset( &m_PreB2, 0, sizeof(VIDEO_B2_FRAME));
		m_pPreFrame.buf = new char[MAX_FRAMESIZE];
		m_pPreFrame.len = MAX_FRAMESIZE;
	}

   
    //CoInitializeEx(0, COINIT_MULTITHREADED);
	// initializes the AVIFile library
	AVIFileInit();
	//Allocdec memnory to object    
	//InitAllCodec();
}

CAVIWriter::~CAVIWriter()
{   
	//Relaese m_cAVIWriter ,AVIFileExit,CoUninitialize

	if( m_pPreFrame.buf )
		delete [] m_pPreFrame.buf;
	ReleaseAllCodec();
	AVIFileExit();
	CoUninitialize();
}


void CAVIWriter::SetOSD( const bool bOSD, const char * pszTitleText, const int nOSDTimeType )
{   
	
	m_bOSD = bOSD;
	m_szOSDText = pszTitleText;
	m_nOSDTime = nOSDTimeType;
}

void CAVIWriter::SetOSD( const bool bOSD )
{
	m_bOSD = bOSD;
}

void CAVIWriter::SetOSDText( const char * pszTitleText)
{   
         
    m_szOSDText = pszTitleText;
}

void CAVIWriter::SetOSDTimeType( const int nOSDTimeType )
{
	m_nOSDTime = nOSDTimeType;
}

int CAVIWriter::GetVideoRate(int iTVStd, int iFPS)
{
	// PAL
	if(NET_TVPAL == iTVStd)
	{
		if (iFPS == 25)
		{
			return 25025;
		}
		else if(iFPS == 12) 
		{
			return 12512; 
		}
		else if(iFPS == 8) 
		{
			return 8341; 
		}
		else if(iFPS == 6)
		{
			return 6256; 
		}
		else if(iFPS == 5)
		{
			return 5005;
		}
		else if(iFPS == 4)
		{
			return 4170;
		}
		else if(iFPS == 3)
		{
			return  3128; 
		}
		else if(iFPS == 2)
		{
			return 2085; 
		}
		else if(iFPS != 0)
		{
			return 1000*iFPS ;
		}
		else
		{
			return 1000;
		}
	}
	// NTSC
	else
	{
		if (iFPS == 30)
		{
			return 30000;
		}
		else if(iFPS == 24)
		{
			return 24024;
		}
		else if(iFPS == 15)
		{
			return 15015;
		}
		else if(iFPS == 10) 
		{
			return 10010;
		}
		else if(iFPS == 8)
		{
			return 8008 ;
		}
		else if(iFPS == 7) 
		{
			return 7075;
		}
		else if(iFPS == 6)
		{
			return 6006;
		}
		else if(iFPS == 5)
		{
			return 5005;
		}
		else if(iFPS == 4)
		{
			return 4289;
		}
		else if(iFPS == 3)
		{
			return 3003;
		}
		else if(iFPS == 2)
		{
			return 2002;
		}
		else if(iFPS !=0 )
		{
			return 1001*iFPS ;
		}
		else
		{
			return 1001;
		}
	}
}

void CAVIWriter::GetVideoSize(int iResolution, int& iWidth, int& iHeight)
{
	switch (iResolution) 
	{
		case 0:
			{
				iWidth = 720;	
				iHeight = 480;
				m_iTVStd = NET_TVNTSC;
			}
			break;
				// NTSC CIF
		case 1:
			{
				iWidth = 352;	
				iHeight = 240;
				m_iTVStd = NET_TVNTSC;
			}
			break;
			// NTSC QCIF
		case 2:	
			{
				iWidth = 160;	
				iHeight = 112;
				m_iTVStd = NET_TVNTSC;
			}
			break;
			// PAL D1
		case 3 :	
			{
				iWidth = 720;	
				iHeight = 576;
				m_iTVStd = NET_TVPAL;
			}
			break;
			// PAL CIF
		case 4 :	
			{
				iWidth = 352;	
				iHeight = 288;
				m_iTVStd = NET_TVPAL;
			}
			break;
			// PAL QCIF
		case 5:	
			{
				iWidth = 176;	
				iHeight = 144;
				m_iTVStd = NET_TVPAL;
			}
			break;
			// NTSC QCIF
		case 6: 
			{
				iWidth = 176;	
				iHeight = 120;
				m_iTVStd = NET_TVNTSC;
			}
			break;
			//Steve
        case 70: 
			{
				iWidth = 320;	
				iHeight = 240;
				m_iTVStd = NET_TVNTSC;
			}
			break;
			//Steve
        case 71: 
			{
				iWidth = 160;	
				iHeight = 120;
				m_iTVStd = NET_TVNTSC;
			}
			break;
			 // VGA
		case 64:
			{
				iWidth = 640;	
				iHeight = 480;
				m_iTVStd = NET_TVNTSC;
			}
			break;
			// Mega Pixel
		case 65: 
			{
				iWidth = 1280;	
				iHeight = 720;
				m_iTVStd = NET_TVNTSC;
			}
			break;
			 // Mega Pixel
		case 66:
			{
				iWidth = 1280;	
				iHeight = 960;
				m_iTVStd = NET_TVNTSC;
			}
			break;
			// Mega Pixel
		case 67: 
			{
				iWidth = 1280;	
				iHeight = 1024;
				m_iTVStd = NET_TVNTSC;
			}
			break;
			 // Mega Pixel
		case 68:
			{
				iWidth = 1920;	
				iHeight = 1080;
				m_iTVStd = NET_TVNTSC;
			}
			break;
			 // VGA
		case 192:
			{
				iWidth = 640;	
				iHeight = 480;
				m_iTVStd = NET_TVPAL;
			}
			break;
			// NTSC D1
		default :				
			{
				iWidth = 720;	
				iHeight = 480;
				m_iTVStd = NET_TVNTSC;
			}
			break;
	}
}

bool CAVIWriter::GetResolutionAndTVStander(int iWidth, int iHeight )
{
	switch(iWidth)
	{
	case 1920:
		{
			if(1080 == iHeight)
			{
				m_iTVStd = NET_TVNTSC;
				m_iResolution = 68;
				return true;
			}
		}
		break;
	case 1280:
		{
			if(720 == iHeight)
			{
				m_iTVStd = NET_TVNTSC;
				m_iResolution = 65;
				return true;
			}
			else if  (960 == iHeight)
			{
				m_iTVStd = NET_TVNTSC;
				m_iResolution = 66;
				return true;				
			}
			else if (1024 == iHeight)
			{
				m_iTVStd = NET_TVNTSC;
				m_iResolution = 67;
				return true;
			}
		}
		break;
	case 720:
		{
			if(480 == iHeight)
			{
				m_iTVStd = NET_TVNTSC;
				m_iResolution = 0;
				return true;
			}
			else if (576 == iHeight)
			{
				m_iTVStd = NET_TVPAL;
				m_iResolution = 3;
				return true;
			}
		}
		break;
	case 352:
		{
			if(240 == iHeight)
			{
				m_iTVStd = NET_TVNTSC;
				m_iResolution = 1;
				return true;
			}
			else if (288 == iHeight)
			{
				m_iTVStd = NET_TVPAL;
				m_iResolution = 4;
				return true;
			}
		}
  
		break;
	case 160:
		{
			//Steve
			if(112 == iHeight)
			{
				m_iTVStd = NET_TVNTSC;
				m_iResolution = 2;
				return true;
			}
			else if (120 == iHeight)
			{
				m_iTVStd = NET_TVNTSC;
				m_iResolution = 71;
				return true;
			}
		
		}
		break;
   
	case 176:
		{
			if(144 == iHeight)
			{
				m_iTVStd = NET_TVPAL;
				m_iResolution = 5;
				return true;
			}
			else if (120 == iHeight)
			{
				m_iTVStd = NET_TVNTSC;
				m_iResolution = 6;
				return true;
			}
		}
        break;
	case 640:
		{
			if(480 == iHeight)
			{
				m_iTVStd = NET_TVNTSC;
				m_iResolution = 64;
				return true;
			}
		}
        break;
		//Steve
    case 320:
		{
			if(240 == iHeight)
			{
				m_iTVStd = NET_TVNTSC;
				m_iResolution = 70;
				return true;
			}
		}
        break;
	default:
		break;
	}
	return false;
}

void CAVIWriter::GetTimeToStr(time_t time, char *buf )
{
	struct tm *newtime;
	if(0 > time)
	{
		buf[0] = '\0';
		return;
	}
	newtime = gmtime((time_t *)&time);
	sprintf( buf, "%04d/%02d/%02d %02d:%02d:%02d\0", newtime->tm_year+1900, newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec );
	
}

bool CAVIWriter::CreateAudioStream()
{
	HRESULT hr;

	if(m_pAviAudio)
		DestroyAudioStream();

	// audio stream
	memset(&m_sAudioStreamInfo, 0, sizeof(AVISTREAMINFO));
	memset(&m_sWavFormat, 0, sizeof(WAVEFORMATEX));
	m_sWavFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_sWavFormat.cbSize = 0;
	m_sWavFormat.nChannels = 1;
	m_sWavFormat.wBitsPerSample = 16;
	m_sWavFormat.nSamplesPerSec = 8000;
	m_sWavFormat.nBlockAlign = m_sWavFormat.nChannels*m_sWavFormat.wBitsPerSample/8;
	m_sWavFormat.nAvgBytesPerSec = m_sWavFormat.nSamplesPerSec*m_sWavFormat.nBlockAlign;

	m_sAudioStreamInfo.fccType  = streamtypeAUDIO;
	m_sAudioStreamInfo.dwScale = m_sWavFormat.nBlockAlign;
	m_sAudioStreamInfo.dwRate = m_sWavFormat.nAvgBytesPerSec;
	m_sAudioStreamInfo.dwSampleSize = m_sWavFormat.nBlockAlign;
	m_sAudioStreamInfo.dwQuality = (DWORD)-1;

	// Create a interface to the new stream.
	hr = AVIFileCreateStream(m_pAviFile, &m_pAviAudio, &m_sAudioStreamInfo);
	if (hr != AVIERR_OK)
	{
		return false;
	}

	// sets the format of a stream at the specified position
	hr = AVIStreamSetFormat(m_pAviAudio, 0, &m_sWavFormat, sizeof(m_sWavFormat));
	if (hr != AVIERR_OK) 
	{
		DestroyAudioStream();
		return false;
	}

	return true;
}

bool CAVIWriter::CreateVideoStream()
{
	HRESULT hr;
	int iAviFrameSize = 0;
	BYTE btSEQHEader[21];

	if(m_pAviVideo)
		DestroyVideoStream();

	if(m_iWidth == 0 || m_iHeight == 0 || m_iFPS == 0)
	{
		if(GetSequenceHeader(m_pFrame, m_dwFullFrameLen, btSEQHEader) == 21)
		{
			if( GetResolutionAndFPSBySequenceHeader( btSEQHEader, 21, m_iWidth, m_iHeight, m_iFPS ) )
			{
				if(m_iWidth == 0 || m_iHeight == 0 || m_iFPS == 0)
					return false;
				else
				{
					GetResolutionAndTVStander(m_iWidth, m_iHeight);
				}
			}
			else
				return false;
		}
		else
			return false;
	}

	iAviFrameSize = m_iWidth*m_iHeight*(m_iBitCount/8);	// 24 bits

	// clear the struct
	memset(&m_sStramInfo, 0, sizeof(AVISTREAMINFO));
	// video
	m_sStramInfo.fccType                = mmioFOURCC('v', 'i', 'd', 's');
	m_sStramInfo.fccHandler             = mmioFOURCC('M', 'J', 'P', 'G');
	//m_sStramInfo.fccHandler             = mmioFOURCC('D', 'X', '5', '0');
	m_sStramInfo.dwScale                = 1001;
	m_sStramInfo.dwRate                 = GetVideoRate(m_iTVStd, m_iFPS);
	m_sStramInfo.dwSuggestedBufferSize  = iAviFrameSize;
	SetRect(&m_sStramInfo.rcFrame, 0, 0, (int) m_iWidth, (int) m_iHeight); 

	// Create a interface to the new stream.
	hr = AVIFileCreateStream(m_pAviFile, &m_pAviVideo, &m_sStramInfo);
	if (hr != AVIERR_OK) 
	{
		return false;
	}

	memset(&m_sBmpInfoHdr, 0x00, sizeof(BITMAPINFOHEADER));
	m_sBmpInfoHdr.biSize = sizeof(BITMAPINFOHEADER);
	m_sBmpInfoHdr.biWidth = m_iWidth;
	m_sBmpInfoHdr.biHeight = m_iHeight;
	m_sBmpInfoHdr.biPlanes = 1;
	m_sBmpInfoHdr.biBitCount = m_iBitCount;
	m_sBmpInfoHdr.biCompression = m_sStramInfo.fccHandler;  
	m_sBmpInfoHdr.biSizeImage = iAviFrameSize;  
	m_sBmpInfoHdr.biXPelsPerMeter = 0;  
	m_sBmpInfoHdr.biYPelsPerMeter = 0;  
	m_sBmpInfoHdr.biClrUsed =0;  
	m_sBmpInfoHdr.biClrImportant =0;

	// sets the format of a stream at the specified position
	hr = AVIStreamSetFormat(m_pAviVideo, 0, &m_sBmpInfoHdr, sizeof(m_sBmpInfoHdr));
	if (hr != AVIERR_OK) 
	{
		DestroyVideoStream();
		return false;
	}
	return true;
}

bool CAVIWriter::DestroyVideoStream()
{
	if(m_pAviVideo)
	{
		AVIStreamRelease(m_pAviVideo);
		m_pAviVideo = NULL;
	}
	return true;
}

bool CAVIWriter::DestroyAudioStream()
{
	if(m_pAviAudio)
	{
		AVIStreamRelease(m_pAviAudio);
		m_pAviAudio = NULL;
	}
	return true;
}

bool CAVIWriter::Open(char *pFIleName, char * pPath )
{
	HRESULT hr = 0;

	m_iBitCount = 24;
	m_iWidth = 0;
	m_iHeight = 0;
	m_iFPS = 0;
	m_iTVStd = 0;
	m_dwVFrameCounter = 0;
	m_dwAFrameCounter = 0;
	m_sAVIFileName = pFIleName;
	m_bProcessing = true;
	m_dwAudioSampleStart = 0;
	m_dwIFrameCount = 0;
	m_dwPFrameCount = 0;

	if(m_pAviFile)
		Close();

	// Create a AVI file.
	
	CoInitializeEx(0, COINIT_MULTITHREADED) ;

	char * pszFullFileName = NULL;
	if( pPath )
	{
		pszFullFileName = new char[ strlen(pPath) + strlen(pFIleName) + 2 ];
		sprintf( pszFullFileName, "%s\\%s", pPath, pFIleName);
	}
	else
	{
		pszFullFileName = new char[ strlen(pFIleName) + 1 ];
		sprintf( pszFullFileName, "%s", pFIleName);
	}
	hr = AVIFileOpen(&m_pAviFile, pszFullFileName, OF_WRITE | OF_CREATE, NULL);

	delete [] pszFullFileName;
	if (hr != AVIERR_OK) 
	{
		switch(hr)
		{
			case AVIERR_BADFORMAT:
				AfxMessageBox("Error AVIERR_BADFORMAT");
				break;
			case AVIERR_MEMORY:
				AfxMessageBox("Error AVIERR_MEMORY");
				break;
			case AVIERR_FILEREAD:
				AfxMessageBox("Error AVIERR_FILEREAD");
				break;
			case AVIERR_FILEOPEN:
				AfxMessageBox("Error AVIERR_FILEOPEN");
				break;
			case REGDB_E_CLASSNOTREG:
				AfxMessageBox("Error REGDB_E_CLASSNOTREG");
				break;
			default:
				AfxMessageBox("Error Unknown");
				break;
		}
		return false;
	}	
	return true;
}

bool CAVIWriter::Close()
{
	if(m_pAviFile)
	{
		AVIFileRelease(m_pAviFile);
		m_pAviFile = NULL;
	}
	return true;
}

bool CAVIWriter::InitFileReader(char *pcaRawFileName)
{   
	//Steve0831
	if(!m_pcFileReader)
	{
		ReleaseFileReader();
	}

	if(NULL == pcaRawFileName)
	{
		return false;
	}

	if(m_sAVIFileName.IsEmpty())
	{
		return false;
	}

	if(NULL == m_pFrame)
		m_pFrame = new unsigned char[2073600];
	memset(m_pFrame, 0x00, 2073600);
	m_dwFullFrameLen = 0;
	m_pcFileReader = new CMpeg4Reader();
	memset(&m_DataInfo, 0x00, sizeof(DataInfo_t));

	MEDIA_CONNECTION_CONFIG mcc;
	memset(&mcc, 0x00, sizeof(MEDIA_CONNECTION_CONFIG));
	strcpy(mcc.PlayFileName, pcaRawFileName);
	mcc.ContactType = CONTACT_TYPE_PLAYBACK;

	m_pcFileReader->SetMediaConfig(&mcc);
	if(!m_pcFileReader->OpenFile())
	{
		ReleaseFileReader();
		return false;
	}

	if(!m_pcFileReader->GetFirstIFrame(m_pFrame, m_DataInfo))
	{
		ReleaseFileReader();
		return false;
	}
	m_pcFileReader->SetFirstIPos();

	m_dwFullFrameLen = m_DataInfo.dw20DataLength + TCP1020_HEADER_SIZE;

	TCP20VideoHeader_t TCP20Header;
	memset(&TCP20Header, 0x00, sizeof(TCP20VideoHeader_t));
	memcpy(&TCP20Header, m_pFrame+TCP1020_HEADER_SIZE, sizeof(TCP20VideoHeader_t));

	m_iFPS = TCP20Header.bFpsNumber;
	m_iResolution = TCP20Header.bResolution;
	GetVideoSize(m_iResolution, m_iWidth, m_iHeight);



    return true;
}

bool CAVIWriter::ReleaseFileReader()
{
	if(m_pFrame)
	{
		delete [] m_pFrame;
		m_pFrame = NULL;
	}
	m_dwFullFrameLen = 0;

	if(m_pcFileReader)
	{
		delete m_pcFileReader;
		m_pcFileReader = NULL;
	}
	return true;
}
//Check Period Of Time: input beginTime,EndTime ; output: return true or off//
bool  CAVIWriter::CheckPeriodOfTime( DWORD dwTime  )
{
	if( m_tSpecifyBeginDateTime >0 &&  m_tSpecifyEndDateTime >0 )
	{
		if( m_tSpecifyEndDateTime >= m_tSpecifyBeginDateTime )
         {
		   
			if( dwTime >= m_tSpecifyBeginDateTime && dwTime <= m_tSpecifyEndDateTime )
			{
                        return true;
			}
	     }
		return false;
	}
	return true;
}

bool CAVIWriter::IsOverSpecifyEndTime( const DWORD dwTime)
{
	if( m_tSpecifyEndDateTime > 0 )
	{
		if( dwTime > m_tSpecifyEndDateTime )
			return true;
	}

	return false;
}

bool CAVIWriter::WriteStream(int iMsgType, int iFrameType, BYTE *pbyBuffer, DWORD dwBuffLen)
{
	HRESULT hr = NULL;
	CString szMsg ;

	// dwBuffLen does not include TCP1020_HEADER_SIZE
	DWORD dwFullLen = dwBuffLen + TCP1020_HEADER_SIZE;

	if(2 == iMsgType || 3 == iMsgType) // audio
	{
		DWORD dwAudioSample;
		if(dwFullLen == 548 || dwFullLen == 652 || dwFullLen == 468) // No timestamp
		{
			dwAudioSample = dwBuffLen*8/m_sWavFormat.wBitsPerSample;
			hr = AVIStreamWrite(m_pAviAudio, m_dwAudioSampleStart, dwAudioSample,  pbyBuffer+TCP1020_HEADER_SIZE, dwBuffLen, 0, NULL, NULL);
		}
		else if(dwFullLen == 564 || dwFullLen == 668 || dwFullLen == 484) // Have timestamp
		{
			// Skip TimeCode (16)
			 dwAudioSample = (dwBuffLen-16)*8/m_sWavFormat.wBitsPerSample;
			 hr = AVIStreamWrite(m_pAviAudio, m_dwAudioSampleStart, dwAudioSample,  pbyBuffer + TCP1020_HEADER_SIZE + 16, dwBuffLen - 16, 0, NULL, NULL);
		}
		
		if (hr!=AVIERR_OK)
		{
			return false;
		}
        m_dwAudioSampleStart += dwAudioSample;
		m_dwAFrameCounter += 1;
	}
	else	// video
	{ 
		DWORD dwFlag;
		if(FRAME_TYPE_P == iFrameType)
		{
            dwFlag=0;			
			m_dwPFrameCount += 1;
			szMsg.Format("P Frame:%d\n",m_dwPFrameCount) ;
			AfxGetApp()->GetMainWnd()->PostMessage(PRINT_INFO, 1, m_dwPFrameCount);
		}
		else if (FRAME_TYPE_I == iFrameType)
		{
			dwFlag = AVIIF_KEYFRAME;
           			
			m_dwIFrameCount += 1;
			szMsg.Format("I Frame:%d\n",m_dwIFrameCount) ;
			AfxGetApp()->GetMainWnd()->PostMessage(PRINT_INFO, 2, m_dwIFrameCount);
		}
		else
		{
			return false;
		}

		// we need to decode MJPEG or H.264 , then encode it with xvid , put only xvid encoded format in AVI file
		if( m_bOSD || iMsgType != 4  )
		{
			int nEncodeDataLen = dwBuffLen-44;
			if( (iMsgType == 1 || iMsgType == 4 || iMsgType == 5 ) && m_dwIFrameCount>=1 )
			{
				if( pbyBuffer[2]==0x01 && pbyBuffer[3]==0xB2 )// 00 00 01 B2 , we have b2 header here
				{
					VIDEO_B2_FRAME * b2 = (VIDEO_B2_FRAME *)pbyBuffer;
					nEncodeDataLen = WriteOSD( b2, dwFlag, iMsgType, &pbyBuffer[44], dwFullLen-44 ) ;
				}
				else
				{
					VIDEO_B2_FRAME b2;
					memset( &b2, 0, sizeof( VIDEO_B2_FRAME));
					nEncodeDataLen = WriteOSD( &b2, dwFlag, iMsgType, pbyBuffer, dwFullLen ) ;
				}

				m_bFirstI = false;
			}

			//OutputDebugString( szMsg ) ;
			// write data to stream
			// +44 to skip B2 header
			hr = AVIStreamWrite(m_pAviVideo, m_dwVFrameCounter, 1, pbyBuffer+44, nEncodeDataLen, dwFlag, NULL, NULL);
			m_dwVFrameCounter++ ;

			if( m_pPreFrame.buf )
			{
				memcpy( m_pPreFrame.buf, pbyBuffer, nEncodeDataLen+44);
				m_pPreFrame.len = nEncodeDataLen+44;
			}
		}
		else
		{
			hr = AVIStreamWrite(m_pAviVideo, m_dwVFrameCounter, 1, pbyBuffer+44, dwBuffLen-44, dwFlag, NULL, NULL);
			m_dwVFrameCounter++ ;

			if( m_pPreFrame.buf )
			{
				memcpy( m_pPreFrame.buf, pbyBuffer, dwBuffLen);
				m_pPreFrame.len = dwBuffLen;
			}
		}
	}

	return true;
}

int CAVIWriter::WriteOSD( VIDEO_B2_FRAME * b2, DWORD dwFlag, int iMsgType, unsigned char* m_pFrame, DWORD dwBuffLen )
{	

	USEENCODER::ROE* roe = NULL;
	LPBITMAPINFO lpbmi = NULL;

	switch(iMsgType)
	{
	case 1:  
		{
			lpbmi= m_XVID_lpfnCDecode( m_XVID_nDecoderHandle, m_iWidth, m_iHeight, m_pFrame, dwBuffLen, NULL, NULL, false);
			//lpbmi= XDecode(m_nDecoder, m_iWidth, m_iHeight, m_pFrame, dwBuffLen, NULL, NULL, false);
			if( m_bFirstI )
			{
				lpbmi= m_XVID_lpfnCDecode( m_XVID_nDecoderHandle, m_iWidth, m_iHeight, m_pFrame, dwBuffLen, NULL, NULL, false);
				//lpbmi = XDecode(m_nDecoder, m_iWidth, m_iHeight, m_pFrame, dwBuffLen, NULL, NULL, false);
			}
			if( !lpbmi ) 
				lpbmi = NULL ;
		}
		break;
	case 4: 
		{
			if( m_MJPG_nDecoderHandle >= 0 )
				lpbmi = m_MJPG_lpfnCDecode(m_MJPG_nDecoderHandle, m_iWidth, m_iHeight, m_pFrame, dwBuffLen, NULL, NULL, false);
	     
		}
		break;
	case 5:
		{
			if( m_H264_nDecoderHandle >= 0 )
				lpbmi = m_H264_lpfnCDecode(m_H264_nDecoderHandle, m_iWidth, m_iHeight, m_pFrame, dwBuffLen, NULL, NULL, false);
		}
		break;
   	default:
		{
			return 0;
		}
		break;
	}

	DWORD dwLastError;
	HDC hdcScreen = GetDC( NULL);
	if(NULL == hdcScreen)
	{
		return 0;
	}

	HDC memDC = CreateCompatibleDC ( NULL);
	if(NULL == memDC)
	{
		::DeleteDC(hdcScreen);
		return 0;
	}

	HBITMAP memBM = CreateCompatibleBitmap ( hdcScreen, abs(m_iWidth), abs(m_iHeight) );
	if(NULL == memBM)
	{
		DWORD dwError = GetLastError();
		::DeleteDC(hdcScreen);
		::DeleteDC(memDC);
		return 0;
	}

	SelectObject ( memDC, memBM );
	::DeleteDC(hdcScreen);

	if( memDC )
	{
		::SetStretchBltMode( memDC, COLORONCOLOR );
		int nError = ::StretchDIBits( memDC, 0, 0, abs(m_iWidth), abs(m_iHeight), 0, 0, abs(m_iWidth), abs(m_iHeight), (const void*)&lpbmi[1], lpbmi, DIB_RGB_COLORS, SRCCOPY);
		if(GDI_ERROR == nError)
		{
			dwLastError = GetLastError();
			::DeleteDC(memDC);
			::DeleteObject(memBM);
			return 0;
		}


		if(m_bOSD)
		{

		int nWeight = FW_BOLD;
		int nSize   = 17 ;
		HFONT hFont = ::CreateFont(nSize, 0, 0, 0, nWeight, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH , "Verdana" ) ;
		::SetBkColor(memDC, RGB(255, 255, 255));
		::SetBkMode( memDC, TRANSPARENT );
		::SelectObject( memDC, hFont); 
		::SetTextColor( memDC, RGB(255, 255, 255) );



		::FillRect( memDC, CRect(0,0,abs(m_iWidth),nSize), CBrush(RGB(0, 0, 0) ) ) ;
//======Get Time Code===========================================
		if( m_nOSDTime )
		{
			CString szDateTime ;
			DWORD dwTime ;
			BYTE  bTimeZone ;
			memcpy( &dwTime,    m_pFrame-32, 4 ) ;
			memcpy( &bTimeZone, m_pFrame-28, 1 ) ;

			tm* gmt = gmtime((time_t*)&dwTime) ;

			COleDateTime ct = COleDateTime(gmt->tm_year+1900,gmt->tm_mon+1,gmt->tm_mday,gmt->tm_hour,gmt->tm_min,gmt->tm_sec) ;
			int	  iTZ  = bTimeZone-12 ;

			ct = ct + COleDateTimeSpan(0,iTZ,0,0) ;
									
			if (m_szOSDText!="")
				szDateTime = m_szOSDText + " " ;  
			else
				szDateTime = m_szOSDText ;
              
			switch ( m_nOSDTime )
			{
			case 1 :
				{
				szDateTime += ct.Format("%Y/%m/%d %H:%M:%S") ;
				}
				break ;
			case 2 :
				szDateTime += ct.Format("%m/%d/%y %H:%M:%S") ;
				break ;
			case 3 :
				szDateTime += ct.Format("%d/%m/%y %H:%M:%S") ;
				break ;
			}

			::TextOut( memDC, 5,  0, szDateTime, szDateTime.GetLength());

#ifdef _DEBUG
			{
				char token[256];
				sprintf( token, "%03u.%06u %08u %d",
					(b2->prdata.timestamp.tv_sec%1000)*1000,
					b2->prdata.timestamp.tv_usec,
					b2->prdata.count,
					(dwFlag==AVIIF_KEYFRAME)?1:0 );
				::TextOut( memDC, 5,  24, token, strlen( token));
			}
#endif
		}

		else
			::TextOut( memDC, 5,  0, m_szOSDText, m_szOSDText.GetLength());
//=============================================================
		

		DeleteObject(hFont);
	}

#if 0
		BYTE* pbbmi = NULL;
		PBITMAPINFO pbmi = NULL; 
		{
			BITMAP bmp;
			WORD    cClrBits; 
			DWORD dwbmiSize = 0;

			if (!GetObject(memBM, sizeof(BITMAP), (LPSTR)&bmp)) 
			{
				::DeleteDC(memDC);
				::DeleteObject(memBM);
				return 0;
			}

			cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
			if (cClrBits == 1) 
				cClrBits = 1; 
			else if (cClrBits <= 4) 
				cClrBits = 4; 
			else if (cClrBits <= 8) 
				cClrBits = 8; 
			else if (cClrBits <= 16) 
				cClrBits = 16; 
			else if (cClrBits <= 24) 
				cClrBits = 24; 
			else 
				cClrBits = 32; 

			if (cClrBits != 24) 
			{
				pbbmi = new BYTE[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1<< cClrBits)];
				pbmi = (PBITMAPINFO)pbbmi;
				dwbmiSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1<< cClrBits);
			}
			else
			{
				pbbmi = new BYTE[sizeof(BITMAPINFOHEADER)];
				pbmi = (PBITMAPINFO)pbbmi;
				dwbmiSize = sizeof(BITMAPINFOHEADER);
			}

			pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
			pbmi->bmiHeader.biWidth = bmp.bmWidth; 
			pbmi->bmiHeader.biHeight = bmp.bmHeight; 
			pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
			pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 

			if (cClrBits < 24) 
				pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 

			pbmi->bmiHeader.biCompression = BI_RGB; 
			pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8 * pbmi->bmiHeader.biHeight; 
			pbmi->bmiHeader.biClrImportant = 0; 

			DWORD m_dwImageSize = pbmi->bmiHeader.biSizeImage;
			//DWORD m_dwImageSize = ((pbmi->bmiHeader.biWidth * 32 +31) & ~31) /8 * pbmi->bmiHeader.biHeight; 
		}

#endif


		LPBYTE p32BMP = new BYTE[ sizeof(BITMAPINFO) + lpbmi->bmiHeader.biSizeImage];
		LPBYTE p32Buff = p32BMP + sizeof(BITMAPINFO) ;
		LPBITMAPINFO p32Bmi = (LPBITMAPINFO)p32BMP ;
		{
			memset(p32BMP, 0x00, sizeof(BITMAPINFO) + lpbmi->bmiHeader.biSizeImage);
			memcpy( p32Bmi, lpbmi, sizeof(BITMAPINFO));
		}

		p32Bmi->bmiHeader.biHeight = abs(p32Bmi->bmiHeader.biHeight);
		if( !GetDIBits( memDC, memBM, 0, (WORD)abs(p32Bmi->bmiHeader.biHeight), p32Buff, p32Bmi, DIB_RGB_COLORS))
		{
			::DeleteDC(memDC);
			::DeleteObject(memBM);
			return 0;
		}
#if 0
		if(pbmi)
			pbmi = NULL;
		if( pbbmi )
			delete [] pbbmi;
		pbbmi = NULL;
#endif
//=================================================================
//======Convert 32 bit BITMAP to 24 bit BITMAP=====================
		//m_dwImageSize = pbmi->bmiHeader.biWidth * pbmi->bmiHeader.biHeight * 3;
		//LPBYTE			p24BMP =  new BYTE[sizeof(BITMAPINFO) + m_dwImageSize ];  //m_dwImageSize
		//LPBYTE			p24Buff = p24BMP + sizeof(BITMAPINFO) ;
		//LPBITMAPINFO	p24Bmi = (LPBITMAPINFO)p24BMP ;

		//memset( p24BMP, 0x00, sizeof(BITMAPINFO) + m_dwImageSize );         //m_dwImageSize

		//p24Bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
		//p24Bmi->bmiHeader.biWidth = p32Bmi->bmiHeader.biWidth;
		//p24Bmi->bmiHeader.biHeight = -1*p32Bmi->bmiHeader.biHeight;
		//p24Bmi->bmiHeader.biPlanes = p32Bmi->bmiHeader.biPlanes;
		//p24Bmi->bmiHeader.biBitCount = 24;
		//p24Bmi->bmiHeader.biCompression = BI_RGB; 
		//p24Bmi->bmiHeader.biSizeImage = p24Bmi->bmiHeader.biWidth * p24Bmi->bmiHeader.biHeight * 3; 
		//p24Bmi->bmiHeader.biClrImportant = 0;

		int dw32WBytes = p32Bmi->bmiHeader.biWidth*p32Bmi->bmiHeader.biBitCount/8 ;
//		int dw24WBytes = p24Bmi->bmiHeader.biWidth*p24Bmi->bmiHeader.biBitCount/8 ;

		LPBYTE lpbyBuffer = new BYTE[dw32WBytes+1];
		int nIdx = 0 ;
  
		int h=0;
		for( int rh=p32Bmi->bmiHeader.biHeight-1; rh>(p32Bmi->bmiHeader.biHeight/2) ; rh--, h++)
		{
			memcpy( lpbyBuffer, p32Buff+(rh*dw32WBytes), dw32WBytes ) ;
			memcpy( p32Buff+(rh*dw32WBytes), p32Buff+(h*dw32WBytes), dw32WBytes ) ;
			memcpy( p32Buff+(h*dw32WBytes), lpbyBuffer, dw32WBytes ) ;
		}
		delete [] lpbyBuffer;


		roe = USEENCODER::XEncode( m_hEncoder, m_iWidth, m_iHeight, m_iFPS, 3000000,
			p32Buff, p32Bmi->bmiHeader.biSizeImage, false );
		//roe = USEENCODER::XEncode( m_hEncoder, m_iWidth, m_iHeight, m_iFPS, 3000000,
		//	p24Buff, p24Bmi->bmiHeader.biSizeImage, false );
		memcpy( m_pFrame, roe->pBuffer, roe->nBufferLen ) ;

		delete [] p32BMP ;
		//delete [] p24BMP ;
	}

	::DeleteDC(memDC);
	::DeleteObject(memBM);

	if( roe )
		return roe->nBufferLen;
	return 1 ;
}

DWORD CAVIWriter::CheckIorP( char* pRawData, int Datalen )
{
	int i = 0;
	char cIorP;
	DWORD dwB6 = 0xB6010000;
	do 
	{
		if( memcmp( &pRawData[i], &dwB6, 4 ) == 0 )
		{
			cIorP = pRawData[i+4];
			cIorP &= 0xC0;
			if( cIorP == 64 )	// PFrame
				return FRAME_TYPE_P;
			if( cIorP == 0 )	// IFrame
				return FRAME_TYPE_I;
		}

	} while( Datalen > i++ );
	return FRAME_TYPE_UNKNOW;
}

bool CAVIWriter::CreateAVIFile(char *pcaAVIFileName, char *pcaRawFileName, char * pszAVIFilePath, 
							   const time_t tSpecifyBeginDateTime, const time_t tSpecifyEndDateTime )
{
	bool bResult = false;
	m_bFirstI = true;

	if( !Open( pcaAVIFileName, pszAVIFilePath) )
		return false;

	if( InitFileReader( pcaRawFileName) && CreateVideoStream() && CreateAudioStream() )
	{   
		m_pcFileReader->GetRawTail( m_TailInfo);
		m_dwFileBeginTime  = m_TailInfo.dwBeginTime; 
		m_dwFileEndTime    = m_TailInfo.dwEndTime;

		{
			m_tSpecifyBeginDateTime = tSpecifyBeginDateTime;
			m_tSpecifyEndDateTime = tSpecifyEndDateTime;
		}
		//Decoder  
		//m_nDecoder = XCInit() ;
		ReInitAllCodec();
		m_hEncoder = USEENCODER::XEInit() ;

		bool bAudioInTimeRage = false;

		bool bError = false;
		time_t dwTime = 0;
		
		BOOL bHasFirstIFrame = FALSE;
        //GetNextFrame
		while( m_bProcessing && !bHasFirstIFrame && m_pcFileReader->GetNextFrame( m_pFrame, m_DataInfo))
		{
			if( m_DataInfo.dwFrameType != 1 )
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
						//dwTime = b2->prdata.timestamp.tv_usec;
						dwTime = b2->prdata.date_time;// + (b2->prdata.time_zone-12)*60*60;
					}
					break;
				}
			}

			if( IsOverSpecifyEndTime( dwTime) )
				break;

			if( CheckPeriodOfTime( dwTime) )
			{
				bHasFirstIFrame = TRUE; 

				m_dwFullFrameLen = m_DataInfo.dw20DataLength  + TCP1020_HEADER_SIZE;
				if(!WriteStream(m_DataInfo.dwMSGType, m_DataInfo.dwFrameType, m_pFrame,m_DataInfo.dw20DataLength))
				{
					CloseAll();
					return false;
				}

				BOOL bInTime = FALSE;
				memcpy( &m_PreB2, (LPVOID)m_pFrame, sizeof(VIDEO_B2_FRAME));
				while( m_bProcessing && m_pcFileReader->GetNextFrame( m_pFrame, m_DataInfo)  )
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
								//dwTime = b2->prdata.timestamp.tv_usec;
								dwTime = b2->prdata.date_time;// + (b2->prdata.time_zone-12)*60*60;
								bAudioInTimeRage = false;
								if( CheckPeriodOfTime( dwTime) == false )
								{
									bInTime = FALSE;
								}
								else
								{
									bInTime = TRUE;
								}
							}
							break;
						}
					}

					if( IsOverSpecifyEndTime( dwTime) )
						break;

					if( bInTime )
					{
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
									// ¸É
									{
										timeval & time1 = m_PreB2.prdata.timestamp;
										timeval & time2 = b2->prdata.timestamp;
										int nDiffTime = (time2.tv_sec-time1.tv_sec)*1000 + 
											(time2.tv_usec - time1.tv_usec)/1000;
										int nDefDiff = (1000)/m_iFPS;

#if 0
										m_dAvgFrameDiffTime += dwDiffTime;
										m_dwAvgFrameDiffTimeValue++;
										if( (m_dAvgFrameDiffTime/m_dwAvgFrameDiffTimeValue) > ((double)dwDefDiff*1.15) )
#endif

#if 1
										static int g_nReFrameCount = 0;
										m_nDiffTimeSum += nDiffTime-nDefDiff;
										m_dwDiffTimeSumCount++;
#endif
										//if( dwDiffTime > dwDefDiff )
										//Steve0912
										if( m_nDiffTimeSum > (int)nDefDiff*1.15 && m_dwDiffTimeSumCount > 3 )
										{
											int nReFrames = (m_nDiffTimeSum-nDefDiff)/nDefDiff;
											g_nReFrameCount += nReFrames;
											for( int i=0; i<nReFrames; i++)
											{
												HRESULT hr = AVIStreamWrite( m_pAviVideo, m_dwVFrameCounter, 1,
													m_pPreFrame.buf+44, m_pPreFrame.len-44, AVIIF_KEYFRAME, NULL, NULL);
												m_dwVFrameCounter++ ;
											}

											m_nDiffTimeSum -= nReFrames*nDefDiff;
										}
									}
									memcpy( &m_PreB2, (LPVOID)m_pFrame, sizeof(VIDEO_B2_FRAME));
								}
								break;
							}
						}

						m_dwFullFrameLen = m_DataInfo.dw20DataLength  + TCP1020_HEADER_SIZE;
						if(!WriteStream(m_DataInfo.dwMSGType, m_DataInfo.dwFrameType, m_pFrame, m_DataInfo.dw20DataLength))
						{
							CloseAll();
							return false;
						}
						bAudioInTimeRage = true;
					}
					//else
					//{
					//	bAudioInTimeRage = false;
					//}

				}
					
				break;
			}
		}
	}

	{
		//if( m_nDecoder != -1 )
		//	XCExit( m_nDecoder ) ;
		//m_nDecoder = -1;

		if( m_hEncoder )
			USEENCODER::XEExit( m_hEncoder );
			m_hEncoder = NULL;

			CloseAll();
	}
	return bResult;
}



void CAVIWriter::CloseAll()
{
	m_bProcessing = false;
	DestroyAudioStream();
	DestroyVideoStream();
	ReleaseFileReader();
	Close();
	//Steve
	if(!m_bConvertFolder)
	{
		AfxGetApp()->GetMainWnd()->PostMessage(FILE_CLOSE, 0, 0);
	}
}
//DWORD& dwIFrame, DWORD& dwPFrame, DWORD& dwAFrame
void CAVIWriter::GetConvertInfo(DWORD& m_nIFrame, DWORD& m_nPFrame, DWORD& m_nAFrame)
{
	m_nIFrame = m_dwIFrameCount;
	m_nPFrame = m_dwPFrameCount;
	m_nAFrame = m_dwAFrameCounter;
	
	
}

bool CAVIWriter::GetResolutionAndFPSBySequenceHeader( unsigned char* ucSEQ, int nSEQLen, int& nWidth, int& nHeight, int& nFPS )
{
	CMemoryBitstream* oBM;

	oBM = new CMemoryBitstream();
	int vop_time_increment_resolution;
	int vop_time_increment;
	int width;
	int height;
	int bitlen;
	int i;

	if( ucSEQ && nSEQLen == 21 )
	{
		oBM->AllocBytes(21);
		oBM->PutBytes( (unsigned char*)ucSEQ, 21);

		oBM->SetBitPosition( 65 );
		int VideoObjectTypeIndication = oBM->GetBits(8);
		if( VideoObjectTypeIndication == 0 )
		{
			oBM->SetBitPosition( 86 );

			i = oBM->GetBits( 16 );
			vop_time_increment_resolution = i;

			for( bitlen = 0; i != 0; i >>= 1 ) 
				++bitlen;

			oBM->SetBitPosition(86+16+1);

			if( oBM->GetBits(1) )
			{ 
				//Fixed VOP rate
				oBM->SetBitPosition( 86 + 16 + 2 );
				vop_time_increment = oBM->GetBits( bitlen );

				oBM->SetBitPosition( 86 + 16 + 2 + bitlen + 1 );
				width = oBM->GetBits( 13 );

				oBM->SetBitPosition( 86 + 16 + 2 + bitlen + 1 + 13 + 1 );
				height = oBM->GetBits( 13 );

				nWidth = width;
				nHeight = height;
				nFPS = ( vop_time_increment_resolution / vop_time_increment );
				if( oBM ) delete oBM;
				oBM = NULL;
				return true;
			}
		}

		else
			if( VideoObjectTypeIndication == 1 )
			{
				oBM->SetBitPosition( 89 );
				i = oBM->GetBits( 16 );
				vop_time_increment_resolution = i;
				for( bitlen = 0; i != 0; i >>= 1 ) 
					++bitlen;

				oBM->SetBitPosition(89+16+1);

				if( oBM->GetBits(1) )
				{ 
					//Fixed VOP rate
					oBM->SetBitPosition( 89 + 16 + 2 );
					vop_time_increment = oBM->GetBits( bitlen );
					oBM->SetBitPosition( 89 + 16 + 2 + bitlen + 1 );
					width = oBM->GetBits( 13 );

					oBM->SetBitPosition( 89 + 16 + 2 + bitlen + 1 + 13 + 1 );
					height = oBM->GetBits( 13 );
					nWidth = width;
					nHeight = height;
					if( vop_time_increment == 1001 )
						vop_time_increment = 1000;
					nFPS = ( vop_time_increment_resolution / vop_time_increment );
					//if( m_nTCPType == 1 ) nFPS++;
					if( oBM ) delete oBM;
					oBM = NULL;
					return true;
				}
			}
	}

	if( oBM ) delete oBM;
	oBM = NULL;
	return false;
}

int CAVIWriter::GetSequenceHeader( BYTE* pRaw, int nRawLen, BYTE* pSEQ )
{
	DWORD dw00 = 0x00010000;
	DWORD dw20 = 0x20010000;

	int iSeqPlace = 0;
	int tLen = nRawLen;
	do 
	{
		if( memcmp( &pRaw[iSeqPlace], &dw00, 4 ) == 0 && memcmp( &pRaw[iSeqPlace+4], &dw20, 4 ) == 0 )
		{
			memcpy( pSEQ, &pRaw[iSeqPlace], 21 );
			return 21;
		}
	} while( tLen > iSeqPlace++ );
	return 0;
}
