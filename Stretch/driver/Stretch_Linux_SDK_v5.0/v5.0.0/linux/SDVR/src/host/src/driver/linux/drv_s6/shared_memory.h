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

#ifndef _SHARED_MEMORY_H
#define _SHARED_MEMORY_H



//========================================
// Dma buffer list node
//========================================
#define SCT_SERIAL_REG_OFFSET (0x000b0070)

struct dma_buf_node_struct
{
    os_list_node_t              list_node; // was m_list;
    void               __iomem *kern_addr; // was m_buf;
    os_physical_address_t       phy_addr;  // was m_phy;
    os_size_t                   size;      // was int m_size;
    os_irp_t                   *irp;

};
typedef struct dma_buf_node_struct dma_buf_node_t;



//========================================
// The DMA pool, free and pending lists
//========================================
struct dma_buffers_struct {
    // Pool of buffers
    dma_buf_node_t  * pool;

    // Free list and lock:  Holds dma_buf_node_t nodes
    os_list_t       buf_free_list;
    os_spinlock_t   buf_free_spinlock;

    // Pending list and lock: Holds channel_node_t nodes
    os_list_t       chan_pending_list;
    os_spinlock_t   chan_pending_spinlock;

};
typedef struct dma_buffers_struct dma_buffers_t;



//========================================
// Prototypes
//========================================

// Init:Map shared memory, reset heap pointer
struct stretch_dev_struct;
os_status_t
shared_memory_init(struct stretch_dev_struct *stretch_dev);

// De-init:Unmap shared memory
void
shared_memory_fini(struct stretch_dev_struct *stretch_dev,
                   int                       cleanup_lev);

// Remove from free-list and return a
// useable DMA buffer
dma_buf_node_t *
getDmaBuffer(dma_buffers_t *dma_buffers);

// Give back a DMA buffer and place back
// into free list
void
freeDmaBuffer(struct stretch_dev_struct *stretch_dev,
              dma_buf_node_t            *dma_buf_node,
              dma_channel_node_t        *dma_channel);

// Add a Dma channel to pending list
void
appendPendingChannel (dma_channel_node_t * dma_channel,
                     dma_buffers_t      *dma_buffers);

// Clean out all pending items in given channel.
void
cleanPendingChannel(dma_buffers_t      *dma_buffers,
                    dma_channel_node_t *dma_channel);

// Go through pending list and search for
// one that is marked pending for the given channel
int
findPendingChannel(dma_buffers_t      *dma_buffers,
                   dma_channel_node_t *dma_channel);

// Find Dma buffer by using Physical address
// Searches through the pool itself no
// locking worries.
dma_buf_node_t *
findDmaBufferByPhy(dma_buffers_t *dma_buffers,
                   uint32_t       addr,
                   uint32_t       addr_hi,
                   uint32_t       max_dma_buffers);

// get Dma Buffer address limits
void
getDmaBufAddrLimit(dma_buffers_t *dma_buffers,
                   uint32_t      *p_low_addr,
                   uint32_t      *p_low_addr_hi,
                   uint32_t      *p_hi_addr,
                   uint32_t      *p_hi_addr_hi,
                   uint32_t       max_dma_buffers,
                   uint32_t      dma_buf_size);


#endif // _SHARED_MEM_H

