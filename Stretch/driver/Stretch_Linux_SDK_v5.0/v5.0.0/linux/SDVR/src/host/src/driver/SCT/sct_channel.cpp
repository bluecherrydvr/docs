/**********************************************************************\
*                                                                      *
* Copyright (c) 2005 Stretch, Inc. All rights reserved.                *
*                                                                      *
* THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF *
* STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT *
* THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.                *
*                                                                      *
\**********************************************************************/

// All the sct channel related functions

//---------------------------------------------------------------------
// Author:Xun Tan
// Date Created: 06 10 2007
//---------------------------------------------------------------------

#ifdef WIN32
#define _WIN32_WINNT 0x500
#include <windows.h>
#include "..\windows\Inc\ioctls.h"
#include "log.h"
#else
#include <linux/ioctl.h>
#include <time.h>
#include "../linux/include/stretch_if.h"
__asm__(".symver io_getevents, io_getevents@LIBAIO_0.4");
#endif
#include "sct.h"
#include "sct_internal.h"

#ifndef WIN32
int my_sem_wait(sem_t* sem, int timeout)
{
    struct timespec ts, tv;

    clock_gettime(CLOCK_REALTIME, &tv);
    ts.tv_sec = tv.tv_sec + timeout / 1000;
    ts.tv_nsec = tv.tv_nsec + (timeout % 1000) * 1000 * 1000;
    if (ts.tv_nsec > 1000 * 1000 * 1000)
    {
        ts.tv_nsec -= 1000 * 1000 * 1000;
        ts.tv_sec++;
    }

    return sem_timedwait((sem),&ts);
}
#endif

chan_buf_node::chan_buf_node(int size)
{
	buf = new char[size];
	size = 0;
#ifdef WIN32
	hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
#endif
}

chan_buf_node::~chan_buf_node()
{
	delete[] buf;
#ifdef WIN32
	CloseHandle( hEvent );
#endif
}

sct_channel_info::sct_channel_info()
{
	m_sig = CHANNEL_SIGNATURE;
	OS_UMUTEX_CREATE (hChanMutex);
#ifdef WIN32
	hChanThread = NULL;
	hSemaphore = INVALID_HANDLE_VALUE;
	hChanEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	hDestroyEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
#else
    hChanThread = 0;
	aio_context = 0;
#endif
	buffer_array = 0;
	nbuf = 0;
	max_size = 0;
	cb_func = 0;
    os_io_setup( 256, &aio_context );
}

sct_channel_info::~sct_channel_info()
{
	releaseResource();
    os_io_destroy ( aio_context );
}

bool sct_channel_info::initMemory( int n, int size)
{
	releaseBuffers();
	nbuf = n;
	max_size = size;
	if ( (nbuf > 0) && (max_size > 0))
	{
		buffer_array = new chan_buf_node*[nbuf];
		int i;
		for (i=0;i<nbuf;i++)
		{
			buffer_array[i] = new chan_buf_node(max_size);
			buf_free_list.push_back( buffer_array[i]);
		}
		return true;
	}
	return  false;
}

void sct_channel_info::releaseResource()
{
#ifdef WIN32
	if ( hChanThread != NULL )
	{
		quit = true;
		SetEvent(hChanEvent);
		WaitForSingleObject(hChanThread,10000);
	}
	if ( hChanEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle( hChanEvent );
		hChanEvent = INVALID_HANDLE_VALUE;
	}
	if ( hDestroyEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle( hDestroyEvent );
		hDestroyEvent = INVALID_HANDLE_VALUE;
	}
#else
	quit = true;
    if ( hChanThread != 0 )
    {
        OS_UTHREAD_WAIT(hChanThread);
        hChanThread = 0;
    }
#endif
    lock();
	releaseBuffers();
    unlock();
	OS_UMUTEX_DESTROY( hChanMutex );
}

void sct_channel_info::releaseBuffers()
{
    OS_USEMAPHORE_DESTROY(hSemaphore);
    /*
	if ( hSemaphore != INVALID_HANDLE_VALUE)
	{
		CloseHandle( hSemaphore );
		hSemaphore = INVALID_HANDLE_VALUE;
	}
    */
    board->check_callbacks(this);
	if ( (nbuf > 0) && buffer_array)
	{
		int i;
		for (i=0;i<nbuf;i++)
		{
			delete buffer_array[i];
		}
		delete[] buffer_array;
	}
	buf_free_list.clear();
	buf_busy_list.clear();
	buf_pending_list.clear();
	buf_ready_list.clear();
}

unsigned int sct_channel_info::close()
{
	// This is required for both send and recv channels.
	int ret = SCT_ERR_CHANNEL_CLOSE;
    t_sct_mgmt_msg  message;

	if (state == CHANNEL_STATE_CLOSED)
		return SCT_NO_ERROR;

    if( type == CHANNEL_RECV )
    {
		unsigned int retry = 0;
        // Handle destroy of recv channel.

		//wait for CHANNEL_STATE_CLOSE_PENDING
		while ( retry < 200)
		{
			if( (state == CHANNEL_STATE_CLOSE_PENDING) || (state == CHANNEL_STATE_DEAD))
			{
				break;
			}
			else
			{
				OS_USLEEP(1);
				retry ++;
			}
		}
		if( (state != CHANNEL_STATE_CLOSE_PENDING) && (state != CHANNEL_STATE_DEAD))
		{
			Trace(  __FUNCTION__ "Close %d timed out\n",port);
		}
		lock();
        {
            // Inform the send side that the close is completing.
            message.id                = SCT_MGMT_MSG_DESTROY;
            message.data.connect.port = port;

           // Trace( __FUNCTION__ ": Calling DESTROY ACK IOCTL for port %d.\n",port );

            if( board->ioControl( WINOFFLD_IOCTL_DESTROY_RESPONSE,
                                    sizeof(t_sct_mgmt_msg),
                                    (unsigned char *)&message,
                                    NULL) )
			{
				//unsigned int retry = 0;
				ret = SCT_NO_ERROR;
				unlock();
				/*
				while(  ((buf_free_list.size()+ buf_busy_list.size()) < (unsigned int)nbuf) && (retry < 200) )
                {
                    Sleep(1);
                    retry ++;
                }
                if ( ((buf_free_list.size() + buf_busy_list.size()) < (unsigned int)nbuf))
                {
					Trace("Error: channel %d: time out, driver did not return all the buffer back\n",port);
                }
				*/
				state = CHANNEL_STATE_CLOSED;
			}
			else
            {
                Trace( __FUNCTION__ ": Error, DESTROY ACK IOCTL failed, code = %d.\n",
                     GetLastError() );
				unlock();
            }
        }
		/*
        else
        {
            //Trace( __FUNCTION__ ": Error, attempt to destroy recv channel in use.\n" );
            ret = SCT_ERR_CHANNEL_IN_USE;
			unlock();
        }
		*/
    }
	else
	{
		lock();
		//send channel
		if (state == CHANNEL_STATE_CLOSED || state == CHANNEL_STATE_CLOSE_PENDING )
		{
			Trace( __FUNCTION__ ": Error, channel is already in destroy or pending destroy state.\n" );
			unlock();
		}
		else
		{
			state = CHANNEL_STATE_CLOSE_PENDING;
			unlock();
            DWORD retry = 0;
			while(	(buf_free_list.size() < (unsigned int)nbuf ) && ( retry < 200 ))
			{
				//Trace( __FUNCTION__ ": channel %d buf_free_list size is %d\n", port, buf_free_list.size());
				OS_USLEEP(1);
                retry++;
			}
            lock();
            unlock();
			//Trace( __FUNCTION__ ": channel %d out of the loop.\n",port );
			message.id                = SCT_MGMT_MSG_DESTROY;
			message.data.connect.port = port;

			if( !board->ioControl( WINOFFLD_IOCTL_WRITE_MGMT_QUEUE,
				sizeof(t_sct_mgmt_msg),
				(unsigned char *)&message,
				NULL) )
			{
				Trace( __FUNCTION__ "(%d): Error, DESTROY ACK IOCTL failed, code = %d.\n",
					port,GetLastError() );
				state = CHANNEL_STATE_DEAD;
			}
			/*
			else if(  message.rsp != 0 )
			{
				Trace( __FUNCTION__ "(%d): Error, channel destroy request rejected by board.\n",port );
				state = CHANNEL_STATE_DEAD;
			}
			*/
			else
			{
				state = CHANNEL_STATE_CLOSED;
				ret = SCT_NO_ERROR;
			}
			/*
            while(  ((buf_free_list.size()+ buf_busy_list.size()) < (unsigned int)nbuf) && (retry < 200) )
            {
                Sleep(1);
                retry ++;
            }
            if ( ((buf_free_list.size() + buf_busy_list.size()) < (unsigned int)nbuf))
            {
                Trace("Error: channel %d: time out, driver did not return all the buffer back\n",port);
            }
			*/
		}
    }
    return ret;
}

void * sct_channel_info::getFreeBuffer( int timeout )
{
    if( type != CHANNEL_SEND )
    {
        Trace( __FUNCTION__ ": Error, channel is a RECV channel.\n" );
        return NULL;
    }

    // It is possible that the channel is dead.

    if( state != CHANNEL_STATE_ACTIVE)
    {
        Trace( __FUNCTION__ ": Error, channel state not ACTIVE.\n" );
        return NULL;
    }


	int ws = OS_USEMAPHORE_WAIT_TIMEOUT( hSemaphore, timeout );
    void * buffer = 0;
    if (!OS_UTIMEOUT(ws))
    {
        // Now we are guaranteed to have at least one free buffer.
	    lock();
	    chan_buf_node * p = buf_free_list.front();
        if ( p && (buf_free_list.size() > 0) )
	    {
			buffer = p->buf;
		    buf_free_list.pop_front();
		    //Trace( __FUNCTION__  ": free_list count %d\n", buf_free_list.size());
		    buf_busy_list.push_back( p );
	    }
	    unlock();
    }
    else
    {
        //printf("time out %d, ws =%d\n",timeout,ws);
    }
    return buffer;
}

void sct_channel_info::lock()
{
	OS_UMUTEX_LOCK( hChanMutex);
}

void sct_channel_info::unlock()
{
	OS_UMUTEX_UNLOCK( hChanMutex);
}

unsigned int sct_channel_info::sendBuffer(void * buffer,sx_int32 size)
{
	chan_buf_node * p = NULL;
    bool ok = true;
    if( type != CHANNEL_SEND )
    {
        Trace( __FUNCTION__ ": Error, channel is a RECV channel.\n" );
        return SCT_ERR_INVALID_PARAMETER;
    }

    if( size > max_size )
    {
        Trace( __FUNCTION__ ": Error, send size > max buffer size.\n" );
        return SCT_ERR_INVALID_PARAMETER;
    }

    if( state == CHANNEL_STATE_DEAD )
    {
        Trace( __FUNCTION__ ": Error, channel %d is DEAD\n",
             port );

        return SCT_ERR_CHANNEL_DEAD;
    }

    if( state != CHANNEL_STATE_ACTIVE)
    {
        Trace( __FUNCTION__ ": Error, channel state not ACTIVE.\n" );
        return SCT_ERR_CHANNEL_NOT_ACTIVE;
    }

    // Verify that the buffer belongs to this channel - check busy buffer list.
	lock();
	sct_buffer_list::iterator it;
	for (it = buf_busy_list.begin(); it != buf_busy_list.end(); ++it)
	{
		p = (*it);
		if ( p->buf == buffer)
		{
			break;
		}
	}

	if( (it == buf_busy_list.end()) || (p == NULL) )
    {
        Trace( __FUNCTION__ ": Error, buffer 0x%08x does not belong to this channel.\n",
             buffer );
		unlock();
        return SCT_ERR_INVALID_PARAMETER;
    }

    if ( size == 0 )
    {
        buf_busy_list.erase( it );
	    buf_free_list.push_back(p);
		unlock();
        OS_USEMAPHORE_POST(hSemaphore);
    }
    else
    {
        //move buffer from busy list to ready list
        buf_busy_list.erase( it );
        p->size = size;
#ifdef WIN32
        buf_ready_list.push_back(p);
		unlock();
        SetEvent(hChanEvent);
#else
        p->overlap.context = aio_context;
        p->overlap.obj = p;
        sct_iocode_t ctl_code=SX_CONSTRUCT_DMA_IOCTL((uint32_t)SX_IOCTL_BUFFER_SEND,
                                                          (uint32_t)p->size,
                                                          (uint32_t)port);
        //for linux async io call, pass 0 in buffer size
        if (board->ioControl(ctl_code,0,(unsigned char*)p->buf,&p->overlap))
        {
            buf_pending_list.push_back(p);
        }
        else
        {
            //channel died.
            if (state == CHANNEL_STATE_ACTIVE)
                state = CHANNEL_STATE_DEAD;
	        buf_free_list.push_back(p);
            ok = false;
        }
		unlock();
#endif
    }
	return ok?SCT_NO_ERROR:SCT_ERR_CHANNEL_DEAD;
}

unsigned int sct_channel_info::connect()
{
	// Build the connect message.

    t_sct_mgmt_msg  message;

    message.id                    = SCT_MGMT_MSG_CONNECT;
    message.data.connect.port     = port;
    //message.data.connect.nbuf     = nbuf;
    message.data.connect.max_size = max_size;

    // Send the connect request to the driver. This will return only when
    // the connection is accepted, or there is an error.

    if( !board->ioControl( WINOFFLD_IOCTL_WRITE_MGMT_QUEUE,
                            sizeof(t_sct_mgmt_msg),
                            (unsigned char *)&message,
                            NULL) )
    {
        Trace( __FUNCTION__ ": CONNECT error from driver, code %d.\n",
             GetLastError() );
		close();
        return SCT_ERR_CHANNEL_CONNECT;
    }

    // Now see if the request was accepted or not.
    if( message.rsp != 0 )
    {
        Trace( __FUNCTION__ ": Error, board rejected connection.\n" );
        return SCT_ERR_CHANNEL_CONNECT;
    }
#ifndef WIN32
	message.data.connect.port       = port;
	memset( &ol_destroy, 0, sizeof(ol_destroy) );
	OS_UEVENT_RESET(hDestroyEvent);
    ol_destroy.context = aio_context;
    ol_destroy.obj = this;
    if( !board->ioControl( WINOFFLD_IOCTL_WAIT_DESTROY,
                           sizeof(t_sct_mgmt_msg),
                           (unsigned char *)&message,
                           &ol_destroy))
	{
		Trace( __FUNCTION__ ": WAIT DESTROY IOCTL returned error.\n" );

        return SCT_ERR_CHANNEL_CONNECT;
	}
#endif
    // Connect successful, complete channel setup.
	state = CHANNEL_STATE_ACTIVE;
	type  = CHANNEL_SEND;
	OS_USEMAPHORE_CREATE(hSemaphore,nbuf);//CreateSemaphore( NULL,nbuf,nbuf,NULL);
	startChannelThread();
    return SCT_NO_ERROR;
}

unsigned int sct_channel_info::accept()
{
	// Create the check connect message.
    t_sct_mgmt_msg  message;

    message.data.connect.port = port;

    // Check if connect request has arrived. This will return only when
    // a request arrives.

    if( !board->ioControl( WINOFFLD_IOCTL_CHECK_CONNECT,
                            sizeof(t_sct_mgmt_msg),
                            (unsigned char *)&message,
                            NULL) )
    {
        Trace( __FUNCTION__ ": CHECK_CONNECT error, code %d.\n",
             GetLastError() );
        return SCT_ERR_CHANNEL_CONNECT;
    }

	initMemory( nbuf, message.data.connect.max_size);
    type = CHANNEL_RECV;


	message.data.connect.port       = port;
	memset( &ol_destroy, 0, sizeof(ol_destroy) );
	OS_UEVENT_RESET(hDestroyEvent);
#ifdef WIN32
	ol_destroy.hEvent = hDestroyEvent;
#else
    ol_destroy.context = aio_context;
    ol_destroy.obj = this;
#endif
    if( !board->ioControl( WINOFFLD_IOCTL_WAIT_DESTROY,
                           sizeof(t_sct_mgmt_msg),
                           (unsigned char *)&message,
                           &ol_destroy))
	{
		Trace( __FUNCTION__ ": WAIT DESTROY IOCTL returned error.\n" );

        return SCT_ERR_CHANNEL_CONNECT;
	}

	message.rsp = RESPONSE_OK;
    message.data.connect.port = port;

    if( !board->ioControl( WINOFFLD_IOCTL_ACCEPT_RESPONSE,
                            sizeof(t_sct_mgmt_msg),
                            (unsigned char*)&message,
                            NULL) )
    {
        Trace( __FUNCTION__ ": ACCEPT RESPONSE IOCTL returned error.\n" );

        return SCT_ERR_CHANNEL_CONNECT;
    }


	state = CHANNEL_STATE_ACTIVE;
	OS_USEMAPHORE_CREATE( hSemaphore,0 );// = CreateSemaphore( NULL,0,nbuf,NULL);
	startChannelThread();
    return SCT_NO_ERROR;
}

bool sct_channel_info::isRecvBufferAvailable()
{
	return (type == CHANNEL_RECV) && (buf_ready_list.size() > 0);
}

void * sct_channel_info::getRecvBuffer( int & size )
{

	void * ret_buf = NULL;
	chan_buf_node * p;
    if(type != CHANNEL_RECV )
    {
        Trace( __FUNCTION__ ": Error, channel is a SEND channel.\n" );
        return NULL;
    }

    if( (state == CHANNEL_STATE_DEAD ) || (state == CHANNEL_STATE_CLOSED) )
    {
        Trace( __FUNCTION__ ": Error, channel is DEAD or CLOSED, no buffers.\n" );
        return NULL;
    }

    if( (state == CHANNEL_STATE_CLOSE_PENDING) && !isRecvBufferAvailable() )
    {
        // No buffers available, and there won't be any more ever.
        Trace( __FUNCTION__ ": Channel is CLOSE_PENDING, no more recv buffers.\n" );
        return NULL;
    }

    if( OS_USEMAPHORE_WAIT(hSemaphore) == 0)
    {
        if( isRecvBufferAvailable() )
        {
			lock();
			p = buf_ready_list.front();
			buf_ready_list.pop_front();
			buf_busy_list.push_back( p );
			ret_buf = p->buf;
			size = p->size;
			unlock();
		}
	}
	return ret_buf;
}

void sct_channel_info::returnRecvBuffer( void * buffer)
{
    chan_buf_node* p;
    if( type != CHANNEL_RECV )
    {
        Trace( __FUNCTION__ ": Error, channel is a SEND channel.\n" );
        return;
    }

    if( (state == CHANNEL_STATE_DEAD) || (state == CHANNEL_STATE_CLOSED) )
    {
        Trace( __FUNCTION__ ": Error, channel is DEAD/CLOSED, no buffers.\n" );
        return;
    }

    lock();

    sct_buffer_list::iterator it;
    for (it = buf_busy_list.begin(); it != buf_busy_list.end(); ++it)
    {
        p = (*it);
        if ( p->buf == buffer)
        {
            break;
        }
    }

    if( it == buf_busy_list.end() )
    {
        //Trace( __FUNCTION__ ": Error, buffer 0x%08x does not belong to this channel.\n",
        //     buffer );
        unlock();
        return;
    }

    buf_busy_list.erase( it );
#ifdef WIN32
    buf_free_list.push_back( p );
    unlock();
    OS_UEVENT_WAKE(hChanEvent);
#else
    p->overlap.context = aio_context;
    p->overlap.obj = p;
    sct_iocode_t ctl_code=SX_CONSTRUCT_DMA_IOCTL((uint32_t)SX_IOCTL_STORE_RECEIVE_BUFFER,
            (uint32_t)max_size,
            (uint32_t)port);
    if (board->ioControl(ctl_code,0,(unsigned char*)p->buf,&p->overlap))
    {
        buf_pending_list.push_back(p);
    }
    else
    {
        if (state == CHANNEL_STATE_ACTIVE)
            state = CHANNEL_STATE_DEAD;
        buf_free_list.push_back( p );
    }
    unlock();
#endif
}

bool sct_channel_info::regCallback( sct_callback_func* func, void * p)
{
	lock();
	cb_func = func;
	context = p;
	unlock();
	return true;
}

#ifdef WIN32
DWORD WINAPI channel_thread_func( LPVOID p)
#else
void * channel_thread_func( void * p)
#endif
{
	sct_channel_info * ci = (sct_channel_info *) p;
	ci->channel_thread();
	return 0;
}

void sct_channel_info::startChannelThread()
{
	if (hChanThread == 0)
	{
        OS_UTHREAD_CREATE(dwThreadId, hChanThread, channel_thread_func,this);
		//hChanThread = CreateThread(NULL,0, channel_thread_func, this, 0, &dwThreadId );
	}
}



void sct_channel_info::constructWaitArray()
{
	bool ok = true;
	chan_buf_node * pnode = NULL;
	if ( type == CHANNEL_RECV )
	{
		waitCount = 0;
		lock();
		if (buf_free_list.size() > 0)
		{
			pnode = buf_free_list.front();
			buf_free_list.pop_front();
		}
		unlock();
		while ( pnode != NULL )
		{
			t_sct_mgmt_msg  message;
			message.data.connect.port = port;
			memset( &(pnode->overlap), 0, sizeof(pnode->overlap) );
#ifdef WIN32
			pnode->overlap.hEvent = pnode->hEvent;
			ResetEvent(pnode->hEvent);
			if (board->ioControlEx(WINOFFLD_IOCTL_STORE_RECEIVE_BUFFER,
				sizeof(t_sct_mgmt_msg),
				(unsigned char *)&message,
				max_size,
				(unsigned char *)pnode->buf,
				&pnode->overlap))
			{
				buf_pending_list.push_back(pnode);
			}
#else
            sct_iocode_t ctl_code=SX_CONSTRUCT_DMA_IOCTL((uint32_t)SX_IOCTL_STORE_RECEIVE_BUFFER,
                                                          (uint32_t)max_size,
                                                          (uint32_t)port);
            pnode->overlap.context = aio_context;
            pnode->overlap.obj = pnode;
            //for linux async io call, pass 0 in buffer size
            if (board->ioControl(ctl_code,0,(unsigned char*)pnode->buf,&pnode->overlap))
			{
                lock();
				buf_pending_list.push_back(pnode);
                unlock();
			}
#endif
			else
			{
				//channel died.
				if (state == CHANNEL_STATE_ACTIVE)
					state = CHANNEL_STATE_DEAD;
				ok = false;
				waitCount = 0;
				break;
			}
			lock();
			if (buf_free_list.size() > 0)
			{
				pnode = buf_free_list.front();
				buf_free_list.pop_front();
			}
			else
			{
				pnode = NULL;
			}
			unlock();
		}
#ifdef WIN32
		if ( ok )
		{
			for ( sct_buffer_list::iterator it = buf_pending_list.begin(); it != buf_pending_list.end(); ++it)
			{
				chan_buf_node* p = *it;
				hWaitHandles[waitCount] = 	p->hEvent;
				waitCount ++;
			}
		}
#endif
	}
#ifdef WIN32
	else
	{
		waitCount = 0;
		lock();
		if ( buf_ready_list.size() > 0)
		{
			pnode = buf_ready_list.front();
			buf_ready_list.pop_front();
		}
		unlock();
		while ( pnode != NULL )
		{
			memset( &(pnode->overlap), 0, sizeof(pnode->overlap) );
			pnode->overlap.hEvent = pnode->hEvent;
			ResetEvent(pnode->hEvent);
			if (board->ioControlEx(WINOFFLD_IOCTL_BUFFER_SEND,
				sizeof(int),
				(unsigned char *)&port,
				pnode->size,
				(unsigned char *)pnode->buf,
				&pnode->overlap))
			{
				buf_pending_list.push_back(pnode);
			}
			else
			{
				//channel died.
				state = CHANNEL_STATE_DEAD;
				ok = false;
				waitCount = 0;
				break;
			}
			lock();
			if ( buf_ready_list.size() > 0)
			{
				pnode = buf_ready_list.front();
				buf_ready_list.pop_front();
			}
			else
			{
				pnode = NULL;
			}
			unlock();
		}
		if ( ok )
		{
			for ( sct_buffer_list::iterator it = buf_pending_list.begin(); it != buf_pending_list.end(); ++it)
			{
				chan_buf_node* p = *it;
				hWaitHandles[waitCount] = 	p->hEvent;
				waitCount ++;
			}
		}
	}
	hWaitHandles[waitCount++] = hChanEvent;
	if ( state == CHANNEL_STATE_ACTIVE )
	{
		hWaitHandles[waitCount++] = hDestroyEvent;
	}
#endif
}

void sct_channel_info::handleIoComplete( sct_io_event hEv)
{
	if ( type == CHANNEL_RECV )
	{
		chan_buf_node * pnode = NULL;
#ifdef WIN32
		sct_buffer_list::iterator it;
		for ( it = buf_pending_list.begin();it != buf_pending_list.end();++it)
		{
			chan_buf_node * p = *it;
			if (p->hEvent == hEv)
			{
				pnode = p;
				buf_pending_list.erase( it );
				break;
			}
		}
#else
        sct_iocb_t * io = (sct_iocb_t *)( (char*)hEv - offsetof(sct_iocb_t, iocb));
        pnode = (chan_buf_node *)(io->obj);
        pnode->size = io->size;
        lock();
		sct_buffer_list::iterator it;
		for ( it = buf_pending_list.begin();it != buf_pending_list.end();++it)
		{
			chan_buf_node * p = *it;
            if (p == pnode)
            {
				buf_pending_list.erase( it );
                break;
            }
        }
        unlock();
#endif
		if ( pnode )
		{
#ifdef WIN32
			if ( board->getReceivedBytes(&pnode->overlap,&pnode->size) )
#else
            if ( pnode->size > 0)
#endif
			{
				if ( cb_func == 0 )
				{
					lock();
					buf_ready_list.push_back( pnode );
					unlock();
					OS_USEMAPHORE_POST(hSemaphore); //ReleaseSemaphore(hSemaphore,1,NULL);
				}
				else
				{
					lock();
					{
						buf_busy_list.push_back( pnode );
						unlock();
						board->issueChanCallback( cb_func, context, pnode->buf, pnode->size, this );
					}
				}
			}
			else
			{
				//Trace ("channel %d: received a 0 size buffer\n",port);
				lock();
				buf_free_list.push_back( pnode );
				unlock();
			}
		}
		else
		{
			Trace ("channel %d: invalid wait event in channel.\n",port);
		}
	}
	else
	{
		chan_buf_node * pnode = NULL;
#ifdef WIN32
		sct_buffer_list::iterator it;
		for ( it = buf_pending_list.begin();it != buf_pending_list.end();++it)
		{
			chan_buf_node * p = *it;
			if (p->hEvent == hEv)
			{
				pnode = p;
				buf_pending_list.erase( it );
				break;
			}
		}
#else
        sct_iocb_t * io = (sct_iocb_t *)( (char*)hEv - offsetof(sct_iocb_t, iocb));
        pnode = (chan_buf_node *)(io->obj);
        lock();
		sct_buffer_list::iterator it;
		for ( it = buf_pending_list.begin();it != buf_pending_list.end();++it)
		{
			chan_buf_node * p = *it;
            if (p == pnode)
            {
				buf_pending_list.erase( it );
                break;
            }
        }
        unlock();
#endif
		if ( pnode )
		{
			lock();
			buf_free_list.push_back( pnode );
			unlock();
			OS_USEMAPHORE_POST(hSemaphore);//ReleaseSemaphore(hSemaphore,1,NULL);
			if ( cb_func )
			{
				board->issueChanCallback( cb_func, context, pnode->buf, pnode->size, this );
			}
		}
		else
		{
			Trace ("channel %d: invalid wait event in channel.\n",port);
		}
	}
}

void sct_channel_info::channel_thread()
{
	quit = 0;
#ifdef WIN32
	while(!quit)
	{
		constructWaitArray();
		if ( waitCount == 0)
			break;
	    DWORD hr;
		hr = WaitForMultipleObjects(waitCount,hWaitHandles,false,INFINITE);

        if (quit)
        {
            break;
        }
		if ( ( hr >= 0 ) && ((int)hr < waitCount) )
		{
			if ( hWaitHandles[hr] == hChanEvent )
			{
				//handle Channel Event
				ResetEvent(hChanEvent);
			}
			else if ( hWaitHandles[hr] == hDestroyEvent )
			{
				//handle DestroyEvent
				state = CHANNEL_STATE_CLOSE_PENDING;
				ResetEvent(hDestroyEvent);
			}
			else
				handleIoComplete( hWaitHandles[hr] );
		}
#else
	constructWaitArray();
	while(!quit)
	{
        struct io_event event;
        os_ssize_t          retval = 0;
        struct timespec   to;
        to.tv_sec = 1;
        to.tv_nsec = 0;
        do {
            retval = io_getevents(aio_context,1,1,&event,&to);
        }
        while (retval == -EINTR);
        if ( quit )
            break;
        /*
	    if ( ( type == CHANNEL_RECV ) && (buf_pending_list.size()<=1))
        {
                printf("channel %d, nbuf=%d, pending_list size %d,busy_list size %d,ready_list size %d\n",port,nbuf,
                        buf_pending_list.size(),
                        buf_busy_list.size(),
                        buf_ready_list.size());
        }
        */
        if (retval == 1)
        {
            sct_io_event ev = (sct_io_event) event.obj;
            if ( ev == &(ol_destroy.iocb))
            {
                //channel get destroyed
                if (type == CHANNEL_RECV)
                {
				    state = CHANNEL_STATE_CLOSE_PENDING;
                }
                else
                {
                    break;
                }
            }
            else
            {
                sct_iocb_t * io = (sct_iocb_t *)( (char*)(ev) - offsetof(sct_iocb_t, iocb));
                io->size = (size_t)event.data;
                if ((int)io->size > 0)
                {
                    handleIoComplete(ev);
                }
                else
                {
                    break;
                }
            }
        }
#endif
	}
}

/*
void sct_channel_info::handleDestroy()
{
	//this function get called when board side closed the chcannel
	//Trace("channel %d: Get Destroy message\n",port);
	lock();
	if (type == CHANNEL_RECV)
	{
		state = CHANNEL_STATE_CLOSE_PENDING;
	}
	else if (type == CHANNEL_SEND)
	{
		if (state == CHANNEL_STATE_CLOSE_PENDING)
		{
			state = CHANNEL_STATE_CLOSED;
		}
	}
	ResetEvent(hDestroyEvent);
	ReleaseSemaphore(hSemaphore,1,NULL);
	unlock();
}
*/
