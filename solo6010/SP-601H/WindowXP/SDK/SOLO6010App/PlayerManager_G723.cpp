// PlayerManager_G723.cpp: implementation of the CPlayerManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "solo6010app.h"
#include "PlayerManager.h"
#include "WndPlayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// When buffering, G723 data is decoded and bufferinged.
// When "Decode_G723" function is executed, wave data is added to wave device.

BOOL Decode_G723 (CPlayerManager *pPlayer);

#define PM_G723_INVALID_CH			-1
#define MAX_WRITTEN_HEADER_NUM		32

static void OpenWaveOutDevice (CPlayerManager *pPlayer, int idxSamplingRate);
static void CloseWaveOutDevice (CPlayerManager *pPlayer);
BOOL CALLBACK CbFn_WaveOut (HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

void CPlayerManager::InitDec_G723 ()
{
	m_hWaveOut = NULL;
}

void CPlayerManager::DestroyDec_G723 ()
{
	CloseFile_G723 ();
}

BOOL CPlayerManager::OpenFile_G723 ()
{
	m_bufIdxStreamFormat[PM_FILE_AUDIO] = PM_STREAM_FORMAT_G723;
	DecodeAudio = Decode_G723;

	int i;

	m_szG723LineBuf = 0;
	for (i=0; i<NUM_G723_CH; i++)
	{
		m_bufOffsetG723ChDataInLineBuf[i] = PM_G723_INVALID_CH;
		if (m_bufFileHeader[PM_FILE_AUDIO].G723.setRec.bufBEnc[i] == TRUE)
		{
			m_bufOffsetG723ChDataInLineBuf[i] = SZ_G723_FRM_HEADER +m_szG723LineBuf *SZ_G723_1_FRM;

			m_szG723LineBuf++;
		}
	}
	m_szG723LineBuf *= SZ_G723_1_FRM;
	m_szG723LineBuf += SZ_G723_FRM_HEADER;
	if (m_szG723LineBuf == SZ_G723_FRM_HEADER)
	{
		return FALSE;
	}
	m_bufNumTotFrm[PM_FILE_AUDIO] = DWORD((m_bufFileHeader[PM_FILE_AUDIO].G723.posIndex -sizeof(HEADER_RECORD_FILE)) /m_szG723LineBuf);

	// Make index[S]
	// Make index[S]

	WAVEFORMATEX stWFEx;

	// waveout device open[S]
	ZeroMemory (&stWFEx, sizeof(WAVEFORMATEX));
	stWFEx.cbSize = sizeof(WAVEFORMATEX);

	stWFEx.wFormatTag = WAVE_FORMAT_PCM;
	stWFEx.nChannels = 1;
	if (m_bufFileHeader[PM_FILE_AUDIO].G723.setRec.idxSampRate == G723_SAMPLING_RATE_8K)
	{
		stWFEx.nSamplesPerSec = 8000;
	}
	else	// G723_SAMPLING_RATE_16K
	{
		stWFEx.nSamplesPerSec = 16000;
	}
	stWFEx.wBitsPerSample = 16;
	stWFEx.nBlockAlign = stWFEx.nChannels *(stWFEx.wBitsPerSample /8);
	stWFEx.nAvgBytesPerSec = stWFEx.nSamplesPerSec *2;

	DWORD nRet;

	if (m_hWaveOut != NULL)
	{
		CloseFile_G723 ();
	}
	nRet = waveOutOpen (&m_hWaveOut, WAVE_MAPPER, &stWFEx, (DWORD)CbFn_WaveOut, (DWORD)this, CALLBACK_FUNCTION);

	if (nRet != MMSYSERR_NOERROR)
	{
		m_hWaveOut = NULL;

		AddReport ("ERROR: OpenFile_G723(): waveOutOpen()");
		return FALSE;
	}
	// waveout device open[E]

	// prepare headers[S]
	for(i=0; i<NUM_AUDIO_BUFFERING_FRM; i++)
	{
		ZeroMemory (&m_hdrWave[i], sizeof(WAVEHDR));

		m_hdrWave[i].lpData = (LPSTR)&iBuffStat[PM_FILE_AUDIO].bufBuffering[i *SZ_BUF_IN_BYTE_G723_1_FRM_DEC];
		m_hdrWave[i].dwBufferLength = SZ_BUF_IN_BYTE_G723_1_FRM_DEC;
		m_hdrWave[i].dwFlags = 0;
		m_hdrWave[i].dwLoops = 0;

		nRet = waveOutPrepareHeader (m_hWaveOut, &m_hdrWave[i], sizeof(WAVEHDR));

		if (nRet != MMSYSERR_NOERROR)
		{
			waveOutClose (m_hWaveOut);
			m_hWaveOut = NULL;

			AddReport ("ERROR: OpenFile_G723(): waveOutPrepareHeader() [%d]", i);
			return FALSE;
		}
	}
	// prepare headers[E]

	return TRUE;
}

void CPlayerManager::CloseFile_G723 ()
{
	if (m_hWaveOut != NULL)
	{
		waveOutReset (m_hWaveOut);

		DWORD nRet;
		int i;

		// unprepare header[S]
		for(i=0; i<NUM_AUDIO_BUFFERING_FRM; i++)
		{
			nRet = waveOutUnprepareHeader (m_hWaveOut, &m_hdrWave[i], sizeof(WAVEHDR));
			if (nRet != MMSYSERR_NOERROR)
			{
				AddReport ("ERROR: CloseFile_G723(): waveOutUnprepareHeader() [%d]: %d", i, nRet);
			}
		}
		// unprepare header[E]

		waveOutClose (m_hWaveOut);

		m_hWaveOut = NULL;
	}
}

BOOL Decode_G723 (CPlayerManager *pPlayer)
{
	int idxCh = pPlayer->m_bufChFromWnd[pPlayer->GetSelWnd ()];

	if (pPlayer->m_bufOffsetG723ChDataInLineBuf[idxCh] == PM_G723_INVALID_CH)
	{	// invalid channel
		return FALSE;
	}

	DWORD tmpIdxFrmFileWrite, tmpIdxFrmDecRead, tmpPosDecRead;

	tmpIdxFrmFileWrite = pPlayer->iBuffStat[PM_FILE_AUDIO].idxFrmFileWrite;
	tmpIdxFrmDecRead = pPlayer->iBuffStat[PM_FILE_AUDIO].idxFrmDecRead;

	tmpPosDecRead = tmpIdxFrmDecRead %NUM_AUDIO_BUFFERING_FRM;

	MMRESULT mmRes;
	DWORD i, numWrite, posHdr;

	numWrite = MAX_WRITTEN_HEADER_NUM -(pPlayer->m_bufPosCurFrm[PM_FILE_AUDIO] -tmpIdxFrmDecRead);
	if (pPlayer->m_bufPosCurFrm[PM_FILE_AUDIO] +numWrite > tmpIdxFrmFileWrite)
	{
		numWrite = tmpIdxFrmFileWrite -pPlayer->m_bufPosCurFrm[PM_FILE_AUDIO];
	}
	for (i=0; i<numWrite; i++)
	{
		posHdr = (pPlayer->m_bufPosCurFrm[PM_FILE_AUDIO] +i) %NUM_AUDIO_BUFFERING_FRM;

		pPlayer->m_hdrWave[posHdr].dwFlags &= ~WHDR_DONE;
		mmRes = waveOutWrite (pPlayer->m_hWaveOut, &pPlayer->m_hdrWave[posHdr], sizeof(WAVEHDR));

		if (mmRes != MMSYSERR_NOERROR)
		{
			break;
		}
	}
	pPlayer->m_bufPosCurFrm[PM_FILE_AUDIO] += numWrite;

	return FALSE;
}

void CPlayerManager::Buffering_G723 ()
{
	int idxCh = m_bufChFromWnd[GetSelWnd ()];

	if (m_bufOffsetG723ChDataInLineBuf[idxCh] == PM_G723_INVALID_CH)
	{	// invalid channel
		return;
	}

	DWORD tmpIdxFrmFileWrite, tmpIdxFrmDecRead, tmpPosFileWrite, tmpPosDecRead;

	tmpIdxFrmDecRead = iBuffStat[PM_FILE_AUDIO].idxFrmDecRead;
	tmpIdxFrmFileWrite = iBuffStat[PM_FILE_AUDIO].idxFrmFileWrite;

	tmpPosDecRead = tmpIdxFrmDecRead %NUM_AUDIO_BUFFERING_FRM;
	tmpPosFileWrite = tmpIdxFrmFileWrite %NUM_AUDIO_BUFFERING_FRM;

	if (tmpIdxFrmDecRead >= m_bufNumTotFrm[PM_FILE_AUDIO])
	{	// end of stream
		return;
	}

	DWORD numReadFrm;

	numReadFrm = (tmpPosDecRead +NUM_AUDIO_BUFFERING_FRM -tmpPosFileWrite -1) %NUM_AUDIO_BUFFERING_FRM;
	if (numReadFrm > m_bufNumTotFrm[PM_FILE_AUDIO] -tmpIdxFrmFileWrite)
	{
		numReadFrm = m_bufNumTotFrm[PM_FILE_AUDIO] -tmpIdxFrmFileWrite;
	}
	if (numReadFrm == 0)
	{	// no need to buffering
		return;
	}

	BYTE *bufTmpG723 = new BYTE[numReadFrm *m_szG723LineBuf];
	DWORD numRead;

	ReadFile (m_bufHFile[PM_FILE_AUDIO], bufTmpG723, numReadFrm *m_szG723LineBuf, &numRead, NULL);

	DWORD i;

	for (i=0; i<numReadFrm; i++)
	{
		m_decG723.Decode (&bufTmpG723[i *m_szG723LineBuf +m_bufOffsetG723ChDataInLineBuf[idxCh]], SZ_G723_1_FRM,
			(short *)&iBuffStat[PM_FILE_AUDIO].bufBuffering[((tmpPosFileWrite +i) %NUM_AUDIO_BUFFERING_FRM) *SZ_BUF_IN_BYTE_G723_1_FRM_DEC],
			NUM_SAMPLE_G723_1_FRM_DEC);
	}

	delete[] bufTmpG723;

	iBuffStat[PM_FILE_AUDIO].idxFrmFileWrite += numReadFrm;
}

BOOL CPlayerManager::Dec1Frame_G723 ()
{
	return TRUE;
}

void CPlayerManager::SetFramePos_G723 (DWORD posFrm)
{
	if (m_hWaveOut != NULL)
	{
		waveOutReset (m_hWaveOut);
	}

	DWORD timeTotal, timeCur;

	timeTotal = m_bufVideoIndex[GetTotalVideoFrameNum () -1].timeMSec -m_bufVideoIndex[0].timeMSec;
	timeCur = GetVideoElapsedTime (posFrm);
	
	m_bufPosCurFrm[PM_FILE_AUDIO] = DWORD(double(timeCur) *m_bufNumTotFrm[PM_FILE_AUDIO] /timeTotal);

	iBuffStat[PM_FILE_AUDIO].idxFrmDecRead = m_bufPosCurFrm[PM_FILE_AUDIO];
	iBuffStat[PM_FILE_AUDIO].idxFrmFileWrite = m_bufPosCurFrm[PM_FILE_AUDIO];

	SetFilePointer (m_bufHFile[PM_FILE_AUDIO],
		sizeof(HEADER_RECORD_FILE) +m_bufPosCurFrm[PM_FILE_AUDIO] *m_szG723LineBuf,
		NULL, FILE_BEGIN);
}

BOOL CALLBACK CbFn_WaveOut (HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	CPlayerManager *pPlayer = (CPlayerManager *)dwInstance;
	
	switch(uMsg)
	{
	case WOM_OPEN:
		break;		
	case WOM_CLOSE:
		break;
	case WOM_DONE:
		pPlayer->iBuffStat[PM_FILE_AUDIO].idxFrmDecRead++;
		break;
	default:
		break;
	}

	return TRUE;
}
