#include "vfw.h"

#include "MPEG4Reader.h"
#include "Encoder.h"



class CMP4Writer : public CXEncoder
{
public:
	
 CMP4Writer( const bool bOSD = false, const char * pszTitleText = "", const int nOSDTimeType = 0);
	virtual ~CMP4Writer (void);
	
	
	void SetMP4OSD( const bool bOSD, const char * pszTitleText, const int nOSDTimeType  );
	void SetMP4OSD( const bool bOSD );
	void SetMP4OSDText( const char * pszTitleText);
	void SetMP4OSDTimeType( const int nOSDTimeType );

	 int GetVideoRate(int iTVStd, int iFPS);
	void GetVideoSize(int iResolution, int* dwWidth, int* dwHeight);
	bool Open(CString csFileName,  char * pszMP4FilePath);
	bool ReleaseFileReader();
    bool Close();
	void CloseAll();
	
	bool WriteStream(int iMsgType, int iFrameType, BYTE *pbyBuffer, DWORD dwBuffLen);
    int StringToken(LPTSTR str, LPCTSTR szToken, char** szArray);
	void swap_bgr24_to_rgb24(char *mem, int n);

	//bool CreateMP4ToJpg(CString sMP4FileName);	
	bool ConvertMP4ToJpg( CString csFileName , char * pszMP4FilePath = NULL ) ;
    
	void GetTimeToStr(time_t time, char *buf );

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
	int m_iTVStd;					// TV Stander
	int m_iWidth;					// Video Width
	int m_iHeight;					// Video Height
	int m_iFPS;						// Video FPS
	int m_iBitCount;				// Bit Count.
//	int m_nDecoder ;				// IPPCODEC decoder handle ;
//	HANDLE m_hEncoder ;				// IMpeg4Encoder handle ;
	CString sMP4FileName;			// MP4 File Name
	DWORD m_dwVFrameCounter;		// Video Frame Counter
	DWORD m_dwAFrameCounter;		// Audio Frame Counter
	int m_iResolution;				// Video Resolution

	DWORD m_dwIFrameCount;
	DWORD m_dwPFrameCount;


	
	BITMAPINFOHEADER m_sBmpInfoHdr;
	
    
	//beginTimeEndTime
    RawFileTail_t    m_TailInfo;
	
 
   
   
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
    BYTE *m_bSaveBuffer;
	DWORD m_dwSaveBufferLen;

};