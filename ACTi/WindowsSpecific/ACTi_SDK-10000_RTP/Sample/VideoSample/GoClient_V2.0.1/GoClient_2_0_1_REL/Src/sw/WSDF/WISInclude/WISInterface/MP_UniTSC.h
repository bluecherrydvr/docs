
#ifndef	_UNITSC_H_
#define	_UNITSC_H_

#include	<windows.h>
#include	<memory.h>
#include	<assert.h>

#include	"WSDF.h"
#include	"MultiMedia.h"
/****************************************************************
// Public Data Structures
****************************************************************/

#define IN
#define OUT

struct FrameBuffer
{
	unsigned char* _bfr;
	unsigned int   _bfr_len;
};

/**	SECTION - interface functions' declaration for C++
 */

class IMP_UniTSC
{
	public:

	// constructor
	STATIC	SINT32	CreateInstance(
				OUT IMP_UniTSC	**pp,	// return a pointer to CUniDecTSC object
				IN SINT8 compress_mode = MPEG4,
				IN SINT8 modeWIS = 0, 
				IN SINT8 trans_option = 0
				);
	// return EFuncReturn ('WSDF.H')

	// distructor
	void	Release();

	virtual SINT32 FrameTSC(	IN struct FrameBuffer* pGoSourceFrame, 
					OUT struct FrameBuffer* pTSCFrame, 
					OUT double* decodeTS, 
					OUT TMP_FrmInfo* pFrameInfo) = 0;
};

#endif
