#include "StdAfx.h"
//#include ".\jpegwriter.h"
#include "JPEGWriter.h"
//#include "XVIDCODEC.h"
#include "MemoryBitstream.h"
#include "JpegEncoder.h"
#define USEENCODER JPEGENCODER




CJPEGWriter::CJPEGWriter( const bool bOSD, const char * pszTitleText, const int nOSDTimeType )
   : CXEncoder()
	
	, m_pcFileReader(NULL)
	, m_bProcessing(false)
	, m_pFrame(NULL)
	, m_bConvertFolder(false)
	, m_nMP4ConvertFormat(1)
	, m_hEncoder( NULL)
//	, m_nDecoder( -1)
	, m_bOSD( bOSD)
	, m_szOSDText( pszTitleText)
	, m_nOSDTime( nOSDTimeType )
	, m_tSpecifyBeginDateTime(0)
	, m_tSpecifyEndDateTime(0)
{

}

CJPEGWriter::~CJPEGWriter(void)
{
	CoUninitialize();
}

void CJPEGWriter::SetJPGOSD( const bool bOSD, const char * pszTitleText, const int nOSDTimeType )
{
	m_bOSD = bOSD;
	m_szOSDText = pszTitleText;
	m_nOSDTime = nOSDTimeType;
}

void CJPEGWriter::SetJPGOSD( const bool bOSD )
{
	m_bOSD = bOSD;
}

void CJPEGWriter::SetJPGOSDText( const char * pszTitleText)
{
	m_szOSDText = pszTitleText;
}

void CJPEGWriter::SetJPGOSDTimeType( const int nOSDTimeType )
{
	m_nOSDTime = nOSDTimeType;
}

int CJPEGWriter::GetVideoRate(int iTVStd, int iFPS)
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

void CJPEGWriter::GetVideoSize(int iResolution, int& iWidth, int& iHeight)
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
		case 1:	// NTSC CIF
			{
				iWidth = 352;	
				iHeight = 240;
				m_iTVStd = NET_TVNTSC;
			}
			break;
		case 2:	// NTSC QCIF
			{
				iWidth = 160;	
				iHeight = 112;
				m_iTVStd = NET_TVNTSC;
			}
			break;
        case 70:	// NTSC CIF
			{
				iWidth = 320;	
				iHeight = 240;
				m_iTVStd = NET_TVNTSC;
			}
			break;
		case 71:	// NTSC QCIF
			{
				iWidth = 160;	
				iHeight = 120;
				m_iTVStd = NET_TVNTSC;
			}
			break;
       
		case 3 :	// PAL D1
			{
				iWidth = 720;	
				iHeight = 576;
				m_iTVStd = NET_TVPAL;
			}
			break;
		case 4 :	// PAL CIF
			{
				iWidth = 352;	
				iHeight = 288;
				m_iTVStd = NET_TVPAL;
			}
			break;
		case 5:	// PAL QCIF
			{
				iWidth = 176;	
				iHeight = 144;
				m_iTVStd = NET_TVPAL;
			}
			break;
		case 6: // NTSC QCIF
			{
				iWidth = 176;	
				iHeight = 120;
				m_iTVStd = NET_TVNTSC;
			}
			break;
		case 64: // VGA
			{
				iWidth = 640;	
				iHeight = 480;
				m_iTVStd = NET_TVNTSC;
			}
			break;
		case 65: // Mega Pixel
			{
				iWidth = 1280;	
				iHeight = 720;
				m_iTVStd = NET_TVNTSC;
			}
			break;
		case 66: // Mega Pixel
			{
				iWidth = 1280;	
				iHeight = 960;
				m_iTVStd = NET_TVNTSC;
			}
			break;
		case 67: // Mega Pixel
			{
				iWidth = 1280;	
				iHeight = 1024;
				m_iTVStd = NET_TVNTSC;
			}
			break;
		case 68: // Mega Pixel
			{
				iWidth = 1920;	
				iHeight = 1080;
				m_iTVStd = NET_TVNTSC;
			}
			break;
		case 192: // VGA
			{
				iWidth = 640;	
				iHeight = 480;
				m_iTVStd = NET_TVPAL;
			}
			break;
		default :				// NTSC D1
			{
				iWidth = 720;	
				iHeight = 480;
				m_iTVStd = NET_TVNTSC;
			}
			break;
	}
}

bool CJPEGWriter::GetResolutionAndTVStander(int iWidth, int iHeight )
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
			else if(960 == iHeight)
			{
				m_iTVStd = NET_TVNTSC;
				m_iResolution = 66;
				return true;				
			}
			else if(1024 == iHeight)
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
			else if(576 == iHeight)
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
			else if(288 == iHeight)
			{
				m_iTVStd = NET_TVPAL;
				m_iResolution = 4;
				return true;
			}
		}
    
		break;
	case 160:
		{
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
			else if(120 == iHeight)
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
    case 320:
		{
			if(240 == iHeight)
			{
				m_iTVStd = NET_TVNTSC;
				m_iResolution = 70;
				return true;
			}
		}
        
	default:
		break;
	}
	return false;
}


void CJPEGWriter::GetTimeToStr(time_t time, char *buf )
{
	struct tm *newtime;
	if(0 > time)
	{
		buf[0] = '\0';
		return;
	}
	newtime = gmtime((time_t *)&time);
	sprintf( buf, "%04d/%02d/%02d %02d:%02d:%02d\0", newtime->tm_year+1900, newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec );
	/*tt = localtime( &time );
	strcpy(buf, asctime(tt));*/
	//strftime( buf, 64, "%Y/%m/%d %H:%M:%S\0", asctime(tt) ); 
}

bool CJPEGWriter::Open(char *pFIleName,char * pPath )
{
	HRESULT hr = 0 ;

	m_iBitCount = 24;
	m_iWidth = 0;
	m_iHeight = 0;
	m_iFPS = 0;
	m_iTVStd = 0;
	m_dwVFrameCounter = 0;
	m_dwAFrameCounter = 0;

	m_bProcessing = true;
	m_dwAudioSampleStart = 0;
	m_dwIFrameCount = 0;
	m_dwPFrameCount = 0;

   
	
	return true;  
} 

bool CJPEGWriter::Close()
{
	
	return true;
}

bool CJPEGWriter::InitFileReader(char *pcaRawFileName)
{
	if(m_pcFileReader)
	{
		ReleaseFileReader();
	}

	if(NULL == pcaRawFileName)
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

bool CJPEGWriter::ReleaseFileReader()
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
bool  CJPEGWriter::CheckPeriodOfTime( DWORD dwTime  )
{
	if( m_tSpecifyBeginDateTime>0 &&  m_tSpecifyEndDateTime>0 )
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

DWORD CJPEGWriter::ConvertVideoFrame( int iMsgType, int iFrameType, BYTE *pbyBuffer, DWORD dwBuffLen)
{
	HRESULT hr = NULL;
	CString szMsg ;

	if(FRAME_TYPE_P == iFrameType)
	{
		m_dwPFrameCount += 1;
		szMsg.Format("P Frame:%d\n",m_dwPFrameCount) ;
		AfxGetApp()->GetMainWnd()->PostMessage(PRINT_INFO, 1, m_dwPFrameCount);
	}
	else if (FRAME_TYPE_I == iFrameType)
	{
		m_dwIFrameCount += 1;
		szMsg.Format("I Frame:%d\n",m_dwIFrameCount) ;
		AfxGetApp()->GetMainWnd()->PostMessage(PRINT_INFO, 2, m_dwIFrameCount);
	}
	else
	{
		return 0;
	}
	// we need to decode MJPEG or H.264 , then encode it with xvid , put only xvid encoded format in AVI file
	//Steve
	//if( m_bOSD || iMsgType != 4 )
	{
		m_dwVFrameCounter++ ;
		int  dwDataLen;
		if( m_dwIFrameCount >= 1 )
		{
			dwDataLen = ConvertToJPG( iMsgType, pbyBuffer,dwBuffLen  );
			m_bFirstI = false;
			return dwDataLen;
		}
		else
		{
			return 0;
		}   
	}   
	return 0;
}


int CJPEGWriter::ConvertToJPG(int iMsgType, unsigned char* pFrame, DWORD dwBuffLen )
{	
	USEENCODER::ROE* roe = NULL;
	LPBITMAPINFO lpbmi = NULL;

	switch(iMsgType)
	{
	case 1:  // MPEG4
		{
			lpbmi = m_XVID_lpfnCDecode( m_XVID_nDecoderHandle, m_iWidth, m_iHeight, pFrame, dwBuffLen, NULL, NULL, false);
			//lpbmi = XDecode(m_nDecoder, m_iWidth, m_iHeight, pFrame, dwBuffLen, NULL, NULL, false);
			if( m_bFirstI )
			{
				lpbmi = m_XVID_lpfnCDecode( m_XVID_nDecoderHandle, m_iWidth, m_iHeight, pFrame, dwBuffLen, NULL, NULL, false);
				//lpbmi = XDecode(m_nDecoder, m_iWidth, m_iHeight, pFrame, dwBuffLen, NULL, NULL, false);
			}
			if( !lpbmi ) 
				lpbmi = NULL ;
		}
		break;
	case 4:  // MJPEG
		{
			if( m_MJPG_nDecoderHandle >= 0 )
				lpbmi = m_MJPG_lpfnCDecode(m_MJPG_nDecoderHandle, m_iWidth, m_iHeight, pFrame, dwBuffLen, NULL, NULL, false);
		  
		}
		break;
	case 5:  //H.264
		{
			if( m_H264_nDecoderHandle >= 0 )
				lpbmi = m_H264_lpfnCDecode(m_H264_nDecoderHandle, m_iWidth, m_iHeight, pFrame, dwBuffLen, NULL, NULL, false);
		}
		break;
	default:
		{
			return 0;
		}
		break;
	}



	DWORD dwLastError;
	HDC hdcScreen = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL); 
	if(NULL == hdcScreen)
	{
		return 0;
	}
	HDC memDC = CreateCompatibleDC (hdcScreen);
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


		if( m_bOSD )
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
				memcpy( &dwTime,    pFrame-32, 4 ) ;
				memcpy( &bTimeZone, pFrame-28, 1 ) ;

				tm* gmt = gmtime((time_t*)&dwTime) ;

				COleDateTime ct = COleDateTime(gmt->tm_year+1900,gmt->tm_mon+1,gmt->tm_mday,gmt->tm_hour,gmt->tm_min,gmt->tm_sec) ;
				int	  iTZ  = bTimeZone-12 ;

				ct = ct + COleDateTimeSpan(0,iTZ,0,0) ;
				if (m_szOSDText!="")
				{
                //"";// m_szOSDText + " " ;
                    szDateTime =m_szOSDText + " " ; 
				}else 
				{
					// m_szOSDText ;
					szDateTime = m_szOSDText ;
				}
			
				switch ( m_nOSDTime )
				{
				case 1 :
					szDateTime += ct.Format("%Y/%m/%d %H:%M:%S") ;
					break ;
				case 2 :
					szDateTime += ct.Format("%m/%d/%y %H:%M:%S") ;
					break ;
				case 3 :
					szDateTime += ct.Format("%d/%m/%y %H:%M:%S") ;
					break ;
				}

				::TextOut( memDC, 5,  0, szDateTime, szDateTime.GetLength());
			}

			else
				::TextOut( memDC, 5,  0, m_szOSDText, m_szOSDText.GetLength());
		
//=============================================================
			DeleteObject(hFont);
		}

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
		
		int dw32WBytes = p32Bmi->bmiHeader.biWidth*p32Bmi->bmiHeader.biBitCount/8 ;
		
		LPBYTE lpbyBuffer = new BYTE[dw32WBytes+1];
		int nIdx = 0 ;
		int h=0;
		for( int rh=(p32Bmi->bmiHeader.biHeight-1) ; rh>(p32Bmi->bmiHeader.biHeight/2) ; rh--, h++)
		{
			memcpy( lpbyBuffer, p32Buff+(rh*dw32WBytes), dw32WBytes ) ;
			memcpy( p32Buff+(rh*dw32WBytes), p32Buff+(h*dw32WBytes), dw32WBytes ) ;
			memcpy( p32Buff+(h*dw32WBytes), lpbyBuffer, dw32WBytes ) ;
		}
		delete [] lpbyBuffer;

		roe = USEENCODER::XEncode( m_hEncoder, m_iWidth, m_iHeight, m_iFPS, 3000000,
			p32Buff, p32Bmi->bmiHeader.biSizeImage, false );

		memcpy( pFrame, roe->pBuffer, roe->nBufferLen ) ;
		delete [] p32BMP ;
	}
	
		
	::DeleteDC(memDC);
	::DeleteObject(memBM);

	if( roe )
		return roe->nBufferLen;
	return 1 ;
}

bool CJPEGWriter::WriteOutputFile( const char * pszJpegFileName, DWORD dwFrameType, VIDEO_B2_FRAME * b2, BYTE * pData, DWORD dwDataLen, char * pszjFilePath )
{  
	char szfullfilename[512];
	memset( szfullfilename,0, sizeof(szfullfilename));   //szfullfilename, 0, sizeof(szfullfilename)
	if( b2 )
	{
//static DWORD gPreDateTime = 0;
static WORD gFrameSeq = 0;

		//if( gPreDateTime != b2->prdata.date_time )
		//{
		//	gPreDateTime = b2->prdata.date_time;
		//	gFrameSeq = 0;
		//}
		if( dwFrameType == 1 ) // I Frame
		{
			gFrameSeq = 0;
		}

		//localtime
		{
			TCHAR szTime[24] = {0};

			DWORD dwUTCTime = b2->prdata.date_time + (b2->prdata.time_zone-12)*60*60;

			struct tm *pTmTime = gmtime( (time_t*)&dwUTCTime );
			_tcsftime(szTime, sizeof(szTime), _T("%Y%m%d_%H%M%S"), pTmTime );

			if( pszjFilePath )
			{
				sprintf( szfullfilename,"%s\\%s_%d_%d_%d_%d.jpg\0",
					pszjFilePath,
					pszJpegFileName,
					b2->header.stream_id,
					//szTime,
					(b2->prdata.timestamp.tv_sec%1000)*1000+b2->prdata.timestamp.tv_usec/1000,
					gFrameSeq++,
					b2->prdata.count);
				//b2->prdata.timestamp.tv_usec/1000);
			}
			else
			{
				sprintf( szfullfilename,"%s_%d_%d_%d_%d.jpg\0",
					pszJpegFileName,
					b2->header.stream_id,
					//szTime,
					(b2->prdata.timestamp.tv_sec%1000)*1000+b2->prdata.timestamp.tv_usec/1000,
					gFrameSeq++,
					b2->prdata.count);
					//b2->prdata.timestamp.tv_usec/1000);
			}
		}
	}
	else
	{
		return false;
	}

	FILE *fp = fopen( szfullfilename, "wb+");
	if( fp )
	{
		BYTE * pframe = pData;
		DWORD dwframelen = dwDataLen;
		fwrite( pframe, sizeof(BYTE ), dwframelen, fp);
		fclose( fp);
	}
	else
	{
		return FALSE;
	}

	
	//{
	//	char * pszMsg = new char[256];
	//	sprintf( pszMsg, "%s", szfullfilename );
	//	AfxGetApp()->GetMainWnd()->PostMessage( PRINT_INFO, 5, (DWORD)pszMsg);
	//}
	return TRUE;
}

DWORD CJPEGWriter::ConvertFrame(  char *pszFileName, DWORD dwMsgType, DWORD dwFrameType, BYTE * pFrame, DWORD dwFullFrameLen,char * pszjFilePath )
{
	BYTE * pData = NULL;
	DWORD dwDataLen = 0;

	if( !pFrame )
	{
		return 0;
	}

	switch( dwMsgType )
	{
	case 1: // video
	case 4:
	case 5:
		{
			
			VIDEO_B2_FRAME * b2 = NULL;
			if( pFrame[2]==0x01 && pFrame[3]==0xB2 )// 00 00 01 B2 , we have b2 header here
			{
				b2 = (VIDEO_B2_FRAME *)pFrame;
				pData = pFrame + 44;
				dwDataLen = dwFullFrameLen - 44;
			}
			else
			{
				pData = pFrame;
				dwDataLen = dwFullFrameLen;
			}
			if( m_bOSD || dwMsgType != 4 )
			{
				dwDataLen = ConvertVideoFrame( dwMsgType, dwFrameType, pData, dwDataLen);
				if( dwDataLen > 0)
				{
					WriteOutputFile( pszFileName, dwFrameType, b2, pData, dwDataLen, pszjFilePath);
				}
			}
			else
			{
				// MJPEG and not need to write osd
				// Camera MJPEG can't write to jpeg file
				if( dwDataLen > 0)
				{
					CString szMsg ;
					m_dwIFrameCount ++;
					szMsg.Format("I Frame:%d\n",m_dwIFrameCount) ;
					AfxGetApp()->GetMainWnd()->PostMessage(PRINT_INFO, 2, m_dwIFrameCount);
					WriteOutputFile( pszFileName, dwFrameType, b2, pData, dwDataLen, pszjFilePath);
				}
			}
		}
	default:
		break;
	}
	return dwDataLen;
}

bool CJPEGWriter::ConvertMultiJPGFile( char * pszFileName,  char *pcaRawFileName, char * pszjFilePath,
									  const time_t tSpecifyBeginDateTime, const time_t tSpecifyEndDateTime )
{
	bool bResult = false;
	m_bFirstI = true;

  
	if( !Open( pszFileName , pszjFilePath) )
		return false;

	DWORD dwDataLen = 0;
	if( InitFileReader( pcaRawFileName) )
	{
		m_pcFileReader->GetRawTail( m_TailInfo);
		m_dwFileBeginTime  = m_TailInfo.dwBeginTime; 
		m_dwFileEndTime    = m_TailInfo.dwEndTime;
			
		//m_nDecoder = XCInit() ;
				
		ReInitAllCodec();
		m_hEncoder = USEENCODER::XEInit() ;

		{
			m_tSpecifyBeginDateTime = tSpecifyBeginDateTime;
			m_tSpecifyEndDateTime = tSpecifyEndDateTime;
		}

		bool bError = false;
		time_t dwTime = 0;
		VIDEO_B2_FRAME * b2 = NULL;
		BOOL bHasFirstIFrame = FALSE;
		while( m_bProcessing && bHasFirstIFrame == FALSE && m_pcFileReader->GetNextFrame( m_pFrame, m_DataInfo) )
		{
			if( m_DataInfo.dwFrameType != 1 ) // I Frame
			{
				continue;
			}
				
			dwTime = 0;
			if( m_pFrame[0]==0x00 && m_pFrame[1]==0x00 && m_pFrame[2]==0x01 && m_pFrame[3]==0xB2 )
			{
				b2 = (VIDEO_B2_FRAME *)m_pFrame;
				//dwTime = b2->prdata.timestamp.tv_usec;
				dwTime = b2->prdata.date_time;
			}

			if( !CheckPeriodOfTime( dwTime) )
				continue;

			m_dwFullFrameLen = m_DataInfo.dw20DataLength + TCP1020_HEADER_SIZE;
			if( ConvertFrame( pszFileName, m_DataInfo.dwMSGType,
				m_DataInfo.dwFrameType, m_pFrame, m_dwFullFrameLen, pszjFilePath) )
			{
				bHasFirstIFrame = TRUE;
				bool bAudioInTimeRage = false;
				BOOL bInTime = FALSE;
				while( m_bProcessing && m_pcFileReader->GetNextFrame( m_pFrame, m_DataInfo) )
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
						case 2:	// audio
						case 3:
							bInTime = FALSE;
							break;
						}
					}

					if( bInTime )
					{
						m_dwFullFrameLen = m_DataInfo.dw20DataLength + TCP1020_HEADER_SIZE;
						if( ConvertFrame( pszFileName, m_DataInfo.dwMSGType, m_DataInfo.dwFrameType,
							m_pFrame, m_dwFullFrameLen, pszjFilePath) )
						{
							;
						}
					}
				}					
			}
			break;
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

void CJPEGWriter::CloseAll()
{
	m_bProcessing = false;
	
	ReleaseFileReader();
	Close();
	if(!m_bConvertFolder)
	{
		AfxGetApp()->GetMainWnd()->PostMessage(FILE_CLOSE, 0, 0);
	}
}
void CJPEGWriter::GetConvertInfo(DWORD& m_nIFrame, DWORD& m_nPFrame, DWORD& m_nAFrame)
{
	m_nIFrame = m_dwIFrameCount;
	m_nPFrame = m_dwPFrameCount;
	m_nAFrame = m_dwAFrameCounter;
}

bool CJPEGWriter::GetResolutionAndFPSBySequenceHeader( unsigned char* ucSEQ, int nSEQLen, int& nWidth, int& nHeight, int& nFPS )
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

int CJPEGWriter::GetSequenceHeader( BYTE* pRaw, int nRawLen, BYTE* pSEQ )
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