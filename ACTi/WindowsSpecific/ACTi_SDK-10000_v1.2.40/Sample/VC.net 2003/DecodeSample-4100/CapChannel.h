#pragma once

#include "SDK10000.h"
#include "RawDataProcess.h"

class CCapChannel
{
public:
    CCapChannel();
    ~CCapChannel();
    int Start();
    int Stop();
    int StartRecord(char *cpFileName);
    int StopRecord();
    void SetChannel(int iChannel){
        m_iChannelNumber = iChannel;
        m_sMCC.ChannelNumber = m_iChannelNumber;
    };
    SetWindowHandle(HWND hWnd, int iMode = 0){
        m_iDisplayMode = iMode;
        
        if(m_iDisplayMode == 0)
        {
            KEnableRender( m_hSDK, false );
            KEnableDecoder( m_hSDK, false );

            m_oRawData.SetWindowHandle(hWnd);
        }
        else if(m_iDisplayMode == 1)
        {
            m_hWnd = hWnd;
            KEnableRender( m_hSDK, true );
            KEnableDecoder( m_hSDK, true );

        }
    }
    
    int StartWithoutPreview();


private:
    HANDLE m_hSDK;
    CRawDataProccess m_oRawData;
    MEDIA_CONNECTION_CONFIG m_sMCC;
    HWND m_hWnd;
    int m_iChannelNumber;
    int m_iDisplayMode;
};

