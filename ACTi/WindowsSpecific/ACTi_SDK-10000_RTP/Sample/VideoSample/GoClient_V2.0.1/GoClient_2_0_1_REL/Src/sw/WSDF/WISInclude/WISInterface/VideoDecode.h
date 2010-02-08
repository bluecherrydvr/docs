// VideoDecode.h: interface for the CVideoDecode class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _VIDEODECODE_H_
#define _VIDEODECODE_H_

#include <WSDF.H>
#include <Struct.h>
#include <MultiMedia.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// The functionality of this class is to decode a compressed stream in MPEG1/2/4, H.263 or WIS GO format 
// into raw video stream in RGB24 or UYVY format

class IVideoDecode  
{
public:

	// create an instance for this interface
	// 
	// parameter:
	//      pp	     -- pointer to a pointer which will receive a pointer to the new instance
	// return:	
	//		SUCCESS	   -- success
	//		ERR_MEMORY -- no enough memory to create an new instance
	//   	ERR_DEVICE -- device can not be opened
	STATIC SINT32 CreateInstance(void **pp);

	// release the instance for this interface
	// 
	// parameter:
	//		none
	// return:	
	//		none
	void Release();

	// initialize the decoder by a stream head and GOP head, the decoder will extract the neccessary information from this head
	//
	// parameter:
	//		mode	-- compress mode
	//		Head	-- a stream that includes a stream head or GOP head
	//      HeadLen -- [Out] stream head or GOP head length, this length of stream should be skipped for subsequent call of DecodeOneFrame
	// return:	
	//		0      -- success
	//		none 0 -- fail
	virtual SINT32	InitDecode(SINT32 mode, UINT8 *Head, SINT32 *HeadLen) PURE;

	// set format and pitch for decoded stream
	//
	// parameter:
	//		out_format -- output format, can be DDRAW_RGB24 and DDRAW_UYVU
	//      pitch      -- pitch (total bytes for one row)
	// return:	
	//		none
	virtual	void	SetDecodeParam(UINT32 out_format, UINT32 pitch) PURE;

	// set post-processing level, the default post-processing level is DPP_NULL
	//
	// parameter:
	//		pp_level -- post-process level, can be DPP_NULL, DPP_DBLK(de-block), DPP_DBLKDRNG(de-block and de-ring)
	// return:	
	//		none
	virtual	void	SetPPLevel(UINT32 pp_level) PURE;

	// decode one frame
	//
	// parameter:
	//		pInBuf   -- encoded frame
	//      pOutBuf  -- [Out] decoded frame
	//		FrmInfo  -- [Out] information of current frame, refer to "TMP_FrmInfo"
	//      decodeTS --	[Out] decode timestamp of current frame
	// return:	
	//		0      -- success
	//		none 0 -- fail
	virtual SINT32	DecodeOneFrame(UINT8 *pInBuf, UINT8 *pOutBuf, TMP_FrmInfo *FrmInfo, REAL64  *decodeTS) PURE;

	// extract the offset in bit from the head of a frame of each macro block, this function is helpful for sending stream via rtp protocol
	// which needs to split one frame into several small packets at the boundary of macro block.
	//
	// parameter:
	//		pInBuf -- encoded stream
	//		Offset -- [Out] offset in bit from the head of a frame of each macro block
	// return:	
	//		none
	virtual void	GetBoundaryInfo(UINT8 *pInBuf, SINT32 Offset[]) PURE;

};

#endif 

