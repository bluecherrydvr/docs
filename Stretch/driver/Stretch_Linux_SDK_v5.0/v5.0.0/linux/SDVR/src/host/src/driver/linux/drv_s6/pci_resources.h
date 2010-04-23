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

#ifndef _PCI_RESOURCES_H
#define _PCI_RESOURCES_H


#define PCI_REG_PTR32(binf,off)  (volatile uint32_t __iomem *)((binf).kern_addr + (off))

//========================================
// Structure containing information about PCI BAR registers, as read
// from the bus.
//========================================
typedef struct
{
    uint64_t  phy_addr;
    void     *kern_addr;
    size_t    size;

} bar_info_t;



//========================================
// PCI data from the board init for the application to see via
// ioctl call
//========================================
typedef struct
{
    // OS structure
    struct pci_dev *pcidev;

    uint16_t   vendor_id;
    uint16_t   device_id;
    uint16_t   command_reg;
    uint16_t   status_reg;
    uint16_t   max_dma_buff;
    uint16_t   max_dma_send_buff;
    uint16_t   max_dma_recv_buff;
    uint32_t   mem_reserve_size;
    uint32_t   dma_buf_size;
    uint8_t    revision_id;
    uint32_t   class_code;
    uint16_t   subsystem_vendor_id;
    uint16_t   subsystem_id;
    uint8_t    interrupt_line;

    bar_info_t bar_info[SCT_MAX_PCI_BAR];

    //dma_buf_node_t  bar_info[SCT_MAX_PCI_BAR];
} pci_resources_t;



//========================================
// Prototypes
//========================================

// Detect PCI-oriented resources used by driver
os_status_t
pci_resources_init(pci_resources_t *pci_resources);

// Unmap PCI-oriented resources used by driver
void
pci_resources_fini(pci_resources_t *pci_resources,
                   int              cleanup_lev);


#endif // _PCI_RESOURCES_H

