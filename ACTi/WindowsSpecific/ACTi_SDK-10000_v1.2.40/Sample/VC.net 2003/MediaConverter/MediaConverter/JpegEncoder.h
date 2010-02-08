#ifndef _JPEG_ENCODER_H
#define _JPEG_ENCODER_H

namespace JPEGENCODER
{
	typedef struct 
	{
		BYTE *pBuffer;
		int nBufferLen;
	}ROE;

	extern "C" __declspec(dllexport) HANDLE XEJpegInit();
	extern "C" __declspec(dllexport) void XEJpegEExit( HANDLE h );
	extern "C"  __declspec(dllexport) ROE* XJpegEncode( HANDLE h, int nInW, int nInH,
		BYTE* pInBuf, int Len, int nQuality = 40, bool bReinit = false );

	inline HANDLE XEInit()
	{
		return XEJpegInit();
	}

	inline void XEExit( HANDLE h )
	{
		XEJpegEExit( h);
	}

	inline ROE* XEncode( HANDLE h, int nW, int nH, int nFPS, int nBitrate, BYTE* pInBuf, int Len, bool bReInit = false )
	{
		return XJpegEncode( h, nW, nH, pInBuf, Len);
	}

};
#endif
