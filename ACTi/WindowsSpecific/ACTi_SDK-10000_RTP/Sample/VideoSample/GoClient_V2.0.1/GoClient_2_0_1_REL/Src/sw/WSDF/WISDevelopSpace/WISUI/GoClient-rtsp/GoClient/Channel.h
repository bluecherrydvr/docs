// Channel.h: interface for the CChannel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHANNEL_H__9C1A8E3F_8E2E_43AE_BC22_4934E31C676E__INCLUDED_)
#define AFX_CHANNEL_H__9C1A8E3F_8E2E_43AE_BC22_4934E31C676E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "winsock2.h" 

class CChannel  
{
public:
	virtual int Initialize(char *addr_str,int local_rtp_port,int local_Artp_port) PURE;
	virtual void Uninitialize() PURE;
	virtual int ReadVideo(unsigned char *pBuf, int& Size) PURE;
	virtual int ReadAudio(unsigned char *pBuf, int& Size) PURE;

	void ShowErrorMessage();
};

#endif // !defined(AFX_CHANNEL_H__9C1A8E3F_8E2E_43AE_BC22_4934E31C676E__INCLUDED_)
