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

#ifndef _IPCQUEUE_API_H
#define _IPCQUEUE_API_H

// Callback function
typedef void (*sx_queue_callback_t)(uint32_t);

#ifdef __cplusplus
extern "C" {
#endif


//**********************************************************
// Define API types
//**********************************************************

// See os_arch.h

//**********************************************************
// Function prototypes
//**********************************************************

//**********************************************************
// Name        : sx_queue_module_init
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Initialise queue module
//**********************************************************
// Definitions for sx_queue_module_init(,,options,)
#define SX_QUEUE_OPTION_MULTI          0x0001   // If set, this module will be used for multi-
                                                // CPU or multi-thread communication.

#define SX_QUEUE_OPTION_PRIMARY        0x0002   // In a multi-CPU/multi-thread system
                                                // (see option MULTI above), 
                                                // One and only one of the CPUs/threads
                                                // must set this flag when initialising a
                                                // queueing module. If MULTI flag is not set,
                                                // this flag is don't care.
                                             
#define SX_QUEUE_OPTION_SAVE_CONTEXT   0x0004   // This flag is reserved for future use and
                                                // is currently don't care.
uint32_t
sx_queue_module_init (
    sx_queue_mod_t                      *queue_mod,    // Pointer to queue module structure in local memory.
    volatile sx_queue_mod_rsc_t __iomem *queue_mod_sha,// Pointer to sx_queue_mod_rsc object in shared memory.
    uint32_t                             options,      // Options (see below defines)
    sx_queue_mod_t                     **mod_ha );     // Output : Module handle


//**********************************************************
// Name        : sx_queue_create
//
// Returns     : 0 if operation is ok, otherwise error code
//
//**********************************************************
uint32_t
sx_queue_create (
             sx_queue_mod_t          *mod_ha,       // Module handle
    volatile sx_queue_t     __iomem  *queue,        // Pointer to queue structure (32 byte aligned)
    uint32_t                          id,           // Unique ID of queue
    uint8_t                     dir,
    volatile sx_queue_t     __iomem **queue_ha);    // Output : Queue handle

//**********************************************************
// Name        : sx_queue_lookup
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Lookup queue handle based on queue ID.
//**********************************************************
uint32_t
sx_queue_lookup (
    sx_queue_mod_t                   *mod_ha,      // Module handle
    uint32_t                          id,          // ID of queue
    volatile sx_queue_t     __iomem **queue_ha);   // Output : Queue handle

//**********************************************************
// Name        : sx_queue_(callback)_register
//
// Returns     : 0 if operation is ok, otherwise error code
//
//**********************************************************
uint32_t
#if defined (OS_IS_LINUX) || defined (OS_IS_WINDOWS)
sx_queue_register (
#else
sx_queue_callback_register (
#endif
    sx_queue_mod_t              *mod_ha,         // Module handle
    volatile sx_queue_t __iomem *queue_ha,       // Queue handle
    sx_queue_callback_t          callback,       // Callback function
    uint32_t                     param,          // Parameter for callback
    sx_bool                      save_context);  // For future use, must be set to TRUE

//**********************************************************
// Name        : sx_queue_delete
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Delete queue.
//               It is strongly recommended that the queue
//               is emptied before performing this operation.
//**********************************************************
uint32_t 
sx_queue_delete (
    sx_queue_mod_t              *mod_ha,     // Module handle
    volatile sx_queue_t __iomem *queue_ha); // Queue handle

//**********************************************************
// Name        : sx_queue_write
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Write entry to queue.
//**********************************************************
uint32_t
sx_queue_write (
    sx_queue_mod_t                      *mod_ha,   // Module handle
    volatile sx_queue_t         __iomem *queue_ha, // Queue handle
    volatile sx_queue_datahdr_t __iomem *data_hdr, // Pointer to data header (32 byte aligned)
    uint32_t                             size);    // Size of message

uint32_t 
sx_queue_mgmt_write(
	sx_queue_mod_t  *mod_ha,    // Module handle
    volatile sx_queue_mgmt __iomem* queue_ha,  // Queue handle
    volatile sx_queue_datahdr_t __iomem *data_hdr, // Pointer to data header (32 byte aligned)
    uint32_t        size );    // Size of message

//**********************************************************
// Name        : sx_queue_read
//
// Returns     : 0 if operation is ok (data available), otherwise error code
//
// Description : Read entry from queue.
//**********************************************************
uint32_t
sx_queue_read (
    sx_queue_mod_t                       *mod_ha,       // Module handle
    volatile sx_queue_t         __iomem  *queue_ha,     // Queue handle
    volatile sx_queue_datahdr_t __iomem **data_hdr);    // Output : Data header (possibly NULL)

uint32_t 
sx_queue_mgmt_read(
	sx_queue_mod_t  *mod_ha,       // Module handle
    volatile sx_queue_mgmt __iomem * queue_ha,     // Queue handle
    volatile sx_queue_datahdr_t __iomem **data_hdr ); // Output : Data header (possibly NULL)

//**********************************************************
// Name        : sx_queue_status_register
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Register STATUS word for particular queue.
//               'status' must be in shared memory.
//               Only one status word can be registered
//               for any given queue, but same status word
//               can be used for several queues as long
//               as 'bitpos' is different.
//               For each bit, a '1' signifies that queue 
//               is non empty.
//**********************************************************
uint32_t
sx_queue_status_register (
    sx_queue_mod_t              *mod_ha,       // Module handle
    volatile sx_queue_t __iomem *queue_ha,     // Queue handle
    volatile uint32_t   __iomem *status,       // Status word
    uint8_t                      bitpos);      // Bit position in status word

//**********************************************************
// Name        : sx_queue_status_unregister
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Unregister STATUS word for particular queue.
//**********************************************************
uint32_t
sx_queue_status_unregister (
    sx_queue_mod_t              *mod_ha,     // Module handle
    volatile sx_queue_t __iomem *queue_ha);  // Queue handle

//**********************************************************
// Name        : sx_queue_status_read
//
// Returns     : Status word
//
// Description : Read STATUS word
//**********************************************************
uint32_t
sx_queue_status_read (
    sx_queue_mod_t            *mod_ha,   // Module handle
    volatile uint32_t __iomem *status);  // Status word to be read

//**********************************************************
// Name        : sx_queue_isr
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Queue module ISR to be invoked by interrupt handler
//**********************************************************
uint32_t
sx_queue_isr (
    sx_queue_mod_t *mod_ha); // Module handle


#ifdef __cplusplus
}
#endif

#endif // #ifdef _IPCQUEUE_API_H


