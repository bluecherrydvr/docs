// InfoS6010StreamIdx.h: interface for the CInfoS6010StreamIdx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INFOS6010STREAMIDX_H__55B5A2B3_6ED5_487E_A31D_C36AF55AE288__INCLUDED_)
#define AFX_INFOS6010STREAMIDX_H__55B5A2B3_6ED5_487E_A31D_C36AF55AE288__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct ST_MPEG4_FILE_IDX
{
	DWORD isKeyFrame	:1,
		  nScale	:4,
		  idxCh	:5,
		  bMotionExist :1,
		  nInterval :10,
		  reserved	:11;

	DWORD idxFramePos;
	DWORD szFrame;
	DWORD nTimeCode;
};

#define IDX_DEF_VIDEO	IDX_VIDEO_NTSC

class CInfoS6010StreamIdx  
{
public:
	CInfoS6010StreamIdx();
	virtual ~CInfoS6010StreamIdx();

	BOOL MakeIndex (CString strMPEG4FileName);
	__inline DWORD GetFramePos (int idxCh, DWORD idxFrm)
		{	return m_bufFileIdx[idxCh][idxFrm].idxFramePos;	}
	__inline DWORD GetFrameSize (int idxCh, DWORD idxFrm)
		{	return m_bufFileIdx[idxCh][idxFrm].szFrame;	}
	__inline DWORD GetTimeCode (int idxCh, DWORD idxFrm)
		{	return m_bufFileIdx[idxCh][idxFrm].nTimeCode;	}
	__inline BOOL IsKeyFrame (int idxCh, DWORD idxFrm)
		{	return m_bufFileIdx[idxCh][idxFrm].isKeyFrame;	}
	__inline DWORD GetMaxFrameNum (int idxCh)
		{	return m_nNumValidFrame[idxCh];	}
	__inline DWORD GetScale (int idxCh, DWORD idxFrm)
		{	return m_bufFileIdx[idxCh][idxFrm].nScale;	}
	__inline DWORD GetInterval (int idxCh, DWORD idxFrm)
		{	return m_bufFileIdx[idxCh][idxFrm].nInterval;	}
	__inline BOOL IsMotionExist (int idxCh, DWORD idxFrm)
		{	return m_bufFileIdx[idxCh][idxFrm].bMotionExist;	}
	__inline BOOL GetVideoType (int idxCh)
		{	return m_bufIdxVideo[idxCh];	}
	__inline BOOL IsInterlace (int idxCh)	// (= Is Frame Mode?)
	{	return ((m_bufFileIdx[idxCh][0].nScale >= 8) ? 1 : 0);	}
	__inline int GetMSPF (int idxCh)
	{	return ((m_bufFileIdx[idxCh][0].nScale < 8)
		? (1000000 *(m_bufFileIdx[idxCh][0].nInterval +1) /60)
		: (1000000 *(m_bufFileIdx[idxCh][0].nInterval +1) /30));	}

private:
	int m_nNumTotFrame[MAX_ENC_CHANNEL];	// include first P frame number
	int m_nNumValidFrame[MAX_ENC_CHANNEL];	// exclude first P frame number
	ST_MPEG4_FILE_IDX *m_bufFileIdx[MAX_ENC_CHANNEL];
	unsigned char m_bufIdxVideo[MAX_ENC_CHANNEL];

	int m_bMotionEx;
	int m_nAlign;

	void Init ();
	void Free ();

	DWORD GetAlignedSzFrame (DWORD szFrameRAW, int nAlign);
	BOOL MakeIndexFromIndexBuf (CString strMPEG4FileName);
};

#endif // !defined(AFX_INFOS6010STREAMIDX_H__55B5A2B3_6ED5_487E_A31D_C36AF55AE288__INCLUDED_)
