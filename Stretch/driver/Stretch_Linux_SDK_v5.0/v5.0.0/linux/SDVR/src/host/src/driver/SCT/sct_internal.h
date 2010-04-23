/**********************************************************************\
*                                                                      *
* Copyright (c) 2005 Stretch, Inc. All rights reserved.                *
*                                                                      *
* THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF *
* STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT *
* THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.                *
*                                                                      *
\**********************************************************************/

// Internal declarations for SCT DLL.

#ifndef _SCT_INTERNAL_H_
#define _SCT_INTERNAL_H_

#include "sct.h"
#ifdef WIN32
#include "systypes.h"
#else
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <libaio.h>
#include <semaphore.h>
#include "../linux/include/os_arch.h"
#include "../linux/include/ipcqueue.h"
#include "../linux/include/ipcmsg.h"
#endif
#include <map>
#include <list>

#ifdef WIN32
    typedef HANDLE sct_io_event;
	typedef HANDLE				os_uevent_t;
	#define OS_UEVENT_CREATE(e)	e = CreateEvent(NULL,TRUE,FALSE,NULL)
	#define OS_UEVENT_WAIT(e)	WaitForSingleObject( e, INFINITE )
	#define OS_UEVENT_WAKE(e)	SetEvent(e)
	#define OS_UEVENT_RESET(e)  ResetEvent(e)
	#define OS_UEVENT_DESTROY(e) CloseHandle(e)
	typedef HANDLE				os_umutex_t;	
	#define OS_UMUTEX_CREATE(m) m = CreateMutex(NULL,FALSE,NULL)
    #define OS_UMUTEX_LOCK(m)   WaitForSingleObject( m, INFINITE )
    #define OS_UMUTEX_TRY_LOCK(m)   ((WaitForSingleObject( m, 0 )==WAIT_OBJECT_0)?0:-1)
    #define OS_UMUTEX_UNLOCK(m) ReleaseMutex( m )
    #define OS_UMUTEX_DESTROY(m) CloseHandle(m)
    typedef HANDLE              os_usemaphore_t; 
    #define OS_USEMAPHORE_CREATE(sem,num) sem = CreateSemaphore(NULL,num,256,NULL);
    #define OS_USEMAPHORE_WAIT(sem)  ((WaitForSingleObject((sem),INFINITE) == WAIT_OBJECT_0 )? 0: 1)
    #define OS_USEMAPHORE_WAIT_TIMEOUT(sem,timeout)  WaitForSingleObject((sem),(timeout))
    #define OS_USEMAPHORE_TRYWAIT(sem) ((WaitForSingleObject( m, 0 )==WAIT_OBJECT_0)?0:-1)
    #define OS_USEMAPHORE_POST(sem) ReleaseSemaphore(sem,1,NULL)
    #define OS_USEMAPHORE_DESTROY(sem)  CloseHandle((sem))
    #define OS_UTIMEOUT(s)  (s == WAIT_TIMEOUT)
    typedef HANDLE              sct_thread_t;
    typedef DWORD               sct_thread_id_t;
    typedef DWORD               sct_iocode_t;
    typedef OVERLAPPED          sct_iocb_t;
	typedef sct_dll_msg			sct_dll_msg_t;
    #define OS_UTHREAD_CREATE(id,th, func,arg)    th = CreateThread(NULL,0,func,arg,0,&id)
    #define OS_UTHREAD_DESTROY(th)  CloseHandle( th )
    #define OS_UTHREAD_WAIT( th)     WaitForSingleObject( th ,INFINITE)
    typedef DWORD  thread_ret_t;
    #define OS_USLEEP(m)       Sleep(m)
    typedef HANDLE              os_ufile_t;
    #define OS_UFILE_OPEN( fh, fn ) fh = CreateFile( (LPCSTR)fn,\
                    			GENERIC_READ | GENERIC_WRITE,\
			                    0,  \
                    			NULL,\
			                    OPEN_EXISTING,\
			                    FILE_FLAG_OVERLAPPED,\
			                    NULL )
    #define OS_UFILE_CLOSE(fh)      CloseHandle(fh);
    #define OS_UFILE_IS_VALID( fh ) (fh != INVALID_HANDLE_VALUE)
    #define OS_UFILE_VALIDATE( fh ) fh = INVALID_HANDLE_VALUE
    #define os_io_setup(a,b)
    #define os_io_destroy(a)
#else    
    typedef struct iocb * sct_io_event;
    typedef pthread_t           sct_thread_t;
    typedef int                 sct_thread_id_t;
    typedef struct  sct_iocb_tag
    {
        struct iocb iocb;
        struct io_context *  context;
        size_t  size;
        void * obj;
    } sct_iocb_t;
    typedef void                VOID;
    typedef int                 BOOL;
    typedef void *              PVOID;
    typedef void *              LPVOID;
    typedef int                 BOOLEAN;
    #define WINAPI
    #define CALLBACK    
    #define TRUE    1
    #define FALSE   0
	#define OS_UEVENT_RESET(e) 
    #define OS_UTHREAD_CREATE(id,th,func,arg)    id = pthread_create(&th,0,func,arg)
    #define OS_UTHREAD_DESTROY(th)  
    #define OS_UTHREAD_WAIT( th)     pthread_join( th, NULL )
    #define OS_USEMAPHORE_WAIT_TIMEOUT(sem,timeout)  my_sem_wait(&sem,timeout)
    #define OS_UTIMEOUT(s)  (s < 0)
    typedef void *  thread_ret_t;
    #define OS_USLEEP(m)       usleep((m)*1000)
    #define Trace   OS_PRINTF
    #define  WINOFFLD_IOCTL_READ_MSG   SX_IOCTL_READ_MSG 
    #define  WINOFFLD_IOCTL_IPC_QUEUE_INIT  SX_IOCTL_IPC_QUEUE_INIT
    #define  WINOFFLD_IOCTL_READ_CONFIG     SX_IOCTL_READ_CONFIG
    #define  WINOFFLD_IOCTL_WRITE_MSG      SX_IOCTL_WRITE_MSG
    #define  WINOFFLD_IOCTL_READ_MSG_POLL   SX_IOCTL_READ_MSG_POLL
    #define  WINOFFLD_IOCTL_BOOT_FIRMWARE   SX_IOCTL_BOOT_FIRMWARE
    #define  WINOFFLD_IOCTL_BOOT_FIRMWARE_WAIT  SX_IOCTL_BOOT_FIRMWARE
    #define  WINOFFLD_IOCTL_GET_BOARD_DETAIL    SX_IOCTL_GET_BOARD_DETAIL
    #define  WINOFFLD_IOCTL_DESTROY_RESPONSE    SX_IOCTL_DESTROY_RESPONSE
    #define  WINOFFLD_IOCTL_WRITE_MGMT_QUEUE    SX_IOCTL_WRITE_MGMT_QUEUE
    #define  WINOFFLD_IOCTL_CHECK_CONNECT    SX_IOCTL_CHECK_CONNECT
    #define  WINOFFLD_IOCTL_WAIT_DESTROY     SX_IOCTL_WAIT_DESTROY
    #define  WINOFFLD_IOCTL_ACCEPT_RESPONSE    SX_IOCTL_ACCEPT_RESPONSE
    #define  WINOFFLD_IOCTL_BUFFER_SEND   SX_IOCTL_BUFFER_SEND
    #define  WINOFFLD_IOCTL_STORE_RECEIVE_BUFFER   SX_IOCTL_STORE_RECEIVE_BUFFER
    #define  WINOFFLD_IOCTL_RESET_S5530 SX_IOCTL_RESET_S6
    typedef int              os_ufile_t;
    #define OS_UFILE_OPEN( fh, fn ) fh = ::open(fn, O_RDWR | O_ASYNC )
    #define OS_UFILE_CLOSE(fh)      close(fh);
    #define OS_UFILE_IS_VALID( fh ) (fh >= 0 )
    #define OS_UFILE_VALIDATE( fh ) fh = -1
    typedef struct QUEUE_TAG {
            int status;
    } QUEUE;
    typedef uint64_t               sct_iocode_t;
    typedef uint32_t                DWORD;
    typedef sct_mgmt_msg_t         t_sct_mgmt_msg;
#endif    

/**********************************************************************
    Data channel related defines.
**********************************************************************/
#define CHANNEL_SEND    0
#define CHANNEL_RECV    1

enum{ 
	CHANNEL_STATE_NONE,
	CHANNEL_STATE_ACTIVE,
	CHANNEL_STATE_CLOSE_PENDING,
	CHANNEL_STATE_CLOSED,
	CHANNEL_STATE_CONNECT_PENDING,
	CHANNEL_STATE_DEAD	
};

class sct_board_info;
struct chan_buf_node
{
public:
	chan_buf_node( int bufsize );
	~chan_buf_node();

	char			        * buf;
	unsigned long			size;
	sct_iocb_t		        overlap;
#ifdef WIN32    
	sct_io_event			hEvent;
#endif    
};
typedef std::list< chan_buf_node *> sct_buffer_list;
#define MAX_EVENT 64
/**********************************************************************
    Data channel structure.
**********************************************************************/
class sct_channel_info
{
public:	
	friend class sct_board_info;
	sct_channel_info();
	~sct_channel_info();	
	bool initMemory(int nbuf, int size);
	void releaseResource();
	void releaseBuffers();
	int  sig(){ return m_sig;}	
	int  getPort(){ return port;}
	int  getType(){ return type;}
	sct_board_info* getBoard(){ return board;}
	unsigned int close();
	void *getFreeBuffer( int timeout=0 );
	void lock();
	void unlock();
	unsigned int sendBuffer(void * buffer,sx_int32 size);
	unsigned int connect();
	unsigned int accept();
	bool isRecvBufferAvailable();
	void * getRecvBuffer( int & size );
	void returnRecvBuffer( void * p);	
	//void handleDestroy();
	bool regCallback( sct_callback_func* func, void * p);	
	void channel_thread();
protected:
	
	void startChannelThread();
	void constructWaitArray();
	void handleIoComplete( sct_io_event hEv );
	
	int m_sig;	
	sct_iocb_t    ol_destroy;
	int port; // to hold channel port number.
	int nbuf; // To hold number of buffers allocated on this channel.
	int max_size; // To maximum size of buffers on this channel. 
	int type; // To hold type of the channel
	sct_board_info*     board;
	chan_buf_node**		buffer_array; // To hold all the buffers
	sct_buffer_list		buf_free_list;
	sct_buffer_list		buf_busy_list;
	sct_buffer_list		buf_ready_list;
	sct_buffer_list		buf_pending_list;	

	int state; // To hold state of the channel use one of the following. 		
	os_umutex_t hChanMutex;
	os_usemaphore_t hSemaphore;
#ifdef WIN32    
	HANDLE hChanEvent;
	HANDLE hDestroyEvent;
	HANDLE hWaitHandles[MAX_EVENT];
#else
    struct io_context  *aio_context;
#endif    
	int		waitCount;
	sct_thread_t	hChanThread;
	sct_thread_id_t	dwThreadId;
	bool	quit;
	sct_callback_func * cb_func;
	void *				context;
};

class sct_board_info;
typedef std::map<int,sct_channel_info*> sct_channel_map;
class sct_msg_info
{
    friend class sct_board_info;
public:
	sct_msg_info( sct_board_info* board, int c);
	~sct_msg_info();
	void wakeup();
	bool isCallBackRegistered() { return cb_func != 0; }
	bool regCallback(sct_message_callback_func* func, void * p);
	void initResources();
	void releaseResources( );
	bool doReceive(void * buffer);

protected:
	void doAsyncReceive();	
	inline void lock();
	inline void unlock();

	sct_board_info * m_board;
	int				m_class;	
	sct_iocb_t			msg_ol;	
	volatile bool		ioctl_pending;
	bool				message_pending;
#ifdef WIN32    
	HANDLE				hWaitHandle;
	HANDLE				hMsgEvent;
#endif    
	sct_message_callback_func	* cb_func;	
	void *				context;	
	sct_dll_msg_t			cur_msg;		
	os_umutex_t				hMutex;	
};

struct callback_node
{
	enum callback_type
	{
		CB_CHAN,
		CB_MSG
	};
	callback_type	type;
	union{
		struct{
			sct_callback_func *func;
			void *	context;
			char *	buf;
			int		size;
            sct_channel_info * chan;
		}chan_param;
		struct {
			sct_message_callback_func *func;
			void * context;
			char 	buf[SCT_MSG_DATA_SIZE];
			int		msg_class;
		}msg_param;
	};
};

typedef std::list< callback_node *> callback_node_list;

class CallbackThread
{
public:
	CallbackThread();
	~CallbackThread();
	void addCallback( sct_callback_func * func, void * context, char * buf, int size, sct_channel_info* chan);
	void addCallback( sct_message_callback_func * func, void * context, char * buf, int msg_class);
    void check_callbacks(sct_channel_info* chan);
	void thread_func();
protected:
	bool	quit;
	sct_thread_t	hCBThread;
	os_umutex_t	hMutex;
	sct_thread_id_t	dwThreadId;
	callback_node_list	cb_list;
	os_uevent_t	hWake;
};

/**********************************************************************
    Board structure.
**********************************************************************/
class sct_board_info
{
public:	
	sct_board_info();
	~sct_board_info();
//member functions
	void removeChannel( sct_channel_info * chan);
	sct_channel_info * newChannel( int port, int nbuf, int size);
	sct_channel_info * findChannelByPort( int port );
	bool isChannelInUse( int port );	

	bool initIPCQueue();
	inline void lock();
	inline void unlock();
	unsigned int sendMessage(int msg_class,void * buffer,int size);
	unsigned int recvMessage(int& msg_class,void * buffer);
	unsigned int pollMessage(int& msg_class,void * buffer);	
	int WriteMem(int bar, int addr, void * buf, int size);
	int ReadMem(int bar, int addr, void * buf, int size);
    bool loadFirmware( char * path, sct_diag_info_t* perrno);
	BOOL ioControl( sct_iocode_t	        dwIoCode,                    
                    DWORD	        dwNumBytes, 
                    unsigned char *  Buf,
                    sct_iocb_t *     pOverlapped );
#ifdef WIN32
	BOOL ioControlEx( sct_iocode_t              dwIoCode,                     
                     DWORD	            dwNumInBytes, 
                     unsigned char *    inBuf,
                     DWORD	            dwNumOutBytes, 
                     unsigned char *    outBuf,
                     sct_iocb_t *       pOverlapped );
	BOOL getReceivedBytes( sct_iocb_t * pop,DWORD* plen);
#else
    struct io_context * msg_aio_context;
#endif    
	char *devicePath() { return device_path;}
	void setDevicePath( char * name );
	bool isOpened() {return open;}
	bool openBoard();
	bool closeBoard();
	int  sig(){ return m_sig;}	
	
	bool regCallback( int msgclass, sct_message_callback_func* func, void * p);	
    void set_timeout(int timeout) { m_timeout = timeout; }
	int  get_timeout() { return m_timeout;}
	int get_bootload_version(){ return m_bootloader_ver;}
	void issueChanCallback( sct_callback_func * func, void * context, char * buf, int size,sct_channel_info * chan);
	void issueMsgCallback( sct_message_callback_func * func, void * context, char * buf, int msg_class); 
    void check_callbacks(sct_channel_info * chan);
	sct_board_detail_t	* getBoardDetail();
	void    setNumber( int i) { bd_num = i;}
#ifndef WIN32
    void    msgThreadFunc();
#endif    
protected:	
	void releaseResources( );
	void initResources();

	int					m_sig;
    char *				device_path;   // device name string for board.
    os_ufile_t			handle;        // board handle, valid while open.
    bool				open;          // true while open.
	bool				booted;		   // true after boot success.
    bool				ipc_inited;    // true if IPC inited OK.	
	dndev_t				tDnDev;
	sct_channel_map		chan_map;	
	os_umutex_t				hBoardMutex;
	int					m_bootloader_ver;
	sct_msg_info *		msg_info[SCT_MSG_CLASS_COUNT];
	sct_board_detail_t	bd_detail;
	int					bd_num;
#ifndef WIN32
	sct_thread_t	    hMsgThread;
	sct_thread_id_t	    dwMsgThreadId;
    bool                quit;
#endif    
	static int	m_timeout;
	static	CallbackThread	cb_thread;
};

#define BOARD_SIGNATURE         0x44425453  //"STBD"
#define CHANNEL_SIGNATURE       0x4e414843  //"CHAN"

#define VERIFYBOARD(board) (( board != 0) && ( (board)->sig() == BOARD_SIGNATURE ))
#define VERIFYCHANNEL(chan)  (( chan != 0) &&( (chan)->sig() == CHANNEL_SIGNATURE ))

#ifdef WIN32
//callback function
VOID CALLBACK BoardEventCallback( PVOID lpParameter,  BOOLEAN TimerOrWaitFired );
#endif

#endif // _SCT_INTERNAL_H_
