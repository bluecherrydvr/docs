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

#ifndef _MSG_POOL_H
#define _MSG_POOL_H



//----------------------------------------
// Message Pool structure
//----------------------------------------
typedef struct
{
    // to hold messages list handle
    volatile sx_queue_datahdr_t __iomem *msg[SCT_MSG_COUNT];

    uint32_t            avail_msgs;

    // spinLock to protect access to the queue
    os_spinlock_t       spinlock;

} message_pool_t;



//----------------------------------------
// Prototypes
//----------------------------------------

// This function initializes a message pool
// to an empty state
// returns NULL if message is NOT available
void
InitMessagePool(message_pool_t *msg_pool);

// This function gets the first available message
// from the message pool.
// returns NULL if message is NOT available
volatile sx_queue_datahdr_t __iomem *
GetMessageFromPool(message_pool_t *msg_pool);

// This function releases the message to
// the avaialble message list
void
ReleaseMessageToPool(message_pool_t                      *msg_pool,
                     volatile sx_queue_datahdr_t __iomem *this_message);


#endif // _MSG_POOL_H


