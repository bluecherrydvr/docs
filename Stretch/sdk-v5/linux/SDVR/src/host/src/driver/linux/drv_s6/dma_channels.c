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

#include <linux/interrupt.h>

#include "os_arch.h"
#include "board_arch.h"
#include "ipcqueue.h"
#include "ipcqueue_api.h"
#include "ipcmsg.h"
#include "pci_resources.h"
#include "dma_channels.h"
#include "shared_memory.h"
#include "dispatch.h"
#include "msg_pool.h"
//#include "msg_queues.h"
#include "stretch_main.h"
#include "control.h"



//----------------------------------------
// Initialize the DmaChannels List
//----------------------------------------
void
InitializeDmaChannels(dma_channels_t *dma_channels)
{
    dma_channels->size = 0;
    OS_SPINLOCK_INIT(&dma_channels->spinlock);
}



//----------------------------------------
// This function initalizes the channel
// using port specified.
//----------------------------------------
dma_channel_node_t *
InitializeChannel(dma_channels_t *dma_channels,
                  uint32_t        port)
{
    dma_channel_node_t *this_channel;
    int                 ndx;

    OS_SPINLOCK(&dma_channels->spinlock);

    // Get the size of the dma channel list, which is the next
    // position in the list
    ndx = dma_channels->size;

    // We have now claimed the channel node.
    dma_channels->size = ndx + 1;

    // Release lock before Kmalloc (which may sleep)
    OS_SPINUNLOCK(&dma_channels->spinlock);

    xOS_PRINTK("Using port %d, got channel at index %d\n",
              port,
              ndx);

    //----------------------------------------
    // Allocate and set up the dma_channel buffer
    //----------------------------------------
    dma_channels->pool[ndx] =
        (dma_channel_node_t *)OS_KMALLOC(sizeof(dma_channel_node_t),
                                         "cha");
    this_channel = dma_channels->pool[ndx];

    // Zero out the channel object.
    // Also ensures that channel->currentIrp is zero.
    // Many of the following explicit field inits could be skipped.
    xOS_PRINTK("Assigned channel to the Channel list - \n");
    OS_BZERO(this_channel, sizeof(dma_channel_node_t));
    this_channel->port = port;
    this_channel->receivePending = 0;    

    //----------------------------------------
    // Initialize the work-queues for this channel
    //----------------------------------------
    OS_PENDQUEUE_INIT(&this_channel->CancelSafeQueue);
    xOS_PRINTK("Initialized Pending Work Queue - %p\n",
              &this_channel->CancelSafeQueue);

    OS_PENDQUEUE_INIT(&this_channel->CancelSafeBuffersQueue);
    xOS_PRINTK("Initialized Pending Buffer - %p\n",
              &this_channel->CancelSafeBuffersQueue);

    // and the lock
    OS_SPINLOCK_INIT(&this_channel->spinlock);

    //----------------------------------------
    // Initialize Channel lists (and their locks)
    //----------------------------------------
    // Following initializations don't return status values.
    OS_LIST_INIT(&this_channel->buf_list);
    
    OS_EVENT_INIT(&this_channel->destroyAck);
    this_channel->pending_count=0;
    this_channel->t_count = 0;
    return this_channel;
}



//----------------------------------------
// This function gets the Channel from the
// channel list using port.
// Returns NULL if channel not found.
//----------------------------------------
dma_channel_node_t *
getChannel(dma_channels_t *dma_channels,
           uint32_t        port)
{
    int ndx;
    int size;
    dma_channel_node_t *this_channel = NULL;

    // Get the size
    OS_SPINLOCK(&dma_channels->spinlock);
    size = dma_channels->size;
    xOS_PRINTK("channelsize = %d\n",
              size);

    for (ndx = 0; ndx < size; ndx++) {
        if (dma_channels->pool[ndx]->port == port) {
            this_channel = dma_channels->pool[ndx];

            xOS_PRINTK("Channel Found\n");
            break;
        }
    }
    OS_SPINUNLOCK(&dma_channels->spinlock);
    return this_channel;
}



//----------------------------------------
// This function gets the channel which is
// in pending connetion state
//----------------------------------------
dma_channel_node_t *
getConnectPendingChannel(dma_channels_t *dma_channels,
                         uint32_t        port)
{
    int                 ndx;
    int                 size;
    dma_channel_node_t *this_channel = NULL;

    // Get the size
    OS_SPINLOCK(&dma_channels->spinlock);
    size = dma_channels->size;

    for (ndx = 0; ndx < size; ndx++) {
        if (dma_channels->pool[ndx]->state ==
            CHANNEL_STATE_CONNECT_PENDING) {
            if ((int)port < 0 ) {
                this_channel = dma_channels->pool[ndx];
                xOS_PRINTK("Channel Found for wildcard accept\n");
                break;
            }
            else if (dma_channels->pool[ndx]->port == port) {
                this_channel = dma_channels->pool[ndx];
                xOS_PRINTK("Channel Found\n");
                break;
            }
        }
    }
    OS_SPINUNLOCK(&dma_channels->spinlock);
    return this_channel;
}



//----------------------------------------
// This function checks if any wild card
// accept is waiting.
//----------------------------------------
dma_channel_node_t *
getWildCardChannel(dma_channels_t *dma_channels)
{
    int                 ndx;
    int                 size;
    dma_channel_node_t *this_channel = NULL;

    // Get the size
    OS_SPINLOCK(&dma_channels->spinlock);
    size = dma_channels->size;

    for (ndx = 0; ndx < size; ndx++) {
        if ((int)dma_channels->pool[ndx]->port < 0) {
            this_channel = dma_channels->pool[ndx];

            xOS_PRINTK("Channel Found\n");
            break;
        }
    }
    OS_SPINUNLOCK(&dma_channels->spinlock);
    return this_channel;
}


//----------------------------------------
// This functions gets the index of the
// channel from global channel list.
//----------------------------------------
int
getChannelIndex(dma_channel_node_t *channel,
                dma_channels_t     *dma_channels)
{
    int ndx;
    int retval = -1;

    if (channel != NULL) {
        for (ndx = 0; ndx < dma_channels->size ; ndx++) {
            if (dma_channels->pool[ndx]->port == channel->port) {
                retval = ndx;
                break;
            }
        }
    }
    return retval;
}



//----------------------------------------
// This function returns the channel messages back to S6
//----------------------------------------
void
ReturnChannelMessages(dma_channel_node_t *this_channel)
{
    return; // de nada.
}



//----------------------------------------
// This function removes the channel from
// channel list.
//----------------------------------------
void
RemoveChannel(dma_channel_node_t *this_channel,
              stretch_dev_t      *stretch_dev)
{
    dma_buffers_t      *dma_buffers;
    dma_channels_t     *dma_channels;
    sct_mgmt_msg_t     *message;
    dma_channel_node_t *temp_channel;
    int                 chan_index;
    int                 ndx;
    os_irp_t           *Irp;
    os_status_t         status;

    dma_buffers =  &stretch_dev->dma_buffers;
    dma_channels = &stretch_dev->dma_channels;

    OS_SPINLOCK(&dma_channels->spinlock);
    chan_index = getChannelIndex(this_channel, dma_channels);
    OS_SPINUNLOCK(&dma_channels->spinlock);
    if (chan_index >= 0) {
        // If channel found
        // If channel is in accept pending state, send "reject" response
        // message back to S6DB before removing channel.
        if ((this_channel->state == CHANNEL_STATE_CONNECT_PENDING) &&
            (this_channel->type == CHANNEL_RECV) &&
            (this_channel->mgmt_queue_msg_hdr)) {
            xOS_PRINTK("Reject S6 connect.  Returning channel messages.\n");

            ReturnChannelMessages(this_channel);

            xOS_PRINTK("%d Return connect msg with rsp=-1 to S6.\n",
                      this_channel->port);

            message = (sct_mgmt_msg_t *)this_channel->mgmt_queue_msg_hdr;
            message->rsp = cpu_to_le32(-1);

            sx_queue_mgmt_write(&stretch_dev->queue_mod,
                           stretch_dev->outgoing_mgmt_queue,
                           this_channel->mgmt_queue_msg_hdr,
                           sizeof(sct_mgmt_msg_t));
        }
    
        OS_SPINLOCK(&dma_channels->spinlock);
        cleanPendingChannel(dma_buffers,
                            this_channel);
        OS_SPINUNLOCK(&dma_channels->spinlock);

        // Complete the pending irps from CancelSafeQueue
        Irp = OS_PENDQUEUE_RETREIVE_NEXT
                (&this_channel->CancelSafeQueue,
                 1);
        while (Irp != NULL) {
            xOS_PRINTK("Port %d:  Cancelling CSQ IRP %p\n",
                      this_channel->port,
                      Irp);

            status = OS_STATUS_FAILURE;
            CompleteRequest(Irp, &status);
            Irp = OS_PENDQUEUE_RETREIVE_NEXT
                    (&this_channel->CancelSafeQueue,
                     1);
        }

        // complete the pending irps from CancelSafeBuffersQueue
        Irp = OS_PENDQUEUE_RETREIVE_NEXT
                (&this_channel->CancelSafeBuffersQueue,
                 1);
        while (Irp != NULL) {
            OS_PRINTK("Port %d:  Cancelling CSBQ IRP %p.... \n",
                      this_channel->port,
                      Irp);

            status = OS_STATUS_FAILURE;
            CompleteRequest(Irp, &status);
            Irp = OS_PENDQUEUE_RETREIVE_NEXT
                    (&this_channel->CancelSafeBuffersQueue,
                     1);
        }
        // send free buf notify for all of the buffers in buf_list
        if ( this_channel->type == CHANNEL_RECV )
        {
            while(!OS_LIST_IS_EMPTY(&this_channel->buf_list))
            {
                os_list_node_t      *temp_node;
                dma_buf_node_t      *dma_buf_node;
                temp_node = OS_LIST_RETREIVE_HEAD(&this_channel->buf_list, 1);
                dma_buf_node = OS_CONTAINER_OF(temp_node, dma_buf_node_t, list_node);
                sendFreeBufNotify(stretch_dev, dma_buf_node);
            }
        }
        else
        {
            /*
            if (this_channel->CurrentIrp != 0)
            {
                status = OS_STATUS_FAILURE;
                xOS_PRINTK("complete irp, state=%d\n",this_channel->CurrentIrp->pend_state);
                CompleteRequest(this_channel->CurrentIrp, &status);
            }
            while(!OS_LIST_IS_EMPTY(&this_channel->buf_list))
            {
                os_list_node_t      *temp_node;
                dma_buf_node_t      *dma_buf_node;
                temp_node = OS_LIST_RETREIVE_HEAD(&this_channel->buf_list, 1);
                dma_buf_node = OS_CONTAINER_OF(temp_node, dma_buf_node_t, list_node);
                if (dma_buf_node->irp)
                {
                    status = OS_STATUS_FAILURE;
                    xOS_PRINTK("complete irp, state=%d\n",dma_buf_node->irp->pend_state);
                    CompleteRequest(dma_buf_node->irp, &status);
                }
            }
            */
        }

        // Acquire index again to get proper index in case
        // of another thread updating this.
        OS_SPINLOCK(&dma_channels->spinlock);
        chan_index = getChannelIndex(this_channel, dma_channels);

        // Update channel list
        temp_channel = dma_channels->pool[chan_index];

        for (ndx = chan_index; ndx < (dma_channels->size - 1); ndx++) {
            dma_channels->pool[ndx] = dma_channels->pool[ndx + 1];
        }

        // decrement the size
        dma_channels->size--;
        // free memory allocated for first one
        OS_KFREE(temp_channel);
        dma_channels->pool[dma_channels->size] = NULL;

        xOS_PRINTK("Ending channel list....  \n");
        //for (ndx = 0; ndx < dma_channels->size; ndx++) {
            xOS_PRINTK("dmaChannel[%d] = %p\n",
                      ndx,
                      dma_channels->pool[ndx]);
        //}
        OS_SPINUNLOCK(&dma_channels->spinlock);
    }
}

