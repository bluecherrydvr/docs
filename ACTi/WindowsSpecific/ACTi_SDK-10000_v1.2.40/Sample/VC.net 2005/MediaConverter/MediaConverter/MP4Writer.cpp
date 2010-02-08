#include "stdafx.h"

#include "MP4Writer.h"
#include "atlimage.h"
#include "MemoryBitstream.h"

#define MAXNUMSTREAMS			1024
#define FRAME_BUFFER_MAX		10



CMP4Writer::CMP4Writer( const bool bOSD, const char * pszTitleText, const int nOSDTimeType) 
    : CXEncoder()
    , m_pcFileReader(NULL)
    , m_bProcessing(false)
    , m_pFrame(NULL)
    , m_bConvertFolder(false)
    , m_nMP4ConvertFormat(1)
//  , m_hEncoder( NULL)
//  , m_nDecoder( -1)
    , m_bOSD( bOSD)
	, m_szOSDText( pszTitleText)
	, m_nOSDTime( nOSDTimeType )
	
	
{
	   //InitAllCodec();
}

CMP4Writer::~CMP4Writer()
{   
		
    //ReleaseAllCodec();
	CoUninitialize();
}


void CMP4Writer::SetMP4OSD( const bool bOSD, const char * pszTitleText, const int nOSDTimeType )
{   
	
	m_bOSD = bOSD;
	m_szOSDText = pszTitleText;
	m_nOSDTime = nOSDTimeType;
}

void CMP4Writer::SetMP4OSD( const bool bOSD )
{
	m_bOSD = bOSD;
}

void CMP4Writer::SetMP4OSDText( const char * pszTitleText)
{   
         
    m_szOSDText = pszTitleText;
}

void CMP4Writer::SetMP4OSDTimeType( const int nOSDTimeType )
{
	m_nOSDTime = nOSDTimeType;
}

int CMP4Writer::GetVideoRate(int iTVStd, int iFPS)
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

void CMP4Writer::GetVideoSize(int iResolution, int* dwWidth, int* dwHeight)
{
    switch(iResolution)
    {
    case 0:
        *dwWidth = 720;
        *dwHeight = 480;
        break;
    case 1:
        *dwWidth = 352;
        *dwHeight = 240;
        break;
    case 2:
        *dwWidth = 160;
        *dwHeight = 112;
        break;
    case 3:
        *dwWidth = 720;
        *dwHeight = 576;
        break;
    case 4:
        *dwWidth = 352;
        *dwHeight = 288;
        break;
    case 5:
        *dwWidth = 176;
        *dwHeight = 144;
        break;
	case 6: 
		*dwWidth = 176;	
		*dwHeight = 120;
		break;
	case 64: 
		*dwWidth = 640;	
		*dwHeight = 480;
		break;
	case 65: 
		*dwWidth = 1280;	
		*dwHeight = 720;
		break;
	case 66: 
		*dwWidth = 1280;	
		*dwHeight = 960;
		break;
	case 67: 
		*dwWidth = 1280;	
		*dwHeight = 1024;
		break;
	case 68: 
		*dwWidth = 1920;	
		*dwHeight = 1080;
		break;
	case 70:
		*dwWidth = 320;
		*dwHeight = 240;
		break;
	case 71:
		*dwWidth = 160;
		*dwHeight = 120;
		break;
	case 192: // VGA
		*dwWidth = 640;	
		*dwHeight = 480;
		break;
	default :				
		*dwWidth = 720;	
		*dwHeight = 480;
    }
}



void CMP4Writer::GetTimeToStr(time_t time, char *buf )
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

unsigned char *GetMpg4Header( short nMode )
{
	return &(VariableHeader[nMode][0]);	
};

unsigned char *GetMpg4Header( short nTCPType, short nMode, short nFPs )
{ 
	return ( nTCPType > 1 ) ? &(ConstantHeader2[(((nMode*30) + nFPs) - 1)][0]) : &(ConstantHeader[(((nMode*30) + nFPs) - 1)][0]);
};


int CMP4Writer::StringToken(LPTSTR str, LPCTSTR szToken, char** szArray)
{
	int pos;
	int iCount = 0;
	// char caTok[16];

	while (str && *str && str != '\0') 
    {
		szArray[iCount ++] = str;
		if ((pos = strcspn(str, szToken)) < 0) 
        {
			break;
		}
		str += (pos);
		// if(*str == '\0' || *str == '\r' || *str == '\n')// if null-terminated
		if( *str == '\0' )// if null-terminated
			break;
		*str = 0;
		str ++;
		while (*str) 
        {
			if (*str != 0x20 && *str != '\t' )
				break;
			str ++;
		}
	}
	return iCount;
}

void CMP4Writer::swap_bgr24_to_rgb24(char *mem, int n)
{
    char  c;
    char *p = mem;
    int   i = n;
    
    while( --i )
	{
		c = p[1]; 
		p[1] = p[2]; 
		p[2] = c;

		c = p[2]; 
		p[2] = p[0]; 
		p[0] = c;

		p += 3;
    }
}

//bool CMP4Writer::Open(CString csFileName ,  char * pszMP4FilePath)
//{
//   
//	
//	
//}

bool CMP4Writer::ConvertMP4ToJpg( CString csFileName , char * pszMP4FilePath )
{

	LPBITMAPINFO lpbmi = NULL ;
	#define ONE_MP4_MAX_FRAMSIZE (1920*1280*4)
	char caTemp[1024] = {0};
	char* caArray[32] = {0};	
	int iToken = 0;
	int iResolution = 0;
	int dwVideoWidth = 0;
	int dwVideoHeight = 0;

    strcpy(caTemp, csFileName.GetBuffer());
    csFileName.ReleaseBuffer();

    iToken = StringToken(caTemp, "_", (char**)caArray);

	//try
	//{
		if( iToken>=2 )
		{
			iResolution = atoi(caArray[2]);
			GetVideoSize( iResolution, &dwVideoWidth, &dwVideoHeight );
		}
		else
		{
			MessageBox( NULL, "Filename format error", "Error", MB_OK ) ;
			return false;
		}
	//}
	//catch(...)
	//{
	//	MessageBox( NULL, "Filename format error", "Error", MB_OK ) ;
	//	return false;
	//}

   /* strcpy(caTemp, csFileName.GetBuffer());
    csFileName.ReleaseBuffer();*/
	
	char szfullfilename[512];
	memset( szfullfilename,0, sizeof(szfullfilename));
	
    if( pszMP4FilePath )
	{
		
		sprintf( szfullfilename,"%s\\%s\0",	pszMP4FilePath,csFileName );		
        
	}
	
    CString szMP4fullfilename = szfullfilename;     
	FILE *fp = fopen(szMP4fullfilename, "rb"); 
    
	if( !fp )
	{
		AfxMessageBox( "Can't Open MP4 file to JPG... ");
		return false;
	}
	
	
// ================
	//{	// check file size
	//	fseek( fp, SEEK_END, 0);
	//	long lFileSize = ftell( fp);

	//	if( lFileSize <= 0 ||
	//		lFileSize > ONE_MP4_MAX_FRAMSIZE )
	//	{
	//		AfxMessageBox( "????");
	//		fclose( fp);
	//		return false;
	//	}
	//	fseek( fp, SEEK_SET, 0);
	//}
// ================
	

    int iCount = 0;
    int iSize = 0;
	DWORD uRawBuffer = 0;
	BYTE* RawBuffer = new BYTE[ONE_MP4_MAX_FRAMSIZE];
    {
		BYTE* fTemp = new BYTE[ONE_MP4_MAX_FRAMSIZE];
		do
        {
			if( iCount+1024 > ONE_MP4_MAX_FRAMSIZE )
			{
				fclose( fp);
				delete [] fTemp;
				return false;
			}
			 //iSize = fwrite(fTemp+iCount, 1, 1024, fp);
            iSize = fread(fTemp+iCount, 1, 1024, fp);
            if( iSize > 0 )iCount += iSize;
        }while( iSize > 0 && !(iSize < 1024) );
        fclose(fp);

		//char * ptr = &RawBuffer[0];
		BYTE * ptr = &RawBuffer[0];
		if( fTemp[3]==0xB3 )
		{
			memcpy( ptr+uRawBuffer, &VariableHeader[iResolution],
				sizeof( VariableHeader[iResolution]) ) ;

			uRawBuffer += sizeof(VariableHeader[iResolution]);
		}
		else
		{
			;	// not i
		}
		memcpy( ptr+uRawBuffer, fTemp, iCount ) ;
		uRawBuffer += iCount;
		delete [] fTemp ;
	}
    
	// -------------------------------------------------------------------------------
	{
	     m_XVID_nDecoderHandle = m_XVID_lpfnCInit();
		

        lpbmi = m_XVID_lpfnCDecode(m_XVID_nDecoderHandle, dwVideoWidth, dwVideoHeight, RawBuffer, iCount, NULL, NULL, false);
		int m_iWidth = lpbmi->bmiHeader.biWidth;
        int m_iHeight = lpbmi->bmiHeader.biHeight;
		if( m_iHeight < 0 )
		{	
			m_iHeight = 0-m_iHeight;  
		}	
		
		{	
			lpbmi = m_XVID_lpfnCDecode(m_XVID_nDecoderHandle, dwVideoWidth, dwVideoHeight, RawBuffer, iCount, NULL, NULL, false);
			int m_iWidth = lpbmi->bmiHeader.biWidth;
			int m_iHeight = lpbmi->bmiHeader.biHeight;
			if( m_iHeight < 0 )
			{	
				m_iHeight = 0-m_iHeight;  
			}	
		}
			

		//swap_bgr24_to_rgb24( (char*)&lpbmi[44], nW * nH );
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
				//if( dwVideoWidth < 200 ) nSize = 12 ;
				HFONT hFont = ::CreateFont(nSize, 0, 0, 0, nWeight, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH , "Verdana" ) ;
				::SetBkColor(memDC, RGB(255, 255, 255));
				::SetBkMode( memDC, TRANSPARENT );
				::SelectObject( memDC, hFont); 
				::SetTextColor( memDC, RGB(255, 255, 255) );

				::FillRect( memDC, CRect(0,0,abs(m_iWidth),nSize), CBrush(RGB(0, 0, 0) ) ) ;

	//======Get Time Code===========================================
			if( m_nOSDTime )
			{
				CString szDateTime = m_szOSDText,st ;
			
				COleDateTime ct ;

		
				memcpy(&st,RawBuffer-32,4);
			     
				try {
					ct = COleDateTime(atoi(st.Mid(11,4)),atoi(st.Mid(15,2)),atoi(st.Mid(17,2)),atoi(st.Mid(19,2)),atoi(st.Mid(21,2)),atoi(st.Mid(23,2))) ;
					int	  iTZ  = atoi( st.Mid(25,3) ) ;
					ct = ct + COleDateTimeSpan(0,iTZ,0,0) ;
				} catch(...) {}

				if (m_szOSDText!="") szDateTime += " "  ;

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
				
				BITMAP bmp;
				PBITMAPINFO pbmi = NULL; 
				WORD    cClrBits; 
				BYTE* pbbmi = NULL;
				DWORD dwbmiSize;

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
				DWORD m_dwImageSize = pbmi->bmiHeader.biSizeImage;

				pbmi->bmiHeader.biClrImportant = 0; 

				LPBYTE			p32BMP =  new BYTE[sizeof(BITMAPINFO)+1920*1280*4];
				LPBYTE			p32Buff = p32BMP + sizeof(BITMAPINFO) ;
				LPBITMAPINFO	p32Bmi = (LPBITMAPINFO)p32BMP ;

				memset(p32BMP, 0x00, sizeof(BITMAPINFO)+1920*1280*4);
				memcpy(p32BMP, pbmi, sizeof(BITMAPINFO) ) ;

				if(!GetDIBits(memDC, memBM, 0, (WORD)pbmi->bmiHeader.biHeight, p32Buff, p32Bmi, DIB_RGB_COLORS))
				{
					::DeleteDC(memDC);
					::DeleteObject(memBM);
					if(pbmi)
					pbmi = NULL;
					if(pbbmi)
						delete [] pbbmi;
					pbbmi = NULL;
					return 0;
				}

               
				{
					CImage cImage;
					HBITMAP hBMP; 

					HDC hDC = ::GetDC( NULL );
					hBMP = CreateDIBitmap( hDC, &(p32Bmi->bmiHeader), CBM_INIT, p32Buff, p32Bmi, DIB_RGB_COLORS );
					::ReleaseDC( NULL, hDC );
					cImage.Attach(hBMP);
					//
                
					szMP4fullfilename.Replace(".mp4", ".jpg");
					
                    cImage.Save( szMP4fullfilename );
                 
					DeleteObject(hBMP);
				}
			
				if(pbmi)
					pbmi = NULL;
				if(pbbmi)
					delete [] pbbmi;
				if(p32BMP)
					delete [] p32BMP ;
		}		

		m_XVID_lpfnCExit(m_XVID_nDecoderHandle);
		ReleaseAllCodec();
		if(RawBuffer)
			delete [] RawBuffer;
		RawBuffer = NULL;
		return true;
	}
    

    
	if(RawBuffer)
		delete [] RawBuffer;
	RawBuffer = NULL;

    return false;
  
	
}



//bool CMP4Writer::CreateMP4ToJpg(CString sMP4FileName )
//{
//	bool bRet = ConvertMP4ToJpg(sMP4FileName);
//	CloseAll();
//	return bRet;
//}    

void CMP4Writer::CloseAll()
{
	m_bProcessing = false;
	
	ReleaseFileReader();
	Close();
	if(!m_bConvertFolder)
	{
		AfxGetApp()->GetMainWnd()->PostMessage(FILE_CLOSE, 0, 0);
	}
}

bool CMP4Writer::ReleaseFileReader()
{
	if(m_pFrame)
	{
		delete [] m_pFrame;
		m_pFrame = NULL;
	}
	
	if(m_pcFileReader)
	{
		delete m_pcFileReader;
		m_pcFileReader = NULL;
	}
	return true;
}

bool CMP4Writer::Close()
{
	
	return true;
}


