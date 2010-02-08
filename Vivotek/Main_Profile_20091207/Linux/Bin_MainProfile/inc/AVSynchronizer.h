/*!
 *******************************************************************************
 * Copyright 2000-2005 Vivotek, Inc. All rights reserved.
 *
 * \file
 * AVSynchronizer.h
 *
 * \brief
 * 
 *
 * \date
 * 2005/12/05
 *
 * \author
 * Steven
 *
 *
 *******************************************************************************
 */
#ifndef _AVSYNCHRONIZER_H 
#define _AVSYNCHRONIZER_H

#if defined(_WIN32) || defined(_WIN32_WCE)
#include <windows.h>
#include <mmreg.h>
#include "typedef.h"

#else

#include "typedef.h"

#define	HWND			HANDLE
#define	HDC				HANDLE
#define	HBITMAP			HANDLE

typedef struct tagRECT
{
    long    left;
    long    top;
    long    right;
    long    bottom;
} RECT;

typedef struct tagPOINT
{
    long  x;
    long  y;
} POINT;

typedef struct tagSIZE 
{ 
  long cx; 
  long cy; 
} SIZE;

typedef	DWORD			COLORREF;
#endif // _WIN32 || _WIN32_WCE

#include <time.h>
#include "common.h"
#include "errordef.h"
#include "mainprofile_error.h"
#include "datapacketdef.h"
#include "SrvTypeDef.h"

#define AVSYNCHRONIZER_VERSION	MAKEFOURCC ( 4, 4, 0, 55) 
#define	MAX_BORDER_SIZE						50
#define	CONST_BITS_PER_SAMPLE			16
#define	CONST_AO_CHANNEL				1
#define	CONST_G7221_SAMPLE_RATE			16000
#define	CONST_G729A_SAMPLE_RATE			8000

#define	MAX_DISP_BLOCK					6

typedef enum  {
	AVSYNCHRONIZER_VIDEOSF_OFFSCREEN_YUY2 = 1,
    AVSYNCHRONIZER_VIDEOSF_OFFSCREEN_UYVY = (1<<1),
	AVSYNCHRONIZER_VIDEOSF_OFFSCREEN_RGB16 = (1<<2),
	AVSYNCHRONIZER_VIDEOSF_OFFSCREEN_RGB24 = (1<<3),
	AVSYNCHRONIZER_VIDEOSF_OFFSCREEN_RGB32 = (1<<4),
	AVSYNCHRONIZER_VIDEOSF_OVERLAY_YUY2	= (1<<5),
	AVSYNCHRONIZER_VIDEOSF_OVERLAY_UYVY = (1<<6),
	AVSYNCHRONIZER_VIDEOSF_OVERLAY_R5G6B5 = (1<<7),
	AVSYNCHRONIZER_VIDEOSF_OVERLAY_RGB24 = (1<<8),
	AVSYNCHRONIZER_VIDEOSF_OVERLAY_RGB32 = (1<<9),
	AVSYNCHRONIZER_VIDEOCAB_BLTFOURCC = (1<<10),
	AVSYNCHRONIZER_VIDEOCAB_OVERLAYFOURCC = (1<<11),
	AVSYNCHRONIZER_VIDEOCAB_CKEY_DESTBLT = (1<<12),
	AVSYNCHRONIZER_VIDEOCAB_CKEY_DESTOVERLAY = (1<<13),
	AVSYNCHRONIZER_VIDEOCAB_CKEY_SRCBLT = (1<<14),
	AVSYNCHRONIZER_VIDEOCAB_CKEY_SRCOVERLAY = (1<<15),
	AVSYNCHRONIZER_VIDEOCAB_EMULATIONONLY = (1<<16),
	AVSYNCHRONIZER_VIDEOCAB_NO_HWSTRETCH = (1<<17),
	AVSYNCHRONIZER_VIDEOCAB_NO_OVERLAYHWSTRETCH = (1<<18),
    AVSYNCHRONIZER_VIDEOCAP_NO_BLT_SYSMEM = (1<<19),
	AVSYNCHRONIZER_VIDEOCAP_NO_AGP = (1<<20),
	AVSYNCHRONIZER_VIDEOSF_OVERLAY_R5G5B5 = (1<<21),
	AVSYNCHRONIZER_VIDEOCAB_BLTSTRETCHY = (1<<22),
	AVSYNCHRONIZER_VIDEOCAB_BLTSTRETCHX = (1<<23),
	AVSYNCHRONIZER_VIDEOCAB_BLTSHRINKY = (1<<24),
	AVSYNCHRONIZER_VIDEOCAB_BLTSHRINKX = (1<<25),
	AVSYNCHRONIZER_VIDEOCAB_OVERLAYSTRETCHY = (1<<26),
	AVSYNCHRONIZER_VIDEOCAB_OVERLAYSTRETCHX = (1<<27),
	AVSYNCHRONIZER_VIDEOCAB_OVERLAYSHRINKY = (1<<28),
	AVSYNCHRONIZER_VIDEOCAB_OVERLAYSHRINKX = (1<<29),
} EDISPLAYCAP;


typedef enum  {
	AUDIO_FRAME = 2,
    VIDEO_FRAME = 1
} EFRAMETYPE;

typedef enum  {
	AVSYNCHRONIZER_MEDIATYPE_VIDEO_ONLY = 0x0001,
	AVSYNCHRONIZER_MEDIATYPE_AUDIO_ONLY = 0x0002,
	AVSYNCHRONIZER_MEDIATYPE_AUDIO_VIDEO = 0x0003,
} EMEDIATYPE, TMediaType;

typedef enum
{
	ALIGN_HORI_CENTER = 0,
	ALIGN_HORI_LEFT = 1,
	ALIGN_HORI_RIGHT = 2,
	ALIGN_VERT_CENTER = 0,
	ALIGN_VERT_TOP = 4,
	ALIGN_VERT_BOTTOM = 8,
} EGRAPHALIGN;

typedef enum
{
	//	Disable the anti-tearing(don't wait for verical blank)
	ANTI_TEARING_DISABLED = 1,

	//	Always wait for vertical blank
	ANTI_TEARING_ALWAYS = 2,

	//	If the height of destination window is bigger than 240, it will wait for vertical blank.
	ANTI_TEARING_AUTO = 3,
} EANTITEARING;


typedef enum  {
	AUDIOOUT_FOCUS_NORMAL = 1,
	AUDIOOUT_FOCUS_STICKY = 2,
	AUDIOOUT_FOCUS_GLOBAL = 3,
} EAUDIOFOCUSTYPE;// TAudioFocusType;

typedef enum EPbChFlag {
	PBCH_STATUSCB		= 0x00000001,
	PBCH_DISPLAYCB		= 0x00000002,
	PBCH_ALLCALLBACK	= 0x10000003,
	PBCH_STATUSCONTEXT	= 0x00000004,
	PBCH_DISPLAYCONTEXT	= 0x00000008,
	PBCH_ALLCONTEXT		= 0x2000000c,
	PBCH_VOLUME			= 0x00000010,
	PBCH_LFBORDER		= 0x00000020,
	PBCH_RTBORDER		= 0x00000040,
	PBCH_TPBORDER		= 0x00000080,
	PBCH_BTBORDER		= 0x00000100,
	PBCH_ALLBORDER		= 0x000001e0,
	PBCH_BITMAP			= 0x00000200,
	PBCH_MOTION			= 0x00000400,
	PBCH_MOTIONRECT 	= 0x00000800,
	PBCH_MOTIONRECTALERT= 0x00001000,
	PBCH_VIDEOOUT		= 0x00002000,
	PBCH_AUDIOOUT		= 0x00004000,
	// --------------------------------------------------
	PBCH_NEXTFRAME		= 0x08000000,
	PBCH_INPUTCB		= 0x10000000,
	PBCH_INPUTCONTEXT	= 0x20000000,
	PBCH_BLOCKING		= 0x40000000,
	PBCH_ALL			= 0xFFFFFFFF
} EPBCHFLAG;

typedef enum EChFlag {
	CH_STATUSCB			= 0x00000001,
	CH_DISPLAYCB		= 0x00000002,
	CH_ALLCALLBACK		= 0x00000003,
	CH_STATUSCONTEXT	= 0x00000004,
	CH_DISPLAYCONTEXT	= 0x00000008,
	CH_ALLCONTEXT		= 0x0000000c,
	CH_VOLUME			= 0x00000010,
	CH_LFBORDER			= 0x00000020,
	CH_RTBORDER			= 0x00000040,
	CH_TPBORDER			= 0x00000080,
	CH_BTBORDER			= 0x00000100,
	CH_ALLBORDER		= 0x000001e0,
	CH_BITMAP			= 0x00000200,
	CH_MOTION			= 0x00000400,
	CH_MOTIONRECT		= 0x00000800,
	CH_MOTIONRECTALERT	= 0x00001000,
	CH_VIDEOOUT			= 0x00002000,
	CH_AUDIOOUT			= 0x00004000,
	//---------------------------------
	CH_ALL				= 0xFFFFFFFF
} ECHFLAG;

typedef enum ESetChOption
{
	SETCH_DEBLOCKING		= 0x00000001,	// param1 = BOOLEAN, param2 is not used
	SETCH_BLANK_BORDER_CB	= 0x00000002,	// param1 = BOOLEAN, param2 is not used, normal and playback channel
	SETCH_GRAPH_STRETCH		= 0x00000003,	// param1 = BOOLEAN, param2 is not used, normal and playback channel
	SETCH_GRAPH_ALIGN		= 0x00000004,	// param1 = Horizontal align, param2 Vertical align, normal and playback channel
	SETCH_NOTIFY_MOTION		= 0x00000005,	// param1 = BOOLEAN, param2 is not used, normal and playback channel
	SETCH_RESERVED_HEADER	= 0x00000006,	// param1 = size in byte, only for decoder channel
	SETCH_LEFT_BORDER	= 0x00000007,	// param1 = new left border size, normal and playback channel
	SETCH_RIGHT_BORDER	= 0x00000008,	// param1 = new right border size, normal and playback channel
	SETCH_TOP_BORDER	= 0x00000009,	// param1 = new top border size, normal and playback channel
	SETCH_BOTTOM_BORDER	= 0x0000000A,	// param1 = new bottom border size, normal and playback channel
	SETCH_DIGITAL_ZOOM	= 0x0000000B,	// param1 = TDIGITALZOOMPARAM *, param2 is the flag 
										// that says which member of TDIGITALZOOMPARAM is meaningful
										// normal and playback channel
	SETCH_REDRAW_DIGITAL_ZOOM = 0x0000000C, // force to redraw the digital zoom window, normal and playback channel
	SETCH_MEDIA_TYPE = 0x0000000D,	// change the media type of the channel, param1 is the new media type
									// normal and playback channel
	SETCH_OUTPUT_VIDEO_FRAME = 0x0000000E,	// if the call to AvSynchronizer_DecodeVideo,
										// AvSynchronizer_InputToBeDecodedMediaFrame should output decoded
										// video or not. This could save CPU speed if you don't need the data
										// when fast forward, dwParam1 = boolean, param2 is not used
										// for normal/playback channel this decide if the channel will
										// output decoded video frame to users or not
	SETCH_OUTPUT_AUDIO_FRAME = 0x0000000F,	// if the call to AvSynchronizer_DecodeAudio or
										// AvSynchronizer_InputToBeDecodedMediaFrame should output
										// audio or not. The decoding will be cancelled if this flag
										// is set. For normal/playback channel, this decide if the audio decoded data
										// should be callback or not.
	SETCH_DECODE_DATA_FORMAT = 0x00000010,	// param1 = the new rawdataformat for decoder/normal/playback channel
	SETCH_DECODE_JPEG_QUALITY = 0x00000011,	// param1 = the new jpeg q value for decoder/normal/playback channel
	SETCH_DECODE_CB = 0x00000012,	// param1 = context, param2 = callback (LPDECODEFRAMECALLBACK),
									// only applied to normal and playback channel
									// with this callback set, the two channels will callback each
									// video and audio frame
	SETCH_DECODE_VIDEO_BUFFER = 0x00000013,	// param1 = size of user given buffer, param2 = pointer of the buffer, normal and playback channel
	SETCH_DECODE_AUDIO_BUFFER = 0x00000014,	// param1 = size of user given bufer, param2 = pointer of the buffer, normal and playback channel
	SETCH_RESUME_PLAYING = 0x00000015,	// param1 = boolean, false to cancel resuming, to tell channel to wait firt I again
	SETCH_DECODE_CBEX = 0x00000016,	// param1 = context, param2 = callback (LPDECODEFRAMECALLBACK_EX),
									// applied to normal, playback and decoder channel
									// for the first two channels, with this callback set the channels will callback each
									// video and audio frame, for decoder channel, the non-ex one will be replaced
	SETCH_DISPLAY_PERIOD = 0x00000017,	// param1 = the frame number. The display thread will display 1 frame per this number
										// param2 will the decode callback affect by this setting, it's a boolean value
	SETCH_AVI_CAPTION_CB_CONFORM = 0x00000018,	// param1 = boolean that the DrawTime is struct tm if true, or time_t if falst
												// default is false. For AVI conversion channel only
	SETCH_BITMAP_FILE = 0x00000019,			// set the channel to show the bitmap contain in the bmp or jpeg file,
											// the module will load the bitmap from the file, dwParam1 is the file name pointer in TCHAR *
											// if the file format is incorrect or does not exist, the set function will return S_FAIL
											// note this option and the bitmap in channel create or the next set bitmpahalde option are exclusive
											// and the show bitmap option also control the display of this setting bitmap
											// this option is only for live and playback channel
											// dwParam1 is the file name in const char *
	SETCH_BITMAP_HANDLE = 0x0000001A,		// set the channel to show the bitmap in the HBITMAP handle
											// note this option and the bitmap in in channel create or the above set bitmap file option are exclusive
											// and the show bitmap option also control the display of this setting bitmap
											// this option is only for live and playback channel
											// dwParam1 contains the handle for the bitmap
	SETCH_CLEAR_QUEUE = 0x0000001B,			// set the way to handle if the video queue is full, the defualt is doing nothing
											// dwParam1 is the way to handle:
											// 0: default, do nothing
											// 1: clear only when I frame reach and the queue is full
											// 2: clear the queue when full, and reset the I frame flag,
											//    the channel will accept packet only when next I frame reach
											// it only applies to normal channel
	SETCH_BACKWARD_I_MODE = 0x0000001C,	// is the playback channel running in backward I frame only mode?
										// in this mode, the channel will show each I frame for one second
										// if the format is non-jpeg, for jpeg frame, the frame will show 
										// in reversed sequence
										// param1 = true to set in backward mode, false to turn backward mode off
										// param2 not used
	SETCH_FASTER_CAPTION_ONGRAPH = 0x0000001D,	// for some cards, get direct draw dc and painting on graph
										// would be slowing, so with this flag, the module will
										// use a faster way to handle caption on graph,
										// but the AP must also prepare to handle this type of callback
										// the drawing area is maximum 640x480 in such mode, and AP
										// must also specify the drawing area before return.
										// to be transparent, mask color must also be given
	SETCH_ROTATE_IMAGE = 0x0000001E,	// should the image be rotated, the rotaged degree could be 90, 180, or 270 degree in clockwise
										// this option only works for Windows CE, other platform does not support this
										// dwParam1 is the degree could be 0, 90, 180, 270 
	SETCH_EMPTY_QUEUE = 0x0000001F,		// clear the queue currently playing (both A/V), this is workable
										// for AP that support switch to time point
    SETCH_DISPLAY_REDRAW = 0x00000020,   
	SETCH_FRAME_BUFF_STRIDE = 0x00000021,   // set the stride of the buffer used for retriving the decoded video
											// frame, dwParam1 is width stride
											// in pixel unit, not in bytes, only works for
											// decoder channel. Set value to 0 would make the scan line
											// continuous in memory
    SETCH_GETAV_TIME = 0x00000022,			// get the current time (only second) of the audio and video
											// the time is the last video and audio frame been played
											// param1 = address to hold video time, return 0 if audio only
											// param2 = address to hold audio time, return 0 if video only
	SETCH_DEINTERLACE = 0x00000023,		//enable deinterlace, param1 is true for enable and false for disable

	SETCH_VIDEO_FRAME_BEFORE_DISPLAY = 0x00000024,	//callback the decoded frame before display. AP can do some operations on 
																									//this frame
																									//param1 = the context of this callback function
																									//param2 = the callbakc function pointer, LPOUTPUTVIDEOFRAMEBEFOREDISPLAY
	SETCH_AV_NOT_SYNC = 0x00000025,			//dwParam1 = TRUE, A/V will not sync if this is set, going on their best speed
	SETCH_AVI_MAX_FILE_SIZE = 0x00000026,	//set max avi file size, only for avi channel
											//This setting should be called before AvSynchronizer_StartAVIChannel
											//param1 = the max file size of avi file. If the file size reaches the max value
											//			AvSychronizer will generate another avi file whose name is to 
											//			append a number(from 1, 2, 3,...) to current avi file name
	SETCH_AVI_FILE_TIME_INTERVAL = 0x00000027, //set max avi file length, only for avi channel
										//This setting should be called before AvSynchronizer_StartAVIChannel
										//param1 = the time interval of avi file and its unit is minute. 
										//			If the time length reaches the max value
										//			AvSychronizer will generate another avi file whose name is to 
										//			append a number(from 1, 2, 3,...) to current avi file name
										//param2 = TRUE, use time length to divide the avi file
										//		   FALSE, disable this function
	SETCH_NOT_FORCE_SHOW_FRAME = 0x00000028,	//Playback channel will force the current video to show if the queue is full
											//dwParam1 = FALSE, A/V will sync according to their time stamp
	SETCH_NOT_BLOCK_WHEN_QUEUE_FULL = 0x00000029,	//In Playback channel, if AP inputs packets to AvSynchronizer and the internal queue fulls, Playback channel will block the calling thread until internal queue has free space.
													//User can turn off this mechanism by setting dwParam1 to TRUE or turn on it again by setting dwParam1 = FALSE
													//This mechanism turn on by default

	SETCH_ANTI_TEARING = 0x0000002a,	//	Set the anti_tearing policy, please refer to EANTITEARING. The default value is ANTI_TEARING_DISABLED.
} ESETCHOPTION;

typedef enum EUpChOption {
	UPCH_VIDEOOUT		= 0x00000001,
	UPCH_AUDIOOUT		= 0x00000002,
	UPCH_MOTIONALERT	= 0x00000004,
	UPCH_MOTIONRECT		= 0x00000008,
	UPCH_MOTIONRECTALERT= 0x00000010,
	UPCH_VOLUME			= 0x00000020,
	UPCH_MEDIATYPE		= 0x00000040,
	UPCH_TIMEITV_CHG	= 0x00000080,
	UPCH_SETSPEED		= 0x00000100,
	UPCH_PAUSE			= 0x00000200,
	UPCH_SHOW_BMP		= 0x00000400,
	UPCH_WIN_HANDLE		= 0x00000800,	// to change to show in a new window
	UPCH_RESTORE_WINPROC = 0x00001000, // restore the window handle proc
	UPCH_IGNORE_BORDER	= 0x00002000, // update normal border size
	UPCH_ALL			= 0xFFFFFFFF
} EUPCHOPTION;

typedef enum EMotionAlertColor {
	RECT_RED		= 0,
	RECT_GREEN		= 1,
	RECT_BLUE		= 2,
	RECT_GRAY		= 3
} EMOTIONALERTCOLOR;

typedef enum EPixelFormat {
	PF_YUY2 = 1,
	PF_RGB16565 = 2,
	PF_RGB24 = 3,
	PF_RGB32 = 4,
	PF_BMP16565 = 5,
	PF_BMP24 = 6,
	PF_BMP32 = 7,
	PF_JPEG = 8,	// 2K servers don't have to use this to get the jpeg data, 
	PF_RGB16555 = 9,
	PF_BMP16555 = 10,
	PF_YUV = 11,	// 4:2:0 format
	PF_IYUV = 11,	// 4:2:0 format
	PF_BGR16565 = 12,	//decoder supports BGR format, only for linux
	PF_BGR24 = 13,		// only for linux
	PF_BGR32 = 14,		// only for linux
	PF_BGR16555 = 15,	// only for linux
	PF_YV12 = 16,		// YVU in order
} EPIXELFORMAT;

typedef enum EDisplayInfoFlag
{
	DIF_PIC_SIZE = 1,
	DIF_CONNECTING = 2,
	DIF_SHOW_BLANK = 4,	// callback for blank caption and border
	DIF_SHOW_AUDIOONLY = 8, // called when playing audio only media
	DIF_IGNORE_BORDER = 16,	// if the ignore border is set for audio only case, the callee should not paint border in such case
	DIF_RESUMING = 32,	// only if resuming and waiting for I
	DIF_TELLME_RECT = 64,	// the callee must tell the callback about the rect and color valid for caption
} EDISPLAYINFOFLAG;

typedef enum ESnapShotFlag
{
    VISIBLE_SIZE = 0x0,
    ORIGINAL_SIZE = 0x1,
	SOURCE_IMAGE = 0x2,	// return the compressed image that is currently displayed, usually this is for jpeg server that
						// user want to get jpeg image
	SPECIFY_FMT = 0x4,	// return the format according to specify, the high word of the flag is the format
						// of the request fmt, the available format is listed as EPIXELFORMAT 
	GIVEN_BUFFER = 0x8,	// the buffer pDataStart is given by caller, in such case the dwdataSize must specify the
						// buffer size, when return, dwdataSize will be modified to the real size of the buffer
						// if caller needs to reuse the buffer, the buffer size value most be saved before
						// calling this function
} ESNAPSHOTFLAG;

typedef enum EAYSyncInitFlag
{
	DECODER_CHANNEL_ONLY = 0x00000001,
	USE_GDI_ONLY = 0x00000002,			// these two settings can't be used together
	USE_DIRECTDRAW_ONLY = 0x00000004,
	CAPTION_ON_GRAPH = 0x00000008,		// the caption cache machenism will be cancelled
	BETTER_GDI_STRETCH = 0x00000010,	// use stretch rather than dib draw
	ONE_PASS_DDRAW = 0x00000020,	// when in DDraw mode (offscreen), don't use intermediate surface for drawing
									// just put the decoded data on to primary surface, it will be faster, but
									// if stretching, the video will shiver when other window moves over
	INDIVIDUAL_SURFACE = 0x00000040,	// if possible, each channel uses one YUV surface
	ORG_SIZE_MODE = 0x00000080,			// only when this flag is set could channel to set org size border
	FORCE_GDI = 0x00000100,				// no matter what mode is now, switch to GDI mode, this mode could be switched back and forth freely
	CHANGE_TIME_PRECISION = 0x00000200,
	FORCE_NON_YUV = 0x00000400,			// for some cards, the yuv operation will cause green screen
										// set this flag could avoid the problem, but
										// the system performance will be worse than yuv mode (maybe 5 ~ 15% decade)
	MEGA_PIXEL = 0x00000800,
	AV_DONT_SYNC = 0x80000000,	// A/V will not sync if this is set, going on their best speed
	
} EAVSYBCINITFLAG;

typedef enum EStartChannelFlag 
{
	DRAW_CONNECTING = 0x00000001,
} ESTARTCHANNELFLAG;

typedef enum EDigitalZoomFlag
{
	DZ_CANVAS = 0x00000001,
	DZ_CANVAS_BORDER = 0x00000002,
	DZ_ZOOM_AREA = 0x00000004,
	DZ_ZOOM_AREA_BORDER = 0x00000008,
	DZ_ENABLE_ZOOM = 0x00000010,
	DZ_CANVAS_DISPLAY = 0x00000020,
} EDIGITALZOOMFLAG;

typedef enum EAvSyncVideoMode
{
	AVSYNCHRONIZER_VIDEO_DDRAW		= 1,	//the current display mode is DDraw
	AVSYNCHRONIZER_VIDEO_GDI		= 2,		//the current display mode is GDI
}EAVSYNCVIDEOMODE;

typedef enum EChannelStatus
{
	AVSYNCHRONIZER_STATUSCODE_DECODE_ERROR = 1,
	AVSYNCHRONIZER_STATUSCODE_MEMALLOC_ERROR = 2,

					// will be called whenever the size is changed
					// will also be called when first packet decoded
					// Param1 is Width, Param2 is Height
	AVSYNCHRONIZER_STATUSCODE_FRAME_SIZE_CHG = 3,

	AVSYNCHRONIZER_STATUSCODE_AUDIO_START = 4,
	AVSYNCHRONIZER_STATUSCODE_AUDIO_STOP = 5,

		// param1 DWORD array of 3 percent, param2 = DWORD array of 3 alert
	AVSYNCHRONIZER_STATUSCODE_MOTION = 6, 
		
		// when a channel has shown a new image
	AVSYNCHRONIZER_NEW_IMAGE = 7,
		// the first image comes, it will be called back if resuming is set and
		// first I got
	AVSYNCHRONIZER_FIRST_IMAGE = 8,

	//the status of current video out mode, param1 = AVSYNCHRONIZER_VIDEO_DDRAW is DDraw, and param1 = AVSYNCHRONIZER_VIDEO_GDI is GDI mode
	AVSYNCHRONIZER_STATUSCODE_VIDEO_MODE = 9,
} ECHANNELSTATUS;

// for digital zoom
typedef struct tDititalZoomParam
{
	BOOL				bCanvasWin;	// TRUE: hTarget is used, False: rectTarget is used
	union
	{
		HWND			hTarget;	// the canvas to show the shrinked full video
		RECT			rectTarget;		// only valid if hThrinkCanvas is null
	} unCanvas;

	RECT			rectCanvasBorder;

	BOOL				bZoomWin;	// TRUE: hZoomArea is used, False: rectZoomArea is used
	union
	{
		HWND			hZoomArea;		// if this parameter is not null, it is prior to rect below
									// this window must be child of hThrinkCanvas
		RECT			rectZoomArea;	// ignored if hZoomArea is not null
	} unZoom;

	RECT			rectZoomBorder;

	BOOL			bZoomEnabled;	// if this flag is set to false, no digital zoom will take effect
	BOOL			bDisplayed;		// should we draw the canvas when painting, if the canvas is not visible
									// set this member to false could reduce consumption of CPU load
} TDIGITALZOOMPARAM;

typedef struct tVFrameInfo {
	DWORD					dwWidth;
	DWORD					dwHeight;
	DWORD					dwSize;
	BYTE					*pbyPicture;
	BYTE					*pbyHeader;
	DWORD					dwReserved;
	TMediaDataPacketInfo	tPacketInfo;	// only those with video related field is valid
} TVFRAMEINFO, *PTVFRAMEINFO;

typedef struct tAFrameInfo {
	DWORD		dwTime;
	DWORD		dwSize;
	BYTE		*pbySound;
	DWORD		dwTimeStampSec;
	DWORD		dwTimeStampMilliSec;
    TMediaDataPacketInfo	tPacketInfo;	// only those with audio related field is valid
} TAFRAMEINFO, *PTAFRAMEINFO;

typedef union tFrameInfo {
		TVFRAMEINFO tVideoFrame;
		TAFRAMEINFO tAudioFrame;
} TFRAMEINFO, *PTFRAMEINFO;

typedef struct tFrameInfoEx {
	TFRAMEINFO	tF;
	DWORD		dwCBCount;			// the count for the same frame in the loop, starts from 1
	DWORD		dwVideoFormat;		// only valid if the callback is video
	DWORD		dwNextFormat;		// the callee could set this parameter for next format if bContinueDecode is true
	BOOL		bContinueDecode;
	BYTE		abyReserved[64];	// reserved area that is set to all zero currently
} TFRAMEINFOEX, *PTFRAMEINFOEX;


typedef struct tOneDispBlock
{
	SIZE		szSrc;	// the valid rect of the text data drawn by callee
	POINT		ptDest;		// the destination point (x, y) to paint the text (size is the same as rectSrc)
	COLORREF	clrMask;	// the mask of the color that should be treated as transparent
} TONEDISPBLOCK;

typedef struct tDisplayInfo {
	HDC		hDC;
	POINT	ptOrigin;
	DWORD	dwWidth;
	DWORD	dwHeight;
	DWORD	DrawTime;
	DWORD	dwCallTimes;	// only valid for connecting callback
	DWORD	dwFlag;	// now only flag for drawing in original picture size or screen size
	DWORD	dwTimet;

	// new after 4.4.0.0, only used when ddraw mode and both caption on graph and onepass is off
	BOOL			bDrawOneMem;	// if true, the following must be filled. else, plese do as before
	DWORD			dwActiveBlock;	// callee must fill this to tell me how many block is correct
	TONEDISPBLOCK	atBlocks[MAX_DISP_BLOCK];
} TDISPLAYINFO, *PTDISPLAYINFO;

typedef struct tAvSyncQueueStatus
{
	DWORD	dwAudioQueueElements;	//the remaining queue elements in audio queue
	DWORD	dwVideoQueueElements;	//the remaining queue elements in video queue
	DWORD	dwAudioQueueSize;		//the size of audio queue
	DWORD	dwVideoQueueSize;		//the size of video queue
} TAVSYNCQUEUESTATUS;

// callback functions
typedef SCODE (__stdcall * LPSTATUSCALLBACK)(DWORD dwContext, DWORD dwStatusCode, DWORD dwParam1, DWORD dwParam2);
typedef SCODE (__stdcall * LPDISPLAYCALLBACK)(DWORD dwContext, TDISPLAYINFO * ptDisplayInfos );

typedef SCODE (__stdcall * LPINPUTFRAMEREQUESTCALLBACK)(DWORD dwContext, EMEDIATYPE tMediaType, BYTE* pbyData, DWORD dwLen );
typedef SCODE (__stdcall * LPDECODEFRAMECALLBACK)(DWORD dwContext, EMEDIATYPE tFrameType, TFRAMEINFO * tframeinfo );
// if both callback is set, the ex will take
typedef SCODE (__stdcall * LPDECODEFRAMECALLBACK_EX)(DWORD dwContext, EMEDIATYPE tFrameType, TFRAMEINFOEX * ptFrameinfoEx );

typedef struct tVideoInfo{
	DWORD		dwStride;
	DWORD		dwMaxHeight;
	DWORD		dwWidth;
	DWORD		dwHeight;
	BYTE		*pbyBuf;
}TVIDEOINFO;

typedef SCODE (__stdcall * LPOUTPUTVIDEOFRAMEBEFOREDISPLAY)(DWORD dwContext, TVIDEOINFO *ptVideoInfo);

typedef struct tPlaybackChannelOption {
	DWORD					dwFlags;			// 
	LPSTATUSCALLBACK		pfStatus;
	LPDISPLAYCALLBACK		pfDisplay;
	DWORD					dwStatusContext;
	DWORD					dwDisplayContext;
	DWORD					dwVolume;			// 0 ~ -5000 ? 5000 ~ 0 ?
	DWORD                   dwLeftBorderSize;	// 0
	DWORD                   dwRightBorderSize;	// 0
	DWORD                   dwTopBorderSize;	// 0
	DWORD                   dwBottomBorderSize; // 0
	HBITMAP					hBMP;				// NULL
	HWND					hVideoOut;
	HWND					hDisplay;
	BOOL					bSaveBmp;			// FALSE
	BOOL					bVideoOut;			// if display
	BOOL					bAudioOut;			// if mute
	BOOL					bMotionAlert;		// if draw the motion detection RECT
	DWORD					dwMotionRectAlert;	// color for motion rect when motion alert
	DWORD					dwMotionRect;		// color for motion rect -1 indicate that not show this rect
	BOOL					bOrgSizeCallback;	// if the caption display callback call with org size graph
	DWORD					dwOrgSizeLeftBorder;
	DWORD					dwOrgSizeTopBorder;
	DWORD					dwOrgSizeRightBorder;
	DWORD					dwOrgSizeBottomBorder;

	// playback only member
	LPINPUTFRAMEREQUESTCALLBACK pfFrameRequest;
	BOOL					bNonBlocking;		// FALSE
	DWORD					dwFrameRequestContext;
	
} TPBCHOPTION, *PTPBCHOPTION;

typedef struct tChannelOption {
	DWORD					dwFlags;
	LPSTATUSCALLBACK		pfStatus;
	LPDISPLAYCALLBACK		pfDisplay;
	DWORD					dwStatusContext;
	DWORD					dwDisplayContext;
	DWORD					dwVolume;			// 0 ~ -5000 ? 5000 ~ 0 ?
	DWORD                   dwLeftBorderSize;	// 0
	DWORD                   dwRightBorderSize;	// 0
	DWORD                   dwTopBorderSize;	// 0
	DWORD                   dwBottomBorderSize; // 0
	HBITMAP					hBMP;				// NULL
	HWND					hVideoOut;
	HWND					hDisplay;
	BOOL					bSaveBmp;			// FALSE
	BOOL					bVideoOut;			// if display
	BOOL					bAudioOut;			// if mute
	BOOL					bMotionAlert;		// if draw the motion detection RECT
	DWORD					dwMotionRectAlert;	// color for motion rect when motion alert
	DWORD					dwMotionRect;		// color for motion rect -1 indicate that not show this rect
	BOOL					bOrgSizeCallback;	// if the caption display callback call with org size graph
	DWORD					dwOrgSizeLeftBorder;
	DWORD					dwOrgSizeTopBorder;
	DWORD					dwOrgSizeRightBorder;
	DWORD					dwOrgSizeBottomBorder;
} TCHANNELOPTION, *PTCHANNELOPTION;

typedef struct tDecoderChannelOption {
	//DWORD					dwFlags;
	LPDECODEFRAMECALLBACK		pfDecodeFrame;
    DWORD						dwAvDecodeContext;
	DWORD						dwRawDataFormat;
	DWORD						dwJpegQualityFactor;		// only used when the output format is JPEG
														// value range 1-125, the smaller the better quality
} TDECCHOPTION, *PTDECCHOPTION;

typedef struct tUpdateChannelOption {
	DWORD		dwFlags;
	BOOL		bVideoOut;
	BOOL		bAudioOut;
	BOOL		bMotionAlert;		// if draw the motion detection RECT
	BOOL		bIgnoreBorder;	// if true, ignore the border and draw the full area
	DWORD		dwMotionRectAlert;	// color for motion rect when motion alert
									// if you want to specify the RGB value, please mark MSB as 1 and
									// the rest of the dword is the RGB value
	DWORD		dwMotionRect;		// color for motion rect -1 indicate that not show this rect
									// if you want to specify the RGB value, please mark MSB as 1 and
									// the rest of the dword is the RGB value
	DWORD		dwVolume;
	EMEDIATYPE  tMediaType;
	HWND		hWnd;				// the new window handle to display graph
} TUPDATECHANNELOPTION, *PTUPDATECHANNELOPTION;

typedef struct tUpdatePBChannelOption {
	DWORD		dwFlags;
	BOOL		bVideoOut;
	BOOL		bAudioOut;
	BOOL		bMotionAlert;		// if draw the motion detection RECT
	BOOL		bIgnoreBorder;	// if true, ignore the border and draw the full area
	DWORD		dwMotionRectAlert;	// color for motion rect when motion alert
									// if you want to specify the RGB value, please mark MSB as 1 and
									// the rest of the dword is the RGB value
	DWORD		dwMotionRect;		// color for motion rect -1 indicate that not show this rect
									// if you want to specify the RGB value, please mark MSB as 1 and
									// the rest of the dword is the RGB value
	DWORD		dwVolume;
	EMEDIATYPE  tMediaType;
	HWND		hWnd;				// the new window handle to display graph
	DWORD		dwFast;				// the speed factor for speed up
	DWORD		dwSlow;				// the speed factor for slow down
	BOOL		bPause;				// TRUE for pause FALSE for resume
} TUPDATEPBCHANNELOPTION, *PTUPDATEPBCHANNELOPTION; // the common part of TUPDATEPBCHANNELOPTION & TUPDATECHANNELOPTION must exactly the same

typedef struct tCurrentDisplayInfo
{
	DWORD	dwDisplayWidth;
	DWORD	dwDisplayHeight;
	DWORD	dwTimet;
	DWORD	dwFlag;			//the flag tells if the channel is resuming or audio only. 
							//EDISPLAYINFOFLAG 
}TCURRENTDISPLAYINFO, *PTCURRENTDISPLAYINFO;

typedef struct tSnapShot {
	DWORD		dwdataSize;		//data size in BYTE 
	DWORD		dwWidth;
	DWORD		dwHeight;
	BYTE *		pDataStart;
} TSNAPSHOT, * PTSNAPSHOT;

typedef struct tSnapshotCropInfo {
	WORD		wSize;
	DWORD		dwStartX;
	DWORD		dwStartY;
	DWORD		dwWidth;
	DWORD		dwHeight;	
} TSNAPSHOTCROPINFO, * PTSNAPSHOTCROPINFO;

// struct for AVICONVERTER ----------------------------------------------------------------------------------
#ifdef _WIN32
// enum
typedef enum EStartAVIChFlag {
    FIXED_VINFO_AT_FIRST_FRAME = (1<<2),
    FIXED_AINFO_AT_FIRST_FRAME = (1<<3)
} ESTARTAVICHFLAG;

// define AVI Status Code
typedef enum EAVIStatusCode
{
	AVI_WRITE_FRAME_TIME = (WM_USER + 100),
} EAVISTATUSCODE;

typedef struct tAviChannelOption
{
	LPSTATUSCALLBACK		pfStatus;
	LPDISPLAYCALLBACK		pfCaption;
    DWORD                   dwStatusContext;
    DWORD                   dwCaptionContext;
    DWORD                   dwBitCount;
} TAVICHOPTION;

typedef WAVEFORMATEX TAUDIOOPTION;

typedef struct tVideoOption
{
    DWORD   dwfccType;
    DWORD   dwHandler;
    LONG    lKey;
    LONG    lDataRate;
    LONG    lQuality;
    LPBITMAPINFO lpbiOut;
} TVIDEOOPTION;

typedef struct tBorderSize
{
    DWORD   dwHeader;
    DWORD   dwFooter;
    DWORD   dwLeft;
    DWORD   dwRight;
} TBORDERSIZE;

typedef struct tAviVideoInfo
{
    DWORD   dwWidth;
    DWORD   dwHeight;
    DWORD   dwBitCount;
} TAVIVINFO;

typedef struct tAviAudioInfo
{
    //! channel number of audio
    DWORD   dwChannels;
    //! sampling frequency
    DWORD   dwSamplesPerSec;
    //! bits per audio sample 24, 16, 8
    DWORD   dwBitsPerSample;
} TAVIAINFO;

typedef struct tAviChannelOption2
{
    TAVIVINFO   tAviVInfo;
    TAVIAINFO   tAviAInfo;
    DWORD       dwFrameRate;
    BOOL        bAudioEnable;
    BOOL        bVideoEnable;
    char        szFileName[MAX_PATH];
    TBORDERSIZE tBorder;
    DWORD       dwFlag;
} TAVICHOPTION2;
#endif // _WIN32

#ifdef __cplusplus
extern "C" {
#endif


//int DLLAPI fnAVSynchronizer(void);

SCODE DLLAPI AvSynchronizer_Initial ( HANDLE * phAvSync, LPSTATUSCALLBACK pfStatus, LPDISPLAYCALLBACK pfDisplay, HWND hWnd, DWORD dwAudioFocusType, long lSyncParameter, DWORD dwVersion, DWORD dwFlag );
SCODE DLLAPI AvSynchronizer_InitialEx ( HANDLE * phAvSync, LPSTATUSCALLBACK pfStatus, LPDISPLAYCALLBACK pfDisplay, HWND hWnd, DWORD dwAudioFocusType, long lSyncParameter, DWORD dwVersion, DWORD dwFlag, DWORD dwBlankColor );

// the DECODER_CHANNEL_ONLY & USE_DIRECTDRAW_ONLY could not be changed after initial
// org size mode could not be changed after initial
SCODE DLLAPI AvSynchronizer_SetFlags ( HANDLE hAvSync, DWORD dwFlags);
SCODE DLLAPI AvSynchronizer_GetFlags ( HANDLE hAvSync, DWORD *pdwFlags);

SCODE DLLAPI AvSynchronizer_Release ( HANDLE * phAvSync );

#ifdef _WIN32
SCODE DLLAPI AvSynchronizer_CreateChannel ( HANDLE hAvSync, HANDLE * phChannel, TCHANNELOPTION tChannelOptions );
SCODE DLLAPI AvSynchronizer_CreatePlaybackChannel ( HANDLE hAvSync, HANDLE * phPlaybackChannel, TPBCHOPTION tPbChOption );
SCODE DLLAPI AvSynchronizer_DeleteChannel ( HANDLE hAvSync, HANDLE* phChannel );
SCODE DLLAPI AvSynchronizer_StartChannel ( HANDLE hChannel, DWORD dwFlag );
SCODE DLLAPI AvSynchronizer_StopChannel ( HANDLE hChannel );

SCODE DLLAPI AvSynchronizer_UpdateChannelSettings ( HANDLE hChannel, TUPDATECHANNELOPTION tUpdateChannelOptions );
SCODE DLLAPI AvSynchronizer_UpdatePlaybackChannelSettings ( HANDLE hChannel, TUPDATEPBCHANNELOPTION tUpdatePBChannelOptions );
SCODE DLLAPI AvSynchronizer_InputEncodedMediaFrame ( HANDLE hChannel, TMediaDataPacketInfo * pMediaDataPacket );
SCODE DLLAPI AvSynchronizer_InputPlaybackMediaFrame ( HANDLE hChannel, TMediaDataPacketInfo * pMediaDataPacket );
SCODE DLLAPI AvSynchronizer_GetCurrentSnapShot ( HANDLE hChannel, PTSNAPSHOT ptSnapShot , DWORD dwFlags);
SCODE DLLAPI AvSynchronizer_GetCurrentCropSnapShot(HANDLE hChannel, PTSNAPSHOT ptSnapShot , DWORD dwFlags, PTSNAPSHOTCROPINFO ptCropInfo);
SCODE DLLAPI AvSynchronizer_GetCurrentDisplay(HANDLE hChannel, HDC hDC, TCURRENTDISPLAYINFO *ptCurrentDisplayInfo);
SCODE DLLAPI AvSynchronizer_FreePicture ( PTSNAPSHOT ptSnapShot );
SCODE DLLAPI AvSynchronizer_GetCapabilities ( HANDLE hAvSync, DWORD * pdwCapabilities, DWORD dwScreen );

#ifndef _WIN32_WCE
// AVI functions 
SCODE DLLAPI AvSynchronizer_CreateAVIChannel ( HANDLE hAvSync, HANDLE * phAVIChannel, TAVICHOPTION * ptAviChOption );
SCODE DLLAPI AvSynchronizer_CreateLiveAVIChannel ( HANDLE hAvSync, HANDLE * phAVIChannel, TAVICHOPTION * ptAviChOption );
SCODE DLLAPI AvSynchronizer_DeleteAVIChannel ( HANDLE hAvSync, HANDLE * phAVIChannel );
SCODE DLLAPI AvSynchronizer_StartAVIChannel ( HANDLE hAVIChannel, TAVICHOPTION2 * ptAviChOption2 );
SCODE DLLAPI AvSynchronizer_StopAVIChannel ( HANDLE hAVIChannel );
SCODE DLLAPI AvSynchronizer_InputAVIMedia ( HANDLE hAVIChannel, TMediaDataPacketInfo * ptMediaDataPacketInfo );
SCODE DLLAPI AvSynchronizer_InputDecodedAVIMedia ( HANDLE hAVIChannel, TMediaType eFrameType, TFRAMEINFO *ptFrameInfo);

SCODE DLLAPI AvSynchronizer_ChooseVideoCompressor(HANDLE hAvSync, HWND hWnd, const char *pszTitle, TAVIVINFO *pVideoInfo);
SCODE DLLAPI AvSynchronizer_ChooseAudioCompressor(HANDLE hAvSync, HWND hWnd, const char *pszTitle, TAVIAINFO *pAudioInfo);
SCODE DLLAPI AvSynchronizer_SetVideoCompressorParam(HANDLE hAvSync, TVIDEOOPTION * ptVideoOption);
SCODE DLLAPI AvSynchronizer_GetVideoCompressorParam(HANDLE hAvSync, TVIDEOOPTION * ptVideoOption);
SCODE DLLAPI AvSynchronizer_SetAudioCompressorParam(HANDLE hAvSync, TAUDIOOPTION * ptAudioOption);
SCODE DLLAPI AvSynchronizer_GetAudioCompressorParam(HANDLE hAvSync, TAUDIOOPTION * ptAudioOption);
SCODE DLLAPI AvSynchronizer_ChooseChannelVideoCompressor(HANDLE hAVIChannel, HWND hWnd, const char *pszTitle, TAVIVINFO *pVideoInfo);
SCODE DLLAPI AvSynchronizer_ChooseChannelAudioCompressor(HANDLE hAVIChannel, HWND hWnd, const char *pszTitle, TAVIAINFO *pAudioInfo);
SCODE DLLAPI AvSynchronizer_SetChannelVideoCompressorParam(HANDLE hAVIChannel, TVIDEOOPTION * ptVideoOption);
SCODE DLLAPI AvSynchronizer_GetChannelVideoCompressorParam(HANDLE hAVIChannel, TVIDEOOPTION * ptVideoOption);
SCODE DLLAPI AvSynchronizer_SetChannelAudioCompressorParam(HANDLE hAVIChannel, TAUDIOOPTION * ptAudioOption);
SCODE DLLAPI AvSynchronizer_GetChannelAudioCompressorParam(HANDLE hAVIChannel, TAUDIOOPTION * ptAudioOption);
#endif // !_WIN32_WCE
#endif // _WIN32


SCODE DLLAPI AvSynchronizer_CreateDecoderChannel ( HANDLE hAvSync, HANDLE * phDecChannel, TDECCHOPTION* ptDecChannelOptions );
SCODE DLLAPI AvSynchronizer_DeleteDecoderChannel ( HANDLE hAvSync, HANDLE * phDecChannel );
// to set one option for a channel one times, the channel could be any one of the three channels
// but the function will return invalid parameter if the channel could not accept
// the option
// each option has it own parameter, please see the document for more detail about this
// now the option is not overlapped with the UpdateChannelSettings series, that is
// any option could be set using the UpdateChannelSettings() or UpdatePlaybackChannelSettings()
// will not be supported by this function currently. later, the support might be added
SCODE DLLAPI AvSynchronizer_SetChannelOption ( HANDLE hChannel, DWORD dwOption, DWORD dwParam1, DWORD dwParam2 );

// input the data into decoder channel, and get the decoded result immediately
// in pfFrameInfo, please set the dwSize, pbyHeader or pbySound value to a buffer sufficient to 
// hold the data: video (max = 704 * 576 * bytes per pixel, example for bytes per pixel is 3 for bmp24)
// audio (max = 2048 * 10 bytes)
// pdwSize will contain the real size used in the decoded media.
// the tPacketInfo member in pfVideoFrameInfo or pfAudioFrameInfo is useless in these two functions
SCODE DLLAPI AvSynchronizer_DecodeVideo ( HANDLE hDecChannel, TMediaDataPacketInfo * pMediaDataPacket, TVFRAMEINFO *pfVideoFrameInfo, DWORD *pdwSize);
SCODE DLLAPI AvSynchronizer_DecodeAudio ( HANDLE hDecChannel, TMediaDataPacketInfo * pMediaDataPacket, TAFRAMEINFO *pfAudioFrameInfo, DWORD *pdwSize);

// this function will not return the decode frame, but only the frame size information and size
SCODE DLLAPI AvSynchronizer_DecodeVideoDecodeOnly ( HANDLE hDecChannel, TMediaDataPacketInfo * pMediaDataPacket, TVFRAMEINFO *pfVideoFrameInfo, DWORD *pdwSize);
SCODE DLLAPI AvSynchronizer_GetDecodedVideoFrame ( HANDLE hDecChannel, 
												  TMediaDataPacketInfo * pMediaDataPacket, 
												  TVFRAMEINFO *pfVideoFrameInfo, 
												  DWORD dwRawDataFormat,
												  DWORD dwReservedHeaderLen,
												  DWORD *pdwSize);


SCODE DLLAPI AvSynchronizer_InputToBeDecodedMediaFrame ( HANDLE hDecChannel, TMediaDataPacketInfo * pMediaDataPacket );
SCODE DLLAPI AvSynchronizer_GetVersionInfo ( BYTE* pbMajor, BYTE* pbMinor, BYTE* pbBuild, BYTE* pbRevision );

SCODE DLLAPI AvSynchronizer_GetRemainingQueueElementCount(HANDLE hChannel, TAVSYNCQUEUESTATUS *ptQueueStatus);

#ifdef __cplusplus
}
#endif



#endif // __AVSYNCHRONIZER_H__
