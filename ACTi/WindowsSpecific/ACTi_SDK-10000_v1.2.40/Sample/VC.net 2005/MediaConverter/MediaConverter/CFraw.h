#include "vfw.h"

#include "MPEG4Reader.h"
#include "Encoder.h"
#include "FRaw.h"

class CFraw
{
private:
	//beginTimeEndTime
    RawFileTail_t    m_TailInfo;
	//DWORD            m_dwTime;
    DWORD            m_dwFileBeginTime;
    DWORD            m_dwFileEndTime; 

    // File reader
	CMpeg4Reader* m_pFileReader;		
	BYTE * m_pFrame;
	
	// Include Header TCP1020_HEADER_SIZE
	DWORD m_dwFullFrameLen;				
	DataInfo_t m_DataInfo;
	bool m_bConvertFolder;
	// 1 : JPEG
	int m_nMP4ConvertFormat;		
	bool m_bFirstI;

	time_t m_tSpecifyBeginDateTime;
	time_t m_tSpecifyEndDateTime;

	HANDLE m_hRecordHandle;
	FFRAW::RAWFILE_HEADER_INFO m_RawFileHeaderInfo;
	FFRAW::RAWFILE_RECORD_INFO m_RawFileRecordInfo;

	DWORD m_dwResolution;
	DWORD m_dwFPS;
	DWORD m_dwBitrate;

	int m_nWidth;
	int m_nHeight;
	int m_nTVStd;

	DWORD m_dwIFrameCount;
	DWORD m_dwPFrameCount;
public:
	CFraw();
	virtual ~CFraw (void);

	void SetConvertFolderFlag(bool bFlag)
	{
		m_bConvertFolder = bFlag;
	}

	BOOL CreateFile( const DWORD dwBiteRate, const DWORD dwFps, const DWORD dwResolution, char * pszFileName, char * pszFilePath);
	void CloseFile( void);

	BOOL InitFileReader( char * pszRawFileName );
	void ReleaseFileReader( void);
	
	BOOL WriteFile( DWORD dwMSGType, DWORD dwFrameType, BYTE * pData, DWORD dwDataLen );

	void SetVideoSize( int nResolution );
	BOOL CheckPeriodOfTime(DWORD dwTime);

	BOOL ConvertFile( char * pszFileName,  char *pcaRawFileName, char * pszFilePath,
		const time_t tSpecifyBeginDateTime, const time_t tSpecifyEndDateTime );
};