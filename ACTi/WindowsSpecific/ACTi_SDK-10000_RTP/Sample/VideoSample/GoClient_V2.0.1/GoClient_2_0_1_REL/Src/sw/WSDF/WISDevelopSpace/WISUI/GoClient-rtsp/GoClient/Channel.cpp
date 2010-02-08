// Channel.cpp: implementation of the CChannel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GoClient.h"
#include "Channel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

void CChannel::ShowErrorMessage()
{
	int err=WSAGetLastError();						
	CString ss;										
	ss.Format("Socket Error : %d",err);				
	AfxMessageBox(ss);								
}
