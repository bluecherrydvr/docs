// VideoCapture.h: interface for the CVideoCapture class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _VIDEOCAPTURE_H_
#define _VIDEOCAPTURE_H_

#include <WSDF.H>
#include <Struct.h>

#if !defined(_WINDOWS)

typedef struct tagRECT
{
    SINT32  left;
    SINT32  top;
    SINT32  right;
    SINT32  bottom;
} RECT;

#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum ERR_CODE_CAPTURE
{
	ERR_NOMOREFRAME = 0x1000,		// no more frame is available
	ERR_DEVICESTATE, 				// error device state for a particular operation
	ERR_INITSENSOR,
	ERR_INITDEVICE,
	ERR_I2CFAIL,
	ERR_SPIFAIL,
};


typedef struct _TFrameInfo
{
	SINT32 VideoLength;
	SINT32 VBILength;
	SINT32 AudioFingerprint;
	SINT32 VIPNumber;
	SINT32 AudioTimestamp;
	SINT32 FrameType;
}TFrameInfo;

#define MAX_VBI_LENGTH (672*4)


// Event : 0 -- report time information
//       : 1 -- high priority interrupt
//       : 2 -- low priority interrupt
//       : 3 -- remote wakeup
//       : 4 -- motion detected in given regions
//
//       : 100 -- device disconnected
//       : 101 -- device fatal error

typedef void WIS_NOTIFY_CALLBACK(void *pContext, int Event, void *pParam);


//////////////////////////////////////////////////////////////////////////
//
//	The functionality of IVideoCapture include:
//
//		(1) Manipulating chip (start capturing, stop capturing, set capture parameters, get video stream from chip)
//		(2) I2C (Read/Write I2C register)
//		(3) OSD (download OSD character bitmap to chip, pass the index of the characters to be displayed to chip while chip is running)
//		(4) GPIO (Read/Write GPIO signal)
//		(5) Set motion detection parameters
//		(6) Notification (detect event from chip and pass it to the application)

class IVideoCapture  
{
public:

	// Device

	// get the total number of connected device 
	// 
	// parameter:
	//		none
	// return:	
	//		number of connected device 
	STATIC SINT32 GetDeviceNumber();

	// create an instance for this interface
	// 
	// parameter:
	//      pp	     -- [Out] pointer to the new instance
	//		instance -- index number of the connected device, should between 0 and the return value of GetDeviceNumber() - 1
	// return:	
	//		SUCCESS	   -- success
	//		ERR_MEMORY -- no enough memory to create an new instance
	//   	ERR_DEVICE -- device can not be opened
	STATIC SINT32 CreateInstance(VOID **pp, SINT8 instance);

	// release the instance for this interface
	// 
	// parameter:
	//		none
	// return:	
	//		none
	VOID Release();
	
	// determine if a device is present
	// 
	// parameter:
	//		none
	// return: 
	//      0 -- device not connected
	//		1 -- device connected
	virtual SINT32 DeviceExist() PURE;

	// set a user-provided callback function, the SDK will call this function when an event occurs.
	// 
	// parameter:
	//		Callback -- a pointer to a user-provided callback function, refer to the type definition "WIS_NOTIFY_CALLBACK"
	//		pContext -- a pointer which will be passed to the callback function
	// return: 
	//		none
	virtual void SetNotifyCallbackFunc(WIS_NOTIFY_CALLBACK *Callback, void *pContext) PURE;

	// Video Capture

	// Get resolution of sensor, the destination size in stream setting should not exceed this resolution
	// 
	// parameter:
	//		pWidth  -- [Out] Sensor width
	//		pHeight -- [Out] Sensor height
	// return: 
	//		none
	virtual void GetSensorResolution(SINT32 *pWidth, SINT32 *pHeight) PURE;

	// Set the stream level setting. The setting will not take effect until next call of StartCapturing
	// 
	// parameter:
	//		pstr -- a pointer to an instance of structure TCFGSTREAM, refer to the comment of "TCFGSTREAM"
	// return: 
	//		none
	virtual void SetStreamSetting(TCFGSTREAM *pstr) PURE;

	virtual void SetResolutionSetting(TCFGRESOLUTION *pres) PURE;

	// Set the bitrate level setting, this function can be called while recording, the setting will take effect immediately
	// 
	// parameter:
	//		pctl -- a pointer to an instance of structure TCFGBRCTRL, refer to the comment of "TCFGBRCTRL"
	// return: 
	//		none
	virtual void SetBitrateSetting(TCFGBRCTRL *pctl) PURE;

	// start capturing video from chip using the pre-set stream and bitrate setting
	// 
	// parameter:
	//		none
	// return: 
	//      SUCCESS         -- success
	//      ERR_DEVICESTATE -- chip is not in idle state
	//      ERR_INITSENSOR  -- initialize sensor failed
	//      ERR_INITDEVICE	-- initialize chip failed
	virtual SINT32 StartCapturing() PURE;

	// Stop capturing
	// 
	// parameter:
	//		none
	// return: 
	//      SUCCESS         -- success
	//      ERR_DEVICESTATE -- chip is not in running state
	virtual SINT32 StopCapturing() PURE;

	// Get one video frame from chip, this function works in block mode, it will not return until a frame is avaible or StopCapturing is called
	// 
	// parameter:
	//		pBuf   -- Buffer for receiving the video stream
	//		BufLen -- [In]  Buffer length, caller is responsible to allocate a buffer big enough
	// return: 
	//		SUCCESS         -- got one frame successfully
	//      ERR_DEVICESTATE -- chip is not in running state
	//		ERR_DEVICE      -- read device error
	//      ERR_NOMOREFRAME -- no frame in chip, in the case StopCapturing is called.				
	virtual SINT32 GetOneFrame(UINT8 *pBuf, SINT32 BufLen, TFrameInfo *pFI) PURE;

	// force the chip to create an intra frame immediately
	// 
	// parameter:
	//		none
	// return:	
	//		none
	virtual void ForceIntraFrame() PURE;

	// I2C

	// Write an I2C register
	// 
	// parameter:
	//		DevAddr  --  device address
	//		AddrLen  --  bits number of register address, can be 8 or 16
	//		RegAddr  --  register address, if its higher byte is 0, the register is considered as with 8 bits address
	//					  otherwise, the address width is 16 bits	
	//		Data	 --  value to be written to the i2c register
	//		I2C_mode --  i2c mode, can be IM_I2C or IM_SCCB
	// return: 
	//      SUCCESS     -- success
	//		ERR_I2CFAIL -- write i2c failed	
	virtual SINT32 I2C_WriteRegister(UINT8 DevAddr, SINT32 AddrLen, UINT16 RegAddr, UINT8 Data, SINT32 I2C_mode) PURE;

	// Read an I2C register
	// 
	// parameter:
	//		DevAddr  --  device address
	//		AddrLen  --  bits number of register address, can be 8 or 16
	//		RegAddr  --  register address, if its higher byte is 0, the register is considered as with 8 bits address
	//					  otherwise, the address width is 16 bits	
	//		pData    --  [Out] the value read from i2c register
	//		I2C_mode --  i2c mode, can be IM_I2C or IM_SCCB
	// return: 
	//      SUCCESS     -- success
	//		ERR_I2CFAIL -- read i2c failed	
	virtual SINT32 I2C_ReadRegister(UINT8 DevAddr, SINT32 AddrLen, UINT16 RegAddr, UINT8 *pData, SINT32 I2C_mode) PURE;

	// Write multiple continuous I2C registers (burst mode)
	// 
	// parameter:
	//		DevAddr      --  device address
	//		AddrWidth    --  bits number of register address, can be 8 or 16
	//		StartRegAddr --  address of the first register, if its higher byte is 0, the register is considered as with 8 bits address
	//					     otherwise, the address width is 16 bits	
	//      RegNum	     --  number of registers to be written
	//		pData        --  an array of values to be written to the i2c registers
	//		I2C_mode     --  i2c mode, can be IM_I2C or IM_SCCB
	// return: 
	//      SUCCESS      -- success
	//		ERR_I2CFAIL  -- write i2c failed	
	virtual SINT32 I2C_BurstWriteRegister(UINT8 DevAddr, SINT32 AddrWidth, UINT16 StartRegAddr, SINT32 RegNum, UINT8 *pData, SINT32 I2C_mode) PURE;

	// Read multiple continuous I2C registers (burst mode)
	// 
	// parameter:
	//		DevAddr      --  device address
	//		AddrWidth    --  bits number of register address, can be 8 or 16
	//		StartRegAddr --  address of the first register, if its higher byte is 0, the register is considered as with 8 bits address
	//					     otherwise, the address width is 16 bits	
	//      RegNum	     --  number of registers to be read
	//		pData        --  an array which will receive the value read from i2c registers
	//		I2C_mode     --  i2c mode, can be IM_I2C or IM_SCCB
	// return: 
	//      SUCCESS      -- success
	//		ERR_I2CFAIL  -- read i2c failed	
	virtual SINT32 I2C_BurstReadRegister(UINT8 DevAddr, SINT32 AddrWidth, UINT16 StartRegAddr, SINT32 RegNum, UINT8 *pData, SINT32 I2C_mode) PURE;


	// SPI

	// Write a SPI register
	//
	// parameter:
	//      OpCodeLen    -- operation code length in bit, can be 1 - 8 
	//      OpCode       -- operation code
	//      AddrLen      -- register address length in bit, can be 1 - 16
	//      RegAddr      -- register address
	//      DataLen      -- data length in bit, can be 0 - 16
	//      Data         -- data to write
	//		SPI_mode     -- spi mode , 0 - 3 : none 3-wire mode,  0x10 : 3-wire mode
	// return: 
	//      SUCCESS     -- success
	//		ERR_SPIFAIL -- write spi failed	
	virtual SINT32 SPI_WriteRegister(SINT32 OpCodeLen, UINT8 OpCode, SINT32 AddrLen, UINT16 RegAddr, SINT32 DataLen, UINT16 Data, SINT32 SPI_mode) PURE;

	// Write a SPI register
	//
	// parameter:
	//      OpCodeLen    -- operation code length in bit, can be 1 - 8 
	//      OpCode       -- operation code
	//      AddrLen      -- register address length in bit, can be 1 - 16
	//      RegAddr      -- register address
	//      DataLen      -- data length in bit, can be 0 - 16
	//      pData        -- [Out] the value read from spi register
	//		SPI_mode     -- spi mode , 0 - 3 : none 3-wire mode,  0x10 : 3-wire mode
	// return: 
	//      SUCCESS     -- success
	//		ERR_SPIFAIL -- read spi failed	
	virtual SINT32 SPI_ReadRegister(SINT32 OpCodeLen, UINT8 OpCode, SINT32 AddrLen, UINT16 RegAddr, SINT32 DataLen, UINT16 *pData, SINT32 SPI_mode) PURE;
	
	virtual SINT32 SPI_BurstWriteRegister(SINT32 OpCodeLen, UINT8 OpCode, SINT32 AddrLen, UINT16 StartRegAddr, SINT32 DataLen, UINT16 *pData, SINT32 SPI_mode) PURE;
	
	virtual SINT32 SPI_BurstReadRegister(SINT32 OpCodeLen, UINT8 OpCode, SINT32 AddrLen, UINT16 StartRegAddr, SINT32 DataLen, UINT16 *pData, SINT32 SPI_mode) PURE;

	// OSD (On Screen Display) operations

	// Set the bitmaps of OSD character set, refer to "OSD_bmp_format.doc" for the format of character bitmaps
	// This function should be called for at least one time if OSD operations will be taken. The default bitmaps are all blank if no bitmap is set.
	// The OSD bitmap buffer can hold up to 256 character bitmaps while index 0 is reserved for blank. This function can be called for multiple times,
	// later calling will overwrite the bitmaps of former calling. This function can not be called while capturing. 
	// 
	// parameter:
	//		CharacterMap -- pointer to buffer that hold the character bitmaps
	//		StartIndex   -- start index of the character bitmaps to be set, this parameter should be between 1 and 255
	//		Number       -- number of character bitmaps to be set, this parameter should be between 1 and 256-StartIndex
	// return: 
	//      none
	virtual void OSD_SetCharacterMap(UINT8 *CharacterMap, SINT32 StartIndex, SINT32 Number) PURE;

	// Set the index for the characters to be displayed. This function can be called before or during capturing. 
	// The characters that exceed the video size will be omitted.
	//
	// parameter:
	//		x      -- x coordinate for displaying characters, must be integral times of 16
	//		y      -- y coordinate for displaying characters, must be integral times of 16
	//      Index  -- pointer of an array of index of characters to be displayed 
	//      Number -- number of characters to be displayed 
	// return: 
	//      none
	virtual void OSD_DisplayCharacters(SINT32 x, SINT32 y, UINT8 *Index, SINT32 Number) PURE;

	// Clear the display in a rectangle region, the region can exceed the video size, the exceeded part will be omitted
	// 
	// parameter:
	//		x      -- x coordinate of the upper-left corner of the region to be cleared
	//      y      -- y coordinate of the upper-left corner of the region to be cleared
	//      Width  -- width of the region to be cleared 
	//      Height -- height of the region to be cleared
	// return: 
	//      none
	virtual void OSD_ClearRegion(SINT32 x, SINT32 y, SINT32 Width, SINT32 Height) PURE;

	// GPIO	(General-Purposed I/O) operations, these operations can be taken before or during capturing

	// Toggle the signal on a GPIO pin
	// 
	// parameter:
	//		Index -- index of GPIO pin, should be between 0 and 5 (there are 6 GPIO pins in 7007SB)
	//      Value -- should be 0 or 1 
	// return: 
	//      SUCCESS       -- success
	//		ERR_DEVICE    -- device operation failed
	//      ERR_PARAMETER -- bad parameter for Index and Value
	virtual SINT32 GPIO_Write(SINT32 Index, SINT32 Value) PURE;

	// Read the signal on a GPIO pin
	// 
	// parameter:
	//		Index  -- index of GPIO pin, should be between 0 and 5 (there are 6 GPIO pins in 7007SB)
	//      pValue -- [Out] value 
	// return: 
	//      SUCCESS       -- success
	//		ERR_DEVICE    -- device operation failed
	//		ERR_PARAMETER -- bad parameter for Index and pValue
	virtual SINT32 GPIO_Read(SINT32 Index, SINT32 *pValue) PURE;

	// Trigger a serial of operations on one or multiple GPIO pins
	// 
	// parameter:
	//		IndexNum --	the number of GPIO pins to be manipulated
	//      Index	 -- array of index of GPIO pins to be manipulated
	//      WorkItem -- a bit stream which represents the work to be done on all GPIO pins whose indexes are listed in Index
	//                  each two bits represent one operation, the value can be
	//                  00 -- write 0
	//					01 -- write 1
	//					10 -- keep
	//					11 -- read 
	//      Interval --	interval time in minisecond between the two continuous operations
	//      Result 	 -- [Out] data read from GPIO pins, each bit holds the result of one read operation 
	// return: 
	//      SUCCESS    -- success
	//		ERR_DEVICE -- device operation failed
	virtual SINT32 GPIO_BatchWork(SINT32 IndexNum, SINT32 Index[], UINT8 *WorkItem, SINT32 Interval, UINT8 *Result) PURE;

	// Motion Detection
	
	// set motion detection parameters
	//
	// parameter:
	//		RegionNum -- number of region for motion detection, should between 0 to 4, 0 means don't do motion detection
	//      Regions   -- array of region position and size for motion detection, the size of array should equal to RegionNum
	//					 the position and size of region should be integral times of 16.
	//      Threshold -- the macroblock is treated as motion if the motion vector is equal or greater than this value
	//      Percent   -- A notification will be reported if this percentage of macro blocks in the given regions have motion
	// return: 
	//		none
	virtual void SetMotionDetectionParams(SINT32 RegionNum, RECT *Regions, SINT32 Threshold, SINT32 Percent) PURE;

};

#endif 

