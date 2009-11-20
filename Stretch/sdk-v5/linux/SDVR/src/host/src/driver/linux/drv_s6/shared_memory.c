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

//
// This file handles all registration/allocation and distribution of
// the shared memory segments for registers, queues, and buffers.
// As of the creation of this driver the S6 board was setting
// up resources as follows (from a 64bit Ubuntu 7.10 machine)
//
//    Vendor ID : 18a2
//    Device ID : 0002
//    Command   : 0007
//    Status    : 0010
//    SubVendID : 18a2
//    SubDevID  : 0002
//    Irq Line  : 0b
//    Revision  : 01
//    Class     : 00040000
//    BAR0      : 00000000fe600000 size : 00200000
//    BAR1      : 0000000000000000 size : 00000000
//    BAR2      : 00000000e8000000 size : 00200000
//    BAR3      : 0000000000000000 size : 00000000
//    BAR4      : 00000000e0000000 size : 08000000
//    BAR5      : 0000000000000000 size : 00000000
//
// This code will set up  the registers in BAR0,
// The queues will be set up in BAR2, and the
// buffers in BAR4
//


//     
// For the buffers, a simple heap is implemented here.
//
#include <linux/interrupt.h>

#include "stretch_if.h"
#include "os_arch.h"
#include "board_arch.h"
#include "ipcqueue.h"
#include "ipcqueue_api.h"
#include "ipcmsg.h"
#include "pci_resources.h"
#include "dma_channels.h"
#include "shared_memory.h"
#include "msg_pool.h"
#include "dispatch.h"
#include "msg_queues.h"
#include "stretch_main.h"

typedef unsigned long uint32;	
static int len = 0;

//static unsigned int shmem_end = 0;
//========================================
// Init:Map shared memory, reset heap pointer
//========================================
os_status_t
shared_memory_init(stretch_dev_t *stretch_dev)
{
    int              ndx;
    int              cleanup_lev = 0;
    os_status_t      retval = OS_STATUS_SUCCESS;
    pci_resources_t *pci_resources = &stretch_dev->pci_resources;
    dma_buffers_t   *dma_buffers = &stretch_dev->dma_buffers;
    //void  * dmabuf_kern_base;
    //os_physical_address_t dmabuf_phy_base;
    //struct sysinfo sysi;
    //unsigned int    shmem_addr=0;

	do {
		//----------------------------------------
		// Initialize the register area (in 1st 2M region)
		//----------------------------------------
		pci_resources->bar_info[0].kern_addr =
			ioremap_nocache(pci_resources->bar_info[0].phy_addr, 
					pci_resources->bar_info[0].size);
		if (pci_resources->bar_info[0].kern_addr == NULL) {
			iOS_PRINTK("Can't ioremap() MMIO memory for register space\n");
			retval = OS_STATUS_NO_MEMORY;
			break;
		}
		OS_PRINTK("Mapped register space at: %p (phy=%p), size: %8.8lx\n",
				pci_resources->bar_info[0].kern_addr,
				(void *)pci_resources->bar_info[0].phy_addr,
				pci_resources->bar_info[0].size);
		cleanup_lev++; // 1

		// Map this segment but don't use it.
		pci_resources->bar_info[1].kern_addr =
			ioremap_nocache(pci_resources->bar_info[1].phy_addr,
					pci_resources->bar_info[1].size);
		if (pci_resources->bar_info[1].kern_addr == NULL) {
			iOS_PRINTK("Can't ioremap() MMIO memory for queues\n");
			retval = OS_STATUS_NO_MEMORY;
			break;
		}
		OS_PRINTK("Mapped data ram space at: %p (phy=%p), size: %8.8lx\n",
				pci_resources->bar_info[1].kern_addr,
				(void *)pci_resources->bar_info[1].phy_addr,
				pci_resources->bar_info[1].size);
		cleanup_lev++; // 2

		cleanup_lev++; // 3

		//----------------------------------------
		// Initialize the register area (in 1st 2M region)
		// Register access is via BAR[0]
		//----------------------------------------
		// Use this segment as-is.
		stretch_dev->registers.size      = pci_resources->bar_info[0].size;
		stretch_dev->registers.phy_addr  = pci_resources->bar_info[0].phy_addr;
		stretch_dev->registers.kern_addr = pci_resources->bar_info[0].kern_addr;

		//----------------------------------------
		// Initialize the queues (in 2nd 2M region)
		// NOT via Bar[1], but in DMA memory.
		//----------------------------------------
		stretch_dev->queues.size = SHARED_MEM_SIZE;
		stretch_dev->queues.kern_addr = 
			pci_alloc_consistent(pci_resources->pcidev,
					SHARED_MEM_SIZE,
					&stretch_dev->queues.phy_addr);
		OS_PRINTK("pci_queue memory=%p\n",stretch_dev->queues.kern_addr);
		if (stretch_dev->queues.kern_addr == NULL) {
			iOS_PRINTK("iomap for msg queues failed\n");
			//iOS_PRINTK("Can't dma_alloc_coherent() MMIO memory for queues\n");
			retval = OS_STATUS_NO_MEMORY;
			break;
		}
		OS_BZERO_MMIO(stretch_dev->queues.kern_addr,
				SHARED_MEM_SIZE);

		OS_PRINTK("Using space at: %p (phy=%p), size: %8.8lx for queues, cl=%d\n",
				stretch_dev->queues.kern_addr,
				(void *)stretch_dev->queues.phy_addr,
				stretch_dev->queues.size,
				cleanup_lev);

		// Set up IPC queue header in beginning of
		// shared memory region.  Indicate that
		// init is NOT done (yet).
		stretch_dev->queue_mod.queue_mod_rsc =
			(sx_queue_mod_rsc_t *)stretch_dev->queues.kern_addr;
		stretch_dev->queue_mod.queue_mod_rsc->init_done = 0;
		cleanup_lev++; // 4

		//allocate pci boot memory
		stretch_dev->boot_kern_addr = 
			pci_alloc_consistent(pci_resources->pcidev,
					BOOT_MEMORY_SIZE,
					&stretch_dev->boot_phy_addr);

		if (stretch_dev->boot_kern_addr == NULL) {
			iOS_PRINTK("Can't dma_alloc_coherent() MMIO memory for boot memory\n");
			retval = OS_STATUS_NO_MEMORY;
			break;
		}
		cleanup_lev++;//5

		for (ndx = 0; ndx < (uint32_t)pci_resources->max_dma_buff; ndx++) {
			dma_buffers->pool[ndx].kern_addr = 0;
		}

		// The free list:  Holds dma_buf_node_t type nodes.
		OS_LIST_INIT(&dma_buffers->buf_free_list);
		OS_SPINLOCK_INIT(&dma_buffers->buf_free_spinlock);

		// The pending list: Holds dma_channel_t nodes.
		OS_LIST_INIT(&dma_buffers->chan_pending_list);
		OS_SPINLOCK_INIT(&dma_buffers->chan_pending_spinlock);
		{
#define MAX_ALLOC_ATTEMPS	768
			unsigned long	allocs[MAX_ALLOC_ATTEMPS];
			int	count[16];
			int i;
			int	base = 0;
			int order = get_order(stretch_dev->pci_resources.dma_buf_size);
			int	failed = 0;

			OS_PRINTK("order is %d\n",order);
			for(i=0;i<16;i++)
			{
				count[i] = 0;
			}
			for(i=0;i< MAX_ALLOC_ATTEMPS ;i++)
			{
				unsigned long phy_addr;
				allocs[i] = __get_free_pages(GFP_KERNEL,order);
				if ( allocs[i] == 0)
				{
					iOS_PRINTK("get_free_pages failed\n");
					failed = 1;
					break;
				}
				phy_addr = __pa(allocs[i]);
				base = phy_addr / (256*1024*1024);
				if ( (base < 16) && base >=0)
				{
					count[base]++;
					if (count[base] >= pci_resources->max_dma_buff)
						break;
				}
				base--;
				if ( (base < 16) && base >=0)
				{
					count[base]++;
					if (count[base] >= pci_resources->max_dma_buff)
						break;
				}
				base--;
				if ( (base < 16) && base >=0)
				{
					count[base]++;
					if (count[base] >= pci_resources->max_dma_buff)
						break;
				}
			}
			if (i <MAX_ALLOC_ATTEMPS  && !failed)
			{
				int j;
				int index = 0;
				for (j=0;j<=i;j++)
				{
					int cb = __pa(allocs[j])/(256*1024*1024);
					if ( (cb >=base) && (cb <= base+2) && (index < pci_resources->max_dma_buff) )
					{
						dma_buffers->pool[index].kern_addr = (void *)allocs[j];
						dma_buffers->pool[index].phy_addr = __pa(allocs[j]);
						dma_buffers->pool[index].size = pci_resources->dma_buf_size;
						OS_LIST_ADD_TAIL(&(dma_buffers->pool[index].list_node),
								&(dma_buffers->buf_free_list));
						index ++;
					}
					else
					{
						free_pages(allocs[j],order);
					}
				}
			}
			else
			{
				int j;
                iOS_PRINTK("dma buffer allocation failed\n");
				for (j=0;j<i;j++)
				{
					free_pages(allocs[j],order);
				}
				retval = OS_STATUS_NO_MEMORY;
				break;
			}
		}
		cleanup_lev++;//6
	} while (0);
    if (retval != OS_STATUS_SUCCESS) {
        shared_memory_fini(stretch_dev,
                           cleanup_lev);
    }
	else
    {
        volatile uint32_t __iomem * preg;
		uint32_t * ps = (uint32_t*)stretch_dev->serial_str;
		int j;
        preg = PCI_REG_PTR32(stretch_dev->registers,
                       SCT_SERIAL_REG_OFFSET);
		for(j=0;j<STRETCH_BOARD_SERIAL_LENGTH/4;j++)
		{
			ps[j] = preg[j];
		}
    }

    return retval;
}



//========================================
// De-init:Unmap shared memory
//========================================
void
shared_memory_fini(stretch_dev_t *stretch_dev,
                   int            cleanup_lev)
{
    int              ndx = 0;
    int              startlev = 6;
    pci_resources_t *pci_resources = &stretch_dev->pci_resources;
    dma_buffers_t   *dma_buffers = &stretch_dev->dma_buffers;

    if (cleanup_lev == CLEANUP_ALL) {
        cleanup_lev = startlev;
    }
    //clean up dma buffer
    if (cleanup_lev >= 6)
    {
	   int order = get_order(stretch_dev->pci_resources.dma_buf_size);
       for (ndx = (uint32_t)pci_resources->max_dma_buff-1; ndx >= 0; ndx--) 
	   {
           if (dma_buffers->pool[ndx].kern_addr)
		   {
			   free_pages((unsigned long)(dma_buffers->pool[ndx].kern_addr),order);
		   }
	   }
    }
    // Clean up boot memory
    if (cleanup_lev >=5)
    {
        pci_free_consistent(pci_resources->pcidev,
                            BOOT_MEMORY_SIZE,
                            stretch_dev->boot_kern_addr,
                            stretch_dev->boot_phy_addr);
    }
    // Clean up the area used by the queues
	if (cleanup_lev >= 4) {
		pci_free_consistent(pci_resources->pcidev,
				stretch_dev->queues.size,
				stretch_dev->queues.kern_addr,
				stretch_dev->queues.phy_addr);
	}

    // Free the shared memory queues
    stretch_dev->queue_mod.queue_mod_rsc = NULL;
    if (cleanup_lev >= 2) {
        iounmap(pci_resources->bar_info[1].kern_addr);
    }

    // Free the shared memory register space
    if (cleanup_lev >= 1) {
        iounmap(pci_resources->bar_info[0].kern_addr);
    }
}



//========================================
// Remove from free-list and return a
// useable DMA buffer
//========================================
dma_buf_node_t *
getDmaBuffer(dma_buffers_t *dma_buffers)
{
    os_list_node_t *temp_node;
    dma_buf_node_t *dma_buf_node = NULL;

    // Place element removed from the free element in dma_buf;
    temp_node = OS_LIST_RETREIVE_HEAD(&(dma_buffers->buf_free_list), 1);
    xOS_PRINTK("Getting DMA buffer %p\n",temp_node);
    if (temp_node != NULL) {
        dma_buf_node = OS_CONTAINER_OF(temp_node,
                                       dma_buf_node_t,
                                       list_node);
    }

    xOS_PRINTK("Got DMA buffer %p\n",dma_buf_node);
    return (temp_node != NULL) ?
             dma_buf_node :
             NULL;
}



//========================================
// Give back a DMA buffer and place back
// into free list
//========================================
void
freeDmaBuffer(stretch_dev_t      *stretch_dev,
              dma_buf_node_t     *dma_buf_node,
              dma_channel_node_t *no_use)
{
    //msg_queues_t        *msg_queues  = &stretch_dev->msg_queues;
    dma_buffers_t       *dma_buffers = &stretch_dev->dma_buffers;
    os_list_node_t      *temp_node;
    dma_channel_node_t  *this_dma_channel;

    dma_buf_node->size = 0;
    dma_buf_node->irp = NULL;

    xOS_PRINTK("Freeing DMA buffer %p\n",dma_buf_node);

    // Is there something in the pending list? (of type dma_channel_node_t)
    // chan_node gets the head of the pending list, which is removed
    // and placed in the request queue
    temp_node = OS_LIST_RETREIVE_HEAD(&(dma_buffers->chan_pending_list), 1);

    if (temp_node != NULL) {
        os_irp_t * Irp;
        len--;

        this_dma_channel =
            OS_CONTAINER_OF(temp_node,
                    dma_channel_node_t,
                    list_node);

        OS_SPINLOCK(&this_dma_channel->spinlock);
        this_dma_channel->pending_count--;
        if (this_dma_channel->pending_count > 0)
        {
            OS_LIST_ADD_TAIL(&this_dma_channel->list_node,
                    &dma_buffers->chan_pending_list);
        }
        OS_SPINUNLOCK(&this_dma_channel->spinlock);
        Irp = OS_PENDQUEUE_RETREIVE_NEXT
            (&this_dma_channel->CancelSafeBuffersQueue,1);
        OS_PRINTK("channel %d removed in pending queue, pending count %d, Irp %p\n",
                this_dma_channel->port,
                this_dma_channel->pending_count,
                Irp);        
        if ((Irp != NULL) && (!Irp->cancel))
        {
            os_status_t                          status;
            status = OS_STATUS_SUCCESS;
            Irp->dma_buf = dma_buf_node;
            CompleteRequest(Irp, &status);  //this will invoke kick_iocb() which will call ki_retry
        }
        else
        {
            OS_PRINTK("NO IRP found\n");
            OS_LIST_ADD_TAIL(&(dma_buf_node->list_node),
                    &(dma_buffers->buf_free_list));
        }
    }
    else
    {
        // Add dma_buf_node_t back to free list
        OS_LIST_ADD_TAIL(&(dma_buf_node->list_node),
                     &(dma_buffers->buf_free_list));
    }
}



//========================================
// Add a DMA buffer to the pending list
//========================================
void
appendPendingChannel(dma_channel_node_t *dma_channel,
                     dma_buffers_t      *dma_buffers)
{
    OS_SPINLOCK(&dma_channel->spinlock);
    if ( dma_channel->pending_count==0)
    {
        // Add to pending list
        OS_LIST_ADD_TAIL(&dma_channel->list_node,
                     &dma_buffers->chan_pending_list);
    }
    dma_channel->pending_count ++;
    OS_SPINUNLOCK(&dma_channel->spinlock);
    len ++;

    OS_PRINTK("channel %d added in pending queue.pending_count=%d\n",
              dma_channel->port,dma_channel->pending_count);
}


//========================================
// Clean out all pending items in given channel.
//========================================
void
cleanPendingChannel(dma_buffers_t      *dma_buffers,
                    dma_channel_node_t *dma_channel)
{
    os_list_node_t     *temp_node;
    dma_channel_node_t *this_dma_channel;

    OS_LIST_ITER_INIT(&dma_buffers->chan_pending_list);

    while ((temp_node = OS_LIST_ITER_NEXT) != NULL) {
        this_dma_channel = OS_CONTAINER_OF(temp_node,
                                           dma_channel_node_t,
                                           list_node);
        if (this_dma_channel == dma_channel ) {
            OS_LIST_REMOVE_ELEMENT(temp_node,&dma_buffers->chan_pending_list);
        }       
    }
    OS_LIST_ITER_CLEANUP;
}

//========================================
// Go through pending list and search for
// one that is marked pending for the given channel
//========================================
int
findPendingChannel(dma_buffers_t      *dma_buffers,
                   dma_channel_node_t *dma_channel)
{
    os_list_node_t     *temp_node;
    dma_channel_node_t *this_dma_channel;
    int                 retval = 0;

    OS_LIST_ITER_INIT(&dma_buffers->chan_pending_list);
    while ((temp_node = OS_LIST_ITER_NEXT) != NULL) {
        this_dma_channel = OS_CONTAINER_OF(temp_node, 
                                           dma_channel_node_t,
                                           list_node);
        if (this_dma_channel == dma_channel) {
            retval = 1;
            break;
        }       
    }
    OS_LIST_ITER_CLEANUP;
    return retval;
}



//========================================
// Find Dma buffer by using Physical address
// Searches through the pool itself no
// locking worries.
//========================================
dma_buf_node_t *
findDmaBufferByPhy(dma_buffers_t *dma_buffers,
                   uint32_t       addr,
                   uint32_t       addr_hi,
                   uint32_t       max_dma_buffers)
{
    int ndx;
    dma_buf_node_t *dma_buf = NULL;

    for (ndx = 0; ndx < max_dma_buffers; ndx++) {

        if ((OS_LO_DWORD(dma_buffers->pool[ndx].phy_addr) == addr) &&
            (OS_HI_DWORD(dma_buffers->pool[ndx].phy_addr) == addr_hi)) {
            dma_buf = &dma_buffers->pool[ndx];
            break;
        }
    }
    return dma_buf;
}



//========================================
// get Dma Buffer address limits
//========================================
void
getDmaBufAddrLimit(dma_buffers_t *dma_buffers,
                   uint32_t      *p_low_addr,
                   uint32_t      *p_low_addr_hi,
                   uint32_t      *p_hi_addr,
                   uint32_t      *p_hi_addr_hi,
                   uint32_t       max_dma_buffers,
                   uint32_t      dma_buf_size)
{
    int ndx;
    uint32_t l,lh,h,hh;

    l =  OS_LO_DWORD(dma_buffers->pool[0].phy_addr);
    lh = OS_HI_DWORD(dma_buffers->pool[0].phy_addr);
    h = l + dma_buf_size - 1 ;
    hh = lh;
    for (ndx = 1; ndx < max_dma_buffers; ndx++) {
        if (l > OS_LO_DWORD(dma_buffers->pool[ndx].phy_addr)) {
            l = OS_LO_DWORD(dma_buffers->pool[ndx].phy_addr);
        }

        if (h < (OS_HI_DWORD(dma_buffers->pool[ndx].phy_addr) +
                 dma_buf_size - 1)) {
            h = OS_HI_DWORD(dma_buffers->pool[ndx].phy_addr) +
                 dma_buf_size - 1;
        }   
    }
    *p_low_addr = l;
    *p_low_addr_hi = lh;
    *p_hi_addr = h;
    *p_hi_addr_hi = hh;
}
