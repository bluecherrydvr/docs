//
// Simple verification test for S6 Linux driver and linsct lib.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

//for filedump
#include <sys/types.h>
#include <sys/stat.h>
#include <endian.h>
#include <byteswap.h>
#include <fcntl.h>

#include "os_arch.h"
#include "sct.h"

//__asm__(".symver io_getevents, io_getevents@LIBAIO_0.4");
//__asm__(".symver io_cancel, io_cancel@LIBAIO_0.4");
typedef struct _sct_test_msg
{
    unsigned int       buf_size;
    unsigned int       transfer_size;
    unsigned short     port;
    unsigned char      test_type;
    unsigned char      nbuf;
    unsigned char      nchannel;
    unsigned char      cntl;
    unsigned char      repeat;
    unsigned char      num_transfers;
} sct_test_msg;

#define TEST_CNTL_CLOSE_CHNL        (0x01)
#define TEST_CNTL_CHECK_DATA        (0x02)

bool            cancel=false;

void ctrl_c_handler( int dummy )
{
    cancel = true;
}


int openBoard( sct_board_t * bds)
{
    sct_board_t      board = NULL;
    int ret=0;
    unsigned int     res;
    int              num_boards = sct_init();

    if( num_boards == 0 )
    {
        printf( "No Stretch boards were found.\n" );
    }
    else
    {  
        for ( int i=0; i< num_boards;i++)
        {
            res = sct_open_board( i, &board );

            if( res )
            {
                printf("Can not open board %d.\n",i);
            }
            else
            {
                bds[ret]=board;
                ret++;
            }
        }
    }
    return ret;
}
#define CHAN_COUNT      16
#define PORT_START      10
#define TRANSFER_COUNT  (CHAN_COUNT * 3) //2)
#define LOOP_COUNT      2
#define TRANSFER_SIZE   24*1024

void fillMessage( sct_test_msg* tmsg)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN 
    tmsg->buf_size      = TRANSFER_SIZE;
    tmsg->transfer_size = TRANSFER_SIZE;
    tmsg->port          = PORT_START;
#else    
    tmsg->buf_size      = bswap_32(TRANSFER_SIZE);
    tmsg->transfer_size = bswap_32(TRANSFER_SIZE);
    tmsg->port          = bswap_16(PORT_START);
#endif    
    tmsg->test_type     =  1;                         
    tmsg->nbuf          = (unsigned char) 2;    
    tmsg->nchannel      = (unsigned char) CHAN_COUNT;
    
    tmsg->cntl             = (TEST_CNTL_CLOSE_CHNL | TEST_CNTL_CHECK_DATA);                         
    tmsg->repeat        = (unsigned char) LOOP_COUNT;
    tmsg->num_transfers = (unsigned char) TRANSFER_COUNT;   
}

typedef struct chan_info_t
{
    sct_channel_t   handle;
    int             port;
    int             count;
    int             id;
    int             board_id;
    volatile int *             pCount;
} chan_info;

void chanCallback( void * context, void * buffer, int size)
{
    chan_info * pInfo = (chan_info *) context;
    char *buf = (char*)buffer;

    printf("board %d:Received %d bytes from port %d,total count=%d\n",pInfo->board_id,size,pInfo->port,pInfo->pCount[0]);
    // Check data
    int badcount= 0;                        

#if 1
    for( int x = 0; x < size; ++x )
    {
        if ( buf[x] != (char)(x+pInfo->count*CHAN_COUNT+pInfo->id) )
        {
            badcount++;
        }
    }
#endif
    if ( badcount )
    {
        printf( "board %d: Received %x bad bytes at channel %d\n", pInfo->board_id, badcount,pInfo->port );
    }   
    pInfo->count++;
    // Put mutex around this because multiple threads are calling this,
    // and may cause this read/modify/write to get corrupted.
    pInfo->pCount[0] ++;
    sct_buffer_free( pInfo->handle, buffer);
}



void testReceive( sct_board_t board, int id)
{
    int i,j;
    chan_info chan[CHAN_COUNT];
    int chan_opened;
    int count;
    for (j=0;j<LOOP_COUNT;j++)
    {
        count = 0;
        chan_opened = 0;
        for (i=0;i<CHAN_COUNT;i++)
        {
            chan[i].handle = sct_channel_accept( board, PORT_START+i, 3);
            if (chan[i].handle != NULL)
            {
                chan[i].port = PORT_START + i;
                chan[i].count = 0;
                chan[i].id = i;
                chan[i].board_id = id;
                chan[i].pCount = &count;
                sct_register_callback(chan[i].handle,&chanCallback, &chan[i] );
                chan_opened++;
            }
            else
            {
                printf( "accept channel port %d failed\n",PORT_START+i);
                cancel = 1;
                break;
            }
        }
        while( (count < TRANSFER_COUNT) && !cancel )
        {
#if defined (OS_IS_LINUX)
            usleep(1000);
#elif defined (OS_IS_WINDOWS)
            Sleep(100);
#endif
        }
        for (i=0;i<chan_opened;i++)
        {
            sct_channel_close(chan[i].handle);
        }
        if (cancel)
        {
            break;
        }
    }
}

void sendCallback( void * context, void * buffer, int size)
{
    chan_info * pInfo = (chan_info *) context;
    pInfo->pCount[0]++;
    printf("board %d, channel port %d send done,total %d\n",pInfo->board_id,pInfo->port,pInfo->pCount[0]);
}

void testSend( sct_board_t board, int id)
{
    int i,j;
    chan_info chan[CHAN_COUNT];
    int chan_opened;
    int count;
    for (j=0;j<LOOP_COUNT;j++)
    {
        count = 0;
        chan_opened = 0;
        for (i=0;i<CHAN_COUNT;i++)
        {
             int ret = sct_channel_connect( board, PORT_START+i, 4, TRANSFER_SIZE, &chan[i].handle);
             chan_opened++;
            if (ret == SCT_NO_ERROR)
            {
                chan[i].port = PORT_START + i;
                chan[i].count = 0;
                chan[i].id = i;
                chan[i].board_id = id;
                chan[i].pCount = &count;
                sct_register_callback(chan[i].handle,&sendCallback, &chan[i] );
            }
            else
            {
                printf( "connect channel port %d failed\n",PORT_START+i);
                cancel = 1;
                break;
            }
        }
        for( i = 0; (i < TRANSFER_COUNT) && !cancel; i++ )
        {           
            char * buf = (char *) sct_buffer_alloc_wait( chan[i%CHAN_COUNT].handle,1000 );
            if (buf)
            {
                // Fill in data
                for( int x = 0; x < TRANSFER_SIZE; ++x )
                {
                    buf[x] = (char)(x + i);
                }

                sct_buffer_send( chan[i%CHAN_COUNT].handle,
                    buf,
                    TRANSFER_SIZE );
            }
            else
            {
                printf("sct_buffer_alloc failed for channel %d\n",i%CHAN_COUNT);
                cancel = 1;
            }
        }
        for (i=0;i<chan_opened;i++)
        {
            sct_channel_close(chan[i].handle);
        }
        if (cancel)
        {
            i=0;
            break;
        }
    }
}

void msgCallBack( void * context, sx_int32  msg_class, void * buffer )
{
    chan_info * pInfo = (chan_info *) context;
    uint8_t n = ((uint8_t *)buffer)[0];
    /*
    printf("board %d, get message class=%d, total=%d (%p)\n",pInfo->board_id,
                                            msg_class,
                                            pInfo->pCount[0],
                                            pInfo->pCount);
    */                                            
    if( (n % CHAN_COUNT) != (uint8_t) msg_class )
    {
        printf( "board %d: LINTEST: invalid message context %d, expect %d, total %d\n",
                pInfo->board_id,
                ((uint8_t *)buffer)[0],
                msg_class,
                pInfo->pCount[0]);
    }
    pInfo->pCount[0]++;
}

chan_info * prepareReceiveMsg( sct_board_t board , int id)
{
    int i;
    chan_info* chan= (chan_info *)malloc(sizeof(chan_info));
    chan->board_id = id;
    chan->pCount = (int *) malloc (sizeof(int));
    chan->pCount[0] = 0;

        for (i = 0; i < CHAN_COUNT; i++) //CHAN_COUNT = 32
        {   
            sct_register_message_callback( board,
                    i, 
                    msgCallBack,
                    (void *)chan );
        }
     return chan;
}
void testReceiveMsg( sct_board_t board , chan_info * chan)
{   
    int i;

        while ((chan->pCount[0] < TRANSFER_COUNT *LOOP_COUNT) && !cancel)
        {
            usleep(1000);
        }
        // Wait for the callbacks
        printf("board %d: received %d messages\n",chan->board_id, chan->pCount[0]);
  
    for (i = 0; i < CHAN_COUNT; i++)
    {   
        sct_register_message_callback( board,
                i, // PORT_START=0
                0,
                0 );
    }
    free((void *)chan->pCount);
    free(chan);
}

void testSendMsg( sct_board_t board , int id )
{   
    int j;
    int count;
    for (j = 0; j < LOOP_COUNT; j++)
    {
        count = 0; 
        while( (count < TRANSFER_COUNT) && !cancel )
        {
            char buf[16];
            int i;
            int32_t mclass = count%CHAN_COUNT;
            for( i = 0;i<16;i++)
            {
                buf[i] = mclass;
            }
            sct_message_send(board,mclass,(void *)buf,16);
            count ++;
        }
    }
    printf("board %d: finish send %d messages\n",id,count);
}

void runtest( sct_board_t bd, int type, int board_id)
{
    int ret;
    sct_test_msg msg;
    fillMessage(&msg);
    switch (type )
    {
        case 1:
        msg.test_type = 1;
        ret = sct_message_send( bd,8,&msg,SCT_MAX_MESSAGE_LEN );
        if (!ret)
        {
            testReceive( bd , board_id);
        }
        else
        {
            printf("board %d,send message failed\n",board_id);
            cancel = 1;
        }
        break;
        case 2:
        msg.test_type = 2;
        ret = sct_message_send( bd,8,&msg,SCT_MAX_MESSAGE_LEN );
        if (!ret)
        {
            testSend( bd , board_id);
        }
        else
        {
            printf("board %d,send message failed\n",board_id);
            cancel = 1;
        }
        break;
        case 4:
        {
            chan_info * pchan;
            msg.test_type = 4;
            pchan = prepareReceiveMsg( bd, board_id);
            ret = sct_message_send( bd,8,&msg,SCT_MAX_MESSAGE_LEN );
            if ( !ret )
            {
                testReceiveMsg( bd, pchan );
            }
            else
            {
                printf("board %d,send message failed\n",board_id);
                cancel = 1;
            }
        }
        break;
        case 5:
        msg.test_type = 5;
        ret = sct_message_send( bd,8,&msg,SCT_MAX_MESSAGE_LEN );
        if ( !ret )
        {
            testSendMsg( bd , board_id);
        }
        else
        {
            printf("board %d,send message failed\n",board_id);
            cancel = 1;
        }
        break;
    }
}

void run_all_test( sct_board_t bd , int board_id)
{
    int i;
    for ( i=0; (i <= 5 && !cancel ); i++)
    {
        runtest(bd,i,board_id);
    }
}

int
main(int argc, char* argv[])
{
typedef void (*sighandler_t)(int);

    sct_board_t bds[8];
    int bd_count;
    int i;
    signal(SIGHUP, ctrl_c_handler);
    signal(SIGINT, ctrl_c_handler);
    signal(SIGTERM, ctrl_c_handler);
    signal(SIGABRT, ctrl_c_handler);

    bd_count = openBoard(bds);
    if ( bd_count == 0 )
        return 0;
    for (i=0;i<bd_count;i++)
    {
        sct_reset_board(bds[i]);
        if (!sct_load_firmware( bds[i], "sctdemo_s6.rom", 0))
        {
            printf("Error: board %d load firmware failed\n",i);
            return 1;
        }
    }
    for (int ndx = 0; (ndx < 1) && !cancel;ndx++) {
        printf("iter %d\n",ndx);
        for ( i = 0;i<bd_count;i++)
        {
            //runtest(bds[i],5,i);
            run_all_test(bds[i], i);
        }
    }
    for (i=0;i<bd_count;i++)
        sct_close_board(bds[i]);
    printf("\n");
    return 0;
}

