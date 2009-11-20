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

#ifndef _BOARD_ARCH_H
#define _BOARD_ARCH_H

/*
   Memory shared by the board and the host is allocated by the board during
   sct_init(). The host driver uses PCI BAR 1 to access the shared memory.
   The host must read the register at this offset in BAR 0. The value in the
   register will be the start address of the shared memory region, specified
   as an offset into BAR 1.

   bar1_offset = *(sx_uint32 *)(bar0_base + SCT_SHARED_MEM_INFO_REG);
   shared_mem_base = bar1_base + bar1_offset;
*/

#define SCT_SHARED_MEM_REG_OFFSET   0x000b0060  // offset into BAR0

// TODO:  Revisit how to reset S6
#define S6_INTERRUPT_OFFSET        0x00002754   // Host writes 1 to interrupt S6
#define HOST_INTERRUPT_OFFSET      0x00002758   // Host writes 0 to clear

//ProVideo products, add by Johnson.
#define PROVIDEO_PCI_VID	0x1540
#define PROVIDEO_PCI_PID_PV282	0x0002  //16 port
#define PROVIDEO_PCI_PID_PV281	0x0003	//8 port
#define PROVIDEO_PCI_PID_PV280	0x0004	//4 port
#define PROVIDEO_PCI_PID_PV283	0x0006	//16 port, 4x PCI-e

// PCI related
#define STRETCH_PCI_VENDOR_ID       0x18a2
#define STRETCH_PCI_DEVICE_ID       PROVIDEO_PCI_PID_PV282
#define STRETCH_PCI_SUB_VENDOR_ID   PROVIDEO_PCI_VID
#define STRETCH_PCI_SUB_DEVICE_ID   0x0000

#define SCT_MSG_DATA_SIZE           16

#define CHANNELS_PER_DEVICE         4
#define DEVICES_ON_BOARD            4


#endif // _BOARD_ARCH_H

