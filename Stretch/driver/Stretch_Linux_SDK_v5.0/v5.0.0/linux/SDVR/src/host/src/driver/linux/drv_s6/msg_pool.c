/****************************************************************************** 
Copyright (c) 2006,2008 Stretch, Inc. 
  
Permission is hereby granted, free of charge, to any person obtaining a copy  
of this software and associated documentation files (the "Software"), to deal 
in the Software without restriction, including without limitation the rights  
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell  
copies of the Software, and to permit persons to whom the Software is  
furnished to do so, subject to the following conditions:  
  
The above copyright notice and this permission notice shall be included in  
all copies or substantial portions of the Software.  
  
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,  
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE  
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN  
THE SOFTWARE.
******************************************************************************/

#include "os_arch.h"
#include "ipcqueue.h"
#include "ipcqueue_api.h"
#include "ipcmsg.h"
#include "msg_pool.h"


//========================================
// This function initializes a message pool
// to an empty state
// returns NULL if message is NOT available
//========================================
void
InitMessagePool(message_pool_t *msg_pool)
{
    if (msg_pool != NULL) {
        msg_pool->avail_msgs = 0;
        OS_SPINLOCK_INIT(&msg_pool->spinlock);
    }
}



//========================================
// This function gets the first available message
// from the message pool.
// returns NULL if message is NOT available
//========================================
volatile sx_queue_datahdr_t __iomem *
GetMessageFromPool(message_pool_t *msg_pool)
{
    volatile sx_queue_datahdr_t __iomem *this_message = NULL;

    OS_SPINLOCK(&msg_pool->spinlock);

    do {
        xOS_PRINTK("%d available messages in pool: %p\n",
                   msg_pool->avail_msgs,
                   msg_pool);

//this should be a linked list
        // Check if mgmt messages are available
        if (msg_pool->avail_msgs == 0) {
            break;
        }

        // Take availble message from pool
        --msg_pool->avail_msgs;

        //if (msg_pool->avail_msgs < 3) {
            xOS_PRINTK("Low on Messages ct=%d\n",msg_pool->avail_msgs);
        //}

        this_message = msg_pool->msg[msg_pool->avail_msgs];

        xOS_PRINTK("Assigned available mgmt message %p\n",
                  this_message);
#if 0
//why not at least get it from the end???
        for (ndx = 1;
             ndx <= msg_pool->avail_msgs;
             ndx++) {
            msg_pool->msg[ndx - 1] = msg_pool->msg[ndx];
        }
#endif
    } while(0);

    OS_SPINUNLOCK(&msg_pool->spinlock);
    return this_message;
}



//========================================
// This function releases the message to
// the avaialble message list
//========================================
void
ReleaseMessageToPool(message_pool_t                      *msg_pool,
                     volatile sx_queue_datahdr_t __iomem *this_message)
{
    // Place back in pool
    OS_SPINLOCK(&msg_pool->spinlock);
    msg_pool->msg[msg_pool->avail_msgs] = this_message;
    msg_pool->avail_msgs++;
    OS_SPINUNLOCK(&msg_pool->spinlock);
    xOS_PRINTK("Returning message: avail now %d\n",
               msg_pool->avail_msgs);
}
