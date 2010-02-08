/**	HEADERFILE: "WSDF/WISInclude/WISInterface/MM_SYNC.h"
 *	Description: Synchronization patch for streaming video/audio.
 *	Section: MultiMedia - Media Mux
 *	History:
 *		08-27-2002 - Alpha, file created
 */
#ifndef	_MM_SYNC_H_
#define	_MM_SYNC_H_

#include	<../WSDF.H>
#include	<../MultiMedia.H>



/**	SECTION - constants and data exchange structures
 */

/**	ENDOFSECTION
 */



/**	SECTION - interface functions' declaration for C++
 */
#ifdef __cplusplus
	class IMM_SyncPatch
	{
		public:
		// constructor
		STATIC	SINT32	CreateInstance(
					IMM_SyncPatch	**pp,	// return a pointer to CSyncPatch object
					REAL64	fps,			// video frame rate
					SINT32	gop,			// video GOP size
					REAL64	aratio,			// audio-realtime ratio
					SINT32	vNO_preload		// video preload frames
							= 0,			//   by default we set to 0
					REAL64	aTS_preload		// audio preload time in second
							= 0				//   by default we set to 0
					);
		// return EFuncReturn ('WSDF.H')

		// distructor
		void	Release();

		// realtime updating audio progress
		virtual	void	SetAudioTS(
					REAL64	StartTS,		// most recent audio buffer start timestamp
					REAL64	EndTS			// most recent audio buffer end timestamp
					) PURE;

		// realtime require and adjust video progress
		virtual SINT32	GetVideoTS(
					SINT8	ftype,			// see EFrameType
					SINT32	fcount,			// numbers of frames returned by FRMDecTSC
					SINT32	ino,			// most recent IFRAME fno reported by firmware
					SINT32	*vNOx,			// adjusted video NO
					REAL64	*vTSx			// adjusted video timestamp
							= NULL			//   NULL indicates no timestamp needed
					) PURE;
		// return FALSE for drop or TRUE for coding
	};
#endif

/**	ENDOFSECTION
 */



#endif
/**	ENDOFHEADERFILE: "MM_SYNC.h"
 */
