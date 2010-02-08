#include "vfw.h"

#include "MPEG4Reader.h"
#include "Encoder.h"
//#include "JpegEncoder.h"
//#define USEENCODER JPEGENCODER


class CAVIWriter : public CXEncoder
{
public:
	const DWORD MAX_FRAMESIZE;
 CAVIWriter( const bool bOSD = false, const char * pszTitleText = "", const int nOSDTimeType = 0);
	virtual ~CAVIWriter (void);
	
	void SetOSD( const bool bOSD, const char * pszTitleText, const int nOSDTimeType  );
	void SetOSD( const bool bOSD );
	void SetOSDText( const char * pszTitleText);
	void SetOSDTimeType( const int nOSDTimeType );

	 int GetVideoRate(int iTVStd, int iFPS);
	void GetVideoSize(int iResolution, int& iWidth, int& iHeight);
	bool Open(char *pFIleName, char * pPath = NULL );
	bool Close();
	bool InitFileReader(char *pcaRawFileName);
	bool ReleaseFileReader();
	bool WriteStream(int iMsgType, int iFrameType, BYTE *pbyBuffer, DWORD dwBuffLen);

	bool CreateAVIFile(char *pcaAVIFileName, char *pcaRawFileName, char * pszAVIFilePath = NULL,
		const time_t tSpecifyBeginDateTime = 0, const time_t tSpecifyEndDateTime = 0);
	
	bool CheckPeriodOfTime(DWORD dwTime);
	bool IsOverSpecifyEndTime( const DWORD dwTime);
    
	void GetTimeToStr(time_t time, char *buf );

	int WriteOSD( VIDEO_B2_FRAME *, DWORD, int iMsgType, unsigned char* pFrame, DWORD dwBuffLen) ;
	bool IsProcessing()
	{
		return m_bProcessing;
	}
	void SetProcessing(bool bProcessing)
	{
		m_bProcessing = bProcessing;
	}
   // DWORD& dwIFrame, DWORD& dwPFrame, DWORD& dwAFram;
	void GetConvertInfo(DWORD& m_nIFrame, DWORD& m_nPFrame, DWORD& m_nAFrame);
	void SetConvertFolderFlag(bool bFlag)
	{
		m_bConvertFolder = bFlag;
	}
	void SetMP4ConvertFormat(int nFormat)
	{
		m_nMP4ConvertFormat = 1;
	}
   
	CString m_szOSDText ;
	int		m_nOSDTime ;
	bool	m_bOSD ;

private:
	bool CreateVideoStream();
	bool CreateAudioStream();
	bool DestroyVideoStream();
	bool DestroyAudioStream();
	DWORD CheckIorP(char* pRawData, int Datalen );
	void CloseAll();
	int GetSequenceHeader( BYTE* pRaw, int nRawLen, BYTE* pSEQ );
	bool GetResolutionAndFPSBySequenceHeader( unsigned char* ucSEQ, int nSEQLen, int& nWidth, int& nHeight, int& nFPS );
	bool GetResolutionAndTVStander(int iWidth, int iHeight );

private:
	int m_iTVStd;					// TV Stander
	int m_iWidth;					// Video Width
	int m_iHeight;					// Video Height
	int m_iFPS;						// Video FPS
	int m_iBitCount;				// Bit Count.
//	int m_nDecoder ;				// IPPCODEC decoder handle ;
	HANDLE m_hEncoder ;				// IMpeg4Encoder handle ;
	CString m_sAVIFileName;			// AVI File Name
	DWORD m_dwVFrameCounter;		// Video Frame Counter
	DWORD m_dwAFrameCounter;		// Audio Frame Counter
	int m_iResolution;				// Video Resolution
	DWORD m_dwAudioSampleStart;
	DWORD m_dwIFrameCount;
	DWORD m_dwPFrameCount;


	IAVIFile		*m_pAviFile;
	IAVIStream		*m_pAviVideo;
	IAVIStream		*m_pAviAudio;

	AVISTREAMINFO    m_sStramInfo;
	AVISTREAMINFO    m_sAudioStreamInfo;
	BITMAPINFOHEADER m_sBmpInfoHdr;
	WAVEFORMATEX     m_sWavFormat;
    
	//beginTimeEndTime
    RawFileTail_t    m_TailInfo;
	//DWORD            m_dwTime;
    DWORD            m_dwFileBeginTime;
    DWORD            m_dwFileEndTime; 
   
   
    // File reader
	CMpeg4Reader* m_pcFileReader;		
	unsigned char* m_pFrame;
	
	// Include Header TCP1020_HEADER_SIZE
	DWORD m_dwFullFrameLen;				
	DataInfo_t m_DataInfo;

	bool m_bProcessing;
	bool m_bConvertFolder;
    
	// 1 : JPEG
	int m_nMP4ConvertFormat;		
	bool m_bFirstI;

	time_t m_tSpecifyBeginDateTime;
	time_t m_tSpecifyEndDateTime;

	double m_dAvgFrameDiffTime;
	DWORD m_dwAvgFrameDiffTimeValue;

	int m_nDiffTimeSum;
	DWORD m_dwDiffTimeSumCount;
protected:
    //Steve0828
	VIDEO_B2_FRAME m_PreB2;
	WSABUF m_pPreFrame;
	DataInfo_t m_PreDataInfo;

};