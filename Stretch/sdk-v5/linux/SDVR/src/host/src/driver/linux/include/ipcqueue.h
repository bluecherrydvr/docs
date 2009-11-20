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

#ifndef _IPCQUEUE_H
#define _IPCQUEUE_H

#ifdef __cplusplus
extern "C" {
#endif


//========================================
// Define types
//========================================

// Message header
struct _sx_queue_datahdr_struct;
typedef OS_32BIT_PTR(struct _sx_queue_datahdr_struct) sx_queue_datahdr_tp;

struct _sx_queue_datahdr_struct
{
  sx_queue_datahdr_tp queue_data_nxt;
};

typedef struct _sx_queue_datahdr_struct     sx_queue_datahdr_t;

// Software semaphore structure
typedef struct sx_queue_sem_t
{
	sx_uint32	  flag[2];
	sx_uint32	  turn;
} sx_queue_sem;


#ifdef MODULE
// Only for the driver's eyes

// Queue structure (must be 32 byte aligned)
typedef void                                    (*sx_callback_t)(uint32_t);
typedef OS_32BIT_REF(sx_callback_t)             sx_callback_tp;

struct  _sx_queue_struct;
typedef OS_32BIT_PTR(struct _sx_queue_struct)   sx_queue_tp;
typedef OS_32BIT_PTR(uint32_t)                  sx_uint32_tp; 

struct _sx_queue_struct
{
    sx_queue_datahdr_tp      data_last;      // 4 byte
    sx_queue_datahdr_tp      data_first;     // 4 byte
    sx_callback_tp           callback;       // 4 byte
    uint32_t                 param;          // 4 byte
    sx_queue_tp              queue_nxt;      // 4 byte
    uint32_t                 id;             // 4 byte
    sx_uint32_tp             status;         // 4 byte
    uint8_t                  bitpos;         // 1 byte
    uint8_t                  options;        // 1 byte
    sx_uint8                 dir;          // 1 byte
} OS_QUEUE_ALIGN(32);

typedef struct _sx_queue_struct                 sx_queue_t;

#define SX_QUEUE_DESCR_SIZE   (sizeof(sx_queue))
#define SX_QUEUE_MGMT_NEVER_FULL
#define SX_QUEUE_MGMT_QUEUE_SIZE    (1 + 256 + 256 + 1)
struct sx_queue_mgmt_struct
{
    sx_queue_t                        base_queue;
    sx_queue_datahdr_tp               mgmt_queue[SX_QUEUE_MGMT_QUEUE_SIZE];
} OS_QUEUE_ALIGN(32);
typedef struct sx_queue_mgmt_struct sx_queue_mgmt;

struct sx_queue_mod_rsc_struct
{
	sx_queue_tp      queue;
	sx_uint32	  init_done;
    sx_queue_sem  sem[2];   // sem[0] = board2host, sem[1]=host2board
};
typedef struct sx_queue_mod_rsc_struct sx_queue_mod_rsc_t;

//========================================
// Local data describing shared queue header in
// top of shared memory (Board-side view)
//========================================
typedef struct
{
    volatile sx_queue_tp        __iomem *pp_queue;
    volatile sx_queue_mod_rsc_t __iomem *queue_mod_rsc; // Shared memory pointer
    uint8_t                     options;
    os_spinlock_t               spinlock;      // spin lock  

} sx_queue_mod_t;



//========================================
// Enter Shared memory critical section
//========================================
OS_INLINE
void sx_queue_crit_section_enter(volatile sx_queue_sem __iomem *smem,
                                 uint32_t                             id)
{
    volatile uint32_t __iomem *turn        = &smem->turn;
    volatile uint32_t __iomem *others_flag = &smem->flag[!id];


    smem->flag[id] = 1;
    *turn = id;

    while (*others_flag && (*turn == id))
        ;
}



//========================================
// Exit Shared memory critical section
//========================================
OS_INLINE
void sx_queue_crit_section_exit(volatile sx_queue_sem __iomem *smem,
                                uint32_t                             id)
{
    smem->flag[id] = 0;
}




// Adjustment of physical addressess due to endianness
#ifdef EB
 #define SWAP_END32(n)    SWAP32((uint32_t)(n))
#else
 #ifdef EL
  #define SWAP_END32(n)    ((uint32_t)(n))
 #else
  #define SWAP_END32(n)    ((uint32_t)(n))
//#error "No Endianness specified"
 #endif
#endif

#endif // MODULE


#ifdef __cplusplus
}
#endif

#endif // _IPCQUEUE_H


