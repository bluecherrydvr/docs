// DooMPEG4ToAVI.h: interface for the CDooMPEG4ToAVI class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOOMPEG4TOAVI_H__CD582F44_049E_46F2_B1F7_A42D07B3BB7C__INCLUDED_)
#define AFX_DOOMPEG4TOAVI_H__CD582F44_049E_46F2_B1F7_A42D07B3BB7C__INCLUDED_

#include "windows.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDooMPEG4ToAVI  
{
public:
	CDooMPEG4ToAVI();
	virtual ~CDooMPEG4ToAVI();

	int StartConvert (unsigned int szH, unsigned int szV, unsigned char *bufStartFrm, unsigned int szStartFrm);
	int AddFrame (unsigned char *bufFrm, unsigned int szFrm);
	int AddDummyFrame ();
	int EndConvertFPSx100 (const char *strDestFileName, unsigned long numFPSx100 = 30 *100);
	int EndConvertMSPF (const char *strDestFileName, unsigned long numMSPF = 33333);

private:
	unsigned int m_numFrm;
	unsigned int m_szH;
	unsigned int m_szV;
	unsigned long m_numMSPF;

	unsigned int m_szFileStream;
	unsigned int m_idxNextOffset;

	HANDLE m_hStreamFile;
	HANDLE m_hIdxFile;

	void AddHeader ();
};

#endif // !defined(AFX_DOOMPEG4TOAVI_H__CD582F44_049E_46F2_B1F7_A42D07B3BB7C__INCLUDED_)
