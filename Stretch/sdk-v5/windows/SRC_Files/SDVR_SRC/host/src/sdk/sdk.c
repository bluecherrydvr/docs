extern int yuv_count;

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
#define STRETCH_SDK_C
#include "sdvr_sdk_common.h"
#include <time.h>
#include "../codecs/g711/g711.h"
#include "../codecs/h264/venc264_authenticate.h"
#ifndef WIN32
#include <sys/time.h>
#endif

static sx_bool _IsBigEndian()
{
   short word = 0x4321;
   if((*(char *)& word) != 0x21 )
     return true;
   else 
     return false;
}

static void _adjust_bug_hdr_little_to_big_endian(void *buf_hdr)
{
    sdvr_av_buffer_t *av_buf_hdr = (sdvr_av_buffer_t*)buf_hdr;

    if (_is_byte_swap)
    {
        av_buf_hdr->hdr_version = _int16_little_endian_TO_big_endian(av_buf_hdr->hdr_version);
        av_buf_hdr->hdr_size = _int16_little_endian_TO_big_endian(av_buf_hdr->hdr_size);

        av_buf_hdr->payload_size = _int32_little_endian_TO_big_endian(av_buf_hdr->payload_size);

        av_buf_hdr->timestamp = _int32_little_endian_TO_big_endian(av_buf_hdr->timestamp);
        av_buf_hdr->timestamp_high = _int32_little_endian_TO_big_endian(av_buf_hdr->timestamp_high);

        av_buf_hdr->active_width = _int16_little_endian_TO_big_endian(av_buf_hdr->active_width);
        av_buf_hdr->padded_width = _int16_little_endian_TO_big_endian(av_buf_hdr->padded_width);
        av_buf_hdr->active_height = _int16_little_endian_TO_big_endian(av_buf_hdr->active_height);
        av_buf_hdr->padded_height = _int16_little_endian_TO_big_endian(av_buf_hdr->padded_height);

        av_buf_hdr->seq_number = _int32_little_endian_TO_big_endian(av_buf_hdr->seq_number);
        av_buf_hdr->frame_number = _int32_little_endian_TO_big_endian(av_buf_hdr->frame_number);
        av_buf_hdr->frame_drop_count = _int32_little_endian_TO_big_endian(av_buf_hdr->frame_drop_count);
    }
}

/****************************************************************************
  This function initializes the SDK and the driver, allocates system resources
  required by them, and discovers all SDVR cards in the system. NOTE: the
  cards are not initialized by this function. 

  This is the first call that you must make in your application before
  you can use any of the other API functions with the exception of get version
  or set callback calls. Also, this function should be called only once per session.

  Parameters: 

    None.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise see the error code list.
****************************************************************************/
sdvr_err_e sdvr_sdk_init()
{
    int a = sizeof(dvr_ver_info_t);
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint32 board_index;
    /******************************************************************
      The following variables are only for debugging purposes
    *******************************************************************/
    sdbg_totalFrame_count_rcvd = 0;
    sdbg_totalFrame_count_freed = 0;


    memset(&_ssdvr_boards_info,0,sizeof(_ssdvr_boards_info));
    sdbg_msg_output(sdvr_sdk_init_func, 0, (void *)"Initializing the SDK...");

    ssdvr_in_callback = 0;

    /**********************************************************************
     Initialize the Device driver and get the number of SDVR PCI board
     installed on the PC. NOTE: we can not continue if there was no PCI
     boards intalled.
    ***********************************************************************/
    _ssdvr_boards_info.board_num = sct_init();
    if (_ssdvr_boards_info.board_num == 0)
    {
        err = SDVR_ERR_NO_DVR_BOARD;
        sdbg_msg_output_err(sdvr_sdk_init_func,err);
        return err;
    }

    /***********************************************
        Initialize the critical section handling
    ***********************************************/
#ifdef WIN32
    InitializeCriticalSection( &_ssdvr_boards_info.cs_board_info );
    InitializeCriticalSection( &_ssdvr_boards_info.cs_decode_buf_count );
#else
    pthread_mutex_init(&_ssdvr_boards_info.cs_board_info,NULL);
    pthread_mutex_init(&_ssdvr_boards_info.cs_decode_buf_count,NULL);
#endif
    /*************************************************************************
            We are resetting the board just incase if the firmware 
            was hung since the last session.
    *************************************************************************/
    for (board_index = 0; board_index < _ssdvr_boards_info.board_num; board_index++)
    {
        _ssdvr_boards_info.boards[board_index].is_reset_required = true;
    }

    /**************************************************************
      If the SDK is running on a Host PC with Big Endian we need
      to perform byte swaps as we exchanged data structure from
      host SDK to/from DVR Firmware. 
      NOTE: The DVR Frimware is alway Little Endian format.
    ***************************************************************/
    _is_byte_swap = (_IsBigEndian() == true);

    sdbg_msg_output_err(sdvr_sdk_init_func,err);

    return err;
}

/****************************************************************************
  This function closes the SDK and the driver, and frees up the system
  resources used by them. You must call this function prior to exiting
  your DVR Application for a clean shutdown of your system. 
  No other API function calls, except sdvr_sdk_init(), are allowed after 
  this function is called. 

  Parameters: 

    None.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_sdk_close()
{
    sx_uint32 board_index;
    sdvr_err_e err = SDVR_ERR_NONE;
    sdvr_err_e tmp_err = SDVR_ERR_NONE;

    sdbg_msg_output(sdvr_sdk_close_func, 0, (void *)"closing the SDK ...");

    if (_ssdvr_boards_info.temp_venc_buf) 
        free(_ssdvr_boards_info.temp_venc_buf);
    _ssdvr_boards_info.temp_venc_buf = NULL;

    // make sure we are disconnecting from any board that is still connected
    for (board_index = 0; board_index < MAX_PCI_BOARDS; board_index++) 
    {
        if (_ssdvr_boards_info.boards[board_index].handle)
        {
            tmp_err = sdvr_board_disconnect(board_index);
            /* Make sure not to re-set the last error*/
            if (err == SDVR_ERR_NONE)
                err = tmp_err;
        }
    }

    sdbg_msg_output_err(sdvr_sdk_close_func,err);

    sdbg_cleanup();
    memset(&_ssdvr_boards_info,0,sizeof(_ssdvr_boards_info));

#ifdef WIN32
    DeleteCriticalSection(&_ssdvr_boards_info.cs_board_info);
    DeleteCriticalSection(&_ssdvr_boards_info.cs_decode_buf_count);
#else
    pthread_mutex_destroy(&_ssdvr_boards_info.cs_board_info);
    pthread_mutex_destroy(&_ssdvr_boards_info.cs_decode_buf_count);
#endif

    return err;
}

/****************************************************************************
  This function returns the number of SDVR boards in the system. Only boards 
  that are functioning properly are discovered and reported by this
  function. 

  Parameters: 

    None

  Returns: 

    The number of SDVR boards discovered.
****************************************************************************/
sx_uint32 sdvr_get_board_count()
{
    sdbg_msg_output(sdvr_get_board_count_func, 0, &_ssdvr_boards_info.board_num);
    return _ssdvr_boards_info.board_num;
}

/****************************************************************************
  This function returns the board attributes of each board. Use this function
  to discover which PCI slot the board is connected to and its 
  type, as well as all the video standards supported  by the firmware.

  Parameters: 
 
    "board_index" - The number of the board whose attributes you want. 
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "board_attrib" - A pointer to a variable that will hold the attributes
    when this function returns.

  Returns: 

    SDVR_ERR_NONE - On success. Otherwise see the error code list.

  Remark:

    Call this function or sdvr_get_supported_vstd() before calling 
    sdvr_board_connect() to select one of the supported video standards 
    by the firmware.

****************************************************************************/
sdvr_err_e sdvr_get_board_attributes(sx_uint32 board_index, 
                                     sdvr_board_attrib_t *board_attrib)
{
    sdvr_err_e err;
    sx_uint32  video_stds;
    dvr_ver_info_t cmd_strct;
    sct_board_detail_t board_detail;
    int temp_connect = 0;

    if (board_index >= _ssdvr_boards_info.board_num)
    {
        err = SDVR_ERR_INVALID_BOARD;
        sdbg_msg_output_err(sdvr_get_board_attributes_func,err);
        return err;
    }

    if (board_attrib == NULL)
    {
        err = SDVR_ERR_INVALID_ARG;
        sdbg_msg_output_err(sdvr_get_board_attributes_func,err);
        return err;
    }

    memset(board_attrib,0, sizeof(sdvr_board_attrib_t));

    if (_ssdvr_boards_info.boards[board_index].is_reset_required &&
        ((err = sdvr_board_reset(board_index)) != SDVR_ERR_NONE))
    {
        sdbg_msg_output_err(sdvr_get_board_attributes_func,err);
        return err;
    }

    /*****************************************************************
     It is possible when this routine is called, we have not connected
     to the board yet. In this case, first connect to the board temporarily
     before getting the video standard, board type and, chip version 
     then disconnect.
    *****************************************************************/
    if (_ssdvr_boards_info.boards[board_index].handle == NULL) {
        temp_connect = 1;
        if ((err = sct_open_board(board_index,
            &_ssdvr_boards_info.boards[board_index].handle)) != SDVR_ERR_NONE) 
        {
            sdbg_msg_output_err(sdvr_get_board_attributes_func,err);
            return err;
        }
    }

    err = sct_get_board_detail(_ssdvr_boards_info.boards[board_index].handle, &board_detail);
    if (err == SDVR_ERR_NONE)
    {
        board_attrib->pci_slot_num = board_detail.board_slot_number;
        board_attrib->board_type = board_detail.subsystem_vendor;
        board_attrib->board_type <<= 16;
        board_attrib->board_type |= (0x0000FFFF & board_detail.device_id);

        memset(&cmd_strct,0, sizeof(cmd_strct));

        /********************************************************
           NOTE: sdvr_get_supported_vstd() should be the very first function
           call that we make to the board. Otherwise, tring to connect
           to a board which has no firmware loaded caused assertion.
        *********************************************************/
        err = sdvr_get_supported_vstd(board_index, &video_stds);

        if (err == SDVR_ERR_NONE) {
            board_attrib->supported_video_stds = video_stds;

            ssdk_message_board_send(board_index, DVR_GET_VER_INFO, &cmd_strct, sizeof(cmd_strct));
            ssdk_message_board_recv(board_index, DVR_REP_VER_INFO, &cmd_strct, 0, 0);

             board_attrib->board_sub_rev = 0;

            if ((err = cmd_strct.status) == SDVR_ERR_NONE && 
                ssdk_is_min_version(cmd_strct.version_major,cmd_strct.version_minor,cmd_strct.version_bug,cmd_strct.version_build,
                        3,2,0,1))
            {
                board_attrib->board_revision = cmd_strct.board_revision; 
                board_attrib->chip_revision = cmd_strct.chip_revision;
            }
            if ((err = cmd_strct.status) == SDVR_ERR_NONE && 
                ssdk_is_min_version(cmd_strct.version_major,cmd_strct.version_minor,cmd_strct.version_bug,cmd_strct.version_build,
                        4,1,0,0))
            {
                board_attrib->board_sub_rev = cmd_strct.board_sub_rev; 

                /*
                   BACKWARD COMPATIBLITY with:
                     The DVR Boards with old EPROM but running 4.1.0.0 or 
                     higher firmware.
                */
                if (cmd_strct.board_revision == 0 && 
                    board_attrib->board_sub_rev < 'A')
                {
                    board_attrib->board_revision = board_attrib->board_sub_rev;
                    board_attrib->board_sub_rev = 0;
                }
            }

            sdbg_msg_output(sdvr_get_board_attributes_func, board_index, board_attrib);
        }
    }
    // close the board if we opened it temporarily
    if (temp_connect)
    {
        sct_close_board(_ssdvr_boards_info.boards[board_index].handle);
        _ssdvr_boards_info.boards[board_index].handle = NULL;
    }

    sdbg_msg_output_err(sdvr_get_board_attributes_func,err);
    return err;
}

/****************************************************************************
  This function returns the board PCI attributes of each board. Use this function
  to discover which PCI slot the board is connected to, its 
  type, vendor and device ID, and serial number.

  Parameters: 
 
    "board_index" - The board number for which to get PCI information.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "pci_attrib" - A pointer to a variable that will hold the PCI information
    when this function returns successfully.

  Returns: 

    SDVR_ERR_NONE - On success. Otherwise see the error code list.

  Remarks:

    This function can be called before the call to sdvr_board_connect() or
    before loading a firmware into the DVR board.
****************************************************************************/
sdvr_err_e sdvr_get_pci_attrib(sx_uint32 board_index, 
                                     sdvr_pci_attrib_t *pci_attrib)
{
    sdvr_err_e err;
    sct_board_detail_t board_detail;
    int temp_connect = 0;

    if (board_index >= _ssdvr_boards_info.board_num)
    {
        err = SDVR_ERR_INVALID_BOARD;
        sdbg_msg_output_err(sdvr_get_pci_attrib_func,err);
        return err;
    }

    if (pci_attrib == NULL)
    {
        err = SDVR_ERR_INVALID_ARG;
        sdbg_msg_output_err(sdvr_get_pci_attrib_func,err);
        return err;
    }

    memset(pci_attrib,0, sizeof(sdvr_pci_attrib_t));

    /*****************************************************************
     It is possible when this routine is called, we have not connected
     to the board yet. In this case, first connect to the board temporarily
     before getting the video standard, board type and, chip version 
     then disconnect.
    *****************************************************************/
    if (_ssdvr_boards_info.boards[board_index].handle == NULL) {
        temp_connect = 1;
        if ((err = sct_open_board(board_index,
            &_ssdvr_boards_info.boards[board_index].handle)) != SDVR_ERR_NONE) 
        {
            sdbg_msg_output_err(sdvr_get_pci_attrib_func,err);
            return err;
        }
    }

    err = sct_get_board_detail(_ssdvr_boards_info.boards[board_index].handle, &board_detail);
    if (err == SDVR_ERR_NONE)
    {
        pci_attrib->pci_slot_num = board_detail.board_slot_number;
        pci_attrib->board_type = board_detail.subsystem_vendor;
        pci_attrib->board_type <<= 16;
        pci_attrib->board_type |= (0x0000FFFF & board_detail.device_id);

        pci_attrib->vendor_id = board_detail.vendor_id;
        pci_attrib->device_id = board_detail.device_id; 
        pci_attrib->subsystem_vendor = board_detail.subsystem_vendor;
        pci_attrib->subsystem_id = board_detail.subsystem_id;
        strcpy(&pci_attrib->serial_number[0],&board_detail.serial_string[0]);

        sdbg_msg_output(sdvr_get_pci_attrib_func, board_index, pci_attrib);
    }
    // close the board if we opened it temporarily
    if (temp_connect)
    {
        sct_close_board(_ssdvr_boards_info.boards[board_index].handle);
        _ssdvr_boards_info.boards[board_index].handle = NULL;
    }

    sdbg_msg_output_err(sdvr_get_pci_attrib_func,err);
    return err;
}

/****************************************************************************
  This function resets the firmware on the given board. We recommend that you not
  connect to any board prior calling this function.  After calling 
  sdvr_board_reset(), you must reload the firmware if it is not already flashed
  on the board. Additionally, your system also needs to be reconfigured after 
  board reset. 

  The usage of this function is heavily discouraged because it can put the
  SDK in an indeterminate stage. In general, if the firmware is not already flashed
  on the DVR board, this function should not be called.

  Parameters:

    "board_index" - The number of the board that you want to reset.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
*****************************************************************************/
sdvr_err_e sdvr_board_reset(sx_uint32 board_index)
{
    const char *name;
    sct_board_t handle;
    sdvr_err_e err;
    char msg[256];

    name = sct_get_board_name(board_index);
    if (name == NULL) {
        err = SDVR_ERR_INVALID_BOARD;
        sprintf(msg,"sdvr_board_reset(): Invalid board index [%d]",board_index);
        sdbg_msg_output(sdvr_dbg_err_msg,board_index,msg);
        return err;
    }

    /***********************************************************
      Reset the firmware by openning the board, reseting it and
      closing it again.
    ***********************************************************/
    sdbg_msg_output(sdvr_dbg_msg,board_index,"Calling sct_open_board...");
    if ((err = sct_open_board(board_index,&handle)) == SDVR_ERR_NONE ||
        err == SCT_ERR_DEVICE_IN_USE) 
    {
        err = SDVR_ERR_NONE;
        sdbg_msg_output(sdvr_dbg_msg,board_index,"Calling sct_reset_board");

        if ((err = sct_reset_board(handle)) == SCT_NO_ERROR)
        {
            err = sct_close_board(handle);
            _ssdvr_boards_info.boards[board_index].is_reset_required = false;
        }
    }

    return err;
}

sdvr_err_e _ssdk_do_board_connect(sx_uint32 board_index,
                                  sdvr_board_settings_t *board_settings)
{
    const char *name;
    sdvr_err_e err;
    sct_board_detail_t board_detail;

    if (_ssdvr_boards_info.boards[board_index].handle)
        return SDVR_ERR_BOARD_CONNECTED;

    name = sct_get_board_name(board_index);
    if (name == NULL) {
        err = SDVR_ERR_INVALID_BOARD;
        sdbg_msg_output_err(sdvr_board_connect_func,err);
        return err;
    }

    /***********************************************************
      Reset the firmware by openning the board, reseting it and
      closing it again.
    ***********************************************************/
    sdbg_msg_output(sdvr_dbg_msg,board_index,"Calling sct_open_board");
    if ((err = sct_open_board(board_index,&_ssdvr_boards_info.boards[board_index].handle)) == SDVR_ERR_NONE ||
        err == SCT_ERR_DEVICE_IN_USE) 
    {
        if 	(_ssdvr_boards_info.boards[board_index].is_reset_required)
        {
            err = SDVR_ERR_NONE;

            sdbg_msg_output(sdvr_dbg_msg,board_index,"Calling sct_reset_board");

            if ((err = sct_reset_board(_ssdvr_boards_info.boards[board_index].handle)) == SCT_NO_ERROR)
            {
                err = sct_close_board(_ssdvr_boards_info.boards[board_index].handle);
                _ssdvr_boards_info.boards[board_index].handle = NULL;
                _ssdvr_boards_info.boards[board_index].is_reset_required = false;
                if (err == SCT_NO_ERROR)
                {
                    err = sct_open_board(board_index,&_ssdvr_boards_info.boards[board_index].handle);
                }
            }
        }
    }
    if (err == SCT_NO_ERROR)
    {
         sx_uint8 major;
         sx_uint8 minor; 
         sx_uint8 revision;
         sx_uint8 build;

        err = sct_get_board_detail(_ssdvr_boards_info.boards[board_index].handle, &board_detail);
        major = board_detail.driver_version[0];
        minor = board_detail.driver_version[1];
        revision = board_detail.driver_version[2];
        build = board_detail.driver_version[3];
        if (err == SDVR_ERR_NONE)
        {
            if (ssdk_is_min_version(major, minor, revision, build, 3,2,0,4))
            {
                /************************************************************
                    Starting version 4.0.0.5 of driver, the sct buffer size
                    is part of the board info. Otherwise, we get the old
                    sct buffer size of 640K.
                *************************************************************/

                _ssdvr_boards_info.boards[board_index].max_sct_buf_size = (640 * 1024); 
                if (ssdk_is_min_version(major, minor, revision, build, 4,0,0,5))
                {
                    _ssdvr_boards_info.boards[board_index].max_sct_buf_size = 
                                                    board_detail.buffer_size;
                }

                /*************************************************************
                   The SDK allocates a temp buffer large enough to hold any 
                   encoded buffer size. This is mostly used for converting 
                   SCE. But it could be used for other temp purposes.
                **************************************************************/                   

                if (_ssdvr_boards_info.temp_venc_buf == NULL)
                {
                    _ssdvr_boards_info.temp_venc_buf_size = 
                        _ssdvr_boards_info.boards[board_index].max_sct_buf_size;

                    _ssdvr_boards_info.temp_venc_buf = malloc(_ssdvr_boards_info.temp_venc_buf_size);
                    if (_ssdvr_boards_info.temp_venc_buf == NULL)
                    {
                        err = SDVR_ERR_OUT_OF_MEMORY;
                    }
                }

            }
            else
            {
                err = SDVR_ERR_WRONG_DRIVER_VERSION;
            }
        }
    }
    if (err == SCT_NO_ERROR)
    {
        sdvr_board_config_t board_config;
        int size_rcv_chan = sizeof(ssdk_pci_channel_info_t);
        int size_decode_chans = sizeof(ssdk_pci_decode_chan_info_t);

        _ssdvr_boards_info.boards[board_index].name = name;
//        _ssdvr_boards_info.boards[board_index].handle = handle;
        _ssdvr_boards_info.boards[board_index].board_index = board_index;
        _ssdvr_boards_info.boards[board_index].is_fw_dead = false;
        _ssdvr_boards_info.boards[board_index].is_h264_SCE = board_settings->is_h264_SCE;
        _ssdvr_boards_info.boards[board_index].audio_rate = board_settings->audio_rate;
        /****************************************************************
          The PCI send data handle get created on the first call to 
          set any of the ROIs map.
        *****************************************************************/
        _ssdvr_boards_info.boards[board_index].pci_send_data_handle = NULL;
        _ssdvr_boards_info.boards[board_index].pci_send_data_buf_size = 0;

        // Set the driver timeout
        sct_set_timeout(_ssdvr_boards_info.boards[board_index].handle,m_sdkParams.timeout);

        /****************************************************************
           Inform the firmware about it's board ID number and the
           video standard that needs to be supported by the system.
        *****************************************************************/
        sdbg_msg_output(sdvr_dbg_msg,board_index,"Calling ssdk_set_board_config");
        if ((err = ssdk_set_board_config(board_index, board_settings)) == SDVR_ERR_NONE)
        {
            // Once communication is stablished with the board via the driver
            // we need to get the SDVR board physical characterstics and
            // allocate memory buffer for the recieving AV frame channels.
            // As well as registering a callback function with the PCI driver 
            // to notify us everytime a sensor was activated or an alarm was 
            // triggered.
            err = SDVR_ERR_CALLBACK_FAILED;
            sdbg_msg_output(sdvr_dbg_msg,board_index,"Calling sct_register_message_callback.");
            if (sct_register_message_callback(_ssdvr_boards_info.boards[board_index].handle,
                        DVR_SIG_HOST,_ssdk_message_callback_func,
                        (void *)&_ssdvr_boards_info.boards[board_index].board_index))
            {
                sdbg_msg_output(sdvr_dbg_msg,board_index,"Calling sdvr_get_board_config.");
                if ((err = sdvr_get_board_config(board_index,&board_config)) == SDVR_ERR_NONE)
                {
                    _ssdvr_boards_info.boards[board_index].max_num_smos =
                        board_config.num_smos;

                    _ssdvr_boards_info.boards[board_index].max_num_encoders =
                    _ssdvr_boards_info.boards[board_index].max_num_video_in =
                        board_config.num_cameras_supported;

                    _ssdvr_boards_info.boards[board_index].recv_channels =
                        calloc(size_rcv_chan * board_config.num_cameras_supported,sizeof(char));

                    _ssdvr_boards_info.boards[board_index].max_num_decoders =
                        board_config.num_decoders_supported;
                    if (board_config.num_decoders_supported)
                        _ssdvr_boards_info.boards[board_index].decode_channels =
                            calloc(size_decode_chans * board_config.num_decoders_supported,sizeof(char));
                    else
                        _ssdvr_boards_info.boards[board_index].decode_channels = NULL;

                    if (_ssdvr_boards_info.boards[board_index].recv_channels == NULL ||
                        (board_config.num_decoders_supported && 
                        _ssdvr_boards_info.boards[board_index].decode_channels == NULL) )
                    {
                        sdbg_msg_output(sdvr_dbg_err_msg,board_index,"Failed to allocat memory for receiving and/or decoder channels");
                        err = SDVR_ERR_INTERNAL;
                    } //if (_ssdvr_boards_info.boards[board_index].recv_channels == NULL)
                    else
                    {
                        time_t tm;
                        /***********************************************
                            Initialize the critical section handling
                        ***********************************************/
#ifdef WIN32
                        time(&tm);
#else
                        {                        
                        struct timeval current_time;
                        gettimeofday(&current_time, NULL);
                        tm = current_time.tv_sec;
                        }


#endif
                        
                        /**********************************************
                                Set the date/time on the board
                        **********************************************/

                        sdvr_set_date_time(board_index, tm);

                        /*********************************************
                         Get the Firmware version and makes sure that
                         this it is supported by this version of the SDK
                        *********************************************/
                        sdvr_get_firmware_version_ex(board_index, &_ssdvr_boards_info.boards[board_index].version);

                        /*
                           Check the firmware version  required by this version of the SDK
                        */
                        if (!ssdk_is_min_version(_ssdvr_boards_info.boards[board_index].version.fw_major,
                            _ssdvr_boards_info.boards[board_index].version.fw_minor,
                            _ssdvr_boards_info.boards[board_index].version.fw_revision,
                            _ssdvr_boards_info.boards[board_index].version.fw_build,
                            4,1,0,24))
                        {
                            err = SDVR_ERR_UNSUPPORTED_FIRMWARE;
                        }

        
                        /* 
                            Only FW versions 4.1.0.8 and above support encoders count        
                        */
                        if (!ssdk_is_min_version(_ssdvr_boards_info.boards[board_index].version.fw_major,
                            _ssdvr_boards_info.boards[board_index].version.fw_minor,
                            _ssdvr_boards_info.boards[board_index].version.fw_revision,
                            _ssdvr_boards_info.boards[board_index].version.fw_build,
                            4,1,0,8))
                        {
                            _ssdvr_boards_info.boards[board_index].max_num_encoders =
                                 board_config.num_encoders_supported;

                        }

                        /*****************************************************
                           Get the authication key for this board.
                        ******************************************************/
                        if (err == SDVR_ERR_NONE && _ssdvr_boards_info.is_auth_key_enabled)
                            err = ssdvr_read_auth_key(board_index);

                    }
                } // if ((err = sdvr_get_board_config()) == SDVR_ERR_NONE)
            } //if (sct_register_message_callback
        } // if ((err = ssdk_set_board_config()) == SDVR_ERR_NONE)
    } //if ((err = sct_open_board()) == SDVR_ERR_NONE)
     
    // Do clean up if there was any error in opening the board.
    if (err != SDVR_ERR_NONE)
    {
        if (_ssdvr_boards_info.boards[board_index].recv_channels != NULL)
            free (_ssdvr_boards_info.boards[board_index].recv_channels);
        if (_ssdvr_boards_info.boards[board_index].decode_channels != NULL)
            free (_ssdvr_boards_info.boards[board_index].decode_channels);

        if (_ssdvr_boards_info.boards[board_index].handle != NULL)
            sct_close_board(_ssdvr_boards_info.boards[board_index].handle);

        _ssdvr_boards_info.boards[board_index].name = NULL;
        _ssdvr_boards_info.boards[board_index].handle = NULL;
        _ssdvr_boards_info.boards[board_index].board_index = 0xFFFF;
        _ssdvr_boards_info.boards[board_index].recv_channels = NULL;
        _ssdvr_boards_info.boards[board_index].decode_channels = NULL;
        _ssdvr_boards_info.boards[board_index].max_num_smos = 0;
        _ssdvr_boards_info.boards[board_index].is_h264_SCE = 0;
        _ssdvr_boards_info.boards[board_index].audio_rate = DVR_AUDIO_RATE_8KHZ;
        _ssdvr_boards_info.boards[board_index].auth_key_len = 0;
        memset(&_ssdvr_boards_info.boards[board_index].auth_key,0, sizeof(_ssdvr_boards_info.boards[board_index].auth_key));
        memset(&_ssdvr_boards_info.boards[board_index].version,0, sizeof(_ssdvr_boards_info.boards[board_index].version));
    }

    sdbg_msg_output_err(sdvr_board_connect_func,err);
    return err;
}

/****************************************************************************
  This function connects to a board and sets up communication channels and
  other system resources required to handle the board.

  This function should only be called once per board. 

  @NOTE: To change and set a new video standard, the DVR firmware
  is reset every time the DVR board is connected.@

  Parameters:

    "board_index" - The number of the board to which you want to connect.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "video_std" - The video standard and maximum system-wide resolution
    of all the cameras connected to the board. 

    "is_h264_SCE" - This field specifies whether the SDK or the H.264 
    encoder should perform  start code emulation (SCE) for h.264 video 
    frames. For performance reasons, 
    it is highly recommended to always set this field to 1 so that the 
    SDK performs SCE, except for the embedded DVR Applications it 
    must be set to 0 which means the encoder to perform this task.

    This field is ignored if connected to any firmware version prior to
    3.2.0.19.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remark:

    All the cameras connected to the board must be of the same video 
    standard (NTSC or PAL). In the event that you have connected mixed video standard
    cameras, the video frames for any camera that is different from the
    specified video standard will be distorted.

    Another method of connecting to the DVR board is by calling 
    sdvr_board_connect_ex(). This function provides more board 
    settings option.
*****************************************************************************/
sdvr_err_e sdvr_board_connect(sx_uint32 board_index,
                              sdvr_video_std_e video_std,
                              sx_bool is_h264_SCE)
{
    sdvr_err_e err;
    sdvr_board_settings_t board_settings;

    memset(&board_settings,0, sizeof(board_settings));

    board_settings.video_std = video_std;
    board_settings.is_h264_SCE = is_h264_SCE;
    board_settings.audio_rate = SDVR_AUDIO_RATE_8KHZ;

    err = _ssdk_do_board_connect(board_index, &board_settings);

    return err;
}

/****************************************************************************
  This function connects to a board and sets up communication channels and
  other system resources required to handle the board. This function is
  very similar to sdvr_board_connect() except it provides more DVR board
  system settings.

  This function should only be called once per board. 

  @NOTE: To change and set a new video standard, the DVR firmware
  is reset every time the DVR board is connected.@

  Parameters:

    "board_index" - The number of the board to which you want to connect.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "board_settings" - A pointer to data structure to set different
    board system settings. (See sdvr_board_settings_t for more
    details.)

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remark:

    All the cameras connected to the board must be of the same video 
    standard (NTSC or PAL). In the event that you have connected mixed video standard
    cameras, the video frames for any camera that is different from the
    specified video standard will be distorted.

    It is highly recommended to memset board_settings parameter to zero 
    before initializing
    its fields and calling this function. This allows backward compatibilty
    to the DVR Application if more board settings are specified in the future.
*****************************************************************************/
sdvr_err_e sdvr_board_connect_ex(sx_uint32 board_index,
                            sdvr_board_settings_t *board_settings)
{
    sdvr_err_e err;

    err = _ssdk_do_board_connect(board_index, board_settings);

    return err;
}
/****************************************************************************
  This function disconnects from a board, and releases all the board
  specific resources allocated in the SDK and driver. 
  After this call, all attempts to communicate with the board fail. 

  To shut down your system cleanly, you must call this function for every 
  SDVR board to which you were connected before you exit your application.

  Parameters:

    "board_index" - The number of the board from which you want to disconnect. 
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
    If disconnect fails, you must restart your application before you 
    can connect to the same board again.

  Remarks:

    You must call sdvr_upgrade_firmware() to load the firmware every time 
    prior to connecting to the DVR board if no firmware is loaded into 
    the board's none-volatile  memory.

*****************************************************************************/
sdvr_err_e sdvr_board_disconnect(sx_uint32 board_index)
{
    sdvr_err_e err;
    sx_uint8 chan_num;
    char msgPrompt[256];

    sdbg_msg_output(sdvr_board_disconnect_func, board_index, (void *)NULL);

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE){
        sdbg_msg_output_err(sdvr_board_disconnect_func,err);
        return err;
    }

    sdbg_msg_output(sdvr_dbg_msg,board_index,"sdvr_board_disconnect(): Destroying all connected encoder channels...");
    for (chan_num = 0; chan_num < _ssdvr_boards_info.boards[board_index].max_num_video_in; chan_num++)
    {
        ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

        if (enc_chan_info->sdk_chan_handle)
        {
            /*
               When destroying the channel on the board disconnect
               force stop all the streaming so that we can destroy the channel.
            */
            if (enc_chan_info->is_smo_enable)
            {
                sdvr_smo_grid_t smo_grid;
                memset(&smo_grid,0,sizeof smo_grid);
                smo_grid.enable = false;
                sdvr_set_smo_grid(enc_chan_info->sdk_chan_handle, &smo_grid);
            }
            if (enc_chan_info->sub_enc_info[SDVR_ENC_PRIMARY].is_enc_av_streaming)
                sdvr_enable_encoder(enc_chan_info->sdk_chan_handle, SDVR_ENC_PRIMARY, false);
            if (enc_chan_info->sub_enc_info[SDVR_ENC_SECONDARY].is_enc_av_streaming)
                sdvr_enable_encoder(enc_chan_info->sdk_chan_handle, SDVR_ENC_SECONDARY, false);
            if (enc_chan_info->is_raw_av_streaming[0])
                sdvr_stream_raw_video(enc_chan_info->sdk_chan_handle,SDVR_VIDEO_RES_DECIMATION_NONE,0,false);
            if (enc_chan_info->is_raw_av_streaming[1])
                sdvr_stream_raw_video_secondary(enc_chan_info->sdk_chan_handle,SDVR_VIDEO_RES_DECIMATION_NONE,0,false);
            if (enc_chan_info->is_raw_audio_streaming)
                sdvr_stream_raw_audio(enc_chan_info->sdk_chan_handle,false);

            if ((err = sdvr_destroy_chan( enc_chan_info->sdk_chan_handle)) != SDVR_ERR_NONE)
            {
                sprintf(msgPrompt,"sdvr_board_disconnect(): Failed to destroy encoder channel %d on board = %d. [Error = %s]\n",chan_num, board_index,
                    sdvr_get_error_text(err));
                sdbg_msg_output(sdvr_dbg_err_msg,board_index,msgPrompt); 
                /********************************************************* 
                   Note: Even after failing to destroy channel we must continue to 
                         destroy others, so the the PCI buffers are released
                **********************************************************/
            }
        }
    }

    // destroy the decoder channels if the firmware support decoding.
    if (_ssdvr_boards_info.boards[board_index].max_num_decoders)
    {
        sdbg_msg_output(sdvr_dbg_msg,board_index,"sdvr_board_disconnect(): Destroying all connected decoder channels...");
        for (chan_num = 0; chan_num < _ssdvr_boards_info.boards[board_index].max_num_decoders; chan_num++)
        {
            if (_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].sdk_chan_handle)
            {
                if ((err = sdvr_destroy_chan( _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].sdk_chan_handle)) != SDVR_ERR_NONE)
                {
                    sprintf(msgPrompt,"sdvr_board_disconnect(): Failed to destroy decoder channel %d on board = %d. [Error = %s]\n",chan_num, board_index,
                        sdvr_get_error_text(err));
                    sdbg_msg_output(sdvr_dbg_err_msg,board_index,msgPrompt); 
                    /******************************************************* 
                       Note: Even after failing to destroy channel we must continue to 
                             destroy others, so the the PCI buffers are released
                    ********************************************************/
                }
            }
        }
    }

    /************************************************************
       Close the PCI send data channel only if it ever was openned.
       NOTE: This channel is openned once and only if there was
             ever a call to set the ROIs map.
    ************************************************************/
    if (_ssdvr_boards_info.boards[board_index].pci_send_data_handle)
    {
        dvr_ioctl_t cmd_strct;
        int cmd_strct_size = sizeof(cmd_strct);
        memset(&cmd_strct,0,cmd_strct_size);


        // close the ROI SCT channel and inform the firmware to do the same.
        cmd_strct.ioctl_code = DVR_IOCTL_CODE_CLOSE_CHAN;
        ssdk_message_board_send(board_index, DVR_SET_IOCTL, &cmd_strct, sizeof(cmd_strct));
        sct_channel_close(_ssdvr_boards_info.boards[board_index].pci_send_data_handle);
        err = ssdk_message_board_recv(board_index, DVR_REP_IOCTL, &cmd_strct, 0, 0);

        _ssdvr_boards_info.boards[board_index].pci_send_data_handle = NULL;
        _ssdvr_boards_info.boards[board_index].pci_send_data_buf_size = 0;

    }
    /************************************************************
      Before exiting the board make sure to disable the watchdog
      timer otherwise, once the application exists, the watchdog
      timer will expire.
    *************************************************************/
    sdvr_set_watchdog_state(board_index, 0);

    /**************************************************************
        We must re-set the board to clean-up the driver.
    ***************************************************************/
    _ssdvr_boards_info.boards[board_index].is_reset_required = true;
    sdbg_msg_output(sdvr_dbg_msg,board_index,"Calling sct_close_board");
    sct_close_board(_ssdvr_boards_info.boards[board_index].handle);


    /*************************************************************
     Free the allocated buffers for the encoder and decoder channels
    *************************************************************/
    _ssdvr_boards_info.boards[board_index].name = NULL;
    _ssdvr_boards_info.boards[board_index].handle = NULL;
    _ssdvr_boards_info.boards[board_index].board_index = 0xFFFF;

    _ssdvr_boards_info.boards[board_index].max_num_video_in = 0;

    if (_ssdvr_boards_info.boards[board_index].recv_channels)
    {
        free(_ssdvr_boards_info.boards[board_index].recv_channels);
        _ssdvr_boards_info.boards[board_index].recv_channels = NULL;
    }

    _ssdvr_boards_info.boards[board_index].max_num_decoders = 0;

    if (_ssdvr_boards_info.boards[board_index].decode_channels)
    {
        free(_ssdvr_boards_info.boards[board_index].decode_channels);
        _ssdvr_boards_info.boards[board_index].decode_channels = NULL;
    }
    
    _ssdvr_boards_info.boards[board_index].max_num_encoders = 0;

    _ssdvr_boards_info.boards[board_index].max_num_smos = 0;


    _ssdvr_boards_info.boards[board_index].is_h264_SCE = 0;
    _ssdvr_boards_info.boards[board_index].audio_rate = DVR_AUDIO_RATE_8KHZ;

    memset(&_ssdvr_boards_info.boards[board_index].auth_key[0],0,
        sizeof(_ssdvr_boards_info.boards[board_index].auth_key));
    _ssdvr_boards_info.boards[board_index].auth_key_len = 0;

    memset(&_ssdvr_boards_info.boards[board_index].version,0, sizeof(_ssdvr_boards_info.boards[board_index].version));
    sdbg_msg_output_err(sdvr_board_disconnect_func,err);
    return err;
}

/****************************************************************************
  This function is used to load a firmware on to the board. This function 
  loads the contents of the given file (which is required to be in a 
  .rom format) into the board memory, and directs the board to 
  burn it into volatile memory. The board then automatically "reboots", 
  i.e., starts up with the new firmware, without requiring a PC reboot. 

  Before you load the firmware, you must disconnect from the board using 
  sdvr_board_disconnect(). You must shut down all board functionality before 
  attempting to load a new firmware. Also, after you upgrade firmware, 
  you must reconnect to the board using sdvr_board_connect(). 

  @NOTE:  You must load the firmware every time prior to connecting 
  to the DVR board if no firmware is loaded into the board's none-volatile 
  memory.@
  
  Parameters:

    "board_index" - The number of the board that you want to upgrade. 
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "firmware_file_name" - A string containing the name of the new
    firmware file in .rom format.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

*****************************************************************************/
sdvr_err_e sdvr_upgrade_firmware(sx_uint32 board_index, 
                                 char *firmware_file_name)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sct_board_t handle;
    int retry = 2;

    if (firmware_file_name == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else if ((board_index + 1) > _ssdvr_boards_info.board_num)
        err = SDVR_ERR_INVALID_BOARD;
    else if (_ssdvr_boards_info.boards[board_index].handle != NULL)
        err = SDVR_ERR_BOARD_CONNECTED;

    if (err == SDVR_ERR_NONE)
    {
        if ((err = sct_open_board(board_index,&handle)) == SDVR_ERR_NONE) 
        {
            err = SDVR_ERR_NONE;
            sdbg_msg_output(sdvr_upgrade_firmware_func,board_index,"Calling sct_reset_board");

            /*************************************************************************
               We must always reset the board before loading new firmware.
               NOTE: There is a 10 seconds interval between the reset which causes the
                     boot-loader to be installed and you can load a new firmware. If 
                     you miss this period the sct_load_firmware() fails. Because of 
                     this restriction you can not have a break point between the 
                     sct_reset_board() and sct_load_firmware().
               ----------------------------------------------------------------------
               Due to any crash in the previous load of the PCI device driver, the
               first call to sct_reset_board() may fail but this will cause unload
               and reload of the driver. That is why we try twice to load reset the
               board before we give up and report and error.
            *************************************************************************/
            do {
                if ((err = sct_reset_board(handle)) == SCT_NO_ERROR)
                {
                    _ssdvr_boards_info.boards[board_index].is_reset_required = false;
                   if (!sct_load_firmware(handle, firmware_file_name, NULL))
                       err = SDVR_ERR_LOAD_FIRMWARE;

                   /***************************************************
                      Note: sct_get_bootloader_version() can only be called
                            after sct_load_firmware().
                   ***************************************************/
                   _ssdvr_boards_info.boards[board_index].boot_loader_ver = sct_get_bootloader_version(handle);
                }
            } while ( err != SDVR_ERR_NONE && --retry);
            sct_close_board(handle);
        }
    }
    sdbg_msg_output_err(sdvr_upgrade_firmware_func,err);
    return err;
}

/****************************************************************************
   This function gets the SDK parameters that control resource allocation
   in the SDK. 
   
   Parameters:

     "sdk_params" - A pointer to a structure containing the SDK information 
     that will be filled when this function returns.
 
   Returns:

     SDVR_ERR_NONE - On success. Otherwise, see the error code list.

   Remarks:

     We recommend that you call this function prior to the call to 
     sdvr_set_sdk_params() to preserve the default values of any
     SDK parameters that do not need to be changed.
****************************************************************************/
sdvr_err_e sdvr_get_sdk_params(sdvr_sdk_params_t *sdk_params)
{
    if (sdk_params == NULL)
        return SDVR_ERR_INVALID_ARG;

    memcpy(sdk_params,&m_sdkParams,sizeof(m_sdkParams));
    sdbg_msg_output(sdvr_get_sdk_params_func, 0, sdk_params);
    return SDVR_ERR_NONE;
}

/****************************************************************************
   This function sets the SDK parameters that control resource allocation
   in the SDK. It must be called before you connect to any boards
   to assign buffer resources. Otherwise, the default buffer resource values
   will be used by the SDK. Any subsequent calls to this function after 
   being connected will only result in changing the debug information 
   or the PCI response timeout value.

   We highly recommend that you not change the default buffer size or numbers
   unless it is needed for performance reasons.

   To change only some SDK parameters and keep the default setting for the
   rest, you should call sdvr_get_sdk_params() to get the existing 
   parameter settings, change the desired parameters, and then call 
   sdvr_set_sdk_params().

   There must be at least one buffer specified for each of the 
   system buffers.

   Parameters:

     "sdk_params" - A pointer to a structure containing the SDK information 
     that you want to set.
 
   Returns:

     SDVR_ERR_NONE - On success. Otherwise, see the error code list.

   Remarks:

      See sdvr_sdk_params_t for detail SDK parameter information and 
      buffer size limits.
****************************************************************************/
sdvr_err_e sdvr_set_sdk_params(sdvr_sdk_params_t *sdk_params)
{
    int i;
    sdvr_err_e err;
    char *debug_file_name = NULL;

    if (sdk_params == NULL)
        return SDVR_ERR_INVALID_ARG;

    // Only the following SDK Parameters can be changed even
    // if we are connected to other boards
    m_sdkParams.timeout = sdk_params->timeout;
    m_sdkParams.debug_flag = sdk_params->debug_flag;

    // If a new debug file name is given, we should pass
    // it to the sdbg_init to change the log file.
    // Otherwise, we keep the same old log file.
    if (m_sdkParams.debug_file_name == NULL ||
        (sdk_params->debug_file_name && strcmp(m_sdkParams.debug_file_name,sdk_params->debug_file_name)))
        debug_file_name = sdk_params->debug_file_name;

    err = sdbg_init(debug_file_name, sdk_params->debug_flag);
    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_set_sdk_params_func,err);
        return err;
    }

    if (sdk_params->enc_buf_num == 0 || sdk_params->raw_buf_num == 0 ||
        sdk_params->dec_buf_num == 0 || sdk_params->dec_buf_size == 0)
    {
        err = SDVR_ERR_SDK_NO_FRAME_BUF;
        sdbg_msg_output_err(sdvr_set_sdk_params_func,err);
        return err;
    }
    if (sdk_params->enc_buf_num > _SSDK_MAX_ENC_BUF || 
        sdk_params->raw_buf_num > _SSDK_MAX_RAW_BUF ||
        sdk_params->dec_buf_num > _SSDK_MAX_DEC_BUF)
    {
        err = SDVR_ERR_SDK_BUF_EXCEEDED;
        sdbg_msg_output_err(sdvr_set_sdk_params_func,err);
        return err;
    }


    if (sdk_params->debug_file_name)
    {
        strncpy(_ssdvr_boards_info.log_file_name,sdk_params->debug_file_name,MAX_FILE_PATH);
        _ssdvr_boards_info.log_file_name[MAX_FILE_PATH] = '\0';
        m_sdkParams.debug_file_name = &_ssdvr_boards_info.log_file_name[0];
    }

    sdbg_msg_output(sdvr_set_sdk_params_func, 0, sdk_params);

    /***********************************************************************
        We can not change the SDK buffer parameters while 
        we are connected to any boards.
    ************************************************************************/
    for (i = 0; i < MAX_PCI_BOARDS; i++)
    {
        if (_ssdvr_boards_info.boards[i].handle) 
        {
            // Set the driver timeout
            sct_set_timeout(_ssdvr_boards_info.boards[i].handle,m_sdkParams.timeout);
            err =  SDVR_ERR_BOARD_CONNECTED;
        }
    }
    if (err == SDVR_ERR_BOARD_CONNECTED)
    {
        sdbg_msg_output_err(sdvr_set_sdk_params_func,SDVR_ERR_NONE);
        return err;
    }
    /***********************************************************************
             Save the sdk buffer parameters 
    ***********************************************************************/
    memcpy(&m_sdkParams,sdk_params,sizeof(m_sdkParams));
    m_sdkParams.enc_buf_num = sdk_params->enc_buf_num;
    m_sdkParams.raw_buf_num = sdk_params->raw_buf_num;
    m_sdkParams.dec_buf_num = sdk_params->dec_buf_num;
    m_sdkParams.dec_buf_size = sdk_params->dec_buf_size;
    m_sdkParams.debug_file_name = &_ssdvr_boards_info.log_file_name[0];
    sdbg_msg_output_err(sdvr_set_sdk_params_func,SDVR_ERR_NONE);
    return SDVR_ERR_NONE;
}

/****************************************************************************
  This function returns the SDK version.

  Stretch follows the convention of using four numbers for version 
  control. A change in the major number indicates major changes to 
  functionality, a change in the minor number indicates minor 
  changes to functionality, and a change in the revision number indicates
  significant bug fixes that were introduced in the minor change functionality. 
  A change to the build number indicates only bug fixes that do not 
  change functionality. 


  This function can be called before or after sdvr_sdk_init();

  Parameters:

    "major" - Pointer to a variable that will hold the major version of the 
    SDK when this function returns. 

    "minor" - Pointer to a variable that will hold the minor version of 
    the SDK when this function returns. 

    "revision" - Pointer to a variable that will hold the revision version of the 
    SDK when this function returns. 

    "build" - Pointer to a variable that will hold the build or bug fix 
    version of the SDK when this function returns. 

  Returns:

    Nothing. 
****************************************************************************/
void sdvr_get_sdk_version(sx_uint8 *major, 
                          sx_uint8 *minor, 
                          sx_uint8 *revision,
                          sx_uint8 *build)
{
    if (major != NULL && minor != NULL && revision != NULL && build != NULL)
    {
        *major = SDVR_SDK_VERSION_MAJOR;
        *minor = SDVR_SDK_VERSION_MINOR;
        *revision = SDVR_SDK_VERSION_REVISION;
        *build = SDVR_SDK_VERSION_BUILD;
    }
    return;
}


/****************************************************************************
  This function returns the driver version.

  Stretch follows the convention of using four numbers for version 
  control. A change in the major number indicates major changes to 
  functionality, a change in the minor number indicates minor 
  changes to functionality, and a change in the revision number indicates
  significant bug fixes that were introduced in the minor change functionality. 
  A change to the build number indicates only bug fixes that do not 
  change functionality. 

  Parameters:

    "board_index" - The number of the board whose driver version we want.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "major" - Pointer to a variable that will hold the major version of the 
    driver when this function returns. 

    "minor" - Pointer to a variable that will hold the minor version of 
    the driver when this function returns.

    "revision" - Pointer to a variable that will hold the revision version of the 
    driver when this function returns. 

    "build" - Pointer to a variable that will hold the build or bug fix 
    version of the driver when this function returns. 

  Returns:

     SDVR_ERR_NONE - On success. Otherwise see the error code list.
****************************************************************************/
sdvr_err_e sdvr_get_driver_version(sx_uint32 board_index,
                             sx_uint8 *major, 
                             sx_uint8 *minor, 
                             sx_uint8 *revision,
                             sx_uint8 *build)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sct_board_detail_t board_detail;
    sx_bool temp_open = false;

    if ((board_index + 1) > _ssdvr_boards_info.board_num)
        err = SDVR_ERR_INVALID_BOARD;
    else if (major == NULL || minor == NULL || build == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else if (_ssdvr_boards_info.boards[board_index].handle == NULL)
    {
        temp_open = true;
        err = sct_open_board(board_index,
            &_ssdvr_boards_info.boards[board_index].handle);
    }
    if (err == SDVR_ERR_NONE)
    {
        err = sct_get_board_detail(_ssdvr_boards_info.boards[board_index].handle, &board_detail);
        if (temp_open)
        {
            sct_close_board(_ssdvr_boards_info.boards[board_index].handle);
            _ssdvr_boards_info.boards[board_index].handle = NULL;
        }
        *major = board_detail.driver_version[0];
        *minor = board_detail.driver_version[1];
        *revision = board_detail.driver_version[2];
        *build = board_detail.driver_version[3];
    }
    sdbg_msg_output_err(sdvr_get_driver_version_func,  err);

    return err ;
}

/****************************************************************************
  This function returns the firmware and boot loader version.

  Stretch follows the convention of using four numbers for version 
  control. A change in the major number indicates major changes to 
  functionality, a change in the minor number indicates minor 
  changes to functionality, and a change in the revision number indicates
  significant bug fixes that were introduced in the minor change functionality. 
  A change to the build number indicates only bug fixes that do not 
  change functionality. 

  You can access the build date of the firmware in addition to the firmware version and
  version of the boot loader.

  This function must be called after calling sdvr_upgrade_firmware() to
  get the boot loader information and after sdvr_board_connect()
  to get both the firmware and boot loader versions.

  This function can still be used to get the BSP and bootloader version even if
  the call to sdvr_upgrade_firmware() returns an error. In such cases, no other
  version information is available.

  Parameters:

    "board_index" - The number of the board whose firmware version we want.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "version_info" - A pointer to a structure that holds the firmware
    and boot loader version numbers.

  Returns:

    SDVR_ERR_NONE - Both the firmware and boot loader version information
    are returned.

    SDVR_ERR_BOARD_NOT_CONNECTED - The firmware version is not returned
    because the given board number is not connected. But the
    boot loader information might have been returned if it is non-zero.

    Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_get_firmware_version_ex(sx_uint32 board_index,
                                      sdvr_firmware_ver_t *version_info)
{
    sdvr_err_e err;
    dvr_ver_info_t cmd_strct;

    if (version_info == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else
    {
        memset(version_info, 0 , sizeof(sdvr_firmware_ver_t));

        err = ssdk_validate_board_ndx(board_index);

        // As long as the given board_index is valid we can get bootloader and
        // BSP version from the sct call made after upgrate firmware call
        if (err != SDVR_ERR_INVALID_BOARD)
        {
            version_info->bootloader_major = _ssdvr_boards_info.boards[board_index].boot_loader_ver.u.version_fields.ml_rev_major;
            version_info->bootloader_minor = _ssdvr_boards_info.boards[board_index].boot_loader_ver.u.version_fields.ml_rev_minor;
            version_info->bsp_major = _ssdvr_boards_info.boards[board_index].boot_loader_ver.u.version_fields.bsp_rev_major;
            version_info->bsp_minor = _ssdvr_boards_info.boards[board_index].boot_loader_ver.u.version_fields.bsp_rev_minor;
        }

        // In case we successfuly connected to a board, get all the 
        // version information including bootloader and BSP.
        // NOTE: if the firmware was flashed on to the board, the only
        //       way to get bootloader and BSP version is from the DVR_GET_VER_INFO
        if (err == SDVR_ERR_NONE)
        {
            memset(&cmd_strct,0, sizeof(cmd_strct));
            ssdk_message_board_send(board_index, DVR_GET_VER_INFO, &cmd_strct, sizeof(cmd_strct));
            ssdk_message_board_recv(board_index, DVR_REP_VER_INFO, &cmd_strct, 0, 0);

            if ((err = cmd_strct.status) == SDVR_ERR_NONE )
            {

                version_info->fw_major = cmd_strct.version_major;
                version_info->fw_minor = cmd_strct.version_minor;
                version_info->fw_revision = cmd_strct.version_bug; // revision
                version_info->fw_build = cmd_strct.version_build;
                version_info->fw_build_year = cmd_strct.build_year;
                version_info->fw_build_month = cmd_strct.build_month;
                version_info->fw_build_day = cmd_strct.build_day;

                // Only FW versions 3.2.0.1 and above support bootload and BSP version
                if (ssdk_is_min_version(cmd_strct.version_major,cmd_strct.version_minor,cmd_strct.version_bug,cmd_strct.version_build,
                    3,2,0,1))
                {
                    version_info->bootloader_major  = cmd_strct.bootloader_ver_major;
                    version_info->bootloader_minor  = cmd_strct.bootloader_ver_minor;
                    version_info->bsp_major         = cmd_strct.bsp_ver_major;
                    version_info->bsp_minor         = cmd_strct.bsp_ver_minor;
                }

            }
        }
        sdbg_msg_output(sdvr_get_firmware_version_ex_func, board_index, (void *)version_info);

    }
    sdbg_msg_output_err(sdvr_get_firmware_version_ex_func,  err);

    return err;
}


/****************************************************************************
  This function is used to register the signal callback function. There
  can be only one function registered for this callback. 

  For the DVR Application to be notified of asynchronous events such as
  error conditions in the DVR firmware, it has to register a callback. 
  The callback function has as its arguments the 
  board index and a pointer to data structure associated with the signal event. 
  This information  can be used in the callback function to determine which
  device caused the signal to be triggered and whether the DVR Application
  should be closed or continue the execution.

  Parameters:

    "signals_callback" -  A pointer to the callback function. 

  Returns:

    Returns a pointer to the previous function that was registered for 
    this callback. If no function was registered previously, the return value is a 
    NULL function pointer. This can be used to temporarily override a 
    callback function with another and then to restore the original callback. 
****************************************************************************/
sdvr_signals_callback sdvr_set_signals_callback(sdvr_signals_callback signals_callback)
{
    sdvr_signals_callback old_callback = _ssdvr_boards_info.signals_callback;

    _ssdvr_boards_info.signals_callback = signals_callback;

    return old_callback;
}
/****************************************************************************
  This function is used to register the sensor callback function. There
  can be only one function registered for this callback. 

  For the DVR Application to be notified of sensor events, it has
  to register a callback. The callback function has as its arguments the 
  board index and the sensor map that contains a bit pattern describing the
  sensors that have triggered. This information can be 
  used in the callback function to determine which sensors have 
  triggered and take the appropriate action. Bit 0 in the
  sensor map corresponds to sensor 0, bit 1 to sensor 1, and so on.

  Parameters:

    "sensor_callback" -  A pointer to the callback function. 

  Returns:

    Returns a pointer to the previous function that was registered for 
    this callback. If no function was registered previously, the return value is a 
    NULL function pointer. This can be used to temporarily override a 
    callback function with another and then to restore the original callback. 
****************************************************************************/
sdvr_sensor_callback sdvr_set_sensor_callback(sdvr_sensor_callback sensor_callback) 
{
    sdvr_sensor_callback old_sensor_callback = _ssdvr_boards_info.sensor_callback;

    _ssdvr_boards_info.sensor_callback = sensor_callback;

    return old_sensor_callback;
}

/****************************************************************************
  This function is used to register the video alarm callback. There
  can only be one function registered for this callback. 

  For the DVR Application to be notified of video alarm events, 
  it has to register a callback. The callback function has as its 
  arguments the video channel handle, the alarm type, and the value
  associated with the alarm.
  This information can be used in the callback function to determine the 
  type of event that occurred and, to take the appropriate action.
  There is one callback per video event, even if the events
  happen simultaneously.

  Parameters:

    "alarm_callback" - A pointer to the callback function.

  Returns:

    Returns a pointer to the previous function that was registered for this 
    callback. If no function was registered previously, the return value is a NULL 
    function pointer. This can be used to temporarily override a 
    callback function with another and then restore the original callback. 

  Remarks:

    See sdvr_video_alarm_callback definition for more detailed information.

****************************************************************************/
sdvr_video_alarm_callback sdvr_set_video_alarm_callback(sdvr_video_alarm_callback video_alarm_callback)
{
    sdvr_video_alarm_callback old_video_alarm_callback = _ssdvr_boards_info.video_alarm_callback;
    _ssdvr_boards_info.video_alarm_callback = video_alarm_callback;
    return old_video_alarm_callback;
}

/****************************************************************************
  This function is used to register the video send confirmation callback. There
  can only be one function registered for this callback. 

  For the DVR Application to be notified of video sent confirmation, 
  it has to register a callback. The callback function has as its 
  arguments the channel handle.
  This information can be used in the callback function to determine  
  for which channel the video was sent.

  Parameters:

    "conf_callback" - A pointer to the callback function.

  Returns:

    Returns a pointer to the previous function that was registered for this 
    callback. If no function was registered previously, the return value is a NULL 
    function pointer. This can be used to temporarily override a 
    callback function with another and then restore the original callback. 

  Remarks:

    See sdvr_send_confirmation_callback definition for more detailed information.

****************************************************************************/
sdvr_send_confirmation_callback sdvr_set_confirmation_callback(sdvr_send_confirmation_callback conf_callback)
{
    sdvr_send_confirmation_callback old_confirmation_callback = _ssdvr_boards_info.conf_callback;
    _ssdvr_boards_info.conf_callback = conf_callback;
    return old_confirmation_callback;
}

/****************************************************************************
  This function is used to register the AV frame callback function. There
  can be only one function registered for this callback. 
  The callback is called every time encoded AV, raw video, and raw 
  audio frames are received from the SDVR boards. The function 
  has as its arguments the board index, the channel number, the 
  frame type, and whether the frame is from a primary channel. 
  This information can be used in the callback function to perform the 
  appropriate action; encoded frames are saved to disk, raw video 
  frames are displayed, and raw audio frames are played. 

  The callback function is used for asynchronous notification, somewhat like
  a hardware interrupt. We recommend that you note the information
  provided in the callback, but not process the A/V frame itself in the
  callback function. That should be done in the appropriate routine or thread
  dedicated to processing encoded and raw frames.

  @NOTE: It is not necessary to register this callback. A polling mechanism
  can be used with sdvr_get_av_buffer() to get the buffers.@

  Parameters:

    "av_frame_callback" - A pointer to the callback function.

  Returns:

    Returns a pointer to the previous function that was registered for this 
    callback. If no function was registered previously, the return value is a NULL 
    function pointer. This can be used to temporarily override a 
    callback function with another and then to restore the original callback.

  Example callback function:
  {
    void av_frame_callback(sdvr_chan_handle_t handle,
       sdvr_frame_type_e frame_type, sx_bool is_primary)
    {
        sdvr_av_buffer_t  *av_buffer;
        sdvr_yuv_buffer_t *yuv_buf;
        switch (frame_type)
        {
        case SDVR_FRAME_AUDIO_ENCODED:
            if (sdvr_get_av_buffer(handle, SDVR_FRAME_AUDIO_ENCODED, 
                                   &av_buffer) == SDVR_ERR_NONE)
            {
                  <add the buffer to an audio queue to be processed from 
                  a different thread>
                  <The buffer should be released from that thread when 
                  you processed the audio frame.>
            }
            break;
        case SDVR_FRAME_RAW_VIDEO:
            if (sdvr_get_yuv_buffer(handle, &yuv_buf) == SDVR_ERR_NONE)
            {
                  <add the buffer to a YUV queue to be processed from 
                  a different thread>
                  <The YUV buffer should be released from that thread
                  when you processed the frame.>
            }
            break;
        case SDVR_FRAME_H264_IDR:
        case SDVR_FRAME_H264_I:
        case SDVR_FRAME_H264_P:
        case SDVR_FRAME_H264_B:
        case SDVR_FRAME_H264_SPS:
        case SDVR_FRAME_H264_PPS:
        case SDVR_FRAME_JPEG:
        case SDVR_FRAME_MPEG4_I:
        case SDVR_FRAME_MPEG4_P:
        case SDVR_FRAME_MPEG4_VOL:
            if (sdvr_get_av_buffer(handle, 
              is_primary ? SDVR_FRAME_VIDEO_ENCODED_PRIMARY : 
              SDVR_FRAME_VIDEO_ENCODED_SECONDARY, &av_buffer) == SDVR_ERR_NONE)
            {
                  <add the buffer to a video encoded queue to be processed
                  from a different thread.>
                  <The buffer should be released from that thread when 
                  you processed the video frame.>
            }
            break;
        }
        NOTE: Any buffer that is not retrieved(i.e. no call to the
              "sdvr_get_xxx_buffer") will be freed by the SDK once
              the frame queue is full.
    }
  }
****************************************************************************/
sdvr_av_frame_callback sdvr_set_av_frame_callback(sdvr_av_frame_callback av_frame_callback)
{
    sdvr_av_frame_callback old_av_frame_callback = _ssdvr_boards_info.av_frame_callback;

    // Save the new callback function to be called by the SDK each time we receive a frame.
    _ssdvr_boards_info.av_frame_callback = av_frame_callback;

    /******************************************************************
    NOTE: At this time we only are saving the user defined callback function.
    Every receiving channel will be assigned an intermidate SDK callback
    function when it is being enabled.
    *******************************************************************/

    // return the previous callback function.
    return old_av_frame_callback;
}
/****************************************************************************
  This function is used to register the callback function that displays the
  debug message.

  Through this callback mechanism, the SDK alerts the DVR 
  Application whenever tracing error messages need to be displayed on the
  screen.

  The callback function takes as its argument the string buffer to display.

  @NOTE: If this callback is not registered and you enable the display-to-screen
  flag, the message is written to stdout.@

  Parameters:

    "display_debug_callback" - A pointer to the callback function. 

  Returns:

    Returns a pointer to the previous function that was registered for this 
    callback. If no function was previously registered, the return value is a NULL 
    function pointer. This can be used to temporarily override a 
    callback function with another and then to restore the original callback.
****************************************************************************/
sdvr_display_debug_callback sdvr_set_display_debug(sdvr_display_debug_callback display_debug_callback)
{
    sdvr_display_debug_callback old_callback = _ssdvr_boards_info.disp_debug_callback;

    // Save the new callback function to be called by the SDK each time we
    // need to display a tracing message.
    _ssdvr_boards_info.disp_debug_callback = display_debug_callback;

    // return the previous callback function.
    return old_callback;

}

/****************************************************************************
  This function returns a pointer to the callback to be used to display
  the tracing debug messages.

  This is an internal function.

  Parameters:

    None.

  Returns:

    Returns a pointer to the display function that was registered for this 
    callback. If no function was registered, the return value is a NULL 
    function pointer. 
****************************************************************************/
sdvr_display_debug_callback ssdvr_get_display_bebug_callback()
{
    return _ssdvr_boards_info.disp_debug_callback;
}

/****************************************************************************
  This functions registers an intermidate SDK buffer callback for the 
  given PCI channel with PCI driver. All the encoders channels and HMO only
  channels have the same callback and all the decoders have the similar
  callback function.

  NOTE: Every receiving channel will be assigned this callback function when
  it is being enabled, regardless of when the DVR Application actually assigns
  its callback.

  Parameters:

    "pci_handle"  The SCT PCI channel handle to a assign the A/V frame callback
    "handle"      The SDK channel handle to identify the channel that
                  the frame belong to when the callback is called.

  Returns:

    None.

  Remark: 
  
    This is an inernal function used by the SDK.
****************************************************************************/
void ssdk_register_sdk_chan_callback(sct_channel_t pci_handle,sdvr_chan_handle_t handle)
{
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);
    ssdk_pci_decode_chan_info_t *dec_chan_info = ssdk_get_dec_chan_strct(board_index, chan_num);
    
    // Setup approporiate call back function for the channel type.

    if (chan_type == SDVR_CHAN_TYPE_OUTPUT)
    {
        sct_register_callback(pci_handle, 
            _ssdk_decoder_send_conf_callback_func, 
            (void*)&_ssdvr_boards_info.boards[board_index].smo_info[chan_num].sdk_chan_handle);
    }
    else if (chan_type == SDVR_CHAN_TYPE_DECODER) 
    {
        sct_register_callback(pci_handle, _ssdk_av_frame_decoder_callback_func, 
            (void*)&dec_chan_info->sdk_chan_handle);
           
        sct_register_callback(
            dec_chan_info->pci_send_chan_handle,
            _ssdk_decoder_send_conf_callback_func, 
            (void*)&dec_chan_info->sdk_chan_handle);
    }
    else
    {
        sct_register_callback(pci_handle, _ssdk_av_frame_callback_func, 
            (void*)&enc_chan_info->sdk_chan_handle);

    }
}
/****************************************************************************
  This is the intermidate frame callback function which is called by the driver
  every time a new frame from a video-in port is arrived. This function will 
  queue up the frames and then calls the DVR Application provided callback.

  Parameters:

    "context"	user defined context when SDK register the callback
    "buffer"	the frame buffer that was just arrived.
    "size"		the frame buffer size.

  Returns:

    None.

  Remark: 
  
    This is an inernal function used by the SDK.

****************************************************************************/
void _ssdk_av_frame_callback_func( void * context, void * buffer, int size)
{
    sdvr_err_e err;
    sdvr_av_buffer_t *frame_buf = (sdvr_av_buffer_t*)buffer;
    sx_uint8 board_index = frame_buf->board_id;
    sx_uint8 chan_type = frame_buf->channel_type;
    sx_uint8 chan_num = frame_buf->channel_id;
    sdvr_frame_type_e frame_type = frame_buf->frame_type;
    sdvr_chan_handle_t handle = ssdk_create_chan_handle(board_index, 
                                                        chan_type,
                                                        chan_num);
    // The channel handle that was given to the driver at the time of
    // registering the callback.
    sdvr_chan_handle_t *callback_chan_handle_ptr = (sdvr_chan_handle_t*)context;
    sdvr_chan_handle_t callback_chan_handle = *callback_chan_handle_ptr;
    sx_uint8 callback_board_index = CH_GET_BOARD_ID(callback_chan_handle);
    sx_uint8 callback_chan_type = CH_GET_CHAN_TYPE(callback_chan_handle);
    sx_uint8 callback_chan_num = CH_GET_CHAN_NUM(callback_chan_handle);
    char err_msg[256];
    ssdk_yuv_frame_t *yuv_buffer = NULL;
    ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

    /********************************************************************
          While we are in the callback function we cannot perform
          any display I/O, otherwise there could be a deadlock
          between the DVR Application and SCT calls
    *********************************************************************/
    ssdvr_in_callback = 1;

    sdbg_totalFrame_count_rcvd++;

    /*******************************************************************
          NOTE: Before we access any of the PCI buffer header makes sure
                to adjust little endian to big endian for any field 
                which is larger that 1 byte.

                There needs to be extra call to this function for YUV
                frame to adjust the U and V buffers.
    ********************************************************************/
    _adjust_bug_hdr_little_to_big_endian(buffer);

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE ||
        chan_num >= _ssdvr_boards_info.boards[board_index].max_num_video_in){
        // We should never get an invalid board_index or channel number from
        // the firmware. But since we got, just throw away the frame.
        // 
        // NOTE: Since the PCI buffer seems to be corrupted or bad, we must
        //       use the channel handle that was sent by the driver to get
        //       the boad_index and chan_num before freeing the frame.

        sprintf(err_msg,"The frame header buffer is corrupted.\n\tFrame Header chan_num = %d\tchan_type = %s\n\tExpected chan_num = %d\tchan_type = %s\n",
            chan_num,dbg_chan_type_text(chan_type),callback_chan_num,dbg_chan_type_text(callback_chan_type));

        sdbg_msg_output( sdvr_dbg_err_msg,board_index,err_msg);

        ssdk_sct_buffer_free(_ssdvr_boards_info.boards[callback_board_index].recv_channels[callback_chan_num].pci_chan_handle,
                        buffer);

        ssdvr_in_callback = 0;
        return;
    }
    enc_chan_info->dbg_frame_count.totalFramesRcvd++;

    sprintf(err_msg,"\tchan_num = %d\tchan_type = %s\tframe type = %s\tframe size = %d\tts = %u %u\n",
            chan_num,dbg_chan_type_text(chan_type),dbg_frame_type_text(frame_type),frame_buf->payload_size,frame_buf->timestamp_high,frame_buf->timestamp);
    sdbg_msg_output( _ssdk_av_frame_callback_func_func,board_index,err_msg);

    /*********************************************************************
       Check if the recieved frame type is expected. If not just
       return the buffer back to the driver
    **********************************************************************/

    switch (frame_type)
    {
    case SDVR_FRAME_RAW_AUDIO:
        // NOTE: We can receive raw audio only if a raw audio is streaming
        if (!enc_chan_info->is_raw_audio_streaming)
        {
            sprintf(err_msg,"\n*** %s was ignored due to channel not expecting it.\n\tchan_num = %d\tchan_type = %s\n",
            dbg_frame_type_text(frame_type), chan_num,dbg_chan_type_text(chan_type));

            sdbg_msg_output( _ssdk_av_frame_callback_func_func,board_index,err_msg);

            ssdk_sct_buffer_free(enc_chan_info->pci_chan_handle, buffer);
            ssdvr_in_callback = 0;
            return;
        }
        break;

    case SDVR_FRAME_RAW_VIDEO:
        if (!enc_chan_info->is_raw_av_streaming[frame_buf->stream_id])
        {
            sprintf(err_msg,"\n*** %s was ignored due to channel not expecting it.\n\tchan_num = %d\tchan_type = %s\n",
            dbg_frame_type_text(frame_type), chan_num,dbg_chan_type_text(chan_type));

            sdbg_msg_output( _ssdk_av_frame_callback_func_func,board_index,err_msg);

            ssdk_sct_buffer_free(enc_chan_info->pci_chan_handle, buffer);
            ssdvr_in_callback = 0;
            return;
        }
        break;

    case SDVR_FRAME_G711:
        // NOTE: We can receive encoded audio only if an encoded video is streaming

    case SDVR_FRAME_H264_IDR:
    case SDVR_FRAME_H264_I:
    case SDVR_FRAME_H264_P:
    case SDVR_FRAME_H264_B:
    case SDVR_FRAME_H264_SPS:
    case SDVR_FRAME_H264_PPS:
    case SDVR_FRAME_JPEG:
    case SDVR_FRAME_MPEG4_I:
    case SDVR_FRAME_MPEG4_P:
    case SDVR_FRAME_MPEG4_VOL:

        if (!enc_chan_info->sub_enc_info[frame_buf->stream_id].is_enc_av_streaming)
        {
            sprintf(err_msg,"\n*** %s was ignored due to channel not expecting it.\n\tchan_num = %d\tchan_type = %s\n\tstream_id = %d\n",
            dbg_frame_type_text(frame_type), chan_num,dbg_chan_type_text(chan_type), frame_buf->stream_id);

            sdbg_msg_output( _ssdk_av_frame_callback_func_func,board_index,err_msg);

            ssdk_sct_buffer_free(enc_chan_info->pci_chan_handle, buffer);
            ssdvr_in_callback = 0;
            return;
        }
        break;
    case SDVR_FRAME_MOTION_VALUES:
        // We can get motion values frames at any time.
        break;

    default:
        // We should never get an invalid type. But since we got, just throw away
        // the frame.
        ssdk_sct_buffer_free(enc_chan_info->pci_chan_handle, buffer);

        sprintf(err_msg,"*** Frame is ignored due to invalid frame type.\n\tFrame Header chan_num = %d\tchan_type = %s\n",
            chan_num,dbg_chan_type_text(chan_type));

        sdbg_msg_output( _ssdk_av_frame_callback_func_func,board_index,err_msg);

        ssdvr_in_callback = 0;
        return;
    }

    /********************************************************************
     Check to see what type of frame we recieved. Add the frame to the
     approporiate queue of the channel.
    ********************************************************************/
    switch (frame_type)
    {
    case SDVR_FRAME_RAW_VIDEO:
    {
        /***************************************************************
          Y, U, and V buffer are concatainted together by the frimware
          with only one header. The header for YUV-frame gets
          adjust for little to big endian if neccessary
          at the start of this callback routine.
        ***************************************************************/
        dvr_data_header_t *yuvHdr = (dvr_data_header_t *)buffer;
        sx_uint8 *yBuffer = (sx_uint8 *)buffer + yuvHdr->y_data_offset;
        sx_uint8 *uBuffer = (sx_uint8 *)buffer + yuvHdr->u_data_offset;
        sx_uint8 *vBuffer = (sx_uint8 *)buffer + yuvHdr->v_data_offset;


        /********************************************************************
           Check if we recieved a valid frame buffer from the latest firmware.
        *********************************************************************/
        if (yuvHdr->signature != SDVR_DATA_HDR_SIG)
        {
            ssdk_sct_buffer_free(enc_chan_info->pci_chan_handle,  buffer);

            sprintf(err_msg,"*** Frame is ignored due to invalid frame signature.\n\tFrame Header chan_num = %d\tchan_type = %s\n\n",
                chan_num,dbg_chan_type_text(chan_type),callback_chan_num,dbg_chan_type_text(callback_chan_type));

            ssdvr_in_callback = 0;
            return;
        }

        /* 
            Raw video buffers are sent to the host as one consecutive buffer with no
            gaps between each video plane. 

            NOTE: Depending the raw video format U and V video plane maybe swapped.

            SDVR_RAWV_FORMAT_YUV_4_2_0 or       SDVR_RAWV_FORMAT_YVU_4_2_0
            DVR_RAWV_FORMAT_YUV_4_2_2

             +-----------------+                +-----------------+
             |  buffer header  |                |  buffer header  |
             +-----------------+                +-----------------+
             |  y video plane  |                |  y video plane  |
             +-----------------+                +-----------------+
             |  u video plane  |                |  v video plane  |
             +-----------------+                +-----------------+
             |  v video plane  |                |  u video plane  |
             +-----------------+                +-----------------+
        */

        enc_chan_info->dbg_frame_count.y_u_vBufferRcvd++;

        yuvHdr->y_data_size =  _ssdk_remove_yuv_padding(handle,
                              yuvHdr->padded_width,
                              yuvHdr->active_width,
                              yuvHdr->active_height,
                              yBuffer,
                              yBuffer);      

        switch (yuvHdr->yuv_format)
        {
        case DVR_RAWV_FORMAT_YUV_4_2_2:
            // TODO: This case needs to be changes once the firmware
            //       supports YUV 4.2.2
            // Currenlty YVU 4.2.2 format is handle by the SDK
            // fall throu the yuv 4.2.0 processing
        case DVR_RAWV_FORMAT_YUV_4_2_0:
            yuvHdr->u_data_size = _ssdk_remove_yuv_padding(handle,
                                  yuvHdr->uv_padded_width,
                                  yuvHdr->uv_active_width,
                                  yuvHdr->uv_active_height,
                                  (sx_uint8 *)buffer + yuvHdr->hdr_size + yuvHdr->y_data_size,      
                                  uBuffer);      
            yuvHdr->u_data_offset = yuvHdr->hdr_size + yuvHdr->y_data_size;

            yuvHdr->v_data_size = _ssdk_remove_yuv_padding(handle,
                                  yuvHdr->uv_padded_width,
                                  yuvHdr->uv_active_width,
                                  yuvHdr->uv_active_height,
                                  (sx_uint8 *)buffer + yuvHdr->hdr_size + yuvHdr->y_data_size + yuvHdr->u_data_size,      
                                  vBuffer);      
     
            yuvHdr->v_data_offset = yuvHdr->hdr_size + yuvHdr->y_data_size + yuvHdr->u_data_size;

            // TODO: Currently the SDK performs YUV 4.2.0 ot 4.2.2 converstion
            if (_ssdvr_boards_info.yuv_format == DVR_RAWV_FORMAT_YUV_4_2_2)
            {
                _ssdk_convert_YUV420_YUV422(yuvHdr);
                yuvHdr->v_data_offset = yuvHdr->hdr_size + yuvHdr->y_data_size + yuvHdr->u_data_size;
            }
            break;

        case DVR_RAWV_FORMAT_YVU_4_2_0:
            yuvHdr->v_data_size = _ssdk_remove_yuv_padding(handle,
                                  yuvHdr->uv_padded_width,
                                  yuvHdr->uv_active_width,
                                  yuvHdr->uv_active_height,
                                  (sx_uint8 *)buffer + yuvHdr->hdr_size + yuvHdr->y_data_size,      
                                  vBuffer);      
            yuvHdr->v_data_offset = yuvHdr->hdr_size + yuvHdr->y_data_size;

            yuvHdr->u_data_size = _ssdk_remove_yuv_padding(handle,
                                  yuvHdr->uv_padded_width,
                                  yuvHdr->uv_active_width,
                                  yuvHdr->uv_active_height,
                                  (sx_uint8 *)buffer + yuvHdr->hdr_size + yuvHdr->y_data_size + yuvHdr->v_data_size,      
                                  uBuffer);      
     
            yuvHdr->u_data_offset = yuvHdr->hdr_size + yuvHdr->y_data_size + yuvHdr->v_data_size;

            break;
        default:
            ssdk_sct_buffer_free(enc_chan_info->pci_chan_handle,  buffer);

            sprintf(err_msg,"*** Frame is ignored due to unsupported YUV format.\n\tFrame Header chan_num = %d\tchan_type = %s YUV_Format = %d\n\n",
                chan_num,dbg_chan_type_text(chan_type),callback_chan_num,
                dbg_chan_type_text(callback_chan_type), yuvHdr->yuv_format);

            ssdvr_in_callback = 0;
            return;

        }

        yuvHdr->data_size = yuvHdr->y_data_size + yuvHdr->u_data_size + yuvHdr->v_data_size;

        err = ssdk_queue_yuv_frame(handle, yuvHdr);
        /*add a ui callback here*/
        if ( err == SDVR_ERR_NONE )
        {
           sdvr_ui_callback    cb;
           void *   context;           
           cb = enc_chan_info->ui_cb_func;
           context = enc_chan_info->ui_cb_context;           
           
           if (cb)
            (*cb)(handle,yuvHdr,context);
        }
        /*end of ui callback change*/
        frame_type = SDVR_FRAME_RAW_VIDEO;
        break;
    }
    case SDVR_FRAME_G711:
    {
        // NOTE: For now the client side does the audio encoding. This
        //       could be changed in the future.
        //       Defaults for the audio encoder: 
        //       region (law) = ulaw 
        //       number of audio channels = 1
        //       the channel used = first one
        //       We need to read the region in the future.
        g711_t *      g711_ulaw;  // Use for North America and Japan
        g711_config_t config_ulaw = {G711_ULAW, 1, 0};
 //       g711_t *      g711_alaw;  // Use for Asia and Europe
 //       g711_config_t config_alaw = {G711_ALAW, 1, 0};
 //       g711_open(&config_alaw, &g711_alaw);

        // The payload consists of 320 samples of raw double bytes audio frame.
        // The is why we have to divide the payload_size by 2 to get the 
        // number of samples. Once we return from g711_encode(), the payload
        // contains the 8 bit encoded audio frame. 
//        frame_buf->payload_size /= 2;

        // Todo: this code must be removed once the firmware encode audio
        {
            g711_open(&config_ulaw, &g711_ulaw);
            g711_encode_stereo(g711_ulaw, frame_buf->payload_size/2, (short *)&frame_buf->payload[0], &frame_buf->payload[0]);
            g711_close(g711_ulaw);

            // Now that audio frame is encoded change its frame type
            frame_buf->frame_type = SDVR_FRAME_AUDIO_ENCODED;
            frame_type = SDVR_FRAME_AUDIO_ENCODED;

            // Before adding the audio frame to the queue, see if it is needed
            // to record to a file
            err = ssdvr_write_Enc_AFrames(handle, frame_buf);

            err =_ssdk_queue_frame(handle, frame_buf,SSDK_RECV_FRAME_TYPE_ENC_AUDIO);
        }

        break;
    }

    case SDVR_FRAME_RAW_AUDIO:
        err =_ssdk_queue_frame(handle, frame_buf,SSDK_RECV_FRAME_TYPE_RAW_AUDIO);
        break;

    case SDVR_FRAME_H264_IDR:
    case SDVR_FRAME_H264_I:
    case SDVR_FRAME_H264_P:
    case SDVR_FRAME_H264_B:
        /*
            For all the H.264 CODEC frame type except the sps & pps
            check to see if the SDK should perform start code 
            emulation. Note that the temp_venc_buf is malloced at the
            sdvr_board_connect() and will be freed when sdvr_sdk_close()
            is called.
        */
        if (_ssdvr_boards_info.boards[board_index].is_h264_SCE &&
            _ssdvr_boards_info.temp_venc_buf)
        {
            sx_uint32 nNewSize;
            /*
                 Make sure the size of temp buffer is large enough to add
                 start code emulation. The expected buffer output buffer
                 size to venc264_start_code_emulation() is 1.5 times the
                 size of encoded frame.
            */
            if (_ssdvr_boards_info.temp_venc_buf_size < frame_buf->payload_size * 1.5)
            {
                _ssdvr_boards_info.temp_venc_buf_size = frame_buf->payload_size * 2;
                free (_ssdvr_boards_info.temp_venc_buf);
                _ssdvr_boards_info.temp_venc_buf = malloc(_ssdvr_boards_info.temp_venc_buf_size);

            }
            nNewSize = venc264_start_code_emulation((unsigned char *)&frame_buf->payload, 
                                       frame_buf->payload_size, 
                                       _ssdvr_boards_info.temp_venc_buf);
            if (nNewSize != frame_buf->payload_size)
            {
                memcpy((void *)&frame_buf->payload,(void *)_ssdvr_boards_info.temp_venc_buf,nNewSize);
                frame_buf->payload_size = nNewSize;
            }
        }
        /*
           Add authentication key to each H.264 frame if one exist.
        */
        if (_ssdvr_boards_info.boards[frame_buf->board_id].auth_key_len)
        {
            venc264_authenticate_t auth_data;

            auth_data.channelID = (sx_uint16)(handle & 0xffff);
            auth_data.frameNumber = frame_buf->seq_number;
            sdvr_get_buffer_timestamp((void *)frame_buf,& auth_data.timestamp);
            frame_buf->payload_size = venc264_authenticate((unsigned char *)&frame_buf->payload, 
                                            frame_buf->payload_size, 
                                            _ssdvr_boards_info.boards[board_index].max_sct_buf_size, 
                                            &auth_data, 
                                            _ssdvr_boards_info.boards[frame_buf->board_id].auth_key, 
                                            _ssdvr_boards_info.boards[frame_buf->board_id].auth_key_len);
        }
        // Fall thru the next cases to add the video frames to the queue
    case SDVR_FRAME_H264_SPS:
    case SDVR_FRAME_H264_PPS:
    case SDVR_FRAME_MPEG4_I:
    case SDVR_FRAME_MPEG4_P:
    case SDVR_FRAME_MPEG4_VOL:
        // Before adding the video frame to the queue, see if it is needed
        // to record to a file
        err = ssdvr_write_Enc_VFrames(handle, frame_buf);

        // Todo: currently we can not write JPEG to .mov file.
    case SDVR_FRAME_JPEG:
        err = _ssdk_queue_frame(handle, frame_buf, 
            (frame_buf->stream_id == 0 ? SSDK_RECV_FRAME_TYPE_ENC_PRIMARY : SSDK_RECV_FRAME_TYPE_ENC_SECONDARY));
        break;

    case SDVR_FRAME_MOTION_VALUES:
        err =_ssdk_queue_frame(handle, frame_buf,SSDK_RECV_FRAME_TYPE_MOTION_VALUES);
        break;

    default:
        // we'll never get here since this case is handle in the previous
        // switch statement.
        break;
    }

    //////////////////////////////////////////////////////////////
    // Since there was an error to add this buffer into the queue
    // just return back the buffer to the driver
    ///////////////////////////////////////////////////////////////
    if (err != SDVR_ERR_NONE)
    {
        if (frame_type != SDVR_FRAME_RAW_VIDEO)
        {
            /********************************************************************
                The none YUV buffers will be sent to the SCT driver to release
            ********************************************************************/
            ssdk_sct_buffer_free( enc_chan_info->pci_chan_handle, buffer);
        } 
        else
        {
            /********************************************************************
                YUV frames with errors where released in the ssdk_queue_yuv_frame()
            ********************************************************************/
        }
    }
    else
    {
        /**************************************************************************
            Notify the application via the given callback function that a new
            buffer has arrived.
        **************************************************************************/
        if (_ssdvr_boards_info.av_frame_callback)
        {
            _ssdvr_boards_info.av_frame_callback(handle,
                frame_type,
                frame_buf->stream_id == 0 ? true : false);
        }
    }
    ssdvr_in_callback = 0;
} // void _ssdk_av_frame_callback_func( void * context, void * buffer, int size)


/***************************************************************************
  This is the intermidate callback function which is called by the driver
  every time a decode buffer which was sent to the FW is freed.

  Remark: This is an inernal function used by the SDK

  Parameters:

    "context"	the SDK handle of a decoder channel
    "buffer"	NULL
    "size"		0

  Returns:

    None.
****************************************************************************/
void _ssdk_decoder_send_conf_callback_func( void * context, void * buffer, int size)
{
    // The channel handle that was given to the driver at the time of
    // registering the callback.
    sdvr_chan_handle_t *callback_chan_handle = (sdvr_chan_handle_t*)context;

    sx_uint8 callback_board_index = CH_GET_BOARD_ID(*callback_chan_handle);
    sx_uint8 callback_chan_type = CH_GET_CHAN_TYPE(*callback_chan_handle);
    sx_uint8 callback_chan_num = CH_GET_CHAN_NUM(*callback_chan_handle);
    char err_msg[256];
    sdvr_err_e err;


    switch (callback_chan_type)
    {
    case SDVR_CHAN_TYPE_DECODER:
        err = ssdk_validate_chan_handle(*callback_chan_handle, SDVR_CHAN_TYPE_DECODER);

        if (err != SDVR_ERR_NONE)
        {
            // We should never get an invalid board_index or channel number from
            // the firmware. But since we got, just throw away the frame.
            sprintf(err_msg,"Invalid decoder send channel confirmation was received.\nboard_index = %d\tchan_num = %d\tchan_type = %s\n",
                callback_board_index, callback_chan_num,dbg_chan_type_text(callback_chan_type));

            sdbg_msg_output( sdvr_dbg_err_msg ,callback_board_index,err_msg);
            return;
        }

        /***********************************************************
          Update the freed send buffer counter for this decoder
        ***********************************************************/
        _ssdk_mutex_decode_buf_lock();    
        if (_ssdvr_boards_info.boards[callback_board_index].decode_channels[callback_chan_num].avail_decode_buf_count <
            m_sdkParams.dec_buf_num)
        {
            _ssdvr_boards_info.boards[callback_board_index].decode_channels[callback_chan_num].avail_decode_buf_count++;
        }
        _ssdk_mutex_decode_buf_unlock();

        break;

    case SDVR_CHAN_TYPE_OUTPUT:
        err = ssdk_validate_chan_handle(*callback_chan_handle, SDVR_CHAN_TYPE_OUTPUT);
        if (err == SDVR_ERR_NONE)
        {
            /***********************************************************
              Update the freed send buffer counter for this SMO send buffer
            ***********************************************************/
            _ssdk_mutex_decode_buf_lock();    
            if (_ssdvr_boards_info.boards[callback_board_index].smo_info[callback_chan_num].avail_send_buf_count <
                SSDK_SMO_SEND_BUF_COUNT)
            {
                _ssdvr_boards_info.boards[callback_board_index].smo_info[callback_chan_num].avail_send_buf_count++;
            }
            _ssdk_mutex_decode_buf_unlock();
//            _ssdvr_boards_info.
            break;
        }
        // else fall through the default to record the error.
    default:
        // We should never get an invalid board_index or channel number from
        // the firmware. But since we got, just throw away the frame.
        sprintf(err_msg,"Invalid send channel confirmation was received.\nboard_index = %d\tchan_num = %d\tchan_type = %d\n",
            callback_board_index, callback_chan_num,callback_chan_type);

        sdbg_msg_output( sdvr_dbg_err_msg ,callback_board_index,err_msg);
        return;
    }

    if (_ssdvr_boards_info.conf_callback)
        _ssdvr_boards_info.conf_callback(*callback_chan_handle);

}
/***************************************************************************
  This is the intermidate frame callback function which is called by the driver
  every time a new raw A/V frame from a decoder is arrived. This function will 
  queue up the frames and then calls the DVR Application provided callback.

  Remark: This is an inernal function used by the SDK

  Parameters:

    "context"	the SDK handle of a encode/decoder A/V channel
    "buffer"	the frame buffer that was just arrived.
    "size"		the frame buffer size.

  Returns:

    None.
****************************************************************************/
void _ssdk_av_frame_decoder_callback_func( void * context, void * buffer, int size)
{
    sdvr_err_e err;
    sdvr_av_buffer_t *frame_buf = (sdvr_av_buffer_t*)buffer;
    sx_uint8 board_index = frame_buf->board_id;
    sx_uint8 chan_type = frame_buf->channel_type;
    sx_uint8 chan_num = frame_buf->channel_id;
    sdvr_frame_type_e frame_type = frame_buf->frame_type;
    sdvr_chan_handle_t handle = ssdk_create_chan_handle(board_index, 
                                                        chan_type,
                                                        chan_num);
    // The channel handle that was given to the driver at the time of
    // registering the callback.
    sdvr_chan_handle_t *callback_chan_handle_ptr = (sdvr_chan_handle_t*)context;
    sdvr_chan_handle_t callback_chan_handle = *callback_chan_handle_ptr;

    sx_uint8 callback_board_index = CH_GET_BOARD_ID(callback_chan_handle);
    sx_uint8 callback_chan_type = CH_GET_CHAN_TYPE(callback_chan_handle);
    sx_uint8 callback_chan_num = CH_GET_CHAN_NUM(callback_chan_handle);
    char err_msg[256];
    ssdk_yuv_frame_t *yuv_buffer = NULL;
    ssdk_pci_decode_chan_info_t *dec_chan_info = ssdk_get_dec_chan_strct(board_index, chan_num);

    /********************************************************************
          While we are in the callback function we cannot perform
          any display I/O, otherwise there could be a deadlock
          between the DVR Application and SCT calls
    *********************************************************************/
    ssdvr_in_callback = 1;

    /*******************************************************************
          NOTE: Before we access any of the PCI buffer header makes sure
                to adjust little endian to big endian for any field 
                which is larger that 1 byte.

                There needs to be extra call to this function for YUV
                frame to adjust the U and V buffers.
    ********************************************************************/
    _adjust_bug_hdr_little_to_big_endian(buffer);

    sdbg_totalFrame_count_rcvd++;

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE ||
        chan_num >= _ssdvr_boards_info.boards[board_index].max_num_decoders){
        // We should never get an invalid board_index or channel number from
        // the firmware. But since we got, just throw away the frame.
        // 
        // NOTE: Since the PCI buffer seems to be corrupted or bad, we must
        //       use the channel handle that was sent by the driver to get
        //       the boad_index and chan_num before freeing the frame.

        sprintf(err_msg,"The frame header buffer is corrupted.\n\tFrame Header chan_num = %d\tchan_type = %s\n\tExpected chan_num = %d\tchan_type = %s\n",
            chan_num,dbg_chan_type_text(chan_type),callback_chan_num,dbg_chan_type_text(callback_chan_type));

        sdbg_msg_output( _ssdk_av_frame_decoder_callback_func_func,board_index,err_msg);

        ssdk_sct_buffer_free(dec_chan_info->pci_chan_handle,
                        buffer);

        ssdvr_in_callback = 0;
        return;
    }
    dec_chan_info->dbg_frame_count.totalFramesRcvd++;

    sprintf(err_msg,"\tchan_num = %d\tchan_type = %s\tframe type = %s\tframe size = %d\tts = %u %u\n",
            chan_num,dbg_chan_type_text(chan_type),dbg_frame_type_text(frame_type),frame_buf->payload_size,frame_buf->timestamp_high,frame_buf->timestamp);
    sdbg_msg_output( _ssdk_av_frame_decoder_callback_func_func,board_index,err_msg);

    /*********************************************************************
       Check if the recieved frame type is expected. If not just
       return the buffer back to the driver
    **********************************************************************/

    switch (frame_type)
    {
    case SDVR_FRAME_RAW_AUDIO:
        // NOTE: We can receive raw audio only if a raw video is streaming
    case SDVR_FRAME_RAW_VIDEO:
        if (!dec_chan_info->is_raw_av_streaming)
        {
            sprintf(err_msg,"\n*** %s was ignored due to channel not expecting it.\n\tchan_num = %d\tchan_type = %s\n",
            dbg_frame_type_text(frame_type), chan_num,dbg_chan_type_text(chan_type));

            sdbg_msg_output( _ssdk_av_frame_decoder_callback_func_func,board_index,err_msg);

            ssdk_sct_buffer_free(dec_chan_info->pci_chan_handle,
                            buffer);
            ssdvr_in_callback = 0;
            return;
        }
        break;

    default:
        // We should never get an invalid type. But since we got, just throw away
        // the frame.
        ssdk_sct_buffer_free(dec_chan_info->pci_chan_handle,
                        buffer);

        sprintf(err_msg,"*** Frame is ignored due to invalid frame type.\n\tFrame Header chan_num = %d\tchan_type = %s\n\tExpected chan_num = %d\tchan_type = %s\n",
            chan_num,dbg_chan_type_text(chan_type),callback_chan_num,dbg_chan_type_text(callback_chan_type));

        sdbg_msg_output( _ssdk_av_frame_decoder_callback_func_func,board_index,err_msg);

        ssdvr_in_callback = 0;
        return;
    }

    /********************************************************************
     Check to see what type of frame we recieved. Add the frame to the
     approporiate queue of the channel.
    ********************************************************************/
    switch (frame_type)
    {
    case SDVR_FRAME_RAW_VIDEO:
        {
        /***************************************************************
          Y, U, and V buffer are concatainted together by the frimware
          with only one header. The header for YUV-frame gets
          adjust for little to big endian if neccessary
          at the start of this callback routine.
        ***************************************************************/
        dvr_data_header_t *yuvHdr = (dvr_data_header_t *)buffer;
        sx_uint8 *yBuffer = (sx_uint8 *)buffer + yuvHdr->y_data_offset;
        sx_uint8 *uBuffer = ((sx_uint8 *)buffer + yuvHdr->u_data_offset);
        sx_uint8 *vBuffer = ((sx_uint8 *)buffer + yuvHdr->v_data_offset);;


        /********************************************************************
           Check if we recieved a valid frame buffer from the latest firmware.
        *********************************************************************/
        if (yuvHdr->signature != SDVR_DATA_HDR_SIG)
        {
            ssdk_sct_buffer_free(dec_chan_info->pci_chan_handle,  buffer);

            sprintf(err_msg,"*** Frame is ignored due to invalid frame signature.\n\tFrame Header chan_num = %d\tchan_type = %s\n\n",
                chan_num,dbg_chan_type_text(chan_type),callback_chan_num,dbg_chan_type_text(callback_chan_type));

            ssdvr_in_callback = 0;
            return;
        }

        /* 
            Raw video buffers are sent to the host as one consecutive buffer with no
            gaps between each video plane. 

            NOTE: Depending the raw video format U and V video plane maybe swapped.

            SDVR_RAWV_FORMAT_YUV_4_2_0 or       SDVR_RAWV_FORMAT_YVU_4_2_0
            DVR_RAWV_FORMAT_YUV_4_2_2

             +-----------------+                +-----------------+
             |  buffer header  |                |  buffer header  |
             +-----------------+                +-----------------+
             |  y video plane  |                |  y video plane  |
             +-----------------+                +-----------------+
             |  u video plane  |                |  v video plane  |
             +-----------------+                +-----------------+
             |  v video plane  |                |  u video plane  |
             +-----------------+                +-----------------+
        */

        dec_chan_info->dbg_frame_count.y_u_vBufferRcvd++;

        yuvHdr->y_data_size =  _ssdk_remove_yuv_padding(handle,
                              yuvHdr->padded_width,
                              yuvHdr->active_width,
                              yuvHdr->active_height,
                              yBuffer,
                              yBuffer);      

        switch (yuvHdr->yuv_format)
        {
        case DVR_RAWV_FORMAT_YUV_4_2_2:
            // TODO: This case needs to be changes once the firmware
            //       supports YUV 4.2.2
            // Currenlty YVU 4.2.2 format is handle by the SDK
            // fall throu the yuv 4.2.0 processing
        case DVR_RAWV_FORMAT_YUV_4_2_0:
            yuvHdr->u_data_size = _ssdk_remove_yuv_padding(handle,
                                  yuvHdr->uv_padded_width,
                                  yuvHdr->uv_active_width,
                                  yuvHdr->uv_active_height,
                                  (sx_uint8 *)buffer + yuvHdr->hdr_size + yuvHdr->y_data_size,      
                                  uBuffer);      
            yuvHdr->u_data_offset = yuvHdr->hdr_size + yuvHdr->y_data_size;

            yuvHdr->v_data_size = _ssdk_remove_yuv_padding(handle,
                                  yuvHdr->uv_padded_width,
                                  yuvHdr->uv_active_width,
                                  yuvHdr->uv_active_height,
                                  (sx_uint8 *)buffer + yuvHdr->hdr_size + yuvHdr->y_data_size + yuvHdr->u_data_size,      
                                  vBuffer);      
     
            yuvHdr->v_data_offset = yuvHdr->hdr_size + yuvHdr->y_data_size + yuvHdr->u_data_size;

            // TODO: Currently the SDK performs YUV 4.2.0 ot 4.2.2 converstion
            if (_ssdvr_boards_info.yuv_format == DVR_RAWV_FORMAT_YUV_4_2_2)
            {
                _ssdk_convert_YUV420_YUV422(yuvHdr);
                yuvHdr->v_data_offset = yuvHdr->hdr_size + yuvHdr->y_data_size + yuvHdr->u_data_size;
            }
 
            break;

        case DVR_RAWV_FORMAT_YVU_4_2_0:
            yuvHdr->v_data_size = _ssdk_remove_yuv_padding(handle,
                                  yuvHdr->uv_padded_width,
                                  yuvHdr->uv_active_width,
                                  yuvHdr->uv_active_height,
                                  (sx_uint8 *)buffer + yuvHdr->hdr_size + yuvHdr->y_data_size,      
                                  vBuffer);   
            yuvHdr->v_data_offset = yuvHdr->hdr_size + yuvHdr->y_data_size;

            yuvHdr->u_data_size = _ssdk_remove_yuv_padding(handle,
                                  yuvHdr->uv_padded_width,
                                  yuvHdr->uv_active_width,
                                  yuvHdr->uv_active_height,
                                  (sx_uint8 *)buffer + yuvHdr->hdr_size + yuvHdr->y_data_size + yuvHdr->v_data_size,      
                                  uBuffer);      
     
            yuvHdr->u_data_offset = yuvHdr->hdr_size + yuvHdr->y_data_size +
                                     yuvHdr->v_data_size;

            break;

        default:
            ssdk_sct_buffer_free(dec_chan_info->pci_chan_handle,  buffer);

            sprintf(err_msg,"*** Frame is ignored due to unsupported YUV format.\n\tFrame Header chan_num = %d\tchan_type = %s YUV_Format = %d\n\n",
                chan_num,dbg_chan_type_text(chan_type),callback_chan_num,
                dbg_chan_type_text(callback_chan_type), yuvHdr->yuv_format);

            ssdvr_in_callback = 0;
            return;

        }

        yuvHdr->data_size = yuvHdr->y_data_size + yuvHdr->u_data_size + yuvHdr->v_data_size;

        err = ssdk_queue_yuv_frame(handle, yuvHdr);
        frame_type = SDVR_FRAME_RAW_VIDEO;
        /*add a ui callback here*/
        if ( err == SDVR_ERR_NONE )
        {
           sdvr_ui_callback    cb;
           void *   context;           
           
           cb = dec_chan_info->ui_cb_func;
           context = dec_chan_info->ui_cb_context;
                      
           if (cb)
            (*cb)(handle,yuvHdr,context);
        }
        /*end of ui callback change*/
        }

        break;

    case SDVR_FRAME_RAW_AUDIO:
        err =_ssdk_queue_frame(handle, frame_buf,SSDK_RECV_FRAME_TYPE_RAW_AUDIO);
        break;

    default:
        // we'll never get here since this case is handle in the previous
        // switch statement.
        break;
    }

    //////////////////////////////////////////////////////////////
    // Since there was an error to add this buffer into the queue
    // just return back the buffer to the driver
    ///////////////////////////////////////////////////////////////
    if (err != SDVR_ERR_NONE)
    {
        if (frame_type != SDVR_FRAME_RAW_VIDEO)
        {
            /********************************************************************
                The none YUV buffers will be sent to the SCT driver to release
            ********************************************************************/
            ssdk_sct_buffer_free( dec_chan_info->pci_chan_handle,
                            buffer);
        } 
        else
        {
            /********************************************************************
                YUV frames with errors where released in the ssdk_queue_yuv_frame()
            ********************************************************************/
        }
    }
    else
    {
        /**************************************************************************
            Notify the application via the given callback function that a new
            buffer has arrived.
        **************************************************************************/
        if (_ssdvr_boards_info.av_frame_callback)
        {
            _ssdvr_boards_info.av_frame_callback(handle, frame_type,true);
        }
    }
    ssdvr_in_callback = 0;
} //void _ssdk_av_frame_decoder_callback_func( void * context, void * buffer, int size)

/****************************************************************************
  This is the intermidate message callback function that will be called by the
  driver every time a the registered message class is recieved. 

  Currently the only message class that is registered is DVR_SIG_HOST. Thru
  this message we recieve any sensor activation or alarms triggered.

  This function deteremines the type of sensor/alarm and will call the 
  corresponding user specified callback.

  Remark: This is an inernal function used by the SDK

  Parameters:

    "context"	user defined context when SDK register the callback.
                In this case the the board_index.
    "msg_class" The class of message that was recieved. Currently this
                should only be DVR_SIG_HOST.
    "buffer"	the structure corresponding to dvr_sig_host_t which will
                hold the data for the signal sent
  Returns:

    None.
****************************************************************************/

void _ssdk_message_callback_func( void * context, sx_int32  msg_class, void * buffer)
{
    dvr_sig_host_t *signal_struct = (dvr_sig_host_t*)buffer;
    sx_uint8 *board_index = (sx_uint8 *)context;
    sx_uint8 chan_type = signal_struct->job_type;
    sx_uint8 chan_num = signal_struct->job_id;
    sdvr_chan_handle_t handle = ssdk_create_chan_handle(*board_index, 
                                                        chan_type,
                                                        chan_num);


    if (msg_class != DVR_SIG_HOST)
        return;     // un-supported message class

    /********************************************************************
          While we are in the callback function we cannot perform
          any display I/O, otherwise there could be a deadlock
          between the DVR Application and SCT calls
    *********************************************************************/
    ssdvr_in_callback = 1;

    sdbg_msg_output( _ssdk_message_callback_func_func,*board_index,signal_struct);

    if ( ssdk_validate_board_ndx(*board_index) !=  SDVR_ERR_NONE){
        sdbg_msg_output_err( _ssdk_message_callback_func_func, SDVR_ERR_BOARD_NOT_CONNECTED);

        ssdvr_in_callback = 0;
        return ;        // received a message from a none-connected board.
    }

    switch (signal_struct->sig_type)
    {
    case DVR_SIGNAL_SENSOR_ACTIVATED:
        if (_ssdvr_boards_info.sensor_callback)
            _ssdvr_boards_info.sensor_callback(*board_index,signal_struct->signal_data);
        break;

    case DVR_SIGNAL_MOTION_DETECTED:
        if (_ssdvr_boards_info.video_alarm_callback)
            _ssdvr_boards_info.video_alarm_callback(handle,SDVR_VIDEO_ALARM_MOTION,
                                               signal_struct->signal_data);
        break;

    case DVR_SIGNAL_BLIND_DETECTED:
        if (_ssdvr_boards_info.video_alarm_callback)
            _ssdvr_boards_info.video_alarm_callback(handle,SDVR_VIDEO_ALARM_BLIND,
                                               signal_struct->signal_data);
        break;

    case DVR_SIGNAL_NIGHT_DETECTED:
        if (_ssdvr_boards_info.video_alarm_callback)
            _ssdvr_boards_info.video_alarm_callback(handle,SDVR_VIDEO_ALARM_NIGHT,
                                               signal_struct->signal_data);
        break;

    case DVR_SIGNAL_VIDEO_LOST:
        if (_ssdvr_boards_info.video_alarm_callback)
            _ssdvr_boards_info.video_alarm_callback(handle,SDVR_VIDEO_ALARM_LOSS,
                                               signal_struct->signal_data);
        break;
    case DVR_SIGNAL_VIDEO_DETECTED:
        if (_ssdvr_boards_info.video_alarm_callback)
            _ssdvr_boards_info.video_alarm_callback(handle,SDVR_VIDEO_ALARM_DETECTED,
                                               signal_struct->signal_data);
        break;

    case DVR_SIGNAL_RUNTIME_ERROR:
        signal_struct->sig_type = SDVR_SIGNAL_RUNTIME_ERROR;
        if (_ssdvr_boards_info.signals_callback)
            _ssdvr_boards_info.signals_callback(*board_index, (sdvr_signal_info_t *)signal_struct);
        break;

    case DVR_SIGNAL_FATAL_ERROR:
        signal_struct->sig_type = SDVR_SIGNAL_FATAL_ERROR;

        if (_ssdvr_boards_info.signals_callback)
            _ssdvr_boards_info.signals_callback(*board_index, (sdvr_signal_info_t *)signal_struct);
        break;

    case DVR_SIGNAL_HEARTBEAT:
        break;
    }
    ssdvr_in_callback = 0;
    return;
}

/****************************************************************************
  This functions gets YUV memory buffer.
  It then copies the given YUV buffer into this new memory and
  returns a pointer to the new memory.

  Remark: This is an inernal function used by the SDK

  Parameters:

    "handle"	the channel handle to get a free yuv temp. buffer from.
    "yuvHdr" the YUV frame buffer that was just arrived.

  Returns:

    A pointer to a new memory location which is filled with the given
    yuv_buffer. 
    NULL - If no memory is available.
****************************************************************************/
void *_ssdk_new_yuv_buf( sdvr_chan_handle_t handle, dvr_data_header_t *yuvHdr)
{
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    ssdk_yuv_malloc_item_t *malloc_yuv_buf;

    if ((ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE ||
         _ssdvr_boards_info.boards[board_index].recv_channels == NULL ||
         ((chan_type == SDVR_CHAN_TYPE_DECODER) && (chan_num >= _ssdvr_boards_info.boards[board_index].max_num_decoders)) ||
         ((chan_type == SDVR_CHAN_TYPE_ENCODER) && (chan_num >= _ssdvr_boards_info.boards[board_index].max_num_video_in)))
    {
        return NULL;    // Bad Handle. Internal error. It should never happen
    }

    
    malloc_yuv_buf = (ssdk_yuv_malloc_item_t*) malloc(sizeof(ssdk_yuv_malloc_item_t));
    malloc_yuv_buf->yuv_frame.signature = yuvHdr->signature;
    malloc_yuv_buf->yuv_frame.hdr_version = yuvHdr->version;
    malloc_yuv_buf->yuv_frame.hdr_size = yuvHdr->hdr_size;

    malloc_yuv_buf->yuv_frame.board_id = yuvHdr->board_id;
    malloc_yuv_buf->yuv_frame.channel_id =  yuvHdr->job_id;
    malloc_yuv_buf->yuv_frame.channel_type =   yuvHdr->job_type;
    malloc_yuv_buf->yuv_frame.frame_type =     yuvHdr->data_type;

    malloc_yuv_buf->yuv_frame.motion_detected = yuvHdr->motion_value;
    malloc_yuv_buf->yuv_frame.blind_detected =  yuvHdr->blind_value;
    malloc_yuv_buf->yuv_frame.night_detected =  yuvHdr->night_value;
    malloc_yuv_buf->yuv_frame.av_state_flags =  yuvHdr->state_flags;

    memcpy(&malloc_yuv_buf->yuv_frame.motion_value[0],
        &yuvHdr->mval[0], sizeof(malloc_yuv_buf->yuv_frame.motion_value));
    memcpy(&malloc_yuv_buf->yuv_frame.blind_value[0],
        &yuvHdr->bval[0], sizeof(malloc_yuv_buf->yuv_frame.blind_value));
    memcpy(&malloc_yuv_buf->yuv_frame.night_value[0],
        &yuvHdr->nval[0], sizeof(malloc_yuv_buf->yuv_frame.night_value));


    malloc_yuv_buf->yuv_frame.timestamp =       yuvHdr->ts;
    malloc_yuv_buf->yuv_frame.timestamp_64 = 
        (sx_uint64)yuvHdr->ts | ((sx_uint64)yuvHdr->ts_high << 32);
    malloc_yuv_buf->yuv_frame.active_height = yuvHdr->active_height;
    malloc_yuv_buf->yuv_frame.active_width = yuvHdr->active_width;

    malloc_yuv_buf->yuv_frame.seq_number = yuvHdr->seq_number;
    malloc_yuv_buf->yuv_frame.frame_number = yuvHdr->frame_number;
    malloc_yuv_buf->yuv_frame.frame_drop_count = yuvHdr->drop_count;

    malloc_yuv_buf->yuv_frame.yuv_format = _ssdvr_boards_info.yuv_format;

    malloc_yuv_buf->yuv_frame.y_data =        (sx_uint8 *)yuvHdr + yuvHdr->y_data_offset;
    malloc_yuv_buf->yuv_frame.y_data_size =   yuvHdr->y_data_size;
    malloc_yuv_buf->yuv_frame.u_data =        (sx_uint8 *)yuvHdr + yuvHdr->u_data_offset;
    malloc_yuv_buf->yuv_frame.u_data_size =   yuvHdr->u_data_size;
    malloc_yuv_buf->yuv_frame.v_data =        (sx_uint8 *)yuvHdr + yuvHdr->v_data_offset;
    malloc_yuv_buf->yuv_frame.v_data_size =   yuvHdr->v_data_size;

    malloc_yuv_buf->yuvHdr =         yuvHdr;

    malloc_yuv_buf->signiture =		   0xFEEDBEEF;

    return (void*)malloc_yuv_buf;
}
/****************************************************************************
    The hardware can receive YUV video frames more efficiently
    if each video line is divisable by 16 bytes.
    Because of this fact each line is padded to be divisible by 16
    bytes long. As result of this padding diagonal lines are displayed when
    displaying the YUV HMO videos. To resolve this problem, the SDK
    strips out all of these padding from the Y-U-V buffers which its
    video lines are not divisible by 16.


    Remark: This is an inernal function used by the SDK

    Parameters:

    handle:	the channel handle to remove the padding on its assoicated 
            YUV buffer
    buffer: the YUV frame buffer to remove the padding on each line

    Returns:
         None.

         Re-adjust the payload after the padding are removed
         Make sure to remove the extra number of lines that are
         not multiple of 16. They contain garbage.

****************************************************************************/
sx_uint32 _ssdk_remove_yuv_padding( sdvr_chan_handle_t handle,
                              sx_uint16 padded_width,
                              sx_uint16 active_width,
                              sx_uint16 active_height,
                              sx_uint8 *dest_buf,
                              sx_uint8 *src_buf)
{
    int line;
    int num_padding;
    sx_uint32 packed_buf_size;

    if (padded_width > active_width)
    {
        num_padding = padded_width - active_width;
        for (line = 0; line < active_height; line++)
        {
            memmove(dest_buf + (line * active_width) , src_buf + (line * padded_width),active_width);
        }

    }

//    frame_buf->payload_size = frame_buf->active_height * frame_buf->active_width;
    packed_buf_size = active_height * active_width;
    return packed_buf_size;

}

/****************************************************************************
  This functions frees each one of Y-U-V buffers of YUV frame. The then
  returns the buffer to the poll of YUV buffer so that they can be used 
  for the future YUV frames.

  Remark: This is an inernal function used by the SDK

  Parameters:

    "handle"	the channel handle to get a free yuv temp. buffer from.
    "yuv_buffer" the YUV frame buffer that was obtained by calling
    sdvr_get_yuv_buffer().

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e _ssdk_delete_yuv_buf( ssdk_yuv_malloc_item_t *yuv_buffer)
{
    sx_uint8 board_index;
    sx_uint8 chan_type;
    sx_uint8 chan_num;
    char msg[256];
    sdvr_chan_handle_t handle;
    sct_channel_t pci_chan_handle;
    ssdk_dbg_frame_count_t *dbg_frame_count;
    ssdk_pci_channel_info_t *enc_chan_info;

    /****************************************************************
            0 - Validate the given frame buffer.
    ******************************************************************/
    if (yuv_buffer->signiture != 0xFEEDBEEF)
    {
        sprintf(msg,"\n_ssdk_delete_yuv_buf(): YUV buffer does not blong to the SDK.");
        sdbg_msg_output(sdvr_dbg_err_msg,0,msg);

        return SDVR_ERR_INVALID_ARG;
    }

    board_index = yuv_buffer->yuv_frame.board_id;
    chan_num = yuv_buffer->yuv_frame.channel_id;
    chan_type = yuv_buffer->yuv_frame.channel_type;
    handle = ssdk_create_chan_handle(board_index, chan_type, chan_num);
    enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

    if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE)
    {
        sprintf(msg,"\n_ssdk_delete_yuv_buf(): The channel handle corresponding to the YUV buffer is invalid.");
        sdbg_msg_output(sdvr_dbg_err_msg,0,msg);

        return SDVR_ERR_INVALID_ARG;    // Bad Handle.
    }
 
    if (chan_type == SDVR_CHAN_TYPE_DECODER)
    {
        pci_chan_handle = _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pci_chan_handle;
        dbg_frame_count = &_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].dbg_frame_count;
    }
    else
    {
        pci_chan_handle = enc_chan_info->pci_chan_handle;
        dbg_frame_count = &enc_chan_info->dbg_frame_count;
    }

    /****************************************************************
    1 - first we must release the sct buffer
    for the corresponding channel type by just calling the SCT driver.
    ******************************************************************/
    ssdk_sct_buffer_free(pci_chan_handle,  yuv_buffer->yuvHdr);

    dbg_frame_count->yuvFrameFreed++;


    /****************************************************************
             2 - free the YUV container frame.
    ******************************************************************/
    free (yuv_buffer);

    return SDVR_ERR_NONE;
}
char *_ssdk_rec_frame_type_text(ssdk_recv_frame_type_e recv_frame_type)
{
    static char *recv_frame_type_text;
    switch (recv_frame_type)
    {
    case SSDK_RECV_FRAME_TYPE_ENC_PRIMARY:
        recv_frame_type_text = "SSDK_RECV_FRAME_TYPE_ENC_PRIMARY";
        break;

    case SSDK_RECV_FRAME_TYPE_ENC_SECONDARY:
        recv_frame_type_text = "SSDK_RECV_FRAME_TYPE_ENC_SECONDARY";
        break;

    case SSDK_RECV_FRAME_TYPE_RAW_VIDEO:
        recv_frame_type_text = "SSDK_RECV_FRAME_TYPE_RAW_VIDEO";
        break;

    case SSDK_RECV_FRAME_TYPE_RAW_AUDIO:
        recv_frame_type_text = "SSDK_RECV_FRAME_TYPE_RAW_AUDIO";
        break;

    case SSDK_RECV_FRAME_TYPE_ENC_AUDIO:
        recv_frame_type_text = "SSDK_RECV_FRAME_TYPE_ENC_AUDIO";
        break;

    case SSDK_RECV_FRAME_TYPE_MOTION_VALUES:
        recv_frame_type_text = "SSDK_RECV_FRAME_TYPE_MOTION_VALUES";
        break;

    default:
        recv_frame_type_text = "INVALID_FRAME_TYPE";
    }

    return recv_frame_type_text;
}

/****************************************************************************
  This function return a pointer to frame queue for the corresponding
  frame type.

  Remark: This is an inernal function used by the SDK

  Parameters:

    "handle":           The channel handle to get its frame queue

    "recv_chan_type":   The type frame queue. (i.e. video encode, raw, etc)

    "is_raw_video":     It indicates if the queue corresponds to a raw video

  Returns:

    NULL - The given frame type is not supported
    Otherwise a pointer to the frame queue.
****************************************************************************/
ssdk_frame_queue_t *_ssdk_get_frame_queue(sdvr_chan_handle_t handle, ssdk_recv_frame_type_e recv_frame_type, sx_bool *is_raw_video)
{
    ssdk_frame_queue_t *queue_buffer = NULL;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

    /************************************************************************
        NOTE: For powerpc or other linux OS, it is possible that when we
            get to this point, the channel just got stopped. That is why
            we check for the streaming in here and if the channel is not 
            streaming the approporiate frame type, we return an error.

        NOTE2: Audio (raw or encoded) can only be streamed if video is
               streaming.

        NOTE3: Motion values can be recieved if Video (raw or encoded)
              is streaming.
    ************************************************************************/
    switch (recv_frame_type)
    {
    case SSDK_RECV_FRAME_TYPE_ENC_PRIMARY:
        if (enc_chan_info->sub_enc_info[0].is_enc_av_streaming)
            queue_buffer = &enc_chan_info->enc_av_frame_queue[0];
        break;

    case SSDK_RECV_FRAME_TYPE_ENC_SECONDARY:
        if (enc_chan_info->sub_enc_info[1].is_enc_av_streaming)
            queue_buffer = &enc_chan_info->enc_av_frame_queue[1];

        break;

    case SSDK_RECV_FRAME_TYPE_RAW_VIDEO:
        if (chan_type == SDVR_CHAN_TYPE_DECODER)
        {
            if (_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].is_raw_av_streaming)
                queue_buffer = &_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue;
        }
        else
        {
            if (enc_chan_info->is_raw_av_streaming)
                queue_buffer = &enc_chan_info->yuv_frame_queue;
        }
        *is_raw_video = 1;
        break;

    case SSDK_RECV_FRAME_TYPE_RAW_AUDIO:
        if (chan_type == SDVR_CHAN_TYPE_DECODER)
        {
            if (_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].is_raw_av_streaming)

                queue_buffer = &_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue;
        }
        else
        {
            if (enc_chan_info->is_raw_av_streaming)
                queue_buffer = &enc_chan_info->raw_audio_frame_queue;
        }
        break;

    case SSDK_RECV_FRAME_TYPE_ENC_AUDIO:
        if (enc_chan_info->sub_enc_info[0].is_enc_av_streaming ||
           (enc_chan_info->sub_enc_info[0].is_enc_av_streaming))
            queue_buffer = &enc_chan_info->enc_audio_frame_queue;
        break;

    case SSDK_RECV_FRAME_TYPE_MOTION_VALUES:
        if (enc_chan_info->sub_enc_info[0].is_enc_av_streaming ||
           enc_chan_info->sub_enc_info[0].is_enc_av_streaming ||
           enc_chan_info->is_raw_av_streaming)
            queue_buffer = &enc_chan_info->motion_values_frame_queue;
        break;

    default:
        return NULL;
    }

    return queue_buffer;
}

/****************************************************************************
  This function inserts the given buffer to the corresponding queue of
  receive channel.

  Remark: This is an inernal function used by the SDK

  Parameters:

    "handle"	the channel handle to add this buffer to its queue
    "buffer"	the frame buffer that was just arrived.
    "recv_chan_type" the type frame buffer. (i.e. video encode, raw, etc)/

  Returns:

    SDVR_ERR_NONE - success

    SDVR_ERR_INTERNAL - Failed to allocate memory.

    SDVR_ERR_INVALID_FRAME_TYPE - The given frame type is not supported
****************************************************************************/
sdvr_err_e _ssdk_queue_frame( sdvr_chan_handle_t handle, void * buffer, ssdk_recv_frame_type_e recv_frame_type)
{
    sdvr_av_buffer_t *frame_buf = (sdvr_av_buffer_t*)buffer;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sdvr_frame_type_e frame_type = frame_buf->frame_type;
    ssdk_frame_queue_t *queue_buffer;
    char msg[256];
    sx_bool is_raw_video = 0;
    sct_channel_t pci_chan_handle;
    ssdk_dbg_frame_count_t *dbg_frame_count;
    int temp_top_ndx;
    ssdk_pci_channel_info_t *enc_chan_info;

    _ssdk_mutex_lock();

    if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE)
    {
        sdbg_msg_output(_ssdk_queue_frame_func,board_index,"\n_ssdk_queue_frame: NO BUFFERS WAS ALLOCATED TO RECEIVE FRAMES\n");
        _ssdk_mutex_unlock();
        return SDVR_ERR_INTERNAL;    // Bad Handle. Internal error. It should never happen
    }

    enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

    if (chan_type == SDVR_CHAN_TYPE_DECODER)
    {
        pci_chan_handle = _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pci_chan_handle;
        dbg_frame_count = &_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].dbg_frame_count;
    }
    else
    {
        pci_chan_handle = enc_chan_info->pci_chan_handle;
        dbg_frame_count = &enc_chan_info->dbg_frame_count;
    }
    /************************************************************
        1. Find the corresponding queue for this frame type.
           If the channel is not streaming the frame type, 
           quue_buffer is also NULL.
    *************************************************************/
    if (!(queue_buffer = _ssdk_get_frame_queue(handle, recv_frame_type, &is_raw_video)))
    {
        sprintf(msg,"\n_ssdk_queue_frame: INVALID only unexpected FRAME TYPE = %d\n",recv_frame_type);
        sdbg_msg_output(_ssdk_queue_frame_func,board_index,msg);

        _ssdk_mutex_unlock();
        return SDVR_ERR_INVALID_FRAME_TYPE;

    }

    /************************************************************
        2. Check if the frame queue for the given type was
        allocated before
    *************************************************************/

    if (queue_buffer->frame_queue == NULL)
    {
        sprintf(msg,"\n_ssdk_queue_frame: NO FRAME QUEUE FOR [%s] WAS ALLOCATED.\n",    
            _ssdk_rec_frame_type_text(recv_frame_type));
        sdbg_msg_output(_ssdk_queue_frame_func,board_index,msg);

        _ssdk_mutex_unlock();
        return SDVR_ERR_INTERNAL;
    }


/******************************************************************************
 * 3. L O G I C   O F   A D D I N G   B U F F E R S   T O   T H E   Q U E U E.*
 ******************************************************************************

    If the queue item at the top of the queue has a buffer that was not  
    recieved by the app.
    a)  return the frame buffer back to the driver.
    b) if the queue item location that we just freed was also the start index
       move the starting index to the next item in the queu

    Add each new frame at the top of the queue and adjust the top of the queue 
    index.
    a) Mark the item at the top of the queue as not available
    b) Save the frame into this location
    c) Go to the next item after the item at the top of the queue
       NOTE: since this queue is setup circular. The next item of the
       last item in the queue will wrap arround.

    At last increment the number of frames received. 
    **************************************************************************/

    // return back the frame to the driver, if the top of the queue is 
    // already has a frame. This can happen due to wrapping arround.

    if (!queue_buffer->frame_queue[queue_buffer->top_item].avail)
    {     
        if (is_raw_video)
        {
            ssdk_yuv_malloc_item_t *yuv_buffer;

            yuv_buffer = (ssdk_yuv_malloc_item_t *)queue_buffer->frame_queue[queue_buffer->top_item].data;
            _ssdk_delete_yuv_buf( yuv_buffer);
        }
        else
            ssdk_sct_buffer_free(pci_chan_handle,
                        queue_buffer->frame_queue[queue_buffer->top_item].data);
        if (queue_buffer->top_item == queue_buffer->start_item)
            queue_buffer->start_item = queue_buffer->frame_queue[queue_buffer->start_item].next;
    }

    // Add the new frame
    queue_buffer->frame_queue[queue_buffer->top_item].avail = 0;
    queue_buffer->frame_queue[queue_buffer->top_item].data = buffer;
    temp_top_ndx = queue_buffer->frame_queue[queue_buffer->top_item].next;

    queue_buffer->top_item = temp_top_ndx;

    if (is_raw_video)
       dbg_frame_count->yuvFrameRcvd++;
    else
       dbg_frame_count->eFrameRcvd++;

    // Adjust the number of frames recieved
    queue_buffer->num_recvd_frames++;
    if (queue_buffer->num_recvd_frames > queue_buffer->size)
    {
        queue_buffer->num_recvd_frames = queue_buffer->size;
    } 
    _ssdk_mutex_unlock();

    sprintf(msg,"\n_ssdk_queue_frame: Added frame type %s\n\tStart_ndx = %d Top_ndx = %d  # of Frames = %d\n\tMD = %d BD = %d ND = %d ts = %u %u\n",
        _ssdk_rec_frame_type_text(recv_frame_type),
        queue_buffer->start_item,
        queue_buffer->top_item,
        queue_buffer->num_recvd_frames,
        frame_buf->motion_detected,
        frame_buf->blind_detected,
        frame_buf->night_detected,
        frame_buf->timestamp_high,
        frame_buf->timestamp);

    sdbg_msg_output(_ssdk_queue_frame_func,board_index,msg);
    return SDVR_ERR_NONE;
}
/****************************************************************************
  This function is used to release the given frame buffer to the SDK. 
  It gets the frame type from frame header before releasing the 
  buffer.

  Parameters:

    "frame_buffer" - The pointer to a buffer to be released. The buffer
    must have been obtained by calling either sdvr_get_yuv_buffer() or
    sdvr_get_av_buffer();

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e ssdk_release_frame(void *frame_buf, sx_bool is_encoded_frame)
{
    sdvr_err_e err;
    sx_uint8 board_index;
    sx_uint8 chan_num;
    sdvr_chan_type_e chan_type;
    sdvr_av_buffer_t *frame;
    sdvr_chan_handle_t handle;
    ssdk_pci_channel_info_t *enc_chan_info;
    /*********************************************************************
             Get a pointer to frame buffer depending on its type
             and then extract the board_index and channel number
    *********************************************************************/
    _ssdk_mutex_lock();
    
    frame = (sdvr_av_buffer_t *)frame_buf;
    board_index = frame->board_id;
    chan_num = frame->channel_id;
    chan_type = frame->channel_type;
    handle = ssdk_create_chan_handle(board_index, chan_type, chan_num);
    enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);
    /*********************************************************************
                        Validate the frame buffer
    *********************************************************************/

    if (ssdk_validate_chan_handle(handle , -1) != SDVR_ERR_NONE)
    {
        _ssdk_mutex_unlock();
        return SDVR_ERR_INVALID_HANDLE;    // Bad Handle.
    }


    if (frame->frame_type != SDVR_FRAME_RAW_VIDEO)
    {
        /********************************************************************
            The none YUV buffers will be sent to the SCT driver to release
        ********************************************************************/
        if (chan_type == SDVR_CHAN_TYPE_DECODER)
        {
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].dbg_frame_count.eFrameFreed++;
            ssdk_sct_buffer_free( _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pci_chan_handle,
                             frame_buf);

        }
        else
        {
            enc_chan_info->dbg_frame_count.eFrameFreed++;
            ssdk_sct_buffer_free( enc_chan_info->pci_chan_handle,
                             frame_buf);
        }
        err = SDVR_ERR_NONE;
    } 
    else
    {
        /********************************************************************
            Since YUV frames contains three buffers we have to release
            each one of them one by one and then release the buffer.
        ********************************************************************/
        err =  _ssdk_delete_yuv_buf( (ssdk_yuv_malloc_item_t *)frame_buf);
    }

    _ssdk_mutex_unlock();
    return err;
}
/****************************************************************************
  This function checks to see if there is any frame requested type is available
  on the given encoding channel. If there is any frame queued, the frame 
  buffer will be removed and returned to the caller.

  Remark: This is an inernal function used by the SDK

  Parameters:

    "handle"	the channel handle to retrieve a frame from its queue
    "buffer"	the frame buffer that was just arrived.
    "recv_frame_type" the type frame buffer. (i.e. video encode, raw, etc)/

  Returns:

    SDVR_ERR_NONE - success

    SDVR_ERR_INVALID_HANDLE - The given channel handle is invalid

    SDVR_ERR_INVALID_FRAME_TYPE - The given frame type is not supported

    SDVR_ERR_NOBUF - No A/V buffer is available. 
****************************************************************************/
sdvr_err_e ssdk_dequeue_frame( sdvr_chan_handle_t handle, void **buffer, 
                                ssdk_recv_frame_type_e recv_frame_type)
{
    sx_uint8 board_index;
    sx_uint8 chan_type;
    sx_uint8 chan_num;
    ssdk_frame_queue_t *queue_buffer;
    sx_bool is_raw_video;
    char msg[256];

    _ssdk_mutex_lock();

    board_index = CH_GET_BOARD_ID(handle);
    chan_type = CH_GET_CHAN_TYPE(handle);
    chan_num = CH_GET_CHAN_NUM(handle);

    if (ssdk_validate_chan_handle(handle, -1) != SDVR_ERR_NONE)
    {
        sprintf(msg,"\n_ssdk_dequeue_frame: INVALID CHANNEL HANDLE = 0x%x\n", handle);
        sdbg_msg_output(_ssdk_dequeue_frame_func,board_index,msg);

        _ssdk_mutex_unlock();
        return SDVR_ERR_INVALID_HANDLE;    // Bad Handle.
    }

    /************************************************************
        1. Find the corresponding queue for this frame type
    *************************************************************/
    if (!(queue_buffer = _ssdk_get_frame_queue(handle, recv_frame_type, &is_raw_video)))
    {
        sprintf(msg,"\n_ssdk_queue_frame: INVALID FRAME TYPE = %d\n",recv_frame_type);
        sdbg_msg_output(_ssdk_queue_frame_func,board_index,msg);

        _ssdk_mutex_unlock();
        return SDVR_ERR_INVALID_FRAME_TYPE;

    }

    /************************************************************
        2. Check if the frame queue is empty.
    *************************************************************/

    if (queue_buffer->frame_queue == NULL || 
        queue_buffer->num_recvd_frames == 0)
    {
        queue_buffer->start_item = 0;
        queue_buffer->top_item = 0;
        queue_buffer->num_recvd_frames = 0;
        sprintf(msg,"\n_ssdk_dequeue_frame: FRAME QUEUE IS EMPTY.\n");
        sdbg_msg_output(_ssdk_dequeue_frame_func,board_index,msg);

        _ssdk_mutex_unlock();  
        return SDVR_ERR_NOBUF;
    }

    /****************************************************************************
        3.  L O G I C   O F   R E M O V I N G   F R A M E S  
                    F R O M   T H E   Q U E U E.
    ****************************************************************************/

    /***************************************************************************
        3.a) Get the frame for current position of queue and mark it available.
    ***************************************************************************/
    *buffer = queue_buffer->frame_queue[queue_buffer->start_item].data;
    queue_buffer->frame_queue[queue_buffer->start_item].data = NULL;
    queue_buffer->frame_queue[queue_buffer->start_item].avail = 1;

    queue_buffer->num_recvd_frames--;

    sprintf(msg,"\n_ssdk_dequeue_frame: Removed frame type %s\n\tStart_ndx = %d New Start_ndx = %d Top_ndx = %d  # of Frames = %d",
        _ssdk_rec_frame_type_text(recv_frame_type), 
        queue_buffer->start_item,
        queue_buffer->frame_queue[queue_buffer->start_item].next,
        queue_buffer->top_item,
        queue_buffer->num_recvd_frames);

    queue_buffer->start_item = queue_buffer->frame_queue[queue_buffer->start_item].next;

    sdbg_msg_output(_ssdk_dequeue_frame_func,board_index,msg);

    _ssdk_mutex_unlock();  
    return SDVR_ERR_NONE;
}
/****************************************************************************
  This function returns the board configuration parameters, i.e., the
  number of cameras, number of sensors and relays, and so on.

  There is no corresponding function to set these parameters
  because the configuration information returned by this function cannot
  be changed. The configuration information essentially is a description 
  of the capabilities of the board.

  Parameters:

    "board_index" - The index of the board for which you want this information.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "board_config" - A pointer to a configuration data structure that 
    is filled appropriately when this function returns.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise see the error code list.

  Remarks:

    It is not necessary to connect to the board to call this function.
****************************************************************************/
sdvr_err_e sdvr_get_board_config(sx_uint32 board_index, 
                                 sdvr_board_config_t *board_config)
{
    sdvr_err_e err;
    dvr_board_t cmd_strct;
    int temp_connect = 0;

    if (board_index >= _ssdvr_boards_info.board_num)
    {
        err = SDVR_ERR_INVALID_BOARD;
        sdbg_msg_output_err(sdvr_get_board_attributes_func,err);
        return err;
    }

    if (board_config == NULL)
        return SDVR_ERR_INVALID_ARG;

    /**************************************************************
     Get the board handle. 
     NOTE: DVR_GET_BOARD command is the only command that the firmware
           accepts without connecting to it by setting the board_index
           and video standard. 
     If the DVR Application did not connect to this board, first connect 
     to the board temporarily,  before getting the board configuration
     and then disconnect from it.
    **************************************************************/
    if (_ssdvr_boards_info.boards[board_index].handle == NULL) {
        temp_connect = 1;
        if ((err = sct_open_board(board_index,&_ssdvr_boards_info.boards[board_index].handle)) != SDVR_ERR_NONE) 
        {
            return err;
        }
    }

    memset((void *)&cmd_strct,0,sizeof(cmd_strct));
    ssdk_message_board_send(board_index, DVR_GET_BOARD, &cmd_strct, sizeof(cmd_strct));
    ssdk_message_board_recv(board_index, DVR_REP_BOARD, &cmd_strct, 0, 0);

    if (cmd_strct.status == DVR_STATUS_OK )
    {
        sdvr_firmware_ver_t version_info;

        board_config->has_smo = (cmd_strct.num_smos > 0);
        board_config->num_smos = cmd_strct.num_smos;
        board_config->num_encoders_supported =
        board_config->num_cameras_supported = cmd_strct.num_cameras;

        board_config->num_microphones_supported = cmd_strct.u1.audio_supported;
        board_config->num_decoders_supported = cmd_strct.num_decoders;

        board_config->num_relays = cmd_strct.num_relays;
        board_config->num_sensors = cmd_strct.num_sensors;
        board_config->camera_type = cmd_strct.camera_type;

        sdvr_get_firmware_version_ex(board_index, &version_info);

        // backward compatibility fix.
        if (version_info.fw_major <= 3 && version_info.fw_build < 13)
        {
            board_config->num_microphones_supported = cmd_strct.num_cameras;
            board_config->num_decoders_supported = cmd_strct.num_cameras;
        }

        // Only FW versions 4.1.0.8 and above support encoders count        
        if (ssdk_is_min_version(version_info.fw_major,version_info.fw_minor,version_info.fw_revision,version_info.fw_build,
            4,1,0,8))
        {
            board_config->num_encoders_supported = cmd_strct.num_encoders;
        }


    }

    // close the board if we opened it temporarily
    if (temp_connect)
    {
        sct_close_board(_ssdvr_boards_info.boards[board_index].handle);
        _ssdvr_boards_info.boards[board_index].handle = NULL;
    }

    sdbg_msg_output(sdvr_get_board_config_func, board_index, (void *)&cmd_strct);
    return cmd_strct.status;
}
/****************************************************************************
  This function sets the board configuration parameters.
  
  Note this is only a internal routine to SDK. 

  Parameters:

    "board_index" - The board index ID to be set for this board
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "board_settings" -  various board settings

  Returns:

   SDVR_ERR_NONE - success

   SDVR_ERR_BOARD_NOT_CONNECTED - Error code if the board was never connected to.

   SDVR_ERR_INVALID_ARG - Error code if the given board index does not exist.

****************************************************************************/
sdvr_err_e ssdk_set_board_config(sx_uint32 board_index, 
                                 sdvr_board_settings_t *board_settings)
{
    sdvr_err_e err;
    dvr_board_t cmd_strct;

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE){
        return err;
    }

    memset(&cmd_strct,0,sizeof(cmd_strct));
    cmd_strct.board_id = board_index;
    cmd_strct.camera_type = board_settings->video_std;
    cmd_strct.u1.h264_skip_SCE = board_settings->is_h264_SCE;
    cmd_strct.audio_rate = board_settings->audio_rate;
    
    ssdk_message_board_send(board_index, DVR_SET_BOARD, &cmd_strct, sizeof(cmd_strct));
    ssdk_message_board_recv(board_index, DVR_REP_BOARD, &cmd_strct, 0, 0);

    if (cmd_strct.status == SDVR_ERR_NONE) {
        _ssdvr_boards_info.boards[board_index].video_std = board_settings->video_std;
        ssdk_get_width_hieght_vstd(board_settings->video_std,
                            &_ssdvr_boards_info.boards[board_index].vstd_width,
                            &_ssdvr_boards_info.boards[board_index].vstd_lines);
    }
    sdbg_msg_output(DVR_SET_BOARD, board_index, (void *)&cmd_strct);

    return cmd_strct.status;
}

/****************************************************************************
  This function enables or disables the watchdog timer. By default each board is 
  equipped with a watchdog timer to expire in 10 second once it is enabled. 
  When the watchdog timer expires, the board is reset and a reset
  signal is issued from the board. If this reset signal is tied to the 
  reset signal on the PC's motherboard, the entire PC is reset and
  rebooted. 

  To change the default watchdog timer period call sdvr_set_watchdog_state_ex().

  @NOTE: When the watchdog timer expires, data on the way to the disk 
  maybe lost and the system may be left in an inconsistent state. Therefore, 
  use this function with caution and make sure to re-boot the PC before
  loading the firmware to it.@

  If you are going to use this function, to guard against the system hanging
  indefinitely, then set the watchdog timer on all boards in your system. 

  Parameters:

    "board_index" - Index of the board on which to set the watchdog timer.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "enable" - Enable or disable the watchdog timer.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise see the error code list.

  Remark:

    Whatever watchdog timer is set at 10 seconds, it is recommended to call
    sdvr_set_watchdog_state() every 5 seconds to prevent the timer from
    expiring.
****************************************************************************/
sdvr_err_e sdvr_set_watchdog_state(sx_uint32 board_index, 
                                   sx_uint32 enable)
{
    dvr_watchdog_t cmd_strct;
    sdvr_err_e err;
    int cmd_strct_size = sizeof(cmd_strct);

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE){
        sdbg_msg_output_err(sdvr_set_watchdog_state_func, err);
        return err;
    }

    cmd_strct.enable = enable;
    cmd_strct.msec   = 0;
    ssdk_message_board_send(board_index, DVR_SET_WATCHDOG, &cmd_strct, cmd_strct_size);
    ssdk_message_board_recv(board_index, DVR_REP_WATCHDOG, &cmd_strct, 0, 0);


    sdbg_msg_output(sdvr_set_watchdog_state_func, board_index, (void *)&cmd_strct);
    return cmd_strct.status;
}

/*******************************************************************
  This function sets the state of the watchdog timer as well as the 
  timer interval. See sdvr_set_watchdog_state() for detailed 
  description.

  Parameters:

    "board_index" - Index of the board on which to set the watchdog timer.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "enable" – Set to true to enable the watchdog timer, 
    false to disable it. Once enabled, the timer must
    be refreshed at intervals not exceeding the specified timeout.

    "msec" - Specifies the watchdog timer duration in milliseconds. 
    The valid range is 1-10,000. If this field is zero and the enable 
    flag is set then the timeout period defaults to 10,000 milliseconds (10 seconds). 
    This field is ignored if "enable" is false.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise see the error code list.

  Remark:

    Since watchdog timer is set at 10 seconds, it is recommended to call
    sdvr_set_watchdog_state_ex() every 5 seconds to prevent the timer from
    expiring.
*******************************************************************/
sdvr_err_e
sdvr_set_watchdog_state_ex(sx_uint32 board_index, sx_bool enable,
                           sx_uint32 msec)
{
    dvr_watchdog_t cmd_strct;
    sdvr_err_e err;
    int cmd_strct_size = sizeof(cmd_strct);

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE){
        sdbg_msg_output_err(sdvr_set_watchdog_state_func, err);
        return err;
    }

    cmd_strct.enable = enable;
    cmd_strct.msec = msec;
    ssdk_message_board_send(board_index, DVR_SET_WATCHDOG, &cmd_strct, cmd_strct_size);
    ssdk_message_board_recv(board_index, DVR_REP_WATCHDOG, &cmd_strct, 0, 0);


    sdbg_msg_output(sdvr_set_watchdog_state_func, board_index, (void *)&cmd_strct);
    return cmd_strct.status;
}

/****************************************************************************
  This function returns the current enable state of the watchdog timer. 

  Parameters:

    "board_index" - The index of the board whose watchdog timer value you 
    want to get. This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.


    "enable" -  A pointer to a variable that will have the enable state
    of watchdog timer on success return.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise see the error code list.
****************************************************************************/
sdvr_err_e sdvr_get_watchdog_state(sx_uint32 board_index, 
                                   sx_uint32 *enable)
{
    dvr_watchdog_t cmd_strct;
    sdvr_err_e err;
    int cmd_strct_size = sizeof(cmd_strct);

    if (enable == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else 
        err = ssdk_validate_board_ndx(board_index);

    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_get_watchdog_state_func, err);
        return err;
    }


    ssdk_message_board_send(board_index, DVR_GET_WATCHDOG, &cmd_strct, cmd_strct_size);
    ssdk_message_board_recv(board_index, DVR_REP_WATCHDOG, &cmd_strct, 0, 0);

    if (cmd_strct.status == SDVR_ERR_NONE) {
        *enable = cmd_strct.enable;
    }

    sdbg_msg_output(sdvr_get_watchdog_state_func, board_index, (void *)&cmd_strct);

    return cmd_strct.status;
}



/****************************************************************************
  This function returns the current video standard for a particular board. 

  Parameters:

    "board_index" - The index of the board.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "video_std_type" - A pointer to a variable that will contain the video 
    standard when this function returns. 

  Returns:

    SDVR_ERR_NONE - On success. Otherwise see the error code list.

  Remarks:

    All boards in a system must have the same video standard. 
****************************************************************************/
sdvr_err_e sdvr_get_video_standard(sx_uint32 board_index, 
                                   sdvr_video_std_e *video_std_type)
{
    sdvr_err_e err;
    dvr_board_t cmd_strct;

    if (video_std_type == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else
        err = ssdk_validate_board_ndx(board_index);

    if (err !=  SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_get_video_standard_func, err);
        return err;
    }

    ssdk_message_board_send(board_index, DVR_GET_BOARD, &cmd_strct, sizeof(cmd_strct));
    ssdk_message_board_recv(board_index, DVR_REP_BOARD, &cmd_strct, 0, 0);

    if (cmd_strct.status == DVR_STATUS_OK )
    {
        *video_std_type = cmd_strct.camera_type;
    }

    sdbg_msg_output(sdvr_get_video_standard_func, board_index, (void *)&cmd_strct);

    return cmd_strct.status;
}

/****************************************************************************
  This function returns the text string mnemonic for the given error number. If the
  given error number is not valid, a string with the number is returned.

  Parameters:

    "error_no" - The error number for the text string
 

  Returns:

    A pointer to a null terminated string with the text of the error.
    @NOTE: This string should not be freed.@

****************************************************************************/
char *sdvr_get_error_text(sdvr_err_e error_no)
{
    static char err_buf[120];

    switch (error_no) {
    case SDVR_ERR_NONE:                     return "SDVR_ERR_NONE";

    case SDVR_FRMW_ERR_WRONG_CAMERA_NUMBER:         return "SDVR_FRMW_ERR_WRONG_CAMERA_NUMBER";
    case SDVR_FRMW_ERR_WRONG_CAMERA_TYPE:           return "SDVR_FRMW_ERR_WRONG_CAMERA_TYPE";
    case SDVR_FRMW_ERR_WRONG_CODEC_FORMAT:          return "SDVR_FRMW_ERR_WRONG_CODEC_FORMAT";
    case SDVR_FRMW_ERR_WRONG_CODEC_RESOLUTION:      return "SDVR_FRMW_ERR_WRONG_CODEC_RESOLUTION";
    case SDVR_FRMW_ERR_WRONG_CHANNEL_TYPE:          return "SDVR_FRMW_ERR_WRONG_CHANNEL_TYPE";
    case SDVR_FRMW_ERR_WRONG_CHANNEL_ID:            return "SDVR_FRMW_ERR_WRONG_CHANNEL_ID";
    case SDVR_FRMW_ERR_WRONG_VIDEO_FORMAT:          return "SDVR_FRMW_ERR_WRONG_VIDEO_FORMAT";
    case SDVR_FRMW_ERR_WRONG_AUDIO_FORMAT:          return "SDVR_FRMW_ERR_WRONG_AUDIO_FORMAT";
    case SDVR_FRMW_ERR_EXCEED_CPU_LIMIT:            return "SDVR_FRMW_ERR_EXCEED_CPU_LIMIT";
    case SDVR_FRMW_ERR_CHANNEL_NOT_CREATED:         return "SDVR_FRMW_ERR_CHANNEL_NOT_CREATED";
    case SDVR_FRMW_ERR_CHANNEL_ALREADY_CREATED:     return "SDVR_FRMW_ERR_CHANNEL_ALREADY_CREATED";
    case SDVR_FRMW_ERR_CHANNEL_NOT_ENABLED:         return "SDVR_FRMW_ERR_CHANNEL_NOT_ENABLED";
    case SDVR_FRMW_ERR_CHANNEL_NOT_DISABLED:        return "SDVR_FRMW_ERR_CHANNEL_NOT_DISABLED";
    case SDVR_FRMW_ERR_SMO_NOT_CREATED:             return "SDVR_FRMW_ERR_SMO_NOT_CREATED";
    case SDVR_FRMW_ERR_INVALID_TIME:                return "SDVR_FRMW_ERR_INVALID_TIME";
    case SDVR_FRMW_ERR_ILLEGAL_SMO_PARAMS:          return "SDVR_FRMW_ERR_ILLEGAL_SMO_PARAMS";
    case SDVR_FRMW_ERR_SMO_NOT_SUPPORTED:           return "SDVR_FRMW_ERR_SMO_NOT_SUPPORTED";
    case SDVR_FRMW_ERR_VDET_ERROR:                  return "SDVR_FRMW_ERR_VDET_ERROR";
    case SDVR_FRMW_ERR_RUNTIME_ERROR:               return "SDVR_FRMW_ERR_RUNTIME_ERROR";
    case SDVR_FRMW_ERR_VPP_RUNTIME_ERROR:           return "SDVR_FRMW_ERR_VPP_RUNTIME_ERROR";
    case SDVR_FRMW_ERR_ENCODER_RUNTIME_ERROR:       return "SDVR_FRMW_ERR_ENCODER_RUNTIME_ERROR";
    case SDVR_FRMW_ERR_DECODER_RUNTIME_ERROR:       return "SDVR_FRMW_ERR_DECODER_RUNTIME_ERROR";
    case SDVR_FRMW_ERR_ILLEGAL_PARAMETER:           return "SDVR_FRMW_ERR_ILLEGAL_PARAMETER";
    case SDVR_FRMW_ERR_INTERNAL_ERROR:              return "SDVR_FRMW_ERR_INTERNAL_ERROR";
    case SDVR_FRMW_ERR_ILLEGAL_COMMAND:             return "SDVR_FRMW_ERR_ILLEGAL_COMMAND";
    case SDVR_FRMW_ERR_SMO_NOT_DISABLED:            return "SDVR_FRMW_ERR_SMO_NOT_DISABLED";
    case SDVR_FRMW_ERR_OUT_OF_MEMORY:               return "SDVR_FRMW_ERR_OUT_OF_MEMORY";
    case SDVR_FRMW_ERR_NO_IO_BOARD:                 return "SDVR_FRMW_ERR_NO_IO_BOARD";
    case SDVR_FRMW_ERR_AUDIO_RUNTIME:               return "SDVR_FRMW_ERR_AUDIO_RUNTIME";
    case SDVR_FRMW_ERR_UNSUPPORTED_COMMAND:         return "SDVR_FRMW_ERR_UNSUPPORTED_COMMAND";
    case SDVR_FRMW_ERR_SMO_CHAN_FAILED:             return "SDVR_FRMW_ERR_SMO_CHAN_FAILED";
    case SDVR_FRMW_ERR_RES_LIMIT_EXCEEDED:          return "SDVR_FRMW_ERR_RES_LIMIT_EXCEEDED";

    case SDVR_DRV_ERR_INVALID_PARAMETER:    return "SDVR_DRV_ERR_INVALID_PARAMETER";
    case SDVR_DRV_ERR_BOARD_IN_USE:         return "SDVR_DRV_ERR_BOARD_IN_USE";
    case SDVR_DRV_ERR_BOARD_CONNECT:        return "SDVR_DRV_ERR_BOARD_CONNECT";
    case SDVR_DRV_ERR_BOARD_CLOSE:          return "SDVR_DRV_ERR_BOARD_CLOSE";
    case SDVR_DRV_ERR_BOARD_RESET:          return "SDVR_DRV_ERR_BOARD_RESET";
    case SDVR_DRV_ERR_IPC_INIT:             return "SDVR_DRV_ERR_IPC_INIT";
    case SDVR_DRV_ERR_NO_CHANNELS:          return "SDVR_DRV_ERR_NO_CHANNELS";
    case SDVR_DRV_ERR_CHANNEL_IN_USE:       return "SDVR_DRV_ERR_CHANNEL_IN_USE";
    case SDVR_DRV_ERR_CHANNEL_CREATE:       return "SDVR_DRV_ERR_CHANNEL_CREATE";
    case SDVR_DRV_ERR_CHANNEL_CONNECT:      return "SDVR_DRV_ERR_CHANNEL_CONNECT";
    case SDVR_DRV_ERR_CHANNEL_CLOSE:        return "SDVR_DRV_ERR_CHANNEL_CLOSE";
    case SDVR_DRV_ERR_CHANNEL_NOT_ACTIVE:   return "SDVR_DRV_ERR_CHANNEL_NOT_ACTIVE";
    case SDVR_DRV_ERR_CHANNEL_DEAD:         return "SDVR_DRV_ERR_CHANNEL_DEAD";
    case SDVR_DRV_ERR_NO_RECV_BUFFERS:      return "SDVR_DRV_ERR_NO_RECV_BUFFERS";
    case SDVR_DRV_ERR_NO_SEND_BUFFERS:      return "SDVR_DRV_ERR_NO_SEND_BUFFERS";
    case SDVR_DRV_ERR_MSG_SEND:             return "SDVR_DRV_ERR_MSG_SEND";
    case SDVR_DRV_ERR_MSG_RECV:             return "SDVR_DRV_ERR_MSG_RECV";
    case SDVR_DRV_BOARD_BOOT_FAIL:          return "SDVR_DRV_BOARD_BOOT_FAIL";

    case SDVR_ERR_OUT_OF_MEMORY:            return "SDVR_ERR_OUT_OF_MEMORY";
    case SDVR_ERR_INVALID_HANDLE:           return "SDVR_ERR_INVALID_HANDLE";
    case SDVR_ERR_INVALID_ARG:              return "SDVR_ERR_INVALID_ARG";
    case SDVR_ERR_INVALID_BOARD:            return "SDVR_ERR_INVALID_BOARD";
    case SDVR_ERR_BOARD_CONNECTED:          return "SDVR_ERR_BOARD_CONNECTED";
    case SDVR_ERR_INVALID_CHANNEL:          return "SDVR_ERR_INVALID_CHANNEL";
    case SDVR_ERR_CHANNEL_CLOSED:           return "SDVR_ERR_CHANNEL_CLOSED";
    case SDVR_ERR_BOARD_CLOSED:             return "SDVR_ERR_BOARD_CLOSED";
    case SDVR_ERR_NO_VFRAME:                return "SDVR_ERR_NO_VFRAME";
    case SDVR_ERR_NO_AFRAME:                return "SDVR_ERR_NO_AFRAME";
    case SDVR_ERR_INTERNAL:                 return "SDVR_ERR_INTERNAL";
    case SDVR_ERR_BOARD_NOT_CONNECTED:      return "SDVR_ERR_BOARD_NOT_CONNECTED";
    case SDVR_ERR_IN_STREAMING:             return "SDVR_ERR_IN_STREAMING";
    case SDVR_ERR_NO_DVR_BOARD:             return "SDVR_ERR_NO_DVR_BOARD";
    case SDVR_ERR_WRONG_DRIVER_VERSION:     return "SDVR_ERR_WRONG_DRIVER_VERSION";
    case SDVR_ERR_DBG_FILE:                 return "SDVR_ERR_DBG_FILE";
    case SDVR_ERR_ENCODER_NOT_ENABLED:      return "SDVR_ERR_ENCODER_NOT_ENABLED";
    case SDVR_ERR_ENCODER_NOT_DISABLED:     return"SDVR_ERR_ENCODER_NOT_DISABLED";
    case SDVR_ERR_SDK_NO_FRAME_BUF:         return "SDVR_ERR_SDK_NO_FRAME_BUF";
    case SDVR_ERR_INVALID_FRAME_TYPE:       return "SDVR_ERR_INVALID_FRAME_TYPE";
    case SDVR_ERR_NOBUF:                    return "SDVR_ERR_NOBUF";
    case SDVR_ERR_CALLBACK_FAILED:			return "SDVR_ERR_CALLBACK_FAILED";
    case SDVR_ERR_INVALID_CHAN_HANDLE:      return "SDVR_ERR_INVALID_CHAN_HANDLE";
    case SDVR_ERR_COMMAND_NOT_SUPPORTED:    return "SDVR_ERR_COMMAND_NOT_SUPPORTED";
    case SDVR_ERR_ODD_SMO_COORDINATES:      return "SDVR_ERR_ODD_SMO_COORDINATES";
    case SDVR_ERR_LOAD_FIRMWARE:            return "SDVR_ERR_LOAD_FIRMWARE";
    case SDVR_ERR_WRONG_CHANNEL_TYPE:       return "SDVR_ERR_WRONG_CHANNEL_TYPE";
    case SDVR_ERR_DECODER_NOT_ENABLED:      return "SDVR_ERR_DECODER_NOT_ENABLED";
    case SDVR_ERR_BUF_NOT_AVAIL:            return "SDVR_ERR_BUF_NOT_AVAIL";
    case SDVR_ERR_MAX_REGIONS:              return "SDVR_ERR_MAX_REGIONS";
    case SDVR_ERR_INVALID_REGION:           return "SDVR_ERR_INVALID_REGION";
    case SDVR_ERR_INVALID_GOP:              return "SDVR_ERR_INVALID_GOP";
    case SDVR_ERR_INVALID_BITRATE:          return "SDVR_ERR_INVALID_BITRATE";
    case SDVR_ERR_INVALID_BITRATE_CONTROL:  return "SDVR_ERR_INVALID_BITRATE_CONTROL";
    case SDVR_ERR_INVALID_QUALITY:          return "SDVR_ERR_INVALID_QUALITY";
    case SDVR_ERR_INVALID_FPS:              return "SDVR_ERR_INVALID_FPS";
    case SDVR_ERR_UNSUPPORTED_FIRMWARE:     return "SDVR_ERR_UNSUPPORTED_FIRMWARE";
    case SDVR_ERR_INVALID_OSD_ID:           return "SDVR_ERR_INVALID_OSD_ID";
    case SDVR_ERR_OSD_LENGTH:               return "SDVR_ERR_OSD_LENGTH";
    case SDVR_ERR_OSD_FONT_FILE:            return "SDVR_ERR_OSD_FONT_FILE";
    case SDVR_ERR_FONT_ID:                  return "SDVR_ERR_FONT_ID";
    case SDVR_ERR_CAMERA_IN_REC:            return "SDVR_ERR_CAMERA_IN_REC";
    case SDVR_ERR_OPEN_REC_FILE:            return "SDVR_ERR_OPEN_REC_FILE";
    case SDVR_ERR_FAILED_ADD_VIDEO_TRACK:   return "SDVR_ERR_FAILED_ADD_VIDEO_TRACK";
    case SDVR_ERR_FAILED_ADD_AUDIO_TRACK:   return "SDVR_ERR_FAILED_ADD_AUDIO_TRACK";
    case SDVR_ERR_SDK_BUF_EXCEEDED:         return "SDVR_ERR_SDK_BUF_EXCEEDED";
    case SDVR_ERR_AUTH_KEY_MISSING:         return "SDVR_ERR_AUTH_KEY_MISSING";
    case SDVR_ERR_AUTH_KEY_LEN:             return "SDVR_ERR_AUTH_KEY_LEN";
    case SDVR_ERR_INVALID_RESOLUTION:       return "SDVR_ERR_INVALID_RESOLUTION";
    case SDVR_ERR_SMO_PORT_NUM:             return "SDVR_ERR_SMO_PORT_NUM";

    default: 
       sprintf(err_buf,"Unknown Error (%d)",error_no);
       return err_buf;

    }
}
/****************************************************************************
  This function gets a list all video standards supported by the
  firmware for a given board.
  
  Parameters:

    "board_index" - The index of the board.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "video_stds" - A pointer to a variable that contains all the supported  
    video standards. It is a bit OR of sdvr_video_std_e.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise see the error code list.

  Remark:

    Call this function or sdvr_get_board_attributes() before calling 
    sdvr_board_connect() to select one of the supported video standards 
    by the firmware.

****************************************************************************/
sdvr_err_e sdvr_get_supported_vstd(sx_uint32 board_index, 
                                   sx_uint32  *video_stds)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    dvr_board_t cmd_strct;
    int temp_connect = 0;

    if (board_index >= _ssdvr_boards_info.board_num)
    {
        err = SDVR_ERR_INVALID_BOARD;
    } 
    else if (video_stds == NULL)
    {
        err = SDVR_ERR_INVALID_ARG;
    }
    if (err != SDVR_ERR_NONE)
    {
        sdbg_msg_output_err(sdvr_get_supported_vstd_func,err);
        return err;
    }

    /*****************************************************************
     It is possible when this routine is called, we have not connected
     to the board yet. In this case, first connect to the board temporarily
     before getting the video standard and then disconnect from the board.
    *****************************************************************/
    if (_ssdvr_boards_info.boards[board_index].handle == NULL) {
        temp_connect = 1;
        if ((err = sct_open_board(board_index,
            &_ssdvr_boards_info.boards[board_index].handle)) != SDVR_ERR_NONE) 
        {
            sdbg_msg_output_err(sdvr_get_supported_vstd_func,err);
            return err;
        }
    }


    // If send board fails, most probably, either the firmware is crashed
    // or it was not loaded.
    err = ssdk_message_board_send(board_index, DVR_GET_BOARD, &cmd_strct, sizeof(cmd_strct));
    if (err == SDVR_ERR_NONE)
    {
        ssdk_message_board_recv(board_index, DVR_REP_BOARD, &cmd_strct, 0, 0);

        // Get all the supported video standards  by the firmware.
        // Return none, if the firmware returned any error.
        *video_stds = SDVR_VIDEO_STD_NONE;
        if (cmd_strct.status == SDVR_ERR_NONE)
            *video_stds = 0x0000FFFF & cmd_strct.camera_info;

        sdbg_msg_output(sdvr_get_supported_vstd_func, board_index, (void *)&cmd_strct);

        err = cmd_strct.status;

    }
    else
        sdbg_msg_output_err(sdvr_get_supported_vstd_func,err);

    // close the board if it temporarily was openned
    if (temp_connect)
    {
        sct_close_board(_ssdvr_boards_info.boards[board_index].handle);
        _ssdvr_boards_info.boards[board_index].handle = NULL;
    }

    return err;
}

/****************************************************************************
  This function sets the date and time in the firmware of a particular 
  SDVR board. The time is specified as a 32-bit value, which is the number
  of seconds elapsed since midnight of January 1, 1970. 

  Parameters:

    "board_index" - Index of the board on which to set the time.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "time" - The time value to be set. 

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_set_date_time(sx_uint32 board_index, 
                              time_t time)
{

    sdvr_err_e err;
    dvr_time_t cmd_strct;

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE)
    {
        return err;
    }
    cmd_strct.tv_sec = (sx_int32)time;
    cmd_strct.tv_usec = 0;

    ssdk_message_board_send(board_index, DVR_SET_TIME, &cmd_strct, sizeof(cmd_strct));
    ssdk_message_board_recv(board_index, DVR_REP_TIME, &cmd_strct, 0, 0);

    sdbg_msg_output(DVR_SET_TIME, board_index, (void *)&cmd_strct);

    return cmd_strct.status;
}
/****************************************************************************
  This function returns the date and time set in the firmware of a particular
  SDVR board. The time returned is a 32-bit value, which gives the number
  of seconds elapsed since midnight of January 1, 1970. 

  Parameters:

    "board_index" - Index of the board for which the time is requested.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "time" - A pointer to a time_t type that will hold the value of time
    when this function returns. 

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_get_date_time(sx_uint32 board_index, 
                              time_t *time)
{
    sdvr_err_e err;
    dvr_time_t cmd_strct;

    if (time == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else
        err = ssdk_validate_board_ndx(board_index);

    if (err !=  SDVR_ERR_NONE)
    {
        return err;
    }

    ssdk_message_board_send(board_index, DVR_GET_TIME, &cmd_strct, sizeof(cmd_strct));
    ssdk_message_board_recv(board_index, DVR_REP_TIME, &cmd_strct, 0, 0);

    sdbg_msg_output(DVR_GET_TIME, board_index, (void *)&cmd_strct);

    if (cmd_strct.status == SDVR_ERR_NONE)
        *time = cmd_strct.tv_sec;

    return cmd_strct.status;
}
/****************************************************************************
  This function is used to run diagnostics on the board. It accepts the
  name of the diagnostics firmware file and returns the diagnostics result
  in the given diag_code parameter. This function overwrites the 
  existing DVR firmware, so you must reload the DVR firmware for the DVR
  to function properly.

  Running diagnostics can take some time. 

  Parameters:

    "board_index" - Index of the board for which to run the diagnostics.
    This is a zero based number. It means the first board number 
    is index zero, second board number is index one, and so on.

    "diag_file_name" - A string containing the name of the file 
    containing the diagnostics firmware.

    "diag_code" - A pointer to an array of @thirty-two@ 32-bit variables that
    hold the diagnostics results for each PE. Upon return diag_code[0] will
    hold the result for PE0, diag_code[1] has the result for PE1, and so on.
    Refer to sdvr_diag_code_e for the possible values.

    "diag_code_size" - A pointer to an integer which holds the actual
    number of valid diagnostics results in the "diag_code" parameter.
    (i.e., In a five PE DVR board, the variable will be set to five on
    successful return.)

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
*****************************************************************************/
sdvr_err_e sdvr_run_diagnostics(sx_uint32 board_index, 
                                char *diag_file_name,
                                sdvr_diag_code_e *diag_code,
                                int *diag_code_size)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sct_board_t handle;
    sct_diag_info_t diag_info;

    if (diag_file_name == NULL || diag_code == NULL || diag_code_size == NULL)
        err = SDVR_ERR_INVALID_ARG;
    else if ((board_index + 1) > _ssdvr_boards_info.board_num)
        err = SDVR_ERR_INVALID_BOARD;
    else if (_ssdvr_boards_info.boards[board_index].handle != NULL)
        err = SDVR_ERR_BOARD_CONNECTED;

    if (err == SDVR_ERR_NONE)
    {
        if ((err = sct_open_board(board_index,&handle)) == SDVR_ERR_NONE) 
        {
            err = SDVR_ERR_NONE;
            /*************************************************************************
               We must always reset the board before loading new firmware.
               NOTE: There is a 10 seconds interval between the reset which causes the
                     boot-loader to be installed and you can load a new firmware. If 
                     you miss this period the sct_load_firmware() fails. Because of 
                     this restriction you can not have a break point between the 
                     sct_reset_board() and sct_load_firmware().
            *************************************************************************/
            if ((err = sct_reset_board(handle)) == SCT_NO_ERROR)
            {
               if (!sct_load_firmware(handle, diag_file_name, &diag_info))
                   err = SDVR_ERR_LOAD_FIRMWARE;
               else
               {
                   *diag_code_size = diag_info.pe_num;
                   memcpy(diag_code, diag_info.err_nums, sizeof(diag_info.err_nums));
               }
            }
            sct_close_board(handle);
        }
    }    
    return err;
}

/****************************************************************************
  This function returns the channel handle corresponding to the given
  frame buffer.

  Parameters:

    "frame_buffer" - The encoded or raw A/V frame or the motion values
    buffer.
    This buffer was obtained by the call to either sdvr_get_av_buffer() or
    sdvr_get_yuv_buffer(). 

  Returns:
   
    The channel handle corresponding to the given frame buffer. A value of
    INVALID_CHAN_HANDLE indicates that the given buffer is invalid.
****************************************************************************/

sdvr_chan_handle_t sdvr_get_buffer_channel(void *frame_buffer)
{
    sdvr_av_buffer_t    *av_buff = (sdvr_av_buffer_t *)frame_buffer;
    sdvr_chan_handle_t handle;

    if (av_buff == NULL) 
    {
        return INVALID_CHAN_HANDLE;
    }

    handle = ssdk_create_chan_handle(av_buff->board_id,
                                     av_buff->channel_type,
                                     av_buff->channel_id);

    // Since this function can be called for both the encoder and
    // decoder frame, we must validate independent of the buffer type.
    // The caller should verify whether this buffer belongs to a 
    // decoder or encoder.
    if (ssdk_validate_chan_handle(handle, -2) != SDVR_ERR_NONE)
        handle = INVALID_CHAN_HANDLE;

    return handle;
}

/****************************************************************************
  This function returns the 64-bit hardware-generated timestamp 
  corresponding to the given frame buffer.

  Parameters:

    "frame_buffer" - The encoded or raw A/V frame or the motion values
    buffer.
    This buffer was obtained by the call to either sdvr_get_av_buffer() or
    sdvr_get_yuv_buffer(). 

    "timestamp64" - A pointer to a 64-bit variable that holds the 
    timestamp on successful return. This is hardware-generated 
    timestamp of when the frame was captured.

  Returns:
   
    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
    
****************************************************************************/
sdvr_err_e sdvr_get_buffer_timestamp(void *frame_buffer, sx_uint64 *timestamp64)
{
    sdvr_av_buffer_t    *av_buff = (sdvr_av_buffer_t *)frame_buffer;
    sdvr_yuv_buffer_t  *yuv_buff = (sdvr_yuv_buffer_t *)frame_buffer;

    if (av_buff == NULL || av_buff->signature != SDVR_DATA_HDR_SIG || timestamp64 == NULL) 
    {
        return SDVR_ERR_INVALID_ARG;
    }

    if (av_buff->frame_type == SDVR_FRAME_RAW_VIDEO)
        *timestamp64 = yuv_buff->timestamp_64;
    else
        *timestamp64 = ((sx_uint64)av_buff->timestamp_high << 32) | av_buff->timestamp;

    return SDVR_ERR_NONE;
}

/****************************************************************************
  This function returns frame type corresponding to the given
  frame buffer.

  Parameters:

    "frame_buffer" - The encoded or raw A/V frame or the motion values
    buffer.
    This buffer was obtained by the call to either sdvr_get_av_buffer() or
    sdvr_get_yuv_buffer(). 

    "frame_type" - A pointer to a variable that holds the 
    frame type on successful return. (See __sdvr_frame_type_e for all the
    possible frame types.)

  Returns:
   
    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
    
****************************************************************************/
sdvr_err_e sdvr_get_buffer_frame_type(void *frame_buffer, sx_uint8 *frame_type)
{
    sdvr_av_buffer_t  *av_buff = (sdvr_av_buffer_t *)frame_buffer;

    if (av_buff == NULL || av_buff->signature != SDVR_DATA_HDR_SIG || frame_type == NULL) 
    {
        return SDVR_ERR_INVALID_ARG;
    }

    *frame_type = av_buff->frame_type;

    return SDVR_ERR_NONE;
}
/****************************************************************************
  This function returns YUV format type corresponding to the given
  raw video frame buffer.

  Parameters:

    "frame_buffer" - The raw video frame.
    This buffer was obtained by the call to 
    sdvr_get_yuv_buffer(). 

    "format" - A pointer to a variable that holds the 
    YUV format type on successful return.

  Returns:
   
    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
    
****************************************************************************/

sdvr_err_e sdvr_get_buffer_yuv_format(sdvr_yuv_buffer_t *frame_buffer, sx_uint8 *format)
{
    sdvr_yuv_buffer_t  *yuv_buff = (sdvr_yuv_buffer_t *)frame_buffer;

    if (yuv_buff == NULL || yuv_buff->signature != SDVR_DATA_HDR_SIG || format == NULL
        || yuv_buff->frame_type != SDVR_FRAME_RAW_VIDEO) 
    {
        return SDVR_ERR_INVALID_ARG;
    }

    *format = yuv_buff->yuv_format;

    return SDVR_ERR_NONE;

}

/****************************************************************************
  This function returns whether the given buffer is associated to 
  the primary or secondary encoder.

  Parameters:

    "frame_buffer" - The encoded video frame.
    This buffer was obtained by the call to sdvr_get_av_buffer(). 

    "sub_enc" - A pointer to a variable that holds the 
    sub-encoder ID on successful return. This field is only valid for encoded frames. 
    It indicates whether the video frame is from the primary (SDVR_ENC_PRIMARY) or
    the secondary (SDVR_ENC_SECONDARY) encoder on this encoded channel.


  Returns:
   
    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:

    The value of sub_enc is nondeterministic if the provided frame_buffer 
    does not correspond to an encoded video frame.
    
****************************************************************************/
sdvr_err_e sdvr_get_buffer_sub_encoder(sdvr_av_buffer_t *frame_buffer, sdvr_sub_encoders_e *sub_enc)
{
    sdvr_av_buffer_t  *av_buff = (sdvr_av_buffer_t *)frame_buffer;

    if (av_buff == NULL || av_buff->signature != SDVR_DATA_HDR_SIG || sub_enc == NULL) 
    {
        return SDVR_ERR_INVALID_ARG;
    }

    *sub_enc = av_buff->stream_id;

    return SDVR_ERR_NONE;
}

/****************************************************************************
  This function returns the alarm value for the given alarm type and overlay
  assoicated with the given video frame buffer.

  Parameters:

    "frame_buffer" - The encoded or raw video frame.
    This buffer was obtained by the call to either sdvr_get_av_buffer() or
    sdvr_get_yuv_buffer(). 

    "alarm_type" - The type of the alarm. SDVR_VIDEO_ALARM_MOTION,
    SDVR_VIDEO_ALARM_BLIND,  and SDVR_VIDEO_ALARM_NIGHT are the
    only supported alarm types.

    "overlay_num" - The overlay alarm region. Currently, this field
    is only valid for alarm_type of SDVR_VIDEO_ALARM_MOTION. The valid
    range is 1-4.

    "alarm_value" - A pointer to a variable to hold the alarm value
    assoicated to the given alarm_type and overlay_num.

  Returns:
   
    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
    
  Remarks:

    The value of alarm value is nondeterministic if the provided frame_buffer 
    does not correspond to a video frame.

****************************************************************************/
sdvr_err_e sdvr_get_buffer_alarm_value(void *frame_buffer, 
                                        sdvr_video_alarm_e  alarm_type,
                                        sx_uint8 overlay_num,
                                        sx_uint8 *alarm_value)
{
    sdvr_av_buffer_t  *av_buff = (sdvr_av_buffer_t *)frame_buffer;
    sdvr_yuv_buffer_t  *yuv_buff = (sdvr_yuv_buffer_t *)frame_buffer;

    if (av_buff == NULL || av_buff->signature != SDVR_DATA_HDR_SIG || alarm_value == NULL) 
    {
        return SDVR_ERR_INVALID_ARG;
    }

    switch (alarm_type)
    {
    case SDVR_VIDEO_ALARM_MOTION:
        if (overlay_num < 1 || overlay_num > 4)
            return SDVR_ERR_INVALID_ARG;
        if (av_buff->frame_type == SDVR_FRAME_RAW_VIDEO)
            *alarm_value = yuv_buff->motion_value[overlay_num - 1];
        else
            *alarm_value = av_buff->motion_value[overlay_num - 1];
        break;
    case SDVR_VIDEO_ALARM_BLIND:
        /*
            NOTE: currently Blind and Night detection only have
                  one layer. the overlay_num is ignored
        */
        if (av_buff->frame_type == SDVR_FRAME_RAW_VIDEO)
            *alarm_value = yuv_buff->blind_value[0];
        else
            *alarm_value = av_buff->blind_value[0];
        break;

    case SDVR_VIDEO_ALARM_NIGHT:
        if (av_buff->frame_type == SDVR_FRAME_RAW_VIDEO)
            *alarm_value = yuv_buff->night_value[0];
        else
            *alarm_value = av_buff->night_value[0];
        break;

    default:
        return SDVR_ERR_INVALID_ARG;

    }

    return SDVR_ERR_NONE;
}

/****************************************************************************
  This function returns the video frame width and number of lines for the given 
  video frame buffer.

  Parameters:

    "frame_buffer" - The encoded or raw video frame.
    This buffer was obtained by the call to either sdvr_get_av_buffer() or
    sdvr_get_yuv_buffer(). 

    "width" - A pointer to a variable to hold the width of the 
    video frame. This field is valid only for raw and encoded video frames.

    "lines" - A pointer to a variable to hold the number of lines of the 
    video frame. This field is valid only for raw and encoded video frames.

  Returns:
   
    SDVR_ERR_NONE - On success. Otherwise, see the error code list.

  Remarks:

    The value of width and lines are nondeterministic if the provided frame_buffer 
    does not correspond to a video frame.
    
****************************************************************************/
sdvr_err_e  sdvr_av_buf_video_dimensions( void *frame_buffer,
                                            sx_uint16 *width, 
                                            sx_uint16 *lines)
{
    sdvr_av_buffer_t  *av_buff = (sdvr_av_buffer_t *)frame_buffer;
    sdvr_yuv_buffer_t  *yuv_buff = (sdvr_yuv_buffer_t *)frame_buffer;

    if (av_buff == NULL || av_buff->signature != SDVR_DATA_HDR_SIG ||
        width == NULL || lines == NULL) 
    {
        return SDVR_ERR_INVALID_ARG;
    }

    if (av_buff->frame_type == SDVR_FRAME_RAW_VIDEO)
    {
        *width = yuv_buff->active_width;
        *lines = yuv_buff->active_height;
    }
    else
    {
        *width = av_buff->active_width;
        *lines = av_buff->active_height;
    }

    return SDVR_ERR_NONE;
}
/****************************************************************************
  Call this function to get the frame drop count, sequence number, and
  frame number associated to the given audio video frame buffer. Once the
  data port is enabled, all of these parameters are reset to zero. "Frame
  number" is incremented by one for each frame that is recieved from the hardware
  whereas the "sequence number" is the Nth frame that is sent out from the firmware
  to the host DVR Application. In streaming full frame rate, these two numbers
  are the same. If streaming less than full frame rate, there will be 
  gaps in "frame number" but "sequence number" should only increment by
  one. Any gap in "sequene number" means a frame was lost between the firmware and
  the host DVR Application. In a full frame rate, any gap in "frame number" means
  the frame was lost at the data port.


  Parameters:

    "frame_buffer" - The encoded or raw A/V frame.
    This buffer was obtained by the call to either sdvr_get_av_buffer() or
    sdvr_get_yuv_buffer(). 

    "seq_number" - A pointer to a variable to hold frame sequence number. 
    This field is valid only
    for raw and encoded video frames. Every channel and
    stream combination will have independent sequence numbering. The raw
    and encoded video streams from the same channel will have independent
    sequence numbering.
    
    "frame_number" - A pointer to a variable to hold  number of frames 
    seen on this channel so far.
    This field is valid only for raw and encoded video frames. The
    frame number and sequence number will be identical when the 
    stream is being run at full frame rate.

    "frame_drop_count" - A pointer to a variable to hold number of frames 
    dropped detected by the firmware on the current stream.

  Returns:
   
    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
    
****************************************************************************/
sdvr_err_e  sdvr_av_buf_sequence( void *frame_buffer, 
                                    sx_uint32 *seq_number,
                                    sx_uint32 *frame_number, 
                                    sx_uint32 *frame_drop_count )
{
    sdvr_av_buffer_t  *av_buff = (sdvr_av_buffer_t *)frame_buffer;
    sdvr_yuv_buffer_t  *yuv_buff = (sdvr_yuv_buffer_t *)frame_buffer;

    if (av_buff == NULL || av_buff->signature != SDVR_DATA_HDR_SIG ||
        seq_number == NULL || frame_number == NULL || frame_drop_count == NULL) 
    {
        return SDVR_ERR_INVALID_ARG;
    }

    if (av_buff->frame_type == SDVR_FRAME_RAW_VIDEO)
    {
        *seq_number = yuv_buff->seq_number;
        *frame_number = yuv_buff->frame_number;
        *frame_drop_count = yuv_buff->frame_drop_count;
    }
    else
    {
        *seq_number = av_buff->seq_number;
        *frame_number = av_buff->frame_number;
        *frame_drop_count = av_buff->frame_drop_count;
    }

    return SDVR_ERR_NONE;

}

/****************************************************************************
  Call this function to get the encoded A/V frame, raw or encoded audio frame, or the 
  motion values buffer that is assoiced to the given frame buffer.

  Parameters:

    "frame_buffer" - The encoded A/V, raw audio, or motion values buffer. This buffer was
    obtained by the call to sdvr_get_av_buffer().

    "payload" - A pointer to a pointer to points to the beginning of data
    for the given frame.

    "payload_size" - A pointer to a variable to hold size of payload.

  Returns:
   
    SDVR_ERR_NONE - On success. Otherwise, see the error code list.  

****************************************************************************/
sdvr_err_e  sdvr_av_buf_payload( sdvr_av_buffer_t *frame_buffer, sx_uint8 **payload, 
              sx_uint32 *payload_size)
{
    sdvr_av_buffer_t  *av_buff = (sdvr_av_buffer_t *)frame_buffer;

    if (av_buff == NULL || av_buff->signature != SDVR_DATA_HDR_SIG ||
        payload == NULL || payload_size == NULL) 
    {
        return SDVR_ERR_INVALID_ARG;
    }

    *payload_size = av_buff->payload_size;
    *payload = &av_buff->payload[0];

    return SDVR_ERR_NONE;

}

/****************************************************************************
  Call this function to get the raw video buffers for the given frame buffer.

  Parameters:

    "yuv_buffer" - The raw video frame. This buffer was obtained by the
    call to sdvr_get_yuv_buffer().

    "y_data" - Pointer to a buffer containing the Y data of a raw frame.

    "u_data" - Pointer to a buffer containing the U data of a raw frame.

    "v_data" - Pointer to a buffer containing the V data of a raw frame.

    "y_data_size" - Pointer to a variable the holds the number of bytes in
    y_data buffer.

    "u_data_size" - Pointer to a variable the holds the number of bytes in
    u_data buffer.

    "v_data_size" - Pointer to a variable the holds the number of bytes in
    v_data buffer.

  Returns:
   
    SDVR_ERR_NONE - On success. Otherwise, see the error code list.  

****************************************************************************/
sdvr_err_e  sdvr_av_buf_yuv_payload( sdvr_yuv_buffer_t *yuv_buffer, sx_uint8 **y_data, 
              sx_uint8 **u_data, sx_uint8 **v_data,
              sx_uint32 *y_data_size, sx_uint32 *u_data_size, sx_uint32 *v_data_size)
{
    sdvr_yuv_buffer_t  *yuv_buff = (sdvr_yuv_buffer_t *)yuv_buffer;

    if (yuv_buff == NULL || yuv_buff->signature != SDVR_DATA_HDR_SIG ||
        yuv_buff->frame_type != SDVR_FRAME_RAW_VIDEO ||
        *y_data == NULL || *u_data == NULL || *v_data == NULL ||
        y_data_size == NULL || u_data_size == NULL || v_data_size == NULL) 
    {
        return SDVR_ERR_INVALID_ARG;
    }

    *y_data_size = yuv_buff->y_data_size;
    *u_data_size = yuv_buff->u_data_size;
    *v_data_size = yuv_buff->v_data_size;
    *y_data = yuv_buff->y_data;
    *u_data = yuv_buff->u_data;
    *v_data = yuv_buff->v_data;

    return SDVR_ERR_NONE;

}

/****************************************************************************
  This function returns the board number, given a channel handle.

  Parameters:

    "handle" - A channel handle.

  Returns:

    Zero-based board index corresponding to this handle. A return value of 0xFF 
    indicates that the channel handle is invalid.
****************************************************************************/
sx_uint8 sdvr_get_board_index(sdvr_chan_handle_t handle)
{

    //  Note: board index starts at the bit 12 of handle and its size is
    //        4 bits.

    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    if (ssdk_validate_chan_handle(handle, -2) != SDVR_ERR_NONE)
        return 0xFF;

    return board_index;
}

/****************************************************************************
  This function returns the channel number, given a channel handle.

  Parameters:

    "handle" - A channel handle.

  Returns:

    Channel number corresponding to this handle. A return value of 0xFF 
    indicates that the channel handle is invalid.
****************************************************************************/
sx_uint8 sdvr_get_chan_num(sdvr_chan_handle_t handle)
{
    //  Note: Channel number is the first 8 bits of the handle
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);

    if (ssdk_validate_chan_handle(handle, -2) != SDVR_ERR_NONE)
        return 0xFF;

    return chan_num;
}

/****************************************************************************
  This function returns the channel type, given a handle.

  Parameters:

    "handle" - A channel handle.

  Returns:

    SDVR_CHAN_TYPE_ENCODER - For an encoding channel.

    SDVR_CHAN_TYPE_DECODER - For a decoding channel.

    SDVR_CHAN_TYPE_OUTPUT - Internal use to the SDK only for videoout.

    SDVR_CHAN_TYPE_NONE - For a channel whose type is either not set, or
    the channel handle is invalid.
****************************************************************************/
sdvr_chan_type_e sdvr_get_chan_type(sdvr_chan_handle_t handle)
{
    /********************************************************************
      Note: channel type starts at the bit 8 of handle and its size is
            4 bits.
    *********************************************************************/
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);

    if (ssdk_validate_chan_handle(handle, -2) != SDVR_ERR_NONE)
        return 0xFF;

    // Translate the job type to channel type

    switch (chan_type)
    {
    case DVR_JOB_CAMERA_ENCODE:
        return SDVR_CHAN_TYPE_ENCODER;

    case DVR_JOB_HOST_DECODE:
        return SDVR_CHAN_TYPE_DECODER;

    case DVR_JOB_VIDEO_OUTPUT:
        return SDVR_CHAN_TYPE_OUTPUT;

    default:
        return SDVR_CHAN_TYPE_NONE;
    }
}
/****************************************************************************
  This function checks to see if the given channel handle is valid for the given
  channel type by testing the high word of the handle for a specific signiture. 
  Then it validates the board index by checking to see if it is conneced. As well 
  as checking the channel by looking at the type and verifying the channel for 
  the type is connected.

  Parameters:

    "handle" - Channel handle to validate.

    "chk_chan_type" - The channel type of the given handle to be verified.
    A value of -1 means any valid encoder or decoder channel type.
    A value of -2 means any valid encoder, decoder channel type or smo port.
  
  Returns:

    SDVR_ERR_NONE - The given board index was in the range and this board was
    connected to previously.

    SDVR_ERR_INVALID_CHAN_HANDLE - Error code if the given handle is invalid
    due to a bad channel signiture or the channel does not exist.

    SDVR_ERR_INVALID_BOARD - Error code if the given handle is invalide due to
    board index does not exist.

    SDVR_ERR_BOARD_NOT_CONNECTED - The given board index is valid but has not
    been connected to yet.

    SDVR_ERR_WRONG_CHANNEL_TYPE - Error code if the given channel handle does
    not match the expected channel type.
****************************************************************************/
sdvr_err_e ssdk_validate_chan_handle(sdvr_chan_handle_t handle, int chk_chan_type) 
{
    sdvr_err_e  err = SDVR_ERR_NONE;
    if (!CH_IS_VALID_SIGNITURE(handle))
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
    else
    {
        /*
        Note: Channel number is the first 8 bits of the handle.
              Channel type starts at the bit 8 of handle and its 
              size is 4 bits.
              Board index starts at the bit 12 of handle and its 
              size is 4 bits.

              we should not call sdvr_get_board_index(), sdvr_get_chan_num(),
              or sdvr_get_chan_type() otherwise we get into a
              recursive condition.
        */
        sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
        sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
        sx_uint8 board_index = CH_GET_BOARD_ID(handle);

        switch (chk_chan_type)
        {
        case -1:
            if (chan_type != SDVR_CHAN_TYPE_ENCODER
                && chan_type != SDVR_CHAN_TYPE_DECODER)
                return SDVR_ERR_WRONG_CHANNEL_TYPE;
            break;

        case -2:
            if (chan_type != SDVR_CHAN_TYPE_ENCODER
                && chan_type != SDVR_CHAN_TYPE_DECODER
                && chan_type != SDVR_CHAN_TYPE_OUTPUT)
                return SDVR_ERR_WRONG_CHANNEL_TYPE;
            break;

        case SDVR_CHAN_TYPE_ENCODER:
            if (chan_type != SDVR_CHAN_TYPE_ENCODER)
                return SDVR_ERR_WRONG_CHANNEL_TYPE;

            break;

        case SDVR_CHAN_TYPE_DECODER:
            if (chan_type != SDVR_CHAN_TYPE_DECODER)
                return SDVR_ERR_WRONG_CHANNEL_TYPE;

            break;

        case SDVR_CHAN_TYPE_OUTPUT:
            if (chan_type != SDVR_CHAN_TYPE_OUTPUT)
                return SDVR_ERR_WRONG_CHANNEL_TYPE;

            break;

        default:
            return SDVR_ERR_INVALID_CHAN_HANDLE;
        }

        if ((board_index + 1) > _ssdvr_boards_info.board_num) {
            err = SDVR_ERR_INVALID_BOARD;
        }
        else
        {
            if (_ssdvr_boards_info.boards[board_index].handle == NULL) {
                return SDVR_ERR_BOARD_NOT_CONNECTED;
            }
            if (chan_type == SDVR_CHAN_TYPE_DECODER)
            {
                if (_ssdvr_boards_info.boards[board_index].decode_channels == NULL ||
                  chan_num >= _ssdvr_boards_info.boards[board_index].max_num_decoders ||
                  !_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pci_chan_handle)
                    return SDVR_ERR_INVALID_CHAN_HANDLE;
            }
            else if (chan_type == SDVR_CHAN_TYPE_OUTPUT)
            {
                if (chan_num >= MAX_NUM_SMO ||
                  !_ssdvr_boards_info.boards[board_index].smo_info[chan_num].pci_send_chan_handle)
                    return SDVR_ERR_INVALID_CHAN_HANDLE;
            }
            else
            {
                if (_ssdvr_boards_info.boards[board_index].recv_channels == NULL ||
                  chan_num >= _ssdvr_boards_info.boards[board_index].max_num_video_in ||
                  !_ssdvr_boards_info.boards[board_index].recv_channels[chan_num].pci_chan_handle)
                    return SDVR_ERR_INVALID_CHAN_HANDLE;
            }
        }
    }
    return err;
}

/****************************************************************************
  This function checks to see if the given board index, channel number, and
  channel type are within a valid range. It validate if the channel number
  does exist for the given type on the given board. To validate a channel
  handle call ssdk_validate_chan_handle()

  Parameters:

    "board_index" - The board index to validate.

    "chan_num" - The channel number to verify it is with range for the
    given channel type.

    "chan_type" - The type of channel to verify.

  Returns:

    SDVR_ERR_NONE - The given board index was in the range and this board was
    connected to previously and the channel number is valid for the given
    channel type. Otherwise, errors.
****************************************************************************/
sdvr_err_e ssdk_validate_board_chan_type(sx_uint8 board_index, sx_uint8 chan_num, sx_uint8 chan_type)
{
    sdvr_err_e  err = SDVR_ERR_NONE;
    {
        switch (chan_type)
        {
//        case SDVR_CHAN_TYPE_NONE:
        case SDVR_CHAN_TYPE_ENCODER:
        case SDVR_CHAN_TYPE_DECODER:
            break;
        default:
            return SDVR_ERR_WRONG_CHANNEL_TYPE;
        }

        if ((board_index + 1) > _ssdvr_boards_info.board_num) {
            err = SDVR_ERR_INVALID_BOARD;
        }
        else
        {
            if (_ssdvr_boards_info.boards[board_index].handle == NULL) {
                return SDVR_ERR_BOARD_NOT_CONNECTED;
            }
            if (chan_type == SDVR_CHAN_TYPE_DECODER)
            {
                if (chan_num >= _ssdvr_boards_info.boards[board_index].max_num_decoders)
                    return SDVR_ERR_INVALID_CHANNEL;
            }
            else
            {
                if (chan_num >= _ssdvr_boards_info.boards[board_index].max_num_video_in)
                    return SDVR_ERR_INVALID_CHANNEL;
            }
        }
    }
    return err;

}

sdvr_err_e ssdk_validate_enc_sub_chan(sdvr_chan_handle_t handle,
                               sdvr_sub_encoders_e sub_chan_enc)
{
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);

    if (sub_chan_enc != SDVR_ENC_PRIMARY && sub_chan_enc != SDVR_ENC_SECONDARY)
        return SDVR_ERR_INVALID_ARG;

    if (_ssdvr_boards_info.boards[board_index].recv_channels[chan_num].enc_av_frame_queue[sub_chan_enc].frame_queue == NULL)
        return SDVR_ERR_INVALID_ARG;

    return SDVR_ERR_NONE;
}

/****************************************************************************
  This function  checks to see if the given board index is valid and
  has been connected.

  Parameters:

    "board_index" - Index of the board to be verified that it is valid and the
    board was connected to previously.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise see the error code list.
****************************************************************************/
sdvr_err_e ssdk_validate_board_ndx(sx_uint32 board_index)
{
    if ((board_index + 1) > _ssdvr_boards_info.board_num) {
        return SDVR_ERR_INVALID_BOARD;
    }
    if (_ssdvr_boards_info.boards[board_index].handle == NULL) {
        return SDVR_ERR_BOARD_NOT_CONNECTED;
    }

    return SDVR_ERR_NONE;
}

/****************************************************************************
   This function returns the maximum supported
   channels for the given channel type on a given board index. 

   NOTE: chan_type of SDVR_CHAN_TYPE_NONE is assumed to be
   HMO only channels which is the same as max_num_video_in.
****************************************************************************/
int ssdk_get_max_chan_num(int board_index, sx_uint8 chan_type)
{
    switch (chan_type)
    {
    case SDVR_CHAN_TYPE_ENCODER:
        return _ssdvr_boards_info.boards[board_index].max_num_video_in;
    case SDVR_CHAN_TYPE_DECODER:
        return _ssdvr_boards_info.boards[board_index].max_num_decoders;
    default:
        return 0;
    }
}

/****************************************************************************
  This function is called to open two channels of communication via PCI driver
  between the firmware and the SDK to send encoded A/V frames to the FW and
  recieve decoded raw A/V from.

  This function must be called only after sending DVR_SET_CONTROL for the
  channel to be created

  Parameters:

    "handle" - Handle of a decoder channel.
  
  Returns:

    SDVR_ERR_NONE - On success. Otherwise see the error code list.
****************************************************************************/
sdvr_err_e ssdk_drv_open_decode_xfr(sdvr_chan_handle_t handle)
{
    sdvr_err_e err;
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sdvr_chan_type_e chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint32 i;
    // The PCI port number to recieve data from the FW is the
    // same number as the 1st 16 bit of channel handle.
    sx_int32 sct_port_chan = handle & 0x0000FFFF;
    // The PCI port number to send data to the FW is the
    // same number as PCI receive port except the 16th bit is set to 1 
    sx_int32 sct_send_port_chan = sct_port_chan | 0x00008000;
    if (chan_type != SDVR_CHAN_TYPE_DECODER)
        return SDVR_ERR_INVALID_HANDLE;

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE){
        return err;
    }

    // Make sure the user has specified enough buffer to queue the frames
    if (m_sdkParams.enc_buf_num == 0 || m_sdkParams.raw_buf_num == 0 ||
        m_sdkParams.dec_buf_num == 0 || m_sdkParams.dec_buf_size == 0)
        return SDVR_ERR_SDK_NO_FRAME_BUF;


    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].user_data = 0;
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].ui_cb_func = NULL;
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].ui_cb_context = NULL;


    /***********************************************************************
        Allocate buffers to hold the rcved raw A/V frame. Since both the
        raw audio and video frames are sent via the same PCI channel, we need
        to allocate the intermidate buffers for both type of frames here.
    ***********************************************************************/

    /***********************************************************************
             Allocate the queue for YUV buffers.
    ************************************************************************/
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.start_item = 0;
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.top_item = 0;
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.num_recvd_frames = 0;
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.size = 0;

    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.frame_queue = 
            calloc(m_sdkParams.raw_buf_num,sizeof(ssdk_frame_queue_item_t));

    if (_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.frame_queue == NULL)
        return SDVR_ERR_INTERNAL;

    // Limiting the YUV buffer so that
    // we will not hang the board if the DVR Application is slow.
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.size = 
        m_sdkParams.raw_buf_num > 1 ? m_sdkParams.raw_buf_num - 1 : m_sdkParams.raw_buf_num;
    // Now Initialize each item in the yuv frame queue
    for (i = 0; i < _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.size; i++)
    {
        _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.frame_queue[i].next = i+1;
        _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.frame_queue[i].avail = 1;
        _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.frame_queue[i].data = NULL;
    }
    // wrap the last item back to the first
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.frame_queue[i-1].next = 0;

    /***********************************************************************
             Allocate the queue for raw audio buffers.
    ************************************************************************/
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.start_item = 0;
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.top_item = 0;
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.num_recvd_frames = 0;
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.size = 0;

    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.frame_queue = 
            calloc(m_sdkParams.raw_buf_num,sizeof(ssdk_frame_queue_item_t));

    if (_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.frame_queue == NULL)
        return SDVR_ERR_INTERNAL;

    // Limiting the raw audio buffer so that
    // we will not hang the board if the DVR Application is slow.
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.size = 
        m_sdkParams.raw_buf_num > 1 ? m_sdkParams.raw_buf_num - 1 : m_sdkParams.raw_buf_num;

    // Now Initialize each item in the frame queue
    for (i = 0; i < _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.size; i++)
    {
        _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.frame_queue[i].next = i+1;
        _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.frame_queue[i].avail = 1;
        _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.frame_queue[i].data = NULL;
    }
    // wrap the last item back to the first
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.frame_queue[i-1].next = 0;

    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].sdk_chan_handle = handle;

    /************************************************************
        Open the PCI channel to recieve frames.
       Also makes sure if a callback function is defined to associate
       it to this channel so that we can recieve AV buffers
    ************************************************************/
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pci_chan_handle = 
        sct_channel_accept(_ssdvr_boards_info.boards[board_index].handle,
        sct_port_chan,
        m_sdkParams.enc_buf_num);
    ssdk_register_sdk_chan_callback(_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pci_chan_handle,
                                    handle);


    /*******************************************************************************
    Once we setup the PCI channel to receive raw A/V frames, we have to setup the
    PCI channels to send encoded A/V frames.
    ********************************************************************************/
    _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].avail_decode_buf_count =
        m_sdkParams.dec_buf_num;
    err = sct_channel_connect(_ssdvr_boards_info.boards[board_index].handle,
        sct_send_port_chan,
        m_sdkParams.dec_buf_num,
        m_sdkParams.dec_buf_size,
        &_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pci_send_chan_handle);

    return err;
}
/****************************************************************************
  This function is called to open a channel of communication via PCI driver
  between the firmware and the SDK.

  This function must be called only after sending DVR_SET_CONTROL for the
  channel to be created

  Parameters:

    "handle" - Handle of an encoder channel.

    "chan_enc_num" - The number of encoders on this channels. 
  
  Returns:

    SDVR_ERR_NONE - On success. Otherwise see the error code list.
****************************************************************************/
sdvr_err_e ssdk_drv_open_rcv(sdvr_chan_handle_t handle, sx_int8 chan_enc_num)
{
    sdvr_err_e err;
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sdvr_chan_type_e chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

    sx_uint32 i;
    sx_uint32 enc_num;
    sx_int32 sct_port_chan = handle & 0x0000FFFF;

    if (chan_type == SDVR_CHAN_TYPE_DECODER)
        return SDVR_ERR_INVALID_HANDLE;

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE){
        return err;
    }

    // Make sure the user has specified enough buffer to queue the frames
    if (m_sdkParams.enc_buf_num == 0 || m_sdkParams.raw_buf_num == 0)
        return SDVR_ERR_SDK_NO_FRAME_BUF;


    enc_chan_info->user_data = 0;
    enc_chan_info->ui_cb_func = NULL;
    enc_chan_info->ui_cb_context = NULL;
    enc_chan_info->vpp_action_flags = 
        (DVR_VPP_ACTION_ANALYTIC | DVR_VPP_ACTION_DEINTERLACE | 
        DVR_VPP_ACTION_MEDIAN_FILTER | DVR_VPP_ACTION_SLATERAL);


    /***********************************************************************
        Allocate buffers to hold the rcved A/V frame. Since both the encoded
        and raw video frames are sent via the same PCI channel, we need
        to allocate the intermidate buffers for both type of frames here.
    ***********************************************************************/

    /***********************************************************************
           First we are allocating memory for all encoder frame queues.
    ************************************************************************/
    memset(&enc_chan_info->enc_av_frame_queue[0],0, sizeof(ssdk_frame_queue_t) * MAX_VIDEO_ENCODING);

    for (enc_num = 0; enc_num < (sx_uint32)chan_enc_num; enc_num++)
    {
        // NOTE: The size of SDK queue for the encoded frames is half of the driver's so that SDK will not
        //       hold on to the buffer for a long time and cause a hang on the firmware.
        enc_chan_info->enc_av_frame_queue[enc_num].size = (m_sdkParams.enc_buf_num + 1) / 2;

        enc_chan_info->enc_av_frame_queue[enc_num].frame_queue = 
                calloc(enc_chan_info->enc_av_frame_queue[enc_num].size, sizeof(ssdk_frame_queue_item_t));

        if (enc_chan_info->enc_av_frame_queue[enc_num].frame_queue == NULL)
        {
            enc_chan_info->enc_av_frame_queue[enc_num].size = 0;
            return SDVR_ERR_INTERNAL;
        }

        // Now Initialize each item in the encoded frame queue for the current encoded sub-channel
        for (i = 0; i < enc_chan_info->enc_av_frame_queue[enc_num].size; i++)
        {
            enc_chan_info->enc_av_frame_queue[enc_num].frame_queue[i].next = i+1;
            enc_chan_info->enc_av_frame_queue[enc_num].frame_queue[i].avail = 1;
            enc_chan_info->enc_av_frame_queue[enc_num].frame_queue[i].data = NULL;
        }
        // wrap the last item back to the first
        enc_chan_info->enc_av_frame_queue[enc_num].frame_queue[i-1].next = 0;

        // Re-set all the sub encoder information.
        memset(&enc_chan_info->sub_enc_info[enc_num],0,sizeof(enc_chan_info->sub_enc_info[enc_num]));

    }

    /***********************************************************************
           Allocating memory for motion values frame queues.
    ************************************************************************/
    enc_chan_info->motion_values_frame_queue.start_item = 0;
    enc_chan_info->motion_values_frame_queue.top_item = 0;
    enc_chan_info->motion_values_frame_queue.num_recvd_frames = 0;
    enc_chan_info->motion_values_frame_queue.size = 0;

    // NOTE: The size of SDK queue for the motion value frames is hard coded.
    enc_chan_info->motion_values_frame_queue.frame_queue = 
            calloc(_SSDK_MOTION_VALUE_QUEUE_SIZE, sizeof(ssdk_frame_queue_item_t));

    if (enc_chan_info->motion_values_frame_queue.frame_queue == NULL)
        return SDVR_ERR_INTERNAL;

    enc_chan_info->motion_values_frame_queue.size = _SSDK_MOTION_VALUE_QUEUE_SIZE;
    // Now Initialize each item in the motion values frame queue
    for (i = 0; i < _SSDK_MOTION_VALUE_QUEUE_SIZE; i++)
    {
        enc_chan_info->motion_values_frame_queue.frame_queue[i].next = i+1;
        enc_chan_info->motion_values_frame_queue.frame_queue[i].avail = 1;
        enc_chan_info->motion_values_frame_queue.frame_queue[i].data = NULL;
    }
    // wrap the last item back to the first
    enc_chan_info->motion_values_frame_queue.frame_queue[i-1].next = 0;


    /***********************************************************************
             Allocate the queue for YUV buffers.
    ************************************************************************/
    enc_chan_info->yuv_frame_queue.start_item = 0;
    enc_chan_info->yuv_frame_queue.top_item = 0;
    enc_chan_info->yuv_frame_queue.num_recvd_frames = 0;
    // Limiting the YUV buffer so that
    // we will not hang the board if the DVR Application is slow.
    enc_chan_info->yuv_frame_queue.size = 
        m_sdkParams.raw_buf_num > 1 ? m_sdkParams.raw_buf_num - 1 : m_sdkParams.raw_buf_num;

    enc_chan_info->yuv_frame_queue.frame_queue = 
            calloc(enc_chan_info->yuv_frame_queue.size,sizeof(ssdk_frame_queue_item_t));

    if (enc_chan_info->yuv_frame_queue.frame_queue == NULL)
        return SDVR_ERR_INTERNAL;


    // Now Initialize each item in the yuv frame queue
    for (i = 0; i < enc_chan_info->yuv_frame_queue.size; i++)
    {
        enc_chan_info->yuv_frame_queue.frame_queue[i].next = i+1;
        enc_chan_info->yuv_frame_queue.frame_queue[i].avail = 1;
        enc_chan_info->yuv_frame_queue.frame_queue[i].data = NULL;
    }
    // wrap the last item back to the first
    enc_chan_info->yuv_frame_queue.frame_queue[i-1].next = 0;

    /***********************************************************************
             Allocate the queue for encoded audio buffers.
    ************************************************************************/
    enc_chan_info->enc_audio_frame_queue.start_item = 0;
    enc_chan_info->enc_audio_frame_queue.top_item = 0;
    enc_chan_info->enc_audio_frame_queue.num_recvd_frames = 0;
    enc_chan_info->enc_audio_frame_queue.size = (m_sdkParams.enc_buf_num + 1) / 2;

    // NOTE: The size of SDK queue for the encoded audio frames is half of the driver's so that SDK will not
    //       hold on to the buffer for a long time and cause a hang on the firmware.
    enc_chan_info->enc_audio_frame_queue.frame_queue = 
            calloc(enc_chan_info->enc_audio_frame_queue.size, sizeof(ssdk_frame_queue_item_t));

    if (enc_chan_info->enc_audio_frame_queue.frame_queue == NULL)
    {
        enc_chan_info->enc_audio_frame_queue.size = 0;
        return SDVR_ERR_INTERNAL;
    }

    // Now Initialize each item in the encoded audio frame queue
    for (i = 0; i < enc_chan_info->enc_audio_frame_queue.size; i++)
    {
        enc_chan_info->enc_audio_frame_queue.frame_queue[i].next = i+1;
        enc_chan_info->enc_audio_frame_queue.frame_queue[i].avail = 1;
        enc_chan_info->enc_audio_frame_queue.frame_queue[i].data = NULL;
    }
    // wrap the last item back to the first
    enc_chan_info->enc_audio_frame_queue.frame_queue[i-1].next = 0;

    /***********************************************************************
             Allocate the queue for raw audio buffers.
    ************************************************************************/
    enc_chan_info->raw_audio_frame_queue.start_item = 0;
    enc_chan_info->raw_audio_frame_queue.top_item = 0;
    enc_chan_info->raw_audio_frame_queue.num_recvd_frames = 0;
    // Limiting the audio buffer so that
    // we will not hang the board if the DVR Application is slow.
    enc_chan_info->raw_audio_frame_queue.size = 
        m_sdkParams.raw_buf_num > 1 ? m_sdkParams.raw_buf_num - 1 : m_sdkParams.raw_buf_num;
    enc_chan_info->is_raw_audio_streaming = 0;

    enc_chan_info->raw_audio_frame_queue.frame_queue = 
            calloc(enc_chan_info->raw_audio_frame_queue.size,sizeof(ssdk_frame_queue_item_t));

    if (enc_chan_info->raw_audio_frame_queue.frame_queue == NULL)
    {
        enc_chan_info->raw_audio_frame_queue.size = 0;
        return SDVR_ERR_INTERNAL;
    }


    // Now Initialize each item in the frame queue
    for (i = 0; i < enc_chan_info->raw_audio_frame_queue.size; i++)
    {
        enc_chan_info->raw_audio_frame_queue.frame_queue[i].next = i+1;
        enc_chan_info->raw_audio_frame_queue.frame_queue[i].avail = 1;
        enc_chan_info->raw_audio_frame_queue.frame_queue[i].data = NULL;
    }
    // wrap the last item back to the first
    enc_chan_info->raw_audio_frame_queue.frame_queue[i-1].next = 0;

    enc_chan_info->sdk_chan_handle = handle;

    /************************************************************
        Open the PCI channel to recieve frames.
       Also makes sure if a callback function is defined to associate
       it to this channel so that we can recieve AV buffers
    ************************************************************/
    enc_chan_info->pci_chan_handle = 
        sct_channel_accept(_ssdvr_boards_info.boards[board_index].handle,
        sct_port_chan,
        m_sdkParams.enc_buf_num * chan_enc_num + m_sdkParams.raw_buf_num * 2 + 3);

    /*************************************************************
       If we failed to create the PCI handle, it could be due to 
       firmware has crashed or been re-set by watchdog timer.
       In this case re-set all the initialization and return
       the error.
    **************************************************************/
    if (enc_chan_info->pci_chan_handle == 0)
    {
        return SDVR_DRV_ERR_CHANNEL_CREATE;
    }
    ssdk_register_sdk_chan_callback(enc_chan_info->pci_chan_handle,
                                    handle);

    /***********************************************
        Initialize the critical section handling
    ***********************************************/
#ifdef WIN32
    InitializeCriticalSection( &enc_chan_info->cs_chan_rec );
#else
    pthread_mutex_init(&enc_chan_info->cs_chan_rec,NULL);
#endif

    return SDVR_ERR_NONE;
}

/****************************************************************************
  This function is called to close channels of PIC communication
  between the firmware and the SDK for the decoding channels.

  Parameters:

    "handle" - Handle of a decoder channel.
  
  Returns:

    SDVR_ERR_NONE - On success. Otherwise see the error code list.
****************************************************************************/
sdvr_err_e ssdk_drv_close_decode_xfr(sdvr_chan_handle_t handle)
{
    sdvr_err_e err;
    int i;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sdvr_chan_type_e chan_type = CH_GET_CHAN_TYPE(handle);
    sct_channel_t pci_chan_handle = 
        _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pci_chan_handle;
    sct_channel_t pci_send_chan_handle = 
        _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pci_send_chan_handle;

    if (chan_type != SDVR_CHAN_TYPE_DECODER)
        return SDVR_ERR_INVALID_HANDLE;

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE){
        return err;
    }

    /***************************************************************
    Close PCI recieve channels
    ****************************************************************/
    if (pci_chan_handle) 
    {
        /***************************************************************************
          Before closing the PCI channel we must make sure all the frames that
          were queued by the SDK are released. This is mostly important for the 
          raw video frames which the SDK allocates wrapper header frame memory.
        ***************************************************************************/
        
        if (_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].is_raw_av_streaming)
            ssdk_enable_raw_av( handle, 0, false, SDVR_VIDEO_RES_DECIMATION_NONE);

        err = sct_channel_close(pci_chan_handle);
        // Close the send channel
        err = sct_channel_close(pci_send_chan_handle);

//        if (err == SDVR_ERR_NONE) 
        {
            /*****************************************************************************
               Reset all the channel properties.
            *****************************************************************************/
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].is_dec_av_enable = 0;
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].is_smo_enable = 0;
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].is_raw_av_streaming = 0;
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].vsize_width = 0;
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].vsize_lines = 0;
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].user_data = 0;

            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].is_channel_running = 0;

            for (i = 0; i < SDVR_MAX_MD_REGIONS; i++)
                _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].md_defined_regions[i] = 0;

            for (i = 0; i < SDVR_MAX_BD_REGIONS; i++)
                _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].bd_defined_regions[i] = 0;

            for (i = 0; i < SDVR_MAX_PR_REGIONS; i++)
                _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pr_defined_regions[i] = 0;

            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pci_chan_handle = 0;
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pci_send_chan_handle = 0;

            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].sdk_chan_handle = 0;

            /********************************************************
              Re-set display properties
            *********************************************************/
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].ui_cb_func = NULL;
            if (_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].ui_cb_context)
            {
                ssdk_free_ui_context(_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].ui_cb_context);
            }
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].ui_cb_context = NULL;

            /********************************************************
                 free the raw video frame buffer queue.
            ********************************************************/
            if (_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.frame_queue != NULL)
            {
                free(_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.frame_queue);
            }
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.frame_queue = NULL;
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.start_item = -1;
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.top_item = 0;
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.num_recvd_frames = 0;
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].yuv_frame_queue.size = 0;

            /********************************************************
                 free the raw audio frame buffer queue.
            ********************************************************/
            if (_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.frame_queue != NULL)
            {
                free(_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.frame_queue);
            }
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.frame_queue = NULL;
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.start_item = -1;
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.top_item = 0;
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.num_recvd_frames = 0;
            _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].raw_audio_frame_queue.size = 0;            

        }
    }
    return err;
}

/****************************************************************************
  This function is called to close a channel of PIC communication
  between the firmware and the SDK.

  Parameters:

    "handle" - Handle of an encoder channel.
  
  Returns:

    SDVR_ERR_NONE - On success. Otherwise see the error code list.
****************************************************************************/
sdvr_err_e ssdk_drv_close_rcv(sdvr_chan_handle_t handle)
{
    sdvr_err_e err;
    int i;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sdvr_chan_type_e chan_type = CH_GET_CHAN_TYPE(handle);
    ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);
    sct_channel_t pci_chan_handle = enc_chan_info->pci_chan_handle;

    if (chan_type == SDVR_CHAN_TYPE_DECODER)
        return SDVR_ERR_INVALID_HANDLE;

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE){
        return err;
    }

    if (pci_chan_handle) 
    {
        /***************************************************************************
          Before closing the PCI channel we must make sure all the frames that
          were queued by the SDK are released. This is mostly important for the 
          raw video frame which the SDK allocates wrapper header frame memory.
        ***************************************************************************/
        for (i = 0; i < MAX_VIDEO_ENCODING; i++)
        {
            if (enc_chan_info->sub_enc_info[i].is_enc_av_streaming)
                ssdk_enable_enc_av( handle, i, false);
			if (enc_chan_info->is_raw_av_streaming[i])
				ssdk_enable_raw_av( handle, i, false, SDVR_VIDEO_RES_DECIMATION_NONE);
        }

        err = sct_channel_close(pci_chan_handle);

        if (err == SDVR_ERR_NONE) {
            ssdk_reset_enc_chan_info(handle);

            /***************************************************
                Delete the critical sections
            ***************************************************/
#ifdef WIN32
            DeleteCriticalSection(&enc_chan_info->cs_chan_rec);
#else
            pthread_mutex_destroy(&enc_chan_info->cs_chan_rec);
#endif
        }
    }
    return err;
}

/*************************************************************
  This function frees all the frame queue memory allocated for
  an encoder channel. Then, it re-sets all the encoder channel
  field. The only exception is critical section field.
*************************************************************/
void ssdk_reset_enc_chan_info(sdvr_chan_handle_t handle)
{
    int i;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sdvr_chan_type_e chan_type = CH_GET_CHAN_TYPE(handle);
    ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

    /*****************************************************************************
       Reset all the channel properties.
    *****************************************************************************/
	for (i = 0; i < MAX_VIDEO_ENCODING; i++)
		enc_chan_info->is_raw_av_streaming[i] = 0;
    enc_chan_info->is_smo_enable = 0;

    enc_chan_info->is_channel_running = 0;
    enc_chan_info->user_data = 0;

    enc_chan_info->vpp_action_flags = 0;

    for (i = 0; i < SDVR_MAX_MD_REGIONS; i++)
        enc_chan_info->md_defined_regions[i] = 0;

    for (i = 0; i < SDVR_MAX_BD_REGIONS; i++)
        enc_chan_info->bd_defined_regions[i] = 0;

    for (i = 0; i < SDVR_MAX_PR_REGIONS; i++)
        enc_chan_info->pr_defined_regions[i] = 0;

    enc_chan_info->pci_chan_handle = 0;

    enc_chan_info->sdk_chan_handle = 0;

    enc_chan_info->aenc_type = SDVR_AUDIO_ENC_NONE;

    /********************************************************
      Re-set display properties
    *********************************************************/
    enc_chan_info->ui_cb_func = NULL;
    if (enc_chan_info->ui_cb_context)
    {
          ssdk_free_ui_context(enc_chan_info->ui_cb_context);
    }
    enc_chan_info->ui_cb_context = NULL;

    
    /********************************************************
         free the raw video frame buffer queue.
    ********************************************************/
    if (enc_chan_info->yuv_frame_queue.frame_queue != NULL)
    {
        free(enc_chan_info->yuv_frame_queue.frame_queue);
    }
    enc_chan_info->yuv_frame_queue.frame_queue = NULL;
    enc_chan_info->yuv_frame_queue.start_item = -1;
    enc_chan_info->yuv_frame_queue.top_item = 0;
    enc_chan_info->yuv_frame_queue.num_recvd_frames = 0;
    enc_chan_info->yuv_frame_queue.size = 0;

    /********************************************************
         free the encoded A/V frame buffer queues.
    ********************************************************/
    for (i = 0; i < MAX_VIDEO_ENCODING; i++)
    {
        enc_chan_info->sub_enc_info[i].venc_type = SDVR_CHAN_TYPE_NONE;
        enc_chan_info->sub_enc_info[i].is_enc_av_streaming = 0;
        enc_chan_info->sub_enc_info[i].res_decimation = SDVR_VIDEO_RES_DECIMATION_NONE;
        enc_chan_info->sub_enc_info[i].has_sps_vol = 0;

        
        if (enc_chan_info->enc_av_frame_queue[i].frame_queue != NULL)
        {
            free(enc_chan_info->enc_av_frame_queue[i].frame_queue);
        }
        enc_chan_info->enc_av_frame_queue[i].frame_queue = NULL;
        enc_chan_info->enc_av_frame_queue[i].start_item = -1;
        enc_chan_info->enc_av_frame_queue[i].top_item = 0;
        enc_chan_info->enc_av_frame_queue[i].num_recvd_frames = 0;
        enc_chan_info->enc_av_frame_queue[i].size = 0;

    }

    /********************************************************
         free the raw audio frame buffer queue.
    ********************************************************/
    if (enc_chan_info->raw_audio_frame_queue.frame_queue != NULL)
    {
        free(enc_chan_info->raw_audio_frame_queue.frame_queue);
    }
    enc_chan_info->raw_audio_frame_queue.frame_queue = NULL;
    enc_chan_info->raw_audio_frame_queue.start_item = -1;
    enc_chan_info->raw_audio_frame_queue.top_item = 0;
    enc_chan_info->raw_audio_frame_queue.num_recvd_frames = 0;
    enc_chan_info->raw_audio_frame_queue.size = 0;
    enc_chan_info->is_raw_audio_streaming = 0;

    /********************************************************
         free the encoded audio frame buffer queue.
    ********************************************************/
    if (enc_chan_info->enc_audio_frame_queue.frame_queue != NULL)
    {
        free(enc_chan_info->enc_audio_frame_queue.frame_queue);
    }
    enc_chan_info->enc_audio_frame_queue.frame_queue = NULL;
    enc_chan_info->enc_audio_frame_queue.start_item = -1;
    enc_chan_info->enc_audio_frame_queue.top_item = 0;
    enc_chan_info->enc_audio_frame_queue.num_recvd_frames = 0;
    enc_chan_info->enc_audio_frame_queue.size = 0;

    /********************************************************
         free the motion values frame buffer queue.
    ********************************************************/
    if (enc_chan_info->motion_values_frame_queue.frame_queue != NULL)
    {
        free(enc_chan_info->motion_values_frame_queue.frame_queue);
    }
    enc_chan_info->motion_values_frame_queue.frame_queue = NULL;
    enc_chan_info->motion_values_frame_queue.start_item = -1;
    enc_chan_info->motion_values_frame_queue.top_item = 0;
    enc_chan_info->motion_values_frame_queue.num_recvd_frames = 0;
    enc_chan_info->motion_values_frame_queue.size = 0;

}
/****************************************************************************
  This function return a pointer to debuging frame structure of 
  Encoder/Decoder channel.

  NOTE: An encoder channel or a HMO only channel is treated the same.

  Parameters:

    "handle" - The Encoder/Decoder channel to get its debuging structure
  
  Returns:

    It returns a pointer to dbg_frame_count structure of Encoder/Decoder
    channel.
****************************************************************************/
ssdk_dbg_frame_count_t *ssdk_get_dbg_frame_count(sdvr_chan_handle_t handle)
{
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sdvr_chan_type_e  chan_type = CH_GET_CHAN_TYPE(handle) ;

    if (chan_type!= SDVR_CHAN_TYPE_DECODER)
        return &_ssdvr_boards_info.boards[board_index].recv_channels[chan_num].dbg_frame_count;
    else
        return &_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].dbg_frame_count;
}

/****************************************************************************
  This function maps the channel type which SDK understand to job type
  which firmware deals with.

  Parameters:

    "chan_type" - the channel type to get its corresponding job type.
  
  Returns:

    It returns the corresponding job type for the given channel type.

****************************************************************************/
dvr_job_type_e ssdk_chan_to_job_type(sdvr_chan_type_e chan_type)
{
    dvr_job_type_e job_type = DVR_JOB_CAMERA_ENCODE;

    switch (chan_type) 
    {
        case SDVR_CHAN_TYPE_ENCODER:
            job_type = DVR_JOB_CAMERA_ENCODE;
            break;
        case SDVR_CHAN_TYPE_DECODER:
            job_type = DVR_JOB_HOST_DECODE;

            break;
        default:
            break;
    }
    return job_type;
}

/****************************************************************************
  This function maps the SDK video format to the firmware video format enum.

  Parameters:

    "video_format" - the SDK video format to get its corresponding firmware 
    enum
  
  Returns:

    Returns the firmware video format enum value
****************************************************************************/
dvr_vc_format_e ssdk_to_fw_video_frmt(sdvr_venc_e video_format)
{
    dvr_vc_format_e vc_format;

    switch (video_format) 
    {
        case SDVR_VIDEO_ENC_H264:
            vc_format = DVR_VC_FORMAT_H264;
            break;

        case SDVR_VIDEO_ENC_JPEG:
            vc_format = DVR_VC_FORMAT_JPEG;
            break;

        case SDVR_VIDEO_ENC_MPEG4:
            vc_format = DVR_VC_FORMAT_MPEG4;
            break;

        case SDVR_VIDEO_ENC_NONE:
        default:
           vc_format = DVR_VC_FORMAT_NONE;
            break;
    }

    return vc_format;
}
/****************************************************************************
  This function maps the SDK aduio format to the firmware aduio format enum.

  Parameters:

    "audio_format" - the SDK audio format to get its corresponding firmware 
    enum
  
  Returns:

    Returns the firmware audio format enum value
****************************************************************************/
dvr_ac_format_e ssdk_to_fw_audio_frmt(sdvr_aenc_e audio_format)
{
    dvr_ac_format_e ac_format;
    switch (audio_format) 
    {
        case SDVR_AUDIO_ENC_G711:
            ac_format = DVR_AC_FORMAT_G711;
            break;

        case SDVR_AUDIO_ENC_G726_16K:
            ac_format = DVR_AC_FORMAT_G726_16K;
            break;

        case SDVR_AUDIO_ENC_G726_24K:
            ac_format = DVR_AC_FORMAT_G726_24K;
            break;
        case SDVR_AUDIO_ENC_G726_32K:
            ac_format = DVR_AC_FORMAT_G726_32K;
            break;
        case SDVR_AUDIO_ENC_G726_48K:
            ac_format = DVR_AC_FORMAT_G726_48K;
            break;

        case SDVR_AUDIO_ENC_NONE:
        default:
           ac_format = DVR_AC_FORMAT_NONE;
            break;
    }
    return ac_format;
}

/****************************************************************************
    Wrapper function for SDK mutex lock.
****************************************************************************/
void _ssdk_mutex_lock()
{
#ifdef WIN32
    EnterCriticalSection( &_ssdvr_boards_info.cs_board_info );
#else
    pthread_mutex_lock( &_ssdvr_boards_info.cs_board_info );
#endif
}

/****************************************************************************
    Wrapper function for SDK mutex unlock.
****************************************************************************/
void _ssdk_mutex_unlock()
{
#ifdef WIN32
    LeaveCriticalSection( &_ssdvr_boards_info.cs_board_info );
#else
    pthread_mutex_unlock(&_ssdvr_boards_info.cs_board_info );
#endif
}

/****************************************************************************
    Wrapper function for decode buffer mutex lock.
****************************************************************************/
void _ssdk_mutex_decode_buf_lock()
{
#ifdef WIN32
    EnterCriticalSection( &_ssdvr_boards_info.cs_decode_buf_count );
#else
    pthread_mutex_lock(&_ssdvr_boards_info.cs_decode_buf_count);
#endif
}

/****************************************************************************
    Wrapper function for decode buffer mutex unlock.
****************************************************************************/
void _ssdk_mutex_decode_buf_unlock()
{
#ifdef WIN32
    LeaveCriticalSection( &_ssdvr_boards_info.cs_decode_buf_count );
#else
    pthread_mutex_unlock(&_ssdvr_boards_info.cs_decode_buf_count);
#endif
}

/****************************************************************************
    Lock the mutex used to serialize the writing to file and channel recording
    state.

   NOTE: This routine assumes the handle is a valid encoder channel handle.
****************************************************************************/
void _ssdk_mutex_chan_rec_lock(sdvr_chan_handle_t handle)
{
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

#ifdef WIN32
    EnterCriticalSection( &enc_chan_info->cs_chan_rec );
#else
    pthread_mutex_lock( &enc_chan_info->cs_chan_rec );
#endif
}

/****************************************************************************
    Unlock the mutex used to serialize the writing to file and channel recording
    state.

   NOTE: This routine assumes the handle is a valid encoder channel handle.
****************************************************************************/
void _ssdk_mutex_chan_rec_unlock(sdvr_chan_handle_t handle)
{
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

#ifdef WIN32
    LeaveCriticalSection( &enc_chan_info->cs_chan_rec );
#else
    pthread_mutex_unlock(&enc_chan_info->cs_chan_rec );
#endif
}

/****************************************************************************
    Returns the SDK debug flags.
****************************************************************************/
sx_uint32 _ssdk_get_debug_flag()
{
    return m_sdkParams.debug_flag;
}

/****************************************************************************
  This function returns the channel handle corresponding to the given a 
  board_index, channel type, and the channel number.

  Format of a channel is:
    Channel Number: the first 8 bits of the handle
    Channel Type:   Starts at the bit 8 of handle and its size is
                    4 bits.
    Board Index:    Sarts at the bit 12 of handle and its size is
                    4 bits.

    Channel signiture: The high 16 bit is CHAN_HANDLE_SIGNITURE

  Parameters:
    board_index:    The index of the board that this channel was created on.

    chan_type:      The type of the channel to

    chan_num:       The encoder/decoder channel ID

  Returns:
    "handle" - Channel Handle of the given encoder/decoder channel on
               the given board.
****************************************************************************/
sdvr_chan_handle_t ssdk_create_chan_handle(sx_uint8 board_index,
                                           sdvr_chan_type_e  chan_type,
                                           sx_uint8          chan_num)
{
    sdvr_chan_handle_t handle;

    handle = DVR_JOB_NUM(board_index, chan_type, chan_num) & 0x0000FFFF;
    handle = handle | (CHAN_HANDLE_SIGNITURE << 16);

    return handle;
}

/****************************************************************************
  This function makes sure the channel is stopped and returns the previous
  running status of the channel


  Parameters:
    handle:    An Encoder/Decoder channel handle to stop its job running status.

    was_contrl_running: The running status of the channel prior to this call.

  Returns:
    SDVR_ERR_NONE - Success

    DVR_FRMW_CHANNEL_NOT_CREATED - The given channel handle was not created

    DVR_FRMW_CHANNEL_NOT_DISABLED - Could not stop the channel
****************************************************************************/
sdvr_err_e ssdk_disable_channel(sdvr_chan_handle_t handle, sx_bool *was_contrl_running)
{
    dvr_control_t cntrl_strct;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    cntrl_strct.job_type =  CH_GET_CHAN_TYPE(handle);
    cntrl_strct.job_id =  CH_GET_CHAN_NUM(handle);

    ssdk_message_chan_send(handle, DVR_GET_CONTROL, &cntrl_strct, sizeof(cntrl_strct));
    ssdk_message_chan_recv(handle, DVR_REP_CONTROL, &cntrl_strct, 0, 0);
    if (cntrl_strct.status != DVR_STATUS_OK)
    {
        sdbg_msg_output_err(DVR_GET_CONTROL, cntrl_strct.status);
        return cntrl_strct.status;
    }

    /***********************************************************************
            NOTE: No attributes of the channel can be changed while the 
                  channel is running. In this case, we must temp. disable
                  the channel, make the change and re-enable it. 
            3. Make sure the channel is disabled
    ************************************************************************/
    *was_contrl_running = cntrl_strct.enable; 
    if (*was_contrl_running)
    {
        cntrl_strct.enable = false;
        ssdk_message_chan_send(handle, DVR_SET_CONTROL, &cntrl_strct, sizeof(cntrl_strct));
        ssdk_message_chan_recv(handle, DVR_REP_CONTROL, &cntrl_strct, 0, 0);
        if (cntrl_strct.status != DVR_STATUS_OK)
            sdbg_msg_output_err(DVR_SET_CONTROL, cntrl_strct.status);
    }
    return cntrl_strct.status;
}

/****************************************************************************
  This function enables the given channel for streaming.

  Parameters:
    handle:    An Encoder/Decoder channel handle to be enabled.

  Returns:
    SDVR_ERR_NONE - Success

    DVR_FRMW_CHANNEL_NOT_CREATED - The given channel handle was not created

    DVR_FRMW_CHANNEL_NOT_ENABLED - Could not start the channel
****************************************************************************/
sdvr_err_e ssdk_enable_channel(sdvr_chan_handle_t handle)
{
    dvr_control_t cntrl_strct;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num;

    cntrl_strct.job_type =  CH_GET_CHAN_TYPE(handle);
    chan_num = cntrl_strct.job_id =  CH_GET_CHAN_NUM(handle);

    cntrl_strct.enable = true;
    ssdk_message_chan_send(handle, DVR_SET_CONTROL, &cntrl_strct, sizeof(cntrl_strct));
    ssdk_message_chan_recv(handle, DVR_REP_CONTROL, &cntrl_strct, 0, 0);
    if (cntrl_strct.status != DVR_STATUS_OK)
        sdbg_msg_output_err(DVR_SET_CONTROL, cntrl_strct.status);

    return cntrl_strct.status;
}
/****************************************************************************
  This function sets the raw A/V streaming flag for the given channel
  so that we can accept or reject the raw A/V frames approporiatly in the
  call back.

  Additionally, if we stopped the streaming of raw video it will empty 
  the raw video queue and returns any frames in the queue to the driver.

  Parameters:
    handle:    A Encoder/Decoder channel handle to be enabled.

    "res_decimation" - Specifies whether the video is streamed at
    the system-wide maximum resolution or 1/4th or 1/16th of that 
    resolution.

    enable:    The current state of A/V streaming on this channel

  Returns:
    None.
****************************************************************************/
void ssdk_enable_raw_av(sdvr_chan_handle_t handle, sx_uint8 stream_id, sx_bool enable,
                        sdvr_video_res_decimation_e res_decimation)
{
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num =  CH_GET_CHAN_NUM(handle);
    sdvr_chan_type_e chan_type = CH_GET_CHAN_TYPE(handle);
    sdvr_err_e err;
    sdvr_yuv_buffer_t *yuv_buffer;
    sdvr_av_buffer_t *av_buffer;
    ssdk_dbg_frame_count_t *dbg_frame_count = ssdk_get_dbg_frame_count(handle);
    ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);


    if (!enable)
    {

#if 0
// Do not need to remove these buffers since they will be ignored when received.
        /******************************************************************
            Makes sure to flush the YUV interal SDK queue when we
            stop the raw video streaming. Additionaly, we must flushout
            any raw audio
        ******************************************************************/ 
        do {
            err = sdvr_get_yuv_buffer(handle,&yuv_buffer);
            if (err == SDVR_ERR_NONE)
                sdvr_release_yuv_buffer(yuv_buffer);
        } while (err == SDVR_ERR_NONE);
#endif
 
        do {
            err = sdvr_get_av_buffer(handle,SDVR_FRAME_RAW_AUDIO,&av_buffer);

            if (err == SDVR_ERR_NONE)
                sdvr_release_av_buffer(av_buffer);
        } while (err == SDVR_ERR_NONE);


        /****************************************************************
           Since the raw video stopped reset the raw audio streaming
        *****************************************************************/
        if (chan_type != SDVR_CHAN_TYPE_DECODER)
        {
            enc_chan_info->is_raw_audio_streaming = 0;

        }
    }
    else
    {
        dbg_frame_count->y_u_vBufferRcvd = 0;
        dbg_frame_count->y_u_vBuffersFreed = 0;
        dbg_frame_count->yuvFrameRcvd = 0;
        dbg_frame_count->yuvFrameFreed = 0;
        dbg_frame_count->yuvMissedCount = 0;
        dbg_frame_count->yuvFrameAppRcvd = 0;
        dbg_frame_count->yuvFrameAppFreed = 0;
    }

    // enable the streaming after we set the raw video lines/width otherwise we could crash
    // because as we receive video stream we don't have valid values for trimming the 
    // raw video which will cause a crash.
    if (chan_type != SDVR_CHAN_TYPE_DECODER)
        enc_chan_info->is_raw_av_streaming[stream_id] = enable;
    else
        _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].is_raw_av_streaming = enable;

}

/****************************************************************************
  This function sets the encoded A/V streaming flag for the requested encoder
  stream of given channel
  so that we can accpet or reject the encoded A/V frame approporiatly in the
  call back.

  Additionally, if we stopped the streaming of encoded video it will empty 
  the encoded audio queue and returns any frames in the queue to the driver.

  Parameters:
    handle:    An Encoded channel handle to be enabled.

    sub_chan_enc - The sub-channel encoder to enable/disable

    enable:    The current state of A/V streaming on this channel

  Returns:
    None.
****************************************************************************/
void ssdk_enable_enc_av(sdvr_chan_handle_t handle, 
                        sdvr_sub_encoders_e sub_chan_enc,
                        sx_bool enable)
{
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num =  CH_GET_CHAN_NUM(handle);
    sdvr_err_e err;
    sdvr_av_buffer_t *av_buffer;
    ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

    enc_chan_info->sub_enc_info[sub_chan_enc].is_enc_av_streaming = enable;

    if (!enable)
    {
        do {
            err = sdvr_get_av_buffer(handle, 
                (sub_chan_enc == SDVR_ENC_PRIMARY ? SDVR_FRAME_VIDEO_ENCODED_PRIMARY : SDVR_FRAME_VIDEO_ENCODED_SECONDARY),
                &av_buffer);
            if (err == SDVR_ERR_NONE)
                sdvr_release_av_buffer(av_buffer);
        } while (err == SDVR_ERR_NONE);
    }
    else
    {
        if (!enc_chan_info->sub_enc_info[0].is_enc_av_streaming &&
            !enc_chan_info->sub_enc_info[1].is_enc_av_streaming)
        {
            enc_chan_info->dbg_frame_count.eFrameAppRcvd = 0;
            enc_chan_info->dbg_frame_count.eFrameAppFreed = 0;
            enc_chan_info->dbg_frame_count.eFrameRcvd = 0;
            enc_chan_info->dbg_frame_count.eFrameFreed = 0;
        }
    }
}
/****************************************************************************
    This function enables or disables the channel as needed.
    If channel must be started if the existing running state
    differs from the streaming state. Otherwise, the channel
    state remains the same.

    Parameters:
        handle:    the channel handle to enable/disable

    Returns:
        SDVR_ERR_NONE - No error
        Otherwise various errors indicating the problem
****************************************************************************/
sdvr_err_e ssdvr_enable_chan(sdvr_chan_handle_t handle)
{
    dvr_control_t cntrl_strct;

    cntrl_strct.job_type = CH_GET_CHAN_TYPE(handle);
    cntrl_strct.job_id =  CH_GET_CHAN_NUM(handle);

    /*********************************************************************
       The channel needs to be started the it was stopped but now
       it is streaming. Also it needs to be stopped if it was running
       but no longer any Video is streaming.
       Otherwise, the channel running state should remain the same.
    **********************************************************************/

    if (ssdk_is_chan_running(handle) && !ssdk_is_chan_streaming(handle))
        cntrl_strct.enable = false;
    else if (!ssdk_is_chan_running(handle) && ssdk_is_chan_streaming(handle))
        cntrl_strct.enable = true;

    if ((ssdk_is_chan_running(handle) && !ssdk_is_chan_streaming(handle)) ||
        (!ssdk_is_chan_running(handle) && ssdk_is_chan_streaming(handle)))
    {
        ssdk_message_chan_send(handle, DVR_SET_CONTROL, &cntrl_strct, sizeof(cntrl_strct));
        ssdk_message_chan_recv(handle, DVR_REP_CONTROL, &cntrl_strct, 0, 0);
        if (cntrl_strct.status != DVR_STATUS_OK)
        {
            sdbg_msg_output_err(DVR_SET_CONTROL, cntrl_strct.status);
            return cntrl_strct.status;
        }
    }
    return SDVR_ERR_NONE;
}
/****************************************************************************
  This function sends the given channel parameter to the firmware.
  Since no attributes of a channel can be changed while it is running,
  this function first makes sure the channel is stopped, then, it sends
  the command to the firmware and re-starts the channel


  Parameters:
    handle:    the channel handle to change its parameters

    command - the command to send to the firmware.
              NOTE: The assumption is that the ID of the
              reply for this command is "command + 1"

    data:	   the buffer data to send to for this command

    data_size: the size of the data to send

  Returns:
    SDVR_ERR_NONE - No error
    Otherwise various errors indicating the probelm
****************************************************************************/
sdvr_err_e	ssdvr_send_chan_param(sdvr_chan_handle_t handle, sx_uint32 command, void *data, sx_uint32 data_size)
{
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_bool		was_contrl_running;
    sdvr_err_e err;

    /************************************************************************
        1. Get the existing running state of the channel and disable it
           if it was running.
            
    ************************************************************************/
    if ((err = ssdk_disable_channel(handle, &was_contrl_running)) != SDVR_ERR_NONE)
        return err;

    /*********************************************************************
     Send the command to the board
     NOTE: We are assuming the reply command ID is right after the set command
    *********************************************************************/
    ssdk_message_chan_send(handle, command, data, data_size);
    ssdk_message_chan_recv(handle, command + 1, data, 0, 0);

    /*******************************************************************************
        Final step is to re-enable the channel if it was running
    *******************************************************************************/
    if (was_contrl_running)
    {
        err = ssdk_enable_channel(handle);
        if (err != SDVR_ERR_NONE )
            return err;
    }

    return SDVR_ERR_NONE;
}

/*****************************************************************************
   2-byte (16-bit) number Convert "Little-Endian" to "Big-Endian" and vise versa
******************************************************************************/
sx_int16 _int16_little_endian_TO_big_endian(sx_int16 nValue)
{
    return ((nValue>>8)&0x00ff)+((nValue << 8)&0xff00);
}

/*****************************************************************************
  4-byte (32-bit( number Convert "Little-Endian" to "Big-Endian" and vise versa
******************************************************************************/

sx_int32 _int32_little_endian_TO_big_endian(sx_int32 nLongNumber)
{
   return (((nLongNumber&0x000000FF)<<24)+((nLongNumber&0x0000FF00)<<8)+
   ((nLongNumber&0x00FF0000)>>8)+((nLongNumber&0xFF000000)>>24));
}

/*****************************************************************************
  This function converts each 16 & 32 bit field within the given message
  class data structure between "Little-Endian" to "Big-Endian".
******************************************************************************/

void  _ssdk_do_message_byte_swap(sx_uint32 board_index, sx_int32  msg_class, void * buffer)
{
    switch (msg_class) {
    case DVR_GET_VER_INFO: 
    case DVR_REP_VER_INFO:
    {
        dvr_ver_info_t *cmd_strct = (dvr_ver_info_t *)buffer;

        cmd_strct->build_year = _int16_little_endian_TO_big_endian(cmd_strct->build_year);

        break;
    }
    case DVR_GET_CODEC_INFO: 
    case DVR_REP_CODEC_INFO:
    {
        dvr_codec_info_t *cmd_strct = (dvr_codec_info_t *)buffer;

        cmd_strct->camera_type = _int16_little_endian_TO_big_endian(cmd_strct->camera_type);
        cmd_strct->encode_fps = _int16_little_endian_TO_big_endian(cmd_strct->encode_fps);
        cmd_strct->decode_fps = _int16_little_endian_TO_big_endian(cmd_strct->decode_fps);

        break;
    }

    case DVR_SET_BOARD: 
    case DVR_GET_BOARD:      
    case DVR_REP_BOARD:
    {
        dvr_board_t *cmd_strct = (dvr_board_t *)buffer;

        cmd_strct->camera_info = _int16_little_endian_TO_big_endian(cmd_strct->camera_info);
        cmd_strct->camera_type = _int16_little_endian_TO_big_endian(cmd_strct->camera_type);

        break;
    }
    case DVR_SET_JOB:    
    case DVR_GET_JOB:        
    case DVR_REP_JOB:        
    {
        dvr_job_t *cmd_strct = (dvr_job_t *)buffer;

        cmd_strct->camera_type = _int16_little_endian_TO_big_endian(cmd_strct->camera_type);

        break;
    }

    case DVR_SET_SMO: 
    case DVR_GET_SMO:  
    case DVR_REP_SMO:
    {
        dvr_smo_t *cmd_strct = (dvr_smo_t *)buffer;

        cmd_strct->position_x = _int16_little_endian_TO_big_endian(cmd_strct->position_x);
        cmd_strct->position_y = _int16_little_endian_TO_big_endian(cmd_strct->position_y);

        break;
    }
    case DVR_SET_HMO: 
    case DVR_GET_HMO:  
    case DVR_REP_HMO:
    {
        dvr_hmo_t *cmd_strct = (dvr_hmo_t *)buffer;
        
        /**************************
          No Converstion is needed.
        **************************/
        break;
    }
    case DVR_SET_OSD:  
    case DVR_GET_OSD: 
    case DVR_REP_OSD:
    {
        dvr_osd_t *cmd_strct = (dvr_osd_t *)buffer;
        
        /**************************
          No Converstion is needed.
        **************************/

        break;
    }
    case DVR_SET_PR: 
    case DVR_GET_PR:   
    case DVR_REP_PR:
    {
        dvr_pr_t *cmd_strct = (dvr_pr_t *)buffer;

        switch (cmd_strct->op_code)
        {
        case DVR_VPP_OPCODE_CONTROL:
            /**************************
              No Converstion is needed.
            **************************/

            break;

        case DVR_VPP_OPCODE_REGIONS:
            cmd_strct->u1.reg.x_TL = _int16_little_endian_TO_big_endian(cmd_strct->u1.reg.x_TL);
            cmd_strct->u1.reg.y_TL = _int16_little_endian_TO_big_endian(cmd_strct->u1.reg.y_TL);
            cmd_strct->u1.reg.x_BR = _int16_little_endian_TO_big_endian(cmd_strct->u1.reg.x_BR);
            cmd_strct->u1.reg.y_BR = _int16_little_endian_TO_big_endian(cmd_strct->u1.reg.y_BR);

            break;

        default:
            break;
        }
        break;
    }
    case DVR_SET_MD:         
    case DVR_GET_MD:   
    case DVR_REP_MD:
    {
        dvr_md_t *cmd_strct = (dvr_md_t *)buffer;

        switch (cmd_strct->op_code)
        {
        case DVR_VPP_OPCODE_CONTROL:
            /**************************
              No Converstion is needed.
            **************************/

            break;

        case DVR_VPP_OPCODE_REGIONS:
            cmd_strct->u1.reg.x_TL = _int16_little_endian_TO_big_endian(cmd_strct->u1.reg.x_TL);
            cmd_strct->u1.reg.y_TL = _int16_little_endian_TO_big_endian(cmd_strct->u1.reg.y_TL);
            cmd_strct->u1.reg.x_BR = _int16_little_endian_TO_big_endian(cmd_strct->u1.reg.x_BR);
            cmd_strct->u1.reg.y_BR = _int16_little_endian_TO_big_endian(cmd_strct->u1.reg.y_BR);

            break;

        default:
            break;
        }

        break;
    }
    case DVR_SET_BD: 
    case DVR_GET_BD:   
    case DVR_REP_BD:
    {
        dvr_bd_t *cmd_strct = (dvr_bd_t *)buffer;

        switch (cmd_strct->op_code)
        {
        case DVR_VPP_OPCODE_CONTROL:
            /**************************
              No Converstion is needed.
            **************************/

            break;

        case DVR_VPP_OPCODE_REGIONS:
            cmd_strct->u1.reg.x_TL = _int16_little_endian_TO_big_endian(cmd_strct->u1.reg.x_TL);
            cmd_strct->u1.reg.y_TL = _int16_little_endian_TO_big_endian(cmd_strct->u1.reg.y_TL);
            cmd_strct->u1.reg.x_BR = _int16_little_endian_TO_big_endian(cmd_strct->u1.reg.x_BR);
            cmd_strct->u1.reg.y_BR = _int16_little_endian_TO_big_endian(cmd_strct->u1.reg.y_BR);

            break;

        default:
            break;
        }

        break;
    }
    case DVR_SET_ND: 
    case DVR_GET_ND:   
    case DVR_REP_ND:
    {
        dvr_nd_t *cmd_strct = (dvr_nd_t *)buffer;

        /**************************
          No Converstion is needed.
        **************************/

        break;
    }
    case DVR_SET_RELAYS:
    case DVR_GET_RELAYS:  
    case DVR_REP_RELAYS:
    {
        dvr_relay_t *cmd_strct = (dvr_relay_t *)buffer;

        cmd_strct->relay_status = _int32_little_endian_TO_big_endian(cmd_strct->relay_status);

        break;
    }
    case DVR_SET_SENSORS: 
    case DVR_GET_SENSORS: 
    case DVR_REP_SENSORS:
    {
        dvr_sensor_t *cmd_strct = (dvr_sensor_t *)buffer;

        cmd_strct->sensor_status = _int32_little_endian_TO_big_endian(cmd_strct->sensor_status);
        cmd_strct->sensor_enable = _int32_little_endian_TO_big_endian(cmd_strct->sensor_enable);
        cmd_strct->edge_triggered = _int32_little_endian_TO_big_endian(cmd_strct->edge_triggered);

        break;
    }
    case DVR_SET_WATCHDOG: 
    case DVR_GET_WATCHDOG: 
    case DVR_REP_WATCHDOG:
    {
        dvr_watchdog_t *cmd_strct = (dvr_watchdog_t *)buffer;

        cmd_strct->enable = _int32_little_endian_TO_big_endian(cmd_strct->enable);

        break;
    }
    case DVR_SET_CONTROL:
    case DVR_GET_CONTROL:  
    case DVR_REP_CONTROL:
    {
        dvr_control_t *cmd_strct = (dvr_control_t *)buffer;

        /**************************
          No Converstion is needed.
        **************************/

        break;
    }
    case DVR_SET_TIME: 
    case DVR_GET_TIME:  
    case DVR_REP_TIME:
    {
        dvr_time_t *cmd_strct = (dvr_time_t *)buffer;

        cmd_strct->tv_sec = _int32_little_endian_TO_big_endian(cmd_strct->tv_sec);
        cmd_strct->tv_usec = _int32_little_endian_TO_big_endian(cmd_strct->tv_usec);
        break;
    }
    case DVR_SET_ENCODE:
    case DVR_GET_ENCODE:
    case DVR_REP_ENCODE:
    {
        dvr_encode_info_t *cmd_strct = (dvr_encode_info_t *)buffer;
        ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, cmd_strct->job_id);

        switch (enc_chan_info->sub_enc_info[cmd_strct->stream_id].venc_type)
        {
        case SDVR_VIDEO_ENC_H264:
            cmd_strct->u1.h264.avg_bitrate = _int16_little_endian_TO_big_endian(cmd_strct->u1.h264.avg_bitrate);
            cmd_strct->u1.h264.max_bitrate = _int16_little_endian_TO_big_endian(cmd_strct->u1.h264.max_bitrate);
            break;

        case SDVR_VIDEO_ENC_MPEG4:
            cmd_strct->u1.mpeg4.avg_bitrate = _int16_little_endian_TO_big_endian(cmd_strct->u1.mpeg4.avg_bitrate);
            cmd_strct->u1.mpeg4.max_bitrate = _int16_little_endian_TO_big_endian(cmd_strct->u1.mpeg4.max_bitrate);
            break;

        case SDVR_VIDEO_ENC_JPEG:
            cmd_strct->u1.jpeg.quality = _int16_little_endian_TO_big_endian(cmd_strct->u1.jpeg.quality);
            break;

        case SDVR_VIDEO_ENC_NONE:
            break;
        default:
            break;
        }

        break;
    }
    case DVR_SET_ENC_ALARM:
    case DVR_GET_ENC_ALARM:
    case DVR_REP_ENC_ALARM:
    {
        dvr_enc_alarm_info_t *cmd_strct = (dvr_enc_alarm_info_t *)buffer;
        ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, cmd_strct->job_id);

        switch (enc_chan_info->sub_enc_info[cmd_strct->stream_id].venc_type)
        {
        case SDVR_VIDEO_ENC_H264:
            cmd_strct->u1.h264.avg_bitrate = _int16_little_endian_TO_big_endian(cmd_strct->u1.h264.avg_bitrate);
            cmd_strct->u1.h264.max_bitrate = _int16_little_endian_TO_big_endian(cmd_strct->u1.h264.max_bitrate);
            break;

        case SDVR_VIDEO_ENC_MPEG4:
            cmd_strct->u1.mpeg4.avg_bitrate = _int16_little_endian_TO_big_endian(cmd_strct->u1.mpeg4.avg_bitrate);
            cmd_strct->u1.mpeg4.max_bitrate = _int16_little_endian_TO_big_endian(cmd_strct->u1.mpeg4.max_bitrate);
            break;

        case SDVR_VIDEO_ENC_JPEG:
            cmd_strct->u1.jpeg.quality = _int16_little_endian_TO_big_endian(cmd_strct->u1.jpeg.quality);
            break;

        case SDVR_VIDEO_ENC_NONE:
            break;
        default:
            break;
        }

        break;
    }
    case DVR_SET_DECODE:
    case DVR_GET_DECODE:
    case DVR_REP_DECODE:
    {
        dvr_decode_info_t *cmd_strct = (dvr_decode_info_t *)buffer;

        cmd_strct->width = _int16_little_endian_TO_big_endian(cmd_strct->width);
        cmd_strct->height = _int16_little_endian_TO_big_endian(cmd_strct->height);

        break;
    }
    case DVR_SET_FONT_TABLE:
    case DVR_REP_FONT_TABLE:
    {
        dvr_font_table_t *cmd_strct = (dvr_font_table_t *)buffer;

        cmd_strct->port_id = _int16_little_endian_TO_big_endian(cmd_strct->port_id);
        cmd_strct->osdheight = _int16_little_endian_TO_big_endian(cmd_strct->osdheight);

        // NOTE: Since osdyysize is union with buf size, there is no need
        //       for converstion
        cmd_strct->u1.bufsize = _int32_little_endian_TO_big_endian(cmd_strct->u1.bufsize);
        break;
    }
    case DVR_SET_OSD_EX:   
    case DVR_GET_OSD_EX: 
    case DVR_REP_OSD_EX: 
    {
        dvr_osd_ex_t *cmd_strct = (dvr_osd_ex_t *)buffer;

        switch (cmd_strct->op_code)
        {
        case DVR_OSD_OPCODE_CONFIG:
            cmd_strct->u1.config.x_TL = _int16_little_endian_TO_big_endian(cmd_strct->u1.config.x_TL);
            cmd_strct->u1.config.y_TL = _int16_little_endian_TO_big_endian(cmd_strct->u1.config.y_TL);

            break;
        case DVR_OSD_OPCODE_SHOW:
            /**************************
              No Converstion is needed.
            **************************/

            break;
        case DVR_OSD_OPCODE_TEXT:
        {
            int i;
            int data_size = sizeof(cmd_strct->u1.text.data) / 2;

            for (i = 0; i < data_size; i++)
                cmd_strct->u1.text.data[i] = _int16_little_endian_TO_big_endian(cmd_strct->u1.text.data[i]);

            break;
        }
        case DVR_OSD_OPCODE_CLEAR_TEXT:
            /**************************
              No Converstion is needed.
            **************************/

            break;
        default:
            break;

        }
        break;
    }
    case DVR_SET_UART:
    case DVR_GET_UART:
    case DVR_REP_UART:
    {
        dvr_uart_t *cmd_strct = (dvr_uart_t *)buffer;
        switch (cmd_strct->cmd)
        {
        case DVR_UART_CMD_CONFIG:
            cmd_strct->u1.config.baud_rate = _int32_little_endian_TO_big_endian(cmd_strct->u1.config.baud_rate);
            break;
        case DVR_UART_CMD_OUTPUT:
            /**************************
              No Converstion is needed.
            **************************/

            break;
        default:
            break;

        }

        break;
    }
    case DVR_SET_IOCTL:
    case DVR_GET_IOCTL:
    case DVR_REP_IOCTL:
    {
        dvr_ioctl_t *cmd_strct = (dvr_ioctl_t *)buffer;
        switch (cmd_strct->ioctl_code)
        {
        case DVR_IOCTL_CODE_IMG_CONTROL:
            cmd_strct->u1.img_ctrl.flags = _int16_little_endian_TO_big_endian(cmd_strct->u1.img_ctrl.flags);
            break;
        case DVR_IOCTL_CODE_DECODER_REGS:
            cmd_strct->u1.reg.val = _int16_little_endian_TO_big_endian(cmd_strct->u1.reg.val);

            break;
        case DVR_IOCTL_CODE_OPEN_CHAN:
        case DVR_IOCTL_CODE_CLOSE_CHAN:
            cmd_strct->u1.chan.port = _int16_little_endian_TO_big_endian(cmd_strct->u1.chan.port);
            break;
        case DVR_IOCTL_CODE_MOTION_FRAME_FREQ:
            /**************************
              No Converstion is needed.
            **************************/
            break;

        default:
            break;

        }

        break;
    }
    case DVR_SET_AUTH_KEY:
    case DVR_GET_AUTH_KEY:
    case DVR_REP_AUTH_KEY:
    {
        /**************************
          No Converstion is needed.
        **************************/

        break;
    }
    case DVR_SET_REGIONS_MAP:
    case DVR_REP_REGIONS_MAP:
    {
        dvr_regions_map_t *cmd_strct = (dvr_regions_map_t *)buffer;
        switch (cmd_strct->opcode)
        {
        case DVR_REGION_OPCODE_CONFIG:
            /**************************
              No Converstion is needed.
            **************************/

            break;
        case DVR_REGION_OPCODE_ENABLE:
            /**************************
              No Converstion is needed.
            **************************/

            break;
        case DVR_REGION_OPCODE_START:
        case DVR_REGION_OPCODE_FINISH:
        {
            cmd_strct->u1.map.map_size = _int16_little_endian_TO_big_endian(cmd_strct->u1.map.map_size);

            break;
        }
        default:
            break;
        }

        break;
    }

    case DVR_SET_VIDEO_OUTPUT:
    case DVR_REP_VIDEO_OUTPUT:
    {
        dvr_videoout_t *cmd_strct = (dvr_videoout_t *)buffer;

        switch (cmd_strct->opcode)
        {
        case DVR_VOUT_OPCODE_START:
        {
            cmd_strct->u1.start.sct_port_id = _int32_little_endian_TO_big_endian(cmd_strct->u1.start.sct_port_id);
            cmd_strct->u1.start.video_format = _int16_little_endian_TO_big_endian(cmd_strct->u1.start.video_format);

            break;
        }

        case DVR_VOUT_OPCODE_STOP:
            /**************************
              No Converstion is needed.
            **************************/

            break;

        default:
            break;
        }


        break;
    }


    case DVR_SIG_HOST:
    {
        dvr_sig_host_t *cmd_strct = (dvr_sig_host_t *)buffer;

        cmd_strct->signal_data = _int32_little_endian_TO_big_endian(cmd_strct->signal_data);
        cmd_strct->extra_data = _int32_little_endian_TO_big_endian(cmd_strct->extra_data);


        break;
    }

    case DVR_FW_INTERNAL:
    {
        /**************************
          No Converstion is needed.
        **************************/

        break;
    }

    default:
        break;
    }

}

/***************************************************************************
 Wrapper to the sct_message_send(). See the sct documentation for the
 description and usage.
****************************************************************************/
sdvr_err_e ssdk_message_chan_send(sdvr_chan_handle_t handle, sx_int32  msg_class, void * buffer, sx_int32 buf_size )
{
    return _ssdk_message_send(handle, 0xFF, msg_class, buffer, buf_size);
}
sdvr_err_e ssdk_message_board_send(sx_uint8 b_index,  sx_int32  msg_class, void * buffer, sx_int32 buf_size )
{
    return _ssdk_message_send(0, b_index, msg_class, buffer, buf_size);
}
sdvr_err_e _ssdk_message_send(sdvr_chan_handle_t handle, sx_uint8 b_index, sx_int32  msg_class, void * buffer, sx_int32 buf_size )
{
    sx_uint32 err;
    sx_uint8 board_index;
    sx_uint8 temp_buffer[SCT_MAX_MESSAGE_LEN];

    /*
        Make a copy of the input buffer, so that we perform all the 
        little endean/big endean byte order changes to the temp_buffer
        in order not to change the original input buffer byte order.
    */
    memcpy((void *)&temp_buffer,buffer, sizeof(temp_buffer));

    /* Get the index of the board to send the message to.
       If the given channel handle is invalid, then we use the 
       specified board index, otherwise we get the board index from the channel handle.
    */
    board_index = (handle ? CH_GET_BOARD_ID(handle) : b_index);

    /*
       Before sending the data structure to the DVR Firmware
       check to see if it is required to perform a byte
       swap in order to resolve little endian/ big endian conflict
    */
    if (_is_byte_swap)
    {
        _ssdk_do_message_byte_swap(board_index,  msg_class, temp_buffer);
    }
    sdbg_fw_msg_output(msg_class, 0, buffer,0);
    if ((err = sct_message_send( _ssdvr_boards_info.boards[board_index].handle, msg_class, temp_buffer, buf_size)) != SCT_NO_ERROR)
    {
        char errStr[256];
        sprintf(errStr,"Failed to send the message. Err = %s [%d]\n",sdvr_get_error_text(err), err);
 //       sdbg_msg_output(msg_class, board_index, errStr);
/*
        // get board is the first message we sent to the board, it is 
        // possible that the firmware was not loaded. In which case,
        // we should give an error message
        if (msg_class != DVR_GET_BOARD)
            assert(err == SCT_NO_ERROR);
*/
    }    
    return err;
}
/***************************************************************************
 Wrapper to the sct_message_recv(). See the sct documentation for the
 description and usage.
****************************************************************************/
sdvr_err_e ssdk_message_chan_recv(sdvr_chan_handle_t handle,
                  sx_int32        msg_class,
                  void *          buffer,
                  sct_board_t *   psrc_board,
                  sx_int32 *      pmsg_class )
{
    return _ssdk_message_recv(handle, 0xFF, msg_class, buffer, psrc_board, pmsg_class);
}
sdvr_err_e ssdk_message_board_recv(sx_uint8 board_index,
                  sx_int32        msg_class,
                  void *          buffer,
                  sct_board_t *   psrc_board,
                  sx_int32 *      pmsg_class )
{
    return _ssdk_message_recv(0, board_index, msg_class, buffer, psrc_board, pmsg_class);
}

sdvr_err_e _ssdk_message_recv(sdvr_chan_handle_t handle,
                  sx_uint8 b_index,
                  sx_int32        msg_class,
                  void *          buffer,
                  sct_board_t *   psrc_board,
                  sx_int32 *      pmsg_class )
{
    sdvr_err_e err;
    sx_uint8 board_index;
    /* Get the index of the board to send the message to.
       If the given channel handle is invalid, then we use the 
       specified board index, otherwise we get the board index from the channel handle.
    */
    board_index = (handle ? CH_GET_BOARD_ID(handle) : b_index);

    // Since the last call timed-out, most probably, the FW is hung.
    // There is no reason to call the FW again.
    if (_ssdvr_boards_info.boards[board_index].is_fw_dead)
    {
        *(char *)buffer = (char)SDVR_DRV_ERR_MSG_RECV;
        return SDVR_DRV_ERR_MSG_RECV;
    }
         
    err = sct_message_recv(_ssdvr_boards_info.boards[board_index].handle, msg_class, buffer, 0, 0);
    sdbg_fw_msg_output(msg_class, 0, buffer,0);

    if (err != SCT_NO_ERROR)
    {
        *(char *)buffer = (char)SDVR_DRV_ERR_MSG_RECV;
        _ssdvr_boards_info.boards[board_index].is_fw_dead = true;
        if (err == SCT_ERR_MSG_RECV)
            err = SDVR_DRV_ERR_MSG_RECV;
    }
    else
    {
        /*
           AFter receivng the response data structure from the DVR Firmware
           check to see if it is required to perform a byte
           swap in order to resolve little endian/ big endian conflict
        */
        if (_is_byte_swap)
        {
            _ssdk_do_message_byte_swap(board_index,  msg_class, buffer);
        }

        switch (msg_class)
        {
            case DVR_REP_MD:
            {
                dvr_md_t *cmd_strct = (dvr_md_t *)buffer;
                sx_uint8 index;
                sx_uint8 enable;

                /***************************************************************
                    Save the motion detection information. It is needed for
                    validating the regions.
                ****************************************************************/
                if (cmd_strct->status == SDVR_ERR_NONE && cmd_strct->op_code == DVR_VPP_OPCODE_REGIONS)
                {
                    index = cmd_strct->u1.reg.index;
                    enable = cmd_strct->u1.reg.enable;
                    if (index < SDVR_MAX_MD_REGIONS)
                    {
                        if (cmd_strct->job_type == SDVR_CHAN_TYPE_DECODER)
                            _ssdvr_boards_info.boards[board_index].decode_channels[cmd_strct->job_id].md_defined_regions[index] = enable;
                        else
                            _ssdvr_boards_info.boards[board_index].recv_channels[cmd_strct->job_id].md_defined_regions[index] = enable;
                    }
                }
                break;
            }
            case DVR_REP_BD:
            {
                dvr_bd_t *cmd_strct = (dvr_bd_t *)buffer;
                sx_uint8 index;
                sx_uint8 enable;

                /***************************************************************
                    Save the blind detection region information. It is needed for
                    validating the regions.
                ****************************************************************/
                if (cmd_strct->status == SDVR_ERR_NONE && cmd_strct->op_code == DVR_VPP_OPCODE_REGIONS)
                {
                    index = cmd_strct->u1.reg.index;
                    enable = cmd_strct->u1.reg.enable;
                    if (index < SDVR_MAX_BD_REGIONS)
                    {
                        if (cmd_strct->job_type == SDVR_CHAN_TYPE_DECODER)
                            _ssdvr_boards_info.boards[board_index].decode_channels[cmd_strct->job_id].bd_defined_regions[index] = enable;
                        else
                            _ssdvr_boards_info.boards[board_index].recv_channels[cmd_strct->job_id].bd_defined_regions[index] = enable;
                    }
                }
                break;
            }
            case DVR_REP_PR:
            {
                dvr_pr_t *cmd_strct = (dvr_pr_t *)buffer;
                sx_uint8 index;
                sx_uint8 enable;

                /***************************************************************
                    Save the privacy region information. It is needed for
                    validating the regions.
                ****************************************************************/
                if (cmd_strct->status == SDVR_ERR_NONE && cmd_strct->op_code == DVR_VPP_OPCODE_REGIONS)
                {
                    index = cmd_strct->u1.reg.index;
                    enable = cmd_strct->u1.reg.enable;
                    if (index < SDVR_MAX_PR_REGIONS)
                    {
                        if (cmd_strct->job_type == SDVR_CHAN_TYPE_DECODER)
                            _ssdvr_boards_info.boards[board_index].decode_channels[cmd_strct->job_id].pr_defined_regions[index] = enable;
                        else
                            _ssdvr_boards_info.boards[board_index].recv_channels[cmd_strct->job_id].pr_defined_regions[index] = enable;
                    }
                }
                break;
            }

            case DVR_REP_CONTROL:
            {
                dvr_control_t *cmd_strct = (dvr_control_t*) buffer;

                if (cmd_strct->status == SDVR_ERR_NONE)
                {
                    if (cmd_strct->job_type == SDVR_CHAN_TYPE_DECODER)
                        _ssdvr_boards_info.boards[board_index].decode_channels[cmd_strct->job_id].is_channel_running = 
                            cmd_strct->enable;
                    else
                        _ssdvr_boards_info.boards[board_index].recv_channels[cmd_strct->job_id].is_channel_running = 
                            cmd_strct->enable;
                }
                break;
            }
            case DVR_REP_JOB:
            {
                dvr_job_t *cmd_strct = (dvr_job_t *)buffer;
                if (cmd_strct->status == SDVR_ERR_NONE)
                {
                    if (cmd_strct->job_type != SDVR_CHAN_TYPE_DECODER)
                    {
                        _ssdvr_boards_info.boards[board_index].recv_channels[cmd_strct->job_id].sub_enc_info[0].venc_type = 
                            cmd_strct->video_format;
                        _ssdvr_boards_info.boards[board_index].recv_channels[cmd_strct->job_id].sub_enc_info[1].venc_type = 
                            cmd_strct->video_format_2;

                        _ssdvr_boards_info.boards[board_index].recv_channels[cmd_strct->job_id].aenc_type = 
                            cmd_strct->audio_format;

                    }
                    else
                    {
                        _ssdvr_boards_info.boards[board_index].decode_channels[cmd_strct->job_id].vdecoder_type = 
                            cmd_strct->video_format;
                    }
                }
                break;
            }
            case DVR_REP_SMO:
            {
                dvr_smo_t *cmd_strct = (dvr_smo_t *)buffer;

                /***************************************************************
                    Save the SMO enable status. This is needed to decide whether
                    we need to re-start a channel after we changed any attribute
                    of it.
                ****************************************************************/
                if (cmd_strct->status == SDVR_ERR_NONE)
                {
                    if (cmd_strct->job_type != SDVR_CHAN_TYPE_DECODER)
                    {
                        _ssdvr_boards_info.boards[board_index].recv_channels[cmd_strct->job_id].is_smo_enable = 
                            cmd_strct->enable;
                    }
                    else
                    {
                        _ssdvr_boards_info.boards[board_index].decode_channels[cmd_strct->job_id].is_smo_enable = 
                            cmd_strct->enable;
                    }
                }
                break;
            }
        }
    }
    return err;
}

/*************************************************************************
  This is a wrapper to sct_buffer_free. This function also keeps track 
  of buffers being freed.

*************************************************************************/

void ssdk_sct_buffer_free( sct_channel_t    channel, void * p )
{
    sdvr_av_buffer_t *frame_buffer = (sdvr_av_buffer_t *)(p);
    ssdk_dbg_frame_count_t *dbg_frame_count;

    //////////////////////////////////////////////////////////////////////
    // Get the data structures for the corresponding channel type of
    // the given channel handle
    //
    if (frame_buffer->channel_type == SDVR_CHAN_TYPE_DECODER)
    {
        dbg_frame_count = &_ssdvr_boards_info.boards[frame_buffer->board_id].decode_channels[frame_buffer->channel_id].dbg_frame_count;
    }
    else
    {
        dbg_frame_count = &_ssdvr_boards_info.boards[frame_buffer->board_id].recv_channels[frame_buffer->channel_id].dbg_frame_count;
    }
        
    switch (frame_buffer->frame_type)
    {
    case SDVR_FRAME_RAW_VIDEO:
        dbg_frame_count->y_u_vBuffersFreed++;
        break;

    case SDVR_FRAME_RAW_AUDIO:
        break;

    case SDVR_FRAME_H264_IDR:
    case SDVR_FRAME_H264_I:
    case SDVR_FRAME_H264_P:
    case SDVR_FRAME_H264_B:
    case SDVR_FRAME_H264_SPS:
    case SDVR_FRAME_H264_PPS:
    case SDVR_FRAME_JPEG:
    case SDVR_FRAME_MPEG4_I:
    case SDVR_FRAME_MPEG4_P:
    case SDVR_FRAME_MPEG4_VOL:
        break;
    default:
        break;
    }

    sdbg_totalFrame_count_freed++;
    dbg_frame_count->totalFramesFreed++;

    sct_buffer_free(channel, p );
}


/***************************************************************************
    This function checks to see if the given channel is in the middle of
    any kind of streaming.

    NOTE: in case of decoder, only if the decoder is enabled we consider
    that the channel is streaming.

    Parameters:
        handle:    the channel handle to be checked

    return:
        0 - if the channel is not streaming at all
        otherwise - the channel is streaming

***************************************************************************/
sx_bool ssdk_is_chan_streaming(sdvr_chan_handle_t handle)
{
    sx_bool ret = false;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sdvr_chan_type_e chan_type = CH_GET_CHAN_TYPE(handle);


    switch (chan_type)
    {
    case SDVR_CHAN_TYPE_DECODER:
        if(_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].is_dec_av_enable)
            ret = true;

        break;
    case SDVR_CHAN_TYPE_ENCODER:
    {
        ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

        if( enc_chan_info->is_raw_av_streaming[0] ||
		    enc_chan_info->is_raw_av_streaming[1] ||
            enc_chan_info->is_raw_audio_streaming ||
            enc_chan_info->sub_enc_info[0].is_enc_av_streaming ||
            enc_chan_info->sub_enc_info[1].is_enc_av_streaming ||
            enc_chan_info->is_smo_enable)
            ret = true;
    }
        break;
    } //switch (chan_type)

    // If the board is dead, we are assuming that we can not be in streaming.
    // This will allow closing of all the channels which is needed for 
    // the driver to close all of the SCT channels in order to close the application.

    return ret && !_ssdvr_boards_info.boards[board_index].is_fw_dead;
}

/***************************************************************************
    This function checks to see if the given channel is running.
    This means either SMO, HMO is running. Or the encoder/decoder is enabled

    Parameters:
        handle:    the channel handle to be checked

    return:
        0 - if the channel is not streaming at all
        otherwise - the channel is streaming

***************************************************************************/
sx_bool ssdk_is_chan_running(sdvr_chan_handle_t handle)
{
    sx_bool ret = false;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sdvr_chan_type_e chan_type = CH_GET_CHAN_TYPE(handle);

    switch (chan_type)
    {
    case SDVR_CHAN_TYPE_DECODER:
        if( _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].is_channel_running)
            ret = true;

        break;
    case SDVR_CHAN_TYPE_ENCODER:
//    case SDVR_CHAN_TYPE_NONE:
        if( _ssdvr_boards_info.boards[board_index].recv_channels[chan_num].is_channel_running)
            ret = true;

        break;
    }

    return ret;
}

/***********************************************************************
    This function returns the width and number of lines for the given
    video standard.

    Parameters:
        video_std: IN - The video standard to get its width and hieght
        width:     OUT - The width of the video standard
        lines:     OUT - The number of lines of the video standard

    return:
        None.
************************************************************************/
void ssdk_get_width_hieght_vstd(sdvr_video_std_e video_std, int *width,
                           int *lines)
{
    switch (video_std)
    {
    case SDVR_VIDEO_STD_D1_PAL:
        *width = 720;
        *lines = 576;
        break;

    case SDVR_VIDEO_STD_D1_NTSC:
        *width = 720;
        *lines = 480;
        break;

    case SDVR_VIDEO_STD_CIF_PAL:
        *width = 352;
        *lines = 288;
        break;

    case SDVR_VIDEO_STD_CIF_NTSC:
        *width = 352;
        *lines = 240;
        break;

    case SDVR_VIDEO_STD_2CIF_PAL:
        *width = 704;
        *lines = 288;
        break;

    case SDVR_VIDEO_STD_2CIF_NTSC:
        *width = 704;
        *lines = 240;
        break;

    case SDVR_VIDEO_STD_4CIF_PAL:
        *width = 704;
        *lines = 576;
        break;

    case SDVR_VIDEO_STD_4CIF_NTSC:
        *width = 704;
        *lines = 480;
        break;

    case SDVR_VIDEO_STD_QCIF_PAL:
        *width = 176;
        *lines = 144;
        break;

    case SDVR_VIDEO_STD_QCIF_NTSC:
        *width = 176;
        *lines = 112;
        break;

    case SDVR_VIDEO_STD_NONE:
    default:
        *width = 0;
        *lines = 0;

    }

    // Make sure the width is divisible by 16
    *width /= 16;
    *width *= 16;


    // Don't force the # of lines to be divisible by 16. The
    // board sends the standard number of lines except the extra
    // lines will be garbaged. SDK needs to trim the extra lines
    // from the buffer before sending it to the caller.
//    *lines /= 16;
//    *lines *= 16;

} 

ssdvr_size_t get_frame_size(sdvr_video_std_e vs, sdvr_video_res_decimation_e dec)
{
    ssdvr_size_t size;

    switch (vs)
    {
        case SDVR_VIDEO_STD_D1_PAL:
            size.width = 720;
            size.height = 576;
            break;
        case SDVR_VIDEO_STD_D1_NTSC:
            size.width = 720;
            size.height = 480;
            break;
        case SDVR_VIDEO_STD_CIF_PAL:
            size.width = 352;
            size.height = 288;
            break;
        case SDVR_VIDEO_STD_CIF_NTSC:
            size.width = 352;
            size.height = 240;
            break;
        case SDVR_VIDEO_STD_2CIF_PAL:
            size.width = 704;
            size.height = 288;
            break;
        case SDVR_VIDEO_STD_2CIF_NTSC:
            size.width = 704;
            size.height = 240;
            break;
        case SDVR_VIDEO_STD_4CIF_PAL:
            size.width = 704;
            size.height = 576;
            break;;
        case SDVR_VIDEO_STD_4CIF_NTSC:
            size.width = 704;
            size.height = 480;
            break;
        case SDVR_VIDEO_STD_QCIF_PAL:
            size.width = 176;
            size.height = 144;
            break;;
        case SDVR_VIDEO_STD_QCIF_NTSC:
            size.width = 176;
//            height = 120;
            size.height = 112;

            break;

        case SDVR_VIDEO_STD_NONE:
            size.width = size.height = 0;
            return size;
    }

    switch (dec) 
    {
    case SDVR_VIDEO_RES_DECIMATION_FOURTH:
        size.width  >>= 1;
        size.height >>= 1;
        break;
    
    case SDVR_VIDEO_RES_DECIMATION_SIXTEENTH:
        size.width  >>= 2;
        size.height >>= 2;
        break;

    case SDVR_VIDEO_RES_DECIMATION_HALF:
        // Same witdh a full size.
        size.height /= 2;
        break;

    case SDVR_VIDEO_RES_DECIMATION_DCIF:
//todo: DCIF is only based on 4CIF vstd
        size.width  = (size.width * 3) / 4;
        size.height = (size.height * 2) / 3;
        break;

    /*
       The following three classic decimations are actually
       resolution and firmware will scale or crop as needed.
    */
    case SDVR_VIDEO_RES_DECIMATION_CLASSIC_CIF:
        size.width = 320;
        size.height = 240;
        break;
    case SDVR_VIDEO_RES_DECIMATION_CLASSIC_2CIF:
        size.width = 640;
        size.height = 240;
        break;
    case SDVR_VIDEO_RES_DECIMATION_CLASSIC_4CIF:
        size.width = 640;
        size.height = 480;
        break;

    default:
        break;
    }

    // Make sure the width is divisible by 16
    size.width /= 16;
    size.width *= 16;

    size.height /= 16;
    size.height *= 16;

    return size;
}

/***********************************************************************
    This function returns the frame rate corresponding to the given
    video standard if the video standard is supported

    Parameters:
        video_std: IN - The video standard to get its width and hieght
        frame_rate:  OUT - The frame rate

    return:
        0 - if the given video_std is not supported
        otherwise - the frame_rate will hold the frame_rate for the 
        given video standard.
************************************************************************/
int ssdk_get_vstd_frame_rate(sdvr_video_std_e video_std, int *frame_rate)
{
    int nRet = 1;

    if (frame_rate == NULL)
        return 0;

    switch (video_std)
    {
    case SDVR_VIDEO_STD_D1_PAL:
    case SDVR_VIDEO_STD_CIF_PAL:
    case SDVR_VIDEO_STD_2CIF_PAL:
    case SDVR_VIDEO_STD_4CIF_PAL:
    case SDVR_VIDEO_STD_QCIF_PAL:
        *frame_rate = 25;
        break;

    case SDVR_VIDEO_STD_D1_NTSC:
    case SDVR_VIDEO_STD_CIF_NTSC:
    case SDVR_VIDEO_STD_2CIF_NTSC:
    case SDVR_VIDEO_STD_4CIF_NTSC:
    case SDVR_VIDEO_STD_QCIF_NTSC:
        *frame_rate = 30;
        break;

    case SDVR_VIDEO_STD_NONE:
    default:
        *frame_rate = 0;
        nRet = 0;
    }

    return nRet;
} 

/****************************************************************************
  This function gets the video/audio codec for the given channel

  Parameters:

    "handle" - Handle of encoder/decoder channel.

    "sub_chan" - This field is only valid encoder channels. It request
    to get the primary or secondary encoder type for the current 
    encoder channel. 
    This field is ignore for channel type different than SDVR_CHAN_TYPE_ENCODER.

    "video_codec" - a pointer to variable to hold the video codec up on  
    a successful return.

    "audio_codec" - a pointer to variable to hold the audio codec up on  
    a successful return.

  Returns:

   SDVR_ERR_NONE - success

   SDVR_ERR_BOARD_NOT_CONNECTED - Error code if the board was never connected to.

   SDVR_ERR_INVALID_BOARD - Error code if the given handle is not valid due to 
   bad board index.

    SDVR_ERR_WRONG_CHANNEL_TYPE - The channel handle belongs to wrong channel
    type for the current operation.
****************************************************************************/
sdvr_err_e ssdk_get_chan_av_codec(sdvr_chan_handle_t handle,
                                  sdvr_sub_encoders_e sub_chan,
                                  sdvr_venc_e *video_codec,
                                  sdvr_aenc_e *audio_codec)
{
    dvr_job_t cmd_strct;
    sdvr_err_e err;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sx_uint8 chan_num;
    sdvr_chan_type_e chan_type = CH_GET_CHAN_TYPE(handle);

    if ((err = ssdk_validate_board_ndx(board_index)) !=  SDVR_ERR_NONE){
        return err;
    }

    chan_num = CH_GET_CHAN_NUM(handle);
    switch (chan_type)
    {
    case SDVR_CHAN_TYPE_ENCODER:
    {
        ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

        if (video_codec != 0)
        {
            *video_codec =  enc_chan_info->sub_enc_info[sub_chan].venc_type;
        }
        if (audio_codec != 0)
        {
            *audio_codec = enc_chan_info->aenc_type;
        }
    }
        break;
    case SDVR_CHAN_TYPE_DECODER:
        if (video_codec != 0)
        {
            *video_codec =  _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].vdecoder_type;
        }
        if (audio_codec != 0)
        {
            *audio_codec = _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].adecoder_type;
        }

        break;
    default:
        err = SDVR_ERR_WRONG_CHANNEL_TYPE;
        break;
    }


    return err;
}


/****************************************************************************
  This function converts the given YUV buffer from 4-2-0 to 4-2-2 by just
  doubling each byte within the U and V plane.

  NOTE: Since the given yuv_buffer is coming from SCT, it is large enough to
        to hold the data 4-2-2, hence we don't have to worry about mallocing
        the memory.


  Parameters:

    "yuv_buffer" - The YUV buffer in 4-2-0 format. Upon exit it will hold
    in 4-2-2 format.

  Returns:

    None.
****************************************************************************/

static void _ssdk_convert_YUV420_YUV422(dvr_data_header_t *yuvHdr)
{
    sx_uint8 *start_8bit_data_u;
    sx_uint8 *start_8bit_data_v;
    sx_uint16 width;
    sx_uint16 lines;
    sx_int32 y;

    // We must first move the V plane otherwise the data will be 
    // over written by U plane doubling.

    start_8bit_data_u = (sx_uint8 *)yuvHdr + yuvHdr->u_data_offset;
    start_8bit_data_v = (sx_uint8 *)yuvHdr + yuvHdr->v_data_offset;

    width = yuvHdr->active_width / 2;
    lines = yuvHdr->active_height / 2;


    for(y=lines-1; y>=0; y--)
    {
        memcpy(&start_8bit_data_u[(2*lines+2*y  )*width], &start_8bit_data_v[y*width], width);
        memcpy(&start_8bit_data_u[(2*lines+2*y+1)*width], &start_8bit_data_v[y*width], width);
    }
    yuvHdr->v_data_size *= 2;
//    yuv_malloc->yuv_frame.v_data=start_8bit_data_u+lines*2*width;

    for(y=lines-1; y>=0; y--)
    {
        memcpy(&start_8bit_data_u[(2*y  )*width], &start_8bit_data_u[y*width], width);
        memcpy(&start_8bit_data_u[(2*y+1)*width], &start_8bit_data_u[y*width], width);
    }
    yuvHdr->u_data_size *= 2;
}


sdvr_err_e ssdk_queue_yuv_frame(sdvr_chan_handle_t handle, dvr_data_header_t * cur_yuv_frame)
{
    char err_msg[256];
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    sdvr_chan_type_e chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);

    sdvr_err_e err = SDVR_ERR_NONE;
    sx_bool ret = 0;
    ssdk_yuv_frame_t * yuv_buffer;

    sct_channel_t pci_chan_handle;
    ssdk_dbg_frame_count_t *dbg_frame_count;

    //////////////////////////////////////////////////////////////////////
    // Get the data structures for the corresponding channel type of
    // the given channel handle
    //
    if (chan_type == SDVR_CHAN_TYPE_DECODER)
    {
        pci_chan_handle = _ssdvr_boards_info.boards[board_index].decode_channels[chan_num].pci_chan_handle;
        dbg_frame_count = &_ssdvr_boards_info.boards[board_index].decode_channels[chan_num].dbg_frame_count;
    }
    else
    {
        ssdk_pci_channel_info_t *enc_chan_info = ssdk_get_enc_chan_strct(board_index, chan_num);

        pci_chan_handle = enc_chan_info->pci_chan_handle;
        dbg_frame_count = &enc_chan_info->dbg_frame_count;
    }


    /*******************************************************************
    We received all three parts of YUV buffer of a raw frame.
    1) Allocate a YUV buffer 
    2) Copy the YUV temp. buffer into this yuv_buffer
    3) Reset YUV temp. buffer
    4) Insert yuv_buffer in the queue 
    5) Notify the DVR Application.
    ********************************************************************/


    yuv_buffer = (ssdk_yuv_frame_t *)_ssdk_new_yuv_buf(handle, cur_yuv_frame);

    // Since Y,U, and V buffers are sent to the host via one SCT buffer
    // with yuvHdr points to the begining of the YUV buffer we only
    // need to release the yuvHdr, the other buffers (y_buffer, u_buffer and v_buffer)
    // are just a pointer within the sct buffer
    // Release all three buffers of YUV frame if we don't have memory to hold
    // the frame. This means we are loosing frame.
    if (yuv_buffer == NULL)
    {
        ssdk_sct_buffer_free(pci_chan_handle, cur_yuv_frame);

       dbg_frame_count->yuvMissedCount++;

        err = SDVR_ERR_INTERNAL;
        sprintf(err_msg,"YUV frame is lost due to not enough memory to hold the YUV frame\n\tFrame Header chan_num = %d\tchan_type = %s\n",
        chan_num,dbg_chan_type_text(chan_type));

        sdbg_msg_output( _ssdk_av_frame_callback_func_func,board_index,err_msg);
    }
    else 
    {
        // queue the raw video frame.
        err = _ssdk_queue_frame(handle, yuv_buffer, SSDK_RECV_FRAME_TYPE_RAW_VIDEO);
        if (err != SDVR_ERR_NONE && yuv_buffer)
        {
            _ssdk_delete_yuv_buf( (ssdk_yuv_malloc_item_t *)yuv_buffer);
        }
    }

    return err;
}


/* This function checks if the given version major.minor.revision.build is grater
   than or equal to the given version majorChk.minorChk.revisionChk.buildChk
*/
sx_bool ssdk_is_min_version(sx_uint8 major, sx_uint8 minor, sx_uint8 revision, sx_uint8 build,
                     sx_uint8 majorChk, sx_uint8 minorChk, sx_uint8 revisionChk, sx_uint8 buildChk)
{

    if (major > majorChk)
        return true;
    if (major < majorChk)
        return false;
    if (minor > minorChk)
        return true;
    if (minor < minorChk)
        return false;
    if (revision > revisionChk)
        return true;
    if (revision < revisionChk)
        return false;
    if (build < buildChk)
        return false;

    return true;

}

/* This function sets the function to be called in order to facilitate
   the hi-level displaying of HMO. This is an internal function to be
   used by the UI module.
*/

sx_bool ssdk_set_ui_callback(sdvr_chan_handle_t handle, sdvr_ui_callback callback_func, void * context)
{
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    
    // Setup approporiate call back function for the channel type.
    if (chan_type == SDVR_CHAN_TYPE_DECODER)
    {
        ssdk_pci_decode_chan_info_t * pchan = &(_ssdvr_boards_info.boards[board_index].decode_channels[chan_num]);
        pchan->ui_cb_func = callback_func;
        pchan->ui_cb_context = context;
    }
    else
    {
        ssdk_pci_channel_info_t * pchan = &(_ssdvr_boards_info.boards[board_index].recv_channels[chan_num]);
        pchan->ui_cb_func = callback_func;
        pchan->ui_cb_context = context;
    }

    return true;
}

void *ssdk_get_ui_context(sdvr_chan_handle_t handle)
{
    sx_uint8 chan_type = CH_GET_CHAN_TYPE(handle);
    sx_uint8 chan_num = CH_GET_CHAN_NUM(handle);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    void *ptrContext;

    // Setup approporiate call back function for the channel type.
    if (chan_type == SDVR_CHAN_TYPE_DECODER)
    {
        ssdk_pci_decode_chan_info_t * pchan = &(_ssdvr_boards_info.boards[board_index].decode_channels[chan_num]);
        ptrContext = pchan->ui_cb_context;
    }
    else
    {
        ssdk_pci_channel_info_t * pchan = &(_ssdvr_boards_info.boards[board_index].recv_channels[chan_num]);
        ptrContext = pchan->ui_cb_context;
    }
    return ptrContext;
}
/************************************************************
  An internal function to be used between the SDK and UI SDK.
************************************************************/
void ssdk_free_ui_context(void *pdi)
{
    if (pdi)
        free(pdi);
}
void *ssdk_malloc_ui_context(size_t size)
{
    return malloc(size);
}

ssdk_pci_decode_chan_info_t * ssdk_get_dec_chan_strct(sx_uint8 board_index, sx_uint8 chan_num)
{
    ssdk_pci_decode_chan_info_t *chan_info = NULL;

    chan_info = &_ssdvr_boards_info.boards[board_index].decode_channels[chan_num];

    return chan_info;
}

ssdk_pci_channel_info_t * ssdk_get_enc_chan_strct(sx_uint8 board_index, sx_uint8 chan_num)
{
    ssdk_pci_channel_info_t *chan_info = NULL;

    chan_info = &_ssdvr_boards_info.boards[board_index].recv_channels[chan_num];

    return chan_info;
}

/****************************************************************
 This function returns the number of miliseconds needed to
 play each recorded video frame during the playback to 
 acheive the maximum frame rate (30 fps for nts and 25 for pal).

 The duration is based on 90K Hz clock

 Parameters:
       video_standard -- The video standard to get its real time
                        video display per seconds

 Return:
      The time based on 90K Hz clock to playback each video frame to
      achieve the maximum video display per seconds for the given
      video standard.
*****************************************************************/
int ssdk_get_video_rec_duration(sdvr_video_std_e video_standard)
{
    
    switch (video_standard)
    {
        case SDVR_VIDEO_STD_NONE:
        case SDVR_VIDEO_STD_D1_PAL:
        case SDVR_VIDEO_STD_CIF_PAL:
        case SDVR_VIDEO_STD_2CIF_PAL:
        case SDVR_VIDEO_STD_4CIF_PAL:
            return 3600;      // 25 frame a second

        case SDVR_VIDEO_STD_D1_NTSC:
        case SDVR_VIDEO_STD_CIF_NTSC:
        case SDVR_VIDEO_STD_2CIF_NTSC:
        case SDVR_VIDEO_STD_4CIF_NTSC:
            return 3003;      // 30 frame a second

        default:
            return 3003;      // 30 frame a second
    }
}

/****************************************************************
 This function reads the authentication key for the given board.
 The result is saved by the SDK so that H.264 video frames can
 be authenticated.

 Parameters:
       board_index -- The zero based index of the board to read
       its authentication key.

 Return:
      SDVR_ERR_NONE if no error.
*****************************************************************/
sdvr_err_e ssdvr_read_auth_key(sx_uint32 board_index)
{
    dvr_auth_key_t cmd_strct;
    sdvr_err_e err = SDVR_ERR_NONE;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 read_offset;

    memset(&cmd_strct,0,cmd_strct_size);

    // Assume there is no authentication key is specifed
    memset(&_ssdvr_boards_info.boards[board_index].auth_key[0],0,
        sizeof(_ssdvr_boards_info.boards[board_index].auth_key));
    _ssdvr_boards_info.boards[board_index].auth_key_len = 0;

    /*
      Ask the firmware to send you authentication key starting with
      1st 13 bytes. length = nth tells the firmware to send the
      Nth framgment of the the authentication key.

      On the reply length includes the total authentication key length.
    */
    cmd_strct.board_id = board_index;
    read_offset = 0;
    do {
        cmd_strct.u1.read_offset = read_offset;
        err = ssdk_message_board_send( board_index, DVR_GET_AUTH_KEY, &cmd_strct, cmd_strct_size);
        if (err == SCT_NO_ERROR)
        {
            err = ssdk_message_board_recv( board_index, DVR_REP_AUTH_KEY, &cmd_strct, 0, 0);

        }
        /*
          Stop of the authentication is missing or an error occured
          in reading the key.
        */
        if (err != SCT_NO_ERROR || cmd_strct.status != SDVR_ERR_NONE ||
            !cmd_strct.total_length || !cmd_strct.u1.read_length)
            break;

        if (cmd_strct.total_length > DVR_AUTH_KEY_LEN)
        {
            err = SDVR_ERR_AUTH_KEY_LEN;
            break;
        }
        _ssdvr_boards_info.boards[board_index].auth_key_len = cmd_strct.total_length;
        memcpy(&_ssdvr_boards_info.boards[board_index].auth_key[read_offset],
            cmd_strct.data, cmd_strct.u1.read_length);

        read_offset += cmd_strct.u1.read_length;

    } while(read_offset < cmd_strct.total_length);


    if (_ssdvr_boards_info.boards[board_index].auth_key_len == 0)
        err = SDVR_ERR_AUTH_KEY_MISSING;
    return err;
}

/****************************************************************************
  This function sets whether or not there must be an H.264 authentication
  key code set on the DVR board at the time of calling to 
  sdvr_board_connect(). This function must be called after sdvr_sdk_init()
  and before connecting to any DVR boards.

  Parameters: 
 
    "enable" - The flag to enforce the authentication key presence.

  Returns: 

    SDVR_ERR_NONE - On success. 

    SDVR_ERR_NO_DVR_BOARD - Error code if sdvr_sdk_init() was not called
    or no DVR board was found on the system.

    SDVR_ERR_BOARD_CONNECTED - Error code if at least one DVR board is 
    connected at the time of calling this function.

  Remarks:

    Once authentication key is enabled, there must be an authentication key
    burned in the eeprom of the DVR board
    before calling sdvr_board_connect(). If the key does not exist
    on the board, board connection will fail.
****************************************************************************/
sdvr_err_e sdvr_enable_auth_key(sx_bool enable)
{
    sx_uint32 board_index;

    if (_ssdvr_boards_info.board_num == 0)
        return SDVR_ERR_NO_DVR_BOARD;

    /* 
        We should not be connected to any board prior to enabling the
        authentication key.
    */

    for (board_index = 0; board_index < _ssdvr_boards_info.board_num; board_index++)
    {
        if (_ssdvr_boards_info.boards[board_index].handle)
        {
            return SDVR_ERR_BOARD_CONNECTED;
        }
    }

    _ssdvr_boards_info.is_auth_key_enabled = enable;

    return SDVR_ERR_NONE;
}

/****************************************************************************
  This function is used to send any random value to a given
  I/O device of particular camera. This function is intended for testing or 
  experimenting the I/O device hardware. As such, there is no
  error checking and must be used with cautious.

  Parameters:

    "handle" - An encoding channel handle.

    "device_id" - Device ID to update its register.

    "reg_num" - Register number to on the given device to write.

    "value" - The value to write to the given register on the
    I/O device.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_write_ioctl(sdvr_chan_handle_t handle,
                           sx_uint8    device_id,
                           sx_uint8 reg_num,
                           sx_uint16 value)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);
    dvr_ioctl_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);

    memset(&cmd_strct,0,cmd_strct_size);

    /**********************************************************
      There is no error checking on the parameters.
    ***********************************************************/
    cmd_strct.ioctl_code = DVR_IOCTL_CODE_DECODER_REGS;
    cmd_strct.u1.reg.device_id = device_id;
    cmd_strct.u1.reg.reg_num = reg_num;
    cmd_strct.u1.reg.val = value;

    /********************************************************
       Validate the parameters:
    ********************************************************/
    if (ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER) !=  SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;

    if (err ==  SDVR_ERR_NONE)
    {
        cmd_strct.job_type  = CH_GET_CHAN_TYPE(handle);
        cmd_strct.job_id    = CH_GET_CHAN_NUM(handle);

        /**********************************************************
          Send the given value to the given register number on the
          given device ID. 
        ***********************************************************/
        ssdk_message_chan_send(handle, DVR_SET_IOCTL, &cmd_strct, cmd_strct_size);
        err = ssdk_message_chan_recv(handle, DVR_REP_IOCTL, &cmd_strct, 0, 0);
        if (err != SDVR_ERR_NONE)
            cmd_strct.status = (unsigned char) err & 0xff;
    }

    sdbg_msg_output(sdvr_write_ioctl_func,board_index,&cmd_strct);

    return err;
}

/****************************************************************************
  This function is used to read a 16-bit value from a given
  I/O device of a particular camera. This function is intended for testing or 
  experimenting with the I/O device hardware. As such, there is no
  error checking and must be used with cautious. 

  In many cases, you may need to call sdvr_write_ioctl() before
  calling this function to get a value.

  Parameters:

    "handle" - An encoding channel handle.

    "device_id" - Device ID to update its register.

    "reg_num" - Register number to on the given device to read.

    "value_ptr" - A pointer to a 16-bit value to read from the given 
    register on the I/O device.

  Returns:

    SDVR_ERR_NONE - On success. Otherwise, see the error code list.
****************************************************************************/
sdvr_err_e sdvr_read_ioctl(sdvr_chan_handle_t handle,
                           sx_uint8  device_id,
                           sx_uint8  reg_num,
                           sx_uint16 *value_ptr)
{
    sdvr_err_e err = SDVR_ERR_NONE;
    dvr_ioctl_t cmd_strct;
    int cmd_strct_size = sizeof(cmd_strct);
    sx_uint8 board_index = CH_GET_BOARD_ID(handle);

    memset(&cmd_strct,0,cmd_strct_size);

    /**********************************************************
      There is no error checking on the parameters.
    ***********************************************************/
    cmd_strct.ioctl_code = DVR_IOCTL_CODE_DECODER_REGS;
    cmd_strct.u1.reg.device_id = device_id;
    cmd_strct.u1.reg.reg_num = reg_num;
    cmd_strct.status = SDVR_ERR_NONE;

    /********************************************************
       Validate the parameters:
    ********************************************************/
    if (ssdk_validate_chan_handle(handle, SDVR_CHAN_TYPE_ENCODER) !=  SDVR_ERR_NONE)
        err = SDVR_ERR_INVALID_CHAN_HANDLE;
    else if (value_ptr == NULL)
        err = SDVR_ERR_INVALID_ARG;


    if (err ==  SDVR_ERR_NONE)
    {
        cmd_strct.job_type  = CH_GET_CHAN_TYPE(handle);
        cmd_strct.job_id    = CH_GET_CHAN_NUM(handle);
        /**********************************************************
          Send the given value to the given register number on the
          given device ID. 
        ***********************************************************/
        ssdk_message_chan_send(handle, DVR_GET_IOCTL, &cmd_strct, cmd_strct_size);
        err = ssdk_message_chan_recv(handle, DVR_REP_IOCTL, &cmd_strct, 0, 0);

        if (err == SDVR_ERR_NONE)
        {
            if (cmd_strct.status == SDVR_ERR_NONE)
                *value_ptr = cmd_strct.u1.reg.val;
            else
                cmd_strct.status = (unsigned char) err & 0xff;

        }

    }

    sdbg_msg_output(sdvr_write_ioctl_func,board_index,&cmd_strct);

    return err;
}



