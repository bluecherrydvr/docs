#ifndef __MPEG4_READER_H__
#define __MPEG4_READER_H__

#include "DefCode.h"
#include "linklist.h"

#include <iostream>
#include <fstream>
#include <string>

#include <WinSock2.h>

// Use for Get I frame from index
#define I_FRAME_PREVIOUS		0
#define I_FRAME_CURRENT			1
#define I_FRAME_NEXT			2

// Marty expended this dll.  01032008
//
//5 types are needed
// 1. mpeg4
// 2. Audio PCM
// 3. Audio PCM must with time stamp
// 4. MJPEG
// 5. H.264
enum Raw_Data_Type
{
	EXCEPTION = 0,
	MPEG4_DATA = 1,
	AUDIO_PCM_DATA = 2,
	AUDIO_PCM_TIMESTAMP_DATA = 3,
	MJPEG_DATA = 4,
	H264_DATA = 5
};
//

typedef struct
{
	unsigned char b2h[4]; /* 00 00 01 B2 */
	unsigned char msg_type;
	unsigned char stream_id; /* video streaming id */
	unsigned char ext_b2_len; /* 1: length of the ext. b2 private data appended
							  to B2 frame */
	unsigned char rsvd;
	unsigned int dwDataLength;
} B2_HEADER_V2;

typedef struct 
{
	time_t date_time;
	unsigned char time_zone; /* 0:-12, ..., 24:+13 */
	unsigned char video_loss; /* 0: video loss, 1 : video ok */
	unsigned char motion; /* 0x02: Motion 1 is active, 0x04: Motion 2 is
						  active, 0x08 Motion 3 is active */

	unsigned char dio; /* for DIs, 0: DI triggered. 1: no triggered */
	unsigned int count; /* frame counter */
	unsigned char resolution; /* 0:N720x480, ... */
	unsigned char bitrate; /* 0:28K, ... */
	unsigned char fps_mode; /* 0:MODE1(constant), 1:0:MODE2 */
	unsigned char fps_number; /* In constant FPS mode, it indicates the video
							  server's constant FPS number.
							  In variable FPS mode, in indicates the variable
							  FPS number which was requested by the TCP
							  host. If it is not in TCP, it indicates the variable
							  FPS number */
	struct timeval timestamp;
	unsigned short md_actives[3]; /* # of active microblocks in motion region */
	unsigned char reserved[2];
} PRIVATE_DATA_B2;

typedef struct 
{
	B2_HEADER_V2 header;
	PRIVATE_DATA_B2 prdata;
} VIDEO_B2_FRAME;

typedef struct 
{
	B2_HEADER_V2 header;
	struct timeval timestamp;
	unsigned char reserved[8];
} AUDIO_B2_V2;


/**	\brief Tail of Raw file.
*
*/
typedef struct 
{
	DWORD			dwLastB2;			/**< '00' '00' '01' 'B2' */
	DWORD			dwHeader;			/**< Must be 0xAC710517 (ACTi0517) */
	DWORD			dwVersion;			/**< Must be 0x01000001 (1.0.0.1) */
	DWORD			dwBeginTime;
	DWORD			dwEndTime;
	DWORD			dwGOP;
	DWORD			dwGOPSize;
	DWORD			dwFPS;
	DWORD			dwWidth;
	DWORD			dwHeight;
	DWORD			dwFrameCount;
	DWORD			dwTimeZone;
	char			nTimeComplementarity;
	BYTE			Reserve[7];
}RawFileTail_t;

/**	\brief Header of Raw file.
*
*/
typedef struct 
{
	DWORD			dwFirstB2;			// '00' '00' '01' 'B2'
	DWORD			dwStreamType;		// 11 : TCP-1.0; 22 TCP-2.0
	DWORD			dwVideoType;		// 11 : ISO 14496; 22 : ...
	DWORD			dwAudioType;		// 00 : NONE; 11 : PCM; ...
	DWORD			dwControlType;		// 00 : NONE; 11 : TCP-2.0
	DWORD			dwAudio2Type;		// Reserve
	DWORD			dwControl2Type;		// Reserve
	DWORD			dwBiteRate;			// Bite rate 0 - 15
	DWORD 			dwFps;				// FPS for the file
	DWORD 			dwResolution;		// Resolution :0 - 5
	DWORD 			dwReserve1;			// Reserve from 1 - 6 :FF
	DWORD 			dwReserve2;			// Reserve from 1 - 6 :FF
	DWORD 			dwReserve3;			// Reserve from 1 - 6 :FF
	DWORD 			dwReserve4;			// Reserve from 1 - 6 :FF
	DWORD 			dwReserve5;			// Reserve from 1 - 6 :FF
	DWORD 			dwReserve6;			// Reserve from 1 - 6 :FF
}RawFileHeader_t;

/**	\brief Data header of TCP 20.
*
*/
typedef struct 
{
	DWORD	dwCustomerHeader;		/**< 00 00 01 B2 */
	DWORD	dwMSGType;				/**< 4 Bytes: Viedo:1 Audio:2 */
	DWORD	dwDataLength;			/**< behind length of stream data */
}TCP20Header_t;

/**	\brief Video Data Header of TCP 20.
*
*/
typedef struct 
{
	time_t	dwTimeCode;
	BYTE	btTimeZone;
	BYTE	btVideoLose;
	BYTE	btMotion;
	BYTE	btDIO;
	DWORD	Count;
	BYTE	bResolution;
	BYTE	bBitrate;	
	BYTE	bFpsMode;	
	BYTE	bFpsNumber;
	BYTE	bReserved[16];
}TCP20VideoHeader_t;

/**	\brief Data of TCP 10.
*
*/
typedef struct 
{
	char	cb2h[4];			/**< b2h identifer */
	DWORD	dwControlCode;		/**< 0x00010022 */
	DWORD	dwDataLength;		/**< behind length of stream data */
}TCP10Header_t;

/** \brief Information of Data
*
*/
typedef struct 
{
	DWORD dwMSGType;		/**< Message type of the data.  4 Bytes: Viedo:1 Audio:2 */
	DWORD dwFrameType;		/**< Frame type of the data. */
	DWORD dwDataLength;		/**< Data length of the stream data */
	DWORD dw20DataLength;
}DataInfo_t;

/**	\brief Class to read MPEG4 file.
*
*	MPEG4 reader class, to handle basic IO on MPEG4 files.
*/
class CMpeg4Reader
{
public:
	/**	\brief Class Constructor
	*
	*	MPEG4 reader class, to handle basic IO on MPEG4 files.
	*/
	CMpeg4Reader();

	/**	\brief Class Destructor
	*
	*/
	~CMpeg4Reader();

	/**	\brief Set media configuration
	*	\param pMediaConfig media configuration
	*
	*	Set media configuration
	*/
	void SetMediaConfig( MEDIA_CONNECTION_CONFIG* pMediaConfig );

	/**	\brief Get Video configuration
	*	\param pVideoConfig Video configuration
	*
	*	Get video configuration
	*/
	void GetVideoConfig( MEDIA_VIDEO_CONFIG* pVideoConfig );

	/**	\brief Set video configuration
	*	\param pVideoConfig Video configuration
	*
	*	Set video configuration
	*/
	void SetVideoConfig( MEDIA_VIDEO_CONFIG* pVideoConfig );

	/** \brief Open a file
	*	\return Open result.
	*/
	bool OpenFile();

	/** \brief Close a file
	*
	*/
	void CloseFile();

	/**	\brief Get next frame.
	*	\param szFrame Frame.
	*	\param DataInfo Datatype info strucrure
	*	\return Ture - Success get, Fail - Fail to Get.
	*
	*	Get next frame from current position.
	*/
	bool GetNextFrame(unsigned char* szFrame, DataInfo_t &DataInfo);

	/**	\brief Get previous frame.
	*	\param szFrame Previous Frame.
	*	\param DataInfo Datatype info strucrure
	*	\return Ture - Success get, Fail - Fail to Get.
	*
	*	Get previous frame from current position.
	*/
	bool GetPrevFrame(unsigned char* szFrame, DataInfo_t &DataInfo);

	/**	\brief Get start position of next frame
	*	\param lStartPos Start pos return
	*	\return Ture - Success get, Fail - Fail to Get.
	*
	*	Get next frame's start position
	*/
	bool GetNextFramePos(long& lStartPos);

	/** \brief Set Frame Position (File position)
	*	\param lPos The position of the frame
	*	\param isI Is it I frame
	*	\return True - set ok, Fail - not set
	*
	*	Set Frame start position (File position)
	*/
	bool SetFramePos(long lPos, bool isI = false);
	
	/** \brief Get current file position.
	*	\return file position. -1 if error.
	*
	*	Get current file position.
	*/
	long GetFilePos();

	/**	\brief Get previous Iframe.
	*	\param szFrame Frame data.
	*	\param DataInfo Datatype info structure
	*	\return Ture - Success get, Fail - Fail to Get.
	*
	*	Get previous Iframe from current position.
	*/
	bool GetPrevIFrame(unsigned char* szFrame, DataInfo_t &DataInfo);

	/**	\brief Get next I frame.
	*	\param szFrame Frame data.
	*	\param DataInfo Datatype info structure
	*	\return Ture - Success get, Fail - Fail to Get.
	*
	*	Get next I frame from current file position.
	*/
	bool GetNextIFrame(unsigned char* szFrame, DataInfo_t &DataInfo);
 
	/**	\brief Get TCP version.
	*	\return TCP version
	*
	*	Get the TCP version of the file.
	*/
	int GetTCPVersion();

	/** \brief Get the size of file
	*
	*/
	void SetFileSize();

	/** \brief Get current reading status in percentage
	*	\return Status of reading
	*/
	double GetReadStatus();

	/** \brief Get sequence header for variable frame rate 
	*	\param nResolution Resolution
	*	\return sequence header : 21 bytes
	*/
	unsigned char*	GetMpg4Header( short nResolution);

	/** \brief Get sequence header for constant frame rate 
	*	\param nResolution Resolution
	*	\param nFPs FPS
	*	\return sequence header : 21 bytes
	*/
	unsigned char*	GetMpg4Header( short nResolution, short nFPs );

	/** \brief Get raw header file information 
	*	\param HeaderInfo struct RawFileHeader_t
	*/
	void GetRawHeader(RawFileHeader_t& HeaderInfo);
	
	/** \brief Get raw tail file information 
	*	\param TailInfo struct RawFileTail_t
	*/
	void GetRawTail(RawFileTail_t& TailInfo);

	/**	\brief Get first I frame.
	*	\param szFrame I Frame.
	*	\param DataInfo Datatype info structure
	*	\return Ture - Success get, Fail - Fail to Get.
	*
	*	Get first I frame of file.
	*/
	bool GetFirstIFrame(unsigned char* szFrame, DataInfo_t &DataInfo);

	/** \brief Set file position to first I frame.
	*	\return true - set, false - not set
	*
	*	Set file position to first I frame.
	*/
	bool SetFirstIPos();

	/** \brief Send define command
	*	\param dwCommand Command type 
	*	\param pData Command data
	*	\param dwDataLen Command data length
	*
	*	Perform defined command.
	*/
	bool SendCommand(DWORD dwCommand, char* pData, DWORD dwDataLen);

	/** \brief To check frame type
	*	\param pRawData Raw data 
	*	\param Datalen Raw data length
	*	\return 0:Unknown Frame, 1:I Frame, 2:P Frame
	*
	*	Perform defined command.
	*/
	DWORD CheckIorP( char* pRawData, int Datalen );

	/** \brief To check frame type
	*	\param pRawData Raw data 
	*	\param Datalen Raw data length
	*	\return 0:error length, 1:I Frame, 2:P Frame or something else
	*
	*	Perform defined command.
	*/
	int CheckH264IorP(BYTE* pData, DWORD dwLen );

private:
	bool m_bIdxWritenFlag;

private:




	FILE	*m_fp;													/**< File Handler */
	DWORD	m_dwFirstIPos;											/**< First I frame position */
	DWORD	m_dwCurIPos;											/**< I frame position of current GOP */
	
	RawFileHeader_t	m_RawFileHeader;								/**< Mpeg4 Raw Data File Header */
	RawFileTail_t	m_RawFileTail;									/**< Mpeg4 Raw Data Info. */
	
	DWORD	m_dwFileSize;											/**< Size of the File */
	linklist<IFrameInfo*> m_llIFrameIndex;						/**< Link List to keep all I frame's information */

	MEDIA_CONNECTION_CONFIG							m_MediaConfig;	/**< Connection Configuration */
	MEDIA_VIDEO_CONFIG								m_VideoConfig;	/**< Media Configuration */

	// Used by GetNextFrame
	unsigned char* m_szTempFrame;		/**< Temp buffer to keep frame */
	unsigned char* m_szTempData;		/**< Temp buffer to keep data */

	// Used by others but not GetNextFrame
	unsigned char* m_szTempFrame2;		/**< Temp buffer to keep frame */

	unsigned char m_szTimeCode[4];				/**< Time Code */
	unsigned char m_cTimeZone;					/**< Time zone */

private:
	/** \brief check the validation of data length 
	*
	*/
	bool CheckLength(DWORD dwlen);

	/**	\brief Read data from file.
	*	\param	buffer Place to put read data.
	*	\param	dwlen Data length that we going to read.
	*	\return read result
	*
	*	Read data from file.
	*/
	bool ReadData(void* buffer, size_t dwlen);

	/**	\brief Given a frame data and determine weather it is I or P frame.
	*	\param szFrameData Frame that we going to determine.
	*	\return Frame type. I:1. P:2.
	*
	*	Get the Frame type of a frame.
	*/
	DWORD GetFrameType(unsigned char* szFrameData);

	/** \brief Test End of File
	*	\return True - end of file	False - not end of file
	*
	*	Test EOF
	*/
	bool IsEOF();

	/** \brief Convert TCP 1.0 data to TCP 2.0 data
	*	\param dwFrameType Frame type. I or P.
	*	\param pSource Source data
	*	\param dwSLen Source data length
	*	\param pDestination Destination string
	*	\param dwDLen Destination string length
	*
	*	Convert TCP 1.0 data to TCP 2.0 data.
	*/
	void Convert10To20(DWORD dwFrameType, unsigned char* pSource, DWORD dwSLen, unsigned char* pDestination, DWORD& dwDLen);

	/** \brief Add I frame information to index
	*	\param dwFilePos I frame file position.
	*	\param dwTimeCode I frame time code.
	*	
	*/
	void AddIFrameInfoToIndex(DWORD dwFilePos, DWORD dwTimeCode);

	/** \brief Set file position to correct I frame by using IFinfo.
	*	\param dwType PLY_REQ_SET_POSITION - set by pos, PLY_REQ_SET_TIMECODE - set by time code
	*	\param dwData Data passed in.
	*	\return Set result
	*/
	bool SetIPosFromIndex(DWORD dwType, DWORD dwData);

	/** \brief Find I frame information from link list.
	*	\param dwMethod METHOD_FILE_POS - find by file position, METHOD_FILE_TIME_CODE - find by time code
	*	\param dwData Data passed in.
	*	\param pIFInfo I frame information return.
	*	\return find result
	*/
	bool FindIFrameInfoFromIndex(DWORD dwMethod, DWORD dwData, IFrameInfo** pIFInfo);
};

#endif // __MPEG4_READER_H__