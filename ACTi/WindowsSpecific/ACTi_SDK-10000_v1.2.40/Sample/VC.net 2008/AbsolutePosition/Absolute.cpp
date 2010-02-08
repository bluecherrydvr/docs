#include "stdafx.h"
#include "Absolute.h"

CAbsolute::CAbsolute()
{
}

CAbsolute::~CAbsolute()
{
}

bool CAbsolute::GetMovePos(int nCurrentX, int nCurrentY, int nNewX, int nNewY, int nZoom, int& nPan, int& nTilt)
{
	switch(nZoom)
	{
	case 1:
		{
		}
		return true;
	case 2:
		{
		}
		return true;
	default:
		{
		}
	}
	return false;
}

bool CAbsolute::GetZoomPos(BYTE* pCommand, DWORD& dwCommandLen)
{
	pCommand[0] = 0x01; // Receiver ID 
	pCommand[1] = 0x00; // Transmitter ID
	pCommand[2] = 0xB2; // OP code
	pCommand[3] = 0x00; // data 0 
	pCommand[4] = 0x00; // data 1
	pCommand[5] = 0x00; // Check sum

	pCommand[5] = pCommand[0]^pCommand[1]^pCommand[2]^pCommand[3]^pCommand[4]^pCommand[5];

	dwCommandLen = 6;
	return true;
}

bool CAbsolute::GetPTZPos(BYTE* pCommand, DWORD& dwCommandLen)
{
	pCommand[0] = 0x01; // Receiver ID 
	pCommand[1] = 0x00; // Transmitter ID
	pCommand[2] = 0xC3; // OP code
	pCommand[3] = 0x00; // data 0 
	pCommand[4] = 0x00; // data 1
	pCommand[5] = 0x00; // data 0 
	pCommand[6] = 0x00; // data 1
	pCommand[7] = 0x00; // data 0 
	pCommand[8] = 0x00; // data 1
	pCommand[9] = 0x00; // Check sum

	pCommand[9] = pCommand[0]^pCommand[1]^pCommand[2]^pCommand[3]^pCommand[4]^pCommand[5]^pCommand[6]^pCommand[7]^pCommand[8];

	dwCommandLen = 10;
	return true;
}

bool CAbsolute::GetPTPos(BYTE* pCommand, DWORD& dwCommandLen)
{
	pCommand[0] = 0x01; // Receiver ID 
	pCommand[1] = 0x00; // Transmitter ID
	pCommand[2] = 0xC1; // OP code
	pCommand[3] = 0x00; // data 0 
	pCommand[4] = 0x00; // data 1
	pCommand[5] = 0x00; // data 0 
	pCommand[6] = 0x00; // data 1
	pCommand[7] = 0x00; // Check sum

	pCommand[7] = pCommand[0]^pCommand[1]^pCommand[2]^pCommand[3]^pCommand[4]^pCommand[5]^pCommand[6];

	dwCommandLen = 8;
	return true;
}

bool CAbsolute::SetZoomPos(BYTE* pCommand, DWORD& dwCommandLen, int nRatio)
{
	BYTE bLow;
	BYTE bHi;
	
	if(!GetHiLowByte(nRatio, bLow, bHi))
		return false;

	pCommand[0] = 0x01; // Receiver ID 
	pCommand[1] = 0x00; // Transmitter ID
	pCommand[2] = 0xB3; // OP code
	pCommand[3] = bLow; // data 0 
	pCommand[4] = bHi; // data 1
	pCommand[5] = 0x00; // Check sum

	pCommand[5] = pCommand[0]^pCommand[1]^pCommand[2]^pCommand[3]^pCommand[4]^pCommand[5];

	dwCommandLen = 6;
	return true;
}

bool CAbsolute::SetPTZPos(int nPan, int nTilt, int nRatio, BYTE* pCommand, DWORD& dwCommandLen)
{
	BYTE bLow;
	BYTE bHi;

	if(!GetHiLowByte(nRatio, bLow, bHi))
		return false;

	pCommand[0] = 0x01; // Receiver ID 
	pCommand[1] = 0x00; // Transmitter ID
	pCommand[2] = 0xC2; // OP code
	pCommand[3] = nPan & 0x000000FF; // data 0 
	pCommand[4] = (nPan & 0x0000FF00)>>8; // data 1
	pCommand[5] = nTilt & 0x000000FF; // data 0 
	pCommand[6] = (nTilt & 0x0000FF00)>>8; // data 1
	pCommand[7] = bLow; // data 0 
	pCommand[8] = bHi; // data 1
	pCommand[9] = 0x00; // Check sum

	pCommand[9] = pCommand[0]^pCommand[1]^pCommand[2]^pCommand[3]^pCommand[4]^pCommand[5]^pCommand[6]^pCommand[7]^pCommand[8];

	dwCommandLen = 10;
	return true;
}

bool CAbsolute::SetPTPos(int nPan, int nTilt, BYTE* pCommand, DWORD& dwCommandLen)
{
	pCommand[0] = 0x01; // Receiver ID 
	pCommand[1] = 0x00; // Transmitter ID
	pCommand[2] = 0xC0; // OP code
	pCommand[3] = nPan & 0x000000FF; // data 0 
	pCommand[4] = (nPan & 0x0000FF00)>>8; // data 1
	pCommand[5] = nTilt & 0x000000FF; // data 0 
	pCommand[6] = (nTilt & 0x0000FF00)>>8; // data 1
	pCommand[7] = 0x00;
	
	pCommand[7] = pCommand[0]^pCommand[1]^pCommand[2]^pCommand[3]^pCommand[4]^pCommand[5]^pCommand[6];

	dwCommandLen = 8;
	return true;
}

bool CAbsolute::GetZoomStop(BYTE* pCommand, DWORD& dwCommandLen)
{
	pCommand[0] = 0x01; // Receiver ID 
	pCommand[1] = 0x00; // Transmitter ID
	pCommand[2] = 0xB3; // OP code
	pCommand[3] = 0x00; // data 0 
	pCommand[4] = 0x00; // data 1
	pCommand[5] = 0x00; // Check sum

	pCommand[5] = pCommand[0]^pCommand[1]^pCommand[2]^pCommand[3]^pCommand[4]^pCommand[5];

	dwCommandLen = 6;
	return true;
}

bool CAbsolute::GetFocusPos(BYTE* pCommand, DWORD& dwCommandLen)
{
	pCommand[0] = 0x01; // Receiver ID 
	pCommand[1] = 0x00; // Transmitter ID
	pCommand[2] = 0xB4; // OP code
	pCommand[3] = 0x00; // data 0 
	pCommand[4] = 0x00; // data 1
	pCommand[5] = 0x00; // Check sum

	pCommand[5] = pCommand[0]^pCommand[1]^pCommand[2]^pCommand[3]^pCommand[4]^pCommand[5];

	dwCommandLen = 6;
	return true;
}

bool CAbsolute::GetHiLowByte(int nRatio, BYTE& bLow, BYTE& bHi)
{
	switch(nRatio)
	{
	case O_ZOOM_POS_X1:
		bLow = 0x00;
		bHi = 0x00;
		break;
	case O_ZOOM_POS_X2:
		bLow = 0x60;
		bHi = 0x17;
		break;
	case O_ZOOM_POS_X3:
		bLow = 0x4C;
		bHi = 0x21;
		break;
	case O_ZOOM_POS_X4:
		bLow = 0x22;
		bHi = 0x27;
		break;
	case O_ZOOM_POS_X5:
		bLow = 0x22;
		bHi = 0x2B;
		break;
	case O_ZOOM_POS_X6:
		bLow = 0x20;
		bHi = 0x2E;
		break;
	case O_ZOOM_POS_X7:
		bLow = 0x80;
		bHi = 0x30;
		break;
	case O_ZOOM_POS_X8:
		bLow = 0x78;
		bHi = 0x32;
		break;
	case O_ZOOM_POS_X9:
		bLow = 0x26;
		bHi = 0x34;
		break;
	case O_ZOOM_POS_X10:
		bLow = 0x9E;
		bHi = 0x35;
		break;
	case O_ZOOM_POS_X11:
		bLow = 0xEE;
		bHi = 0x36;
		break;
	case O_ZOOM_POS_X12:
		bLow = 0x1C;
		bHi = 0x38;
		break;
	case O_ZOOM_POS_X13:
		bLow = 0x2E;
		bHi = 0x39;
		break;
	case O_ZOOM_POS_X14:
		bLow = 0x26;
		bHi = 0x3A;
		break;
	case O_ZOOM_POS_X15:
		bLow = 0x08;
		bHi = 0x3B;
		break;
	case O_ZOOM_POS_X16:
		bLow = 0xD4;
		bHi = 0x3B;
		break;
	case O_ZOOM_POS_X17:
		bLow = 0x8C;
		bHi = 0x3C;
		break;
	case O_ZOOM_POS_X18:
		bLow = 0x2E;
		bHi = 0x3D;
		break;
	case D_ZOOM_POS_X1:
		bLow = 0x00;
		bHi = 0x40;
		break;
	case D_ZOOM_POS_X2:
		bLow = 0x00;
		bHi = 0x60;
		break;
	case D_ZOOM_POS_X3:
		bLow = 0x80;
		bHi = 0x6A;
		break;
	case D_ZOOM_POS_X4:
		bLow = 0x00;
		bHi = 0x70;
		break;
	case D_ZOOM_POS_X5:
		bLow = 0x00;
		bHi = 0x73;
		break;
	case D_ZOOM_POS_X6:
		bLow = 0x40;
		bHi = 0x75;
		break;
	case D_ZOOM_POS_X7:
		bLow = 0xC0;
		bHi = 0x76;
		break;
	case D_ZOOM_POS_X8:
		bLow = 0x00;
		bHi = 0x78;
		break;
	case D_ZOOM_POS_X9:
		bLow = 0xC0;
		bHi = 0x78;
		break;
	case D_ZOOM_POS_X10:
		bLow = 0x80;
		bHi = 0x79;
		break;
	case D_ZOOM_POS_X11:
		bLow = 0x00;
		bHi = 0x7A;
		break;
	case D_ZOOM_POS_X12:
		bLow = 0xC0;
		bHi = 0x7A;
		break;
	default:
		return false;
	}
	return true;
}