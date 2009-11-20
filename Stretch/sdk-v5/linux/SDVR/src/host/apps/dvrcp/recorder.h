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

#ifndef RECORDER_H
#define RECORDER_H

#include <QThread>
#include <QMutex>
#include <QSemaphore>
#include <QQueue>
#include <QPair>
#include "recorder.h"

#include "sdvr_sdk.h"
#include "avi_api.h"
#include "../../src/codecs/avmux/mp4/mp4mux.h"
#include "camera.h"

typedef struct
{
	sx_int32 vtrack;            // Video track handle within mp4 file
    sx_int32 atrack;            // Audio track handle within mp4 file
    mp4_handle FileHandle;      // The file handle to the mp4 file.
    sx_int64 dts;
} mp4File_t;


class Recorder : public QThread
{
	Q_OBJECT

public:
    struct stat_t
    {
        stat_t(Camera *cam, uint subEnc, FILE *fp, aviFile_t avi, mp4File_t mp4)
        {
            camera = cam;
            m_subEnc = subEnc;
            m_fpElementary = fp;

            totalFramesWrtn = bytes = frames = 0;
            firstTimeStamp = lastTimeStamp = 0;

			bIsStopped = false;

            aviFile.fp = avi.fp;
            aviFile.frameLen = avi.frameLen;
            // TODO: For now we are limiting each avi file to 
            //       1 Hr worth of frames.
            aviFile.maxFramesAllowed = avi.maxFramesAllowed;
            aviFile.totalFramesWrtn = avi.totalFramesWrtn;

            mp4File.vtrack = mp4.vtrack;
            mp4File.atrack = mp4.atrack;
            mp4File.FileHandle = mp4.FileHandle;
            mp4File.dts = 0;

            bKeyFrameOnly = false;

        }

        Camera *camera;
        uint m_subEnc;            // The subEncoder that is running to record;
        aviFile_t aviFile;      // The AVI file wrapper for mjpeg files.
        FILE *m_fpElementary;      // File handles for elemantry stream
        mp4File_t mp4File;      // The mp4 file wrapper for A/V files.
        uint bytes;             // number of bytes to be used for
                                // statistics calculation
        uint frames;            // number of frames rcvd to be used for
                                // statistics calculation
        sx_uint64 firstTimeStamp;
        sx_uint64 lastTimeStamp;
        bool bIsStopped;	    // A flag to indicate if the recording is intruppted
        unsigned int totalFramesWrtn;    // actual number of frames recorded 

        bool bKeyFrameOnly;     // A flag to indicate if we have to start
                                // recording on the key frame.
    };

    typedef QList<stat_t> StatList;
	typedef QPair<sdvr_chan_handle_t, sdvr_av_buffer_t *> QueueItem;

    Recorder();
    ~Recorder();

    void enqueue(sdvr_chan_handle_t handle, sdvr_av_buffer_t *buf);
    bool startRecording(uint subEnc, Camera *cam, const QString &fileName);
    bool stopRecording(uint subEnc, Camera *cam);

    void enableStats(uint subEnc);
    void disableStats(uint subEnc);
    StatList takeStats(uint subEnc);
    void terminate();
    void flushFrameBufQueue();

signals:
    void interruptRecording(uint subEnc, int nCamId);

protected:
    virtual void run();

private:
    void writeBuffer(sdvr_chan_handle_t handle, sdvr_av_buffer_t *buf);
    void writeEncAudioFrames(sdvr_chan_handle_t handle, sdvr_av_buffer_t *buf);
    void writeEncVideoFrames(sdvr_chan_handle_t handle, sdvr_av_buffer_t *buf);
    bool isKeyFrame(sdvr_frame_type_e frame_type);
    bool isPictureInfoFrame(sdvr_frame_type_e frame_type);
    bool doStopRecording(StatList::iterator &i, Camera *cam, uint subEnc);
    bool doOpenRecording(Camera *cam, uint subEnc, const QString &fileName,
                               mp4File_t *mp4File,
                               aviFile_t *aviFile, FILE **fpElementary);
    sx_int32 doPutMP4Packet(StatList::iterator &i, sdvr_av_buffer_t *buf);


    QQueue<QueueItem> m_bufferQueue;
    QMutex m_queueMutex;
    QSemaphore m_semNext;

    StatList m_statTable[MAX_SUB_ENCODERS];
    QMutex m_statTableMutex;

    bool m_statsEnabled[MAX_SUB_ENCODERS];
    bool m_exit;
};

#endif
