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

#include "os_arch.h"
#include "sx_port.h"
#include "ipcqueue.h"
#include "ipcqueue_api.h"

#if defined (OS_IS_WINDOWS)
    #include "stddcls.h"
#endif



//**********************************************************
// Name        : sx_queue_module_init
//
// Parameters  :
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Initialise queue module
//**********************************************************
uint32_t
sx_queue_module_init (
    sx_queue_mod_t                      *queue_mod,      // in local memory.
    volatile sx_queue_mod_rsc_t __iomem *queue_mod_sha,  // in shared memory.
    uint32_t                             options,        // Options
    sx_queue_mod_t                     **mod_ha)         // Output : Module handle
{  
#ifdef SX_QUEUE_VALIDATE_PARMS
    if (!queue_mod_sha) {
        return SX_QUEUE_ERR_PARMS;
    }
#endif
    if (options & SX_QUEUE_OPTION_PRIMARY)
    {
        // Initialize semaphore flags to 0
        queue_mod_sha->sem[0].flag[0] = 0;
        queue_mod_sha->sem[0].flag[1] = 0;
        queue_mod_sha->sem[1].flag[0] = 0;
        queue_mod_sha->sem[1].flag[1] = 0;
    }

    if (!(options & SX_QUEUE_OPTION_MULTI) ||
         (options & SX_QUEUE_OPTION_PRIMARY)) {
        queue_mod_sha->queue = (sx_queue_tp)0;
        DCACHE_WBI_LINE(queue_mod_sha);
    }
    queue_mod->pp_queue = &(queue_mod_sha->queue);
    queue_mod->queue_mod_rsc = queue_mod_sha;
    queue_mod->options  = (uint8_t)options;
    OS_SPINLOCK_INIT(&queue_mod->spinlock);

    xOS_PRINTK("pp_queue %p\n",
              queue_mod->pp_queue);

    *mod_ha = queue_mod; 

    xOS_PRINTK("*mod_ha %p\n",
               *mod_ha);

    return 0;
}



//**********************************************************
// Name        : sx_queue_create
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Create queue 
//**********************************************************
uint32_t
sx_queue_create (
    sx_queue_mod_t                *mod_ha,   // Module handle
    volatile sx_queue_t  __iomem  *queue,    // Pointer to q struct(32 byte aligned)
    uint32_t                       id,       // Unique ID of queue
    uint8_t                       dir,        // Queue direction.  0=board2host, 1=host2board.
    volatile sx_queue_t  __iomem **queue_ha) // Output : Queue handle
{
    OS_QUEUE_MUTEX_INIT;

#ifdef SX_QUEUE_VALIDATE_PARMS
    if (queue_ha) {
        *queue_ha = NULL; // Default
    }

    if (!queue) {
        return SX_QUEUE_ERR_PARMS;
    }

    if((dir != SX_QUEUE_DIR_BOARD2HOST) && (dir != SX_QUEUE_DIR_HOST2BOARD))
        return SX_QUEUE_ERR_PARAMS;

    // Check 32 byte alignment
    if ((uint32_t)queue & (uint32_t)(32-1)) {
        return SX_QUEUE_ERR_ALIGN;
    }

    if (!sx_queue_lookup(mod_ha, id, NULL)) {
        return SX_QUEUE_ERR_EXISTS;   // Another queue with this ID exists
    }
#endif

    if (queue_ha) {
        *queue_ha = queue;
    }

    // Prepare queue
    queue->data_last  = (sx_queue_datahdr_tp)0;
    queue->data_first = (sx_queue_datahdr_tp)0;
    queue->callback   = (sx_callback_tp)0;
    queue->status     = (sx_uint32_tp)0;
    queue->options    = mod_ha->options & SX_QUEUE_OPTION_MULTI;
    queue->dir        = dir;

    if (mod_ha->options & SX_QUEUE_OPTION_MULTI) {
        OS_SPINLOCK(&mod_ha->spinlock);
        OS_QUEUE_MUTEX_GET(&(mod_ha->queue_mod_rsc->sem[dir]));

        queue->id = cpu_to_le32(id);

        // Insert in list and Flush
        queue->queue_nxt  = *mod_ha->pp_queue;
        *mod_ha->pp_queue =
          (sx_queue_tp) 
            OS_HOST2BOARD_PTR(queue,
                              mod_ha->queue_mod_rsc);
        DCACHE_WBI_REGION(queue, SX_QUEUE_DESCR_SIZE)
        DCACHE_WBI_LINE(mod_ha->pp_queue) 
        SYNC

        OS_QUEUE_MUTEX_FREE(&(mod_ha->queue_mod_rsc->sem[dir]));
        OS_SPINUNLOCK(&mod_ha->spinlock);
    } else {
        queue->id = id;

        // Insert in list
        queue->queue_nxt  = *mod_ha->pp_queue;
        *mod_ha->pp_queue =
          (sx_queue_tp)((os_uint_arch_t)queue);
    }
    return 0;
}



//**********************************************************
// Name        : sx_queue_lookup
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Lookup queue handle based on queue ID.
//**********************************************************
uint32_t
sx_queue_lookup (
    sx_queue_mod_t               *mod_ha,     // Module handle
    uint32_t                      id,         // ID of queue
    volatile sx_queue_t __iomem **queue_ha)   // Output : Queue handle
{
    volatile sx_queue_t __iomem *queue;
    sx_queue_tp                  queue_nxt;
    uint32_t                     rc = SX_QUEUE_ERR_NOTFOUND;
#if defined (OS_IS_WINDOWS)
    uint32_t                     cachedAddress;
#endif

    OS_QUEUE_MUTEX_INIT;

    if (queue_ha) {
        *queue_ha = NULL; // Default
    }
    if (mod_ha->options & SX_QUEUE_OPTION_MULTI) {
        OS_SPINLOCK(&mod_ha->spinlock);
        //no lock in reading1
        //OS_QUEUE_MUTEX_GET(mod_ha->queue_mod_rsc);

        queue_nxt = *mod_ha->pp_queue;

        xOS_PRINTK("pp_queue %x, id %x\n",
                  mod_ha->pp_queue,
                  id);

        DCACHE_INV_LINE(mod_ha->pp_queue) 

        while (queue_nxt && rc) {
#if defined(OS_IS_WINDOWS)
            cachedAddress = (uint32_t)queue_nxt + (uint32_t)mod_ha->pp_queue;
#endif
            queue =
              (volatile sx_queue_t __iomem *)
                OS_BOARD2HOST_PTR(queue_nxt,
                                  mod_ha->queue_mod_rsc);

            if (id == le32_to_cpu(queue->id)) {
                if (queue_ha) {
                    *queue_ha = queue;
                }
                rc = 0;
            } else {
                queue_nxt = queue->queue_nxt;
            }
            DCACHE_INV_REGION(queue, SX_QUEUE_DESCR_SIZE)
        }
        //OS_QUEUE_MUTEX_FREE(&(mod_ha->queue_mod_rsc->sem[queue->dir]));
        OS_SPINUNLOCK(&mod_ha->spinlock);
    } else {
        queue_nxt = *mod_ha->pp_queue;
        while (queue_nxt && rc) {
            queue =
              (sx_queue_t *)((os_uint_arch_t)queue_nxt);
            if (id == queue->id) {
                if (queue_ha) {
                    *queue_ha = queue;
                }
                rc = 0;
            } else {
                queue_nxt = queue->queue_nxt;
            }
        }
    }
    xOS_PRINTK("queue_ha %p\n",
              *queue_ha);
    return rc;
}



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
    sx_bool                      save_context)   // For future use, must be TRUE 
{
    volatile sx_queue_t __iomem *queue;

    OS_QUEUE_MUTEX_INIT;

#ifdef SX_QUEUE_VALIDATE_PARMS
    if (!(void *)queue_ha || !callback || !save_context) {
        return SX_QUEUE_ERR_PARMS;
    }
#endif

    queue = queue_ha;

    if (queue->options & SX_QUEUE_OPTION_MULTI) {
        OS_SPINLOCK(&mod_ha->spinlock);
        OS_QUEUE_MUTEX_GET(&(mod_ha->queue_mod_rsc->sem[queue->dir]));

        queue->options |=
          (mod_ha->options & SX_QUEUE_OPTION_PRIMARY);
        queue->callback =
          (sx_callback_tp)((os_uint_arch_t)callback);
        queue->param = param;

        if (save_context) {
            queue->options |= SX_QUEUE_OPTION_SAVE_CONTEXT;
        }
        DCACHE_WBI_REGION(queue, SX_QUEUE_DESCR_SIZE)
        SYNC

        OS_QUEUE_MUTEX_FREE(&(mod_ha->queue_mod_rsc->sem[queue->dir]));
        OS_SPINUNLOCK(&mod_ha->spinlock);
    } else {
        queue->callback =
          (sx_callback_tp)((os_uint_arch_t)callback);
        queue->param        = param;

        if (save_context) {
            queue->options |= SX_QUEUE_OPTION_SAVE_CONTEXT;
        }
  }
  return 0;
}



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
sx_queue_delete(
             sx_queue_mod_t         *mod_ha,    // Module handle 
    volatile sx_queue_t     __iomem *queue_ha)  // Queue handle
{
    volatile sx_queue_tp __iomem *pp_queue;
    volatile sx_queue_t  __iomem *p_queue;
    volatile sx_queue_t  __iomem *this_queue;
    uint32_t                      rc = SX_QUEUE_ERR_NOTFOUND;

    OS_QUEUE_MUTEX_INIT;

#ifdef SX_QUEUE_VALIDATE_PARMS
    if (!(void *)queue_ha) {
        return SX_QUEUE_ERR_PARMS;
    }
#endif

    this_queue = queue_ha;
    pp_queue   = mod_ha->pp_queue;

    if (this_queue->options & SX_QUEUE_OPTION_MULTI) {
        OS_SPINLOCK(&mod_ha->spinlock);
        OS_QUEUE_MUTEX_GET(&(mod_ha->queue_mod_rsc->sem[this_queue->dir]));

        while (*pp_queue && rc) {
            p_queue =
              (sx_queue_t *)
                OS_BOARD2HOST_PTR(*pp_queue,
                                  mod_ha->queue_mod_rsc);
            if (p_queue == this_queue) {
                *pp_queue = p_queue->queue_nxt;

                DCACHE_WBI_LINE(pp_queue) 
                SYNC
                DCACHE_INV_REGION(p_queue, SX_QUEUE_DESCR_SIZE)
                rc = 0;
            } else {
                DCACHE_INV_LINE(pp_queue); 
                pp_queue = &p_queue->queue_nxt;
                DCACHE_INV_REGION(p_queue, SX_QUEUE_DESCR_SIZE)
            }
        }
        DCACHE_INV_LINE(pp_queue) 

        OS_QUEUE_MUTEX_FREE(&(mod_ha->queue_mod_rsc->sem[this_queue->dir]));
        OS_SPINUNLOCK(&mod_ha->spinlock);
    } else {
        while (*pp_queue && rc) {
            p_queue =
              (sx_queue_t *)((os_uint_arch_t)*pp_queue);
            if (p_queue == this_queue) {
                *pp_queue = p_queue->queue_nxt;
                rc = 0;
            } else {
                pp_queue = &p_queue->queue_nxt;
            }
        }
    }
    return rc;
}



#if defined (OS_IS_WINDOWS)
#pragma LOCKEDCODE
#endif

//**********************************************************
// Name        : sx_queue_write
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Write entry to queue.
//**********************************************************

uint32_t
sx_queue_write(
    sx_queue_mod_t                      *mod_ha,       // Module handle
    volatile sx_queue_t         __iomem *queue_ha,     // Queue handle
    volatile sx_queue_datahdr_t __iomem *loc_data_hdr, // Ptr to data header(32 byte aligned)
    uint32_t                             size)         // Size of message
{
    volatile sx_queue_t          __iomem *queue;
    volatile sx_queue_datahdr_t  __iomem *loc_data_last;
    sx_queue_datahdr_tp                   brd_data_last;
    volatile uint32_t            __iomem *loc_status_addr;
    volatile uint32_t            __iomem  status;
    uint32_t                              bitpos;
    sx_queue_datahdr_tp                   brd_data_hdr;

    OS_QUEUE_MUTEX_INIT;

#ifdef SX_QUEUE_VALIDATE_PARMS
    if (!(void *)queue_ha || !loc_data_hdr || !size) {
        return SX_QUEUE_ERR_PARMS;
    }

    // Check 32 byte alignment
    if ((uint64_t)loc_data_hdr & (uint64_t)(32-1)) {
        return SX_QUEUE_ERR_ALIGN;
    }
#endif

    queue = queue_ha;
    loc_data_hdr->queue_data_nxt = (sx_queue_datahdr_tp)0;

    if (queue->options & SX_QUEUE_OPTION_MULTI) {
        DCACHE_WBI_REGION(data_hdr, size)
        SYNC

        brd_data_hdr =
          (sx_queue_datahdr_tp)
            OS_HOST2BOARD_PTR(loc_data_hdr,
                              mod_ha->queue_mod_rsc);
        OS_SPINLOCK(&mod_ha->spinlock);
        OS_QUEUE_MUTEX_GET(&(mod_ha->queue_mod_rsc->sem[queue->dir]));

        DCACHE_INV_REGION(queue, SX_QUEUE_DESCR_SIZE)

        brd_data_last = queue->data_last;
        if (brd_data_last) {   
            loc_data_last =
              (sx_queue_datahdr_t *)
                OS_BOARD2HOST_PTR(brd_data_last,
                                  mod_ha->queue_mod_rsc);
            loc_data_last->queue_data_nxt = brd_data_hdr;

            DCACHE_WBI_LINE(loc_data_last)
            SYNC
        }
        queue->data_last = brd_data_hdr;

        if (!queue->data_first) {
            queue->data_first = brd_data_hdr;
        }
        if (queue->status) {
            loc_status_addr =
              (volatile uint32_t __iomem *)
                OS_BOARD2HOST_PTR(queue->status,
                                  mod_ha->queue_mod_rsc);
            bitpos = (queue->bitpos);

            status = le32_to_cpu(*loc_status_addr);

            status |= (1 << bitpos);

            *loc_status_addr = cpu_to_le32(status);
            OS_PRINTK("set bitpos %x in status reg %p, %x\n",
                      bitpos,
                      loc_status_addr,cpu_to_le32(status));

            DCACHE_WBI_LINE(loc_status_addr)
            SYNC
        }

        DCACHE_WBI_REGION(queue, SX_QUEUE_DESCR_SIZE)
        SYNC

        OS_QUEUE_MUTEX_FREE(&(mod_ha->queue_mod_rsc->sem[queue->dir]));
        OS_SPINUNLOCK(&mod_ha->spinlock);
    } else {
        brd_data_hdr =
          (sx_queue_datahdr_tp)((os_uint_arch_t)loc_data_hdr);

        brd_data_last = queue->data_last;
        if (brd_data_last) {   
            loc_data_last =
              (sx_queue_datahdr_t *)((os_uint_arch_t)brd_data_last);

            loc_data_last->queue_data_nxt = brd_data_hdr;
        }

        queue->data_last = brd_data_hdr;
        if (!queue->data_first) {
            queue->data_first = brd_data_hdr;
        }

        if (queue->status) {
            loc_status_addr =
              (uint32_t *)((os_uint_arch_t)queue->status);

            *loc_status_addr |= (1 << queue->bitpos);
        }
    }

    xOS_PRINTK("datafirst %x, datalast %x\n",
               queue->data_first,
               queue->data_last);
    return 0;
}



#if defined (OS_IS_WINDOWS)
#pragma LOCKEDCODE
#endif

//**********************************************************
// Name        : sx_queue_read
//
// Returns     : 0 if operation is ok (data available), otherwise error code
//
// Description : Read entry from queue.
//**********************************************************
uint32_t
sx_queue_read(
    sx_queue_mod_t                       *mod_ha,   // Module handle
    volatile sx_queue_t         __iomem  *queue_ha, // Queue handle
    volatile sx_queue_datahdr_t __iomem **data_hdr) // Output : Data header (may be NULL)
{
    volatile sx_queue_t         __iomem *queue;
    volatile sx_queue_datahdr_t __iomem *loc_data_first;
    volatile uint32_t           __iomem *loc_status_addr;
    uint32_t                             status;
    uint32_t                             bitpos;
    uint32_t                             rc = 0;

    OS_QUEUE_MUTEX_INIT;

#ifdef SX_QUEUE_VALIDATE_PARMS
    if (!(void *)queue_ha) {
        return SX_QUEUE_ERR_PARMS;
    }
#endif

    queue = queue_ha;

    xOS_PRINTK("queue_ha %p\n",
              queue_ha);

    if (queue->options & SX_QUEUE_OPTION_MULTI) {
        OS_SPINLOCK(&mod_ha->spinlock);
        OS_QUEUE_MUTEX_GET(&(mod_ha->queue_mod_rsc->sem[queue->dir]));

        DCACHE_INV_REGION(queue, SX_QUEUE_DESCR_SIZE)

        if (!queue->data_first) {
            if (data_hdr) {
                *data_hdr = NULL;
            }
            rc = SX_QUEUE_ERR_EMPTY;
            DCACHE_INV_REGION(queue, SX_QUEUE_DESCR_SIZE)
        } else {
            loc_data_first =
             (sx_queue_datahdr_t *)
               OS_BOARD2HOST_PTR(queue->data_first,
                                 mod_ha->queue_mod_rsc);
            queue->data_first = loc_data_first->queue_data_nxt;
            if (!queue->data_first) {
                queue->data_last = (sx_queue_datahdr_tp)0;

                if (queue->status) {
                    loc_status_addr =
                      (uint32_t *)
                        OS_BOARD2HOST_PTR(queue->status,
                                          mod_ha->queue_mod_rsc);
                    bitpos = (queue->bitpos);

                    xOS_PRINTK("clear bitpos %d in status reg %p\n",
                              bitpos,
                              loc_status_addr);

                    status =  le32_to_cpu(*loc_status_addr);
                    status &= ~(1 << bitpos); 
                    *loc_status_addr = cpu_to_le32(status);
                    DCACHE_WBI_LINE(loc_status_addr)
                    SYNC
                }
            }

            if (data_hdr) {
                *data_hdr = loc_data_first; 
                xOS_PRINTK("*data_hdr %p\n",
                          *data_hdr);
            }
            DCACHE_WBI_REGION(queue, SX_QUEUE_DESCR_SIZE)
            SYNC
        }

        OS_QUEUE_MUTEX_FREE(&(mod_ha->queue_mod_rsc->sem[queue->dir]));
        OS_SPINUNLOCK(&mod_ha->spinlock);
    } else {
        if (!queue->data_first) {
            if (data_hdr) {
                *data_hdr = NULL;
            }
            rc = SX_QUEUE_ERR_EMPTY;
        } else {
            loc_data_first =
             (sx_queue_datahdr_t *)((os_uint_arch_t)queue->data_first);

            queue->data_first = loc_data_first->queue_data_nxt;

            if (!queue->data_first) {
                queue->data_last = (sx_queue_datahdr_tp)0;

                if (queue->status) {
                    loc_status_addr =
                      (uint32_t *)((os_uint_arch_t)queue->status);
                    *loc_status_addr &= ~(1 << queue->bitpos);
                }
            }
            if (data_hdr) {
                *data_hdr = loc_data_first; 
            }
        }
    }
    return rc;
}



#if defined (OS_IS_WINDOOWS)
#pragma LOCKEDCODE
#endif

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
sx_queue_status_register(
    sx_queue_mod_t              *mod_ha,       // Module handle
    volatile sx_queue_t __iomem *queue_ha,     // Queue handle
    volatile uint32_t   __iomem *status,       // Status word
    uint8_t                      bitpos)       // Bit position in status word
{
    volatile sx_queue_t __iomem *queue;

    OS_QUEUE_MUTEX_INIT;

#ifdef SX_QUEUE_VALIDATE_PARMS
    if (!(void *)queue_ha) {
        return SX_QUEUE_ERR_PARMS;
    }
#endif

    OS_PRINTK("queue_ha %p register bitpos %d in status reg %p\n",
              queue_ha,
              bitpos,
              status);

    queue = queue_ha;

    if (queue->options & SX_QUEUE_OPTION_MULTI) {
        OS_SPINLOCK(&mod_ha->spinlock);
        OS_QUEUE_MUTEX_GET(&(mod_ha->queue_mod_rsc->sem[queue->dir]));

        DCACHE_INV_REGION(queue, SX_QUEUE_DESCR_SIZE)
        queue->status =
          (sx_uint32_tp)
            OS_HOST2BOARD_PTR(status,
                              mod_ha->queue_mod_rsc);
        queue->bitpos = (bitpos);

        DCACHE_WBI_REGION(queue, SX_QUEUE_DESCR_SIZE)
        SYNC

        OS_QUEUE_MUTEX_FREE(&(mod_ha->queue_mod_rsc->sem[queue->dir]));
        OS_SPINUNLOCK(&mod_ha->spinlock);
    } else {
        queue->status =
          (sx_uint32_tp)((os_uint_arch_t)status);
        queue->bitpos = (bitpos);
    }

    return 0;
}



#if defined (OS_IS_WINDOWS)
#pragma LOCKEDCODE
#endif

//**********************************************************
// Name        : sx_queue_status_unregister
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Unregister STATUS word for particular queue.
//**********************************************************
uint32_t
sx_queue_status_unregister(
    sx_queue_mod_t                  *mod_ha,   // Module handle
    volatile sx_queue_t     __iomem *queue_ha) // Queue handle
{
    volatile sx_queue_t __iomem *queue;

    OS_QUEUE_MUTEX_INIT;

#ifdef SX_QUEUE_VALIDATE_PARMS
    if (!(void *)queue_ha) {
        return SX_QUEUE_ERR_PARMS;
    }
#endif

    xOS_PRINTK("queue_ha %p unregister bit pos\n",
               queue_ha);

    queue = queue_ha;

    if (queue->options & SX_QUEUE_OPTION_MULTI) {
        OS_SPINLOCK(&mod_ha->spinlock);
        OS_QUEUE_MUTEX_GET(&(mod_ha->queue_mod_rsc->sem[queue->dir]));

        DCACHE_INV_REGION(queue, SX_QUEUE_DESCR_SIZE)

        queue->status = (sx_uint32_tp)0;

        DCACHE_WBI_REGION(queue, SX_QUEUE_DESCR_SIZE)
        SYNC

        OS_QUEUE_MUTEX_FREE(&(mod_ha->queue_mod_rsc->sem[queue->dir]));
        OS_SPINUNLOCK(&mod_ha->spinlock);
    } else {
        queue->status = (sx_uint32_tp)0;
    }
    return 0;
}



#if defined (OS_IS_WINDOWS)
#pragma LOCKEDCODE
#endif

//**********************************************************
// Name        : sx_queue_status_read
//
// Returns     : Status word
//
// Description : Read STATUS word
//**********************************************************
uint32_t
sx_queue_status_read(
    sx_queue_mod_t            *mod_ha,   // Module handle
    volatile uint32_t __iomem *status)   // Status word to be read
{
#if defined (OS_IS_STRETCH)
    (void)&mod_ha;      // avoid compiler warning
#endif

#ifdef SX_QUEUE_VALIDATE_PARMS
    if (!status) {
        return 0;
    }
#endif  

    DCACHE_INV_REGION(status, sizeof(uint32_t))

    xOS_PRINTK("status %p = %x\n",
              status,
              *status);

    return le32_to_cpu(*status);
}



#if defined (OS_IS_WINDOWS)
#pragma
#endif

//**********************************************************
// Name        : sx_queue_isr
//
// Returns     : 0 if operation is ok, otherwise error code
//
// Description : Create queue 
//**********************************************************
uint32_t
sx_queue_isr( 
    sx_queue_mod_t  *mod_ha)    // Module handle
{
    volatile sx_queue_t __iomem *loc_queue;
    sx_queue_tp                  queue_nxt;
    sx_queue_callback_t          loc_callback;
    uint32_t                     param;

    do {
        loc_callback = NULL;
        loc_queue = NULL;
        if (mod_ha->options & SX_QUEUE_OPTION_MULTI) {
            OS_SPINLOCK(&mod_ha->spinlock);
            queue_nxt = *mod_ha->pp_queue;
            DCACHE_INV_LINE(mod_ha->pp_queue) 
            while (queue_nxt && !loc_callback) {
                loc_queue =
                  (sx_queue_t *)
                    OS_BOARD2HOST_PTR(queue_nxt,
                                      mod_ha->queue_mod_rsc);

                if (loc_queue->data_first && 
                    loc_queue->callback   &&
                    ((loc_queue->options  & SX_QUEUE_OPTION_PRIMARY) == 
                     (mod_ha->options & SX_QUEUE_OPTION_PRIMARY))) {
                    loc_callback =
                      (sx_callback_t)
                        OS_BOARD2HOST_PTR(loc_queue->callback,
                                          mod_ha->queue_mod_rsc);
                    param    = loc_queue->param;
                } else {
                    queue_nxt = loc_queue->queue_nxt;
                }
                DCACHE_INV_REGION(loc_queue, SX_QUEUE_DESCR_SIZE)
            }

            OS_SPINUNLOCK(&mod_ha->spinlock);
        } else {
            queue_nxt = *mod_ha->pp_queue;
            while (queue_nxt && !loc_callback) {
                loc_queue =
                  (sx_queue_t *)((os_uint_arch_t)queue_nxt);
                if (loc_queue->data_first && loc_queue->callback) {
                    loc_callback =
                      (sx_callback_t)((os_uint_arch_t)loc_queue->callback);
                    param    = loc_queue->param;
                } else {
                    queue_nxt = loc_queue->queue_nxt;
                }
            }
        }

        if (loc_callback) {
            loc_callback(loc_queue->param);
        }
    } while (loc_callback);
    return 0;
}

uint32_t 
sx_queue_mgmt_write(
    sx_queue_mod_t *   mod_ha,    // Module handle 
    volatile sx_queue_mgmt __iomem*  queue_ha,  // Queue handle
    volatile sx_queue_datahdr_t  __iomem *data_hdr, // Pointer to data header (32 byte aligned)
    uint32_t         size )     // Size of message
{
    sx_queue_t         *queue;
    sx_queue_mgmt    *mgmt_queue;
    uint32_t        data_last;
	uint32_t		 ret = 0;
    sx_queue_datahdr_tp                   brd_data_hdr;
 
#ifdef HOST_SHARED_MEMORY
    volatile sx_uint8 last_byte;
#endif

#ifdef SX_QUEUE_VALIDATE_PARMS
    if ( !(void *)queue_ha || !data_hdr || !size )
        return SX_QUEUE_ERR_PARMS;

    // Check 32 byte alignment
    if ( (sx_uint32)data_hdr & (sx_uint32)(32-1) )
        return SX_QUEUE_ERR_ALIGN;
#else
    size = size;
#endif

    queue = (sx_queue_t *)queue_ha;
    mgmt_queue = (sx_queue_mgmt *)queue_ha;

    // Make sure we are reading the current value of queue in shared memory
    DCACHE_INV_REGION( queue, SX_QUEUE_DESCR_SIZE );

    if ( queue->options & SX_QUEUE_OPTION_MULTI )
    {
        OS_SPINLOCK(&mod_ha->spinlock);

        data_last = le32_to_cpu((uint32_t)(queue->data_last));

#ifndef SX_QUEUE_MGMT_NEVER_FULL
        // If queue is full, return error.
        if ( le32_to_cpu((uint32_t)(queue->data_first)) ==
            ((data_last + 1) % SX_QUEUE_MGMT_QUEUE_SIZE) )
        {
DbgPrint (DRIVERNAME ": -" __FUNCTION__ "- "
          "MGMT QUEUE IS FULL\n");

            ret  = SX_QUEUE_ERR_FULL;
        }
        else
#endif
        {
            // Make sure data_hdr is written to shared memory
            DCACHE_WBI_REGION( data_hdr, size );
            SYNC

            brd_data_hdr = (sx_queue_datahdr_tp)OS_HOST2BOARD_PTR(data_hdr,
                            mod_ha->queue_mod_rsc);

            mgmt_queue->mgmt_queue[data_last] = (sx_queue_datahdr_tp) ((int32_t)brd_data_hdr);

            DCACHE_WBI_REGION( &(mgmt_queue->mgmt_queue[data_last]), sizeof(mgmt_queue->mgmt_queue[0]) );

            data_last = ((data_last + 1) == SX_QUEUE_MGMT_QUEUE_SIZE) ?
                               0 :
                               (data_last + 1);
            queue->data_last = (sx_queue_datahdr_tp) cpu_to_le32(data_last);

            DCACHE_WBI_REGION( &(queue->data_last), sizeof(queue->data_last) );
            SYNC
        }

        OS_SPINUNLOCK(&mod_ha->spinlock);
    }
    else
    {
        data_last = (uint32_t)(queue->data_last);

#ifndef SX_QUEUE_MGMT_NEVER_FULL
        // If queue is full, return error.
        if ( (uint32_t)(queue->data_first) ==
            ((data_last + 1) % SX_QUEUE_MGMT_QUEUE_SIZE) )
        {
DbgPrint (DRIVERNAME ": -" __FUNCTION__ "- "
          "MGMT QUEUE IS FULL\n");

            ret = SX_QUEUE_ERR_FULL;
        }
        else
#endif
        {
            brd_data_hdr =
                (sx_queue_datahdr_tp)((os_uint_arch_t)data_hdr);
            mgmt_queue->mgmt_queue[data_last] = brd_data_hdr;
            data_last = ((data_last + 1) == SX_QUEUE_MGMT_QUEUE_SIZE) ?
                               0 :
                               (data_last + 1);
            queue->data_last = (sx_queue_datahdr_tp) data_last;                              
        }
    }

    return ret;
}


//**********************************************************
// Name        : sx_queue_mgmt_read
//
// Returns     : 0 if operation is ok (data available), otherwise error code
//
// Description : Read entry from mgmt queue.
//
// Note        : This implementation does not use a mutex
//               and does not use the queue_ha->status field.
//**********************************************************
uint32_t 
sx_queue_mgmt_read(
    sx_queue_mod_t*   mod_ha,      // Module handle 
    volatile sx_queue_mgmt __iomem *  queue_ha,    // Queue handle
    volatile sx_queue_datahdr_t  __iomem **data_hdr ) // Output : Data header (possibly NULL)
{
    sx_queue_t         *queue;
    volatile sx_queue_datahdr_tp read_data;
    sx_queue_datahdr_t  * data_ptr;
    sx_queue_mgmt    *mgmt_queue;
    uint32_t        data_first;
	uint32_t		 ret = 0;

#ifdef SX_QUEUE_VALIDATE_PARMS
    if ( !(void *)queue_ha )
        return SX_QUEUE_ERR_PARMS;
#endif

    queue = (sx_queue_t *)queue_ha;
    mgmt_queue = (sx_queue_mgmt *)queue_ha;

    // Make sure we are reading the current value of queue in shared memory
    DCACHE_INV_REGION( queue, SX_QUEUE_DESCR_SIZE );
    
    if ( queue->options & SX_QUEUE_OPTION_MULTI ) 
    {
        OS_SPINLOCK(&mod_ha->spinlock);

		data_first = le32_to_cpu((uint32_t)(queue->data_first));

		// If queue is empty, return error.
		if ( data_first == le32_to_cpu((uint32_t)(queue->data_last)) )
		{
			if ( data_hdr )
				*data_hdr = NULL;

			ret = SX_QUEUE_ERR_EMPTY;
		}        
		else
		{        
            // Make sure we are reading the current value of mgmt_queue item
            DCACHE_INV_REGION( &(mgmt_queue->mgmt_queue[data_first]), sizeof(mgmt_queue->mgmt_queue[0]) );

            read_data = (sx_queue_datahdr_tp)(mgmt_queue->mgmt_queue[data_first]);

            data_ptr = (sx_queue_datahdr_t *)OS_BOARD2HOST_PTR(read_data,mod_ha->queue_mod_rsc);

            if ( data_hdr )
                *data_hdr = data_ptr;

            data_first = ((data_first + 1) == SX_QUEUE_MGMT_QUEUE_SIZE) ?
                                0 :
                                (data_first + 1);
            queue->data_first = (sx_queue_datahdr_tp) cpu_to_le32(data_first);
            DCACHE_WBI_REGION( &(queue->data_first), sizeof(queue->data_first) );
            SYNC
		}

        OS_SPINUNLOCK(&mod_ha->spinlock);
    }
    else
    {
		data_first = (uint32_t)(queue->data_first);

		// If queue is empty, return error.
		if ( data_first == (uint32_t)(queue->data_last) )
		{
			if ( data_hdr )
				*data_hdr = NULL;

			ret = SX_QUEUE_ERR_EMPTY;
		}
		else
		{
			read_data = mgmt_queue->mgmt_queue[data_first];
			queue->data_first = ((data_first + 1) == SX_QUEUE_MGMT_QUEUE_SIZE) ?
                            (sx_queue_datahdr_tp)0 :
                            (sx_queue_datahdr_tp)(data_first + 1);

			if ( data_hdr )
				*data_hdr = (sx_queue_datahdr_t*)(os_uint_arch_t)read_data; 
		}
    }

    return ret;
}

