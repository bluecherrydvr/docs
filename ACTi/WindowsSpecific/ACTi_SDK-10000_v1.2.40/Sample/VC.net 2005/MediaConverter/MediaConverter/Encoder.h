#pragma once
#include "vfw.h"
#include "atlimage.h"

///////////////////////////////////////////
// Decoder by DLLs
///////////////////////////////////////////
//#define DECODER_IPP				"IPPCODEC"
#define DECODER_XVID			_T("XVIDCODEC")
//#define DECODER_WIS				"WISCODEC"
//#define DECODER_P51				"P51CODEC"
#define DECODER_MJPEG			_T("MJPEGCODEC")
#define DECODER_IH264			_T("IH264CODEC")

// CODEC ADAPTER
typedef int				(* XXCInit			)();
typedef void			(* XXCExit			)( int );
typedef LPBITMAPINFO	(* XXDecode			)( int, int, int, BYTE*, LONG, HWND, PRECT, bool );

class CXEncoder
{
protected:
	///////////////////////////////////////////
	// Codec Intreface
	///////////////////////////////////////////
	HINSTANCE m_XVID_hinstDecoderAdapter;
	HINSTANCE m_MJPG_hinstDecoderAdapter;
	HINSTANCE m_H264_hinstDecoderAdapter;

	XXCInit m_XVID_lpfnCInit;
	XXCExit m_XVID_lpfnCExit;
	XXDecode m_XVID_lpfnCDecode;

	XXCInit m_MJPG_lpfnCInit;
	XXCExit m_MJPG_lpfnCExit;
	XXDecode m_MJPG_lpfnCDecode;

	XXCInit m_H264_lpfnCInit;
	XXCExit m_H264_lpfnCExit;
	XXDecode m_H264_lpfnCDecode;

	//
	int m_XVID_nDecoderHandle;
	int m_MJPG_nDecoderHandle;
	int m_H264_nDecoderHandle;

public:
	CXEncoder();
	virtual ~CXEncoder();

	bool InitAllCodec();
	void ReleaseAllCodec();

	void ReInitAllCodec()
	{
		ReleaseAllCodec();
		InitAllCodec();
	}
};

