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
  This function is used to trigger relays on each SDVR board.

  Parameters:

    "board_index" - Index of the board whose relay you want to set.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "relay_num" - The number of the relay - a board with M relays has the 
    relays numbered sequentially from 0 to M-1 (see schematics for position 
    of relay number 0, 1, and so on). You can determine the number of relays 
    from the board configuration data structure. 

    "is_triggered" - If true, the relay is triggered. 
    If false, the relay is reset.

  Returns:

    SDVR_ERR_NONE - On success.
****************************************************************************/
sdvr_err_e sdvr_trigger_relay(sx_uint32 board_index, 
                              sx_uint32 relay_num, 
                              sx_bool is_triggered)
{
    sdvr_err_e err;
    dvr_relay_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint32 relay_pos = 1;

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE){
        sdbg_msg_output_err(sdvr_trigger_relay_func, err);
        return err;
    }

    memset(&cmd_strct,0,cmd_strct_size);

    /************************************************************************
            1. Get the existing relays status of all the relays
            so that we won't lose any of of the existing relays enable status.
    ************************************************************************/
    ssdk_message_board_send(board_index, DVR_GET_RELAYS, &cmd_strct, sizeof(cmd_strct));
    ssdk_message_board_recv(board_index, DVR_REP_RELAYS, &cmd_strct, 0, 0);

    if (cmd_strct.status == DVR_STATUS_OK )
    {
        /******************************************************************
            2. Set the new enable status of the given relay number
            send it to the firmware.
        ******************************************************************/

        // Go to the corresponding bit for this sensor
        relay_pos <<= relay_num;
        if (is_triggered)
        {
            cmd_strct.relay_status |= relay_pos;
        }
        else
        {
            cmd_strct.relay_status &= ~relay_pos;
        }

        ssdk_message_board_send(board_index, DVR_SET_RELAYS, &cmd_strct, sizeof(cmd_strct));
        ssdk_message_board_recv(board_index, DVR_REP_RELAYS, &cmd_strct, 0, 0);
    }

    sdbg_msg_output(sdvr_trigger_relay_func, board_index, (void *)&cmd_strct);

    return cmd_strct.status;
}

/****************************************************************************
  This function is used to enable or disable a sensor on each SDVR board.

  Parameters:

    "board_index" - Index of the board whose sensor you want to set.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "sensor_num" - The number of the sensor - a board with M sensors has the 
    sensors numbered sequentially from 0 to M-1 (see schematics for position 
    of sensors number 0, 1, etc.). You can determine the number of sensors 
    from the board configuration data structure. 

    "enable" - If true, the sensor is enabled, and if triggered you are
    notified. If false, the sensor is disabled and no notification is
    sent to the DVR Application. Default status of the sensor is disabled.

  Returns:

    SDVR_ERR_NONE - On success.

    SDVR_ERR_BOARD_NOT_CONNECTED - Error code if the board was never connected.

    SDVR_ERR_INVALID_ARG - Error code if the given board index does not exist.
****************************************************************************/
sdvr_err_e sdvr_enable_sensor(sx_uint32 board_index, 
                              sx_uint32 sensor_num,
                              sx_bool enable)
{
    sdvr_err_e err;
    dvr_sensor_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint32 sensor_pos = 1;

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE){
        sdbg_msg_output_err(sdvr_enable_sensor_func, err);
        return err;
    }

    memset(&cmd_strct,0,cmd_strct_size);

    /************************************************************************
            1. Get the existing Sensor parameter so that we won't lose any 
            of sensor configuration or status for other sensors.
    ************************************************************************/
    ssdk_message_board_send(board_index, DVR_GET_SENSORS, &cmd_strct, sizeof(cmd_strct));
    ssdk_message_board_recv(board_index, DVR_REP_SENSORS, &cmd_strct, 0, 0);

    if (cmd_strct.status == DVR_STATUS_OK )
    {
        /******************************************************************
            2. Set the new enable status of the given sensor number
            send it to the firmware.
        ******************************************************************/

        // Go to the corresponding bit for this sensor
        sensor_pos <<= sensor_num;
        if (enable)
        {
            cmd_strct.sensor_enable |= sensor_pos;
        }
        else
        {
            cmd_strct.sensor_enable &= ~sensor_pos;
        }

        ssdk_message_board_send(board_index, DVR_SET_SENSORS, &cmd_strct, sizeof(cmd_strct));
        ssdk_message_board_recv(board_index, DVR_REP_SENSORS, &cmd_strct, 0, 0);
    }

    sdbg_msg_output(sdvr_enable_sensor_func, board_index, (void *)&cmd_strct);

    return cmd_strct.status;
}

/****************************************************************************
  This function is used to specify how each sensor should be triggered as well
  as how to enable or disable each of the sensors on each SDVR board.

  Sensors can be either edge-triggered or level-sensitive. You can specify
  the method to use to determine whether a sensor is triggered.

  A notification message will be sent to the DVR Application for every sensor 
  that is enabled and is triggered. The sensors that are disabled receive no 
  notification messages.

  Parameters:

    "board_index" - Index of the board whose sensors you want to set.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "sensor_enable_map" - A bit map corresponding to each sensor to indicate
    whether it is enabled. The least most significant (LMS) bit of this map
    corresponds to the first sensor, the next LMS bit to the second sensor,
    and so on.  You can determine the number of sensors from the board 
    configuration data structure. The value of 1 for a bit means that the sensor 
    is enabled for the corresponding sensor, and 0 means it is disabled.
    Default status of the sensors is disabled.

    "edge_triggered_map" -  A bit map corresponding to each sensor to indicate
    whether it is edge triggered. The least most significant (LMS) bit of this map
    corresponds to the first sensor, the next LMS bit to the second sensor,
    and so on.  You can determine the number of sensors from the board 
    configuration data structure. The value of 1 for a bit means that the sensor 
    is edge triggered for the corresponding sensor, and 0 means it is 
    level sensitive.

  Returns:

   SDVR_ERR_NONE - On success.

   SDVR_ERR_BOARD_NOT_CONNECTED - Error code if the board was never connected.

   SDVR_ERR_INVALID_ARG - Error code if the given board index does not exist.
****************************************************************************/
 sdvr_err_e sdvr_config_sensors(sx_uint32 board_index, 
                                sx_uint32 sensor_enable_map,
                                sx_uint32 edge_triggered_map)
{
    sdvr_err_e err;
    dvr_sensor_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint32 sensor_pos = 1;

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE){
        sdbg_msg_output_err(sdvr_config_sensors_func, err);
        return err;
    }

    memset(&cmd_strct,0,cmd_strct_size);

    /************************************************************************
            1. Get the existing Sensor information so that we won't lose 
            of sensor's status when changing the triggers configuration.
    ************************************************************************/
    ssdk_message_board_send(board_index, DVR_GET_SENSORS, &cmd_strct, sizeof(cmd_strct));
    ssdk_message_board_recv(board_index, DVR_REP_SENSORS, &cmd_strct, 0, 0);

    if (cmd_strct.status == DVR_STATUS_OK )
    {
        /******************************************************************
            2. Set the new enable status and the trigger format of the 
            sensors.
        ******************************************************************/

        cmd_strct.sensor_enable = sensor_enable_map;
        cmd_strct.edge_triggered = edge_triggered_map;

        ssdk_message_board_send(board_index, DVR_SET_SENSORS, &cmd_strct, sizeof(cmd_strct));
        ssdk_message_board_recv(board_index, DVR_REP_SENSORS, &cmd_strct, 0, 0);
    }

    sdbg_msg_output(sdvr_config_sensors_func, board_index, (void *)&cmd_strct);

    return cmd_strct.status;
}

/*******************************************************************
  This function returns the enable status of the sensor inputs as
  well as whether it is edge triggered

  Parameters:

    "board_index" – Index of the board to be queried.

    "sensor_enable_map" - A pointer to a bit a map corresponding to each 
    sensor to indicate
    whether it is enabled. The least most significant (LMS) bit of this map
    corresponds to the first sensor, the next LMS bit to the second sensor,
    and so on.  The value of 1 for a bit means that the sensor 
    is enabled for the corresponding sensor, and 0 means it is disabled.
    Default status of the sensors is disabled.

    "edge_triggered_map" -  A pointer to a bit map corresponding to each 
    sensor to indicate
    whether it is edge triggered. The least most significant (LMS) bit of this map
    corresponds to the first sensor, the next LMS bit to the second sensor,
    and so on.  The value of 1 for a bit means that the sensor 
    is edge triggered for the corresponding sensor, and 0 means it is 
    level sensitive.

  Returns:

    SDVR_ERR_NONE on success. Otherwise, see the error list for possible
    error codes.

  Remarks:

    You can determine the number of sensors from the board 
    configuration data structure. 
*******************************************************************/
sdvr_err_e
sdvr_get_sensors(sx_uint32 board_index, sx_uint32 *sensor_enable_map,
                                sx_uint32 *edge_triggered_map)

{
    sdvr_err_e err = SDVR_ERR_NONE;
    dvr_sensor_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);

    memset(&cmd_strct,0,cmd_strct_size);

    if (sensor_enable_map == NULL || edge_triggered_map == NULL)
    {
        err = SDVR_ERR_INVALID_ARG;
    }
    else if ((err = ssdk_validate_board_ndx(board_index)) ==  SDVR_ERR_NONE)
    {
        ssdk_message_board_send(board_index, DVR_GET_SENSORS, &cmd_strct, sizeof(cmd_strct));
        err = ssdk_message_board_recv(board_index, DVR_REP_SENSORS, &cmd_strct, 0, 0);
        if (err == SDVR_ERR_NONE)
        {
            *sensor_enable_map = cmd_strct.sensor_enable;
            *edge_triggered_map = cmd_strct.edge_triggered;
        }
        else
        {
            cmd_strct.status = (unsigned char) err & 0xff;
        }
    }

    sdbg_msg_output(sdvr_get_sensors_func, board_index, (void *)&cmd_strct);

    return err;
}

/*******************************************************************
  This function returns the status of the relay outputs.

  Parameters:

    "board_index" – Index of the board to be queried.

    "relay_status" – Pointer to a bit map storage where the current
    relay status is returned.
    Each bit of the bit map identifies the enable 
    state of each relay ordered from the least most significant (LMS) 
    bit corresponding to the first relay.
    The value of 1 for a bit means that the relay 
    is enabled for the corresponding relay, and 0 means it is disabled.

  Returns:

    SDVR_ERR_NONE on success. Otherwise, see the error list for possible
    error codes.

  Remarks:

    You can determine the number of relays from the board 
    configuration data structure. 
*******************************************************************/
sdvr_err_e
sdvr_get_relays(sx_uint32 board_index, sx_uint32 *relays_status)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    dvr_relay_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);

    memset(&cmd_strct,0,cmd_strct_size);

    if (relays_status == NULL)
    {
        err = SDVR_ERR_INVALID_ARG;
    }
    else if ((err = ssdk_validate_board_ndx(board_index)) ==  SDVR_ERR_NONE)
    {
        ssdk_message_board_send(board_index, DVR_GET_RELAYS, &cmd_strct, sizeof(cmd_strct));
        err = ssdk_message_board_recv(board_index, DVR_REP_RELAYS, &cmd_strct, 0, 0);
        if (err == SDVR_ERR_NONE)
        {
            *relays_status = cmd_strct.relay_status;
        }
        else
        {
            cmd_strct.status = (unsigned char) err & 0xff;
        }
    }

    sdbg_msg_output(sdvr_get_relays_func, board_index, (void *)&cmd_strct);

    return err;
}

/*******************************************************************
  This function sets the state of an LED within a given LED type 
  on the board.

  Parameters:

    "board_index" – Index of the board.

    "led_type" – The type of LED to set its enable status.

    "led_num" – A zero based LED number within the LED type group.
    The first LED number is specified by zero (0), second LED number
    is one (1), and so on.

    "enable" – Set to true to turn the LED on, false to turn it off.

  Returns:

    SDVR_ERR_NONE on success. Otherwise, see the error list for possible
    error codes.
*******************************************************************/
sdvr_err_e
sdvr_set_led(sx_uint32 board_index,  sdvr_led_type_e led_type, 
                    sx_uint8 led_num, sx_bool enable)
{
    dvr_ioctl_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sdvr_err_e err;
    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.ioctl_code = DVR_IOCTL_CODE_LED;
    cmd_strct.u1.led.type = (sx_uint8)led_type;
    cmd_strct.u1.led.number = led_num;
    cmd_strct.u1.led.enable = enable;

    cmd_strct.status = SDVR_ERR_NONE;
    /********************************************************
       Validate the parameters:
    ********************************************************/
    cmd_strct.status = ssdk_validate_board_ndx(board_index);

    if (cmd_strct.status ==  SDVR_ERR_NONE)
    {
        cmd_strct.job_type  = 0; // Not Used
        cmd_strct.job_id    = 0; // Not Used

        /**********************************************************
          Send the given value to the given register number on the
          given device ID. 
        ***********************************************************/
        ssdk_message_board_send(board_index, DVR_SET_IOCTL, &cmd_strct, sizeof(cmd_strct));
        err = ssdk_message_board_recv(board_index, DVR_REP_IOCTL, &cmd_strct, 0, 0);
        if (err != SDVR_ERR_NONE)
            cmd_strct.status = err;

    }

    sdbg_msg_output(sdvr_set_led_func,board_index,&cmd_strct);

    return cmd_strct.status;
}



