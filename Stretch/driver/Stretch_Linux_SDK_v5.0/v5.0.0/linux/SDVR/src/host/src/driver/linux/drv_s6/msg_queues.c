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

#include <linux/kthread.h>
#include <linux/interrupt.h>

#include "os_arch.h"
#ifndef KVER_LESS_2620
#include <linux/freezer.h>
#endif
#include "board_arch.h"
#include "ipcqueue.h"
#include "ipcqueue_api.h"
#include "ipcmsg.h"
#include "pci_resources.h"
#include "dma_channels.h"
#include "shared_memory.h"
#include "ipcmsg.h"
#include "msg_pool.h"
#include "dispatch.h"
#include "msg_queues.h"
#include "control.h"
#include "stretch_main.h"



//----------------------------------------
// Allocate memory for and initialize the
// message handling queue buffers and lists
//----------------------------------------
void
initMessageHandlingQueue(msg_queues_t *msg_queues)
{
    int ndx;

    OS_LIST_INIT(&msg_queues->active_queue);
    OS_LIST_INIT(&msg_queues->free_queue);

    // Allocate the pool
    msg_queues->total = SCT_MAX_CHANNEL;
    msg_queues->pool =
      (msg_queue_node_t *)
      OS_KMALLOC(msg_queues->total * sizeof(msg_queue_node_t),
                 "XMSG");

    // Fill the free buffer list
    for (ndx = 0; ndx < msg_queues->total; ndx++) {
        OS_LIST_ADD_TAIL(&(msg_queues->pool[ndx].list_node),
                         &(msg_queues->free_queue));
    }

    msg_queues->started = 0;   
}



//----------------------------------------
// Append a message to the message handling queue
//----------------------------------------
int
appendMessage(msg_queues_t             *msg_queues,
              dma_channel_node_t       *dma_channel,
              int                       event,
              dma_buf_node_t           *dma_buf)
{
    os_list_node_t   *temp_node;
    msg_queue_node_t *msg;
    int               retval = 0;   
    
    temp_node = OS_LIST_RETREIVE_HEAD(&msg_queues->free_queue,
                                      1);
    if (temp_node != NULL) {
        msg = OS_CONTAINER_OF(temp_node,
                              msg_queue_node_t,
                              list_node);
        msg->dma_channel = dma_channel;
        msg->event_id = event;
        msg->dma_buffer = dma_buf;

        OS_LIST_ADD_TAIL(temp_node,
                         &msg_queues->active_queue);

        if (msg_queues->started) {
            OS_THREAD_WAKE(msg_queues->thread);
            retval = 1;
        } else {
            xOS_PRINTK("MsgQ thread not started yet. Tossing buffer!\n");
        }
    }               
    return retval;
}



//----------------------------------------
// Remove and return the first message in the
// message handling queue
//----------------------------------------
int
getFirstMessage(msg_queues_t     *msg_queues,
                msg_queue_node_t *msg,
                int               remove )
{
    os_list_node_t      *temp_node;
    msg_queue_node_t    *this_msg;
    int                  retval = 0;

    temp_node = OS_LIST_RETREIVE_HEAD(&msg_queues->active_queue,
                                      remove);
    if (temp_node != NULL) {             
        this_msg = OS_CONTAINER_OF(temp_node,
                                   msg_queue_node_t,
                                   list_node);
        memcpy(msg, this_msg, sizeof(msg_queue_node_t));
        OS_LIST_ADD_TAIL(temp_node,
                         &msg_queues->free_queue);
        retval = 1;
    }   
    return retval;
}



//----------------------------------------
// Stop the current message-handling thread
//----------------------------------------
int
stopMsgQueueThread(msg_queues_t *msg_queues)
{
    if (msg_queues->started) {
        // Wake up the queue to die.
        OS_THREAD_STOP(msg_queues->thread);
    }
    return 1;
}



//----------------------------------------
// Process Channel events
// Returns 1 if message is to be removed from
// msg queue (and the buffer too).
// 0 if removal will happen during callback
//----------------------------------------
void
processChannelEvents(stretch_dev_t    *stretch_dev,
                     msg_queue_node_t *msg)
{
    os_irp_t                            *Irp;
    os_status_t                          status;
    os_size_t                            size;
    dma_buf_node_t                      *this_dma_buf;
    int                                  ok;
    dma_channel_node_t                  *dma_channel  = msg->dma_channel;
    int                                  event        = msg->event_id;
    dma_buf_node_t                      *dma_buf_node = msg->dma_buffer;
    
    switch (event) {
    case DESTROY_EVENT: // Channel destroy event
        {
            iOS_PRINTK("STATUS_WAIT_0 - Channel destroy event\n");                
            break;
        }
    case ERROR_EVENT: // Channel error event
        xOS_PRINTK("STATUS_WAIT_1 - Channel error event\n");
                
        OS_DELAY_MS(10, KernelMode);

        Irp = dma_channel->CurrentIrp;
        if (Irp != NULL) {
            // Complete current processed IRP
            status = OS_STATUS_SUCCESS;
            CompleteRequest(Irp, &status);
        }

        break;

    case DMA_DONE_EVENT: // DMA DONE
        xOS_PRINTK("STATUS_WAIT_2 - DMA DONE event\n");               
        if (dma_buf_node == 0) {
            //special case, the board has send a destroy channel message
            xOS_PRINTK("dma_buf_node is 0\n");
            if (dma_channel->type == CHANNEL_RECV) {                       
                Irp = OS_PENDQUEUE_RETREIVE_NEXT
                        (&dma_channel->CancelSafeBuffersQueue,
                         1);
                if (Irp != NULL) {
                    OS_PRINTK("complete a failure store irp\n");
                    status = OS_STATUS_FAILURE;
                    CompleteRequest(Irp, &status);
                }
            }

        } else {
            if (dma_channel->state == CHANNEL_STATE_DEAD) {
                xOS_PRINTK("STATUS_WAIT_2 - DMA DONE event"
                          " - Channel port %x is DEAD\n",
                          dma_channel->port);
                status = OS_STATUS_FAILURE;
            } else {
                status = OS_STATUS_SUCCESS;
            }

            if (dma_channel->type != CHANNEL_RECV) {
                os_list_node_t  *temp_node;
                OS_LIST_ITER_INIT(&dma_channel->buf_list);
                {
                    struct timeval tv;
                    do_gettimeofday(&tv);
                    dma_buf_node->irp->dma_time = tv.tv_usec;
                    OS_PRINTK("Dma done for channel %d sec=%ld,usec=%ld\n",dma_channel->port,tv.tv_sec,tv.tv_usec);
                }
                //remove dma_buf from dma_channel->buf_list
                while((temp_node = OS_LIST_ITER_NEXT) != NULL){
                    if ((void*) temp_node == (void*)dma_buf_node )
                    {
                        OS_LIST_REMOVE_ELEMENT(temp_node,&dma_channel->buf_list);
                    }
                }
                // Complete the IRP
                Irp = dma_buf_node->irp;
                if (Irp) {                           
                    status = OS_STATUS_SUCCESS;
                    freeDmaBuffer(stretch_dev,
                                  dma_buf_node,
                                  dma_channel);                         
                    //CompleteRequest(Irp, &status);
                } else {
                    OS_PRINTK("Dma buffer Irp is invalid\n");
                }
            } else {
                // CHANNEL_RECV.  Incoming buffer.
                OS_SPINLOCK(&dma_channel->spinlock);

                // Defer msg_queue and buffer removal until completion
                Irp = OS_PENDQUEUE_RETREIVE_NEXT
                        (&dma_channel->CancelSafeBuffersQueue,
                         1);
                if (Irp == NULL) {
                    xOS_PRINTK("Buffer arrived before store buffer ioctl call\n");

                    OS_LIST_ADD_TAIL(&dma_buf_node->list_node,
                                     &dma_channel->buf_list);
                    OS_SPINUNLOCK(&dma_channel->spinlock);
                } else {
                    dma_channel->lastReceivedSize = dma_buf_node->size;
                    Irp->dma_buf = dma_buf_node;
                    OS_SPINUNLOCK(&dma_channel->spinlock);

                    // Move from PENDING to AWAITING_COMPLETION,
                    // which calls the completion to copyback data.
                    CompleteRequest(Irp, &status);
                }
            }
        }                   
        break;

    // Request buffer event used only for send channel
    case REQUEST_BUF_EVENT:
        ok = 1;
        size = 0;
        OS_PRINTK("STATUS_WAIT_3 - REQ BUF event\n");

        if (dma_channel->type != CHANNEL_RECV) {                       
            /*
            if (dma_channel->CurrentIrp) {
                iOS_PRINTK("channel %d, needs to wait till current Irp equals to 0\n",
                          dma_channel->port);
                appendPendingChannel(dma_channel,
                                     &stretch_dev->dma_buffers);
            } else 
            */
            {
                OS_SPINLOCK(&dma_channel->spinlock);
                if (!OS_LIST_IS_EMPTY( &dma_channel->CancelSafeBuffersQueue ))
                {
                    //let's grab a dma buffer
                    this_dma_buf = getDmaBuffer(&stretch_dev->dma_buffers);
                    if (this_dma_buf == NULL) {
                        xOS_PRINTK("Port:%d: No extra dma buffer avaliable\n",
                                dma_channel->port);
                        OS_SPINUNLOCK(&dma_channel->spinlock);
                        appendPendingChannel(dma_channel,
                                &stretch_dev->dma_buffers);
                    } else {
                        Irp = OS_PENDQUEUE_RETREIVE_NEXT
                            (&dma_channel->CancelSafeBuffersQueue,
                             1);
                        //dma_channel->CurrentIrp = Irp;
                        Irp->dma_buf = this_dma_buf;
                        OS_SPINUNLOCK(&dma_channel->spinlock);
                        /*
                        {
                            volatile sx_queue_datahdr_t __iomem *temp_node;
                            volatile sct_mgmt_msg_t     __iomem *message;
                            temp_node = GetMessageFromPool(&stretch_dev->mgmt_message_pool);
                            message =
                                (volatile sct_mgmt_msg_t __iomem *)
                                OS_CONTAINER_OF(temp_node,
                                        volatile __iomem sct_mgmt_msg_t,
                                        hdr);
                            message->id = SCT_MGMT_MSG_HOST_SENDBUF;
                            message->data.buffer.port = dma_channel->port;
                            this_dma_buf->size = Irp->size;
                            this_dma_buf->irp = Irp;
                            dma_channel->CurrentIrp = NULL;
                            OS_LIST_ADD_TAIL(&this_dma_buf->list_node, &dma_channel->buf_list);
                            message->data.buffer.size = Irp->size;
                            message->data.buffer.pci_addr =
                                OS_LO_DWORD(this_dma_buf->phy_addr);
                            message->data.buffer.pci_addr_hi =
                                OS_HI_DWORD(this_dma_buf->phy_addr);
                            sx_queue_write(&stretch_dev->queue_mod,
                                    stretch_dev->outgoing_mgmt_queue,
                                    &message->hdr,
                                    sizeof(sct_mgmt_msg_t)); 
                            GenerateInterrupt(stretch_dev);
                        }
                        */
                        status = OS_STATUS_SUCCESS;
                        CompleteRequest(Irp, &status);
                    }                   
                }
                else
                {
                    OS_SPINUNLOCK(&dma_channel->spinlock);
                }
            }
        }
        break;

    default:
        xOS_PRINTK("Completed wait with invalid status\n");
        break;
    }       
}



//----------------------------------------
// Message Handling Queue thread
//----------------------------------------
int
mhqProcessingThread(void *context)
{
    msg_queues_t     *msg_queues;
    msg_queue_node_t  msg;
    stretch_dev_t    *stretch_dev;

    OS_PRINTK("++> s6_msgQ:\n");

    stretch_dev = (stretch_dev_t *)context;
    msg_queues = &stretch_dev->msg_queues;
    msg_queues->started = 1;
    while (!OS_THREAD_STOPPED) {
        xOS_PRINTK("MsgQ:waiting for event\n");
        while (getFirstMessage(msg_queues,
               &msg,
               1)) {
            xOS_PRINTK("MsgQ:processing event\n");
            processChannelEvents(stretch_dev,
                                 &msg);
        }
        set_current_state(TASK_INTERRUPTIBLE);
        if (!(OS_THREAD_STOPPED)) {
            schedule();
        }
        set_current_state(TASK_RUNNING);
        try_to_freeze();
    }
    msg_queues->started = 0;
    OS_PRINTK("<++\n");

    return 0;
}



//----------------------------------------
// Start the message-queueing thread
//----------------------------------------
os_status_t
startMsgQueueThread(msg_queues_t *msg_queues,
                    stretch_dev_t *stretch_dev)
{
    os_status_t   retval = OS_STATUS_SUCCESS;

    if (msg_queues->started) {
        return retval;
    }
        // Invoke mhqProcessingThread
        msg_queues->thread = OS_THREAD_RUN(mhqProcessingThread,
                                           stretch_dev,
                                           "s6_MsgQ");
        if (msg_queues->thread == NULL) {
            OS_PRINTK("Attempt to run mhqProcessingThread failed\n");
            retval = OS_STATUS_FAILURE;
        }
    return retval;
}



//----------------------------------------
// De-allocate and de-initialize the message handling queue
//----------------------------------------
void releaseMessageHandlingQueue(stretch_dev_t *stretch_dev)
{
    msg_queues_t *msg_queues = &stretch_dev->msg_queues;

    stopMsgQueueThread(&stretch_dev->msg_queues);
    OS_KFREE(msg_queues->pool);
}
