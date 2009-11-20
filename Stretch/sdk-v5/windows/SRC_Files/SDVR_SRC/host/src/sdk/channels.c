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

/**************************************************************************
                I N T E R N A L   F U N C T I O N S
**************************************************************************/
static sdvr_err_e _send_venc_params(sdvr_chan_handle_t handle,
                             sdvr_sub_encoders_e sub_chan,
                             sdvr_video_enc_chan_params_t *video_enc_params);
static sdvr_err_e _get_venc_params(sdvr_chan_handle_t handle,
                             sdvr_sub_encoders_e sub_chan,
                             sdvr_video_enc_chan_params_t *video_enc_params,
                             sx_uint8 *enable);
static sdvr_err_e _send_enc_alarm_params(sdvr_chan_handle_t handle,
                             sdvr_sub_encoders_e sub_chan,
                             sdvr_alarm_video_enc_params_t *video_enc_params);
static sdvr_err_e _get_enc_alarm_params(sdvr_chan_handle_t handle,
                             sdvr_sub_encoders_e sub_chan,
                             sdvr_alarm_video_enc_params_t *video_enc_params);

static sx_bool _Get_free_region(sdvr_chan_handle_t handle, sdvr_regions_type_e region, int *reg_num);
static sx_bool _is_valid_region(sdvr_chan_handle_t handle, sdvr_regions_type_e region, int reg_num);
static sx_bool _is_valid_region_coordinates(sx_uint8 board_index,
                       sx_uint16 upper_left_x, sx_uint16 upper_left_y,
                       sx_uint16 lower_right_x, sx_uint16 lower_right_y);
static sdvr_err_e _add_motion_region(sdvr_chan_handle_t handle, sdvr_region_t *region);
static sdvr_err_e _add_blind_region(sdvr_chan_handle_t handle, sdvr_region_t *region);
static sdvr_err_e _add_privacy_region(sdvr_chan_handle_t handle, sdvr_region_t *region);
static sdvr_err_e _change_motion_region(sdvr_chan_handle_t handle, sdvr_region_t *region);
static sdvr_err_e _change_blind_region(sdvr_chan_handle_t handle, sdvr_region_t *region);
static sdvr_err_e _change_privacy_region(sdvr_chan_handle_t handle, sdvr_region_t *region);
static sdvr_err_e _remove_motion_region(sdvr_chan_handle_t handle, int reg_num);
static sdvr_err_e _remove_blind_region(sdvr_chan_handle_t handle, int reg_num);
static sdvr_err_e _remove_privacy_region(sdvr_chan_handle_t handle, int reg_num);

/****************************************************************************
  This function is used to get the blind detection parameters for a 
  particular channel. It includes the regions, threshold, and enable/disable
  status.

  Parameters:

    "handle" - Handle of an encoding channel.

    "blind_params" - A pointer to blind detection structure. Up on return
    it will be filled with all the current blind settings for the given 
    channel.
 
  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e _get_blind_params(sdvr_chan_handle_t handle,
                             dvr_bd_t *blind_params)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    dvr_bd_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = CH_GET_CHAN_NUM(handle);
    /*****************************************************************************
             Send Get Blind Detection to the firmware to retrieve the existing
             Blind Detection attributes.
    *****************************************************************************/

    ssdk_message_chan_send(handle, DVR_GET_BD, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_BD, &cmd_strct, 0, 0);

    if (cmd_strct.status == SDVR_ERR_NONE)
    {
        memcpy((void *)blind_params,&cmd_strct,cmd_strct_size);
    }
    sdbg_msg_output(DVR_GET_BD, board_index, (void *)blind_params);

    return cmd_strct.status;
}
/****************************************************************************
  This function sends the video codec parameters for the 
  given encoder on the given channel to the firmware.

  Parameters:

    "handle" - Handle of an encoding channel.

    "sub_chan" - The primary or secondary video encoder on the channel

    "video_enc_params" - video encoder parameters for the given sub-channel
    encoder

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e _send_venc_params(sdvr_chan_handle_t handle,
                             sdvr_sub_encoders_e sub_chan,
                             sdvr_video_enc_chan_params_t *video_enc_params)
{
    dvr_encode_info_t cmd_strct;
    sdvr_video_enc_chan_params_t old_venc_params;
    sdvr_err_e err = SDVR_ERR_NONE;
    int cmd_strct_size = sizeof(cmd_strct);
    int venc_size = sizeof(video_enc_params->encoder);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 enable;
    sdvr_firmware_ver_t *version =  &_ssdvr_boards_info.boards[board_index].version;


    memset(&cmd_strct,0,cmd_strct_size);

    /****************************************************************************
      Get the current enable status of the encoder so that it will not be changed
    *****************************************************************************/
    if ((err = _get_venc_params(handle, sub_chan, &old_venc_params, &enable)) !=  SDVR_ERR_NONE)
    {
        return err;
    }

    /*****************************************************************************
             Setup the video codec parameters
    *****************************************************************************/
    cmd_strct.job_type  = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id    = CH_GET_CHAN_NUM(handle);
    cmd_strct.stream_id = sub_chan;
    cmd_strct.record_frame_rate = video_enc_params->frame_rate;
    cmd_strct.record_res = video_enc_params->res_decimation;
    cmd_strct.enable = enable;
    video_enc_params->reserved1 = 0;
    memcpy(&cmd_strct.u1, &video_enc_params->encoder, venc_size);

    /*****************************************************************************
             send the codec parameters
    *****************************************************************************/
    if (ssdk_is_min_version(version->fw_major,version->fw_minor,version->fw_revision,version->fw_build,
                3,2,0,16))
    {
        ssdk_message_chan_send(handle, DVR_SET_ENCODE, &cmd_strct, cmd_strct_size);
        ssdk_message_chan_recv(handle, DVR_REP_ENCODE, &cmd_strct, 0, 0);
    }
    else  if ((err = ssdvr_send_chan_param( handle, DVR_SET_ENCODE,&cmd_strct, cmd_strct_size)) != SDVR_ERR_NONE)
    {
        return err;
    }

    sdbg_msg_output(DVR_SET_ENCODE, board_index, (void *)&cmd_strct);

    return cmd_strct.status;
}
/****************************************************************************
  This function gets the video codec parameters for the given encoder on the 
  given channel from the firmware. As well as the enable status of the 
  sub-channel encoder.

  Parameters:

    "handle" - Handle of an encoding channel.

    "sub_chan" - The primary or secondary video encoder on the channel

    "video_enc_params" - A pointer to video channel encoder parameter structure.
    Upon successful return, the video encoder parameters will be set.
    This parameter will be ignored if it is NULL

    "enable" - A pointer to hold the enable status of encoder.
    This parameter will be ignored if it is NULL

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e _get_venc_params(sdvr_chan_handle_t handle,
                             sdvr_sub_encoders_e sub_chan,
                             sdvr_video_enc_chan_params_t *video_enc_params,
                             sx_uint8 *enable)
{
    dvr_encode_info_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    int venc_size = sizeof(video_enc_params->encoder);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    /*****************************************************************************
             Setup the video codec parameters
    *****************************************************************************/
    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = CH_GET_CHAN_NUM(handle);
    cmd_strct.stream_id = sub_chan;


    /*****************************************************************************
             get the codec parameters
    *****************************************************************************/
    ssdk_message_chan_send(handle, DVR_GET_ENCODE, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_ENCODE, &cmd_strct, 0, 0);

    sdbg_msg_output(DVR_GET_ENCODE, board_index, (void *)&cmd_strct);

    if (cmd_strct.status == SDVR_ERR_NONE && video_enc_params)
    {
        video_enc_params->frame_rate = cmd_strct.record_frame_rate;
        video_enc_params->res_decimation = cmd_strct.record_res;

        memcpy(&video_enc_params->encoder, &cmd_strct.u1, venc_size);
    }
    if (cmd_strct.status == SDVR_ERR_NONE && enable)
    {
        *enable = cmd_strct.enable;
    }

    return cmd_strct.status;
}

/****************************************************************************
  This function sends the post alarm video codec parameters for the 
  given sub-channel encoder on the given channel to the firmware.

  Parameters:

    "handle" - Handle of an encoding channel.

    "sub_chan" - The primary or secondary video encoder on the channel

    "video_enc_params" - video encoder parameters for the given sub-channel
    encoder

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e _send_enc_alarm_params(sdvr_chan_handle_t handle,
                             sdvr_sub_encoders_e sub_chan,
                             sdvr_alarm_video_enc_params_t *alarm_video_enc_params)
{
    dvr_enc_alarm_info_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    int encoder_size = sizeof(alarm_video_enc_params->encoder);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    memset(&cmd_strct,0,cmd_strct_size);

    /*****************************************************************************
             Setup the video codec parameters
    *****************************************************************************/
    cmd_strct.job_type  = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id    = CH_GET_CHAN_NUM(handle);
    cmd_strct.stream_id = sub_chan;
    cmd_strct.record_frame_rate = alarm_video_enc_params->frame_rate;
    cmd_strct.min_off_seconds   = alarm_video_enc_params->min_off_seconds;
    cmd_strct.min_on_seconds    = alarm_video_enc_params->min_on_seconds;
    cmd_strct.enable            = alarm_video_enc_params->enable;

    memcpy(&cmd_strct.u1, &alarm_video_enc_params->encoder, encoder_size);

    /*****************************************************************************
             send the codec parameters
    *****************************************************************************/
    ssdk_message_chan_send(handle, DVR_SET_ENC_ALARM, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_ENC_ALARM, &cmd_strct, 0, 0);

    if (cmd_strct.status == SDVR_ERR_NONE)
        sdbg_msg_output(DVR_SET_ENC_ALARM, board_index, (void *)&cmd_strct);

    return cmd_strct.status;
}
/****************************************************************************
  This function gets the post alarm video codec parameters for the 
  given sub-channel encoder on the given channel to the firmware.

  Parameters:

    "handle" - Handle of an encoding channel.

    "sub_chan" - The primary or secondary video encoder on the channel

    "video_enc_params" - video encoder parameters for the given sub-channel
    encoder

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e _get_enc_alarm_params(sdvr_chan_handle_t handle,
                             sdvr_sub_encoders_e sub_chan,
                             sdvr_alarm_video_enc_params_t *alarm_video_enc_params)
{
    dvr_enc_alarm_info_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    int encoder_size = sizeof(alarm_video_enc_params->encoder);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    memset(&cmd_strct,0,cmd_strct_size);

    /*****************************************************************************
             Setup the video codec parameters
    *****************************************************************************/
    cmd_strct.job_type  = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id    = CH_GET_CHAN_NUM(handle);
    cmd_strct.stream_id = sub_chan;

    /*****************************************************************************
             send the codec parameters
    *****************************************************************************/
    ssdk_message_chan_send(handle, DVR_GET_ENC_ALARM, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_ENC_ALARM, &cmd_strct, 0, 0);

    if (cmd_strct.status == SDVR_ERR_NONE)
    {
        alarm_video_enc_params->frame_rate         = cmd_strct.record_frame_rate;
        alarm_video_enc_params->min_off_seconds    = cmd_strct.min_off_seconds;
        alarm_video_enc_params->min_on_seconds     = cmd_strct.min_on_seconds;
        alarm_video_enc_params->enable             = cmd_strct.enable;

        memcpy(&alarm_video_enc_params->encoder,&cmd_strct.u1, encoder_size);

        sdbg_msg_output(DVR_GET_ENC_ALARM, board_index, (void *)&cmd_strct);
    }
    return cmd_strct.status;
}

/****************************************************
 Find an available region for the given region type.

 Return:
    false - if no region is availabe for the given type
    otherwise - return true and reg_num contains the
    region number
****************************************************/
sx_bool _Get_free_region(sdvr_chan_handle_t handle, sdvr_regions_type_e region, int *reg_num)
{
    sx_int8 board_index = CH_GET_BOARD_ID(handle);
    sx_int8 chan_num = CH_GET_CHAN_NUM(handle);
    sdvr_chan_type_e chan_type = CH_GET_CHAN_TYPE(handle);

    switch (region)
    {
    case SDVR_REGION_MOTION:
        for (*reg_num = 0; *reg_num < SDVR_MAX_MD_REGIONS; *reg_num +=1)
        {
            if (chan_type == SDVR_CHAN_TYPE_DECODER)
            {
                if (!_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].md_defined_regions[*reg_num])
                    break;
            }
            else
            {
                if (!_ssdvr_boards_info.boards[board_index].recv_channels[chan_num].md_defined_regions[*reg_num])
                    break;
            }
        }
        return (*reg_num < SDVR_MAX_MD_REGIONS);

    case SDVR_REGION_BLIND:
        for (*reg_num = 0; *reg_num < SDVR_MAX_BD_REGIONS; *reg_num +=1)
        {
            if (chan_type == SDVR_CHAN_TYPE_DECODER)
            {
                if (!_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].bd_defined_regions[*reg_num])
                    break;
            }
            else
            {
                if (!_ssdvr_boards_info.boards[board_index].recv_channels[chan_num].bd_defined_regions[*reg_num])
                    break;
            }
        }
        return (*reg_num < SDVR_MAX_BD_REGIONS);

    case SDVR_REGION_PRIVACY:
        for (*reg_num = 0; *reg_num < SDVR_MAX_PR_REGIONS; *reg_num +=1)
        {
            if (chan_type == SDVR_CHAN_TYPE_DECODER)
            {
                if (!_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pr_defined_regions[*reg_num])
                    break;
            }
            else
            {
                if (!_ssdvr_boards_info.boards[board_index].recv_channels[chan_num].pr_defined_regions[*reg_num])
                    break;
            }
        }
    return (*reg_num < SDVR_MAX_PR_REGIONS);
    }

    return false;
}

/****************************************************
 Validate the region for the given region type to make sure it was added.

 Return:
    false - if region was not added
    otherwise true
****************************************************/
sx_bool _is_valid_region(sdvr_chan_handle_t handle, sdvr_regions_type_e region, int reg_num)
{
    sx_int8 board_index = CH_GET_BOARD_ID(handle);
    sx_int8 chan_num = CH_GET_CHAN_NUM(handle);
    sdvr_chan_type_e chan_type = CH_GET_CHAN_TYPE(handle);

    if (reg_num < 0)
        return false;

    switch (region)
    {
    case SDVR_REGION_MOTION:
        if (reg_num < SDVR_MAX_MD_REGIONS)
        {
            if (chan_type == SDVR_CHAN_TYPE_DECODER)
            {
                return _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].md_defined_regions[reg_num];
            }
            else
            {
                return _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].md_defined_regions[reg_num];
            }
        }
        break;

    case SDVR_REGION_BLIND:
        if (reg_num < SDVR_MAX_BD_REGIONS)
        {
            if (chan_type == SDVR_CHAN_TYPE_DECODER)
            {
                return _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].bd_defined_regions[reg_num];
            }
            else
            {
                return _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].bd_defined_regions[reg_num];
            }
        }
        break;

    case SDVR_REGION_PRIVACY:
        if (reg_num < SDVR_MAX_PR_REGIONS)
        {
            if (chan_type == SDVR_CHAN_TYPE_DECODER)
            {
                return _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pr_defined_regions[reg_num];
            }
            else
            {
                return _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].pr_defined_regions[reg_num];
            }
        }
    } //switch (region)

    return false;
}

sx_bool _is_valid_region_type(sdvr_regions_type_e region)
{
    switch (region)
    {
        case SDVR_REGION_MOTION:
        case SDVR_REGION_BLIND:
        case SDVR_REGION_PRIVACY:
            return true;
        default:
            return false;
    }
}

/**********************************************
  This function validate that the given region coordinate is within
  the current video standard of the camera. As well as
  checking the upper left coordinate is less that the lower right 
  coordinates.

********************************************************/
sx_bool _is_valid_region_coordinates(sx_uint8 board_index,
                                     sx_uint16 upper_left_x, sx_uint16 upper_left_y,
                                     sx_uint16 lower_right_x, sx_uint16 lower_right_y)
{

    /*
     Make sure the lower right coordinate is down and to the right 
     of the upper left coordinate
    */
    if (lower_right_x < upper_left_x || lower_right_y < upper_left_y)
        return false;

    /*
       Make sure the lower right coordinate is within the valid
       video frame size for the current video standard.
    */
    if (lower_right_x > _ssdvr_boards_info.boards[board_index].vstd_width ||
        lower_right_y > _ssdvr_boards_info.boards[board_index].vstd_lines)
        return false;

    return true;
}

/****************************************************************************
    Add regions to the specific region type. Up on successful return,
    set the region ID in the given region structure.

****************************************************************************/
sdvr_err_e _add_motion_region(sdvr_chan_handle_t handle, sdvr_region_t *region)
{
    int reg_num;
    sdebug_sdvr_msg_wrapper_t msg_wrapper;
    dvr_md_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_int8 board_index = CH_GET_BOARD_ID(handle);
    sx_int8 chan_num = CH_GET_CHAN_NUM(handle);

    /***************************************
      Try to find an available region to add
    ****************************************/
    if (!_Get_free_region(handle, SDVR_REGION_MOTION, &reg_num))
        return SDVR_ERR_MAX_REGIONS;

    /*************************************
      Add the new region by enabling it
    **************************************/
    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = chan_num;
    cmd_strct.op_code = DVR_VPP_OPCODE_REGIONS;
    cmd_strct.u1.reg.index = reg_num;
    cmd_strct.u1.reg.enable = 1;
    cmd_strct.u1.reg.x_TL = region->upper_left_x;
    cmd_strct.u1.reg.y_TL = region->upper_left_y;
    cmd_strct.u1.reg.x_BR = region->lower_right_x;
    cmd_strct.u1.reg.y_BR = region->lower_right_y;

    msg_wrapper.func_id = SDVR_REGION_MOTION;
    msg_wrapper.strct = &cmd_strct;

    sdbg_msg_output(sdvr_add_region_func, board_index, (void *)&msg_wrapper);

    /*****************************************************************************
             Send Set Motion Detection to the firmware to enable the motion
             detection for the given region.
    *****************************************************************************/

    ssdk_message_chan_send(handle, DVR_SET_MD, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_MD, &cmd_strct, 0, 0);

    /***********************************************
      If the region was enabled successfuly return the ID
      back to the caller. 
      NOTE: That the user region ID are 1 based.
    ************************************************/
    if (cmd_strct.status == SDVR_ERR_NONE)
        region->region_id = cmd_strct.u1.reg.index + 1;

    sdbg_msg_output_err(sdvr_add_region_func, cmd_strct.status);
    return cmd_strct.status;
}

/******************************************************************/

sdvr_err_e _add_blind_region(sdvr_chan_handle_t handle, sdvr_region_t *region)
{
    int reg_num;
    sdebug_sdvr_msg_wrapper_t msg_wrapper;
    dvr_bd_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_int8 board_index = CH_GET_BOARD_ID(handle);
    sx_int8 chan_num = CH_GET_CHAN_NUM(handle);

    /***************************************
      Try to find an available region to add
    ****************************************/
    if (!_Get_free_region(handle, SDVR_REGION_BLIND, &reg_num))
        return SDVR_ERR_MAX_REGIONS;

    /*************************************
      Add the new region by enabling it
    **************************************/
    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = chan_num;
    cmd_strct.op_code = DVR_VPP_OPCODE_REGIONS;
    cmd_strct.u1.reg.index = reg_num;
    cmd_strct.u1.reg.enable = 1;
    cmd_strct.u1.reg.x_TL = region->upper_left_x;
    cmd_strct.u1.reg.y_TL = region->upper_left_y;
    cmd_strct.u1.reg.x_BR = region->lower_right_x;
    cmd_strct.u1.reg.y_BR = region->lower_right_y;

    msg_wrapper.func_id = SDVR_REGION_BLIND;
    msg_wrapper.strct = &cmd_strct;

    sdbg_msg_output(sdvr_add_region_func, board_index, (void *)&msg_wrapper);

    /*****************************************************************************
             Send Set Blind Detection to the firmware to enable the blind
             detection for the given region.
    *****************************************************************************/

    ssdk_message_chan_send(handle, DVR_SET_BD, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_BD, &cmd_strct, 0, 0);

    /***********************************************
      If the region was enabled successfuly return the ID
      back to the caller. 
      NOTE: That the user region ID are 1 based.
    ************************************************/
    if (cmd_strct.status == SDVR_ERR_NONE)
        region->region_id = cmd_strct.u1.reg.index + 1;

    sdbg_msg_output_err(sdvr_add_region_func, cmd_strct.status);
    return cmd_strct.status;
}
/****************************************************************************/
sdvr_err_e _add_privacy_region(sdvr_chan_handle_t handle, sdvr_region_t *region)
{
    int reg_num;
    sdebug_sdvr_msg_wrapper_t msg_wrapper;
    dvr_pr_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_int8 board_index = CH_GET_BOARD_ID(handle);
    sx_int8 chan_num = CH_GET_CHAN_NUM(handle);

    /***************************************
      Try to find an available region to add
    ****************************************/
    if (!_Get_free_region(handle, SDVR_REGION_PRIVACY, &reg_num))
        return SDVR_ERR_MAX_REGIONS;

    /*************************************
      Add the new region by enabling it
    **************************************/
    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = chan_num;
    cmd_strct.op_code = DVR_VPP_OPCODE_REGIONS;
    cmd_strct.u1.reg.index = reg_num;
    cmd_strct.u1.reg.enable = 1;
    cmd_strct.u1.reg.x_TL = region->upper_left_x;
    cmd_strct.u1.reg.y_TL = region->upper_left_y;
    cmd_strct.u1.reg.x_BR = region->lower_right_x;
    cmd_strct.u1.reg.y_BR = region->lower_right_y;

    msg_wrapper.func_id = SDVR_REGION_PRIVACY;
    msg_wrapper.strct = &cmd_strct;

    sdbg_msg_output(sdvr_add_region_func, board_index, (void *)&msg_wrapper);

    /*****************************************************************************
             Send Set Privacy region to the firmware to enable the privacy blacking
             for the given region.
    *****************************************************************************/

    ssdk_message_chan_send(handle, DVR_SET_PR, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_PR, &cmd_strct, 0, 0);

    /***********************************************
      If the region was enabled successfuly return the ID
      back to the caller. 
      NOTE: That the user region ID are 1 based.
    ************************************************/
    if (cmd_strct.status == SDVR_ERR_NONE)
        region->region_id = cmd_strct.u1.reg.index + 1;

    sdbg_msg_output_err(sdvr_add_region_func, cmd_strct.status);
    return cmd_strct.status;
}
/****************************************************************************
    Change the coordinates of the given region for the specific region type. 
****************************************************************************/
sdvr_err_e _change_motion_region(sdvr_chan_handle_t handle, sdvr_region_t *region)
{
    int reg_num = region->region_id;
    sdebug_sdvr_msg_wrapper_t msg_wrapper;
    dvr_md_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_int8 board_index = CH_GET_BOARD_ID(handle);
    sx_int8 chan_num = CH_GET_CHAN_NUM(handle);

    /***************************************
      validate the region number to be changed.
    ****************************************/
    if (!_is_valid_region(handle, SDVR_REGION_MOTION, reg_num-1))
        return SDVR_ERR_INVALID_REGION;

    /*************************************
      Change the region coordinates
    **************************************/
    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = chan_num;
    cmd_strct.op_code = DVR_VPP_OPCODE_REGIONS;
    cmd_strct.u1.reg.index = reg_num - 1;
    cmd_strct.u1.reg.enable = 1;
    cmd_strct.u1.reg.x_TL = region->upper_left_x;
    cmd_strct.u1.reg.y_TL = region->upper_left_y;
    cmd_strct.u1.reg.x_BR = region->lower_right_x;
    cmd_strct.u1.reg.y_BR = region->lower_right_y;

    msg_wrapper.func_id = SDVR_REGION_MOTION;
    msg_wrapper.strct = &cmd_strct;

    sdbg_msg_output(sdvr_change_region_func, board_index, (void *)&msg_wrapper);

    /*****************************************************************************
             Send Set Motion Detection to the firmware to enable the motion
             detection for the given region.
    *****************************************************************************/

    ssdk_message_chan_send(handle, DVR_SET_MD, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_MD, &cmd_strct, 0, 0);

    sdbg_msg_output_err(sdvr_change_region_func, cmd_strct.status);
    return cmd_strct.status;
}

/******************************************************************/

sdvr_err_e _change_blind_region(sdvr_chan_handle_t handle, sdvr_region_t *region)
{
    int reg_num = region->region_id;
    sdebug_sdvr_msg_wrapper_t msg_wrapper;
    dvr_bd_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_int8 board_index = CH_GET_BOARD_ID(handle);
    sx_int8 chan_num = CH_GET_CHAN_NUM(handle);

    /***************************************
      validate the region number to be changed.
    ****************************************/
    if (!_is_valid_region(handle, SDVR_REGION_BLIND, reg_num-1))
        return SDVR_ERR_INVALID_REGION;

    /*************************************
      Change the region coordinates
    **************************************/
    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = chan_num;
    cmd_strct.op_code = DVR_VPP_OPCODE_REGIONS;
    cmd_strct.u1.reg.index = reg_num - 1;
    cmd_strct.u1.reg.enable = 1;
    cmd_strct.u1.reg.x_TL = region->upper_left_x;
    cmd_strct.u1.reg.y_TL = region->upper_left_y;
    cmd_strct.u1.reg.x_BR = region->lower_right_x;
    cmd_strct.u1.reg.y_BR = region->lower_right_y;

    msg_wrapper.func_id = SDVR_REGION_BLIND;
    msg_wrapper.strct = &cmd_strct;

    sdbg_msg_output(sdvr_change_region_func, board_index, (void *)&msg_wrapper);

    /*****************************************************************************
             Send Set Blind Detection to the firmware to enable the blind
             detection for the given region.
    *****************************************************************************/

    ssdk_message_chan_send(handle, DVR_SET_BD, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_BD, &cmd_strct, 0, 0);

    sdbg_msg_output_err(sdvr_change_region_func, cmd_strct.status);
    return cmd_strct.status;
}
/****************************************************************************/
sdvr_err_e _change_privacy_region(sdvr_chan_handle_t handle, sdvr_region_t *region)
{
    int reg_num = region->region_id;
    sdebug_sdvr_msg_wrapper_t msg_wrapper;
    dvr_pr_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_int8 board_index = CH_GET_BOARD_ID(handle);
    sx_int8 chan_num = CH_GET_CHAN_NUM(handle);

    /***************************************
      validate the region number to be changed.
    ****************************************/
    if (!_is_valid_region(handle, SDVR_REGION_PRIVACY, reg_num-1))
        return SDVR_ERR_INVALID_REGION;

    /*************************************
      Change the region coordinates
    **************************************/
    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = chan_num;
    cmd_strct.op_code = DVR_VPP_OPCODE_REGIONS;
    cmd_strct.u1.reg.index = reg_num - 1;
    cmd_strct.u1.reg.enable = 1;
    cmd_strct.u1.reg.x_TL = region->upper_left_x;
    cmd_strct.u1.reg.y_TL = region->upper_left_y;
    cmd_strct.u1.reg.x_BR = region->lower_right_x;
    cmd_strct.u1.reg.y_BR = region->lower_right_y;

    msg_wrapper.func_id = SDVR_REGION_PRIVACY;
    msg_wrapper.strct = &cmd_strct;

    sdbg_msg_output(sdvr_change_region_func, board_index, (void *)&msg_wrapper);

    /*****************************************************************************
             Send Set Privacy region to the firmware to enable the privacy blacking
             for the given region.
    *****************************************************************************/

    ssdk_message_chan_send(handle, DVR_SET_PR, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_PR, &cmd_strct, 0, 0);

    sdbg_msg_output_err(sdvr_change_region_func, cmd_strct.status);
    return cmd_strct.status;
}
/****************************************************************************
    Remove the given region for the specific region type. 

****************************************************************************/
sdvr_err_e _remove_motion_region(sdvr_chan_handle_t handle, int reg_num)
{
    sdebug_sdvr_msg_wrapper_t msg_wrapper;
    dvr_md_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_int8 board_index = CH_GET_BOARD_ID(handle);
    sx_int8 chan_num = CH_GET_CHAN_NUM(handle);

    /***************************************
      validate the region number to be removed.
    ****************************************/
    if (!_is_valid_region(handle, SDVR_REGION_MOTION, reg_num-1))
        return SDVR_ERR_INVALID_REGION;

    /*************************************
      Remove the region by disabling it.
    **************************************/
    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = chan_num;
    cmd_strct.op_code = DVR_VPP_OPCODE_REGIONS;
    cmd_strct.u1.reg.index = reg_num - 1;
    cmd_strct.u1.reg.enable = 0;
    cmd_strct.u1.reg.x_TL = 0;
    cmd_strct.u1.reg.y_TL = 0;
    cmd_strct.u1.reg.x_BR = 0;
    cmd_strct.u1.reg.y_BR = 0;

    msg_wrapper.func_id = SDVR_REGION_MOTION;
    msg_wrapper.strct = &cmd_strct;

    sdbg_msg_output(sdvr_remove_region_func, board_index, (void *)&msg_wrapper);

    /*****************************************************************************
             Send Set Motion Detection to the firmware to disable
             the given region.
    *****************************************************************************/

    ssdk_message_chan_send(handle, DVR_SET_MD, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_MD, &cmd_strct, 0, 0);

    sdbg_msg_output_err(sdvr_remove_region_func, cmd_strct.status);
    return cmd_strct.status;
}

/******************************************************************/

sdvr_err_e _remove_blind_region(sdvr_chan_handle_t handle, int reg_num)
{
    sdebug_sdvr_msg_wrapper_t msg_wrapper;
    dvr_bd_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_int8 board_index = CH_GET_BOARD_ID(handle);
    sx_int8 chan_num = CH_GET_CHAN_NUM(handle);

    /***************************************
      validate the region number to be removed.
    ****************************************/
    if (!_is_valid_region(handle, SDVR_REGION_BLIND, reg_num-1))
        return SDVR_ERR_INVALID_REGION;

    /*************************************
      Remove the region by disabling it.
    **************************************/
    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = chan_num;
    cmd_strct.op_code = DVR_VPP_OPCODE_REGIONS;
    cmd_strct.u1.reg.index = reg_num - 1;
    cmd_strct.u1.reg.enable = 0;
    cmd_strct.u1.reg.x_TL = 0;
    cmd_strct.u1.reg.y_TL = 0;
    cmd_strct.u1.reg.x_BR = 0;
    cmd_strct.u1.reg.y_BR = 0;

    msg_wrapper.func_id = SDVR_REGION_BLIND;
    msg_wrapper.strct = &cmd_strct;

    sdbg_msg_output(sdvr_remove_region_func, board_index, (void *)&msg_wrapper);

    /*****************************************************************************
             Send Set Blind Detection to the firmware to disable 
             the given region.
    *****************************************************************************/

    ssdk_message_chan_send(handle, DVR_SET_BD, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_BD, &cmd_strct, 0, 0);

    sdbg_msg_output_err(sdvr_remove_region_func, cmd_strct.status);
    return cmd_strct.status;
}
/****************************************************************************/
sdvr_err_e _remove_privacy_region(sdvr_chan_handle_t handle, int reg_num)
{
    sdebug_sdvr_msg_wrapper_t msg_wrapper;
    dvr_pr_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_int8 board_index = CH_GET_BOARD_ID(handle);
    sx_int8 chan_num = CH_GET_CHAN_NUM(handle);

    /***************************************
      validate the region number to be removed.
    ****************************************/
    if (!_is_valid_region(handle, SDVR_REGION_PRIVACY, reg_num-1))
        return SDVR_ERR_INVALID_REGION;

    /*************************************
      Remove the region by disabling it.
    **************************************/
    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = chan_num;
    cmd_strct.op_code = DVR_VPP_OPCODE_REGIONS;
    cmd_strct.u1.reg.index = reg_num - 1;
    cmd_strct.u1.reg.enable = 0;
    cmd_strct.u1.reg.x_TL = 0;
    cmd_strct.u1.reg.y_TL = 0;
    cmd_strct.u1.reg.x_BR = 0;
    cmd_strct.u1.reg.y_BR = 0;

    msg_wrapper.func_id = SDVR_REGION_PRIVACY;
    msg_wrapper.strct = &cmd_strct;

    sdbg_msg_output(sdvr_remove_region_func, board_index, (void *)&msg_wrapper);

    /*****************************************************************************
             Send Set Privacy Region to the firmware to disable the 
             region
    *****************************************************************************/

    ssdk_message_chan_send(handle, DVR_SET_PR, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_PR, &cmd_strct, 0, 0);

    sdbg_msg_output_err(sdvr_remove_region_func, cmd_strct.status);
    return cmd_strct.status;
}

/****************************************************************************/
sdvr_err_e _validate_venc_params(sx_uint8 board_index,
                                 dvr_vc_format_e venc_type, sdvr_video_enc_chan_params_t *video_enc_params)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    int max_frame_rate = 0; 
    ssdk_get_vstd_frame_rate(_ssdvr_boards_info.boards[board_index].video_std, &max_frame_rate);

    if (video_enc_params->frame_rate < 1 || video_enc_params->frame_rate > max_frame_rate)
        return SDVR_ERR_INVALID_FPS;

    switch (venc_type)
    {
    case SDVR_VIDEO_ENC_H264:
        if (video_enc_params->encoder.h264.gop == 0)
            return SDVR_ERR_INVALID_GOP;

        switch (video_enc_params->encoder.h264.bitrate_control)
        {
        case SDVR_BITRATE_CONTROL_VBR:
            if (video_enc_params->encoder.h264.avg_bitrate > video_enc_params->encoder.h264.max_bitrate)
                return SDVR_ERR_INVALID_BITRATE;
            break;

        case SDVR_BITRATE_CONTROL_CBR:
            break;

        case SDVR_BITRATE_CONTROL_CQP:
        case SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
            if (video_enc_params->encoder.h264.quality > 100)
                return SDVR_ERR_INVALID_QUALITY;

            break;

        default:
            return SDVR_ERR_INVALID_BITRATE_CONTROL;
        }

        break;

    case SDVR_VIDEO_ENC_MPEG4:
        if (video_enc_params->encoder.mpeg4.gop == 0)
            return SDVR_ERR_INVALID_GOP;

        switch (video_enc_params->encoder.mpeg4.bitrate_control)
        {
        case SDVR_BITRATE_CONTROL_VBR:
            if (video_enc_params->encoder.mpeg4.avg_bitrate > video_enc_params->encoder.mpeg4.max_bitrate)
                return SDVR_ERR_INVALID_BITRATE;
            break;

        case SDVR_BITRATE_CONTROL_CBR:
        case SDVR_BITRATE_CONTROL_CQP:
        case SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
            if (video_enc_params->encoder.mpeg4.quality > 100)
                return SDVR_ERR_INVALID_QUALITY; 

            break;

        default:
            return SDVR_ERR_INVALID_BITRATE_CONTROL;
        }

        break;

    case SDVR_VIDEO_ENC_JPEG:
        if (video_enc_params->encoder.jpeg.quality > 99 ||
            video_enc_params->encoder.jpeg.quality < 1)
            return SDVR_ERR_INVALID_QUALITY;
        break;

    case SDVR_VIDEO_ENC_NONE:
        // This is a HMO channel, there are no encoding parameters
        err = SDVR_ERR_NONE;
        break;
    default:
        err = DVR_STATUS_WRONG_CODEC_FORMAT;
        break;
    }
    return err;
}

/****************************************************************************/
sdvr_err_e _validate_alarm_venc_params(sx_uint8 board_index,
                                       dvr_vc_format_e venc_type, sdvr_alarm_video_enc_params_t *video_enc_params)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    int max_frame_rate = 0; 
    ssdk_get_vstd_frame_rate(_ssdvr_boards_info.boards[board_index].video_std, &max_frame_rate);

    if (video_enc_params->frame_rate < 1 || video_enc_params->frame_rate > max_frame_rate)
        return SDVR_ERR_INVALID_FPS;

    switch (venc_type)
    {
    case SDVR_VIDEO_ENC_H264:
        if (video_enc_params->encoder.h264.gop == 0)
            return SDVR_ERR_INVALID_GOP;

        switch (video_enc_params->encoder.h264.bitrate_control)
        {
        case SDVR_BITRATE_CONTROL_VBR:
            if (video_enc_params->encoder.h264.avg_bitrate > video_enc_params->encoder.h264.max_bitrate)
                return SDVR_ERR_INVALID_BITRATE;
            break;

        case SDVR_BITRATE_CONTROL_CBR:
            break;

        case SDVR_BITRATE_CONTROL_CQP:
        case SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
            if (video_enc_params->encoder.h264.quality > 100)
                return SDVR_ERR_INVALID_QUALITY;

            break;

        default:
            return SDVR_ERR_INVALID_BITRATE_CONTROL;
        }

        break;

    case SDVR_VIDEO_ENC_MPEG4:
        if (video_enc_params->encoder.mpeg4.gop == 0)
            return SDVR_ERR_INVALID_GOP;

        switch (video_enc_params->encoder.mpeg4.bitrate_control)
        {
        case SDVR_BITRATE_CONTROL_VBR:
            if (video_enc_params->encoder.mpeg4.avg_bitrate > video_enc_params->encoder.mpeg4.max_bitrate)
                return SDVR_ERR_INVALID_BITRATE;
            break;

        case SDVR_BITRATE_CONTROL_CBR:
        case SDVR_BITRATE_CONTROL_CQP:
        case SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
            if (video_enc_params->encoder.mpeg4.quality > 100)
                return SDVR_ERR_INVALID_QUALITY; 

            break;

        default:
            return SDVR_ERR_INVALID_BITRATE_CONTROL;
        }
        break;

    case SDVR_VIDEO_ENC_JPEG:
        if (video_enc_params->encoder.jpeg.quality > 99 ||
            video_enc_params->encoder.jpeg.quality < 1)
            return SDVR_ERR_INVALID_QUALITY;
        break;

    case SDVR_VIDEO_ENC_NONE:
        // This is a HMO channel, there are no encoding parameters
        err = SDVR_ERR_NONE;
        break;
    default:
        err = DVR_STATUS_WRONG_CODEC_FORMAT;
        break;
    }
    return err;
}
/****************************************************************************
                            External Function Definitions
 ****************************************************************************/

/****************************************************************************
  This function is used to reset an encoding or decoding channel to its factory 
  default settings. 
  Make sure to close down the channel (stop encoding or decoding) before 
  resetting the channel.

  The default settings of an encoding channel can be found in the 
  description of the sdvr_video_enc_chan_params_t data structure. 
  The call to this function for decoder channel results in no operation
  since currently there is no decoder parameters defined.

  Parameters:

    "handle" - An encoding or decoding channel handle.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_set_channel_default(sdvr_chan_handle_t handle)
{

    dvr_job_t cmd_strct;
    sdvr_err_e err = SDVR_ERR_NONE;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    ssdk_pci_channel_info_t *enc_chan_info; 

    if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE)
    {
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
        sdbg_msg_output_err(sdvr_set_channel_default_func, err);
        return err;
    }

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = CH_GET_CHAN_NUM(handle);
    cmd_strct.control = DVR_JOB_RESET;
    enc_chan_info = ssdk_get_enc_chan_strct(board_index, cmd_strct.job_id);
    switch (cmd_strct.job_type)
    {
        case SDVR_CHAN_TYPE_DECODER:
            cmd_strct.video_format = _ssdvr_boards_info.boards[board_index].decode_channels[cmd_strct.job_id].vdecoder_type;
            break;

        case SDVR_CHAN_TYPE_ENCODER:
            cmd_strct.video_format = enc_chan_info->sub_enc_info[0].venc_type;
            cmd_strct.video_format_2 = enc_chan_info->sub_enc_info[1].venc_type;

            cmd_strct.audio_format = enc_chan_info->aenc_type;
            break;
        default:
            break;
    }

    /*****************************************************************************
             Send Set Video Channel Type to the firmware
    *****************************************************************************/
    if ((err = ssdvr_send_chan_param( handle, DVR_SET_JOB,&cmd_strct, cmd_strct_size)) != SDVR_ERR_NONE)
    {
        return err;
    }

    sdbg_msg_output(sdvr_set_channel_default_func, board_index, (void *)&cmd_strct);

    return cmd_strct.status;
}
/****************************************************************************
  This function is used to create an encoding, decoding, or HMO-only channel.
  
  Each board can have a certain number of cameras connected to it. 
  Call this number M. The encoding channels on the board
  are then numbered from 0 to M-1. Any channel number in this range
  is an encoding channel and corresponds to the physical camera connection
  on the board. Additionally, you can have a certain number of decoders.
  Call this number N. The decoding channels on the board are also numbered
  from 0 to N-1. These decoder channel number are virtual because there is no
  physical hardware attached to them unlike the encoder channels.

  Each Stretch chip is capable of encoding or decoding four D1 video channels,
  but the sum of encoding and decoding channels may not exceed six. 
  This means that channel numbers 0 to 3 are assigned to first Stretch chip, 
  the next four channel numbers are assigned to the second Stretch chip and so on.

  The number of decoding channels
  that can be setup depends on the processing power left after all encoding
  channels are assigned. When this function is called to set up a decoding
  channel, it may fail because there is no more processing power left for
  decoding. If the call is successful in setting up a decoding channel,
  another call to this function can be tried to set up an additional 
  decoding channel. This procedure can be repeated until the calls fails, 
  which indicates that there is no more processing power left.

  Even though you can have 0 to M-1 encoding channels, you do not have
  to use all of them for encoding (perhaps because not that many cameras
  are connected to the board). This allows you to have more decoding
  channels. Use this function to set the encoding channels to the ones
  that have cameras connected to them. 

  A video encoding channel can have up to two different video encoder, as 
  well as an optional audio encoder. By specifying
  a secondary video encoder for a channel, you are taking a way
  more computing power which may not allow you to have four D1 video
  encoding or decoding channels.

  Upon success, this call returns a handle to a channel that is
  used for all other calls that need a channel identifier.

  Parameters:

    "chan_def" - Data structure defining the new channel attributes.

    "handle_ptr" - Upon return, if there is no error, this field holds
    the channel handle.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_create_chan(sdvr_chan_def_t *chan_def,
                            sdvr_chan_handle_t *handle_ptr)
{
    dvr_job_t cmd_strct;
    sdvr_err_e err = SDVR_ERR_NONE;
    int cmd_strct_size = sizeof(cmd_strct);
    sdvr_chan_handle_t handle;
    sx_int8 chan_enc_num = 0;

    if (chan_def == NULL || handle_ptr == NULL)
    {
        err = SDVR_ERR_INVALID_ARG;
    }
    else 
    {
        /*
            Validate the given channel type, number, and board index
            on which we should create the channel.
        */
        err = ssdk_validate_board_chan_type(chan_def->board_index,chan_def->chan_num,chan_def->chan_type);
    }
    /* Validate the channel was not already created */
    if (err == SDVR_ERR_NONE)
    {
        if (chan_def->chan_type == SDVR_CHAN_TYPE_DECODER)
        {
            if (_ssdvr_boards_info.boards[chan_def->board_index].decode_channels[chan_def->chan_num].pci_chan_handle)
                err = SDVR_FRMW_ERR_CHANNEL_ALREADY_CREATED;
        }
        else
        {
            if (_ssdvr_boards_info.boards[chan_def->board_index].recv_channels[chan_def->chan_num].pci_chan_handle)
                err = SDVR_FRMW_ERR_CHANNEL_ALREADY_CREATED;
        }
    }

    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_create_chan_func, err);
        return err;
    }

    /*
        Construct the channel handle using the input
    */
    handle = ssdk_create_chan_handle(chan_def->board_index, 
                                     chan_def->chan_type, 
                                     chan_def->chan_num);

    cmd_strct.job_type = chan_def->chan_type;
    cmd_strct.job_id = chan_def->chan_num;
    cmd_strct.control = DVR_JOB_CREATE;

    cmd_strct.video_format = ssdk_to_fw_video_frmt(chan_def->video_format_primary);
    cmd_strct.video_format_2 = SDVR_VIDEO_ENC_NONE;
    if (cmd_strct.video_format != SDVR_VIDEO_ENC_NONE) {
        chan_enc_num++;
        cmd_strct.video_format_2 = ssdk_to_fw_video_frmt(chan_def->video_format_secondary);
    }
    if (cmd_strct.video_format_2 != SDVR_VIDEO_ENC_NONE)
        chan_enc_num++;

    cmd_strct.audio_format = ssdk_to_fw_audio_frmt(chan_def->audio_format);

    cmd_strct.vpp_mode = chan_def->vpp_mode;

    // In case of encoders we set the camera_type to have the existing
    // video channels for all the cameras
    // Eventhough this field is used by firmware internaly; yet, we intialize it.
    // In case of decoders we ignore it
    if (chan_def->chan_type != SDVR_CHAN_TYPE_DECODER)
        cmd_strct.camera_type = _ssdvr_boards_info.boards[chan_def->board_index].video_std;

    /*****************************************************************************
             Send Set Video Channel Type to the firmware
    *****************************************************************************/

    err = ssdk_message_board_send(chan_def->board_index, DVR_SET_JOB, &cmd_strct, cmd_strct_size);
    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_create_chan_func, err);
        return err;
    }

    /*******************************************************************
     If the channel is being created we must first open a PCI port 
     connection to receive frames. The proper hand shaking is:
     1) send DVR_SET_JOB so the the firmware opens connection PCI port
     2) issue a SCT  accept channel and create all the receiving frame
        buffers.
     3) wait for DVR_REP_JOB
    ********************************************************************/

    /**********************************************************************
       If we are creating a decoder channel we have to open two SCT
       channels one to send the encoded A/V frame to the FW and the other
       to recieve decoded raw A/V frames. Also we have to remember the
       video size supported by this decoder. 
       NOTE: Each decoder can only support decoding of the video frame
             with the size that it was created. This is different than
             the video standard which is assoicated to a camera.
       
       If we are creating an encoder channel we only need to open one
       SCT channel to recieve encoded or raw A/V frames.
    ***********************************************************************/
    if (chan_def->chan_type == SDVR_CHAN_TYPE_DECODER)
    {
        err = ssdk_drv_open_decode_xfr(handle);
        if (err == SDVR_ERR_NONE)
        {
            /*
               Set a default video size for the decoder which
               is the same as current video standard.
               The user should call sdvr_set_decoder_size() to 
               set the desired video size prior enabling the
               decoder
            */
            ssdk_get_width_hieght_vstd(_ssdvr_boards_info.boards[chan_def->board_index].video_std,
                            &_ssdvr_boards_info.boards[chan_def->board_index].decode_channels[chan_def->chan_num].vsize_width,
                            &_ssdvr_boards_info.boards[chan_def->board_index].decode_channels[chan_def->chan_num].vsize_lines);
        }
    }
    else
    {
        err = ssdk_drv_open_rcv(handle,chan_enc_num);
    }

    ssdk_message_chan_recv(handle, DVR_REP_JOB, &cmd_strct, 0, 0);

    /*****************************************************************
       If there was no error, create the channel handle and return it.
    ******************************************************************/
    if (cmd_strct.status == SDVR_ERR_NONE && (err == SDVR_ERR_NONE))
    {
        *handle_ptr = handle;
    }
    else
    {
        if (chan_def->chan_type != SDVR_CHAN_TYPE_DECODER)
            ssdk_reset_enc_chan_info(handle);
        *handle_ptr = 0;
    }

    sdbg_msg_output(sdvr_create_chan_func, chan_def->board_index, (void *)&cmd_strct);
    return cmd_strct.status;
}
/****************************************************************************
  This function is used to destroy the given channel handle. 

  After you have setup some encoder or decoder channels, and you need to add
  more, you may get an error message that there is not enough CPU power to
  add the new channel. In this case, you may decide to temporarily destroy
  one or more channels and add the new channel type.

  NOTE: You must stop all A/V streaming on this channel before 
  destroying it.

  Parameters:

    "handle" - The channel handle to be destroyed.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_destroy_chan(sdvr_chan_handle_t handle)
{
    dvr_job_t cmd_strct;
    dvr_control_t cntrl_strct;
    sdvr_err_e err = SDVR_ERR_NONE;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);

    if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
    else if (ssdk_is_chan_streaming(handle))
        err = SDVR_ERR_IN_STREAMING;

    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_destroy_chan_func, err);
        return err;
    }

    memset(&cntrl_strct,0,sizeof(cntrl_strct));

    cntrl_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cntrl_strct.job_id = CH_GET_CHAN_NUM(handle);

    /************************************************************************
            1. Get the existing running state of the channel.
            
    ************************************************************************/
    memset(&cmd_strct,0,sizeof(cmd_strct));

    cmd_strct.control = DVR_JOB_DESTROY;
    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = CH_GET_CHAN_NUM(handle);

    /*****************************************************************************
         Send Set Video Channel Type to the firmware to destroy the channel
    *****************************************************************************/
    /*******************************************************************
     If the channel is being destroyed we must 
     1) Send the destroy message to the firmware.
     2) close the PCI port
     3) wait for DVR_REP_JOB
    ********************************************************************/

    /**************************************************************
      Send the command to the FW to close the sct channels.
    **************************************************************/
    ssdk_message_chan_send(handle, DVR_SET_JOB, &cmd_strct, cmd_strct_size);

    if (cmd_strct.job_type == SDVR_CHAN_TYPE_DECODER)
        err = ssdk_drv_close_decode_xfr(handle);
    else
        err = ssdk_drv_close_rcv(handle);

    ssdk_message_chan_recv(handle, DVR_REP_JOB, &cmd_strct, 0, 0);

    sdbg_msg_output(sdvr_destroy_chan_func, board_index, (void *)&cmd_strct);

    return cmd_strct.status;
}
/****************************************************************************
  In your DVR Application, you may need to access a certain data structure
  which is associated with an encoder or decoder channel.

  This function allows you to associate a 64-bit variable to a specified channel 
  handle. This data can be retrieved at any time by calling sdvr_get_chan_user_data().

  Parameters:

    "handle" - An encoding or decoding video channel handle.

    "user_data" - Specifies the data to be associated with the channel.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_set_chan_user_data(sdvr_chan_handle_t handle, sx_uint64 user_data)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);

    /**********************************************************************
        Validate the input parameters
    **********************************************************************/
    if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE)
    {
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
    } 

    if (err == SDVR_ERR_NONE)
    {
        if (CH_GET_CHAN_TYPE(handle) == SDVR_CHAN_TYPE_DECODER)
        {
             _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].user_data = user_data;
        }
        else
        {
             _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].user_data = user_data;
        }
    }

    sdbg_msg_output_err(sdvr_set_chan_user_data_func, err);
    return err;
}

/****************************************************************************
  This function lets you retrieve the application-defined data associated 
  with the specified channel handle. This 64-bit variable was set by calling 
  sdvr_get_chan_user_data().

  Parameters:

    "handle" - An encoding or decoding video channel handle.

    "user_data" - A pointer to hold the data associated with this 
    channel.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_get_chan_user_data(sdvr_chan_handle_t handle, sx_uint64 *user_data)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);

    /**********************************************************************
        Validate the input parameters
    **********************************************************************/
    if (user_data == NULL)
    {
        err = SDVR_ERR_INVALID_ARG;
    } 
    else if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE)
    {
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
    } 

    if (err == SDVR_ERR_NONE)
    {
        if (CH_GET_CHAN_TYPE(handle) == SDVR_CHAN_TYPE_DECODER)
        {
            *user_data = _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].user_data;
        }
        else
        {
            *user_data = _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].user_data;
        }
    }

    sdbg_msg_output_err(sdvr_get_chan_user_data_func, err);
    return err;
}

/****************************************************************************
  Regions can be added to any encoding or decoding video channel for motion or 
  blind detection as well as privacy. 
  
  This function adds a new region type based on pixel coordinates to a video channel.
  Number of regions define using this function is limited to 4. 

  After regions are created, you must enable them by calling sdvr_enable_motion_detection(),
  sdvr_enable_blind_detection(), pr sdvr_enable_privacy_regions() as appropriate. 

  @NOTE: This function is kept for backward compatibility.@

  Parameters:

    "handle" - An encoding or decoding video channel handle.

    "region_type" - The type of region to be added.

    "region" - A structure to specify the coordinates of the region as input.
    If the region was added successfully, the region_id field of the structure holds
    the region ID as output. This ID needs to be set to remove or change
    the coordinates of the region.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:
  
  You should call  sdvr_set_regions() or sdvr_set_regions_map() to create 
  unlimited number of regions based on Macro Blocks.
****************************************************************************/
sdvr_err_e sdvr_add_region(sdvr_chan_handle_t handle, sdvr_regions_type_e region_type,
                                    sdvr_region_t *region)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    /**********************************************************************
        Validate the input parameters
    **********************************************************************/
    if (region == NULL)
    {
        err = SDVR_ERR_INVALID_ARG;
    } 
    else if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE)
    {
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
    } 
    else if (!_is_valid_region_coordinates(board_index,
                                           region->upper_left_x,region->upper_left_y,
                                           region->lower_right_x,region->lower_right_y))
    {
        err = SDVR_ERR_INVALID_REGION;
    }

    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_add_region_func, err);
        return err;
    }

    switch (region_type)
    {
    case SDVR_REGION_MOTION:
        err = _add_motion_region(handle, region);
        break;

    case SDVR_REGION_BLIND:
        err = _add_blind_region(handle, region);

        break;
    case SDVR_REGION_PRIVACY:
        err = _add_privacy_region(handle, region);
        break;
    default:
        err = SDVR_ERR_INVALID_ARG;

    }
    sdbg_msg_output_err(sdvr_add_region_func, err);
    return err;
}
/****************************************************************************
  This function is used to change the coordinates of a region that is 
  defined by calling sdvr_add_region(). 

  Parameters:

    "handle" - An encoding/decoding video channel handle.

    "region_type" - The type of region.

    "region" - A structure that specifies the new coordinates of the region.
    The region_id field of the structure holds the region to be changed.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_change_region(sdvr_chan_handle_t handle, sdvr_regions_type_e region_type,
                                     sdvr_region_t *region)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    /**********************************************************************
        Validate the input parameters
    **********************************************************************/
    if (region == NULL)
    {
        err = SDVR_ERR_INVALID_ARG;
    } 
    else if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE)
    {
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
    }

    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_change_region_func, err);
        return err;
    }

    switch (region_type)
    {
    case SDVR_REGION_MOTION:
        err = _change_motion_region(handle, region);
        break;

    case SDVR_REGION_BLIND:
        err = _change_blind_region(handle, region);

        break;
    case SDVR_REGION_PRIVACY:
        err = _change_privacy_region(handle, region);

        break;
    default:
        err = SDVR_ERR_INVALID_ARG;

    }
    sdbg_msg_output_err(sdvr_change_region_func, err);
    return err;

}

/****************************************************************************
  This function is used to remove an existing region that was added by 
  calling sdvr_add_region().

  Parameters:

    "handle" - An encoding or decoding video channel handle.

    "region_type" - The type of region.

    "region_id" - The region ID to be removed from the given region type.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_remove_region(sdvr_chan_handle_t handle, sdvr_regions_type_e region_type,
                                     sx_uint8 region_id)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    /**********************************************************************
        Validate the input parameters
    **********************************************************************/
    if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE)
    {
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
    }

    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_remove_region_func, err);
        return err;
    }

    switch (region_type)
    {
    case SDVR_REGION_MOTION:
        err = _remove_motion_region(handle, region_id);
        break;

    case SDVR_REGION_BLIND:
        err = _remove_blind_region(handle, region_id);

        break;
    case SDVR_REGION_PRIVACY:
        err = _remove_privacy_region(handle, region_id);
        break;

    default:
        err = SDVR_ERR_INVALID_ARG;

    }
    sdbg_msg_output_err(sdvr_remove_region_func, err);
    return err;

}

/****************************************************************************
  This function is used to get the motion threshold, motion detection state,
  and regions for a particular channel. A motion region is where motion is 
  detected by the encoder. If no motion regions are defined, the entire
  video picture is used for motion detection.

  Parameters:

    "handle" -An encoding or decoding video channel handle.

    "motion_detection" - A pointer to a structure describing the motion 
    threshold, regions, and enable state.
 
  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:
  
    Information regarding regions returned in this function only 
    applies to those that were added using sdvr_add_region(). 
    This function is kept for backward compatibility.
****************************************************************************/
sdvr_err_e sdvr_get_motion_detection(sdvr_chan_handle_t handle,
                                     sdvr_motion_detection_t *motion_detection)
{
    dvr_md_t cmd_strct;
    sdebug_sdvr_msg_wrapper_t msg_wrapper;
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 num_regions;
    char last_region_num = -1;
    sx_uint32 cmd_strct_size = sizeof(cmd_strct);

    /**********************************************************************
        Validate the input parameters
    **********************************************************************/
    if (motion_detection == NULL)
    {
        err = SDVR_ERR_INVALID_ARG;
    } 
    else if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE)
    {
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
    }

    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_get_motion_detection_func, err);
        return err;
    }

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = CH_GET_CHAN_NUM(handle);
    
    /*****************************************************************************
       Get all the motion detection regions that are defined for this channel.
    *****************************************************************************/
    for (num_regions = 0; num_regions < SDVR_MAX_MD_REGIONS; num_regions++)
    {
        if (_is_valid_region(handle, SDVR_REGION_MOTION, num_regions))
        {
            cmd_strct.op_code = DVR_VPP_OPCODE_REGIONS;
            cmd_strct.u1.reg.index = num_regions;
            ssdk_message_chan_send(handle, DVR_GET_MD, &cmd_strct, cmd_strct_size);
            ssdk_message_chan_recv(handle, DVR_REP_MD, &cmd_strct, 0, 0);

            err = cmd_strct.status;
            if (err != SDVR_ERR_NONE) {
                sdbg_msg_output_err(sdvr_get_motion_detection_func, err);
                return err;
            }
            // Only add the region if is enabled
            if (cmd_strct.u1.reg.enable)
            {
                last_region_num++;
                motion_detection->regions[last_region_num].region_id = num_regions + 1;
                motion_detection->regions[last_region_num].upper_left_x  = cmd_strct.u1.reg.x_TL;
                motion_detection->regions[last_region_num].upper_left_y  = cmd_strct.u1.reg.y_TL;
                motion_detection->regions[last_region_num].lower_right_x = cmd_strct.u1.reg.x_BR;
                motion_detection->regions[last_region_num].lower_right_y = cmd_strct.u1.reg.y_BR;
            }
        }
    }
    /*****************************************************************************
            retrieve the existing attribute of Motion Detection.
    *****************************************************************************/
    cmd_strct.op_code = DVR_VPP_OPCODE_CONTROL;
    ssdk_message_chan_send(handle, DVR_GET_MD, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_MD, &cmd_strct, 0, 0);

    if (cmd_strct.status != SDVR_ERR_NONE) {
        sdbg_msg_output_err(sdvr_get_motion_detection_func, cmd_strct.status);
        return cmd_strct.status;
    }
    else
    {
        motion_detection->enable = cmd_strct.u1.ctrl.enable;
        motion_detection->threshold = cmd_strct.u1.ctrl.threshold;
        motion_detection->num_regions = last_region_num + 1;
    }

    msg_wrapper.chan_num = cmd_strct.job_id;
    msg_wrapper.chan_type = cmd_strct.job_type;
    msg_wrapper.func_id = sdvr_get_motion_detection_func;
    msg_wrapper.strct = (void *)motion_detection;
    sdbg_msg_output(sdvr_get_motion_detection_func, board_index, (void *)&msg_wrapper);

    return cmd_strct.status;
}
/****************************************************************************
  This function is used to get the blind threshold and regions for a 
  particular channel. A blind detection region is where constant video image is 
  detected by the encoder in the specified region.

  Parameters:

    "handle" - An encoding or decoding channel handle.

    "blind_detection" - A pointer to a structure describing the 
    blind threshold, regions, and enable state.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:
  
    Information regarding regions returned in this function only 
    applies to those that were added using sdvr_add_region(). 
    This function is kept for backward compatibility.

****************************************************************************/
sdvr_err_e sdvr_get_blind_detection(sdvr_chan_handle_t handle,
                                    sdvr_blind_detection_t *blind_detection)
{
    dvr_bd_t cmd_strct;
    sdebug_sdvr_msg_wrapper_t msg_wrapper;
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 num_regions;
    char last_region_num = -1;
    sx_uint32 cmd_strct_size = sizeof(cmd_strct);

    /**********************************************************************
        Validate the input parameters
    **********************************************************************/
    if (blind_detection == NULL)
    {
        err = SDVR_ERR_INVALID_ARG;
    } 
    else if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE)
    {
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
    }

    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_get_blind_detection_func, err);
        return err;
    }

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = CH_GET_CHAN_NUM(handle);
    
    /*****************************************************************************
       Get all the blind detection regions that are defined for this channel.
    *****************************************************************************/
    for (num_regions = 0; num_regions < SDVR_MAX_BD_REGIONS; num_regions++)
    {
        if (_is_valid_region(handle, SDVR_REGION_BLIND, num_regions))
        {
            cmd_strct.op_code = DVR_VPP_OPCODE_REGIONS;
            cmd_strct.u1.reg.index = num_regions;
            ssdk_message_chan_send(handle, DVR_GET_BD, &cmd_strct, cmd_strct_size);
            ssdk_message_chan_recv(handle, DVR_REP_BD, &cmd_strct, 0, 0);

            err = cmd_strct.status;
            if (err != SDVR_ERR_NONE) {
                sdbg_msg_output_err(sdvr_get_blind_detection_func, err);
                return err;
            }
            // Only add the region if is enabled
            if (cmd_strct.u1.reg.enable)
            {
                last_region_num++;
                blind_detection->regions[last_region_num].region_id = num_regions + 1;
                blind_detection->regions[last_region_num].upper_left_x  = cmd_strct.u1.reg.x_TL;
                blind_detection->regions[last_region_num].upper_left_y  = cmd_strct.u1.reg.y_TL;
                blind_detection->regions[last_region_num].lower_right_x = cmd_strct.u1.reg.x_BR;
                blind_detection->regions[last_region_num].lower_right_y = cmd_strct.u1.reg.y_BR;
            }
        }
    }
    /*****************************************************************************
            retrieve the existing attribute of Blind Detection.
    *****************************************************************************/
    cmd_strct.op_code = DVR_VPP_OPCODE_CONTROL;
    ssdk_message_chan_send(handle, DVR_GET_BD, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_BD, &cmd_strct, 0, 0);

    if (cmd_strct.status != SDVR_ERR_NONE) {
        sdbg_msg_output_err(sdvr_get_blind_detection_func, cmd_strct.status);
        return cmd_strct.status;
    }
    else
    {
        blind_detection->enable = cmd_strct.u1.ctrl.enable;
        blind_detection->threshold = cmd_strct.u1.ctrl.threshold;
        blind_detection->num_regions = last_region_num + 1;
    }

    msg_wrapper.chan_num = cmd_strct.job_id;
    msg_wrapper.chan_type = cmd_strct.job_type;
    msg_wrapper.func_id = sdvr_get_blind_detection_func;
    msg_wrapper.strct = (void *)blind_detection;
    sdbg_msg_output(sdvr_get_blind_detection_func, board_index, (void *)&msg_wrapper);

    return cmd_strct.status;
}
/****************************************************************************
  This function is used to get the privacy regions for a particular channel.
  Privacy regions are blanked in live and encoded video.

  Parameters:

    "handle" - Handle of an encoding or decoding channel.

    "privacy_region" - A pointer to a data structure containing the region(s)
    to blanked as well as their enable state(s).

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:
  
    Information regarding regions returned in this function only 
    applies to those that were added using sdvr_add_region(). 
    This function is kept for backward compatibility.

****************************************************************************/
sdvr_err_e sdvr_get_privacy_regions(sdvr_chan_handle_t handle,
                                    sdvr_privacy_region_t *privacy_regions)
{
    dvr_pr_t cmd_strct;
    sdebug_sdvr_msg_wrapper_t msg_wrapper;
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 num_regions;
    char last_region_num = -1;
    sx_uint32 cmd_strct_size = sizeof(cmd_strct);

    /**********************************************************************
        Validate the input parameters
    **********************************************************************/
    if (privacy_regions == NULL)
    {
        err = SDVR_ERR_INVALID_ARG;
    } 
    else if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE)
    {
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
    }

    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_get_privacy_regions_func, err);
        return err;
    }

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = CH_GET_CHAN_NUM(handle);
    
    /*****************************************************************************
       Get all the privacy regions that are defined for this channel.
    *****************************************************************************/
    for (num_regions = 0; num_regions < SDVR_MAX_PR_REGIONS; num_regions++)
    {
        if (_is_valid_region(handle, SDVR_REGION_PRIVACY, num_regions))
        {
            cmd_strct.op_code = DVR_VPP_OPCODE_REGIONS;
            cmd_strct.u1.reg.index = num_regions;
            ssdk_message_chan_send(handle, DVR_GET_PR, &cmd_strct, cmd_strct_size);
            ssdk_message_chan_recv(handle, DVR_REP_PR, &cmd_strct, 0, 0);

            err = cmd_strct.status;
            if (err != SDVR_ERR_NONE) {
                sdbg_msg_output_err(sdvr_get_privacy_regions_func, err);
                return err;
            }
            // Only add the region if is enabled
            if (cmd_strct.u1.reg.enable)
            {
                last_region_num++;
                privacy_regions->regions[last_region_num].region_id = num_regions + 1;
                privacy_regions->regions[last_region_num].upper_left_x  = cmd_strct.u1.reg.x_TL;
                privacy_regions->regions[last_region_num].upper_left_y  = cmd_strct.u1.reg.y_TL;
                privacy_regions->regions[last_region_num].lower_right_x = cmd_strct.u1.reg.x_BR;
                privacy_regions->regions[last_region_num].lower_right_y = cmd_strct.u1.reg.y_BR;
            }
        }
    }
    /*****************************************************************************
            retrieve the existing attribute of Blind Detection.
    *****************************************************************************/
    cmd_strct.op_code = DVR_VPP_OPCODE_CONTROL;
    ssdk_message_chan_send(handle, DVR_GET_PR, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_PR, &cmd_strct, 0, 0);

    if (cmd_strct.status != SDVR_ERR_NONE) {
        sdbg_msg_output_err(sdvr_get_privacy_regions_func, cmd_strct.status);
        return cmd_strct.status;
    }
    else
    {
        privacy_regions->enable = cmd_strct.u1.ctrl.enable;
        privacy_regions->num_regions = last_region_num + 1;
    }

    msg_wrapper.chan_num = cmd_strct.job_id;
    msg_wrapper.chan_type = cmd_strct.job_type;
    msg_wrapper.func_id = sdvr_get_privacy_regions_func;
    msg_wrapper.strct = (void *)privacy_regions;
    sdbg_msg_output(sdvr_get_privacy_regions_func, board_index, (void *)&msg_wrapper);

    return cmd_strct.status;
}
/****************************************************************************
  This function is used to get the threshold and enable state of 
  night detection.

  Night detection refers to the entire picture, and 
  not to any particular region of the picture. 

  Parameters:

    "handle" - An encoding or decoding channel handle.

    "night_detection" - Pointer to a structure containing the threshold and
    enable state for night detection.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_get_night_detection(sdvr_chan_handle_t handle,
                                    sdvr_night_detection_t *night_detection)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    dvr_nd_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    /**************************************************************
      Validate the input parameters
    **************************************************************/
    if (night_detection == NULL)
        err =  SDVR_ERR_INVALID_ARG;
    else if (ssdk_validate_chan_handle(handle, -1) !=  SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;

    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_get_night_detection_func, err);
        return err;
    }

    /************************************************************************
   Retrieve Night Detection parameters for the given channel from the firmware
    *************************************************************************/

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = CH_GET_CHAN_NUM(handle);

    ssdk_message_chan_send(handle, DVR_GET_ND, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_ND, &cmd_strct, 0, 0);
    sdbg_msg_output(sdvr_get_night_detection_func, board_index, (void *)&cmd_strct);

    if (cmd_strct.status == SDVR_ERR_NONE)
    {
        night_detection->enable = cmd_strct.enable;
        night_detection->threshold = cmd_strct.threshold;
    }

    sdbg_msg_output_err(sdvr_get_night_detection_func, err);

    return cmd_strct.status;
}
/****************************************************************************
  This function is used to enable or disable all the privacy regions that are
  defined by calling the function sdvr_add_region(). After
  privacy region is enabled, all the regions are blacked out in both the live/decoded
  and encoded video. If no privacy region is defined, the entire video picture
  will be blacked out.

  Parameters:

    "handle" - An encoding or decoding channel handle.

    "enable" - A non-zero value enables privacy regions, zero 
    disables privacy regions. 

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_enable_privacy_regions(sdvr_chan_handle_t handle,
                                       sx_bool enable)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    dvr_pr_t cmd_strct;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    int cmd_strct_size = sizeof(cmd_strct);

    if (ssdk_validate_chan_handle(handle, -1) !=  SDVR_ERR_NONE)
    {
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
        sdbg_msg_output_err(sdvr_enable_privacy_regions_func, err);
        return err;
    }

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = CH_GET_CHAN_NUM(handle);
    cmd_strct.op_code = DVR_VPP_OPCODE_CONTROL;
    cmd_strct.u1.ctrl.enable = enable;

    sdbg_msg_output(sdvr_enable_privacy_regions_func, board_index, (void *)&cmd_strct);

    /*****************************************************************************
             Send Set privacy region to the firmware to black out
             all the defined regions.
    *****************************************************************************/

    ssdk_message_chan_send(handle, DVR_SET_PR, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_PR, &cmd_strct, 0, 0);

    err = cmd_strct.status;

    sdbg_msg_output_err(sdvr_enable_privacy_regions_func, err);
    return err;
}
/****************************************************************************
  After regions are specified for motion detection by calling the function 
  sdvr_add_region() or sdvr_set_regions_map(), you must enable motion detection.  

  This function is used to enable and disable motion detection on all the 
  specified regions. The motion detection uses the entire picture if no motion
  region is specified. As you enable the motion detection, you must specify 
  a threshold above which motion should be detected.

  After motion detection is enabled, the video alarm callback function is 
  called every time motion is detected. Addtionally, video frame callback
  function is called with SDVR_FRAME_MOTION_VALUES frame type that holds the 
  macro block motion values for the video frame with the motion.


  Parameters:

    "handle" - An encoding or decoding channel handle.

    "enable" - A non-zero value enables motion detection, 
    zero disables motion detection.

    "threshold" - The threshold value for motion detection. This field is
    ignored if you are disabling motion detection.
    The valid range is 0 - 99. 

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:

    This function only sets the motion threshold for the first motion overlay 
    region. To set the threshold for other motion overlay call 
    sdvr_enable_motion_detection_ex().
****************************************************************************/
sdvr_err_e sdvr_enable_motion_detection(sdvr_chan_handle_t handle,
                                        sx_bool enable,
                                        sx_uint8 threshold)
{
    dvr_md_t cmd_strct;
    sdvr_err_e err = SDVR_ERR_NONE;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    if (ssdk_validate_chan_handle(handle, -1) !=  SDVR_ERR_NONE)
    {
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
        sdbg_msg_output_err(sdvr_enable_motion_detection_func, err);
        return err;
    }

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = CH_GET_CHAN_NUM(handle);
    cmd_strct.op_code = DVR_VPP_OPCODE_CONTROL;
    cmd_strct.u1.ctrl.enable = enable;
    cmd_strct.u1.ctrl.threshold = threshold;

    sdbg_msg_output(sdvr_enable_motion_detection_func, board_index, (void *)&cmd_strct);

    /*****************************************************************************
             Send Set Motion Detection to the firmware to enable the motion
             detection on all the defined regions.
    *****************************************************************************/

    ssdk_message_chan_send(handle, DVR_SET_MD, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_MD, &cmd_strct, 0, 0);

    err = cmd_strct.status;

    sdbg_msg_output_err(sdvr_enable_motion_detection_func, err);
    return err;
}

/****************************************************************************
  After regions are specified for motion detection by calling the function 
  sdvr_set_regions_map(), you must enable motion detection.  
  This is an extended version of sdvr_enable_motion_detection() that allows 
  you to set the motion detection threshold for multiple motion
  overlay regions. 

  This function is used to enable and disable motion detection on all the 
  specified regions. The motion detection uses the entire picture if no motion
  region is specified. As you enable the motion detection, you must specify 
  a threshold above which motion should be detected for each motion region overlay.

  After motion detection is enabled, the video alarm callback function is 
  called every time motion is detected. Addtionally, video frame callback
  function is called with SDVR_FRAME_MOTION_VALUES frame type that holds the 
  macro block motion values for the video frame with the motion.

  Parameters:

    "handle" - An encoding or decoding channel handle.

    "enable" - A non-zero value enables motion detection, 
    zero disables motion detection.

    "threshold1" - The threshold value for the first motion detection overlay
    region. 
    The valid range is 0 - 99. 

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:

    Threshold fields are ignored if you are disabling motion detection.
****************************************************************************/
sdvr_err_e sdvr_enable_motion_detection_ex(sdvr_chan_handle_t handle,
                                        sx_bool enable,
                                        sx_uint8 threshold1,
                                        sx_uint8 threshold2,
                                        sx_uint8 threshold3,
                                        sx_uint8 threshold4)
{
    dvr_regions_map_t cmd_strct;
    sdvr_err_e err = SDVR_ERR_NONE;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    if (ssdk_validate_chan_handle(handle, -1) !=  SDVR_ERR_NONE)
    {
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
        sdbg_msg_output_err(sdvr_enable_motion_detection_ex_func, err);
        return err;
    }

    if (enable)
    {
        /********************************************************
          If we are enabling the motion detection,
          set the motion threshold values for all the 
          overlay regions.
        ********************************************************/

        memset(&cmd_strct,0,cmd_strct_size);

        cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
        cmd_strct.job_id = CH_GET_CHAN_NUM(handle);
        cmd_strct.region_type = DVR_REGION_TYPE_MD;
        cmd_strct.opcode = DVR_REGION_OPCODE_CONFIG;
        cmd_strct.u1.config.threshold1 = threshold1;
        cmd_strct.u1.config.threshold2 = threshold2;
        cmd_strct.u1.config.threshold3 = threshold3;
        cmd_strct.u1.config.threshold4 = threshold4;

        sdbg_msg_output(sdvr_enable_motion_detection_ex_func, board_index, (void *)&cmd_strct);

        /*****************************************************************************
                 Send Set Motion Detection to the firmware to enable the motion
                 detection on all the defined regions.
        *****************************************************************************/

        err = ssdk_message_chan_send(handle, DVR_SET_REGIONS_MAP, &cmd_strct, cmd_strct_size);
        err = ssdk_message_chan_recv(handle, DVR_REP_REGIONS_MAP, &cmd_strct, 0, 0);

        err = cmd_strct.status;
    }
    if (err == SDVR_ERR_NONE)
    {
        /********************************************************
          After  motion threshold values for all the overlay regions
          are set successfully, enable/disable motion detection.
        ********************************************************/

        memset(&cmd_strct,0,cmd_strct_size);

        cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
        cmd_strct.job_id = CH_GET_CHAN_NUM(handle);
        cmd_strct.region_type = DVR_REGION_TYPE_MD;
        cmd_strct.opcode = DVR_REGION_OPCODE_ENABLE;
        cmd_strct.u1.enable.enable_flag = enable;

        sdbg_msg_output(sdvr_enable_motion_detection_ex_func, board_index, (void *)&cmd_strct);

        /*****************************************************************************
                 Send Set Motion Detection to the firmware to enable the motion
                 detection on all the defined regions.
        *****************************************************************************/

        err = ssdk_message_chan_send(handle, DVR_SET_REGIONS_MAP, &cmd_strct, cmd_strct_size);
        err = ssdk_message_chan_recv(handle, DVR_REP_REGIONS_MAP, &cmd_strct, 0, 0);
       
        err = cmd_strct.status;

    }

    sdbg_msg_output_err(sdvr_enable_motion_detection_ex_func, err);
    return err;
}
/****************************************************************************
  After regions are specified for blind detection by calling the function 
  sdvr_add_region(), you must enable blind detection.  

  This function is used to enable and disable blind detection on all the 
  specified regions. The blind detection uses the entire picture if no
  region is specified. As you enable the blind detection, you must specify 
  a threshold above which blind detection is triggered.

  After blind detection is enabled, the video alarm callback function is 
  called every time video blind is detected.

  Parameters:

    "handle" - Handle of an encoding/decoding channel.

    "enable" - A non-zero value enables blind detection, 
    zero disables blind detection.

    "threshold" - The threshold value for blind detection. This field is
    ignored if you are disabling the blind detection.
    The valid range is 0 - 99. 

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_enable_blind_detection(sdvr_chan_handle_t handle,
                                       sx_bool enable,
                                       sx_uint8 threshold)
{
    dvr_md_t cmd_strct;
    sdvr_err_e err = SDVR_ERR_NONE;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    if (ssdk_validate_chan_handle(handle, -1) !=  SDVR_ERR_NONE)
    {
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
        sdbg_msg_output_err(sdvr_enable_blind_detection_func, err);
        return err;
    }

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = CH_GET_CHAN_NUM(handle);
    cmd_strct.op_code = DVR_VPP_OPCODE_CONTROL;
    cmd_strct.u1.ctrl.enable = enable;
    cmd_strct.u1.ctrl.threshold = threshold;

    sdbg_msg_output(sdvr_enable_blind_detection_func, board_index, (void *)&cmd_strct);

    /*****************************************************************************
             Send Set Blind Detection to the firmware to enable the motion
             detection on all the defined regions.
    *****************************************************************************/

    ssdk_message_chan_send(handle, DVR_SET_BD, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_BD, &cmd_strct, 0, 0);

    err = cmd_strct.status;

    sdbg_msg_output_err(sdvr_enable_blind_detection_func, err);
    return err;
}

/****************************************************************************
  This function is used to enable and disable night detection.

  Night detection refers to the entire picture, and 
  not to any particular region of the picture. As you enable the night 
  detection, you must specify 
  a threshold below which night should be detected.

  Parameters:

    "handle" - An encoding or decoding channel handle.

    "enable" - A non-zero value enables night detection,
    zero disables night detection.

    "threshold" - The threshold value for night detection. This field is
    ignored if you are disabling night detection.
    The valid range is 0 - 255. 

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_enable_night_detection(sdvr_chan_handle_t handle,
                                       sx_bool enable,
                                       sx_uint8 threshold)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    dvr_nd_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    if (ssdk_validate_chan_handle(handle, -1) !=  SDVR_ERR_NONE)
    {
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
        sdbg_msg_output_err(sdvr_enable_night_detection_func, err);
        return err;
    }

    memset(&cmd_strct,0,cmd_strct_size);

    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.job_id = CH_GET_CHAN_NUM(handle);
    cmd_strct.enable = enable;
    cmd_strct.threshold = threshold;

    sdbg_msg_output(sdvr_enable_night_detection_func, board_index, (void *)&cmd_strct);

    /*****************************************************************************
             Send Set Night Detection to the firmware to enable the night
             detection on all the defined regions.
    *****************************************************************************/

    ssdk_message_chan_send(handle, DVR_SET_ND, &cmd_strct, cmd_strct_size);
    ssdk_message_chan_recv(handle, DVR_REP_ND, &cmd_strct, 0, 0);

    err = cmd_strct.status;

    sdbg_msg_output_err(sdvr_enable_night_detection_func, err);
    return err;
}

/****************************************************************************
  This function is used to set the video parameters for either the primary or
  secondary encoder of a video encoder channel.
  The encoder channel parameters are determined by the type of connected camera,
  the resolution, frame rate, and so on. All of these parameters are
  defined in sdvr_video_enc_chan_params_t, and are set using this
  function. Any video encoder parameter except the video resolution can
  be changed regardless of the encoder enable status. This means to change
  the video resolution of a channel the encoder must be disabled first before
  calling this function.

  There are no corresponding functions for setting the parameters
  for a video decoding channel. This is because the video decoding 
  channel takes its parameters from the encoded stream. 

  The encoder-specific parameters are ignored for any channel
  that has its primary encoder set to SDVR_VIDEO_ENC_NONE.

  Parameters:

    "handle" - An encoding channel handle.

    "sub_chan_enc" - Specifies whether to set the parameters for the primary
    or secondary encoder.

    "video_enc_params" - The parameters for the video encoder channel. 
    See sdvr_video_enc_chan_params_t for detailed information of each
    parameter.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:
  
    Any of the video encoder parameters can be changed while the encoder is enabled,
    except video resolution. If the encoder is enabled, all the encoder
    parameter changes take place at the beginning of the next GOP with the exception of
    frame rate and GOP size which takes place immediately.
****************************************************************************/
sdvr_err_e sdvr_set_video_encoder_channel_params(sdvr_chan_handle_t handle,
                                                 sdvr_sub_encoders_e sub_chan_enc,
                                                 sdvr_video_enc_chan_params_t *video_enc_params)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

    if (video_enc_params == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else if (ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER) !=  SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
    else if ((err = ssdk_validate_enc_sub_chan( handle, sub_chan_enc)) == SDVR_ERR_NONE)
            err = _validate_venc_params(board_index,
                                       enc_chan_info->sub_enc_info[sub_chan_enc].venc_type,
                                       video_enc_params);
    else
        err = SDVR_ERR_INVALID_CHAN_HANDLE;


    if (err !=  SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_set_video_encoder_channel_params_func, err);
        return err;
    }

    sdbg_msg_output(sdvr_set_video_encoder_channel_params_func, board_index, (void*)&handle);

    /***************************************************************************
        Depending on the current video codec on the given channel, retrieve the
        parameters from the given video_enc_params and send it to the firmware.
    ****************************************************************************/
    switch (enc_chan_info->sub_enc_info[sub_chan_enc].venc_type)
    {
    case SDVR_VIDEO_ENC_H264:
    case SDVR_VIDEO_ENC_MPEG4:
    case SDVR_VIDEO_ENC_JPEG:
        err = _send_venc_params(handle, sub_chan_enc, video_enc_params);
        if (err == SDVR_ERR_NONE)
        {
            enc_chan_info->sub_enc_info[sub_chan_enc].res_decimation =
                video_enc_params->res_decimation;
        }
        break;

    case SDVR_VIDEO_ENC_NONE:
        // This is a HMO channel, there are no encoding parameters
        err = SDVR_ERR_NONE;
        break;
    default:
        err = DVR_STATUS_WRONG_CODEC_FORMAT;
        break;
    }

    sdbg_msg_output_err(sdvr_set_video_encoder_channel_params_func, err);
    return err;
}

/****************************************************************************
  This function is used to get the parameters of a video encoder channel. 

  The encoder-specific parameters for any channel
  that has its primary encoder set to SDVR_VIDEO_ENC_NONE have no meaning.

  Parameters:

    "handle" - An encoding channel handle.

    "sub_chan_enc" - Specifies whether to set the parameters for the primary
    or secondary encoder.

    "video_enc_params" - A pointer to a variable that will be filled with
    encoder channel parameters when this function returns. 

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_get_video_encoder_channel_params(sdvr_chan_handle_t handle,
                                                 sdvr_sub_encoders_e sub_chan_enc,
                                                 sdvr_video_enc_chan_params_t *video_enc_params)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);


    if (video_enc_params == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else if (ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER) !=  SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
    else
        err = ssdk_validate_enc_sub_chan( handle, sub_chan_enc);

    if (err !=  SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_get_video_encoder_channel_params_func, err);
        return err;
    }

    sdbg_msg_output(sdvr_get_video_encoder_channel_params_func, board_index, (void*)&handle);

    /***************************************************************************
        Depending on the current video codec on the given channel, retrieve the
        parameters from the given video_enc_params and send it to the firmware.
    ****************************************************************************/
    switch (ssdk_get_enc_chan_strct(board_index, chan_num)->sub_enc_info[sub_chan_enc].venc_type)
    {
    case SDVR_VIDEO_ENC_H264:
    case SDVR_VIDEO_ENC_MPEG4:
    case SDVR_VIDEO_ENC_JPEG:
        err = _get_venc_params(handle, sub_chan_enc, video_enc_params,NULL);
        break;

    case SDVR_VIDEO_ENC_NONE:
        // This is a HMO channel, there are no encoding parameters
        err = SDVR_ERR_NONE;
        break;

    default:
        err = DVR_STATUS_WRONG_CODEC_FORMAT;
        break;
    }

    sdbg_msg_output_err(sdvr_get_video_encoder_channel_params_func, err);
    return err;
}
/****************************************************************************
  This function is used to set the parameters of a video encoder channel
  after any of the alarms are triggered. You can specify two sets of
  video encoder parameters. One to be used on streaming of video encoded
  frames before alarms are triggered. The other to be used after any of the
  alarms is triggered for a minimum duration. 

  Note: These parameters are used only for encoded channels and while 
  they are enabled. Otherwise, the value of the encoder parameters for both
  pre- and post-alarm condition is ignored.

  Parameters:

    "handle" - Handle of an encoding channel.

    "sub_chan_enc" - Specifies whether to set the parameters for the primary
    or secondary encoder.

    "alarm_video_enc_params" - The parameters for the video encoder channel
    after an alarm is triggered.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_set_alarm_video_encoder_params(sdvr_chan_handle_t handle,
                                               sdvr_sub_encoders_e sub_chan_enc,
                        sdvr_alarm_video_enc_params_t *alarm_video_enc_params)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

    if (alarm_video_enc_params == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else if (ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER) !=  SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
    else if ((err = ssdk_validate_enc_sub_chan( handle, sub_chan_enc)) == SDVR_ERR_NONE)
            err = _validate_alarm_venc_params(board_index,
                                       enc_chan_info->sub_enc_info[sub_chan_enc].venc_type,
                                       alarm_video_enc_params);
    if (err !=  SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_set_alarm_video_encoder_params_func, err);
        return err;
    }

    sdbg_msg_output(sdvr_set_alarm_video_encoder_params_func, board_index, (void*)&handle);

    /***************************************************************************
        Depending on the current video codec on the given channel, retrieve the
        parameters from the given video_enc_params and send it to the firmware.
    ****************************************************************************/
    switch (enc_chan_info->sub_enc_info[sub_chan_enc].venc_type)
    {
    case SDVR_VIDEO_ENC_H264:
    case SDVR_VIDEO_ENC_MPEG4:
    case SDVR_VIDEO_ENC_JPEG:
        err = _send_enc_alarm_params(handle, sub_chan_enc, alarm_video_enc_params);
        break;
    case SDVR_VIDEO_ENC_NONE:
        // This is a HMO channel, there are no encoding parameters
        err = SDVR_ERR_NONE;
        break;
    default:
        err = DVR_STATUS_WRONG_CODEC_FORMAT;
        break;
    }

    sdbg_msg_output_err(sdvr_set_alarm_video_encoder_params_func, err);
    return err;
}
/****************************************************************************
  This function is used to get the parameters of a video encoder channel after
  an alarm has triggered.

  Parameters:

    "handle" - Handle of an encoding channel.

    "sub_chan_enc" - Specifies whether to set the parameters for the primary
    or secondary encoder.

    "alarm_video_enc_params" - A pointer to a variable that will be filled 
    with post-alarm encoder channel parameters when this function returns. 

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_get_alarm_video_encoder_params(sdvr_chan_handle_t handle,
                                               sdvr_sub_encoders_e sub_chan_enc,
                        sdvr_alarm_video_enc_params_t *alarm_video_enc_params)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);

    if (alarm_video_enc_params == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else if (ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER) !=  SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
    else
        err = ssdk_validate_enc_sub_chan( handle, sub_chan_enc);

    if (err !=  SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_get_alarm_video_encoder_params_func, err);
        return err;
    }

    sdbg_msg_output(sdvr_get_alarm_video_encoder_params_func, board_index, (void*)&handle);

    /***************************************************************************
        Depending on the current video codec on the given channel, retrieve the
        parameters from the given video_enc_params and send it to the firmware.
    ****************************************************************************/
    switch (ssdk_get_enc_chan_strct(board_index,chan_num)->sub_enc_info[sub_chan_enc].venc_type)
    {
    case SDVR_VIDEO_ENC_H264:
    case SDVR_VIDEO_ENC_MPEG4:
    case SDVR_VIDEO_ENC_JPEG:
        err = _get_enc_alarm_params(handle, sub_chan_enc, alarm_video_enc_params);
        break;
    case SDVR_VIDEO_ENC_NONE:
        // This is a HMO channel, there are no encoding parameters
        err = SDVR_ERR_NONE;
        break;

    default:
        err = DVR_STATUS_WRONG_CODEC_FORMAT;
        break;
    }

    sdbg_msg_output_err(sdvr_get_alarm_video_encoder_params_func, err);
    return err;
}

/****************************************************************************
  This function is used to set the parameters of an audio encoder channel. 
  There are no corresponding functions for setting the parameters
  for an audio decoding channel. This is because the decoding 
  channel takes its parameters from the encoded stream. 

  The parameters of an audio encoding channel can only be changed when
  encoding is stopped.
  
  @NOTE: This feature is not implemented in this release. Only G.711
  audio encoding is supported for this release.@

  Parameters:

    "handle" - An encoding channel handle.

    "audio_enc_params" - The parameters for the audio encoder channel. 

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_set_audio_encoder_channel_params(sdvr_chan_handle_t handle, 
                                                 sdvr_audio_enc_chan_params_t *audio_enc_params)
{
    return SDVR_ERR_NONE;
}

/****************************************************************************
  This function is used to get the parameters of an audio encoder channel. 

  You must set the parameters before you can get them. 

  Parameters:

    "handle" -An encoding channel handle.

    "audio_enc_params" - A pointer to a variable that will be filled with
    encoder channel parameters when this function returns. 

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_get_audio_encoder_channel_params(sdvr_chan_handle_t handle,
                                                 sdvr_audio_enc_chan_params_t *audio_enc_params)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);

    if (audio_enc_params == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else if (ssdk_validate_chan_handle(handle, -1) !=  SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;

    if (err !=  SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_get_audio_encoder_channel_params_func, err);
        return err;
    }

    // todo: For now the only supported Audio encoded is G711 and it is done
    //       at the host side.
    audio_enc_params->audio_enc_type = SDVR_AUDIO_ENC_G711;

    return SDVR_ERR_NONE;
}


/****************************************************************************
  This function returns the size of ROI map base on 16x16 macro block and
  the currently selected video standard on the given DVR board number.


****************************************************************************/
static ssdvr_size_t _get_roi_map_size(sdvr_video_std_e video_std)
{
    ssdvr_size_t cur_video_std_size;

    cur_video_std_size = get_frame_size(video_std, 
        SDVR_VIDEO_RES_DECIMATION_EQUAL);

    cur_video_std_size.width /= 16;
    cur_video_std_size.height /= 16;

    return cur_video_std_size;
}

/****************************************************************************
  This function creates a SCT channel to send large data such as
  the ROIs map from the DVR SDK to the firmware if one is not created yet.

  NOTE: This PCI channel handle is added to the boards data structure for
  the given board index. It 
****************************************************************************/
sdvr_err_e _do_create_sct_send_chan(sx_uint32 board_index)
{
    sdvr_err_e err;

    dvr_ioctl_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    ssdvr_size_t map_size;
    sct_channel_t pci_send_chan_handle = NULL;

    /*
       connect the sct channel to send the map, if it is not connected
       yet.
    */
    if (_ssdvr_boards_info.boards[board_index].pci_send_data_handle)
    {
        return SDVR_ERR_NONE;
    }

    map_size = _get_roi_map_size(_ssdvr_boards_info.boards[board_index].video_std);

    /*
       Inform the firmware to open the ROI sct channel to get ready
       to recieve a ROIs map.
    */
    memset(&cmd_strct,0,cmd_strct_size);
    cmd_strct.ioctl_code = DVR_IOCTL_CODE_OPEN_CHAN;
    cmd_strct.u1.chan.port = SCT_PORT_SEND_DATA;
    err = ssdk_message_board_send( board_index, DVR_SET_IOCTL, &cmd_strct, cmd_strct_size);

    /*
        we must try to open a SCT channel to send region map.
    */
    err = sct_channel_connect(_ssdvr_boards_info.boards[board_index].handle,
                              SCT_PORT_SEND_DATA,
                              3,                                // number of send buffers to allocate
                              map_size.height * map_size.width, // the send buffer size
                              &pci_send_chan_handle);

    if (err == SDVR_ERR_NONE)
    {
        err = ssdk_message_board_recv(board_index, DVR_REP_IOCTL, &cmd_strct, 0, 0);
        if (err == SDVR_ERR_NONE)
        {
            err = cmd_strct.status;
            if (err ==  SDVR_ERR_NONE)
            {
                _ssdvr_boards_info.boards[board_index].pci_send_data_buf_size = 
                    map_size.height * map_size.width;
                _ssdvr_boards_info.boards[board_index].pci_send_data_handle = pci_send_chan_handle;
            }
            else
            {
                sct_channel_close(pci_send_chan_handle);
            }

        }
    }

    return err;
}

sdvr_err_e _send_regions_map(sdvr_chan_handle_t handle,
                             sdvr_regions_type_e region_type,
                             sx_uint8 *regions_map,
                             sx_uint32 map_size)
{
    sx_uint32 board_index = sdvr_get_board_index(handle);
    dvr_regions_map_t cmd_strct;
    sdvr_err_e err;
    int cmd_strct_size = sizeof(cmd_strct);
    void *send_buffer = NULL;
    int retry;
    sct_channel_t pci_send_data_handle;


    pci_send_data_handle = _ssdvr_boards_info.boards[board_index].pci_send_data_handle;
    /*
       For now we transmit everything in one shot.
    */
    for (retry = 0; retry < 20; retry++)
    {
        send_buffer = sct_buffer_alloc(pci_send_data_handle);
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

        memcpy(send_buffer, regions_map, map_size);
        err = sct_buffer_send(pci_send_data_handle, 
                              send_buffer, map_size);
    }
    else
    {
        err = SDVR_DRV_ERR_NO_SEND_BUFFERS;
    }

    return err;
}

static sdvr_err_e _do_send_regions_map(sdvr_chan_handle_t handle,
                                sdvr_regions_type_e region_type,
                                sx_uint8 overlay_num,
                                sx_uint8 *regions_map)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sdvr_err_e send_err = SDVR_ERR_NONE;
    dvr_regions_map_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 board_index;

    board_index = sdvr_get_board_index(handle);

    err = _do_create_sct_send_chan(board_index);
    if (err == SDVR_ERR_NONE)
    {
        memset(&cmd_strct,0,cmd_strct_size);
        cmd_strct.job_type = sdvr_get_chan_type(handle);
        cmd_strct.job_id = sdvr_get_chan_num(handle);
        cmd_strct.opcode = DVR_REGION_OPCODE_START;
        cmd_strct.region_type = region_type;

        cmd_strct.u1.map.map_size = _ssdvr_boards_info.boards[board_index].pci_send_data_buf_size;
        cmd_strct.u1.map.overlay_num = overlay_num; 

        // Tell the firmware to prepare to recieve new ROIs map for the specified region type.
        err = ssdk_message_chan_send(handle, DVR_SET_REGIONS_MAP, &cmd_strct, sizeof(cmd_strct));
        err = ssdk_message_chan_recv(handle, DVR_REP_REGIONS_MAP,&cmd_strct, 0 ,0);

        if (err == SDVR_ERR_NONE && cmd_strct.status == SDVR_ERR_NONE)
        {
            send_err = _send_regions_map( handle, region_type, regions_map,
                _ssdvr_boards_info.boards[board_index].pci_send_data_buf_size);

            // Tell the firmware you are done sending the ROI
            cmd_strct.opcode = DVR_FONT_OPCODE_FINISH;
            err = ssdk_message_chan_send(handle, DVR_SET_REGIONS_MAP, &cmd_strct, sizeof(cmd_strct));
            err = ssdk_message_chan_recv(handle, DVR_REP_REGIONS_MAP,&cmd_strct, 0 ,0);
        }
        /*
           If there was no error in stopping/starting the ROI map, set the return
           code to be the send map error.
        */
        if (err == SDVR_ERR_NONE)
        {
            err = (cmd_strct.status != SDVR_ERR_NONE) ? cmd_strct.status : send_err;
        }
    }
    cmd_strct.status = err;
    sdbg_msg_output(sdvr_set_regions_map_func, board_index, (void*)&cmd_strct);

    return err;
}
/****************************************************************************
  This function is used to map out the Regions Of Interest (ROI) within a 
  full decimation of the current video standard size frame to be used for different 
  alarms detections or privacy blocking.

  The map of ROI is made of (width x lines) Macro Blocks (MB) based on
  full resolution of the current video standard,
  where, the size of each MB is 16x16 pixels, the value of
  width is calculated by dividing the width of the current video standard by 
  16 and the value of lines is calculated by dividing the height of the 
  current video standard by 16. Following table shows the ROI map dimensions
  for mostly used video standards.
  {

       Video Standard |  ROI Map width  | ROI Map lines
       ---------------+-----------------+---------------
          NTSC D1     |        45       |       30
       ---------------+-----------------+---------------
          PAL D1      |        45       |       36
       ---------------+-----------------+---------------
          NTSC 4CIF   |        44       |       30
       ---------------+-----------------+---------------
          PAL 4CIF    |        44       |       36

  }

  There are four (4) layers of ROI for motion detections but only one 
  layer for other alarms detection and privacy blocking. Each bit within
  the map of ROI elements corresponds to a ROI layer. Where the right-most
  side bit is layer one, second right-most side bit is layer two,
  and so on. To include an MB for alarm detection or privacy blocking, set
  the corresponding bit of the map of ROI array element to one (1).

  D1 PAL ROI map size = (SDVR_REGION_MAP_D1_WIDTH*SDVR_REGION_MAP_LINE_PAL)

  D1 NTSC ROI map size = (SDVR_REGION_MAP_D1_WIDTH*SDVR_REGION_MAP_LINE_NTSC)

  4-CIF PAL ROI map size = (SDVR_REGION_MAP_4CIF_WIDTH*SDVR_REGION_MAP_LINE_PAL)

  4-CIF NTSC ROI map size = (SDVR_REGION_MAP_4CIF_WIDTH*SDVR_REGION_MAP_LINE_NTSC)


  Note: For NTSC video standard the last six lines are missing, whereas for
  4-CIF the last column is missing.
  { 
MAP of ROI
Layer:  87654321   87654321   87654321   ...   87654321   87654321      
bit map     0          1          2      ...      43        44
      +----------+----------+----------+-----+----------+----------+
 +-> 0| 00000000 | 00000000 | 00000000 | ... | 00000101 | 00000000 |
 |    +----------+----------+----------+-----+----------+----------+
 |-> 1| 00000000 | 00000000 | 00000000 | ... | 00000000 | 00000000 |
 |    +----------+----------+----------+-----+----------+----------+
 |-> 2| 00000000 | 00000000 | 00000000 | ... | 00000000 | 00000000 |
 |    +----------+----------+----------+-----+----------+----------+
 |         .          .          .       .        .          .
lines      .          .          .       .        .          .
 |         .          .          .       .        .          .
 |    +----------+----------+----------+-----+----------+----------+
 |->34| 00000000 | 00000000 | 00000000 | ... | 00000000 | 00000000 |
 |    +----------+----------+----------+-----+----------+----------+
 +->35| 00000000 | 00000000 | 00000000 | ... | 00000000 | 00000000 |
      +----------+----------+----------+-----+----------+----------+

      |<------------------------ width --------------------------->|
  }
  Parameters:

    "handle" -An encoding channel handle.

    "region_type" - The type of region of interest.

    "regions_map" - A pointer to an array of (width x lines) 8-bit elements
    that provides a map of regions of interests for the given region
    type. The area of the map is based on Macro Blocks (MB) of size
    16x16 pixels within a 1:1 video decimation frame based on the current
    video standard. Each bit within
    an element of the array corresponds to the layer of regions of
    interest for the region type. 

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Example:
  {
    // The following code adds MB[0,43] ROI to the layer 1 and 3 of the
    // motion detection map

    sdvr_chan_handle_t handle;
    sx_uint8 *regions_map;
    int line, column;
    int width_size, line_size;

    // Assuming the current video standard is D1 NTSC:
    width_size = SDVR_REGION_MAP_D1_WIDTH;
    line_size = SDVR_REGION_MAP_LINE_NTSC;

    regions_map = (sx_uint8 *)calloc(width_size*line_size, sizeof(sx_uint8));

    column = 43;
    row = 0;
    regions_map[row * width_size + column] = 0x5;
    sdvr_set_regions_map(handle, SDVR_REGION_MOTION, regions_map);
  }
****************************************************************************/
sdvr_err_e sdvr_set_regions_map(sdvr_chan_handle_t handle,
                                sdvr_regions_type_e region_type,
                                sx_uint8 *regions_map)
{
    sdvr_err_e err = SDVR_ERR_NONE;


    if (regions_map == NULL || !_is_valid_region_type(region_type))
        err = SDVR_ERR_INVALID_ARG;
    else if (ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER) !=  SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;

    if (err !=  SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_set_regions_map_func, err);
        return err;
    }

    /*
       send the region map.
       Indicates all the ROI overlay are being set.
    */
    err = _do_send_regions_map(handle, region_type, 0, regions_map);

    return err;
}

/****************************************************************************
  This is a convenient function for specifying the Regions Of Interest 
  (ROI) based on Macro Blocks (MB) rectangles within a full decimation 
  of the current video standard size frame to be 
  used for different alarms detection or privacy blocking.

  By calling this function with parameter regions_list set to NULL or
  specifing a value of zero for num_of_regions, the
  regions within the given overlay will be reset.

  Parameters:

    "handle" -An encoding channel handle.

    "region_type" - The type of region of interest.

    "overlay_num" - The overlay number to add the regions.
    For region_type of SDVR_REGION_MOTION overlay_num is in the
    range 1-4. For all other region types, it is ignored.

    "regions_list" - A pointer to an array of macro block
    rectangles. NULL means to reset the regions for the 
    given overlay_num.

    "num_of_regions" - The number of elements in regions_list array.
    A value of zero resets the regions for the given overlay_num.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:

    By default the entire first overlay is enabled. In case you are interested to
    set an overlay other than the first overlay, call this function for 
    first overlay with regions_list parameter set to NULL prior setting the new
    overlay.
****************************************************************************/
sdvr_err_e sdvr_set_regions(sdvr_chan_handle_t handle,
                                sdvr_regions_type_e region_type,
                                sx_uint8 overlay_num,
                                sdvr_mb_region_t *regions_list,
                                sx_uint8 num_of_regions)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint32 row, col;
    sx_uint32 board_index;
    ssdvr_size_t map_size;
    sx_uint8 *roi_map;
    sx_uint8 *map_element;
    sx_uint32 region_num;
    sdvr_mb_region_t *cur_region;

    if (!_is_valid_region_type(region_type) || overlay_num > 4)
        err = SDVR_ERR_INVALID_ARG;
    else if (ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER) !=  SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;

    if (err !=  SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_set_regions_func, err);
        return err;
    }

    /*
       overlay_num is ignored for not motion detection regions.
    */
    if (region_type != SDVR_REGION_MOTION)
        overlay_num = 1;

    board_index = sdvr_get_board_index(handle);
    /************************************************
      Calculate the size of Regions of Interest map
      and allocate memory for it.
    ************************************************/
    map_size = _get_roi_map_size(_ssdvr_boards_info.boards[board_index].video_std);
    roi_map = (sx_uint8 *)calloc(map_size.height * map_size.width,sizeof(sx_uint8));
    if (roi_map == NULL)
    {
        err = SDVR_ERR_OUT_OF_MEMORY;
        sdbg_msg_output_err(sdvr_set_regions_func, err);
        return err;
    }


    /************************************************
        If the regions_list is NULL the caller wants
        to clear the given overlay region.
    *************************************************/
    if (regions_list != NULL && num_of_regions > 0)
    {
        /************************************************
           Transform the regions of interest rectangles to
           the ROI map by selecting the corresponding Macro Block.
        ************************************************/
        for (region_num = 0; region_num < num_of_regions; region_num++)
        {
            /****************************************************************
            Go to the region element within the given region list and
            validate the region.
            *****************************************************************/
            cur_region = regions_list + region_num;

            if (cur_region->upper_left_x >  cur_region->lower_right_x ||
                cur_region->upper_left_y > cur_region->lower_right_y ||
                cur_region->lower_right_y > map_size.width)
            {
                free (roi_map);
                err = SDVR_ERR_INVALID_REGION;
                sdbg_msg_output_err(sdvr_set_regions_func, err);
                return err;
            }

            /****************************************************************
             write the MB rectangle to the ROI map.
            *****************************************************************/
            for (row = cur_region->upper_left_x; row <= cur_region->lower_right_x; row++)
            {
                for (col = cur_region->upper_left_y; col <= cur_region->lower_right_y; col++)
                {
                    map_element = roi_map + (row * map_size.width + col);
                    *map_element = 1;
                    *map_element <<= (overlay_num - 1);

                }
            }
        } // for (region_num = 0; region_num < num_of_regions; region_num++)
    } // if (regions_list == NULL)

    /************************************************
       send the region map for the given overlay
    ************************************************/
    err = _do_send_regions_map(handle, region_type, overlay_num, roi_map);

    free (roi_map);
    return err;
}

/****************************************************************************
  This function allows you to determine if an alarm occurred on any of
  the given Macro Block (MB) rectangular regions within a given motion 
  values frame buffer.

  To calculate an alarm condition, the average motion values within each
  region of interest in the given regions list is used against the 
  given threshold.

  Parameters:

    "motion_values" - The motion values frame received from the DVR
    board. This the payload in the frame buffer returned by 
    sdvr_get_av_buffer() with the
    frame type of SDVR_FRAME_MOTION_VALUES.

    "motion_values_vstd" - The video standard for which the 
    motion_values buffer was based.

    "regions_list" - A pointer to an array of macro block
    rectangles. Upon return, the alarm_flag field of each entry
    is set according to the specified threshold.

    "num_of_regions" - The number of elements in regions_list array.

    "threshold" - The value to be used to determine the
    alarm condition for a region in the regions_list. The motion_flag
    for each region within the regions_list is set if the motion
    value within the given region exceeds this threshold.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:

    You may choose to perform different motion value analysis  by directly
    looking at the motion value of each field in the motion_values buffer.
    Each element of motion_values buffer contains the motion value for
    the corresponding MB, where the first index is the first
    16x16 MB within the FULL size video frame. See sdvr_set_regions_map() for
    more information regarding the buffer format. The exception is that each
    element in motion_values buffer contains the motion value from 0 - 255 instead
    of the bit indicating to motion detection.
****************************************************************************/
sdvr_err_e sdvr_motion_value_analyzer(
                                sx_uint8 *motion_values,
                                sdvr_video_std_e motion_values_vstd,
                                sdvr_mb_region_t *regions_list,
                                sx_uint8 num_of_regions,
                                sx_uint32 threshold)
{
    sx_uint32 row, col;
    sx_uint32 motion_value;
    sx_uint8 cur_region;
    sdvr_mb_region_t *regionPtr;
    ssdvr_size_t map_size;
    sx_uint32 index;
    sx_uint32 region_area;

    map_size = _get_roi_map_size(motion_values_vstd);

    if (motion_values == NULL || regions_list == NULL)
        return SDVR_ERR_INVALID_ARG;

    for (cur_region = 0; cur_region < num_of_regions; cur_region++)
    {
        /****************************************************************
        Go to the region element within the given region list and
        validate the region.
        *****************************************************************/
        regionPtr = regions_list + cur_region;

        if (regionPtr->upper_left_x  > regionPtr->lower_right_x ||
            regionPtr->upper_left_y  > regionPtr->lower_right_y ||
            regionPtr->lower_right_y > map_size.width ||
            regionPtr->lower_right_x > map_size.height)
        {
            return SDVR_ERR_INVALID_REGION;
        }

        /****************************************************************
         Calculate the motion value for within the given Macro Block
         rectangle on the current region.
        *****************************************************************/
        motion_value = 0;
        for (row = regionPtr->upper_left_x; row <= regionPtr->lower_right_x; row++)
        {
            for (col = regionPtr->upper_left_y; col <= regionPtr->lower_right_y; col++)
            {
                index = row * map_size.width + col;
                motion_value += motion_values[index];
            }
        }

        /****************************************************************
         Set the alarm flag for the current region if the average motion value
         within the region exceeds the given threshold.
        *****************************************************************/
        region_area = (regionPtr->lower_right_y - regionPtr->upper_left_y) *
            (regionPtr->lower_right_x - regionPtr->upper_left_x);
        regionPtr->alarm_flag = ((motion_value / region_area) >= threshold);
    }
    return SDVR_ERR_NONE;
}

/****************************************************************************
  This function sets the frequency of sending the motion value frames from the
  DVR board to the DVR Application per encoder channel. Once "every_n_frames"
  is set to a non-zero value, motion value frames are sent at the requested
  intervals. For example if "every_n_frames" is set to 5, then a motion value
  frame will be sent every 5 video frames. However, note that as long as motion
  detection is enabled, a motion value frame will be sent every time that the
  motion threshold is exceeded. This is true even if "every_n_frames" is set to
  zero.

  Parameters:

    "handle" -An encoding channel handle.

    "every_n_frames" - The frequency of alarm value frame to send. Setting this
    field to one (1) means send with every frame, two (2) means send every other
    frame, and so on.  Default is zero (0). The valid range is 0-255. If this
    is set to zero (0) while motion detection is enabled, then a motion value
    frame will still be sent every time that the motion threshold is exceeded.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:

    Call this function with frequency of zero (0) after disabling the
    motion detection if you are not interested to receive motion value frames
    while the motion detection is disabled.
****************************************************************************/
sdvr_err_e sdvr_set_motion_value_frequency(sdvr_chan_handle_t handle, sx_uint8 every_n_frames)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    dvr_ioctl_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);

    if (ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER) !=  SDVR_ERR_NONE)
    {
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
        sdbg_msg_output_err(sdvr_set_motion_value_frequency_func, err);
        return err;
    }

    memset(&cmd_strct,0,cmd_strct_size);
    cmd_strct.ioctl_code = DVR_IOCTL_CODE_MOTION_FRAME_FREQ;
    cmd_strct.job_id = CH_GET_CHAN_NUM(handle);
    cmd_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cmd_strct.u1.motion.frequency = every_n_frames;
    err = ssdk_message_board_send( board_index, DVR_SET_IOCTL, &cmd_strct, cmd_strct_size);
    err = ssdk_message_board_recv(board_index, DVR_REP_IOCTL, &cmd_strct, 0, 0);
    if (err != SDVR_ERR_NONE)

    {
        cmd_strct.status = err;
    }
    sdbg_msg_output(sdvr_set_motion_value_frequency_func, board_index, (void*)&cmd_strct);
    return cmd_strct.status;
}


/****************************************************************************
  This function returns the motion value for the specified motion region
  overlay number. 
  
  If an alarm callback function is registered by the DVR Application,
  Once motion alarm is detected by the DVR firmware, the alarm callback 
  is called with a "data" parameter. Call this function to get each
  motion value for each region overlay.

  Parameters:

    "data" - The data parameter returned by alarm callback function.

    "overlay_num" - The motion region overlay number from which to get its
    motion value. The valid range in the field is 1-4.

    "motion_value" - A pointer to hold the motion value for the requested
    motion region overlay.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

****************************************************************************/
sdvr_err_e sdvr_get_alarm_motion_value(sx_uint32 data, sx_uint8 overlay_num, sx_uint8 *motion_value)
{
    sdvr_err_e err = SDVR_ERR_NONE;

    if (overlay_num > 4 || overlay_num < 1 || motion_value == NULL)
    {
        err =  SDVR_ERR_INVALID_ARG;
    }
    else
    {
        *motion_value = ((data >> ((overlay_num - 1) * 8)) & 0x000000FF);
    }
    return err;
}
