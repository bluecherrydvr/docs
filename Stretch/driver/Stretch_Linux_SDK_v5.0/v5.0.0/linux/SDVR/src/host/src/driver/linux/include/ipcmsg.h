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

#ifndef _IPCMSG_H
#define _IPCMSG_H

#ifdef __cplusplus
extern "C" {
#endif


#define SCT_MSG_DATA_SIZE 16

// Number of Queue Status words available for each side
#define SCT_QUEUE_STATUS_CNT          8

// Message counts
#define SCT_MGMT_MSG_COUNT            256   // per direction
#define SCT_MSG_COUNT                 256   // per direction (shared by queues)

// Direction
#define SCT_DIR_BOARD2HOST            0
#define SCT_DIR_HOST2BOARD            1

// Management queues, 1 queue in each direction (0x30000 - 0x30001
#define SCT_QUEUE_MGMT_ID(dir)        (0x30000 + (dir))

// Queues for Message classes, 2 queues for each (0x20000 - 0x200ff)
#define SCT_QUEUE_MSG_ID(class, dir)  (0x20000 + ((class) << 1) + (dir))

// Queues for return of messages, any class (0x21000 - 0x21001)
#define SCT_QUEUE_MSG_RTN_ID(dir)     (0x21000 + (dir))

// Maximum bar registers in PCI.  Probably could derive from pci.h
// or somwhere
#define SCT_MAX_PCI_BAR                 6

// Structure for passing PCI information
typedef struct _DnDev_bar_type
{
     uint64_t          log_addr;
     void *            mapped_addr;
     size_t            size;

} dndev_bar_t;

// PCI config space info for each device
typedef struct _DnDev_Data
{
    uint16_t          vid;
    uint16_t          did;
    dndev_bar_t       bar[SCT_MAX_PCI_BAR];

} dndev_t;






//========================================
// Management message structure
//========================================

// Definitions for sct_mgmt_msg_t:id field
#define SCT_MGMT_MSG_INIT               1    // Sender = Board only
#define SCT_MGMT_MSG_CONNECT            2    // Sender = Board or Host
#define SCT_MGMT_MSG_DESTROY            3    // Sender = Board or Host
#define SCT_MGMT_MSG_DMA_DONE           4    // Sender = Board only
#define SCT_MGMT_MSG_HOST_RECVBUF       5    // Sender = Host only
#define SCT_MGMT_MSG_HOST_SENDBUF       6    // Sender = Host only


// sct_mgmt_msg_t structure
typedef struct
{
    // Message header (Don't touch - reserved for IPC)
    sx_queue_datahdr_t hdr;

    // Message identifier (see above #defines)
    uint32_t id;

    // Response code (0=OK, any other value=ERROR)
    // (This also contains the incoming IOCTL-style command code
    //  for the driver to use to know what to do)
    uint32_t rsp;

    // Message specific data (determined by 'id' field)
    union
    {
        //
        // sct_init: id = SCT_MGMT_MSG_INIT
        //
        // Message is only sent from Board to Host since Board allocates the
        // messages.  The allocated messages (mgmt messages first, then regular
        // messages) are written by Board to MGMT queue.
        struct
        {
            uint32_t  mgmt_msg_count; // Number of mgmt messages sent to MGMT queue
            uint32_t  msg_count;      // Number of reg. messages sent to MGMT queue
            OS_32BIT_PTR(uint32_t) status;
                                      // (PCI addr) for Host to use for IPC queues.
            uint32_t  pci_min;        // Lowest PCI addr Host uses for
            uint32_t  pci_min_hi;     // buffers (low and high 32 bits)
            uint32_t  pci_max;        // Highest PCI addr Host uses for
            uint32_t  pci_max_hi;     // buffers (low and high 32 bits)

        } init;

        //
        // sct_channel_connect: id = SCT_MGMT_MSG_CONNECT
        //
        // Message is sent by either Host or Board, depending on which side receives the
        // sct_channel_connect() call.
        struct
        {
            uint32_t    port;           // port (aka channel)
            uint32_t    max_size;       // Size of the buffers (max DMA transfer)

        } connect;

        //
        // sct_channel_destroy: id = SCT_MGMT_MSG_DESTROY
        //
        struct
        {
            uint32_t    port;           // port (aka channel)

        } destroy;

        //
        // buffer: id = SCT_MGMT_MSG_DMA_DONE
        //              SCT_MGMT_MSG_HOST_RECVBUF
        //              SCT_MGMT_MSG_HOST_SENDBUF
        //
        // This message used by Host to send buffers to Board, and by Board
        // to send DMA done (i.e., return buffer) to Host.
        // If buffer is for DMA from Board to Host (Board send), port is set to
        // SCT_PORT_INVALID.  Board does not assign buffer to a SCT channel
        // until a send transfer is actually requested.
        // If buffer is for DMA from Host to Board (Board receive), port is used
        // to identify the SCT channel for which the buffer is intended.  Board
        // adds buffer to pending receives for that SCT channel.
        struct
        {
            int32_t     port;         // port (aka channel), or SCT_PORT_INVALID
            uint32_t    pci_addr;     // PCI address of buffer (low 32 bits)
            uint32_t    pci_addr_hi;  // PCI address of buffer (high 32 bits)
            uint32_t    size;         // Size of buffer

        } buffer;

    } data;

} sct_mgmt_msg_t;



//========================================
// Non mgmt message structure 
//========================================
typedef struct
{
    // Message header (Don't touch - reserved for IPC)
    sx_queue_datahdr_t hdr;

    uint32_t        size;
    uint8_t         data[SCT_MSG_DATA_SIZE];

} sct_msg_t;


//========================================
// Class container for non-mgmt messages
//========================================
typedef struct
{
    int             classId;
    sct_msg_t       msg;

} sct_dll_msg_t;

//=======================================
// Should use the define in sct.h
//=======================================
#ifndef SCT_MAX_PE
#define SCT_MAX_PE 32
#endif

//=======================================
//structure to be used in pci boot
//=======================================
typedef struct
{
    uint32_t        bwait;
    uint8_t         *data;
    uint32_t        size;
    uint32_t        version;
    uint32_t        pe_num;
    uint32_t        result[SCT_MAX_PE];
} sct_boot_info_t;

typedef struct sct_board_detail_internal {
    sx_uint32   board_number;
    sx_uint32   board_slot_number;
    sx_uint16   vendor_id;
    sx_uint16   device_id;
    sx_uint8    revision;
    sx_uint16   subsystem_vendor;
    sx_uint16   subsystem_id;
    sx_int8     serial_string[17];  //null terminated serial number string
    sx_uint8    driver_version[4];
    sx_uint32   buffer_size;
} sct_board_detail_internal_t;

#ifdef __cplusplus
}
#endif

#endif // _IPCMSG_H

