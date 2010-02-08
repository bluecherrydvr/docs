#include "stdafx.h"
#include "MPEG4Reader.h"

// Constructor
CMpeg4Reader::CMpeg4Reader():m_fp(NULL), m_dwFileSize(0), m_dwCurIPos(0), m_dwFirstIPos(0), m_szTempFrame(NULL), m_szTempData(NULL), m_szTempFrame2(NULL)
{
	memset(&m_RawFileHeader, 0x00, sizeof(RawFileHeader_t));
	memset(&m_RawFileTail, 0x00, sizeof(RawFileTail_t));
	m_llIFrameIndex.Empty();
	memset(&m_MediaConfig, 0x00, sizeof(MEDIA_CONNECTION_CONFIG));
	memset(&m_VideoConfig, 0x00, sizeof(MEDIA_VIDEO_CONFIG));
	memset(&m_cTimeZone, 0x00, sizeof(unsigned char));
	memset(&m_szTimeCode, 0x00, 4);

	m_bIdxWritenFlag = false;
}

// Destructor
CMpeg4Reader::~CMpeg4Reader()
{
	// Close file
	if(m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}

	if(NULL != m_szTempData)
	{
		delete [] m_szTempData;
		m_szTempData = NULL;
	}

	if(NULL != m_szTempFrame)
	{
		delete [] m_szTempFrame;
		m_szTempFrame = NULL;
	}

	if(NULL != m_szTempFrame2)
	{
		delete [] m_szTempFrame2;
		m_szTempFrame2 = NULL;
	}

	IFrameInfo* pIFInfo;
	pIFInfo = m_llIFrameIndex.First();
	while(pIFInfo != NULL)
	{
		delete pIFInfo;
		pIFInfo = m_llIFrameIndex.Next();
	}

	// Free link list.
	m_llIFrameIndex.Empty();

	m_bIdxWritenFlag = false;
}

// Set media configuration
void CMpeg4Reader::SetMediaConfig(MEDIA_CONNECTION_CONFIG* pMediaConfig)
{
	memcpy(&m_MediaConfig, pMediaConfig, sizeof(MEDIA_CONNECTION_CONFIG));
}

// Get video configuration
void CMpeg4Reader::GetVideoConfig(MEDIA_VIDEO_CONFIG* pVideoConfig )
{
	m_VideoConfig.dwVideoResolution = m_RawFileHeader.dwResolution;
	m_VideoConfig.dwFps = m_RawFileHeader.dwFps;
	memcpy(pVideoConfig, &m_VideoConfig, sizeof(MEDIA_VIDEO_CONFIG));
}

// Set video configuration
void CMpeg4Reader::SetVideoConfig(MEDIA_VIDEO_CONFIG* pVideoConfig)
{
	memcpy(&m_VideoConfig, pVideoConfig, sizeof(MEDIA_VIDEO_CONFIG));
}

// Open MP4 File
bool CMpeg4Reader::OpenFile()
{
	if(NULL != m_fp)
		return false;

	if(NULL == m_MediaConfig.PlayFileName)
		return false;

	if(0 == strlen(m_MediaConfig.PlayFileName))
		return false;

	// Init. frame and data buffer.
	m_szTempFrame = new unsigned char[FRAME_LEN];
	m_szTempFrame2 = new unsigned char[FRAME_LEN];
	m_szTempData = new unsigned char[DATA_LEN];
	memset(m_szTempFrame, 0x00, FRAME_LEN);
	memset(m_szTempFrame2, 0x00, FRAME_LEN);
	memset(m_szTempData, 0x00, DATA_LEN);

	// File open for read.
	if((m_fp = fopen(m_MediaConfig.PlayFileName, "rb")) == NULL)
	{
		return false;
	}

	// Set file size 
	SetFileSize();

	// Read File header
	if(!ReadData(&m_RawFileHeader, sizeof(RawFileHeader_t)))
	{
		return false;
	}

	// Read File Tail
	fseek(m_fp, m_dwFileSize - sizeof(RawFileTail_t), SEEK_SET);

	if(!ReadData(&m_RawFileTail, sizeof(RawFileTail_t)))
	{
		return false;
	}

	// Set file position to first data
	fseek(m_fp, sizeof(RawFileHeader_t), SEEK_SET);

	// Add .idx reading. 12 06 2007 Marty
	std::string idx_file_name = m_MediaConfig.PlayFileName;
	std::string::size_type str_pos = idx_file_name.rfind('.');
	std::string strBasename;
	if(str_pos == std::string::npos)
	{
		idx_file_name = idx_file_name + std::string(".idx");
	}
	else
	{
		strBasename = idx_file_name.substr(0, str_pos);
		idx_file_name = strBasename + std::string(".idx");
	}
	std::ifstream IndexFileStream;
	IndexFileStream.open(idx_file_name.c_str(), std::ios::in | std::ios::binary);
	if(IndexFileStream.is_open())
	{
		DWORD IFrameTime = 0;
		DWORD IFramePosInFile = 0;
		while(IndexFileStream)
		{
			IFrameTime = 0;
			IFramePosInFile = 0;
			IndexFileStream >> IFramePosInFile;
			IndexFileStream >> IFrameTime;
			if(!IndexFileStream.eof())
			{
				AddIFrameInfoToIndex(IFramePosInFile,IFrameTime);
			}
		}
		m_bIdxWritenFlag = true;
	}

	// Jo_20060522 - Set file pointer to first I frame(S)
	return SetFirstIPos();
	// Jo_20060522 - Set file pointer to first I frame(E)
}

// Close file
void CMpeg4Reader::CloseFile()
{
	// Close file
	if(m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}

	if(NULL != m_szTempData)
	{
		delete [] m_szTempData;
		m_szTempData = NULL;
	}

	if(NULL != m_szTempFrame)
	{
		delete [] m_szTempFrame;
		m_szTempFrame = NULL;
	}

	if(NULL != m_szTempFrame2)
	{
		delete [] m_szTempFrame2;
		m_szTempFrame2 = NULL;
	}

	IFrameInfo* pIFInfo;
	pIFInfo = m_llIFrameIndex.First();
	while(pIFInfo != NULL)
	{
		delete pIFInfo;
		pIFInfo = m_llIFrameIndex.Next();
	}

	// Free link list.
	m_llIFrameIndex.Empty();
}

// Get next frame from current file position.
bool CMpeg4Reader::GetNextFrame(unsigned char* szFrame, DataInfo_t &DataInfo)
{
	// Set initial data info
	DataInfo.dwMSGType = MSG_TYPE_UNKNOW;
	DataInfo.dwFrameType = FRAME_TYPE_UNKNOW;
	DataInfo.dwDataLength = 0;

	// Test eof
	if(IsEOF())
	{
		DataInfo.dwMSGType = MSG_TYPE_EOF;
		return false;
	}

	// Switch to read different stream data.
	switch(m_RawFileHeader.dwStreamType)
	{
		// TCP 10
	case TCP_VERSION_10:
		TCP10Header_t tempTCP10Header;
		// Read Header
		if(ReadData(&tempTCP10Header, sizeof(TCP10Header_t)))
		{
			// Check identifer and Control Code
			if(strncmp(tempTCP10Header.cb2h, Temp, 4) == 0 && TCP10_VIDEO_CONTROL_CODE == tempTCP10Header.dwControlCode)
			{
				// Read Data
				memset(m_szTempFrame, 0x00, FRAME_LEN);
				memset(m_szTempData, 0x00, DATA_LEN);
				if(ReadData(m_szTempData, tempTCP10Header.dwDataLength))
				{
					memcpy(m_szTempFrame, &tempTCP10Header, sizeof(TCP10Header_t));
					memcpy(m_szTempFrame + sizeof(TCP10Header_t), m_szTempData, tempTCP10Header.dwDataLength);
					DataInfo.dwMSGType = MSG_TYPE_10;
					// Determine Frame Type
					DataInfo.dwFrameType = CheckIorP((char*)m_szTempData, 64);
					if(DataInfo.dwFrameType)
					//if(DataInfo.dwFrameType = GetFrameType(m_szTempData + TCP10_CUSTOMER_DATA_SIZE))
					{
						DWORD dwTempDLen;
						Convert10To20(DataInfo.dwFrameType, m_szTempFrame, tempTCP10Header.dwDataLength + TCP1020_HEADER_SIZE, szFrame, dwTempDLen);
						DataInfo.dwDataLength = tempTCP10Header.dwDataLength;
						DataInfo.dw20DataLength = dwTempDLen;
						if(FRAME_TYPE_I == DataInfo.dwFrameType)
						{
							m_dwCurIPos = GetFilePos()  - sizeof(TCP10Header_t) - tempTCP10Header.dwDataLength;
							// Jo_200600906 - add I frame info to list & calculate current time(S)
							DWORD dwTimeZone = m_cTimeZone;
							DWORD dwTimeCode;
							memcpy(&dwTimeCode, m_szTimeCode, sizeof(DWORD));
							DWORD dwTempTimeCode = dwTimeCode + ((dwTimeZone - 12)*3600);
							AddIFrameInfoToIndex(m_dwCurIPos, dwTempTimeCode);
							// Jo_200600906 - add I frame info to list & calculate current time(E)
						}
						return true;
					}
				}
			}
		}
		break;
		// TCP 20
	case TCP_VERSION_20:
		// Read header to verify it is video or audio
		B2_HEADER_V2 tempTCP20Header;
		PRIVATE_DATA_B2 tempTCP20VideoHeader;
		if(ReadData(&tempTCP20Header, sizeof(B2_HEADER_V2)))
		{
			// Check customer header
			if( 0x00 == tempTCP20Header.b2h[0] && 0x00 == tempTCP20Header.b2h[1] && 0x01 == tempTCP20Header.b2h[2] && 0xB2 == tempTCP20Header.b2h[3] )
			{
				// Determine Message type & Read data
				memset(m_szTempData, 0x00, DATA_LEN);
				switch(tempTCP20Header.msg_type) 
				{
				case MPEG4_DATA:
					// Read the rest customer data
					if(ReadData(&tempTCP20VideoHeader, sizeof(PRIVATE_DATA_B2)))
					{
						// Read data
						if(ReadData(m_szTempData, tempTCP20Header.dwDataLength - sizeof(PRIVATE_DATA_B2)))
						{
							DataInfo.dwMSGType = MPEG4_DATA;
							memcpy(szFrame, &tempTCP20Header, sizeof(B2_HEADER_V2));
							memcpy(szFrame + sizeof(B2_HEADER_V2), &tempTCP20VideoHeader, sizeof(PRIVATE_DATA_B2));
							memcpy(szFrame + sizeof(B2_HEADER_V2) + sizeof(PRIVATE_DATA_B2), m_szTempData, tempTCP20Header.dwDataLength - sizeof(PRIVATE_DATA_B2));
							DataInfo.dwDataLength = tempTCP20Header.dwDataLength;
							DataInfo.dw20DataLength = tempTCP20Header.dwDataLength;
						}
					}
					break;
				case AUDIO_PCM_DATA:
					// Read data
					if(ReadData(m_szTempData, tempTCP20Header.dwDataLength))
					{
						memcpy(szFrame, &tempTCP20Header, sizeof(B2_HEADER_V2));
						memcpy(szFrame + sizeof(B2_HEADER_V2), m_szTempData, tempTCP20Header.dwDataLength);
						DataInfo.dwMSGType = AUDIO_PCM_DATA;
						DataInfo.dwDataLength = tempTCP20Header.dwDataLength;
						DataInfo.dw20DataLength = tempTCP20Header.dwDataLength;
						return true;
					}
					break;
				case AUDIO_PCM_TIMESTAMP_DATA:
					// Read data
					if(ReadData(m_szTempData, tempTCP20Header.dwDataLength))
					{
						memcpy(szFrame, &tempTCP20Header, sizeof(B2_HEADER_V2));
						memcpy(szFrame + sizeof(B2_HEADER_V2), m_szTempData, tempTCP20Header.dwDataLength);
						DataInfo.dwMSGType = AUDIO_PCM_TIMESTAMP_DATA;
						DataInfo.dwDataLength = tempTCP20Header.dwDataLength;
						DataInfo.dw20DataLength = tempTCP20Header.dwDataLength;
						return true;
					}
					break;
				case MJPEG_DATA:
					// Read the rest customer data
					if(ReadData(&tempTCP20VideoHeader, sizeof(PRIVATE_DATA_B2)))
					{
						// Read data
						if(ReadData(m_szTempData, tempTCP20Header.dwDataLength - sizeof(PRIVATE_DATA_B2)))
						{
							DataInfo.dwMSGType = MJPEG_DATA;
							memcpy(szFrame, &tempTCP20Header, sizeof(B2_HEADER_V2));
							memcpy(szFrame + sizeof(B2_HEADER_V2), &tempTCP20VideoHeader, sizeof(PRIVATE_DATA_B2));
							memcpy(szFrame + sizeof(B2_HEADER_V2) + sizeof(PRIVATE_DATA_B2), m_szTempData, tempTCP20Header.dwDataLength - sizeof(PRIVATE_DATA_B2));
							DataInfo.dwDataLength = tempTCP20Header.dwDataLength;
							DataInfo.dw20DataLength = tempTCP20Header.dwDataLength;
						}
					}
					break;
				case H264_DATA:
					// Read the rest customer data
					if(ReadData(&tempTCP20VideoHeader, sizeof(PRIVATE_DATA_B2)))
					{
						// Read data
						if(ReadData(m_szTempData, tempTCP20Header.dwDataLength - sizeof(PRIVATE_DATA_B2)))
						{
							DataInfo.dwMSGType = H264_DATA;
							memcpy(szFrame, &tempTCP20Header, sizeof(B2_HEADER_V2));
							memcpy(szFrame + sizeof(B2_HEADER_V2), &tempTCP20VideoHeader, sizeof(PRIVATE_DATA_B2));
							memcpy(szFrame + sizeof(B2_HEADER_V2) + sizeof(PRIVATE_DATA_B2), m_szTempData, tempTCP20Header.dwDataLength - sizeof(PRIVATE_DATA_B2));
							DataInfo.dwDataLength = tempTCP20Header.dwDataLength;
							DataInfo.dw20DataLength = tempTCP20Header.dwDataLength;
						}
					}
					break;
				default:
					return false;
				}

				// Determine Frame Type
				// move to correct position for GOP check.
				// mpeg4
				if(DataInfo.dwMSGType == MPEG4_DATA)
				{
					DataInfo.dwFrameType = CheckIorP((char*)m_szTempData, 64);
				}
				// h.264
				else if(DataInfo.dwMSGType == H264_DATA)
				{
					DataInfo.dwFrameType = CheckH264IorP(m_szTempData, 64);
				}
				// m-Jpeg always I frame
				else if( DataInfo.dwMSGType == MJPEG_DATA )
				{
					DataInfo.dwFrameType = FRAME_TYPE_I;
				}
				

				if(FRAME_TYPE_I == DataInfo.dwFrameType)
				{
					m_dwCurIPos = GetFilePos()  - sizeof(B2_HEADER_V2) - tempTCP20Header.dwDataLength;
					// Jo_10060525 - add I frame info to list(S)
					DWORD dwTempTimeCode;
					memcpy(&dwTempTimeCode, &szFrame[TCP1020_HEADER_SIZE], sizeof(DWORD));
					char cTimeZone = szFrame[TCP1020_HEADER_SIZE + 4];
					DWORD dwTimeZone = cTimeZone;
					dwTempTimeCode = dwTempTimeCode + ((dwTimeZone - 12)*3600);

					//M JPEG checking Time. (I hope insert 1 I-frame per second) Marty. 01032008
					static DWORD dwLastTimeInputIframeTimeCode = 0;
					if(dwLastTimeInputIframeTimeCode != dwTempTimeCode)
					{
						AddIFrameInfoToIndex(m_dwCurIPos, dwTempTimeCode);
						dwLastTimeInputIframeTimeCode = dwTempTimeCode;
					}
					
					// Jo_10060525 - add I frame info to list(E)
				}

				return true;
			}
		}
		break;
	default:
		return false;
	}
	return false;
}

// Get next I frame by slowly parse the file and check each frame's type.
bool CMpeg4Reader::GetNextIFrame(unsigned char* szFrame, DataInfo_t &DataInfo)
{
	bool bFind = false;

	while(!bFind)
	{
		// Must aware the size of szFrame that passed in from caller.
		memset(szFrame, 0x00, FRAME_LEN);

		if(GetNextFrame(szFrame, DataInfo))
		{
			if(FRAME_TYPE_I == DataInfo.dwFrameType)
			{
				bFind = true;
				return true;
			}
		}
		else
		{
			return false;
		}
	}
	return false;
}

// Get start position of next frame. step through the file byte by byte
// This function will not set the file pointer position to next frame.
bool CMpeg4Reader::GetNextFramePos(long& lStartPos)
{
	char szBuf[TEMP_BUFFER_SZE + 1] = {0,};
	char szCustomerHeader[5] = {0x00, 0x00, 0x01, 0xB2, 0x00};
	// Back up current position
	long lPosCurrent = ftell(m_fp);

	// Parse the file and find flag.
	while(!IsEOF())
	{
		memset(szBuf, 0x00, TEMP_BUFFER_SZE + 1);
		if(ReadData(szBuf, TEMP_BUFFER_SZE))
		{
			DWORD dwTempReadPos = 0;
			// Parse the read buffer.  Find Customer flag 0xB2010000
			while(dwTempReadPos <= (TEMP_BUFFER_SZE - 4))
			{
				// find customer header
				if(0 == memcmp(szBuf + dwTempReadPos, szCustomerHeader, 4))
				{
					// position after read the data (temp)
					long lTempCurrent = ftell(m_fp);
					// Switch to read different stream data.
					switch(m_RawFileHeader.dwStreamType)
					{
						// TCP 10
					case TCP_VERSION_10:
						// Set start pos at header
						lStartPos = lTempCurrent + dwTempReadPos - sizeof(TCP10Header_t) - TEMP_BUFFER_SZE;
						SetFramePos(lPosCurrent);
						return true;
						// TCP 20
					case TCP_VERSION_20:
						lStartPos = lTempCurrent + dwTempReadPos - TEMP_BUFFER_SZE;
						SetFramePos(lPosCurrent);
						return true;
					default:
						SetFramePos(lPosCurrent);
						return false;
					}
					SetFramePos(lPosCurrent);
					return false;
				}
				dwTempReadPos += 1;
			}
		}
		else
		{
			SetFramePos(lPosCurrent);
			return false;
		}
	}
	SetFramePos(lPosCurrent);
	return false;
}

// Simple checking  on data length
bool CMpeg4Reader::CheckLength(DWORD dwlen)
{
	long lCurrent = ftell(m_fp);
	if((m_dwFileSize - lCurrent) < dwlen)
		return false;
	return true;
}

// Read data from file
bool CMpeg4Reader::ReadData(void* buffer, size_t dwlen)
{
	int iRead;
	if(CheckLength(dwlen))
	{
		if((iRead = fread(buffer,  sizeof(char), dwlen, m_fp)))
		{
			return true;
		}
	}
	return false;
}

// Get TCP version.
int CMpeg4Reader::GetTCPVersion()
{
	return m_RawFileHeader.dwStreamType;
}

// Set the size of file.
void CMpeg4Reader::SetFileSize()
{
	if(NULL != m_fp)
	{
		// Get Current position for reset
		long lCurrent = ftell(m_fp);
		// Seek to end of file
		fseek(m_fp, 0, SEEK_END);
		// Get the last position of the file (file size)
		m_dwFileSize = ftell(m_fp);
		// Set back to the original position
		fseek(m_fp, lCurrent, SEEK_SET);
	}
}

// Get frame type from data.
DWORD CMpeg4Reader::GetFrameType(unsigned char* szFrameData)
{
	DWORD dwFrameType;
	memcpy(&dwFrameType, szFrameData, sizeof(DWORD));
	switch(dwFrameType)
	{
	case FLAG_GOP_HEADER:
		return FRAME_TYPE_I;
	default:
		return FRAME_TYPE_P;
	}
}

// Status in percentage
double CMpeg4Reader::GetReadStatus()
{
	if(0 != m_dwFileSize)
	{
		long lCurrent = ftell(m_fp);
		if(lCurrent >= m_dwFileSize - sizeof(RawFileTail_t))
			return 100.00;
		else
			return ((double)lCurrent/(double)m_dwFileSize)*100;
	}
	return 0;
}

// Test EOF
bool CMpeg4Reader::IsEOF()
{
	if(NULL == m_fp)
		return false;

	if(feof(m_fp))
	{
		return true;
	}
	return false;
}

// Set frame start position
bool CMpeg4Reader::SetFramePos(long lPos, bool isI)
{
	if(!m_fp)
	{
		return false;
	}

	if(lPos >= m_dwFileSize)
	{
		return false;
	}
	else
	{
		fseek(m_fp, lPos, SEEK_SET);
		if(isI)
		{
			m_dwCurIPos = lPos;
		}
		return true;
	}
}

// Get file position (file position)
long CMpeg4Reader::GetFilePos()
{
	if(m_fp)
		return ftell(m_fp);
	return -1;
}

// Convert TCP 1.0 data to TCP 2.0 data.
void CMpeg4Reader::Convert10To20(DWORD dwFrameType,  unsigned char* szSource, DWORD dwSLen, unsigned char* szDestination, DWORD& dwDLen)
{
	DWORD dwB2 = 0xB2010000;									/**< B2 Header */
	DWORD dwB0 = 0xB0010000;									/**< B0 Header */
	unsigned char cB0 = 0xF5;									/**< Data after B0 */
	DWORD dwB5 = 0xB5010000;									/**< B5 Header */
	unsigned char cB5 = 0x09;									/**< Data after B5 */
	DWORD dwVideo = 0x00000001;									/**< Video Flag */
	unsigned char cDIO = 0x03;									/**< DIO information */
	unsigned char cMotion = 0x00;								/**< Motion information */
	unsigned char szTemp16[16]={0,};							/**< Reserve area for Video header */
	DWORD dwResolution = m_RawFileHeader.dwResolution;			/**< Resoltion */
	DWORD dwBitrate = m_RawFileHeader.dwBiteRate;				/**< Bit Rate */
	DWORD dwFpsMode = 0x00000000;								/**< Constant or Variable */
	DWORD dwFpsNumber = 0x00000000;								/**< FPS number for Constant or Variable */
	DWORD dwDataLength;											/**< Data length */
	DWORD dwCount = 0x00000000;									/**< count */

	DWORD dwPos = 0;
	int i = 0;
	switch(dwFrameType) 
	{
	case FRAME_TYPE_I:
		{
			// B2 Header
			memcpy(szDestination, &dwB2, sizeof(DWORD));
			dwPos += sizeof(DWORD);
			// Video Type
			memcpy(szDestination + dwPos, &dwVideo, sizeof(DWORD));
			dwPos += sizeof(DWORD);
			// Length of the data
			// Data length = total length - length header - TCP 1.0 B2 data length + TCP 2.0 Video Header + TCP 2.0 B0 + TCP 2.0 B5 + TCP 2.0 Sequence Header
			dwDataLength = dwSLen - TCP1020_HEADER_SIZE - TCP10_CUSTOMER_DATA_SIZE + TCP20_VIDEO_HEADER_DATA_SIZE + TCP20_B0_DATA_SIZE + TCP20_B5_DATA_SIZE + TCP20_00_DATA_SIZE;
			memcpy(szDestination + dwPos, &dwDataLength, sizeof(DWORD));
			dwPos += sizeof(DWORD);
			// Time code
			m_szTimeCode[0] = szSource[TCP1020_HEADER_SIZE + 5];
			m_szTimeCode[1] = szSource[TCP1020_HEADER_SIZE + 5 + 2];
			m_szTimeCode[2] = szSource[TCP1020_HEADER_SIZE + 5 + 2 + 2];
			m_szTimeCode[3] = szSource[TCP1020_HEADER_SIZE + 5 + 2 + 2 +2];
			memcpy(szDestination + dwPos, m_szTimeCode, 4);
			dwPos += 4;
			// Time Zone - dwPos 16
			m_cTimeZone = szSource[TCP1020_HEADER_SIZE + 5 + 2 + 2 + 2 + 2];
			szDestination[dwPos] = m_cTimeZone;
			dwPos += 1;
			// dwPos 17 ~ 18
			for(i = 0; i < 2; i++)
			{
				szDestination[dwPos] = 0;
				dwPos += 1;
			}
			// dwPos 19
			szDestination[dwPos] = cDIO;
			dwPos+=1;
			// dwPos 20 ~ 23
			memcpy(&szDestination + dwPos, &dwCount, sizeof(DWORD));
			dwPos += sizeof(DWORD);
			// Resolution.  dwPos 24
			szDestination[dwPos] = m_RawFileHeader.dwResolution;
			dwPos += 1;
			// Bitrate dwPos 25
			szDestination[dwPos] = m_RawFileHeader.dwBiteRate;
			dwPos+=1;
			// FPS Mode
			szDestination[dwPos] = dwFpsMode;
			dwPos += 1;
			// FPS number
			szDestination[dwPos] = dwFpsNumber;
			dwPos +=1;
			// Reserve area
			memcpy(szDestination + dwPos, szTemp16, 16);
			dwPos += 16;
			// B0
			memcpy(szDestination + dwPos, &dwB0, sizeof(DWORD));
			dwPos += sizeof(DWORD);
			// B0 Data
			szDestination[dwPos] = cB0;
			dwPos += 1;
			// B5
			memcpy(szDestination + dwPos, &dwB5, sizeof(DWORD));
			dwPos += sizeof(DWORD);
			// B5 Data
			szDestination[dwPos] = cB5;
			dwPos+=1;
			// Sequence Header
			memcpy(szDestination + dwPos, GetMpg4Header(dwResolution, m_RawFileHeader.dwFps), TCP20_00_DATA_SIZE);
			dwPos += TCP20_00_DATA_SIZE;
			// MP4 Data start from B3 GOP
			memcpy(szDestination + dwPos, szSource + TCP1020_HEADER_SIZE + TCP10_CUSTOMER_DATA_SIZE, dwDataLength - TCP10_CUSTOMER_DATA_SIZE);
			dwDLen = dwDataLength;
		}
		break;
	case FRAME_TYPE_P:
		{
			// B2 Header
			memcpy(szDestination, &dwB2, sizeof(DWORD));
			dwPos += sizeof(DWORD);
			// Video Type
			memcpy(szDestination + dwPos, &dwVideo, sizeof(DWORD));
			dwPos += sizeof(DWORD);
			// Length of the data
			// Data length = total length - length header + TCP 2.0 Video Header
			dwDataLength = dwSLen - TCP1020_HEADER_SIZE + TCP20_VIDEO_HEADER_DATA_SIZE;
			memcpy(szDestination + dwPos, &dwDataLength, sizeof(DWORD));
			dwPos += sizeof(DWORD);
			// Time code
			memcpy(szDestination + dwPos, m_szTimeCode, 4);
			dwPos += 4;
			// Time Zone
			szDestination[dwPos] = m_cTimeZone;
			dwPos += 1;
			// dwPos 17 ~ 18
			for(i = 0; i < 2; i++)
			{
				szDestination[dwPos] = 0;
				dwPos += 1;
			}
			// dwPos 19
			szDestination[dwPos] = cDIO;
			dwPos+=1;
			// dwPos 20 ~ 23
			memcpy(&szDestination + dwPos, &dwCount, sizeof(DWORD));
			dwPos += sizeof(DWORD);
			// Resolution.  dwPos 24
			szDestination[dwPos] = m_RawFileHeader.dwResolution;
			dwPos += 1;
			// Bitrate dwPos 25
			szDestination[dwPos] = m_RawFileHeader.dwBiteRate;
			dwPos+=1;
			// FPS Mode
			szDestination[dwPos] = dwFpsMode;
			dwPos += 1;
			// FPS number
			szDestination[dwPos] = dwFpsNumber;
			dwPos +=1;
			// Reserve area
			memcpy(szDestination + dwPos, szTemp16, 16);
			dwPos += 16;
			// MP4 Data start from B3 GOP
			memcpy(szDestination + dwPos, szSource + TCP1020_HEADER_SIZE, dwDataLength);
			dwDLen = dwDataLength;
		}
		break;
	}
}

// Get sequence header for varialbe frame rate
unsigned char * CMpeg4Reader::GetMpg4Header( short nResolution )
{
	return &(VariableHeader[nResolution][0]);	
}

// Get sequence header for constant frame rate
unsigned char * CMpeg4Reader::GetMpg4Header( short nResolution, short nFPs )
{ 
	return ( GetTCPVersion() > 1 ) ? &(ConstantHeader2[(((nResolution*30) + nFPs) - 1)][0]) : &(ConstantHeader[(((nResolution*30) + nFPs) - 1)][0]);
}

// Get raw header file information 
void CMpeg4Reader::GetRawHeader(RawFileHeader_t& HeaderInfo)
{
	memcpy(&HeaderInfo, &m_RawFileHeader, sizeof(RawFileHeader_t));
}

/** \brief Get raw tail file information 
*	\param TailInfo struct RawFileTail_t
*/
void CMpeg4Reader::GetRawTail(RawFileTail_t& TailInfo)
{
	memcpy(&TailInfo, &m_RawFileTail, sizeof(RawFileTail_t));
}


// Get first I frame of file.
bool CMpeg4Reader::GetFirstIFrame(unsigned char* szFrame, DataInfo_t &DataInfo)
{									
	// Set file position to first frame (after header)
	SetFramePos(sizeof(RawFileHeader_t));
	if(GetNextIFrame(szFrame, DataInfo))
	{
		return true;
	}
	return false;
}

// Set file position to first I
bool CMpeg4Reader::SetFirstIPos()
{
	memset(m_szTempFrame2, 0x00, FRAME_LEN);

	DataInfo_t DataInfo;

	// Set file position to first frame (after header)
	if(GetFirstIFrame(m_szTempFrame2, DataInfo))
	{
		m_dwFirstIPos = GetFilePos() - sizeof(TCP10Header_t) - DataInfo.dwDataLength;
		SetFramePos(m_dwFirstIPos, true);
		return true;
	}
	return false;
}

// Get previous I fame.
bool CMpeg4Reader::GetPrevIFrame(unsigned char* szFrame, DataInfo_t &DataInfo)
{
	// If the current file position is less than first I frame or greater than file size, we get first i frame
	if(GetFilePos() <= m_dwFirstIPos || GetFilePos() >= m_dwFileSize - sizeof(RawFileTail_t))
	{
		return false;
	}

	// At first i frame
	if(m_dwFirstIPos ==  m_dwCurIPos)
	{
		return false;
	}

	IFrameInfo* pTempIFInfo = NULL;;
	// Use -1 here becuase weant to find the i frame of previous one.
	FindIFrameInfoFromIndex(METHOD_FILE_POS, m_dwCurIPos - 1, &pTempIFInfo);
	SetFramePos(pTempIFInfo->dwFilePos, true);
	return GetNextFrame(szFrame, DataInfo);
}

bool CMpeg4Reader::SendCommand(DWORD dwCommand, char* pData, DWORD dwDataLen)
{
	if(NULL == m_fp)
		return false;

	switch(dwCommand) 
	{
		// Given ith I frame and seek to there.
	case PLY_REQ_SET_POSITION:
		{
			DWORD dwPos;
			memcpy(&dwPos, pData, sizeof(DWORD));
			return SetIPosFromIndex(PLY_REQ_SET_POSITION, dwPos);
		}
		break;
		// Given a time code and seek to there.
	case PLY_REQ_SET_TIMECODE:
		{
			DWORD dwTimeCode;
			memcpy(&dwTimeCode, pData, sizeof(DWORD));
			return SetIPosFromIndex(PLY_REQ_SET_TIMECODE, dwTimeCode);
		}
		break;
	default:
		return false;
	}
	return true;
}

// Add I frame information to link list
void CMpeg4Reader::AddIFrameInfoToIndex(DWORD dwFilePos, DWORD dwTimeCode)
{
	// 已經從 idx file 產生過 index.  m_llIFrameIndex is generated already
	if( m_bIdxWritenFlag == true )
	{
		return;
	}

	IFrameInfo* pNewIFinfo;
	IFrameInfo* pTempIFInfo;
	bool bAddNewNode = false;
	// Get the last i frame information.
	pTempIFInfo = m_llIFrameIndex.Last();
	
	if(NULL == pTempIFInfo)
	{
		bAddNewNode = true;
	}
	else
	{
		// If last file position is great than the one we going to add then ignore it (already add)
		if(pTempIFInfo->dwFilePos >= dwFilePos)
		{
			bAddNewNode = false;
		}
		// Add information is it is great than the last one
		else
		{
			bAddNewNode = true;
		}
	}
	if(bAddNewNode)
	{
		pNewIFinfo = new IFrameInfo;
		pNewIFinfo->dwFilePos = dwFilePos;
		pNewIFinfo->dwTimeCode = dwTimeCode;
		m_llIFrameIndex.Add(pNewIFinfo);
		pNewIFinfo =  NULL;
	}
}

// Using link list to find i frame information and set correct file position
// dwType - PLY_REQ_SET_POSITION - Set by position.
//		  - PLY_REQ_SET_TIMECODE - Set by time code.
// dwTimeCode - Given time code
// dwIPos - Give dwIPosth position
bool CMpeg4Reader::SetIPosFromIndex(DWORD dwType, DWORD dwData)
{
	// Get current file position and I frame position for backup
	long lCurrentPos =  ftell(m_fp);
	long lCurrentIPos = m_dwCurIPos;

	IFrameInfo* pTempIFInfo;
	switch(dwType)
	{
	case PLY_REQ_SET_POSITION:
		{
			// Get the number of index in the link list
			DWORD dwCount = m_llIFrameIndex.Count();
			// link list count great than the one we going to find.
			if(dwCount > dwData)
			{
				pTempIFInfo =  m_llIFrameIndex.First();
				// Get the i frame information
				for(int i = 0; i < dwData; i++)
					pTempIFInfo =  m_llIFrameIndex.Next();
				// Set to that position
				SetFramePos(pTempIFInfo->dwFilePos, true);
				return true;
			}
			else
			{
				// Set file pointer to last i frame in the link list
				pTempIFInfo =  m_llIFrameIndex.Last();
				SetFramePos(pTempIFInfo->dwFilePos, true);
			}

			DataInfo_t DataInfo;
			// Start searching for dwIPosth I frame.
			while(dwCount < dwData)
			{
				if(GetNextIFrame(m_szTempFrame2, DataInfo))
				{
					dwCount = m_llIFrameIndex.Count();
				}
				else
				{
					// End of file or maybe error
					// Set back to original position
					SetFramePos(lCurrentPos);
					m_dwCurIPos = lCurrentIPos;
					return false;
				}
			}
			// FOUND!! Set to the position.
			pTempIFInfo =  m_llIFrameIndex.Last();
			SetFramePos(pTempIFInfo->dwFilePos, true);
			return true;
		}
		break;
	case PLY_REQ_SET_TIMECODE:
		{
			// Get the last I frame information.
			pTempIFInfo =  m_llIFrameIndex.Last();
			// LUCKY....
			if(pTempIFInfo->dwTimeCode == dwData)
			{
				SetFramePos(pTempIFInfo->dwFilePos, true);
				return true;
			}
			// The last time code is less than the one we want to find.
			if(pTempIFInfo->dwTimeCode < dwData)
			{
				// Set to last I frame position from link list.
				SetFramePos(pTempIFInfo->dwFilePos, true);
				DataInfo_t DataInfo;
				IFrameInfo TempIFInfoPrev;
				// Start to get frame until the time code is great than the one we want to find
				while(pTempIFInfo->dwTimeCode < dwData)
				{
					// Back up the previous I frame information
					TempIFInfoPrev.dwFilePos = pTempIFInfo->dwFilePos;
					TempIFInfoPrev.dwTimeCode = pTempIFInfo->dwTimeCode;
				
					if(GetNextIFrame(m_szTempFrame2, DataInfo))
					{
						pTempIFInfo = m_llIFrameIndex.Last();
					}
					else
					{
						// End of file or maybe error
						// Set back to original position
						SetFramePos(lCurrentPos);
						m_dwCurIPos = lCurrentIPos;
						return false;
					}
				}
				// Found!! Set to previous frame.
				SetFramePos(TempIFInfoPrev.dwFilePos, true);
				return true;
			}
			// The last time code is great than the one we want to find
			else
			{
				// Find I frame from link list
				FindIFrameInfoFromIndex(METHOD_FILE_TIME_CODE, dwData, &pTempIFInfo);
				SetFramePos(pTempIFInfo->dwFilePos, true);
				return true;
			}
		}
		break;
	default:
		return false;
	}
	return true;
}

bool CMpeg4Reader::FindIFrameInfoFromIndex(DWORD dwMethod, DWORD dwData, IFrameInfo** pIFInfo)
{
	bool bFind = false;
	IFrameInfo* pTempIFInfo;
	IFrameInfo* pTempIFInfoPrev;

	// Check the last information.
	pTempIFInfo = m_llIFrameIndex.Last();

	if(pTempIFInfo == NULL)
		return false;

	if(dwMethod ==  METHOD_FILE_POS)
	{
		if(pTempIFInfo->dwFilePos <= dwData)
		{
			*pIFInfo = pTempIFInfo;
			return true;
		}
	}
	else if(dwMethod == METHOD_FILE_TIME_CODE)
	{
		if(pTempIFInfo->dwTimeCode <= dwData)
		{
			*pIFInfo = pTempIFInfo;
			return true;
		}
	}

	// Start search from first one.
	pTempIFInfo = m_llIFrameIndex.First();
	pTempIFInfoPrev = pTempIFInfo;
	
	while(!bFind)
	{
		if(dwMethod == METHOD_FILE_POS)
		{
			if(pTempIFInfo->dwFilePos > dwData)
			{
				*pIFInfo = pTempIFInfoPrev;
				return true;
			}
		}
		else if(dwMethod == METHOD_FILE_TIME_CODE)
		{
			if(pTempIFInfo->dwTimeCode > dwData)
			{
				*pIFInfo = pTempIFInfoPrev;
				return true;
			}
		}
		pTempIFInfoPrev = pTempIFInfo;
		pTempIFInfo = m_llIFrameIndex.Next();

	}
	return true;
}

// Get previous frame
bool CMpeg4Reader::GetPrevFrame(unsigned char* szFrame, DataInfo_t &DataInfo)
{
	// check file position
	if(GetFilePos() <= m_dwFirstIPos || GetFilePos() >= m_dwFileSize)
		return false;

	// Keep the target position
	DWORD dwTargetPos = GetFilePos();
	
	// Find I position from index.
	IFrameInfo* pTempIFInfo =  NULL;
	if(!FindIFrameInfoFromIndex(METHOD_FILE_POS, dwTargetPos, &pTempIFInfo))
		return false;
	// Seek to that position and ready to search.
	SetFramePos(pTempIFInfo->dwFilePos, true);

	DWORD dwPrevPos = GetFilePos();
	DWORD dwCurrentPos = dwPrevPos;
	
	// It is an I frame
	if(dwTargetPos == dwCurrentPos)
	{
		if(!FindIFrameInfoFromIndex(METHOD_FILE_POS, dwTargetPos - 1, &pTempIFInfo))
			return false;
		SetFramePos(pTempIFInfo->dwFilePos, true);
	}

	IFrameInfo* pTempIFInfoFirst =  NULL;
	pTempIFInfoFirst = m_llIFrameIndex.First();
	if(pTempIFInfoFirst->dwFilePos == dwTargetPos)
	{
		// Beginning of the file.
		return false;
	}

	dwPrevPos = GetFilePos();
	dwCurrentPos = dwPrevPos;

	while(dwCurrentPos != dwTargetPos)
	{
		dwPrevPos = dwCurrentPos;
		// Get next frame
		if(GetNextFrame(szFrame, DataInfo))
		{
			if(dwCurrentPos == dwTargetPos)
			{
				break;
			}
		}
		// Get current position
		dwCurrentPos = GetFilePos();
	}

	SetFramePos(dwPrevPos);

	return true;
}

DWORD CMpeg4Reader::CheckIorP( char* pRawData, int Datalen )
{
	int i = 0;
	char cIorP;
	DWORD dwB6 = 0xB6010000;
	do 
	{
		if( memcmp( &pRawData[i], &dwB6, 4 ) == 0 )
		{
			cIorP = pRawData[i+4];
			cIorP &= 0xC0;
			if( cIorP == 64 )	// PFrame
				return FRAME_TYPE_P;
			if( cIorP == 0 )	// IFrame
				return FRAME_TYPE_I;
		}

	} while( Datalen > i++ );
	return FRAME_TYPE_UNKNOW;
}

// only works on H264 // pdata is after b2 header
int CMpeg4Reader::CheckH264IorP(BYTE* pData, DWORD dwLen )
{
	if(dwLen < 50)
		return FRAME_TYPE_UNKNOW;


	if( pData[3] == 0x01 && pData[4] == 0x67 )  // I Frame
	{
		return FRAME_TYPE_I;
	}

	return FRAME_TYPE_P; // P Frame or something else

}