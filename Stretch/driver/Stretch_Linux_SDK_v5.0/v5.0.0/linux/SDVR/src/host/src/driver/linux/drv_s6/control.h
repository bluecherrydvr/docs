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

#ifndef _CONTROL_H
#define _CONTROL_H

#define LOOKUP_TIMEOUT_COUNT    5

struct stretch_dev_struct;

// Write a management message to the management
// queue
os_status_t
WriteMgmtQueue(struct stretch_dev_struct *stretch_dev,
               os_irp_t                  *Irp);

// Complete Mgmt CONNECT message (returned).
ssize_t
ConnectMgmtMsgComplete(struct kiocb *iocb);

// Complete Mgmt DESTROY message (returned).
ssize_t
DestroyMgmtMsgComplete(struct kiocb *iocb);

// Wait for the Channel to destroy
os_status_t
WaitDestroy(struct stretch_dev_struct *stretch_dev,
            os_irp_t                  *irp);

// Wait for Channel Error
os_status_t
WaitError(struct stretch_dev_struct *stretch_dev,
          os_irp_t                  *Irp);

// Send a buffer
os_status_t
BufferSend(struct stretch_dev_struct *stretch_dev,
           os_irp_t                  *Irp);

ssize_t
BufferSendComplete(struct kiocb *iocb);

// Gather (store) the next buffer-ful of data
os_status_t
StoreReceiveBuffer(struct stretch_dev_struct *stretch_dev,
                   os_irp_t                  *Irp);

// (actually) Read a frame buffer from a port.
// This gets called from the user-space callback
// routine that gets called when the dispatch/irq
// indicates data ready in DMA buffer.
ssize_t
StoreReceiveBufferComplete(struct kiocb *iocb);

// Write a message to a channel.
os_status_t
WriteMsg(struct stretch_dev_struct *stretch_dev,
         os_irp_t                  *Irp);

// (initiate) Read a message from a channel.
os_status_t
ReadMsg(struct stretch_dev_struct *stretch_dev,
        os_irp_t                  *Irp);

// (actually) Read a message from a channel
// Used by AIO callback
ssize_t
ReadMsgComplete(struct kiocb *iocb);

// See if there is a message from a channel.
os_status_t
ReadMsgPoll(struct stretch_dev_struct *stretch_dev,
            os_irp_t                  *Irp);

// Initialize the IPC queue structure
os_status_t
InitIPCQueue(struct stretch_dev_struct *stretch_dev,
             os_irp_t                  *Irp);

// Check for Connect request from Board
os_status_t
CheckConnect(struct stretch_dev_struct *stretch_dev,
             os_irp_t                  *Irp);

// Check (accept) for Board connect
ssize_t
CheckConnectComplete(struct kiocb *iocb);

// Respond to Destroy message
os_status_t
DestroyResponse(struct stretch_dev_struct *stretch_dev,
                os_irp_t                  *Irp);

// Respond to Accept message
os_status_t
AcceptResponse(struct stretch_dev_struct *stretch_dev,
               os_irp_t                  *Irp);


// This function checks if message is
// received for specified class or not.
// If message is received, it returns the
// message in datahdr and returns true.
int
IsMessageRecvd(int                                   classId,
               struct stretch_dev_struct            *stretch_dev,
               volatile sx_queue_datahdr_t __iomem **datahdr);

// Generate an interrupt by writing to the
// interrupt register on board
void
GenerateInterrupt(struct stretch_dev_struct *stretch_dev);

// Reset the board by stopping services/queues
// and issuing reset at GPIO pins
void
ResetBoard(struct stretch_dev_struct *stretch_dev);

// Notify the board about a free buffer via
// the management message queue
void
sendFreeBufNotify(struct stretch_dev_struct *stretch_dev,
                  dma_buf_node_t *dma_buffer);

// This function destroys ALL channels
void
DestroyAllChannels(struct stretch_dev_struct *stretch_dev);

// Clear out all items in the cancel safe queue
void
ClearCancelSafeMsgQueue(struct stretch_dev_struct *stretch_dev,
                        uint32_t                   classId);

// PCIBoot function
os_status_t
PCIBoot(struct stretch_dev_struct *stretch_dev,
             os_irp_t                  *Irp);

os_status_t
GetBoardDetail(struct stretch_dev_struct *stretch_dev,
             os_irp_t                  *Irp);


int
IoCancel(struct kiocb *iocb,struct io_event * e);

#endif // _CONTROL_H

