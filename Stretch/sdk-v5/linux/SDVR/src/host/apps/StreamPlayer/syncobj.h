// syncobj.h

#ifndef _SYNCOBJ_H_
#define _SYNCOBJ_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class CSTMutex
{
public:
    ~CSTMutex();
    CSTMutex();
    int Lock();
    int Unlock();

private:
	HANDLE m_id;
};

class CSTSemaphore
{
public:
    ~CSTSemaphore();
    CSTSemaphore(unsigned int initial_value = 0);

    int WaitTimeout(DWORD timeout);
    int TryWait();
    int Wait();
    unsigned int GetValue();
    int Post();

private:
	HANDLE m_id;
    volatile LONG m_count;
};

class CSTCondition
{
public:
    ~CSTCondition();
    CSTCondition();

    int Signal();
    int Broadcast();
    int WaitTimeout(CSTMutex *mutex, int ms);
    int Wait(CSTMutex *mutex);

private:
	CSTMutex m_lock;
	int m_waiting;
	int m_signals;
	CSTSemaphore m_WaitSem;
	CSTSemaphore m_WaitDone;
};

class CSTEvent
{
public:
    ~CSTEvent();
    CSTEvent();

    int Set();
    int Reset();
    int Wait(int ms);

private:
	HANDLE m_id;
};

#endif
