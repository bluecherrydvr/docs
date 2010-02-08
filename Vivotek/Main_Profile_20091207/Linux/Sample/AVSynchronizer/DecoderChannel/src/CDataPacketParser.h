#ifndef DATAPACKETPARSER_H
#define DATAPACKETPARSER_H

#include "datapacketdef.h"


class CDataPacketParser
{
private:
	FILE *m_pf;

	DWORD m_dwTLVExtBufSize;
	BYTE *m_pbyTLVExt;

	DWORD m_dwDataBufSize;
	BYTE *m_pbyDataBuf;
	
	DWORD m_dwVExtBufSize;
	BYTE *m_pbyVExtBuf;
	void  EmptyBuffer();

public:
	CDataPacketParser(char *pszFilePath);
	~CDataPacketParser();

	SCODE GetOneMediaDataPacketInfo(TMediaDataPacketInfoV3 *ptPacketInfo);
	DWORD GetMediaStreamType();
};

#endif  // DATAPACKETPARSER_H
