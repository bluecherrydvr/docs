// VideoRender.cpp: implementation of the CVideoRender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VideoRender.h"
#include "DebugPrint.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define	VIDEO_PTS_MAX_GAP	1200000		//	1.2s
#define	VIDEO_PTS_MIN_GAP	20000		//	0.02s
#define	VIDEO_WAKEUP_TIME	2000		//	0.002s

// public function

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVideoRender::CVideoRender(CWnd *pParent, int ForceRGB)
{
	m_pParent = pParent;
	m_ForceRGB = ForceRGB;
	
	m_bAvSyncEnabled = FALSE;
	
	m_player = playerCreate(m_pParent);
	if(ForceRGB)
	{
		playerEnableDefaultMenu(m_player, FALSE);
		playerSetWindowStyle(m_player, 
			WS_POPUPWINDOW|WS_CAPTION|WS_DLGFRAME|WS_MINIMIZEBOX);
		
	}
}

CVideoRender::~CVideoRender()
{
	playerDestroy(m_player);
}

/*int CVideoRender::Initialize()
{
m_mode = MPEG4;

  decoder = 0;
  StartDecoder();
  return 0;
}*/
int CVideoRender::Initialize(BOOL bAvSyncEnabled)
{
	m_bAvSyncEnabled = bAvSyncEnabled;
	m_mode = MPEG4;
	
	decoder = 0;
	m_FirstVideoPts = 0;
	m_FirstPlayTime = 0;
	m_LastVideoPts = 0;
	m_FirstBPts = 0;
	m_FirstBPlayTime = 0;
	StartDecoder();
	return 0;
}

void CVideoRender::Uninitialize()
{
	StopDecoder();
	DestroyVideoWindow();
	
}

void CVideoRender::SourceInput(unsigned char *pBuf, int BufLen)
{
	AddToDecodeQueue(pBuf, BufLen);
}

void CVideoRender::SourceInput(unsigned char *pBuf, int BufLen, UINT64 pts)
{
	VideoPts[DecodeTail] = pts;
	AddToDecodeQueue(pBuf, BufLen);
}

// private function

void CVideoRender::CreateVideoWindow()
{
	if(m_ForceRGB)
		playerInit(m_player, m_width, m_height, DDRAW_DIB24);
	else
		playerInit(m_player, m_width, m_height);
	
	HWND hwnd = playerGetWindow(m_player);
	SetWindowPos(hwnd,NULL,20,20,0,0,SWP_NOSIZE|SWP_NOZORDER);
	
	//	ShowWindow(hwnd, SW_MAXIMIZE);
	
	m_ddraw = playerGetVideoMode(m_player);
	m_pitch = playerGetPitch(m_player);
}

void CVideoRender::DestroyVideoWindow()
{
	playerClose(m_player);
}



DWORD WINAPI DecodeThreadProc(LPVOID lpParameter)   
{
	((CVideoRender *)lpParameter)->DoInDecodeThread();
	return 0;
} 

void 	CVideoRender::DoInDecodeThread()
{
	//	ShowMessage("Decode thread begin");
	
	int NumInQueue;
	
	while(m_DecodeThreadState==1)
	{							   
		//		PrintDualInt("Head De",DecodeHead,"Tail",DecodeTail);
		
		NumInQueue = DecodeTail - DecodeHead;
		if(NumInQueue<0)
			NumInQueue += DECODE_QUEUE_SIZE;
		
		if(NumInQueue==0)
			Sleep(5);
		else
		{
			UINT8 *pDDBuf = playerBeginDraw(m_player);
			TMP_FrmInfo frmInfo;
			/*Max modified 2004-04-01 for adding a parameter*/
			//int r = DecodeOneFrame(DecodeQueue[DecodeHead],pDDBuf);
			int r = DecodeOneFrame(DecodeQueue[DecodeHead],pDDBuf, DecodeBufLenQueue[DecodeHead], &frmInfo);
			if(r==0)
			{
				if(m_bAvSyncEnabled == TRUE)
				{	
					BOOL bDraw = FALSE;
					
					if((frmInfo.ftype == I_FRAME) || (frmInfo.ftype == P_FRAME))
					{
						GetRefTime(m_pInputClock, &m_FirstVideoPts, &m_FirstPlayTime);

						if(VideoPts[DecodeHead] > m_LastVideoPts)
						{
							INT64 PlayDiff;
							INT64 PtsDiff;
							
							PtsDiff = VideoPts[DecodeHead] - m_FirstVideoPts;
							PlayDiff = mdate() - m_FirstPlayTime;
							
							if(PtsDiff > PlayDiff) 
							{
								if(((PtsDiff - PlayDiff) > VIDEO_PTS_MIN_GAP) && ((PtsDiff - PlayDiff) < VIDEO_PTS_MAX_GAP))
								{
									Sleep((UINT32)((PtsDiff - PlayDiff- VIDEO_WAKEUP_TIME) / 1000));
								}
								else if((PtsDiff - PlayDiff) >= VIDEO_PTS_MAX_GAP)
								{
									m_FirstVideoPts = VideoPts[DecodeHead];
									m_FirstPlayTime = mdate();
									
									m_FirstBPts = 0;
									m_FirstBPlayTime = 0;
								}
							}	
						}
						
						bDraw = TRUE;
					}
					else if(frmInfo.ftype == B_FRAME)
					{
						/*First Decoded frame must be I Frame*/
						if(m_FirstVideoPts == 0)
						{
							bDraw = FALSE;
						}
						else
						{
							if(m_FirstBPts == 0)
							{
								m_FirstBPts = VideoPts[DecodeHead];
								m_FirstBPlayTime = mdate();
							}
							else
							{
								INT64 PlayDiff;
								INT64 PtsDiff;
								
								PtsDiff = VideoPts[DecodeHead] - m_FirstBPts;
								PlayDiff = mdate() - m_FirstBPlayTime;
								
								if(PtsDiff > PlayDiff) 
								{
									if(((PtsDiff - PlayDiff) > VIDEO_PTS_MIN_GAP) && ((PtsDiff - PlayDiff) < VIDEO_PTS_MAX_GAP))
									{
										Sleep((UINT32)((PtsDiff - PlayDiff- VIDEO_WAKEUP_TIME) / 1000));
									}
									else if((PtsDiff - PlayDiff) >= VIDEO_PTS_MAX_GAP)
									{
										m_FirstVideoPts = 0;
										m_FirstPlayTime = 0;
										
										m_FirstBPts = VideoPts[DecodeHead];
										m_FirstBPlayTime = mdate();
									}
								}	
							}
							bDraw = TRUE;
						}
					}
					playerEndDraw(m_player, bDraw);
				}
				else
					playerEndDraw(m_player, TRUE);
			}
			/*2005-03-04 add for fixing bug 1161*/
			else if(r > 4)
			{
				DecodeBufLenQueue[DecodeHead] -= r;
				continue;
			}
			/*2005-03-04 add end*/
			
			delete DecodeQueue[DecodeHead];
			/*Max add 2004-04-01 for reseting the buffer length*/
			DecodeBufLenQueue[DecodeHead] = 0;
			if(m_bAvSyncEnabled == TRUE)
			{
				///*skip B frame*/
				if((frmInfo.ftype == I_FRAME) || (frmInfo.ftype == P_FRAME))
				{		
					if(m_LastVideoPts < VideoPts[DecodeHead])
						m_LastVideoPts = VideoPts[DecodeHead];
				}
				
				VideoPts[DecodeHead] = 0;
			}
			
			DecodeHead++;	
			if(DecodeHead >= DECODE_QUEUE_SIZE)
				DecodeHead = 0;
		}
	}
	
	while(1)	// remove undecoded frame
	{
		NumInQueue = DecodeTail - DecodeHead;
		if(NumInQueue<0)
			NumInQueue += DECODE_QUEUE_SIZE;
		
		if(NumInQueue==0)
			break;
		else
		{
			delete DecodeQueue[DecodeHead];
			DecodeHead++;							    
			if(DecodeHead >= DECODE_QUEUE_SIZE)
				DecodeHead = 0;
		}
	}
	
	decoder->Release();
	decoder = NULL;
	
	m_DecodeThreadState=2;
	//	ShowMessage("Decode thread end");
}

void CVideoRender::AddToDecodeQueue(unsigned char *pBuf, int Len)
{
	unsigned char *p=new unsigned char[Len+16];
	memcpy(p,pBuf,Len);
	int DecodeTail1 = DecodeTail;
	
	DecodeQueue[DecodeTail1]=p;
	/*Max add 2004-04-01 for storing the buffer length*/
	DecodeBufLenQueue[DecodeTail1] = Len;
	DecodeTail1 ++;
	if(DecodeTail1 >= DECODE_QUEUE_SIZE)
		DecodeTail1 = 0;
	
	DecodeTail = DecodeTail1;
	//	PrintDualInt("Head Add",DecodeHead,"Tail",DecodeTail);
}



// return 0 -- success

SINT32	CVideoRender::StartDecoder()
{
	if(decoder)
		return 0;
	
	IMP_UniDecTSC::CreateInstance(
		&decoder,	// IMP_UniDecTSC	**pp,	// return a pointer to CUniDecTSC object
		m_mode,		// SINT8	modeSrc,		// source video format
		0,			// SINT8	modeTSC,		// transcoded video stream format
		NULL		//TMP_StrInfo	*sInfo		// optional pass stream information
								//		= NULL			//   by defualt we use Sync to get stream information
								);
	m_bDecSynced = FALSE;
	
	DecodeHead = DecodeTail = 0;
	
	
	for(int i=0;i < DECODE_QUEUE_SIZE;i++)
	{
		DecodeQueue[i] = NULL;
		DecodeBufLenQueue[i] = 0;
		VideoPts[i] = 0;
	}
	
	m_DecodeThreadState = 1;
	m_hDecodeThread = CreateThread(NULL,0,DecodeThreadProc,this,0,NULL);
	
	return 0;
}

void	CVideoRender::StopDecoder()
{
	m_DecodeThreadState = 0;
	WaitForSingleObject(m_hDecodeThread,INFINITE);
}

/*Max modified 2004-04-01 for storing the buffer length*/
//SINT32	CVideoRender::DecodeOneFrame(UINT8 *pInBuf, UINT8 *pOutBuf)
SINT32	CVideoRender::DecodeOneFrame(UINT8 *pInBuf, UINT8 *pOutBuf, UINT32 Len, TMP_FrmInfo *frmInfo)
{
	if(decoder==0)
		return 1;
	
	//	PrintBinary("Decode", pInBuf, 16);
	
	if(pInBuf[0] != 0x00 || pInBuf[1] != 0x00 || pInBuf[2] != 0x01)
		return 2;
	
	unsigned int Head = *((unsigned int *)pInBuf);
	int SyncLen=0;
#if 0
    /*Max add for Go-server changing the capture resolution*/
	{
		char s[100];
		sprintf(s, "\nHead is %x\n", Head);
		OutputDebugString(s);
	}
#endif
	if(m_bDecSynced)
	{
		if((Head == 0x00010000) || (Head == 0xB0010000))
		{
            TMP_StrInfo	StrInfo;
			SINT32 r = decoder->Sync(&StrInfo,NULL,NULL,&SyncLen,pInBuf);
			
            if ((m_width != StrInfo.cols*16) || (m_height != StrInfo.rows*16))
            {
				
                m_width = StrInfo.cols*16;
				m_height = StrInfo.rows*16;
				
				decoder->Release();
				IMP_UniDecTSC::CreateInstance(
					&decoder,	// IMP_UniDecTSC	**pp,	// return a pointer to CUniDecTSC object
					m_mode,		// SINT8	modeSrc,		// source video format
					0,			// SINT8	modeTSC,		// transcoded video stream format
					NULL		//TMP_StrInfo	*sInfo		// optional pass stream information
					//		= NULL			//   by defualt we use Sync to get stream information
					);
				DestroyVideoWindow();
				m_bDecSynced = FALSE;
				
            }
		}
	}
	/*Max add end*/
	
	if(!m_bDecSynced)
	{
		if(Head != 0x00010000 && Head != 0xB3010000 && Head != 0xB0010000)
			return 2;
		
		TMP_StrInfo	StrInfo;
		SINT32 r = decoder->Sync(&StrInfo,NULL,NULL,&SyncLen,pInBuf);
		//		char ssss[256];
		//		sprintf(ssss,"width : %d, height : %d",StrInfo.cols*16, StrInfo.rows*16);
		//		PrintString(ssss);
		
		if(r != 0)
			return 2;
		
		m_width = StrInfo.cols*16;
		m_height = StrInfo.rows*16;
		
		CreateVideoWindow();
		
		TMP_PPrcSet	ppSet;
		ppSet.pproc = DPP_DBLK;
		ppSet.rcLeft = 0;
		ppSet.rcTop = 0;
		ppSet.rcWidth = 4096;
		ppSet.rcHeight = 4096;
		
		decoder->UniDecTSCSetup(m_ddraw, FP_AUTO, 0, &ppSet);
		m_bDecSynced = TRUE;
		
		/*2005-03-04 add for fixing bug 1161*/
		memmove(pInBuf, pInBuf + SyncLen, Len - SyncLen);
		return SyncLen;
		/*2005-03-04 add end*/
	}	
	
	/*Max add 2004-04-01 for adding an parameter*/
	//decoder->FRMDecTSC(NULL, pOutBuf, m_pitch, NULL, NULL, NULL, NULL, pInBuf+SyncLen);
	decoder->FRMDecTSC(NULL, pOutBuf, m_pitch, frmInfo, Len - SyncLen, NULL, NULL, NULL, pInBuf+SyncLen);
	
	unsigned char OSDText[256];
	decoder->GetCurrentOnScreenDisplayText(OSDText);
	PrintString((char *)OSDText);
	
	return 0;
}


void CVideoRender::SetInputClock(input_clock_t *pClock)
{
	m_pInputClock = pClock;
}
