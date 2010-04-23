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

#ifndef STRETCH_SDVR_SDK_H
#define STRETCH_SDVR_SDK_H

#include <stdio.h>

/****************************************************************************
  PACKAGE: sdvr_sdk Stretch DVR SDK

  DESCRIPTION:

  SECTION: Include
  {
  #include "sdvr_sdk.h"
  }

  SECTION: Introduction
  Stretch has developed a reference design for PC-based and embedded
  DVRs. The architecture of a DVR (either PC-based or embedded) using
  components supplied by Stretch is as follows:
  {
         Host                                 Hardware Codec
                                              consisting of
     DVR Application                          one or more S6
           |                                  chips running
          SDK                                 the Stretch-developed
           |                                      Codec
        Driver                                    Firmware
           |----------------------------------------|
                   Host-to-hardware interface
  }
  The Stretch DVR (SDVR) hardware consists of one or more S6 processors
  running the @DVR  Firmware@. 
  Together, the hardware and the firmware are responsible 
  for encoding, decoding, and interfacing to the cameras, microphones, 
  sensors, and relays, and communicating with the host. 

  On the host there is a @Driver@ that handles the 
  low-level communication with the firmware. Sitting on top of the 
  driver is the @SDK@, which provides an Application-level Programming 
  Interface (API) to the SDVR hardware. It is through this API that
  an application talks to the hardware. The SDK provides the 
  ability to:

    Discover the capabilities of the hardware CODEC.

    Configure the hardware.

    Encode and decode video and audio.

    Control other capabilities like on-screen display, spot monitor, and so on.

  This document describes the Application Programming Interface (API)
  implemented in the SDK. 
  Your DVR Application can link with the SDK library either statically or dynamically 
  (MS Windows only). Use sdvr_sdk.lib to link statically with the SDK library. 
  Use sdvr_sdk_dll.lib to link dynamically with the SDK library; the 
  corresponding sdvr_sdk_dll.dll needs to be copied with your DVR 
  Application executable.
  
  Throughout this document, we refer to the application that uses the
  SDK as the @DVR Application@, as it will implement the functionality 
  of a DVR.

  SECTION: Function Groups
  The functions in the SDK API are divided into nine major groups.

    @System Set Up@. Functions in 
    this group enable you to initialize the SDK, set up the system
    parameters, and discover and initialize boards at start-up. When you want
    to exit the application, the APIs in this group allow you to 
    shut down the system gracefully and to free all system resources.

    @Channel Set Up@. Functions in this group 
    enable you to initialize each channel at start-up or to reinitialize
    a channel during operation.

    @Encoding@. Functions in this group allow you to start
    and stop encoding, and to receive encoded and raw audio/video (A/V) buffers.
    
    @Decoding@. Functions in this group allow you to send encoded 
    A/V streams to the decoder for decoding and display.

    @Video Display and Sound@. Functions in this group allow you to
    control raw audio and video streaming. Stretch provides a separate
    UI SDK library that helps you to display video frames in any regions 
    within a display window handle. (See sdvr_ui_sdk.html for details.)

    @On-Screen Display@. Functions in this group allow you to 
    control how on-screen information is displayed and to enable or disable 
    on-screen display (OSD).

    @Spot Monitor@. Functions in this group allow you to control 
    the grid pattern and channels appearing in the spot monitor output (SMO). 

    @RS485 Communication API@. Functions in this group allow you to 
    control the RS-485 interface over which you can implement any PTZ
    protocol that works over RS-485.

    @Sensors and Relays@. Functions in this group allow you to
    set and reset sensors and relays attached to the hardware.

    @Recording to File API@. Functions in this group allow you to
    save the encoded audio and video frames in to a file.

  Naming conventions. Functions and data structures start with @sdvr_@
  so as to avoid conflict with symbols in the DVR Application.
 
  All enumerated types end with @_e@ and all data structures end with @_t@.

  SECTION: Important Notes about the SDK
  Note the following:

    The SDK and the firmware on the board is stateless, i.e. parameters 
    used to configure the SDK and the firmware are not stored across 
    sessions (a session starts with initialization and ends when the SDK 
    is closed or the system rebooted). Remember that every time you call 
    sdvr_sdk_init() at the beginning of your application you must 
    reinitialize all the system and channel parameters after you connect 
    to the hardware.

    To set the time in firmware, the DVR Application needs to compute the
    number of seconds elapsed since January 1, 1970 UTC and to present it to
    each board using sdvr_set_date_time(). When this function is used, 
    the firmware takes the date and time and sets it in hardware.
    It is required that the DVR Application set the 
    time whenever the time changes (e.g., Daylight  Savings Time). We also 
    recommend that the DVR Application monitor the time on 
    the hardware, and if the drift between the host clock and the 
    clock on the hardware gets beyond a certain limit, then reset the time 
    on the hardware.

    Depending on the type of DVR (embedded or PC-based), the hardware
    may consist of one or more boards, each having one or more S6 chips, 
    and each running a copy of the CODEC firmware. 
    If there are N boards in a system, the SDK numbers them from 0 to N-1. 

    When configuring the system, the DVR Application is required to set
    the system-wide video standard and resolution. This is the maximum
    video resolution that is supported by the SDK. Lesser video
    resolutions can be supported, but the lesser resolutions can only be 1/4
    of or 1/16 of the maximum resolution.

    If you mark a region as private, that region is blocked out in both
    the live and the encoded stream. 

    When decoding a frame, you must enable raw video or SMO for the decoder
    channel in order to receive and display the decoded raw video frames.

    For any alarm detection (i.e., motion detection, blind detection, and so on)
    to be active, the camera must be either encoding video frames or streaming
    raw video.

    If the specified decimation and x,y coordinates of an SMO grid do not correctly
    fit on the monitor, the grid is not displayed.

  SECTION: Important Restrictions
  Note the following restrictions when using the SDK. The
  restrictions, apart from those explicitly noted, are not permanent
  and will be removed in future versions of the SDK. 

    Only one DVR Application can use the SDK on one host. Multiple 
    copies of the SDK and associated driver cannot reside on the same host. 
    This is a permanent restriction. 

    All SDVR cards in a system must have the same video standards and camera
    resolution, e.g., NTSC D1 720x480 at 30 fps or PAL 720x576 at 25 fps.

    Only two OSD text strings are supported per channel. 

    For OSD text only the ASCII character set is supported. 
    Double-byte and Unicode character sets are supported only if the corresponding
    font table is loaded in the firmware.

    OSD text color and logo display are currently not supported.  

    OSD text for decoder channels is currently not supported.

    OSD text must be positioned such that it does not cause wrapping
    within the video frame. Otherwise, the behavior is not deterministic.

    Only H.264 decoding is supported.

    Audio decoding is not supported.

    Currently the APIs that save A/V frames into a file is only limited to
    .mov file format. Additionally, these APIs are not officially supported
    in this release.

    SMO dwell time field is not supported. This means only one video channel can be
    specified in any SMO grid.

    A channel must be streaming video (encoder, raw, or SMO) in order for
    any of the alarm detection occures.

  SECTION: Format of data buffer exchange between SDK and DVR Application

  There are two type of data buffers: sdvr_av_buffer_t and sdvr_yuv_buffer_t.

  sdvr_av_buffer_t is used to exchange encoded video, encoded audio, and 
  raw audio data. sdvr_yuv_buffer_t is used to exchange raw video data.

  The raw A/V frames can be
  either from the live or decoded video stream. 
  The format of a raw video is YUV 4:2:0. 
  A raw video frame is of type sdvr_yuv_buffer_t that 
  consists of a header followed by three payloads. Each payload is
  a pointer to a Y, U, or V buffer. Each Y, U, or V buffer is of type sdvr_av_buffer_t.
  The format of a raw audio is PCM. A raw audio frame is of type sdvr_av_buffer_t
  that consists of a header followed by a payload.

  The encoder encodes the incoming video and audio 
  frame-by-frame and each encoded
  frame is sent to the host. A frame consists of a header generated by
  the encoder and a payload. The header format is Stretch proprietary
  and has information that may be relevant to the DVR Application (e.g., 
  whether motion was detected). The header format is sdvr_av_buffer_t described later
  in this document. 

  The payload format contains the video stream in
  elementary stream format. Therefore, if the video payload is stored in 
  its own file, it can be played by any player that supports the elementary
  stream format and has the corresponding decoder. 


  SECTION: Using the SDK API
  This section provides a high-level overview of the various DVR
  Application tasks and the functions used to accomplish those tasks.

  SUBSECTION: SDK and Board Initialization and Set-Up
  The DVR Application needs to perform the following tasks during
  initialization and set-up.

    Initialize the SDK using sdvr_sdk_init().

    Load firmware into every DVR board that needs to be connected by
    calling sdvr_upgrade_firmware(). This is required only in cases when 
    no firmware is
    burned onto the board, or when you want to use a different firmware than 
    that already burned. You can get the board type as well as other PCI information
    in order to decide which version of firmware to load by calling 
    sdvr_get_pci_attrib().

    For a PC-based DVR, get the number of boards using sdvr_get_board_count().
    For an embedded DVR, the board count is almost always 1. 

    For both PC-based and embedded DVRs, get the board attributes using
    sdvr_get_board_attributes().

    Set the amount of memory allocated for communication with the 
    hardware using sdvr_set_sdk_params(). We recommend that you use default
    video buffer settings for that call sdvr_get_sdk_params(), and only
    change the fields of interest to you. During development, we
    recommend that you enable debugging using many different flags in the SDK parameter
    structure, and specify a logfile to record tracing of communication between
    the DVR Application and the DVR firmware.

    Connect to each board using sdvr_board_connect() and set the 
    video standard (NTSC or PAL) and maximum resolution. (You can get a list of
    supported video standards from sdvr_get_board_attributes().) Additionally, 
    specify whether the host PC should perform start code emulation for h.264
    codecs. For performance reasons, it is highly recommended to always set 
    this field to 1 except for the embedded DVR Applications which must be set
    to 0. This establishes a connection to the board for further control and data
    communication.

    If required, the SDK, driver, boot loader, and firmware versions can be obtained
    using sdvr_get_sdk_version(), sdvr_get_driver_version() and 
    sdvr_get_firmware_version_ex(). Stretch will publish a matrix of which
    SDK, driver, boot loader, and firmware versions are compatible. This information
    can be used to check that compatible versions are used.

    Get the capabilities of each board, i.e., the number of cameras it
    can have, the number of sensors, etc. using sdvr_get_board_config().

    Set the date and time in the firmware using sdvr_set_date_time(). 
    Subsequently, the DVR Application should periodically monitor the
    time on the firmware using sdvr_get_date_time(), and if there is
    drift between the host and the firmware clock, reset the clock on 
    the firmware using sdvr_set_date_time().

    Set the callback function for the SDK to call when an error is
    encountered in the firmware 
    using sdvr_signals_callback(). This is not mandatory, but is a very
    useful tool for debugging.

    Set the callback function for the SDK to call when sensors are 
    triggered using sdvr_set_sensor_callback(). This is mandatory if the
    DVR Application wants to be notified about sensors being triggered.

    Set the callback function for the SDK to call when video alarms are
    triggered using sdvr_set_video_alarm_callback(). This is not mandatory as 
    you can get some of this information from the A/V buffer header, but we 
    highly recommend that you register a callback for video alarms.

    Set the callback function for the SDK to call when AV frames are available
    from the hardware using sdvr_set_av_frame_callback(). This callback 
    function is not mandatory, as described later. 

    If you need to display OSD text other than English language, load
    the corresponding font file using sdvr_osd_set_font_table().

    If necessary, set the watchdog timer in the hardware using
    sdvr_set_watchdog_state(). The watchdog timer is used to reset the
    entire DVR system to prevent it from hanging. If the software on the 
    host and the firmware is alive, then the watchdog should be periodically
    reset before it expires (the current value can be obtained using
    sdvr_get_watchdog_state()). 

    If necessary, run diagnostics on the hardware using sdvr_run_diagnostics().
    The diagnostics run pertain only to the S6-part of the
    system, as described later.

  SUBSECTION: Channel Set-Up
  Before using the system, you must create different channel type to peform
  encoding or decoding tasks. An encoder channel provides encoded video and
  audio and raw video and audio data.

  To create and configure each channel type, the DVR Application needs to perform the 
  following tasks during initialization.

    If there are N cameras supported by the system, then channels 0 to
    N-1 are encoding channels. Each encoding channel number corresponds to the 
    camera position number in the back panel. 
    Set each of these channels as encoding 
    channels unless you want to leave some cameras unconnected and
    use the processing power for decoding. To create an encoding
    channel, use sdvr_create_chan(). You can dynamically
    turn off encoding channels to free up processing power for decoding. 
    Additionally, if there are channels created with secondary encoding support,
    you may not be able to connect to all the cameras for recording. You can
    also specify audio encoding for all cameras that have corresponding
    audio channels connected. Additionally, you may want to create some
    streaming only channels that do not need to have any encoding capabilities.
    These are called HMO- or SMO-only channels. To create HMO- or SMO-only 
    channel, set the channel type to SDVR_CHAN_TYPE_ENCODER
    and the primary video format to SDVR_VIDEO_ENC_NONE.
    @Note: The HMO- or SMO-only channels take away from the maximum number
    of encoding channels that are allowed by the system. This means that if there are N
    cameras supported and you create M HMO- or SMO-only
    channels, you can only create N minus M number of encoding channels.@

    If necessary, you can change the video-in image parameters (i.e., hue,
    saturation) using sdvr_set_video_in_params(). These parameters can
    be changed only for encoder or HMO- or SMO-only channels.

    After encoding channels are set up, set up decoding channels using
    sdvr_create_chan(). The maximum number of decoding channels depends
    on the processing power left after setting up the encoder channels. 
    If the call to sdvr_create_chan() returns successfully, then the decoding
    channel was set up. Otherwise, there is no more processing power available and 
    the decoding channel could not be set up. Currently, the decoding channel number
    indicates in which S6 chip the decoder is created. Similar to encoder channels,
    decoder channel numbers run from 0 to N-1. Where N is the maximum number of
    decoders supported returned by sdvr_get_board_config(). Every 4 consecutive
    decoder channel numbers are created in one S6 chip starting from decoder 
    channel 0 in the first S6 chip.
    For example, to have one decoder channel created in every S6 chip in
    a four-S6 chip DVR board, you should assign channel numbers 0, 4, 9, and
    13 when creating the decoder channels.

    When you set a channel to be encoding or decoding, you receive
    a unique channel handle of type sdvr_chan_handle_t. Subsequently, 
    you use this handle in all function calls requiring a channel
    identifier.

    Existing encoding or decoding channels can be destroyed at any time by
    calling sdvr_destroy_chan(). This call lets you rebalance
    channel types as needed.

    Set the parameters of each video encoding channel using
    sdvr_set_video_encoder_channel_params().

    Set the parameters of each alarm-triggered video encoding channel using
    sdvr_set_alarm_video_encoder_params().

    Set the parameters of each audio encoding channel using
    sdvr_set_audio_encoder_channel_params(). (Currently, there are no audio
    encoder parameters.)

    Set regions for motion and blind detection, and privacy regions
    using sdvr_add_region().

    You can associate application-defined data with each encoding or
    decoding channel by calling sdvr_set_chan_user_data(). This data
    can be retrieved at a later time by calling sdvr_get_chan_user_data().

    Enable motion, blind, and night detection using 
    sdvr_enable_motion_detection(), sdvr_enable_blind_detection(), and
    sdvr_enable_night_detection(), respectively.

    Enable privacy regions using sdvr_enable_privacy_regions().

  The hardware is now set up for encoding, decoding, and display. 

  SUBSECTION: Video Analytics and Privacy Blocking
  Stretch provides variety of video frame analysis to be used with various
  video alarm detection as well as blocking different regions within 
  video frame for privacy. These video frame analysis include the Motion
  Detection (MD), Blind Detection (BD), Night Detection (ND) within a
  video frame according to some user defined Regions of Interest (ROI) that
  are based on Macro Blocks (MB). The size of a MB is 16x16 pixels. The
  ROIs are always marked within a 1:1 decimation of a video standard size. (See 
  sdvr_set_regions_map() for more information.)

  To start any of the video alarm detections or setting up privacy blocking, you
  must specify ROIs within a 1:1 decimation of a video standard size for each 
  encoder channel.
  This can be accomplished by either calling sdvr_set_regions_map() or
  sdvr_set_regions(). (By default, the entire video frame is used for any of
  the alarm detections.) 

  The next step is to enable different alarm detections based on the currently
  specified ROIs for each channel and a specific threshold.  Enable motion, blind, 
  and night detection using  sdvr_enable_motion_detection() or sdvr_enable_motion_detection_ex(), 
  sdvr_enable_blind_detection(), and  sdvr_enable_night_detection(), respectively.
  Enable privacy regions using sdvr_enable_privacy_regions().
  Once alarm detections are enabled, the DVR Application gets notified through
  the alarm callback as alarms are triggered according to the given threshold. (By default 
  all of the video alarms are disabled.)
  Additionally, you receive different alarm values
  with every video frame (encoded or raw) as part of the video frame buffer. 
  You may choose to receive  motion values every 'N' video 
  frames by calling sdvr_set_motion_value_frequency(). (Note: Motion values
  are always generated once an alram is detected by the firmware
  regardless of the setting in sdvr_set_motion_value_frequency().)

  Having a motion value buffer, you can perform more detailed analysis
  by calling sdvr_motion_value_analyzer() and passing it different list of
  ROIs to detect motions in various regions of the current video frame.

  SUBSECTION: Encode
  The DVR Application needs to perform the following tasks during
  encoding. 

    Enable the encoder for each channel using sdvr_enable_encoder(). 

    Get a frame of encoded video for a particular channel using
    sdvr_get_av_buffer(). If a callback was registered for AV frames, 
    then the DVR Application should have received one or more callbacks
    with information about the channels for which encoded frames are
    available. (See sdvr_set_av_frame_callback()
    for a usage example of such a callback). That information can be used to 
    determine which channels have
    data and to request frames only from those channels. If, however, a callback
    was not registered, sdvr_get_av_buffer() can be used as a polling
    function - when channels have encoded AV frames this function returns 
    valid buffers, but returns appropriate error codes for channels 
    that do not have new frames available. 

    Stretch has implemented a one-copy buffer management policy. The buffers
    required to hold incoming data from the board (and data going to the
    board) are allocated in the SDK. The driver, however, 
    allocates a few buffers that are contiguous in physical memory to 
    enable efficient DMA. Data coming from the board is first stored in the
    driver buffers and then copied over to the SDK buffers. Similarly,
    data going to the board is first stored in the SDK buffers and then
    copied to the driver buffers before being DMAed to the board. 

    It is important to release buffers obtained from the SDK using
    sdvr_release_av_buffer() for encoded frames and sdvr_release_yuv_buffer()
    for raw video frames. The SDK then recycles these buffers 
    and uses them for holding future incoming frames. Under no 
    circumstances should a buffer be freed by the DVR Application. 
    Also, holding on to the buffer for too long causes the SDK 
    to run out of buffers, and frames are lost. It is therefore 
    important that enough buffers be allocated and that buffers are 
    released in a timely manner.

  It is the responsibility of the DVR Application to save the data to disk. 
  Stretch does not provide functions to write to disk because disk
  management and recording policy is dependent on the DVR Application and 
  best managed by it. 

  SUBSECTION: Decode
  The DVR Application needs to perform the following tasks during decoding.

    Set the encoded video frame size for each channel
    to be decoded by calling sdvr_set_decoder_size().

    Enable the decoder for each channel using sdvr_enable_decoder().

    In keeping with Stretch's buffer management policy whereby the
    SDK manages all the buffers, the
    DVR Application must request a free frame buffer from the SDK using
    sdvr_alloc_av_buffer(). 

    The DVR Application should fill this buffer with encoded data 
    from the disk. After this buffer is full, it can be sent to the 
    hardware for decoding using sdvr_send_av_frame(). Decoder buffers are
    released as part of the call to sdvr_send_av_frame(). In the event that
    a decoder buffer is acquired, but needs to be released without sending, 
    you can call sdvr_release_av_buffer().

    The decoded raw video frames can either be displayed on the SMO by calling
    sdvr_set_smo_grid() or requested to be sent to the DVR Application 
    for displaying on the host monitor by calling sdvr_stream_raw_video().
    Once the streaming of raw video frames is enabled for the decoded channel, the
    raw video frames will be sent for the corresponding decoder via the
    av_frame_callback function (refer to next Section, “Video Display and Sound” 
    for detailed information).

  SUBSECTION: Raw video and audio data
  The SDK provides raw (unencoded) audio and video for each channel 
  as a separate stream to the DVR Application. These raw A/V frames can be
  either from the live or decoded video stream. 

  The DVR Application needs 
  to do the following to get raw video and audio data.

    Enable streaming of video and audio from the hardware or decoder to the host
    using sdvr_stream_raw_video() and sdvr_stream_raw_audio() calls. 
    Although video for various channels can be enabled for streaming, it 
    makes sense to enable streaming for only one audio channel 
    (although the SDK supports streaming of multiple audio channels). 
    To conserve communication bandwidth between the host and the board, we 
    recommend that raw video streaming be enabled only for channels
    that are being displayed, and not for all channels.

    Register a callback function using sdvr_set_av_frame_callback()
    so that the DVR Application can be informed
    when raw audio and video frames are available. This is not 
    strictly required (see preceding discussion). 

    Call sdvr_get_av_buffer() to get the raw audio or encoded audio/video 
    frames from the SDK. Call sdvr_get_yuv_buffer() to get raw video frames.
    The format of raw video is YUV 4:2:0, and will be
    received as three separate buffers containing Y, U, and V.
    Use appropriate rendering and sound playback software
    and hardware to display the video and to play the sound. Video for 
    each channel is obtained separately, and it is the responsibility of the
    DVR Application to display the video in its appropriate window. Typically,
    the DVR Application creates tiled windows for each channel to be
    displayed, and the video for each channel is rendered in its own
    window. If interested, Stretch provides a UI SDK library that helps you
    to display video frames in any regions within a display window handle.
    (See sdvr_ui_sdk.html for details.)

    Release the frame buffer obtained in the previous step using
    sdvr_release_av_buffer() or sdvr_release_yuv_buffer() as appropriate. 
    Raw video buffers are large, so there will
    usually not be too many of these buffers per channel. Therefore, it
    is extremely important that these buffers be promptly released.


  SUBSECTION: On-Screen Display (OSD)
  Each encoder or decoder channel can have two different OSD text items to be
  displayed at any position in the video frame. To show OSD, 
  the DVR Application needs to do the following:

    It first needs to configure each OSD text using sdvr_osd_text_config_ex().
    The "osd_text_config" data structure allows the DVR Application to 
    specify the text string, its location, whether date and time should
    be appended to the string, and if so, the style in which the date and 
    time is displayed. Stretch provides muliti-language support through multiple 
    font tables. You can load a non-English language font by calling 
    sdvr_osd_set_font_table(). If no font table is loaded, the default English
    font is used.

    After an OSD text item is configured, it can be shown or hidden using 
    sdvr_osd_text_show(). In 
    the case of encoder channels, OSD text is blended into the video before the video is 
    encoded. Therefore, the OSD text present at the time of encoding is
    displayed during decoding. In the case of decoder channels, OSD text is added
    after the video frame is decoded. Therefore, it is possible to have two different
    OSD text items on the decoded frame, one when it was encoded and the other 
    when it is decoded.

    @NOTE: New style OSD (OSD with transparent background and Unicode UTF-16 coding 
    scheme supporting Basic Multilingual Plane (BMP)) is only supported for encoder 
    channels with vpp_mode fields set to SDVR_VPP_MODE_SLATERAL.@

  SUBSECTION: Spot Monitor Output (SMO)
  Stretch provides displaying of the live video output from
  each encoder channel or the decode frames output of each decode channel on any where
  within the spot monitor display. The combination of SMO display location and
  its video decimation for each channel is called a SMO grid. Each SMO grid
  can be defined and enabled by calling sdvr_set_smo_grid(). You can achieve different
  display pattern by placing the channel video out put in various display location.

  Stretch provides two SMO feature solutions. One is the "analog SMO", which uses an analog
  switch to provide a  direct pass through of one video input channel at a time to the Spot
  Monitor output. In this mode, only one video input channel can be displayed at a time, 
  and it is always displayed at its full resolution. No OSD text and no output
  from decoder channels can be viewed in this mode.
  The other SMO mode ("full SMO") provides full-function SMO capabilities supporting multiple
  channel display and OSD. If this mode is selected, the last video input channel is lost.
  This means that in a 16 channel board only 15 channels are available.

  To enable SMO, the DVR Application needs to do the following:

    Specify the grid pattern using sdvr_set_smo_grid().
    The SMO grid pattern is flexible and is defined by the user. Each tile can 
    be either enabled or disabled. When enabled, there are one or more 
    channels to display at this tile. If disabled, the tile is not used.
    The channel specified at each location can be either an encoding
    or decoding channel. If it is an encoding channel, live video is 
    displayed, and if it is a decode channel then playback video is displayed.
    Each tile also has a display resolution decimation.

    Setting the SMO configuration using sdvr_set_smo_grid() enables displaying
    of SMO. No further action is required on the part of the DVR Application.

  @NOTE@: In "analog SMO" mode, only one tile at a time can be specified by calling
  sdvr_set_smo_grid(), since the display can handle only one channel at a time.
  To rotate the display between multiple channels (i.e. implement "dwell time")
  the host application must implement the logic to do so, by disabling one channel
  and enabling the next one every time period.

  SUBSECTION: Pan, Tilt, and Zoom
  Typically, the PTZ controller for a camera is connected to the processor
  implementing the PTZ protocol through an RS-485 interface. The S6
  processor does not have an RS-485 interface, so the Stretch reference 
  design provides the mechanism to talk to the RS-485 port on the alarm 
  I/O card. 
  
  The SDK does not support any particular PTZ protocol. Instead, it gives
  access to the RS-485 interface through the RS-232C interface. The PTZ
  section of the SDK allows the DVR Application to set up the RS232C port
  on the S6 to talk to the RS-485. The following functions are available:

    sdvr_init_uart() allows the DVR Application to set the baud rate, 
    stop bits, etc. for the UART interface.

    sdvr_read_uart() returns up to 255 character from the UART port.

    sdvr_write_uart() writes up to 255 character to the UART port.

  SUBSECTION: Sensors and Relays
  Sensors are external inputs that can be triggered, for example, by a 
  door opening. The SDK allows the DVR Application to register a callback
  so that any time one or more sensors are triggered, the callback function
  is called. This callback is registered using sdvr_set_sensor_callback()
  and the type of the callback function is sdvr_sensor_callback. Some sensors
  are edge triggered whereas others are level sensitive. You can specify
  how each sensor should be triggered, as well as its initial enable status
  by calling sdvr_config_sensors(). You can also enable or disable an 
  individual sensor at different times by calling sdvr_enable_sensor().

  Relays are actuators that are activated by the DVR Application. To 
  activate or deactivate a relay, the DVR Application calls 
  sdvr_trigger_relay() with the proper value for the "is_triggered"
  flag.

  SUBSECTION: System Shutdown
  The DVR Application needs to perform the following tasks during
  shutdown:

    Disable encoding on all channels using sdvr_enable_encoder(), with the
    "enable" flag set to false.

    Disable decoding on all channels using sdvr_enable_decoder(), with the
    "enable" flag set to false.

    Disable all relays using sdvr_trigger_relay(), with the "is_triggered"
    flag set to false.

    Disconnect from boards using sdvr_board_disconnect(). This frees up
    all board-specific resources in the SDK and driver.
    @NOTE:  After you call sdvr_board_disconnect(), you must load the firmware
    by calling sdvr_upgrade_firmware() every time prior to re-connecting 
    to the DVR board if no firmware is 
    loaded into the board's none-volatile  memory.@

    Close the SDK and free up all system resources using sdvr_sdk_close().
    Although disabling and destroying all the channels, as well as disconnecting 
    from all boards is good programming  practice,
    it is not required because sdvr_sdk_close() performs these actions.
****************************************************************************/

#ifndef __H_SXTYPES
#define __H_SXTYPES


/****************************************************************************
    8-bit unsigned integer.
****************************************************************************/
typedef unsigned char  sx_uint8;

/****************************************************************************
    16-bit unsigned integer.
****************************************************************************/
typedef unsigned short sx_uint16;

/****************************************************************************
    32-bit unsigned integer.
****************************************************************************/
typedef unsigned int   sx_uint32;

/****************************************************************************
    64-bit unsigned integer.
****************************************************************************/
typedef unsigned long long sx_uint64;

/****************************************************************************
    Boolean value.
****************************************************************************/
typedef unsigned int   sx_bool;

/****************************************************************************
    true is a non-zero value
****************************************************************************/

#ifndef true
#define true 1
#endif

/****************************************************************************
    false is zero value
****************************************************************************/

#ifndef false
#define false 0
#endif

/****************************************************************************
    8-bit signed integer.
****************************************************************************/
typedef signed char               sx_int8;     // 8  bit signed integer

/****************************************************************************
    16-bit signed integer.
****************************************************************************/
typedef  short              sx_int16;    // 16 bit signed integer

/****************************************************************************
    32-bit signed integer.
****************************************************************************/
typedef  int                sx_int32;    // 32 bit signed integer

/****************************************************************************
    64-bit signed integer.
****************************************************************************/
typedef  long long          sx_int64;    // 64 bit signed integer 
#endif

/****************************************************************************
    32 bit time value, the number of seconds elapsed since midnight 01/01/1970.
****************************************************************************/
#ifndef _TIME_T_DEFINED
typedef    long    time_t;
#define    _TIME_T_DEFINED
#endif

/****************************************************************************
  VISIBLE: Typedef for the errors returned by the SDK.

    SDVR_ERR_NONE - No error, or in other words, success!

    @Following error codes are generated by the DVR firmware:@

    SDVR_FRMW_ERR_WRONG_CAMERA_NUMBER - The given camera number is invalid.

    SDVR_FRMW_ERR_WRONG_CAMERA_TYPE - Error code if the specified video standard
    is not supported by the firmware.

    SDVR_FRMW_ERR_WRONG_CODEC_FORMAT - Error code if the specified video
    codec is not supported.

    SDVR_FRMW_ERR_WRONG_CODEC_RESOLUTION

    SDVR_FRMW_ERR_WRONG_CHANNEL_TYPE

    SDVR_FRMW_ERR_WRONG_CHANNEL_ID

    SDVR_FRMW_ERR_WRONG_VIDEO_FORMAT

    SDVR_FRMW_ERR_WRONG_AUDIO_FORMAT

    SDVR_FRMW_ERR_EXCEED_CPU_LIMIT

    SDVR_FRMW_ERR_CHANNEL_NOT_CREATED

    SDVR_FRMW_ERR_CHANNEL_ALREADY_CREATED

    SDVR_FRMW_ERR_CHANNEL_NOT_ENABLED

    SDVR_FRMW_ERR_CHANNEL_NOT_DISABLED

    SDVR_FRMW_ERR_INVALID_TIME

    SDVR_FRMW_ERR_ILLEGAL_SMO_PARAMS

    SDVR_FRMW_ERR_SMO_NOT_SUPPORTED - There is no SMO support in the firmware.

    SDVR_FRMW_ERR_VDET_ERROR - Error code if the firmware detects cameras 
    connected with mixed video standard connected. (i.e., some NTSC cameras and
    some PAL cameras).

    SDVR_FRMW_ERR_RUNTIME_ERROR

    SDVR_FRMW_ERR_VPP_RUNTIME_ERROR

    SDVR_FRMW_ERR_ENCODER_RUNTIME_ERROR

    SDVR_FRMW_ERR_DECODER_RUNTIME_ERROR

    SDVR_FRMW_ERR_ILLEGAL_PARAMETER

    SDVR_FRMW_ERR_INTERNAL_ERROR

    SDVR_FRMW_ERR_ILLEGAL_COMMAND

    SDVR_FRMW_ERR_SMO_NOT_DISABLED - Error code if you tried to reset channel
    parameters to factory default while SMO was enabled.

    SDVR_FRMW_ERR_OUT_OF_MEMORY - Error code if the firmware runs out of memory
    in the middle of the current operation.

    SDVR_FRMW_ERR_NO_IO_BOARD - Error code if the current command requires
    an I/O board for the operation but the I/O board is not connected to the
    DVR board.

    SDVR_FRMW_ERR_AUDIO_RUNTIME

    SDVR_FRMW_ERR_UNSUPPORTED_COMMAND - Error code if the command is not 
    supported by the current version of the DVR firmware.

    SDVR_FRMW_ERR_SMO_CHAN_FAILED -

    SDVR_FRMW_ERR_RES_LIMIT_EXCEEDED - Error code if you exceeded 
    the processing capabilities for the current operation. (i.e. If you
    try to start 4 channels of D1 dual encoding.)

    @Following error codes are generated by the PCI driver interface:@

    SDVR_DRV_ERR_INVALID_PARAMETER - Refer to the function prototype for information.

    SDVR_DRV_ERR_BOARD_IN_USE - The given board index is already in use.

    SDVR_DRV_ERR_BOARD_CONNECT - Failed to connect to the board.

    SDVR_DRV_ERR_BOARD_CLOSE - Failed while trying to close the board.

    SDVR_DRV_ERR_BOARD_RESET - Error code if failed to reset the DVR board.

    SDVR_DRV_ERR_IPC_INIT

    SDVR_DRV_ERR_NO_CHANNELS

    SDVR_DRV_ERR_CHANNEL_IN_USE - If a receive channel has not been closed by
    the firmware.

    SDVR_DRV_ERR_CHANNEL_CREATE

    SDVR_DRV_ERR_CHANNEL_CONNECT

    SDVR_DRV_ERR_CHANNEL_CLOSE - If an error occurred while closing the 
    channel, or the firmware did not respond correctly to the close request.

    SDVR_DRV_ERR_CHANNEL_NOT_ACTIVE

    SDVR_DRV_ERR_CHANNEL_DEAD

    SDVR_DRV_ERR_NO_RECV_BUFFERS

    SDVR_DRV_ERR_NO_SEND_BUFFERS

    SDVR_DRV_ERR_MSG_SEND - Error code if the driver failed to send the
    command to the firmware.

    SDVR_DRV_ERR_MSG_RECV - Error code if the driver timeout while waiting
    to receive a response from the firmware. This error could be an indication
    that the code on the firmware hung.

    SDVR_DRV_BOARD_BOOT_FAIL - Failed to boot PCI board.

    @Following error codes are generated by the DVR SDK:@

    SDVR_ERR_OUT_OF_MEMORY - System is out of memory.

    SDVR_ERR_INVALID_HANDLE - Invalid buffer handle.

    SDVR_ERR_INVALID_ARG - Invalid argument to a function call.

    SDVR_ERR_INVALID_BOARD - Invalid board number.

    SDVR_ERR_BOARD_CONNECTED - The current operation is invalid while
    connecting to a board (i.e., setting SDK parameters or connect to a board
    that is already connected).

    SDVR_ERR_INVALID_CHANNEL - Invalid channel number.

    SDVR_ERR_CHANNEL_CLOSED - Channel is closed - cannot communicate with it.

    SDVR_ERR_BOARD_CLOSED - Board is closed - cannot communicate with it.

    SDVR_ERR_NO_VFRAME - No video frame is available.

    SDVR_ERR_NO_AFRAME - No audio frame is available.

    SDVR_ERR_INTERNAL - Internal error in the SDK. Please contact
    Stretch support for assistance.

    SDVR_ERR_BOARD_NOT_CONNECTED - The specified board index was not 
    connected.

    SDVR_ERR_IN_STREAMING - Failed to close the board because some channels
    are still active (encoding or decoding). 

    SDVR_ERR_NO_DVR_BOARD - No PCIe DVR board was found on the PC.

    SDVR_ERR_WRONG_DRIVER_VERSION - The current DVR PCIe driver is
    not supported.

    SDVR_ERR_DBG_FILE - Failed to open the debug file.

    SDVR_ERR_ENCODER_NOT_ENABLED - Failed to start the encoder on the 
    given channel.

    SDVR_ERR_ENCODER_NOT_DISABLED - Failed to stop the encoder on the 
    given channel.

    SDVR_ERR_SDK_NO_FRAME_BUF - There is not enough buffer allocated to 
    receive encoded or raw frame buffers. Or the frame size is zero.

    SDVR_ERR_INVALID_FRAME_TYPE - The given frame type is not supported.

    SDVR_ERR_NOBUF - No A/V buffer is available. 

    SDVR_ERR_CALLBACK_FAILED - Failed to register callback with the driver.

    SDVR_ERR_INVALID_CHAN_HANDLE - The given channel handle is invalid.

    SDVR_ERR_COMMAND_NOT_SUPPORTED - The function is not implemented.

    SDVR_ERR_ODD_SMO_COORDINATES - Error code if either of the x or y value of the
    SMO grid is an odd number.

    SDVR_ERR_LOAD_FIRMWARE - Error code if failed to load the firmware. This 
    could be as result of invalid file path or failure to load from PCIe driver.

    SDVR_ERR_WRONG_CHANNEL_TYPE - The channel handle belongs to wrong channel
    type for the current operation.

    SDVR_ERR_DECODER_NOT_ENABLED - Error code if we are trying to send 
    frames to be decoded but the decoder is not enabled.

    SDVR_ERR_BUF_NOT_AVAIL - Error code if no buffer is available to send
    frames.

    SDVR_ERR_MAX_REGIONS - Error code if the maximum allowed regions is reached
    when you request to add a new motion or blind detection as well as 
    privacy region.

    SDVR_ERR_INVALID_REGION - Error code if the given region is either does
    not exist or is invalid.

    SDVR_ERR_INVALID_GOP - Error code if the specified GOP value of the 
    encoder parameter to be set is zero.

    SDVR_ERR_INVALID_BITRATE - Error code if the specified maximum bit rate is
    less than the average bit rate while setting the encoder parameters.

    SDVR_ERR_INVALID_BITRATE_CONTROL - Error code if an unknown encoder bit rate
    control is specified.

    SDVR_ERR_INVALID_QUALITY - Error code if the encoder quality parameter
    is out of range for the current video encoder.

    SDVR_ERR_INVALID_FPS - Error code if the specified encoder frame rate is 
    not supported by the current video standard.

    SDVR_ERR_UNSUPPORTED_FIRMWARE - Error code if the DVR firmware version is
    not supported by the current version of SDK.

    SDVR_ERR_INVALID_OSD_ID - Error code if the specified OSD ID has not been
    configured before using or is out of the valid range of OSD items on a
    channel.

    SDVR_ERR_OSD_LENGTH - Error code if the given OSD text length is too large.

    SDVR_ERR_OSD_FONT_FILE - Error code if the given font table file does not
    exist, is invalid, or can not be opened.

    SDVR_ERR_FONT_ID - Error code if the given font ID does not fall within
    the valid range or the font table with this ID does not exist.

    SDVR_ERR_CAMERA_IN_REC - Error code if the camera that is being
    requested for recording, is currently in recording.

    SDVR_ERR_OPEN_REC_FILE - Error code for failure  to open the given
    file for recording.

    SDVR_ERR_FAILED_ADD_VIDEO_TRACK - Error code for failure to add
    the video track to the recording file.

    SDVR_ERR_FAILED_ADD_AUDIO_TRACK - Error code for failure  to add
    the audio track to the recording file.

    SDVR_ERR_SDK_BUF_EXCEEDED - Error code if any buffer size, while 
    setting up SDK buffers, exceeds the maximum allowed.

    SDVR_FRMW_ERR_AUDIO_RUNTIME - Error code if audio buffers overflow.

    SDVR_ERR_AUTH_KEY_MISSING - Error code if a required authentication key 
    is missing on the DVR board while connecting to it.

    SDVR_ERR_AUTH_KEY_LEN - Error code if the size of authentication key is
    larger than what the SDK can support.

    SDVR_ERR_INVALID_RESOLUTION - Error code if the specified resolution
    decimation is not supported by the current operation. (i.e. DCIF or
    2CIF resolution is not supported in SMO).

****************************************************************************/
typedef enum _sdvr_err_e {
    SDVR_ERR_NONE = 0,
    SDVR_FRMW_ERR_WRONG_CAMERA_NUMBER,
    SDVR_FRMW_ERR_WRONG_CAMERA_TYPE,
    SDVR_FRMW_ERR_WRONG_CODEC_FORMAT,
    SDVR_FRMW_ERR_WRONG_CODEC_RESOLUTION,
    SDVR_FRMW_ERR_WRONG_CHANNEL_TYPE,
    SDVR_FRMW_ERR_WRONG_CHANNEL_ID,
    SDVR_FRMW_ERR_WRONG_VIDEO_FORMAT,
    SDVR_FRMW_ERR_WRONG_AUDIO_FORMAT,
    SDVR_FRMW_ERR_EXCEED_CPU_LIMIT,
    SDVR_FRMW_ERR_CHANNEL_NOT_CREATED,
    SDVR_FRMW_ERR_CHANNEL_ALREADY_CREATED,
    SDVR_FRMW_ERR_CHANNEL_NOT_ENABLED,
    SDVR_FRMW_ERR_CHANNEL_NOT_DISABLED,
    SDVR_FRMW_ERR_SMO_NOT_CREATED,
    SDVR_FRMW_ERR_INVALID_TIME,
    SDVR_FRMW_ERR_ILLEGAL_SMO_PARAMS,
    SDVR_FRMW_ERR_SMO_NOT_SUPPORTED,
    SDVR_FRMW_ERR_VDET_ERROR,
    SDVR_FRMW_ERR_RUNTIME_ERROR,
    SDVR_FRMW_ERR_VPP_RUNTIME_ERROR,
    SDVR_FRMW_ERR_ENCODER_RUNTIME_ERROR,
    SDVR_FRMW_ERR_DECODER_RUNTIME_ERROR,
    SDVR_FRMW_ERR_ILLEGAL_PARAMETER,
    SDVR_FRMW_ERR_INTERNAL_ERROR,
    SDVR_FRMW_ERR_ILLEGAL_COMMAND,
    SDVR_FRMW_ERR_SMO_NOT_DISABLED,
    SDVR_FRMW_ERR_OUT_OF_MEMORY,
    SDVR_FRMW_ERR_NO_IO_BOARD,
    SDVR_FRMW_ERR_AUDIO_RUNTIME,
    SDVR_FRMW_ERR_UNSUPPORTED_COMMAND,
    SDVR_FRMW_ERR_SMO_CHAN_FAILED,
    SDVR_FRMW_ERR_RES_LIMIT_EXCEEDED,

    SDVR_DRV_ERR_MSG_RECV = 255,

    SDVR_DRV_ERR_INVALID_PARAMETER = 1000,
    SDVR_DRV_ERR_BOARD_IN_USE,
    SDVR_DRV_ERR_BOARD_CONNECT,
    SDVR_DRV_ERR_BOARD_CLOSE,
    SDVR_DRV_ERR_BOARD_RESET,
    SDVR_DRV_ERR_IPC_INIT,
    SDVR_DRV_ERR_NO_CHANNELS,
    SDVR_DRV_ERR_CHANNEL_IN_USE,
    SDVR_DRV_ERR_CHANNEL_CREATE,
    SDVR_DRV_ERR_CHANNEL_CONNECT,
    SDVR_DRV_ERR_CHANNEL_CLOSE,
    SDVR_DRV_ERR_CHANNEL_NOT_ACTIVE,
    SDVR_DRV_ERR_CHANNEL_DEAD,
    SDVR_DRV_ERR_NO_RECV_BUFFERS,
    SDVR_DRV_ERR_NO_SEND_BUFFERS,
    SDVR_DRV_ERR_MSG_SEND = 1015,
    /* 1016 is left missing*/
    SDVR_DRV_BOARD_BOOT_FAIL = 1017,
    
    SDVR_ERR_OUT_OF_MEMORY = 2000,
    SDVR_ERR_INVALID_HANDLE,
    SDVR_ERR_INVALID_ARG,
    SDVR_ERR_INVALID_BOARD,
    SDVR_ERR_BOARD_CONNECTED,
    SDVR_ERR_INVALID_CHANNEL,
    SDVR_ERR_CHANNEL_CLOSED,
    SDVR_ERR_BOARD_CLOSED,
    SDVR_ERR_NO_VFRAME,
    SDVR_ERR_NO_AFRAME,
    SDVR_ERR_INTERNAL,
    SDVR_ERR_BOARD_NOT_CONNECTED,
    SDVR_ERR_IN_STREAMING,
    SDVR_ERR_NO_DVR_BOARD,
    SDVR_ERR_WRONG_DRIVER_VERSION,
    SDVR_ERR_DBG_FILE,
    SDVR_ERR_ENCODER_NOT_ENABLED,
    SDVR_ERR_ENCODER_NOT_DISABLED,
    SDVR_ERR_SDK_NO_FRAME_BUF,
    SDVR_ERR_INVALID_FRAME_TYPE,
    SDVR_ERR_NOBUF,
    SDVR_ERR_CALLBACK_FAILED,
    SDVR_ERR_INVALID_CHAN_HANDLE,
    SDVR_ERR_COMMAND_NOT_SUPPORTED,
    SDVR_ERR_ODD_SMO_COORDINATES,
    SDVR_ERR_LOAD_FIRMWARE,
    SDVR_ERR_WRONG_CHANNEL_TYPE,
    SDVR_ERR_DECODER_NOT_ENABLED,
    SDVR_ERR_BUF_NOT_AVAIL,
    SDVR_ERR_MAX_REGIONS,
    SDVR_ERR_INVALID_REGION,
    SDVR_ERR_INVALID_GOP,
    SDVR_ERR_INVALID_BITRATE,
    SDVR_ERR_INVALID_BITRATE_CONTROL,
    SDVR_ERR_INVALID_QUALITY,
    SDVR_ERR_INVALID_FPS,
    SDVR_ERR_UNSUPPORTED_FIRMWARE,
    SDVR_ERR_INVALID_OSD_ID,
    SDVR_ERR_OSD_LENGTH,
    SDVR_ERR_OSD_FONT_FILE,
    SDVR_ERR_FONT_ID,
    SDVR_ERR_CAMERA_IN_REC,
    SDVR_ERR_OPEN_REC_FILE,
    SDVR_ERR_FAILED_ADD_VIDEO_TRACK,
    SDVR_ERR_FAILED_ADD_AUDIO_TRACK,
    SDVR_ERR_SDK_BUF_EXCEEDED,
    SDVR_ERR_AUTH_KEY_MISSING,
    SDVR_ERR_AUTH_KEY_LEN,
    SDVR_ERR_INVALID_RESOLUTION
} sdvr_err_e;

/****************************************************************************
    VISIBLE: Typedef for the board diagnostics codes.

        SDVR_DIAG_OK - All the diagnostic tests passed.

    @Diagnostics failure codes for Boot-loader DDR test:@

        SDVR_DIAG_DDR_WRITEREAD_FAIL - DDR write/read test failed.

        SDVR_DIAG_DDR_ADDRLINES_FAIL - DDR address lines test failed.

        SDVR_DIAG_DDR_BITFLIP_FAIL   - DDR bit-flip test failed.

        SDVR_DIAG_DDR_DMA_FAIL       - DDR DMA test failed.

        SDVR_DIAG_DDR_READ_DMA_FAIL  - DDR read/DMA test failed.

    @Diagnostics failure codes for PLL test:@

        SDVR_DIAG_PLL_TEST_MHZ       - Processor speed test failed.

        SDVR_DIAG_PLL_TEST_SYS       - PLL_SYS test failed.

        SDVR_DIAG_PLL_TEST_IO        - PLL_IO test failed.

        SDVR_DIAG_PLL_TEST_AIM       - PLL_AIM test failed.

        SDVR_DIAG_PLL_TEST_DP0       - PLL_DP0 test failed.

        SDVR_DIAG_PLL_TEST_DP2       - PLL_DP2 test failed.

        SDVR_DIAG_PLL_TEST_DDR       - DLL_DDR test failed.

    @Diagnostics failure codes for SPI flash test:@

        SDVR_DIAG_SPI_TEST_READ      - Flash read error.

        SDVR_DIAG_SPI_TEST_ERASE     - Flash erase error.

        SDVR_DIAG_SPI_TEST_PROG      - Flash program error.

        SDVR_DIAG_SPI_TEST_UNLOCK    - Flash unlock error.

        SDVR_DIAG_SPI_TEST_COMPARE   - Flash data compare error.

        SDVR_DIAG_SPI_TEST_MAINT     - Flash maintenance command error.

        SDVR_DIAG_SPI_TEST_MISC      - Miscellaneous Flash error.

    @Diagnostics failure codes for TWI EEPROM test:@

        SDVR_DIAG_TWI_EEPROM_TEST_READ     - TWI EEPROM read error.

        SDVR_DIAG_TWI_EEPROM_TEST_WRITE    - TWI EEPROM write error.

        SDVR_DIAG_TWI_EEPROM_TEST_INIT     - TWI EEPROM initialization error.

        SDVR_DIAG_TWI_EEPROM_TEST_COMPARE  - TWI EEPROM data compare error.

    @Diagnostics failure codes for Epson test:@

        SDVR_DIAG_EPSON_REG_TEST_INIT     - Epson test initialization error.

        SDVR_DIAG_EPSON_REG_TEST_WALKING  - Epson register bit-walk error.

    @Diagnostics failure codes for Techwell test:@

        SDVR_DIAG_TW2815_AUDIO_TEST_INIT      - Techwell audio test init error.

        SDVR_DIAG_TW2815_AUDIO_TEST_NO_AUDIO  - Techwell audio not received error.

        SDVR_DIAG_TW2815_REG_TEST             - Techwell register test error.

        SDVR_DIAG_TW2815_VIDEO_TEST_INIT      - Techwell video test init error.

        SDVR_DIAG_TW2815_VIDEO_TEST_NO_VIDEO  - Techwell video not received error.

        SDVR_DIAG_TW28XX_VIDEO_TEST_TIMEOUT   - Techwell video test timeout error.

        SDVR_DIAG_TW28XX_VIDDET_TEST_INIT_ERR     - Techwell video detect test init error.

        SDVR_DIAG_TW28XX_VIDDET_TEST_UNKNOWN_CHIP - Techwell video detect test unknown chip error.

        SDVR_DIAG_TW28XX_VIDDET_TEST_NO_INPUT_ERR - Techwell video detect test no input error.

        SDVR_DIAG_TW28XX_VIDDET_TEST_CONFLICT_ERR - Techwell video detect test conflict error.

        SDVR_DIAG_TW28XX_VIDDET_TEST_NO_SYNC_ERR  - Techwell video detect test no synch. error.

        SDVR_DIAG_TW28XX_AUDDET_TEST_NO_SYNC_ERR  - Techwell audio detect test no sync. error.

    @Diagnostics failure codes for PCIe test:@

        SDVR_DIAG_PCIE_EYEMASK_TEST_NO_CBB    - PCIe did not detect the CBB test board.

        SDVR_DIAG_PCIE_EYEMASK_TEST_ERR       - PCIe eye mask test failure.

        SDVR_DIAG_PCIE_EYEMASK_TEST_TIMEOUT   - PCIe eye mask test timeout.

*****************************************************************************/
typedef enum _sdvr_diag_code_e {
    SDVR_DIAG_OK                        = 0x00000000,

    /* Boot-loader DDR test errors */

    SDVR_DIAG_DDR_WRITEREAD_FAIL        = 0xb007e001,
    SDVR_DIAG_DDR_ADDRLINES_FAIL        = 0xb007e002,
    SDVR_DIAG_DDR_BITFLIP_FAIL          = 0xb007e003,
    SDVR_DIAG_DDR_DMA_FAIL              = 0xb007e004,
    SDVR_DIAG_DDR_READ_DMA_FAIL         = 0xb007e005,

    /* PLL test errors */

    SDVR_DIAG_PLL_TEST_MHZ               = 0x1000e001,
    SDVR_DIAG_PLL_TEST_SYS               = 0x1000e002,
    SDVR_DIAG_PLL_TEST_IO                = 0x1000e003,
    SDVR_DIAG_PLL_TEST_AIM               = 0x1000e004,
    SDVR_DIAG_PLL_TEST_DP0               = 0x1000e005,
    SDVR_DIAG_PLL_TEST_DP2               = 0x1000e006,
    SDVR_DIAG_PLL_TEST_DDR               = 0x1000e007,

    /* SPI flash test errors */

    SDVR_DIAG_SPI_TEST_READ              = 0x1001e001,
    SDVR_DIAG_SPI_TEST_ERASE             = 0x1001e002,
    SDVR_DIAG_SPI_TEST_PROG              = 0x1001e003,
    SDVR_DIAG_SPI_TEST_UNLOCK            = 0x1001e004,
    SDVR_DIAG_SPI_TEST_COMPARE           = 0x1001e005,
    SDVR_DIAG_SPI_TEST_MAINT             = 0x1001e006,
    SDVR_DIAG_SPI_TEST_MISC              = 0x1001e007,

    /* TWI EEPROM test errors */

    SDVR_DIAG_TWI_EEPROM_TEST_READ       = 0x1002e001,
    SDVR_DIAG_TWI_EEPROM_TEST_WRITE      = 0x1002e002,
    SDVR_DIAG_TWI_EEPROM_TEST_INIT       = 0x1002e003,
    SDVR_DIAG_TWI_EEPROM_TEST_COMPARE    = 0x1002e004,

    /* Epson test errors */

    SDVR_DIAG_EPSON_REG_TEST_INIT       = 0x1003e001,
    SDVR_DIAG_EPSON_REG_TEST_WALKING    = 0x1003e002,

    /* Techwell test errors */

    SDVR_DIAG_TW2815_AUDIO_TEST_INIT     = 0x1004e001,
    SDVR_DIAG_TW2815_AUDIO_TEST_NO_AUDIO = 0x1004e002,
    SDVR_DIAG_TW2815_REG_TEST            = 0x1004e003,
    SDVR_DIAG_TW2815_VIDEO_TEST_INIT     = 0x1004e004,
    SDVR_DIAG_TW2815_VIDEO_TEST_NO_VIDEO = 0x1004e005,
    SDVR_DIAG_TW28XX_VIDEO_TEST_TIMEOUT  = 0x1004e015,

    SDVR_DIAG_TW28XX_VIDDET_TEST_INIT_ERR     = 0x1004e006,
    SDVR_DIAG_TW28XX_VIDDET_TEST_UNKNOWN_CHIP = 0x1004e007,
    SDVR_DIAG_TW28XX_VIDDET_TEST_NO_INPUT_ERR = 0x1004e008,
    SDVR_DIAG_TW28XX_VIDDET_TEST_CONFLICT_ERR = 0x1004e009,
    SDVR_DIAG_TW28XX_VIDDET_TEST_NO_SYNC_ERR  = 0x1004e00a,
    SDVR_DIAG_TW28XX_AUDDET_TEST_NO_SYNC_ERR  = 0x1004e00b,

    /* PCIe test errors */

    SDVR_DIAG_PCIE_EYEMASK_TEST_NO_CBB   = 0x1005e001,
    SDVR_DIAG_PCIE_EYEMASK_TEST_ERR      = 0x1005e002,
    SDVR_DIAG_PCIE_EYEMASK_TEST_TIMEOUT  = 0x1005e003

} sdvr_diag_code_e;

/**************************************************************************
   VISIBLE: A handle to a channel. 
***************************************************************************/
typedef sx_int32 sdvr_chan_handle_t;

/****************************************************************************
  VISIBLE: Microsoft compiler work around, sdvr_signals_type_e cannot
  be enum because it is being used as :8 bit in the data structure.
*****************************************************************************/
typedef sx_uint8 sdvr_signals_type_e;

/************************************************************************
  VISIBLE: This enumerated type defines the types of asynchronous messages  
  that can be sent from the DVR firmware to the DVR Application.

    SDVR_SIGNAL_RUNTIME_ERROR - Indicates that a non-fatal runtime
    error has occurred on the board. There is extra data associated with
    this signal that gives more information regarding the error. The
    meaning of those values varies depending on where the error occurred.

    SDVR_SIGNAL_FATAL_ERROR - Indicates that a fatal error has
    occurred on the board. If this signal is received, the board must
    be reset. There is extra data associated with
    this signal that gives more information regarding the error. The
    meaning of those values varies depending on where the error occurred.
************************************************************************/
typedef enum _sdvr_signals_type_e {
    SDVR_SIGNAL_RUNTIME_ERROR = 1,
    SDVR_SIGNAL_FATAL_ERROR
} __sdvr_signals_type_e;

/****************************************************************************
  VISIBLE: This enumerated type defines the types of events that 
  can be detected by the video encoder.

    SDVR_VIDEO_ALARM_NONE - No event or alarm.

    SDVR_VIDEO_ALARM_MOTION - Motion detected.

    SDVR_VIDEO_ALARM_BLIND - Blind detected.

    SDVR_VIDEO_ALARM_NIGHT - Night detected.

    SDVR_VIDEO_ALARM_LOSS - Video loss detected. If a channel is set
    to be an encoding channel and no camera is connected to it, this
    video alarm is triggered whenever the encoder is enabled. 

    SDVR_VIDEO_ALARM_DETECTED - This alarm is sent when a video signal
    is detected on an encoding channel that was created but had no video
    signal previously.
****************************************************************************/
typedef enum _sdvr_video_alarm_e {
    SDVR_VIDEO_ALARM_NONE,
    SDVR_VIDEO_ALARM_MOTION,
    SDVR_VIDEO_ALARM_BLIND,
    SDVR_VIDEO_ALARM_NIGHT,
    SDVR_VIDEO_ALARM_LOSS,
    SDVR_VIDEO_ALARM_DETECTED
} sdvr_video_alarm_e;

/****************************************************************************
  VISIBLE: This enumerated type defines the types of regions that 
  can be defined for a camera.

    SDVR_REGION_MOTION - Motion detected region.

    SDVR_REGION_BLIND - Blind detected region.

    SDVR_REGION_PRIVACY - Privacy region.
****************************************************************************/

typedef enum _sdvr_regions_type_e {
    SDVR_REGION_MOTION = 1,
    SDVR_REGION_BLIND = 2,
    SDVR_REGION_PRIVACY = 4,
} sdvr_regions_type_e;

/****************************************************************************
  VISIBLE: The width and number of lines to be used to calculate regions of 
  interest (ROI) map buffer for different video standards. The size of ROI
  map buffer is calculated by (width x lines) for the currently specified
  video standard. 

    SDVR_REGION_MAP_D1_WIDTH - The width of ROI map for a D1 NTSC or PAL 
    video standard.

    SDVR_REGION_MAP_4CIF_WIDTH - The width of ROI map for a 4-CIF NTSC or 
    PAL video standard.

    SDVR_REGION_MAP_LINE_NTSC - The number of lines in ROI map for a NTSC video
    standard.

    SDVR_REGION_MAP_LINE_PAL - The number of lines in ROI map for a PAL video
    standard.

****************************************************************************/

#define SDVR_REGION_MAP_D1_WIDTH       45
#define SDVR_REGION_MAP_4CIF_WIDTH     44
#define SDVR_REGION_MAP_LINE_NTSC      30
#define SDVR_REGION_MAP_LINE_PAL       36

/************************************************************************
  VISIBLE: The current audio/video signal state on the channel in the
  sdvr_av_buffer_t structure.

    SDVR_AV_STATE_VIDEO_LOST - No video signal is being detected on the channel.

    SDVR_AV_STATE_AUDIO_LOST - No audio signal is being detected on the channel. 
************************************************************************/
#define	SDVR_AV_STATE_VIDEO_LOST	0x01
#define SDVR_AV_STATE_AUDIO_LOST	0x02

/****************************************************************************
  VISIBLE: This enumerated type defines the kind of frames that can 
  be exchanged between the SDK and the DVR Application. 

    SDVR_FRAME_RAW_VIDEO - Generic Raw video frame type. This is the
    type to pass to sdvr_get_yuv_buffer() to get the raw video frames.

    SDVR_FRAME_RAW_AUDIO - Generic Raw audio PCM frame. This is the
    type to pass to sdvr_get_av_buffer() to get the raw audio frames.

    SDVR_FRAME_H264_IDR - Encoded H.264 IDR frame.

    SDVR_FRAME_H264_I   - Encoded H.264 I frame.

    SDVR_FRAME_H264_P   - Encoded H.264 P frame.

    SDVR_FRAME_H264_B   - Encoded H.264 B frame.

    SDVR_FRAME_H264_SPS - Encoded H.264 SPS frame.

    SDVR_FRAME_H264_PPS - Encoded H.264 PPS frame.
        
    SDVR_FRAME_JPEG - Encoded JPEG image frame.

    SDVR_FRAME_G711 - Encoded G.711 audio frame.

    SDVR_FRAME_MPEG4_I   - Encoded MPEG4 I frame.

    SDVR_FRAME_MPEG4_P   - Encoded MPEG4 P frame.

    SDVR_FRAME_MPEG4_VOL - Encoded MPEG4 VOL frame which contains
    the video frame header information.

    SDVR_FRAME_MOTION_VALUES - Macro Block (MB) based motion values
    buffer. Each element in this buffer contains the motion value
    from 0 to 255 indicating amount of motion for a particular
    MB.

    SDVR_FRAME_VIDEO_ENCODED_PRIMARY - Any encoded video frame from the 
    primary encoder. This is the
    type to pass to sdvr_get_av_buffer() to get the primary encoded video frames.

    SDVR_FRAME_VIDEO_ENCODED_SECONDARY - Any encoded video frame from the 
    secondary encoder. This is the
    type to pass to sdvr_get_av_buffer() to get the secondary encoded video frames.

    SDVR_FRAME_AUDIO_ENCODED - Any encoded audio frame. This is the
    type to pass to sdvr_get_av_buffer() to get the encoded audio frames.
****************************************************************************/
typedef enum _sdvr_frame_type_e {
    SDVR_FRAME_RAW_Y_VIDEO = 0,
    SDVR_FRAME_RAW_U_VIDEO,
    SDVR_FRAME_RAW_V_VIDEO,
    SDVR_FRAME_RAW_AUDIO,
    SDVR_FRAME_H264_IDR,
    SDVR_FRAME_H264_I,
    SDVR_FRAME_H264_P,
    SDVR_FRAME_H264_B,
    SDVR_FRAME_H264_SPS,
    SDVR_FRAME_H264_PPS,
    SDVR_FRAME_JPEG,
    SDVR_FRAME_G711,
    SDVR_FRAME_MPEG4_I,
    SDVR_FRAME_MPEG4_P,
    SDVR_FRAME_MPEG4_B,
    SDVR_FRAME_MPEG4_VOL,
    SDVR_FRAME_MOTION_VALUES = 18,
    SDVR_FRAME_RAW_VIDEO = 100,
    SDVR_FRAME_VIDEO_ENCODED_PRIMARY,
    SDVR_FRAME_VIDEO_ENCODED_SECONDARY,
    SDVR_FRAME_AUDIO_ENCODED
}__sdvr_frame_type_e;
/****************************************************************************
  VISIBLE: Microsoft compiler work around, sdvr_frame_type_e cannot
  be enum because it is being used as :8 bit in the data structure.
*****************************************************************************/
typedef sx_uint8 sdvr_frame_type_e;
/****************************************************************************
  VISIBLE: Typedef describing locations for OSD.

    SDVR_LOC_TOP_LEFT - Top left of the screen

    SDVR_LOC_TOP_RIGHT - Top right of the screen

    SDVR_LOC_BOTTOM_LEFT - Bottom left of the screen

    SDVR_LOC_BOTTOM_RIGHT - Bottom right of the screen

    SDVR_LOC_CUSTOM - A user-defined position. The upper left corner of 
    the video frame is the origin (0,0).
****************************************************************************/
typedef enum _sdvr_location_e {
    SDVR_LOC_TOP_LEFT = 0,
    SDVR_LOC_BOTTOM_LEFT,
    SDVR_LOC_TOP_RIGHT,
    SDVR_LOC_BOTTOM_RIGHT,
    SDVR_LOC_CUSTOM
} sdvr_location_e;

/****************************************************************************
  VISIBLE: This enumerated type describes the video standards supported 
  by SDVR. 

    SDVR_VIDEO_STD_NONE - No standard defined

    SDVR_VIDEO_STD_D1_PAL  - PAL 720x576 at 25 fps.

    SDVR_VIDEO_STD_D1_NTSC - NTSC 720x480 at 30 fps.

    SDVR_VIDEO_STD_CIF_PAL - PAL 352x288 at 25 fps.

    SDVR_VIDEO_STD_CIF_NTSC - NTSC 352x240 at 30 fps.

    SDVR_VIDEO_STD_2CIF_PAL - PAL 704x288 at 25 fps.

    SDVR_VIDEO_STD_2CIF_NTSC - NTSC 704x240 at 30 fps.

    SDVR_VIDEO_STD_4CIF_PAL - PAL 704x576 at 25 fps.

    SDVR_VIDEO_STD_4CIF_NTSC - NTSC 704x480 at 30 fps.

    SDVR_VIDEO_STD_QCIF_PAL - PAL 176x144 at 25 fps.

    SDVR_VIDEO_STD_QCIF_NTSC - NTSC 176x112 at 30 fps.
    @NOTE: This is not an standard QCIF size 176x120@
************************************************************************/
typedef enum _sdvr_video_std_e {
    SDVR_VIDEO_STD_NONE = 0,
    SDVR_VIDEO_STD_D1_PAL    = (1 << 0),
    SDVR_VIDEO_STD_D1_NTSC   = (1 << 1),
    SDVR_VIDEO_STD_CIF_PAL   = (1 << 2),
    SDVR_VIDEO_STD_CIF_NTSC  = (1 << 3),
    SDVR_VIDEO_STD_2CIF_PAL  = (1 << 4),
    SDVR_VIDEO_STD_2CIF_NTSC = (1 << 5),
    SDVR_VIDEO_STD_4CIF_PAL  = (1 << 6),
    SDVR_VIDEO_STD_4CIF_NTSC = (1 << 7),
    SDVR_VIDEO_STD_QCIF_PAL =  (1 << 8),
    SDVR_VIDEO_STD_QCIF_NTSC = (1 << 9)
} sdvr_video_std_e;

/****************************************************************************
  VISIBLE: This enumerated type describes the various supported video sizes.

    SDVR_VIDEO_SIZE_720x576 - D1-PAL video width of 720 and number of 
    lines of 576.

    SDVR_VIDEO_SIZE_720x480 - D1-NTSC video width of 720 and number of 
    lines of 480.

    SDVR_VIDEO_SIZE_352x288 - CIF-PAL video width of 352 and number of 
    lines of 288.

    SDVR_VIDEO_SIZE_352x240 - CIF-NTSC video width of 352 and number of 
    lines of 240.

    SDVR_VIDEO_SIZE_704x288 - 2CIF-PAL video width of 704 and number of 
    lines of 288.

    SDVR_VIDEO_SIZE_704x240 - 2CIF-NTSC video width of 704 and number of 
    lines of 240.

    SDVR_VIDEO_SIZE_704x576 - 4CIF-PAL video width of 704 and number of 
    lines of 576.

    SDVR_VIDEO_SIZE_704x480 - 4CIF-NTSC video width of 704 and number of 
    lines of 480.

    SDVR_VIDEO_SIZE_176x144 - QCIF_PAL video width of 176 and number of
    lines of 144.
  
    SDVR_VIDEO_SIZE_176x112 - QCIF_NTSC video width of 176 and number of
    lines of 112. @NOTE: This is not an standard QCIF size 176x120@

    SDVR_VIDEO_SIZE_528x320 - DCIF based on 4CIF NTSC video width of 528 and number of
    lines of 320.

    SDVR_VIDEO_SIZE_528x384 - DCIF based on 4CIF PAL video width of 528 and number of
    lines of 384.

    SDVR_VIDEO_SIZE_720x288 - HALF-D1-PAL video width of 720 and number of 
    lines of 288.

    SDVR_VIDEO_SIZE_720x240 - HALF_D1-NTSC video width of 720 and number of 
    lines of 240.


************************************************************************/
typedef enum _sdvr_video_size_e {
    SDVR_VIDEO_SIZE_720x576 = (1 << 0),
    SDVR_VIDEO_SIZE_720x480 = (1 << 1),
    SDVR_VIDEO_SIZE_352x288 = (1 << 2),
    SDVR_VIDEO_SIZE_352x240 = (1 << 3),
    SDVR_VIDEO_SIZE_704x288 = (1 << 4),
    SDVR_VIDEO_SIZE_704x240 = (1 << 5),
    SDVR_VIDEO_SIZE_704x576 = (1 << 6),
    SDVR_VIDEO_SIZE_704x480 = (1 << 7),
    SDVR_VIDEO_SIZE_176x144 = (1 << 8),
    SDVR_VIDEO_SIZE_176x112 = (1 << 9),
    SDVR_VIDEO_SIZE_528x320 = (1 << 10),
    SDVR_VIDEO_SIZE_528x384 = (1 << 11),
    SDVR_VIDEO_SIZE_720x288 = (1 << 12),
    SDVR_VIDEO_SIZE_720x240 = (1 << 13)
} sdvr_video_size_e;

/****************************************************************************
  VISIBLE: Microsoft compiler work around, sdvr_chan_type_e cannot
  be enum because it is being used as :8 bit in the data structure.
*****************************************************************************/
typedef sx_uint8 sdvr_chan_type_e;

/****************************************************************************
  VISIBLE: This enumerated type describes the kinds of channels supported 
  by SDVR. To create a channel that only allows it to be used in HMO or 
  SMO, you must use SDVR_CHAN_TYPE_ENCODER, and set the encoder type to 
  SDVR_VIDEO_ENC_NONE.

    SDVR_CHAN_TYPE_NONE - Channel type not specified. 

    SDVR_CHAN_TYPE_ENCODER - Encoder channel.

    SDVR_CHAN_TYPE_DECODER - Decoder channel.
****************************************************************************/
typedef enum _sdvr_chan_type_e{
    SDVR_CHAN_TYPE_NONE = 255,
    SDVR_CHAN_TYPE_ENCODER = 0,
    SDVR_CHAN_TYPE_DECODER = 2
}__sdvr_chan_type_e;

/****************************************************************************
  VISIBLE: Microsoft compiler work around, sdvr_vpp_mode_e cannot
  be enum because it is being used as :8 bit in the data structure.
*****************************************************************************/
typedef sx_uint8 sdvr_vpp_mode_e;

/************************************************************************
    VISIBLE: Enumerated type describing video pre-processing modes.
    
    @SDVR_VPP_MODE_ANALYTICS@    Run VPP in analytics mode.

    @SDVR_VPP_MODE_SLATERAL@     Run VPP in Stretch-lateral-filter mode.

    @Note: Privacy blanking and blind detection are not supported SDVR_VPP_MODE_SLATERAL.@
************************************************************************/
enum _sdvr_vpp_mode_e {
    SDVR_VPP_MODE_SLATERAL   = 0,
    SDVR_VPP_MODE_ANALYTICS  = 1,
};

/****************************************************************************
  VISIBLE: Enumerated type describing encoder subchannels supported 
  by the SDVR.

    SDVR_ENC_PRIMARY - The primary encoder.

    SDVR_ENC_SECONDARY - The secondary encoder.

****************************************************************************/
typedef enum _sdvr_sub_encoders_e {
    SDVR_ENC_PRIMARY,
    SDVR_ENC_SECONDARY
} sdvr_sub_encoders_e;

/****************************************************************************
  VISIBLE: Enumerated type describing video encoders supported by the SDVR.

    SDVR_VIDEO_ENC_NONE - No video encoder specified.

    SDVR_VIDEO_ENC_H264 - H.264 encoder.

    SDVR_VIDEO_ENC_JPEG - Motion JPEG encoder.

    SDVR_VIDEO_ENC_MPEG4 - MPEG4 encoder.
****************************************************************************/
typedef enum _sdvr_venc_e {
    SDVR_VIDEO_ENC_NONE,
    SDVR_VIDEO_ENC_H264,
    SDVR_VIDEO_ENC_JPEG,
    SDVR_VIDEO_ENC_MPEG4
} sdvr_venc_e;

/****************************************************************************
  VISIBLE: Enumerated type describing audio encoders supported by the SDVR.

    SDVR_AUDIO_ENC_NONE - No audio encoder specified.

    SDVR_AUDIO_ENC_G711 - G.711 audio encoder.

    SDVR_AUDIO_ENC_G726_16 - G.726 audio encoder at 16K bits/sec.

    SDVR_AUDIO_ENC_G726_24 - G.726 audio encoder at 24K bits/sec.

    SDVR_AUDIO_ENC_G726_32 - G.726 audio encoder at 32K bits/sec.

    SDVR_AUDIO_ENC_G726_48 - G.726 audio encoder at 48K bits/sec.

****************************************************************************/
typedef enum _sdvr_aenc_e {
    SDVR_AUDIO_ENC_NONE,
    SDVR_AUDIO_ENC_G711,
    SDVR_AUDIO_ENC_G726_16K,
    SDVR_AUDIO_ENC_G726_24K,
    SDVR_AUDIO_ENC_G726_32K,
    SDVR_AUDIO_ENC_G726_48K
} sdvr_aenc_e;

/****************************************************************************
  VISIBLE: Enumerated type describing the various encoding and display 
  resolution decimation.

  When configuring the DVR, the DVR Application sets the maximum 
  system-wide resolution. The video resolution for a particular channel
  is described in terms of this maximum resolution. The video resolution
  for a channel can be the same as, 1/4 of (CIF), or 1/16 (QCIF), 2CIF,
  or DCIF of the maximum system-wide resolution.

    SDVR_VIDEO_RES_DECIMATION_NONE - No resolution set.

    SDVR_VIDEO_RES_DECIMATION_EQUAL - Same resolution as the maximum.
    For D1, it is 720x480 for NTSC and 720x576 for PAL. For 4CIF, it is 704x480 
    for NTSC and 704x576 for PAL.

    SDVR_VIDEO_RES_DECIMATION_CIF - 1/4 of the maximum resolution. CIF
    size is 352x240 for NTSC and 352x288 for PAL.

    SDVR_VIDEO_RES_DECIMATION_FOURTH - Same a SDVR_VIDEO_RES_DECIMATION_CIF.
    Kept for backward compatibility.

    SDVR_VIDEO_RES_DECIMATION_QCIF - 1/16 of the maximum resolution.
    QCIF size is 176x112 for NTSC and 176x144 for PAL.

    SDVR_VIDEO_RES_DECIMATION_SIXTEENTH - Same a SDVR_VIDEO_RES_DECIMATION_QCIF.
    Kept for backward compatibility.

    SDVR_VIDEO_RES_DECIMATION_HALF - 2CIF size. It is same width as 
    SDVR_VIDEO_RES_DECIMATION_EQUAL but half the height.
    
    SDVR_VIDEO_RES_DECIMATION_DCIF - DCIF size. It is 528x320 for NTSC and
    528x384 for PAL.

****************************************************************************/
typedef enum _sdvr_video_res_decimation_e {
    SDVR_VIDEO_RES_DECIMATION_NONE = 0,
    SDVR_VIDEO_RES_DECIMATION_EQUAL = 1,
    SDVR_VIDEO_RES_DECIMATION_CIF = 2,
    SDVR_VIDEO_RES_DECIMATION_FOURTH = SDVR_VIDEO_RES_DECIMATION_CIF,
    SDVR_VIDEO_RES_DECIMATION_QCIF = 4,
    SDVR_VIDEO_RES_DECIMATION_SIXTEENTH = SDVR_VIDEO_RES_DECIMATION_QCIF,
    SDVR_VIDEO_RES_DECIMATION_HALF = 5,
    SDVR_VIDEO_RES_DECIMATION_DCIF = 6
} sdvr_video_res_decimation_e;

/****************************************************************************
  VISIBLE: Enumerated type describing various bit rate control schemes 
  available in the SDVR.

    SDVR_BITRATE_CONTROL_NONE - No bit rate control.

    SDVR_BITRATE_CONTROL_VBR - Variable bit rate.

    SDVR_BITRATE_CONTROL_CBR - Constant bit rate.

    SDVR_BITRATE_CONTROL_CQP - Quantization parameter - Not Supported.

    SDVR_BITRATE_CONTROL_CONSTANT_QUALITY - Constant quality bit rate.

****************************************************************************/
typedef enum _sdvr_br_control_e{
    SDVR_BITRATE_CONTROL_NONE = 255,
    SDVR_BITRATE_CONTROL_VBR = 0,
    SDVR_BITRATE_CONTROL_CBR,
    SDVR_BITRATE_CONTROL_CQP,
    SDVR_BITRATE_CONTROL_CONSTANT_QUALITY
} sdvr_br_control_e;

/****************************************************************************
    VISIBLE: Enumerated type describing the date and time display styles
    supported by the SDVR.

    SDVR_OSD_DTS_NONE - No date and time is displayed after the
    OSD text.

    SDVR_OSD_DTS_DEBUG - Enables a special debug display mode.

    SDVR_OSD_DTS_MDY_12H - Displays the date in Month/Day/Year format
    followed by time in HH:MM:SS am/pm format.

    SDVR_OSD_DTS_DMY_12H - Displays the date in Day/Month/Year format
    followed by time in HH:MM:SS am/pm format.

    SDVR_OSD_DTS_YMD_12H - Displays the date in Year/Month/Day format
    followed by time in HH:MM:SS am/pm format.

    SDVR_OSD_DTS_MDY_24H - Displays the date in Month/Day/Year format
    followed by time in 24 hour HH:MM:SS format.

    SDVR_OSD_DTS_DMY_24H - Displays the date in Day/Month/Year format
    followed by time in 24 hour HH:MM:SS format.

    SDVR_OSD_DTS_YMD_24H - Displays the date in Year/Month/Day format
    followed by time in 24 hour HH:MM:SS format.
****************************************************************************/
typedef enum _sdvr_dts_style_e {
    SDVR_OSD_DTS_NONE = 0,
    SDVR_OSD_DTS_DEBUG,
    SDVR_OSD_DTS_MDY_12H,
    SDVR_OSD_DTS_DMY_12H,
    SDVR_OSD_DTS_YMD_12H,
    SDVR_OSD_DTS_MDY_24H,
    SDVR_OSD_DTS_DMY_24H,
    SDVR_OSD_DTS_YMD_24H,
} sdvr_dts_style_e;

/************************************************************************
    VISIBLE: Chip revision definitions.
************************************************************************/
typedef enum _sdvr_chip_rev_e {
    SDVR_CHIP_S6100_3_REV_C = 0,
    SDVR_CHIP_S6105_3_REV_C,
    SDVR_CHIP_S6106_3_REV_C,
    SDVR_CHIP_S6100_3_REV_D = 16,
    SDVR_CHIP_S6105_3_REV_D,
    SDVR_CHIP_S6106_3_REV_D,
    SDVR_CHIP_S6100_3_REV_F = 32,
    SDVR_CHIP_S6105_3_REV_F,
    SDVR_CHIP_S6106_3_REV_F,
    SDVR_CHIP_S6100_3_UNKNOWN = 48,
    SDVR_CHIP_S6105_3_UNKNOWN,
    SDVR_CHIP_S6106_3_UNKNOWN,
    SDVR_CHIP_UNKNOWN = 255
}sdvr_chip_rev_e;

/*******************************************************************************
    Length of the serial number string.
*******************************************************************************/
#define SDVR_BOARD_SERIAL_LENGTH      16

/****************************************************************************
  VISIBLE: This data structure holds board attributes.

    @pci_slot_num@ - The PCI slot number in which the board is located.

    @board_type@ - The board type which is combination of device_id and
    the subsystem_vendor.

    @supported_video_stds@ - Specifies what cameras are supported by the board.
    It is a bit-wise OR of video standards, such as @SDVR_VIDEO_STD_D1_PAL@
    or @SDVR_VIDEO_STD_D1_NTSC@.

    @chip_revision@ - The Stretch chip number and revision. Refer to 
    @sdvr_chip_rev_e@ for a list of Stretch chip numbers.

    @board_revision@ - The board revision.

****************************************************************************/
typedef struct _sdvr_board_attrib_t {
    sx_uint32      pci_slot_num;
    sx_uint32      board_type;
    sx_uint32      supported_video_stds;
    sdvr_chip_rev_e chip_revision;
    sx_uint32      board_revision;
} sdvr_board_attrib_t;

/****************************************************************************
  VISIBLE: This data structure holds PCIe board attributes. You get these
  attributes by calling sdvr_get_pci_attrib(). This function can be called
  before or after loading of the firmware into the DVR board.

    @pci_slot_num@ - The PCI slot number in which the board is located.

    @board_type@ - The board type which is combination of device_id and
    the subsystem_vendor.

    @vendor_id@ - Always Stretch (0x18A2)

    @device_id@ - Board ID per each vendor.

    @subsystem_vendor@ - Vendor ID. For Stretch boards it is (0x18A2).

    @subsystem_id@ - Currently is always set to zero.

    @serial_number@ - A null terminated serial number string.
****************************************************************************/
typedef struct _sdvr_pci_attrib_t {
    sx_uint32       pci_slot_num;
    sx_uint32       board_type;
    sx_uint16       vendor_id;
    sx_uint16       device_id;
    sx_uint16       subsystem_vendor;
    sx_uint16       subsystem_id;
    sx_uint8        serial_number[SDVR_BOARD_SERIAL_LENGTH + 1];
} sdvr_pci_attrib_t;

/****************************************************************************
  VISIBLE: These debug flags are available to help you turn on 
  debugging in the SDK.

    @DEBUG_FLAG_DEBUGGING_ON@ - Turn on debugging.

    @DEBUG_FLAG_ALL@ - Turn on all debugging flags.

    @DEBUG_FLAG_WRITE_TO_FILE@ - Turn on writing debug information to file.

    @DEBUG_FLAG_OUTPUT_TO_SCREEN@ - Write debug information to TTY.

    @DEBUG_FLAG_ENCODER@ - Turn on debugging for encoder.

    @DEBUG_FLAG_DECODER@ - Turn on debugging for decoder.

    @DEBUG_FLAG_VIDEO_ALARM@ - Turn on debugging for video alarm.

    @DEBUG_FLAG_SENSORS_RELAYS@ - Turn on debugging for sensors and relays.

    @DEBUG_FLAG_AUDIO_OPERATIONS@ - Turn on debugging for audio operations.

    @DEBUG_FLAG_DISPLAY_OPERATIONS@ - Turn on debugging for video operations.

    @DEBUG_FLAG_BOARD@ - Turn on debugging for board configuration.

    @DEBUG_FLAG_GENERAL_SDK@ - Turn on debugging for SDK configuration.

    @DEBUG_FLAG_SMO@ - Turn on debugging for SMO configuration.

    @DEBUG_FLAG_OSD@ - Turn on debugging for OSD configuration.

    @DEBUG_FLAG_CHANNEL@ - Turn on debugging for channel configuration.

    @DEBUG_FLAG_VIDEO_FRAME@ - Turn on debugging of video frames.

    @DEBUG_FLAG_FW_WRITE_TO_FILE@ - Turn on firmware only messages.

    @DEBUG_FLAG_RECORD_TO_FILE@ - Turn on debugging of recording the
    audio and video frame to a file.
****************************************************************************/
#define DEBUG_FLAG_DEBUGGING_ON       0x1
#define DEBUG_FLAG_ALL                0xFFFFFFFE
#define DEBUG_FLAG_WRITE_TO_FILE      0x2
#define DEBUG_FLAG_OUTPUT_TO_SCREEN   0x4 
#define DEBUG_FLAG_ENCODER            0x8
#define DEBUG_FLAG_DECODER            0x10
#define DEBUG_FLAG_VIDEO_ALARM        0x20
#define DEBUG_FLAG_SENSORS_RELAYS     0x40
#define DEBUG_FLAG_AUDIO_OPERATIONS   0x80
#define DEBUG_FLAG_DISPLAY_OPERATIONS 0x100
#define DEBUG_FLAG_BOARD              0x200
#define DEBUG_FLAG_GENERAL_SDK        0x400
#define DEBUG_FLAG_SMO                0x800
#define DEBUG_FLAG_OSD                0x1000
#define DEBUG_FLAG_CHANNEL            0x2000
#define DEBUG_FLAG_VIDEO_FRAME        0x4000
#define DEBUG_FLAG_FW_WRITE_TO_FILE   0x8000
#define DEBUG_FLAG_RECORD_TO_FILE     0x00010000 

/****************************************************************************
  VISIBLE: This data structure is used to hold SDK configuration 
  parameters. 

  To exchange encoded and raw video with the board, the SDK needs to
  allocate buffers to hold the data on its way to and from the DVR
  Application and the board. The number of buffers to be allocated on 
  each of these paths and the sizes of these buffers are set using this
  data structure. 

  We recommend that you use the default values for each frame buffer as if they
  are optimized for streaming at 30 fps for NTSC, and 25fps for PAL video standards.

    @enc_buf_num@ - The number of buffers to be allocated for each encoder
    channel. Each encoder channel will have the same number of 
    buffers allocated.
    It is important that you allocate enough buffers for encoded frames
    to be held between the times you can process them. The maximum allowed encoded
    audio and video frame buffer size is 22.
    The default value is 20. 
    NOTE: The size of each buffer is  determined by the firmware.

    @raw_buf_num@ - The number of buffers to be allocated for each channel that
    will be sending raw video to the DVR Application. Each channel sending
    raw video will have the same number of buffers. 
    Typically, you will need to display raw frames 30 times per second. 
    Therefore, you only need between 2 and 4 buffers to hold raw video.
    The maximum allowed raw audio and video frame buffer size is 5.
    The default value is 2.
    NOTE: The size of each buffer is determined by the firmware.

    @dec_buf_num@ - The number of buffers to be allocated for each decoder
    channel. Each decoder channel will have the same number of buffers 
    allocated. The maximum number of decoder buffers allowed is 5.
    The default value is 5.

    @dec_buf_size@ - The size of each buffer used to hold encoded frames on
    the way to the decoder hardware. Typically, this is the size of the
    largest encoded frame that needs to be decoded.
    The same buffer size is used across all decoder channels.
    The default value is 414720.

    @timeout@ - For a variety of reasons, it is possible for the board to 
    hang. By setting the timeout value, you specify when the SDK will
    give up on a response from the board and inform the DVR Application
    that the board is hung. The value of this parameter is in seconds. 
    A value of 0 indicates that there is no timeout and the SDK will
    wait indefinitely for the board to respond.
    Do not set the value of timeout too low, or during times of heavy
    traffic on the bus, you might get a false warning that the board
    has hung. Setting the value to 0 (no timeout), may cause the
    PC to hang if the firmware on the board dies. 
    The default value is 5 seconds.

    @debug_flag@ - This is a bit field of flags that can be set to enable
    various levels of debugging as defined by the debug flags.
    See the defines for DEBUG_FLAG_xxx for the definition of each field.
    Setting debugging flags has a noticeable effect on
    system performance. The default value is zero.

    @debug_file_name@ - The name of the file where the debug information
    is stored. This string should include the full path name to the 
    file, or a file in the current working directory (depends on the OS
    as to how this is defined) is created. 
    If the file does not exist, it is created. If it already exists, 
    it will be truncated. You also must enable the DEBUG_FLAG_WRITE_TO_FILE
    bit in the @debug_flag@ filed to save the debugging information
    into the file. In addition to the given specified @debug_file_name@, which 
    includes all the SDK tracing, a new file will be created with @_fw@ appended
    to the @debug_file_name@, which contains all the low level commands sent to the
    DVR firmware.
****************************************************************************/
typedef struct _sdvr_sdk_params_t {
    sx_uint32 enc_buf_num;      
    sx_uint32 raw_buf_num; 
    sx_uint32 dec_buf_num; 
    sx_uint32 dec_buf_size; 
    sx_uint32 timeout; 
    sx_uint32 debug_flag;
    char *debug_file_name;
} sdvr_sdk_params_t;

/****************************************************************************
  VISIBLE: This data structure defines the capabilities of the SDVR boards.

    @num_cameras_supported@ - Number of cameras supported by the board, i.e., 
    the number of physical camera connectors on the board.

    @num_microphones_supported@ - Number of microphones supported by the
    board, i.e., the number of physical microphone connectors on the board.

    @has_smo@ - If true, this board has a spot monitor output.

    @num_sensors@ - Number of sensors on this board.

    @num_relays@ - Number of relays on this board.

    @camera_type@ - The maximum resolution that was specified by the
    DVR Application when connecting to the board. Typically, all 
    cameras connected to the DVR have this resolution, hence the 
    name of the field.

    @num_decoders_supported@ - The number of decoding channel supported
    by the board. This value is zero if the board does not support decoding.
****************************************************************************/
typedef struct _sdvr_board_config_t {
    sx_uint32 num_cameras_supported;
    sx_uint32 num_microphones_supported;
    sx_bool has_smo;
    sx_uint32 num_sensors;
    sx_uint32 num_relays;
    sdvr_video_std_e camera_type;
    sx_uint32   num_decoders_supported;
} sdvr_board_config_t;

/****************************************************************************
  VISIBLE: This data structure defines the firmware, boot loader and 
  BSP version, and build information.

  Stretch follows the convention of using four numbers for version 
  control. A change in the major number indicates major changes to 
  functionality, a change in the minor number indicates minor 
  changes to functionality, and a change in the revision number indicates
  significant bug fixes that were introduced in the minor change functionality. 
  A change to the build number indicates only bug fixes that do not 
  change functionality. 

    @fw_major@ - The firmware major version number. A change in this field
    indicates major changes to functionality.

    @fw_minor@ - The firmware minor version number. A change in this field 
    indicates minor changes to functionality.

    @fw_revision@ - The firmware revision version number. A change in this
    field indicates significant bug fixes that were introduced in the minor 
    change functionality.
 
    @fw_build@ - The firmware build version number. A change in this field
    indicates only bug fixes that do not change functionality. 

    @fw_build_year@ - The year of firmware build.

    @fw_build_month@ - The month of firmware build.

    @fw_build_day@ - The day of firmware build.

    @bootloader_major@ - The major version number of bootloader.

    @bootloader_minor@ - The minor version number of boot loader.

    @bsp_major@ - The major version number of BSP.

    @bsp_minor@ - The minor version number of BSP.
****************************************************************************/
typedef struct _sdvr_firmware_ver_t {
    sx_uint8 fw_major;
    sx_uint8 fw_minor;
    sx_uint8 fw_revision;
    sx_uint8 fw_build;
    sx_uint16 fw_build_year;
    sx_uint8  fw_build_month;
    sx_uint8  fw_build_day;
    sx_uint8  bootloader_major;
    sx_uint8  bootloader_minor;
    sx_uint8  bsp_major;
    sx_uint8  bsp_minor;
} sdvr_firmware_ver_t;

/****************************************************************************
  VISIBLE: These defines specify the different maximum regions.

    @SDVR_MAX_MD_REGIONS@ - Maximum number of motion detection regions.

    @SDVR_MAX_BD_REGIONS@ - Maximum number of blind detection regions.

    @SDVR_MAX_PR_REGIONS@ - Maximum number of privacy regions.
****************************************************************************/
#define SDVR_MAX_MD_REGIONS          4
#define SDVR_MAX_BD_REGIONS          4
#define SDVR_MAX_PR_REGIONS          4

/**************************************************************************
  VISIBLE: This defines the maximum number of OSD text characters that can be display as an
  OSD text string.

    @SDVR_MAX_OSD_TEXT@ - The maximum size of OSD text for single byte
    OSD APIs is 10 characters.

    @SDVR_MAX_OSD_EX_TEXT@ - The maximum length of OSD text in OSD APIs
    supporting double byte is 100 unsigned short.
    Available only in firmware version 3.2.0.0 or later.
***************************************************************************/
#define SDVR_MAX_OSD_TEXT          10
#define SDVR_MAX_OSD_EX_TEXT       100

/**************************************************************************
    VISIBLE: This defines the maximum number of OSD item that can be configure
    per each channel. Available only in firmware version 3.2.0.0 or later.
***************************************************************************/
#define SDVR_MAX_OSD                2

/**************************************************************************
    VISIBLE: This defines the invalid channel handle.
***************************************************************************/
#define INVALID_CHAN_HANDLE (sx_int32)0xFFFFFFFF

/****************************************************************************
  VISIBLE: The DVR firmware sends asynchronous signal messages to the DVR
  Application as it encounters errors not related to any direct function
  call.
  This data structure defines parameters associated with the asynchronous
  signals sent from the DVR firmware to the host DVR Application.

    "sig_type" - The type identifying the signal cause.

    "chan_type" -  The type of channel causing the signal.

    "chan_num" - The ID of channel causing the signal.

    "data" - The error code associated with the sig_type.

    "extra_data" - Optional data information associated with the
    signal_data.

***************************************************************************/

typedef struct sdvr_signal_info {
    sdvr_signals_type_e sig_type;
    sdvr_chan_type_e    chan_type;
    sx_uint8            chan_num;
    sx_uint8            reserved1;
    sx_uint32           data;
    sx_uint32           extra_data;
    sx_uint32           reserved3;
} sdvr_signal_info_t;


/****************************************************************************
  VISIBLE: This data structure defines parameters that are needed to create
  a new encoder, decoder, or HMO-only channel.

    "board_index" - The zero based index of the board where this channel resides.

    "chan_num" - The channel number identifier.
    In the case of encoding, the range is 0 to M-1, where M is the number of cameras
    supported by the board that are designated encoding channels. 
    In the case of decoding, the range is 0 to N-1, where N is the number of decoders
    supported by the board that are designated decoding channels.
    NOTE: The channel number specifies on which S6 chip the channel is going to be
    created. The first set of four (0 - 3) is created on the first S6 chip,
    the second set of four (4 - 7) on the second S6 chip, and so on.

    "chan_type" -  The type channel to create as specified in sdvr_chan_type_e,
    i.e., encoder or decoder.
    NOTE: To create an HMO- or SMO-only channel, set the chan_type to SDVR_CHAN_TYPE_ENCODER
    and the video_format_primary to SDVR_VIDEO_ENC_NONE.

    "video_format_primary" - The primary encode or decode video format, e.g., H.264

    "audio_format" - The encode or decode audio format, e.g., G.711. If no audio
    is associated with this channel, you can specify SDVR_AUDIO_ENC_NONE.
    This field is ignored in version 1.0.

    "video_format_secondary" - The secondary encode video format, e.g., H.264
    This field is ignored when creating a decoder channel, or if the primary
    video format is SDVR_VIDEO_ENC_NONE.
    NOTE: Specifying a secondary encoder on a camera takes away processing
    power from other channels. As result you may not be able to create some
    encoder channels if secondary encoding is used in all the encoder 
    channels.

    "vpp_mode" - This field specifies whether to run VPP in analytics or
    Stretch-lateral-filter mode for an encoder channel.
    Blind detection, privacy blocking, and old style OSD (black background,
    only English characters) are disabled in SDVR_VPP_MODE_SLATERAL vpp_mode,
    but new style OSD (transparent background and Unicode UTF-16 coding scheme
    supporting Basic Multilingual Plane (BMP)) is supported and encoder 
    quality is higher. In SDVR_VPP_MODE_ANALYTICS, only old style OSD is supported.

***************************************************************************/

typedef struct _sdvr_chan_def_t {
    sx_uint8 board_index;
    sx_uint8 chan_num;
    sdvr_chan_type_e chan_type;
    sdvr_venc_e video_format_primary;
    sdvr_aenc_e audio_format;
    sdvr_venc_e video_format_secondary;
    sdvr_vpp_mode_e vpp_mode;
} sdvr_chan_def_t;

/************************************************************************
  VISIBLE: These defines specify the flags that can be used in order
  to change one or more of image control parameters assoicated to each
  video-in data port by ORing each defines when calling 
  sdvr_set_video_in_params()

    @SDVR_ICFLAG_ALL@ - Use this define to change all the image 
    control parameters.

    @SDVR_ICFLAG_HUE@ - Indicates changing of the hue image value.

    @SDVR_ICFLAG_SATURATION@ - Indicates changing of the saturation
    image value.

    @SDVR_ICFLAG_BRIGHTNESS@ - Indicates changing of the brightness
    image value.

    @SDVR_ICFLAG_CONTRAST@ - Indicates changing of the contrast
    image value.

    @SDVR_ICFLAG_SHARPNESS@ - Indicates changing of the sharpness
    image value.
************************************************************************/
#define SDVR_ICFLAG_ALL                  0xFFFF
#define SDVR_ICFLAG_HUE                  0x1
#define SDVR_ICFLAG_SATURATION           0x2
#define SDVR_ICFLAG_BRIGHTNESS           0x4
#define SDVR_ICFLAG_CONTRAST             0x8
#define SDVR_ICFLAG_SHARPNESS            0x10

/****************************************************************************
  VISIBLE: This data structure defines various parameters that
  can be changed for each video-in data port. These parameters effect both the
  raw and encoded video frames that may affect the blind detection alarm.

    @hue@ - Value of hue control in the range of 0 to 255.
    
    @saturation@ - Value of saturation control in the range of 0 to 255.
    
    @brightness@ - Value of brightness control in the range of 0 to 255.
    
    @contrast@ - Value of contrast control in the range of 0 to 255.
    
    @sharpness@ - Value of sharpness control in the range of 1 to 15.

  Remarks:

    Default values for these fields are hardware specific.
***************************************************************************/
typedef struct _sdvr_image_ctrl_t {
    sx_uint8 hue;
    sx_uint8 saturation;
    sx_uint8 brightness;
    sx_uint8 contrast;
    sx_uint8 sharpness;
} sdvr_image_ctrl_t;

/****************************************************************************
  VISIBLE: This data structure defines video encoder parameters.
  For each channel used for encoding, use this data structure to set its
  parameters.

    @frame_rate@ - The frame rate of the channel in frames per 
    second. Valid values are 1-30 for NTSC video and 1-25 for PAL. 
    The default is 30 for NTSC and 25 for PAL.

    @res_decimation@ - Resolution decimation of the channel. You can specify
    the resolution of the encoded channel to be the same as, 1/4, or
    1/16 of the system-wide maximum resolution.
    The default for primary encoding is SDVR_VIDEO_RES_DECIMATION_EQUAL whereas
    for secondary encoding is SDVR_VIDEO_RES_DECIMATION_QCIF.

  The parameters for each encoder are set in the following
  "union".

  H.264 Parameters:

     @avg_bitrate@ - The average bit rate in Kbits per second
     if CBR or VBR is selected.
     The default is 2000.

     @max_bitrate@ - The maximum target bit rate in Kbits per second 
     if VBR is selected.
     The default is 4000.

     @bitrate_control@ - Choose one of CBR, VBR, QP (not supported)
     or constant quality. The default is CBR.

     @gop_size@ - GOP size for the H.264 encoder. GOP size must be
     greater than 1 (GOP size 1 is currently not supported).
     The default is 15.

     @quality@ - The quality level to be maintained when bitrate_control
     is set to constant quality. The valid range is 1 - 100.
     The default is 50.

  JPEG Parameters:   

     @quality@ - A number in the range of 1 - 99 to control the quality of the
     compression. A higher number implies better quality. 
     The default is 50.

     @is_image_style@ - The JPEG encoder generates an image-style JPEG frame
     header to be used for RTP when this value is 1. The JPEG encoder generates a
     video-style JPEG (Motion JPEG) frame header when this value is zero. This is
     suitable in AVI MJPEG or still JPEG image files. This field must always be 0.
     The default is 0.

  MPEG4 Parameters:

     @avg_bitrate@ - The average bit rate in Kbits per second
     if CBR or VBR is selected.
     The default is 2000.

     @max_bitrate@ - The maximum target bit rate in Kbits per second 
     if VBR is selected.
     The default is 4000.

     @bitrate_control@ - Choose one of CBR, VBR, QP (not supported),
     or constant quality. The default is CBR.

     @gop_size@ - GOP size for the MPEG4 encoder. GOP size must be
     greater than zero. The default is 15.

     @quality@ - The quality level to be maintained when bitrate_control
     is set to constant quality. The valid range is 1 - 100.
     The default is 50.
****************************************************************************/
typedef struct _sdvr_video_enc_chan_params_t {
    sx_uint8                    frame_rate;
    sx_uint8                    res_decimation;
    sx_uint16                   reserved1;

    // Video encoder specific channel parameters
    union {
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            sx_uint8            bitrate_control;
            sx_uint8            gop;
            sx_uint8            quality;
            sx_uint8            reserved1;
        } h264;
        struct {
            sx_uint16           quality;
            sx_uint8            is_image_style;
            sx_uint8            reserved1;
            sx_uint32           reserved2;
        } jpeg;
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            sx_uint8            bitrate_control;
            sx_uint8            gop;
            sx_uint8            quality;
            sx_uint8            reserved1;
        } mpeg4;

    } encoder;

} sdvr_video_enc_chan_params_t;


/****************************************************************************
  VISIBLE: This data structure defines video encode channel parameters for
  alarm video streaming. After any of the alarms reaches its specified 
  threshold, the video encoded frame is streamed using these new
  parameters for the given minimum duration.

  NOTE: Currently these parameters are used for all triggered alarms.

    @frame_rate@ - The new on-alarm recording frame rate of the channel 
    in frames per second. Valid values are 1-30 for NTSC video and 
    1-25 for PAL. 
    The default is 30 for NTSC and 25 for PAL.

    @min_on_seconds@ - Minimum number of seconds to stream using the
    new encoder parameter after the alarm is triggered.

    @min_off_seconds@ - Minimum number of quiet periods between each
    alarm streaming condition.

    @enable@ - A flag to enable or disable on-alarm video streaming.
    0 turns off the on-alarm streaming. 1 turns on the on-alarm 
    streaming.

    @encoder@ - The encoder-specific parameters. See the encoder union in
    sdvr_video_enc_chan_params_t for a detailed description.
****************************************************************************/
typedef struct _sdvr_alarm_video_enc_params_t 
{
    sx_uint8    frame_rate;
    sx_uint8    min_on_seconds;
    sx_uint8    min_off_seconds;
    sx_uint8    enable;

    // Video encoder specific channel parameters
    union {
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            sx_uint8            bitrate_control;
            sx_uint8            gop;
            sx_uint8            quality;
            sx_uint8            reserved1;
        } h264;
        struct {
            sx_uint16           quality;
            sx_uint8            is_image_style;
            sx_uint8            reserved1;
            sx_uint32           reserved2;
        } jpeg;
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            sx_uint8            bitrate_control;
            sx_uint8            gop;
            sx_uint8            quality;
            sx_uint8            reserved1;
        } mpeg4;
    } encoder;

} sdvr_alarm_video_enc_params_t;

/****************************************************************************
  VISIBLE: Structure defining audio encoder channel parameters.
  
    @audio_enc_type@ - The type of audio encoder to use. 
****************************************************************************/
typedef  struct _sdvr_audio_enc_chan_params_t {
    sdvr_aenc_e   audio_enc_type;
} sdvr_audio_enc_chan_params_t;

/****************************************************************************
  VISIBLE: Data structure for a Macro Block (MB) rectangular region or intrest. 
  An MB region is specified by upper left row and column macro block
  and lower right macro block within a map of ROI buffers. See 
  sdvr_set_regions_map()for details.

    @alarm_flag@    - A flag indicating if the motion value in this
    Macro Block rectangle exceeds the specified threshold. This
    field is set by sdvr_motion_value_analyzer() up on successful
    return.

    @mb_upper_left_row@ - MB row number of the upper left corner.

    @mb_upper_left_col@ - MB column number of the upper left corner.

    @mb_lower_right_row@ - MB row number of the lower right corner.

    @mb_lower_right_col@ - MB column number of the lower right corner.
****************************************************************************/
typedef struct _sdvr_mb_region_t {
    sx_bool   alarm_flag;
    sx_uint8  reserved1;
    sx_uint16 reserved2;
    sx_uint16 upper_left_x;
    sx_uint16 upper_left_y;
    sx_uint16 lower_right_x;
    sx_uint16 lower_right_y;
} sdvr_mb_region_t;

/****************************************************************************
  VISIBLE: Data structure for a region. 
  A region is specified by its upper left and lower right coordinates 
  in pixels. The upper left corner of an image is the origin (0,0).

    @region_id@    - The region identifier. It is needed to change or
    remove a region. When adding a new region, this field will be set
    by the system.

    @upper_left_x@ - X-coordinate of the upper left corner.

    @upper_left_y@ - Y-coordinate of the upper left corner.

    @lower_right_x@ - X-coordinate of the lower right corner.

    @lower_right_y@ - Y-coordinate of the lower right corner.
****************************************************************************/
typedef struct _sdvr_region_t {
    sx_uint8  region_id;  
    sx_uint16 upper_left_x;
    sx_uint16 upper_left_y;
    sx_uint16 lower_right_x;
    sx_uint16 lower_right_y;
} sdvr_region_t;

/****************************************************************************
  VISIBLE: Data structure for motion detection alarm. 

    @threshold@ - The threshold value for motion detection. 
    Motion above the threshold is reported.
    A threshold of zero means the motion detection alarm is
    triggered constantly. The valid range is 0 - 99. 
    The default is 20.

    @enable@ - A value of 1 enables motion detection for all the specified
    regions. If no region is defined, the entire picture will be used for
    motion detection. A value of 0 disables the motion detection.

    @num_regions@ - This field specifies the number of motion detection
    regions added to the current video channel. A value of zero means 
    motion detection is applied to the entire picture. This is a read-only 
    field. 

    @regions@ - An array of regions definition. The regions in this array
    are not in any order. Each array item has an ID to indentify the region.
    This is a read-only field.
****************************************************************************/
typedef struct _sdvr_motion_detection {
    sx_uint8        threshold;
    sx_uint8        enable;
    sx_uint8        num_regions;
    sdvr_region_t   regions[SDVR_MAX_MD_REGIONS];
} sdvr_motion_detection_t;

/****************************************************************************
  VISIBLE: Data structure for blind detection alarm. 

    @threshold@ - The threshold value for blind detection. Blinding
    above the threshold is reported.  The valid range is 0 - 99.
    The default is 60.

    @enable@ - A value of 1 enables blind detection for all the specified
    regions. If no region is defined, the entire picture will be used for
    blind detection. A value of 0 disables the blind detection.

    @num_regions@ - This field specifies the number of blind detection
    regions added to the current video channel. A value of 0 means 
    blind detection is applied to the entire picture. This is a read-only 
    field. 

    @regions@ - An array of regions definition. The regions in this array
    are not in any order. Each array item has an ID to indentify the region.
    This is a read-only field.
****************************************************************************/
typedef struct _sdvr_blind_detection {
    sx_uint8        threshold;
    sx_uint8        enable;
    sx_uint8        num_regions;
    sdvr_region_t   regions[SDVR_MAX_BD_REGIONS];
} sdvr_blind_detection_t;

/****************************************************************************
  VISIBLE: Data structure for night detection alarm. 

    @night_detect_threshold@ - The threshold value for night detection. Values
    below the threshold are reported. Setting this value to 255 
    disables night detection. The valid range is 0 - 255.
    The default is 40.

    @enable@ - A value of 1 enables night detection. A value of 0 disables 
    the night detection.

****************************************************************************/
typedef struct _sdvr_night_detection {
    sx_uint8        threshold;
    sx_bool         enable;
} sdvr_night_detection_t;

/****************************************************************************
  VISIBLE: Data structure for privacy regions. 

    @enable@ - A value of 1 enables block out for all the specified
    regions. If no region is defined, the entire picture will be blocked
    out. A value of zero turns off privacy.

    @num_regions@ - This field specifies the number of blocked out
    regions added to the current video channel. A value of 0 means 
    the entire picture will be blocked out. This is a read-only field. 

    @regions@ - An array of regions definition. The regions in this array
    are not in any order. Each array item has an ID to indentify the region.
    This is a read-only field.


****************************************************************************/
typedef struct _sdvr_privacy_region {
    sx_uint8        enable;
    sx_uint8        num_regions;
    sdvr_region_t   regions[SDVR_MAX_PR_REGIONS];
} sdvr_privacy_region_t;

/****************************************************************************
  VISIBLE: The AV buffer structure used in the SDK for A/V frames.

  The AV buffer has a header followed by the payload. The header contains 
  the following information:

      @board_id@ - The board ID from where the frame was received.

      @channel_type@ - The type of channel (encoder or decoder).

      @channel_id@ - The channel ID.

      @frame_type@ - The type of video/audio frame associated with this buffer.

      @motion_detected@ - The motion value detected on this channel.
      You must compare this value against the motion threshold
      to decide if there are any motions on this video frame.

      @blind_detected@ - The blind value on this channel.
      You must compare this value against the blind threshold
      to decide if the camera is blinded.

      @night_detected@ - The night value on this channel.
      You must compare this value against the night threshold
      to decide if night is detected.

      @av_state_flags@ - A set of one-bit flags about the audio/video
      state of the camera. The loss occurs if the corresponding bit is
      set. The possible flags are SDVR_AV_STATE_VIDEO_LOST & SDVR_AV_STATE_AUDIO_LOST.

      @stream_id@ - This field is only used for encoded frames. It indicates
      whether the payload corresponds to a video frame from the primary (0) or
      the secondary (1) encoder on this encoded channel.

      @payload_size@ - The size of the payload, in bytes, that follows the header.

      @timestamp@ - A hardware-generated timestamp of when the frame was captured.
      Low 32 bits of hardware-generated timestamp.

      @timestamp_high@ - A hardware-generated timestamp of when the frame was captured.
      High 32 bits of hardware-generated timestamp.
      @NOTE@: This field is valid only in firmware build versions 3.2.0.1 or later.

        For video, the time stamp is generated using a
        100 KHz clock, and can be used for A/V synchronization.

        For audio, the time stamp is generated using an
        100 KHz clock, and can be used for A/V synchronization.

      @motion_value@ - Motion detect values for up to four regions of interest.
      motion_value[0] is the same as "motion_detected".
      
      @blind_value@ - Blind detect values for up to four regions of interest.
      blind_value[0] is the same as "blind_detected".
      
      @night_value@ - Night detect values for up to four regions of interest.
      blind_value[0] is the same as "night_detected".
      
      @active_width@ - Active width of video frame buffer. This field is
      valid only for raw and encoded video frames.
      
      @padded_width@ - Padded width of video frame buffer. This field is
      valid only for raw and encoded video frames.
      
      @active_height@ - Active height of video frame buffer. This field is
      valid only for raw and encoded video frames.
      
      @padded_height@ - Padded height of video frame buffer. This field is
      valid only for raw and encoded video frames.
      
      @seq_number@ - Frame sequence number. This field is valid only
      for raw and encoded video frames. Every channel and
      stream combination will have independent sequence numbering. The raw
      and encoded video streams from the same channel will have independent
      sequence numbering.
      
      @frame_number@ - Number of frames seen on this channel so far.
      This field is valid only for raw and encoded video frames. The
      frame number and sequence number will be identical when the 
      stream is being run at full frame rate.

      @frame_drop_count@ - Number of frames dropped detected by the
      firmware on the current stream.

  In addition to the header, there is the @payload@ that contains the
  data for the frame. 

  We highly recommend that when storing each encoded frame to disk,
  both the payload and the header be stored. The header provides 
  information that can be useful when searching through the file. 

  Additionally, we recommend that the DVR Application create a separate
  data and tag file for each channel. The data file contains the
  payload, and the tag file contains the header information. This
  makes searching stored video a lot faster.

  The @payload@ field points to the beginning of data
  for a frame. If you have a pointer to an sdvr_av_buffer_t object 
  (call this pointer p), then you can access the payload by:
  {
       ... = (sx_int8) p->payload[0]; // First byte of buffer
       ... = (sx_int8) p->payload[1]; // Second byte of buffer
  } 

  @NOTE: You should not access fields in this data structure directly. Instead
  you should call functions in the "Frame Buffer Field Access API" group.
  It is possible in the future that the fields in this structure to be 
  changed or removed.@
****************************************************************************/
typedef struct _sdvr_av_buffer_t {
    sx_uint32       signature;          // Must be set to SDVR_DATA_HDR_SIG
    sx_uint16       hdr_version;        
    sx_uint16       hdr_size;           

    sx_uint8        board_id;
    sdvr_chan_type_e  channel_type;
    sx_uint8        channel_id;
    sdvr_frame_type_e frame_type;

    sx_uint8        motion_detected;
    sx_uint8        blind_detected;
    sx_uint8        night_detected;
    sx_uint8        av_state_flags;


    sx_uint8        stream_id;
    sx_uint8        reserved2;
    sx_uint16       reserved3;
    sx_uint32       payload_size;

    sx_uint32       timestamp;
    sx_uint32       timestamp_high;

    // New fields. motion_value/blind_valueb/night_value[0] same as the compatible fields above
    sx_uint8        motion_value[4];
    sx_uint8        blind_value[4];
    sx_uint8        night_value[4];
    
    sx_uint16       active_width;
    sx_uint16       padded_width;
    sx_uint16       active_height;
    sx_uint16       padded_height;
    
    sx_uint32       seq_number;
    sx_uint32       frame_number;
    sx_uint32       frame_drop_count;
    
    sx_uint32       reserved[48];

    sx_uint8        payload[1];     // This means that the payload starts here
} sdvr_av_buffer_t;

/************************************************************************
    Data header signature value. Only present in new-style header.
************************************************************************/
#define SDVR_DATA_HDR_SIG    (0x00524448)

/************************************************************************
    Current data header version.
************************************************************************/
#define SDVR_DATA_HDR_VER    (0x1)


/****************************************************************************
  VISIBLE: The YUV buffer structure used for holding a raw video
  frame. The structure contains three pointers to the Y, U, and V parts of
  a raw video frame. The raw video is in YUV 4:2:0 format.

    The AV buffer has a header followed by the payload. The header 
    contains the following information:

      @board_id@ - The board ID from where the frame was received.

      @channel_type@ - The type of channel (encoder or decoder).

      @channel_id@ - The channel ID.

      @frame_type@ - The type of video/audio frame associated with this buffer.

      @motion_detected@ - The motion value detected on this channel.
      You must compare this value against the motion threshold
      to decide if there are any motions on this video frame.

      @blind_detected@ - The blind value on this channel.
      You must compare this value against the blind threshold
      to decide if the camera is blinded.

      @night_detected@ - The night value on this channel.
      You must compare this value against the night threshold
      to decide if night is detected.

      @reserved1@ - Reserved for future use. This field should not be
      used.

      @y_buff_size@ - The size of the y_buffer in bytes that follows the header.

      @u_buff_size@ - The size of the u_buffer in bytes that follows the header.

      @v_buff_size@ - The size of the v_buffer in bytes that follows the header.

      @timestamp@ - A hardware time stamp of when the frame was created.

        For live video, the time stamp is generated using a
        100 KHz clock, and can be used for A/V synchronization.

        For live audio, the time stamp is generated using an
        100 KHz clock, and can be used for A/V synchronization.

        For decoded audio and video, the time stamp generated during
        encoding (using the 90 KHz) clock is passed to the decoded
        audio and video frames.

  In addition to the header, there are @y_buff@, @u_buff@, and @v_buff@
  that contain the data for the three part of a YUV frame. 

      @y_data@ - Pointer to a buffer containing the Y data of a raw frame.

      @u_data@ - Pointer to a buffer containing the U data of a raw frame.

      @v_data@ - Pointer to a buffer containing the V data of a raw frame.

  NOTE: When you are ready to release the frame, you must pass this structure to
  sdvr_release_yuv_buffer().

  NOTE: Raw audio is stored in sdvr_av_buffer_t, and does not need a 
  separate data structure. 

  @NOTE: You should not access fields in this data structure directly. Instead
  you should call functions in the "Frame Buffer Field Access API" group.
  It is possible in the future that the fields in this structure to be 
  changed or removed.@
****************************************************************************/
typedef struct _sdvr_yuv_buffer_t {
    sx_uint32           signature;          // Must be set to SDVR_DATA_HDR_SIG
    sx_uint8            hdr_version;        
    sx_uint8            reserved1;
    sx_uint16           hdr_size;           

    sx_uint8            board_id;
    sdvr_chan_type_e    channel_type;
    sx_uint8            channel_id;
    sdvr_frame_type_e   frame_type;

    sx_uint8            motion_detected;
    sx_uint8            blind_detected;
    sx_uint8            night_detected;
    sx_uint8            reserved2;

    // New fields. motion_value/blind_valueb/night_value[0] same as the compatible fields above
    sx_uint8            motion_value[4];
    sx_uint8            blind_value[4];
    sx_uint8            night_value[4];

    sx_uint32           timestamp;

    sx_uint64           timestamp_64;

    sx_uint16           active_width;
    sx_uint16           active_height;
    
    sx_uint32           seq_number;
    sx_uint32           frame_number;
    sx_uint32           frame_drop_count;

    sx_uint32           reserved[44];

    sx_uint32           y_data_size;
    sx_uint32           u_data_size;
    sx_uint32           v_data_size;
    sx_uint8            *y_data;
    sx_uint8            *u_data;
    sx_uint8            *v_data;
} sdvr_yuv_buffer_t;

/****************************************************************************
  VISIBLE: Data structure to a single-byte string OSD text.
  
    @display_text@ - String to display as OSD text. This is a 
    single-byte NULL terminated string, and cannot be longer than 
    SDVR_MAX_OSD_TEXT characters. 

    @text_location@ - Location of the string on the window.

    @append_dts@ - True means date and time are appended to the text.

    @dts_style@ - The style in which DTS is shown (time format).

    @enable@ - A non-zero value indicates that the given OSD text is
    part of the video stream. A zero value disables OSD.

  @NOTE: This data structure, and all of its supporting functions, will be replaced
  with a set of more general functions starting with version 3.4.x.
  You should plan to stop using this data structure.@
****************************************************************************/
typedef  struct _sdvr_osd_text_config_t {
    char             display_text[SDVR_MAX_OSD_TEXT + 1];
    sdvr_location_e  text_location;
    sx_bool          append_dts; 
    sdvr_dts_style_e dts_style;
    sx_bool          enable;
} sdvr_osd_text_config_t;


/************************************************************************
    VISIBLE: This data structure is used to configure each one of the OSD items 
    associated with video frames of any camera or player. The number of OSD
    items is currently limited to 2 text of 100 double-byte string per OSD item.

    After an OSD item is configured, it can be shown or hidden at any time.

        @translucent@ - This field specifies the intensity of
        translucence when overlay OSD text is on the active video.
        0 means least translucent, 255 means most translucent.

        @position_ctrl@ - The position of OSD text.
        It can be any of the predefined locations in
        @sdvr_location_e@ or a custom defined location.

        @top_left_x@, @top_left_y@ - The top left coordinates of the OSD
        text when the custom @position_ctrl@ is specified, otherwise
        these fields are ignored. The upper left corner of video frame 
        is the origin (0,0).

        @dts_format@ - The format of date and time to optionally
        be appended to the end of the OSD text.

        @text_len@ - The number of unsigned double byte characters
        in the text field.

        @text@ - Up to 100 unsigned double byte Unicode text to be displayed.
        
************************************************************************/
typedef struct _sdvr_osd_config_ex_t {
            sx_uint8    translucent;
            sdvr_location_e    location_ctrl;
            sx_uint16   top_left_x;
            sx_uint16   top_left_y;
            sdvr_dts_style_e  dts_format;
            sx_uint8    text_len;
            sx_uint16   text[SDVR_MAX_OSD_EX_TEXT];
} sdvr_osd_config_ex_t;

/************************************************************************
    VISIBLE: Stretch DVR pre-loaded font tables.
    
    @SDVR_FT_FONT_ENGLISH@ - Fonts for English character sets.
    
************************************************************************/
#define SDVR_FT_FONT_ENGLISH         0

/************************************************************************
    VISIBLE: The format of the font file.
    
    @SDVR_FT_FORMAT_BDF@ - The Glyph Bitmap Distribution Format (BDF) by 
    Adobe is a file format for storing bitmap fonts. BDF is most commonly 
    used font file within the linux operating system. 
    Currently, this is the only supported format.
    
************************************************************************/
#define SDVR_FT_FORMAT_BDF     1

/***********************************************************************
    VISIBLE: This data structure is used to specify a new OSD font table.
    You can either use all the characters or sub-set of characters within
    the font file. Additionally, you can choose a color to be used for all
    the characters. The same OSD font is used for all the DVR boards that
    are connected at the time of calling sdvr_osd_set_font_table().

    @font_file@ - Full path to a the font file. Currently this must be
    a .bdf file.

    @font_table_id@ - The font table ID. User defined font table IDs are
    from 8 to 15, IDs 0 to 7 are reserved for system fonts.
    @This field is ignored in this release.@

    @font_table_format@ - The format of font file. Currently the only
    supported font format is SDVR_FT_FORMAT_BDF. @This field is ignored
    in this release.@

    @start_font_code@ - The first font character to use within the font
    file. Use 0 for the lowest character code.

    @end_font_code@ - The last font character to use within the font file. Use
    65535 for the highest character code.

    @color_y@ - Y component color of the character in YUV space. Use 255 for
    white.

    @color_u@ - U component color of the character in YUV space. Use 128 for
    white.

    @color_v@ - V component color of the character in YUV space. Use 128 for
    white.
******************************************************************************/
typedef struct _sdvr_font_table_t {
    char *font_file;
    sx_uint8 font_table_id;
    sx_uint8 font_table_format;
    sx_uint32 start_font_code;
    sx_uint32 end_font_code;
    sx_uint8 color_y;
    sx_uint8 color_u; 
    sx_uint8 color_v;
} sdvr_font_table_t;

/****************************************************************************
  VISIBLE: This data structure is used to specify the spot monitor output
  grid pattern. The SMO display is divided into different grids specified
  by a top left location and a resolution decimation of the original video
  camera assigned to that grid.

  Each encode or decode video channel can display its raw image at a particular 
  pixel position on the SMO display.

  Each grid on the SMO screen consists of one or more encode or decode channel 
  outputs with a specific resolution decimation.

  After a grid is defined, you can temporarily enable or disable its output.

    @top_left_mb_x@, @top_left_mb_y@ - These two numbers specify the top-left 
    macro block coordinates of the display position and must be even number
    values.  The coordinate of the top-left corner of SMO is (0,0).

    @res_decimation@ - The resolution to use to display the channels at this
    grid position. The image is resized based on this number.

    @dwell_time@ - If you put multiple channels in the same grid, then you
    must specify the length of time each channel is displayed before switching to 
    the next channel. The firmware cycles through all the channels 
    periodically, and switches them every N seconds, where N is the dwell time. 

    @enable@ - To disable and not display this grid on the SMO, set the value
    of this field to zero. Otherwise, the channel assigned to this grid
    is displayed on the SMO.
****************************************************************************/
typedef struct _sdvr_smo_grid_t {
    sx_uint16           top_left_mb_x;
    sx_uint16           top_left_mb_y;
    sdvr_video_res_decimation_e  res_decimation;        
    sx_uint8            dwell_time;
    sx_bool             enable;
} sdvr_smo_grid_t;

/****************************************************************************
  VISIBLE: Typedef for video alarm callback function. This function has
  to be written by the DVR Application writer and be registered as a
  callback. Through this callback mechanism, the SDK will alert the DVR 
  Application when video alarms happen above the specified threshold for
  the specific alarm.

  The video alarm callback function is called whenever the encoder detects 
  motion, blinding, nighttime light conditions, or video loss or detection..
  The callback function takes as its arguments the channel handle, 
  the type of alarm, and alarm data. The meaning of alarm data varies 
  depending on the type of alarm (i.e., for motion alarm, the alarm data
  is the actual amount of motion over the given threshold).
  These arguments are set by the SDK so that the callback function can 
  determine which board and which video channel the alarm is coming from,
  and the type of alarm. 
****************************************************************************/
typedef void (*sdvr_video_alarm_callback) (sdvr_chan_handle_t handle,
                                           sdvr_video_alarm_e alarm_type,
                                           sx_uint32 data);

/****************************************************************************
  VISIBLE: Typedef for sensor callback function. This function has
  to be written by the DVR Application writer and be registered as a
  callback. Through this callback mechanism, the SDK alerts the DVR 
  Application when sensors trigger.

  The sensor callback is called whenever one or more sensors on each 
  board are triggered. The callback function takes as its arguments the 
  board index and the sensor map. These arguments are set by the SDK 
  so that the callback function can determine which board and which sensors
  have triggered. The sensors that have triggered are in 
  sensor_map, with bit 0 corresponding to sensor 0, bit 1 to sensor 1, 
  and so on.
****************************************************************************/
typedef void (*sdvr_sensor_callback) (sx_uint32 board_index, 
                                      sx_uint32 sensor_map);

/****************************************************************************
  VISIBLE: Typedef for AV frame callback function. This function has
  to be written by the DVR Application writer and be registered as a
  callback. Through this callback mechanism, the SDK alerts the DVR 
  Application whenever encoded or raw AV frames are available.

  The AV frame callback function is called whenever a new 
  encoded AV frame is available, or when a raw video or 
  audio frame is available. The callback function takes as its arguments 
  the channel handle and the frame type to 
  determine what the frame is and where it came from. The last argument is 
  to distinguish between primary and secondary subchannels in a 
  particular encoding channel. This parameter is only valid for 
  encoded video frames and has no meaning for audio or raw video frames.
****************************************************************************/
typedef void (*sdvr_av_frame_callback)(sdvr_chan_handle_t handle,
                                       sdvr_frame_type_e frame_type, 
                                       sx_bool primary_frame);

/****************************************************************************
  VISIBLE: Typedef for displaying debug callback function. This function has
  to be written by the DVR Application writer and be registered as a
  callback. Through this callback mechanism, the SDK alerts the DVR 
  Application whenever tracing error messages need to be displayed on the
  screen.

  The callback function takes as its argument the string buffer to display.
****************************************************************************/
typedef void (*sdvr_display_debug_callback)(char *message);


/****************************************************************************
  VISIBLE: Typedef for firmware asynchronous send message callback function. 
  This function has to be written by the DVR Application writer and be 
  registered as a callback. Through this callback mechanism, the SDK will 
  signal the DVR Application when firmware needs to send messages to the DVR
  Application without the application initiating a request. These messages 
  are currently only error conditions generated on the firmware side, but in 
  the future it could include other types of messages.

  The callback function takes as its argument the signal type and a pointer
  to the signal information data structure.
****************************************************************************/
typedef void (*sdvr_signals_callback) (sx_uint32 board_index,
                                       sdvr_signal_info_t *signal_info);
/*******************************************************************************
   A necessary evil introduced for C++ compatibility.  C source files must
   not declare a function "extern"; instead, they must declare the function
   "EXTERN".  For example:
   {
       EXTERN void my_external_symbol(int a, double f);
   }
   This specifies that the function has C linkage so that it can be used
   when compiled with a C++ compiler.
*******************************************************************************/
#if defined(__cplusplus)
   #define EXTERN              extern "C"
#else
   #define EXTERN              extern
#endif
/****************************************************************************
  GROUP: System Set Up, SDK and Board Initialization API
****************************************************************************/
EXTERN char *sdvr_get_error_text(sdvr_err_e error_no);

EXTERN sdvr_err_e sdvr_sdk_init();
EXTERN sdvr_err_e sdvr_sdk_close();

EXTERN sx_uint32 sdvr_get_board_count();
EXTERN sdvr_err_e sdvr_get_board_attributes(sx_uint32 board_index, 
                                     sdvr_board_attrib_t *board_attrib);
EXTERN sdvr_err_e sdvr_get_pci_attrib(sx_uint32 board_index, 
                                     sdvr_pci_attrib_t *pci_attrib);

EXTERN sdvr_err_e sdvr_board_reset(sx_uint32 board_index);
EXTERN sdvr_err_e sdvr_board_connect(sx_uint32 board_index, 
                                     sdvr_video_std_e video_std, 
                                     sx_bool is_h264_SCE);
EXTERN sdvr_err_e sdvr_board_disconnect(sx_uint32 board_index);

EXTERN sdvr_err_e sdvr_upgrade_firmware(sx_uint32 board_index, 
                                        char *firmware_file_name);

EXTERN sdvr_err_e sdvr_get_sdk_params(sdvr_sdk_params_t *sdk_params);
EXTERN sdvr_err_e sdvr_set_sdk_params(sdvr_sdk_params_t *sdk_params);

EXTERN void sdvr_get_sdk_version(sx_uint8 *major, 
                                 sx_uint8 *minor, 
                                 sx_uint8 *revision,
                                 sx_uint8 *build);
EXTERN sdvr_err_e sdvr_get_driver_version(sx_uint32 board_index,
                                    sx_uint8 *major, 
                                    sx_uint8 *minor, 
                                    sx_uint8 *revision,
                                    sx_uint8 *build);
/*
removed in version 4.0.0.9
EXTERN sdvr_err_e sdvr_get_firmware_version(sx_uint32 board_index,
                                      sx_uint8 *major, 
                                      sx_uint8 *minor, 
                                      sx_uint8 *build);
*/
EXTERN sdvr_err_e sdvr_get_firmware_version_ex(sx_uint32 board_index,
                                      sdvr_firmware_ver_t *version_info);

EXTERN sdvr_sensor_callback sdvr_set_sensor_callback(sdvr_sensor_callback sensor_callback);
EXTERN sdvr_video_alarm_callback sdvr_set_video_alarm_callback(sdvr_video_alarm_callback video_alarm_callback);
EXTERN sdvr_av_frame_callback sdvr_set_av_frame_callback(sdvr_av_frame_callback av_frame_callback);
EXTERN sdvr_display_debug_callback sdvr_set_display_debug(sdvr_display_debug_callback display_debug_callback);
EXTERN sdvr_signals_callback sdvr_set_signals_callback(sdvr_signals_callback signals_callback);

EXTERN sdvr_err_e sdvr_get_board_config(sx_uint32 board_index, 
                                        sdvr_board_config_t *board_config);
EXTERN sdvr_err_e sdvr_get_supported_vstd(sx_uint32 board_index, 
                                          sx_uint32 *vstd);
EXTERN sdvr_err_e sdvr_get_video_standard(sx_uint32 board_index, 
                                          sdvr_video_std_e *video_std_type);
EXTERN sdvr_err_e sdvr_set_watchdog_state(sx_uint32 board_index, 
                                          sx_uint32 enable);
EXTERN sdvr_err_e sdvr_get_watchdog_state(sx_uint32 board_index, 
                                          sx_uint32 *enable);
EXTERN sdvr_err_e sdvr_set_date_time(sx_uint32 board_index, 
                                     time_t unix_time);
EXTERN sdvr_err_e sdvr_get_date_time(sx_uint32 board_index, 
                                     time_t *unix_time);
EXTERN sdvr_err_e sdvr_run_diagnostics(sx_uint32 board_index, 
                                       char *diag_file_name,
                                       sdvr_diag_code_e *diag_code,
                                       int *diag_code_size);

EXTERN sx_uint8 sdvr_get_board_index(sdvr_chan_handle_t handle);
EXTERN sx_uint8 sdvr_get_chan_num(sdvr_chan_handle_t handle);
EXTERN sdvr_chan_type_e sdvr_get_chan_type(sdvr_chan_handle_t handle);

/****************************************************************************
  GROUP: Channel Set Up API
****************************************************************************/
EXTERN sdvr_err_e sdvr_create_chan(sdvr_chan_def_t *chan_def,
                                   sdvr_chan_handle_t *handle_ptr);
EXTERN sdvr_err_e sdvr_set_channel_default(sdvr_chan_handle_t handle);
EXTERN sdvr_err_e sdvr_destroy_chan(sdvr_chan_handle_t handle);
EXTERN sdvr_err_e sdvr_set_chan_user_data(sdvr_chan_handle_t handle, sx_uint64 user_data);
EXTERN sdvr_err_e sdvr_get_chan_user_data(sdvr_chan_handle_t handle, sx_uint64 *user_data);

EXTERN sdvr_err_e sdvr_set_video_encoder_channel_params(sdvr_chan_handle_t handle,
                                                        sdvr_sub_encoders_e sub_chan,
                                                        sdvr_video_enc_chan_params_t *video_enc_params);
EXTERN sdvr_err_e sdvr_get_video_encoder_channel_params(sdvr_chan_handle_t handle,
                                                        sdvr_sub_encoders_e sub_chan,
                                                        sdvr_video_enc_chan_params_t *video_enc_params);
EXTERN sdvr_err_e sdvr_set_alarm_video_encoder_params(sdvr_chan_handle_t handle,
                                                        sdvr_sub_encoders_e sub_chan,
                                                      sdvr_alarm_video_enc_params_t *alarm_video_enc_params);
EXTERN sdvr_err_e sdvr_get_alarm_video_encoder_params(sdvr_chan_handle_t handle,
                                                        sdvr_sub_encoders_e sub_chan,
                                                      sdvr_alarm_video_enc_params_t *alarm_video_enc_params);


EXTERN sdvr_err_e sdvr_set_audio_encoder_channel_params(sdvr_chan_handle_t handle, 
                                                        sdvr_audio_enc_chan_params_t *audio_enc_params);
EXTERN sdvr_err_e sdvr_get_audio_encoder_channel_params(sdvr_chan_handle_t handle,
                                                        sdvr_audio_enc_chan_params_t *audio_enc_params);

/****************************************************************************
  GROUP: Video Analytics and Privacy Blocking API
****************************************************************************/

EXTERN sdvr_err_e sdvr_set_regions_map(sdvr_chan_handle_t handle,
                                sdvr_regions_type_e region_type,
                                sx_uint8 *regions_map);
EXTERN sdvr_err_e sdvr_set_regions(sdvr_chan_handle_t handle,
                                sdvr_regions_type_e region_type,
                                sx_uint8 overlay_num,
                                sdvr_mb_region_t *regions_list,
                                sx_uint8 num_of_regions);
EXTERN sdvr_err_e sdvr_motion_value_analyzer(
                                sx_uint8 *motion_values,
                                sdvr_video_std_e motion_values_vstd,
                                sdvr_mb_region_t *regions_list,
                                sx_uint8 num_of_regions,
                                sx_uint32 threshold);
EXTERN sdvr_err_e sdvr_set_motion_value_frequency(sdvr_chan_handle_t handle, sx_uint8 every_n_frames);
EXTERN sdvr_err_e sdvr_get_alarm_motion_value(sx_uint32 data, sx_uint8 overlay_num, sx_uint8 *motion_value);

EXTERN sdvr_err_e sdvr_add_region(sdvr_chan_handle_t handle, sdvr_regions_type_e region_type,
                                    sdvr_region_t *region);
EXTERN sdvr_err_e sdvr_change_region(sdvr_chan_handle_t handle, sdvr_regions_type_e region_type,
                                     sdvr_region_t *region);
EXTERN sdvr_err_e sdvr_remove_region(sdvr_chan_handle_t handle, sdvr_regions_type_e region_type,
                                     sx_uint8 region_id);

EXTERN sdvr_err_e sdvr_get_motion_detection(sdvr_chan_handle_t handle,
                                    sdvr_motion_detection_t *motion_detection);

EXTERN sdvr_err_e sdvr_get_blind_detection(sdvr_chan_handle_t handle,
                                    sdvr_blind_detection_t *blind_detection);
EXTERN sdvr_err_e sdvr_get_privacy_regions(sdvr_chan_handle_t handle,
                                    sdvr_privacy_region_t *privacy_regions);
EXTERN sdvr_err_e sdvr_get_night_detection(sdvr_chan_handle_t handle,
                                    sdvr_night_detection_t *night_detection);

EXTERN sdvr_err_e sdvr_enable_privacy_regions(sdvr_chan_handle_t handle,
                                              sx_bool enable);
EXTERN sdvr_err_e sdvr_enable_motion_detection(sdvr_chan_handle_t handle,
                                              sx_bool enable,
                                              sx_uint8 threshold);
EXTERN sdvr_err_e sdvr_enable_motion_detection_ex(sdvr_chan_handle_t handle,
                                              sx_bool enable,
                                              sx_uint8 threshold1,
                                              sx_uint8 threshold2,
                                              sx_uint8 threshold3,
                                              sx_uint8 threshold4);

EXTERN sdvr_err_e sdvr_enable_blind_detection(sdvr_chan_handle_t handle,
                                              sx_bool enable,
                                              sx_uint8 threshold);
EXTERN sdvr_err_e sdvr_enable_night_detection(sdvr_chan_handle_t handle,
                                              sx_bool enable,
                                              sx_uint8 threshold);

/****************************************************************************
  GROUP: Encoding and Raw Audio/Video API
****************************************************************************/
EXTERN sdvr_err_e sdvr_enable_encoder(sdvr_chan_handle_t handle,
                                      sdvr_sub_encoders_e sub_chan,
                                      sx_bool enable);

EXTERN sdvr_err_e sdvr_get_av_buffer(sdvr_chan_handle_t handle,
                                     sdvr_frame_type_e frame_type, 
                                     sdvr_av_buffer_t **frame_buffer);
EXTERN sdvr_err_e sdvr_get_yuv_buffer(sdvr_chan_handle_t handle, 
                              sdvr_yuv_buffer_t **frame_buffer);

EXTERN sdvr_err_e sdvr_release_av_buffer(sdvr_av_buffer_t *frame_buffer);
EXTERN sdvr_err_e sdvr_release_yuv_buffer(sdvr_yuv_buffer_t *frame_buffer);

EXTERN sdvr_err_e sdvr_enable_auth_key(sx_bool enable);

/****************************************************************************
  GROUP: Frame Buffer Field Access API
****************************************************************************/
EXTERN sdvr_chan_handle_t sdvr_get_buffer_channel(void *frame_buffer);
EXTERN sdvr_err_e sdvr_get_buffer_timestamp(void *frame_buffer, sx_uint64 *timestamp64);
EXTERN sdvr_err_e sdvr_get_buffer_frame_type(void *frame_buffer, sx_uint8 *frame_type);
EXTERN sdvr_err_e sdvr_get_buffer_sub_encoder(void *frame_buffer, sdvr_sub_encoders_e *sub_enc);
EXTERN sdvr_err_e sdvr_get_buffer_alarm_value(void *frame_buffer, 
                                        sdvr_video_alarm_e  alarm_type,
                                        sx_uint8 overlay_num,
                                        sx_uint8 *alarm_value);
EXTERN sdvr_err_e  sdvr_av_buf_video_dimensions( sdvr_av_buffer_t *frame_buffer,
                                            sx_uint16 *width, 
                                            sx_uint16 *lines);
EXTERN sdvr_err_e  sdvr_av_buf_sequence( sdvr_av_buffer_t *frame_buffer, 
                                    sx_uint32 *seq_number,
                                    sx_uint32 *frame_number, 
                                    sx_uint32 *frame_drop_count );
EXTERN sdvr_err_e  sdvr_av_buf_payload( sdvr_av_buffer_t *frame_buffer, 
                                          sx_uint8 **payload, 
                                          sx_uint32 *payload_size);
EXTERN sdvr_err_e  sdvr_av_buf_yuv_payload( sdvr_yuv_buffer_t *yuv_buffer, 
                                    sx_uint8 **y_data, 
                                    sx_uint8 **u_data, 
                                    sx_uint8 **v_data,
                                    sx_uint32 *y_data_size, 
                                    sx_uint32 *u_data_size, 
                                    sx_uint32 *v_data_size);

/****************************************************************************
  GROUP: Decoding API
****************************************************************************/
EXTERN sdvr_err_e sdvr_enable_decoder(sdvr_chan_handle_t handle,
                                      sx_bool enable);
EXTERN sdvr_err_e sdvr_set_decoder_size(sdvr_chan_handle_t handle,
                        sx_uint16 width, sx_uint16 height);

EXTERN sdvr_err_e sdvr_alloc_av_buffer(sdvr_chan_handle_t handle, 
                                       sdvr_av_buffer_t **frame_buffer);

EXTERN sdvr_err_e sdvr_send_av_frame(sdvr_av_buffer_t *frame_buffer);
/****************************************************************************
  GROUP: Display and Sound API
****************************************************************************/
EXTERN sdvr_err_e sdvr_stream_raw_video(sdvr_chan_handle_t handle,
                                        sdvr_video_res_decimation_e res_decimation,
                                        sx_uint8 frame_rate,
                                        sx_bool enable);
EXTERN sdvr_err_e sdvr_stream_raw_audio(sdvr_chan_handle_t handle,
                                        sx_bool enable);
EXTERN sdvr_err_e sdvr_get_video_in_params(sdvr_chan_handle_t handle, 
                                            sdvr_image_ctrl_t *image_ctrl);
EXTERN sdvr_err_e sdvr_set_video_in_params(sdvr_chan_handle_t handle,
                                           sx_uint16 image_ctrl_flag,
                                           sdvr_image_ctrl_t *image_ctrl);

/****************************************************************************
  GROUP: On-Screen Display API
****************************************************************************/
EXTERN sdvr_err_e sdvr_osd_text_config_ex(sdvr_chan_handle_t handle, 
                                          sx_uint8 osd_id,
                                          sdvr_osd_config_ex_t *osd_text_config);
EXTERN sdvr_err_e sdvr_osd_text_show(sdvr_chan_handle_t handle,
                              sx_uint8 osd_id,
                              sx_bool show);
EXTERN sdvr_err_e sdvr_osd_set_font_table(sdvr_font_table_t *font_desc);
EXTERN sdvr_err_e sdvr_osd_use_font_table(sx_uint8 font_id);

/****************************************************************************
  GROUP: Spot Monitor Output API
****************************************************************************/
EXTERN sdvr_err_e sdvr_set_smo_grid(sdvr_chan_handle_t  handle,
                             sdvr_smo_grid_t *smo_grid);
 
EXTERN sdvr_err_e sdvr_get_smo_grid(sdvr_chan_handle_t  handle,
                             sdvr_smo_grid_t *smo_grid);

/****************************************************************************
  GROUP: RS485 Communication API
****************************************************************************/
EXTERN sdvr_err_e sdvr_init_uart(sx_uint32 board_index,
                                  sx_uint32 baud_rate,
                                  sx_uint8 data_bits,
                                  sx_uint8 stop_bits,
                                  sx_uint8 parity_enable,
                                  sx_uint8 parity_even);
EXTERN sdvr_err_e sdvr_write_uart(sx_uint32 board_index,
                                  sx_uint8 count,
                                  sx_uint8 *data);
EXTERN sdvr_err_e sdvr_read_uart(sx_uint32 board_index,
                                  sx_uint8 *data_count_read,
                                  sx_uint8 max_data_size,
                                  sx_uint8 *data);

/****************************************************************************
  GROUP: Sensors and Relays API
****************************************************************************/
EXTERN sdvr_err_e sdvr_trigger_relay(sx_uint32 board_index, 
                                     sx_uint32 relay_num, 
                                     sx_bool is_triggered);
EXTERN sdvr_err_e sdvr_enable_sensor(sx_uint32 board_index, 
                                     sx_uint32 sensor_num,
                                     sx_bool enable);
EXTERN sdvr_err_e sdvr_config_sensors(sx_uint32 board_index, 
                                     sx_uint32 sensor_enable_map,
                                      sx_uint32 edge_triggered_map);

/****************************************************************************
  GROUP: Recording to File API
****************************************************************************/
EXTERN sdvr_err_e sdvr_start_recording(sdvr_chan_handle_t handle,
                               sdvr_sub_encoders_e sub_chan_enc,
                               char *file_name);
EXTERN sdvr_err_e sdvr_stop_recording(sdvr_chan_handle_t handle,
                               sdvr_sub_encoders_e sub_chan_enc);


#endif /* STRETCH_SDVR_SDK_H */
