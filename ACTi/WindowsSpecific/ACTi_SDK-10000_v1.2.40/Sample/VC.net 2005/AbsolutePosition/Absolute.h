#ifndef __CABSOLUTE_H__
#define __CABSOLUTE_H__

#include "AbsoluteDefine.h"

class CAbsolute
{
public:
	CAbsolute();
	~CAbsolute();
	bool GetMovePos(int nCurrentX, int nCurrentY, int nNewX, int nNewY, int nZoom, int& nPan, int& nTilt);
	bool GetZoomPos(BYTE* pCommand, DWORD& dwCommandLen);
	bool GetPTZPos(BYTE* pCommand, DWORD& dwCommandLen);
	bool GetPTPos(BYTE* pCommand, DWORD& dwCommandLen);
	bool SetZoomPos(BYTE* pCommand, DWORD& dwCommandLen, int nRatio);
	bool SetPTZPos(int nPan, int nTilt, int nRatio, BYTE* pCommand, DWORD& dwCommandLen);
	bool SetPTPos(int nPan, int nTilt, BYTE* pCommand, DWORD& dwCommandLen);
	bool GetZoomStop(BYTE* pCommand, DWORD& dwCommandLen);
	bool GetFocusPos(BYTE* pCommand, DWORD& dwCommandLen);

private:
	bool GetHiLowByte(int nRatio, BYTE& bLow, BYTE& bHi);
};

#endif	// __CABSOLUTE_H__