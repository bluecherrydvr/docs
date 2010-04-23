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

#include <linux/interrupt.h>

#include "stretch_if.h"
#include "os_arch.h"
#include "board_arch.h"
#include "ipcqueue.h"
#include "ipcqueue_api.h"
#include "ipcmsg.h"
#include "pci_resources.h"
#include "dma_channels.h"
#include "shared_memory.h"
#include "ipcmsg.h"
#include "dispatch.h"
#include "msg_pool.h"
//#include "msg_queues.h"
#include "stretch_main.h"



//========================================
// Unmap PCI-oriented resources used by driver
//========================================
void
pci_resources_fini(pci_resources_t *pci_resources,
                   int              cleanup_lev)
{
    switch (cleanup_lev) {
    case CLEANUP_ALL:
    case 3:
        release_mem_region(pci_resources->bar_info[2].phy_addr,
                           pci_resources->bar_info[2].size);
    case 2:
        release_mem_region(pci_resources->bar_info[1].phy_addr,
                           pci_resources->bar_info[1].size);
    case 1:
        release_mem_region(pci_resources->bar_info[0].phy_addr,
                           pci_resources->bar_info[0].size);
    default:
        break;
    }
}



//========================================
// Detect PCI-oriented resources used by driver
//========================================
os_status_t
pci_resources_init(pci_resources_t *pci_resources)
{
    int                 ndx;
    uint32_t            temp32;
    stretch_dev_t      *stretch_dev;
    int                 cleanup_lev = 0;
    os_status_t         retval = OS_STATUS_SUCCESS;
    struct pci_dev     *pcidev = pci_resources->pcidev;

    do {
        //----------------------------------------
        // Enable the PCI device with the OS
        //----------------------------------------
        if (pci_enable_device(pci_resources->pcidev)) {
            OS_PRINTK("Failure to enable pci device\n");
            return OS_STATUS_IO_FAILURE;
        }

        //----------------------------------------
        // Give the driver a name from PCI information
        //----------------------------------------
        stretch_dev = pci_get_drvdata(pci_resources->pcidev);
        sprintf(stretch_dev->ifname,
                "stretch%4.4x[%d]",
                pci_resources->pcidev->device,
                0); // FIXME?  stretch_dev->nr);

        //----------------------------------------
        // Read all the standard header stuff directly into structure
        //----------------------------------------
        pcidev = pci_resources->pcidev;
        os_read_pci_config_space_word(PCI_COMMAND,
                                      &pci_resources->command_reg,
                                      pci_resources->pcidev);
        os_read_pci_config_space_word(PCI_STATUS,
                                      &pci_resources->status_reg,
                                      pci_resources->pcidev);
        os_read_pci_config_space_dword(PCI_CLASS_REVISION, &temp32,
                                      pci_resources->pcidev);
        pci_resources->revision_id = (uint8_t)(temp32 & 0x000000ff);
        pci_resources->class_code = (temp32 & 0xffffff00) >> 8;

        pci_resources->vendor_id = pcidev->vendor;
        pci_resources->device_id = pcidev->device;
        pci_resources->subsystem_vendor_id = pcidev->subsystem_vendor;
        pci_resources->subsystem_id = pcidev->subsystem_device;
        pci_resources->interrupt_line = pcidev->irq;

        // We have to skip every other bar.  Memory Resources currently
        // found at BAR0 (regs), BAR0(2meg) BAR4(128meg)
        // Pack it in so our bar_info_t has the good stuff in BAR 0,1,2 for
        // the rest of the driver to use.

        for (ndx = 0; ndx < SCT_MAX_PCI_BAR; ndx += 2) {
            pci_resources->bar_info[ndx/2].phy_addr =
              pci_resource_start(pci_resources->pcidev,ndx);
            pci_resources->bar_info[ndx/2].size =
              pci_resource_len(pci_resources->pcidev,ndx);
        }

        //---------------------------------------
        // Talk about it.
        //----------------------------------------
        iOS_PRINTK("\n\nPCI Details:\n\n    %s : %4.4x\n"
                                       "    %s : %4.4x\n"
                                       "    %s : %4.4x\n"
                                       "    %s : %4.4x\n"
                                       "    %s : %4.4x\n"
                                       "    %s : %4.4x\n"
                                       "    %s : %2.2x\n"
                                       "    %s : %2.2x\n"
                                       "    %s : %8.8x\n\n"
                                       "    %s : %llx\n"
                                       "    %s : %8.8x\n"
                                       "    %s : %llx\n"
                                       "    %s : %8.8x\n"
                                       "    %s : %llx\n"
                                       "    %s : %8.8x\n\n"
                                       "    %s : %2.2x\n"
                                       "    %s : %2.2x\n"
                                       "    %s : %2.2x\n\n",
                  "Vendor ID", pci_resources->vendor_id,
                  "Device ID", pci_resources->device_id,
                  "Command  ", pci_resources->command_reg,
                  "Status   ", pci_resources->status_reg,
                  "SubVendID", pci_resources->subsystem_vendor_id,
                  "SubDevID ", pci_resources->subsystem_id,
                  "Irq Line ", pci_resources->interrupt_line,
                  "Revision ", pci_resources->revision_id,
                  "Class    ", pci_resources->class_code,
                  "Mem Area0 (regs)   ", pci_resources->bar_info[0].phy_addr,
                  "Mem Size0          ", (int)pci_resources->bar_info[0].size,
                  "Mem Area2 (queues) ", pci_resources->bar_info[1].phy_addr,
                  "Mem Size2          ", (int)pci_resources->bar_info[1].size,
                  "Mem Area4 (buffers)", pci_resources->bar_info[2].phy_addr,
                  "Mem Size4          ", (int)pci_resources->bar_info[2].size,
                  "Max of DMA Buffers     ", pci_resources->max_dma_buff,
                  "Max of DMA Send Buffers", pci_resources->max_dma_send_buff,
                  "Max of DMA Revc Buffers", pci_resources->max_dma_recv_buff);

        // Enable Bus Mastering and query if DMA supported.
        pci_set_master(pci_resources->pcidev);
        if (!pci_dma_supported(pci_resources->pcidev, DMA_32BIT_MASK)) {
            OS_PRINTK("Oops: no 32bit PCI DMA ???\n");
            retval = OS_STATUS_IO_FAILURE;
            break;
        }

        //----------------------------------------
        // Request for and Map the 3 segments
        //----------------------------------------
        stretch_dev = pci_get_drvdata(pci_resources->pcidev);

        // Request memory region (from pci resources)
        if (!request_mem_region(pci_resources->bar_info[0].phy_addr,
                                pci_resources->bar_info[0].size,
                                stretch_dev->ifname)) {
            OS_PRINTK("Can't get MMIO memory @ %p\n",
                      (void *)pci_resources->bar_info[0].size);
            retval = OS_STATUS_IO_BUSY;
            break;
        }
        xOS_PRINTK("Shared memory region at(phy): %p, size %ld granted\n",
                  (void *)pci_resources->bar_info[0].phy_addr,
                  pci_resources->bar_info[0].size);
        cleanup_lev++; // 1

        if (!request_mem_region(pci_resources->bar_info[1].phy_addr,
                                pci_resources->bar_info[1].size,
                                stretch_dev->ifname)) {
            OS_PRINTK("Can't get MMIO memory @ %p\n",
                      (void *)pci_resources->bar_info[1].phy_addr);
            retval = OS_STATUS_IO_BUSY;
            break;
        }
        xOS_PRINTK("Shared memory region at(phy): %p, size %ld granted\n",
                  (void *)pci_resources->bar_info[1].phy_addr,
                  pci_resources->bar_info[1].size);
        cleanup_lev++; // 2

        if (!request_mem_region(pci_resources->bar_info[2].phy_addr,
                                pci_resources->bar_info[2].size,
                                stretch_dev->ifname)) {
            OS_PRINTK("Can't get MMIO memory @ %p\n",
                      (void *)pci_resources->bar_info[2].phy_addr);
            retval = OS_STATUS_IO_BUSY;
            break;
        }
        xOS_PRINTK("Shared memory region at(phy): %p, size %ld granted\n",
                  (void *)pci_resources->bar_info[2].phy_addr,
                  pci_resources->bar_info[2].size);
        cleanup_lev++; // 3

    } while(0);

    if (retval != OS_STATUS_SUCCESS) {
        pci_resources_fini(pci_resources,
                           cleanup_lev);
    }
    return retval;
}

