// Buffer.cpp: implementation of the CBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "Buffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBuffer::CBuffer(DWORD size, bool AutoDelete)
{
	m_bAutoDelete = AutoDelete;
	try
	{
		ptr.b = new BYTE[size];
		if(ptr.b) ByteLen = size;
	}
	catch(...)
	{
		ErrorMsg("Out of memory!");
	}
}

CBuffer::CBuffer(void* buffer, DWORD length)
{
	m_bAutoDelete = false;
	if(buffer)
	{
		ptr.v = buffer;
		ByteLen = length;
	}
	else
	{
		ptr.b = NULL;
		ByteLen = 0;
	}
}

CBuffer::~CBuffer()
{
	// remember to delete the memory
	if(m_bAutoDelete && ptr.b != NULL)  delete ptr.b;
}

void CBuffer::Erase()
{
	if(ptr.b) ZeroMemory(ptr.b,ByteLen);
}
