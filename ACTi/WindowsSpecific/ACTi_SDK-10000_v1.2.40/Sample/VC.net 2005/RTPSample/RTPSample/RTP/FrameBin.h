#pragma once
#include <iostream>
#include <iterator>
#include <vector>
#include <list>

using namespace std;

class CCriticalSection
{
public:
	// Constructor
	CCriticalSection()
	{
		InitializeCriticalSection(&m_CS);
	}

	// Destructor
	virtual ~CCriticalSection()
	{
		DeleteCriticalSection(&m_CS);
	}

	// Enter critical section
	void Enter()
	{
		EnterCriticalSection(&m_CS);
	}

	// Leave critical section
	void Leave()
	{
		LeaveCriticalSection(&m_CS);
	}

private:
	CRITICAL_SECTION m_CS;
};

class CScopeLock
{
public:
	// Automatically Enter Critical Section
	CScopeLock(CCriticalSection *cCS):m_CS(cCS)
	{
		m_CS->Enter();
	}
	// Automatically Leave Critical Section
	virtual ~CScopeLock()
	{
		m_CS->Leave();
	}
	// Enter Critical Section
	void Enter()
	{
		m_CS->Enter();
	}
	// Leave Critical Section
	void Leave()
	{
		m_CS->Leave();
	}
private:
	CCriticalSection* m_CS;
};

class CFrameBin
{
protected:
	list< RTP_FRAME * >  m_Frames;
	CCriticalSection m_cCS;

public:
	const int MAX_UNIFRAME_SIZE;
	const int MAX_BIN_ROW;

	unsigned int size( void) const { return (unsigned int)m_Frames.size(); }
	CFrameBin( const int unifsize = 1024*1024 )
		: MAX_UNIFRAME_SIZE( unifsize), MAX_BIN_ROW( 64)
	{
		m_Frames.clear();
	}

	virtual ~CFrameBin()
	{
		ReleaseAllFrames();
	}

	int GetFrameSize()
	{
		return (int)m_Frames.size();
	}

	RTP_FRAME * NewFrames( const DWORD FrameSie )
	{
		char *buf = NULL;
		RTP_FRAME *tmp;

		tmp = new RTP_FRAME;
		tmp->Type = RTP_FRAME_UNKNOWN;
		tmp->Buf = new unsigned char[ FrameSie];
		memset( tmp->Buf, 0, FrameSie*sizeof(unsigned char));
		tmp->Size = MAX_UNIFRAME_SIZE;
		tmp->Len = 0;
		return tmp;
	}

	BOOL PushNewFrames( RTP_FRAME * Frame)
	{
		CScopeLock CSL(&m_cCS);
		if( Frame == NULL ) return FALSE;
		m_Frames.push_back( Frame);
		return TRUE;
	}

	RTP_FRAME * PopFrames( void)
	{
		CScopeLock CSL(&m_cCS);
		RTP_FRAME *tmp;
		list< RTP_FRAME * >::iterator rip;

		if( m_Frames.size() <= 0 ) return NULL;
		rip = m_Frames.begin();
		tmp = (*rip);
		m_Frames.erase( rip );
		return tmp;
	}

	BOOL FreeFrames( RTP_FRAME * Frame )
	{
		delete [] Frame->Buf;
		delete Frame;
		return TRUE;
	}

	void ReleaseAllFrames(void)
	{
		CScopeLock CSL(&m_cCS);
		RTP_FRAME *tmp;

		list< RTP_FRAME * >::iterator rip;

		while( m_Frames.size() > 0 )
		{
			rip = m_Frames.begin();
			if( rip == m_Frames.end() ) break;
			tmp = (*rip);
			delete [] tmp->Buf;
			delete tmp;
			m_Frames.erase( rip);
		}
	}
};
