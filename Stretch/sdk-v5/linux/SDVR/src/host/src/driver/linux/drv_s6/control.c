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

#include <linux/fs.h>
#include "os_arch.h"
#include "board_arch.h"
#include "ipcqueue.h"
#include "ipcqueue_api.h"
#include "ipcmsg.h"
#include "msg_pool.h"
#include "pci_resources.h"
#include "dma_channels.h"
#include "shared_memory.h"
//#include "msg_queues.h"
#include "dispatch.h"
#include "control.h"
#include "stretch_main.h"
#include "sct_version.h"

//----------------------------------------
// Initialize the IPC queue structure
//   FromUser: none
//   ToUser:   none
// Does NOT return PENDING status
//----------------------------------------
os_status_t
InitIPCQueue(stretch_dev_t *stretch_dev,
             os_irp_t      *Irp)
{
    int                                  ndx;
    volatile sx_queue_datahdr_t __iomem *msg_hdr;
    sx_queue_mod_t                      *temp_mod;
    volatile uint32_t           __iomem *init_done;
    volatile sct_mgmt_msg_t     __iomem *init_mgmt_msg;
    volatile sx_queue_datahdr_t __iomem *message;
    dma_buf_node_t                      *buf;
    uint32_t                             minl;
    uint32_t                             minh;
    uint32_t                             maxl;
    uint32_t                             maxh;
    os_uint_arch_t                       offset;
    int                                  bits;
    int                                  retry = 0;
    os_status_t                          status = OS_STATUS_SUCCESS;
    uint32_t                             rc = 0;
    uint32_t                             done = 0;
    volatile uint32_t __iomem * preg;

    
    do {
        //----------------------------------------
        // If driver instance is already running, then we need to use
        // existing data
        //----------------------------------------
        if (stretch_dev->initialized) {
            break;
        }
        // Clear the init_done flag
        stretch_dev->queue_mod.queue_mod_rsc->init_done = 0;

        //----------------------------------------
        // SET PCI BAR[0] to start of shared memory.
        // This triggers Board to begin its own init.
        //----------------------------------------
        // Write the physical address to the scratch register to
        // fire off the Stretch board's INIT function
        preg = PCI_REG_PTR32(stretch_dev->registers,
                       SCT_SHARED_MEM_REG_OFFSET); 
        *preg = cpu_to_le32(OS_LO_DWORD(stretch_dev->queues.phy_addr));

        // Overlay shared queue header struct onto top of shared memory
        // and point to the init_done flag.
        init_done = (volatile uint32_t __iomem *)
                      &(stretch_dev->queue_mod.queue_mod_rsc->init_done);    
        iOS_PRINTK("init_done = %p,dev=%p\n",init_done,stretch_dev);

        //----------------------------------------
        // Wait for init_done to indicate that board is finished
        // initializing the shared memory queues.
        // retry for 10 seconds max.
        //----------------------------------------
        while (retry < LOOKUP_TIMEOUT_COUNT * 100) {

            if ((done = *init_done) !=0 ) {           
                break; // out of the retry loop
            } else {
                // Delay 10ms
                OS_DELAY_MS(10, KernelMode);
                retry ++;
            }
        }
        OS_PRINTK("after loop, init_done = %p\n",init_done);
        // Handle Timeout. Board sick.
        if (!done) {
            status = OS_STATUS_IO_FAILURE;
            iOS_PRINTK("Failure: Scratch register 0, after %d retrys, %p, %x\n",
                      retry, init_done, *(init_done)); 
            break;
        }   

        if ( done == cpu_to_le32(2) )
        {
            uint32_t offset = le32_to_cpu(*preg);
            void * pshmem = stretch_dev->pci_resources.bar_info[1].kern_addr + offset;
            stretch_dev->queue_mod.queue_mod_rsc=pshmem;
            OS_PRINTK("newer version of the board\n");
            iOS_PRINTK("reg value %x, shmem %p\n",offset, pshmem);
        }
        //----------------------------------------
        // Start up the msg handler thread. It
        // will start by taking a nap
        //----------------------------------------
        /*
        if ((status =
               startMsgHandlerThread
                 (&stretch_dev->msg_handler,
                  stretch_dev)) != OS_STATUS_SUCCESS) {
            break;
        }
        */
        //----------------------------------------
        // Start up the msg queue thread. It
        // will start by taking a nap
        //----------------------------------------
        /*
        if ((status =
               startMsgQueueThread
                 (&stretch_dev->msg_queues,
                 stretch_dev)) != OS_STATUS_SUCCESS) {
            break;
        }
        */
        //----------------------------------------
        // Call IPC API to initialize Queue Handle.
        //----------------------------------------
        sx_queue_module_init(&stretch_dev->queue_mod,
                             stretch_dev->queue_mod.queue_mod_rsc,
                             SX_QUEUE_OPTION_MULTI,
                             &temp_mod);
        if (temp_mod == NULL) {
            iOS_PRINTK("Failure to initialize queue module\n");
            break;
        }

        // Initialize the spinlock for the queue module
        OS_SPINLOCK_INIT(&stretch_dev->queue_mod.spinlock);

        //----------------------------------------
        // Find the incoming and outgoing management queues
        // and store locally
        //----------------------------------------
        sx_queue_lookup(&stretch_dev->queue_mod,
                        SCT_QUEUE_MGMT_ID(SCT_DIR_BOARD2HOST),
                        (volatile sx_queue_t __iomem**)
                        &stretch_dev->incoming_mgmt_queue);
        if (stretch_dev->incoming_mgmt_queue == NULL) {
            status = OS_STATUS_NO_RESOURCE;
            iOS_PRINTK("Failure to initialize incoming queue\n");
            break;
        }

        sx_queue_lookup(&stretch_dev->queue_mod,
                        SCT_QUEUE_MGMT_ID(SCT_DIR_HOST2BOARD),
                        (volatile sx_queue_t __iomem**)
                        &stretch_dev->outgoing_mgmt_queue);
        if (stretch_dev->outgoing_mgmt_queue == NULL) {
            status = OS_STATUS_NO_RESOURCE;
            iOS_PRINTK("Failure to initialize outgoing queue\n");
            break;
        }

        //----------------------------------------
        // Get the init message from the incoming management
        // queue that the board should have sent.
        // This init message has all the message blocks
        // attached and counts for each of the mgmt
        // and regular messages.
        //----------------------------------------
        rc = sx_queue_mgmt_read(&stretch_dev->queue_mod,
                           stretch_dev->incoming_mgmt_queue,
                           &msg_hdr);
        if (rc != 0) {
            iOS_PRINTK("Failure to read mgmt queue\n");
            status = OS_STATUS_IO_FAILURE;
            break;
        }

        // read allocated messages
        init_mgmt_msg =
          (volatile sct_mgmt_msg_t __iomem *)
            OS_CONTAINER_OF(msg_hdr,
                            volatile __iomem sct_mgmt_msg_t,
                            hdr);
        if (init_mgmt_msg->id != cpu_to_le32(SCT_MGMT_MSG_INIT)) {
            iOS_PRINTK("Failure to get init message, id is %d\n", init_mgmt_msg->id);
            status = OS_STATUS_NO_RESOURCE;
            break;
        }

        // get the status word
        stretch_dev->queue_status_base =
          (uint32_t __iomem *)
          OS_BOARD2HOST_PTR( init_mgmt_msg->data.init.status,
                            stretch_dev->queue_mod.queue_mod_rsc);
        *stretch_dev->queue_status_base = 0;
        sx_queue_status_register(&stretch_dev->queue_mod,
                                 (volatile sx_queue_t __iomem*)
                                 stretch_dev->incoming_mgmt_queue,
                                 stretch_dev->queue_status_base,
                                 0);

        //----------------------------------------
        // Lookup incoming message return queue
        // and store locally
        // (1 for all the incoming message queues).
        //----------------------------------------
        // Register the return queue
        sx_queue_lookup(&stretch_dev->queue_mod,
                        SCT_QUEUE_MSG_RTN_ID(SCT_DIR_BOARD2HOST),
                        &(stretch_dev->incoming_message_return_queue));
        if (stretch_dev->incoming_message_return_queue == NULL) {
            iOS_PRINTK("Failure to get return message queues for S6 to PC\n");
            status = OS_STATUS_NO_RESOURCE;
            break;
        }

        sx_queue_status_register(&stretch_dev->queue_mod,
                                 stretch_dev->incoming_message_return_queue,
                                 stretch_dev->queue_status_base,
                                 1);

        //----------------------------------------
        // Lookup incoming message queues (class 0..128)
        // and store locally
        //----------------------------------------
        for (ndx = 0; ndx < SCT_MSG_CLASS_COUNT; ndx++) {
            sx_queue_lookup(&stretch_dev->queue_mod,
                           SCT_QUEUE_MSG_ID(ndx, SCT_DIR_BOARD2HOST),
                           &stretch_dev->incoming_message_queue[ndx]);

            if (stretch_dev->incoming_message_queue[ndx] == NULL) {
                OS_PRINTK("FAILED to get message queues for S6 to Host\n");
                status = OS_STATUS_NO_RESOURCE;
                break;
            }
            // This is pointer math (uint32 *)
            offset = (ndx / 32) + 1;
            bits = ndx % 32;
            sx_queue_status_register(&stretch_dev->queue_mod,
                                     stretch_dev->incoming_message_queue[ndx],
                                     stretch_dev->queue_status_base + offset,
                                     (uint8_t)bits);
        }
        if (status != 0) {
            break;
        }

        //----------------------------------------
        // Lookup outgoing message return queue
        // and store locally
        // (1 for all the outgoing message queues).
        //----------------------------------------
        sx_queue_lookup(&stretch_dev->queue_mod,
                        SCT_QUEUE_MSG_RTN_ID(SCT_DIR_HOST2BOARD),
                        &stretch_dev->outgoing_message_return_queue);
        if (stretch_dev->outgoing_message_return_queue == NULL)
        {
            OS_PRINTK("FAILED to get return message queues for Host to S6\n");
            status = OS_STATUS_NO_RESOURCE;
            break;
        }

        //----------------------------------------
        // Lookup outgoing message queues (class 0..128)
        // and store locally
        //----------------------------------------
        for (ndx = 0; ndx < SCT_MSG_CLASS_COUNT; ndx++) {
            sx_queue_lookup(&stretch_dev->queue_mod,
                            SCT_QUEUE_MSG_ID(ndx, SCT_DIR_HOST2BOARD),
                            &stretch_dev->outgoing_message_queue[ndx]);
            if (stretch_dev->outgoing_message_queue[ndx] == NULL)
            {
                OS_PRINTK("FAILED to get message queues for Host to S6");
                status = OS_STATUS_NO_RESOURCE;
                break;
            } 
        }
        if (status != 0) {
            break;
        }

        //----------------------------------------
        // store management message blocks attached to
        // the init message into the available
        // management message pool
        //----------------------------------------
//        pdx->mgmtMessages.avail_msgs = init_mgmt_msg->data.init.mgmt_msg_count;
        InitMessagePool(&stretch_dev->mgmt_message_pool);
        for (ndx = 0;
             ndx < (int)(le32_to_cpu(init_mgmt_msg->data.init.mgmt_msg_count));
             ndx++) {
            if ((rc = sx_queue_mgmt_read(&stretch_dev->queue_mod,
                                    stretch_dev->incoming_mgmt_queue,
                                    &message)) != 0) {
                break;
            }
            ReleaseMessageToPool(&stretch_dev->mgmt_message_pool,
                                 message);
        }
        iOS_PRINTK("got %d mgmt messages from board\n",ndx);
        if (rc != 0) {
            status = OS_STATUS_NO_RESOURCE;
            break;
        }

        //----------------------------------------
        // store regular message blocks attached to
        // the init message into the available
        // regular message pool
        //----------------------------------------
//        pdx->regularMessages.avail_msgs = init_mgmt_msg->data.init.msg_count;
        InitMessagePool(&stretch_dev->regular_message_pool);
        for (ndx = 0;
             ndx < (int)(le32_to_cpu(init_mgmt_msg->data.init.msg_count));
             ndx++) {
            if ((rc = sx_queue_mgmt_read(&stretch_dev->queue_mod,
                                    stretch_dev->incoming_mgmt_queue,
                                    &message)) != 0) {
                break;
            }
            ReleaseMessageToPool(&stretch_dev->regular_message_pool,
                                 message);
        }
        iOS_PRINTK("got %d regular messages from board,msg_queue=%p\n",ndx,&(stretch_dev->regular_message_pool));
        if (rc != 0) {
            status = OS_STATUS_NO_RESOURCE;
            break;
        }

        //----------------------------------------
        // We are now intialized. Prepare and send back
        // the init message to the Board.
        //----------------------------------------
        stretch_dev->initialized = 1;
        init_mgmt_msg->rsp = 0;

        getDmaBufAddrLimit(&stretch_dev->dma_buffers,
                           &minl,
                           &minh,
                           &maxl,
                           &maxh,
                           (uint32_t)stretch_dev->pci_resources.max_dma_buff,
                           stretch_dev->pci_resources.dma_buf_size);

        xOS_PRINTK("~~> Sending Msg: DMA buffer limit %x %x %x %x\n",
                  minl,
                  minh,
                  maxl,
                  maxh);

        init_mgmt_msg->data.init.pci_min    = cpu_to_le32(minl);
        init_mgmt_msg->data.init.pci_min_hi = cpu_to_le32(minh);
        init_mgmt_msg->data.init.pci_max    = cpu_to_le32(maxl);
        init_mgmt_msg->data.init.pci_max_hi = cpu_to_le32(maxh);

        rc = sx_queue_mgmt_write(&stretch_dev->queue_mod,
                            stretch_dev->outgoing_mgmt_queue,
                            &init_mgmt_msg->hdr,
                            sizeof(sct_mgmt_msg_t));

        if (rc != 0) {
            status = OS_STATUS_IO_FAILURE;
            break;
        }

        for (ndx = 0; ndx < stretch_dev->pci_resources.max_dma_recv_buff; ndx++) {
            buf = getDmaBuffer(&stretch_dev->dma_buffers);
            sendFreeBufNotify(stretch_dev, buf);
        }
    } while (0);

    *((uint32_t *)Irp->user_buffer) = status;
    //IoReleaseRemoveLock(&pdx->RemoveLock, Irp);
    CompleteRequest(Irp, &status);
    return status;
}

os_status_t
do_send_destroy_msg(stretch_dev_t *stretch_dev,
           os_irp_t      *Irp)
{
    os_status_t status = OS_STATUS_SUCCESS;
    volatile sct_mgmt_msg_t     __iomem *new_message = NULL;
    volatile sx_queue_datahdr_t __iomem *temp_node;
    int                                  retry = 0;
    sct_mgmt_msg_t                      *this_message = NULL;

    do {
        this_message = (sct_mgmt_msg_t *)Irp->system_buffer;
        //----------------------------------------
        // Get a buffer from the available mgmt message pool
        // and place input data into it
        //----------------------------------------
        // Retry until a message area is available from pool
        do {
            temp_node = GetMessageFromPool(&stretch_dev->mgmt_message_pool);
            if (temp_node == NULL) { schedule(); }
            retry ++;
        } while ((temp_node == NULL) && (retry < 1000));

        if (temp_node == NULL )
        {
            status = OS_STATUS_FAILURE;
            break;
        }

        new_message =
            (volatile sct_mgmt_msg_t __iomem *)
            OS_CONTAINER_OF(temp_node,
                    volatile __iomem sct_mgmt_msg_t,
                    hdr);
        Irp->dma_channel->state = CHANNEL_STATE_CLOSE_PENDING;
        Irp->dma_channel->mgmt_queue_msg_hdr=NULL;
        new_message->id = cpu_to_le32(Irp->request_id);
        new_message->data.destroy.port = cpu_to_le32(Irp->dma_channel->port);
        // hack to distinguish the destroy message from our side
        new_message->data.connect.max_size = cpu_to_le32(0xa5a5a5a5);

        // write the message
        sx_queue_mgmt_write(&stretch_dev->queue_mod,
                stretch_dev->outgoing_mgmt_queue,
                &new_message->hdr,
                sizeof(sct_mgmt_msg_t));

        // Generate Interrupt
        GenerateInterrupt(stretch_dev);

        xOS_PRINTK("Waiting for DESTROY Ack from S6. port %d, t_count=%d, pending_count=%d\n",
                Irp->dma_channel->port,Irp->dma_channel->t_count, Irp->dma_channel->pending_count);
        OS_EVENT_WAIT_TIMEOUT(&Irp->dma_channel->destroyAck,HZ/3);
        Irp->dma_channel->state = CHANNEL_STATE_CLOSE;

        // Gather the response code then release message back to pool
        new_message =
            (volatile sct_mgmt_msg_t __iomem *)
            OS_CONTAINER_OF(Irp->dma_channel->mgmt_queue_msg_hdr,
                    volatile __iomem sct_mgmt_msg_t,
                    hdr);
        if ( new_message )
        {
            xOS_PRINTK("Wait done.  Received DESTROY Ack from S6\n");
            this_message->rsp = new_message->rsp;
            ReleaseMessageToPool(&stretch_dev->mgmt_message_pool,
                    &new_message->hdr);
        }
        else
        {
            xOS_PRINTK("Wait done. DESTROY Ack timeout\n");
            //timeout ,force close
            this_message->rsp = 0;
        }
        if (this_message->rsp == 0) {
            xOS_PRINTK("remove channel %d\n",Irp->port);
            // Destroy created channel structure
            RemoveChannel(Irp->dma_channel,
                    stretch_dev);
            xOS_PRINTK("remove channel %d done\n",Irp->port);
            // Success
            status = sizeof(sct_mgmt_msg_t);
        }
        else
        {
            status = OS_STATUS_FAILURE;
            xOS_PRINTK("S6 rejected DESTROY request\n");
        }
    } while(0);

    CompleteRequest(Irp, &status);
    return status;
}


//----------------------------------------
// Write a management message to the management
// queue (board)
//   FromUser: sct_mgmt_msg_t
//   ToUser:   sct_mgmt_msg_t
// May return PENDING status or suspend.
//----------------------------------------
os_status_t
WriteMgmtQueue(stretch_dev_t *stretch_dev,
               os_irp_t      *Irp)
{
    os_status_t                          status = OS_STATUS_SUCCESS;
    sct_mgmt_msg_t                      *this_message = NULL;

    if (!stretch_dev->initialized)
    {
        status = OS_STATUS_FAILURE;
        CompleteRequest(Irp, &status);
        return status;
    }

    this_message = (sct_mgmt_msg_t *)Irp->system_buffer;
    Irp->request_id = this_message->id;
    Irp->port = this_message->data.connect.port;
    switch (Irp->request_id) {
        //----------------------------------------
        // Send CONNECT message
        // Set up a channel and send the message to
        // board and PEND, waiting to copy msg back up.
        //----------------------------------------
        case SCT_MGMT_MSG_CONNECT:
            {
                volatile sct_mgmt_msg_t     __iomem *new_message = NULL;
                volatile sx_queue_datahdr_t __iomem *temp_node;
                int                                  retry = 0;
                OS_PRINTK("CONNECT msg: Id %d, Port %d\n",
                        Irp->request_id,
                        Irp->port);

                //----------------------------------------
                // Get a buffer from the available mgmt message pool
                // and place input data into it
                //----------------------------------------
                // Retry until a message area is available from pool
                do {
                    temp_node = GetMessageFromPool(&stretch_dev->mgmt_message_pool);
                    if (temp_node == NULL) { schedule(); }
                    retry ++;
                } while ((temp_node == NULL) && (retry < 1000));

                if (temp_node == NULL )
                {
                    status = OS_STATUS_FAILURE;
                    CompleteRequest(Irp, &status);
                    break;
                }

                new_message =
                    (volatile sct_mgmt_msg_t __iomem *)
                    OS_CONTAINER_OF(temp_node,
                            volatile __iomem sct_mgmt_msg_t,
                            hdr);

                //----------------------------------------
                // Prepare send channel
                Irp->dma_channel = InitializeChannel(&stretch_dev->dma_channels,
                        Irp->port);
                // Assign rest of the data.
                Irp->dma_channel->max_size = this_message->data.connect.max_size;

                Irp->dma_channel->state = CHANNEL_STATE_CONNECT_PENDING;
                Irp->dma_channel->type = CHANNEL_SEND;

                // re-assigning values from channel structure to message
                new_message->id = cpu_to_le32(Irp->request_id);
                new_message->data.connect.port = cpu_to_le32(Irp->dma_channel->port);
                new_message->data.connect.max_size  = cpu_to_le32(Irp->dma_channel->max_size);

                // Add this process and the completion routine
                // to cancel-safe pendqueue.
                Irp->iocb->ki_retry = ConnectMgmtMsgComplete;
                Irp->iocb->ki_cancel = IoCancel;
                status = OS_STATUS_PENDING_COPYBACK;
                OS_PENDQUEUE_INSERT(Irp,
                        &Irp->dma_channel->CancelSafeQueue,
                        CONNECT_PENDING_CONTEXT);

                CompleteRequest(Irp, &status);

                // write the message to the board, and interrupt it

                if (!sx_queue_mgmt_write(&stretch_dev->queue_mod,
                            stretch_dev->outgoing_mgmt_queue,
                            &new_message->hdr,
                            sizeof(sct_mgmt_msg_t)));
                GenerateInterrupt(stretch_dev);
            }
            break;

            //
            // Send DESTROY message to Board channel
            // Blocks but doesn't pend
            //
        case SCT_MGMT_MSG_DESTROY:
            do {
                xOS_PRINTK("DESTROY msg: Id %d, Port %d\n",
                        Irp->request_id,
                        Irp->port);
                Irp->dma_channel = getChannel(&stretch_dev->dma_channels,
                        Irp->port);
                if (Irp->dma_channel == NULL) {
                    iOS_PRINTK("FAILED to find channel for port %d\n",
                            Irp->port);
                    status = OS_STATUS_NO_RESOURCE;
                    break;
                }
                if (Irp->dma_channel->type == CHANNEL_RECV) {
                    iOS_PRINTK("Can not send SCT_MGMT_MSG_DESTROY on "
                            "receive channel port %d\n",
                            Irp->port);

                    status = OS_STATUS_INVALID_DEV;
                    break;
                }
                if (Irp->dma_channel->pending_count > 0)
                {
                    // Add this process and the completion routine
                    // to cancel-safe pendqueue
                    xOS_PRINTK("port %d: destroy pending, Irp=%p\n",Irp->dma_channel->port,Irp);
                    Irp->iocb->ki_retry = DestroyMgmtMsgComplete;
                    Irp->iocb->ki_cancel = IoCancel;
                    status = OS_STATUS_PENDING_COPYBACK;
                    OS_PENDQUEUE_INSERT(Irp,
                            &Irp->dma_channel->CancelSafeQueue,
                            DESTROY_PENDING_CONTEXT);
                    CompleteRequest(Irp, &status);
                }
                else
                {
                    status = do_send_destroy_msg(stretch_dev,Irp);
                }
            } while (0);
            break;
    }
    return status;
}

ssize_t
DestroyMgmtMsgComplete(struct kiocb *iocb)
{
    os_irp_t       *Irp = (os_irp_t *)iocb->private;
    stretch_dev_t  *stretch_dev = (stretch_dev_t *)iocb->ki_filp->private_data;
    OS_PRINTK("DestroyMgmtMsg(complete)\n");
    return do_send_destroy_msg(stretch_dev,Irp);
}

//----------------------------------------
// Collect a response from a Connect
// message via WriteMgmtQueue
// queue (board)
//   FromUser: sct_mgmt_msg_t
//   ToUser:   sct_mgmt_msg_t
// May return PENDING status or suspend.
//----------------------------------------
ssize_t
ConnectMgmtMsgComplete(struct kiocb *iocb)
{
    os_irp_t *Irp = (os_irp_t *)iocb->private;
    os_status_t         status = OS_STATUS_SUCCESS;

    OS_PRINTK("ConnectMgmtMsg(complete)\n");

    // Dispatch should have set up the return msg,
    // but wait to change the state here
    Irp->dma_channel->state = CHANNEL_STATE_ACTIVE;
    status = sizeof(sct_mgmt_msg_t);

    // Move from PENDING_COPYBACK to AWAITING_COMPLETION
    CompleteRequest(Irp, &status);
    return status;
}



//----------------------------------------
// Check for Connect request from Board
//   FromUser: sct_mgmt_msg_t
//   ToUser:   sct_mgmt_msg_t
// May return PENDING status
//----------------------------------------
os_status_t
CheckConnect(stretch_dev_t *stretch_dev,
             os_irp_t      *Irp)
{
    dma_channel_node_t *dma_channel;
    os_status_t         status = OS_STATUS_SUCCESS;
    sct_mgmt_msg_t     *irp_message;

    xOS_PRINTK("IOCTL_CHECK_CONNECT\n");
    // get the Input message structure from DLL.
    irp_message = (sct_mgmt_msg_t *)Irp->system_buffer;

    // Get the channel information from input DLL data
    Irp->port = irp_message->data.connect.port;

    OS_SPINLOCK(&stretch_dev->accept_spinlock);

    dma_channel =
      getConnectPendingChannel(&stretch_dev->dma_channels,
                               Irp->port);
    if (dma_channel == NULL) {
        // Connect is NOT yet requested from s6, so make this IRP
        // pending.
        xOS_PRINTK("Connect not yet requested by S6.  port %d\n",
                  Irp->port);

        // Initialize channel
        dma_channel =
          InitializeChannel(&stretch_dev->dma_channels,
                            Irp->port);

        // Store Current device object in this channel.
        dma_channel->type = CHANNEL_RECV;
        dma_channel->state = CHANNEL_STATE_CONNECT_PENDING;

        xOS_PRINTK("Channel %p initialized\n",
                  dma_channel);

        // Add IRP and completion routine in cancel-safe
        // queue for future processing.
        Irp->iocb->ki_retry = CheckConnectComplete;
        Irp->iocb->ki_cancel = IoCancel;
        status = OS_STATUS_PENDING_COPYBACK;
        OS_PENDQUEUE_INSERT(Irp,
                            &dma_channel->CancelSafeQueue,
                            CONNECT_PENDING_CONTEXT);
    } else {
        xOS_PRINTK("Already received connect from S6.  port %d chan %p\n",
                  Irp->port,
                  dma_channel);

        // Pass requested buffers information to DLL.
        irp_message->data.connect.max_size = dma_channel->max_size;
        irp_message->data.connect.port = dma_channel->port;
        dma_channel->state = CHANNEL_STATE_ACTIVE;

        // Update and Complete the IRP
        status = sizeof(sct_mgmt_msg_t);
    }

    OS_SPINUNLOCK(&stretch_dev->accept_spinlock);

    //IoReleaseRemoveLock(&stretch_dev->RemoveLock, Irp);
    CompleteRequest(Irp, &status);
    return status;
}



//----------------------------------------
// Complete Check Connect (accept) message
//----------------------------------------
ssize_t
CheckConnectComplete(struct kiocb *iocb)
{
    os_irp_t    *Irp = (os_irp_t *)iocb->private;
    os_status_t  status = sizeof(sct_mgmt_msg_t);

    xOS_PRINTK("*********> CHECK_CONNECT(complete)\n");

    //IoReleaseRemoveLock(&stretch_dev->RemoveLock, Irp);
    CompleteRequest(Irp, &status);
    return status;
}



//----------------------------------------
// Wait for the Channel to destroy
//   FromUser: sct_mgmt_msg_t
//   ToUser:   none
// May return PENDING status
//----------------------------------------
os_status_t
WaitDestroy(stretch_dev_t *stretch_dev,
            os_irp_t      *Irp)
{
    sct_mgmt_msg_t     *irp_message;
    uint32_t            port;
    dma_channel_node_t *dma_channel;
    os_status_t         status = OS_STATUS_SUCCESS;

    // get the Input message structure from DLL.
    irp_message = (sct_mgmt_msg_t *)Irp->system_buffer;

    // Get the channel information from input DLL data
    port = irp_message->data.connect.port;
    Irp->port = port;
    xOS_PRINTK("Port %d, response %d\n",
              port,
              irp_message->rsp);
    dma_channel = getChannel(&stretch_dev->dma_channels,
                             port);
    if (dma_channel == NULL) {
        OS_PRINTK("FAILED to find channel for port %d\n",
                  port);
        status = OS_STATUS_FAILURE;
    } else {
        // Mark this IRP as pending and insert in cancel-safe queue
        status = OS_STATUS_PENDING;
        Irp->iocb->ki_cancel = IoCancel;
        OS_PENDQUEUE_INSERT(Irp,
                            &dma_channel->CancelSafeQueue,
                            DESTROY_CONTEXT);
    }

    //IoReleaseRemoveLock(&stretch_dev->RemoveLock, Irp);
    CompleteRequest(Irp, &status);
    return status;
}

//----------------------------------------
// Wait for Channel Error
//   FromUser: sct_mgmt_msg_t
//   ToUser:   none
// May return PENDING status
//----------------------------------------
os_status_t
WaitError(stretch_dev_t *stretch_dev,
          os_irp_t      *Irp)
{
    sct_mgmt_msg_t     *irp_message;
    uint32_t            port;
    dma_channel_node_t *dma_channel;
    os_status_t         status;

    // get the Input message structure from DLL.
    irp_message = (sct_mgmt_msg_t *)Irp->system_buffer;

    // Get the channel information from input DLL data
    port = irp_message->data.connect.port;
    xOS_PRINTK("Port %d, response %d\n",
              port,
              irp_message->rsp);
    dma_channel = getChannel(&stretch_dev->dma_channels,
                             port);
    if (dma_channel == NULL) {
        OS_PRINTK("FAILED to find channel for port %d\n",
                  port);
        status = OS_STATUS_FAILURE;
    } else {
        // Mark this IRP as pending and insert in cancel-safe queue
        status = OS_STATUS_PENDING;
        OS_PENDQUEUE_INSERT(Irp,
                            &dma_channel->CancelSafeQueue,
                            ERROR_CONTEXT);
    }

    //IoReleaseRemoveLock(&stretch_dev->RemoveLock, Irp);
    CompleteRequest(Irp, &status);
    return status;
}



//----------------------------------------
// Respond to Destroy message
//   FromUser: sct_mgmt_msg_t
//   ToUser:   none
// Does NOT return PENDING status
//----------------------------------------
os_status_t
DestroyResponse(stretch_dev_t *stretch_dev,
                os_irp_t      *Irp)
{
    sct_mgmt_msg_t                  *irp_message; 
    volatile sct_mgmt_msg_t __iomem *chan_message;
    uint32_t                         port;
    dma_channel_node_t              *dma_channel;
    os_status_t                      status;

    // get the Input message structure from DLL.
    irp_message = (sct_mgmt_msg_t *)Irp->system_buffer;

    // Get the channel information from input DLL data
    port = irp_message->data.connect.port;
    xOS_PRINTK("Port %d\n", // response %d\n",
              port);

    dma_channel = getChannel(&stretch_dev->dma_channels,
                             port);

    if (dma_channel == NULL) {
        OS_PRINTK("IOCTL_DESTROY_RESPONSE - "
                  "FAILED to find channel for port %d\n",
                  port);

        status = OS_STATUS_FAILURE;
    } else {
        dma_channel->state =  CHANNEL_STATE_CLOSE_PENDING;
        chan_message =
          (volatile sct_mgmt_msg_t __iomem *)
            OS_CONTAINER_OF(dma_channel->mgmt_queue_msg_hdr,
                            volatile __iomem sct_mgmt_msg_t,
                            hdr);

        // Destroy created channel structure
        RemoveChannel(dma_channel, stretch_dev);

        if (chan_message)
        {
            // Update status to S6
            chan_message->rsp = 0;

            // return the message back to S6
            sx_queue_mgmt_write(&stretch_dev->queue_mod,
                       stretch_dev->outgoing_mgmt_queue,
                       &chan_message->hdr,
                       sizeof(sct_mgmt_msg_t));

            GenerateInterrupt(stretch_dev);
        }
        status = OS_STATUS_SUCCESS;
    }
    //IoReleaseRemoveLock(&stretch_dev->RemoveLock, Irp);
    CompleteRequest(Irp, &status);
    return status;
}



//----------------------------------------
// Respond (Accept) to a potential connect message
//   FromUser: sct_mgmt_msg_t
//   ToUser:   none
// Does NOT return PENDING status
//----------------------------------------
os_status_t
AcceptResponse(stretch_dev_t *stretch_dev,
               os_irp_t      *Irp)
{
    sct_mgmt_msg_t     *irp_message;
    sct_mgmt_msg_t     *message;
    uint32_t            port;
    dma_channel_node_t *dma_channel;
    os_status_t         status = OS_STATUS_SUCCESS;

    // get the Input message structure from DLL.
    irp_message = (sct_mgmt_msg_t *)Irp->system_buffer;

    // Get the channel information from input DLL data
    port = irp_message->data.connect.port;
    dma_channel = getChannel(&stretch_dev->dma_channels,
                             port);
    if (dma_channel == NULL) {
        xOS_PRINTK("IOCTL_ACCEPT_RESPONSE - "
                  "FAILED to find channel for port %d\n",
                  port);
        status = OS_STATUS_FAILURE;
    }
    else if (dma_channel->mgmt_queue_msg_hdr== 0)
    {
        iOS_PRINTK("IOCTL_ACCEPT_RESPONSE - "
                  "invalid msg hdr in port %d\n",
                  port);
        status = OS_STATUS_FAILURE;
    }
    else {
        // update resp to the user
        message = (sct_mgmt_msg_t *)dma_channel->mgmt_queue_msg_hdr;
        message->rsp = cpu_to_le32(irp_message->rsp);
        status = OS_STATUS_SUCCESS;
    }
    //IoReleaseRemoveLock(&stretch_dev->RemoveLock, Irp);
    CompleteRequest(Irp, &status);

    if (status == OS_STATUS_SUCCESS) {
        // write response message.
        sx_queue_mgmt_write(&stretch_dev->queue_mod,
                       stretch_dev->outgoing_mgmt_queue,
                       dma_channel->mgmt_queue_msg_hdr,
                       sizeof(sct_mgmt_msg_t));
        dma_channel->mgmt_queue_msg_hdr = NULL;
        GenerateInterrupt(stretch_dev);
    }
    return status;
}

os_status_t
do_buffer_send(stretch_dev_t *stretch_dev,
           os_irp_t      *Irp)
{           
    dma_channel_node_t *dma_channel;
    os_status_t         status = OS_STATUS_SUCCESS;
    dma_buf_node_t                      *this_dma_buf;
    volatile sx_queue_datahdr_t __iomem *temp_node;
    volatile sct_mgmt_msg_t     __iomem *message;
            
    do {
        dma_channel = getChannel(&stretch_dev->dma_channels,
                Irp->port);
        if (dma_channel == NULL) {
            iOS_PRINTK("No channel found for port %d\n",
                    Irp->port);
            status = OS_STATUS_FAILURE;
            break;
        }
        if (Irp->size > stretch_dev->pci_resources.dma_buf_size)
        {
            iOS_PRINTK("Buffer size (%d) is too large at port %d\n", (int)Irp->size,
                    Irp->port);
            status = OS_STATUS_FAILURE;
            break;
        }
        if (Irp->dma_buf == 0)
            this_dma_buf = getDmaBuffer(&stretch_dev->dma_buffers);
        else
        {
            this_dma_buf = Irp->dma_buf;
            Irp->dma_buf = 0;
        }
        if ((this_dma_buf == NULL)) {
            OS_PRINTK("Port:%d: No extra dma buffer avaliable\n",
                    dma_channel->port);
            status = OS_STATUS_PENDING_COPYBACK;
            break;
        }
        temp_node = GetMessageFromPool(&stretch_dev->mgmt_message_pool);
        if ( temp_node == NULL)
        {

            freeDmaBuffer(stretch_dev, this_dma_buf, dma_channel);                         
            status = OS_STATUS_PENDING_COPYBACK;
        }
        else
        {
            int rc;
            message =
                (volatile sct_mgmt_msg_t __iomem *)
                OS_CONTAINER_OF(temp_node,
                        volatile __iomem sct_mgmt_msg_t,
                        hdr);
            rc = OS_COPY_FROM_USER(this_dma_buf->kern_addr,
                                   Irp->user_buffer,
                                   Irp->size);
            message->id = cpu_to_le32(SCT_MGMT_MSG_HOST_SENDBUF);
            message->data.buffer.port = cpu_to_le32(dma_channel->port);
            this_dma_buf->size = Irp->size;
            this_dma_buf->irp = Irp;
            OS_LIST_ADD_TAIL(&this_dma_buf->list_node, &dma_channel->buf_list);
            message->data.buffer.size = cpu_to_le32(Irp->size);
            message->data.buffer.pci_addr = cpu_to_le32(
                OS_LO_DWORD(this_dma_buf->phy_addr));
            message->data.buffer.pci_addr_hi = cpu_to_le32(
                OS_HI_DWORD(this_dma_buf->phy_addr));
            sx_queue_mgmt_write(&stretch_dev->queue_mod,
                    stretch_dev->outgoing_mgmt_queue,
                    &message->hdr,
                    sizeof(sct_mgmt_msg_t));
            dma_channel->t_count++;
            xOS_PRINTK("interrupt board port %d, count %d\n",Irp->port,dma_channel->t_count);
            GenerateInterrupt(stretch_dev);
            status = Irp->size;
        }
    } while (0);
    if( status == OS_STATUS_PENDING_COPYBACK)
    {
        Irp->iocb->ki_retry = BufferSendComplete;
        Irp->iocb->ki_cancel = IoCancel;
        OS_PENDQUEUE_INSERT(Irp,
                &dma_channel->CancelSafeBuffersQueue,
                ANY_CONTEXT);
        appendPendingChannel(dma_channel,
                &stretch_dev->dma_buffers);
    }
    //IoReleaseRemoveLock(&stretch_dev->RemoveLock, Irp);
    CompleteRequest(Irp, &status);
    return status;
}

os_status_t
BufferSend(stretch_dev_t *stretch_dev,
           os_irp_t      *Irp)
{   
    /*
    {
        struct timeval tv;
        do_gettimeofday(&tv);
        Irp->start_time = tv.tv_usec;
        OS_PRINTK("send request port %d, sec=%ld,usec=%ld\n",Irp->port,tv.tv_sec,tv.tv_usec);
    }
    */
    return do_buffer_send(stretch_dev,Irp);
}

ssize_t
BufferSendComplete(struct kiocb *iocb)
{
    os_irp_t       *Irp = (os_irp_t *)iocb->private;
    stretch_dev_t  *stretch_dev = (stretch_dev_t *)iocb->ki_filp->private_data;
    OS_PRINTK("send retry port %d\n",Irp->port);
    return do_buffer_send( stretch_dev,Irp);
}

//----------------------------------------
// Gather (store) the next buffer-ful of data
//   FromUser: offset contains port value
//   ToUser:   vectored buffer/size or PEND
// May return PENDING status
//----------------------------------------
os_status_t
StoreReceiveBuffer(stretch_dev_t *stretch_dev,
                   os_irp_t      *Irp)
{           
    dma_channel_node_t  *dma_channel;
    os_status_t          status = OS_STATUS_SUCCESS;

    OS_PRINTK("Requested for port %d pointer %p, size %ld\n",
              Irp->port,
              Irp->user_buffer,
              Irp->size);

    dma_channel = getChannel(&stretch_dev->dma_channels,
                             Irp->port);
    if (dma_channel == NULL) {
        iOS_PRINTK("Channel not found %d\n",Irp->port);
        status = OS_STATUS_FAILURE;
    }
    else
    {
        OS_SPINLOCK(&dma_channel->spinlock);
        if (!OS_LIST_IS_EMPTY(&dma_channel->buf_list)) {
            dma_buf_node_t      *dma_buf_node = NULL;
            os_list_node_t      *temp_node;
            int retval;
            size_t cpsize;
            OS_PRINTK("buffer ready\n");
            temp_node = OS_LIST_RETREIVE_HEAD(&dma_channel->buf_list,
                                              1);   
            dma_buf_node = OS_CONTAINER_OF(temp_node,
                                           dma_buf_node_t,
                                           list_node);
            OS_SPINUNLOCK(&dma_channel->spinlock);
            cpsize = min( dma_buf_node->size, (size_t)stretch_dev->pci_resources.dma_buf_size );
            if ((retval = OS_COPY_TO_USER(Irp->user_buffer,
                                 dma_buf_node->kern_addr,
                                 cpsize)) != 0) {
                iOS_PRINTK("Failed to copy %d bytes\n",retval);
                status = OS_STATUS_FAILURE;
            }
            else {
                OS_PRINTK("direct copied %d bytes, port %d\n",
                  (uint32_t)dma_buf_node->size,dma_channel->port);
                status = dma_buf_node->size;
                //Irp->iocb->ki_left = 0;
                //Irp->iocb->ki_nbytes = dma_buf_node->size;
            }
            sendFreeBufNotify(stretch_dev,
                              dma_buf_node);
        } else 
        if (dma_channel->state ==
                   CHANNEL_STATE_CLOSE_PENDING) {
            OS_SPINUNLOCK(&dma_channel->spinlock);
            xOS_PRINTK("This chan %p port %d has DESTROY pending\n",
                      dma_channel,
                      Irp->port);
            status = OS_STATUS_FAILURE;
        } else {
            // Add IRP in cancel-safe queue for future processing.
            Irp->iocb->ki_retry = StoreReceiveBufferComplete;
            Irp->iocb->ki_cancel = IoCancel;
            status = OS_STATUS_PENDING_COPYBACK;
            OS_PRINTK("no ready buffer, irp pending\n");
            OS_PENDQUEUE_INSERT(Irp,
                                &dma_channel->CancelSafeBuffersQueue,
                                ANY_CONTEXT);              
            OS_SPINUNLOCK(&dma_channel->spinlock);
        }
    }
    CompleteRequest(Irp, &status); 
    return status;
}

//----------------------------------------
// (actually) Read a frame buffer from a port.
// This gets called from the user-space callback
// routine that gets called when the dispatch/irq
// indicates data ready in DMA buffer.
//   FromUser: IOCTL command in size
//   ToUser:   Frame buffer
// Does NOT return PENDING status
//----------------------------------------
ssize_t
StoreReceiveBufferComplete(struct kiocb *iocb)
{
    int             retval = 0;
    os_status_t     status = OS_STATUS_SUCCESS;
    os_irp_t       *Irp = (os_irp_t *)iocb->private;
    stretch_dev_t  *stretch_dev = (stretch_dev_t *)iocb->ki_filp->private_data;
    dma_buf_node_t *dma_buf_node = Irp->dma_buf;
    size_t     cpsize;

    OS_PRINTK("*********> STORE_RECEIVE_BUFFER(complete)\n");
    // Handle this special DMA mem copy here and not in
    // CompleteRequest.
    cpsize = min(dma_buf_node->size,(size_t)stretch_dev->pci_resources.dma_buf_size);
    if ((retval = OS_COPY_TO_USER(Irp->user_buffer,
                                  dma_buf_node->kern_addr,
                                  cpsize)) != 0)
    {
        iOS_PRINTK("Copy to user space failed with %d,dst=%p,src=%p,size=%d\n",
                retval, Irp->user_buffer , dma_buf_node->kern_addr, (int)dma_buf_node->size);
        status = OS_STATUS_IO_FAILURE;
    } else {
        OS_PRINTK("copied %d bytes\n",
                  (uint32_t)dma_buf_node->size);
        status = dma_buf_node->size;
    }

    // Complete the IRP
    CompleteRequest(Irp, &status);
    sendFreeBufNotify(stretch_dev,
                      dma_buf_node);
    return status;
}



//----------------------------------------
// Write a message to a channel.
//   FromUser: sct_dll_msg_t
//   ToUser:   none
// May return PENDING status
//----------------------------------------
os_status_t
WriteMsg(stretch_dev_t *stretch_dev,
         os_irp_t      *Irp)
{           
    sct_dll_msg_t                       *irp_message;
    volatile sx_queue_datahdr_t __iomem *temp_node;
    volatile sct_msg_t          __iomem *new_message;
    uint32_t                             ndx;
    uint32_t                             writeStatus;
    uint32_t                             status = OS_STATUS_SUCCESS;

    // Get the input message structure from DLL.
    irp_message = (sct_dll_msg_t *)Irp->system_buffer;
    Irp->class_id = irp_message->classId;

    temp_node = GetMessageFromPool(&stretch_dev->regular_message_pool);
    if (temp_node == NULL) {
        iOS_PRINTK("Message NOT yet available.  Make IRP pending.\n");

        // wait till message is available
        // change following logic with event

        // Add IRP in cancel-safe queue for future processing.
        status = OS_STATUS_PENDING;
        OS_PENDQUEUE_INSERT(Irp,
                            &stretch_dev->CancelSafeMsgQueue,
                            SCT_MSG_SEND_CONTEXT);
    } else {
        new_message =
          (volatile sct_msg_t *)
            OS_CONTAINER_OF(temp_node,
                            volatile __iomem sct_msg_t,
                            hdr);

        // Copy the message
        for (ndx = 0; ndx < irp_message->msg.size; ndx++) {
            new_message->data[ndx] = irp_message->msg.data[ndx];
        }
        status = OS_STATUS_SUCCESS;

        writeStatus =
          sx_queue_write(&stretch_dev->queue_mod,
                         stretch_dev->outgoing_message_queue[Irp->class_id],
                         &new_message->hdr,
                         sizeof(sct_msg_t));
        if (writeStatus) {
            OS_PRINTK("FAILED to write msg. status %x\n",
                      writeStatus);
        }
        GenerateInterrupt(stretch_dev);
    }           

    //IoReleaseRemoveLock(&stretch_dev->RemoveLock, Irp);
    CompleteRequest(Irp, &status);
    return status;
}



//----------------------------------------
// (initiate) Read a message from a channel.
//   FromUser: sct_dll_msg_t
//   ToUser:   sct_dll_msg_t (modified) or PENDING
// May return PENDING status
//----------------------------------------
os_status_t
ReadMsg(stretch_dev_t *stretch_dev,
        os_irp_t      *Irp)
{
    sct_dll_msg_t                       *irp_message;
    volatile sx_queue_datahdr_t __iomem *rcvd_msg_hdr;
    int                                  messageRecvd;
    volatile sct_msg_t          __iomem *rcvd_message;
    uint32_t                             msize;
    uint32_t                             ndx;
    uint32_t                             writeStatus;
    os_status_t                          status = OS_STATUS_SUCCESS;

    OS_SPINLOCK(&stretch_dev->MsgSpinLock);

    irp_message = (sct_dll_msg_t *)Irp->system_buffer;
    Irp->class_id = irp_message->classId;
    OS_PRINTK("Read message class %d\n",irp_message->classId);

    messageRecvd = IsMessageRecvd(Irp->class_id,
                                  stretch_dev,
                                  &rcvd_msg_hdr);

    if (messageRecvd == 1) {
        OS_SPINUNLOCK(&stretch_dev->MsgSpinLock);

        // Copy the message
        rcvd_message =
          (volatile sct_msg_t __iomem *)
             OS_CONTAINER_OF(rcvd_msg_hdr,
                             volatile __iomem sct_msg_t,
                             hdr);

        irp_message->msg.size = le32_to_cpu(rcvd_message->size);
        irp_message->classId = Irp->class_id;

        msize = (rcvd_message->size < SCT_MSG_DATA_SIZE) ?
                   rcvd_message->size :
                   SCT_MSG_DATA_SIZE;
        OS_PRINTK("Message waiting from class %d, size =%d Irp state = %d\n",
                 Irp->class_id, msize, Irp->pend_state);
        for (ndx = 0; ndx < msize; ndx++) {
            irp_message->msg.data[ndx] = rcvd_message->data[ndx];
        }
        // return the message back to S6
        writeStatus =
          sx_queue_write(&stretch_dev->queue_mod,
                         stretch_dev->outgoing_message_return_queue,
                         &rcvd_message->hdr,
                         sizeof(sct_msg_t));

        if (writeStatus) {
            xOS_PRINTK("FAILED to return msg. status %x\n",
                      writeStatus);
            status = 0;
        } else {
            status = msize;
        }
        GenerateInterrupt(stretch_dev);
    } else {

        // wait for the message by making IRP pending.
        OS_PRINTK("Message %d NOT yet received.  Make IRP pending.\n",
                  Irp->class_id);

        // Add IRP, and completion routine in cancel-safe queue
        // for future processing.
        Irp->iocb->ki_retry = ReadMsgComplete;
        Irp->iocb->ki_cancel = IoCancel;
        status = OS_STATUS_PENDING_COPYBACK;
        if (Irp->class_id < 0) {
            OS_PENDQUEUE_INSERT(Irp,
                                &stretch_dev->CancelSafeMsgQueue,
                                SCT_MSG_CLASS_COUNT);
        } else {
            OS_PENDQUEUE_INSERT(Irp,
                                &stretch_dev->CancelSafeMsgQueue,
                                Irp->class_id);
        }

        // Release the lock now
        OS_SPINUNLOCK(&stretch_dev->MsgSpinLock);
    }           

    //IoReleaseRemoveLock(&stretch_dev->RemoveLock, Irp);
    CompleteRequest(Irp, &status);
    return status;
}

//----------------------------------------
// (actually) Read a message from a channel.
// This gets called from the user-space callback
// routine that gets called when the dispatch/irq
// indicates data ready in queue(s).
//   FromUser: IOCTL command in offset (via lseek call)
//   ToUser:   sct_dll_msg_t
// Does NOT return PENDING status
//----------------------------------------
ssize_t ReadMsgComplete(struct kiocb *iocb)
{
    os_status_t  status;
    os_irp_t    *Irp = (os_irp_t *)iocb->private;

    OS_PRINTK("ReadMsgComplete class %d\n",Irp->class_id);
    status = sizeof(sct_dll_msg_t);
    CompleteRequest(Irp, &status);
    return status;
}



//----------------------------------------
// See if there is a message from a channel.
//   FromUser: sct_dll_msg_t
//   ToUser:   sct_dll_msg_t (modified) or PENDING
// Does NOT pend
//----------------------------------------
os_status_t
ReadMsgPoll(stretch_dev_t *stretch_dev,
            os_irp_t      *Irp)
{
    sct_dll_msg_t                        *irp_message;
    volatile sx_queue_datahdr_t __iomem **temp_data_hdr;
    int                                   messageRecvd;
    volatile sct_msg_t          __iomem  *temp_message;
    uint32_t                              ndx;
    os_status_t                           status = OS_STATUS_SUCCESS;

    irp_message = (sct_dll_msg_t *)Irp->system_buffer;
    temp_data_hdr =
      (volatile sx_queue_datahdr_t __iomem **)
         OS_KMALLOC(sizeof(sx_queue_datahdr_t *), "q");

    Irp->class_id = irp_message->classId;
    messageRecvd = IsMessageRecvd(Irp->class_id,
                                  stretch_dev,
                                  temp_data_hdr);

    if (messageRecvd) {
        // Copy the message
        temp_message =
          (volatile sct_msg_t __iomem *)
             OS_CONTAINER_OF(*temp_data_hdr,
                             volatile __iomem sct_msg_t,
                             hdr);
        irp_message->msg.size = le32_to_cpu(temp_message->size);
        irp_message->classId = (Irp->class_id);

        for (ndx = 0; ndx < temp_message->size; ndx++) {
            irp_message->msg.data[ndx] = temp_message->data[ndx];
        }

        // return the message back to S6
        sx_queue_write(&stretch_dev->queue_mod,
                       stretch_dev->outgoing_message_queue[Irp->class_id],
                       *temp_data_hdr,
                       sizeof(sct_msg_t));

        // return to the DLL
        status = sizeof(sct_dll_msg_t);
        GenerateInterrupt(stretch_dev);
    } else {
        OS_PRINTK("no msg received\n");
        status = OS_STATUS_FAILURE;
    }

    //IoReleaseRemoveLock(&pdx->RemoveLock, Irp);
    CompleteRequest(Irp,
                    &status);
    OS_KFREE(temp_data_hdr);
    return status;
}



//----------------------------------------
// This function checks if message is
// received for specified class or not.
// If message is received, it returns the
// message in datahdr and returns true.
//----------------------------------------
int
IsMessageRecvd(int                                   classId,
               stretch_dev_t                        *stretch_dev,
               volatile sx_queue_datahdr_t __iomem **datahdr)
{
    int           ndx;
    int           recvd = 0;
    uint32_t      rc = 1;

    if (classId < 0) {
        // check message from all message queues
        for (ndx = 0; ndx < SCT_MSG_CLASS_COUNT; ndx++) {
            rc = sx_queue_read(&stretch_dev->queue_mod,
                               stretch_dev->incoming_message_queue[ndx],
                               datahdr);
            if (rc == 0) {
                xOS_PRINTK("Message received from class %d\n",
                          ndx);
                classId = ndx;
                recvd = 1;
                break;
            }
        }
    } else {
        // read specified queue
        rc = sx_queue_read(&stretch_dev->queue_mod,
                           stretch_dev->incoming_message_queue[classId],
                           datahdr);
        if (rc == 0) {
            xOS_PRINTK("Message received from specified class %d\n",
                      classId);
            recvd = 1;
        }
    }
    return recvd;
}

//----------------------------------------
// Generate an interrupt by writing to the
// interrupt register on board
//----------------------------------------
void
GenerateInterrupt(stretch_dev_t *stretch_dev)
{
    *PCI_REG_PTR32(stretch_dev->registers,
                   S6_INTERRUPT_OFFSET) = cpu_to_le32(1);
}



//----------------------------------------
// Reset the board by stopping services/queues
// and issuing reset at GPIO pins
//----------------------------------------
void
ResetBoard(stretch_dev_t *stretch_dev)
{
#if 0
    uint32_t *interrupt;
#endif

    //stopMsgQueueThread(&stretch_dev->msg_queues);
    //stopMsgHandlerThread(&stretch_dev->msg_handler);
    DestroyAllChannels(stretch_dev);
    ClearCancelSafeMsgQueue(stretch_dev,
                            SCT_MSG_CLASS_ANY);
    *PCI_REG_PTR32(stretch_dev->registers,
                   0x70114) = cpu_to_le32(1000000);

    *PCI_REG_PTR32(stretch_dev->registers,
                   0x70118) = cpu_to_le32(1000000);

    //after reset, we reinitialize all of the list and status, to support PCI boot
    stretch_dev->initialized = 0;
    stretch_dev->booted = 0;
    {
        dma_buffers_t   *dma_buffers = &stretch_dev->dma_buffers;
        //msg_queues_t *msg_queues = (&stretch_dev->msg_queues);
        int ndx;
        OS_LIST_INIT( &(dma_buffers->buf_free_list));
        OS_LIST_INIT( &(dma_buffers->chan_pending_list));
        for (ndx = 0; ndx < stretch_dev->pci_resources.max_dma_buff; ndx++) {
           OS_LIST_ADD_TAIL(&(dma_buffers->pool[ndx].list_node),
                            &(dma_buffers->buf_free_list));
        }
        /*
        stopMsgQueueThread(msg_queues);
        msg_queues->started = 0;   
        OS_LIST_INIT(&msg_queues->active_queue);
        OS_LIST_INIT(&msg_queues->free_queue);
        for (ndx = 0; ndx < msg_queues->total; ndx++) {
            OS_LIST_ADD_TAIL(&(msg_queues->pool[ndx].list_node),
                         &(msg_queues->free_queue));
        }
        */
        stretch_dev->queue_mod.queue_mod_rsc =
            (sx_queue_mod_rsc_t *)stretch_dev->queues.kern_addr;
        DeInitializeDispatch( stretch_dev );
        InitializeDispatch( stretch_dev );
    }
}



//----------------------------------------
// Notify the board about a free buffer via
// the management message queue
//----------------------------------------
void
sendFreeBufNotify(stretch_dev_t *stretch_dev,
                  dma_buf_node_t *dma_buffer)
{
    volatile sct_mgmt_msg_t     __iomem *message;
    volatile sx_queue_datahdr_t __iomem *datahdr;

    // Notify S6 about freed buffer
    datahdr = GetMessageFromPool(&stretch_dev->mgmt_message_pool);     
    if (datahdr && dma_buffer) {
        message =
          (volatile sct_mgmt_msg_t *)
            OS_CONTAINER_OF(datahdr,
                            volatile __iomem sct_mgmt_msg_t,
                            hdr);
        message->id = cpu_to_le32(SCT_MGMT_MSG_HOST_RECVBUF);
        message->data.buffer.pci_addr = cpu_to_le32(OS_LO_DWORD(dma_buffer->phy_addr));
        message->data.buffer.pci_addr_hi = cpu_to_le32(OS_HI_DWORD(dma_buffer->phy_addr)); 
        OS_PRINTK("pci_addr= %x, pci_addr_hi= %x\n",message->data.buffer.pci_addr,message->data.buffer.pci_addr_hi);
        sx_queue_mgmt_write(&stretch_dev->queue_mod,
                       stretch_dev->outgoing_mgmt_queue,
                       datahdr,
                       sizeof(sct_mgmt_msg_t));

        xOS_PRINTK("send freebuf %x\n",
                  OS_LO_DWORD(dma_buffer->phy_addr));
        GenerateInterrupt(stretch_dev);
    }
}



//----------------------------------------
// This function destroys ALL channels
//----------------------------------------
void
DestroyAllChannels(stretch_dev_t *stretch_dev)
{
    dma_channels_t *dma_channels = &stretch_dev->dma_channels;
    
    while (dma_channels->size > 0) {
        OS_PRINTK("Destroying channel index %d\n", dma_channels->pool[0]->port
                  );
        RemoveChannel(dma_channels->pool[0],
                      stretch_dev);
    }
}



//----------------------------------------
// Clear out all items in the cancel safe queue
//----------------------------------------
void
ClearCancelSafeMsgQueue(stretch_dev_t *stretch_dev,
                        uint32_t       classId)
{
    int          ndx;
    int          start = classId;
    int          finish = classId;
    os_irp_t    *Irp;
    os_status_t  status = OS_STATUS_CANCELLED;

    if (classId == SCT_MSG_CLASS_ANY) {
        start = 0;
        finish = SCT_MSG_SEND_CONTEXT;
    }
    if (finish > SCT_MSG_SEND_CONTEXT)
        finish = SCT_MSG_SEND_CONTEXT;

    OS_SPINLOCK(&stretch_dev->MsgSpinLock);
    for (ndx = start; ndx <= finish;  ndx++) {
        Irp =
          OS_PENDQUEUE_RETREIVE_CONTEXT
            (&stretch_dev->CancelSafeMsgQueue,
             ndx,
             1);
        while (Irp != NULL) {
            CompleteRequest(Irp, &status);
            Irp =
              OS_PENDQUEUE_RETREIVE_CONTEXT
                (&stretch_dev->CancelSafeMsgQueue,
                 ndx,
                 1);
        }
    }
    OS_SPINUNLOCK(&stretch_dev->MsgSpinLock);
}



//----------------------------------------
// Get size of receive buffer
//----------------------------------------
void
GetReceivedSize(stretch_dev_t *stretch_dev,
                os_irp_t      *Irp)
{
    os_status_t status = OS_STATUS_FAILURE;

    uint32_t           *ptrPort;
    uint32_t            port;
    dma_channel_node_t *dma_channel;

    // Get the channel information from input DLL data
    ptrPort = (uint32_t *)Irp->system_buffer;
    if (ptrPort != NULL) {
        port = *ptrPort;
        dma_channel = getChannel(&stretch_dev->dma_channels,
                      port);

        if (dma_channel == NULL) {
            OS_PRINTK("Requested channel does not exist\n");
            status = OS_STATUS_SUCCESS;

            // Update and Complete the IRP
            *(uint32_t *)Irp->system_buffer = 0;
        } else {
            status = sizeof(uint32_t);

            // Update and Complete the IRP
            *(uint32_t *)Irp->system_buffer =
              dma_channel->lastReceivedSize;
        }
    }
    //IoReleaseRemoveLock(&pdx->RemoveLock, Irp);
    CompleteRequest(Irp, &status);
}

os_status_t
PCIBoot(stretch_dev_t * stretch_dev, os_irp_t * Irp)
{
    os_status_t status = OS_STATUS_FAILURE;
    volatile uint32_t __iomem * bootreg;
    uint32_t __iomem * bootddr;
    volatile uint32_t           __iomem *init_done;
    uint32_t                done = 0;
    int                     retry = 0;
    int                     i;

    if(stretch_dev->booted)
    {
        iOS_PRINTK("Already booted. %d\n",stretch_dev->booted);
        return OS_STATUS_SUCCESS;
    }
    do {
        sct_boot_info_t * bi = (sct_boot_info_t *)(Irp->system_buffer);
        OS_BZERO_MMIO(bi->result,BOOT_MEMORY_SIZE);
        OS_PRINTK("PCIBOOT requested for pointer %p, size %d\n",
                bi->data,
                bi->size);
        if ( bi->size > (16*1024*1024) )
        {
            status = OS_STATUS_IO_FAILURE;
            iOS_PRINTK("Failure: pci boot, buffer size %d is over the limit",
                    bi->size); 
            break;
        }
        memset(stretch_dev->boot_kern_addr,0,BOOT_MEMORY_SIZE);
        bootreg = PCI_REG_PTR32(stretch_dev->registers,
                SCT_BOOT_REG_OFFSET); 
        *bootreg = cpu_to_le32(OS_LO_DWORD(stretch_dev->boot_phy_addr));
        init_done = (volatile uint32_t *)(stretch_dev->boot_kern_addr);
        while (retry < LOOKUP_TIMEOUT_COUNT * 20) { 
            if ((done = *init_done) != 0) {           
                break; // out of the retry loop
            } else {
                // Delay 100ms
                *bootreg = cpu_to_le32(OS_LO_DWORD(stretch_dev->boot_phy_addr));
                OS_DELAY_MS(100, KernelMode);
                retry ++;
            }
        }
        done = le32_to_cpu(done);
        if ( !done )
        {
            status = OS_STATUS_IO_FAILURE;
            iOS_PRINTK("Failure: pci boot, first stage after %d retrys, %p, %x, %x\n",
                    retry, bootreg, *bootreg,  OS_LO_DWORD(stretch_dev->boot_phy_addr)); 
            break;
        }

        /* Map MMIO segment to load DVR Board's Boot Image */
        bootddr = ioremap_nocache(stretch_dev->pci_resources.bar_info[2].phy_addr + done,
                                  bi->size);
        //bootddr = PCI_REG_PTR32(stretch_dev->pci_resources.bar_info[2],done);
        if (bootddr) {
            if (OS_COPY_FROM_USER((void*)bootddr,bi->data,bi->size) != 0) {
                status = OS_STATUS_IO_FAILURE;
                iOS_PRINTK("Failure: pci boot, failed to copy %d bytes to %p(%x)\n",bi->size,bootddr,done);
                break;
            }
            /* Un-Map IO regardless */
            iounmap(bootddr);
        } else {
            status = OS_STATUS_IO_FAILURE;
            iOS_PRINTK("Failure: pci boot, failed to map %d bytes to %llx(%x)\n",
                       bi->size,
                       stretch_dev->pci_resources.bar_info[2].phy_addr,
                       done);
            break;
        }

        /*
        {
            uint32_t * testbuf = bootddr;
            int i=0;
            int offset = 0xde7d0;
            for (i=0;i<2;i++)
                iOS_PRINTK("%x: %8.8x %8.8x %8.8x %8.8x\n",offset+i*16, testbuf[offset/4+i*4],testbuf[offset/4+i*4+1],testbuf[offset/4+i*4+2],testbuf[offset/4+i*4+3]);
        }
        */
        *bootreg = 0;
        stretch_dev->booted = 1;
        bi->version = le32_to_cpu(init_done[0]);
        if ( !bi->bwait )
        {
            // no wait for return code, just return           
            status = OS_STATUS_SUCCESS;
            break;
        }
        while (retry < LOOKUP_TIMEOUT_COUNT * 10000) { 
            if ((done = *init_done) == 0) {           
                break; // out of the retry loop
            } else {
                // Delay 100ms
                OS_DELAY_MS(100, KernelMode);
                retry ++;
            }
        }
        if (done !=0)
        {
            status = OS_STATUS_IO_FAILURE;
            iOS_PRINTK("Failure: pci boot, second stage after %d retrys, %p, %x\n",
                    retry, bootddr, *bootddr); 
            break;
        }
        if (le32_to_cpu(init_done[SCT_MAX_PE+1]) == 0)
        {
            bi->pe_num = 4;
            for ( i = 0; i<4 ;i++)
            {
                bi->result[i] = le32_to_cpu(init_done[i+1]);
            }
        }
        else
        {
            for ( i = 0; i<SCT_MAX_PE ;i++)
            {
                bi->result[i] = le32_to_cpu(init_done[i+1]);
            }
            bi->pe_num = le32_to_cpu(init_done[SCT_MAX_PE+1]);
        }
        status = OS_STATUS_SUCCESS;
    } while (0);

    return status;
}

int
IoCancel(struct kiocb * iocb, struct io_event * e)
{
    //stretch_dev_t  *stretch_dev = (stretch_dev_t *)iocb->ki_filp->private_data;
    os_irp_t       *Irp = (os_irp_t *)iocb->private;
    int clear = 0;

    OS_PRINTK("cancel Irp, port %d\n", Irp->port);
    Irp->cancel = 1;
    while ( !clear )
    {
        clear = aio_put_req(iocb);
    }
    return 0;
}

os_status_t
GetBoardDetail(stretch_dev_t *stretch_dev,
             os_irp_t      *Irp)
{
    os_status_t status = OS_STATUS_SUCCESS;
    sct_board_detail_internal_t * bdi = 
            (sct_board_detail_internal_t *)(Irp->system_buffer);

    bdi->vendor_id = stretch_dev->pci_resources.vendor_id;
    bdi->device_id = stretch_dev->pci_resources.device_id;
    bdi->revision = stretch_dev->pci_resources.revision_id;
    bdi->subsystem_vendor = stretch_dev->pci_resources.subsystem_vendor_id;
    bdi->subsystem_id = stretch_dev->pci_resources.subsystem_id;
    bdi->board_slot_number = stretch_dev->slot_num;
    memcpy(bdi->serial_string,stretch_dev->serial_str,sizeof(stretch_dev->serial_str));
    bdi->serial_string[STRETCH_BOARD_SERIAL_LENGTH]=0;
    bdi->driver_version[0]=SCT_MAJOR_VERSION;
    bdi->driver_version[1]=SCT_MINOR_VERSION;
    bdi->driver_version[2]=SCT_MINUS_VERSION;
    bdi->driver_version[3]=SCT_BUILD_VERSION;
    bdi->buffer_size=stretch_dev->pci_resources.dma_buf_size;
    return status;
}

