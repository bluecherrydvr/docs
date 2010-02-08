// VideoRender.h: interface for the CVideoRender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIDEORENDER_H__238CA1CE_A601_43EF_B01C_97C0E0C03C50__INCLUDED_)
#define AFX_VIDEORENDER_H__238CA1CE_A601_43EF_B01C_97C0E0C03C50__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SDK7007.h"
#include <MP_UniDecTSC.h>
#include <VideoPlayer.h>

#include "PtsClock.h"

#define DECODE_QUEUE_SIZE	1024

class CVideoRender  
{
public:
	CVideoRender(CWnd *pParent, int ForceRGB);
	virtual ~CVideoRender();

//	int Initialize();
	int Initialize(BOOL bAvSyncEnabled = FALSE);
	void Uninitialize();

	void SourceInput(unsigned char *pBuf, int BufLen);
	void SourceInput(unsigned char *pBuf, int BufLen, UINT64 pts);

	void 	DoInDecodeThread();

	void GetVideoFirstTime(UINT64& firstPlayTime, UINT64& firstPts);
	void 	SetInputClock(input_clock_t *pClock);

//public:
protected:

	int			m_player;

	int			m_ddraw;
	LONG		m_pitch;

	int m_width, m_height;
	int m_mode;
	CWnd *m_pParent;
	int m_ForceRGB;

	IMP_UniDecTSC *decoder;
	SINT8   m_bDecSynced;
	unsigned char *DecodeQueue[DECODE_QUEUE_SIZE];
	/*Max added 2004-04-01 for storing the buffer length*/
	unsigned int DecodeBufLenQueue[DECODE_QUEUE_SIZE];
	int		DecodeHead, DecodeTail;

	int		m_DecodeThreadState;
	HANDLE  m_hDecodeThread;

	UINT64 VideoPts[DECODE_QUEUE_SIZE];
	UINT64	m_FirstVideoPts;
	UINT64	m_FirstPlayTime;
	UINT64	m_LastVideoPts;
	BOOL m_bAvSyncEnabled;
	UINT64	m_FirstBPts;
	UINT64   m_FirstBPlayTime;

	input_clock_t *m_pInputClock;

	void	CreateVideoWindow();
	void	DestroyVideoWindow();

	SINT32	StartDecoder();
	void 	StopDecoder();
	/*Max modified 2004-04-01 for adding a parameter*/
	//SINT32  DecodeOneFrame(UINT8 *pInBuf, UINT8 *pOutBuf);
	SINT32  DecodeOneFrame(UINT8 *pInBuf, UINT8 *pOutBuf, UINT32 Len, TMP_FrmInfo *frmInfo);

	void	AddToDecodeQueue(unsigned char *pBuf, int Len);

};

#endif // !defined(AFX_VIDEORENDER_H__238CA1CE_A601_43EF_B01C_97C0E0C03C50__INCLUDED_)
