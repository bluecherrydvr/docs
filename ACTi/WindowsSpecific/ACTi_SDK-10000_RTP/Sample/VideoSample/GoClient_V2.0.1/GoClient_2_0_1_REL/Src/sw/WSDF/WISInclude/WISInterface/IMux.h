#pragma once

#include	<mmsystem.h>
#include	<../WSDF.h>
#include	<../MultiMedia.H>

class ICallback
{
public:
	virtual HRESULT OnStart(const char* szMsg) PURE;
	virtual HRESULT OnStatus(const char* szMsg) PURE;
	virtual HRESULT OnEnd(const char* szMsg) PURE;
};
class IMux
{
public:
	IMux(void);
	~IMux(void);
public:

	//  CreateInstance : Create an instance of IMux
	//	Parameter
	//		pp  : a pointer to the variant that receives the instance
	//		type: muxed file type, 0: AVI, 1: MPEG1, 2: MPEG2, 3: AVI using Microsoft Video For Windows API
	//		filename : muxed file name
	//	Return:
	//		0 : success
	//		not 0 : fail

	static SINT32 CreateInstance(void **pp, SINT32 type, SINT8 *filename,ICallback* pCB = NULL);

	//	Release : Release the IMux instance
	void	Release();
	void    SetMuxSpecialParameter(SINT32 para);
	//	MuxInit : Do the initializing work for mux
	//  Parameter
	//		vdoMode	:	EVideoFormat defined in MultiMedia.h
	//		vdoWidth:	video width
	//		vdoHeight:	video hieght
	//		vdoFrameRate: video frame rate
	//		vdoBitrate  : video bitrate
	//		adoMode ,	-1 : No audio
	//					0  : mp3
	//					1  : mp2
	//					2  : PCM
	//                  3  : G723
	//		adoBitrate :      audio bitrate bits/second
	//		adoChannels:	  audio channel number (1 or 2)
	//		adoSamplesPerSec: audio sampling rate
	//		adoBitsPerSample: audio bits per sample (8 or 16)
	//	Return:
	//		0 : success
	//		not 0 : fail
	virtual SINT32 MuxInit(SINT32 vdoMode, SINT32 vdoWidth, SINT32 vdoHeight, 
		REAL64 vdoFrameRate, SINT32 vdoBitrate, SINT32 adoMode, SINT32 adoBitrate, SINT32 adoChannels, 
		SINT32 adoSamplesPerSec, SINT32 adoBitsPerSample) PURE; 

	//	PackVideo : Write a video packet(frame) to file
	//  Parameter
	//		inPacket : buffer for video data 
	//		length   : length of video data		
	//		frameNumber: serial number of current frame
	//		frameType: frame type, defined in EFrameType in Multimedia.h
	//		nDecodingTimeStamp: decoding time stamp(refer to mpeg1,2,4 spec)
	//		nPlaybackTimeStamp: playback time stamp(refer to mpeg1,2,4 spec)
	//	Return:
	//		0 : success
	//		not 0 : fail
	virtual SINT32 PackVideo(UINT8 *inPacket, SINT32 length, SINT32 frameNumber, 
		SINT32 frameType, REAL64 nDecodingTimeStamp, REAL64 nPlaybackTimeStamp) PURE;

	//	PackAudio : Write an audio packet to file
	//  Parameter
	//		inPacket : buffer for video data 
	//		length   : length of video data		
	//		timeStamp: seconds elapsed from the beginning of capture
	//		nDecodingTimeStamp: decoding time stamp(refer to mpeg1,2,4 spec)
	//		nPlaybackTimeStamp: playback time stamp(refer to mpeg1,2,4 spec)
	//	Return:
	//		0 : success
	//		not 0 : fail
	virtual SINT32 PackAudio(UINT8 *inPacket, SINT32 length, REAL64 timeStamp, 
		REAL64 nDecodingTimeStamp, REAL64 nPlaybackTimeStamp) PURE;
	
	//	MuxEnd : Do the uninitializing work for mux
	virtual SINT32 MuxEnd() = 0;
protected:
	SINT32 m_siMuxSpecialParameter;
};

