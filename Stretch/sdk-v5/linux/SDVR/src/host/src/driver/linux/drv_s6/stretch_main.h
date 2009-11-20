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

#ifndef _STRETCH_MAIN_H
#define _STRETCH_MAIN_H


#include "os_arch.h"

#ifndef SCT_MSG_CLASS_COUNT
#define SCT_MSG_CLASS_COUNT 128
#endif

#ifndef SCT_MSG_CLASS_ANY
#define SCT_MSG_CLASS_ANY -1
#endif

/* Macros Definition */
#define MB(x) (x*1024*1024)
#define KB(x) (x*1024)

//----------------------------------------
// Queue definitions.
//----------------------------------------
// Calculate size of a structure, rounding up to nearest block of 32 bytes
#define BLOCK32_SIZEOF(n)       (32*(1+((sizeof(n)-1)>>5)))

#define QUEUE_SIZE              BLOCK32_SIZEOF(sx_queue_t)
#define MGMT_MSG_SIZE           BLOCK32_SIZEOF(sct_mgmt_msg_t)
#define MSG_SIZE                BLOCK32_SIZEOF(sct_msg_t)
#define QUEUE_COUNT             (2 * (1 +     /* MGMT queue */ \
                                      1 +     /* SCT_MSG return queue */ \
                                      SCT_MSG_CLASS_COUNT))
#define  IPC_STATUS_WORD_CNT    (SCT_QUEUE_STATUS_CNT * 2)

// Number of messages (for both Board and Host)
#define MGMT_MSG_COUNT          (2 * SCT_MGMT_MSG_COUNT)
#define MSG_COUNT               (2 * SCT_MSG_COUNT)

// All totalled together
#define SHARED_MEM_SIZE         (BLOCK32_SIZEOF(sx_queue_mod_rsc_t) + \
                                 (sizeof(uint32_t)  * IPC_STATUS_WORD_CNT) + \
                                 (QUEUE_SIZE        * QUEUE_COUNT) + \
                                 (MGMT_MSG_SIZE     * MGMT_MSG_COUNT) + \
                                 (MSG_SIZE          * MSG_COUNT) + \
                                 32) // for alignment?

#define BOOT_MEMORY_SIZE        1024
#define SCT_BOOT_REG_OFFSET   (0x000b0064)
#define STRETCH_BOARD_SERIAL_LENGTH 16

// For marking contexts in CancelSafeMsgQueue
#define SCT_MSG_SEND_CONTEXT    (SCT_MSG_CLASS_COUNT + 1)



//----------------------------------------
// The MAIN device structure for this driver
//----------------------------------------
struct stretch_dev_struct
{
    // Driver and Board Initialized?  (0=no,1=yes)
    int                                  initialized;
    //Booted?
    int                                  booted;

    // Device registration related things
    int                                  major;       // major device number
    int                                  minor;       // start of minor device number
    struct cdev                          cdev;

    // Stretch ISR Dispatch routine tasklet
    os_workpool_t                        dpc_workpool;

    // name of driver derived from PCI
    char                                 ifname[1024];

    // PCI board related info
    pci_resources_t                      pci_resources;

    // PCI boot memory
    void               __iomem *        boot_kern_addr; 
    os_physical_address_t               boot_phy_addr; 


    // The message pools
    message_pool_t                       mgmt_message_pool;
    message_pool_t                       regular_message_pool;

    // Messsaging Spinlock
    os_spinlock_t                        MsgSpinLock;

    // Spinlock for connection accepts
    os_spinlock_t                        accept_spinlock;

    // Message handling queues, pools, buffers
    //msg_queues_t                         msg_queues;

    // Messge handler thread (for mgmt and regular msgs)
    //msg_handler_t                        msg_handler;

    // Cancel-safe message queue and lock
    os_pendqueue_t                       CancelSafeMsgQueue;

    // Local header containing addressing info
    // for the register area
    dma_buf_node_t                       registers;

    // Data buffers also in shared memory.
    dma_buffers_t                        dma_buffers;

    // Dma channels pool
    dma_channels_t                       dma_channels;

    // Local header containing addressing info
    // for the queue structure (and a convenience pointer)
    dma_buf_node_t                       queues;

    // Remote (Board) header for queue structure
    sx_queue_mod_t                       queue_mod;

    // the base address of sx_queue status
    volatile uint32_t           __iomem *queue_status_base;

    // The queues themselves
    volatile sx_queue_mgmt         __iomem *outgoing_mgmt_queue;
                                         // was pQueue_mgmt_handle;

    volatile sx_queue_mgmt         __iomem *incoming_mgmt_queue;
                                         // was pQueue_in_mgmt_handle;

    volatile sx_queue_t         __iomem *incoming_message_queue[SCT_MSG_CLASS_COUNT];
                                         // was pQueue_in_msgQueue_handles[];

    volatile sx_queue_t         __iomem *incoming_message_return_queue;
                                         // was pQueue_in_return;

    volatile sx_queue_t         __iomem *outgoing_message_queue[SCT_MSG_CLASS_COUNT];
                                         // was pQueue_out_msgQueue_handles[];

    volatile sx_queue_t         __iomem *outgoing_message_return_queue;
                                         // was pQueue_out_return;
    char    serial_str[STRETCH_BOARD_SERIAL_LENGTH];
    int     slot_num;
};
typedef struct stretch_dev_struct stretch_dev_t;



//----------------------------------------
// Prototypes
//----------------------------------------

// Initiate a Request.  Creates an Irp which
// Follows the operation till the bitter end,
// At which point CompleteRequests is called
// and frees the Irp, if not Pended.
os_irp_t *
InitiateRequest(struct kiocb        *iocb, void * base, int64_t ctlcode);

// Complete a Request
void
CompleteRequest(os_irp_t    *Irp,
                os_status_t *status);

// Main FOPS open call
// Places stretch_dev in a more convenient place
int
sx_fops_open(struct inode *inode,
             struct file *file);

// Main FOPS close/release call
// Does a whole bunch of stuff.
int
sx_fops_release(struct inode *inode,
                struct file  *file);

// Main FOPS ioctl call
int
sx_fops_ioctl(struct inode *inode,
              struct file *file,
              unsigned int cmd,
              unsigned long arg);

// Asynchronous FOPS read call
#ifdef KVER_LESS_2620
ssize_t
sx_fops_async_read(struct kiocb*       iocb,
                   const char __user*  userbuf,
                   size_t              size,
                   loff_t              pos);
#else
ssize_t
sx_fops_async_read(struct kiocb*       iocb,
                   const struct iovec* iov,
                   unsigned long       nr_segs,
                   loff_t              pos);

#endif
// Main FOPS asynchronous read complete call
// This gets called by the callback after the PEND
ssize_t
sx_fops_async_read_complete(struct file *file,
                            char __user *buf,
                            size_t       size,
                            loff_t      *pos);

// Driver de-init. DeInit Driver and Board
// resources (PCI, etc.)
void
_stretch_finidev(struct pci_dev *pcidev,
                 int             cleanup_lev);



#endif // _STRETCH_MAIN_H

