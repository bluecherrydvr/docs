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

#ifndef BOARD_LIST_ARCH_H
#define BOARD_LIST_ARCH_H

typedef struct board_list_node_tag
{
    unsigned short vendor_id;
    unsigned short device_id;
    unsigned short sub_vendor_id;
    unsigned short sub_device_id;
    unsigned short devs_onboard;
    unsigned short chans_per_dev;
    unsigned short send_chans;
    unsigned short bufs_per_chan;
    unsigned long  buf_size;
} board_list_node_t;

extern board_list_node_t stretch_board_list[];

#endif /* BOARD_LIST_ARCH_H */

