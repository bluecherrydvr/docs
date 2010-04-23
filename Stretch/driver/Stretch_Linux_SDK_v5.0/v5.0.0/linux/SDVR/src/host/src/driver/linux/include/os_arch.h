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

#ifndef _OS_ARCH_H
#define _OS_ARCH_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MODULE
    #include <linux/pci.h>
    #include <linux/spinlock.h>
    #include <linux/workqueue.h>
    #include <linux/interrupt.h>
    #include <linux/cdev.h>
    #include <asm/uaccess.h>
#else
    #include <stdint.h>
    #include <semaphore.h>
    #include <pthread.h>
//  #include <libaio.h>   // Do yourself a favor.  DON't include it!
    #include <linux/errno.h>
    #include <string.h>
#endif

#define LINUX 1
#if defined(__ia64__) || defined(__x86_64__) || defined(__alpha__) || defined(__powerpc64__)
#define OS_IS_64BIT
#endif    

//FIXME: use __x86_64__ to determine arch.



//----------------------------------------
// First things first, sort out the OS's
//----------------------------------------
#if defined (LINUX)
    #define OS_IS_LINUX             1
#elif defined (IPC_S6DB) || define (IPC_S55DB) || defined (JAMAICA)
    #define OS_IS_STRETCH           1
#else
    #define OS_IS_WINDOWS           1
#endif


//----------------------------------------
// Misc. definitions.
//----------------------------------------
#if defined (OS_IS_LINUX)
    #define OS_STATUS_SUCCESS           0
    #define OS_STATUS_FAILURE           -1
    #define OS_STATUS_NO_RESOURCE       -ENODATA
    #define OS_STATUS_PENDING           -EIOCBQUEUED
    #define OS_STATUS_PENDING_COPYBACK  -EIOCBRETRY
    #define OS_STATUS_TYPE_MISMATCH     -EINVAL
    #define OS_STATUS_NO_PERMISSION     -ENOPERM
    #define OS_STATUS_INVALID_PTR       -EINVAL
    #define OS_STATUS_INVALID_DEV       -EINVAL
    #define OS_STATUS_NO_MEMORY         -ENOMEM
    #define OS_STATUS_IO_FAILURE        -EIO
    #define OS_STATUS_IO_BUSY           -EBUSY
    #define OS_STATUS_BAD_ADDR          -EFAULT
    #define OS_STATUS_CANCELLED         -EPIPE

    typedef size_t                      os_size_t;
    typedef ssize_t                     os_ssize_t;
    typedef void                       *os_voidp_t;
    typedef uint32_t                    os_ulong_t;
    typedef int                         os_status_t;

    #define OS_BZERO(ptr,len)           memset(ptr,0,len)
#else
#endif



//----------------------------------------
// 32/64-bit overloading (Kernel or Userspace)
//----------------------------------------
#if defined (OS_IS_LINUX)
    #if defined (OS_IS_64BIT)
        typedef uint64_t                os_uint_arch_t;
        #define OS_32BIT_PTR(n)         uint32_t
        #define OS_32BIT_REF(n)         uint32_t
    #else
        typedef uint32_t                os_uint_arch_t;
        #define OS_32BIT_PTR(n)         n *
        #define OS_32BIT_REF(n)         n
    #endif
#else
#endif



#ifndef MODULE
//========================================
// These are definitions for user space apps
//========================================

//----------------------------------------
// User-space Debug/Trace print
//
// (x)OS_PRINTF(_RAW)... add/del "x" and "z" for convenient 
// removal/insertion "_RAW" doesn't print line header.
//
// Generally,
//    OS_PRINTF... is for error msgs and is disabled
//   iOS_PRINTF... is for info and is enabled
//   xOS_PRINTF... is trace debug, and is disabled
//   zOS_PRINTF... is driver info and is enabled by default.
//
// Easy tip:
//   If you want to see stuff in the code appear
//   on the console, put a "z" in front of the
//   OS_PRINTF... statement.
//   That way you don't have to muck around in here.
//   Maybe more variations for stderr.  TODO.
//----------------------------------------
#if defined (OS_IS_LINUX) || defined (OS_IS_WINDOWS)
    #define  OS_PRINTF(str, ...)
    #define zOS_PRINTF(str, ...)
    #define xOS_PRINTF(stf, ...)
    #define iOS_PRINTF(str, ...) \
        printf("%s: " str, __FUNCTION__, ##__VA_ARGS__)
    #define  OS_PRINTF_RAW(str, ...)
    #define zOS_PRINTF_RAW(str, ...)
    #define xOS_PRINTF_RAW(str, ...)
    #define iOS_PRINTF_RAW(str, ...) \
        printf((str), ##__VA_ARGS__)
#else
#endif



//----------------------------------------
// Misc. definitions.
//----------------------------------------
#if defined (OS_IS_LINUX)
    typedef int                 os_ufilehandle_t;
    #define OS_UGET_LAST_ERROR(ol) \
                                aio_error(ol);
#else
#endif



//----------------------------------------
// Events, Mutexes, Semaphores, etc.
//----------------------------------------
#if defined (OS_IS_LINUX)
    typedef sem_t               os_uevent_t;
    typedef pthread_mutex_t     os_umutex_t;
    typedef sem_t               os_usemaphore_t;
    typedef struct io_context  *os_uasyncio_context_t;
    typedef struct io_event     os_uasyncio_event_t;
    typedef struct iocb         os_uasyncio_t;

    #define OS_UFILEHANDLE_INVALID(fh) \
                                ((fh) == -1)
    #define OS_UFILEHANDLE_VALID(fh) \
                                ((fh) != -1)
    #define OS_UFILEHANDLE_INVALIDATE(fh) \
                                (fh) = -1

    #define OS_UEVENT_INIT(ev)
    #define OS_UEVENT_CREATE(ev) \
                                sem_init(&(ev),0,0)
    #define OS_UEVENT_WAIT(ev)  sem_wait(&(ev))
    #define OS_UEVENT_WAKE(ev)  sem_post(&(ev))
    #define OS_UEVENT_DESTROY(ev) \
                                sem_destroy(&(ev))

    #define OS_USEMAPHORE_INIT(sem)
    #define OS_USEMAPHORE_CREATE(sem,num) \
                                sem_init(&(sem),0,(num));
    #define OS_USEMAPHORE_WAIT(sem) \
                                sem_wait(&(sem))
    #define OS_USEMAPHORE_TRYWAIT(sem) \
                                sem_trywait(&(sem))
    #define OS_USEMAPHORE_POST(sem) \
                                sem_post(&(sem))
    #define OS_USEMAPHORE_DESTROY(sem) \
                                sem_destroy(&(sem))

    #define OS_UMUTEX_INIT(m)
    #define OS_UMUTEX_CREATE(m) pthread_mutex_init(&(m),NULL)
    #define OS_UMUTEX_LOCK(m)   pthread_mutex_lock(&(m))
    #define OS_UMUTEX_UNLOCK(m) pthread_mutex_unlock(&(m))
    #define OS_UMUTEX_DESTROY(m) \
                                pthread_mutex_destroy(&(m))
#else
#endif



//----------------------------------------
// libaio overload
//
// libaio version 3.106-5 (and probably similar variants)
// is broken (on x86_64 Ubuntu 7.10 Kernel 2.6.22-14 and
// probably also on similar variants) The code is fine, 
// but the binary form that comes with the user-side 
// package won't seem to run as it is
// compiled elsewhere (probably with the wrong "syscall"
// definition).  libaio is merely a thin API allowing user-side
// access to the Kernel-side AIO functionality anyway.
// (in fact, the below definitions essentially implement
//  the entire libaio functionality except for the io_queue_XX
//  conveneince wrapper routines)
// Extracting the direct in-line assembly
// calls from the library source and compiling them
// locally does, however, work.  Therefore, the libaio
// stuff is wrapped by the "os_" macros in os_arch.h.
//----------------------------------------
#if defined (OS_IS_LINUX)
    #if defined (OS_LINUX_AIO_IS_BROKEN)
        typedef void (*io_callback_t)(io_context *ctx,
                                      struct iocb *iocb,
                                      long res,
                                      long res2);

        typedef enum io_iocb_cmd {
            IO_CMD_PREAD    = 0,
            IO_CMD_PWRITE   = 1,
            IO_CMD_FSYNC    = 2,
            IO_CMD_FDSYNC   = 3,
            IO_CMD_PREADX   = 4, // experimental
            IO_CMD_POLL     = 5, // experimental
            IO_CMD_NOOP     = 6,
            IO_CMD_PREADV   = 7,
            IO_CMD_PWRITEV  = 8,
        } io_iocb_cmd_t;

        struct iocb {
            // these are internal to the kernel (2.6.22)
            uint64_t    aio_data;       // Callback addr

            uint32_t    aio_key;
            uint32_t    aio_reserved1;

            uint16_t    aio_lio_opcode;
            int16_t     aio_reqprio;
            uint32_t    aio_fildes;     // File descriptor

            uint64_t    aio_buf;        // Buffer pointer

            uint64_t    aio_nbytes;     // Size of data to move

            int64_t     aio_offset;     // Offset info

            uint64_t    aio_reserved2;

            uint32_t    aio_flags;
            uint32_t    aio_resfd;
        }; // 64 bytes

        struct io_event {
            uint64_t       data;        // Context.
            uint64_t       obj;
            int64_t        res;         // Result for aio_read
            int64_t        res2;
        };

        static inline void
        io_prep_pwrite(struct iocb *iocb,
                       uint32_t     fd,
                       void        *buf,
                       uint64_t     count,
                       int64_t      offset)
        {
            OS_BZERO(iocb, sizeof(struct iocb));
            iocb->aio_fildes = fd;
            iocb->aio_lio_opcode = IO_CMD_PWRITE;
            iocb->aio_reqprio = 0;
            iocb->aio_buf = (uint64_t)buf;
            iocb->aio_nbytes = count;
            iocb->aio_offset = offset;
        }

        static inline void
        io_set_callback(struct iocb *iocb,
                        io_callback_t cb)
        {
            iocb->aio_data = (uint64_t)cb;
        }


        #define __syscall_clobber   "r11","rcx","memory"
        #define __syscall           "syscall"

        #define io_setup_NR         206
        #define io_destroy_NR       207
        #define io_getevents_NR     208
        #define io_submit_NR        209
        #define io_cancel_NR        210

        #define os_io_setup(mx,cp) \
                                ({ \
                                    long __res; \
__asm__ volatile (__syscall \
	: "=a" (__res) \
	: "0" (io_setup_NR),"D" ((long)(mx)),"S" ((long)(cp)) : __syscall_clobber ); \
                                    __res; \
                                })

        #define os_io_destroy(cx) \
                                ({ \
                                    long __res; \
__asm__ volatile (__syscall \
	: "=a" (__res) \
	: "0" (io_destroy_NR),"D" ((long)(cx)) : __syscall_clobber ); \
                                    __res; \
                                })

        #define os_io_submit(cx,nr,cb) \
                                ({ \
                                    long __res; \
__asm__ volatile (__syscall \
	: "=a" (__res) \
	: "0" (io_submit_NR),"D" ((long)(cx)),"S" ((long)(nr)), \
      "d" ((long)(cb)) : __syscall_clobber); \
                                    __res; \
                                })

        #define os_io_getevents(cx,mn,nr,ev,to) \
                                ({ \
                                    long __res; \
__asm__ volatile ("movq %5,%%r10 ; movq %6,%%r8 ; " __syscall \
	: "=a" (__res) \
	: "0" (io_getevents_NR),"D" ((long)(cx)),"S" ((long)(mn)), \
	  "d" ((long)(nr)),"g" ((long)(ev)),"g" ((long)(to)) : \
	__syscall_clobber,"r8","r10" ); \
                                    __res; \
                                })

        #define os_io_cancel(cx,cb,ev) \
                                ({ \
                                    long __res; \
__asm__ volatile (__syscall \
	: "=a" (__res) \
	: "0" (io_cancel_NR),"D" ((long)(cx)),"S" ((long)(cb)), \
	  "d" ((long)(ev)) : __syscall_clobber); \
                                    __res; \
                                })

    #else
#include <libaio.h>
        #define os_io_setup(mx,cp) \
                                io_setup(mx,cp)

        #define os_io_destroy(cx) \
                                io_destroy(cx)

        #define os_io_submit(cx,nr,cb) \
                                io_submit(cx,nr,cb)

        #define os_io_getevents(cx,mn,nr,ev,to) \
                                io_getevents(cx,mn,nr,ev,to)

        #define os_io_cancel(cx,cb,ev) \
                                io_cancel(cx,cb,ev)

    #endif
#else
#endif




#else // MODULE
//========================================
// All that follows is for drivers
// in kernel-land only
//========================================

//----------------------------------------
// Some OS-dependent definitions
//----------------------------------------
#if defined (OS_IS_LINUX)
    #define SX_QUEUE_ERR_NONE       0
    #define SX_QUEUE_ERR_PARMS      EINVAL
    #define SX_QUEUE_ERR_ALIGN      EFAULT
    #define SX_QUEUE_ERR_EXISTS     EEXIST
    #define SX_QUEUE_ERR_NOTFOUND   ENOENT
    #define SX_QUEUE_ERR_EMPTY      ENODATA

    #define OS_QUEUE_ALIGN(n)       __attribute__ ((aligned(n)))
    #define OS_INLINE               static inline
    #define OS_DRIVERNAME           "s6stretch"
#else
#endif



//----------------------------------------
// Data-type overloads
//----------------------------------------
#if defined (OS_IS_LINUX)
    typedef char                    os_string_t[1024];
    typedef dma_addr_t              os_physical_address_t;
#if defined (OS_IS_64BIT)
    #define OS_HI_DWORD(pa)         (uint32_t)(((pa)>>32) & 0x00000000ffffffff)
    #define OS_LO_DWORD(pa)         (uint32_t)((pa)       & 0x00000000ffffffff)
#else
    #define OS_HI_DWORD(pa)         0
    #define OS_LO_DWORD(pa)         (uint32_t)((pa))
#endif    
#else
#endif



//----------------------------------------
// Kernel Debug/Trace print
//
// (x)OS_PRINTK(_RAW)... add/del "x" and "z" for convenient 
// removal/insertion "_RAW" doesn't print line header.
//
// Generally,
//    OS_PRINTK... is for error msgs and is disabled
//   xOS_PRINTK... is trace info, and is disabled
//   zOS_PRINTK... is driver debug and is disabled
//   iOS_PRINTK... is for loading info and is enabled
//
// Easy tip:
//   If you want to see stuff in the code appear
//   in /var/log/kern.log (or similar),
//   put a "z" in front of the OS_PRINTK...statement.
//   That way you don't have to muck around in here.
//----------------------------------------
#if defined (OS_IS_LINUX)
    #define  OS_PRINTK(str, ...)
    #define xOS_PRINTK(str, ...)
    #define zOS_PRINTK(str, ...)
    #define iOS_PRINTK(str, ...) \
        printk("%s[%s]: " str, OS_DRIVERNAME, __FUNCTION__, ##__VA_ARGS__)
    #define  OS_PRINTK_RAW(str, ...)
    #define xOS_PRINTK_RAW(str, ...)
    #define zOS_PRINTK_RAW(str, ...)
    #define iOS_PRINTK_RAW(str, ...) \
        printk((str), ##__VA_ARGS__)
#else
#endif



//----------------------------------------
// Memory
//----------------------------------------
#if defined (OS_IS_LINUX)
//    #define OS_SWAP32(n)        (n)
    #define OS_PHYS2CACHED(offset, baseaddr) \
                                    ((offset) + (baseaddr))
    #define OS_CACHED2PHYS(addr, baseaddr) \
                                    ((addr) - (baseaddr))
    #define OS_BOARD2HOST_PTR(addr,offs) \
                                    OS_PHYS2CACHED \
                                      ((os_uint_arch_t)(le32_to_cpu(addr)), \
                                       (os_uint_arch_t)offs)
    #define OS_HOST2BOARD_PTR(addr,offs) \
                                    cpu_to_le32 \
                                      ((uint32_t) \
                                        (OS_CACHED2PHYS((os_uint_arch_t)addr, \
                                                        (os_uint_arch_t)offs)))

    #define OS_BZERO_MMIO(ptr,len) \
                                { \
                                    int ndx; \
                                    for (ndx = 0; ndx < len; ndx++) { \
                                        writeb(0, ptr + len); \
                                    } \
                                }
    #define OS_COPY_TO_USER(u,k,sz) \
                                copy_to_user((u),(k),(sz))
    #define OS_COPY_TO_USER_IOVEC(v,k,sz,c) \
                                ({ \
                                    uint32_t seg, thislen, total; \
                                    for (seg = 0, total = (sz); \
                                         (seg < (c)) && (total > 0); \
                                         seg++) { \
                                        thislen = (v)[seg].iov_len < total ? \
                                                    (v)[seg].iov_len : \
                                                    total; \
                                        if (copy_to_user \
                                             ((v)[seg].iov_base, \
                                              ((void *)(k)) + \
                                                ((sz) - total), \
                                              thislen)) { \
                                            break; \
                                        } \
                                        total -= thislen; \
                                    } \
                                    total ? 1 : 0; \
                                })
                                
    #define OS_COPY_FROM_USER(k,u,sz) \
                                copy_from_user((k),(u),(sz))
    #define OS_COPY_FROM_USER_IOVEC(k,v,sz,c) \
                                ({ \
                                    uint32_t seg, total; \
                                    for (seg = 0, total = (sz); \
                                         (seg < (c)) && (total > 0); \
                                         seg++) { \
                                        if (copy_from_user \
                                              (((void *)(k)) + \
                                                 ((sz) - total), \
                                               (v)[seg].iov_base, \
                                               (v)[seg].iov_len)) { \
                                            break; \
                                        } \
                                        total -= (v)[seg].iov_len; \
                                    } \
                                    total ? 1 : 0; \
                                })
                                         
    #define OS_KMALLOC(sz,xx)   kzalloc(sz,GFP_KERNEL)
    #define OS_KFREE(bf)        kfree(bf)
    #define OS_CONTAINER_OF(s,t,r) \
                                container_of(s,t,r)
    #define DCACHE_INV_LINE( addr )           // Invalidate cache line
    #define DCACHE_WBI_LINE( addr )           // Writeback and invalidate cache line
    #define DCACHE_INV_REGION( base, size )   // Invalidate cache region
    #define DCACHE_WBI_REGION( base, size )   // Write back and invalidate cache region
    #define SYNC
#else
#endif



//----------------------------------------
// Spinlocks
//----------------------------------------
#if defined (OS_IS_LINUX)
    typedef spinlock_t              os_spinlock_t;
    #define OS_SPINLOCK_INIT(sl) \
                        spin_lock_init(sl)
    #define OS_SPINLOCK(sl) \
                        /* spin_lock_irqsave(sl) ?????????????  */ \
                        spin_lock(sl)
    #define OS_SPINUNLOCK(sl) \
                        spin_unlock(sl)
    #define OS_QUEUE_MUTEX_INIT
    #define OS_QUEUE_MUTEX_GET(addr) \
                        do { \
                            sx_queue_crit_section_enter(addr, 1); \
                        } while (0)
    #define OS_QUEUE_MUTEX_FREE(addr) \
                        do { \
                            sx_queue_crit_section_exit(addr, 1); \
                        } while (0)
#else
#endif



//----------------------------------------
// Lists
//----------------------------------------
#if defined (OS_IS_LINUX)
#include <linux/list.h>    
    typedef struct os_list_struct
    {
        struct list_head    list;
        os_spinlock_t       lock;
    } os_list_t;
    typedef struct list_head    os_list_node_t;
    #define OS_LIST_INIT(li)    {\
                                    INIT_LIST_HEAD( &((li)->list) );\
                                    OS_SPINLOCK_INIT(&((li)->lock));\
                                }
    //                                  list_add_tail(&((li)->list),ln);
    #define OS_LIST_ADD_TAIL(ln,li) {\
                                      OS_SPINLOCK(&((li)->lock));\
                                      (ln)->next = &((li)->list);\
                                      (ln)->prev=(li)->list.prev;\
                                      (li)->list.prev = ln;\
                                      (ln)->prev->next=ln;\
                                      OS_SPINUNLOCK(&((li)->lock));\
                                     }
    #define OS_LIST_IS_EMPTY(li)    list_empty(&((li)->list))
    #define OS_LIST_REMOVE_ELEMENT(ln,li)  {\
                                              OS_SPINLOCK(&((li)->lock));\
                                              list_del(ln);\
                                              OS_SPINUNLOCK(&((li)->lock));\
                                            }
    #define OS_LIST_RETREIVE_HEAD(li, rm)   ({\
                                            os_list_node_t *temp;\
                                            OS_SPINLOCK(&((li)->lock));\
                                            if( list_empty(&((li)->list)))\
                                            {\
                                                temp = NULL;\
                                            }\
                                            else {\
                                                temp = (li)->list.next;\
                                                if (rm) list_del(temp);\
                                            }\
                                            OS_SPINUNLOCK(&((li)->lock));\
                                            temp;\
                                          })

    #define OS_LIST_ITER_INIT(li) \
                                os_list_node_t * itr=(li)->list.next;\
                                os_list_node_t * itr_li=&((li)->list);
    #define OS_LIST_ITER_NEXT   ({os_list_node_t * temp=(itr==itr_li)?NULL:itr;itr=itr->next;temp;})
    #define OS_LIST_ITER_CLEANUP 
#if 0    
    typedef struct klist        os_list_t;
    typedef struct klist_node   os_list_node_t;

    // Initialize a list head (li:os_list_t*)
    #define OS_LIST_INIT(li)    klist_init((li),NULL,NULL) // INIT_LIST_HEAD((li))

    // Add node (ln:os_list_node_t*) to list (li:os_list_t*)
    // while spinlocked with (lo:os_spinlock_t*)
    #define OS_LIST_ADD_TAIL(ln,li) \
                                klist_add_tail(ln,li)

    #define OS_LIST_IS_EMPTY(li) \
                                list_empty(&(li)->k_list)

    // Remove node (ln) from whatever list it is in.
    #define OS_LIST_REMOVE_ELEMENT(ln) \
                                   klist_del((ln))

    // Take node from list(li:os_list_t*).
    //  Spinlocks handled by klist functionality
    // returns os_list_node_t*
    #define OS_LIST_RETREIVE_HEAD(li,rm) \
                                ({ \
                                    struct list_head *temp_node = \
                                      (li)->k_list.next; \
                                    OS_LIST_IS_EMPTY((li)) ? \
                                        NULL : \
                                        ({ \
                                            struct klist_node *temp_knode = \
                                              container_of(temp_node, \
                                                           struct klist_node, \
                                                           n_node); \
                                            if ((rm)) { \
                                                OS_LIST_REMOVE_ELEMENT(temp_knode); \
                                            } \
                                            temp_knode; \
                                        }); \
                                })

    // The list iterator
    // Use these within the same function!
    #define OS_LIST_ITER_INIT(li) \
                                struct klist_iter itr; \
                                klist_iter_init((li), &itr)
    #define OS_LIST_ITER_NEXT   klist_next(&itr)
    #define OS_LIST_ITER_CLEANUP \
                                klist_iter_exit(&itr)
#endif    
#else
#endif


//----------------------------------------
// WORKQUEUES.  Dispatched from low-level ISR.
//----------------------------------------
#if defined (OS_IS_LINUX)
    #define OS_WORKQUEUE_POOL_SIZE  32
    typedef struct {
        struct work_struct  item;
        void               *context;
    } os_workitem_t;

    typedef struct {
        struct workqueue_struct *queue;
        os_workitem_t            pool[OS_WORKQUEUE_POOL_SIZE];
        int                      pos;
    } os_workpool_t;

    #define OS_WORKQUEUE_INIT(wq,nm) \
                                wq = create_singlethread_workqueue((nm))
    #define OS_WORKQUEUE_DESTROY(wq) \
                                destroy_workqueue((wq))
#ifdef KVER_LESS_2620
    #define OS_WORKQUEUE_INIT_ITEM(wi,fn) \
                                INIT_WORK((wi),(fn),(wi))
#else    
    #define OS_WORKQUEUE_INIT_ITEM(wi,fn) \
                                INIT_WORK((wi),(fn))
#endif    
    #define OS_WORKQUEUE_QUEUE_ITEM(wq,wi) \
                                queue_work((wq),(wi))
    #define OS_WORKQUEUE_CANCEL_WORK(wi) \
                                cancel_delayed_work((wi))
    #define OS_WORKQUEUE_FLUSH_QUEUE(wq) \
                                flush_workqueue((wq))
#else
#endif



#if defined (OS_IS_LINUX)
    //----------------------------------------
    // The pending queues a.k.a. Cancel-safe queues
    //----------------------------------------
    #define OS_PEND_STATE_BEGIN_IO              0
    #define OS_PEND_STATE_PENDING               1
    #define OS_PEND_STATE_PENDING_COPYBACK      2
    #define OS_PEND_STATE_AWAITING_COMPLETION   3
    #define OS_PEND_STATE_DONE                  4

    // Our very own Irp.  Attaches to (k)iocb->private
    struct dma_buf_node_struct;
    struct dma_channel_node_struct;
    typedef struct
    {
        // Thread list entry - allows queueing the IRP to the thread pending I/O
        // request packet list.
        os_list_node_t                   list_node;

        // The Kernel IOCB back pointer.
        // Describes the asynchronous operation to OS.
        struct kiocb                    *iocb;

        // The buffers (Assumed, for now, to be 1 segment.)
        // If more than one use :const struct iovec *iovec;
        // and alloc the chain.
        void                     __user *user_buffer;
        void                            *system_buffer;

        // Dynamic IOCTL Action Command
        // Comes from "offset" parm in read and write calls.
        // May have embedded info, such as port value
        int64_t                          ioctl_cmd;
        uint8_t                          iotype;
        uint8_t                          iodir;

        // Total (requested) size of data
        // (stat_size contains result size)
        size_t                           size;

        // Context of operation
        uint32_t                         context;

        // Message class (if needed)
        uint32_t                         class_id;

        // Port of operation
        uint32_t                         port;

        // Command id (from msg)
        uint32_t                         request_id;

        // Dma buffer (for BufferSend/StoreReceive buffer)
        struct dma_buf_node_struct      *dma_buf;

        // Dma channel for WriteMgmtMsg
        struct dma_channel_node_struct  *dma_channel;

        // Ongoing status of operation (ultimately the size)
        // Negative value is error code, pos value is size of xfer.
        os_status_t                      stat_size;

        // State of this operation (controls callbacks, etc.)
        uint32_t                         pend_state;

        // Spinlock to protect completions
        os_spinlock_t                    spinlock;

        // mark whether this irp is cancelled
        int                              cancel;
        long                             start_time;
        long                             msg_time;
        long                             dma_time;
    } os_irp_t;

    typedef os_list_t   os_pendqueue_t;

    #define OS_PENDQUEUE_INIT(wq) \
                                OS_LIST_INIT(wq)

    #define OS_PENDQUEUE_INSERT(ir,wq,cx) \
                                (ir)->context = (cx); \
                                OS_LIST_ADD_TAIL(&((ir)->list_node),wq)

    #define OS_PENDQUEUE_RETREIVE_CONTEXT(wq,cx,rm) \
                                ({ \
                                    os_list_node_t *list_node; \
                                    os_irp_t       *this_irp; \
                                    OS_LIST_ITER_INIT(wq); \
                                    while((list_node = OS_LIST_ITER_NEXT) != NULL) { \
                                        this_irp = \
                                          OS_CONTAINER_OF(list_node, \
                                                          os_irp_t, \
                                                          list_node); \
                                        if ((cx) == this_irp->context) { \
                                            break; \
                                        } \
                                    } \
                                    OS_LIST_ITER_CLEANUP; \
                                    if (list_node != NULL) { \
                                        if ((rm)) { \
                                            OS_LIST_REMOVE_ELEMENT(list_node,wq); \
                                        } \
                                    } else { \
                                        this_irp = NULL; \
                                    } \
                                    this_irp; \
                                })

    #define OS_PENDQUEUE_RETREIVE_NEXT(wq,rm) \
                                ({ \
                                    os_list_node_t *list_node; \
                                    list_node = OS_LIST_RETREIVE_HEAD(wq,rm); \
                                    list_node != NULL ? \
                                      OS_CONTAINER_OF(list_node, \
                                                      os_irp_t, \
                                                      list_node) : \
                                      NULL; \
                                })
#else
#endif



//----------------------------------------
// Events
//----------------------------------------
#if defined (OS_IS_LINUX)
    // Completion events
    typedef struct completion   os_event_t;
    #define OS_EVENT_INIT(ev)   init_completion(ev)
    #define OS_EVENT_WAIT(ev)   wait_for_completion(ev)
    #define OS_EVENT_WAIT_TIMEOUT(ev,to)   wait_for_completion_timeout(ev,to)
    #define OS_EVENT_WAKE(ev)   complete(ev)
#else
#endif


//----------------------------------------
// Tasks (and Threads)
// Threads run continuously, Tasklets
// dont suspend, and run till they finish,
// and are dispatched from the isr.
//----------------------------------------
#if defined (OS_IS_LINUX)
    typedef struct task_struct      os_thread_t;
    #define OS_THREAD_RUN(fn,dt,na) \
                                kthread_run(fn,dt,na)
    #define OS_THREAD_STOPPED   kthread_should_stop()
    #define OS_THREAD_WAKE(th)  wake_up_process(th)
    #define OS_THREAD_STOP(th)  kthread_stop(th)

    #define OS_TASKLET_DISABLE(tk)
    #define OS_TASKLET_ENABLE(tk)
#else
#endif



//----------------------------------------
// Time delays
//----------------------------------------
#if defined (OS_IS_LINUX)
    // BEWARE OF SIGNALS!!!!!!
    #define OS_DELAY_MS(ms,mo)  /* mo ignored */ \
                                set_current_state(TASK_INTERRUPTIBLE); \
                                schedule_timeout(msecs_to_jiffies((ms))); // or HZ/100
#else
#endif



//----------------------------------------
// Misc
//----------------------------------------
static inline
int
os_is_success(os_status_t status)
{
    return (status == OS_STATUS_SUCCESS);
}

static inline
int
os_is_failure(os_status_t status)
{
    return (status != OS_STATUS_SUCCESS);
}



//----------------------------------------
// Read/write PCI header of board (byte,word,dword)
//----------------------------------------
static inline
void
os_read_pci_config_space_byte(int       offset,
                              uint8_t  *val,
                              void     *pci_device)
{
    #if defined (OS_IS_LINUX)
        pci_read_config_byte((struct pci_dev *)pci_device,
                             offset,
                             val);
    #else
    #endif
}

static inline
void
os_read_pci_config_space_word(int       offset,
                              uint16_t *val,
                              void     *pci_device)
{
    #if defined (OS_IS_LINUX)
        pci_read_config_word((struct pci_dev *)pci_device,
                             offset,
                             val);
    #else
    #endif
}

static inline
void
os_read_pci_config_space_dword(int       offset,
                               uint32_t *val,
                               void     *pci_device)
{
    #if defined (OS_IS_LINUX)
        pci_read_config_dword((struct pci_dev *)pci_device,
                             offset,
                             val);
    #else
    #endif
}


static inline
void
os_write_pci_config_space_byte(int      offset,
                               uint8_t  val,
                               void    *pci_device)
{
    #if defined (OS_IS_LINUX)
        pci_write_config_byte((struct pci_dev *)pci_device,
                             offset,
                             val);
    #else
    #endif
}

static inline
void
os_write_pci_config_space_word(int      offset,
                               uint16_t val,
                               void    *pci_device)
{
    #if defined (OS_IS_LINUX)
        pci_write_config_word((struct pci_dev *)pci_device,
                             offset,
                             val);
    #else
    #endif
}

static inline
void
os_write_pci_config_space_dword(int      offset,
                                uint32_t val,
                                void    *pci_device)
{
    #if defined (OS_IS_LINUX)
        pci_write_config_dword((struct pci_dev *)pci_device,
                             offset,
                             val);
    #else
    #endif
}
#endif // MODULE

#ifndef __H_SXTYPES
#define __H_SXTYPES
  typedef unsigned char             sx_uint8;    // 8  bit unsigned integer
  typedef unsigned short            sx_uint16;   // 16 bit unsigned integer
  typedef unsigned int              sx_uint32;   // 32 bit unsigned integer
  typedef unsigned long             sx_uint64;   // 64 bit unsigned integer //TODO: verify long long
  typedef signed char               sx_int8;     // 8  bit signed integer
  typedef signed short              sx_int16;    // 16 bit signed integer
  typedef signed int                sx_int32;    // 32 bit signed integer
  typedef signed long               sx_int64;    // 64 bit signed integer // TODO: verify long long

  // Boolean type
  //typedef BOOL                    sx_bool;
  typedef int                       sx_bool;
#endif

#ifdef __cplusplus
}
#endif

#endif //_OS_ARCH_H


