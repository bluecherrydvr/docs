#include <string>
#include "parsedatapacket.h"
#include "mediatypedef.h"
#include "AVSynchronizer.h"
#include "CDataPacketParser.h"
#include <exception>

#define LI_SIZE 4  // Length indicator size


// Function     : HTONL
// Description  : Conversion between big-endian and little-endian
// Parameter    : 
//              | [in] dwVal @32-bit unsigned integer variable 
// Return value : 
//              | dwRetVal @The converted value
//
inline DWORD HTONL(DWORD dwVal)
{
	DWORD dwRetVal = ((dwVal >> 24) & 0x000000FF) | ((dwVal << 8) & 0x00FF0000) | ((dwVal >> 8) & 0x0000FF00) | ((dwVal << 24) & 0xFF000000);

	return dwRetVal;
}

// Function     : CDataPacketParser::CDataPacketParser
// Description  : Constructor
// Parameter    : 
//              | [in] pszFilePath @File path
// Return value : N/A
//
CDataPacketParser::CDataPacketParser(char *pszFilePath)
{
	// Notice! 
	// Open the file use binary mode 'b'
	//
	m_pf = fopen(pszFilePath, "rb");
	
	if (!m_pf)
	{
		printf("open file %s fail\n", pszFilePath);
		throw std::exception();
	}

	m_dwTLVExtBufSize = 0;
	m_pbyTLVExt  = NULL;

	m_dwDataBufSize = 0;
	m_pbyDataBuf = NULL;

	m_dwVExtBufSize = 0;
	m_pbyVExtBuf = NULL;
}

// Function     : CDataPacketParser::~CDataPacketParser
// Description  : Destructor
// Parameter    : N/A
// Return value : N/A
//
CDataPacketParser::~CDataPacketParser()
{
	fclose(m_pf);

	EmptyBuffer();
}

// Function     : CDataPacketParser::GetOneMediaDataPacketInfo
// Description  : Get one media data packet info from file
// Parameter    : 
//              | [out] ptPacketInfo @Pointer to TMediaDataPacketInfoV3 structure
// Return value :
//              | S_OK   @Success
//              | S_FAIL @Fail
//
SCODE CDataPacketParser::GetOneMediaDataPacketInfo(TMediaDataPacketInfoV3 *ptPacketInfo) 
{	
	memset(ptPacketInfo, 0, sizeof(TMediaDataPacketInfoV3));

	ptPacketInfo->tIfEx.tRv1.tExt.dwStructSize = sizeof(TMediaDataPacketInfoV3);

	DWORD dwRead;

	// STEP 1: Read data into buffer and assign to "m_pbyTLVExt"
	DWORD dwTLVExtSizeLI, dwTLVExtSize;

	dwRead = fread(&dwTLVExtSizeLI, 1, LI_SIZE, m_pf);

	if	(dwRead != LI_SIZE)
	{
		return S_FAIL;
	}

	dwTLVExtSize = HTONL(dwTLVExtSizeLI);

	if (m_dwTLVExtBufSize  < dwTLVExtSize + LI_SIZE)
	{
		delete [] m_pbyTLVExt;
 		m_pbyTLVExt = new BYTE [dwTLVExtSize + LI_SIZE];
 	}

	memcpy(m_pbyTLVExt, &dwTLVExtSizeLI, LI_SIZE);

	fread(m_pbyTLVExt + LI_SIZE, 1, dwTLVExtSize, m_pf);

	ptPacketInfo->tIfEx.tRv1.tExt.pbyTLVExt = m_pbyTLVExt;

	ptPacketInfo->tIfEx.tRv1.tExt.dwTLVExtLen = dwTLVExtSize + LI_SIZE;

	// STEP 2: Read data into buffer and assign to "m_pbyBuff" 
	DWORD dwFrameSizeLI, dwFrameSize;

	dwRead = fread(&dwFrameSizeLI, 1, LI_SIZE, m_pf);

	if	(dwRead != LI_SIZE)
	{
		return S_FAIL;
	}

	dwFrameSize = HTONL(dwFrameSizeLI);

	if (m_dwDataBufSize < dwFrameSize + LI_SIZE)
	{
		delete [] m_pbyDataBuf;
		m_pbyDataBuf = new BYTE [dwFrameSize + LI_SIZE];
	}

	memcpy(m_pbyDataBuf, &dwFrameSizeLI, LI_SIZE);

	fread(m_pbyDataBuf + LI_SIZE, 1, dwFrameSize, m_pf);

	ptPacketInfo->tIfEx.tInfo.pbyBuff = m_pbyDataBuf;

	// STEP 3: Read data into buffer and assign to "m_pbyVExtBuf"
	DWORD dwVExtSizeLI, dwVExtSize;

	dwRead = fread(&dwVExtSizeLI, 1, LI_SIZE, m_pf);

	if	(dwRead != LI_SIZE)
	{
		return S_FAIL;
	}

	dwVExtSize = HTONL(dwVExtSizeLI);

	if (m_dwVExtBufSize < dwVExtSize + LI_SIZE)
	{
		delete [] m_pbyVExtBuf;
		m_pbyVExtBuf = new BYTE [dwVExtSize + LI_SIZE];
	}

	memcpy(m_pbyVExtBuf, &dwVExtSizeLI, LI_SIZE);

	fread(m_pbyVExtBuf + LI_SIZE, 1, dwVExtSize, m_pf);

	ptPacketInfo->pbyVExtBuf = m_pbyVExtBuf;

	// STEP 4: Retrieve media data packet info
	SCODE scRet = S_OK;
	
	scRet = DataPacket_ParseV3(ptPacketInfo);

	return scRet;
}

// Function     : CDataPacketParser::GetMediaStreamType
// Description  : Get the media data packet stream type in the file
// Parameter    : N/A
// Return value :
//              | EMEDIATYPE @Media type enumeration
//
DWORD CDataPacketParser::GetMediaStreamType()
{
	BOOL bVideo = FALSE;
	BOOL bAudio = FALSE;

	TMediaDataPacketInfoV3 tPacketInfo;

	SCODE scRet = S_OK;

	while	(!feof(m_pf) && (scRet == S_OK))
	{
		// Get one media data packet info
		scRet = GetOneMediaDataPacketInfo(&tPacketInfo);

		if	(scRet != S_OK)
		{
			EmptyBuffer();

			break;
		}

		if	(tPacketInfo.tIfEx.tInfo.dwStreamType & (mctJPEG | mctH263 | mctMP4V | mctDMYV))
		{
			bVideo = TRUE;
		}
		else
		{
			bAudio = TRUE;
		}

		EmptyBuffer();

		if	(bVideo && bAudio)
		{
			break;
		}
	}

	fseek(m_pf, 0, SEEK_SET);

	if	(bVideo && bAudio)
	{
		return AVSYNCHRONIZER_MEDIATYPE_AUDIO_VIDEO;
	}	
	else if	(bVideo && !bAudio)
	{
		return AVSYNCHRONIZER_MEDIATYPE_VIDEO_ONLY;
	}
	else if	(!bVideo && bAudio)
	{
		return AVSYNCHRONIZER_MEDIATYPE_AUDIO_ONLY;
	}
	else
	{
		return 0;
	}
}

// Function     : CDataPacketParser::EmptyBuffer
// Description  : Release the buffer created in function "GetOneMediaDataPacketInfo"
// Parameter    : N/A
// Return value : N/A
//
void CDataPacketParser::EmptyBuffer()
{
	if	(m_pbyTLVExt)
	{
		delete [] m_pbyTLVExt;
		
		m_pbyTLVExt = NULL;
	}

	if	(m_pbyDataBuf)
	{
		delete [] m_pbyDataBuf;

		m_pbyDataBuf = NULL;
	}

	if	(m_pbyVExtBuf)
	{
		delete [] m_pbyVExtBuf;

		m_pbyVExtBuf = NULL;
	}
}
