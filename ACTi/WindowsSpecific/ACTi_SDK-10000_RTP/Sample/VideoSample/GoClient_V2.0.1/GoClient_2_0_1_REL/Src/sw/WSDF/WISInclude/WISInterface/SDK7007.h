/**	HEADERFILE: "WSDF/WISInclude/WISInterface/SDK7007.h"
 *	Description: Provide all the configuration and operating functions in
 *		initialize stage as well as run time.
 *	Section: GO7007 SDK project
 *	History:
 *		04-21-2002 - Alpha & DJ, file created
 *		07-16-2002 - Weimin, Update interface, add I2C supporting
 */
#ifndef	_SDK7007_H_
#define	_SDK7007_H_

#include	<../WSDF.H>
#include	<../MultiMedia.H>
#include	<Uti_DSM.h>
#include	<MP_OSD.h>


/**	SECTION - constants and data exchange structures
 */
	typedef enum
	{
		ERR7007_SYSTEM	= 0x0C01,
		ERR7007_STREAM	= 0x0C02,
		ERR7007_BRCTRL	= 0x0C03,

		ERR7007_IOCHECK	= 0x0C11,
		ERR7007_MEMBIST	= 0x0C12,
		ERR7007_REGSCAN	= 0x0C13,				   
		ERR7007_RAMSCAN	= 0x0C14,

		ERR7007_OPEN_DEVICE		= 0x0C20,
		ERR7007_BOOTUP_DEVICE	= 0x0C21,
		ERR7007_INIT_DEVICE		= 0x0C22,

		ERR7007_NOT_SUPPORTED	= 0x0C30,
		ERR7007_DEVNOTOPEN		= 0x0C31,
		ERR7007_FAILACK			= 0x0C32,
		ERR7007_I2CTIMEOUT		= 0x0C33,
		ERR7007_I2CINIT			= 0x0C34,

		ERR7007_OFF		= 0x0C00
	} E7007_FuncError;

#ifdef WIN32
#define		WM_DRV7007		(0x0400 + 1000)
#endif

	typedef enum
	{
		RTN7007_REALFRM	= 0x0CC1,
		RTN7007_FAKEFRM	= 0x0CC2,
		RTN7007_NOFRM	= 0x0CC0,
		RTN7007_REALFRM_NODEC = 0x0CC3
	} E7007_FuncReturn;

	typedef enum
	{
		CFG7007_SYSTEM	= 0,
		CFG7007_STREAM	= 1,
		CFG7007_BRCTRL	= 2,
		CFG7007_ROOT	= - 1
	} E7007_CFGLevel;

	typedef enum
	{
		D7007_USB		= 1,
		D7007_EZUSB		= 2,
		D7007_HPI		= 0
	} E7007_DeviceDriver;

	typedef enum
	{
		I2C7007_NONE	= 0,
		I2C7007_I2C		= 1,
		I2C7007_SCCB	= 2,
		I2C7007_TW9901	= 3,
		I2C7007_ICMEDIA	= 4,
		I2C7007_ICM202	= 5,
		I2C7007_TW9903	= 6,
		I2C7007_SAA7114	= 7,
		I2C7007_SAA7115	= 8
	} E7007_I2CStandard;

	typedef enum
	{
		IP7007_RUNBLOCK		= 0,
		IP7007_INITUNBLOCK	= 1,
		IP7007_INITBLOCK	= 2,
		IP7007_RUNUNBLOCK	= 3
	} E7007_I2CPackage;

	typedef enum
	{
		W_ZERO	= 0,
		W_ONEB	= 1,
		I_MODE	= 2,
		R_MODE	= 3,
		S_KEEP	= 0xff
	} E7007_GPIOCommand;

	typedef enum
	{
		// following commands are for setting the chip interface mode through EZUSB

		// set the interface to HPI mode
		// pInBuf  : NULL
		// pOutBuf : NULL

		CMD7007_HPIMODE,
		
		// set the interface to USB mode
		// pInBuf  : NULL
		// pOutBuf : NULL
		
		CMD7007_USBMODE,
		
		// set the interface to standby mode
		// pInBuf  : NULL
		// pOutBuf : NULL
		
		CMD7007_STANDBY,					  

		// following commands are for self test

		// start a hardware mbist
		// pInBuf  : NULL
		// pOutBuf : NULL
		
		CMD7007_HARDWAREMBIST,
		
		// start a basic hpi check
		// pInBuf  : NULL
		// pOutBuf : NULL
		
		CMD7007_BASICHPICHECK,

		// start an io check
		// pInBuf  : NULL
		// pOutBuf : NULL
		
		CMD7007_IOCHECK,
		
		// start an software mbist
		// pInBuf  : NULL
		// pOutBuf : NULL
		
		CMD7007_SOFTWAREMBIST,
		
		// start an register scan
		// pInBuf  : NULL
		// pOutBuf : NULL

		CMD7007_REGSCAN, 
		
		// start an memory scan
		// pInBuf  : NULL
		// pOutBuf : NULL
		
		CMD7007_MEMSCAN,

		// following commands are for chip debugging

		// read a HPI register
		// pInBuf  : a word that holds HPI address
		// pOutBuf : a word that holds read result data
		
		CMD7007_READHPIREGITER,	
		
		// write a HPI register
		// pInBuf  : two words that hold address and data to write
		// pOutBuf : NULL
		
		CMD7007_WRITEHPIREGITER,

		// read cbus registers via firmware
		// pInBuf  : word[0] = number of register, followed by all address(word for each)
		// pOutBuf : 
		
		CMD7007_READCBUSREGFW,	
		
		// write cbus registers via firmware
		// pInBuf  : word[0] = number of register, followed by all address(word)-data(word) pairs
		// pOutBuf : all read result data, word for each
		
		CMD7007_WRITECBUSREGFW,
		
		// read a cbus register via debug mode
		// pInBuf  : a word that holds cbus address
		// pOutBuf : a word that holds read result data
		
		CMD7007_READCBUSREGDBG,	
		
		// write a cbus register via debug mode
		// pInBuf  : two words that hold address and data to write
		// pOutBuf : NULL
		
		CMD7007_WRITECBUSREGDBG,

		// set the chip to debug mode 
		// pInBuf  : NULL
		// pOutBuf : NULL
		
		CMD7007_ENTERDEBUGMODE,
		
		// set the chip to no-debug mode 
		// pInBuf  : NULL
		// pOutBuf : NULL
		
		CMD7007_LEAVEDEBUGMODE,

		// set the power state  
		// pInBuf  : *((unsigned int *)pInBuf) = power state
		// pOutBuf : NULL

		CMD7007_SETPOWERSTATE,

	} E7007_DeviceCommand;

	typedef struct
	{
		SINT8	name[64];		// configuration name
		SINT8	desc[256];		// configuration description
	} TCFGHEADER7007;

	/**	-------------------------------------------------------------------------------------------
	 */
	typedef enum
	{
		VLSI7007_ASIC	= 1,
		VLSI7007_FPGA	= 2
	} E7007_VLSI;

	typedef enum
	{
		DRAM7007_4M		= 1,
		DRAM7007_8M		= 2
	} E7007_DRAM;

	typedef struct
	{
		TCFGHEADER7007	hdr;	// name of configuration for system setting

		SINT8	sensor_type;	// sensor type, see 'E7007_I2CStandard'
		SINT8	VLSImode;		// chip/FPGA mode
		SINT32	clockrate;		// chip/FPGA running clock rate (Hz)
								//   sample: 96000000
		SINT8	DRAM;			// on-chip DRAM size, see E7007_DRAM
		REAL64	timeout;		// input timeout alert in seconds
								//   sample: 3.3

		SINT32	sensor_cps;		// sensor clock rate (Hz)
								//   sample: 27000000
		SINT8	sensor_656;		// if input format is 656 mode
		SINT8	sensor_href;	// h reference 0 or 1 valid
		SINT8	sensor_vref;	// v reference 0 or 1 valid
		SINT32	sensor_h;		// sensor valid pixels in horizontal
								//   sample: 720
		SINT32	sensor_v;		// sensor valid pixels in vertical
								//   sample: 480
		SINT32	sensor_hperiod;	// sensor period in horizontal
								//   sample: 1716 (NTSC)
		SINT32	sensor_vperiod;	// sensor period in vertical
								//   sample: 525  (NTSC)
		SINT8	sensor_pformat;	// 'WSDF.H': EPixelMode
		SINT32	reserved;
	} TCFGSYSTEM7007;

	/**	-------------------------------------------------------------------------------------------
	 */
	typedef enum
	{
		GO7007_X_SCALE	= 1,
		GO7007_Y_SCALE	= 2,
		GO7007_XYSCALE	= 3,
		GO7007_NOSCALE	= 0
	} E7007_PictureScale;

	typedef enum
	{
		GO7007_MIDIAN	= 1,
		GO7007_LOWPASS	= 2,
		GO7007_NOFILTER	= 0
	} E7007_PreFilter;

	typedef struct
	{
		TCFGHEADER7007	hdr;	// name of configuration for stream information

		SINT8	sequence;		// 'WSDF.H': ESequenceMode
		SINT8	compress_mode;	// 'WSDF.H': EVideoFormat
		SINT32	width;			// output stream resolution: horizontal size
		SINT32	height;			// output stream resolution: vertical size

		SINT8	subsample;		// If do sub-sampling after pixel input (before process)
		SINT8	scale_enb;		// enable/disable vip scaling, see E7007_PictureScale
		SINT8	hfilter_mode;	// enable/disable vip x-prefiltering, see E7007_PreFilter
		SINT8	vfilter_mode;	// enable/disable vip y-prefiltering, see E7007_PreFilter
		SINT8	hflt_weight[3];	// weight values for horizontal filter
								//   sample: { 4, 8, 4 }
		SINT8	vflt_weight[3];	// weight values for vertical filter
								//   sample: { 4, 8, 4 }

		SINT8	FGS_ELBP;		// FGS enhanced layer bit planes (0-3), only 0 (no FGS) is valid
		SINT8	scnchg_quota;	// scene change if these percent of macroblocks are I-blocks
								//   sample: 50

		SINT8	user_qmatrix;	// if use user defined quantize tables
		SINT8	intra_qtab[64];	// intra quantize table if user_qmatrix = 1
		SINT8	inter_qtab[64];	// inter quantize table if user_qmatrix = 1
		UINT8	search_range;	// motion search range
		SINT8	sub_mode;		// only valid if compress_mode is MPEG4, 0: WIS, 1: MS, 2: DivX, 3: Sigma Design
		SINT8	four_channel;	// 1: enable 4 channels, 0: one channel
		SINT8	interlace_coding;	// 0: progressive coding, 1: interlacing coding
		SINT8	is_closed_gop;	// 0: open GOP, 1: closed GOP
//		SINT8	reserved[2];	// reserved for future use
	} TCFGSTREAM7007;

	/**	-------------------------------------------------------------------------------------------
	 */
	typedef struct
	{
		TCFGHEADER7007	hdr;	// name of configuration for bitrate control

		SINT32	target_rate;	// target bitrate (bps), a non-zero value enables WIS adaptive bitrate control
								// 0 means no bit rate control
		REAL64	base_fps;		// base output framerate (fps)
		SINT32	GOP;			// maximum count of pictures in a group of picture

		SINT8	IQ;				// fixed quantize scale of I frames (only for target_rate = 0)
		SINT8	PQ;				// fixed quantize scale of P frames (only for target_rate = 0)
		SINT8	BQ;				// fixed quantize scale of B frames (only for target_rate = 0)

		SINT32	time_period;	// bitrate control session (in seconds)	(only for tartget_rate != 0)
								//   for recommended values see 'MultiMedia.H' - RCTP_xxx
		REAL64	LAMBDA;			// [0, 1], from best picture quality to fixed framerate
								//   for recommended values see 'MultiMedia.H' - RCLAMBDA_xxx
								// (only for tartget_rate != 0)
		SINT8	comm;			// if it's for a communication application
								// (only for tartget_rate != 0)
		SINT32	reserved;
	} TCFGBRCTRL7007;

	/**	-------------------------------------------------------------------------------------------
	 */
	typedef struct
	{
		TCFGSYSTEM7007	syscfg;	// configuration for system setting
		TCFGSTREAM7007	strcfg;	// configuration for stream information
		TCFGBRCTRL7007	ctlcfg;	// configuration for bitrate control
	} TSDKCFG7007;

	/**	-------------------------------------------------------------------------------------------
	 */
	typedef struct
	{
		SINT32	address;
		SINT32	asize;
		SINT32	value;
		SINT32	vsize;
	} TI2CREG7007;

/**	ENDOFSECTION
 */

typedef struct _VIDEO_TIME_INFO
{
	unsigned int TotalDelay;
	unsigned int ChipDelay;
	unsigned int FrameInBuf;
	unsigned int VIPFrameNum;
	unsigned long LastChipTick;

} VIDEO_TIME_INFO;

typedef enum
{
	NT_FATALERROR,
	NT_TIMEOUT,
	NT_SNAPSHOT,
	NT_REPORTTS,
	NT_DEVICEREMOVED=0x0100,
	NT_TOOMUCHDELAY,
}NOTIFY_TYPE;

/**	SECTION - interface functions' declaration for C++
 */
#ifdef __cplusplus
	class ISDK7007
	{
	public:
		STATIC	SINT32  GetDeviceNumber();

		public:
		// constructor
		STATIC	SINT32	CreateInstance(
					void	**pp,			// return a pointer to CSDK7007 object
					UINT8	*initData,		// binary packages of GO7007 initial data (partial)
					SINT32	ninitSize,		// size of initData
					SINT8   instance,
					SINT8	drv				// see E7007_DeviceDriver
							= D7007_USB		//   by default we use on-chip USB mode
					);
		// return EFuncReturn ('WSDF.H')

		// distructor
		void	Release();

		// to set a system configuration
		virtual	SINT32	SystemSetting(
					TCFGSYSTEM7007	*psys	// non-zero pointer to system-CFG
					) PURE;
		// return EFuncReturn ('WSDF.H')

		// to set a stream configuration
		virtual	SINT32	StreamSetting(
					TCFGSTREAM7007	*pstr,	// non-zero pointer to stream-CFG
					SINT8	dDraw,			// 'MultiMedia.H': EDDrawMode
					SINT32	widthShow		// width of display line in bytes (direct draw pitch)
					) PURE;
		// return EFuncReturn ('WSDF.H')

		// to set a bitrate (and optionally apply) a CFG
		virtual	SINT32	BitrateSet(
					TCFGBRCTRL7007	*pctl,	// non-zero pointer to brctrl-CFG
					SINT8	enable			// if apply new CFG immediately
							= 0				//   by default we don't apply new CFG
					) PURE;
		// return EFuncReturn ('WSDF.H') or E7007_FuncError

		virtual	SINT32	DeviceExist() PURE;

		//0: confused, 1: Sanyo, 2: Ovt, 3: tw9901, 4: icm205b, 5: icm202, 6: tw9903
		virtual SINT32  GetSensorId() PURE;

		// to start encoding by given CFG
		virtual	SINT32	StartDevice() PURE;					 
		// return EFuncReturn ('WSDF.H') or E7007_FuncError			  

		// to stop and close device
		virtual	void	CloseDevice() PURE;

		// to send a command to device, for parameters please refer to enum E7007_DeviceCommand
		virtual	SINT32	DeviceSendCommand(
					SINT32	command,	
					UINT8	*pInBuf,
					UINT8	*pOutBuf
					) PURE;
		// return EFuncReturn ('WSDF.H') or E7007_FuncError

		// to set a device event receiver
		virtual	void	DeviceSetReceiver(
					SINT32	receiverHandle	// in MS-OS we use a HWND (a window handle)
					) PURE;

		// require to decode/transcode a frame
		virtual	SINT32	GetOneFrame(
					UINT8	*mxStr,			// pre-allocated frame buffer for transcoded stream
					SINT32	*mxNum,			// return mxStr size
					TMP_FrmInfo	*frmInfo,	// return information of current frame, see above
											//   NULL indcates no return information needed
					UINT8	*decodeBuf,		// buffer for decoded frame
											//   NULL indcates no buffer copy needed
					REAL64  *decodeTS,		// decode time stamp
											//   NULL indcates no decode timestamp information needed
					UINT8	*rgbBuf			// Snapshot RGB buffer of most recent frame
							= NULL,			//   NULL indcates no snapshot buffer needed 
					UINT8	*yuv411			// Snapshot YUV 411 buffer of most recent frame
							= NULL,			//	 NULL indicates no snapshot YUV buffer needed
					SINT32	TSCOption		// transcoding options, see 'multimedia.h': ETranscodeOption
							= 0				// 0 means do normal transcoding with all options
					) PURE;
		// return E7007_FuncReturn

		// Get information of delay and encoding
		virtual SINT32	GetFrameNumInfo(
					VIDEO_TIME_INFO *pvti
					) PURE;	
		// return SUCCESS if successful
		// return ERR7007_OFF if device is not opened.


		// require a go stream frame
		virtual	SINT32	GetOneGoFrame(
					UINT8	**goStr,			// pre-allocated frame buffer for GO stream
					SINT32	&goNum			// return goStr size
					) PURE;
		// return E7007_FuncReturn

		// set the on-screen-display text into the next gop header of the transcoded stream.
		// this is optional operation and currently only implemented with MPEG4 and GO stream
		// if there is no option specified, default setting is used ( bottom aligned vertically
		// , center aligned horizontally and shaded text etc. )
		// if option is not specified then the on-screen-display text is at maximum 128 bytes, 
		// including the trailing 0; if option is specified, the on-screen-display text is at 
		// maximum 118 bytes, including the trailing 0;
		virtual SINT32 SetCurrentOnScreenDisplayText(SINT8* szText) PURE;
		virtual SINT32 SetOnScreenDisplayOptions(OSD_DISPLAY_OPTIONS option, UINT8 bPackOSDOpt = 0) PURE;

		// require the stream sequence header
		virtual SINT32	GetSequenceHead(
					UINT8	**SequenceHead,		// a pointer to the buffer of sequence header
					SINT32	&SequenceHeadLen	// length, in bytes, of sequence header
					) PURE;
		// return RTN7007_REALFRM if successful.
		// return RTN7007_NOFRM if failed
		// return ERR7007_OFF if device is not opened.


		// configurate decoding/transcoding/display options at any time
		virtual	void	Configurate(
					SINT8	DecLvl			// 'MULTIMEDIA.H': ESequenceMode
							= FP_KEEP,		//   by default we keep old value
											//   0 for no decoding on any type of frames
											//   FP_AUTO will set to IPBDROP (decode dropt frames)
					SINT8	TSCLvl			// 'MULTIMEDIA.H': ESequenceMode
							= FP_KEEP,		//   by default we keep old value
											//   0 for no encoding on any type of frames
											//   FP_AUTO will set to IPBDROP (encode dropt frames)
					TMP_PPrcSet	*ppSet		// postprocess setting
							= NULL			//   by default will set pproc level to DPP_DBLK and
											//   set pproc area same as picture area
					) PURE;

		// The following function is for I2C sensor control

		// Force the chip to encode an I frame immediately
		virtual SINT32 ForceIntraFrame() PURE;
		// return SUCCESS
		// return ERR7007_DEVNOTOPEN if the device is not opened.
		// return ERR7007_BOOTUP_DEVICE if the chip does not boot-up

		// initialize I2C
		virtual SINT32 I2CInitialize(
					SINT8	i2c,			// sensor type, see E7007_I2CStandard
					SINT8	dev_addr,		// 7-bit sensor device address
					TI2CREG7007	*i2cData,	// I2C data for sensor registers
					SINT32	ni2cSize		// size of i2cData
					) PURE;
		// return SUCCESS or other value for fail

		// generate start signal samples, one byte per sample
		virtual SINT32 StartSignal(
					UINT8 *pack				// outside allocated buffer to hold the samples
					) PURE;
		// return number of samples

		// generate stop signal samples, one byte per sample
		virtual SINT32 StopSignal(
					UINT8 *pack				// outside allocated buffer to hold the samples
					) PURE;
		// return number of samples

		//generate samples of writing 8 bits data, one byte per sample
		virtual SINT32 W8bits(
					UINT8	*pack,			// outside allocated buffer to hold the samples
					UINT8	data,			// 8-bit data
					SINT8	rAck			// if reading ACK required
					) PURE;
		// return number of samples

		//generate samples of reading 8 bits data, one byte per sample
		virtual SINT32 R8bits(
					UINT8	*pack,			// outside allocated buffer to hold the samples
					SINT8	wAck			// if writing ACK required
					) PURE;
		// return number of samples
		
		// according to the GPIO signal samples, generate GPIO package that can be sent to the firmware.
		virtual SINT32 GpioPackage(
					UINT8 *packBuff,		// outside allocated buffer to hold the GPIO package
					UINT8 *packData,		// input buffer holding the data of samples.
					SINT32 size,			// size of samples
					SINT32 mode				// GPIO package mode, see 'E7007_I2CPackage'
					) PURE;
		// return the length, in word (16 bits), of the GPIO package.


		// according to status of those four GPIO Pins, make a command byte that is ready to put into GPIO package.
		// see 'E7007_GPIOCommand' for available status
		virtual UINT8 MakeGpioCmdByte(
					UINT8 gpio_0 = S_KEEP,	// status of GPIO PIN 0, default value is to keep previous value
					UINT8 gpio_1 = S_KEEP,	// status of GPIO PIN 1, default value is to keep previous value
					UINT8 gpio_2 = S_KEEP,	// status of GPIO PIN 2, default value is to keep previous value
					UINT8 gpio_3 = S_KEEP	// status of GPIO PIN 3, default value is to keep previous value
					) PURE;
		// return the command byte
		
		// Send a GPIO package to firmware
		virtual SINT32 SendGpioPackage(
					UINT8 *packBuf,			// input buffer holding the GPIO package 
					SINT8 mode,				// GPIO package mode, see 'E7007_I2CPackage'
					SINT8 rw = 0			// Specify if this package is to read(1) or write register value,
											// It is only valid when the GPIO package mode is IP7007_RUNBLOCK
					) PURE;
		// return SUCCESS if package is sent to firmware successfully
		// return ERR7007_DEVNOTOPEN if the GO7007 device is not open
		// return ERR7007_BOOTUP_DEVICE if the chip does not boot-up
		// return ERR7007_FAILACK if failed to read ACK
		// return ERR7007_I2CTIMEOUT if timeout

		// writing a register
		virtual SINT32 I2CWriteRegister(
					UINT8	dev_addr,		// device address (7 bits)
					UINT8	*reg_addr,		// a buffer holding the address of register. 
											// The address can be multi bytes
					SINT32	asize,			// number of bytes of the register address value
					UINT8	*reg_value,		// a buffer holding the register value to be written
											// The value can be multi bytes
					SINT32	vsize,			// number of bytes of the register value to be written
					SINT8	mode			// mode of I2C package, see enum 'E7007_I2CPackage'
							= IP7007_RUNBLOCK,
					SINT8	i2c				// standard of I2C, see enum 'E7007_I2CStandard'
							= I2C7007_I2C
					) PURE;
		// return SUCCESS if written successfully
		// return ERR7007_DEVNOTOPEN if the GO7007 device is not open
		// return ERR7007_BOOTUP_DEVICE if the chip does not boot-up
		// return ERR7007_FAILACK if failed to read ACK
		// return ERR7007_I2CTIMEOUT if timeout

		// reading a register
		virtual SINT32 I2CReadRegister(
					UINT8	dev_addr,		// device address (7 bits)
					UINT8	*reg_addr,		// a buffer holding the address of register. 
											// The address can be multi bytes
					SINT32	asize,			// number of bytes of the register address value
					UINT8	*reg_value,		// buffer of the register value read from 'reg_addr'
											// The value can be multi bytes
					SINT32	vsize,			// number of bytes of the read buffer
					SINT8	mode			// mode of I2C package, see enum 'E7007_I2CPackage'
							= IP7007_RUNBLOCK,
					SINT8	i2c				// standard of I2C, see enum 'E7007_I2CStandard'
							= I2C7007_I2C
					) PURE;
		// return SUCCESS if read successfully
		// return ERR7007_DEVNOTOPEN if the GO7007 device is not open
		// return ERR7007_BOOTUP_DEVICE if the chip does not boot-up
		// return ERR7007_FAILACK if failed to read ACK
		// return ERR7007_I2CTIMEOUT if timeout

		// for 4-CIF mode
		// set the SDK into splitter mode or reset to nomal mode
		virtual void SetSplitterMode(SINT32 bSplit) PURE;

		// require to decode/transcode a frame in splitted mode
		virtual	SINT32	GetOneSplittedFrame(
					UINT8	*mxStr[4],			// pre-allocated frame buffer for transcoded stream
					SINT32	*mxNum[4],			// return mxStr size
					TMP_FrmInfo	*frmInfo,	// return information of current frame, see above
											//   NULL indcates no return information needed
					UINT8	*decodeBuf[4],		// buffer for decoded frame
											//   NULL indcates no buffer copy needed
					REAL64  *decodeTS,		// decode time stamp
											//   NULL indcates no decode timestamp information needed
					SINT32	TSCOption		// transcoding options, see 'multimedia.h': ETranscodeOption
					) PURE;
		// return E7007_FuncReturn
		
		// require a go stream frame in splitted mode
		virtual	SINT32	GetOneSplittedGoFrame(
					UINT8	**goStr[4],			// pre-allocated frame buffer for GO stream
					SINT32*	goNum[4]			// return goStr size
					) PURE;
		// return E7007_FuncReturn		
	};

	/**	This class is special for configurations' management.
	 */

	class ICFG7007
	{
		public:
		// constructor
		STATIC	SINT32	CreateInstance(
					void	**pp,			// return a pointer to CCFG7007 object
					UINT8	*cfg			// memory buffer of the whole pre-defined CFG tree
					);

		// distructor
		void	Release();

		// to get all sub-CFG names under a given CFG (root, system-CFG or stream-CFG)
		virtual TCFGHEADER7007*	GetSubCFGNameList(
					TDSM_NodePrefix	*pfx,	// outside allocated buffer to receive CFG prefix
					SINT32	lvl,			// see E7007_CFGLevel
											//   CFG7007_ROOT  :
											//     get all system-CFG names
											//   CFG7007_SYSTEM:
											//     get all stream-CFG names under a system-CFG
											//   CFG7007_STREAM:
											//     get all brctrl-CFG names under a stream-CFG
					SINT32	syscfgIndex		// index of system-CFG
							= FP_KEEP,		//   by default we use current system-CFG
											//   FP_AUTO will choose first system-cfg
					SINT32	strcfgIndex		// index of stream-CFG
							= FP_KEEP,		//   by default we use current stream-CFG
											//   FP_AUTO will choose first stream-cfg
					SINT32	ctlcfgIndex		// index of brctrl-CFG
							= FP_KEEP		//   by default we use current brctrl-CFG
											//   FP_AUTO will choose first brctrl-cfg
					) PURE;
		// return a non-zero pointer of inside allocated name list (need delete outside)

		// to select (and optionally apply) a CFG
		virtual SINT32	SelCFG(
					TSDKCFG7007		*cfg,	// outside allocated buffer to receive selected CFG
											//   NULL is acceptable (will not return CFG data)
					SINT32	syscfgIndex		// index of system-CFG
							= FP_KEEP,		//   by default we use current system-CFG
											//   FP_AUTO will choose first system-cfg
					SINT32	strcfgIndex		// index of stream-CFG
							= FP_KEEP,		//   by default we use current stream-CFG
											//   FP_AUTO will choose first stream-cfg
					SINT32	ctlcfgIndex		// index of brctrl-CFG
							= FP_KEEP		//   by default we use current brctrl-CFG
											//   FP_AUTO will choose first brctrl-cfg
					) PURE;
		// return EFuncReturn ('WSDF.H') or EDSM_FuncError ('Uti_DSM.h') or E7007_FuncError

		// to export current CFG tree
		virtual UINT8*	CFGExport(
					SINT32	&cfgSize		// return size of CFG tree data
					) PURE;
		// return a pointer of inside allocated buffer of CFG tree (need delete outside)
	};

#endif

/**	ENDOFSECTION
 */



#endif
/**	ENDOFHEADERFILE: "SDK7007.h"
 */
