// G723Coder.h: interface for the CG723Coder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_G723CODER_H__14460277_FE57_4442_B27A_455381353ACC__INCLUDED_)
#define AFX_G723CODER_H__14460277_FE57_4442_B27A_455381353ACC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxwin.h>         // MFC core and standard components


#define _AUDIO_LITTLE_ENDIAN
#define	AUDIO_ENCODING_LINEAR	(3)	/* PCM 2's-complement (0-center) */
struct g72x_state {
	long yl;	/* Locked or steady state step size multiplier. */
	short yu;	/* Unlocked or non-steady state step size multiplier. */
	short dms;	/* Short term energy estimate. */
	short dml;	/* Long term energy estimate. */
	short ap;	/* Linear weighting coefficient of 'yl' and 'yu'. */

	short a[2];	/* Coefficients of pole portion of prediction filter. */
	short b[6];	/* Coefficients of zero portion of prediction filter. */
	short pk[2];	/*
			 * Signs of previous two samples of a partially
			 * reconstructed signal.
			 */
	short dq[6];	/*
			 * Previous 6 samples of the quantized difference
			 * signal represented in an internal floating point
			 * format.
			 */
	short sr[2];	/*
			 * Previous 2 samples of the quantized difference
			 * signal represented in an internal floating point
			 * format.
			 */
	char td;	/* delayed tone detect, new in 1988 version */
};


class CG723Coder  
{
public:
	CG723Coder();
	virtual ~CG723Coder();

	int Encode (short *bufOrgData, int lenOrgData, BYTE *bufEncCode, int lenEncData);
	int Decode (BYTE *bufEncCode, int lenEncCode, short *bufDecData, int lenDecData);

	struct g72x_state m_encState;
	struct g72x_state m_decState;
};

#endif // !defined(AFX_G723CODER_H__14460277_FE57_4442_B27A_455381353ACC__INCLUDED_)
