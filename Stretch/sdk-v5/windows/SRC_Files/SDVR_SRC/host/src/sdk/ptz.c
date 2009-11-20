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

#include "sdvr_sdk_common.h"

/****************************************************************************
  This function is used to initialize the UART port on the S6 to talk
  to the RS-485. 

  Parameters:

    "board_index" - The board number. 
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "baud_rate" - The output baud rate. Must be between 50 and 19200.

    "data_bits" - Number of data bits. Valid values are 5-8.

    "stop_bits" - Number of stop bits. Valid values are 1 and 2.

    "parity_enable" - If this field is set to zero, parity is disabled.
    If set to a nonzero value, parity is enabled.

    "parity_even" - If this field is set to zero, odd parity is used.
    If set to a nonzero value, even parity is used. This field is
    ignored if parity is disabled.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
EXTERN sdvr_err_e sdvr_init_uart(sx_uint32 board_index,
                                  sx_uint32 baud_rate,
                                  sx_uint8 data_bits,
                                  sx_uint8 stop_bits,
                                  sx_uint8 parity_enable,
                                  sx_uint8 parity_even)
{
    sdvr_err_e err;
    dvr_uart_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);

    /********************************************************
       Validate the parameters:
    ********************************************************/
    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE){
        return err;
    }
    if ((baud_rate < 50 || baud_rate > 19200) ||
        (data_bits < 5 || data_bits > 8) ||
        (stop_bits != 1 && stop_bits != 2))
        return SDVR_ERR_INVALID_ARG;


    /**********************************************************
      Initialize the UART
    ***********************************************************/

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.cmd = DVR_UART_CMD_CONFIG;

    cmd_strct.u1.config.baud_rate       = baud_rate;
    cmd_strct.u1.config.data_bits       = data_bits;
    cmd_strct.u1.config.stop_bits       = stop_bits;
    cmd_strct.u1.config.parity_enable   = parity_enable;
    cmd_strct.u1.config.parity_even     = parity_even;

    ssdk_message_board_send(board_index, DVR_SET_UART, &cmd_strct, sizeof(cmd_strct));
    ssdk_message_board_recv(board_index, DVR_REP_UART, &cmd_strct, 0, 0);

    return cmd_strct.status;
}

/****************************************************************************
  This function is used to write a number of 8-bit characters to the RS485 port.

  Before calling this function, the uart must be initialized by 
  calling sdvr_init_uart().

  Parameters:

    "board_index" - The board number. 
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "count" - Number of bytes to transmit. 

    "data" - The data bytes to transmit. The number of valid bytes in
    the array is determined by the "count" field.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_write_uart(sx_uint32 board_index,
                                  sx_uint8 count,
                                  sx_uint8 *data)
{
    sdvr_err_e err;
    dvr_uart_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    int numDataToSend;
    int numDataSent;
    int xfrDataSize = sizeof(cmd_strct.u1.output.data);

    /********************************************************
       Validate the parameters:
    ********************************************************/
    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE){
        return err;
    }
    if (count == 0 || data == NULL)
        return SDVR_ERR_INVALID_ARG;



    /*
       Calculate the number of transfer time to sent
       data in the chuncks of sizeof(cmd_strct.u1.output.data)

    */
    numDataToSend = count / xfrDataSize;
    if ( count % xfrDataSize > 0)
        numDataToSend++;

    /**********************************************************
      Send the given bytes to the UART.

      As we send each chunck of data, we have to recalculate
      the number of bytes that are being sent.

      As the default, each transimission is the size of data 
      field except for the last transmit which could be partial
      one and we have to subtract any extra byte over the
      size of data.
    ***********************************************************/

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.cmd = DVR_UART_CMD_OUTPUT;

    for (numDataSent = 0; numDataSent < numDataToSend; numDataSent++ )
    {
        cmd_strct.u1.output.count = xfrDataSize;

        if (((numDataSent * xfrDataSize) + xfrDataSize) > count)
        {
            int extraBytes = ((numDataSent * xfrDataSize) + xfrDataSize) - count;
            cmd_strct.u1.output.count = xfrDataSize - extraBytes;
        }

        memcpy(cmd_strct.u1.output.data, data + (numDataSent * xfrDataSize), xfrDataSize);
        ssdk_message_board_send(board_index, DVR_SET_UART, &cmd_strct, sizeof(cmd_strct));
        ssdk_message_board_recv(board_index, DVR_REP_UART, &cmd_strct, 0, 0);

        if (cmd_strct.status != SDVR_ERR_NONE)
            break;

    }
 
    return cmd_strct.status;
}

/****************************************************************************
  This function is used to read up to requested number of 8-bit characters 
  from the RS485 port.

  You must initialize the uart by calling sdvr_init_uart()and sent commands 
  to receive data before calling this function.

  It is possible for the data not to be ready on the port at the time of 
  calling this function. In this case, you need to call
  this function multiple time while the actual number of bytes received is 
  zero or is less than the expected amount.

  Parameters:

    "board_index" - The board number. 
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "data_count_read" - A pointer to a variable that holds the actual number
    characters read upon successful return. This count is less than or
    equal to the max_data_size. A value of zero means, there is no data
    available at the port.

    "max_data_size" - The maximum number of bytes request to read from the
    board. @NOTE: The actual returned bytes maybe less than the requested
    ones.@

    "data" - A character pointer long enough to hold the maximum number of
    bytes request.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_read_uart(sx_uint32 board_index,
                          sx_uint8 *data_count_read,
                          sx_uint8 max_data_size,
                          sx_uint8 *data)
{
    sdvr_err_e err;
    dvr_uart_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    int numDataChunkToRcv;
    int numChunkCount;
    int xfrDataSize = sizeof(cmd_strct.u1.input.data);

    /********************************************************
       Validate the parameters:
    ********************************************************/
    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE){
        return err;
    }
    if (max_data_size == 0 || data == NULL || data_count_read == NULL)
        return SDVR_ERR_INVALID_ARG;

    /*
       Calculate the number of transfer time to receive
       data in the chuncks of sizeof(cmd_strct.u1.output.data)

    */
    numDataChunkToRcv = max_data_size / xfrDataSize;
    if ( max_data_size % xfrDataSize > 0)
        numDataChunkToRcv++;

    /**********************************************************
      Receive the request number of bytes from the UART.

      As we receive each chunck of data, we have to recalculate
      the remaining number of requested data.

      As the default, each transimission is the size of data 
      field except for the last transmit which could be partial
      one and we have to subtract any extra byte over the
      size of data.
    ***********************************************************/

    memset(&cmd_strct,0,cmd_strct_size);

    *data_count_read = 0;

    for (numChunkCount = 0; numChunkCount < numDataChunkToRcv; numChunkCount++ )
    {
        cmd_strct.u1.input.count = xfrDataSize;

        if (((numChunkCount * xfrDataSize) + xfrDataSize) > max_data_size)
        {
            int extraBytes = ((numChunkCount * xfrDataSize) + xfrDataSize) - max_data_size;
            cmd_strct.u1.input.count = xfrDataSize - extraBytes;
        }

        ssdk_message_board_send(board_index, DVR_GET_UART, &cmd_strct, sizeof(cmd_strct));
        ssdk_message_board_recv(board_index, DVR_REP_UART, &cmd_strct, 0, 0);

        if (cmd_strct.status != SDVR_ERR_NONE || cmd_strct.u1.input.count == 0)
            break;

        *data_count_read += cmd_strct.u1.input.count;
        memcpy(data + (numChunkCount * xfrDataSize), cmd_strct.u1.output.data,
               cmd_strct.u1.input.count);
    }
 
    return cmd_strct.status;
}


