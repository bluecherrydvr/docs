
#include "stdafx.h"
#include "CapChannel.h"
#include "RawDataProcess.h"



CCapChannel::CCapChannel():
m_hSDK(NULL),
m_iChannelNumber(1)
{
    m_hSDK = KOpenInterface();
    KSetRawDataCallback(m_hSDK, (DWORD)&m_oRawData, RAW_CB);

    // Set config file
    
    m_sMCC.RegisterPort	= 0;				// Register port setting
    m_sMCC.ControlPort		= 0;				// Control port setting
    m_sMCC.StreamingPort   = 0;				// Stream(Video) port setting
    m_sMCC.MultiCastPort   = 0;				// Multicast port setting
    m_sMCC.HTTPPort	    = 0;			// HTTP port setting;
    strcpy(m_sMCC.UserID, "");		            // User login name
    strcpy(m_sMCC.Password, "");		        // User password
    strcpy(m_sMCC.UniCastIP, "");

    m_iDisplayMode = 0;
    m_hWnd = NULL;


    m_sMCC.ContactType		= CONTACT_TYPE_CARD_PREVIEW;	// Connect type, Unicast with control


}

CCapChannel::~CCapChannel()
{
    if(m_hSDK)
    {
        KCloseInterface(m_hSDK);
        m_hSDK = NULL;
    }

}
int CCapChannel::StartRecord(char *cpFileName)
{
    if(m_hSDK)
    {
        if(KStartRecord( m_hSDK, cpFileName ))
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }

    return 0;

}
int CCapChannel::StopRecord()
{

    MP4FILE_RECORD_INFO mri;

    if(m_hSDK)
    {
        KStopRecord( m_hSDK, &mri );
    }


    return 0;
}
int CCapChannel::StartWithoutPreview()
{
    if(KSetMediaConfig(m_hSDK, &m_sMCC))
    {
        if(KConnect(m_hSDK))
        {
            if(KStartStreaming(m_hSDK))
            {
                KEnableRender( m_hSDK, false );
                KEnableDecoder( m_hSDK, false );

                return 1;
            }
        }
    }
    return 0;
}
int CCapChannel::Start()
{

    if(KSetMediaConfig(m_hSDK, &m_sMCC))
    {
        if(KConnect(m_hSDK))
        {
            if(KStartStreaming(m_hSDK))
            {

                if(m_iDisplayMode == 1)
                {
                    MEDIA_RENDER_INFO RenderInfo;
                    RenderInfo.hWnd = m_hWnd;
                    RECT rcVW;
                    ::GetWindowRect(m_hWnd, &rcVW);
                    RenderInfo.rect.left = 0;
                    RenderInfo.rect.top = 0;

                    RenderInfo.rect.right = rcVW.right- rcVW.left;
                    RenderInfo.rect.bottom = rcVW.bottom- rcVW.top;
                    
                    //RenderInfo.RenderInterface = DGDI;
                    KSetRenderInfo( m_hSDK, &RenderInfo );

                
                }
                KPlay(m_hSDK);
                return 1;
            }
        }
    }


    return 0;
}


int CCapChannel::Stop()
{
    KStop(m_hSDK);
    KStopStreaming(m_hSDK);
    KDisconnect(m_hSDK);


    return 0;
}
