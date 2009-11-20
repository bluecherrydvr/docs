#include "stdafx.h"
#include "PlayDvrFCD.h"
#include "PlayDvrFCDDlg.h"
#include "WaveOut.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

void CALLBACK WaveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);
CWaveOut	*g_pWaveOut;

//================================================================================================================================//
void CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance,  DWORD dwParam1, DWORD dwParam2)
{
	int	*freeBlockCounter = (int*)dwInstance;

	if(uMsg != WOM_DONE)
		return;

	EnterCriticalSection(&g_pWaveOut->m_WaveCriticalSection);
	(*freeBlockCounter)++;
	LeaveCriticalSection(&g_pWaveOut->m_WaveCriticalSection);
}

//================================================================================================================================//
CWaveOut::CWaveOut(CWnd* pParent)
{
	m_pDvrFCDDlg = (CDvrFCDDlg *)pParent;
	g_pWaveOut = this;
	m_hWaveOut = NULL;
	m_bInWriteAudioBlock = FALSE;	
}

//================================================================================================================================//
CWaveOut::~CWaveOut()
{
	
}

//================================================================================================================================//
BOOL CWaveOut::WaveOutOpen()
{
	WAVEFORMATEX	wfx;
	
	m_WaveFreeBlockCount = WAVE_BLOCK_COUNT;
	
	wfx.wFormatTag      = WAVE_FORMAT_PCM;
	wfx.wBitsPerSample  = 16;     // sample size /
	wfx.nChannels       = 1;      // channels    /
	wfx.cbSize          = 0;      // size of _extra_ info /
	wfx.nBlockAlign     = 2;
	wfx.nSamplesPerSec = 8000; //48000;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign*wfx.nSamplesPerSec;
	
	InitializeCriticalSection(&m_WaveCriticalSection);
	
	if(waveOutOpen(&m_hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)waveOutProc, (DWORD_PTR)&m_WaveFreeBlockCount, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) 
	{
		OutputDebugString("waveOutOpen fail\n");
		return FALSE;
	}	
	m_WaveBlocks = AllocateBlocks(WAVE_BLOCK_SIZE, WAVE_BLOCK_COUNT);
	if(!m_WaveBlocks)
	{
		OutputDebugString("AllocateBlocks fail\n");
		WaveOutClose();
		return FALSE;
	}	
	m_WaveCurrentBlock   = 0;

	return TRUE;
}

//================================================================================================================================//
void CWaveOut::WaveOutClose()
{
	int			i;

	if(m_hWaveOut)
	{
		while(m_WaveFreeBlockCount < WAVE_BLOCK_COUNT)
			Sleep(10);

		for(i=0; i<m_WaveFreeBlockCount; i++)
		{
			if(m_WaveBlocks[i].dwFlags & WHDR_PREPARED)
				waveOutUnprepareHeader(m_hWaveOut, &m_WaveBlocks[i], sizeof(WAVEHDR));
		}
		waveOutClose(m_hWaveOut);
		m_hWaveOut = NULL;		
	}	
	DeleteCriticalSection(&m_WaveCriticalSection);
	FreeBlocks(m_WaveBlocks);
}

//================================================================================================================================//
BOOL CWaveOut::WriteAudioBlock(BYTE *pblock, DWORD size)
{
	WAVEHDR	*current;
	int		remain;
	
	m_bInWriteAudioBlock = TRUE;

	if((!m_WaveBlocks) || (!m_hWaveOut))
	{
		m_bInWriteAudioBlock = FALSE;
		return FALSE;
	}
	current = &m_WaveBlocks[m_WaveCurrentBlock];
	
	while(size > 0) 
	{
		if(current->dwFlags & WHDR_PREPARED)
			waveOutUnprepareHeader(m_hWaveOut, current, sizeof(WAVEHDR));

		if(size < (int)(WAVE_BLOCK_SIZE - current->dwUser))
		{
			memcpy(current->lpData + current->dwUser, pblock, size);
			current->dwUser += size;
			break;
		}
		
		remain = WAVE_BLOCK_SIZE - current->dwUser;
		memcpy(current->lpData + current->dwUser, pblock, remain);
		size -= remain;
		pblock += remain;
		current->dwBufferLength = WAVE_BLOCK_SIZE;
		
		waveOutPrepareHeader(m_hWaveOut, current, sizeof(WAVEHDR));
		waveOutWrite(m_hWaveOut, current, sizeof(WAVEHDR));
		
		EnterCriticalSection(&m_WaveCriticalSection);
		m_WaveFreeBlockCount--;
		LeaveCriticalSection(&m_WaveCriticalSection);
		
		while(!m_WaveFreeBlockCount)
			Sleep(5);

		m_WaveCurrentBlock++;
		m_WaveCurrentBlock %= WAVE_BLOCK_COUNT;
		
		current = &m_WaveBlocks[m_WaveCurrentBlock];
		current->dwUser = 0;
	}
	m_bInWriteAudioBlock = FALSE;

	return TRUE;
}

//================================================================================================================================//
WAVEHDR* CWaveOut::AllocateBlocks(int size, int count)
{
	WAVEHDR		*pBlocks;
	BYTE		*pBuffer;
	int			i;
	
	DWORD totalBufferSize = (size + sizeof(WAVEHDR)) * count;
	
	if((pBuffer = (BYTE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, totalBufferSize)) == NULL) 
	{
		OutputDebugString("CWaveOut::Memory allocation error\n");
		return NULL;
	}
	
	pBlocks = (WAVEHDR*)pBuffer;
	pBuffer += sizeof(WAVEHDR) * count;

	for(i = 0; i < count; i++)
	{
		pBlocks[i].dwBufferLength = size;
		pBlocks[i].lpData = (char *)pBuffer;
		pBuffer += size;
	}	
	return pBlocks;
}

//================================================================================================================================//
void CWaveOut::FreeBlocks(WAVEHDR* blockArray)
{
	HeapFree(GetProcessHeap(), 0, blockArray);
}
