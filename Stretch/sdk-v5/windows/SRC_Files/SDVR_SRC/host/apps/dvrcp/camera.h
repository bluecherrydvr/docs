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

#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QVector>
#include <QDomElement>
#include <QSize>
#include "sdvr_sdk.h"

// Un-comment the following to support writing as video elementary stream
// in this mode there is no audio recording.
//#define elementary_stream

// Un-comment the following to save a/v elementary stream mux together
// with the PCI header while recording.
// NOTE: elementary_stream must also be defined.
//#define rawav_packet 

#ifdef rawav_packet
typedef struct {
    unsigned int sync_word;
    unsigned short header_size;
    unsigned char stream_id;
    unsigned char version;
    unsigned int flags;
    unsigned int payload_size;
    __int64 timestamp;
} RawHeader;
#define BYTESWAP_16(x) ((((x) & 0xff) << 8) + (((x) & 0xff00) >> 8))
#define BYTESWAP_32(x) ((BYTESWAP_16(x) << 16) + BYTESWAP_16((x) >> 16))
#endif

static const int MAX_NLAYERS = 4;    // Maximum number of layers

class CRegion
{
public:
	static const int BLOCK_SIZE = 16;    // Metablock size

	CRegion();
    CRegion(CRegion &other);
	CRegion(sdvr_video_std_e std, int nlayers);

	bool init(sdvr_video_std_e std, int nlayers);
    bool isInitialized() { return videoStd() != SDVR_VIDEO_STD_NONE; }
	void clear();

	sdvr_video_std_e videoStd() const { return m_videoStd; }
	int frameWidth() const { return m_frameSize.width(); }
	int frameHeight() const { return m_frameSize.height(); }
	int rows() const { return m_rows; }
	int cols() const { return m_cols; }
	int layers() const { return m_layers; }

	void setBlock(int row, int col, int layer, bool enabled);
	bool block(int row, int col, int layer) const;
    bool migrate(sdvr_video_std_e otherVideoStd);
    char *data() { return m_array.data(); }
	bool isEmpty() const;

    CRegion & operator=(CRegion & other);

	bool fromElement(QDomElement e);
    QDomElement toElement(QDomDocument &doc);

private:
	sdvr_video_std_e m_videoStd;
	int m_rows;
	int m_cols;
	int m_layers;
	QByteArray m_array;
	QSize m_frameSize;
};

// Number of sub-channel encoder supported for each encoder channel.

#define MAX_SUB_ENCODERS 2

class Camera
{
public:
    Camera();
    ~Camera();
    Camera(Camera &c, bool bDuplicate = false);

    void setId(int id) { m_id = id; }
    int id() const { return m_id; }

    void setName(const QString &name) { m_name = name; }
    const QString &name() const { return m_name; }

    bool attach(int boardIndex, int input); // Use DVRSystem::attach
    bool detach();
    bool isAttached() const;

    bool openChannel();
    bool closeChannel();
    bool isChannelOpen() const { return m_handle != 0; }

    int boardIndex() const { return m_boardIndex; }
    int input() const { return m_input; }
    sdvr_chan_handle_t handle() const { return m_handle; }

    void setVideoFormat(uint subEnc, sdvr_venc_e format) {m_videoFormat[subEnc] = format; }
    sdvr_venc_e videoFormat(uint subEnc) const { return m_videoFormat[subEnc];}

    void setAudioFormat(sdvr_aenc_e format) { m_audioFormat = format; }
    sdvr_aenc_e audioFormat() const { return m_audioFormat; }

    void setVPPMode(sdvr_vpp_mode_e vppMode)  {m_vppMode = vppMode;}
    sdvr_vpp_mode_e getVPPMode() {return m_vppMode;}

    uint getMotionValueFreq() { return m_motionValueFreq; }
    void setMotionValueFreq(uint motionValueFreq) {m_motionValueFreq = motionValueFreq; }
    sdvr_err_e applyMotionFrequency();


    // OSD methods
    bool isOsdEnabled() const { return m_osdEnabled; }
    void setOsdEnabled(bool enabled) { m_osdEnabled = enabled; }

    bool isOsdShow(int osdItem) const { return m_osdShow[osdItem]; }
    void setOsdShow(int osdItem, bool bShow) { m_osdShow[osdItem] = bShow; }

    QString osdText(int osdItem) const { return m_osdText[osdItem]; }
    void setOsdText(int osdItem, const QString &text) { m_osdText[osdItem] = text; }

    sdvr_location_e osdLocation(int osdItem) const { return m_osdLoc[osdItem]; }
    void setOsdLocation(int osdItem, sdvr_location_e loc) { m_osdLoc[osdItem] = loc; }

    void getOsdCustomLoc(int osdItem, sx_uint16 *x, sx_uint16 *y) 
    {
        *x = m_osdLocx[osdItem];
        *y = m_osdLocy[osdItem];
    }

    void setOsdCustomLoc(int osdItem, sx_uint16 x, sx_uint16 y) 
    {
        m_osdLocx[osdItem] = x;
        m_osdLocy[osdItem] = y;
    }

    sx_uint8 getOsdTranslucent(int osdItem) { return m_osdTranslucent[osdItem];}
    void setOsdTranslucent(int osdItem, sx_uint8 translucent) { m_osdTranslucent[osdItem] = translucent;}

    bool isOsdDtsEnabled(int osdItem) const { return m_osdDtsEnabled[osdItem]; }
    void setOsdDtsEnabled(int osdItem, bool enabled) { m_osdDtsEnabled[osdItem] = enabled; }

    sdvr_dts_style_e osdDtsFormat(int osdItem) const { return m_osdDtsFormat[osdItem]; }
    void setOsdDtsFormat(int osdItem, sdvr_dts_style_e fmt) { m_osdDtsFormat[osdItem] = fmt; }

    sdvr_err_e applyAllParameters();

    sdvr_err_e applyOsd();

    // Raw Video methods
    void setYUVFrameRate(sx_uint8 rate);
    sx_uint8 getYUVFrameRate(); 

    sdvr_err_e applyYUVFrameRate();


    // Encoder methods
    void setDecimation(uint subEnc, sdvr_video_res_decimation_e d) { m_decimation[subEnc] = d; }
    sdvr_video_res_decimation_e decimation(uint subEnc) const { return m_decimation[subEnc]; }

    void setFrameRate(uint subEnc,int rate) { m_frameRate[subEnc] = rate; }
    int frameRate(uint subEnc) const { return m_frameRate[subEnc]; }

    uint h264GopSize(uint subEnc) const { return m_h264GopSize[subEnc]; }
    void setH264GopSize(uint subEnc,int size) { m_h264GopSize[subEnc] = size; }

    sdvr_br_control_e h264BitrateControl(uint subEnc) const { return m_h264BitrateControl[subEnc]; }
    void setH264BitrateControl(uint subEnc,sdvr_br_control_e brc) { m_h264BitrateControl[subEnc] = brc; }

    uint h264AvgBitrate(uint subEnc) const { return m_h264AvgBitrate[subEnc]; }
    void setH264AvgBitrate(uint subEnc,int avgRate) { m_h264AvgBitrate[subEnc] = avgRate; }

    uint h264MaxBitrate(uint subEnc) const { return m_h264MaxBitrate[subEnc]; }
    void setH264MaxBitrate(uint subEnc,int maxRate) { m_h264MaxBitrate[subEnc] = maxRate; }

    uint h264Quality(uint subEnc) const { return m_h264Quality[subEnc]; }
    void setH264Quality(uint subEnc,int quality) { m_h264Quality[subEnc] = quality; }

    bool jpegIsImageStyle(uint subEnc) const;
    void setJpegImageStyle(uint subEnc,bool imageStyle) { m_jpegImageStyle[subEnc] = imageStyle; }

    uint jpegQuality(uint subEnc) const { return m_jpegQuality[subEnc]; }
    void setJpegQuality(uint subEnc,int quality) { m_jpegQuality[subEnc] = quality; }

    uint MPEG4GopSize(uint subEnc) const { return m_MPEG4GopSize[subEnc]; }
    void setMPEG4GopSize(uint subEnc,int size) { m_MPEG4GopSize[subEnc] = size; }

    sdvr_br_control_e MPEG4BitrateControl(uint subEnc) const { return m_MPEG4BitrateControl[subEnc]; }
    void setMPEG4BitrateControl(uint subEnc,sdvr_br_control_e brc) { m_MPEG4BitrateControl[subEnc] = brc; }

    uint MPEG4AvgBitrate(uint subEnc) const { return m_MPEG4AvgBitrate[subEnc]; }
    void setMPEG4AvgBitrate(uint subEnc,int avgRate) { m_MPEG4AvgBitrate[subEnc] = avgRate; }

    uint MPEG4MaxBitrate(uint subEnc) const { return m_MPEG4MaxBitrate[subEnc]; }
    void setMPEG4MaxBitrate(uint subEnc,int maxRate) { m_MPEG4MaxBitrate[subEnc] = maxRate; }

    uint MPEG4Quality(uint subEnc) const { return m_MPEG4Quality[subEnc]; }
    void setMPEG4Quality(uint subEnc,int quality) { m_MPEG4Quality[subEnc] = quality; }


    sdvr_err_e applyVideoEncoding(uint subEnc);


    // On alarm H.264 encoder configuration access methods
    bool isAdjustEncodingOnAlarmEnabled(uint subEnc) const { return m_adjustEncodingOnAlarmEnabled[subEnc]; }
    void setAdjustEncodingOnAlarmEnabled(uint subEnc,bool enabled) { m_adjustEncodingOnAlarmEnabled[subEnc] = enabled; }

    int onAlarmFrameRate(uint subEnc) const { return m_onAlarmFrameRate[subEnc]; }
    void setOnAlarmFrameRate(uint subEnc,int rate) { m_onAlarmFrameRate[subEnc] = rate; }

    uint onAlarmMinOnTime(uint subEnc) const { return m_onAlarmMinOnTime[subEnc]; }
    void setOnAlarmMinOnTime(uint subEnc,uint time) { m_onAlarmMinOnTime[subEnc] = time; }

    uint onAlarmMinOffTime(uint subEnc) const { return m_onAlarmMinOffTime[subEnc]; }
    void setOnAlarmMinOffTime(uint subEnc,uint time) { m_onAlarmMinOffTime[subEnc] = time; }

    uint onAlarmH264GopSize(uint subEnc) const { return m_onAlarmH264GopSize[subEnc]; }
    void setOnAlarmH264GopSize(uint subEnc,int size) { m_onAlarmH264GopSize[subEnc] = size; }

    sdvr_br_control_e onAlarmH264BitrateControl(uint subEnc) const { return m_onAlarmH264BitrateControl[subEnc]; }
    void setOnAlarmH264BitrateControl(uint subEnc,sdvr_br_control_e brc) { m_onAlarmH264BitrateControl[subEnc] = brc; }

    uint onAlarmH264AvgBitrate(uint subEnc) const { return m_onAlarmH264AvgBitrate[subEnc]; }
    void setOnAlarmH264AvgBitrate(uint subEnc,int avgRate) { m_onAlarmH264AvgBitrate[subEnc] = avgRate; }

    uint onAlarmH264MaxBitrate(uint subEnc) const { return m_onAlarmH264MaxBitrate[subEnc]; }
    void setOnAlarmH264MaxBitrate(uint subEnc,int maxRate) { m_onAlarmH264MaxBitrate[subEnc] = maxRate; }

    uint onAlarmH264Quality(uint subEnc) const { return m_onAlarmH264Quality[subEnc]; }
    void setOnAlarmH264Quality(uint subEnc,int quality) { m_onAlarmH264Quality[subEnc] = quality; }

    uint getOnAlarmJpegQuality(uint subEnc) const { return m_onAlarmJpegQuality[subEnc]; }
    void setonAlarmJpegQuality(uint subEnc,int quality) { m_onAlarmJpegQuality[subEnc] = quality; }

    uint onAlarmMPEG4GopSize(uint subEnc) const { return m_onAlarmMPEG4GopSize[subEnc]; }
    void setOnAlarmMPEG4GopSize(uint subEnc,int size) { m_onAlarmMPEG4GopSize[subEnc] = size; }

    sdvr_br_control_e onAlarmMPEG4BitrateControl(uint subEnc) const { return m_onAlarmMPEG4BitrateControl[subEnc]; }
    void setOnAlarmMPEG4BitrateControl(uint subEnc,sdvr_br_control_e brc) { m_onAlarmMPEG4BitrateControl[subEnc] = brc; }

    uint onAlarmMPEG4AvgBitrate(uint subEnc) const { return m_onAlarmMPEG4AvgBitrate[subEnc]; }
    void setOnAlarmMPEG4AvgBitrate(uint subEnc,int avgRate) { m_onAlarmMPEG4AvgBitrate[subEnc] = avgRate; }

    uint onAlarmMPEG4MaxBitrate(uint subEnc) const { return m_onAlarmMPEG4MaxBitrate[subEnc]; }
    void setOnAlarmMPEG4MaxBitrate(uint subEnc,int maxRate) { m_onAlarmMPEG4MaxBitrate[subEnc] = maxRate; }

    uint onAlarmMPEG4Quality(uint subEnc) const { return m_onAlarmMPEG4Quality[subEnc]; }
    void setOnAlarmMPEG4Quality(uint subEnc,int quality) { m_onAlarmMPEG4Quality[subEnc] = quality; }

    sdvr_err_e applyOnAlarmVideoEncoding(uint subEnc) ;


    // Motion detection methods
    bool isMDEnabled() const  { return m_MDEnabled; }
    void setMDEnabled(bool enabled) { m_MDEnabled = enabled; }

    int MDThreshold(int Nth) const { return m_MDThreshold[Nth]; }
    void setMDThreshold(int Nth, int threshold) { m_MDThreshold[Nth] = threshold; }

    CRegion &MDRegion() { return m_MDRegion; }
    void setMDRegion(CRegion &reg) { m_MDRegion = reg; }

    bool isMDTriggered() const  { return m_MDTriggered; }
    void setMDTriggered(bool state) { m_MDTriggered = state; }

    sdvr_err_e applyMD();
    sdvr_err_e applyMDRegions();


    // Blind detection methods
    bool isBDEnabled() const  { return m_BDEnabled; }
    void setBDEnabled(bool enabled) { m_BDEnabled = enabled; }

    int BDThreshold() const { return m_BDThreshold; }
    void setBDThreshold(int threshold) { m_BDThreshold = threshold; }

    CRegion &BDRegion() { return m_BDRegion; }
    void setBDRegion(CRegion &reg) { m_BDRegion = reg; }

    bool isBDTriggered() const  { return m_BDTriggered; }
    void setBDTriggered(bool state) { m_BDTriggered = state; }

    sdvr_err_e applyBD();
    sdvr_err_e applyBDRegions();

    // Night detection methods
    bool isNDEnabled() const  { return m_NDEnabled; }
    void setNDEnabled(bool enabled) { m_NDEnabled = enabled; }

    int NDThreshold() const { return m_NDThreshold; }
    void setNDThreshold(int threshold) { m_NDThreshold = threshold; }

    bool isNDTriggered() const  { return m_NDTriggered; }
    void setNDTriggered(bool state) { m_NDTriggered = state; }

    sdvr_err_e applyND() const;

    // Privacy Regions methods
    bool isPREnabled() const  { return m_PREnabled; }
    void setPREnabled(bool enabled) { m_PREnabled = enabled; }

    CRegion &PRRegion() { return m_PRRegion; }
    void setPRRegion(CRegion &reg) { m_PRRegion = reg; }

    sdvr_err_e applyPR();
    sdvr_err_e applyPRRegions();

    // Video loss detection
    bool isVLEnabled() const { return m_VLEnabled; }
    void setVLEnabled(bool enabled) { m_VLEnabled = enabled; }

    bool isVLTriggered() const  { return m_VLTriggered; }
    void setVLTriggered(bool state) { m_VLTriggered = state; }

    bool isVDTriggered() const  { return m_VDTriggered; }
    void setVDTriggered(bool state) { m_VDTriggered = state; }

    sdvr_err_e applyVL() const;

    // Video-in parameters
    sx_uint8 getVParamHue() {return m_vParamHue;}
    void setVParamHue(sx_uint8 nHue) { m_vParamHue = nHue;}

    sx_uint8 getVParamContrast() {return m_vParamContrast; }
    void setVParamContrast(sx_uint8 nContrast) {m_vParamContrast = nContrast; }

    sx_uint8 getVParamSaturation() {return m_vParamSaturation;}
    void setVParamSaturation(sx_uint8 nSaturation) {m_vParamSaturation = nSaturation;}

    sx_uint8 getVParamBrightness() {return m_vParamBrightness; }
    void setVParamBrightness(sx_uint8 nBrightness) {m_vParamBrightness = nBrightness; }

    sx_uint8 getVParamSharpness() {return m_vParamSharpness;}
    void setVParamSharpness(sx_uint8 nSharpness) {m_vParamSharpness = nSharpness;}

    void setVParamsDeinterlacing(sx_bool bEnable) {m_bVParamDeinterlacing = bEnable;}
    bool isVParamsDeinterlacing() {return m_bVParamDeinterlacing;}

    void setVParamNoiseReduction(sx_bool bEnable) {m_bVParamNoiseReduction = bEnable;}
    bool isVParamNoiseReduction() {return m_bVParamNoiseReduction; }

    sx_uint8 getVParamGainMode() {return m_nVParamGainMode; }
    void setVParamGainMode(sx_uint32 nGainMode) {m_nVParamGainMode = nGainMode; }

    sdvr_term_e getVParamCamTermination() {return m_VParamCamTermination; }
    void setVParamCamTermination(sdvr_term_e value) {m_VParamCamTermination = value; }

    sdvr_err_e applyVideoInHCSBSSettings(int nHue, int nContrast, 
        int nSaturation, int nBrightness, int nSharpness);
    sdvr_err_e applyVideoInSettings();


    sdvr_err_e setRawVideo(sdvr_video_res_decimation_e dec, bool enabled);
    bool isRawVideoEnabled() const { return m_rawVideoEnabled; }

    sdvr_err_e setRawAudio(bool enabled);
    bool isRawAudioEnabled();

    sdvr_err_e setSmoGrid(int x, int y, sdvr_video_res_decimation_e dec,
                         int dwellTime, bool enabled);
    bool isSmoGridEnabled() const { return m_smoGridEnabled; }

    bool isAVStreaming();

    void setLastStatTimeStamp (uint subEnc, sx_uint64 timeStamp) {m_lastStatTimeStamp[subEnc] = timeStamp;}
    sx_uint64 getLastStatTimeStamp(uint subEnc) const {return m_lastStatTimeStamp[subEnc];}

    void setFirstStatTimeStamp (uint subEnc, sx_uint64 timeStamp) {m_firstStatTimeStamp[subEnc] = timeStamp;}
    sx_uint64 getFirstStatTimeStamp(uint subEnc) const {return m_firstStatTimeStamp[subEnc];}

    void setLastAudioTimeStamp (uint timeStamp) {m_lastAudioTimeStamp = timeStamp;}
    uint getLastAudioTimeStamp() const {return m_lastAudioTimeStamp;}

    void setLastAudioSize (uint size) {m_lastAudioSize = size;}
    uint getLastAudioSize() const {return m_lastAudioSize;}

    void setAccuAudioDelta ( int timeStamp) {m_AccuAudioDelta += timeStamp;}
    int getAccuAduioDelta() const {return m_AccuAudioDelta;}
    
    void incDropFrameCount(uint subEnc, int nInc) {m_dropFrameCount[subEnc] += nInc;}
    uint getDropFrameCount(uint subEnc) {return m_dropFrameCount[subEnc];}

    bool startRecording(uint  subEnc);
    bool stopRecording(uint subEnc, bool bInterrupted = false);
    bool isRecording(uint subEnc) const { return m_recording[subEnc]; };
    bool canPlayBack(uint subEnc) const;
    QString recordFilePath(uint subEnc, bool bAppendTimeDate, bool bYUVFileFormat = false) const;


    void savePictureInfoFrame(sdvr_av_buffer_t *buf);
    sdvr_av_buffer_t *getPictureInfoFrame(sdvr_frame_type_e frame_type);

    bool fromElement(QDomElement e);
    QDomElement toElement(QDomDocument &doc);

    Camera & operator=( Camera & other);

private:
	int m_id;
    int m_boardIndex;
    int m_input;
    sdvr_chan_handle_t m_handle;

    sdvr_venc_e m_videoFormat[MAX_SUB_ENCODERS];
    sdvr_aenc_e m_audioFormat;

    sdvr_vpp_mode_e m_vppMode;

    uint m_motionValueFreq;

    QString m_name;

    bool m_osdEnabled;
    bool m_osdShow[SDVR_MAX_OSD];
    QString m_osdText[SDVR_MAX_OSD];
    sdvr_location_e m_osdLoc[SDVR_MAX_OSD];
    bool m_osdDtsEnabled[SDVR_MAX_OSD];
    sdvr_dts_style_e m_osdDtsFormat[SDVR_MAX_OSD];
    sx_uint16 m_osdLocx[SDVR_MAX_OSD];
    sx_uint16 m_osdLocy[SDVR_MAX_OSD];
    sx_uint8 m_osdTranslucent[SDVR_MAX_OSD];

    // Raw Video parameters
    sx_uint8 m_yuvFrameRate;

    ///////////////////////////////////////////////////////////////////////
    //                          Encoder parameters
    ///////////////////////////////////////////////////////////////////////
    int m_frameRate[MAX_SUB_ENCODERS];
    sdvr_video_res_decimation_e m_decimation[MAX_SUB_ENCODERS];

    // H264 parameters
    uint m_h264GopSize[MAX_SUB_ENCODERS]; // GOP size for the h.264 encoder. The default is 30.
    sdvr_br_control_e m_h264BitrateControl[MAX_SUB_ENCODERS];

    uint m_h264AvgBitrate[MAX_SUB_ENCODERS]; // The average bit rate if cbr or vbr is selected.
                           // In Kbits per sec. The default is 2000.

    uint m_h264MaxBitrate[MAX_SUB_ENCODERS]; // The maximum bit rate if vbr is selected.
                           // In Kbits per sec. The default is 2000.

    uint m_h264Quality[MAX_SUB_ENCODERS];    // A number between 0 - 100 to control the quality
                           // to be maitained while the bitrate control is set
                           // to constant quality.

    // MJPEG parameters
    bool m_jpegImageStyle[MAX_SUB_ENCODERS]; // 0 - motion JPEG, 1 - image style JPEG
    uint m_jpegQuality[MAX_SUB_ENCODERS];    // A number in the range of 10 - 300 to control the
                           // quality of the compression.

    // MPEG4 parameters
    uint m_MPEG4GopSize[MAX_SUB_ENCODERS]; // GOP size for the MPEG4 encoder. The default is 30.
    sdvr_br_control_e m_MPEG4BitrateControl[MAX_SUB_ENCODERS];

    uint m_MPEG4AvgBitrate[MAX_SUB_ENCODERS]; // The average bit rate if cbr or vbr is selected.
                           // In Kbits per sec. The default is 2000.

    uint m_MPEG4MaxBitrate[MAX_SUB_ENCODERS]; // The maximum bit rate if vbr is selected.
                           // In Kbits per sec. The default is 2000.

    uint m_MPEG4Quality[MAX_SUB_ENCODERS];    // A number between 0 - 100 to control the quality
                           // to be maitained while the bitrate control is set
                           // to constant quality.

    ///////////////////////////////////////////////////////////////////////
    //                      On alarm encoder parameters
    ///////////////////////////////////////////////////////////////////////

    bool m_adjustEncodingOnAlarmEnabled[MAX_SUB_ENCODERS];
    int m_onAlarmFrameRate[MAX_SUB_ENCODERS];
    uint m_onAlarmMinOnTime[MAX_SUB_ENCODERS];
    uint m_onAlarmMinOffTime[MAX_SUB_ENCODERS];

    // H.264 parameters
    uint m_onAlarmH264GopSize[MAX_SUB_ENCODERS];
    sdvr_br_control_e m_onAlarmH264BitrateControl[MAX_SUB_ENCODERS];
    uint m_onAlarmH264AvgBitrate[MAX_SUB_ENCODERS];
    uint m_onAlarmH264MaxBitrate[MAX_SUB_ENCODERS];
    uint m_onAlarmH264Quality[MAX_SUB_ENCODERS];

    // MJPEG parameters
    uint m_onAlarmJpegQuality[MAX_SUB_ENCODERS]; // A number in the range of 10 - 300 to control the
                               // quality of the compression.

    uint m_onAlarmMPEG4GopSize[MAX_SUB_ENCODERS];
    sdvr_br_control_e m_onAlarmMPEG4BitrateControl[MAX_SUB_ENCODERS];
    uint m_onAlarmMPEG4AvgBitrate[MAX_SUB_ENCODERS];
    uint m_onAlarmMPEG4MaxBitrate[MAX_SUB_ENCODERS];
    uint m_onAlarmMPEG4Quality[MAX_SUB_ENCODERS];

    //////////////////////////////////////////////////////////////////////////
    bool m_MDEnabled;
    int m_MDThreshold[MAX_NLAYERS];
    CRegion m_MDRegion;

    bool m_BDEnabled;
    int m_BDThreshold;
    CRegion m_BDRegion;

    bool m_PREnabled;
    CRegion m_PRRegion;

    bool m_NDEnabled;
    int m_NDThreshold;

    bool m_VLEnabled;

    bool m_MDTriggered;
    bool m_BDTriggered;
    bool m_NDTriggered;
    bool m_VLTriggered;
    bool m_VDTriggered;

    sx_uint8 m_vParamHue;
    sx_uint8 m_vParamContrast;
    sx_uint8 m_vParamSaturation;
    sx_uint8 m_vParamBrightness;
    sx_uint8 m_vParamSharpness;

    bool m_bVParamDeinterlacing;
    bool m_bVParamNoiseReduction;
    sx_uint8 m_nVParamGainMode;
    sdvr_term_e m_VParamCamTermination;

    bool m_enableWatermark;
    sdvr_location_e m_watermarkLoc;
    QString m_watermarkText;

    bool m_rawVideoEnabled;
    bool m_rawAudioEnabled; 

    bool m_smoGridEnabled;
    bool m_recording[MAX_SUB_ENCODERS];

    // This member varaiable is used to check the drop 
    // frames by checking the time interval.
    sx_uint64 m_firstStatTimeStamp[MAX_SUB_ENCODERS];
    sx_uint64 m_lastStatTimeStamp[MAX_SUB_ENCODERS];

    sx_uint32 m_lastAudioTimeStamp;
    sx_uint32 m_lastAudioSize;

    sx_int32 m_AccuAudioDelta;

    sx_uint32 m_dropFrameCount[MAX_SUB_ENCODERS];

    QByteArray m_SPScache;
    QByteArray m_PPScache;
    QByteArray m_VOLcache;
};

#endif /* CAMERA_H */
