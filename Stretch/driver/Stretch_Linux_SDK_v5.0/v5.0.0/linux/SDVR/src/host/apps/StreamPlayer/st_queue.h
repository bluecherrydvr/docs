// st_queue.h

#ifndef _ST_QUEUE_H_
#define _ST_QUEUE_H_

//#include <queue>
#include "syncobj.h"

template <class t>
class CStretchQueue
{
public:
    ~CStretchQueue() {
        if (m_pBuf != NULL) {
            delete[] m_pBuf;
            m_pBuf = NULL;
        }
    }
	CStretchQueue()	: 
        m_pBuf(NULL),
		m_pHead(NULL),
		m_pTail(NULL),
		m_iUsedSize(0),
		m_iSize(0)
        {;}

    int Create(int iSize) {
        m_pBuf = new t[iSize];
        if (m_pBuf == NULL)
            return 0;
        m_iSize = iSize;
        m_iUsedSize = 0;
        m_pHead = m_pTail = m_pBuf;
        m_pEnd = m_pHead + iSize - 1;
		return 1;
    }
    int GetSize() {return m_iSize;}
    int GetUsedSize() {return m_iUsedSize;}
	int IsFull() {
		m_Mutex.Lock();
		int ret = m_iUsedSize < m_iSize ? 0 : 1;
		m_Mutex.Unlock();
		return ret;
	}
	int IsEmpty() {
		m_Mutex.Lock();
		int ret = m_iUsedSize > 0 ? 0 : 1;
		m_Mutex.Unlock();
		return ret;
	}
    t* GetHead() {
		if (IsFull()) {
            m_EventWrite.Wait(INFINITE);
		}
		m_Mutex.Lock();
        t* ret = m_pHead;
		m_Mutex.Unlock();
        return ret;
    }
    void CommitHead() {
		m_Mutex.Lock();
        if (m_pHead == m_pEnd)
            m_pHead = m_pBuf;
        else
            m_pHead++;
        m_iUsedSize++;
		//printf("w %d\n", m_iUsedSize);
        m_EventRead.Set();
        m_Mutex.Unlock();
    }
    t* GetTail() {
        if (IsEmpty()) {
            m_EventRead.Wait(INFINITE);
        }
        m_Mutex.Lock();
		if (m_iUsedSize == 0) {
			m_Mutex.Unlock();
            return NULL;
		}
        t* ret = m_pTail;
        m_Mutex.Unlock();
        return ret;
    }

    void CommitTail() {
        m_Mutex.Lock();
        if (m_pTail == m_pEnd)
            m_pTail = m_pBuf;
        else
            m_pTail++;
        m_iUsedSize--;
		//printf("r %d\n", m_iUsedSize);
        m_EventWrite.Set();
        m_Mutex.Unlock();
    }
    void Flush() {
		m_Mutex.Lock();
        m_pHead = m_pTail = m_pBuf;
        m_iUsedSize = 0;
        m_Mutex.Unlock();
    }
    void Abort() {
		m_Mutex.Lock();
        m_EventWrite.Set();
        m_EventRead.Set();
        m_Mutex.Unlock();
    }

protected:
    t* m_pBuf;
    t* m_pEnd;
    t* m_pHead;
	t* m_pTail;
    int m_iUsedSize;
    int m_iSize;
    CSTMutex m_Mutex;
    CSTEvent m_EventWrite;
    CSTEvent m_EventRead;
};

#endif
