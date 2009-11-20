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

/************************************************************************
    Returns status name, cannot be freed.
************************************************************************/
char *dvrfw_status_name(dvr_status_e status)
{
    switch (status) {
    case DVR_STATUS_OK:                     return "ok";
    case DVR_STATUS_WRONG_CAMERA_NUMBER:    return "wrong camera number";
    case DVR_STATUS_WRONG_CAMERA_TYPE:      return "wrong camera type";
    case DVR_STATUS_WRONG_CODEC_FORMAT:     return "wrong codec format";
    case DVR_STATUS_WRONG_JOB_TYPE:         return "wrong job type";
    case DVR_STATUS_WRONG_JOB_ID:           return "wrong job id";
    case DVR_STATUS_WRONG_VIDEO_FORMAT:     return "wrong video format";
    case DVR_STATUS_WRONG_AUDIO_FORMAT:     return "wrong audio format";
    case DVR_STATUS_EXCEED_CPU_LIMIT:       return "exceed cpu limit";
    case DVR_STATUS_JOB_NOT_CREATED:        return "job not created";
    case DVR_STATUS_JOB_ALREADY_CREATED:    return "job_already created";
    case DVR_STATUS_JOB_NOT_ENABLED:        return "job not enabled";
    case DVR_STATUS_JOB_NOT_DISABLED:       return "job not disabled";
    default:                                return "unknown status";
    }
}

/************************************************************************
    Returns job type name.  Cannot be freed.
************************************************************************/
char *dvrfw_job_type_name(dvr_job_type_e job_type)
{
    switch (job_type) {
    case DVR_JOB_CAMERA_ENCODE: return "camera encode";
    case DVR_JOB_HOST_ENCODE:   return "host encode";
    case DVR_JOB_HOST_DECODE:   return "host decode";
    default:                    return "unknown job type";
    }
}

/************************************************************************
    Returns job type name.  Cannot be freed.
************************************************************************/
dvr_job_type_e dvrfw_job_type_by_name(char *job_type)
{
    if (strcmp(job_type, "camera_encode") == 0) {
        return DVR_JOB_CAMERA_ENCODE;
    } else if (strcmp(job_type, "host_encode") == 0) {
        return DVR_JOB_HOST_ENCODE;
    } else if (strcmp(job_type, "host_decode") == 0) {
        return DVR_JOB_HOST_DECODE;
    } else {
        assert(0);
        return 0;
    }
}

/************************************************************************
    Returns video format name.  Cannot be freed.
************************************************************************/
char *dvrfw_video_format_name(dvr_vc_format_e video_format)
{
    switch (video_format) {
    case DVR_VC_FORMAT_NONE:    return "none";
    case DVR_VC_FORMAT_H264:    return "h264";
    case DVR_VC_FORMAT_JPEG:    return "jpeg";
    default:                    return "unknown video format";
    }
}

/************************************************************************
    Returns audio format name.  Cannot be freed.
************************************************************************/
char *dvrfw_audio_format_name(dvr_ac_format_e audio_format)
{
    switch (audio_format) {
    case DVR_AC_FORMAT_NONE:        return "none";
    case DVR_AC_FORMAT_G711:        return "g711";
    case DVR_AC_FORMAT_G726_16K:    return "g726-16k";
    case DVR_AC_FORMAT_G726_24K:    return "g726-24k";
    case DVR_AC_FORMAT_G726_32K:    return "g726-32k";
    case DVR_AC_FORMAT_G726_48K:    return "g726-48k";
    default:                        return "unknown audio format";
    }
}

/************************************************************************
    Returns job control name.  Cannot be freed.
************************************************************************/
char *dvrfw_job_control_name(dvr_job_action_e control)
{
    switch (control) {
    case DVR_JOB_CREATE:    return "create";
    case DVR_JOB_DESTROY:   return "destroy";
    case DVR_JOB_RESET:     return "reset";
    default:                return "unknown job action";
    }
}

/************************************************************************
    Video format name
************************************************************************/
dvr_vc_format_e dvrfw_video_format_by_name(char *video_format)
{
    if (strcmp(video_format, "h264") == 0) {
        return DVR_VC_FORMAT_H264;
    } else if (strcmp(video_format, "jpeg") == 0) {
        return DVR_VC_FORMAT_JPEG;
    } else {
        return DVR_VC_FORMAT_NONE;
    }
}

/************************************************************************
    Audio format name
************************************************************************/
dvr_ac_format_e dvrfw_audio_format_by_name(char *audio_format)
{
    if (strcmp(audio_format, "g711") == 0) {
        return DVR_AC_FORMAT_G711;
    } else if (strcmp(audio_format, "g726-16k") == 0) {
        return DVR_AC_FORMAT_G726_16K;
    } else if (strcmp(audio_format, "g726-24k") == 0) {
        return DVR_AC_FORMAT_G726_24K;
    } else if (strcmp(audio_format, "g726-32k") == 0) {
        return DVR_AC_FORMAT_G726_32K;
    } else if (strcmp(audio_format, "g726-48k") == 0) {
        return DVR_AC_FORMAT_G726_48K;
    } else {
        return DVR_AC_FORMAT_NONE;
    }
}

/************************************************************************
    Returns job control name.  Cannot be freed.
************************************************************************/
char *dvrfw_camera_type_name(sx_uint16 camera_type)
{
    if (camera_type & DVR_VSTD_D1_PAL) {
        return "d1-pal";
    } else if (camera_type & DVR_VSTD_D1_NTSC) {
        return "d1-ntsc";
    } else if (camera_type & DVR_VSTD_CIF_PAL) {
        return "cif-pal";
    } else if (camera_type & DVR_VSTD_CIF_NTSC) {
        return "cif-ntsc";
    } else if (camera_type & DVR_VSTD_2CIF_PAL) {
        return "2cif-pal";
    } else if (camera_type & DVR_VSTD_2CIF_NTSC) {
        return "2cif-ntsc";
    } else if (camera_type & DVR_VSTD_4CIF_PAL) {
        return "4cif-pal";
    } else if (camera_type & DVR_VSTD_4CIF_NTSC) {
        return "4cif-ntsc";
    } else {
        return "unknown camera type";
    }
}

/************************************************************************
    Camera format name
************************************************************************/
sx_uint16 dvrfw_camera_type_by_name(char *camera_type)
{
    if (strcmp(camera_type, "d1-pal") == 0) {
        return DVR_VSTD_D1_PAL;
    } else if (strcmp(camera_type, "d1-ntsc") == 0) {
        return DVR_VSTD_D1_NTSC;
    } else if (strcmp(camera_type, "cif-pal") == 0) {
        return DVR_VSTD_CIF_PAL;
    } else if (strcmp(camera_type, "cif-ntsc") == 0) {
        return DVR_VSTD_CIF_NTSC;
    } else if (strcmp(camera_type, "2cif-pal") == 0) {
        return DVR_VSTD_2CIF_PAL;
    } else if (strcmp(camera_type, "2cif-ntsc") == 0) {
        return DVR_VSTD_2CIF_NTSC;
    } else if (strcmp(camera_type, "4cif-pal") == 0) {
        return DVR_VSTD_4CIF_PAL;
    } else if (strcmp(camera_type, "4cif-ntsc") == 0) {
        return DVR_VSTD_4CIF_NTSC;
    } else {
        return DVR_VSTD_UNKNOWN;
    }
}

/************************************************************************
    Job action name
************************************************************************/
dvr_job_action_e dvrfw_job_action_by_name(char *action)
{
    if (strcmp(action, "create") == 0) {
        return DVR_JOB_CREATE;
    } else if (strcmp(action, "destroy") == 0) {
        return DVR_JOB_DESTROY;
    } else if (strcmp(action, "reset") == 0) {
        return DVR_JOB_RESET;
    } else {
        assert(0);;
        return DVR_JOB_RESET;
    }
}


