/**********************************************************************\
*                                                                      *
* Copyright (c) 2005 Stretch, Inc. All rights reserved.                *
*                                                                      *
* THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF *
* STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT *
* THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.                *
*                                                                      *
\**********************************************************************/

// All the sct board related functions

//---------------------------------------------------------------------
// Author:Xun Tan
// Date Created: 06 10 2007
//---------------------------------------------------------------------

#ifdef WIN32
#define _WIN32_WINNT 0x500
#include <windows.h>
#include "sct.h"
#include "sct_internal.h"

#include "Log.h"
#include "..\windows\Inc\ioctls.h"
//#pragma data_seg("SHARED")
//#pragma data_seg()
//#pragma comment(linker, "/section:SHARED,RWS")
#else
#include "sct.h"
#include "sct_internal.h"
#include "../linux/include/stretch_if.h"
#include <sys/ioctl.h>
#include <fcntl.h>
__asm__(".symver io_getevents, io_getevents@LIBAIO_0.4");
#endif


int sct_board_info::m_timeout = 10000;

thread_ret_t WINAPI callback_thread_func( LPVOID p)
{
	CallbackThread * ct = (CallbackThread *) p;
	ct->thread_func();
	return 0;
}

CallbackThread::CallbackThread()
{	
	quit = false;
    dwThreadId = 0;
	cb_list.clear();
    OS_UEVENT_CREATE(hWake);
    OS_UMUTEX_CREATE(hMutex);
	//hCBThread = CreateThread(NULL,0, callback_thread_func, this, 0, &dwThreadId );
    OS_UTHREAD_CREATE(dwThreadId,hCBThread,callback_thread_func,this);
}

CallbackThread::~CallbackThread()
{
	quit = true;
	OS_UEVENT_WAKE(hWake);
	if ( dwThreadId != 0 )
	{
        OS_UTHREAD_WAIT(hCBThread);
        OS_UTHREAD_DESTROY( hCBThread);
	}
    OS_UEVENT_DESTROY( hWake );
    OS_UMUTEX_DESTROY( hMutex );
}

void CallbackThread::thread_func()
{
	while ( !quit )
	{
		callback_node * p = NULL;
        OS_UMUTEX_LOCK(hMutex);
		if ( cb_list.size() > 0)
		{			
			p = cb_list.front();
			cb_list.pop_front();		
		}
		OS_UMUTEX_UNLOCK( hMutex );
		while ( p )
		{
			if (p->type == callback_node::CB_CHAN)
			{
                if (p->chan_param.chan)
				    (*p->chan_param.func)(p->chan_param.context,p->chan_param.buf,p->chan_param.size);
			}
			else if (p->type == callback_node::CB_MSG)
			{
				(*p->msg_param.func)(p->msg_param.context,p->msg_param.msg_class,p->msg_param.buf);
			}
			delete p;
            OS_UMUTEX_LOCK(hMutex);
			if ( cb_list.size() > 0)
			{			
				p = cb_list.front();
				cb_list.pop_front();		
			}
			else
			{
				p = 0;
			}
			OS_UMUTEX_UNLOCK( hMutex );
		}
		OS_UEVENT_RESET(hWake);
		OS_UEVENT_WAIT(hWake);
	}
}

void CallbackThread::addCallback( sct_callback_func * func, void * context, char * buf, int size, sct_channel_info * chan)
{
	callback_node *p = new callback_node;
	p->type = callback_node::CB_CHAN;
    p->chan_param.chan = chan;
	p->chan_param.func = func;
	p->chan_param.context = context;
	p->chan_param.buf = buf;
	p->chan_param.size = size;
	OS_UMUTEX_LOCK( hMutex);
	cb_list.push_back( p );
	OS_UMUTEX_UNLOCK( hMutex );
	OS_UEVENT_WAKE(hWake);
}


void CallbackThread::check_callbacks(sct_channel_info* chan)
{
    callback_node_list::iterator i;
	OS_UMUTEX_LOCK( hMutex);
    for (i=cb_list.begin();i!=cb_list.end();i++)
    {
        callback_node *p = *i;
        if (p->chan_param.chan == chan)
        {
            p->chan_param.chan = 0;
        }
    }
	OS_UMUTEX_UNLOCK( hMutex );
}


void CallbackThread::addCallback( sct_message_callback_func * func, void * context, char * buf, int msg_class)
{
	callback_node *p = new callback_node;
	p->type = callback_node::CB_MSG;
	p->msg_param.func = func;
	p->msg_param.context = context;
	memcpy(p->msg_param.buf,buf,SCT_MSG_DATA_SIZE);
	p->msg_param.msg_class = msg_class;
	OS_UMUTEX_LOCK( hMutex);
	cb_list.push_back( p );
	OS_UMUTEX_UNLOCK( hMutex );
	OS_UEVENT_WAKE(hWake);
}


CallbackThread	sct_board_info::cb_thread;

sct_msg_info::sct_msg_info(sct_board_info * bd, int c)
{
	m_board = bd;
	m_class = c;
#ifdef WIN32    
	hMsgEvent = INVALID_HANDLE_VALUE; 
	hWaitHandle = INVALID_HANDLE_VALUE;
#endif    
	OS_UMUTEX_CREATE(hMutex);
	initResources();
}

void sct_msg_info::initResources()
{
	cb_func = 0;
	ioctl_pending = false;
	message_pending = false;
}

void sct_msg_info::releaseResources()
{
}

sct_msg_info::~sct_msg_info()
{	
	//do this to make sure the last WINOFFLD_IOCTL_READ_MSG iocontrol is complete
	lock();
	if ( ioctl_pending )
	{
        int retry = 0;
		cb_func = 0;
		unlock();
		//m_board->getReceivedBytes(&msg_ol,&bytes);
        while ( ioctl_pending && (retry < 200))
        {
            retry++;
            OS_USLEEP(1);
        }
	}
	else
	{
		cb_func = 0;
		unlock();
	}
	releaseResources();
#ifdef WIN32    
	if (hMsgEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle( hMsgEvent );
		hMsgEvent = INVALID_HANDLE_VALUE;
	}
#endif    
    OS_UMUTEX_DESTROY(hMutex);
}

void sct_msg_info::lock()
{
	OS_UMUTEX_LOCK(hMutex);
}

void sct_msg_info::unlock()
{
	OS_UMUTEX_UNLOCK(hMutex);
}

void sct_msg_info::doAsyncReceive( )
{
	if (cb_func != 0)
	{	
		lock();
		if ( !ioctl_pending)
		{
			message_pending = false;
			ioctl_pending = true;
			memset( &cur_msg, 0, sizeof(sct_dll_msg_t) );
			cur_msg.classId = m_class;
    
			memset( &msg_ol, 0, sizeof(msg_ol) );
#ifdef WIN32
			if ( hMsgEvent == INVALID_HANDLE_VALUE)
			{
				hMsgEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
			}
			ResetEvent( hMsgEvent );
			msg_ol.hEvent = hMsgEvent;
			if (hWaitHandle != INVALID_HANDLE_VALUE)
			{		
				UnregisterWait( hWaitHandle );
			}
			RegisterWaitForSingleObject(&hWaitHandle, hMsgEvent, &BoardEventCallback,this,INFINITE,WT_EXECUTEONLYONCE);
			//Trace("Call iocontrol in %d\n",m_class);
#else
            msg_ol.context = m_board->msg_aio_context;
            msg_ol.obj = this;
#endif            
			if( !m_board->ioControl( WINOFFLD_IOCTL_READ_MSG,
                               sizeof(sct_dll_msg_t),
                               (unsigned char *)&cur_msg,
                               &msg_ol) )
			{
				Trace(  __FUNCTION__ "iocontrol failed\n" );
				ioctl_pending = false;
			}
		}		
		unlock();
	}
}

bool sct_msg_info::doReceive(void * buffer)
{
	bool ret = false;
	while (ioctl_pending)
	{
		OS_USLEEP(1);
	}
	lock();
	if ( !message_pending)
	{		
		memset( &cur_msg, 0, sizeof(sct_dll_msg_t) );
		cur_msg.classId = m_class;
		if( m_board->ioControl( WINOFFLD_IOCTL_READ_MSG,
			sizeof(sct_dll_msg_t),
			(unsigned char *)&cur_msg,
			NULL) ) 
		{			
			memcpy( buffer, cur_msg.msg.data, SCT_MAX_MESSAGE_LEN );
			ret = true;     
		}
		else
		{
			Trace( __FUNCTION__ ": ioControl failed\n" );
		}
		unlock();
	}
	else
	{	
		message_pending = false;
		//copy the received message
		memcpy(buffer, cur_msg.msg.data, SCT_MAX_MESSAGE_LEN);
		ret = true;
		unlock();
	}
	return ret;
}

void sct_msg_info::wakeup( )
{
	lock();
	//hWaitHandle = INVALID_HANDLE_VALUE;		
	message_pending = (cb_func == 0);	
	if (cb_func !=0)
	{
		m_board->issueMsgCallback(cb_func,context, (char*)cur_msg.msg.data,cur_msg.classId);		
	}
	ioctl_pending = false;
	unlock();
	doAsyncReceive( );
}

bool sct_msg_info::regCallback( sct_message_callback_func* func, void * p)
{
	lock();
	cb_func = func;
	context = p;
	unlock();
	doAsyncReceive();
	return true;
}

#ifndef WIN32
void * msg_thread_func( void * p)
{
	sct_board_info * bi = (sct_board_info *) p;
	bi->msgThreadFunc();
	return 0;
}

void sct_board_info::msgThreadFunc()
{
    while (!quit)
    {
        struct io_event event;
        os_ssize_t          retval = 0;
        struct timespec   to;
        to.tv_sec = 1;
        to.tv_nsec = 0;
        do
        {
            retval = io_getevents(msg_aio_context,1,1,&event,&to);
        }
        while (retval == -EINTR);
        if (retval == 1)
        {
            sct_io_event ev = (sct_io_event) event.obj;
            sct_iocb_t * msg_io = (sct_iocb_t *)( (char*)ev - offsetof(sct_iocb_t, iocb));
            sct_msg_info * mi = (sct_msg_info*)(msg_io->obj);
            if ( &(mi->msg_ol.iocb) == ev )
            {
                mi->wakeup();
            }
        }
    }
}

#endif

sct_board_info::sct_board_info()
{
	//Trace("board_info %x size %x\n",this,sizeof(sct_board_info));
	int i;
	OS_UMUTEX_CREATE(hBoardMutex);
	for ( i = 0; i< SCT_MSG_CLASS_COUNT; i++)
	{
		msg_info[i] = new sct_msg_info(this,i);
	}
	device_path = NULL;
	m_bootloader_ver = 0;
#ifndef WIN32
    hMsgThread = 0;
    os_io_setup( 256, &msg_aio_context );
#endif
	initResources();
}

void sct_board_info::initResources()
{
	int i;
	m_sig = BOARD_SIGNATURE;
	ipc_inited = 0;
	open = false;
	booted = false;	
    OS_UFILE_VALIDATE( handle );
	for ( i = 0; i< SCT_MSG_CLASS_COUNT; i++)
	{
		msg_info[i]->initResources();
	}
}

void sct_board_info::releaseResources( )
{
	sct_channel_map::iterator it;
	int i;
	if ( open )
	{	
		OS_UFILE_CLOSE( handle);
	}	
	for (it = chan_map.begin();it !=chan_map.end();++it)
	{
		sct_channel_info * chan = (*it).second;
		if (chan)
		{
			chan->close();
			delete chan;
		}
	}
	chan_map.clear();
	for ( i = 0; i< SCT_MSG_CLASS_COUNT; i++)
	{
		msg_info[i]->releaseResources();
	}
}

sct_board_info::~sct_board_info()
{
    releaseResources();
#ifndef WIN32
    quit = 1;
    if (hMsgThread != 0)
        OS_UTHREAD_WAIT(hMsgThread);
    hMsgThread = 0;
#endif    
    os_io_destroy ( msg_aio_context );
    if ( device_path != 0)
    {
        delete device_path;		
    }	
    OS_UMUTEX_DESTROY( hBoardMutex );		
    for ( int i=0;i<SCT_MSG_CLASS_COUNT;i++)
    {
        delete msg_info[i];
    }
}

void sct_board_info::issueChanCallback( sct_callback_func * func, void * context, char * buf, int size, sct_channel_info * chan)
{
	cb_thread.addCallback( func,context,buf,size, chan);
}

void sct_board_info::check_callbacks(sct_channel_info * chan)
{
    cb_thread.check_callbacks(chan);
}
        

void sct_board_info::issueMsgCallback( sct_message_callback_func * func, void * context, char * buf, int msg_class)
{
	cb_thread.addCallback( func,context,buf,msg_class);
}

void sct_board_info::removeChannel( sct_channel_info * chan)
{
	lock();
	if ( chan_map.erase( chan->port ) )
	{
		delete chan;
	}
	unlock();
}

sct_channel_info * sct_board_info::newChannel( int port, int nbuf, int size)
{
	sct_channel_info * chan = 0;
	lock();
	if ( !isChannelInUse ( port ) )
	{		
		chan = new sct_channel_info;
		chan->port = port;		
		// Initalizae channel structure.    
		chan->board      = this;           		     
		chan->initMemory(nbuf,size);
		chan_map[port] = chan;
	}
	unlock();
	return chan;
}


bool sct_board_info::isChannelInUse( int port )
{
	return chan_map.find( port ) != chan_map.end();
}

void sct_board_info::setDevicePath( char * name)
{
	if ( device_path != 0)
	{
		delete[] device_path;
	}

	int len = (int) strlen( name ) + 1;
	device_path = new char[len];
#ifdef strcpy_s
	strcpy_s(device_path,len,name);
#else
	strcpy(device_path,name);
#endif
}

void sct_board_info::lock()
{
	OS_UMUTEX_LOCK( hBoardMutex );
}

void sct_board_info::unlock()
{
	OS_UMUTEX_UNLOCK( hBoardMutex );
}

bool sct_board_info::initIPCQueue()
{
	if( ipc_inited )
    {
        return true;
    }

	lock();
	// Same buffer for input and output.
	QUEUE				ipc_queue;
    ipc_queue.status = -1;

    if( !ioControl( WINOFFLD_IOCTL_IPC_QUEUE_INIT,                           
                            sizeof(QUEUE),
                            (unsigned char*) &(ipc_queue),
                            NULL) )
    {
        Trace( __FUNCTION__ ": Error initializing queue module.\n" );        
    }    
	else
	{
		ipc_inited = 1;
	}
	unlock();
	return ipc_inited;
}

#ifdef WIN32
BOOL sct_board_info::getReceivedBytes( sct_iocb_t* pop,DWORD *plen)
{
    return GetOverlappedResult(handle,pop,plen,0);
}
#endif

#ifdef WIN32
BOOL sct_board_info::ioControl( DWORD	        dwIoCode,                    
                    DWORD	        dwNumBytes, 
                    unsigned char *  Buf,
                    sct_iocb_t *     pOverlapped )
{
	ULONG   nOutput;
	BOOL        ret = false;

	if ( handle == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	if( pOverlapped )
	{
		BOOL ok = DeviceIoControl( handle, 
        						dwIoCode,
        						Buf, 
        						dwNumBytes,
        						Buf, 
        						dwNumBytes,
        						&nOutput, 
        						pOverlapped );
		if (ok)
		{
			ret = true;
		}
		else if ((GetLastError() == ERROR_IO_PENDING))
		{
			ret = true;
		}
		return ret;
	}

	
	OVERLAPPED  ol;

    memset( &ol, 0, sizeof(OVERLAPPED) );
    ol.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	ret = DeviceIoControl( handle, 
        				   dwIoCode,
        				   Buf, 
        				   dwNumBytes,
        				   Buf, 
        				   dwNumBytes,
        				   &nOutput, 
        				   &ol );

	if( !ret )
	{
		if( (GetLastError() == ERROR_IO_PENDING) )
		{
			DWORD hr = WaitForSingleObject(ol.hEvent, m_timeout);
			if (hr == WAIT_OBJECT_0)
			{
				ret = true;
			}
		}
	}

	CloseHandle( ol.hEvent );
    return ret;
}

BOOL sct_board_info::ioControlEx( DWORD              dwIoCode,                     
                     DWORD	            dwNumInBytes, 
                     unsigned char *    inBuf,
                     DWORD	            dwNumOutBytes, 
                     unsigned char *    outBuf,
                     OVERLAPPED *       pOverlapped )
{
	ULONG   nOutput;
	BOOL        ret = false;

	if ( handle == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	if( pOverlapped )
	{
		BOOL ok = DeviceIoControl(	handle, 
        						dwIoCode,
        						inBuf, 
        						dwNumInBytes,
        						outBuf, 
        						dwNumOutBytes,
        						&nOutput, 
        						pOverlapped );
		int err = GetLastError();
		if ( ok )
		{
			ret = true;
		}
		else if (( err == ERROR_IO_PENDING))
		{
			ret = true;
		}
		return ret;
	}
	
	OVERLAPPED  ol;

    memset( &ol, 0, sizeof(OVERLAPPED) );
    ol.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	ret = DeviceIoControl(	handle,
        					dwIoCode,
        					inBuf, 
        					dwNumInBytes,
        					outBuf, 
        					dwNumOutBytes,
        					&nOutput, 
        					&ol );

	if( !ret )
	{
		if( (GetLastError() == ERROR_IO_PENDING) &&
			(WaitForSingleObject(ol.hEvent, m_timeout) == WAIT_OBJECT_0) )
		{
			ret = true;
		}
	}

	CloseHandle( ol.hEvent );
    return ret;
}
#else
BOOL sct_board_info::ioControl( sct_iocode_t	        dwIoCode,                    
                    DWORD	        dwNumBytes, 
                    unsigned char *  buf,
                    sct_iocb_t *     pOverlapped )
{
    BOOL    ret = 0;
    if ( !OS_UFILE_IS_VALID( handle))
    {
        return ret;
    }
    if ( pOverlapped )
    {
        if (pOverlapped->context !=0 )
        {
            int retval;
            struct iocb * pol = &(pOverlapped->iocb);
            io_prep_pwrite(pol,handle,buf,1,dwIoCode);
            io_set_callback(pol,NULL);
            pol->key = -1;
            retval = io_submit(pOverlapped->context,1,&pol);
            ret = (retval == 1);
        }
    }
    else
    {
        if (_IOC_TYPE(dwIoCode) == SX_IOTYPE_NORM)
        {
         int r = ioctl(handle, (unsigned long int)dwIoCode, buf);
         ret = (r == 0); 
        }
        else 
        {
            int retval;
            struct iocb   iocb;
            struct iocb  *pol = &iocb;
            struct io_context * int_aio_context = 0;
            os_io_setup( 1, &int_aio_context );
            io_prep_pwrite(pol,handle,buf,1,dwIoCode);
            io_set_callback(pol,NULL);
            pol->key = -1;
            retval = io_submit(int_aio_context,1,&pol);
            if (retval == 1)
            {
                struct io_event event;
    			struct timespec     to;
                do 
                {                    
            		to.tv_sec = m_timeout/1000;
            		to.tv_nsec = (m_timeout%1000)*1000;
                    retval = io_getevents(int_aio_context,1,1,&event,&to);
                }
                while ( retval == -EINTR );
            }
            os_io_destroy ( int_aio_context );
            ret = (retval == 1);
        }
    }
    return ret;
}
#endif

bool sct_board_info::closeBoard()
{
	lock();
	releaseResources();
	initResources();
	unlock();	
	return true;
}

bool sct_board_info::openBoard()
{
	// Open a handle to the board. Access is unshared, and I/O must be
    // overlapped for the API to work.
    lock();
	if ( !open )
    {
        OS_UFILE_OPEN(handle,device_path);
		if( OS_UFILE_IS_VALID(handle) )
		{
			
			if( !ioControl( WINOFFLD_IOCTL_READ_CONFIG,
				sizeof(dndev_t),
				(unsigned char*)&(tDnDev),
				NULL) ) 
			{
                OS_UFILE_CLOSE(handle);
                OS_UFILE_VALIDATE(handle);
			}
		}
		if( OS_UFILE_IS_VALID(handle) )
		{
			open   = true;
		}
	}
	unlock();
	if (open)
	{
#ifndef WIN32
        quit = 0;        
        if ( hMsgThread == 0 )
        {
            OS_UTHREAD_CREATE(dwMsgThreadId, hMsgThread, msg_thread_func,this);
        }
#endif    
		ioControl(WINOFFLD_IOCTL_GET_BOARD_DETAIL,sizeof(sct_board_detail_t),(unsigned char*)&bd_detail,NULL);
		bd_detail.board_number = bd_num;
	}
	//initIPCQueue();
	return open;
}

unsigned int sct_board_info::sendMessage(int msg_class,void * buffer,int size)
{
	unsigned int ret = SCT_ERR_MSG_SEND;
	if( !buffer )
    {
        Trace( __FUNCTION__ ": Error, NULL buffer for message.\n" );
        return SCT_ERR_MSG_SEND;
    }

    if( !size || size > SCT_MAX_MESSAGE_LEN)
    {
        Trace( __FUNCTION__ ": Error, zero size for message.\n" );
       return SCT_ERR_MSG_SEND;;
    }
  
    if( !initIPCQueue( ) )
    {
		return SCT_ERR_MSG_SEND;
    }
	
    
    sct_dll_msg_t  msg;

    msg.classId  = msg_class;
    msg.msg.size = size;
    
    memset( msg.msg.data, 0, SCT_MAX_MESSAGE_LEN );
    memcpy( msg.msg.data, buffer, size );

    if( !ioControl( WINOFFLD_IOCTL_WRITE_MSG,
                            sizeof(sct_dll_msg_t),
                            (unsigned char *)&msg,
                            NULL) ) 
    {		
			Trace( __FUNCTION__ ": Error sending message, code %d.\n",
				GetLastError() );		
    }
	else
	{
			ret = SCT_NO_ERROR;	
	}
      
	return ret;
}

unsigned int sct_board_info::recvMessage(int& msg_class,void * buffer)
{
	unsigned int ret = SCT_ERR_MSG_RECV;
	if( !buffer )
    {
        Trace( __FUNCTION__ ": Error, NULL buffer for message.\n" );        
        return SCT_ERR_MSG_RECV;
    }

	if (msg_class >= 0)
	{
		if ( msg_info[msg_class]->isCallBackRegistered() )
		{
			Trace( __FUNCTION__ ": receive message can not work if a message callback function is registered.\n" );
			return SCT_ERR_MSG_RECV;
		}
	}

    if( !initIPCQueue( ) )
    {
        return SCT_ERR_MSG_RECV;
    }          
    
	if ( msg_info[msg_class]->doReceive(buffer) )
	{
		ret = SCT_NO_ERROR;
	}

	return ret;    	
}

unsigned int sct_board_info::pollMessage(int& msg_class,void * buffer)
{
	unsigned int ret = SCT_ERR_MSG_RECV;
	if( !buffer )
    {
        Trace( __FUNCTION__ ": Error, NULL buffer for message.\n" );        
		return ret;
    }
     
    if( !initIPCQueue( ) )
    {
        return ret;
    }
   
    sct_dll_msg_t   msg;

    msg.classId = msg_class;
    if( ioControl( WINOFFLD_IOCTL_READ_MSG_POLL,
                           sizeof(sct_dll_msg_t),
                           (unsigned char *)&msg,
                           NULL) ) 
    {   
		msg_class = msg.classId;
		memcpy( buffer, msg.msg.data, SCT_MAX_MESSAGE_LEN );
		ret = SCT_NO_ERROR;
    }
	else
	{
		Trace( __FUNCTION__ ": ioControl failed\n" );
	}

	return ret;
}

bool sct_board_info::regCallback( int msgclass, sct_message_callback_func* func, void * p)
{
	return 	msg_info[msgclass]->regCallback( func, p);	
}

int sct_board_info::WriteMem(int bar, int addr, void * buf, int size)
{
#ifdef WIN32    
   int args[3];
   unsigned long   nOutput;
   args[0] = 0;
   args[1] = bar;
   args[2] = addr;
   BOOL ok = DeviceIoControl( handle,WINOFFLD_IOCTL_TEST_MEMORY,
                              args,
                              sizeof(args),
                              buf,
                              size,
                              &nOutput,
                              NULL );
   // Return 0=PASS, 1=FAIL
   return (!ok);
#else
   return 1;
#endif

}

int sct_board_info::ReadMem(int bar, int addr, void * buf, int size)
{
#ifdef WIN32    
   int args[3];
   unsigned long   nOutput;
   args[0] = 1;
   args[1] = bar;
   args[2] = addr;
   BOOL ok = DeviceIoControl( handle, WINOFFLD_IOCTL_TEST_MEMORY,
                              args,
                              sizeof(args),
                              buf,
                              size,
                              &nOutput,
                              NULL );
   // Return 0=PASS, 1=FAIL
   return (!ok);
#else 
   return 1;
#endif   
}

bool sct_board_info::loadFirmware( char * path, sct_diag_info_t* pdiag)
{
	BOOL ret = 0;
	char * filebuf;
    int size = 0;
#ifdef WIN32
    {
	    WIN32_FILE_ATTRIBUTE_DATA   fileInfo;
		int ok;
	    ok = GetFileAttributesEx(path, GetFileExInfoStandard, (void*)&fileInfo);
        if  (ok )
            size = fileInfo.nFileSizeLow;
    }
#else
    {
        struct stat st_buf;
        if (stat( path, &st_buf) == 0)
        {
            size = st_buf.st_size;
        }
    }
#endif    
	if ( size > 0 )
	{
		filebuf = new char[ size ];
		FILE * fp = fopen( path,"rb");
		if ( fp != 0)
		{
			fread(filebuf,1,size,fp);
			fclose(fp);
#ifdef WIN32            
			//ret = DeviceIoControl( handle,WINOFFLD_IOCTL_BOOT_FIRMWARE, filebuf, size,0,0,&nOutput,0);
			if ( pdiag == NULL )
			{
				ret = ioControl( WINOFFLD_IOCTL_BOOT_FIRMWARE,
                            size,
                            (unsigned char *)filebuf,						
                            NULL);
				if (ret)
				{
					m_bootloader_ver = ((int *)filebuf)[0];
				}
			}
			else
			{
				ret = ioControl( WINOFFLD_IOCTL_BOOT_FIRMWARE_WAIT,
                            size,
                            (unsigned char *)filebuf,						
                            NULL);
				if (ret)
				{
					sx_uint32 * p = (sx_uint32*) filebuf;
					m_bootloader_ver = p[0];
					p++;
					if( p[SCT_MAX_PE]==0)
					{
						//default case, assume 4 pes
						pdiag->pe_num = 4;						
					}
					else
					{
						pdiag->pe_num = p[SCT_MAX_PE];
						if (pdiag->pe_num > SCT_MAX_PE)
							pdiag->pe_num = SCT_MAX_PE;
					}
					memcpy(pdiag->err_nums,p,pdiag->pe_num*sizeof(sx_uint32));
				}
			}
#else
            sct_boot_info_t bi;
            memset( &bi, 0, sizeof(bi));
            bi.data = (uint8_t *)filebuf;
            bi.size = size;
            bi.bwait = (pdiag != 0);
            ret = ioControl(SX_IOCTL_BOOT_FIRMWARE, sizeof(bi),(uint8_t*)&bi,NULL);
            if (ret )
            {
                m_bootloader_ver = bi.version;
                if (pdiag)
                {
                    pdiag->pe_num = bi.pe_num;
                    memcpy(pdiag->err_nums,&(bi.result[0]),32*sizeof(sx_uint32));
                }
            }
#endif            
		}
		delete[] filebuf;
	}
	return ret==1;
}

sct_board_detail_t	* sct_board_info::getBoardDetail()
{
	return &bd_detail;
}

