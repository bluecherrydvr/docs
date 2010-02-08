#pragma once

namespace XVIDDECODER
{
	class __declspec(dllexport) XVIDClass
	{
	public:
		XVIDClass();
		~XVIDClass();
		LPBITMAPINFO XDecode( int nW, int nH, BYTE* pInBuf, LONG Len, HWND hWnd, PRECT pRt, bool bReSync);
	private:
		void *m_pObj;
	};

	extern "C"  __declspec(dllexport) LPBITMAPINFO XDecode(int nID, int nW, int nH, BYTE* pInBuf, LONG Len, HWND hWnd = NULL, PRECT pRt = NULL, bool bReSync = false);
	extern "C" __declspec(dllexport) void XCExit(int nID);
	extern "C" __declspec(dllexport) int XCInit();
}

namespace WISCODER
{
	extern "C" __declspec(dllexport) int XCWISInit();
	extern "C" __declspec(dllexport) void XCWISExit(int nID);
	extern "C"  __declspec(dllexport) LPBITMAPINFO XWISDecode(int nID, int nW, int nH, BYTE* pInBuf, LONG Len, HWND hWnd = NULL, PRECT pRt = NULL, bool bReSync = false);
}

namespace MJPEGDECODER
{
	extern "C" __declspec(dllexport) int XCJInit();
	extern "C" __declspec(dllexport) void XCJExit(int nID);
	extern "C"  __declspec(dllexport) LPBITMAPINFO XJDecode(int nID, int nW, int nH, BYTE* pInBuf, LONG Len, HWND hWnd = NULL, PRECT pRt = NULL, bool bReSync = false);
}

namespace IPPDECODER
{
	extern "C" __declspec(dllexport) int XCIInit();
	extern "C" __declspec(dllexport) void XCIExit(int nID);
	extern "C"  __declspec(dllexport) LPBITMAPINFO XIDecode(int nID, int nW, int nH, BYTE* pInBuf, LONG Len, HWND hWnd = NULL, PRECT pRt = NULL, bool bReSync = false);
}

namespace IH264DECODER
{
	extern "C" __declspec(dllexport) int XCHInit();
	extern "C" __declspec(dllexport) void XCHExit(int nID);
	extern "C"  __declspec(dllexport) LPBITMAPINFO XHDecode(int nID, int nW, int nH, BYTE* pInBuf, LONG Len, HWND hWnd = NULL, PRECT pRt = NULL, bool bReSync = false);
};