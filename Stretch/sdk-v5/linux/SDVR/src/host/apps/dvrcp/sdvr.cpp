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
#include "sdvr.h"
#include "decoder.h"
#include "dvrsystem.h"

extern DVRSystem *g_DVR;

const QString sdvr::BoardSubRevToString(sx_uint8 sub_rev)
{
    if ((sub_rev >= 'A' && sub_rev <= 'Z') ||
        (sub_rev >= 'a' && sub_rev <= 'z'))
        return QString("%1").arg((char)sub_rev).toUpper();
    else
        return QString("");
}

const char *sdvr::ChipRevToString(sdvr_chip_rev_e chip_rev)
{
    switch (chip_rev)
    {
    case SDVR_CHIP_S6100_3_REV_C:   return "S6100_3 Rev_C";
    case SDVR_CHIP_S6105_3_REV_C:   return "S6105_3 Rev_C";
    case SDVR_CHIP_S6106_3_REV_C:   return "S6106_3 Rev_C";
    case SDVR_CHIP_S6100_3_REV_D:   return "S6100_3 Rev_D";
    case SDVR_CHIP_S6105_3_REV_D:   return "S6105_3 Rev_D";
    case SDVR_CHIP_S6106_3_REV_D:   return "S6106_3 Rev_D";
    case SDVR_CHIP_S6100_3_REV_F:   return "S6100_3 Rev_F";
    case SDVR_CHIP_S6105_3_REV_F:   return "S6105_3 Rev_F";
    case SDVR_CHIP_S6106_3_REV_F:   return "S6106_3 Rev_F";
    case SDVR_CHIP_S6100_3_UNKNOWN: return "S6100_3 Unkown";
    case SDVR_CHIP_S6105_3_UNKNOWN: return "S6105_3 Unkown";
    case SDVR_CHIP_S6106_3_UNKNOWN: return "S6106_3 Unkown";
    case SDVR_CHIP_UNKNOWN:         return "Unknown";
    default:                    return "unknown ID";

    }
}
const char *sdvr::ChannelTypeToString(sdvr_chan_type_e chan_type)
{
    switch (chan_type) {
    case SDVR_CHAN_TYPE_ENCODER: return "Encoder";
    case SDVR_CHAN_TYPE_DECODER: return "Decoder";
    default:                     return "unknown type";
    }
}
sdvr_video_std_e sdvr::stringToVideoStandard(const QString &str)
{
	sdvr_video_std_e vs;

	if (str == "D1_PAL")
        vs = SDVR_VIDEO_STD_D1_PAL;
    else if (str == "D1_NTSC")
        vs = SDVR_VIDEO_STD_D1_NTSC;
    else if (str == "CIF_PAL")
        vs = SDVR_VIDEO_STD_CIF_PAL;
    else if (str == "CIF_NTSC")
        vs = SDVR_VIDEO_STD_CIF_NTSC;
    else if (str == "2CIF_PAL")
        vs = SDVR_VIDEO_STD_2CIF_PAL;
    else if (str == "2CIF_NTSC")
        vs = SDVR_VIDEO_STD_2CIF_NTSC;
    else if (str == "4CIF_PAL")
        vs = SDVR_VIDEO_STD_4CIF_PAL;
    else if (str == "4CIF_NTSC")
        vs = SDVR_VIDEO_STD_4CIF_NTSC;
    else
        vs = SDVR_VIDEO_STD_NONE;

    return vs;
}

const char *sdvr::videoStandardToString(sdvr_video_std_e vs)
{
    switch (vs)
    {
        case SDVR_VIDEO_STD_D1_PAL:
        	return "D1_PAL";
        case SDVR_VIDEO_STD_D1_NTSC:
        	return "D1_NTSC";
        case SDVR_VIDEO_STD_CIF_PAL:
        	return "CIF_PAL";
        case SDVR_VIDEO_STD_CIF_NTSC:
        	return "CIF_NTSC";
        case SDVR_VIDEO_STD_2CIF_PAL:
        	return "2CIF_PAL";
        case SDVR_VIDEO_STD_2CIF_NTSC:
        	return "2CIF_NTSC";
        case SDVR_VIDEO_STD_4CIF_PAL:
        	return "4CIF_PAL";
        case SDVR_VIDEO_STD_4CIF_NTSC:
        	return "4CIF_NTSC";
        case SDVR_VIDEO_STD_QCIF_PAL:
        	return "QCIF_PAL";
        case SDVR_VIDEO_STD_QCIF_NTSC:
        	return "QCIF_NTSC";
        case SDVR_VIDEO_STD_NONE:
            return "NONE";
    }

    return "(null)";
}

sdvr_vpp_mode_e sdvr::stringToVPPMode(const QString &str)
{
    sdvr_vpp_mode_e vppMode;

    if (str == "analytics")
        vppMode = SDVR_VPP_MODE_ANALYTICS;
    else
        vppMode = SDVR_VPP_MODE_SLATERAL;
    return vppMode;
}

const char * sdvr::vppModeToString(sdvr_vpp_mode_e vppMode)
{
    switch(vppMode)
    {
    case SDVR_VPP_MODE_SLATERAL:
        return "slateral";
    case SDVR_VPP_MODE_ANALYTICS:
        return "analytics";
    default:
        return "(null)";
    }
}
sdvr_venc_e sdvr::stringToVideoEnc(const QString &str)
{
    sdvr_venc_e enc;

    if (str == "H264")
        enc = SDVR_VIDEO_ENC_H264;
    else if (str == "JPEG")
        enc = SDVR_VIDEO_ENC_JPEG;
    else if (str == "MPEG4")
        enc = SDVR_VIDEO_ENC_MPEG4;

    else
        enc = SDVR_VIDEO_ENC_NONE;

    return enc;
}

const char *sdvr::videoEncToString(sdvr_venc_e enc)
{
    switch (enc)
    {
    case SDVR_VIDEO_ENC_H264:
        return "H264";
    case SDVR_VIDEO_ENC_JPEG:
        return "JPEG";
    case SDVR_VIDEO_ENC_MPEG4:
        return "MPEG4";

    case SDVR_VIDEO_ENC_NONE:
        return "NONE";
    }

    return "(null)";
}

sdvr_aenc_e sdvr::stringToAudioEnc(const QString &str)
{
    sdvr_aenc_e enc;

    if (str == "G711")
        enc = SDVR_AUDIO_ENC_G711;
    else if (str == "G726_16K")
        enc = SDVR_AUDIO_ENC_G726_16K;
    else if (str == "G726_24K")
        enc = SDVR_AUDIO_ENC_G726_24K;
    else if (str == "G726_32K")
        enc = SDVR_AUDIO_ENC_G726_32K;
    else if (str == "G726_48K")
        enc = SDVR_AUDIO_ENC_G726_48K;
    else
        enc = SDVR_AUDIO_ENC_NONE;

    return enc;
}

const char *sdvr::audioEncToString(sdvr_aenc_e enc)
{
    switch (enc)
    {
    case SDVR_AUDIO_ENC_G711:
        return "G711";
    case SDVR_AUDIO_ENC_G726_16K:
        return "G726_16K";
    case SDVR_AUDIO_ENC_G726_24K:
        return "G726_24K";
    case SDVR_AUDIO_ENC_G726_32K:
        return "G726_32K";
    case SDVR_AUDIO_ENC_G726_48K:
        return "G726_48K";
    case SDVR_AUDIO_ENC_NONE:
        return "NONE";
    }

    return "(null)";
}

sdvr_audio_rate_e sdvr::stringToAudioRate(const QString &str)

{
    sdvr_audio_rate_e audioRate;

    if (str == "NONE")
        audioRate = SDVR_AUDIO_RATE_NONE;
    else if (str == "8KHZ")
        audioRate = SDVR_AUDIO_RATE_8KHZ;
    else if (str == "16KHZ")
        audioRate = SDVR_AUDIO_RATE_16KHZ;
    else if (str == "32KHZ")
        audioRate = SDVR_AUDIO_RATE_32KHZ;
    else
        audioRate = SDVR_AUDIO_RATE_NONE;

    return audioRate;
}

const char *sdvr::audioRateToString(sdvr_audio_rate_e audioRate)

{
    switch (audioRate)
    {
    case SDVR_AUDIO_RATE_NONE:
        return "NONE";
    case SDVR_AUDIO_RATE_8KHZ:
        return "8KHZ";
    case SDVR_AUDIO_RATE_16KHZ:
        return "16KHZ";
    case SDVR_AUDIO_RATE_32KHZ:
        return "32KHZ";
    default:
        return "(null)";
    }
}


const char *sdvr::decimationToString(sdvr_video_res_decimation_e dec)
{
    switch (dec)
    {
    case SDVR_VIDEO_RES_DECIMATION_EQUAL:
        return "EQUAL";
    case SDVR_VIDEO_RES_DECIMATION_FOURTH:
        return "CIF";
    case SDVR_VIDEO_RES_DECIMATION_SIXTEENTH:
        return "QCIF";
    case SDVR_VIDEO_RES_DECIMATION_HALF:
        return "2CIF";
    case SDVR_VIDEO_RES_DECIMATION_DCIF:
        return "DCIF";
    case SDVR_VIDEO_RES_DECIMATION_CLASSIC_CIF:
        return "Classic CIF";
        break;
    case SDVR_VIDEO_RES_DECIMATION_CLASSIC_2CIF:
        return "Classic 2CIF";
        break;
    case SDVR_VIDEO_RES_DECIMATION_CLASSIC_4CIF:
        return "Classic 4CIF";
        break;

    case SDVR_VIDEO_RES_DECIMATION_NONE:
        return "NONE";
    }
    return "(null)";
}

sdvr_video_res_decimation_e sdvr::stringToDecimation(const QString &str)
{
	sdvr_video_res_decimation_e dec;

    if (str == "1:1" || str == "EQUAL")
        dec = SDVR_VIDEO_RES_DECIMATION_EQUAL;
    else if (str == "1:4" || str == "CIF")
        dec = SDVR_VIDEO_RES_DECIMATION_FOURTH;
    else if (str == "1:16" || str == "QCIF")
        dec = SDVR_VIDEO_RES_DECIMATION_SIXTEENTH;
    else if (str == "2CIF")
        dec = SDVR_VIDEO_RES_DECIMATION_HALF;
    else if (str == "DCIF")
        dec = SDVR_VIDEO_RES_DECIMATION_DCIF;
    else if (str == "Classic CIF")
        dec = SDVR_VIDEO_RES_DECIMATION_CLASSIC_CIF;
    else if (str == "Classic 2CIF")
        dec = SDVR_VIDEO_RES_DECIMATION_CLASSIC_2CIF;
    else if (str == "Classic 4CIF")
        dec = SDVR_VIDEO_RES_DECIMATION_CLASSIC_4CIF;
    else
        dec = SDVR_VIDEO_RES_DECIMATION_NONE;

    return dec;
}


const char *sdvr::osdLocationToString(sdvr_location_e loc)
{
    switch (loc)
    {
    case SDVR_LOC_TOP_LEFT:
        return "Top Left";
    case SDVR_LOC_BOTTOM_LEFT:
        return "Bottom Left";
    case SDVR_LOC_TOP_RIGHT:
        return "Top Right";
    case SDVR_LOC_BOTTOM_RIGHT:
        return "Bottom Right";
    case SDVR_LOC_CUSTOM:
        return "Custom";
    }

    return "(null)";
}

sdvr_location_e sdvr::stringToOsdLocation(const QString &str)
{
	sdvr_location_e loc = SDVR_LOC_TOP_LEFT;  // TODO: Should have SDVR_LOC_NONE

    if (str == "Top Left")
        loc = SDVR_LOC_TOP_LEFT;
    else if (str == "Bottom Left")
        loc = SDVR_LOC_BOTTOM_LEFT;
    else if (str == "Top Right")
        loc = SDVR_LOC_TOP_RIGHT;
    else if (str == "Bottom Right")
        loc = SDVR_LOC_BOTTOM_RIGHT;
    else if (str == "Custom")
        loc = SDVR_LOC_CUSTOM;

    return loc;
}

const char *sdvr::dtsFormatToString(sdvr_dts_style_e fmt)
{
    switch (fmt)
    {
    case SDVR_OSD_DTS_DEBUG:
        return "Debug Mode";
    case SDVR_OSD_DTS_MDY_12H:
        return "MDY 12H";
    case SDVR_OSD_DTS_DMY_12H:
        return "DMY 12H";
    case SDVR_OSD_DTS_YMD_12H:
        return "YMD 12H";
    case SDVR_OSD_DTS_MDY_24H:
        return "MDY 24H";
    case SDVR_OSD_DTS_DMY_24H:
        return "DMY 24H";
    case SDVR_OSD_DTS_YMD_24H:
        return "YMD 24H";
    case SDVR_OSD_DTS_NONE:
    	return "None";
    }

    return "(null)";
}

sdvr_dts_style_e sdvr::stringToDtsFormat(const QString &str)
{
	sdvr_dts_style_e fmt;

    if (str == "Debug Mode")
        fmt = SDVR_OSD_DTS_DEBUG;
    else if (str == "MDY 12H")
        fmt = SDVR_OSD_DTS_MDY_12H;
    else if (str == "DMY 12H")
    	fmt = SDVR_OSD_DTS_DMY_12H;
    else if (str == "YMD 12H")
    	fmt = SDVR_OSD_DTS_YMD_12H;
    else if (str == "MDY 24H")
    	fmt = SDVR_OSD_DTS_MDY_24H;
    else if (str == "DMY 24H")
    	fmt = SDVR_OSD_DTS_DMY_24H;
    else if (str == "YMD 24H")
    	fmt = SDVR_OSD_DTS_YMD_24H;
    else
    	fmt = SDVR_OSD_DTS_NONE;

    return fmt;
}

const char *sdvr::brcToString(sdvr_br_control_e brc)
{
    switch (brc)
    {
    case SDVR_BITRATE_CONTROL_CBR:
        return "CBR";
    case SDVR_BITRATE_CONTROL_VBR:
        return "VBR";
    case SDVR_BITRATE_CONTROL_CQP:
        return "CQP";
    case SDVR_BITRATE_CONTROL_CONSTANT_QUALITY:
        return "CQ";
    case SDVR_BITRATE_CONTROL_NONE:
        return "NONE";
    }

    return "(null)";
}

sdvr_br_control_e sdvr::stringToBrc(const QString &str)
{
    sdvr_br_control_e brc;

    if (str == "CBR")
        brc = SDVR_BITRATE_CONTROL_CBR;
    else if (str == "VBR")
        brc = SDVR_BITRATE_CONTROL_VBR;
    else if (str == "CQ")
        brc = SDVR_BITRATE_CONTROL_CONSTANT_QUALITY;
    else
        brc = SDVR_BITRATE_CONTROL_NONE;

    return brc;
}


sdvr_term_e sdvr::StringToCamTermination(const QString &str)
{
    sdvr_term_e term = SDVR_TERM_75OHM;

    if (str == "75_OHM")
        term = SDVR_TERM_75OHM;
    else if (str == "HIGH_IMPEDANCE")
        term = SDVR_TERM_HIGH_IMPEDANCE;

    return term;
}
    
const char *sdvr::camTerminationToString(sdvr_term_e term)
{
    switch (term)
    {
    case SDVR_TERM_75OHM:
        return "75_OHM";
    case SDVR_TERM_HIGH_IMPEDANCE:
        return "HIGH_IMPEDANCE";
    default:
        return "(null)";
    }
}


QSize sdvr::frameSize(sdvr_video_std_e vs, sdvr_video_res_decimation_e dec)
{
    int width;
    int height;

    switch (vs)
    {
        case SDVR_VIDEO_STD_D1_PAL:
            width = 720;
            height = 576;
        	break;
        case SDVR_VIDEO_STD_D1_NTSC:
            width = 720;
            height = 480;
        	break;
        case SDVR_VIDEO_STD_CIF_PAL:
            width = 352;
            height = 288;
        	break;
        case SDVR_VIDEO_STD_CIF_NTSC:
            width = 352;
            height = 240;
        	break;
        case SDVR_VIDEO_STD_2CIF_PAL:
            width = 704;
            height = 288;
        	break;
        case SDVR_VIDEO_STD_2CIF_NTSC:
            width = 704;
            height = 240;
        	break;
        case SDVR_VIDEO_STD_4CIF_PAL:
            width = 704;
            height = 576;
        	break;;
        case SDVR_VIDEO_STD_4CIF_NTSC:
            width = 704;
            height = 480;
        	break;
        case SDVR_VIDEO_STD_QCIF_PAL:
            width = 176;
            height = 144;
        	break;;
        case SDVR_VIDEO_STD_QCIF_NTSC:
            width = 176;
//            height = 120;
            height = 112;

        	break;

        case SDVR_VIDEO_STD_NONE:
        default:
            width = height = 0;
    }

    switch (dec) 
    {
    case SDVR_VIDEO_RES_DECIMATION_FOURTH:
        width  >>= 1;
        height >>= 1;
        break;
    
    case SDVR_VIDEO_RES_DECIMATION_SIXTEENTH:
        width  >>= 2;
        height >>= 2;
        break;

    case SDVR_VIDEO_RES_DECIMATION_HALF:
        // Same width as full size.
        height /= 2;
        break;

    case SDVR_VIDEO_RES_DECIMATION_DCIF:
        // NOTE: DCIF is always based on 4CIF video standard
        width = 704;

        width  = (width * 3) / 4;
        height = (height * 2) / 3;
        break;

        // NOTE: Classic sizes are absolute an not decimated.
    case SDVR_VIDEO_RES_DECIMATION_CLASSIC_CIF:
        width = 320;
        height = 240;
        break;
    case SDVR_VIDEO_RES_DECIMATION_CLASSIC_2CIF:
        width = 640;
        height = 240;
        break;
    case SDVR_VIDEO_RES_DECIMATION_CLASSIC_4CIF:
        width = 640;
        height = 480;

        break;

    default:
        break;
    }



    // Make sure the width is divisible by 16
    width /= 16;
    width *= 16;

    height /= 16;
    height *= 16;

    return QSize(width, height);
}

int sdvr::frameRate(sdvr_video_std_e vs)
{
    int fps = 30;
    switch (vs)
    {
        case SDVR_VIDEO_STD_D1_PAL:
        case SDVR_VIDEO_STD_CIF_PAL:
        case SDVR_VIDEO_STD_2CIF_PAL:
        case SDVR_VIDEO_STD_4CIF_PAL:
        case SDVR_VIDEO_STD_QCIF_PAL:
            fps = 25;
            break;
        case SDVR_VIDEO_STD_D1_NTSC:
        case SDVR_VIDEO_STD_CIF_NTSC:
        case SDVR_VIDEO_STD_2CIF_NTSC:
        case SDVR_VIDEO_STD_4CIF_NTSC:
        case SDVR_VIDEO_STD_QCIF_NTSC:
            fps = 30;
            break;
        default:
            fps = 0;
    }

    return fps;
}

sdvr_video_size_e sdvr::WidthHeightToVSize(uint width, uint height)
{
    if (width ==  720 && height == 576)
        return SDVR_VIDEO_SIZE_720x576;
    else  if (width ==  720 && height == 480)
        return SDVR_VIDEO_SIZE_720x480;
    else  if (width ==  352 && height == 288)
        return SDVR_VIDEO_SIZE_352x288;
    else  if (width ==  352 && height == 240)
        return SDVR_VIDEO_SIZE_352x240;
    else  if (width ==  704 && height == 288)
        return SDVR_VIDEO_SIZE_704x288;
    else  if (width ==  704 && height == 240)
        return SDVR_VIDEO_SIZE_704x240;
    else  if (width ==  704 && height == 576)
        return SDVR_VIDEO_SIZE_704x576;
    else  if (width ==  704 && height == 480)
        return SDVR_VIDEO_SIZE_704x480;
    else  if (width ==  528 && height == 320)
        return SDVR_VIDEO_SIZE_528x320;
    else  if (width ==  528 && height == 384)
        return SDVR_VIDEO_SIZE_528x384;
    else  if (width ==  176 && height == 144)
        return SDVR_VIDEO_SIZE_176x144;
    else  if (width ==  720 && height == 288)
        return  SDVR_VIDEO_SIZE_720x288;
    else  if (width ==  720 && height == 240)
        return  SDVR_VIDEO_SIZE_720x240;
    else  // if (width ==  176 && height == 112)
        return SDVR_VIDEO_SIZE_176x112;
}

QSize sdvr::VSizeToWidthHeight(sdvr_video_size_e vStd)
{
    uint width;
    uint height;

    switch (vStd)
    {
    case SDVR_VIDEO_SIZE_720x576:
        width  = 720;
        height = 576;
        break;
    case SDVR_VIDEO_SIZE_720x480:
        width  = 720;
        height = 480;
        break;

    case SDVR_VIDEO_SIZE_352x288:
        width  = 352;
        height = 288;
        break;

    case SDVR_VIDEO_SIZE_352x240:
        width  = 352;
        height = 240;
        break;

    case SDVR_VIDEO_SIZE_704x288:
        width  = 704;
        height = 288;
        break;

    case SDVR_VIDEO_SIZE_704x240:
        width  = 704;
        height = 240;
        break;

    case SDVR_VIDEO_SIZE_704x576:
        width  = 704;
        height = 576;
        break;

    case SDVR_VIDEO_SIZE_704x480:
        width  = 704;
        height = 480;
        break;

    case SDVR_VIDEO_SIZE_528x320:
        width  = 528;
        height = 320;
        break;

    case SDVR_VIDEO_SIZE_528x384:
        width  = 528;
        height = 384;
        break;

    case SDVR_VIDEO_SIZE_176x144:
        width  = 176;
        height = 144;
        break;

    case SDVR_VIDEO_SIZE_720x288:
        width  = 720;
        height = 288;
        break;

    case SDVR_VIDEO_SIZE_720x240:
        width  = 720;
        height = 240;
        break;


    default:  // if (width ==  176 && height == 112)
        width  = 176;
        height = 112;
        break;
    }

    return QSize(width, height);
}
/*
   This function makes sure that the given verifyFPS
   parameter is valid for the given vidoe standard.
   Also it makes sure the verifyFPS parameter stays
   at the highest FPS for NTSC so that when video standard
   is changed within the config file we keep the relative FPS.
*/
int sdvr::verifyFrameRate(sdvr_video_std_e vs, int verifyFPS)
{
    int fps = verifyFPS;
    switch (vs)
    {
        case SDVR_VIDEO_STD_D1_PAL:
        case SDVR_VIDEO_STD_CIF_PAL:
        case SDVR_VIDEO_STD_2CIF_PAL:
        case SDVR_VIDEO_STD_4CIF_PAL:
        case SDVR_VIDEO_STD_QCIF_PAL:
            if (verifyFPS > 25)
                fps = 25;
            break;
        case SDVR_VIDEO_STD_D1_NTSC:
        case SDVR_VIDEO_STD_CIF_NTSC:
        case SDVR_VIDEO_STD_2CIF_NTSC:
        case SDVR_VIDEO_STD_4CIF_NTSC:
        case SDVR_VIDEO_STD_QCIF_NTSC:
            if ((verifyFPS > 30) ||
                (g_DVR->getLastHighVSTD_FPS() == 25 && verifyFPS == 25))
                fps = 30;
            break;
        default:
            fps = 0;
            break;
    }

    return fps;
}

///////////////////////////////////////////////////////////////////////////////
// This function returns the resolution decimation for the given source
// video standard that produces the size equal or less than the target
// video standard decimated.
//
// Parameters:
//  vSTDTarget: This is usually the size current system video standard
//  decimationTarget: This is the current decimation related to the
//                    target file that the given source video standard 
//                    needs to fit in.
//  vSTDSrc: This is usually the size of encoded file video standard.
//
sdvr_video_res_decimation_e sdvr::getResDecimation(sdvr_video_std_e vSTDTarget, 
                                                   sdvr_video_res_decimation_e decimationTarget, 
                                                   sdvr_video_std_e vSTDSrc)
{
    QSize target, src;
    QSize sizeTest;

    // If the two video standard are the same, the deicmation
    // is also the same.
    if (vSTDTarget == vSTDSrc)
    {
        return decimationTarget;
    }

    // Get the width and height of source video standard using
    // the given decimation
    target = frameSize(vSTDTarget, decimationTarget);

    // Get the largest possible decimation for the given target
    // video standard
    src = frameSize(vSTDSrc, SDVR_VIDEO_RES_DECIMATION_EQUAL);

    // check if the target video standard decimated by DCIF is
    // the same as source video deicmated
    sizeTest = frameSize(vSTDSrc, SDVR_VIDEO_RES_DECIMATION_DCIF);
    if (target.height() == sizeTest.height() && target.width() == sizeTest.width())
    {
        return SDVR_VIDEO_RES_DECIMATION_DCIF;
    }


    // check if the target video standard decimated by 2CIF is
    // the same as source video deicmated
    sizeTest = frameSize(vSTDSrc, SDVR_VIDEO_RES_DECIMATION_HALF);
    if (target.height() == sizeTest.height() && target.width() == sizeTest.width())
    {
        return SDVR_VIDEO_RES_DECIMATION_HALF;
    }

    // check if the target video standard decimated by 4th is
    // the same as source video deicmated
    sizeTest = frameSize(vSTDSrc, SDVR_VIDEO_RES_DECIMATION_FOURTH);
    if (target.height() == sizeTest.height() && target.width() == sizeTest.width())
    {
        return SDVR_VIDEO_RES_DECIMATION_FOURTH;
    }

    // check if the target video standard decimated by 16th is
    // the same as source video deicmated
    sizeTest = frameSize(vSTDSrc, SDVR_VIDEO_RES_DECIMATION_SIXTEENTH);
    if (target.height() == sizeTest.height() && target.width() == sizeTest.width())
    {
        return SDVR_VIDEO_RES_DECIMATION_SIXTEENTH;
    }

    // otherwise use the largest decimation
    return SDVR_VIDEO_RES_DECIMATION_EQUAL;
}

///////////////////////////////////////////////////////////////////
// This function calculates the correct aspect ration width size
// of a raw vide display according for the current video standard.
//
int sdvr::getHMOAspectRatio(int width, sdvr_video_std_e vs)
{
    // Re-adjust for the correct display aspect ration. 
    // We should display the current raw video frame into 
    // 4x3 aspect ration.

    switch (vs)
    {
        case SDVR_VIDEO_STD_D1_PAL:
        case SDVR_VIDEO_STD_CIF_PAL:
        case SDVR_VIDEO_STD_2CIF_PAL:
        case SDVR_VIDEO_STD_4CIF_PAL:
        case SDVR_VIDEO_STD_QCIF_PAL:
            width = width * 16 / 15;
            break;
        case SDVR_VIDEO_STD_D1_NTSC:
        case SDVR_VIDEO_STD_CIF_NTSC:
        case SDVR_VIDEO_STD_2CIF_NTSC:
        case SDVR_VIDEO_STD_4CIF_NTSC:
        case SDVR_VIDEO_STD_QCIF_NTSC:
        default:
            width = width * 8 / 9;
        break;
    }

    return width;
}

const char *sdvr::errorMessage(int code)
{
    const char * msg;
    switch (code)
    {
    case SDVR_ERR_NONE:
        msg = "Success";
        break;
    case SDVR_ERR_NO_DVR_BOARD:
        msg = "Board detection failed";
        break;
    case SDVR_ERR_WRONG_DRIVER_VERSION:
        msg = "Driver is not compatible with the version of SDK";
        break;
    default:
        msg = "Unknown error";
    }

    return msg;
}

bool sdvr::getVideoSizeFromFile(sdvr_video_size_e *video_size, const QString &fileName)
 {
    mp4PlayerFile_t mp4File;
    mp4_info_t info;
    mp4_track_info_t track;
    QByteArray baFile(fileName.toAscii());
    bool rc = false;

    memset(&info, 0, sizeof(mp4_info_t));
    info.file_name = baFile.data();
    info.mux_mode = MP4_READ_INFO;
    info.file_mode = MODE_MOV;
 //   info.log = fopen("c:/dec.log", "w");

    mp4File.FileHandle = mp4mux_open(&info);
    if (mp4File.FileHandle == 0) {
        return false;
    }
    for (int i=0; i<info.nb_tracks; i++) 
    {
        if (!mp4mux_get_track_info(mp4File.FileHandle, i, &track)) 
        {
            if (track.type == MP4_TRACK_VIDEO) 
            {
                if  (track.codec_id ==  CODEC_ID_H264 ||
                    track.codec_id ==  CODEC_ID_MPEG4)
                {
                        *video_size = sdvr::WidthHeightToVSize(track.width,track.height);
                        rc = true;
                        break;
                }
            }
        }
    }
 //   fclose(info.log);
    mp4mux_close(mp4File.FileHandle);
    return rc;
 }
/* This function checks if the given version major.minor.revision.build is grater
   than or equal to the given version majorChk.minorChk.revisionChk.buildChk
*/
bool sdvr::isMinVersion(sx_uint8 major, sx_uint8 minor, sx_uint8 revision, sx_uint8 build,
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


