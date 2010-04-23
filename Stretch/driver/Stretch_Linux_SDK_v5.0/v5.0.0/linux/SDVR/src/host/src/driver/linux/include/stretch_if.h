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

#ifndef _STRETCH_IF_H
#define _STRETCH_IF_H

// Stretch Driver interface for Linux driver

// For function cleanup in do{}while(0) loops.
#define CLEANUP_ALL     255

#define RESPONSE_OK     0


// Ioctls for IOCTL as well as async read/write calls
// BUFFER_SEND and STORE_RECEIVE_BUFFER get port
// embedded into the _IOC_SIZE field at runtime.
// Typed of xfer: DMA or normal
#define SX_IOTYPE_NORM                'm'
#define SX_IOTYPE_PEND                'p'
#define SX_IOTYPE_DMA                 0

#define SX_IOCODE_READ_CONFIG          0x00
#define SX_IOCODE_IPC_QUEUE_INIT       0x01
#define SX_IOCODE_RESET_S6             0x02
#define SX_IOCODE_AIO_CANCEL           0x03

#define SX_IOCODE_STORE_RECEIVE_BUFFER  0x10
#define SX_IOCODE_BUFFER_SEND    0x11
#define SX_IOCODE_WRITE_MSG      0x20
#define SX_IOCODE_READ_MSG       0x21
#define SX_IOCODE_READ_MSG_POLL  0x22
#define SX_IOCODE_CHECK_CONNECT  0x23
#define SX_IOCODE_WRITE_MGMT_QUEUE     0x24
#define SX_IOCODE_DESTROY_RESPONSE     0x25
#define SX_IOCODE_ACCEPT_RESPONSE      0x26
#define SX_IOCODE_WAIT_DESTROY         0x27
#define SX_IOCODE_WAIT_ERROR           0x28
#define SX_IOCODE_BOOT_FIRMWARE        0x29
#define SX_IOCODE_GET_BOARD_DETAIL     0x30

// True IOCTLS
#define SX_IOCTL_READ_CONFIG          _IOR( SX_IOTYPE_NORM,SX_IOCODE_READ_CONFIG,dndev_t)
#define SX_IOCTL_IPC_QUEUE_INIT       _IOR( SX_IOTYPE_NORM,SX_IOCODE_IPC_QUEUE_INIT,uint32_t)
#define SX_IOCTL_RESET_S6             _IO(  SX_IOTYPE_NORM,SX_IOCODE_RESET_S6)
#define SX_IOCTL_AIO_CANCEL           _IOW( SX_IOTYPE_NORM,SX_IOCODE_AIO_CANCEL,uint8_t)


// IOCTL commands that go in via "offset" to aio_read
// Port and Size(1k blocks) info overlayed on cmd for DMA ops.
// (see helper macros, below).
// Had to squeeze things in to avoid overhead of IOCTL calls
// for performance and atomicity of operations.
// (these should just be the cmd value, with 0 in all other fields.)
#define SX_IOCTL_STORE_RECEIVE_BUFFER _IO(  SX_IOTYPE_DMA ,SX_IOCODE_STORE_RECEIVE_BUFFER)
#define SX_IOCTL_BUFFER_SEND          _IO(  SX_IOTYPE_DMA ,SX_IOCODE_BUFFER_SEND) 

// Non-Buffer, messaging ops.
#define SX_IOCTL_WRITE_MSG            _IOW( SX_IOTYPE_PEND,SX_IOCODE_WRITE_MSG,sct_dll_msg_t)
#define SX_IOCTL_READ_MSG             _IOWR(SX_IOTYPE_PEND,SX_IOCODE_READ_MSG,sct_dll_msg_t)
#define SX_IOCTL_READ_MSG_POLL        _IOWR(SX_IOTYPE_NORM,SX_IOCODE_READ_MSG_POLL,sct_dll_msg_t)
#define SX_IOCTL_CHECK_CONNECT        _IOWR(SX_IOTYPE_PEND,SX_IOCODE_CHECK_CONNECT,sct_mgmt_msg_t)
#define SX_IOCTL_WRITE_MGMT_QUEUE     _IOWR(SX_IOTYPE_PEND,SX_IOCODE_WRITE_MGMT_QUEUE,sct_mgmt_msg_t)
#define SX_IOCTL_DESTROY_RESPONSE     _IOW( SX_IOTYPE_PEND,SX_IOCODE_DESTROY_RESPONSE,sct_mgmt_msg_t)
#define SX_IOCTL_ACCEPT_RESPONSE      _IOW( SX_IOTYPE_PEND,SX_IOCODE_ACCEPT_RESPONSE,sct_mgmt_msg_t)
#define SX_IOCTL_WAIT_DESTROY         _IOW( SX_IOTYPE_PEND,SX_IOCODE_WAIT_DESTROY,sct_mgmt_msg_t)
#define SX_IOCTL_WAIT_ERROR           _IOW( SX_IOTYPE_PEND,SX_IOCODE_WAIT_ERROR,sct_mgmt_msg_t)
#define SX_IOCTL_BOOT_FIRMWARE        _IOWR(SX_IOTYPE_NORM,SX_IOCODE_BOOT_FIRMWARE,sct_boot_info_t)
#define SX_IOCTL_GET_BOARD_DETAIL     _IOR( SX_IOTYPE_NORM,SX_IOCODE_GET_BOARD_DETAIL,sct_board_detail_t)
#ifndef MODULE
    // Construct special DMA IOCTL with embedded size and port info
    // Embedded size is in 1k blocks
    #define SX_CONSTRUCT_DMA_IOCTL(io,sz,pt) \
                                    (((int64_t)io&0xffff) | ((int64_t)pt << 16) | ((int64_t)sz << 32) )
#else
    #define SX_DMA_IOCTL_GET_SIZE(io) \
                                    ((uint32_t)((io) >> 32))
    #define SX_DMA_IOCTL_GET_PORT(io) \
                                    (((uint32_t)(io)) >> 16)
    #define SX_DMA_IOCTL_GET_CMD(io) \
                                    (((uint32_t)(io)) & 0x000000ff)
#endif

#endif //_STRETCH_IF_H

