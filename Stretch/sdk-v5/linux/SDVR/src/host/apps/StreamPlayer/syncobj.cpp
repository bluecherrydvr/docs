// syncobj.cpp
#include <assert.h>
#include "syncobj.h"

// mutex
CSTMutex::CSTMutex()
{
	m_id = CreateMutex(NULL, FALSE, NULL);
	assert(m_id != NULL);
}

CSTMutex::~CSTMutex()
{
	if (m_id != NULL) {
		CloseHandle(m_id);
		m_id = NULL;
	}
}

int CSTMutex::Lock()
{
	if (WaitForSingleObject(m_id, INFINITE) == WAIT_FAILED) {
		assert(0);
		//return -1;
	}
	return 0;
}

int CSTMutex::Unlock()
{
	if (ReleaseMutex(m_id) == FALSE) {
		assert(0);
//		return -1;
	}
	return 0;
}

// semaphore
CSTSemaphore::CSTSemaphore(unsigned int initial_value)
{
	m_id = CreateSemaphore(NULL, initial_value, 32*1024, NULL);
	m_count = initial_value;
	assert(m_id != NULL);
}

CSTSemaphore::~CSTSemaphore()
{
	if (m_id != NULL) {
		CloseHandle(m_id);
		m_id = NULL;
	}
}

int CSTSemaphore::WaitTimeout(DWORD timeout)
{
	int retval;

	switch (WaitForSingleObject(m_id, timeout)) {
    case WAIT_OBJECT_0:
		InterlockedDecrement(&m_count);
		retval = 0;
		break;
    case WAIT_TIMEOUT:
		retval = WAIT_TIMEOUT;
		break;
    default:
		retval = -1;
		break;
	}
	return retval;
}

int CSTSemaphore::TryWait()
{
	return WaitTimeout(0);
}

int CSTSemaphore::Wait()
{
	return WaitTimeout(INFINITE);
}

unsigned int CSTSemaphore::GetValue()
{
	return m_count;
}

int CSTSemaphore::Post()
{
	InterlockedIncrement(&m_count);
	if (ReleaseSemaphore(m_id, 1, NULL) == FALSE) {
		InterlockedDecrement(&m_count);	/* restore */
		return -1;
	}
	return 0;
}

// event
CSTEvent::~CSTEvent()
{
	if (m_id != NULL) {
		CloseHandle(m_id);
		m_id = NULL;
	}
}

CSTEvent::CSTEvent()
{
	m_id = CreateEvent( 
        NULL,         // no security attributes
        FALSE,         // manual-reset event
        FALSE,         // initial state is signaled
        NULL  // object name
        ); 

}

int CSTEvent::Set()
{
	return SetEvent(m_id);
}

int CSTEvent::Reset()
{
	return ResetEvent(m_id);
}

int CSTEvent::Wait(int ms)
{
	if (WaitForSingleObject(m_id, ms) == WAIT_FAILED) {
		assert(0);
//		return -1;
	}
	return 0;
}

// condition
CSTCondition::CSTCondition()
{
	m_waiting = m_signals = 0;
}

CSTCondition::~CSTCondition()
{
}

int CSTCondition::Signal()
{
	m_lock.Lock();
	if (m_waiting > m_signals) {
		++m_signals;
		m_WaitSem.Post();
		m_lock.Unlock();
		m_WaitDone.Wait();
	} 
    else
		m_lock.Unlock();
	return 0;
}

int CSTCondition::Broadcast()
{
	m_lock.Lock();
	if (m_waiting > m_signals) {
		int i, num_waiting;

		num_waiting = (m_waiting - m_signals);
		m_signals = m_waiting;
		for ( i=0; i<num_waiting; ++i ) {
			m_WaitSem.Post();
		}
		m_lock.Unlock();
		for (i=0; i<num_waiting; ++i) {
		    m_WaitDone.Wait();
		}
	} 
    else
		m_lock.Unlock();

	return 0;
}

/* Wait on the condition variable for at most 'ms' milliseconds.
   The mutex must be locked before entering this function!
   The mutex is unlocked during the wait, and locked again after the wait.

Typical use:

Thread A:
	SDL_LockMutex(lock);
	while ( ! condition ) {
		SDL_CondWait(cond);
	}
	SDL_UnlockMutex(lock);

Thread B:
	SDL_LockMutex(lock);
	...
	condition = true;
	...
	SDL_UnlockMutex(lock);
 */
int CSTCondition::WaitTimeout(CSTMutex *mutex, int ms)
{
	int retval;

	m_lock.Lock();
	m_waiting++;
	m_lock.Unlock();

	/* Unlock the mutex, as is required by condition variable semantics */
	mutex->Unlock();

	/* Wait for a signal */
	if (ms == INFINITE)
		retval = m_WaitSem.Wait();
	else
		retval = m_WaitSem.WaitTimeout(ms);

	m_lock.Lock();
	if (m_signals > 0) {
		/* If we timed out, we need to eat a condition signal */
		if (retval > 0) {
			m_WaitSem.Wait();
		}
		/* We always notify the signal thread that we are done */
		m_WaitDone.Post();

		/* Signal handshake complete */
		--m_signals;
	}
	--m_waiting;
	m_lock.Unlock();

	/* Lock the mutex, as is required by condition variable semantics */
	mutex->Lock();

	return retval;
}

int CSTCondition::Wait(CSTMutex *mutex)
{
	return WaitTimeout(mutex, INFINITE);
}
