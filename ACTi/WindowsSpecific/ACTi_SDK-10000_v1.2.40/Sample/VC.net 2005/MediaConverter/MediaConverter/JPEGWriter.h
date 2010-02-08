//#pragma once

#include "MPEG4Reader.h"
#include "Encoder.h"
//#include "JpegEncoder.h"
//#define USEENCODER JPEGENCODER

class CJPEGWriter  : public CXEncoder
{
private:
	int m_iTVStd;					// TV Stander
	int m_iWidth;					// Video Width
	int m_iHeight;					// Video Height
	int m_iFPS;						// Video FPS
	int m_iBitCount;				// Bit Count.
//	int m_nDecoder ;				// IPPCODEC decoder handle ;
	HANDLE m_hEncoder ;				// IMpeg4Encoder handle ;
	//CString m_sAVIFileName;			// AVI File Name
	DWORD m_dwVFrameCounter;		// Video Frame Counter
	DWORD m_dwAFrameCounter;		// Audio Frame Counter
	int m_iResolution;				// Video Resolution
	DWORD m_dwAudioSampleStart;
	DWORD m_dwIFrameCount;
	DWORD m_dwPFrameCount;

	BITMAPINFOHEADER m_sBmpInfoHdr;

	CMpeg4Reader* m_pcFileReader;		// File reader
	unsigned char* m_pFrame;
	DWORD m_dwFullFrameLen;				// Include Header TCP1020_HEADER_SIZE
	DataInfo_t m_DataInfo;

	bool m_bProcessing;
	bool m_bConvertFolder;

	int m_nMP4ConvertFormat;			// 1 : JPEG
	bool m_bFirstI;
    
	//Write to JPG

    CString m_FileName;
	
    
	//
    RawFileTail_t    m_TailInfo;
	time_t m_tSpecifyBeginDateTime;
	time_t m_tSpecifyEndDateTime;
    //
	DWORD  m_dwFileBeginTime;
    DWORD  m_dwFileEndTime; 
	
	
	
	//DWORD CheckIorP(char* pRawData, int Datalen );
    void CloseAll();
	int GetSequenceHeader( BYTE* pRaw, int nRawLen, BYTE* pSEQ );
	bool GetResolutionAndFPSBySequenceHeader( unsigned char* ucSEQ, int nSEQLen, int& nWidth, int& nHeight, int& nFPS );
	bool GetResolutionAndTVStander(int iWidth, int iHeight );



public:
	CJPEGWriter( const bool bOSD = false, const char * pszTitleText = "", const int nOSDTimeType = 0);
	virtual ~CJPEGWriter(void);
	
	void SetJPGOSD( const bool bOSD, const char * pszTitleText, const int nOSDTimeType );
	void SetJPGOSD( const bool bOSD );
	void SetJPGOSDText( const char * pszTitleText);
	void SetJPGOSDTimeType( const int nOSDTimeType );
    void GetTimeToStr(time_t time, char *buf );
    
	
	CString m_szOSDText ;
	int		m_nOSDTime ;
	bool	m_bOSD ;
	
	// read raw (CMpeg4Reader* m_pcFileReader;) while(...)


    int GetVideoRate(int iTVStd, int iFPS);
	void GetVideoSize(int iResolution, int& iWidth, int& iHeight);
	bool Open(char *pFIleName, char * pPath = NULL);
	bool Close();

	
	bool InitFileReader(char *pcaRawFileName);
	bool ReleaseFileReader();
	
	bool WriteOutputFile( const char * pszJpegFileName, DWORD dwFrameType, VIDEO_B2_FRAME * b2, BYTE * pData, DWORD dwDataLen,char * pszjFilePath = NULL);
	DWORD ConvertFrame( char *pszFileName, DWORD dwMsgType, DWORD dwFrameType, BYTE * pFrame, DWORD dwFullFrameLen,char * pszjFilePath = NULL);
	
	bool ConvertMultiJPGFile( char * pszFileName, char *pcaRawFileName,char * pszjFilePath = NULL,
		const time_t tSpecifyBeginDateTime = 0, const time_t tSpecifyEndDateTime = 0 );
	
	bool CheckPeriodOfTime(DWORD dwTime);

	DWORD ConvertVideoFrame(int iMsgType, int iFrameType, BYTE *pbyBuffer, DWORD dwBuffLen);
	int  ConvertToJPG(int iMsgType, unsigned char* pFrame, DWORD dwBuffLen );
	bool IsProcessing()
	{
		return m_bProcessing;
	}
	void SetProcessing(bool bProcessing)
	{
		m_bProcessing = bProcessing;
	}

	void GetConvertInfo(DWORD& m_nIFrame, DWORD& m_nPFrame, DWORD& m_nAFrame);

	void SetConvertFolderFlag(bool bFlag)
   	{
    	m_bConvertFolder = bFlag;
	}
   void SetMP4ConvertFormat(int nFormat)
	{
		m_nMP4ConvertFormat = 1;
	}
  

};
