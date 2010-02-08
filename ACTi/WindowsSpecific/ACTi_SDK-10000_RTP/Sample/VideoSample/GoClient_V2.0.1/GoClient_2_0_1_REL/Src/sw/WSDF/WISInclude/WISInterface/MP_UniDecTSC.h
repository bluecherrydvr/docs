/**	HEADERFILE: "WSDF/WISInclude/WISInterface/MP_UniDecTSC.h"
 *	Description: Universal Video Decoder & Transcoder for GO, MPEG1/2/4 and H261/H263.
 *	Section: MultiMedia - Motion Picture
 *	History:
 *		05-05-2002 - Alpha, file created
 */
#ifndef	_MP_UNIDECTSC_H_
#define	_MP_UNIDECTSC_H_

#include	<../WSDF.H>
#include	<../MultiMedia.H>



/**	SECTION - constants and data exchange structures
 */

/**	ENDOFSECTION
 */



/**	SECTION - interface functions' declaration for C++
 */
#ifdef __cplusplus
	class IMP_UniDecTSC
	{
		public:
		// adjusting DecLvl to fit current situation
		STATIC	SINT8	AdjDecLvl(
					SINT8	DecLvl,			// original decoding level
					SINT8	adjDecLvl,		// current adjusted decoding level
					SINT32	bufferredFrm,	// current latency (undecoded buffer)
					SINT32	*adjTab,		// brackets of bufferred frames to keep system smooth
											//   sample: { 5, 10 15, 20 }
					SINT8	ftype			// current frame type
					);
		// return adjusted DecLvl

		// constructor
		STATIC	SINT32	CreateInstance(
					IMP_UniDecTSC	**pp,	// return a pointer to CUniDecTSC object
					SINT8	modeSrc,		// source video format
					SINT8	modeTSC,		// transcoded video stream format
					SINT8	modeWIS,		// Is it a WIS proprietary MPEG4 stream?
					TMP_StrInfo	*sInfo		// optional pass stream information
							= NULL			//   by defualt we use Sync to get stream information
					);
		// return EFuncReturn ('WSDF.H')

		// distructor
		void	Release();

		// to synchronize input stream and get all necessary information
		virtual	SINT32	Sync(
					TMP_StrInfo	*sInfo,		// return stream information if success
											//   NULL is acceptable
											//   but usually outside needs its cols/rows and fps
					SINT32	*numTSC			// optional return bytes used for transcoded stream
							= NULL,			//   by defualt we return nothing
					UINT8	*ptr4HdrTSC		// optional set a buffer to receive transcoded stream
							= NULL,			//   by default we use internal output buffer

				/**	Following parameters are unnecessary if using internal buffer management, and
				 *	use SourceInput (see below) to input data.
				 */
					SINT32	*numSrc			// optional return bytes used in synchronization
							= NULL,			//   by defualt we return nothing
					UINT8	*ptr4HdrSrc		// optional pass a stream header
							= NULL			//   by default we use internal bufferred input data
					) PURE;
		// return SUCCESS or
		//        ERR_MISMCH (failed or not enough bufferred data)

		// to setup decoding/transcoding after get synchronized
		virtual SINT32	UniDecTSCSetup(
					SINT8	dDraw,			// 'MULTIMEDIA.H': EDDrawMode
					SINT8	DecLvl			// 'MULTIMEDIA.H': ESequenceMode
							= FP_AUTO,		//   FP_AUTO will set to IPB
											//   0 for no decoding on any type of frames
					SINT8	TSCLvl			// 'MULTIMEDIA.H': ESequenceMode
							= FP_AUTO,		//   FP_AUTO will set to IPB
											//   0 for no encoding on any type of frames
					TMP_PPrcSet	*ppSet		// postprocess setting
							= NULL,			//   by default will set pproc level to DPP_NULL and
											//   set pproc area same as picture area
					SINT32	*fifoLvl		// brackets of bufferred frames to keep system smooth
							= NULL,			//   NULL will keep old value
											//   we initialize it as
											//     { 0, 10000, 20000, 30000, 40000 }

				/**	Following parameters are unnecessary if no mask-clipping output required.
				 *	Only output a macroblock if its clipping value equals to its mask id.
				 */
					SINT32	id				// mask id
							= 0,			//   by default set to 0
					SINT8	*clipTbl		// clipping table of mask
							= NULL,			//   by default set to NULL (no mask)
					SINT32	mbln			// macroblock count of clipping table line
							= - 1			//   will set mbln same as horizontal macroblock count
					) PURE;
		// return SUCCESS or
		//        ERR_ACCESS (not synchronized)

		// to decode/transcode next single frame
		virtual	SINT32	FRMDecTSC(
					REAL64	*timeStamp,		// return time-stamp of displayed picture
											//   NULL is acceptable if no decoding is required
					UINT8	*ddMem,			// return displayed picture to this Direct-Draw buffer
											//   NULL is acceptable if no decoding is required
					SINT32	ddWidth,		// row length of ddMem (Direct-Draw pitch)

					TMP_FrmInfo	*fInfo,		// return decoded frame header if success
											//   NULL is acceptable
											//   but usually outside needs its fno/timestamp or fq
					SINT32	*numTSC			// optional return bytes used for transcoded frames
							= NULL,			//   by defualt we return nothing
											//   numTSC[0] indicates numbers of frames returned
											//   numTSC[i] indicates bytes of ith frame (i > 0)
					UINT8	*ptr4FrmTSC		// optional set a buffer to receive transcoded frame
							= NULL,			//   by default we use internal output buffer

				/**	Following parameters are unnecessary if using internal buffer management, and
				 *	use SourceInput/GetStrTSC (see below) to send/receive data.
				 */
					SINT32	*numSrc			// optional return bytes used in decoding this frame
							= NULL,			//   by defualt we return nothing
					UINT8	*ptr4FrmSrc		// optional pass a complete frame data
							= NULL			//   by default we use internal bufferred input data
					) PURE;
		/*Max add 2004-04-01 for adding an parameter*/
		virtual	SINT32	FRMDecTSC(
					REAL64	*timeStamp,		// return time-stamp of displayed picture
											//   NULL is acceptable if no decoding is required
					UINT8	*ddMem,			// return displayed picture to this Direct-Draw buffer
											//   NULL is acceptable if no decoding is required
					SINT32	ddWidth,		// row length of ddMem (Direct-Draw pitch)

					TMP_FrmInfo	*fInfo,		// return decoded frame header if success
											//   NULL is acceptable
											//   but usually outside needs its fno/timestamp or fq
					UINT32	frmLen,			//	the length of ptr4FrmSrc
					SINT32	*numTSC			// optional return bytes used for transcoded frames
							= NULL,			//   by defualt we return nothing
											//   numTSC[0] indicates numbers of frames returned
											//   numTSC[i] indicates bytes of ith frame (i > 0)
					UINT8	*ptr4FrmTSC		// optional set a buffer to receive transcoded frame
							= NULL,			//   by default we use internal output buffer

				/**	Following parameters are unnecessary if using internal buffer management, and
				 *	use SourceInput/GetStrTSC (see below) to send/receive data.
				 */
					SINT32	*numSrc			// optional return bytes used in decoding this frame
							= NULL,			//   by defualt we return nothing
					UINT8	*ptr4FrmSrc		// optional pass a complete frame data
							= NULL			//   by default we use internal bufferred input data
					) PURE;		
		// return SUCCESS or ERR_UNKNOWN (no error but also no data output) or
		//        ERR_ACCESS (not synchronized) or
		//        ERR_MISMCH (failed) or
		//        ERR_MEMORY (not enough bufferred data)

		// to duplicate last decoded frame
		virtual	SINT32	Duplicate(
					UINT8	*ddMem,			// return displayed picture to this Direct-Draw buffer
					SINT32	ddWidth			// row length of ddMem (Direct-Draw pitch)
					) PURE;
		// return SUCCESS or ERR_UNKNOWN (no error but also no data output) or
		//        ERR_ACCESS (not synchronized)

		// Set transcoding options
		virtual void	SetTSCOption(
					SINT32 TSCOption		// transcoding options, see 'MULTIMEDIA.H': ETranscodeOption
					) PURE;

		// configurate decoding/transcoding/display options at any time
		virtual	void	Configurate(
					SINT8	DecLvl			// 'MULTIMEDIA.H': ESequenceMode
							= FP_KEEP,		//   by default we keep old value
											//   0 for no decoding on any type of frames
											//   FP_AUTO will set to IPB
					SINT8	TSCLvl			// 'MULTIMEDIA.H': ESequenceMode
							= FP_KEEP,		//   by default we keep old value
											//   0 for no encoding on any type of frames
											//   FP_AUTO will set to IPB
					TMP_PPrcSet	*ppSet		// postprocess setting
							= NULL			//   by default will set pproc level to DPP_NULL and
											//   set pproc area same as picture area
					) PURE;

		// fill internal source buffer from input stream
		virtual	SINT32	SourceInput(
					UINT8	*srcStr,		// input buffer of source stream
											//   NULL indicates reset (need re-sync)
					SINT32	quota			// valid bytes of srcStr
					) PURE;
		// return SUCCESS or
		//        ERR_MEMORY (buffer almost overflow, failed to input)

		// ask for FIFO information
		virtual	REAL64	GetFiFoRatio(
					SINT32	*bufferredFrm	// return bufferred frame header in FIFO
							= NULL			//   NULL indicates no return value required
					) PURE;
		// return ratio (from 0 to 1) of occupied FIFO buffer

		// ask for and return macroblock bit-stream information
		virtual	TMP_StrMBInfo*	GetStrMBInfo() PURE;

		// return Snapshot bitmap buffer
		virtual SINT32	GetSnapShotBMP(UINT8 *bmp, SINT8 rgb24or32) PURE;

		// return Snapshot YUV411 buffer
		virtual SINT32	GetSnapShotYUV411(UINT8 *yuv411) PURE;

		// get the on-screen-display text from last gop header from decoded stream. 
		// this is optional operation and currently only implemented with MPEG4 and GO stream
		virtual SINT32 GetCurrentOnScreenDisplayText(UINT8* data) PURE;

		// set the on-screen-display text into the next gop header of the transcoded stream.
		// this is optional operation and currently only implemented with MPEG4 and GO stream
		virtual SINT32 SetCurrentOnScreenDisplayText(UINT8* data) PURE;
	};



	/**	Multi-Channel Player - can accept multi video streams and decode/mux to a single buffer
	 */
	class IMP_MCHNLPlayer
	{
		public:
		// constructor
		STATIC	SINT32	CreateInstance(
					IMP_MCHNLPlayer	**pp,	// return a pointer to CMCHNLPlayer object
					SINT32	channels,		// number of channels
					SINT8	*mode,			// source video format of all channels
					SINT32	*offsetX,		// x-offset of each channel in final display screen
					SINT32	*offsetY,		// y-offset of each channel in final display screen
					SINT8	dDraw,			// Direct-Draw mode
					SINT32	ddWidth,		// row length of display screen (Direct-Draw pitch)
					SINT32	ddHeight		// row length of display screen (Direct-Draw pitch)
					);
		// return EFuncReturn ('WSDF.H')

		// distructor
		void	Release();

		// to reset a channel
		virtual	void	ReSetChnl(
					SINT32	chnl,			// which channel to reset
					SINT8	mode			// new video format of this channel
							= FP_KEEP,		//   by default we keep old value
					SINT32	offsetX			// x-offset this channel
							= FP_KEEP,		//   by default we keep old value
					SINT32	offsetY			// y-offset this channel
							= FP_KEEP,		//   by default we keep old value
					SINT32	*fifoLvl		// brackets of bufferred frames to keep system smooth
							= NULL,			//   NULL will keep old value
											//   we initialize it as
											//     { 0, 10000, 20000, 30000, 40000 }
											//   or for realtime applications we suggest
											//     { 0, 5, 10, 15, 20 }
					REAL64	ratioSpeed		// ratio of playing speed
							= 1				//   by default we keep stream speed
					) PURE;

		// fill internal source buffer from input stream
		virtual	SINT32	SourceInput(
					SINT32	chnl,			// which channel to input
					UINT8	*srcStr,		// input buffer of source stream
											//   NULL indicates reset (need re-sync)
					SINT32	quota			// valid bytes of srcStr
					) PURE;
		// return SUCCESS or
		//        ERR_MEMORY (buffer almost overflow, failed to input)

		// ask for FIFO information of a channel
		virtual	REAL64	GetFiFoRatio(
					SINT32	chnl,			// which channel's information to get
					SINT32	*bufferredFrm	// return bufferred frame header in FIFO
							= NULL			//   NULL indicates no return value required
					) PURE;
		// return ratio (from 0 to 1) of occupied FIFO buffer

		// configurate decoding/display options at any time (will apply to all channel)
		virtual	void	Configurate(
					SINT8	DecLvl			// 'MULTIMEDIA.H': ESequenceMode
							= FP_KEEP,		//   by default we keep old value
											//   0 for no decoding on any type of frames
											//   FP_AUTO will set to IPB
					TMP_PPrcSet	*ppSet		// postprocess setting
							= NULL			//   by default will set pproc level to DPP_NULL and
											//   set pproc area same as picture area
					) PURE;

		// to decode next frame
		virtual	SINT32	DecRequest(
					REAL64	TS,				// video render current time-stamp
					REAL64	*timeStamp,		// return next video render time-stamp
					UINT8	*ddMem,			// return displayed picture to this Direct-Draw buffer
					SINT32	*chnlResult		// result of each channel's decoding
											//   SUCCESS or
											//   ERR_ACCESS (not synchronized) or
											//   ERR_MISMCH (synchronization lost) or
											//   ERR_MEMORY (not enough bufferred data) or
											//   ERR_TIMING (channel time-stamp is ahead of TS)
					) PURE;
		// return SUCCESS if new pictures decoded or
		//        ERR_ACCESS (no channel synchronized yet) or
		//        ERR_MEMORY (not enough buffer, don't change TS next time) or
		//        ERR_TIMING (all channel time-stamps are ahead of TS)

		// to get number of channel that given point belongs to
		virtual	SINT32	GetChannelNo(
					SINT32	x,				// x position of the point
					SINT32	y				// y position of the point
					) PURE;
		// return channel no, - 1 indicates the point is out of all channels
	};
#endif

/**	ENDOFSECTION
 */



#endif
/**	ENDOFHEADERFILE: "MP_UniDecTSC.h"
 */
