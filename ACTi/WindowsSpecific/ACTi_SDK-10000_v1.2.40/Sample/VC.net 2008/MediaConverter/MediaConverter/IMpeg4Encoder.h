#pragma once

namespace MPEG4ENCODER
{
typedef struct 
{
	BYTE *pMpeg4Buffer;
	int	nBufferLen;
}ROE;


extern "C" __declspec(dllexport) HANDLE XEInit();
extern "C" __declspec(dllexport) void XEExit( HANDLE h );
extern "C" __declspec(dllexport) ROE* XEncode( HANDLE h, int nW, int nH, int nFPS, int nBitrate, BYTE* pInBuf, int Len, bool bReInit = false );
};
