/**	HEADERFILE: "WSDF/WISInclude/WSDF.H"
 *	Description: Base headerfile to be included in any WSDF project.
 *	History:
 *		04-14-2002 - Alpha, file created
 */
#ifndef	_WSDF_H_
#define	_WSDF_H_



/**	SECTION - for debug only
 */


#ifdef	WIN32
	#if ( defined(_DEBUG) && !defined(DRIVER) )
		#include	<stdio.h>
		#include	<windows.h>

		#define	OUTPUTDEBUGSTRING(str)	OutputDebugString(str)
	#else
		#define	OUTPUTDEBUGSTRING(str)
	#endif
	#define MMX_SUPPORT	1
#else
	#define MMX_SUPPORT	0
#endif


/**	ENDOFSECTION
 */



/**	SECTION - data types (platform dependant)
 */
	typedef	float				REAL32;
	typedef	double				REAL64;

	typedef	unsigned char		UINT8;
	typedef	char				SINT8;
	typedef	unsigned short		UINT16;
	typedef	short				SINT16;

#if ( defined(WIN32) )
	
	typedef	unsigned int		UINT32;
	typedef	int					SINT32;
	typedef	unsigned __int64	UINT64;
	typedef	__int64				SINT64;

#elif ( defined(_LINUX) )

	typedef	unsigned int		UINT32;
	typedef	int					SINT32;
	typedef void				*PVOID;
	typedef void				VOID;

#elif ( defined(SPARC) || defined(BIG_ENDIAN) )
	
	typedef uint32_t			UINT32;
	typedef int32_t				SINT32;
	typedef uint64_t			UINT64;

#else

	typedef u_int32_t			UINT32;
	typedef int32_t				SINT32;
	typedef u_int64_t			UINT64;

#endif

/**	ENDOFSECTION
 */



/**	SECTION - symbols
 */
	#define	Mptr	qword ptr
	#define	Tptr	dword ptr
	#define	CNT		edx

	#define STATIC	static
	#define CONST	const

#ifdef	WIN32
	#define	INLINE	__forceinline
#else
	#define INLINE	inline
#endif

#ifndef	NULL
	#define NULL	0
#endif

#ifndef	PURE
	#define PURE	= 0
#endif

/**	ENDOFSECTION
 */



/**	SECTION - constants
 */
	typedef enum
	{
		FP_KEEP		= - 1,
		FP_AUTO		= - 2
	} EFuncParameter;

	typedef enum
	{
		SUCCESS		= 0,
		ERR_DEVICE	= 0x0001,
		ERR_MEMORY	= 0x0002,
		ERR_FILE	= 0x0003,
		ERR_ACCESS	= 0x0004,
		ERR_MISMCH	= 0x0005,
		ERR_TIMING	= 0x0006,
		ERR_PARAMETER = 0x0007,
		ERR_UNKNOWN	= - 1
	} EFuncReturn;

/**	ENDOFSECTION
 */



/**	SECTION - quick functions
 */
	#define	INLSRC(src)			<../../WISDevelopSpace/WISLib/src>
	// used in inline interface:
	// #include INLSRC(SProj_inline/source.cpp)
	// note: 'WSDF/WISInclude/WISInterface' always set as include path in WSDF projects

	#define	MEM32(mem)			(mem + 32 - ((UINT32)mem & 31))
	// this macro helps to get more optimized memory pointer

	#define	MIN(a, b)			((a) < (b) ? (a) : (b))
	#define	MAX(a, b)			((a) > (b) ? (a) : (b))
	#define	ABS(x)				((x) < 0 ? - (x) : (x))
	#define	CLP(x, min, max)	MAX(MIN((x), (max)), (min))
	#define	ROUND(x)			((SINT32)floor((x) + 0.5))

	#define	LET(x, param, dft)	if((param) != FP_KEEP) x = ((param) == FP_AUTO) ? (dft) : (param);
	#define	MODPLUS(x, i, mod)	if((x += i) >= (mod)) x -= (mod);
	// this macro helps the analysis of function parameters

/**	ENDOFSECTION
 */



#endif
/**	ENDOFHEADERFILE: "WSDF.H"
 */
