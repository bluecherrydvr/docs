/************************************************************************
 *
 * Copyright C 2007 Stretch, Inc. All rights reserved.  Stretch products are
 * protected under numerous U.S. and foreign patents, maskwork rights,
 * copyrights and other intellectual property laws.  
 *
 * This source code and the related tools, software code and documentation, and
 * your use thereof, are subject to and governed by the terms and conditions of
 * the applicable Stretch IDE or SDK and RDK License Agreement
 * (either as agreed by you or found at www.stretchinc.com).  By using these
 * items, you indicate your acceptance of such terms and conditions between you
 * and Stretch, Inc.  In the event that you do not agree with such terms and
 * conditions, you may not use any of these items and must immediately destroy
 * any copies you have made.
 *
 ************************************************************************/

#include <stdio.h>
//#include <malloc.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#endif

#include "sdvr_sdk_common.h"

/************************************************************************
 This function validates if the given osd id is configured for the given
 channel. 

 NOTE: It is assumed that handle is a valid encoder/decoder channel.
**************************************************************************/
static sdvr_err_e _validate_osd_item(sdvr_chan_handle_t handle, sx_uint8 osd_id) 
{
    sx_uint8 osd_id_configured = 0;
    /* chan_num in case of SDVR_CHAN_TYPE_OUTPUT is the SMO port*/
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    if (osd_id < SDVR_MAX_OSD)
    {
        switch (chan_type)
        {
        case SDVR_CHAN_TYPE_NONE:
        case SDVR_CHAN_TYPE_ENCODER:
             osd_id_configured = _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].osd_defined_map[osd_id];

            break;

        case SDVR_CHAN_TYPE_DECODER:
             osd_id_configured = _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].osd_defined_map[osd_id];

            break;

        case SDVR_CHAN_TYPE_OUTPUT:
            osd_id_configured = _ssdvr_boards_info.boards[board_index].smo_info[chan_num].osd_defined_map[osd_id];
            break;

        default:
            break;
        }
    }

    if (osd_id_configured)
        return SDVR_ERR_NONE;
    else
        return SDVR_ERR_INVALID_OSD_ID;
}
static sdvr_err_e _set_osd_item_defined(sdvr_chan_handle_t handle, sx_uint8 osd_id, sx_bool bDefined) 
{
    /* chan_num in case of SDVR_CHAN_TYPE_OUTPUT is the SMO port*/
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sdvr_err_e err = SDVR_ERR_NONE;

    if (osd_id < SDVR_MAX_OSD)
    {
        switch (chan_type)
        {
        case SDVR_CHAN_TYPE_NONE:
        case SDVR_CHAN_TYPE_ENCODER:
             _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].osd_defined_map[osd_id] = bDefined;

            break;

        case SDVR_CHAN_TYPE_DECODER:
             _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].osd_defined_map[osd_id] = bDefined;

            break;

        case SDVR_CHAN_TYPE_OUTPUT:
            _ssdvr_boards_info.boards[board_index].smo_info[chan_num].osd_defined_map[osd_id] = bDefined;
            break;

        default:
            err =  SDVR_ERR_INVALID_OSD_ID;
        }
    }

    return err;
}

/************************************************************************
 This function sends OSD configuration information for the given osd id
 of the given channel. These are information such as DTS format,
 location, ect. It does not send the text data assoicated to this OSD

 NOTE: It is assume prior to this call all the parameters are validated and
       are correct.

       Also this function is used to send the SMO OSD which does not apply
       to any channel handle. In this case only the board index is used
       from the channel handle.
**************************************************************************/
static sdvr_err_e _send_osd_config(sdvr_chan_handle_t handle, sx_uint8 osd_id,
                             sdvr_osd_config_ex_t *osd_text_config,
                             sx_uint8 smo_port)
{
    dvr_osd_ex_t cmd_strct;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    int cmd_strct_size = sizeof(cmd_strct);

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = CH_GET_CHAN_NUM(handle);
    cmd_strct.op_code = DVR_OSD_OPCODE_CONFIG;

    cmd_strct.u1.text.osd_id = osd_id;
    cmd_strct.u1.config.translucent = osd_text_config->translucent;
    cmd_strct.u1.config.position_ctrl = osd_text_config->location_ctrl;
    cmd_strct.u1.config.x_TL = osd_text_config->top_left_x;
    cmd_strct.u1.config.y_TL = osd_text_config->top_left_y;
    cmd_strct.u1.config.dts_format = osd_text_config->dts_format;
    cmd_strct.u1.config.smo_port = smo_port;
    
    ssdk_message_chan_send(handle, DVR_SET_OSD_EX, &cmd_strct, sizeof(cmd_strct));
    ssdk_message_chan_recv(handle, DVR_REP_OSD_EX, &cmd_strct, 0, 0);

    /************************************************************
      Mark this OSD item as defined if the Firmware accepted.
      This is needed for the future calls to OSD show calls.
    *************************************************************/
    if (cmd_strct.status == SDVR_ERR_NONE)
        _set_osd_item_defined(handle, osd_id, true);
    else
        _set_osd_item_defined(handle, osd_id, false);

    sdbg_msg_output_err(sdvr_osd_text_ex_func, cmd_strct.status);

    return cmd_strct.status;
}

/************************************************************************
 This function sends OSD text information for the given osd id
 of the given channel. No other OSD configuration information will be sent.

 NOTE: It is assume prior to this call all the parameters are validated and
       are correct.

       Also this function is used to send the SMO OSD which does not apply
       to any channel handle. In this case only the board index is used
       from the channel handle.
**************************************************************************/
static sdvr_err_e _send_osd_text(sdvr_chan_handle_t handle, sx_uint8 osd_id,
                             sdvr_osd_config_ex_t *osd_text_config,
                             sx_uint8 smo_port)
{
    dvr_osd_ex_t cmd_strct;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    int cmd_strct_size = sizeof(cmd_strct);
    int len = 0;
    int numToSend;
    int numTextSent;
                                    // The text size in uint16;
    int xfrTextSize = sizeof(cmd_strct.u1.text.data) / 2;

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = CH_GET_CHAN_NUM(handle);
    cmd_strct.op_code = DVR_OSD_OPCODE_TEXT;

    cmd_strct.u1.text.osd_id = osd_id;
    cmd_strct.u1.text.length = osd_text_config->text_len;
    cmd_strct.u1.text.smo_port = smo_port;
    
    /******************************************************************
      Check to see if the OSD test was deleted or changed.
      We need to send different message to delete a text.
    ******************************************************************/
    if (cmd_strct.u1.text.length == 0)
    {
        cmd_strct.op_code = DVR_OSD_OPCODE_CLEAR_TEXT;
        ssdk_message_chan_send(handle, DVR_SET_OSD_EX, &cmd_strct, sizeof(cmd_strct));
        ssdk_message_chan_recv(handle, DVR_REP_OSD_EX, &cmd_strct, 0, 0);
    }
    else
    {
        numToSend = cmd_strct.u1.text.length / xfrTextSize;
        if (cmd_strct.u1.text.length % xfrTextSize > 0)
            numToSend++;

        // change from uint16 to uint8
        xfrTextSize *= 2;
        for (numTextSent = 0; numTextSent < numToSend; numTextSent++ )
        {
            memcpy(cmd_strct.u1.text.data, (char*)&osd_text_config->text + (numTextSent * xfrTextSize), xfrTextSize);
            ssdk_message_chan_send(handle, DVR_SET_OSD_EX, &cmd_strct, sizeof(cmd_strct));
            ssdk_message_chan_recv(handle, DVR_REP_OSD_EX, &cmd_strct, 0, 0);
            if (cmd_strct.status != SDVR_ERR_NONE)
                break;

            // NOTE: Only the first part of the text data transmission
            // should have the length. The subsequents text length transmission
            // should be set to zero. This is a firmware requirement.
            cmd_strct.u1.text.length = 0;
        }
    }

    return cmd_strct.status;
}

/************************************************************************
 This function reads the font file into  three
 buffers Y, U, V holding the corresponding bitmap so that we can send it
 to the firmware.

 NOTE: It is assume prior to this call all the parameters are validated and
       are correct.
**************************************************************************/

static sdvr_err_e _osd_read_font_table(FILE *fp, int startglyph, int endglyph, 
                                 int coly, int colu, int colv, 
                                 unsigned char **yy, unsigned char **uu, unsigned char **vv, 
                                 int **utf16, int **index, 
                                 int **width, int *height, int *nglyph, int *yysize)
{
    ssdk_bdf_t *bdf, bdf_space;
    char s[2048];
    int state;
    int nibble;
    int y, c, i, j, run, totwidth, curwidth, cglyph;

    state = 0;
    bdf = &bdf_space;

    curwidth = 0;
    cglyph = 0;
    index[0] = 0;

    for(run = 0; run < 2; run++)
    {
        if(run == 1)
        {
            /* no glyphs found in [startglyph, endglyph] range - may not be a valid bdf file */
            if (cglyph==0) 
                return SDVR_ERR_OSD_FONT_FILE; 

            totwidth = curwidth;
            curwidth = 0;
            *nglyph = cglyph;
            cglyph = 0;
            fseek(fp, 0, SEEK_SET);
            /* allocate space for fonts in yuv format */
            *yysize = totwidth * bdf->fbby;
            if ((*yy = (unsigned char *) malloc(totwidth*bdf->fbby)) == NULL) 
                return SDVR_ERR_OUT_OF_MEMORY;
            if((*uu=(unsigned char *) malloc(totwidth/2*bdf->fbby/2))==NULL)
            {
                free(*yy);
                return SDVR_ERR_OUT_OF_MEMORY;
            }
            if((*vv=(unsigned char *) malloc(totwidth/2*bdf->fbby/2))==NULL)
            {
                free(*yy);
                free(*uu);
                return SDVR_ERR_OUT_OF_MEMORY;
            }
            if((*index=(int *) malloc(((*nglyph)+1)*sizeof(int)))==NULL)
            {
                free(*yy);
                free(*uu);
                free(*vv);
                return SDVR_ERR_OUT_OF_MEMORY;
            }
            if((*width=(int *) malloc((*nglyph)*sizeof(int)))==NULL)
            {
                free(*yy);
                free(*uu);
                free(*vv);
                free(*index);
                return SDVR_ERR_OUT_OF_MEMORY;
            }
            if((*utf16=(int *) malloc((*nglyph)*sizeof(int)))==NULL)
            {
                free(*yy);
                free(*uu);
                free(*vv);
                free(*index);
                free(*width);
                return SDVR_ERR_OUT_OF_MEMORY;
            }
            memset(*yy, 0, totwidth*bdf->fbby);
            memset(*uu, 0, totwidth/2*bdf->fbby/2);
            memset(*vv, 0, totwidth/2*bdf->fbby/2);
        } // if(run == 1)


        while(1)
        {
            fscanf(fp, "%2047[^\n\r]%*[\n\r]", s);
            if (feof(fp)) 
            {
                break;
            }
            if (strncmp(s, "ENDFONT", 7)==0) 
            {
                break;
            }
            if (strncmp(s, "CHARS", 5)==0) 
                sscanf(s, "%*s%d", &bdf->chars);
            if (strncmp(s, "ENCODING", 8)==0) 
                sscanf(s, "%*s%d", &bdf->encoding);
            if (strncmp(s, "FONTBOUNDINGBOX", 15)==0) 
            {
                sscanf(s, "%*s%d%d%d%d", &bdf->fbbx, &bdf->fbby, &bdf->xoff, &bdf->yoff);
                if(bdf->fbby & 1) bdf->fbby++;
            }
            if (strncmp(s, "BBX", 3)==0) 
                sscanf(s, "%*s%d%d%d%d", &bdf->bbw, &bdf->bbh, &bdf->bbxoff0x, &bdf->bbyoff0y);
            if (strncmp(s, "DWIDTH", 6)==0) 
            {
                sscanf(s, "%*s%d%d", &bdf->dwx0, &bdf->dwy0);
                if(bdf->dwx0 & 1) bdf->dwx0++;
            }
            if(strncmp(s, "BITMAP", 6)==0)
            {
                c=bdf->encoding;
                if(c>=startglyph && c<=endglyph)
                {
                  curwidth+=((bdf->dwx0+15)/16)*16;
                  cglyph++;
                }
                for(y=0; ; y++)
                {
                    fscanf(fp, "%2047[^\n\r]%*[\n\r]", s);
                    if(strncmp(s, "ENDCHAR", 7)==0) break;
                    if(run==1 && c>=startglyph && c<=endglyph && bdf->dwx0!=0 && bdf->fbby!=0 && bdf->encoding!=-1)
                    {
                        (*width)[cglyph-1]=bdf->dwx0; /* move outside loop */
                        (*index)[cglyph-1]=curwidth-((bdf->dwx0+15)/16)*16;
                        (*utf16)[cglyph-1]=c;

                        for(i=0; s[i]!='\0'; i++)
                        {
                            nibble=s[i]-'0'-(s[i]>='A' && s[i]<='F'?'A'-'9'-1:0)-(s[i]>='a' && s[i]<='f'?'a'-'9'-1:0);
                            for(j=0; j<4; j++)
                            {
                                if(i*4+j<bdf->dwx0 && y<bdf->fbby)          /* check to see if pixel is inside glyph box */
                                {
                                    (*yy)[(curwidth-((bdf->dwx0+15)/16)*16)*bdf->fbby+y*(((bdf->dwx0+15)/16)*16)+i*4+j]
                                        =(nibble & 0x08)?coly:0;
                                    if(nibble & 0x08)
                                    {
                                        (*uu)[(curwidth-((bdf->dwx0+15)/16)*16)/2*bdf->fbby/2+y/2*(((bdf->dwx0+15)/16)*16)/2+(i*4+j)/2]=colu;
                                        (*vv)[(curwidth-((bdf->dwx0+15)/16)*16)/2*bdf->fbby/2+y/2*(((bdf->dwx0+15)/16)*16)/2+(i*4+j)/2]=colv;
                                    }
                                }
                            nibble<<=1;
                            }
                        }
                    }
                } //for(y=0; ; y++)
                bdf->encoding=-1;
            } //if(strncmp(s, "BITMAP", 15)==0)
        } //  while(1)
    } //for(run = 0; run < 2; run++)

    *height=bdf->fbby;
  
  return SDVR_ERR_NONE;
}
/*
    This function sends the given font table buffer part to the firmware.
    If the size of the buffer is larger than one send SCT buffer,
    the buffer will be divided into chunks.
*/
static sdvr_err_e _send_font_buffer(sx_uint8 board_index, sct_channel_t pci_send_chan_handle,
                                        sx_uint8 font_opcode, 
                                        unsigned char *font_buffer, sx_uint32 buf_size)
{
    dvr_font_table_t cmd_strct;
    sdvr_err_e err;
    sdvr_err_e send_err = SDVR_ERR_NONE;
    int cmd_strct_size = sizeof(cmd_strct);
    int len = 0;
    int numToSend;
    int numTextSent;
    void *send_buffer = NULL;
    sx_uint32 send_buf_size;
    int retry;

    memset(&cmd_strct,0,cmd_strct_size);
    cmd_strct.opcode = font_opcode;
    cmd_strct.u1.bufsize = buf_size;

    err = ssdk_message_board_send( board_index, DVR_SET_FONT_TABLE, &cmd_strct, cmd_strct_size);

    /*
       Calculate the chunks of data to send.
    */
    numToSend = buf_size / XFR_SCT_FONT_TABLE_SIZE;
    if (buf_size % XFR_SCT_FONT_TABLE_SIZE > 0)
        numToSend++;

    for (numTextSent = 0; numTextSent < numToSend; numTextSent++ )
    {     
        for (retry = 0; retry < 20; retry++)
        {
            send_buffer = sct_buffer_alloc(pci_send_chan_handle);
            if (send_buffer)
                break;
#ifdef WIN32
            Sleep(100);
#else
            usleep(100*1000);
#endif
        }

        if (send_buffer != NULL)
        {

            if (buf_size < XFR_SCT_FONT_TABLE_SIZE)
                send_buf_size = buf_size;
            else
                send_buf_size = XFR_SCT_FONT_TABLE_SIZE;
            memcpy(send_buffer, font_buffer + (numTextSent * XFR_SCT_FONT_TABLE_SIZE), send_buf_size);
            send_err = sct_buffer_send(pci_send_chan_handle, send_buffer, send_buf_size);
            if (send_err != SCT_NO_ERROR)
                break;
            buf_size -= send_buf_size;
        }
        else
        {
            send_err = SDVR_DRV_ERR_NO_SEND_BUFFERS;
            break;
        }
    }
    err = ssdk_message_board_recv( board_index, DVR_REP_FONT_TABLE, &cmd_strct, 0, 0);
    if (err != SCT_NO_ERROR)
        return err;
    if (cmd_strct.status != SDVR_ERR_NONE)
        return cmd_strct.status;

    /*
      If we get to this point it means the font table is closed properly,
      but incase we had an error in sending all of the font table,
      return the send error.
    */
    return send_err;
}
/*
  This function opens a SCT channel to send the font table description
  to every DVR board that is connected.
  Before returning the SCT channel is closed.
*/
static sdvr_err_e _send_font_table(sx_uint8 font_table_id,
                                 sx_uint8   font_table_format,
                                 unsigned char *osdyy, unsigned char *osduu, unsigned char *osdvv, 
                                 unsigned char *osdutf16, unsigned char *osdindex, unsigned char *osdwidth, 
                                 int osdheight, int osdnglyph, int osdyysize)
{
    dvr_font_table_t cmd_strct;
    sx_uint8 board_index;
    int cmd_strct_size = sizeof(cmd_strct);
    sdvr_err_e err = SDVR_ERR_BOARD_NOT_CONNECTED;
    int len = 0;
    // The PCI port number to send data to the FW is the
    // same number as the 1st 16 bit of channel handle.
    sct_channel_t pci_send_chan_handle = NULL;

    /*
       Send the same font table to all the boards that are connected and 
       is still functional
    */
    for (board_index = 0 ; board_index < _ssdvr_boards_info.board_num ; board_index++)
    {
        /* 
           Try a different board to load the font table
           If the board is not connected or is dead.
        */
        if (!_ssdvr_boards_info.boards[board_index].handle,
            _ssdvr_boards_info.boards[board_index].is_fw_dead)
            continue;

        /*
           Tell the firmware to open the given SCT port ID to receive font table.
        */
        memset(&cmd_strct,0,cmd_strct_size);
        cmd_strct.opcode = DVR_FONT_OPCODE_START;
        cmd_strct.font_index = font_table_id;
        cmd_strct.font_format = font_table_format;
        cmd_strct.port_id = SCT_PORT_FONT_TABLE;
        cmd_strct.osdheight = osdheight;
        cmd_strct.osdnglyph = osdnglyph;
        cmd_strct.u1.osdyysize = osdyysize;
        ssdk_message_board_send(board_index, DVR_SET_FONT_TABLE, &cmd_strct, sizeof(cmd_strct));
        /*
            we must try to open a SCT channel to send font table.
        */

        err = sct_channel_connect( _ssdvr_boards_info.boards[board_index].handle,
                     SCT_PORT_FONT_TABLE,
                     3,                         // number of send buffers to allocate
                     XFR_SCT_FONT_TABLE_SIZE,   // the send buffer size
                     &pci_send_chan_handle);
        if (err != SDVR_ERR_NONE)
            break;

        ssdk_message_board_recv(board_index, DVR_REP_FONT_TABLE, &cmd_strct, 0, 0);
        err = cmd_strct.status;
        if (err != SDVR_ERR_NONE)
        {
            // If there was any error, close the sct channel and stop.
            sct_channel_close(pci_send_chan_handle);
            break;
        }

        /*
            Now firmware is ready to receive the actual YUV font table
        */
        if ((err = _send_font_buffer(board_index, pci_send_chan_handle,DVR_FONT_OPCODE_BUF_YY,osdyy, osdyysize * sizeof(char))) == SDVR_ERR_NONE &&
            (err = _send_font_buffer(board_index, pci_send_chan_handle,DVR_FONT_OPCODE_BUF_UU,osduu, osdyysize / 4 * sizeof(char))) == SDVR_ERR_NONE &&
            (err = _send_font_buffer(board_index, pci_send_chan_handle,DVR_FONT_OPCODE_BUF_VV,osdvv, osdyysize / 4 * sizeof(char))) == SDVR_ERR_NONE &&
            (err = _send_font_buffer(board_index, pci_send_chan_handle,DVR_FONT_OPCODE_BUF_UTF16,osdutf16, osdnglyph*sizeof(int))) == SDVR_ERR_NONE &&
            (err = _send_font_buffer(board_index, pci_send_chan_handle,DVR_FONT_OPCODE_BUF_INDEX,osdindex, osdnglyph*sizeof(int))) == SDVR_ERR_NONE &&
            (err = _send_font_buffer(board_index, pci_send_chan_handle,DVR_FONT_OPCODE_BUF_WIDTH,osdwidth, osdnglyph*sizeof(int))) == SDVR_ERR_NONE)
            ;

        // We must close the font table SCT channel before connecting to another board
        cmd_strct.opcode = DVR_FONT_OPCODE_FINISH;
        ssdk_message_board_send(board_index, DVR_SET_FONT_TABLE, &cmd_strct, sizeof(cmd_strct));
        sct_channel_close(pci_send_chan_handle);
        ssdk_message_board_recv(board_index, DVR_REP_FONT_TABLE, &cmd_strct, 0, 0);

        // Stop and report any error
        if (err != SDVR_ERR_NONE) 
            break;

        if (cmd_strct.status != SDVR_ERR_NONE) 
        {
            err = cmd_strct.status;
            break;
        }

    } //for (board_index = 0 ; board_index < _ssdvr_boards_info.board_num ; board_index++)


    return err;
}
/******************************************************************************
  This function enables/disables the OSD text on a specific video stream. 
  
  "smo_port" of 0xFF means to enable/disable the OSD text on 
  all the video streams  (HMO, encoder, and SMOs). 

  "smo_port" of 0x0 means to enable/disable the OSD text on 
  HMO or encoder video stream.

  "smo_port" > 0 means to enable/disable the OSD text only
  on the SMO port.
******************************************************************************/
static sdvr_err_e _osd_text_show(sdvr_chan_handle_t handle,
                              sx_uint8 osd_id,
                              sx_bool show,
                              sx_uint8 smo_port)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    dvr_osd_ex_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    /*
    NOTE: The channel validation should be done by the
          caller. Since in the case of SMO OSD the handle
          is a bogus one and in only used for its board index.
        err = ssdk_validate_chan_handle(handle, -1);
    */


    if (smo_port > _ssdvr_boards_info.boards[board_index].max_num_smos &&
             smo_port != 0xFF) /*port_num of 0xFF means apply to all*/
    {
        err = SDVR_ERR_SMO_PORT_NUM;
    }
    else
    {
        err = _validate_osd_item(handle, osd_id);
    }
    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_osd_text_show_func, err);
        return err;
    }

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = CH_GET_CHAN_NUM(handle);
    cmd_strct.op_code = DVR_OSD_OPCODE_SHOW;

    cmd_strct.u1.show.osd_id = osd_id;
    cmd_strct.u1.show.state = show;
    cmd_strct.u1.show.smo_port = smo_port;

    ssdk_message_chan_send(handle, DVR_SET_OSD_EX, &cmd_strct, sizeof(cmd_strct));
    ssdk_message_chan_recv(handle, DVR_REP_OSD_EX, &cmd_strct, 0, 0);

    sdbg_msg_output(sdvr_osd_text_show_func, board_index, (void *)&cmd_strct);

    return cmd_strct.status;
}

static sdvr_err_e _osd_text_config(sdvr_chan_handle_t handle, sx_uint8 osd_id,
                             sdvr_osd_config_ex_t *osd_text_config,
                             sx_uint8 smo_port)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    dvr_osd_ex_t cmd_strct;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    int cmd_strct_size = sizeof(cmd_strct);

    if (osd_text_config == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else if (osd_id >= SDVR_MAX_OSD)
        err = SDVR_ERR_INVALID_OSD_ID;
    else if (osd_text_config->text_len > SDVR_MAX_OSD_EX_TEXT)
        err = SDVR_ERR_OSD_LENGTH;
    else if (smo_port > _ssdvr_boards_info.boards[board_index].max_num_smos &&
             smo_port != 0xFF) /*port_num of 0xFF means apply to all*/
        err = SDVR_ERR_SMO_PORT_NUM;
    /*
    NOTE: The channel validation should be done by the
          caller. Since in the case of SMO OSD the handle
          is a bogus one and in only used for its board index.
    else
        err = ssdk_validate_chan_handle(handle, -1);
    */

    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_osd_text_config_ex_func, err);
        return err;
    }

    err = _send_osd_config(handle, osd_id, osd_text_config, smo_port);
    
    if (err == SDVR_ERR_NONE)
        err = _send_osd_text(handle, osd_id, osd_text_config, smo_port);

    sdbg_msg_output_err(sdvr_osd_text_config_ex_func, err);

    return err;
}

/****************************************************************************
  This function is used to configure the OSD text for an OSD item on all
  the video streams (HMO, encoded, and SMOs). The text
  associated with each OSD item can be up to 100 characters long but the actual
  display is limited to the size of the video and the starting location of
  the text. Additionally, you can specify whether to append a timestamp 
  to the displayed text. Appending a time stamp reduces the number of characters
  that can be displayed for your OSD item.

  After you configure an OSD item, you can show or hide its display state
  by calling sdvr_osd_text_show(). The original state of the OSD item is
  hidden.

  Parameters:

    "handle" - An encoding or decoding channel handle.

    "osd_id" - The OSD item to be configured. Currently there can be two OSD
    items per each channel. ID zero corresponds to the first OSD, 1 to the second,
    and so on. The current valid range is 0 - 1.

    "osd_text_config" - This data structure defines the text and its position
    for display over the video for the given channel as well as whether a 
    timestamp should be appended to the text.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remark:

    Call sdvr_smo_osd_text_config()
    to selectively configure the OSD text on a specific video stream.

****************************************************************************/
sdvr_err_e sdvr_osd_text_config_ex(sdvr_chan_handle_t handle, sx_uint8 osd_id,
                             sdvr_osd_config_ex_t *osd_text_config)
{
    sdvr_err_e err;

    err = ssdk_validate_chan_handle(handle, -1);

    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_osd_text_config_ex_func, err);
    }
    else
    {
        err = _osd_text_config(handle, osd_id, osd_text_config, 0xFF);
    }
    return err;
}

/****************************************************************************
  This function is used to control the display state of an OSD item for
  a given channel. The OSD text is displayed as soon as 
  encoding, raw video, or SMO streaming is enabled for the current channel
  handle. Changing the video streaming has no affect on the display state of 
  an OSD item for the given channels.
  
  This function must be called only after an OSD item is configured by
  calling sdvr_osd_text_config_ex() for the given channel handle.

  Parameters:

    "handle" - An encoding or decoding channel handle.

    "osd_id" - The OSD item to show or hide. Currently each channel is 
    limited to two OSD items. ID zero corresponds to the first OSD, 1 to the second,
    and so on. The current valid range is 0 - 1.

    "show" - If true, OSD item is displayed; otherwise, it is not
    displayed.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remark:

    This function controls the display state of OSD text on all the video
    stream (HMO, encoded, and SMO). Call sdvr_smo_osd_text_show()
    to selectively change the display
    state of OSD text on a specific video stream for this channel.

****************************************************************************/
sdvr_err_e sdvr_osd_text_show(sdvr_chan_handle_t handle,
                              sx_uint8 osd_id,
                              sx_bool show)
{
    sdvr_err_e err;

    err = ssdk_validate_chan_handle(handle, -1);

    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_osd_text_show_func, err);
    }
    else
    {
        err = _osd_text_show(handle, osd_id, show, 0xFF);
    }

    return err;
}

/****************************************************************************
  This function is used to configure the OSD text for an OSD item on a specific
  SMO port. The given OSD text applies to the entire SMO monitor and not to any 
  particular video channel. The text
  associated with each OSD item can be up to 100 characters long but the actual
  display is limited to the size of the video and the starting location of
  the text. Additionally, you can specify whether to append a timestamp 
  to the displayed text. Appending a time stamp reduces the number of characters
  that can be displayed for your OSD item.

  After you configure an OSD item, you can show or hide its display state
  by calling sdvr_smo_osd_text_show(). The original state of the OSD item is
  hidden.

  Parameters:

    "board_index" - Index of the board to set its SMO OSD.
 
    "smo_port" - Enter the SMO port number for which to configure its OSD text
    where SMO port numbers start from one (1).

    "osd_id" - The OSD item to be configured. Currently there can be two OSD
    items per each SMO monitor. ID zero corresponds to the first OSD, 1 to the second,
    and so on. The current valid range is 0 - 1.

    "osd_text_config" - This data structure defines the text and its position
    for display over the given SMO video as well as whether a 
    timestamp should be appended to the text.


  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:

    The OSD text is not associated to displaying of any SMO streaming.
    As result, you may show or hide OSD text while no other video is
    being displayed on the SMO monitor.

****************************************************************************/
sdvr_err_e sdvr_smo_osd_text_config(sx_uint32 board_index,
                             sx_uint8 smo_port, 
                             sx_uint8 osd_id,
                             sdvr_osd_config_ex_t *osd_text_config)
{
    sdvr_err_e err;
    sdvr_chan_handle_t handle;

    /*
       Create a dummy channel handle based on the given board
       index and smo port. It is needed for the underlying
       call in send the OSD config and text to the board.
    */
    handle = ssdk_create_chan_handle(board_index, 
                                     SDVR_CHAN_TYPE_OUTPUT, 
                                     smo_port);

    err = _osd_text_config(handle, osd_id, osd_text_config, smo_port);
    return err;
}

/****************************************************************************
  This function is used to control the display state of an OSD item for
  a given SMO display on a given DVR board. The OSD text is displayed as soon as 
  SMO streaming is enabled for any channel handle on this SMO monitor.
  
  This function must be called only after an OSD item is configured by
  calling sdvr_smo_osd_text_config() for the given channel handle.

  Parameters:

    "board_index" - Index of the board to change the show state of
    its SMO OSD for a given SMO port.
 
    "smo_port" - The SMO port number for which to change its 
    OSD text display state where SMO port numbers start from one (1).

    "show" - If true, OSD item is displayed; otherwise, it is not
    displayed.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

****************************************************************************/
sdvr_err_e sdvr_smo_osd_text_show(sx_uint32 board_index,
                              sx_uint8 smo_port,
                              sx_uint8 osd_id,
                              sx_bool show)

{
    sdvr_err_e err;
    sdvr_chan_handle_t handle;

    /*
       Create a dummy channel handle based on the given board
       index and smo port. It is needed for the underlying
       call in send the OSD config and text to the board.
    */
    handle = ssdk_create_chan_handle(board_index, 
                                     SDVR_CHAN_TYPE_OUTPUT, 
                                     smo_port);

    err = _osd_text_show(handle, osd_id, show, smo_port);

    return err;
}
/****************************************************************************
  Use this function to specify a different OSD text display font table than 
  the default ASCII font. Currently, Bitmap Distribution 
  Format (BDF) file is the only support font format. (The BDF 
  by Adobe is a file format for storing bitmap fonts. BDF is most commonly used
  font file within the linux operation system.)

  The new font table is used for all the DVR boards that are connected
  at the time of calling sdvr_osd_set_font_table().
  You can either choose to use all the characters within
  the font table or a sub-set of it. To use all the characters, set
  start_font_code parameter to 0 and end_font_code parameter to 65536.
  The specified YUV font color is used for all the
  characters within the table.

  Parameters:

    "font_desc" - A pointer to font descriptor data structure defining
    the new font table to load.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:

    You should only call this function once and after you connected to all the
    DVR boards.
****************************************************************************/
sdvr_err_e sdvr_osd_set_font_table(sdvr_font_table_t *font_desc)
{
    sdvr_err_e err;
    FILE *fp;
    unsigned char *osdyy, *osduu, *osdvv;
    int *osdutf16, *osdindex, *osdwidth, osdheight, osdnglyph, osdyysize;

    if (font_desc == NULL)
        return SDVR_ERR_INVALID_ARG;

    if ((font_desc->font_file == NULL) || (fp=fopen(font_desc->font_file, "rb")) == NULL)
        return SDVR_ERR_OSD_FONT_FILE;

    if (font_desc->start_font_code > font_desc->end_font_code)
        return SDVR_ERR_INVALID_ARG;

    if (font_desc->font_table_id < 8 || font_desc->font_table_id > 16)
        return SDVR_ERR_FONT_ID;

    osdyy = osduu = osdvv = NULL;
    osdutf16 = osdindex = osdwidth= NULL;

    err = _osd_read_font_table(fp, font_desc->start_font_code, font_desc->end_font_code, 
        font_desc->color_y, font_desc->color_u, font_desc->color_v, 
        &osdyy, &osduu, &osdvv, &osdutf16, &osdindex, &osdwidth, &osdheight, &osdnglyph, &osdyysize);
    fclose(fp);

    if (err != SDVR_ERR_NONE)
    {
        return err;
    }

    /*
        Send the YUV font table to all the boards that are connected.
    */

    err = _send_font_table(8, DVR_FONT_FORMAT_YUV_4_2_0,
                           osdyy, osduu, osdvv, 
                           (unsigned char *)osdutf16, (unsigned char *)osdindex, (unsigned char *)osdwidth, 
                           osdheight, osdnglyph, osdyysize);


    if (err == SDVR_ERR_NONE)
        _ssdvr_boards_info.custom_font_table_id = font_desc->font_table_id;
    /*
        free all the OSD tables components.
    */
    if (osdyy)
        free(osdyy);
    if (osduu)
        free (osduu);
    if (osdvv)
        free (osdvv);
    if (osdutf16)
        free (osdutf16);
    if (osdindex)
        free (osdindex);
    if (osdwidth)
        free (osdwidth);

    return err;
}
/****************************************************************************
  After you down load font tables into the firmware by calling 
  sdvr_osd_set_font_table(), you can select which font table to use. By default,
  the last font table down loaded is selected. If no, font table is specified,
  the default English font table is used.

  Font table IDs of less than 8 are reserved for pre-defined system fonts. 
  Currently, there is only one system font ID @SDVR_FT_FONT_ENGLISH@ defined.

  Parameters:

    "font_id" - The font table ID to be selected.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:

    The given font table is used for all the DVR boards that are connected
    at the time of calling this function.
****************************************************************************/
sdvr_err_e sdvr_osd_use_font_table(sx_uint8 font_id)
{
    sdvr_err_e err = SDVR_ERR_BOARD_NOT_CONNECTED;
    sx_uint8 board_index;
    dvr_font_table_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);


    if ((font_id < 8 && font_id != 0) ||
        font_id > 16 ||
        (font_id >= 8 &&  font_id != _ssdvr_boards_info.custom_font_table_id))
        return SDVR_ERR_FONT_ID;

    /*
       select the same font table ID for all the boards that are connected and 
       is still functional
    */
    for (board_index = 0 ; board_index < _ssdvr_boards_info.board_num ; board_index++)
    {
        /* 
           Try a different board to set the font table ID
           If the board is not connected or is dead.
        */
        if (!_ssdvr_boards_info.boards[board_index].handle,
            _ssdvr_boards_info.boards[board_index].is_fw_dead)
            continue;

        /*
           Tell the firmware to use this font table
        */
        memset(&cmd_strct,0,cmd_strct_size);
        cmd_strct.opcode = DVR_FONT_OPCODE_SELECT;
        cmd_strct.font_index = font_id;
        ssdk_message_board_send(board_index, DVR_SET_FONT_TABLE, &cmd_strct, sizeof(cmd_strct));
        ssdk_message_board_recv(board_index, DVR_REP_FONT_TABLE, &cmd_strct, 0, 0);
        err = cmd_strct.status;

        if (err != SDVR_ERR_NONE)
            break;
    } //for (board_index = 0 ; board_index < _ssdvr_boards_info.board_num ; board_index++)

    return err;
}

