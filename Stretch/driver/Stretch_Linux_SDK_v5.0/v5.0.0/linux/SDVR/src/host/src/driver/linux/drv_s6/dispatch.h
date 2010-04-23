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

#ifndef _DISPATCH_H
#define _DISPATCH_H


//----------------------------------------
// Message Handler Thread
//----------------------------------------
typedef struct
{
    // Allocated memory buffer pool for messages
    os_thread_t        *thread;
    int                 started;

} msg_handler_t;



//----------------------------------------
// Prototypes
//----------------------------------------

// Initialize the dispatch resources.
void
InitializeDispatch(struct stretch_dev_struct *stretch_dev);

// De-initialize the dispatch resources
void
DeInitializeDispatch(struct stretch_dev_struct *stretch_dev);

/*
// Allocate memory for and initialize the
// message handling queue buffers and lists
void
initMsgHandlerThread(msg_handler_t *msg_handler);

// Start the message handling thread
os_status_t
startMsgHandlerThread(msg_handler_t *msg_handler,
                      struct stretch_dev_struct *stretch_dev);

// Stop the current message-handling thread
int
stopMsgHandlerThread(msg_handler_t *msg_handler);

// Wake up the current message-handling thread
void
WakeMsgHandlerThread(msg_handler_t *msg_handler);

// De-allocate and de-initialize the message handling queue
void releaseMsgHandlerThread(msg_handler_t *msg_handler);
*/

// Main low-level interrupt routine
// Called upon PCI interrupt
#ifdef KVER_LESS_2620
irqreturn_t
stretch_isr(int   irq,
            void *dev_id,
            struct pt_regs*);
#else
irqreturn_t
stretch_isr(int   irq,
            void *dev_id);
#endif

// This function will be called by the
// low-level interrupt handler.
// as an interrupt dispatch routine (DPC)
#ifdef KVER_LESS_2620
void
stretch_dpc(void *dev_arg);
#else
void
stretch_dpc(struct work_struct *dev);
#endif



#endif // _DISPATCH_H


