/**	HEADERFILE: "WSDF/WISInclude/WISInterface/Uti_TOOL.h"
 *	Description: Tool Utilities.
 *	Section: Utilities
 *	History:
 *		05-02-2002 - Alpha, file created
 */
#ifndef	_UTI_TOOL_H_
#define	_UTI_TOOL_H_

#include	<../WSDF.H>
#include	<../Utilities.H>



/**	SECTION - constants and data exchange structures
 */

/**	ENDOFSECTION
 */



/**	SECTION - interface functions' declaration
 */
	REAL64	currSec();
	// return current system time in second

	// fast memory copy from *src to *mem by using mmx
	SINT32	copyMem(
				UINT8	*mem,		// target memory buffer
				UINT8	*src,		// source memory buffer
				SINT32	length		// buffer size
				);
	// return real bytes copied (should be same as length)

	// fast memory copy of a rectangle from *src to *tgt
	void	copyRec(
				UINT8	*tgt,		// target memory buffer
				SINT32	wtgt,		// width of target memory buffer
				UINT8	*src,		// source memory buffer
				SINT32	wsrc,		// width of source memory buffer
				SINT32	width,		// real bytes to be copied in a line
				SINT32	height		// real lines to be copied
				);

	// print a real number to a string
	void	realFmt(
				REAL64	r,			// the real number
				SINT8	*fmt,		// outside allocated string buffer
				SINT32	nPost,		// most digits after the point
				SINT32	nPrev		// aligned digits before the point
						= 0,		//   by default we don't align it
				SINT8	align		// if tail-aligned
						= false,	//   by default we don't align it
				SINT8	autoUnit	// if automatically convert to 'K' or 'M'
						= true		//   by default we do it
				);

/**	ENDOFSECTION
 */



#endif
/**	ENDOFHEADERFILE: "Uti_TOOL.h"
 */
