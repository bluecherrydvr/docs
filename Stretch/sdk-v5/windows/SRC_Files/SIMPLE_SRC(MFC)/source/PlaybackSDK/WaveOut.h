// WaveOut.h: interface for the CWaveOut class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVEOUT_H__4F898491_8111_40D2_834F_1A119DDB5610__INCLUDED_)
#define AFX_WAVEOUT_H__4F898491_8111_40D2_834F_1A119DDB5610__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "mmsystem.h"

#define WAVE_BLOCK_SIZE  16000//20460//61380//60000//61440 //8192, Alice modify, 61440 = MAX_AUDIO_BYTE_PER_LINE*MAX_AUDIO_PACKET_SIZE/AUDIO_STEREO_CH
#define WAVE_BLOCK_COUNT 16

class CAMFR480Dlg;

class CWaveOut  
{
public:
	CWaveOut(CWnd* pParent = NULL);
	virtual ~CWaveOut();

public:
	CDvrFCDDlg			*m_pDvrFCDDlg;

	HWAVEOUT			m_hWaveOut;
	CRITICAL_SECTION	m_WaveCriticalSection;
	WAVEHDR				*m_WaveBlocks;	
	int					m_WaveFreeBlockCount;
	int					m_WaveCurrentBlock;
	BOOL				m_bInWriteAudioBlock;
	
public:
	BOOL				WaveOutOpen();
	void				WaveOutClose();
	WAVEHDR				*AllocateBlocks(int size, int count);
	void				FreeBlocks(WAVEHDR* blockArray);
	BOOL				WriteAudioBlock(BYTE *pblock, DWORD size);	
};

#endif // !defined(AFX_WAVEOUT_H__4F898491_8111_40D2_834F_1A119DDB5610__INCLUDED_)