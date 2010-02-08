#include "stdafx.h"
#include "RawDataProcess.h"

#define CAM_WIDTH	720	
#define CAM_HEIGHT	480


bool m_bInit = false;



// raw data call back
void WINAPI RAW_CB(DWORD UserParam, DWORD dwDateType, BYTE* pBuff, DWORD dwLen)
{
	CRawDataProccess* pRawData = (CRawDataProccess*) UserParam;
    pRawData->Process(dwDateType, pBuff, dwLen);

}


CRawDataProccess::CRawDataProccess()
{
    m_pInBuf = new char[256*1000];
    m_pOutBuf = new char[CAM_WIDTH*576*4];
    m_pAudioBuf = new char[256*1000];
    memset(m_pInBuf, 0x00, 256*1000);
    memset(m_pOutBuf, 0x00, 256*1000);
    memset(m_pAudioBuf, 0x00, 256*1000);

    m_dwBufLen = 0;
    m_hWnd = NULL;


    //XVID
    m_dwWidth = CAM_WIDTH;
    m_dwHeight = CAM_HEIGHT;
    
    int xvidret;
    
    memset(&m_xvidDecHandle, 0, sizeof(m_xvidDecHandle));

    m_xvidDecHandle.version = XVID_VERSION;
    m_xvidDecHandle.height = 0;
    m_xvidDecHandle.width = 0;

    if(!m_bInit)
    {
        xvid_gbl_init_t xvid_gbl_init;
        memset(&xvid_gbl_init, 0, sizeof(xvid_gbl_init));
        xvid_gbl_init.version = XVID_VERSION;
        xvidret = xvid_global(0, XVID_GBL_INIT, &xvid_gbl_init, 0);
        m_bInit = true;
    }

    xvidret = xvid_decore(NULL, XVID_DEC_CREATE, &m_xvidDecHandle, NULL);

    mf_PrepareDibBuffer(&m_Bmi, CAM_WIDTH, CAM_HEIGHT);
}

CRawDataProccess::~CRawDataProccess()
{
	if(m_pOutBuf)
	{
		delete [] m_pOutBuf;
		m_pOutBuf = NULL;
	}

	if(m_pInBuf)
	{
		delete [] m_pInBuf;
		m_pInBuf = NULL;
	}

	if(m_pAudioBuf)
	{
		delete [] m_pAudioBuf;
		m_pAudioBuf = NULL;
	}

	if (m_Bmi != NULL) {
		delete m_Bmi;
		m_Bmi = NULL;
	}



	int xvidret = xvid_decore(m_xvidDecHandle.handle, XVID_DEC_DESTROY, 0, 0);


}

int CRawDataProccess::Video(BYTE* pBuff, DWORD dwLen)
{
    BYTE* buf = pBuff;
    DWORD len = dwLen;


	// Reset buffer
	memset(m_pInBuf, 0x00, 256*1000);
	m_dwBufLen = 0;

	// Skip the B2 header
	m_dwBufLen = len - 44;
	memcpy(m_pInBuf, buf + 44, len - 44);
	
	DWORD dwHead;
	bool bValid = false;
	CString csFrame;

	memcpy(&dwHead, m_pInBuf, sizeof(DWORD));
	// I Frame
	if(0xB0010000 == dwHead)
	{
		DWORD dwB3;
		memcpy(&dwB3, m_pInBuf + 31, sizeof(DWORD));
		// GOP B3 data
		if(0xB3010000 == dwB3)
		{
			bValid = true;
			csFrame = "I Frame";
		}
	}
	// P Frame
	else if(0xB6010000 == dwHead)
	{
		// We return here if we only want to decode I frame.
        /*
		if(m_bIOnly)
		{
			return;
		}
        */
		bValid = true;
		csFrame = "P Frame";
	}
	// Invalid data
	else
	{
		csFrame = "Unknow data";
		bValid = false;
	}

    /*
	if(m_bDebugMessageShow)
	{
		CString csDebug;
		csDebug.Format("Get %s\n", csFrame);
		OutputDebugString(csDebug);
	}

	if(bValid)
	{
		VideoDisply();
	}
    */

    	// Decode video frame
	LPBITMAPINFOHEADER lpbih;
	xvid_dec_frame_t    xvidDecFrame;
	int iSyncLen = 0;

	xvidDecFrame.output.csp = XVID_CSP_BGR;

	xvidDecFrame.output.plane[0] = m_pOutBuf;
	xvidDecFrame.output.stride[0] = m_dwWidth*3;
	xvidDecFrame.bitstream = m_pInBuf+iSyncLen;
	xvidDecFrame.length = m_dwBufLen;-iSyncLen;

	xvidDecFrame.general = XVID_LOWDELAY;
	xvidDecFrame.version = XVID_VERSION;

	int xvidret = xvid_decore(m_xvidDecHandle.handle, XVID_DEC_DECODE, &xvidDecFrame, 0);

    

    
	
	// Draw
    /*
	CDC* thedc = m_cCtrlVW.GetDC();
	if (!thedc)
		return ;
        m_hDrawDC = ::GetDC(m_hWnd);;
        // m_cCtrlVW.GetWindowRect(&rect);
        */
	m_hDrawDC = ::GetDC(m_hWnd);;
	RECT rect;
    ::GetWindowRect(m_hWnd, &rect);
	DWORD dwWidth = rect.right - rect.left, dwHeight = rect.bottom - rect.top;
	lpbih = (LPBITMAPINFOHEADER) m_Bmi;
	::SetStretchBltMode(m_hDrawDC, COLORONCOLOR);
	::StretchDIBits(m_hDrawDC, 0, dwHeight, dwWidth, -dwHeight, 0 ,0, lpbih->biWidth, (lpbih->biHeight), (const void*)m_pOutBuf, m_Bmi, DIB_RGB_COLORS, SRCCOPY);
    ::ReleaseDC( m_hWnd, m_hDrawDC);
	// m_cCtrlVW.ReleaseDC(thedc);
    

    return 0;
}

int CRawDataProccess::Audio(BYTE* pBuff, DWORD dwLen)
{

    return 0;
}
#ifndef DATA_TYPE_VIDEO
#define DATA_TYPE_VIDEO 0x01
#endif
#ifndef DATA_TYPE_AUDIO
#define DATA_TYPE_AUDIO 0x02
#endif

int CRawDataProccess::Process(DWORD dwDataType, BYTE* pBuff, DWORD dwLen)
{
    
    switch(dwDataType)
    {
    // Video
    case DATA_TYPE_VIDEO:
        Video(pBuff, dwLen);
    break;

    // Audio
    case DATA_TYPE_AUDIO:
        Audio(pBuff, dwLen);
    break;
    default:
    break;
    }
    

    return 0;
}

bool CRawDataProccess::mf_PrepareDibBuffer(LPBITMAPINFO *lplpbi, DWORD dwX, DWORD dwY)
{
	DWORD				cbSize;
	DWORD				dwActX;
	HDC					hdc;
	LPBITMAPINFOHEADER	lpbih;
	
	dwActX = (dwX * 24 / 8 + 3) >> 2 << 2;
	cbSize = sizeof(BITMAPINFO) + dwActX * dwY;

	//Allocate buffer
	*lplpbi = new BITMAPINFO;

	lpbih = (LPBITMAPINFOHEADER)*lplpbi;

	hdc = ::GetDC(NULL);
	GetDeviceCaps(hdc, BITSPIXEL); 

	//Fill information
	lpbih->biSize = sizeof(BITMAPINFOHEADER);
	lpbih->biWidth = dwX;
	lpbih->biHeight = dwY;
	lpbih->biPlanes = 1;
	lpbih->biBitCount = 24;
	lpbih->biCompression = BI_RGB;
	lpbih->biSizeImage = 0;
	lpbih->biXPelsPerMeter = (GetDeviceCaps(hdc, HORZRES) * 1000) / GetDeviceCaps(hdc, HORZSIZE);
	lpbih->biYPelsPerMeter = (GetDeviceCaps(hdc, VERTRES) * 1000) / GetDeviceCaps(hdc, VERTSIZE);
	lpbih->biClrUsed = 0;
	lpbih->biClrImportant = 0;
	::ReleaseDC(NULL, hdc);
	
	return	TRUE;
}
