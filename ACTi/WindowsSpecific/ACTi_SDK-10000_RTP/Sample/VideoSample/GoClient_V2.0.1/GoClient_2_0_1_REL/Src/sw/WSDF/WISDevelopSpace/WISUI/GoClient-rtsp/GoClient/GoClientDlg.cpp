// GoClientDlg.cpp : implementation file
//

#include "stdafx.h"

#include <vector>

#include "GoClient.h"
#include "GoClientDlg.h"
#include "debugprint.h"
#include "MulticastAddrDlg.h"
#include ".\goclientdlg.h"

#include "PtsClock.h"

// LIVE.COM #includes and #defines
#include <BasicUsageEnvironment.hh>
#include <liveMedia.hh>
#include <GroupsockHelper.hh>
#ifdef DEBUG
#define CHECK(expression) if (!(expression)) { WriteErr("RTSP/RTP error: " #expression "\n");   break; }
#else
#define CHECK(expression) if (!(expression)) break
#endif
TaskScheduler* scheduler = NULL;
UsageEnvironment* env = NULL;
FramedSource* videoSource = NULL;
FramedSource* audioSource = NULL;
Boolean usingLiveDotCom = False; // by default
unsigned liveNumChannels = 1; // by default
unsigned liveAudioFrequency = 8000; // by default
Boolean networkAudioIsULaw = False; // by default
// END LIVE.COM #includes and #defines

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mpr.lib") 




#define MAXFRAMESIZE (512*1024)
#define SOUNDBUFSIZE 48000*4/30 //the largest

#define FIRST_PTS_MAX_DIFF	UI64(600000000)     //10min

struct SPacketHead
{
	unsigned char StreamId;     // 
	unsigned char StreamSubId;	// 0 -- video , 1 -- audio 
	unsigned char KeyPacket;	// last packet of a frame
	unsigned char TotalPacket;  // Total packet number in a frame
	unsigned char PacketNum;	// Packet serial number in a frame
	unsigned char FrameCheckSum;
	unsigned char Reserve[2];
	unsigned int  FrameNum;		// timestamp of a frame
	unsigned int  FrameLen;     // total length of a frame
};

void checkerror(int rtperr)
{
	if (rtperr < 0)
	{
		PrintString("Error:");
	}
}

RTPSource    *rtpVideoSource;
RTPSource    *rtpAudioSource;
mtime_t	       iAudioPts = 0;
mtime_t		iVideoPts = 0;
input_clock_t  inputClock;
mtime_t          iGlobalPcr = 0;
mtime_t          iGlobalPcrStart = 0;
mtime_t          iGlobalLength = 0;
int 			bAudioRtcpSync = 0;
int 			bVideoRtcpSync = 0;
CRITICAL_SECTION  CriticalSec;
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGoClientDlg dialog

CGoClientDlg::CGoClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGoClientDlg::IDD, pParent)
    , m_Mute(FALSE)
{
	//{{AFX_DATA_INIT(CGoClientDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGoClientDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CGoClientDlg)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
    DDX_Check(pDX, IDC_CHECK1, m_Mute);
}

BEGIN_MESSAGE_MAP(CGoClientDlg, CDialog)
	//{{AFX_MSG_MAP(CGoClientDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck1)
END_MESSAGE_MAP()


UINT AReceiveThreadProc(LPVOID pParam)
{
	CGoClientDlg *pDlg = (CGoClientDlg *)pParam;
	pDlg->DoInAReceiveThread();
	return 0;
} 
UINT VReceiveThreadProc(LPVOID pParam)
{
	CGoClientDlg *pDlg = (CGoClientDlg *)pParam;
	pDlg->DoInVReceiveThread();
	return 0;
} 

/////////////////////////////////////////////////////////////////////////////
// CGoClientDlg message handlers
/////////////////////////////////////////////////////////////////////////////

extern NAME_ENUM_PAIR AudioFormatList[];

static void doLiveEventLoop(CGoClientDlg* pDlg); // forward

BOOL CGoClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	pAReceiveThread = NULL;
	pVReceiveThread = NULL;

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	//CMulticastAddrDlg dlg;

ShowAddrDlg:
	if(dlg.DoModal()==IDCANCEL)
	{
		EndDialog(0);
		return true;
	}
	//dlg.m_rtp_port = AfxGetApp()->GetProfileString("Setting","LOCAL_RTP_PORT","6000");
	//dlg.m_Audio_rtp_port = AfxGetApp()->GetProfileString("Setting","LOCAL_AUDIORTP_PORT","6002");
	AfxGetApp()->WriteProfileString("Setting","LOCAL_RTP_PORT",dlg.m_rtp_port);
    AfxGetApp()->WriteProfileString("Setting","LOCAL_AUDIORTP_PORT",dlg.m_Audio_rtp_port);

	int local_rtp_port = atoi((char *)(LPCSTR)(dlg.m_rtp_port));
	int local_Artp_port = atoi((char *)(LPCSTR)(dlg.m_Audio_rtp_port));
	
	if (!dlg.m_radio) {
		// A RTSP/RTP stream:
		WriteErr("#####@@@@@BEGINNING LIVE.COM\n");
		usingLiveDotCom = True;
		Boolean success = False;
		char const* rtspURL = dlg.m_mcast_addr;

		do {
			CHECK(scheduler = BasicTaskScheduler::createNew());
			CHECK(env = BasicUsageEnvironment::createNew(*scheduler));

			RTSPClient* rtspClient;
			CHECK(rtspClient = RTSPClient::createNew(*env, 0,"GoClient"));

			char* sdpDescription = rtspClient->describeURL(rtspURL);
			if (sdpDescription == NULL) {
				WriteErr2("#####@@@@@RTSP \"DESCRIBE\" failed: %s\n", env->getResultMsg());
				PrintString("Failed to open RTSP URL");
				break;
			}

			MediaSession* mediaSession;
			CHECK(mediaSession = MediaSession::createNew(*env, sdpDescription));

			// Set up an audio and video subsession, if present.
			// We currently support:
			//     - MPEG-4 video
			//     - raw PCM audio
			MediaSubsession* subsession;
			PrioritizedRTPStreamSelector* multiSource;
			int multiSourceSessionId;
			if (mediaSession->initiateByMediaType("video/MP4V-ES", subsession,
						multiSource, multiSourceSessionId)) {
				WriteErr2("#####@@@@@Initiated video/%s subsession\n", subsession->codecName());
				videoSource = subsession->readSource();
				rtpVideoSource  = subsession->rtpSource();

				int rtpSocketNum = subsession->rtpSource()->RTPgs()->socketNum();
				increaseReceiveBufferTo(*env, rtpSocketNum, 2000000);

				// Issue a RTSP "SETUP" command on the chosen subsession:
				if (!rtspClient->setupMediaSubsession(*subsession)) break;
			}
			if (mediaSession->initiateByMediaType("audio/L16", subsession,
												multiSource, multiSourceSessionId) ||
				mediaSession->initiateByMediaType("audio/PCMU", subsession,
												multiSource, multiSourceSessionId)) {
				WriteErr2("#####@@@@@Initiated audio/%s subsession, ", subsession->codecName());
				liveNumChannels = subsession->numChannels();
				WriteErr2("%d channels, ", liveNumChannels);
				liveAudioFrequency = subsession->rtpTimestampFrequency();
				WriteErr2("%d Hz\n", liveAudioFrequency);
				audioSource = subsession->readSource();
				rtpAudioSource = subsession->rtpSource();
				networkAudioIsULaw = strcmp(subsession->codecName(), "PCMU") == 0;
				if (networkAudioIsULaw) {
					// The data is 8-bit u-law audio.
					// Add a filter that converts it to 16-bit PCM:
					audioSource = PCMFromuLawAudioSource::createNew(*env, audioSource);
				} else {
					// The data is raw 16-bit PCM audio, in network (i.e., big-endian) order
#ifndef _BIG_ENDIAN_
					// Add a filter that converts the data to little-endian order, for playing:
					audioSource = EndianSwap16::createNew(*env, audioSource);
#endif
				}

				int rtpSocketNum = subsession->rtpSource()->RTPgs()->socketNum();
				increaseReceiveBufferTo(*env, rtpSocketNum, 100000);

				// Issue a RTSP "SETUP" command on the chosen subsession:
				if (!rtspClient->setupMediaSubsession(*subsession)) break;
			}

			// Issue a RTSP aggregate "PLAY" command on the whole session:
			if (!rtspClient->playMediaSession(*mediaSession)) break;

			WriteErr("#####@@@@@ENDING LIVE.COM SUCCESS\n");
			iGlobalLength = (mtime_t)(mediaSession->playEndTime() * 1000000.0);
			if(iGlobalLength < 0 )
			{
				iGlobalLength = 0;
			}
			input_ClockInit( &inputClock, 60);
			success = True;
		} while (0);

		if (!success) {
			WriteErr("#####@@@@@ENDING LIVE.COM FAILURE\n");
			AfxMessageBox("Unknown failure opening RTSP/RTP stream");
			//return FALSE;
		goto ShowAddrDlg;
		}
		InitializeCriticalSection(&CriticalSec);
	} else if (dlg.m_radio==1) {
		// A UDP multicast stream:
        char tmp[]="000.000.000.000";
        strncpy(tmp,dlg.m_mcast_addr,3);
        int inetAddr = inet_addr(tmp);
        if ((inetAddr >= 224)&&(inetAddr <= 239))
        {
		    Channel.Initialize((char *)(LPCSTR)(dlg.m_mcast_addr),local_rtp_port,local_Artp_port);

        }
        else
        {
            AfxMessageBox("It is not a valid multicast address", MB_ICONEXCLAMATION | MB_OK);
		    //EndDialog(0);
		    goto ShowAddrDlg;
        }
	} else {
		// A UDP unicast stream:
        char tmp[]="000.000.000.000";
        strncpy(tmp,dlg.m_mcast_addr,3);
        int inetAddr = inet_addr(tmp);
        if (((inetAddr <= 224) && (inetAddr > 0))||(inetAddr >= 239))
        {
		    UChannel.Initialize((char *)(LPCSTR)(dlg.m_mcast_addr),local_rtp_port,local_Artp_port);
        }
       else
        {
            AfxMessageBox("It is not a valid unicast address", MB_ICONEXCLAMATION | MB_OK);
		    //EndDialog(0);
		    goto ShowAddrDlg;
        } 
	}

	int local_NoChannel;
	int local_SampleRate;
	if (usingLiveDotCom) {
	    local_NoChannel = liveNumChannels;
		local_SampleRate = liveAudioFrequency;
    } else if (dlg.AudioIdx == 1) {
	    local_NoChannel = 1;
	    local_SampleRate = 8000;
    } else {
	    local_NoChannel = 2;
	    local_SampleRate = 48000;
    }

	FrameBufPtr = 0;
	TotalVideoPacket = 0;
	TotalAudioPacket = 0;
	p_vr = new CVideoRender(this, FALSE);
	if(usingLiveDotCom)
	{
		p_vr->Initialize(TRUE);
		p_vr->SetInputClock(&inputClock);
	}
	else
		p_vr->Initialize();
	pFrameBuf = new unsigned char[MAXFRAMESIZE];
	VReceiveThreadState=1; 
	if (!usingLiveDotCom) 
		pVReceiveThread = AfxBeginThread(VReceiveThreadProc,this,THREAD_PRIORITY_NORMAL);
    
	if (dlg.DoAudio || usingLiveDotCom) {
		int ret;

		if(usingLiveDotCom)
		{
			ret = SoundDevice.InitializeDevice(local_NoChannel,local_SampleRate, TRUE);
			SoundDevice.SetInputClock(&inputClock);
		}
		else
			ret = SoundDevice.InitializeDevice(local_NoChannel,local_SampleRate);
	    if(ret == 0)
	        PrintString("Success");
	    else {
            AfxMessageBox("Open Sound device fail!", MB_ICONEXCLAMATION | MB_OK);
            return TRUE;
        }

		pSoundBuf = new unsigned char[SOUNDBUFSIZE]; 
        SoundBufPtr = 0;
	    AReceiveThreadState=1; 

		if (!usingLiveDotCom) 
			pAReceiveThread = AfxBeginThread(AReceiveThreadProc,this,THREAD_PRIORITY_NORMAL);
    }

	VideoFrameGot = 0;
	VideoByteGot = 0;
	AudioFrameGot = 0;
	AudioByteGot = 0;
	SetTimer(1, 1000, NULL);

	if (usingLiveDotCom) doLiveEventLoop(this);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

class liveAudioSink: public MediaSink {
public:
	liveAudioSink(UsageEnvironment& env, CGoClientDlg* pDlg)
		: MediaSink(env),
		fPDlg(pDlg) {
	}

private: // redefined virtual functions:
	virtual Boolean continuePlaying() {
		mtime_t   i_pcr;
		EnterCriticalSection(&CriticalSec);
		i_pcr = iAudioPts;
		if((iVideoPts != 0) && (i_pcr > iVideoPts))
			i_pcr = iVideoPts;

		if( i_pcr != iGlobalPcr && i_pcr > 0 )
		{
		        iGlobalPcr = i_pcr;

			input_ClockSetPCR(&inputClock, (i_pcr + 11 ) * 9 / 100);
			
		        if(iGlobalPcrStart <= 0 || iGlobalPcrStart > i_pcr ||
		            (iGlobalLength > 0 && i_pcr - iGlobalPcrStart > iGlobalLength))
		        {
		           iGlobalPcrStart = i_pcr;
		        }
		}

		LeaveCriticalSection (&CriticalSec);
		
		fSource->getNextFrame(fPDlg->pSoundBuf, SOUNDBUFSIZE,
                        afterGettingFrame, this,
                        onSourceClosure, this);

	        if(!bAudioRtcpSync && rtpAudioSource->hasBeenSynchronizedUsingRTCP() )
	        {
            		OutputDebugString("rtpAudioSource->hasBeenSynchronizedUsingRTCP\n");
			EnterCriticalSection(&CriticalSec);
					
			inputClock.i_synchro_state = SYNCHRO_REINIT;
			inputClock.last_pts = 0;
				
			bAudioRtcpSync = 1;

			/* reset PCR and PCR start, mmh won't work well for multi-stream I fear */
			iAudioPts= 0;
			iGlobalPcrStart = 0;
			iGlobalPcr = 0;

			LeaveCriticalSection (&CriticalSec);
		}
		return True;
	}

private:
	static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
                                struct timeval presentationTime,
                                unsigned durationInMicroseconds) {
		liveAudioSink* audioSink = (liveAudioSink*)clientData;
		audioSink->afterGettingFrame1(frameSize, presentationTime);
	}
	virtual void afterGettingFrame1(unsigned frameSize, struct timeval presentationTime) {
		// Render this frame:
		++fPDlg->AudioFrameGot;
		fPDlg->AudioByteGot += networkAudioIsULaw ? frameSize/2 : frameSize;
		//fPDlg->SoundDevice.Write(fPDlg->pSoundBuf, frameSize);
		//##### Use "presentationTime" to implement A/V sync
		UINT64 apts;
		apts = (UINT64)presentationTime.tv_sec * UI64C(1000000) + (UINT64)presentationTime.tv_usec;
		apts &= UI64C(0x00ffffffffffffff);

		/*2005-03-10 add for support spook*/
		EnterCriticalSection(&CriticalSec);
		if(apts > 0)
			iAudioPts = apts;

		apts = input_ClockGetTS(&inputClock, (apts + 11 ) * 9 / 100 );
		
		LeaveCriticalSection (&CriticalSec);
		/*2005-03-10 add end*/
		
		fPDlg->SoundDevice.Write(fPDlg->pSoundBuf, frameSize, apts);
			
		// Then arrange to get the next frame:
		continuePlaying();
	}

private:
	CGoClientDlg* fPDlg;
};

class liveVideoSink: public MediaSink {
public:
	liveVideoSink(UsageEnvironment& env, CGoClientDlg* pDlg)
		: MediaSink(env),
		fPDlg(pDlg) {
	}

private: // redefined virtual functions:
	virtual Boolean continuePlaying() {
		mtime_t   i_pcr;
		EnterCriticalSection(&CriticalSec);
		i_pcr = iVideoPts;
		if((iAudioPts != 0) && (i_pcr > iAudioPts))
			i_pcr = iAudioPts;

		if( i_pcr != iGlobalPcr && i_pcr > 0 )
		{
		        iGlobalPcr = i_pcr;

			input_ClockSetPCR(&inputClock, (i_pcr + 11 ) * 9 / 100);
			
		        if(iGlobalPcrStart <= 0 || iGlobalPcrStart > i_pcr ||
		            (iGlobalLength > 0 && i_pcr - iGlobalPcrStart > iGlobalLength))
		        {
		           iGlobalPcrStart = i_pcr;
		        }
		}

		LeaveCriticalSection (&CriticalSec);
		
		fSource->getNextFrame(fPDlg->pFrameBuf, MAXFRAMESIZE,
                        afterGettingFrame, this,
                        onSourceClosure, this);

	        if(!bVideoRtcpSync && rtpVideoSource->hasBeenSynchronizedUsingRTCP() )
	        {
            		//msg_Dbg( p_demux, "tk->rtpSource->hasBeenSynchronizedUsingRTCP()" );
            		OutputDebugString("rtpVideoSource->hasBeenSynchronizedUsingRTCP\n");
			EnterCriticalSection(&CriticalSec);
					
			inputClock.i_synchro_state = SYNCHRO_REINIT;
			inputClock.last_pts = 0;
				
			bVideoRtcpSync = 1;

			/* reset PCR and PCR start, mmh won't work well for multi-stream I fear */
			iVideoPts= 0;
			iGlobalPcrStart = 0;
			iGlobalPcr = 0;

			LeaveCriticalSection (&CriticalSec);
		}		

		return True;
	}

private:
	static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
                                struct timeval presentationTime,
                                unsigned durationInMicroseconds) {
		liveVideoSink* videoSink = (liveVideoSink*)clientData;
		videoSink->afterGettingFrame1(frameSize, presentationTime);
	}
	virtual void afterGettingFrame1(unsigned frameSize, struct timeval presentationTime) {
		static int count = 0;
		// Render this frame:
		++fPDlg->VideoFrameGot;
		fPDlg->VideoByteGot += frameSize;
		//if (fPDlg->p_vr != NULL) fPDlg->p_vr->SourceInput(fPDlg->pFrameBuf, frameSize);
		//##### Use "presentationTime" to implement A/V sync
		UINT64 vpts;
		vpts = (UINT64)presentationTime.tv_sec * UI64C(1000000) + (UINT64)presentationTime.tv_usec;
		vpts &= UI64C(0x00ffffffffffffff);

		/*2005-03-10 add for support spook*/
		EnterCriticalSection(&CriticalSec);
		if(vpts > 0)
			iVideoPts = vpts;
		vpts = input_ClockGetTS(&inputClock, (vpts + 11 ) * 9 / 100 );
		
		LeaveCriticalSection (&CriticalSec);
		/*2005-03-10 add end*/
		
		if (fPDlg->p_vr != NULL) 
			fPDlg->p_vr->SourceInput(fPDlg->pFrameBuf, frameSize, vpts);
		// Then arrange to get the next frame:
		continuePlaying();
	}

private:
	CGoClientDlg* fPDlg;
};

static void liveAfterPlaying(void* clientData) {
	WriteErr2("#####@@@@@liveAfterPlaying(): sink %p\n", clientData);
}

	static char liveStopFlag= 0;
UINT liveEventLoopThreadProc(LPVOID pParam) {
	WriteErr2("#####@@@@@liveEventLoopThreadProc() starting - videoSource %p\n", videoSource);
	WriteErr2("#####@@@@@liveEventLoopThreadProc() starting - audioSource %p\n", audioSource);
	CGoClientDlg* pDlg = (CGoClientDlg*)pParam;

	liveVideoSink videoSink(*env, pDlg);
	if (videoSource != NULL) videoSink.startPlaying(*videoSource, liveAfterPlaying, &videoSink);

	liveAudioSink audioSink(*env, pDlg);
	if (audioSource != NULL) audioSink.startPlaying(*audioSource, liveAfterPlaying, &audioSink);

	WriteErr("#####@@@@@calling doEventLoop()\n");
	if (scheduler != NULL) scheduler->doEventLoop(&liveStopFlag);
	WriteErr("#####@@@@@called doEventLoop()\n");
	return 0;
} 
static void doLiveEventLoop(CGoClientDlg* pDlg) {
	// Run the LIVE.COM event loop in a separate thread.  We can do this, because at
	// this point, this is the only code that will be accessing the LIVE.COM libraries.
	AfxBeginThread(liveEventLoopThreadProc, pDlg, THREAD_PRIORITY_NORMAL);
}

void CGoClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGoClientDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGoClientDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CGoClientDlg::OnCancel() 
{
	liveStopFlag = ~0;
	AReceiveThreadState=2;
	VReceiveThreadState=2;
	
	if (dlg.m_radio)
		Channel.Uninitialize();
	else
    {
		WSACleanup();
    }

	if (pVReceiveThread)//think about audio later
	{
		WaitForSingleObject(pVReceiveThread->m_hThread,INFINITE);
		pVReceiveThread = NULL;
	}
	
    if (dlg.DoAudio || usingLiveDotCom)
    {
        SoundDevice.UninitializeDevice();
	    delete pSoundBuf;
    }
	
	p_vr->Uninitialize();
    delete p_vr;
	delete pFrameBuf;

	KillTimer(1);

	if(usingLiveDotCom)
		DeleteCriticalSection(&CriticalSec);

	CDialog::OnCancel();
}

#define RECV_BUF_LEN 65536

void CGoClientDlg::DoInAReceiveThread()
{
	PrintString("Audio Receive Thread Begin");
	
		int TotalLen = 0;
		unsigned char *recv_buf = new unsigned char[RECV_BUF_LEN];
		while(AReceiveThreadState==1)
		{
            int r;
			int rcvd = RECV_BUF_LEN;
            if (dlg.m_radio==1)
			    r = Channel.ReadAudio(recv_buf, rcvd);
            else
			    r = UChannel.ReadAudio(recv_buf, rcvd);
			if(r<0)
				break;

			ProcessAPacket(recv_buf, rcvd);
			TotalLen += rcvd;
		}
        AReceiveThreadState = 3;
		delete recv_buf;

		PrintInt("Audio Total Len",TotalLen);
	
	PrintString("AReceive Thread End");
}
void CGoClientDlg::DoInVReceiveThread()
{
//	PrintString("Video Receive Thread Begin");
		int TotalLen = 0;
		unsigned char *recv_buf = new unsigned char[RECV_BUF_LEN];
		while(VReceiveThreadState==1)
		{
            int r;
			int rcvd = RECV_BUF_LEN;
            if (dlg.m_radio==1)
			    r = Channel.ReadVideo(recv_buf, rcvd);
            else
			    r = UChannel.ReadVideo(recv_buf, rcvd);
			if(r<0)
				break;

			ProcessVPacket(recv_buf, rcvd);
			TotalLen += rcvd;
		}
	    PrintInt("Total Len",TotalLen);
	
    VReceiveThreadState = 3;
	delete recv_buf;

	PrintString("AReceive Thread End");
}

void CGoClientDlg::ProcessVPacket(unsigned char *pPacket, int Len)
{
	SPacketHead *pHead = (SPacketHead *)pPacket;

	if(pHead->StreamSubId != 0)  // video
	{
        printf("Error recieved Videio from wrong port!!!!\n");
        goto _Reset;
    }
    else
    {
        if ((pFrameBuf+FrameBufPtr+Len-sizeof(SPacketHead)) > (pFrameBuf+MAXFRAMESIZE))//drop this pack possible loss the keypacket
	    {
		    PrintFormat("Video Keypacket Lost, %d %d\n",pHead->FrameLen,pSoundBuf+SoundBufPtr+Len-sizeof(SPacketHead));
			goto _Reset;
		}
        memcpy(pFrameBuf+FrameBufPtr, pPacket+sizeof(SPacketHead), Len-sizeof(SPacketHead));
		FrameBufPtr += (Len-sizeof(SPacketHead));
		TotalVideoPacket ++;

		if(pHead->KeyPacket == 1)
		{
#ifdef _BIG_ENDIAN_
			Endian_Reverse32(pHead->FrameLen);
			Endian_Reverse32(pHead->FrameNum);
#endif
			if(TotalVideoPacket != pHead->TotalPacket)
			{
				PrintDualInt(" Video Packet Lost, Got", TotalVideoPacket, "Should be", pHead->TotalPacket);
				goto _Reset;
			}
			else if(FrameBufPtr != pHead->FrameLen)
			{
				PrintFormat(" video Byte Lost, %d %d\n",pHead->FrameLen,FrameBufPtr);
				goto _Reset;
			}
			else
			{
				unsigned char check_sum = 0;
				for(int pp=0;pp<FrameBufPtr;pp++)
					check_sum = check_sum ^ pFrameBuf[pp];

				if(check_sum != pHead->FrameCheckSum)
				{
					PrintString(" !!!!!!!!!!! Video Checksum Error");
					goto _Reset;
				}
			}

//			PrintInt("Get One Frame", GetTickCount());
			p_vr->SourceInput(pFrameBuf, FrameBufPtr);
_Reset:
	        VideoFrameGot ++;
	        VideoByteGot += FrameBufPtr;
	        FrameBufPtr = 0;
	        TotalVideoPacket = 0;

		}
	}

}
void CGoClientDlg::ProcessAPacket(unsigned char *pPacket, int Len)
{
	SPacketHead *pHead = (SPacketHead *)pPacket;

	if(pHead->StreamSubId != 1)  // Audio
	{
        printf("Error recieved Audio from wrong port!!!!\n");
        goto _reset;
	}
    else  // audio
	{
        if ((pSoundBuf+SoundBufPtr+Len-sizeof(SPacketHead)) > (pSoundBuf+6400))//drop this pack possible loss the keypacket
		{
				PrintFormat("audio KeyPacket Lost, %d %d\n",pHead->FrameLen,pSoundBuf+SoundBufPtr+Len-sizeof(SPacketHead));
                Len = (pSoundBuf+6400) - (pSoundBuf+SoundBufPtr);//get whatever we can
		}
	    memcpy(pSoundBuf+SoundBufPtr, pPacket+sizeof(SPacketHead), Len-sizeof(SPacketHead));
	    SoundBufPtr += (Len-sizeof(SPacketHead));
		TotalAudioPacket ++;

		if(pHead->KeyPacket == 1)
		{
#ifdef _BIG_ENDIAN_
			Endian_Reverse32(pHead->FrameLen);
			Endian_Reverse32(pHead->FrameNum);
#endif
			if(TotalAudioPacket != pHead->TotalPacket)
			{
				PrintDualInt(" Audio Packet Lost, Got", TotalAudioPacket, "Should be", pHead->TotalPacket);
			}
			else if(SoundBufPtr != pHead->FrameLen)
			{
				PrintFormat(" audio Byte Lost, %d %d\n",pHead->FrameLen,SoundBufPtr);
			}
			else
			{
				unsigned char check_sum = 0;
				for(int pp=0;pp<SoundBufPtr;pp++)
					check_sum = check_sum ^ pSoundBuf[pp];

				if(check_sum != pHead->FrameCheckSum)
				{
					PrintString(" !!!!!!!!!!! Audio Checksum Error");
				}
			}

	        SoundDevice.Write(pSoundBuf, SoundBufPtr);//need to use audio buffer
_reset:
	        AudioFrameGot ++;
	        AudioByteGot += SoundBufPtr;
	        SoundBufPtr = 0;
	        TotalAudioPacket = 0;
		}
	}

}

void CGoClientDlg::OnTimer(UINT nIDEvent) 
{
 //   if (VideoFrameGot != AudioFrameGot)//check if AudioFrameGot equal to VideoFrameGot diag only!!
 //       AfxMessageBox("VideoFrameGot!=AudioFrameGot", MB_ICONEXCLAMATION | MB_OK);
	SetDlgItemInt(IDC_FRAMERATETEXT, VideoFrameGot, FALSE);
	VideoFrameGot = 0;
	AudioFrameGot = 0;

	CString s;
	s.Format("%d kb", VideoByteGot*8/1024);
	SetDlgItemText(IDC_BITRATETEXT, s);
	VideoByteGot = 0;

	s.Format("%d kb", AudioByteGot*8/1024);
	SetDlgItemText(IDC_AUDIOBITRATETEXT, s);
	AudioByteGot = 0;
    CDialog::OnTimer(nIDEvent);
}

void CGoClientDlg::OnBnClickedCheck1()
{
    m_Mute=!m_Mute;
    SoundDevice.Mute(m_Mute);
}


