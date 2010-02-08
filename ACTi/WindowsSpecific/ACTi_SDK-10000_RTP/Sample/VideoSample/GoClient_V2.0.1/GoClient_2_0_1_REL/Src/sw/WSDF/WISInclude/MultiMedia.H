/**	HEADERFILE: "WSDF/WISInclude/MultiMedia.H"
 *	Description: MultiMedia related definitions.
 *	History:
 *		04-29-2002 - Alpha, file created
 */
#ifndef	_MULTIMEDIA_H_
#define	_MULTIMEDIA_H_



/**	SECTION - constants
 */
	typedef enum
	{
		MMSF_RAW		= 0,
		MMSF_AVI_WIS	= 1,
		MMSF_AVI_DIVX	= 2,
		MMSF_AVI_SIGMA	= 3,
		MMSF_AVI_MSFT	= 4,
		MMSF_MPEG1		= 5,
		MMSF_MPEG2		= 6,
		MMSF_WMV		= 7,
		MMSF_WMA		= 8,
		MMSF_MP2		= 9,
		MMSF_MP3		= 10,
		MMSF_NONE		= -1
	} EMMStrFormat;

	typedef enum
	{
		AUDIO_MP1	= 0x01,
		AUDIO_MP2	= 0x02,
		AUDIO_MP3	= 0x03,
		VOICE_G7231	= 0x14
	} EAudioFormat;

	typedef enum
	{
		MPEG1		= 0x00,
		MPEG2		= 0x01,
		H261		= 0x02,
		H263		= 0x03,
		MPEG4		= 0x04,
		MPEG4XGO	= 0x05,
		MPEG2X4		= 0x06,
		MOTIONJPEG	= 0x08,
		DV			= 0x09,
		H26L		= 0x20,
		GO			= 0x40
	} EVideoFormat;

	typedef enum
	{
		UYVYP		= 0,
		UYVYI		= 1,
		BAYER_GB	= 2,
		BAYER_GR	= 3,
		BAYER_BG	= 4,
		BAYER_RG	= 5
	} EPixelMode;

	typedef enum
	{
		DDRAW_YUY2	= 0x01,
		DDRAW_UYVY	= 0x02,
		DDRAW_YV12	= 0x08,
		DDRAW_RGB24	= 0x03,
		DDRAW_RGB32	= 0x04,
		DDRAW_RGB555= 0x05,
		DDRAW_RGB565= 0x06,
		DDRAW_DIB24	= 0x13,
		DDRAW_DIB32	= 0x14,
		DDRAW_DIB555= 0x15,
		DDRAW_DIB565= 0x16,
		DDRAW_NULL	= 0
	} EDDrawMode;

	typedef enum
	{
		IONLY		= 1,
		IPONLY		= 2,
		IPB			= 3,
		IPBDROP		= 4
	} ESequenceMode;

	typedef enum
	{
		I_FRAME		= 0,
		P_FRAME		= 1,
		B_FRAME		= 2,
		D_FRAME		= 3
	} EFrameType;

	typedef enum
	{
		MB_INTRA	= 0x01,
		MB_PATTERN	= 0x02,
		MB_BACKWARD	= 0x04,
		MB_FORWARD	= 0x08,
		MB_QUANT	= 0x10,
		MB_4V		= 0x20,
		MB_DIRECT	= 0x40
	} EMBType;

	typedef enum
	{
		PIXEL_XI_YI	= 0x00,
		PIXEL_XH_YI	= 0x01,
		PIXEL_XI_YH	= 0x10,
		PIXEL_XH_YH	= 0x11
	} EFractionPixel;

	typedef enum
	{
		FLHALF		= 1,
		FLQUATER	= 2,
		FLEIGHTH	= 3,
		FLINTEGER	= 0
	} EFractionLevel;

	typedef enum
	{
		NOSEQUENCE_HEADER	= 0x0001,
		NOGOP_HEADER		= 0x0002,
		NOFRAME_HEADER		= 0x0004,
		NOFRAME_TAIL		= 0x0008,
		NOSEQUENCE_TAIL		= 0x0010,
		NOIFRAME			= 0x0020,
		NOPFRAME			= 0x0040,
		NOBFRAME			= 0x0080
	} ETranscodeOption;

	#define	FPS_N30				(30000. / 1001)
	#define	FPS_P25				25.
	#define	FPS_N24				(24000. / 1001)
	#define	FPS_P24				24.

	#define RC_MIN_Q			1
	#define RC_MAX_Q			31

	#define	DPP_NULL			0
	#define	DPP_DBLKLUM			3
	#define	DPP_DBLK			5
	#define	DPP_DBLKDRNGLUM		6
	#define	DPP_DBLKDRNG		9

	#define	RCTP_CBR			30
	#define	RCTP_DEFAULT		60
	#define	RCTP_HIQUALITY		300

	#define	RCLAMBDA_HQPIC		0.25
	#define	RCLAMBDA_LESSDROP	0.5
	#define	RCLAMBDA_CFR		1.

/**	ENDOFSECTION
 */



/**	SECTION - data structures
 */
	typedef struct
	{
		char	interlace;		// 0 for progressive or 1 for interlace orignal input
		char	mode;			// 'MultiMedia.H': EVideoFormat
		char	seq;			// see ESequenceMode (IONLY, IPONLY or IPB)
		int		cols;			// count of macroblocks in horizontal
		int		rows;			// count of macroblocks in vertical
		double	fps;			// frame rate of current stream
		char	uvmode;			// see EVideoFormat (for uv motion vectors)
		char	dqmode;			// see EVideoFormat (for DCTQ mismatch control)
								//   GO (0x40) indicates no iQ-mismatch-control and use JDCT
		char	fpmode;			// see EFractionLevel
								//   by default we only use FLHALF
		char	wismode;		// Is it WIS proprietary MPEG4 mode? Yes(1) : No(0)
		char	acpred;			// If do AC prediction in intra macro-block
		char	userq;			// if use user customized quantization table(1: Yes, 0: No)
		unsigned char	intraq[64];		// quantization table for intra block
		unsigned char	interq[64];		// quantization table for inter block
	} TMP_StrInfo;

	typedef struct {
		double	timeStamp;		// return time stamp of current frame, in second
		int	nGOP;			// serial number of current GOP
		int	nSubGOP;		// serial number of current SubGOP in GOP
		int	nPicture;		// serial number of current Picture in SubGOP
		double	fno;			// serial number of current frame (dropt frames counted)
		char	ftype;			// see EFrameType
		double	fq;				// equivalent frame quantize scale
	} TMP_FrmInfo;

	typedef struct
	{
		int	col;			// macroblock index in horizontal
		int	row;			// macroblock index in vertical
		int	dc[6];			// DC coefficients of each 8x8 block
		int	vx[6];			// motion vector (horizontal) of each 8x8 block
		int	vy[6];			// motion vector (vertical) of each 8x8 block
		char	mbtype;			// see EMBType
		char	Q;				// quantize scale of current macroblock
		char	cbp;			// CBP of current macroblock
		int	ach[6][7];		// first row AC coefficients of each 8x8 block in current MB
		int	acv[6][7];		// first column AC coefficients of each 8x8 block in current MB
	} TMP_BlkInfo;

	typedef struct
	{
		char	run[6][64];		// run values
		int	lvl[6][64];		// level values
		int	pair[6];		// count of pairs
	} TMP_BlkPair;

	typedef struct {
		char	pproc;			// postprocess level, see 'WSDF.H'
		int	rcLeft;			// postprocess area position: left boundary
		int	rcTop;			// postprocess area position: top boundary
		int	rcWidth;		// postprocess area size: horizontal
		int	rcHeight;		// postprocess area size: vertical
	} TMP_PPrcSet;

	typedef struct
	{
		int	SBYT;			// macroblock head byte of frame stream
		int	EBYT;			// macroblock tail byte of frame stream

		char	SBIT;			// position of head bit in SBYT
		char	EBIT;			// position of tail bit in EBYT

		char	Q;				// quantizer
		char	I;				// 0 for intra MB, 1 for inter MB

		short	GOBN;			// GOB index
		short	MBA;			// MB index of current GOB

		short	hmvp00;			// X-direction motion vector predictor of blk00
		short	vmvp00;			// Y-direction motion vector predictor of blk00
		short	hmvp10;			// X-direction motion vector predictor of blk10
		short	vmvp10;			// Y-direction motion vector predictor of blk10
	} TMP_StrMBInfo;

/**	ENDOFSECTION
 */



#endif
/**	ENDOFHEADERFILE: "MultiMedia.H"
 */
