/**********************************************************************\
*                                                                      *
* Copyright (c) 2005 Stretch, Inc. All rights reserved.                *
*                                                                      *
* THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF *
* STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT *
* THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.                *
*                                                                      *
\**********************************************************************/

// Main file, defines DLL entry point and public API functions.

//---------------------------------------------------------------------
// Author:Xun Tan
// Date Created: 06 10 2007
//---------------------------------------------------------------------

#ifdef WIN32
#define _WIN32_WINNT 0x500
#include <windows.h>
#include <crtdbg.h>
#include <setupapi.h>
#include <initguid.h>

#include "..\windows\Inc\guids.h"
#include "..\windows\Inc\ioctls.h"
#include "log.h"
#else
#include <sys/ioctl.h>
#include "../linux/include/stretch_if.h"
#endif
#include "sct.h"
#include "sct_internal.h"
#include "sct_version.h"

#ifdef WIN32
//#pragma data_seg(".SHARED")

static sct_board_info board_info[ SCT_MAX_BOARDS ]={};

//#pragma data_seg()
//#pragma comment(linker, "/section:.SHARED,RWS")


//static BOOL reloadDriver(sct_board_info * bd);

/**********************************************************************
    DLL entry point.
**********************************************************************/
BOOL APIENTRY
DllMain( HANDLE, 
         DWORD  ul_reason_for_call, 
         LPVOID )
{
    switch( ul_reason_for_call )
    {
    case DLL_PROCESS_ATTACH:

#ifdef _DEBUG

        _CrtSetDbgFlag( _CrtSetDbgFlag(0) | _CRTDBG_LEAK_CHECK_DF );


#endif

        //Trace( "DllMain() : DLL_PROCESS_ATTACH\n" );       
        break;

    case DLL_PROCESS_DETACH:

        //Trace( "DllMain() : DLL_PROCESS_DETACH\n" );      

        break;

    case DLL_THREAD_ATTACH:

        //Trace( "DllMain() : DLL_THREAD_ATTACH\n" );
        break;

    case DLL_THREAD_DETACH:

        //Trace( "DllMain() : DLL_THREAD_DETACH\n" );
        break;
    }

    return TRUE;
}
#else
static sct_board_info board_info[ SCT_MAX_BOARDS ]={};
#endif

/*******************************************************************************
    Returns the name of the specified board. The name is a text string supplied
    by the system, and should not be interpreted to have any specific meaning.

    Parameters:

        "board_index" - Board number, from 0 to N-1, where N is the number
        returned by sct_init().

    Returns:

        A pointer to the board's name string. The pointer will be NULL if
        no board was found at that index, or if the index was invalid.
*******************************************************************************/
const char *
sct_get_board_name( sx_uint32 board_index )
{
    if( board_index >= SCT_MAX_BOARDS )
    {
        return NULL;
    }
    
    return board_info[board_index].devicePath();
}

/*******************************************************************************
    Initializes the SCT library, and scans the system to discover all Stretch
    boards. This function must be called before any other API function.

    Parameters:

        None.

    Returns:

        The number of boards found in the system.
*******************************************************************************/
#ifdef WIN32
sx_uint32
sct_init()
{
    HDEVINFO                            hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA            deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData = NULL;
    ULONG                               predictedLength           = 0;
    ULONG                               requiredLength            = 0;

	sx_uint32 num_boards = 0;
	int   index      = 0;

    // Open a handle to the device interface information set of all present
    // Stretch class interfaces. It uses the GUID with which we have 
    // registered the driver with OS (from ADDDevice routine of driver code).

    hardwareDeviceInfo = SetupDiGetClassDevs( (LPGUID)&GUID_INTERFACE_WINOFFLOAD,
                                               NULL, // Define no enumerator (global)
                                               NULL, // Define no
                                               (DIGCF_PRESENT|
                                               DIGCF_DEVICEINTERFACE) );

    if( INVALID_HANDLE_VALUE == hardwareDeviceInfo )
    {
        Trace( __FUNCTION__ ": No boards found.\n" );
        
        return num_boards;
    }
    
    deviceInterfaceData.cbSize = sizeof( SP_DEVICE_INTERFACE_DATA );

    // Enumerate all instances with this GUID.
    
    for( index = 0; index < SCT_MAX_BOARDS; ++index )
    {
        if( SetupDiEnumDeviceInterfaces(    hardwareDeviceInfo,
                                            0, // No care about specific PDOs
                                            (LPGUID)&GUID_INTERFACE_WINOFFLOAD,
                                            index, //
                                            &deviceInterfaceData) ) 
        {
            // Allocate a function class device data structure to 
            // receive the information about this particular device.
            // First find out required length of the buffer
           
            SetupDiGetDeviceInterfaceDetail( hardwareDeviceInfo,
                                             &deviceInterfaceData,
                                             NULL,  // probing so no output buffer yet
                                             0,     // probing so output buffer length of zero
                                             &requiredLength,
                                             NULL ); // not interested in the specific dev-node

            predictedLength = requiredLength;
            
            if( predictedLength == 0 )
            {
            }

            deviceInterfaceDetailData = 
                (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc( predictedLength );

            deviceInterfaceDetailData->cbSize = 
                            sizeof( SP_DEVICE_INTERFACE_DETAIL_DATA );

            if( SetupDiGetDeviceInterfaceDetail( hardwareDeviceInfo,
                                                 &deviceInterfaceData,
                                                 deviceInterfaceDetailData,
                                                 predictedLength,
                                                 &requiredLength,
                                                 NULL) ) 
            {
                
                board_info[index].setDevicePath( deviceInterfaceDetailData->DevicePath );				
                //Trace( __FUNCTION__ ": board index %d, name [%s]\n",
                //     index,
                //     deviceInterfaceDetailData->DevicePath );				
            }
            else
            {
                SetupDiDestroyDeviceInfoList( hardwareDeviceInfo );                
            }

            free( deviceInterfaceDetailData );
        }
        else 
        {
            // check if ALL items are over else continue with next
            if( GetLastError()== ERROR_NO_MORE_ITEMS )
            {                
                break;
            }
        }
    }

    num_boards = index;
    SetupDiDestroyDeviceInfoList( hardwareDeviceInfo );
    return num_boards;
}
#else
sx_uint32
sct_init()
{
    unsigned long num_boards = 0;
    int i;
    for (i = 0;i<SCT_MAX_BOARDS;i++)
    {
        FILE * fp;
        char fname[4096];
        sprintf(fname,"/dev/s6stretch%d",i);
        fp = fopen(fname,"rb");
        if (!fp)
        {
            break;
        }
        else
        {
            board_info[i].setDevicePath(fname);
            num_boards++;
            fclose(fp);
        }
    }
    return num_boards;
}
#endif
/*******************************************************************************
    Opens the board for access and returns a handle to it if successful. This
    function must be called to obtain a board handle before any operations
    can be performed on the board.

    Parameters:

        "board_index" - Board number, from 0 to N-1, where N is the number
        returned by sct_init()..

        "phandle" - Pointer to caller-assigned storage for the board handle.
        If the function is successful, the board handle is returned in this
        location.

    Returns:

        SCT_NO_ERROR - Success.

        SCT_ERR_INVALID_PARAMETER - Board name is NULL or invalid.

        SCT_ERR_DEVICE_IN_USE - Board is already in use.

        SCT_ERR_DEVICE_OPEN - Error occurred while opening board.
*******************************************************************************/
sx_uint32
sct_open_board( int	board_index,
                sct_board_t *   phandle )
{   
	if( board_index >= SCT_MAX_BOARDS )
    {
        return SCT_ERR_INVALID_PARAMETER;
    }
    
    sct_board_info * bd = &(board_info[board_index]);

    if( bd == 0 )
    {
        Trace( __FUNCTION__ ": Error, invalid board name.\n" );
        return SCT_ERR_INVALID_PARAMETER;
    }

    if( bd->isOpened() )
    {
        Trace( __FUNCTION__ ": Board already open.\n" );
		*phandle = bd;
        return SCT_ERR_DEVICE_IN_USE;
    }
    
    // Return the board handle.
	if ( bd->openBoard() )
	{
		*phandle = bd;
		bd->setNumber(board_index);
		return SCT_NO_ERROR;
	}
	else
	{
		*phandle = 0;
		return SCT_ERR_DEVICE_OPEN;
	}
}


/*******************************************************************************
    Closes the board handle and frees the board, releasing associated system
    resources.

    Parameters:

        "board" - Handle to the board.

    Returns:

        SCT_NO_ERROR - Success.

        SCT_ERR_INVALID_PARAMETER - Board handle is NULL or invalid.

        SCT_ERR_DEVICE_CLOSE - Error occurred while closing board.
*******************************************************************************/
sx_uint32
sct_close_board( sct_board_t board )
{    
	sct_board_info * bd = (sct_board_info *)board;

    if( !VERIFYBOARD(bd) )
    {
        Trace( __FUNCTION__ ": Error, invalid board handle 0x%08x.\n", board );
        
        return SCT_ERR_INVALID_PARAMETER;
    }    

	bool result = bd->closeBoard( );   

    return result ? SCT_NO_ERROR : SCT_ERR_DEVICE_CLOSE;
}


/*******************************************************************************
    Resets the specified board. The board will reboot. After a successful
    reset, the board handle must be closed and reopened before trying to use
    the board.

    Parameters:

        "board" - Handle to the board.

    Returns:

        SCT_NO_ERROR - Success.

        SCT_ERR_INVALID_PARAMETER - Board handle is NULL or invalid.

        SCT_ERR_DEVICE_RESET - Error occurred while resetting board.
*******************************************************************************/
sx_uint32
sct_reset_board( sct_board_t board )
{
#ifdef PPC
	return SCT_NO_ERROR;
#else	
	sct_board_info * bd = (sct_board_info *)board;
    sx_uint32 err;
    int retry = 10;

    if( !VERIFYBOARD(bd) )
    {    
        Trace( __FUNCTION__ ": Error, invalid board handle 0x%08x.\n", board );
        
        return SCT_ERR_INVALID_PARAMETER;
    }

    sx_int32  status = 0;

    //Trace( __FUNCTION__ ": Trying to reset board, handle 0x%08x.\n", board );

    if( !bd->ioControl( WINOFFLD_IOCTL_RESET_S5530,                      
                            sizeof(sx_int32),
                            (unsigned char*)&status,
                            NULL) ) 
    {
        Trace( __FUNCTION__ ": IOCTL returned error, code %d.\n",
             GetLastError() );
        
        return SCT_ERR_DEVICE_RESET;
    }   
    else	
    {	
		bd->closeBoard();
        err = SCT_ERR_DEVICE_RESET;
        // In some PCs or Laptops, it may take upto 2 seconds before
        // we can re-open the board but in most cases 1/2 second
        // wait is enough.
        while (retry)
		{				
    		OS_USLEEP(500);
			if (bd->openBoard())
            {
                err = SCT_NO_ERROR;
                break;
            }
            retry--;
		}
        if (err != SCT_NO_ERROR)
        {
            Trace( __FUNCTION__ ": Failed to re-open the board after reset.\n");           
        }   

		return err;

    }
#endif	
}


/*******************************************************************************
    Open a send connection from the application to the specified board, and
    allocate resources for it. The "port" parameter is a unique identifier for
    the channel, similar to a TCP/IP port number.

    This function blocks until a response is received from the board.

    Parameters:

        "board" - Handle to the board.

        "port" - Port number to connect to.

        "nbuf" - Number of transfer buffers to allocate. The number
        of buffers per channel must be at most SCT_MAX_BUF_PER_CHANNEL.

        "size" - The size in bytes of each buffer. The buffer size
        must be at most SCT_MAX_BUFFER_SIZE.

        "chan_return" - Pointer to storage for the channel handle. This is
        filled in if the function succeeds.

    Returns:

        SCT_NO_ERROR - Success.

        SCT_ERR_INVALID_PARAMETER - One of the paremeters is invalid.

        SCT_ERR_IPC_INIT - Error occurred setting up IPC queues
        to the board software.

        SCT_ERR_NO_CHANNELS - No free channels.

        SCT_ERR_CHANNEL_IN_USE - Port number already in use.

        SCT_ERR_CHANNEL_CREATE - Error occurred creating the channel.

        SCT_ERR_CHANNEL_CONNECT - Error occurred connecting the channel,
        or the board rejected the connection.
*******************************************************************************/
sx_uint32
sct_channel_connect( sct_board_t    board,
                     sx_int32       port,
                     sx_uint32      nbuf,
                     sx_uint32      size,
                     sct_channel_t *chan_return )
{
	sct_channel_info * pchan;

	sct_board_info * bd = (sct_board_info *)board;

    if( !VERIFYBOARD(bd) )
    {    
        Trace( __FUNCTION__ ": Error, invalid board handle 0x%08x.\n", board );
        
        return SCT_ERR_INVALID_PARAMETER;
    }

    if( chan_return == 0 )
    {
        Trace( __FUNCTION__ ": Error, invalid channel return pointer\n");
        return SCT_ERR_INVALID_PARAMETER;
    }
    *chan_return = NULL;   

    if( (port < 0) || (port > SCT_MAX_PORT_NUMBER) )
    {
        return SCT_ERR_INVALID_PARAMETER;
    }

    if( (nbuf <= 0) || (nbuf > SCT_MAX_BUF_PER_CHANNEL) )
    {
        return SCT_ERR_INVALID_PARAMETER;
    }
    
    if( (size <= 0) || (size > bd->getBoardDetail()->buffer_size) )
    {
        return SCT_ERR_INVALID_PARAMETER;
    }
	    
    // Init IPC if required.

    if( !bd->initIPCQueue() )
    {  
        return SCT_ERR_IPC_INIT;
    }            
    
	pchan = bd->newChannel(port,nbuf,size);
	if (pchan == 0)
	{
		return SCT_ERR_CHANNEL_IN_USE;
	}
	sx_uint32 ret = pchan->connect();
	if ( ret != SCT_NO_ERROR)
	{
		bd->removeChannel(pchan);		
	}
	else
	{
		*chan_return = (sct_channel_t)pchan;
	}
    return ret;
}	

/*******************************************************************************
    Accept a receive connection from the specified board, and allocate resources
    for it. The "port" parameter is a unique identifier for the channel, similar
    to a TCP/IP port number.

    If "port" is < 0, then it is treated as a wildcard, i.e. any connection
    from the specified board is accepted.  For convenience, SCT_PORTNUM_ANY is
    defined for use as a wildcard port.

    Blocks until the connection is established, and then returns the channel
    handle.

    Parameters:

        "board" - Handle to the board.

        "port" - Port number to accept on.

        "nbuf" - Number of receive buffers to allocate. The number
        of buffers per channel must be at most SCT_MAX_BUF_PER_CHANNEL.

    Returns:

        Handle to the opened channel. There is no error return.
*******************************************************************************/
sct_channel_t
sct_channel_accept( sct_board_t board,
                    sx_int32    port,
					sx_int32    nbuf)
{
    sct_channel_info * recv_channel;
	sct_board_info * bd = (sct_board_info *)board;

	if (! VERIFYBOARD(bd) )
	{
		Trace( __FUNCTION__ ": Error, Invalid board handle\n");             
        return NULL;
	}

	if ( nbuf > SCT_MAX_BUF_PER_CHANNEL )
	{
		nbuf = SCT_MAX_BUF_PER_CHANNEL;
	}

    if( !bd->initIPCQueue() )
    {  
        return NULL;
    }            
	recv_channel = bd->newChannel(port,nbuf,0);
            
	if ( recv_channel )
	{
		if (recv_channel->accept() != SCT_NO_ERROR)
		{		
			bd->removeChannel(recv_channel);
			recv_channel = NULL;
		}
	}
	return recv_channel;
}


/*******************************************************************************
    Close a channel. The sender may close a channel at any time, but the
    receiver can close a channel only after the sender has closed it.
    The receiver will know that a receive channel has been closed when
    sct_buffer_recv() returns NULL.

    For a send channel, sct_buffer_alloc() and sct_buffer_send() may not be
    called after sct_channel_close() has been called. For a receive channel,
    sct_buffer_recv() will always return NULL after the channel has been
    closed by the sender, and sct_buffer_free() may be called for any buffers
    that have already been received. After a channel has been closed, all
    buffer pointers associated with it become invalid, and should never be
    referenced.

    Parameters:

        "channel" - Handle to the channel to be closed.

    Returns:

        SCT_NO_ERROR - Success.

        SCT_ERR_INVALID_PARAMETER - One of the paremeters is invalid.

        SCT_ERR_CHANNEL_IN_USE - A receive channel has not been closed
        yet by the sender.

        SCT_ERR_CHANNEL_CLOSE - Error occurred closing the channel, or
        the board did not respond correctly to the close request.
*******************************************************************************/
sx_uint32
sct_channel_close( sct_channel_t channel )
{
	sx_uint32 ret = SCT_ERR_CHANNEL_CLOSE;
    sct_channel_info * pChannel = (sct_channel_info *) channel;
	if (VERIFYCHANNEL( pChannel ) )
	{
		int retry = 0;
		while (retry < 5)
		{
			//Trace("Retry %d\n",retry);
			ret = pChannel->close();			
			if ( ret == SCT_NO_ERROR)
			{				
				pChannel->getBoard()->removeChannel(pChannel);
				break;
			}
			else
			{	
				OS_USLEEP(100);
			}
			retry ++;
		}
	}
	return ret;
}


/*******************************************************************************
    Allocate a buffer for specified channel. The buffer size is that specified
    at channel creation time. This function will not block.
    Parameters:

        "channel" - Channel for which buffer is to be allocated.

    Returns:

        Pointer to the allocated buffer. There is no error return.

        Returns NULL, if there is no buffer avaliable.

*******************************************************************************/
void *
sct_buffer_alloc( sct_channel_t channel )
{
    sct_channel_info * pChannel = (sct_channel_info *) channel;
	return pChannel->getFreeBuffer(0);    
}

/*******************************************************************************
    Allocate a buffer for specified channel. The buffer size is that specified
    at channel creation time. This function is a blocking call, it will return when
    a buffer is avaliable or timeout.

    Parameters:

        "channel" - Channel for which buffer is to be allocated.

        "timeout" - timeout value in milli seconds.

    Returns:

        Pointer to the allocated buffer. There is no error return.

        Returns NULL, if there is no buffer avaliable.

*******************************************************************************/
void *
sct_buffer_alloc_wait( sct_channel_t channel, int timeout )
{
    sct_channel_info * pChannel = (sct_channel_info *) channel;
	return pChannel->getFreeBuffer( timeout );    
}
/*******************************************************************************
    Send the buffer and return it to the free list when done.  

    This function may block if the receiver has no free buffers available to
    receive the data.

    Parameters:

        "channel" - Channel on which buffer is to be sent.

        "buffer" - Pointer to data buffer. The buffer must belong to
        the channel, i.e. it must have been returned by a previous call to
        sct_buffer_alloc() for this channel.

        "size" - Number of bytes to send. Must be less than or equal to the
        buffer size specified at channel creation time.

    Returns:

        SCT_NO_ERROR - Success.

        SCT_ERR_INVALID_PARAMETER - The buffer does not belong to the channel,
        the send size is too large or the channel is not a send channel.

        SCT_ERR_CHANNEL_DEAD - Channel is dead, i.e. an error occurred
        transferring data.

        SCT_ERR_CHANNEL_NOT_ACTIVE - Channel is closing or has been closed.
*******************************************************************************/
sx_uint32
sct_buffer_send( sct_channel_t    channel,
                 void *            buffer,
                 sx_int32          size )
{
    sct_channel_info * pChannel = (sct_channel_info *) channel;

    if( !VERIFYCHANNEL(pChannel) )
    {
        Trace( __FUNCTION__ ": Error, channel is NULL.\n" );
        return SCT_ERR_INVALID_PARAMETER;
    }
	return pChannel->sendBuffer( buffer, size);  
}

/*******************************************************************************
    Polls all the specified channels to see if any have data available. The
    first channel that has data available is returned. This function does not
    block.

    Parameters:

        "channels" - Pointer to array of channel handles.

        "n" - Number of valid handles in "channels".

    Returns:

        Handle to the first channel found that has data available, or NULL
        if no channel has data available.
*******************************************************************************/
sct_channel_t
sct_buffer_poll( sct_channel_t *    channels,
                 sx_int32           n )
{
    if( channels == NULL )
    {
        Trace( __FUNCTION__ ": Error, input channel array is NULL\n" );
        return NULL;
    }
    
    sct_channel_t ret_channel = NULL;

    // Find the first channel that has a recv buffer pending.
    
    for( int i = 0; i < n; i++ )
    {       
        sct_channel_info * pChan = (sct_channel_info *) channels[i];

        if( !VERIFYCHANNEL( pChan) )
        {
            Trace( __FUNCTION__ ": Warning, channel at index %d is NULL, skipping\n",
                 i );
        }
        else
        {                          
            if( pChan->isRecvBufferAvailable() )
                {                    
                    ret_channel = (sct_channel_t)pChan;
					break;
                }            
        }
    }
    
    return ret_channel;
}


/*******************************************************************************
    Returns the next available buffer on the specified channel. Blocks until
    a buffer is available. Buffers are returned in the order that they are
    received.

    Parameters:

        "channel" - Channel for which buffer is to be returned.

        "psize" - Pointer to storage for the buffer size, i.e.
        the number of bytes received in this buffer.
        This may be NULL if the size is not required.

    Returns:

        A pointer to the received buffer, or NULL if the channel is closed
        and there are no pending received buffers.
*******************************************************************************/
void *
sct_buffer_recv( sct_channel_t    channel,
                 sx_int32 *       psize )
{
    // Check for valid channel and type.

    void *     pVoid    = NULL;
	int		   size;
    sct_channel_info * pChannel = (sct_channel_info *) channel;

    if( !VERIFYCHANNEL(pChannel) )
    {
        Trace( __FUNCTION__ ": Error, invalid channel handle.\n" );
        return NULL;
    }

	pVoid = pChannel->getRecvBuffer( size );  
	if ( pVoid )
	{
		*psize = size;
	}
    return pVoid;
}

/*******************************************************************************
    Frees one buffer returned by sct_buffer_recv() for the specified channel.
    The buffers are freed in the same order that they are returned by
    sct_buffer_recv(). If there are no buffers to be freed, does nothing.

    Parameters:

        "channel" - Handle to the channel to free from.

        "p" - Pointer to the memory buffer that needs to be freed

    Returns:

        Nothing.
*******************************************************************************/
void
sct_buffer_free( sct_channel_t    channel, void * p )
{
    // Check for valid channel and type.
	sct_channel_info * pChannel = (sct_channel_info *) channel;

    if( !VERIFYCHANNEL(pChannel) )
    {
        Trace( __FUNCTION__ ": Error, invalid channel handle.\n" );
        return ;
    }

	pChannel->returnRecvBuffer( p );   
}

/*******************************************************************************
    Sends the specified message data to the specified board. The function will
    always send SCT_MAX_MESSAGE_LEN bytes, padding the message with zeros if
    required. The receiver must know the number of valid bytes in the message.
    The message will be of type "msg_class". The use of the message class is
    completely up to the applications. The API will check for valid message
    classes but does not interpret the class in any way.

    Parameters:

        "board" - Handle to the board to send to.

        "msg_class" - The class of the message. The class can be used by
        the receiver to filter messages. Must be between
        SCT_MSG_CLASS_MIN and SCT_MSG_CLASS_MAX, inclusive.

        "buffer" - Pointer to message data.

        "size" - Size of message data in bytes. Must not be more than
        SCT_MAX_MESSAGE_LEN bytes.

    Returns:

        0 if Message is sent.  Non-zero if message send failed.
*******************************************************************************/
sx_uint32
sct_message_send( sct_board_t     board,
                  sx_int32        msg_class,
                  void *          buffer,
                  sx_int32        size )
{
	sx_uint32 ret = SCT_ERR_MSG_SEND;
    sct_board_info * bd = (sct_board_info *)board;

    if( !VERIFYBOARD(bd) )
    {    
        Trace( __FUNCTION__ ": Error, invalid board handle 0x%08x.\n", board );
        
        return ret;
    }
	if( !bd->initIPCQueue() )
    {  
        return ret;
    }            
	return bd->sendMessage( msg_class,buffer,size );
	
}

/*******************************************************************************
    Receives the next message from the specified board, of the specified class.
    Blocks until a message is available. The received message will always be
    SCT_MAX_MESSAGE_LEN bytes long, possibly padded with zeros. The receiver
    must know the number of valid bytes in the message.

    Parameters:

        "board" - Handle to the board to receive from.

        "msg_class" - The class of message to receive. If this is < 0,
        then it is treated as a wildcard, i.e. the next message of any class
        will be returned. Else, it must be between SCT_MSG_CLASS_MIN and
        SCT_MSG_CLASS_MAX, inclusive.  For convenience, SCT_MSG_CLASS_ANY
        is defined for use as a wildcard message class.

        "buffer" - Pointer to receive buffer where message contents will be
        placed. Must be at least SCT_MAX_MESSAGE_LEN bytes long.

        "psrc_board" - Pointer to storage for board handle.  This is not
        currently used, but will be when wildcard for "board" is supported.
        The calling function can pass in NULL if it does not need a handle to
        the board.

        "pmsg_class" - Pointer to storage for message class. The function
        fills this in with the class of the received message when "msg_class"
        is a wildcard.  The calling function can pass in NULL if it does not
        need to know the message class.

    Returns:
    
        0 if Message is received.  Non-zero if message receive failed.
*******************************************************************************/
sx_uint32
sct_message_recv( sct_board_t     board,
                  sx_int32        msg_class,
                  void *          buffer,
                  sct_board_t *   psrc_board,
                  sx_int32 *      pmsg_class )
{
	sx_uint32 ret = SCT_ERR_MSG_RECV;
    sct_board_info * bd = (sct_board_info *)board;

    if( !VERIFYBOARD(bd) )
    {    
        Trace( __FUNCTION__ ": Error, invalid board handle 0x%08x.\n", board );        
        return ret;
    }
	if (!buffer)
	{		
		return ret;
	}
	if( !bd->initIPCQueue() )
    {  		
        return ret;
    }

	sx_int32 c = msg_class;
	ret = bd->recvMessage(c,buffer);
	if ( ret == SCT_NO_ERROR )
	{
		if (pmsg_class)
			*pmsg_class = c;
		if (psrc_board)
			*psrc_board = board;		
	}
	return ret;
}

/*******************************************************************************
    Receives the next message from the specified board, of the specified class.
    Unlike sct_message_recv(), this function never blocks.

    Parameters:

        "board" - Handle to the board to receive from.

        "msg_class" - The class of message to receive. If this is < 0,
        then it is treated as a wildcard, i.e. the next message of any class
        will be returned. Else, it must be between SCT_MSG_CLASS_MIN and
        SCT_MSG_CLASS_MAX, inclusive.  For convenience, SCT_MSG_CLASS_ANY is
        defined for use as a wildcard message class.

        "buffer" - Pointer to receive buffer where message contents
        will be placed. Must be at least SCT_MAX_MESSAGE_LEN bytes long.

        "psrc_board" - Pointer to storage for board handle.  This is not
        currently used, but will be when wildcard for "board" is supported.
        The calling function can pass in NULL if it does not need a handle to
        the board.

        "pmsg_class" - Pointer to storage for message class. The function
        fills this in with the class of the received message when "msg_class"
        is a wildcard.  The calling function can pass in NULL if it does not
        need to know the message class.

    Returns:
    
       True if message is received.  False if no message received.
*******************************************************************************/
sx_bool
sct_message_recv_poll( sct_board_t    board,
                       sx_int32       msg_class,
                       void *         buffer,
                       sct_board_t *  psrc_board,
                       sx_int32 *     pmsg_class )
{
    
	sct_board_info * bd = (sct_board_info *)board;

    if( !VERIFYBOARD(bd) )
    {    
        Trace( __FUNCTION__ ": Error, invalid board handle 0x%08x.\n", board );
        
        return false;
    }
	sx_int32 c = msg_class;
	if ( bd->pollMessage(c,buffer) == SCT_NO_ERROR )
	{
		*pmsg_class = c;
		*psrc_board = board;
		return true;
	}
	else
	{
		*pmsg_class = -1;
		return false;
	}	

}



void
sct_test( sct_board_t )
{
	//Dummy function that needs to be removed
}

#ifdef WIN32
VOID CALLBACK BoardEventCallback( PVOID lpParameter,  BOOLEAN bTimeOut)
{
	sct_msg_info * mi = (sct_msg_info *)lpParameter;	
	if (!bTimeOut)
		mi->wakeup();
}
#endif
/*******************************************************************************
    Register a message callback routine for one board.  The callback routine
    will be called when the board receives a message of that class.

    sct_message_recv_poll() always returns NULL when a board has a callback
    function registered.  Because the received message will be returned in the
    callback function, the poll function will never see any messages of that
    class.


    Callback function parameters:

        "context" - User defined context when the callback was registered.

        "msg_class" - The class of the message.

        "buffer" - The message buffer.

    Function's own parameters:

        "board" - Handle to the board.

        "func" - Address of the user callback function.

        "msg_class" - The class of the message.  The valid range for msg_class
        is 0 to SCT_MSG_CLASS_COUNT.

        "context" - User defined context.

    Returns:

        True if the callback was successfully registered.
        False if registration failed.
*******************************************************************************/
sx_bool sct_register_message_callback( sct_board_t    board, sx_int32 msgclass, sct_message_callback_func* func, void * context)
{
	sct_board_info * bd = (sct_board_info *)board;

    if( !VERIFYBOARD(bd) )
    {    
        Trace( __FUNCTION__ ": Error, invalid board handle 0x%08x.\n", board );
        
        return false;
    }
	if ( msgclass < 0 || msgclass>= SCT_MSG_CLASS_COUNT )
	{
		return false;
	}
	if( !bd->initIPCQueue() )
    {  
        return SCT_ERR_IPC_INIT;
    }
	return bd->regCallback(msgclass,func,context);
}


/*******************************************************************************
    Register a callback routine for a particular channel.  The callback routine
    will be called when the channel has received data or when send is complete.

    sct_buffer_poll() always returns NULL when a channel has a callback function
    registered.  Because the received buffer will be returned in the callback
    function, the poll function will never see any buffers for that channel.

    Callback function parameters:

        "context" - User defined context when the callback was registered.

        "buffer" - For receive channel, the received data.  For send channel,
        the buffer just sent.

        "size" - For receive channel, the received data size.  For send channel,
        size is always 0.

    Function's own parameters:

        "channel" - Channel handle.

        "func" - Address of user callback function.

        "context" - User defined context.

    Returns:

        True if the callback was successfully registered.
        False if registration failed.
*******************************************************************************/
sx_bool sct_register_callback( sct_channel_t channel,sct_callback_func* func, void * context)
{
	sct_channel_info * pChannel = (sct_channel_info *) channel;
	if (!VERIFYCHANNEL( pChannel ) )
	{
		return false;
	}	
	return pChannel->regCallback( func,context);
}

/*******************************************************************************
    Sets the number of seconds that the driver waits before it timeout, while
    waiting for a response from the firmware.

    Parameters:

        "board" - Handle to the board.

        "timeout" - Number of seconds to wait before timeout.

    Returns:

        There is no error return.
*******************************************************************************/
void sct_set_timeout(sct_board_t    board, sx_int32  timeout )
{
	sct_board_info * bd = (sct_board_info *)board;
    bd->set_timeout(timeout * 1000);
}


int getBoardIndex( sct_board_info * bd)
{
	int i = (int)((char *)bd - (char *)board_info)/ sizeof(sct_board_info);
	return i;
}

#ifdef WIN32
BOOL reloadDriver(sct_board_info * bd)
{	
	BOOL ret = 0;
	DWORD index = getBoardIndex( bd );		
	
	HDEVINFO	hardwareDeviceInfo;
	SP_DEVINFO_DATA  DeviceInfoData;
	hardwareDeviceInfo = SetupDiGetClassDevs( (LPGUID)&GUID_INTERFACE_WINOFFLOAD,
                                               NULL, // Define no enumerator (global)
                                               NULL, // Define no
                                               (DIGCF_PRESENT|
                                               DIGCF_DEVICEINTERFACE) );
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	if (SetupDiEnumDeviceInfo(  hardwareDeviceInfo,  index,  &DeviceInfoData ))
	{
		SP_PROPCHANGE_PARAMS pcp;
		pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
        pcp.StateChange = DICS_DISABLE ;
        pcp.Scope = DICS_FLAG_GLOBAL;
        pcp.HwProfile = 0;

		if(SetupDiSetClassInstallParams(hardwareDeviceInfo,&DeviceInfoData,&pcp.ClassInstallHeader,sizeof(pcp))) {
			ret = SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hardwareDeviceInfo,&DeviceInfoData);
        }
		if (ret )
		{
			pcp.StateChange = DICS_ENABLE ;
			if(SetupDiSetClassInstallParams(hardwareDeviceInfo,&DeviceInfoData,&pcp.ClassInstallHeader,sizeof(pcp))) {
				ret = SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hardwareDeviceInfo,&DeviceInfoData);
			}
		}
	}
	return ret;
}
#endif

int WriteMem(sct_board_t board, int bar, int addr, void * buf, int size)
{
   sct_board_info * bd = (sct_board_info *)board;
   return(bd->WriteMem(bar, addr, buf, size));
}

int ReadMem(sct_board_t board, int bar, int addr, void * buf, int size)
{
   sct_board_info * bd = (sct_board_info *)board;
   return(bd->ReadMem(bar, addr, buf, size));
}

/*******************************************************************************
    Loads the specified firmware application image into the board and starts it
    running. The file must be a ROMable image file, identical to what is burned
    into the boot flash by st-flash. This function cannot load ELF files.

    Parameters:

        "board" - Handle to the board.

        "path" - Path to the executable image file.

    Returns:

        TRUE if the load succeeds, FALSE if it fails.
*******************************************************************************/
sx_bool sct_load_firmware( sct_board_t board, char * path, sct_diag_info_t* pdiag_info)
{
	sct_board_info * bd = (sct_board_info *)board;
	return bd->loadFirmware(path, pdiag_info);
}

/*******************************************************************************
    This function returns the SCT/Driver version.

    Parameters:

        None.

    Returns:

        The SCT/Driver version information. The 32 bit return value contains
        the 4 part version number defined least to high bit:

        Major Version: bit 0 - 7

        Minor Version: bits 8 - 15

        Revision Version: bits 16 - 23

        Build Version: bits 24 - 32
*******************************************************************************/

sx_uint32
sct_get_version()
{
	return SCT_VERSION;
}

/*******************************************************************************
    This function returns the the boot-loader and BSP version for the given
    board. The board must be opened and the firmware must be loaded via PCI
    in order to get these information.

    In case the firmware is flashed on to the board this function returns
    all zero values.

    Parameters:

        "board" - Handle to the board.

    Returns:

        The boot-loader and BSP version information
*******************************************************************************/

ml_versions sct_get_bootloader_version( sct_board_t board )
{
	_ml_versions v;
	sct_board_info * bd = (sct_board_info *)board;
	v.u.version_data = bd->get_bootload_version();
	return v;
}

/*******************************************************************************
    This function returns detailed board information for the given
    board. The board must be opened and the firmware must be loaded via PCI
    in order to get these information.
    
    Parameters:

        "board" - Handle to the board.
        "pbd"   - Pointer to a stretch_board_detail structure, this structure will be
        filled with returned data if the function call is success.

    Returns:

        Returns SCT_NO_ERROR when success. otherwise returns a error code.
*******************************************************************************/

sx_uint32 sct_get_board_detail(sct_board_t board, sct_board_detail_t * pbd)
{
	sct_board_info * bd = (sct_board_info *)board;
	sct_board_detail_t *	p;
	if( !VERIFYBOARD(bd) )
    {    
        Trace( __FUNCTION__ ": Error, invalid board handle 0x%08x.\n", board );
        
        return SCT_ERR_INVALID_PARAMETER;
    }
	p = bd->getBoardDetail();
	if (p)
	{
		memcpy(pbd,p,sizeof(sct_board_detail_t));
		return SCT_NO_ERROR;
	}
	else
	{
		return SCT_ERR_BOARD_BOOT_FAIL;
	}
}

