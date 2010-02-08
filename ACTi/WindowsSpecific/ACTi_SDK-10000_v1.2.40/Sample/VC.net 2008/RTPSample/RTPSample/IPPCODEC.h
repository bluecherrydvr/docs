#pragma once
extern "C" __declspec(dllexport) int XCInit();
extern "C" __declspec(dllexport) void XCExit(int nID);
extern "C"  __declspec(dllexport) LPBITMAPINFO XDecode(int nID, int nW, int nH, BYTE* pInBuf, LONG Len, HWND hWnd = NULL, PRECT pRt = NULL, bool bReSync = false);
