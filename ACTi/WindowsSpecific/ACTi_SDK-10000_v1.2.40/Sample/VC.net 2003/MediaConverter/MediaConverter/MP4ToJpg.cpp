#include "stdafx.h"
#include "DefCode.h"
#include "XVIDCODEC.h"
#include "atlimage.h"





//extern "C" __declspec(dllexport) int XCInit();
//extern "C" __declspec(dllexport) void XCExit(int nID);
//extern "C" __declspec(dllexport) LPBITMAPINFO XDecode(int nID, int nW, int nH, BYTE* pInBuf, LONG Len, HWND hWnd = NULL, PRECT pRt = NULL, bool bReSync = false);


#define MAXNUMSTREAMS			1024
#define FRAME_BUFFER_MAX		10

// FPS

// CMCAVIConverterDlg dialog
// |===================================|
// |Resolution,	Res Index,	FPS Index. |
// |===================================|
// |N720x480		0,			0~29   |
// |N352x240		1,			30~59  |
// |N160x112		2,			60~89  |
// |P720x576		3,			90~119 |
// |P352x288		4,			120~149|
// |P176x144		5,			150~179|
// |===================================|

unsigned char *GetMpg4Header( short nMode )
{
	return &(VariableHeader[nMode][0]);	
};

unsigned char *GetMpg4Header( short nTCPType, short nMode, short nFPs )
{ 
	return ( nTCPType > 1 ) ? &(ConstantHeader2[(((nMode*30) + nFPs) - 1)][0]) : &(ConstantHeader[(((nMode*30) + nFPs) - 1)][0]);
};



void GetVideoSize( DWORD dwResolutionIndex, DWORD* dwWidth, DWORD* dwHeight )
{
    switch(dwResolutionIndex)
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

// ----------------------------------------------------------------------------
// Function name   : StringToken
// Description     : Parse a string and return a 2deman
// Return type     : static int 
// Argument        : LPTSTR str
// Argument        : LPCTSTR szToken
// Argument        : char** szArray
int StringToken(LPTSTR str, LPCTSTR szToken, char** szArray)
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

void swap_bgr24_to_rgb24(char *mem, int n)
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

extern "C" __declspec(dllexport) bool ConvertRaw2Jpg(CString csFileName,int nOSDTime,CString csOSDText,bool bOSD)
// bool ConvertRaw2Jpg(CString csFileName)
{
	char caTemp[1024] = {0};
	char* caArray[32] = {0};
	BYTE* RawBuffer = new BYTE[1920*1280*4];
	BYTE* fTemp     = new BYTE[1920*1280*4];
	memset(RawBuffer, 0x00, 1920*1280*4);
	memset(fTemp    , 0x00, 1920*1280*4);
	LPBITMAPINFO lpbmi = NULL;;

    int iToken = 0;
    int iResolution = 0;
    DWORD dwVideoWidth = 0;
    DWORD dwVideoHeight = 0;

    strcpy(caTemp, csFileName.GetBuffer());
    csFileName.ReleaseBuffer();

    iToken = StringToken(caTemp, "_", (char**)caArray);

	try {
		if( iToken>=2 )
		{
			iResolution = atoi(caArray[2]);
			GetVideoSize( iResolution, &dwVideoWidth, &dwVideoHeight );
		}
		else
		{
			MessageBox( NULL, "Filename format error", "Error", MB_OK ) ;
			delete [] RawBuffer ;
			delete [] fTemp ;
			return false;
		}
	} catch(...) {
		MessageBox( NULL, "Filename format error", "Error", MB_OK ) ;
		delete [] RawBuffer ;
		delete [] fTemp ;
		return false;
	}

    strcpy(caTemp, csFileName.GetBuffer());
    csFileName.ReleaseBuffer();

    // Is the file exist?
    FILE *fp = fopen(caTemp, "rb");  
    int iCount = 0;
    int iSize = 0;
    

    if (fp)  
    {
        do
        {
            iSize = 0;
            iSize = fread(fTemp+iCount, 1, 1024, fp);
            iCount+=iSize;
        }
        while(!(iSize < 1024));
        fclose(fp);

		if( fTemp[3]==0xB3 )
		{
			memcpy( RawBuffer, &VariableHeader[iResolution], sizeof(VariableHeader[iResolution]) ) ;
			memcpy( RawBuffer+sizeof(VariableHeader[iResolution]), fTemp, iCount ) ;
		}
		else
			memcpy( RawBuffer, fTemp, iCount ) ;

		delete [] fTemp ;

		int h = XCInit();

		lpbmi = XDecode(h, dwVideoWidth, dwVideoHeight, RawBuffer, iCount, NULL, NULL, false);
		lpbmi = XDecode(h, dwVideoWidth, dwVideoHeight, RawBuffer, iCount, NULL, NULL, false);

		int nW = lpbmi->bmiHeader.biWidth;
		int nH = lpbmi->bmiHeader.biHeight;
		if( nH < 0 )
			nH = 0-nH;

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

		HBITMAP memBM = CreateCompatibleBitmap ( hdcScreen, abs(nW), abs(nH) );
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
			int nError = ::StretchDIBits( memDC, 0, 0, abs(nW), abs(nH), 0, 0, abs(nW), abs(nH), (const void*)&lpbmi[1], lpbmi, DIB_RGB_COLORS, SRCCOPY);

			if(GDI_ERROR == nError)
			{
				dwLastError = GetLastError();
				::DeleteDC(memDC);
				::DeleteObject(memBM);
				return 0;
			}

			
			int nWeight = FW_BOLD;
			int nSize   = 17 ;
			if( nW<200 ) nSize = 12 ;
			HFONT hFont = ::CreateFont(nSize, 0, 0, 0, nWeight, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH , "Verdana" ) ;
			::SetBkColor(memDC, RGB(255, 255, 255));
			::SetBkMode( memDC, TRANSPARENT );
			::SelectObject( memDC, hFont); 
			::SetTextColor( memDC, RGB(255, 255, 255) );

			::FillRect( memDC, CRect(0,0,abs(nW),nSize), CBrush(RGB(0, 0, 0) ) ) ;

//======Get Time Code===========================================
				if( nOSDTime && csFileName.GetLength()>30 )
				{
					CString szDateTime = csOSDText;
				
					DWORD dwTime ;
					BYTE  bTimeZone ;
					memcpy( &dwTime, RawBuffer-20, 4 ) ;
					memcpy( &bTimeZone, RawBuffer-28, 1 ) ;

					tm* gmt = gmtime((time_t*)&dwTime) ;

					COleDateTime ct = COleDateTime(gmt->tm_year+1900,gmt->tm_mon+1,gmt->tm_mday,gmt->tm_hour,gmt->tm_min,gmt->tm_sec) ;
					int	  iTZ  = bTimeZone-12 ;

					ct = ct + COleDateTimeSpan(0,iTZ,0,0) ;			
				     
				/*	try {
						ct = COleDateTime(atoi(st.Mid(11,4)),atoi(st.Mid(15,2)),atoi(st.Mid(17,2)),atoi(st.Mid(19,2)),atoi(st.Mid(21,2)),atoi(st.Mid(23,2))) ;
						int	  iTZ  = atoi( st.Mid(25,3) ) ;
						ct = ct + COleDateTimeSpan(0,iTZ,0,0) ;
					} catch(...) {}*/

											
					if (csOSDText!="")
						szDateTime = csOSDText + " " ;  
					else
						szDateTime = csOSDText ;

					switch ( nOSDTime )
					{
					case 1 :
						szDateTime += ct.Format("%Y-%m-%d %H:%M:%S") ;
						break ;
					case 2 :
						szDateTime += ct.Format("%m-%d-%y %H:%M:%S") ;
						break ;
					case 3 :
						szDateTime += ct.Format("%d-%m-%y %H:%M:%S") ;
						break ;
					}

					::TextOut( memDC, 5,  0, szDateTime, szDateTime.GetLength());
				}

				else
					::TextOut( memDC, 5,  0, csOSDText, csOSDText.GetLength());
		//=============================================================


					DeleteObject(hFont);
			
#if 1
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


			CImage cImage;
			HBITMAP hBMP; 

			HDC hDC = ::GetDC( NULL );
			hBMP = CreateDIBitmap( hDC, &(p32Bmi->bmiHeader), CBM_INIT, p32Buff, p32Bmi, DIB_RGB_COLORS );
			::ReleaseDC( NULL, hDC );
			cImage.Attach(hBMP);
			
			csFileName.Replace(".mp4", ".jpg");
			cImage.Save( csFileName );
			DeleteObject(hBMP);


			if(pbmi)
				pbmi = NULL;
			if(pbbmi)
				delete [] pbbmi;
			if(p32BMP)
				delete [] p32BMP ;


		}

		XCExit(h);
		if(RawBuffer)
			delete [] RawBuffer;
		RawBuffer = NULL;
		return true;
	}
    else { 
        // can't find the file
    }                                                                

	if(RawBuffer)
		delete [] RawBuffer;
	RawBuffer = NULL;

    return false;
}