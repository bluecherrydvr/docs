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
#include <linux/kthread.h>

#include "os_arch.h"
#ifndef KVER_LESS_2620
#include <linux/freezer.h>
#endif
#include "board_arch.h"
#include "ipcqueue.h"
#include "ipcqueue_api.h"
#include "ipcmsg.h"
#include "stretch_if.h"
#include "pci_resources.h"
#include "dma_channels.h"
#include "shared_memory.h"
#include "msg_pool.h"
//#include "msg_queues.h"
#include "dispatch.h"
#include "control.h"
#include "stretch_main.h"

void s_dpc(stretch_dev_t * stretch_dev);
long    my_time;
//----------------------------------------
// Initialize the dispatch routine
//----------------------------------------
void
InitializeDispatch(stretch_dev_t *stretch_dev)
{
    os_workpool_t *dpc_workpool = &stretch_dev->dpc_workpool;
    int            this_pos;
    
    OS_WORKQUEUE_INIT(dpc_workpool->queue, "s6_DpcQ");
    for (this_pos = 0; this_pos < OS_WORKQUEUE_POOL_SIZE; this_pos++) {
        dpc_workpool->pool[this_pos].context = (void *)stretch_dev;
        OS_WORKQUEUE_INIT_ITEM(&dpc_workpool->pool[this_pos].item,
                  stretch_dpc);
    }
    dpc_workpool->pos = 0;
}



//----------------------------------------
// De-initialize the dispatch routine
//----------------------------------------
void
DeInitializeDispatch(stretch_dev_t *stretch_dev)
{
    os_workpool_t *dpc_workpool = &stretch_dev->dpc_workpool;
    
    OS_WORKQUEUE_FLUSH_QUEUE(dpc_workpool->queue);
    OS_WORKQUEUE_DESTROY(dpc_workpool->queue);
}

int checkIrq( stretch_dev_t * stretch_dev)
{
    uint32_t                             statusRegister;
    int i;
    statusRegister = sx_queue_status_read(&stretch_dev->queue_mod,
                           stretch_dev->queue_status_base);
    if ((statusRegister & 3) !=0)
    {
        return 1;
    }
    for(i=0;i<SCT_MSG_CLASS_COUNT/32;i++)
    {
        statusRegister = sx_queue_status_read(&stretch_dev->queue_mod,
                           stretch_dev->queue_status_base+3+i);
        if( statusRegister != 0)
        {
            return 1;
        }
    }
    return 0;
}

//----------------------------------------
// Main low-level interrupt routine
// Called upon PCI interrupt
//----------------------------------------
#ifdef KVER_LESS_2620
irqreturn_t
stretch_isr(int     irq,
            void    *dev_id,
            struct  pt_regs * regs)
#else
irqreturn_t
stretch_isr(int   irq,
            void *dev_id)
#endif
{
    os_workpool_t       *dpc_workpool;
    struct work_struct  *this_workitem;
    int                  this_pos;
    stretch_dev_t *stretch_dev = (stretch_dev_t *)dev_id;
    volatile uint32_t __iomem    * pInt = PCI_REG_PTR32(stretch_dev->registers,
                            HOST_INTERRUPT_OFFSET);
    uint32_t val=le32_to_cpu(*pInt);

    OS_PRINTK("GOT an interrupt from board stretch_dev=%p\n",stretch_dev);

    //if (checkIrq(stretch_dev))
    if ( (val & 1) !=0)
    {
        // Valid interrupt.
        // Turn the interrupt off at board
        *PCI_REG_PTR32(stretch_dev->registers,
                HOST_INTERRUPT_OFFSET) = 0;
        // Invalid interrupt.  Throw away.  Bye bye.
        if (!stretch_dev->initialized) {
            return IRQ_HANDLED;
        }
        // OS_LO_DWORD(stretch_dev->queues.phy_addr);
        /*
        {
            struct timeval tv;
            do_gettimeofday(&tv);
            my_time = tv.tv_usec;
        }
        */

        dpc_workpool = &stretch_dev->dpc_workpool;
        this_pos = dpc_workpool->pos;
        this_workitem = &dpc_workpool->pool[this_pos].item;
        this_pos = (this_pos + 1) % OS_WORKQUEUE_POOL_SIZE;
        dpc_workpool->pos = this_pos;
        OS_WORKQUEUE_QUEUE_ITEM(dpc_workpool->queue,
                this_workitem);
        return IRQ_HANDLED;
    }
    else
    {
        OS_PRINTK("not our irq\n");
        return IRQ_NONE;
    }
}



//----------------------------------------
// Process Incoming Management messages
// coming from the board's queue.
//----------------------------------------
static void
ProcessMgmtMsg(stretch_dev_t                       *stretch_dev,
               volatile sx_queue_datahdr_t __iomem *rcvd_msg_hdr)
{
    dma_channel_node_t              *dma_channel;
    dma_buf_node_t                  *dma_buffer;
    volatile sct_mgmt_msg_t __iomem *rcvd_message;
    uint32_t                         id;
    uint32_t                         response;
    os_status_t                      status;
    sct_mgmt_msg_t                  *irp_message;
    os_irp_t                        *Irp;
    dma_channels_t                  *dma_channels = &stretch_dev->dma_channels;
    //msg_queues_t                    *msg_queues = &stretch_dev->msg_queues;
    int port;


    OS_PRINTK("process mgmt msg, id=%d\n",le32_to_cpu(rcvd_message->id));
    // read allocated messages
    rcvd_message =
      (volatile sct_mgmt_msg_t __iomem *)
        OS_CONTAINER_OF(rcvd_msg_hdr, volatile __iomem sct_mgmt_msg_t, hdr);
    id = le32_to_cpu(rcvd_message->id);

    switch (id) {
    case SCT_MGMT_MSG_CONNECT:
        OS_SPINLOCK(&stretch_dev->accept_spinlock);

        port = le32_to_cpu(rcvd_message->data.connect.port);
        //----------------------------------------
        // check whether it's a connect response message or new
        // connect request
        //----------------------------------------
        dma_channel = getChannel(dma_channels,port);

        if (dma_channel == NULL) {
            xOS_PRINTK("No channels found for port. "
                      "Check for wildcard ACCEPT from application.\n");

            // Check for wildcard connect request from board
            dma_channel = getWildCardChannel(dma_channels);
        }

        //----------------------------------------
        // No dma channel has been initialized yet.
        // Must be a new connect message from board.
        // Set channel state to CONNECT_PENDING
        //----------------------------------------
        if (dma_channel == NULL) {
           xOS_PRINTK("++> MsgH: Mgmt new CONNECT Msg: id=%d, port=%d\n",
                     id,port);

            // Initialize the channel structure.
            dma_channel = InitializeChannel(dma_channels,port);

            xOS_PRINTK("Connecting channel %p\n",
                      dma_channel);

            dma_channel->max_size = le32_to_cpu(rcvd_message->data.connect.max_size);
            dma_channel->mgmt_queue_msg_hdr = &rcvd_message->hdr;
            dma_channel->type = CHANNEL_RECV;
            dma_channel->state = CHANNEL_STATE_CONNECT_PENDING;
            xOS_PRINTK("Wait for application to call ACCEPT\n");

            // Release the lock now
            OS_SPINUNLOCK(&stretch_dev->accept_spinlock);
            
        //----------------------------------------
        // A Dma channel has been initialized.  If
        // type CHANNEL_RECV, it is a response from the board
        // to our pending ACCEPT. This Completes the
        // receive "connect" request.
        // (This performs COPYBACK.  Can't do it in dispatch.)
        //----------------------------------------
        } else if (dma_channel->type == CHANNEL_RECV) {
            xOS_PRINTK("++> MsgH: Mgmt CONNECT(pended accept) Msg: id=%d, port=%d\n",
                      id,port);

            OS_SPINUNLOCK(&stretch_dev->accept_spinlock);

            // Look at an item from the work queue
            Irp = OS_PENDQUEUE_RETREIVE_CONTEXT
                    (&dma_channel->CancelSafeQueue,
                     CONNECT_PENDING_CONTEXT,
                     1);

            xOS_PRINTK("IRP %p from CSQ with context %d\n",
                      Irp,
                      Irp->context);

            // Update channel structure
            dma_channel->port = port;
            dma_channel->max_size = le32_to_cpu(rcvd_message->data.connect.max_size);
            dma_channel->mgmt_queue_msg_hdr = &rcvd_message->hdr;
            dma_channel->state = CHANNEL_STATE_ACTIVE;

            irp_message = (sct_mgmt_msg_t *)Irp->system_buffer;
            irp_message->data.connect.max_size = dma_channel->max_size;
            irp_message->data.connect.port = dma_channel->port;

            status = OS_STATUS_SUCCESS;
            CompleteRequest(Irp, &status);

        //----------------------------------------
        // CONNECT response message from board
        // (This performs COPYBACK.  Can't do it in dispatch.)
        //----------------------------------------
        } else {
            xOS_PRINTK("++> MsgH: Mgmt CONNECT response Msg: id=%d, port=%d\n",
                      id,port);
            OS_SPINUNLOCK(&stretch_dev->accept_spinlock);

            response = rcvd_message->rsp;

            Irp = OS_PENDQUEUE_RETREIVE_CONTEXT
                    (&dma_channel->CancelSafeQueue,
                     CONNECT_PENDING_CONTEXT,
                     1);
            xOS_PRINTK("IRP %p from CSQ with context %d\n",
                      Irp,
                      Irp->context);


            if (response == 0) {
                status = OS_STATUS_SUCCESS;
            } else {
                OS_PRINTK("S6 rejected CONNECT for channel %p\n",
                          dma_channel);

                // connection was rejected
                RemoveChannel(dma_channel, stretch_dev);
                status = OS_STATUS_FAILURE;
            }

            // release message to free list of message
            xOS_PRINTK("Release CONNECT msg to free list\n");
            ReleaseMessageToPool(&stretch_dev->mgmt_message_pool,
                                 &rcvd_message->hdr);

            // complete the IRP.  Response gets left in system buffer.
            irp_message = (sct_mgmt_msg_t *)Irp->system_buffer;
            irp_message->rsp = response;
            irp_message->data.connect.port = dma_channel->port;

            // Move from PENDING to AWAITING_COMPLETION
            CompleteRequest(Irp, &status);
        }
        break;

    case SCT_MGMT_MSG_DESTROY:
        port = le32_to_cpu(rcvd_message->data.destroy.port);
        OS_PRINTK("++> MsgH: Mgmt DESTROY Msg: id=%d, port=%d\n",
                  id,port);

        dma_channel = getChannel(&stretch_dev->dma_channels,port);
        if (dma_channel == NULL) {
            //the channel might be force closed, just return success to firmware
            OS_PRINTK("channel %p not found\n",
                    dma_channel);
            if ( rcvd_message->data.connect.max_size == 0xa5a5a5a5 )
            {
                OS_PRINTK("Destroy Ack, no channel found\n");
                ReleaseMessageToPool(&stretch_dev->mgmt_message_pool,
                                     &rcvd_message->hdr);
            }
            else
            {
                rcvd_message->rsp = 0;
                sx_queue_mgmt_write(&stretch_dev->queue_mod,
                        stretch_dev->outgoing_mgmt_queue,
                        &rcvd_message->hdr,
                        sizeof(sct_mgmt_msg_t));
                GenerateInterrupt(stretch_dev);
            }
            break;
        }

        if (dma_channel->type == CHANNEL_SEND) {
            OS_PRINTK("chan %p DESTROY response msg from S6\n",
                      dma_channel);
            if (dma_channel->state == CHANNEL_STATE_CLOSE_PENDING)
            {
                xOS_PRINTK("wake Destroy Ack\n");
                dma_channel->mgmt_queue_msg_hdr = &rcvd_message->hdr;
                OS_EVENT_WAKE(&dma_channel->destroyAck);
            }
            else
            {
                xOS_PRINTK("Desktroy Ack, just release message\n");
                ReleaseMessageToPool(&stretch_dev->mgmt_message_pool,
                                                     &rcvd_message->hdr);
            }
        } else {
            xOS_PRINTK("chan %p new DESTROY msg from S6\n",
                      dma_channel);
            if (dma_channel->state == CHANNEL_STATE_ACTIVE) {
                dma_channel->state = CHANNEL_STATE_CLOSE_PENDING;
                dma_channel->mgmt_queue_msg_hdr = &rcvd_message->hdr;
                Irp = OS_PENDQUEUE_RETREIVE_CONTEXT
                        (&dma_channel->CancelSafeQueue,
                         DESTROY_CONTEXT,
                         1);
                if ( Irp )
                {
                    OS_PRINTK("IRP %p from CSQ with context %d\n",
                          Irp,
                          Irp->context);

                    // The will complete the async transaction, but
                    // no further action gets performed (copy_to_user, etc)
                    status = OS_STATUS_SUCCESS;
                    CompleteRequest(Irp, &status);
                }
            } else {
                xOS_PRINTK("Chan %d not active\n",
                          dma_channel->port);
                //we just return the destory message
                rcvd_message->rsp = 0;
                sx_queue_mgmt_write(&stretch_dev->queue_mod,
                    stretch_dev->outgoing_mgmt_queue,
                    &rcvd_message->hdr,
                    sizeof(sct_mgmt_msg_t));
                GenerateInterrupt(stretch_dev);
            }               
        }           
        break;

    case SCT_MGMT_MSG_DMA_DONE:
        port = le32_to_cpu(rcvd_message->data.buffer.port);
        dma_channel = getChannel(&stretch_dev->dma_channels,port);
        dma_buffer = findDmaBufferByPhy(&stretch_dev->dma_buffers,
                                        le32_to_cpu(rcvd_message->data.buffer.pci_addr),
                                        le32_to_cpu(rcvd_message->data.buffer.pci_addr_hi),
                                        (uint32_t)stretch_dev->pci_resources.max_dma_buff);
        if ( dma_buffer )
        {
            OS_PRINTK("MsgH: Mgmt DMA DONE Msg: id=%d, port=%d\n",
                    id,
                    port);
            dma_buffer->size = le32_to_cpu(rcvd_message->data.buffer.size);
            ReleaseMessageToPool(&stretch_dev->mgmt_message_pool,&rcvd_message->hdr);
            if (dma_channel) {
                if (dma_channel->type != CHANNEL_RECV)
                {
                    os_irp_t   *Irp=NULL;
                    os_list_node_t  *temp_node;
                    OS_LIST_ITER_INIT(&dma_channel->buf_list);
                    while((temp_node = OS_LIST_ITER_NEXT) != NULL){
                        if ((void*) temp_node == (void*)dma_buffer )
                        {
                            OS_LIST_REMOVE_ELEMENT(temp_node,&dma_channel->buf_list);
                        }
                    }
                    if ( dma_channel->pending_count == 0)
                    {
                        Irp = OS_PENDQUEUE_RETREIVE_CONTEXT
                            (&dma_channel->CancelSafeQueue,
                             DESTROY_PENDING_CONTEXT,1);
                    }
                    freeDmaBuffer(stretch_dev,
                            dma_buffer,
                            dma_channel);                         
                    if (Irp)
                    {
                        OS_PRINTK("invoke destroy compelete, Irp=%p, Irp->pend_state=%d\n",Irp,Irp->pend_state);
                        status = OS_STATUS_SUCCESS;
                        CompleteRequest(Irp, &status);
                    }
                }
                else
                {
                    /*
                       appendMessage(msg_queues,
                       dma_channel,
                       DMA_DONE_EVENT,
                       dma_buffer);
                       */
                    // CHANNEL_RECV.  Incoming buffer.
                    os_irp_t                            *Irp;
                    OS_SPINLOCK(&dma_channel->spinlock);

                    // Defer msg_queue and buffer removal until completion
                    Irp = OS_PENDQUEUE_RETREIVE_NEXT
                        (&dma_channel->CancelSafeBuffersQueue,
                         1);
                    if (Irp == NULL) {
                        OS_LIST_ADD_TAIL(&dma_buffer->list_node,
                                &dma_channel->buf_list);
                        OS_SPINUNLOCK(&dma_channel->spinlock);
                    } else {
                        dma_channel->lastReceivedSize = dma_buffer->size;
                        Irp->dma_buf = dma_buffer;
                        OS_SPINUNLOCK(&dma_channel->spinlock);

                        // Move from PENDING to AWAITING_COMPLETION,
                        // which calls the completion to copyback data.
                        status = OS_STATUS_SUCCESS;
                        CompleteRequest(Irp, &status);
                    }
                }
            } 
            else 
            {
                //channel might be force closed on pc side
                OS_PRINTK("no channel is found %d\n",port);
                if (dma_buffer >= 
                        &(stretch_dev->dma_buffers.pool[stretch_dev->pci_resources.max_dma_recv_buff]))
                {
                    freeDmaBuffer( stretch_dev, dma_buffer,0);
                }
                else
                {
                    sendFreeBufNotify( stretch_dev, dma_buffer );
                }
            }
        }
        else{
            //return error
            iOS_PRINTK("MsgH: Mgmt Msg is INVALID: id=%d, port=%d\n",
                      id,
                      port);
            ReleaseMessageToPool(&stretch_dev->mgmt_message_pool,&rcvd_message->hdr);
        }
        break;

    case SCT_MGMT_MSG_HOST_RECVBUF:
    case SCT_MGMT_MSG_HOST_SENDBUF:
        xOS_PRINTK("++> MsgH: Mgmt SEND/RECV buf returned  Msg: id=%d\n",
                  id);
        //should be returned message
        //just free it.
        ReleaseMessageToPool(&stretch_dev->mgmt_message_pool,
                             &rcvd_message->hdr);
        break;

    default:
        break;
    }
}



//----------------------------------------
// Process regular messsages from the
// board's message return queue
// (This performs COPYBACK.  Can't do it in dispatch.)
//----------------------------------------
static void
ProcessRegularMsg(stretch_dev_t *stretch_dev,
                  uint32_t       class_id)
{
    volatile sx_queue_datahdr_t __iomem *rcvd_msg_hdr;
    volatile sct_msg_t          __iomem *rcvd_message;
    os_irp_t                            *Irp;
    sct_dll_msg_t                       *return_message;
    uint32_t                             msize;
    uint32_t                             ndx;
    uint32_t                             writeStatus;
    os_status_t                          status = OS_STATUS_SUCCESS;

    OS_PRINTK("REGULAR message. Class = %d\n",
              class_id);

    if (class_id < SCT_MSG_CLASS_COUNT) {   
        OS_SPINLOCK(&stretch_dev->MsgSpinLock);

        // check if application is waiting to read this message
        // (but don't remove from the queue.  Read method
        // from the user-space callback will do that)
        Irp = OS_PENDQUEUE_RETREIVE_CONTEXT
                (&stretch_dev->CancelSafeMsgQueue,
                 class_id,
                 1);

        if (Irp == NULL) {
            // check if wildcard recv is pending (but don't remove yet)
            Irp = OS_PENDQUEUE_RETREIVE_CONTEXT
                    (&stretch_dev->CancelSafeMsgQueue,
                     SCT_MSG_CLASS_COUNT,
                     1);
        }

        // if message received, give message to the application and
        // return the message to S6
        if (Irp == NULL) {
            // else, it will be read when recv called by application
            OS_SPINUNLOCK(&stretch_dev->MsgSpinLock);
            OS_PRINTK("Application had not yet called "
                      "recv for msg class %d\n",
                      class_id);
        } else {

            Irp->class_id = class_id;
            OS_PRINTK("Application expecting message %d. IRP %p\n",
                      Irp->class_id,
                      Irp);

            sx_queue_read(&stretch_dev->queue_mod,
                          stretch_dev->incoming_message_queue[Irp->class_id],
                          &rcvd_msg_hdr);

            rcvd_message =
              (volatile sct_msg_t __iomem *)
                OS_CONTAINER_OF(rcvd_msg_hdr,
                                volatile __iomem sct_msg_t,
                                hdr);

            return_message = (sct_dll_msg_t *)Irp->system_buffer;
            return_message->msg.size = le32_to_cpu(rcvd_message->size);
            return_message->classId = Irp->class_id;

            xOS_PRINTK("Message %p read from queue: size %d\n",
                      rcvd_message,
                      rcvd_message->size);
            msize = (return_message->msg.size < SCT_MSG_DATA_SIZE) ?
                       return_message->msg.size :
                       SCT_MSG_DATA_SIZE;
            for (ndx = 0; ndx < msize; ndx++) {
                return_message->msg.data[ndx] = rcvd_message->data[ndx];
            }

            status = OS_STATUS_SUCCESS;
            CompleteRequest(Irp, &status);

            xOS_PRINTK("Returned message %p, size %lx to DLL\n",
                      return_message,
                      sizeof(sct_dll_msg_t));
            // Writing after returning message because S6 might
            // interrupt with response to write etc.
            // return the message back to S6
            writeStatus = sx_queue_write(&stretch_dev->queue_mod,
                                         stretch_dev->outgoing_message_return_queue,
                                         &rcvd_message->hdr,
                                         sizeof(sct_msg_t));
            xOS_PRINTK("Returned message to S6.  STATUS => %d\n",
                      writeStatus);
            if (writeStatus) {
                OS_PRINTK("Write queue failed!!!\n");
                status = OS_STATUS_IO_FAILURE;
            }
            GenerateInterrupt(stretch_dev);
            OS_SPINUNLOCK(&stretch_dev->MsgSpinLock);
        }
    }
}



//----------------------------------------
// Process returned messsages from the
// board's message return queue
//----------------------------------------
static void
ProcessReturnedMsg(stretch_dev_t                       *stretch_dev,
                   volatile sx_queue_datahdr_t __iomem *rcvd_msg_hdr)
{
    sct_dll_msg_t                  *irp_message;
    volatile sct_msg_t     __iomem *rcvd_message;
    int                             ndx;
    os_irp_t                       *Irp;
    os_status_t                     status;

    OS_PRINTK("++> DPC: Msg Returned by S6\n");

    // check if any message send is pending due to unavaialble messages.
    Irp = OS_PENDQUEUE_RETREIVE_CONTEXT
            (&stretch_dev->CancelSafeMsgQueue,
             SCT_MSG_SEND_CONTEXT,
             1);

    // Nothing Pending.
    if (Irp == NULL) {
        // No message send pending. so release the message
        ReleaseMessageToPool(&stretch_dev->regular_message_pool,
                             rcvd_msg_hdr);
    } else {
        xOS_PRINTK("Message available for pending message send\n");

        // Access buffer from original request
        irp_message = (sct_dll_msg_t *)Irp->system_buffer;


        // send message
        rcvd_message =
          (volatile sct_msg_t __iomem *)
            OS_CONTAINER_OF(rcvd_msg_hdr,
                            volatile __iomem sct_msg_t,
                            hdr);

        // Copy the message to send
        for (ndx = 0; ndx < irp_message->msg.size; ndx++) {
            rcvd_message->data[ndx] = irp_message->msg.data[ndx];
        }
        xOS_PRINTK("Sending message for class %d\n",
                  Irp->class_id);

        // Write the message to the board's queue
        sx_queue_write(&stretch_dev->queue_mod,
                       stretch_dev->outgoing_message_queue[Irp->class_id],
                       &rcvd_message->hdr,
                       sizeof(sct_msg_t));

        GenerateInterrupt(stretch_dev);

        status = OS_STATUS_SUCCESS;
        CompleteRequest(Irp, &status);
    }
}

void foo(stretch_dev_t *stretch_dev)
{
    volatile sx_queue_datahdr_t __iomem *msg_hdr;
    uint32_t                             offset;
    uint32_t                             ndx;
    uint32_t                             statusRegister = 0;

    // Handle all management messages in queue if indicated
    sx_queue_mgmt_read(&stretch_dev->queue_mod,
            stretch_dev->incoming_mgmt_queue,
            &msg_hdr);
    while (msg_hdr) {
        ProcessMgmtMsg(stretch_dev,
                msg_hdr);
        sx_queue_mgmt_read(&stretch_dev->queue_mod,
                stretch_dev->incoming_mgmt_queue,
                &msg_hdr);
    }
    // Otherwise check for regular messages
    // in each of the message queues, denoted by
    // a bitfied in queue_status_base+3, etc.
    offset = 1;
    for (ndx = 0; ndx < SCT_MSG_CLASS_COUNT; ndx++) {
        if ((ndx % 32) == 0) {
            statusRegister =
                sx_queue_status_read(&(stretch_dev->queue_mod),
                        stretch_dev->queue_status_base +
                        offset);
            offset++;
        }
        if (statusRegister & (1 << (ndx % 32))) {
            ProcessRegularMsg(stretch_dev, ndx);
        }
    }
}

//----------------------------------------
// This function will be called by the
// low-level interrupt handler.
// as an interrupt dispatch routine (DPC)
//----------------------------------------
#ifdef KVER_LESS_2620
void
stretch_dpc(void *dev_arg)
{
    struct work_struct * dev = (struct work_struct *)dev_arg;
#else
void
stretch_dpc(struct work_struct *dev)
{
#endif    
    //uint32_t                             statusRegister;
    //volatile sx_queue_datahdr_t __iomem *msg_hdr;
    os_workitem_t                       *os_workitem;
    stretch_dev_t                       *stretch_dev;

    os_workitem = OS_CONTAINER_OF(dev,
                                  os_workitem_t,
                                  item);
    stretch_dev = (stretch_dev_t *)os_workitem->context;
    OS_PRINTK("stretch_dev=%p, item=%p\n",stretch_dev,os_workitem);
    s_dpc( stretch_dev);
}

void s_dpc(stretch_dev_t * stretch_dev)
{
    uint32_t                             statusRegister;
    volatile sx_queue_datahdr_t __iomem *msg_hdr;
    xOS_PRINTK("DPC:Q=%p\n",os_workitem);
    // check status of mgmt and return queues
    statusRegister =
      sx_queue_status_read(&stretch_dev->queue_mod,
                           stretch_dev->queue_status_base);

    xOS_PRINTK("statusRegister 0 (%p) = %x\n",
              stretch_dev->queue_status_base,
              statusRegister);

    // In init the MsgHandlerThread isn't running, but
    // that's OK.  all that happens is returned
    // messages that don't need to complete, and
    // that don't need the context that the MsgH
    // thread is running under, so the thread isn't
    // needed.
    //WakeMsgHandlerThread(&stretch_dev->msg_handler);
    foo(stretch_dev);

    // For the rest, we do in the MsgHandler thread, because these
    // operations may want to copy back to user, which we can't do
    // here in dispatch because we have no user context, like the
    // thread does.  Just wake the thread here.

    // Handle returned messages if indicated
    if (statusRegister & 2) {
        sx_queue_read(&stretch_dev->queue_mod,
                      stretch_dev->incoming_message_return_queue,
                      &msg_hdr);
        while (msg_hdr) {
            ProcessReturnedMsg(stretch_dev, msg_hdr);
            sx_queue_read(&stretch_dev->queue_mod,
                          stretch_dev->incoming_message_return_queue,
                          &msg_hdr);
        }
    }

    xOS_PRINTK("<++ DPC\n");
}


/*
//----------------------------------------
// Message Handling Queue thread
//----------------------------------------
int
MsgHandlerThread(void *context)
{
    msg_handler_t                       *msg_handler;
    volatile sx_queue_datahdr_t __iomem *msg_hdr;
    stretch_dev_t                       *stretch_dev;
    uint32_t                             offset;
    uint32_t                             ndx;
    uint32_t                             statusRegister = 0;


    xOS_PRINTK("++> s6_msgH:\n");

    stretch_dev = (stretch_dev_t *)context;
    msg_handler = &stretch_dev->msg_handler;
    msg_handler->started = 1;
    while (!OS_THREAD_STOPPED) {
        set_current_state(TASK_INTERRUPTIBLE);
        if (!(OS_THREAD_STOPPED)) {
            schedule();
        }
        set_current_state(TASK_RUNNING);
        try_to_freeze();

        // Handle all management messages in queue if indicated
        sx_queue_mgmt_read(&stretch_dev->queue_mod,
                      stretch_dev->incoming_mgmt_queue,
                      &msg_hdr);
        while (msg_hdr) {
            ProcessMgmtMsg(stretch_dev,
                           msg_hdr);
            sx_queue_mgmt_read(&stretch_dev->queue_mod,
                          stretch_dev->incoming_mgmt_queue,
                          &msg_hdr);
        }
        // Otherwise check for regular messages
        // in each of the message queues, denoted by
        // a bitfied in queue_status_base+3, etc.
        {
            offset = 1;
            for (ndx = 0; ndx < SCT_MSG_CLASS_COUNT; ndx++) {
                if ((ndx % 32) == 0) {
                    statusRegister =
                        sx_queue_status_read(&(stretch_dev->queue_mod),
                                stretch_dev->queue_status_base +
                                offset);
                    offset++;
                }
                if (statusRegister & (1 << (ndx % 32))) {
                    ProcessRegularMsg(stretch_dev, ndx);
                }
            }
        }
    }
    msg_handler->started = 0;
    xOS_PRINTK("<++ s6_msgH\n");
    return 0;
}


//----------------------------------------
// Start the message-handler thread
//----------------------------------------
os_status_t
startMsgHandlerThread(msg_handler_t *msg_handler,
                      stretch_dev_t *stretch_dev)
{
    os_status_t   retval = OS_STATUS_SUCCESS;

    if (msg_handler->started) {
        xOS_PRINTK("Already started!\n");
        return retval;
    }

    // Invoke mhqProcessingThread
    msg_handler->thread = OS_THREAD_RUN(MsgHandlerThread,
                                        stretch_dev,
                                        "s6_MsgH");
    if (msg_handler->thread == NULL) {
        OS_PRINTK("Attempt to run MsgHandlerThread failed\n");
        retval = OS_STATUS_FAILURE;
    }
    return retval;
}


//----------------------------------------
// Stop the current message-handling thread
//----------------------------------------
int
stopMsgHandlerThread(msg_handler_t *msg_handler)
{
    if (msg_handler->started) {
        OS_THREAD_STOP(msg_handler->thread);
    }
    return 1;
}



//----------------------------------------
// Allocate memory for and initialize the
// message handling queue buffers and lists
//----------------------------------------
void
initMsgHandlerThread(msg_handler_t *msg_handler)
{
    msg_handler->started = 0;
}



//----------------------------------------
// De-allocate and de-initialize the message handling queue
//----------------------------------------
void releaseMsgHandlerThread(msg_handler_t *msg_handler)
{
    stopMsgHandlerThread(msg_handler);
}



//----------------------------------------
// Wake up the current message-handling thread
//----------------------------------------
void
WakeMsgHandlerThread(msg_handler_t *msg_handler)
{
    if (msg_handler->started != 0) {
        OS_THREAD_WAKE(msg_handler->thread);
    }
}
*/
