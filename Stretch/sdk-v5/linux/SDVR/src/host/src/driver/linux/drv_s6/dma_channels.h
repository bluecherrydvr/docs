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

#ifndef _DMA_CHANNELS_H
#define _DMA_CHANNELS_H

#ifndef SCT_MAX_CHANNEL
#define SCT_MAX_CHANNEL 64
#endif


// Definitions for dma_channel_node_t:state field
#define CHANNEL_STATE_ACTIVE            0
#define CHANNEL_STATE_CLOSE_PENDING     1
#define CHANNEL_STATE_CLOSE             2
#define CHANNEL_STATE_CONNECT_PENDING   3
#define CHANNEL_STATE_DEAD              4

// Definitions for dma_channel_node_t:type field
#define CHANNEL_SEND                    0
#define CHANNEL_RECV                    1

// Definitions for dma_channel_node_t:irpContext field
#define BUFFER_PENDING_CONTEXT          0
#define DESTROY_CONTEXT                 1
#define BUFFER_PROCESSING_CONTEXT       2
#define CONNECT_PENDING_CONTEXT         3
#define DESTROY_PENDING_CONTEXT         4
#define ERROR_CONTEXT                   5
#define NUMBER_OF_CONTEXTS              6
#define ANY_CONTEXT                     NUMBER_OF_CONTEXTS



//========================================
// Dma Channel (node) structure
//========================================
struct dma_channel_node_struct
{
    // The list linkage
    os_list_node_t                       list_node;

    // Channel port number
    uint32_t                             port; 

    // Number of bufs allocated for this channel
    uint32_t                             nbuf;

    // Maximum size of buffers on this channel
    uint32_t                             max_size;

    // Size of last received buffer
    os_size_t                            lastReceivedSize;

    // Channel state (see above definitions)
    int                                  state;

    // Indicates pending receive
    int                                  receivePending;

    // Channel type (see above definitiions)
    int                                  type;

    // The buffer list
    os_list_t                            buf_list;

    // Event that waits for a destroy acknowlege
    os_event_t                           destroyAck;

    // The current Irp being processed.
    os_irp_t                            *CurrentIrp;
    
    // Response message
    volatile sx_queue_datahdr_t __iomem *mgmt_queue_msg_hdr;

    // Cancel-safe queues
    os_pendqueue_t                       CancelSafeQueue;
    os_pendqueue_t                       CancelSafeBuffersQueue;

    // Spinlock
    os_spinlock_t                        spinlock;
    int                                  pending_count;
    int                                  t_count;
};
typedef struct dma_channel_node_struct dma_channel_node_t;



//========================================
// Dma Channel list
//========================================
struct dma_channels_struct
{
    // The pool of buffer pointers
    dma_channel_node_t *pool[SCT_MAX_CHANNEL];

    // Number of active channels
    int                 size;

    // Lock for the list.
    // Note: Each node has its own locks.
    os_spinlock_t       spinlock;

};
typedef struct dma_channels_struct dma_channels_t;




//----------------------------------------
// Dma Channel prototypes
//----------------------------------------
// Fix forward references
struct stretch_dev_struct;

// This function initalizes the channel
// using port specified.
dma_channel_node_t *
InitializeChannel(dma_channels_t *dma_channels,
                  uint32_t        port);

// This function gets the Channel from the
// channel list using port.
// Returns NULL if channel not found.
dma_channel_node_t *
getChannel(dma_channels_t *dma_channels,
           uint32_t        port);

// This function gets the channel which is
// in pending connetion state
dma_channel_node_t *
getConnectPendingChannel(dma_channels_t *dma_channels,
                         uint32_t        port);

// This function checks if any wild card
// accept is waiting.
dma_channel_node_t *
getWildCardChannel(dma_channels_t *dma_channels);

// This functions gets the index of the
// channel from global channel list.
int
getChannelIndex(dma_channel_node_t *channel,
                dma_channels_t     *dma_channels);

// This function returns the channel messages back to S55
void
ReturnChannelMessages(dma_channel_node_t *this_chan);

// This function removes the channel from
// channel list.
void
RemoveChannel(dma_channel_node_t        *this_chan,
              struct stretch_dev_struct *stretch_dev);

// Initialize the DmaChannels List
void
InitializeDmaChannels(dma_channels_t *dma_channels);


#endif //_DMA_CHANNELS_H

