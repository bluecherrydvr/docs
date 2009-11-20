/****************************************************************************\
*  Copyright C 2007 Stretch, Inc. All rights reserved. Stretch products are  *
*  protected under numerous U.S. and foreign patents, maskwork rights,       *
*  copyrights and other intellectual property laws.                          *
*                                                                            *
*  This source code and the related tools, software code and documentation,  *
*  and your use thereof, are subject to and governed by the terms and        *
*  conditions of the applicable Stretch IDE or SDK and RDK License Agreement *
*  (either as agreed by you or found at www.stretchinc.com). By using these  *
*  items, you indicate your acceptance of such terms and conditions between  *
*  you and Stretch, Inc. In the event that you do not agree with such terms  *
*  and conditions, you may not use any of these items and must immediately   *
*  destroy any copies you have made.                                         *
\****************************************************************************/

#ifndef SDVR_H
#define SDVR_H

#include <QString>
#include <QSize>
#include "sdvr_sdk.h"

namespace sdvr {

    const QString BoardSubRevToString(sx_uint8 sub_rev);

    const char *ChipRevToString(sdvr_chip_rev_e chip_rev);

    const char *ChannelTypeToString(sdvr_chan_type_e chan_type);

    const char *videoStandardToString(sdvr_video_std_e std);
    sdvr_video_std_e stringToVideoStandard(const QString &str);

    sdvr_vpp_mode_e stringToVPPMode(const QString &str);
    const char * vppModeToString(sdvr_vpp_mode_e vppMode);

    const char *videoEncToString(sdvr_venc_e enc);
    sdvr_venc_e stringToVideoEnc(const QString &str);

    const char *audioEncToString(sdvr_aenc_e enc);
    sdvr_aenc_e stringToAudioEnc(const QString &str);

    const char *audioRateToString(sdvr_audio_rate_e audioRate);
    sdvr_audio_rate_e stringToAudioRate(const QString &str);

    const char *decimationToString(sdvr_video_res_decimation_e dec);
    sdvr_video_res_decimation_e stringToDecimation(const QString &str);

    const char *osdLocationToString(sdvr_location_e loc);
    sdvr_location_e stringToOsdLocation(const QString &str);

    const char *dtsFormatToString(sdvr_dts_style_e fmt);
    sdvr_dts_style_e stringToDtsFormat(const QString &str);

    const char *brcToString(sdvr_br_control_e brc);
    sdvr_br_control_e stringToBrc(const QString &str);

    const char *camTerminationToString(sdvr_term_e term);
    sdvr_term_e StringToCamTermination(const QString &str);

    QSize frameSize(sdvr_video_std_e vs, sdvr_video_res_decimation_e dec);
    int   frameRate(sdvr_video_std_e vs);
    int   verifyFrameRate(sdvr_video_std_e vs, int verifyFPS);

    sdvr_video_res_decimation_e getResDecimation(sdvr_video_std_e vSTDTarget, 
                                                 sdvr_video_res_decimation_e decimationTarget, 
                                                 sdvr_video_std_e vSTDSrc);

    sdvr_video_size_e WidthHeightToVSize(uint width, uint height);
    QSize VSizeToWidthHeight(sdvr_video_size_e vStd);

    int getHMOAspectRatio(int width, sdvr_video_std_e vs);

    const char *errorMessage(int code);

    bool getVideoSizeFromFile(sdvr_video_size_e *video_size, const QString &fileName);

    bool isMinVersion(sx_uint8 major, sx_uint8 minor, sx_uint8 revision, sx_uint8 build,
                 sx_uint8 majorChk, sx_uint8 minorChk, sx_uint8 revisionChk, sx_uint8 buildChk);

};

#endif
