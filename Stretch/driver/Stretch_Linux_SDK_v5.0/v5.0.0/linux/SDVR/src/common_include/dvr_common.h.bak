/**************************************************************************
Copyright (c) 2008 Stretch, Inc. All rights reserved.  Stretch products are
protected under numerous U.S. and foreign patents, maskwork rights,
copyrights and other intellectual property laws.  

This source code and the related tools, software code and documentation, and
your use thereof, are subject to and governed by the terms and conditions of
the applicable Stretch IDE or SDK and RDK License Agreement (either as agreed
by you or found at www.stretchinc.com).  By using these items, you indicate
your acceptance of such terms and conditions between you and Stretch, Inc.
In the event that you do not agree with such terms and conditions, you may
not use any of these items and must immediately destroy any copies you have
made.
***************************************************************************/

#ifndef STRETCH_DVR_COMMON_H
#define STRETCH_DVR_COMMON_H

/************************************************************************
    PACKAGE: dvr_common -- Common Definitions Shared Between Board and PC

    DESCRIPTION:

    SECTION: Include
    {
    #include "dvr_common.h"
    }


    SECTION: Introduction

    @dvr_common@ is a collection of common declarations to be shared
    between the Stretch PCIe DVR card software and the Stretch DVR SDK 
    running on the host PC.

    SECTION: Basic Communication Method Between the PCIe Card and the Host PC

    The Stretch PCIe card communicates with the host PC via layered APIs.
    Sitting at the bottom level is the basic PCIe device drivers running
    on the board and on the host PC.  It provides basic PCIe hardware
    abstraction.  For details on the PCIe driver, refer to @pci@
    in Stretch SBIOS.

    The next level up is a basic communication layer @sct@.  @sct@
    provides basic communication channels and messages between the
    application code running on the PCIe card and on the host PC.  
    Refer to @sct@ for more details about the communication layer API.

    The next level up is the application code.  Specific to the DVR,
    the application code on the PCIe is generally referred to as "DVR
    Firmware" and the application code on the host PC as "DVR SDK".
    Using @sct@, the DVR firmware and DVR SDK exchange all required data
    using agreed-upon @sct@ messages and @sct@ channels.  The purpose
    of this document is to detail the common message types, semantics of
    the message data, and the @sct@ channel creation process.  This file must
    be included by the DVR firmware and DVR SDK to ensure consistency.


    SECTION: Usage Model

    This interface file is designed with a particular usage model in mind.
    Using a PCIe card is defined as creating and configuring various
    jobs running on the board.  The following is a list of basic
    jobs that the board supports:
    
        Encoding of a real-time camera input.  In this job, the input
        video is encoded and the bit stream is sent to the host.
        Optionally, a decimated raw input image can be sent to the Spot 
        Monitor Out (SMO) or the host (HMO) for display.

        Decoding a bit stream supplied by the host.  In this job, the
        bit stream supplied by the host is decoded.  The decoded
        image can be sent to SMO or host for display.

        Encoding of raw images supplied by the host.  In this job, the
        raw images supplied by the host are encoded.  The resulting
        bit stream is sent back to the host.  Optionally, a decimated
        raw input image can be sent to the SMO for display.

    A job can be created, destroyed, configured, enabled, or disabled.
    The following are the rules must be obeyed when managing the jobs:

        A job must be created before it can be configured.

        A job can only be configured or reconfigured when it is disabled.
        (When a job is first created, it is disabled.)

    After a basic job is properly configured, it can be activated
    or deactivated.  These basic jobs are described in more
    detail in the following subsections.

    SUBSECTION: Command and Response Messages

    The host queries board state by sending "GET" messages. Information
    about a specific module is queried by sending a "DVR_GET_xxx" message.
    The board replies with the corresponding "DVR_REP_xxx" message.
    Job settings and state are modified by sending "DVR_SET_xxx" messages.
    The board replies to these using "DVR_REP_xxx" messages as well.
    The GET/SET/REP messages use the same data structure as payload.
    
    SUBSECTION: Creating, Destroying, and Configuring a Job

    A specific handshake between the host application and the firmware
    must be followed when creating a job. A new job is created by the host
    application initiating the request. The following scenario describes
    the sequence to be followed in setting up a camera encode job.
    
        1) The host sends a DVR_SET_JOB  message with the control field set to
        DVR_JOB_CREATE. In response, the firmware opens a PCI port connection
        for sending data, initializes the dataport for its camera input, and
        pre-allocates all the necessary resources for the job.
        
        2) The host calls the @sct_channel_accept@ function to accept the
        data connection from the board.
        (The host DVR application must call the macro DVR_JOB_NUM to get 
        the PCI port number.)
        
        3) The host waits for the DVR_REP_JOB message from the firmware.

    To destroy the job:
    
        1) The host sends a DVR_SET_JOB message with control field set
        to DVR_JOB_DESTROY. In response, the firmware closes the PCI port 
        connection and frees up all resources allocated for this job.
        
        2) The host calls the @sct_channel_close@ function to shut down
        the data connection to the job.
        
        3) The host waits for DVR_REP_JOB message from the firmware.

    SUBSECTION: Dynamic Control of Basic Jobs

    One special group of messages are used to activate or deactivate jobs
    on the board.  The messages are @DVR_GET_JOB@, @DVR_SET_JOB@,
    and @DVR_REP_JOB@.

    SECTION: A Convention

    We use the following convention to uniquely associate a message 
    with a particular instance of a job:  

        A job type, which must be one of
        @DVR_JOB_CAMERA_ENCODE@, @DVR_JOB_HOST_DECODE@,
        or @DVR_JOB_HOST_ENCODE@.

        A job id, which is a 8-bit number.  The IDs do not have
        to start at 0 and do not need to be sequential.

    SECTION: Data Communication Between Firmware and SDK

    All data exchanged between board and host are between specific
    job on the board and SDK on the host.  A job on the board
    is uniquely identified by its type and id.  By combining
    the board id, the job type and the job ID into a single 16 number 
    <board_id, job_type, job_id>,
    we have a unique job number (JN).  This JN will be used
    as the port number for exchange all the data between 
    the job and the SDK.  The macro @DVR_JOB_NUM@ should
    be used to construct the job number to ensure
    compatability between SDK and firmware.

    The actual data exchanged is either raw video, raw audio,
    or bitstream.  In addition, other ancillary data might
    be needed.  So, the following convention will be used
    to exchange the data.

    Data buffers sent to or received from the board are of
    variable size. When the system is initialized, buffers
    are allocated to handle the maximum possible size. The
    maximum is currently defined by one frame of raw YUV
    data for a D1 PAL image, the buffer size being determined
    by the Y plane size which is the largest component.
    For decimated images, encoded frames, and ancillary data,
    the actual transfer sizes could be a lot smaller.
    
    All data buffers must begin with a header of type
    @dvr_data_header_t@. This allows identification of the job
    associated with the data, specifies the transfer size, and
    provides other ancillary information. The rest of the data
    follows the header. The total number of bytes transferred is
    equal to the size of the header structure plus the actual
    buffer size.
        
    SECTION: Asynchronous Events 

    The board can generate the following asynchronous events.
    These are all signalled to the host via the @DVR_SIG_HOST@
    message.
    
        - Sensor activated.

        - Motion detection alarm triggered.

        - Blind detection alarm triggered.

        - Night detection alarm triggered.

        - Video signal from camera lost.
        
        - Video signal from camera detected.
        
        - Runtime error in the firmware.
        
        - Fatal error in the firmware.
        
        - Board heartbeat. (This is currently not implemented).

    SECTION: Restrictions

    The resolutions at which a video channel (incoming or decoded)
    can be displayed on the spot monitor (SMO) or the host monitor
    (HMO) are restricted to its original resolution, 2x-decimated, or
    4x-decimated resolutions. Enlargement is not supported.
************************************************************************/

#include <sx-types.h>

/************************************************************************
    Defines the convention for mapping a job to a job number
************************************************************************/
#define DVR_JOB_NUM(board_id, job_type, job_id) \
    (((board_id & 0xf) << 12) | ((job_type & 0xf) << 8) | (job_id & 0xff))

/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_status_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_status_enum        dvr_status_e;
#else
typedef unsigned char dvr_status_e;
#endif

/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_chip_rev_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_chip_rev_enum        dvr_chip_rev_e;
#else
typedef unsigned char dvr_chip_rev_e;
#endif

/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_job_type_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_job_type_enum      dvr_job_type_e;
#else
typedef unsigned char dvr_job_type_e;
#endif

/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_video_res_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_video_res_enum     dvr_video_res_e;
#else
typedef unsigned char dvr_video_res_e;
#endif

/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_vc_format_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_vc_format_enum     dvr_vc_format_e;
#else
typedef unsigned char dvr_vc_format_e;
#endif

/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_ac_format_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_ac_format_enum     dvr_ac_format_e;
#else
typedef unsigned char dvr_ac_format_e;
#endif

/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_audio_rate_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_audio_rate_enum     dvr_audio_rate_e;
#else
typedef unsigned char dvr_audio_rate_e;
#endif

/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_vpp_mode_enum@ for the description of this type.

        @NOTE@: This is deprecated and will be removed in a future release.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_vpp_mode_enum     dvr_vpp_mode_e;
#else
typedef unsigned char dvr_vpp_mode_e;
#endif

/***********************************************************************
        This is a work around for MSC compiler. The ":n" with 
        enum type fields within a structure is not supported. See 
        @dvr_vpp_action_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_vpp_action_enum     dvr_vpp_action_e;
#else
typedef unsigned char dvr_vpp_action_e;
#endif

/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_rc_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_rc_enum       dvr_rc_e;
#else
typedef unsigned char dvr_rc_e;
#endif

/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_job_action_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_job_action_enum    dvr_job_action_e;
#else
typedef unsigned char dvr_job_action_e;
#endif

/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_osd_pos_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_osd_pos_enum       dvr_osd_pos_e;
#else
typedef unsigned char dvr_osd_pos_e;
#endif

/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_osd_dts_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_osd_dts_enum       dvr_osd_dts_e;
#else
typedef unsigned char dvr_osd_dts_e;
#endif

/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_data_type_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_data_type_enum     dvr_data_type_e;
#else
typedef unsigned char dvr_data_type_e;
#endif

/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_signal_type_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_signal_type_enum   dvr_signal_type_e;
#else
typedef unsigned char dvr_signal_type_e;
#endif

/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_term_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_term_enum   dvr_term_e;
#else
typedef unsigned char dvr_term_e;
#endif

/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_led_type_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_led_type_enum   dvr_led_type_e;
#else
typedef unsigned char dvr_led_type_e;
#endif


/***********************************************************************
	This is a work around for MSC compiler. The ":n" with 
	enum type fields within a structure is not supported. See 
	@dvr_message_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_message_enum       dvr_message_e;
#else
typedef unsigned char dvr_message_e;
#endif

/***********************************************************************
        This is a work around for MSC compiler. The ":n" with 
        enum type fields within a structure is not supported. See 
        @dvr_rawv_format_enum@ for the description of this type.
************************************************************************/
#ifndef _MSC_VER
typedef enum dvr_rawv_format_enum  dvr_rawv_format_e;
#else
typedef unsigned char dvr_rawv_format_e;
#endif

/************************************************************************
    Status for all message exchanges. All reply messages must set the
    status field to let the originator know how the message was handled.
    @DVR_STATUS_OK@ means a message was successfully processed.
************************************************************************/
enum dvr_status_enum {
    DVR_STATUS_OK = 0,
    DVR_STATUS_WRONG_CAMERA_NUMBER,
    DVR_STATUS_WRONG_CAMERA_TYPE,
    DVR_STATUS_WRONG_CODEC_FORMAT,
    DVR_STATUS_WRONG_CODEC_RESOLUTION,
    DVR_STATUS_WRONG_JOB_TYPE,
    DVR_STATUS_WRONG_JOB_ID,
    DVR_STATUS_WRONG_VIDEO_FORMAT,
    DVR_STATUS_WRONG_AUDIO_FORMAT,
    DVR_STATUS_EXCEED_CPU_LIMIT,
    DVR_STATUS_JOB_NOT_CREATED,
    DVR_STATUS_JOB_ALREADY_CREATED,
    DVR_STATUS_JOB_NOT_ENABLED,
    DVR_STATUS_JOB_NOT_DISABLED,
    DVR_STATUS_SMO_NOT_CREATED,
    DVR_STATUS_INVALID_TIME,
    DVR_STATUS_ILLEGAL_SMO_PARAMS,
    DVR_STATUS_SMO_NOT_SUPPORTED,
    DVR_STATUS_VDET_ERROR,
    DVR_STATUS_RUNTIME_ERROR,
    DVR_STATUS_VPP_RUNTIME_ERROR,
    DVR_STATUS_ENCODER_RUNTIME_ERROR,
    DVR_STATUS_DECODER_RUNTIME_ERROR,
    DVR_STATUS_ILLEGAL_PARAMETER,
    DVR_STATUS_INTERNAL_ERROR,
    DVR_STATUS_ILLEGAL_COMMAND,
    DVR_STATUS_SMO_NOT_DISABLED,
    DVR_STATUS_OUT_OF_MEMORY,
    DVR_STATUS_NO_IO_BOARD,
    DVR_STATUS_AUDIO_RUNTIME_ERROR,
    DVR_STATUS_UNSUPPORTED_COMMAND,
    DVR_STATUS_SMO_CHAN_FAILED,
    DVR_STATUS_RES_LIMIT_EXCEEDED,
};

/************************************************************************
    Chip revision definitions.
************************************************************************/
enum dvr_chip_rev_enum {
	CHIP_S6100_3_REV_C = 0,
	CHIP_S6105_3_REV_C,
	CHIP_S6106_3_REV_C,
	CHIP_S6100_3_REV_D = 16,
	CHIP_S6105_3_REV_D,
	CHIP_S6106_3_REV_D,
	CHIP_S6100_3_REV_F = 32,
	CHIP_S6105_3_REV_F,
	CHIP_S6106_3_REV_F,
    CHIP_S6100_3_UNKNOWN = 48,
    CHIP_S6105_3_UNKNOWN,
    CHIP_S6106_3_UNKNOWN,
    CHIP_UNKNOWN = 255
};
/************************************************************************
    Camera types:  

        @DVR_VSTD_D1_PAL@    is defined as 704x576 at 25fps.

        @DVR_VSTD_D1_NTSC@   is defined as 704x480 at 30fps.

        @DVR_VSTD_CIF_PAL@   is defined as 352x288 at 25fps.

        @DVR_VSTD_CIF_NTSC@  is defined as 352x240 at 30fps.

        @DVR_VSTD_2CIF_PAL@  is defined as 704x288 at 25fps.

        @DVR_VSTD_2CIF_NTSC@ is defined as 704x240 at 30fps.

        @DVR_VSTD_4CIF_PAL@  is defined as 704x576 at 25fps.

        @DVR_VSTD_4CIF_NTSC@ is defined as 704x480 at 30fps.
        
        @DVR_VSTD_QCIF_PAL@  is defined as 176x144 at 25 fps.
        
        @DVR_VSTD_QCIF_NTSC@ is defined as 176x112 at 30 fps.
************************************************************************/
#define DVR_VSTD_UNKNOWN     0
#define DVR_VSTD_D1_PAL      (1 << 0)
#define DVR_VSTD_D1_NTSC     (1 << 1)
#define DVR_VSTD_CIF_PAL     (1 << 2)
#define DVR_VSTD_CIF_NTSC    (1 << 3)
#define DVR_VSTD_2CIF_PAL    (1 << 4)
#define DVR_VSTD_2CIF_NTSC   (1 << 5)
#define DVR_VSTD_4CIF_PAL    (1 << 6)
#define DVR_VSTD_4CIF_NTSC   (1 << 7)
#define DVR_VSTD_QCIF_PAL    (1 << 8)
#define DVR_VSTD_QCIF_NTSC   (1 << 9)

/************************************************************************
    Supported job types.
    
    @DVR_JOB_CAMERA_ENCODE@ encodes real-time video captured by a camera.
    
    @DVR_JOB_HOST_ENCODE@   encodes raw images supplied by the host.
    
    @DVR_JOB_HOST_DECODE@   decodes bitstreams supplied by the host.

    @DVR_JOB_VIDEO_OUTPUT@  manages video output from the board.
************************************************************************/
enum dvr_job_type_enum {
    DVR_JOB_CAMERA_ENCODE   = 0,
    DVR_JOB_HOST_ENCODE     = 1,
    DVR_JOB_HOST_DECODE     = 2,
    DVR_JOB_VIDEO_OUTPUT    = 0xF
};

/************************************************************************
    Video output resolution formats.

    @DVR_VIDEO_RES_INVALID@ - Invalid.
    
    @DVR_VIDEO_RES_FULL@    - Full size picture, size depends on the video
    standard selected. For D1, it is 720x480 (NTSC) or 720x576 (PAL). For
    4CIF, it is 704x480 (NTSC) or 704x576 (PAL).
    
    @DVR_VIDEO_RES_CIF@     - CIF size.  352x240 (NTSC) or 352x288 (PAL).
    
    @DVR_VIDEO_RES_QCIF@    - QCIF size. 176x112 (NTSC) or 176x144 (PAL).
    
    @DVR_VIDEO_RES_HALF@    - Half size. Same width as @DVR_VIDEO_RES_FULL@
    but half the height.
    
    @DVR_VIDEO_RES_DCIF@    - DCIF size. 528x320 (NTSC) or 528x384 (PAL).

    @DVR_VIDEO_RES_CLASSIC_CIF@ - 320x240 - Not supported for 
    encoding or decoding.

    @DVR_VIDEO_RES_CLASSIC_2CIF@ - 640x240  - Not supported for 
    encoding or decoding.

    @DVR_VIDEO_RES_CLASSIC_4CIF@ - 640x480  - Not supported for 
    encoding or decoding.

************************************************************************/
enum dvr_video_res_enum {
    DVR_VIDEO_RES_INVALID = 0,
    DVR_VIDEO_RES_FULL    = 1,
    DVR_VIDEO_RES_CIF     = 2,
    DVR_VIDEO_RES_QCIF    = 4,
    DVR_VIDEO_RES_HALF    = 5,
    DVR_VIDEO_RES_DCIF    = 6,
    DVR_VIDEO_RES_CLASSIC_CIF  = 7,
    DVR_VIDEO_RES_CLASSIC_2CIF = 8,
    DVR_VIDEO_RES_CLASSIC_4CIF = 9
};

/************************************************************************
    Video encoding and decoding formats.
    
    @DVR_VC_FORMAT_NONE@     no video codec.
    
    @DVR_VC_FORMAT_H264@     H.264 baseline.
    
    @DVR_VC_FORMAT_JPEG@     motion JPEG.
    
    @DVR_VC_FORMAT_MPEG4@    MPEG4 simple profile.
************************************************************************/
enum dvr_vc_format_enum {
    DVR_VC_FORMAT_NONE      = 0,
    DVR_VC_FORMAT_H264      = 1,
    DVR_VC_FORMAT_JPEG      = 2,
    DVR_VC_FORMAT_MPEG4     = 3,
};

/************************************************************************
    Audio encoding and decoding formats.
    
    @DVR_AC_FORMAT_NONE@        no audio codec.
    
    @DVR_AC_FORMAT_G711@        G.711 audio.

    @DVR_AC_FORMAT_G726_16K@    G.726 audio at 16 Kbits/sec.

    @DVR_AC_FORMAT_G726_32K@    G.726 audio at 32 Kbits/sec.
    
    The other formats defined are not currently supported.
************************************************************************/
enum dvr_ac_format_enum {
    DVR_AC_FORMAT_NONE      = 0,
    DVR_AC_FORMAT_G711      = 1,
    DVR_AC_FORMAT_G726_16K  = 2,
    DVR_AC_FORMAT_G726_24K  = 3,
    DVR_AC_FORMAT_G726_32K  = 4,
    DVR_AC_FORMAT_G726_48K  = 5,
};

/************************************************************************
    Audio sampling rates.
    
    @DVR_AUDIO_RATE_INVALID@    Invalid setting.
    
    @DVR_AUDIO_RATE_8KHZ@       8KHz audio sampling.
    
    @DVR_AUDIO_RATE_16KHZ@      16KHz audio sampling.
    
    @DVR_AUDIO_RATE_32KHZ@      32KHz audio sampling.
************************************************************************/
enum dvr_audio_rate_enum {
    DVR_AUDIO_RATE_INVALID  = 0,
    DVR_AUDIO_RATE_8KHZ     = 1,
    DVR_AUDIO_RATE_16KHZ    = 2,
    DVR_AUDIO_RATE_32KHZ    = 3
};

/************************************************************************
    Video pre-processing modes.
    
    @DVR_VPP_MODE_SLATERAL@     Run VPP in Stretch-lateral-filter mode.

    @DVR_VPP_MODE_ANALYTICS@    Run VPP in analytics mode.

    @NOTE@: These flags are deprecated. They are ignored by the firmware
    and will be removed in a future release. Use VPP actions to control
    video preprocessing (see @dvr_vpp_action_enum@).
************************************************************************/
enum dvr_vpp_mode_enum {
    DVR_VPP_MODE_SLATERAL   = 0,
    DVR_VPP_MODE_ANALYTICS  = 1,
};

/************************************************************************
    Video preprocessing actions supported. Each action is independent of
    the others. Action flags must be or-ed together to specify a combined
    set of actions.

    @NOTE@: Some actions may impact the frame rate when enabled.

    @DVR_VPP_ACTION_ANALYTIC@       Enables VPP analytics. If this action
    is disabled then motion detection, night detection, blind detection
    and privacy blocking will all be disabled.

    @DVR_VPP_ACTION_DEINTERLACE@    Enables deinterlacing. If this action
    is disabled then the picture will remain in interlaced format, with 2
    separate fields.

    @DVR_VPP_ACTION_MEDIAN_FILTER@  Enables median filtering as a part of
    deinterlacing. If this action is disabled then the two fields will be
    weaved together but no filtering will be performed. If deinterlacing
    is disabled, this action will have no effect.

    @DVR_VPP_ACTION_SLATERAL@       Enables Stretch-lateral noise filtering.

    @NOTE@: The default preprocessing configuration at startup is set to
    ( DVR_VPP_ACTION_ANALYTIC | DVR_VPP_ACTION_DEINTERLACE | DVR_VPP_ACTION_MEDIAN_FILTER |
    DVR_VPP_ACTION_SLATERAL ).
************************************************************************/
enum dvr_vpp_action_enum {
    DVR_VPP_ACTION_ANALYTIC             = 1,
    DVR_VPP_ACTION_DEINTERLACE          = 2,
    DVR_VPP_ACTION_MEDIAN_FILTER        = 4,
    DVR_VPP_ACTION_SLATERAL             = 8,
};

/************************************************************************
    Encoder rate-control algorithm selectors.
    
    @DVR_RC_VBR@ generates a variable bit rate stream.
    
    @DVR_RC_CBR@ generates a constant bit rate stream.
    
    @DVR_RC_CQP@ generates a constant QP stream.
    
    @DVR_RC_CQ@  generates a constant quality stream.
    
    @DVR_RC_CQP@ is not supported at this time.
************************************************************************/
enum dvr_rc_enum {
    DVR_RC_NONE = -1,
    DVR_RC_VBR  = 0,
    DVR_RC_CBR,
    DVR_RC_CQP,
    DVR_RC_CQ,
};

/************************************************************************
    Defines job control constants.
    
    @DVR_JOB_CREATE@    creates a new job.
    
    @DVR_JOB_DESTROY@   destroys an existing job.
    
    @DVR_JOB_RESET@     resets a job to its default configuration.
************************************************************************/
enum dvr_job_action_enum {
    DVR_JOB_CREATE = 0,
    DVR_JOB_DESTROY,
    DVR_JOB_RESET,
};

/************************************************************************
    Defines the on-screen display style.
    
    @DVR_OSD_POS_TL@     puts OSD at top-left corner.
    
    @DVR_OSD_POS_BL@     puts OSD at bottom-left corner.
    
    @DVR_OSD_POS_TR@     puts OSD at top-right corner.
    
    @DVR_OSD_POS_BR@     puts OSD at bottom-right corner.

    @DVR_OSD_POS_CUSTOM@ puts OSD at the specified location on the screen.
    This item can only be used in conjunction with @DVR_SET_OSD_EX@ message.
************************************************************************/
enum dvr_osd_pos_enum {
    DVR_OSD_POS_TL = 0,
    DVR_OSD_POS_BL,
    DVR_OSD_POS_TR,
    DVR_OSD_POS_BR,
    DVR_OSD_POS_CUSTOM
};

/************************************************************************
    Format of the date on OSD.
      
    @DVR_OSD_DTS_NONE@     disables DTS.
    
    @DVR_OSD_DTS_DEBUG@    enables a special debug display mode.
    
    @DVR_OSD_DTS_MDY_12H@  is "MM-DD-YYYY HH:MM:SS am/pm".
    
    @DVR_OSD_DTS_DMY_12H@  is "DD-MM-YYYY HH:MM:SS am/pm".
    
    @DVR_OSD_DTS_YMD_12H@  is "YYYY-MM-DD HH:MM:SS am/pm".
    
    @DVR_OSD_DTS_MDY_24H@  is "MM-DD-YYYY HH:MM:SS".
    
    @DVR_OSD_DTS_DMY_24H@  is "DD-MM-YYYY HH:MM:SS".
    
    @DVR_OSD_DTS_YMD_24H@  is "YYYY-MM-DD HH:MM:SS".
************************************************************************/
enum dvr_osd_dts_enum {
    DVR_OSD_DTS_NONE = 0,
    DVR_OSD_DTS_DEBUG,
    DVR_OSD_DTS_MDY_12H,
    DVR_OSD_DTS_DMY_12H,
    DVR_OSD_DTS_YMD_12H,
    DVR_OSD_DTS_MDY_24H,
    DVR_OSD_DTS_DMY_24H,
    DVR_OSD_DTS_YMD_24H,
    DVR_OSD_DTS_DEBUG_2,
};

/************************************************************************
    Type of data transfered between firmware and SDK.
    
    @DVR_DATA_RAW_VIDEO_xxx@   Y,U, and V component of a raw video frame.
    For interlaced video, a frame must contain the top field followed by
    the bottom field.
    
    @DVR_DATA_RAW_AUDIO@       Raw audio PCM data.
    
    @DVR_DATA_H264_xxx@        NAL unit of H.264 bitstream.
    
    @DVR_DATA_MPEG4_x@         NAL unit of MPEG4 encoded bitstream.
    
    @DVR_DATA_JPEG@            JPEG-compressed image.
    
    @DVR_DATA_G711@            G.711-compressed audio data.

    @DVR_DATA_G726_16K@        G.726-compressed audio data at 16 Kbits/sec.

    @DVR_DATA_G726_32K@        G.726-compressed audio data at 32 Kbits/sec.

    @DVR_DATA_MOTION_VALUES@   Macro block motion values per raw video frames.
************************************************************************/
enum dvr_data_type_enum {
    DVR_DATA_RAW_VIDEO_Y = 0,
    DVR_DATA_RAW_VIDEO_U,
    DVR_DATA_RAW_VIDEO_V,
    DVR_DATA_RAW_AUDIO,
    DVR_DATA_H264_IDR,
    DVR_DATA_H264_I,
    DVR_DATA_H264_P,
    DVR_DATA_H264_B,
    DVR_DATA_H264_SPS,
    DVR_DATA_H264_PPS,
    DVR_DATA_JPEG,
    DVR_DATA_G711,
    DVR_DATA_MPEG4_I,
    DVR_DATA_MPEG4_P,
    DVR_DATA_MPEG4_B,
    DVR_DATA_MPEG4_VOL,
    DVR_DATA_G726_16K,
    DVR_DATA_G726_32K,
    DVR_DATA_MOTION_VALUES,
    DVR_DATA_RAW_VIDEO,
    
    DVR_DATA_UNKNOWN = 255,
};

/************************************************************************
    Defines signal types from board to host.
    
    @DVR_SIGNAL_SENSOR_ACTIVATED@  - sent when a sensor is activated or
    deactivated.

    @DVR_SIGNAL_MOTION_DETECTED@   - sent when motion is detected.

    @DVR_SIGNAL_BLIND_DETECTED@    - sent when the camera is blind.

    @DVR_SIGNAL_NIGHT_DETECTED@    - sent when the image is too dark.

    @DVR_SIGNAL_VIDEO_LOST@        - sent when the video signal from a
    camera is lost.

    @DVR_SIGNAL_VIDEO_DETECTED@    - sent when a video signal is detected
    on an input channel, where there was no signal before.

    @DVR_SIGNAL_RUNTIME_ERROR@     - indicates that a non-fatal runtime
    error has occurred on the board.

    @DVR_SIGNAL_FATAL_ERROR@       - indicates that a fatal error has
    occurred on the board. If this signal is received, the board must
    be reset.

    @DVR_SIGNAL_HEARTBEAT@         - if the heartbeat function is enabled
    by the host, then this message will be sent once a second by the board
    as long as the firmware is operational.

    @DVR_SIGNAL_WATCHDOG_EXPIRED@  - indicates that the watchdog timer has
    expired and the board is about to be reset. If the PC reset function
    is enabled then the host PC will also be reset.
************************************************************************/
enum dvr_signal_type_enum {
    DVR_SIGNAL_SENSOR_ACTIVATED = 0,
    DVR_SIGNAL_MOTION_DETECTED,
    DVR_SIGNAL_BLIND_DETECTED,
    DVR_SIGNAL_NIGHT_DETECTED,
    DVR_SIGNAL_VIDEO_LOST,
    DVR_SIGNAL_VIDEO_DETECTED,
    DVR_SIGNAL_RUNTIME_ERROR,
    DVR_SIGNAL_FATAL_ERROR,
    DVR_SIGNAL_HEARTBEAT,
    DVR_SIGNAL_WATCHDOG_EXPIRED,
};

/****************************************************************************
    VISIBLE: The following enum describe various raw video formats.
    Each enum value must correspond to a unique bit to be used in 
    a bitmap field.

    DVR_RAWV_FORMAT_YUV_4_2_0 - 4:2:0 YUV format.

    DVR_RAWV_FORMAT_YUV_4_2_2 - 4:2:2 YUV format.

    DVR_RAWV_FORMAT_YVU_4_2_0 - 4:2:0 YVU (YV12) format.
****************************************************************************/
enum dvr_rawv_format_enum {
    DVR_RAWV_FORMAT_YUV_4_2_0 = 1,
    DVR_RAWV_FORMAT_YUV_4_2_2 = 2,
    DVR_RAWV_FORMAT_YVU_4_2_0 = 4,
};

/************************************************************************
    DVR job state flags. The flags are OR-ed together and sent to the
    host in every data buffer header.
    
    @DVR_JOB_STATE_VIDEO_LOST@ is set when the video signal is lost.
    
    @DVR_JOB_STATE_AUDIO_LOST@ is set when the audio signal is lost.
************************************************************************/
#define	DVR_JOB_STATE_VIDEO_LOST	0x01
#define DVR_JOB_STATE_AUDIO_LOST	0x02

/************************************************************************
    Message class shared between the DVR firmware and DVR SDK.

    The communication between firmware and DVR is done in a transaction
    style with agreed upon conventions.  Both firmware and SDK can
    originate a message transaction.  The receiver of the message must
    reply with requested data.  The first byte of the data
    contains the return status.  If the return status is not DVR_STATUS_OK,
    it is used as an error code.  This error code is used to
    look up a string message for the user.

    The messages fall into two categories:

        Information messages for the host to find out specific attributes
        of the board.  These messages have the form DVR_GET_xxx and
        DVR_REP_xxx where the GET message is the request from the
        host, and the REP message is the reply from the board.

        Parameter-setting messages for the host to control the board
        functions.  These messages have the form DVR_SET_xxx and DVR_REP_xxx
        where the SET message is originated from the host, and REP is the
        reply from the board.

    The following set of messages are used to exchange information
    about the board.  A message from the SDK to the firmware represents a
    request, and a message from the firmware to the SDK is the response.

        @DVR_GET_VER_INFO@, @DVR_REP_VER_INFO@ - Firmware version.
        The data associated with this message must
        be of type @dvr_ver_info_t@.

        @DVR_GET_CODEC_INFO@, @DVR_REP_CODEC_INFO@ - 
        These messages are used to get specific encoding and decoding
        capabilities of the board for a given resolution.  Multiple
        messages can be used to exchange the CODEC capability of a board
        for multiple resolutions.  As a general rule, the CPU time scales
        linearly with the number of encoding or decoding channels
        for a given resolution, but may not scale linearly with the number
        of pixles.  The data assoicated with this message must be of
        type @dvr_codec_info_t@.

    The following set of messages is used to configure the board
    for various jobs.  

        @DVR_GET_BOARD@, @DVR_SET_BOARD@, @DVR_REP_BOARD@ - Get
        information about the board and configure the board.
        The data for this message is of type @dvr_board_t@.

        @DVR_GET_JOB@, @DVR_SET_JOB@, @DVR_REP_JOB@ - Configure a
        job on the board using the information in the message data of
        type @dvr_job_t@.  The firmware cannot migrate a job from one
        chip to another once it is created.  So, it is important that
        real-time encoding jobs be created before other jobs.

        @DVR_GET_SMO@, @DVR_SET_SMO@, @DVR_REP_SMO@ - Configure the PCIe
        card spot monitor for a particular encoding or decoding channel.
        The configuration data is of type @dvr_smo_t@.

        @DVR_GET_HMO@, @DVR_SET_HMO@, @DVR_REP_HMO@ - Configure a
        particular video channel (incoming or decoded) for display on
        the host PC.  This raw video image will be transmitted to the
        host side.  The configuration data is of type @dvr_hmo_t@.

        @DVR_GET_OSD@, @DVR_SET_OSD@, @DVR_REP_OSD@ - Defines the on screen
        display.  The configuration data is of type @dvr_osd_t@. These messages
        will be obseleted after releae 3.4 and replaced with the 
        @DVR_GET_OSD_EX@, @DVR_SET_OSD_EX@, @DVR_REP_OSD_EX@ messages.

        @DVR_GET_PR@, @DVR_SET_PR@, @DVR_REP_PR@ - Defines private regions
        to be blocked.  The configuration data is of type @dvr_pr_t@.

        @DVR_GET_MD@, @DVR_SET_MD@, @DVR_REP_MD@ - Configure the motion
        detection algorithm for a particular encoding or decoding channel.
        The configuration data is of type @dvr_md_t@.

        @DVR_GET_BD@, @DVR_SET_BD@, @DVR_REP_BD@ - Configure the blind
        detection algorithm for a particular encoding or decoding channel.
        The configuration data is of type @dvr_bd_t@.

        @DVR_GET_ND@, @DVR_SET_ND@, @DVR_REP_ND@ - Configure the night
        detection algorithm for a particular encoding or decoding channel.
        The configuration data is of type @dvr_nd_t@.

        @DVR_GET_ENCODE@, @DVR_SET_ENCODE@,
        @DVR_REP_ENCODE@ - Configure the encoder for a particular encoding
        channel.  The configuration data is of type @dvr_encode_info_t@.

        @DVR_GET_ENC_ALARM@, @DVR_SET_ENC_ALARM@,
        @DVR_REP_ENC_ALARM@ - Configure the encoder alarm settings for a
        particular encoding channel.  The configuration data is of type
        @dvr_enc_alarm_info_t@.

        @DVR_GET_DECODE@, @DVR_SET_DECODE@,
        @DVR_REP_DECODE@ - Configure the decoder for a particular decode
        channel.  The configuration data is of type @dvr_decode_info_t@.

        @DVR_SET_FONT_TABLE@, @DVR_REP_FONT_TABLE@ - These messages are
        used to sent a new font table from host DVR to the DVR frimare
        or select an existing pre-defined font_table. There can only be
        one user defined font table at a time and wants it is sent any
        previously user defined font table will be erased and the new one
        is selected.
        NOTE: The font table messages are not in 3.2 release.

        @DVR_GET_OSD_EX@, @DVR_SET_OSD_EX@, @DVR_REP_OSD_EX@ - These
        messages are used to set/get OSD text, positions, and show
        state for each OSD definitions. Currently only 2 different
        OSD can be defined per channel. DVR_xxx_OSD_EX and DVR_xxx_OSD
        message can not be called at the same time for the same channel.
        Doing so, invalidate the previous call.

        @DVR_GET_RELAYS@, @DVR_SET_RELAYS@, @DVR_REP_RELAYS@ - Configure
        relay outputs.  The configuration data is of type @dvr_relay_t@.

        @DVR_GET_SENSORS@, @DVR_SET_SENSORS@, @DVR_REP_SENSORS@ - Configure
        sensor inputs.  The configuration data is of type @dvr_sensor_t@.

        @DVR_GET_WATCHDOG@, @DVR_SET_WATCHDOG@, @DVR_REP_WATCHDOG@ -
        Configure the watchdog timer.  The configuration data is of type
        @dvr_watchdog_t@.  The host is responsible for sending this
        message before the previous watchdog timer expires.

        @DVR_GET_CONTROL@, @DVR_SET_CONTROL@, @DVR_REP_CONTROL@ - These messages
        are used to enable, disable, or get the status of a job.
        The configuration data is of type @dvr_control_t@.

        @DVR_GET_TIME@, @DVR_SET_TIME@, @DVR_REP_TIME@ - These messages
        are used to get the time of day on the board and to set the time of
        the day on the board based on PC host time.
        The configuration data is of type @dvr_time_t@.

        @DVR_GET_UART@, @DVR_SET_UART@ and @DVR_REP_UART@ - These messages
        are used to configure the RS-485 port and to send data out through
        the port.

        @DVR_GET_IOCTL@, @DVR_SET_IOCTL@, and @DVR_REP_IOCTL@ - These are
        used to get and set device-specific parameters for various onboard
        devices.
 
        @DVR_GET_AUTH_KEY@, @DVR_REP_AUTH_KEY@ - These are used to get the
        security authentication key from the board. The data structure associated
        with these messages is @dvr_auth_key_t@;

        @DVR_SET_AUTH_KEY@ - Setting the authentication key is not 
        supported at this time.

        @DVR_SET_REGIONS_MAP@, @DVR_REP_REGIONS_MAP@ -
        Configure the regions map for motion/blind/night detection as well
        as privacy blocking, for an encoding or decoding channel.
        The configuration data is of type @dvr_regions_map_t@.

        @DVR_SET_VIDEO_OUTPUT@, @DVR_REP_VIDEO_OUTPUT@ -
        These are used to configure the firmware to start accepting raw
        video frame to display on specified SMO port.

        @DVR_GET_SMO_ATTRIB@, @DVR_REP_SMO_ATTRIB@ -
        Get the current attributes setting of the requested Spot Monitor
        display.

    The following message is for the board to signal the host about an event.

        @DVR_SIG_HOST@ signals the host that an event just happened and
        needs the host's attention.  The data associated with this
        message must be of type @dvr_sig_host_t@.

************************************************************************/
enum dvr_message_enum {
    DVR_GET_VER_INFO,                               DVR_REP_VER_INFO,
    DVR_GET_CODEC_INFO,                             DVR_REP_CODEC_INFO,
    DVR_GET_BOARD,          DVR_SET_BOARD,          DVR_REP_BOARD,
    DVR_GET_JOB,            DVR_SET_JOB,            DVR_REP_JOB,
    DVR_GET_SMO,            DVR_SET_SMO,            DVR_REP_SMO,
    DVR_GET_HMO,            DVR_SET_HMO,            DVR_REP_HMO,
    DVR_GET_OSD,            DVR_SET_OSD,            DVR_REP_OSD,
    DVR_GET_PR,             DVR_SET_PR,             DVR_REP_PR,
    DVR_GET_MD,             DVR_SET_MD,             DVR_REP_MD,
    DVR_GET_BD,             DVR_SET_BD,             DVR_REP_BD,
    DVR_GET_ND,             DVR_SET_ND,             DVR_REP_ND,
    DVR_GET_RELAYS,         DVR_SET_RELAYS,         DVR_REP_RELAYS,
    DVR_GET_SENSORS,        DVR_SET_SENSORS,        DVR_REP_SENSORS,
    DVR_GET_WATCHDOG,       DVR_SET_WATCHDOG,       DVR_REP_WATCHDOG,
    DVR_GET_CONTROL,        DVR_SET_CONTROL,        DVR_REP_CONTROL,
    DVR_GET_TIME,           DVR_SET_TIME,           DVR_REP_TIME,
    DVR_GET_ENCODE,         DVR_SET_ENCODE,         DVR_REP_ENCODE,
    DVR_GET_ENC_ALARM,      DVR_SET_ENC_ALARM,      DVR_REP_ENC_ALARM,
    DVR_GET_DECODE,         DVR_SET_DECODE,         DVR_REP_DECODE,
                            DVR_SET_FONT_TABLE,     DVR_REP_FONT_TABLE,
    DVR_GET_OSD_EX,         DVR_SET_OSD_EX,         DVR_REP_OSD_EX,
    DVR_GET_UART,           DVR_SET_UART,           DVR_REP_UART,
    DVR_GET_IOCTL,          DVR_SET_IOCTL,          DVR_REP_IOCTL,
    DVR_GET_AUTH_KEY,       DVR_SET_AUTH_KEY,       DVR_REP_AUTH_KEY,
                            DVR_SET_REGIONS_MAP,    DVR_REP_REGIONS_MAP,
                            DVR_SET_VIDEO_OUTPUT,   DVR_REP_VIDEO_OUTPUT,
    DVR_GET_SMO_ATTRIB,                             DVR_REP_SMO_ATTRIB,
    /* The last legal message ID is 95 (SCPA_CLASS_LAST) */
    DVR_SIG_HOST        = 90,
    DVR_FW_INTERNAL     = 95,
};

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_VER_INFO@ and @DVR_REP_VER_INFO@.
    
    "status" should always be @DVR_STATUS_OK@.
    
    "version_major" is the major release version number.
    
    "version_minor" is the minor release version number.
    
    "version_bug" is the bug release version number.

    "version_build" is the build version number.
    
    The complete firmware version is "<major>.<minor>.<build>.<bug>".

    "board_revision" is the revision  number of Stretch DVR PCIe board.

    "chip_revision" is the S6 chip revision used in the board_revision.
    See @dvr_chip_rev_e@ for all the different chip revisions.

    "board_sub_rev" is the sub-revision number of Stretch DVR PCIe board.

    "bootloader_ver_major" is the major version number of bootloader.

    "bootloader_ver_minor" is the minor version number of bootloader.

    "bsp_ver_major" is the major version number of BSP.

    "bsp_ver_minor" is the minor version number of BSP.
************************************************************************/
typedef struct dvr_ver_info_struct {
    dvr_status_e    status:8;
    sx_uint8        version_major;
    sx_uint8        version_minor;
    sx_uint8        version_bug;
    sx_uint16       build_year;
    sx_uint8        build_month;
    sx_uint8        build_day;
    sx_uint8        board_revision;
    dvr_chip_rev_e  chip_revision:8;
    sx_uint8        board_sub_rev;
    sx_uint8        version_build;
    sx_uint8        bootloader_ver_major;
    sx_uint8        bootloader_ver_minor;
    sx_uint8        bsp_ver_major;
    sx_uint8        bsp_ver_minor;
} dvr_ver_info_t;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_CODEC_INFO@ and @DVR_REP_CODEC_INFO@.
    
    For a message from the SDK to the firmware,
    it is a request  for encoding and decoding capabilities of the
    board for a given resolution.  Fields "encode_fps" and "decode_fps"
    are ignored in this case.  For a message from the firmware to the SDK,
    the data specifies the resolution "camera_type" and total number of 
    frames to encode "encode_fps" or to decode "decode_fps".
************************************************************************/
typedef struct dvr_codec_info_struct {
    dvr_status_e    status:8;
    dvr_vc_format_e video_encode_format:8;
    dvr_ac_format_e audio_encode_format:8;
    sx_uint8        reserved1;
    sx_uint16       camera_type;
    sx_uint16       encode_fps;
    sx_uint16       decode_fps;
    sx_uint16       reserved2;
    sx_uint32       reserved3;
} dvr_codec_info_t;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_BOARD@, @DVR_SET_BOARD@, and @DVR_REP_BOARD@.  
    
        "status" - status of the reply.

        "board_id" - This field is specified by the DVR SDK
        to give the board a unique ID number.

        "num_cameras" - Number of cameras supported by the board.

        "num_sensors" - Number of sensors available on the board.

        "num_relays" - Number of relays available on the board.

        "num_smos" - Number of spot monitors supported by the board.
        In the current version, this number must be 0 or 1.

        "camera_info" - Specifies what cameras are supported by the board.
        It is a bit-wise or of camera types, such as @DVR_VSTD_D1_PAL@
        or @DVR_VSTD_D1_NTSC@.

        "camera_type" - This field is specified by the DVR application.
        It must be one of the supported camera types specified in
        "camera_info". All the cameras connected to a board must
        of the same type. (i.e. you cannot mix NTSC and PAL cameras
        connected to a board).

        "audio_rate" - Audio sampling rate. Must be one of the valid
        @dvr_audio_rate_enum@ values.
        
        "num_encoders" - Number of encoders supported by the board.

        "num_decoders" - Number of decoders supported by the board.

        "audio_supported" - Number of audio channels supported by the
        board. Zero indicates no audio support.

        "h264_skip_SCE" - This field is valid only for the @DVR_SET_BOARD@
        command. If set to true, the firmware will skip start code emulation
        for H.264 encoding. If SCE is required, it will have to be done by
        the host SDK.
************************************************************************/
typedef struct dvr_board_struct {
    dvr_status_e        status:8;
    sx_uint8            board_id;
    sx_uint8            num_cameras;
    sx_uint8            num_sensors;
    sx_uint8            num_relays;
    sx_uint8            num_smos;
    sx_uint16           camera_info;
    sx_uint16           camera_type;
    dvr_audio_rate_e    audio_rate:8;
    sx_uint8            reserved1;
    sx_uint8            reserved2;
    sx_uint8            num_encoders;
    sx_uint8            num_decoders;

    union {
        sx_uint8        audio_supported;
        sx_uint8        h264_skip_SCE;
    } u1;
} dvr_board_t;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_JOB@, @DVR_SET_JOB@ and @DVR_REP_JOB@.  
    
        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.  The default is the camera number
        for the encoder job, and a sequentially assigned number for all
        other jobs.

        "control" - Specifies what to do with the job, for example,
        @DVR_JOB_CREATE@.

        "camera_type" - For job type @DVR_JOB_CAMERA_ENCODE@, this 
        specifies how to configure the input camera, and must be
        consistent with the information returned by @DVR_GET_BOARD@.
        For job type @DVR_JOB_HOST_DECODE@, this specifies the video
        resolution of the stream to be decoded.
        
        "audio_format" - Audio CODEC format. Must be one of the valid
        @dvr_ac_format_enum@ values.
        
        "vpp_mode" - Video preprocessing mode. This is deprecated and is
        ignored by the firmware. Use @DVR_IOCTL_CODE_VPP_CONTROL@ to set
        video preprocessing modes.

        "video_format" - Video CODEC format for primary encoded stream.
        Must be one of the valid @dvr_vc_format_enum@ values.

        "video_format_2" - Video CODEC format for secondary encoded stream.
        Must be one of the valid @dvr_vc_format_enum@ values.

************************************************************************/
typedef struct dvr_job_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    dvr_job_action_e    control:8;
    sx_uint16           camera_type;
    dvr_ac_format_e     audio_format:8;
    dvr_vpp_mode_e      vpp_mode:8;             /* Deprecated, unused */
    dvr_vc_format_e     video_format:8;         /* Primary stream   */
    dvr_vc_format_e     video_format_2:8;       /* Secondary stream */
    sx_uint16           reserved2;
    sx_uint32           reserved3;
} dvr_job_t;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_SMO@, @DVR_SET_SMO@ and @DVR_REP_SMO@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "port_num" - Zero based SMO port number.

        "position_x","position_y" - X and Y coordinates for the top left
        corner of the display image. The display image for this channel
        will be tiled into the SMO display at this position. The top left
        corner of the SMO display is (0,0).
        @NOTE@: X and Y coordinates must be even numbers. If not, the set
        command will fail.

        "output_res" - Must be a valid enum of type @dvr_video_res_e@.
        The image is resized according to this enum before being sent
        to the SMO for display. The default is @DVR_VIDEO_RES_QCIF@.
        @NOTE@: Currently, only @DVR_VIDEO_RES_FULL@, @DVR_VIDEO_RES_CIF@,
        and @DVR_VIDEO_RES_QCIF@ are supported.

        "seconds" - Number of seconds to display the image before skipping
        to the next channel. This is to support multiple channels sharing
        the same display tile in the SMO display. Each channel will display
        for the specified "seconds" in round-robin order.
        @NOTE@: This feature is currently not implemented, so this field
        is ignored by the firmware.

        "enable" - Enable SMO when set to 1.

        "check_only" - Only check to see if SMO creation would succeed.
        Don't actually create anything.  Used internally by the firmware.
************************************************************************/
typedef struct dvr_smo_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            port_num;
    sx_uint16           position_x;
    sx_uint16           position_y;
    dvr_video_res_e     output_res:8;
    sx_uint8            seconds;
    sx_uint8            enable;
    sx_uint8            check_only;
    sx_uint32           reserved2;
} dvr_smo_t;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_HMO@, @DVR_SET_HMO@, and @DVR_REP_HMO@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "output_res" - Must be a valid enum of type @dvr_video_res_e@.
        The image is resized according to this enum before being sent
        to the host. The default is @DVR_VIDEO_RES_QCIF@.

        "enable" - enables HMO when set to 1.

        "audio_enable" - enables HMO audio when set to 1.
************************************************************************/
typedef struct dvr_hmo_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            output_res[2];
    sx_uint8            frame_rate[2];
    sx_uint8            enable[2];
    sx_uint8            audio_enable;
    sx_uint16           reserved1;
    sx_uint32           reserved2;
} dvr_hmo_t;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_OSD@, @DVR_SET_OSD@, and @DVR_REP_OSD@.

    NOTE: These messages and the data structure will be obseleted and
    replaces with @DVR_GET_OSD_EX@, @DVR_SET_OSD_EX@, and @DVR_REP_OSD_EX@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "position" - OSD position, for example, @DVR_OSD_POS_TL@.

        "format" - DTS format, for example, @DVR_OSD_DTS_MDY_12H@.

        "enable" - Set to 1 to turn on and to 0 to turn off OSD.
        OSD configuration is maintained while it is off.
************************************************************************/
typedef struct dvr_osd_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    dvr_osd_pos_e       position:8;
    dvr_osd_dts_e       format:8;
    sx_uint8            enable;
    sx_uint8            title[10];
} dvr_osd_t;

/************************************************************************
    VISIBLE: Font table related Opcodes.

    @DVR_FONT_OPCODE_START@ - Indicates that the host application is ready
    to start downloading font information for a new font.
    
    @DVR_FONT_OPCODE_FINISH@ - Indicates that the download is complete
    and the connection should be closed.
    
    @DVR_FONT_OPCODE_SELECT@ - Indicates that a new font index is to be
    selected.
    
    @DVR_FONT_OPCODE_BUF_YY@ - Indicates that the download for the font
    bitmap Y buffers is about to start.
    
    @DVR_FONT_OPCODE_BUF_UU@ - Indicates that the download for the font
    bitmap U buffers is about to start.
    
    @DVR_FONT_OPCODE_BUF_VV@ - Indicates that the download for the font
    bitmap V buffers is about to start.

    @DVR_FONT_OPCODE_BUF_UTF16@ - Indicates that the download for the
    UTF16 character codes is about to start.
    
    @DVR_FONT_OPCODE_BUF_INDEX@ - Indicates that the download for the
    character indexes is about to start.
    
    @DVR_FONT_OPCODE_BUF_WIDTH@ - Indicates that the download for the
    character width values is about to start.
    
    @DVR_FONT_OPCODE_COPY@ - This opcode is used internally by the
    firmware.

    @DVR_FONT_OPCODE_COPY_DONE@ - This opcode is used internally by
    the firmware.
************************************************************************/
#define DVR_FONT_OPCODE_START           1
#define DVR_FONT_OPCODE_FINISH          2
#define DVR_FONT_OPCODE_SELECT          3
#define DVR_FONT_OPCODE_BUF_YY          4
#define DVR_FONT_OPCODE_BUF_UU          5
#define DVR_FONT_OPCODE_BUF_VV          6
#define DVR_FONT_OPCODE_BUF_UTF16       7
#define DVR_FONT_OPCODE_BUF_INDEX       8
#define DVR_FONT_OPCODE_BUF_WIDTH       9
#define DVR_FONT_OPCODE_COPY            10
#define DVR_FONT_OPCODE_COPY_DONE       11

/************************************************************************
    VISIBLE: Stretch DVR pre-loaded font tables.
    
    @DVR_FONT_ENGLISH@ - Default font for English (ASCII) character set.
************************************************************************/
#define DVR_FONT_ENGLISH                0

/************************************************************************
    VISIBLE: The format of the font table (i.e. bmp 4-2-0).
    
    @DVR_FONT_FORMAT_YUV_4_2_0@ - The font table is constructed as YUV
    4-2-0 format. Currently, this is the only supported format.
************************************************************************/
#define DVR_FONT_FORMAT_YUV_4_2_0       1

/************************************************************************
    VISIBLE: This structure is associated with the @DVR_SET_FONT_TABLE@
    and @DVR_REP_FONT_TABLE@ messages.

        "status" - Status of the reply.
        
        "opcode" - The code defining the operation to be performed.
        Always set by the host.
        
        "font_index" - Index of the font being downloaded / selected.
        Index 0 is the default font that is built into the firmware.
        Values 0-7 are reserved by Stretch. User fonts must have index
        values between 8 and 15.
        
        "font_format" - The format of the font table that is being set.
        Currently @DVR_FT_FORMAT_BMP_4_2_0@ is the only supported format.
        This field is assumed to be set by the host only when the opcode
        is @DVR_FONT_OPCODE_START@.
        
        "port_id" - The SCT port ID that should be used by the firmware
        to download the font data. This field is assumed to be set by the
        host only when the opcode is @DVR_FONT_OPCODE_START@.

        "osdheight" - The height of all the characters in the font table.
        Set only when the op_code is @DVR_FONT_OPCODE_START@.

        "osdnglyph" - The number of characters (glyphs) that are to be
        downloaded. Set only when the opcode is @DVR_FONT_OPCODE_START@.
        
        "osdyysize" - The size of the buffer required to store all the
        Y fields of the YUV character bitmaps. Set only when the opcode
        is @DVR_FONT_OPCODE_START@.
        
        "bufsize" - The size of the buffer being sent. Set only if the
        opcode is one of the "DVR_FONT_OPCODE_BUF_XXX" codes.
************************************************************************/
typedef struct dvr_font_table_struct {
    dvr_status_e        status:8;
    sx_uint8            opcode;
    sx_uint8            font_index;
    sx_uint8            font_format;
    sx_uint16           port_id;
    sx_uint16           osdheight;
    sx_uint16           osdnglyph;
    sx_uint16           reserved1;
    union {
        sx_uint32       osdyysize;
        sx_uint32       bufsize;
    } u1;
} dvr_font_table_t;

/************************************************************************
    VISIBLE: OSD commands Opcodes.

    @DVR_OSD_OPCODE_CONFIG@ - OSD opcode to configure displaying of 
    each OSD line.

    @DVR_OSD_OPCODE_SHOW@   - OSD opcode to set the show status of
    each of the OSD line.
    
    @DVR_OSD_OPCODE_TEXT@   - OSD opcode to set the text for each
    one of the OSD line. This opcode is invalid in @DVR_GET_OSD_EX@.
    NOTE: It is possible to receive multiple @DVR_OSD_OPCODE_TEXT@
    messages for large texts.

    @DVR_OSD_OPCODE_CLEAR_TEXT@ - OSD opcode to remove the OSD text.
************************************************************************/
#define DVR_OSD_OPCODE_CONFIG     1
#define DVR_OSD_OPCODE_SHOW       2
#define DVR_OSD_OPCODE_TEXT       3
#define DVR_OSD_OPCODE_CLEAR_TEXT 4

/************************************************************************
    VISIBLE: Maximum OSD text length.
************************************************************************/
#define DVRFW_OSD_LEN             100 /* does not include DTS text */

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_OSD_EX@, @DVR_SET_OSD_EX@, and @DVR_REP_OSD_EX@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "op_code" - The currently selected operation on the 
        given OSD line ID.

        "config" - This union is associated to opcode
        @DVR_OSD_OPCODE_CONFIG@ in order to configure one
        OSD item.

            "osd_id"  - The OSD identifier to be configured.
            id zero correspond to the 1st OSD, 1 to the 2nd, etc.
            Currently only 2 OSDs can be defined.

            "translucent" - This field specifies the intensity of
            translucent when overlay OSD text is on the active video.
            0 means least translucent, 255 most  translucent.

            "postion_ctrl" - You can select the location of OSD
            text to be displayed using of the pre-define location in
            @dvr_osd_pos_enum@ or specify a custom define location.

            "x_TL", "y_TL" - The top left coordinates of the OSD
            test if the custom @postion_ctrl@ is specified, otherwise
            these fields are ignored.

            "dts_format" - The format of date/time to optionally
            be appended to the end end of OSD text.

            "smo_port" - Setting this field to 0xFF causes the OSD settings to 
            apply to HMO, encoder, and all the spot monitor displays.
            A value of zero causes the OSD setting to apply to HMO and encoder
            but not any of the sport monitor displays.
            Otherwise, the setting applies to the specific spot monitor displays.

        "show" - This union is associated to opcode
        @DVR_OSD_OPCODE_SHOW@ in order to show or hide the given OSD
        ID for the current job.

            "osd_id"  - The OSD identifier to set its show state.

            "state"  - Show state of the currently selected OSD id.
            zero means to hide. Otherwise, show the osd text.

            "smo_port" - Setting this field to 0xFF causes the OSD text to 
            be shown or hidden on HMO, encoder, and all the spot monitor displays.
            A value of zero causes the OSD text to be shown or hidden on HMO and encoder
            but not any of the sport monitor displays.
            Otherwise, the OSD text will be shown or hidden on the specific spot monitor 
            displays.

        "text" - This union is associated to opcode
        @DVR_OSD_OPCODE_TEXT@ in order to set the OSD text of the given OSD
        ID for the current job. The OSD text is a unicode short string.
        It can be divided into multiple of 4 text and sent to the 
        firmware.

            "osd_id"  - The OSD identifier to set its display text.

            "length" - The total length of OSD display text.
            The maximum OSD display text size is 256.

            "data[4]" - Array of unsigned short unicode containing the
            nth 4 string of the OSD text. The maximum size of an OSD
            text is 256 unsigned short.

            "smo_port" - Setting this field to 0xFF causes the OSD text to 
            be changes on HMO, encoder, and all the spot monitor displays.
            A value of zero causes the OSD text to be changed on HMO and encoder
            but not any of the sport monitor displays.
            Otherwise, the OSD text will be changed on the specific spot monitor 
            displays.

************************************************************************/
typedef struct dvr_osd_ex_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            op_code;
    
    union {
        struct {
            sx_uint8    osd_id;
            sx_uint8    translucent;
            sx_uint8    position_ctrl;
            sx_uint16   x_TL;
            sx_uint16   y_TL;
            sx_uint8    dts_format;
            sx_uint8    smo_port;
            sx_uint16   reserved2;
        } config;
        struct {
            sx_uint8    osd_id;
            sx_uint8    state;
            sx_uint8    smo_port;
            sx_uint8    reserved1;
            sx_uint32   reserved2;
            sx_uint32   reserved3;
        } show;
        struct {
            sx_uint8    osd_id;
            sx_uint8    length;
            sx_uint16   data[4];
            sx_uint8    smo_port;
            sx_uint8    reserved1;
        } text;
    } u1;
} dvr_osd_ex_t;

/************************************************************************
    Defines the length of the authentication key in bytes.
************************************************************************/
#define DVR_AUTH_KEY_LEN    (32)

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_AUTH_KEY@, @DVR_REP_AUTH_KEY@.

        "status" - Status of the reply.

        "board_id" - The board identifier.

        "read_offset" - For the GET command, this is offset of the key
        to read from. For example, setting this to zero is a command to
        read the first 12 bytes of the key.
        
        "read_length" - For the REP command, this specifies the number
        of bytes actually copied into the data field. This value will
        always be less than or equal to 12.
        
        "total_length" - For the REP command, this specifies the total
        length of the key in bytes. Currently the maximum key length
        is 256 bits, or 32 bytes. For the GET command, this field is
        unused.
        
        "data" - Array of unsigned chars in which the key fragments
        are returned.
************************************************************************/
typedef struct dvr_auth_key_struct {
    dvr_status_e    status:8;
    sx_uint8        board_id;
    
    union {
        sx_uint8    read_offset;
        sx_uint8    read_length;
    } u1;
    
    sx_uint8        total_length;
    sx_uint8        data[12];
} dvr_auth_key_t;

/************************************************************************
    VISIBLE: Region map related opcodes.

    @DVR_REGION_OPCODE_START@ - Indicates that the host application is
    ready to start downloading the region map for the given region type.
    The data is downloaded through the control data channel (opened via
    an IOCTL command). Each buffer size N bytes where N is the number of
    16x16 macroblocks in the current D1 picture frame. When transferring
    the regions map, each bit in the region map index corresponding to a
    region layer for the region type. The layer bit map start from the
    right to left, where the most rightmost bit indicates the first layer.
    
    @DVR_REGION_OPCODE_FINISH@ - Indicates that the download is complete.
    
    @DVR_REGION_OPCODE_CONFIG@ - Indicates the we are configuring the 
    region by setting one or more alarm thresholds for the region type.
    
    @DVR_REGION_OPCODE_ENABLE@ - Used to enable or disable the specified
    alarm, or privacy blocking.
************************************************************************/
#define DVR_REGION_OPCODE_START           1
#define DVR_REGION_OPCODE_FINISH          2
#define DVR_REGION_OPCODE_CONFIG          3
#define DVR_REGION_OPCODE_ENABLE          4

/************************************************************************
    VISIBLE: Region type related Opcodes.

    @DVR_REGION_TYPE_MD@ - Indicates motion detection regions.
    
    @DVR_REGION_TYPE_BD@ - Indicates blind detection regions.
    
    @DVR_REGION_TYPE_ND@ - Indicates night detection regions.

    @DVR_REGION_TYPE_PR@ - Indicates privacy blocking regions.

************************************************************************/
#define DVR_REGION_TYPE_MD           1
#define DVR_REGION_TYPE_BD           2
#define DVR_REGION_TYPE_ND           3
#define DVR_REGION_TYPE_PR           4

/************************************************************************
    VISIBLE: Maximum size of a region map in bytes.
************************************************************************/
#define DVR_REGION_MAP_MAX_SIZE      2048

/************************************************************************
    VISIBLE: This structure is associated with the @DVR_SET_REGIONS_MAP@
    and @DVR_REP_REGIONS_MAP@ messages. It is used to send a different
    regions map for alarm detection and privacy blocking, as well as,
    configuring them.

    Common Fields:
    
        "status" - Status of the reply.
        
        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "opcode" - The code defining the operation to be performed.
        Always set by the host.
        
        "region_type" - The region map type to be affected by the opcode.
        
    Config Command Fields:
    
        "threshold1" - Alarm threshold for overlay number 1 of the specified
        region type.
              
        "threshold2" - Alarm threshold for overlay number 1 of the specified
        region type.
              
        "threshold3" - Alarm threshold for overlay number 1 of the specified
        region type.
              
        "threshold4" - Alarm threshold for overlay number 1 of the specified
        region type.
        
        @NOTE@: Currently, overlay numbers 2-4 are valid only for region type
        @DVR_REGION_TYPE_MD@. For all others, these are ignored.
        
    Enable Command Fields:
              
        "enable_flag" - Set to 1 to enable alarm detection, set to zero to
        disable alarm detection.

    Map Command Fields:
    
        "map_size" - The size of the regions map. This size is always the
        same as the D1 size video frame for the current video standard.
        This field must be set by the host for @DVR_REGION_OPCODE_START@
        and @DVR_REGION_OPCODE_FINISH@.

        "overlay_num" - The region overlay number to set its ROI map.
        Value of zero (0) in this field means the ROI map which is being
        sent includes all the overlay ROI for the current region type.
        Otherwise, it is the nth overlay number.
        For region_type of DVR_REGION_TYPE_MD: the valid
        range is 0 - 4. For all other region_type the valid value is 0 or 1.
        This field must be set by the host for @DVR_REGION_OPCODE_START@
        and @DVR_REGION_OPCODE_FINISH@.
************************************************************************/
typedef struct dvr_regions_map_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            opcode;
    sx_uint8            region_type;
    sx_uint8            reserved1;
    sx_uint16           reserved2;

    union {
        struct {
            sx_uint8    threshold1;
            sx_uint8    threshold2;
            sx_uint8    threshold3;
            sx_uint8    threshold4;
            sx_uint32   reserved3;
        } config;
        struct {
            sx_uint8    enable_flag;
            sx_uint8    reserved3;
            sx_uint16   reserved4;
            sx_uint32   reserved5;
        } enable;
        struct {
            sx_uint16   map_size;
            sx_uint8    overlay_num;
            sx_uint8    reserved3;
            sx_uint32   reserved4;
        } map;
    } u1;
} dvr_regions_map_t;

/************************************************************************
    VISIBLE: Opcodes for commands that support regions.
    
    @DVR_VPP_OPCODE_CONTROL@   Opcode for control messages.
    
    @DVR_VPP_OPCODE_REGIONS@   Opcode for region messages.
************************************************************************/
#define DVR_VPP_OPCODE_CONTROL  0
#define DVR_VPP_OPCODE_REGIONS  1

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_PR@, @DVR_SET_PR@, and @DVR_REP_PR@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "enable" - Set to 1 to enable and to 0 to disable motion detection.
        The default is 0.

        "num_region" - Number of private regions. A value of 1 means only
        blank out the first region. A value of 2 means blank out
        both regions.

        "x00,y00,x01,y01" - First rectangular region to be blanked
        for privacy.

        "x10,y10,x11,y11" - Second rectangular region to be blanked
        for privacy.
************************************************************************/
typedef struct dvr_pr_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            op_code;
    
    union {
        struct {
            sx_uint8    enable;
            sx_uint8    reserved1;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } ctrl;
        struct {
            sx_uint8    index;
            sx_uint8    enable;
            sx_uint16   x_TL;
            sx_uint16   y_TL;
            sx_uint16   x_BR;
            sx_uint16   y_BR;
            sx_uint16   reserved1;
        } reg;
    } u1;
} dvr_pr_t;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_MD@, @DVR_SET_MD@, and @DVR_REP_MD@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "threshold" - Motion detection threshold.  The valid range is
        0 - 99. Default is 15.

        "enable" - Set to 1 to enable and to 0 to disable motion detection.

        "num_region" - Number of regions to be used.  0
        implies the whole image, 1 uses the first region,
        and 2 uses both regions.

        "x00,y00,x01,y01" - First rectangular region to be used
        for detecting motion.

        "x10,y10,x11,y11" - Second rectangular region to be used
        for detecting motion.
************************************************************************/
typedef struct dvr_md_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            op_code;
    
    union {
        struct {
            sx_uint8    enable;
            sx_uint8    threshold;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } ctrl;
        struct {
            sx_uint8    index;
            sx_uint8    enable;
            sx_uint16   x_TL;
            sx_uint16   y_TL;
            sx_uint16   x_BR;
            sx_uint16   y_BR;
            sx_uint16   reserved1;
        } reg;
    } u1;
} dvr_md_t;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_BD@, @DVR_SET_BD@, and @DVR_REP_BD@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "threshold" - Blind detection threshold.  The valid range is
        0 - 99. Default is 50.

        "enable" - Set to 1 to enable and to 0 to disable blind detection.

        "num_region" - Number of regions to be used.  0
        implies the whole image, 1 uses the first region,
        and 2 uses both regions.

        "x00,y00,x01,y01" - First rectangular region to be used
        for blind detection.

        "x10,y10,x11,y11" - Second rectangular region to be used
        for blind detection.
************************************************************************/
typedef struct dvr_bd_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            op_code;
    
    union {
        struct {
            sx_uint8    enable;
            sx_uint8    threshold;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } ctrl;
        struct {
            sx_uint8    index;
            sx_uint8    enable;
            sx_uint16   x_TL;
            sx_uint16   y_TL;
            sx_uint16   x_BR;
            sx_uint16   y_BR;
            sx_uint16   reserved1;
        } reg;
    } u1;
} dvr_bd_t;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_ND@, @DVR_SET_ND@, and @DVR_REP_ND@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "enable" - Set to 1 to enable and to 0 to disable night detection.
        The default is 0.

        "threshold" - Night detection threshold. The valid range is
        0 - 255. Default is 40.
************************************************************************/
typedef struct dvr_nd_struct {
    sx_int8   status;
    sx_uint8  job_type;
    sx_uint8  job_id;
    sx_uint8  enable;
    sx_uint8  threshold;
    sx_uint8  reserved0;
    sx_uint16 reserved1;
    sx_uint32 reserved2;
    sx_uint32 reserved3;
} dvr_nd_t;

/************************************************************************
    VISIBLE: This structure is associated with the following messages:
    @DVR_GET_ENCODE@, @DVR_SET_ENCODE@, and @DVR_REP_ENCODE@.

    Common fields:
        
        "status" - Status of the reply. This field is only valid for
        a @DVR_REP_ENCODE@ message.

        "job_type" - Must be @DVR_JOB_CAMERA_ENCODE@.

        "job_id" - A unique job ID.

        "stream_id" - The stream identifier, specifies which encode
        stream is being queried or configured. Currently, must be 
        0 (primary) or 1 (secondary).
        
        "record_frame_rate" - Recording frame rate, must be between 1
        and the maximum allowed by the video standard (25 for PAL and
        30 for NTSC).

        "record_res" - The video resolution to record at. Must be a
        valid enum of type @dvr_video_res_e@. Not all values of 
        @dvr_video_res_e@ may be supported by all boards. Selecting
        any value other than @DVR_VIDEO_RES_FULL@, @DVR_VIDEO_RES_CIF@,
        or @DVR_VIDEO_RES_QCIF@ can have an impact on performance due
        to additional rescaling performed prior to encoding.
        
        "enable" - Set to 1 to enable or to 0 to disable the encode
        stream.
        
    H.264-specific fields:

        "avg_bitrate" - Average bit rate to be maintained, in Kbps.
        Only applicable to @DVR_H264_RC_VBR@ and @DVR_H264_RC_CBR@,
        ignored for other RC modes.

        "max_bitrate" - Maximum bit rate allowed, in Kbps. Ignored
        for @DVR_H264_RC_CBR@.

        "rate_control" - Rate control algorithm. Must be one of
        @DVR_H264_RC_VBR@, @DVR_H264_RC_CBR@, or @DVR_H264_RC_CQ@.

        "gop_size" - Number of pictures in a GOP. It is recommended
        that this value be set to have one GOP per second of video.

        "quality" - Quality level, must be between 0 and 100. This is
        applicable only for @DVR_H264_RC_CQ@ mode, and is ignored for
        other RC modes.

    MJPEG-specific fields:
    
        "quality" - Specifies the quality of compression. A higher number
        implies better quality. Must be between 10 to 300.

        "jpeg_style" - Controls frame header generation. Must be set to
        either 0 (for motion JPEG frame header) or 1 (for image style 
        frame header).
        
    MPEG4-specific fields:
    
        "avg_bitrate" - Average bit rate to be maintained, in Kbps.
        Only applicable to @DVR_MPEG4_RC_VBR@ and @DVR_MPEG4_RC_CBR@,
        ignored for other RC modes.

        "max_bitrate" - Maximum bit rate allowed, in Kbps. Ignored
        for @DVR_MPEG4_RC_CBR@.

        "rate_control" - Rate control algorithm. Must be one of
        @DVR_MPEG4_RC_VBR@ or @DVR_MPEG4_RC_CBR@.

        "gop_size" - Number of pictures in a GOP. It is recommended
        that this value be set to have one GOP per second of video.

        "quality" - Quality level, must be between 0 (best) and 7 (worst).
        The recommended (and default) value is 3. Only applicable to
        @DVR_MPEG4_RC_VBR@, ignored for other modes.
        
************************************************************************/
typedef struct dvr_encode_info_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            stream_id;
    sx_uint8            record_frame_rate;
    dvr_video_res_e     record_res:8;
    sx_uint8            enable;
    sx_uint8            reserved1;
    
    union {
        struct {
            sx_uint32           reserved2;
            sx_uint32           reserved3;
        } none;
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            dvr_rc_e            rate_control:8;
            sx_uint8            gop_size;
            sx_uint8            quality;
            sx_uint8            reserved2;
        } h264;
        struct {
            sx_uint16           quality;
            sx_uint8            jpeg_style;
            sx_uint8            reserved2;
            sx_uint32           reserved3;
        } jpeg;
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            dvr_rc_e            rate_control:8;
            sx_uint8            gop_size;
            sx_uint8            quality;
            sx_uint8            reserved2;
        } mpeg4;
    } u1;
} dvr_encode_info_t;

/************************************************************************
    VISIBLE: This structure is associated with the following messages:
    @DVR_GET_ALARM@, @DVR_SET_ALARM@, and @DVR_REP_ALARM@.
    
    Common fields:
        
        "status" - Status of the reply. This field is only valid for
        a @DVR_REP_ENC_ALARM@ message.

        "job_type" - Must be @DVR_JOB_CAMERA_ENCODE@.

        "job_id" - A unique job ID.

        "stream_id" - The stream identifier, specifies which encode
        stream is being queried or configured. Currently, must be 
        0 (primary) or 1 (secondary).
        
        "record_frame_rate" - Recording frame rate, must be between 1
        and the maximum allowed by the video standard (25 for PAL and
        30 for NTSC).

        "min_on_seconds","min_off_seconds" - Minimum number of seconds
        to remain in alarm mode (normal mode) before returning to normal
        mode (alarm mode).  This is to prevent the alarm being on and off
        too frequently.

        "enable" - Set to 1 to enable or to 0 to disable the encode
        stream.

    H.264-specific fields:

        "avg_bitrate" - Average bit rate to be maintained, in Kbps.
        Only applicable to @DVR_H264_RC_VBR@ and @DVR_H264_RC_CBR@,
        ignored for other RC modes.

        "max_bitrate" - Maximum bit rate allowed, in Kbps. Ignored
        for @DVR_H264_RC_CBR@.

        "rate_control" - Rate control algorithm. Must be one of
        @DVR_H264_RC_VBR@, @DVR_H264_RC_CBR@, or @DVR_H264_RC_CQ@.

        "gop_size" - Number of pictures in a GOP. It is recommended
        that this value be set to have one GOP per second of video.

        "quality" - Quality level, must be between 0 and 100. This is
        applicable only for @DVR_H264_RC_CQ@ mode, and is ignored for
        other RC modes.

    MJPEG-specific fields:
    
        "quality" - Specifies the quality of compression. A higher number
        implies better quality. Must be between 10 to 300.

        "jpeg_style" - Controls frame header generation. Must be set to
        either 0 (for motion JPEG frame header) or 1 (for image style 
        frame header).
    
    MPEG4-specific fields:
    
        "avg_bitrate" - Average bit rate to be maintained, in Kbps.
        Only applicable to @DVR_MPEG4_RC_VBR@ and @DVR_MPEG4_RC_CBR@,
        ignored for other RC modes.

        "max_bitrate" - Maximum bit rate allowed, in Kbps. Ignored
        for @DVR_MPEG4_RC_CBR@.

        "rate_control" - Rate control algorithm. Must be one of
        @DVR_MPEG4_RC_VBR@ or @DVR_MPEG4_RC_CBR@.

        "gop_size" - Number of pictures in a GOP. It is recommended
        that this value be set to have one GOP per second of video.

        "quality" - Quality level, must be between 0 (best) and 7 (worst).
        The recommended (and default) value is 3. Only applicable to
        @DVR_MPEG4_RC_VBR@, ignored for other modes.
        
************************************************************************/
typedef struct dvr_enc_alarm_info_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            stream_id;
    sx_uint8            record_frame_rate;
    sx_uint8            min_on_seconds;
    sx_uint8            min_off_seconds;
    sx_uint8            enable;
    
    union {
        struct {
            sx_uint32           reserved1;
            sx_uint32           reserved2;
        } none;
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            dvr_rc_e            rate_control:8;
            sx_uint8            gop_size;
            sx_uint8            quality;
            sx_uint8            reserved1;
        } h264;
        struct {
            sx_uint16           quality;
            sx_uint8            jpeg_style;
            sx_uint8            reserved1;
            sx_uint32           reserved2;
        } jpeg;
        struct {
            sx_uint16           avg_bitrate;
            sx_uint16           max_bitrate;
            dvr_rc_e            rate_control:8;
            sx_uint8            gop_size;
            sx_uint8            quality;
            sx_uint8            reserved1;
        } mpeg4;
    } u1;
} dvr_enc_alarm_info_t;


/************************************************************************
    VISIBLE: This structure is associated with the following messages:
    @DVR_GET_DECODE@, @DVR_SET_DECODE@, and @DVR_REP_DECODE@.

    Common fields:
        
        "status" - Status of the reply. This field is only valid for
        a @DVR_REP_DECODE@ message.

        "job_type" - Must be @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "enable" - Set to 1 to enable or to 0 to disable the decoder.

        "width" - Width in pixels of the encoded frames.

        "height" - Height in pixels of the encoded frames.

    @NOTE@: Width and height combinations must match one of the supported
    video resolutions. These parameters can be changed every time the job
    is being enabled.

    H.264-specific fields:
    
        None.
        
    MJPEG-specific fields:
    
        None.

************************************************************************/
typedef struct dvr_decode_info_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            enable;
    sx_uint16           width;
    sx_uint16           height;
    
    union {
        struct {
            sx_uint32       reserved3;
            sx_uint32       reserved4;
        } h264;
        struct {
            sx_uint32       reserved3;
            sx_uint32       reserved4;
        } jpeg;
        struct {
            sx_uint32       reserved3;
            sx_uint32       reserved4;
        } mpeg4;
    } u1;
} dvr_decode_info_t;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_RELAYS@, @DVR_SET_RELAYS@, and @DVR_REP_RELAYS@.

        "status" - Status of the reply.

        "relay_status" - This is a bit field, one bit per relay output:
        
            For @DVR_GET_RELAYS@, this field is unused.
            
            For @DVR_REP_RELAYS@, this field reflects the status of the
            relay outputs. If a bit is 1, the corresponding relay is on;
            if the bit is 0, the relay is off.
            
            For @DVR_SET_RELAYS@, this field specifies the new states of
            the relay outputs.
************************************************************************/
typedef struct dvr_relay_struct {
    dvr_status_e        status:8;
    sx_int8             reserved0;
    sx_int16            reserved1;
    sx_uint32           relay_status;
    sx_uint32           reserved3;
    sx_uint32           reserved4;
} dvr_relay_t;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_SENSORS@, @DVR_SET_SENSORS@, and @DVR_REP_SENSORS@.

        "status" - Status of the reply.

        "sensor_status" - For @DVR_REP_SENSORS@, this reports the states
        of the sensors, one bit per sensor input.

        "sensor_enable" - For @DVR_SET_SENSORS@, this specifies the set of
        sensors to enable. If a bit is set to 1, the corresponding sensor
        is enabled.

        "edge_triggered" - For @DVR_SET_SENSORS@, this specifies the trigger
        mode. Set to 1 for edge triggered and to 0 for level sensitive, one
        bit per sensor input.
************************************************************************/
typedef struct dvr_sensor_struct {
    dvr_status_e        status:8;
    sx_int8             reserved0;
    sx_int16            reserved1;
    sx_uint32           sensor_status;
    sx_uint32           sensor_enable;
    sx_uint32           edge_triggered;
} dvr_sensor_t;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_WATCHDOG@, @DVR_SET_WATCHDOG@, and @DVR_REP_WATCHDOG@.

        "status" - status of the reply.
        
        "enable" - Enables the watchdog timer if set to a nonzero value,
        disables the timer if set to zero. Once enabled, the timer must
        be refreshed at intervals not exceeding the specified timeout.
        For @DVR_REP_WATCHDOG@ messages, this field indicates the current
        enable state of the watchdog timer.

        "msec" - Specifies the watchdog timeout period. The valid range is
        1 to 10,000 msec. If this field is zero and the enable flag is set
        then the timeout period defaults to 10,000 msec (10 sec). If the 
        enable flag is not set, this field is ignored.
        For @DVR_REP_WATCHDOG@ messages, this field is always zero.

        @NOTE@: Once the watchdog function is enabled, successive
        @DVR_SET_WATCHDOG@ commands must be sent to the board before
        the previous timeout expires.
************************************************************************/
typedef struct dvr_watchdog_struct {
    dvr_status_e        status:8;
    sx_int8             reserved0;
    sx_int16            reserved1;
    sx_uint32           enable;
    sx_uint32           msec;
    sx_uint32           reserved3;
} dvr_watchdog_t;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_CONTROL@, @DVR_SET_CONTROL@, and @DVR_REP_CONTROL@.

        "status" - Status of the reply.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "enable" - Set to none-zero value to enable and to 0 to
        disables the job.
************************************************************************/
typedef struct dvr_control_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            enable;
    sx_uint32           reserved1;
    sx_uint32           reserved2;
    sx_uint32           reserved3;
} dvr_control_t;

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_TIME@, @DVR_SET_TIME@, and @DVR_REP_TIME@.

        "status" - Status of the reply.

        "tv_sec", "tv_usec" - Number of seconds and microseconds
        since Epoch.
************************************************************************/
typedef struct dvr_time_struct {
    dvr_status_e    status:8;
    sx_uint8        reserved0;
    sx_uint16       reserved1;
    sx_int32        tv_sec;
    sx_int32        tv_usec;
    sx_uint32       reserved3;
} dvr_time_t;

/************************************************************************
    VISIBLE: This structure describes the contents of every data buffer
    exchanged between the host and the firmware. Every data buffer must
    begin with this structure at its head.
    
        "signature" - The header signature. This is used to detect a
        valid header. Must always be set to @DVR_DATA_HDR_SIG@.
        
        "version" - Indicates the header version. Must be set to the
        current header version @DVR_DATA_HDR_VER@.
        
        "hdr_size" - Size of the header in bytes. Must be equal to
        @DVR_DATA_HDR_LEN@.
        
        "board_id" - The board identifier.

        "job_type" - The job type.

        "job_id" - The ID of the job that generated this buffer, or
        the job that is the recipient of this buffer.

        "data_type" - Indicates the type of data. Must be one of the
        valid @dvr_data_type_enum@ values.

        "motion_value" - Current motion detect parameter value.
        The value is between 0 and 255.

        "blind_value" - Current blind detect parameter value.
        The value is between 0 and 255.

        "night_value" - Current night detect parameter value.
        The value is between 0 and 255.

        "state_flags" - Set of one-bit state flags about the state of
        the job.
        
        "stream_id" - For encoder jobs, this specifies the stream ID
        of the encode stream that generated this data. For all other
        cases, this should be set to 0.

        "data_size" - Size of the data buffer in bytes.

        "ts" - Low 32 bits of hardware-generated timestamp associated
        with the data.

        "ts_high" - High 32 bits of hardware-generated timestamp.
        @NOTE@: This field is valid only in firmware build versions 3.1.1
        or later.
        
        "mval" - Motion detect values for up to four regions of interest.
        mval[0] is the same as "motion_value".
        
        "bval" - Blind detect values for up to four regions of interest.
        mval[0] is the same as "blind_value".
        
        "nval" - Night detect values for up to four regions of interest.
        mval[0] is the same as "night_value".
        
        "active_width" - Active width of video frame buffer. This field is
        valid only for raw and encoded video frames.
        
        "padded_width" - Padded width of video frame buffer. This field is
        valid only for raw and encoded video frames.
        
        "active_height" - Active height of video frame buffer. This field is
        valid only for raw and encoded video frames.
        
        "padded_height" - Padded height of video frame buffer. This field is
        valid only for raw and encoded video frames.
        
        "seq_number" - Frame sequence number. This field is valid only
        for raw and encoded video frames. Each board_id/job_id/stream
        combination will have independent sequence numbering. The raw
        and encoded video streams from the same job will have independent
        sequence numbering. The sequence number starts from 1 and is reset
        when the stream is disabled.
        
        "frame_number" - Number of frames seen on this channel so far.
        This field is valid only for raw and encoded video frames. The
        frame number and sequence number will be identical when the 
        stream is being run at full frame rate and no frames are dropped.
        The counter is reset when the stream is disabled.

        "drop_count" - Number of frames drop detected by the firmware on the
        current stream. The drop count is reset when the stream is disabled.

        The following fields are only used in receiving raw YUV frames
        from the host DVR Application:

            "yuv_format" - The current YUV format.

            "y_data_size" - The size of the Y data in bytes.

            "u_data_size" - The size of the U data in bytes.

            "v_data_size" - The size of the V data in bytes.

            "y_data_offset" -  The offset of Y data of a raw frame  
            relative from the start of the header

            "u_data_offset" -  The offset of U data of a raw frame  
            relative from the start of the header

            "v_data_offset" -  The offset of V data of a raw frame  
            relative from the start of the header

************************************************************************/
typedef struct dvr_data_header_struct {
    sx_uint32       signature;          // Must be set to DVR_DATA_HDR_SIG
    sx_uint16       version;            // Must be set to DVR_DATA_HDR_VER
    sx_uint16       hdr_size;           // Must be set to sizeof(dvr_data_header_t)
    
    // These 4 fields are backward compatible
    sx_uint8        board_id;
    dvr_job_type_e  job_type:8;
    sx_uint8        job_id;
    dvr_data_type_e data_type:8;
    
    // These 4 fields are backward compatible
    sx_uint8        motion_value;
    sx_uint8        blind_value;
    sx_uint8        night_value;
    sx_uint8        state_flags;

    // These 2 fields are backward compatible
    sx_uint8        stream_id;
    sx_uint8        reserved2;
    sx_uint16       reserved3;
    sx_uint32       data_size;

    // These 2 fields are backward compatible
    sx_uint32       ts;
    sx_uint32       ts_high;

    // New fields. m/b/nval[0] same as the compatible fields above
    sx_uint8        mval[4];
    sx_uint8        bval[4];
    sx_uint8        nval[4];

    // These fields are for the Y plane
    sx_uint16       active_width;
    sx_uint16       padded_width;
    sx_uint16       active_height;
    sx_uint16       padded_height;
    
    sx_uint32       seq_number;
    sx_uint32       frame_number;
    sx_uint32       drop_count;

    sx_uint8             reserved4[3];
    dvr_rawv_format_e    yuv_format:8;

    sx_uint32       y_data_size;
    sx_uint32       u_data_size;
    sx_uint32       v_data_size;
    sx_uint32       y_data_offset;
    sx_uint32       u_data_offset;
    sx_uint32       v_data_offset;

    // These fields are for the U and V planes. Assume U and V are always
    // equal wisth and height
    sx_uint16       uv_active_width;
    sx_uint16       uv_padded_width;
    sx_uint16       uv_active_height;
    sx_uint16       uv_padded_height;

    sx_uint32       reserved[39];
} dvr_data_header_t;

/************************************************************************
    Data header signature value. Only present in new-style header.
************************************************************************/
#define DVR_DATA_HDR_SIG    (0x00524448)

/************************************************************************
    Current data header version.
************************************************************************/
#define DVR_DATA_HDR_VER    (0x2)

/************************************************************************
    Data header size. Must always be a multiple of 16.
************************************************************************/
#define DVR_DATA_HDR_LEN    (sizeof(dvr_data_header_t))

/************************************************************************
    VISIBLE: This structure defines the data associated with message
    @DVR_SIG_HOST@ from the board to the host.

        "sig_type" - Type of signal, e.g. @DVR_SIGNAL_SENSOR_ACTIVATED@.

        "job_type" - Must be one of @DVR_JOB_CAMERA_ENCODE@,
        @DVR_JOB_HOST_ENCODE@, or @DVR_JOB_HOST_DECODE@.

        "job_id" - A unique job ID.

        "signal_data" - Information associated with the particlar signal
        type.

            For @DVR_SIGNAL_SENSOR_ACTIVATED@, it is the sensor status.
            
            For @DVR_SIGNAL_MOTION_DETECTED@,  it is the motion value.
            
            For @DVR_SIGNAL_BLIND_DETECTED@,   it is the blind value.
            
            For @DVR_SIGNAL_NIGHT_DETECTED@,   it is the night value.
            
            For @DVR_SIGNAL_VIDEO_LOST@,       there is no data.
            
            For @DVR_SIGNAL_VIDEO_DETECTED@,   there is no data.
            
            For @DVR_SIGNAL_RUNTIME_ERROR@,    it is the error code.
            
            For @DVR_SIGNAL_FATAL_ERROR@,      it is the error code.
            
            For @DVR_SIGNAL_HEARTBEAT@,        it is the board id.

            For @DVR_SIGNAL_WATCHDOG_EXPIRED@, there is no data.

        "extra_data" - Extra information associated with the signal, if any.

            For @DVR_SIGNAL_RUNTIME_ERROR@,    it is the extended error code.
            
            For @DVR_SIGNAL_FATAL_ERROR@,      it is the extended error code.

    @NOTE@: For messages such as @DVR_SIGNAL_WATCHDOG_EXPIRED@, job_type
    and job_id are not meaningful, and are always zero.
************************************************************************/
typedef struct dvr_sig_host_struct {
    dvr_signal_type_e   sig_type:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            reserved1;
    sx_uint32           signal_data;
    sx_uint32           extra_data;
    sx_uint32           reserved3;
} dvr_sig_host_t;

/************************************************************************
    VISIBLE: Command codes for DVR RS-485 port control.

    @DVR_UART_CMD_CONFIG@ - UART configuration command.

    @DVR_UART_CMD_OUTPUT@ - UART output command.
************************************************************************/
#define DVR_UART_CMD_CONFIG    1
#define DVR_UART_CMD_OUTPUT    2

/************************************************************************
    VISIBLE: This structure is used for @DVR_GET_UART@, @DVR_SET_UART@
    and @DVR_REP_UART@ messages.

    Common Fields:
    
        "status" - Status of the reply.

        "cmd" - Command code, either @DVR_UART_CONFIG@ or @DVR_UART_OUTPUT@.
        Only valid for @DVR_SET_UART@ messages.
    
    Config Command Fields:

        "baud_rate" - The output baud rate. Must be between 50 and 115200.

        "data_bits" - Number of data bits. Valid values are 5-8.

        "stop_bits" - Number of stop bits. Valid values are 1 and 2.

        "parity_enable" - If this field is set to zero, parity is disabled.
        If set to a nonzero value, parity is enabled.

        "parity_even" - If this field is set to zero, odd parity is used.
        If set to a nonzero value, even parity is used. This field is
        ignored if parity is disabled.

    Output Command Fields:

        "count" - Number of bytes to transmit. Must be between 1 and 11.

        "data" - The data bytes to transmit. The number of valid bytes in
        the array is determined by the "count" field.

    Input Command Fields:

        "count" - For @DVR_GET_UART@, specifies the maximum number of bytes
        to read. For @DVR_REP_UART@, specifies the actual number of bytes
        read, which can be less than the number requested.

        "data" - For @DVR_REP_UART@, contains the bytes read, if any.
        The number of valid bytes is determined by the "count" field.
************************************************************************/
typedef struct dvr_uart_struct {
    dvr_status_e       status:8;
    sx_uint8           cmd;
    sx_uint16          reserved1;

    union {
        struct {
            sx_uint32    baud_rate;
            sx_uint8     data_bits;
            sx_uint8     stop_bits;
            sx_uint8     parity_enable;
            sx_uint8     parity_even;
            sx_uint32    reserved2;
        } config;
        struct {
            sx_uint8     count;
            sx_uint8     data[11];
        } output;
        struct {
            sx_uint8     count;
            sx_uint8     data[11];
        } input;
    } u1;
} dvr_uart_t;


/************************************************************************
    VISIBLE: Stretch DVR IOCTL codes. These codes are used by the
    @DVR_GET_IOCTL@, @DVR_SET_IOCTL@, and @DVR_REP_IOCTL@ commands.
    
    @DVR_IOCTL_CODE_IMG_CONTROL@ - Used to get and set video decoder image
    control parameters.
    
    @DVR_IOCTL_CODE_DECODER_REGS@ - Used to get and set video decoder
    registers. Provides low level access to decoder configuration.
    
    @DVR_IOCTL_CODE_OPEN_CHAN@ - Used to open a data channel between the
    host and the firmware for control data, e.g. region map data. Used
    only with the SET and REP commands. This command is per board.
    
    @DVR_IOCTL_CODE_CLOSE_CHAN@ - Used to close the data channel between
    the host and the firmware. Used only with SET and REP commands.
    This command is per board.

    @DVR_IOCTL_CODE_MOTION_FRAME_FREQ@ - Used to specify how often to send
    motion value frames. The default is zero. 1 means send for every raw
    video frame recieved, 2 means send every second frame, and so on.
    Used only with the SET and REP commands.
    This command is per board.

    @DVR_IOCTL_CODE_VPP_CONTROL@ - Controls video preprocessing actions.
    See @dvr_vpp_action_enum@ for the supported actions.

    @DVR_IOCTL_CODE_GAIN_MODE@ - TBD

    @DVR_IOCTL_CODE_TERMINATION@ - TBD

    @DVR_IOCTL_CODE_LED@ - Used to set/get the current enable status of
    different LEDs on the DVR Board.

    @DVR_IOCTL_CODE_RAWV_FORMAT@ - Used to set the format of raw video output
    from the board.
************************************************************************/
#define DVR_IOCTL_CODE_IMG_CONTROL          1
#define DVR_IOCTL_CODE_DECODER_REGS         2
#define DVR_IOCTL_CODE_OPEN_CHAN            3
#define DVR_IOCTL_CODE_CLOSE_CHAN           4
#define DVR_IOCTL_CODE_MOTION_FRAME_FREQ    5
#define DVR_IOCTL_CODE_VPP_CONTROL          7
#define DVR_IOCTL_CODE_GAIN_MODE            9
#define DVR_IOCTL_CODE_TERMINATION          10
#define DVR_IOCTL_CODE_LED                  11
#define DVR_IOCTL_CODE_RAWV_FORMAT          12

/************************************************************************
    VISIBLE: Flags for use with the @DVR_SET_IOCTL@ command. These flags
    indicate which fields of the data structure are valid.
    
    @DVR_ICFLAG_HUE@ - Use the hue parameter.
    
    @DVR_ICFLAG_SAT@ - Use the saturation parameter.
    
    @DVR_ICFLAG_BRT@ - Use the brightness parameter.
    
    @DVR_ICFLAG_CONT@ - Use the contrast parameter.
    
    @DVR_ICFLAG_SHARP@ - Use the sharpness parameter.
************************************************************************/
#define DVR_ICFLAG_HUE                  0x1
#define DVR_ICFLAG_SAT                  0x2
#define DVR_ICFLAG_BRT                  0x4
#define DVR_ICFLAG_CONT                 0x8
#define DVR_ICFLAG_SHARP                0x10

/****************************************************************************
  VISIBLE: Enumerated type describing various camera input impedance 
  termination used with @DVR_IOCTL_CODE_CAMERA_SETTINGS@ for
  termination field.

    @DVR_TERM_75OHM@ - 75 ohm impedance termination.

    @DVR_TERM_HIGH_IMPEDANCE@ - High impedance termination.

***************************************************************************/
enum dvr_term_enum {
    DVR_TERM_75OHM,
    DVR_TERM_HIGH_IMPEDANCE
};

/****************************************************************************
  VISIBLE: Enumerated type describing various LED types.

    @DVR_LED_TYPE_RECORD@ - A group of LEDs indicating the current state of
    recording.

    @SDVR_LED_TYPE_ALARM@ - A group of LEDs indicating the current state
    of the alarms.

***************************************************************************/

enum dvr_led_type_enum {
	DVR_LED_TYPE_RECORD,
	DVR_LED_TYPE_ALARM
};

/************************************************************************
    VISIBLE: This structure is used by the @DVR_GET_IOCTL@, @DVR_SET_IOCTL@,
    and @DVR_REP_IOCTL@ messages.
    
    Common Fields:
    
        "status" - Status of the reply.
        
        "job_type" - Must be @DVR_JOB_CAMERA_ENCODE@, except for per-board
        ioctl commands. For these, the field is ignored.

        "job_id" - A unique job ID. Ignored for per-board ioctl commands.
        
        "ioctl_code" - The IOCTL command code.

    Image Control Fields (used with @DVR_IOCTL_CODE_IMG_CONTROL@):
    
        "flags" - Bit field specifying which of the following fields is
        valid.
        
        "hue" - Value to set for hue control (for set), current value of
        hue control (for get).
        
        "sat" - Value to set for saturation control (for set), current
        value of saturation control (for get).
        
        "brt" - Value to set for brightness control (for set), current
        value of brightness control (for get).
        
        "cont" - Value to set for contrast control (for set), current
        value of contrast control (for get).
        
        "sharp" - Value to set for sharpness control (for set), current
        value of sharpness control (for get).
        
    Register R/W Fields (used with @DVR_IOCTL_CODE_DECODER_REGS@):
    
        "device_id" - Device ID of the decoder to access.
        
        "reg_num" - Register number to read or write.
        
        "val" - Value to write, or value read back.
        
    Channel control fields (used with @DVR_IOCTL_CODE_OPEN_CHAN@ etc.):
    This port ID is per board not per channel.

        "port" - The SCT port number to use for the channel.
        
    Motion control fields (used with @DVR_IOCTL_CODE_MOTION_FRAME_FREQ@):

        "frequency" - How often to send the motion value frames.
        0 means never, 1 means every frame, and so on. Note that
        even if this is set to zero, a motion value frame will be
        sent every time the motion alarm threshold is exceeded.

    Gain mode field (used with @DVR_IOCTL_CODE_GAIN_MODE@):

        "value" - Gain setting value. 
        This value will be passed through as-is to the hardware.

    Video preprocessing control field (used with @DVR_IOCTL_CODE_VPP_CONTROL@):

        "actions" - Bitfield of VPP actions to control preprocessing
        behavior. Set the corresponding action flags to enable the 
        actions, clear the flags to disable them.

    Termination value field (used with @DVR_IOCTL_CODE_TERMINATION@):

        "value" - Enumerated constant defining the termination value.

    LED enable status field (used with @DVR_IOCTL_CODE_LED@):

        "type" - The type of LED to set its enable status.

        "number" - A zero based LED number within the LED type group.

        "enable" - Set to true to turn the LED on, false to turn it off.

    Raw video format field (used with @DVR_IOCTL_CODE_RAWV_FORMAT@):

        "format" - The raw video output format. Must be one of the
        @dvr_rawv_format_e@ values. The default format at startup is
        @DVR_RAWV_FORMAT_YUV_4_2_0@.

    All reserved fields must be set to zero.
    
    @NOTE@: Default values for image control parameters are dependent
    on the type of hardware used. To obtain the defaults for a specific
    board, use the @DVR_GET_IOCTL@ command to read the parameters before
    changing any of them.
************************************************************************/
typedef struct dvr_ioctl_struct {
    dvr_status_e        status:8;
    dvr_job_type_e      job_type:8;
    sx_uint8            job_id;
    sx_uint8            ioctl_code;
    
    union {
        struct {
            sx_uint16   flags;
            sx_uint16   reserved2;
            sx_uint8    hue;
            sx_uint8    sat;
            sx_uint8    brt;
            sx_uint8    cont;
            sx_uint8    sharp;
            sx_uint8    reserved3;
            sx_uint16   reserved4;
        } img_ctrl;
        struct {
            sx_uint8    device_id;
            sx_uint8    reg_num;
            sx_uint16   val;
            sx_uint32   reserved2;
            sx_uint32   reserved3;
        } reg;
        struct {
            sx_uint16   port;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } chan;
        struct {
            sx_uint8    frequency;
            sx_uint8    reserved1;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } motion;
        struct {
            sx_uint8    value;
            sx_uint8    reserved1;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } gain_mode;
        struct {
            dvr_term_e  value:8;
            sx_uint8    reserved1;
            sx_uint16   reserved2;
            sx_uint32   reserved3;
            sx_uint32   reserved4;
        } termination;
        struct {
            dvr_led_type_e  type:8;
            sx_uint8        number;
            sx_uint8        enable;
            sx_uint8        reserved1;
            sx_uint32       reserved2;
            sx_uint32       reserved3;
        } led;
        struct {
            sx_uint32       actions;
            sx_uint32       reserved1;
            sx_uint32       reserved2;
        } vpp_ctrl;
        struct {
            dvr_rawv_format_e    format:8;
            sx_uint8             reserved1;
            sx_uint16            reserved2;
            sx_uint32            reserved3;
            sx_uint32            reserved4;
        } rawv_format;
    } u1;
} dvr_ioctl_t;


/************************************************************************
    VISIBLE: Flags for use with the @DVR_SET_VIDEO_OUTPUT@ command. These flags
    indicate which fields of the data structure are valid.
    
    @DVR_VOUT_OPCODE_START@ - Prepare SCT channel to start receiving
    YUV video to be displayed on the given SMO port number. After this
    message, DVR firmware accepts video frames from the DVR Host Application.
    
    @DVR_VOUT_OPCODE_STOP@ - The SMO SCT receive channel will be closed
    and no more SMO video frames will be accepted from the DVR Host
    Applicaiton.
    
************************************************************************/
#define DVR_VOUT_OPCODE_START       0x1
#define DVR_VOUT_OPCODE_STOP        0x2


/************************************************************************
    VISIBLE: This structure is used by the @DVR_SET_VIDEO_OUTPUT@
    and @DVR_REP_VIDEO_OUTPUT@ messages.
    
    Common Fields:
    
        "status" - Status of the reply.
        
        "opcode" - The VOUT command code.
        
        "smo_port_num" - The zero based SMO port number

    Fields (used with @DVR_VOUT_OPCODE_START@):
    
        "sct_port_id" - The SCT port ID that should be used by the firmware
        to receive YUV frames to be displayed on the given SMO. 
        This field is assumed to be set by the
        host only when the opcode is @DVR_VOUT_OPCODE_START@.

        "video_format" - The raw video format that is going to be 
        received from the host DVR Application.
        see @dvr_rawv_formats_e@.


************************************************************************/
typedef struct dvr_videoout_struct {
    dvr_status_e        status:8;
    sx_uint8            opcode;
    sx_uint8            smo_port_num;
    sx_uint8            reserved;
    
    union {
        struct {
            sx_uint32   sct_port_id;
            sx_uint16   video_format;
            sx_uint16   reserved1;
            sx_uint32   reserved2;
        } start;
    } u1;

} dvr_videoout_t;

/****************************************************************************
    VISIBLE: The following enum describes various supported SMO capabilities.
    Each feature is supported by the SMO port if the corresponding bit is set.

    DVR_SMO_CAP_OUTPUT - Supports outputing of DVR Host Application 
    generated raw video.

    DVR_SMO_CAP_OSD - Supports OSD text that is displayed only on this SMO port.

    DVR_SMO_CAP_ALPHA_BLENDING - Supports alpha blending of OSD.

    DVR_SMO_CAP_TILING - Supports tiling of video, streaming from different
    Cameras or host output on this SMO display.

    DVR_SMO_CAP_ANALOG - Supports one analog camera input.
****************************************************************************/
enum dvr_smo_capabilities_e {
    DVR_SMO_CAP_OUTPUT = 1,
    DVR_SMO_CAP_OSD = 2,
    DVR_SMO_CAP_ALPHA_BLENDING = 4,
    DVR_SMO_CAP_TILING = 8,
    DVR_SMO_CAP_ANALOG = 16,
};

/************************************************************************
    VISIBLE: This structure is associated with the messages
    @DVR_GET_SMO_ATTRIB@ and @DVR_REP_SMO_ATTRIB@.  
    
        "status" - Status of the reply.

        "smo_port_num" - The SMO port number to get its capablities.
  
        "video_formats" - A bit map to indicate supported raw video formats.
        see @dvr_rawv_formats_e@.
        
        "width" - The width of the SMO display.
        
        "height" - The number of lines of the SMO display

        "cap_flags" - A bit map specifing all the capabilities of the 
        requested SMO port. See @dvr_smo_capabilities_e@.

************************************************************************/
typedef struct dvr_smo_attrib_struct {
    dvr_status_e        status:8;
    sx_uint8            smo_port_num;
    sx_uint16           video_formats;
    
    sx_uint16           width;
    sx_uint16           height;

    sx_uint16           cap_flags;
    sx_uint16           reserved1;
    sx_uint32           reserved2;

} dvr_smo_attrib_t;



#endif /* STRETCH_DVR_COMMON_H */


