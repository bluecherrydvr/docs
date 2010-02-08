/**	HEADERFILE: "WSDF/WISInclude/WISInterface/MP_BRCTRL.h"
 *	Description: Bitrate control interface for video compression.
 *	Section: MultiMedia - Motion Picture
 *	History:
 *		04-15-2002 - Alpha & Weimin, file created
 */
#ifndef	_MP_BRCTRL_H_
#define	_MP_BRCTRL_H_

#include	<../WSDF.H>
#include	<../MultiMedia.H>



/**	SECTION - constants and data exchange structures
 */
	typedef struct
	{
		SINT32	target_rate;	// target bitrate (bps)
								//   sample: 128000
		SINT32	time_period;	// bitrate control session (in seconds)
								//   see 'MultiMedia.h' - RCTP_xxx
								//   sample: 30 for internet (more constant bitrate), or
								//           600 for movie recording (more constant quality)
		REAL64	LAMBDA;			// [0, 1], 0 for best picture quality, 1 for fixed-framerate mode
								//   see 'MultiMedia.h' - RCLAMBDA_xxx
								//   sample: 0.25 (recommend)
		SINT8	comm;			// if it's for a communication application
								//   sample: 0 (non-communication mode, which is more accurate)
		REAL64	base_fps;		// output stream will not have higher framerate than it (fps)
								//   sample: 15 (fixed output framerate if LMBDA = 1)
		SINT32	update_period;	// Q & dropframe update every this count of frames (drop included)
								//   sample: 1 (recommend)
	} TMP_BRCTRL;

/**	ENDOFSECTION
 */



/**	SECTION - interface functions' declaration for C++
 */
#ifdef __cplusplus
	class IMP_BRCTRL
	{
		SINT32 id;				// handle pointing to a Crctrl object

		public:
		// constructor
		// see 'rctrlInit'
				IMP_BRCTRL(
					REAL64	fps,			// original framerate (fps)
					SINT32	f_area,			// area of video frames (in pixels)
					SINT32	ini_frm_no		// initial frame number of coming stream
							= 1				//   usually starting from 1
					);

		// distructor
		// see 'rctrlFree'
				~IMP_BRCTRL();

		// re-setup bitrate control information by defined structure
		// see 'rctrlSetup'
		void	Setup(
					TMP_BRCTRL	*info		// as defined above
					);

		// re-setup bitrate control information by all the parameters
		void	Setup(
					SINT32	target_rate,	// as defined above
					SINT32	time_period		// as defined above
							= FP_KEEP,		//   by default we keep old value
											//   FP_AUTO will set to RCTP_DEFAULT
					REAL64	LAMBDA			// as defined above
							= FP_KEEP,		//   by default we keep old value
											//   FP_AUTO will set to RCLAMBDA_HQPIC
					SINT8	comm			// as defined above
							= FP_KEEP,		//   by default we keep old value
											//   FP_AUTO will set to 0 (non-communication mode)
					REAL64	base_fps		// as defined above
							= FP_KEEP,		//   by default we keep old value
											//   FP_AUTO will set base_fps = fps
					SINT32	update_period	// as defined above
							= FP_KEEP		//   by default we keep old value
											//   FP_AUTO will set to 1 (always update)
					);

		// LMBDA can be changed at any time to adjust the balance of picture quality and framerate
		// see 'rctrlSetLAMBDA'
		void	SetLAMBDA(
					REAL64	LAMBDA			// as defined above
							= FP_AUTO		//   we automatically choose RCLAMBDA_HQPIC (0.25)
					);

		// see 'rctrlInit'
		SINT32	GetIQ();
		// return equivalent quantize scale for IFRAMEs

		// see 'rctrlInit'
		SINT32	GetPQ();
		// return equivalent quantize scale for PFRAMEs

		// see 'rctrlInit'
		SINT32	GetBQ();
		// return equivalent quantize scale for BFRAMEs

		// see 'rctrlGetDropframe'
		SINT8	GetDropframe();
		// return dropframe count (0-based)

		// we must update bitrate control system everytime when a new frame data received
		// see 'rctrlUpdate'
		void	Update(
					SINT32	fbits,			// bits count of new frame
					SINT32	fno,			// frame number
					SINT8	ftype,			// frame type (IPB)
					REAL64	fq				// equivalent quantize scale of new frame
					);
	};
#endif

/**	ENDOFSECTION
 */



/**	SECTION - interface functions' declaration for ANSI C
 */
#ifdef __cplusplus
	extern "C"
	{
#endif
		// return a handle pointing to the Crctrl object, the parameter 'id' below
		SINT32	rctrlInit(REAL64 fps, SINT32 f_area, SINT32 ini_frm_no);
		void	rctrlFree(SINT32 id);

		void	rctrlSetup(SINT32 id, TMP_BRCTRL *info);
		void	rctrlSetLAMBDA(SINT32 id, REAL64 LAMBDA);

		SINT32	rctrlGetIQ(SINT32 id);
		SINT32	rctrlGetPQ(SINT32 id);
		SINT32	rctrlGetBQ(SINT32 id);
		SINT8	rctrlGetDropframe(SINT32 id);

		void	rctrlUpdate(SINT32 id, SINT32 fbits, SINT32 fno, SINT8 ftype, REAL64 fq);

#ifdef __cplusplus
	}
#endif

/**	ENDOFSECTION
 */



#endif
/**	ENDOFHEADERFILE: "MP_BRCTRL.h"
 */
