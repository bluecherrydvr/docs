#pragma once

#include "SDK10000.h"
#include "xvid.h"

void WINAPI RAW_CB(DWORD UserParam, DWORD dwDateType, BYTE* pBuff, DWORD dwLen);



class CRawDataProccess
{
public:
    CRawDataProccess();
    ~CRawDataProccess();
    int Process(DWORD dwDataType, BYTE* pBuff, DWORD dwLen);
    bool mf_PrepareDibBuffer(LPBITMAPINFO *lplpbi, DWORD dwX, DWORD dwY);
    void SetWindowHandle(HWND hWnd) { m_hWnd = hWnd; };
    

private:

    HWND        m_hWnd;

    // For raw data call back
    char* m_pInBuf;
    char* m_pOutBuf;
    char* m_pAudioBuf;
    DWORD m_dwBufLen;

    // Save BMP Button 
    BITMAPINFO* m_Bmi;
    

    // DC handle
    HDC		            m_hDrawDC;

    // XVID
    xvid_dec_create_t   m_xvidDecHandle;
    DWORD               m_dwWidth;
    DWORD               m_dwHeight;

    int Video(BYTE* pBuff, DWORD dwLen);
    int Audio(BYTE* pBuff, DWORD dwLen);
};