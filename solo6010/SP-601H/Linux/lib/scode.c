#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/vfs.h>            /* statfs() */
#include <sys/stat.h>           /* stat() */
#include <dirent.h>             /* opendir() */
#include <unistd.h>             /* unlink(), getcwd() */
#include <semaphore.h>
#include <pthread.h>

#include "DataType.h"
#include "solo6010sdk.h"
#include "scode.h"
#include "tmserverpu.h"

static char enc_device_path[256] = "/dev/solo6010_enc0";
static char disp_device_path[256] = "/dev/solo6010_disp0";
static char dec_device_path[256] = "/dev/solo6010_dec0";

//int enc_fd = -1;
int disp_fd = -1;
int dec_fd = -1;

struct ENC_TBL enc_tbl;

SOLO6010_DISPLAY *disp;

static int display_mode = MUX_MODE_16CH;

int GetLastErrorNum ()
{
    return 0;
}

int InitDSPs ()
{
    int channel;

//	enc_fd = open(enc_device_path, O_RDWR);
//	disp_fd = open(disp_device_path, O_RDWR);
	dec_fd = open(dec_device_path, O_RDWR);

//    printf("fd %x %x %x \n", enc_fd, disp_fd, dec_fd );
    
    if(enc_tbl_init(&enc_tbl) != 0)
    {
        return -1;
    }

    for(channel=0; channel<16; channel++)
    {
        enc_chan_preset(&enc_tbl, 0, channel);
    }

    
	disp = solo6010_display_open(disp_device_path, MUX_MODE_16CH);
    return 0;
}

int DeInitDSPs ()
{
//    close(enc_fd);
//    close(disp_fd);
    close(dec_fd);
    return 0;
}

int ChannelOpen (unsigned int channelNum, STREAM_READ_CALLBACK streamReadCallback)
{
    return channelNum;
}

int ChannelClose (unsigned int hChannelHandle)
{
    return 0;
}

int GetTotalChannels ()
{
    return 16;
}

int GetTotalDSPs ()
{
    return 1;
}

int GetBoardCount ()
{
    return 0;
}

int GetBoardDetail (unsigned int boardNum, DS_BOARD_DETAIL * BoardDetail)
{
    return 0;
}

int GetDspDetail (unsigned int dspNum, DSP_DETAIL * DspDetail)
{
    return 0;
}

int GetEncodeChannelCount ()
{
    return 0;
}

int GetDecodeChannelCount ()
{
    return 0;
}

int GetDisplayChannelCount ()
{
    return 0;
}

int GetBoardInfo (int hChannelHandle, unsigned int *boardType, char *serialNo)
{
    return 0;
}

int GetCapability (int hChannelHandle, CHANNEL_CAPABILITY * capability)
{
    return 0;
}

int StopVideoPreview (int hChannelHandle)
{
    return 0;
}

int StartVideoPreview (int hChannelHandle, PREVIEWCONFIG * pPreviewConf, unsigned int useSyncSem)
{
    return 0;
}

int SetBlockArea(int hChannelHandle, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
    return 0;
}

int StartVideoPreviewEx (int hChannelHandle, PREVIEWCONFIG * pPreviewConf, unsigned int useSyncSem, unsigned int mode)
{
    return 0;
}

int SetEncodePara (int hChannelHandle, int scale, int qp, int intvl, int gop, int mask)
{
    enc_para_set(&enc_tbl, hChannelHandle, scale, qp, intvl, gop, mask);
    return 0;
}

int GetEncodePara (int hChannelHandle, VideoStandard_t * videoStandard, 
                  int *scale, int *qp, int *intvl, int *gop)
{
    enc_para_get(&enc_tbl, hChannelHandle, scale, qp, intvl, gop);
    return 0;
}

void GetSDKVersion (PVERSION_INFO versionInfo)
{
}

int SetStreamType (int hChannelHandle, int type)
{
    return 0;
}

int GetStreamType (int hChannelHandle, int *streamType)
{
    return 0;
}
int SetSubStreamType (int hChannelHandle, int type)
{
    return 0;
}
int GetSubStreamType (int hChannelHandle, int *streamType)
{
    return 0;
}

int StartVideoCapture (int hChannelHandle)
{
    enc_chan_preset(&enc_tbl, 1, hChannelHandle);
    return 0;
}

int VideoDecode(unsigned char* buf, int size)
{
    static int tmpinit=0;

    if(tmpinit==0)
    {
        solo6010_display_channel_mode(disp, 0, 5);
        solo6010_display_channel_mode(disp, 1, 5);
        solo6010_display_channel_mode(disp, 2, 5);
        solo6010_display_channel_mode(disp, 3, 5);
        solo6010_display_channel_mode(disp, 4, 5);
        solo6010_display_channel_mode(disp, 5, 5);
        solo6010_display_channel_mode(disp, 6, 5);
        solo6010_display_channel_mode(disp, 7, 5);
        solo6010_display_channel_mode(disp, 8, 5);
        solo6010_display_channel_mode(disp, 9, 5);
        solo6010_display_channel_mode(disp, 10, 5);
        solo6010_display_channel_mode(disp, 11, 5);
        solo6010_display_channel_mode(disp, 12, 5);
        solo6010_display_channel_mode(disp, 13, 5);
        solo6010_display_channel_mode(disp, 14, 5);
        solo6010_display_channel_mode(disp, 15, 5);        

        tmpinit=1;
    }
    
    write(dec_fd, buf, size);
    return 0;
}

int StartSubVideoCapture (int hChannelHandle)
{
    return 0;
}
int StopVideoCapture (int hChannelHandle)
{
    enc_chan_preset(&enc_tbl, 0, hChannelHandle);
    return 0;
}
int StopSubVideoCapture (int hChannelHandle)
{
    return 0;
}

int SetIBPMode (int hChannelHandle, int KeyFrameIntervals, int BFrames, int PFrames, int FrameRate)
{
    return 0;
}
int SetDefaultQuant (int hChannelHandle, int IQuantVal, int PQuantVal, int BQuantVal)
{
    return 0;
}

int SetEncoderPictureFormat (int hChannelHandle, PictureFormat_t pictureFormat)
{
    return 0;
}
int SetSubEncoderPictureFormat (int hChannelHandle, PictureFormat_t pictureFormat)
{
    return 0;
}

int SetupBitrateControl (int hChannelHandle, ULONG maxbps)
{
    return 0;
}
int SetBitrateControlMode (int hChannelHandle, BitrateControlType_t brc)
{
    return 0;
}

int SetVideoStandard (int hChannelHandle, VideoStandard_t videoStandard)
{
    return 0;
}

int SetDefaultVideoStandard (VideoStandard_t VideoStandard)
{
    return 0;
}

int GetVideoStandard ()
{
    return disp->norm;
}

int SetVideoDetectPrecision (int hChannelHandle, unsigned int value)
{
    return 0;
}
int GetVideoSignal (int hChannelHandle)
{
    return 0;
}
int SetInputVideoPosition (int hChannelHandle, unsigned int x, unsigned int y)
{
    return 0;
}

int SetOsdDisplayMode (int hChannelHandle, int brightness, int translucent,
                       int twinkInterval, unsigned short *format1, unsigned short *format2)
{
    return 0;
}
int SetOsd (int hChannelHandle, int enable)
{
    return 0;
}
int SetupDateTime (int hChannelHandle, SYSTEMTIME * now)
{
    return 0;
}                               /* now is invalid for Ver4.0 */
int SetOsdDisplayModeEx (int hChannelHandle, int color, int Translucent,
                         int param, int nLineCount, unsigned short **Format)
{
    return 0;
}

int LoadYUVFromBmpFile (char *FileName, unsigned char *yuv, int BufLen, int *Width, int *Height)
{
    return 0;
}
int SetLogoDisplayMode (int hChannelHandle, unsigned short ColorKeyR,
                        unsigned short ColorKeyG, unsigned short ColorKeyB,
                        unsigned short bTranslucent, int TwinkleInterval)
{
    return 0;
}
int SetLogo (int hChannelHandle, int x, int y, int w, int h, unsigned char *yuv)
{
    return 0;
}
int StopLogo (int hChannelHandle)
{
    return 0;
}

int SetupWM (int poly, int key, int str)
{
    enc_set_wm(&enc_tbl, poly, key, str);
    return 0;
}

int StopWM ()
{
    enc_set_wm(&enc_tbl, 0, 0, 0);
    return 0;
}

int SetupMask (int hChannelHandle, RECT * rectList, int iAreas)
{
    return 0;
}
int StopMask (int hChannelHandle)
{
    return 0;
}

int AdjustMotionDetectPrecision (int hChannelHandle, int iGrade,
                                 int iFastMotionDetectFps, int iSlowMotionDetectFps)
{
    return 0;
}
int SetupMotionDetection (int hChannelHandle, RECT * RectList, int iAreas)
{
    return 0;
}
int StartMotionDetection (int hChannelHandle)
{
    solo6010_display_motion_trace(disp, 1);

    return 0;
}
int StopMotionDetection (int hChannelHandle)
{
    solo6010_display_motion_trace(disp, 0);

    return 0;
}
int MotionAnalyzer (int hChannelHandle, char *MotionData, int iThreshold, int *iResult)
{
    return 0;
}
int SetupMotionDetectionEx (int hChannelHandle, int iGrade, int iFastMotionDetectFps, 
                        int iSlowMotionDetectFps, unsigned int delay, RECT * RectList, 
                        int iAreas, MOTION_DETECTION_CALLBACK MotionDetectionCallback, int reserved)
{
    return 0;
}

int SetAudioPreview (int hChannelHandle, int bEnable)
{
    return 0;
}
int GetSoundLevel (int hChannelHandle)
{
    return 0;
}

int RegisterImageStreamCallback (IMAGE_STREAM_CALLBACK pFunc, void *context)
{
    return 0;
}
int SetImageStream (int hChannelHandle, int bStart, unsigned int fps, 
                    unsigned width, unsigned height, unsigned char *imageBuf)
{
    return 0;
}

int GetOriginalImage (int hChannelHandle, unsigned char *imageBuf, int *size)
{
    return 0;
}
int SaveYUVToBmpFile (char *FileName, unsigned char *yuv, int Width, int Height)
{
    return 0;
}
int GetJpegImage (int hChannelHandle, unsigned char *ImageBuf, unsigned long *Size, unsigned int nQuality)
{
    return 0;
}

int SetupSubChannel (int hChannelHandle, int iSubChannel)
{
    return 0;
}
int GetSubChannelStreamType (void *dataBuf, int frameType)
{
    return 0;
}

int GetFramesStatistics (int hChannelHandle, PFRAMES_STATISTICS framesStatistics)
{
    return 0;
}
int CaptureIFrame (int hChannelHandle)
{
    return 0;
}

int CaptureFrames(int card, int enable, unsigned char* addr)
{
    read(disp->fd, addr, 1024*576*2);
    return 0;
}

int SetDeInterlace (int hChannelHandle, unsigned int mode, unsigned int level)
{
    return 0;
}
int ResetDSP (int hChannelHandle)
{
    return 0;
}

int SetCoverDetection (int hChannelHandle, int bEnable, int iGrade, int delay,
                       COVER_DETECTION_CALLBACK CoverDetectionCallback)
{
    return 0;
}
int SetupMosaic (int hChannelHandle, int iAreas, RECT * rectList, int reserved)
{
    return 0;
}
int SetWatchDog (unsigned int boardNumber, int bEnable)
{
    return 0;
}                               /* just use in ds4016hcs */

int SetAlarmIn (unsigned int boardNumber, unsigned int alarmInEnableFlag,
                unsigned int alarmInParam, ALARM_IN_CALLBACK pCallbackFunc, int reserved)
{
    return 0;
}                               /* just use in ds4016hcs */
int SetAlarmOut (unsigned int boardNumber, unsigned int alarmOutFlag, unsigned int alarmOutValue, int reserved)
{
    return 0;
}                               /* just use in ds4016hcs */

/* modified at 2007.7.14 */
int SetFaceDetectionDemo (int hChannelHandle, int bEnable, unsigned int nFrameInterval,
                          FACE_DETECTION_DEMO_CALLBACK pFunc, int bCompress,
                          unsigned int nCompressQuality, int bLocateEyePos)
{
    return 0;
}

/* The api list for DS400xMD */
int HW_InitDecDevice (long *pDeviceTotal)
{
    return 0;
}

int HW_ReleaseDecDevice ()
{
    return 0;
}
int HW_ChannelOpen (long ChannelNum, int *phChannel)
{
    return 0;
}
int HW_ChannelClose (int hChannel)
{
    return 0;
}

int HW_OpenStream (int hChannel, char *pFileHeadBuf, int nSize)
{
    return 0;
}
int HW_CloseStream (int hChannel)
{
    return 0;
}
int HW_InputData (int hChannel, char *pBuf, int nSize)
{
    return 0;
}

int HW_OpenFile (int hChannel, char *sFileName)
{
    return 0;
}
int HW_CloseFile (int hChannel)
{
    return 0;
}

int HW_Play (int hChannel)
{
    return 0;
}
int HW_Stop (int hChannel)
{
    return 0;
}
int HW_Pause (int hChannel, ULONG bPause)
{
    return 0;
}

int HW_PlaySound (int hChannel)
{
    return 0;
}
int HW_StopSound (int hChannel)
{
    return 0;
}
int HW_SetVolume (int hChannel, ULONG nVolume)
{
    return 0;
}

int HW_StartCapFile (int hChannel, char *sFileName)
{
    return 0;
}
int HW_StopCapFile (int hChannel)
{
    return 0;
}

int HW_GetPictureSize (int hChannel, ULONG * pWidth, ULONG * pHeight)
{
    return 0;
}
int HW_GetYV12Image (int hChannel, char *pBuffer, ULONG nSize)
{
    return 0;
}
int HW_ConvertToBmpFile (char *pBuf, ULONG nSize, ULONG nWidth, ULONG nHeight, char *sFileName, ULONG nReserved)
{
    return 0;
}
int HW_GetSpeed (int hChannel, long *pSpeed)
{
    return 0;
}
int HW_SetSpeed (int hChannel, long nSpeed)
{
    return 0;
}
int HW_SetPlayPos (int hChannel, ULONG nPos)
{
    return 0;
}
int HW_GetPlayPos (int hChannel, ULONG * pPos)
{
    return 0;
}
int HW_SetJumpInterval (int hChannel, ULONG nSecond)
{
    return 0;
}
int HW_Jump (int hChannel, ULONG nDirection)
{
    return 0;
}

int HW_GetVersion (PHW_VERSION pVersion)
{
    return 0;
}
int HW_GetCurrentFrameRate (int hChannel, ULONG * pFrameRate)
{
    return 0;
}
int HW_GetCurrentFrameNum (int hChannel, ULONG * pFrameNum)
{
    return 0;
}
int HW_GetFileTotalFrames (int hChannel, ULONG * pTotalFrames)
{
    return 0;
}
int HW_GetFileTime (int hChannel, ULONG * pFileTime)
{
    return 0;
}
int HW_GetCurrentFrameTime (int hChannel, ULONG * pFrameTime)
{
    return 0;
}
int HW_GetPlayedFrames (int hChannel, ULONG * pDecVFrames)
{
    return 0;
}
int HW_SetFileEndMsg (int hChannel, sem_t * nMsg)
{
    return 0;
}
int HW_SetStreamOpenMode (int hChannel, ULONG nMode)
{
    return 0;
}
int HW_GetStreamOpenMode (int hChannel, ULONG * pMode)
{
    return 0;
}
int HW_SetAudioPreview (int hChannel, unsigned int bEnable)
{
    return 0;
}

int HW_StartDecVgaDisplay (int hChannel, PREVIEWCONFIG * pPreviewConf, unsigned int useSyncSem)
{
    return 0;
}
int HW_StopDecChanVgaDisplay (int hChannel)
{
    return 0;
}

int SetDisplayStandard (unsigned int nDisplayChan, VideoStandard_t VideoStandard)
{
    return 0;
}
int SetDisplayRegion (unsigned int nDisplayChannel, unsigned int nRegionCount, REGION_PARAM * pInit, unsigned int nReserved)
{
    return 0;
}
int ClearDisplayRegion (unsigned int nDisplayChannel, unsigned int flgRegion)
{
    return 0;
}
int SetDisplayRegionPosition (unsigned int nDisplayChan, unsigned int region, unsigned int x, unsigned int y)
{
    return 0;
}
int FillDisplayRegion (unsigned int nDisplayChan, unsigned int region, unsigned char *pImage)
{
    return 0;
}
int SetDecoderAudioOutput (unsigned int nDecodeChannel, unsigned int bEnable, unsigned int nOutputChannel)
{
    return 0;
}
int SetDecoderVideoOutput (unsigned int nDecodeChannel, unsigned int nPort, 
                           unsigned int bOpen, unsigned int nDisplayChannel, 
                           unsigned int nDisplayRegion, unsigned int nReserved)
{
    return 0;
}
int SetDecoderVideoExtOutput (unsigned int nDecodeChannel, unsigned int nPort, 
                              unsigned int bOpen, unsigned int nDisplayChannel,
                              unsigned int nDisplayRegion, unsigned int nReserved)
{
    return 0;
}

int SetEncoderVideoExtOutput (unsigned int nEncodeChannel, unsigned int nPort,
                              unsigned int bOpen, unsigned int nDisplayChannel,
                              unsigned int nDisplayRegion, unsigned int nReserved)
{
    return 0;
}

/* V4.1 add */
int HW_SetFileRef (int hChannel, unsigned int bEnable, FILE_REF_DONE_CALLBACK FileRefDoneCallback)
{
    return 0;
}
int HW_GetFileAbsoluteTime (int hChannel, SYSTEMTIME * pStartTime, SYSTEMTIME * pEndTime)
{
    return 0;
}
int HW_GetCurrentAbsoluteTime (int hChannel, SYSTEMTIME * pTime)
{
    return 0;
}
int HW_LocateByAbsoluteTime (int hChannel, SYSTEMTIME time)
{
    return 0;
}
int HW_LocateByFrameNumber (int hChannel, unsigned int frmNum)
{
    return 0;
}

/* V4.1.5 add at 2006.1.19 */
int HW_ExportFileRef (int hChannel, char *pBuffer, unsigned int nSize)
{
    return 0;
}
int HW_ImportFileRef (int hChannel, char *pBuffer, unsigned int nSize)
{
    return 0;
}
int SetDisplayVideoCapture (unsigned int chan, unsigned int bStart, 
                            unsigned int fps, unsigned int width, 
                            unsigned int height, unsigned char *imageBuffer)
{
    return 0;
}
int RegisterDisplayVideoCaptureCallback (IMAGE_STREAM_CALLBACK pFunc, void *context)
{
    return 0;
}

/* add at 2006.3.21 */
int SetDisplayVideoBrightness (int hChannel, int Brightness)
{
    return 0;
}

/* add at 2006.3.29 */
int HW_ResetStream (int hChannel)
{
    return 0;
}

/* add at 2006.4.6 */
int HW_InputDataByFrame (int hChannel, char *pBuf, int nSize)
{
    return 0;
}

/* add at 2006.4.25 */
int SetChannelStreamCRC (int hChannelHandle, int bEnable)
{
    return 0;
}
int SetSubChannelStreamCRC (int hChannelHandle, int bEnable)
{
    return 0;
}

/* add at 2006.6.29 */
int HW_SetDecoderPostProcess (int hChannel, unsigned int param)
{
    return 0;
}

/* add at 2006.10.19 */
int RegisterDecoderVideoCaptureCallback (DECODER_VIDEO_CAPTURE_CALLBACK DecoderVideoCaptureCallback, void *context)
{
    return 0;
}
int HW_SetDecoderVideoCapture (int hChannel, int bStart, unsigned int param)
{
    return 0;
}

/* add at 2007.07.14 */
int SetSystemParameters (unsigned int *param)
{
    return 0;
}

int MP4_ServerGetState()
{
    return 0;
}

int MP4_ServerSetNetPort(int iServerPort, int iClientPort)
{
    return 0;
}

int MP4_ServerSetMessage(void (*MessageCallBack) (char*buf, int iLen))
{
    return 0;
}

int MP4_ServerStart(PSERVER_VIDEOINFO videoinfo)
{
    return 0;
}

int MP4_ServerStop()
{
    return 0;
}

int MP4_ServerCheckIP(int (*CheckIPCallBack) (int iChannel, char*sIP))
{
    return 0;
}

int MP4_ServerCheckPassword(int (*CheckPasswordCallBack) (char*usename, int namelen, char*password, int passlen))
{
    return 0;
}


int MP4_ServerWriteData(int nPort, char*  pPacketBuffer, int nPacketSize,
                                      int frameType, int breakable)
{
    return 0;
}
                                  
int MP4_ServerStringToClient(char* m_lAddrIP, char* m_sCommand, int miLen)
{
    return 0;
}

int MP4_ServerSetBufNum(int nChannel, int dBufNum)
{
    return 0;
}

int MP4_ServerResetChannel(int nChannel)
{
    return 0;
}

int MP4_ServerSetStart(void (*StartCapCallBack) (int port))
{
    return 0;
}

int MP4_ServerSetStop(void (*StopCapCallBack) (int port))
{
    return 0;
}

int MP4_ServerSetTTL(unsigned char cTTLVal)
{
    return 0;
}

int MP4_ServerCommandToClient(int iChannel, unsigned char cCommand)
{
    return 0;
}


/* add at 2003.11.24 to set the multicast add and port */
int MP4_ServerSetMulticast(int iChannel, char* m_sIpAddr, int iPort)
{
    return 0;
}


/*
 * add at 2004.11.8 to support two stream type data in the same channel in server
 * side. subchan: only be 0 or 1
 */
int MP4_ServerWriteDataEx(int nPort, char*  pPacketBuffer, int nPacketSize,
                                        int frameType, int breakable, int subchan)
{
    return 0;
}
                                        
int MP4_ServerGetLastErrorNum()
{
    return 0;
}


/* add at 2005.10.7 to set the ip and passwd check */
int MP4_ServerCheckIPandPassword(int (*CheckIPandPass) (int iChannel, char*username,
                                               int namelen, char*password, int passlen, char*sIP))
{
    return 0;
}
                                           

/* add at 2005.10.7 to register disconnect event */
int MP4_ServerDisconnectEvent(void (*DisconnectEvent) (int nport, char*sIP))
{
    return 0;
}


/* add at 2005.10.27 to disconnect the client */
int MP4_ServerDisconnectClient(int iChannel, char* ClientAddr)
{
    return 0;
}

