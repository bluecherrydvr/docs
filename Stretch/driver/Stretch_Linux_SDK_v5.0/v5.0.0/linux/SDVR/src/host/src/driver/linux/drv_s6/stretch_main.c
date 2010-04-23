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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/uio.h>
#include <linux/fs.h>
#include <asm/io.h>

#include "os_arch.h"
#include "board_arch.h"
#include "ipcqueue.h"
#include "ipcqueue_api.h"
#include "dma_channels.h"
#include "ipcmsg.h"
#include "stretch_if.h"
#include "pci_resources.h"
#include "shared_memory.h"
#include "msg_pool.h"
//#include "msg_queues.h"
#include "dispatch.h"
#include "control.h"
#include "stretch_main.h"
#include "boardlist.c"

// Legal stuff
MODULE_LICENSE("GPL");

//global variable store where the shared memory starts
int debug_level;
module_param(debug_level,int,0644);

int dev_count = 0;
int dev_code = 0;

/* Linux PCI ID-Table */
struct pci_device_id stretch_pci_tbl[BOARD_LIST_COUNT] =  
{
    {
        .vendor    = STRETCH_PCI_VENDOR_ID, 
        .device    = 0x0002,
        .subvendor = STRETCH_PCI_SUB_VENDOR_ID,
        .subdevice = STRETCH_PCI_SUB_DEVICE_ID,
    },
};
MODULE_DEVICE_TABLE(pci, stretch_pci_tbl); /* exports data ID table */



//----------------------------------------
// Table defining File operations for a Linux
// character driver.
//----------------------------------------
#ifdef KVER_LESS_2617
static struct file_operations sx_fops =
#else
static const struct file_operations sx_fops =
#endif
{
    .aio_write  = sx_fops_async_read,
    .open       = sx_fops_open,
    .release    = sx_fops_release,
    .ioctl      = sx_fops_ioctl,
    .owner      = THIS_MODULE,
};


//----------------------------------------
// Initiate a Request.  Creates an Irp which
// Follows the operation till the bitter end,
// At which point CompleteRequests is called
// and frees the Irp, if not Pended.
// 'iov->iov_len' contains the IOCTL command with embedded size info
// Leave pos alone.
//----------------------------------------
os_irp_t *
InitiateRequest(struct kiocb *iocb, void * base, int64_t ctlcode)
{  
    os_irp_t    *Irp = NULL;
    int          cleanup_lev = 0;
    os_status_t  status = OS_STATUS_FAILURE;
    uint32_t     base_cmd;

    do {
        // We are assuming that we are getting only
        // one segment
        if (iocb != NULL) {
            if (iocb->private != NULL) {
                Irp = iocb->private;
                status = OS_STATUS_SUCCESS;
                break;
            }
        }
        // Allocate the Irp Area
        Irp = OS_KMALLOC(sizeof(os_irp_t),xx);
        if (Irp == NULL) {
            OS_PRINTK("FAILED to obtain Irp memory region\n");
            break;
        }
        // Everybody needs a private Irp.
        if (iocb != NULL) {
            iocb->private = Irp;
        }
        cleanup_lev++;

        // Allocate a space for non-dma buffer, if called for.
        Irp->pend_state = OS_PEND_STATE_BEGIN_IO;
        Irp->ioctl_cmd = ctlcode;
        Irp->user_buffer = base;
        Irp->system_buffer = NULL;
        Irp->dma_buf = NULL;
        Irp->dma_channel = NULL;
        Irp->cancel = 0;
        Irp->start_time = 0;
        Irp->msg_time = 0;
        Irp->dma_time = 0;

        base_cmd = _IOC_NR(Irp->ioctl_cmd);
        if ((base_cmd == SX_IOCODE_STORE_RECEIVE_BUFFER) ||
            (base_cmd == SX_IOCODE_BUFFER_SEND)) {
            // DMA buffer type access.  We leave the buffer
            // mgmt to lower logic.
            Irp->size = SX_DMA_IOCTL_GET_SIZE(ctlcode);
            Irp->port = SX_DMA_IOCTL_GET_PORT(Irp->ioctl_cmd);
            Irp->ioctl_cmd = base_cmd;
            Irp->iotype = SX_IOTYPE_DMA;
            Irp->iodir = (Irp->ioctl_cmd == SX_IOCTL_BUFFER_SEND) ?
                           _IOC_WRITE :
                           _IOC_READ;
        } else {
            // Normal type access.  Deal with getting user data in
            // and out here (and in completion).
            Irp->iodir = _IOC_DIR(Irp->ioctl_cmd);
            Irp->size = _IOC_SIZE(Irp->ioctl_cmd);
            Irp->iotype = _IOC_TYPE(Irp->ioctl_cmd);
            if (Irp->size > 0) {
                Irp->system_buffer = OS_KMALLOC(Irp->size,xx);
                if (Irp->system_buffer == NULL) {
                    OS_PRINTK("Could not allocate Ioctl buffer area\n");
                    break;
                }
                cleanup_lev++;

                // If a (non-dma) write, Copy that from user-space
                // to our special universe.  Dma is handled
                // by those who need it down below.
                if (Irp->iodir & _IOC_WRITE) {
                    if (OS_COPY_FROM_USER(Irp->system_buffer,
                                          Irp->user_buffer,
                                          Irp->size)) {
                        OS_PRINTK("Could not copy buffer area from user space\n");
                        break;
                    }
                }
            }
        }

        // Wrap it up
        Irp->iocb = iocb;

        status = OS_STATUS_SUCCESS;
        Irp->stat_size = status;
        xOS_PRINTK("Irp %p initiated\n", Irp);
    } while (0);

    if (status != OS_STATUS_SUCCESS) {
        // Cleanup.
        switch(cleanup_lev) {
        case 2:
            OS_KFREE(Irp->system_buffer);
        case 1:
            OS_KFREE(Irp);
            Irp = NULL;
        case 0:
        default:
            break;
        }
    }
    return Irp;
}



//----------------------------------------
// Complete a Request 
//----------------------------------------
void
CompleteRequest(os_irp_t    *Irp,
                os_status_t *status)
{
    int aioretval;

    int retval;
    if (status == NULL) {
        OS_PRINTK("Nowhere to put result code?\n");
        return;
    }

    if (Irp == NULL) {
        *status = OS_STATUS_INVALID_PTR;
        return;
    }

    if (Irp->cancel)
    {
        //cancelled Irp, only free up memory
        if (Irp->system_buffer)
        {
            OS_KFREE(Irp->system_buffer);
        }
        OS_KFREE(Irp);
        return;
    }

    switch (Irp->pend_state) {

    // Invocation: At the end of the first IO attempt
    case OS_PEND_STATE_BEGIN_IO:
        switch(*status) 
        {
          case OS_STATUS_PENDING:
            xOS_PRINTK("STATE: begin-->pending,%p\n",Irp);
            Irp->pend_state = OS_PEND_STATE_PENDING;
            break;

          case OS_STATUS_PENDING_COPYBACK:
            xOS_PRINTK("STATE: begin-->pending_copyback,%p\n",Irp);
            Irp->pend_state = OS_PEND_STATE_PENDING_COPYBACK;
            break;

          default:
            // Here we must return "0" aio subsystem
            // Otherwise it retries.  This for normal
            // non-pending operations.  aio_complete()
            // is where size info gets reported back up.
            // The AIO subsystem calls aio_complete() when
            // it sees "0" return value.  Perhaps in
            // the future we can set ki_retry to something
            // else  before the initial operaion so that
            // "aio_rw_vect_retry()" (within the AIO
            // subsystem) doesn't get invoked
            // allowing a non-zero return code to be given.
            xOS_PRINTK("STATE: begin-->done, %p\n",Irp);
            Irp->stat_size = *status;
            if (Irp->iocb != NULL) {
                Irp->iocb->ki_user_data = (uint64_t)Irp->stat_size;
            }
            *status = (*status < 0) ? *status : 0;
            Irp->pend_state = OS_PEND_STATE_DONE;
        }
        break;

    // Irq/dispatch: Pended but nothing to copy back
    case OS_PEND_STATE_PENDING:
        // Here we merely complete the AIO transaction,
        // which only notifies userspace io_getevents().
        // No copyback completion routine is invoked in
        // this case.
        xOS_PRINTK("STATE: pending-->done (status = %d) ,%p\n",
                  *status,Irp);
        Irp->pend_state = OS_PEND_STATE_DONE;
        *status = (*status < 0) ? *status : 0;
        Irp->iocb->ki_user_data = (uint64_t)*status;
        aioretval = aio_complete(Irp->iocb,
                                 *status,
                                 0);
        /*
        if ( Irp->start_time !=0)
        {
            struct timeval tv;
            long t ;
            long d1,d2,d3;
            do_gettimeofday(&tv);
            t = tv.tv_usec;
            d1=Irp->msg_time-Irp->start_time;
            d1 = (d1<0)?d1+1000000:d1;
            d2=Irp->dma_time-Irp->msg_time;
            d2 = (d2<0)?d2+1000000:d2;
            d3=t-Irp->dma_time;
            d3 = (d3<0)?d3+1000000:d3;
            iOS_PRINTK("msg=%ld,dma=%ld,finish=%ld\n",d1,d2,d3);
        }
        */
        xOS_PRINTK("aioretval = %d\n",aioretval);
        Irp->iocb = NULL;
        break;

    // Irq/dispatch: Pended with data to copy back (extra state)
    case OS_PEND_STATE_PENDING_COPYBACK:
        if (*status < OS_STATUS_SUCCESS) {
            // Complete, but don't run the completion routine.
            OS_PRINTK("STATE: pending_copyback-->done(failure) (status = %d) ,%p\n",
                      *status,Irp);
            Irp->pend_state = OS_PEND_STATE_DONE;
            aio_complete(Irp->iocb,
                         *status,
                         0);
            Irp->iocb = NULL;
        } else {
            // Here we kick AIO to call the Completion routine
            // given during the initiation routine.
            // No need to aio_complete() after kick_iocb().
            // Size info doesn't matter here. It will in the next phase.
            Irp->pend_state = OS_PEND_STATE_AWAITING_COMPLETION;
            *status = OS_STATUS_SUCCESS;
            kick_iocb(Irp->iocb);
            OS_PRINTK("STATE: pending_copyback-->awaiting_completion (status = %d),%p\n",
                      *status,status);
        }
        break;

    // When the retry/copyback operation completes...
    // Collect size info here too.  we DO want to return size info here,
    // because we have over-ridden aio_rw_vect_retry() with our
    // own (non-retrying) completion routine in iocb->ki_retry,
    // and we won't get retries.   Size info floats up though
    // AIOs internal aio_complete(..,res,..)
    case OS_PEND_STATE_AWAITING_COMPLETION:
        Irp->pend_state = OS_PEND_STATE_DONE;
        Irp->stat_size = *status;
        Irp->iocb->ki_user_data = (uint64_t)Irp->stat_size;
        OS_PRINTK("STATE: awaiting_completion-->done (status = %d),%p\n",
                  *status,Irp);
        break;

    default:
        *status = OS_STATUS_INVALID_PTR;
        xOS_PRINTK("Invalid asynchronous operation state (%d)\n",
                  Irp->pend_state);
    }

    if (Irp->pend_state == OS_PEND_STATE_DONE) {
        // Cleanup and copyback.  enter this stage from Callback
        // Read completion.
        // Only non DMA does copy to/from user here upon success
        // The buffers are mapped, not copied.  Handled beneath this.

        if ((Irp->iotype != SX_IOTYPE_DMA) &&
            (Irp->stat_size > OS_STATUS_SUCCESS)) {
            // Copy to user-space from our special universe.
            // Dma is handled by those who need it.
            if (Irp->iodir & _IOC_READ) {
                OS_PRINTK("copy %d bytes to user space %p\n",Irp->size,Irp->user_buffer);
                if ((retval = OS_COPY_TO_USER(Irp->user_buffer,
                                             Irp->system_buffer,
                                             Irp->size)) != 0) {
                    iOS_PRINTK("Could not copy buffer area to user space rv=%d\n",
                              retval);
                    *status = OS_STATUS_NO_RESOURCE;
                }
            }

            // Free the buffer and Irp we allocated.
            OS_KFREE(Irp->system_buffer);
        }
        OS_KFREE(Irp);
    }
}



//----------------------------------------
// Main FOPS open call
// Places stretch_dev in a more convenient place
//----------------------------------------
int
sx_fops_open(struct inode *inode,
             struct file *file)
{
    stretch_dev_t *stretch_dev;
    int minor = iminor(inode);
    int retcode = 0;

    stretch_dev = OS_CONTAINER_OF(inode->i_cdev,
                                  stretch_dev_t,
                                  cdev);
    if ( minor < dev_count)
    {
        file->private_data = stretch_dev;
    }
    else
    {
        retcode = -ENODEV;
    }
    return retcode;
}



//----------------------------------------
// Main FOPS close/release call
//----------------------------------------
int
sx_fops_release(struct inode *inode,
                struct file  *file)
{
    stretch_dev_t *stretch_dev;

    stretch_dev = OS_CONTAINER_OF(inode->i_cdev,
                                  stretch_dev_t,
                                  cdev);
    if (atomic_read(&(inode->i_count)) == 1)
    {
        //It is the last opened reference
        ClearCancelSafeMsgQueue(stretch_dev,SCT_MSG_CLASS_ANY);
        DestroyAllChannels(stretch_dev);
    }
    return 0;
}



//----------------------------------------
// Main FOPS ioctl call
// Nothing PENDs here.
//----------------------------------------
int
sx_fops_ioctl(struct inode *inode,
              struct file  *file,
              unsigned int  cmd,
              unsigned long arg)
{
    os_status_t      status = OS_STATUS_SUCCESS;
    os_irp_t        *Irp;
    dndev_t         *dndev;
    stretch_dev_t   *stretch_dev;
    uint32_t         ndx;
    int              iocode;
    
    do {
        // Allocate the (Pseudo) Io Request Packet, the "Irp".
        if ((Irp = InitiateRequest(NULL, (void *) arg, (int64_t)cmd)
                                   ) == NULL) {
            status = OS_STATUS_NO_MEMORY;
            break;
        }

        // Perform the ioctl action
        stretch_dev = (stretch_dev_t *)file->private_data;
        iocode = _IOC_NR(Irp->ioctl_cmd);
        switch(iocode) {

        case SX_IOCODE_READ_CONFIG:
            xOS_PRINTK("==> IOCTL_READ_CONFIG\n");
            dndev = (dndev_t *)Irp->system_buffer;
            dndev->vid = stretch_dev->pci_resources.vendor_id;
            dndev->did = stretch_dev->pci_resources.device_id;
            for (ndx = 0; ndx < SCT_MAX_PCI_BAR; ndx++) {
                dndev->bar[ndx].log_addr =
                  stretch_dev->pci_resources.bar_info[ndx].phy_addr;
                dndev->bar[ndx].mapped_addr =
                  stretch_dev->pci_resources.bar_info[ndx].kern_addr;
                dndev->bar[ndx].size =
                  stretch_dev->pci_resources.bar_info[ndx].size;
            }
            status = sizeof(dndev_t);
            CompleteRequest(Irp,
                            &status);
            break;

        case SX_IOCODE_IPC_QUEUE_INIT: /* none */
            xOS_PRINTK("==> IOCTL_IPC_QUEUE_INIT\n");
            pci_restore_state(stretch_dev->pci_resources.pcidev);
            status = InitIPCQueue(stretch_dev, Irp);
            break;

        // Replacement for aio_cancel call which wouldn't cooperate...
        case SX_IOCODE_AIO_CANCEL:
            {
                int8_t class = *(int8_t *)Irp->system_buffer;
                OS_PRINTK("==> IOCTL_AIO_CANCEL,class = %d\n",class);
                ClearCancelSafeMsgQueue(stretch_dev,class);
                CompleteRequest(Irp, &status);
            }
            break;

        case SX_IOCODE_RESET_S6: /* none */
            xOS_PRINTK("=========> RESET_BOARD\n");
            ResetBoard(stretch_dev);
            break;

        case SX_IOCODE_BOOT_FIRMWARE: /* input & output */
            xOS_PRINTK("=========> PCI_BOOT_BOARD\n");
            pci_restore_state(stretch_dev->pci_resources.pcidev);
            status = PCIBoot(stretch_dev, Irp);
            status = Irp->size;
            CompleteRequest(Irp,&status);
            break;

        case SX_IOCODE_GET_BOARD_DETAIL: /* output */
            xOS_PRINTK("=========> PCI_BOOT_BOARD\n");
            GetBoardDetail(stretch_dev,Irp);
            status = Irp->size; /* Set status to user's argument size */
            CompleteRequest(Irp,&status);
            break;

        default:
            xOS_PRINTK("INVALID IOCTL number called");
            status = OS_STATUS_INVALID_PTR;
            CompleteRequest(Irp, &status);
            break;
        }
        xOS_PRINTK("<==\n");
    } while (0);
    return status;
}



//----------------------------------------
// Main FOPS asynchronous read call
// Everything PENDs here.
//----------------------------------------
#ifdef KVER_LESS_2620
ssize_t
sx_fops_async_read(struct kiocb*       iocb,
                   const char __user*  userbuf,
                   size_t              size_arg,
                   loff_t              pos)
#else
ssize_t
sx_fops_async_read(struct kiocb*       iocb,
                   const struct iovec* iov,
                   unsigned long       nr_segs,
                   loff_t              pos)
#endif
{
    os_status_t      status = OS_STATUS_SUCCESS;
    os_irp_t        *Irp;
    stretch_dev_t   *stretch_dev;
    int              iocode;

    do {
        // Allocate the (Pseudo) Io Request Packet, the "Irp".
        // iov->size contains command,
        // pos(dont touch) contains user env.
        if ((Irp = InitiateRequest(iocb, (void *)iocb->ki_buf, iocb->ki_pos)
                                   ) == NULL) {
            status = OS_STATUS_NO_MEMORY;
            xOS_PRINTK(" Irp is null\n");
            break;
        }
        stretch_dev = (stretch_dev_t *)iocb->ki_filp->private_data;

        iocode = _IOC_NR(Irp->ioctl_cmd);
        switch(iocode) {
        case SX_IOCODE_STORE_RECEIVE_BUFFER:
            // Input:  offset=portval
            // Output: vectored buffer/size or PEND
            xOS_PRINTK("=========> STORE_RECEIVE_BUFFER\n");
            status = StoreReceiveBuffer(stretch_dev, Irp);
            break;

        case SX_IOCODE_BUFFER_SEND:
            // Input: offet=portval, vectored buffer/size
            xOS_PRINTK("=========> BUFFER_SEND\n");
            status = BufferSend(stretch_dev, Irp);
            break;

        case SX_IOCODE_WRITE_MSG:
            // Input: sct_dll_msg_t, offset=cmd
            xOS_PRINTK("=========> WRITE_MSG\n");
            status = WriteMsg(stretch_dev, Irp);
            break;

        case SX_IOCODE_READ_MSG:
            // Input:  sct_dll_msg_t, offset=cmd
            // Output: sct_dll_msg_t
            OS_PRINTK("=========> READ_MSG(initiate)\n");
            status = ReadMsg(stretch_dev, Irp);
            break;

        case SX_IOCODE_READ_MSG_POLL:
            // Input:  sct_dll_msg_t, offset=cmd
            // Output: sct_dll_msg_t
            xOS_PRINTK("=========> READ_MSG_POLL\n");
            status = ReadMsgPoll(stretch_dev, Irp);
            break;

        case SX_IOCODE_WRITE_MGMT_QUEUE:
            // Input: sct_dll_msg_t, offset=cmd
            xOS_PRINTK("=========> WRITE_MGMT_QUEUE\n");
            status = WriteMgmtQueue(stretch_dev, Irp);
            break;

        case SX_IOCODE_CHECK_CONNECT:
            // Input: sct_dll_msg_t, offset=cmd
            xOS_PRINTK("=========> CHECK_CONNECT\n");
            status = CheckConnect(stretch_dev, Irp);
            break;

        case SX_IOCODE_WAIT_DESTROY:
            // Input: sct_dll_msg_t, offset=cmd
            xOS_PRINTK("=========> WAIT_DESTROY\n");
            status = WaitDestroy(stretch_dev, Irp);
            break;

        case SX_IOCODE_WAIT_ERROR:
            // Input: sct_dll_msg_t, offset=cmd
            xOS_PRINTK("=========> WAIT_ERROR\n");
            status = WaitError(stretch_dev, Irp);
            break;

        case SX_IOCODE_DESTROY_RESPONSE:
            // Input: sct_dll_msg_t, offset=cmd
            xOS_PRINTK("=========> DESTROY_RESPONSE\n");
            status = DestroyResponse(stretch_dev, Irp);
            break;

        case SX_IOCODE_ACCEPT_RESPONSE:
            // Input: sct_dll_msg_t, offset=cmd
            xOS_PRINTK("=========> ACCEPT_RESPONSE\n");
            status = AcceptResponse(stretch_dev, Irp);
            break;
        default:
            xOS_PRINTK("INVALID Read/Write operation request\n");
            status = OS_STATUS_INVALID_PTR;
            CompleteRequest(Irp, &status);
            break;
        }
        xOS_PRINTK("<==\n");

    } while (0);
    return status;
}



//----------------------------------------
// Driver Init/Probe. Init Driver and Board
// resources (PCI, etc.)
//----------------------------------------
static
os_status_t
__devinit
stretch_initdev(struct pci_dev             *pcidev,
                const struct pci_device_id *id)
{
    stretch_dev_t       *stretch_dev;
    int                  cleanup_lev = 0;
    os_status_t          retval = OS_STATUS_SUCCESS;
    dev_t                dev;
    pci_resources_t     *pci_resources;
    dma_buf_node_t      *pool_da;
    board_list_node_t   *pd;

    do {
        //----------------------------------------
        // Allocate and set up main driver data area
        //----------------------------------------
        stretch_dev = (stretch_dev_t *)OS_KMALLOC(sizeof(stretch_dev_t),GFP_KERNEL);
        if (stretch_dev == NULL) {
            OS_PRINTK("Failure to allocate memory for driver data.\n");
            retval = OS_STATUS_NO_MEMORY;
            break;
        }
        stretch_dev->initialized = 0;
        stretch_dev->booted = 0;
        stretch_dev->slot_num = (int)(pcidev->bus->number);
        //IoInitializeRemoveLock(&pdx->RemoveLock, 0, 0, 0);

        OS_PRINTK("Allocated %lx bytes at %p for main driver data\n",
                  sizeof(stretch_dev_t),
                  stretch_dev);
        pci_set_drvdata(pcidev, stretch_dev);
        cleanup_lev++; // 1

        //----------------------------------------
        // Register the character device driver
        //----------------------------------------
        stretch_dev->major = dev_code; // for now...
        stretch_dev->minor = dev_count; // for now...
        if (stretch_dev->major) {
            OS_PRINTK("MAKEDEV(%d,%d)\n",stretch_dev->major,stretch_dev->minor);
            dev = MKDEV(stretch_dev->major,
                        stretch_dev->minor);
            retval = register_chrdev_region(dev,
                                            1,
                                            OS_DRIVERNAME);
        } else 
        {
            retval = alloc_chrdev_region(&dev,
                                         stretch_dev->minor,
                                         1,
                                         OS_DRIVERNAME);
            if (!retval) {
                stretch_dev->major = MAJOR(dev);
                stretch_dev->minor = MINOR(dev);
                dev_code = MAJOR(dev);
                dev_count = MINOR(dev);
                OS_PRINTK("first device, (%d,%d)\n",stretch_dev->major,stretch_dev->minor);
            }
            else
            {
                OS_PRINTK("alloc_chrdev_region returns %d\n",retval);
            }
        }
        if (retval < 0) {
            iOS_PRINTK("Can't get major %d\n",
                      stretch_dev->major);
            break;
        }
        cleanup_lev++; // 2

        cdev_init(&stretch_dev->cdev, //no de-init. (?)
                  &sx_fops);
        stretch_dev->cdev.owner = THIS_MODULE;
        stretch_dev->cdev.ops = &sx_fops;
        retval = cdev_add(&stretch_dev->cdev,
                          dev,
                          4);
        iOS_PRINTK("register stretch_dev %p\n",stretch_dev);
        if (retval) {
            OS_PRINTK("Failure registering character driver.\n");
            break;
        }
        cleanup_lev++; // 3

        //----------------------------------------
        // Hook up PCI device data to our local data
        // pci_set_drvdata() usually gets called
        // at the end of init, but it is needed
        // before that, so it is being done now.
        //----------------------------------------
        pci_resources = &stretch_dev->pci_resources;
        pci_resources->pcidev = pcidev;

        pd = (board_list_node_t *)(&stretch_board_list[id->driver_data]);
        /* Determine Board Memory Usage */
        pci_resources->max_dma_send_buff =  pd->send_chans;
        pci_resources->max_dma_recv_buff = pd->chans_per_dev * pd->bufs_per_chan * pd->devs_onboard;
        pci_resources->max_dma_buff      = (pci_resources->max_dma_recv_buff + 
                                                pci_resources->max_dma_send_buff);
        pci_resources->dma_buf_size = pd->buf_size;
        pci_resources->mem_reserve_size  = (uint32_t)(pci_resources->max_dma_buff * 
                                                          pci_resources->dma_buf_size + MB(1));
        //----------------------------------------
        // Detect PCI-oriented resources used by driver and hook
        // into main driver data structure.  Map the
        // Memory too.
        //----------------------------------------
        retval = pci_resources_init(pci_resources);
        if (os_is_failure(retval)) {
            break;
        }
        cleanup_lev++; // 4

        //----------------------------------------
        // Set up Shared memory resources
        //----------------------------------------
        pool_da = (dma_buf_node_t *)OS_KMALLOC(sizeof(dma_buf_node_t) * pci_resources->max_dma_buff,
                                                   GFP_KERNEL);
        if (pool_da == NULL) {
            OS_PRINTK("Failure to allocate memory for dma_buf_node_t pool.\n");
            retval = OS_STATUS_NO_MEMORY;
            break;
        }
        cleanup_lev++; // 5

        stretch_dev->dma_buffers.pool = pool_da;
        retval = shared_memory_init(stretch_dev);
        if (os_is_failure(retval)) {
            break;
        }
        cleanup_lev++; // 6

        //----------------------------------------
        // Set up Lists, structures, data, misc, etc.
        //----------------------------------------
        InitializeDmaChannels(&stretch_dev->dma_channels);
        InitializeDispatch(stretch_dev);
        //initMsgHandlerThread(&stretch_dev->msg_handler);
        //initMessageHandlingQueue(&stretch_dev->msg_queues);

        OS_SPINLOCK_INIT(&stretch_dev->MsgSpinLock);
        OS_SPINLOCK_INIT(&stretch_dev->accept_spinlock);
        OS_PENDQUEUE_INIT(&stretch_dev->CancelSafeMsgQueue);

        //----------------------------------------
        // Get the Interrupt
        //----------------------------------------
        retval = request_irq(pcidev->irq,
                             stretch_isr,
#ifdef KVER_LESS_2617
                             SA_SHIRQ | SA_INTERRUPT,
#else
                             IRQF_SHARED | IRQF_DISABLED,
#endif
                             stretch_dev->ifname,
                             stretch_dev);
        if (os_is_failure(retval)) {
            OS_PRINTK("Can't get IRQ %d\n",
                      pcidev->irq);
            break;
        }
        cleanup_lev++; // 7
        pci_save_state(stretch_dev->pci_resources.pcidev);

        // Test read of scratch register
        OS_PRINTK("%8.8x\n",
                   *PCI_REG_PTR32(stretch_dev->registers, SCT_SHARED_MEM_REG_OFFSET));
        
        // Write the physical address to the boot register to
        // fire off the Stretch board's INIT function
        /*
        *PCI_REG_PTR32(stretch_dev->registers, SCT_BOOT_REG_OFFSET) =
          OS_LO_DWORD(stretch_dev->boot_phy_addr);

        OS_PRINTK("set offset register (offs=%8.8x) to %8.8x\n",
                  SCT_BOOT_REG_OFFSET,
                  OS_LO_DWORD(stretch_dev->boot_phy_addr));
        */                  
                          
    } while(0);

    if (retval != OS_STATUS_SUCCESS) {
        _stretch_finidev(pcidev,
                         cleanup_lev);
    }
    else
    {
        dev_count++;
    }
    return retval;
}



//----------------------------------------
// Driver de-init. DeInit Driver and Board
// resources (PCI, etc.)
//----------------------------------------
//(internal form)
void
_stretch_finidev(struct pci_dev *pcidev,
                 int             cleanup_lev)
{
    stretch_dev_t *stretch_dev= pci_get_drvdata(pcidev);
    dma_buf_node_t *pool_da= stretch_dev->dma_buffers.pool;

    if( !stretch_dev )
        return;
    switch(cleanup_lev) {
    case CLEANUP_ALL:
    case 7:
        pci_restore_state(pcidev);
        pci_set_drvdata(pcidev, NULL);
        free_irq(pcidev->irq, stretch_dev);
        //releaseMsgHandlerThread(&stretch_dev->msg_handler);
        //releaseMessageHandlingQueue(stretch_dev);
        DeInitializeDispatch(stretch_dev);
    case 6:
        shared_memory_fini(stretch_dev,
                           CLEANUP_ALL);
    case 5:
        OS_KFREE(pool_da);
    case 4:
        pci_resources_fini(&stretch_dev->pci_resources,
                           CLEANUP_ALL);
    case 3:
        cdev_del(&stretch_dev->cdev);
    case 2:
        unregister_chrdev_region(MKDEV(stretch_dev->major,
                                       stretch_dev->minor),
                                 1);
    case 1:
        OS_KFREE(stretch_dev);
    }
}

// (external form)
static void
__devexit
stretch_finidev(struct pci_dev *pcidev)
{
    _stretch_finidev(pcidev,
                     CLEANUP_ALL);
}

/*some pci recover debug code*/
#if 0
static pci_ers_result_t stretch_pci_error_detected(struct pci_dev *pdev,
           pci_channel_state_t state)
{
    iOS_PRINTK("pci error %d\n",state);
    return PCI_ERS_RESULT_NEED_RESET;
}

static pci_ers_result_t stretch_pci_slot_reset(struct pci_dev *pdev)
{
    iOS_PRINTK("pci reset\n");
    return PCI_ERS_RESULT_RECOVERED;
}

static pci_ers_result_t stretch_pci_link_reset(struct pci_dev *pdev)
{
    iOS_PRINTK("pci link reset\n");
    return PCI_ERS_RESULT_NEED_RESET;
}
static pci_ers_result_t stretch_pci_resume(struct pci_dev *pdev)
{
    iOS_PRINTK("pci resume\n");
    return PCI_ERS_RESULT_NEED_RESET;
}

static struct pci_error_handlers pci_err_handler = {
            .error_detected = stretch_pci_error_detected,
            .slot_reset = stretch_pci_slot_reset,
            .link_reset = stretch_pci_link_reset,
            .resume = stretch_pci_resume,
    };
#endif
//----------------------------------------
// Table to give to pci_driver device
//----------------------------------------
static
struct pci_driver
stretch_pci_driver = 
{
    .name     = OS_DRIVERNAME,
    .id_table = stretch_pci_tbl,
    .probe    = stretch_initdev,
    .remove   = __devexit_p(stretch_finidev),
    //.err_handler = &pci_err_handler
};



//----------------------------------------
// Driver De-init.  Runs at unload time (rmmod)
//----------------------------------------
static void
stretch_fini(void)
{
    pci_unregister_driver(&stretch_pci_driver);

    iOS_PRINTK("Goodbye, s6stretch\n");
}



//----------------------------------------
// Driver init.  Runs at load time (insmod)
//----------------------------------------
static int
stretch_init(void)
{
    os_status_t status;
    int         ii = 0;

    /* extract board support list into pci_id table */
    do {
        stretch_pci_tbl[ii].vendor      = stretch_board_list[ii].vendor_id;
        stretch_pci_tbl[ii].device      = stretch_board_list[ii].device_id;
        stretch_pci_tbl[ii].subvendor   = stretch_board_list[ii].sub_vendor_id;
        stretch_pci_tbl[ii].subdevice   = stretch_board_list[ii].sub_device_id;
        stretch_pci_tbl[ii].driver_data = (kernel_ulong_t)ii;
    } while (++ii < BOARD_LIST_COUNT); 
    
    iOS_PRINTK("Hello, s6stretch\n");

    status = pci_register_driver(&stretch_pci_driver);
    if (os_is_failure(status)) {
        iOS_PRINTK("FAILURE to register PCI driver, rc = %d", status); 
        return status;
    }
    return OS_STATUS_SUCCESS;
}



module_init(stretch_init);
module_exit(stretch_fini);
