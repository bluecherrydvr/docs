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

#ifndef DECODER_H
#define DECODER_H

#include <QThread>
#include "sdvr_sdk.h"
#include "../../src/codecs/avmux/mp4/mp4mux.h"

class CPlayer;

typedef struct
{
    sx_int32 vtrack;        // Video track handle within mp4 file
    sx_int32 atrack;        // Audio track handle within mp4 file
    mp4_handle FileHandle;  // The file handle to the mp4 file.
    sx_int64 dts;
} mp4PlayerFile_t;

class CDecoder : public QThread
{
	Q_OBJECT

public:
    CDecoder(CPlayer *player);

    bool startDecoding(const QString &fileName,  bool bContinous = false);
    bool stopDecoding();

    void resetStats() { m_statBytes = m_statFrames = 0; }
    uint statBytes()  { return m_statBytes; }
    uint statFrames() { return m_statFrames; }
    uint statMissedFrames() { return m_statMissedFrames; }

protected:
    virtual void run();

private:
    bool openAVFile();
    bool rewind();

    sx_int32  getNextAVFrame(sdvr_av_buffer_t *buf);
    sx_uint64 getAVFrameTS(sdvr_av_buffer_t *av_frame);

    CPlayer *m_player;
    sdvr_av_buffer_t *m_buf;
    uint m_statBytes;
    uint m_statFrames;
    uint m_statMissedFrames;
    bool m_continuous;
    bool m_stopRequested;

    sdvr_firmware_ver_t m_FWVersion;
    mp4PlayerFile_t m_mp4File;
    QByteArray m_mp4FileName;
};

#endif
