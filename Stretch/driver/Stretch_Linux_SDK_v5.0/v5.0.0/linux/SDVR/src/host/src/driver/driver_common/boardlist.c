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

#include "boardlist.h"

//#define STRETCH_PCI_VENDOR_ID	   0x18a2 //Johnson.
//#define STRETCH_PCI_SUB_VENDOR_ID  STRETCH_PCI_VENDOR_ID
//#define STRETCH_PCI_SUB_DEVICE_ID  0x0000

/*
** This list is created to accommodate the expanding configurations of 
** Stretch's PCI boards. It is used to determine the amount of memory
** needed to be mapped based on the number of channel counts. Since it
** is to be commonly shared between Windows' and Linux's driver, 
**            !!! PLEASE EDIT THIS WITH CARE !!! 
*/
#ifdef PPC
//For powerpc no raw video streaming is required
//the dma buffer size will be set to largest encoded frame size
#define YUV_420_SIZE    256*1024
#define YUV_422_SIZE    256*1024
#else
//YUV_420 size based on pal resolution , 720*576*1.5 roughly 620k
#define YUV_420_SIZE    620*1024
//YUV_422 size based on pal resolution , 720*576*2 roughly 811k
#define YUV_422_SIZE    811*1024
#endif

board_list_node_t stretch_board_list[] = 
{
    /* VRC6016 : 4xS6, 16 chan */
    { 
        STRETCH_PCI_VENDOR_ID,		/* PCI Vendor ID            */ 
        0x0002,				/* PCI Device ID            */
        STRETCH_PCI_SUB_VENDOR_ID,	/* PCI Sub Vendor ID        */
        STRETCH_PCI_SUB_DEVICE_ID,      /* PCI Sub Device ID        */
        4,				/* # of devices on board    */
    	4, 				/* # of channels per device */
    	8, 				/* # of send channels per board */
        9,                              /* # of buffers needed per channel*/
        YUV_420_SIZE                    /* dma buffer size */
    },
    /* VRC6008 : 2xS6, 8 chan */
    { 
        STRETCH_PCI_VENDOR_ID,		/* PCI Vendor ID     */ 
        0x0003,				/* PCI Device ID     */
        STRETCH_PCI_SUB_VENDOR_ID,      /* PCI Sub Vendor ID */ 
        STRETCH_PCI_SUB_DEVICE_ID,      /* PCI Sub Device ID */
        2,				/* # of devices on board    */
	4, 				/* # of channels per device */
    	8, 				/* # of send channels per board */
        9,                              /* # of buffers needed per channel*/
        YUV_420_SIZE                    /* dma buffer size */
    },
    /* VRC6004 : 1xS6, 4 chan */
    {  
        STRETCH_PCI_VENDOR_ID,          /* PCI Vendor ID     */
        0x0004,                         /* PCI Device ID     */
        STRETCH_PCI_SUB_VENDOR_ID,      /* PCI Sub Vendor ID */
        STRETCH_PCI_SUB_DEVICE_ID,      /* PCI Sub Device ID */
        1,                              /* # of devices on board    */
        4,                              /* # of channels per device */
    	8, 				/* # of send channels per board */
        9,                              /* # of buffers needed per channel*/
        YUV_420_SIZE                    /* dma buffer size */
    },
    /* VCC6416 : 1xS6, 16 chan capture, 4-lane PCIe */
    { 
        STRETCH_PCI_VENDOR_ID,		/* PCI Vendor ID     */ 
        0x0005,				/* PCI Device ID     */
        STRETCH_PCI_SUB_VENDOR_ID,      /* PCI Sub Vendor ID */ 
        STRETCH_PCI_SUB_DEVICE_ID,      /* PCI Sub Device ID */
        1,				/* # of devices on board    */
	16, 				/* # of channels per device */
    	8, 				/* # of send channels per board */
        6,                              /* # of buffers needed per channel*/
        YUV_422_SIZE                    /* dma buffer size */
    },
    /* VRC6416 : 4xS6, 16 chan, 4-lane PCIe */
    {  
        STRETCH_PCI_VENDOR_ID,          /* PCI Vendor ID     */
        0x0006,                         /* PCI Device ID     */
        STRETCH_PCI_SUB_VENDOR_ID,      /* PCI Sub Vendor ID */
        STRETCH_PCI_SUB_DEVICE_ID,      /* PCI Sub Device ID */
        4,                              /* # of devices on board    */
        4,                              /* # of channels per device */
    	8, 				/* # of send channels per board */
        9,                              /* # of buffers needed per channel*/
        YUV_420_SIZE                    /* dma buffer size */
    },
    /* VRC6416-ba : 4xS6, 16 chan, 4-lane PCIe (special) */
    {   
        STRETCH_PCI_VENDOR_ID,          /* PCI Vendor ID     */
        0x0007,                         /* PCI Device ID     */
        STRETCH_PCI_SUB_VENDOR_ID,      /* PCI Sub Vendor ID */
        STRETCH_PCI_SUB_DEVICE_ID,      /* PCI Sub Device ID */
        4,                              /* # of devices on board    */
        4,                              /* # of channels per device */
    	8, 				/* # of send channels per board */
        9,                              /* # of buffers needed per channel*/
        YUV_420_SIZE                    /* dma buffer size */
    },
    /* ID0008 (special) : 2xS6, 16 chan capture, 1-lane PCIe */
    {
        STRETCH_PCI_VENDOR_ID,          /* PCI Vendor ID            */
        0x0008,                         /* PCI Device ID            */
        STRETCH_PCI_SUB_VENDOR_ID,      /* PCI Sub Vendor ID        */
        STRETCH_PCI_SUB_DEVICE_ID,      /* PCI Sub Device ID        */
        2,                              /* # of devices on board    */
        8,                              /* # of channels per device */
    	8, 				/* # of send channels per board */
        6,                              /* # of buffers needed per channel*/
        YUV_420_SIZE                    /* dma buffer size */
    },
    /* ID0009 (special) : 2xS6, 16 chan capture, 4-lane PCIe */
    {
        STRETCH_PCI_VENDOR_ID,          /* PCI Vendor ID            */
        0x0009,                         /* PCI Device ID            */
        STRETCH_PCI_SUB_VENDOR_ID,      /* PCI Sub Vendor ID        */
        STRETCH_PCI_SUB_DEVICE_ID,      /* PCI Sub Device ID        */
        2,                              /* # of devices on board    */
        8,                              /* # of channels per device */
    	8, 				/* # of send channels per board */
        6,                              /* # of buffers needed per channel*/
        YUV_420_SIZE                    /* dma buffer size */
    },
    /* VRC6016C : 2xS6, 16chanCIF/8chanD1/16chanD1, 1-lane PCIe */
    {
        STRETCH_PCI_VENDOR_ID,          /* PCI Vendor ID            */
        0x000A,                         /* PCI Device ID            */
        STRETCH_PCI_SUB_VENDOR_ID,      /* PCI Sub Vendor ID        */
        STRETCH_PCI_SUB_DEVICE_ID,      /* PCI Sub Device ID        */
        2,                              /* # of devices on board    */
        8,                              /* # of channels per device */
        8,                              /* # of send channels per board */
        9,                              /* # of buffers needed per channel*/
        YUV_420_SIZE                    /* dma buffer size */
    },
    { 0, 0, 0, 0, 0, 0 }, /* end of list */
};

#define BOARD_LIST_COUNT  (sizeof(stretch_board_list)/sizeof(board_list_node_t))

