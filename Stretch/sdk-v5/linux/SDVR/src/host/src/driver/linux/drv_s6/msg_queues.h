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

#ifndef _MSG_QUEUES_H
#define _MSG_QUEUES_H

//----------------------------------------
// Types of events
//----------------------------------------
#define DESTROY_EVENT       0
#define ERROR_EVENT         1
#define DMA_DONE_EVENT      2
#define REQUEST_BUF_EVENT   3



//----------------------------------------
// Message Queue structure
//----------------------------------------
typedef struct
{
    os_list_node_t      list_node;
    dma_channel_node_t *dma_channel;
    dma_buf_node_t     *dma_buffer;
    int                 event_id;

} msg_queue_node_t;



//----------------------------------------
// Message Handling queue, pools and buffers
//----------------------------------------
typedef struct
{
    // Lists that contain msg_queue_node_t nodes
    os_list_t           active_queue;
    os_list_t           free_queue;

    // Allocated memory buffer pool for messages
    msg_queue_node_t   *pool;
    os_thread_t        *thread;
    int                 total;
    int                 started;
} msg_queues_t;



//----------------------------------------
// Prototypes
//----------------------------------------

// For forward header reference resolution
struct stretch_dev_struct;

// Allocate memory for and initialize the
// message handling queue buffers and lists
void
initMessageHandlingQueue(msg_queues_t *msg_queue);

// Append a message to the message handling queue
int
appendMessage(msg_queues_t             *msg_queues,
              dma_channel_node_t       *dma_channel,
              int                       event,
              dma_buf_node_t           *dma_buf);

// Remove and return the first message in the
// message handling queue
int
getFirstMessage(msg_queues_t     *msg_queues,
                msg_queue_node_t *msg,
                int               remove);

// Stop the current message-handling thread
int
stopMsgQueueThread(msg_queues_t *msg_queues);

// Process Channel Event
void
processChannelEvents(struct stretch_dev_struct *stretch_dev,
                     msg_queue_node_t          *msg);

// Message Handling Queue thread
int
mhqProcessingThread(void *context);

// Start the message-handling thread
int
startMsgQueueThread(msg_queues_t              *msg_queues,
                    struct stretch_dev_struct *stretch_dev);

// De-allocate and de-initialize the message handling queue
void releaseMessageHandlingQueue(struct stretch_dev_struct *stretch_dev);



#endif //_MSG_QUEUES_H

