#pragma once

namespace ATCP10NS
{
	typedef void ( WINAPI *CONTROL_DATA_CALLBACK	)( DWORD UserParam, BYTE* buf, DWORD len );

	typedef struct structural_MEDIA_CONNECTION_CONFIG
	{		
		int	ContactType;

		unsigned char	ChannelNumber;	// For URL Command CHANNEL tag, when set it to 0, URL command won't bring CHANNEL tag, or the URL command will bring CHANNEL=ChannelNumber tag  )
		unsigned char	TCPVideoStreamID;	// 0 based to specify video track, value 0 to 255 for 1 to 256 video track
		unsigned char	RTPVideoTrackNumber;	// set it to 0, ARTP will use 1st video track,  1 to 255 is for specify video track
		unsigned char	RTPAudioTrackNumber;	// set it to 0, ARTP will use 1st audio track,  1 to 255 is for specify audio track

		char			UniCastIP[16];
		char			MultiCastIP[16];
		char			PlayFileName[256];
		char			UserID[64];
		char			Password[64];
		unsigned long	RegisterPort;
		unsigned long	StreamingPort;
		unsigned long	ControlPort;
		unsigned long	MultiCastPort;
		unsigned long	SearchPortC2S;
		unsigned long	SearchPortS2C;
		unsigned long	HTTPPort;
		unsigned long	RTSPPort;
		unsigned long	Reserved1;
		unsigned long	Reserved2;

		unsigned short	ConnectTimeOut;
		unsigned short	EncryptionType;
	}MEDIA_CONNECTION_CONFIG;

	typedef struct structural_MEDIA_VIDEO_CONFIG
	{
		DWORD dwTvStander;			// 0:NTSC 1:PAL
		DWORD dwVideoResolution;	// See the definition above
		DWORD dwBitsRate;			// See the definition above
		DWORD dwVideoBrightness;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoContrast;		// 0 ~ 100 : Low ~ High
		DWORD dwVideoSaturation;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoHue;			// 0 ~ 100 : Low ~ High
		DWORD dwFps;				// 0 ~ 30 frame pre second
	} MEDIA_VIDEO_CONFIG;

	typedef struct structural_MEDIA_COMMAND
	{
		DWORD dwCommandType;
		char* pCommand;
		int	  nCommandLength;
		char* pResult;
		int   nResultLength;
	} MEDIA_COMMAND;

	typedef struct structural_MEDIA_MOTION_INFO
	{
		DWORD dwEnable;
		DWORD dwRangeCount;
		DWORD dwRange[3][4];
		DWORD dwSensitive[3];
	} MEDIA_MOTION_INFO;

	class __declspec(dllexport) ATCP10Class
	{
	public:
		ATCP10Class();
		~ATCP10Class();
		bool XConnect();
		void XDisconnect();
		void XSetMediaConfig( MEDIA_CONNECTION_CONFIG* pMediaCfg );
		void XSetVideoConfig( MEDIA_VIDEO_CONFIG* pVideoCfg );
		void XGetVideoConfig( MEDIA_VIDEO_CONFIG* pVideoCfg );
		bool XStartStreaming();
		void XStopStreaming();
		int XGetNextFrame( int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
		int XGetNextIFrame( BYTE* pFrameBuffer, int nBufferSize );
		int XGetPrevFrame( int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
		int XGetPrevIFrame( BYTE* pFrameBuffer, int nBufferSize );
		bool XSendCommand( MEDIA_COMMAND* pMediaCommand );
		void XSetControlDataCallBack( DWORD UserParam, CONTROL_DATA_CALLBACK fnCallBack );
		void XSetMotionInfo( MEDIA_MOTION_INFO* Motioninfo );
		void XGetMotionInfo( MEDIA_MOTION_INFO* Motioninfo );
		DWORD XGetBeginTime();
		DWORD XGetEndTime();
	private:
		void *m_pAVC;
	};

	extern "C" __declspec(dllexport) void XT1GetVersion( HANDLE h, char* VersionInfo );
	extern "C" __declspec(dllexport) HANDLE XT1Init();
	extern "C" __declspec(dllexport) void XT1Exit( HANDLE h );
	extern "C" __declspec(dllexport) bool XT1Connect( HANDLE h );
	extern "C" __declspec(dllexport) void XT1Disconnect( HANDLE h );
	extern "C" __declspec(dllexport) void XT1SetMediaConfig( HANDLE h, MEDIA_CONNECTION_CONFIG* pMediaCfg );
	extern "C" __declspec(dllexport) void XT1SetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
	extern "C" __declspec(dllexport) void XT1GetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
	extern "C" __declspec(dllexport) bool XT1StartStreaming( HANDLE h );
	extern "C" __declspec(dllexport) void XT1StopStreaming( HANDLE h );
	extern "C" __declspec(dllexport) int XT1GetNextFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XT1GetNextIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XT1GetPrevFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XT1GetPrevIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) bool XT1SendCommand( HANDLE h, MEDIA_COMMAND* pMediaCommand );
	extern "C" __declspec(dllexport) void XT1SetControlDataCallBack( HANDLE h, DWORD UserParam, CONTROL_DATA_CALLBACK fnCallBack );
	extern "C" __declspec(dllexport) void XT1SetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
	extern "C" __declspec(dllexport) void XT1GetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
	extern "C" __declspec(dllexport) DWORD XT1GetBeginTime( HANDLE h );
	extern "C" __declspec(dllexport) DWORD XT1GetEndTime( HANDLE h );
	extern "C" __declspec(dllexport) void XT1SetDebugMessageLevel( HANDLE h, int nDebugLevel );
}


namespace AMCST10NS
{

	typedef void ( WINAPI *CONTROL_DATA_CALLBACK	)( DWORD UserParam, BYTE* buf, DWORD len );

	typedef struct structural_MEDIA_CONNECTION_CONFIG
	{		
		int	ContactType;

		unsigned char	ChannelNumber;	// For URL Command CHANNEL tag, when set it to 0, URL command won't bring CHANNEL tag, or the URL command will bring CHANNEL=ChannelNumber tag  )
		unsigned char	TCPVideoStreamID;	// 0 based to specify video track, value 0 to 255 for 1 to 256 video track
		unsigned char	RTPVideoTrackNumber;	// set it to 0, ARTP will use 1st video track,  1 to 255 is for specify video track
		unsigned char	RTPAudioTrackNumber;	// set it to 0, ARTP will use 1st audio track,  1 to 255 is for specify audio track

		char			UniCastIP[16];
		char			MultiCastIP[16];
		char			PlayFileName[256];
		char			UserID[64];
		char			Password[64];
		unsigned long	RegisterPort;
		unsigned long	StreamingPort;
		unsigned long	ControlPort;
		unsigned long	MultiCastPort;
		unsigned long	SearchPortC2S;
		unsigned long	SearchPortS2C;
		unsigned long	HTTPPort;
		unsigned long	RTSPPort;
		unsigned long	Reserved1;
		unsigned long	Reserved2;

		unsigned short	ConnectTimeOut;
		unsigned short	EncryptionType;
	}MEDIA_CONNECTION_CONFIG;

	typedef struct structural_MEDIA_VIDEO_CONFIG
	{
		DWORD dwTvStander;			// 0:NTSC 1:PAL
		DWORD dwVideoResolution;	// See the definition above
		DWORD dwBitsRate;			// See the definition above
		DWORD dwVideoBrightness;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoContrast;		// 0 ~ 100 : Low ~ High
		DWORD dwVideoSaturation;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoHue;			// 0 ~ 100 : Low ~ High
		DWORD dwFps;				// 0 ~ 30 frame pre second
	} MEDIA_VIDEO_CONFIG;

	typedef struct structural_MEDIA_COMMAND
	{
		DWORD dwCommandType;
		char* pCommand;
		int	  nCommandLength;
		char* pResult;
		int   nResultLength;
	} MEDIA_COMMAND;

	typedef struct structural_MEDIA_MOTION_INFO
	{
		DWORD dwEnable;
		DWORD dwRangeCount;
		DWORD dwRange[3][4];
		DWORD dwSensitive[3];
	} MEDIA_MOTION_INFO;


	extern "C" __declspec(dllexport) HANDLE XAMCST10Init();
	extern "C" __declspec(dllexport) void XAMCST10SetMediaConfig( HANDLE h, MEDIA_CONNECTION_CONFIG* pMediaCfg );
	extern "C" __declspec(dllexport) bool XAMCST10Connect( HANDLE h );
	extern "C" __declspec(dllexport) void XAMCST10SetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
	extern "C" __declspec(dllexport) void XAMCST10GetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
	extern "C" __declspec(dllexport) bool XAMCST10StartStreaming( HANDLE h );
	extern "C" __declspec(dllexport) void XAMCST10StopStreaming( HANDLE h );
	extern "C" __declspec(dllexport) int XAMCST10GetNextFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XAMCST10GetPrevFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XAMCST10GetNextIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XAMCST10GetPrevIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) void XAMCST10Disconnect( HANDLE h );
	extern "C" __declspec(dllexport) bool XAMCST10SendCommand( HANDLE h, MEDIA_COMMAND* pMediaCommand );
	extern "C" __declspec(dllexport) void XAMCST10Exit( HANDLE h );
	extern "C" __declspec(dllexport) void XAMCST10SetControlDataCallBack( HANDLE h, DWORD UserParam, CONTROL_DATA_CALLBACK fnCallBack );
	extern "C" __declspec(dllexport) void XAMCST10SetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
	extern "C" __declspec(dllexport) void XAMCST10GetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
}

namespace ATCP20
{
	typedef void ( WINAPI *CONTROL_DATA_CALLBACK	)( DWORD UserParam, BYTE* buf, DWORD len );

	typedef struct structural_MEDIA_CONNECTION_CONFIG
	{		
		int	ContactType;

		unsigned char	ChannelNumber;	// For URL Command CHANNEL tag, when set it to 0, URL command won't bring CHANNEL tag, or the URL command will bring CHANNEL=ChannelNumber tag  )
		unsigned char	TCPVideoStreamID;	// 0 based to specify video track, value 0 to 255 for 1 to 256 video track
		unsigned char	RTPVideoTrackNumber;	// set it to 0, ARTP will use 1st video track,  1 to 255 is for specify video track
		unsigned char	RTPAudioTrackNumber;	// set it to 0, ARTP will use 1st audio track,  1 to 255 is for specify audio track

		char			UniCastIP[16];
		char			MultiCastIP[16];
		char			PlayFileName[256];
		char			UserID[64];
		char			Password[64];
		unsigned long	RegisterPort;
		unsigned long	StreamingPort;
		unsigned long	ControlPort;
		unsigned long	MultiCastPort;
		unsigned long	SearchPortC2S;
		unsigned long	SearchPortS2C;
		unsigned long	HTTPPort;
		unsigned long	RTSPPort;
		unsigned long	Reserved1;
		unsigned long	Reserved2;

		unsigned short	ConnectTimeOut;
		unsigned short	EncryptionType;
	}MEDIA_CONNECTION_CONFIG;

	typedef struct structural_MEDIA_VIDEO_CONFIG
	{
		DWORD dwTvStander;			// 0:NTSC 1:PAL
		DWORD dwVideoResolution;	// See the definition above
		DWORD dwBitsRate;			// See the definition above
		DWORD dwVideoBrightness;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoContrast;		// 0 ~ 100 : Low ~ High
		DWORD dwVideoSaturation;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoHue;			// 0 ~ 100 : Low ~ High
		DWORD dwFps;				// 0 ~ 30 frame pre second
	} MEDIA_VIDEO_CONFIG;

	typedef struct structural_MEDIA_COMMAND
	{
		DWORD dwCommandType;
		char* pCommand;
		int	  nCommandLength;
		char* pResult;
		int   nResultLength;
	} MEDIA_COMMAND;

	typedef struct structural_MEDIA_MOTION_INFO
	{
		DWORD dwEnable;
		DWORD dwRangeCount;
		DWORD dwRange[3][4];
		DWORD dwSensitive[3];
	} MEDIA_MOTION_INFO;

	class __declspec(dllexport) ATCP20Class
	{
	public:
		ATCP20Class();
		~ATCP20Class();
		bool XConnect();
		void XDisconnect();
		void XSetMediaConfig( MEDIA_CONNECTION_CONFIG* pMediaCfg );
		void XSetVideoConfig( MEDIA_VIDEO_CONFIG* pVideoCfg );
		void XGetVideoConfig( MEDIA_VIDEO_CONFIG* pVideoCfg );
		bool XStartStreaming();
		void XStopStreaming();
		int XGetNextFrame( int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
		int XGetNextIFrame( BYTE* pFrameBuffer, int nBufferSize );
		int XGetPrevFrame( int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
		int XGetPrevIFrame( BYTE* pFrameBuffer, int nBufferSize );
		bool XSendCommand( MEDIA_COMMAND* pMediaCommand );
		void XSetControlDataCallBack( DWORD UserParam, CONTROL_DATA_CALLBACK fnCallBack );
		void XSetMotionInfo( MEDIA_MOTION_INFO* Motioninfo );
		void XGetMotionInfo( MEDIA_MOTION_INFO* Motioninfo );
		DWORD XGetBeginTime();
		DWORD XGetEndTime();
	private:
		void *m_pAVC;
	};

	extern "C" __declspec(dllexport) void XGetVersion( HANDLE h, char* VersionInfo );
	extern "C" __declspec(dllexport) HANDLE XInit();
	extern "C" __declspec(dllexport) void XExit( HANDLE h );
	extern "C" __declspec(dllexport) bool XConnect( HANDLE h );
	extern "C" __declspec(dllexport) void XDisconnect( HANDLE h );
	extern "C" __declspec(dllexport) void XSetMediaConfig( HANDLE h, MEDIA_CONNECTION_CONFIG* pMediaCfg );
	extern "C" __declspec(dllexport) void XSetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
	extern "C" __declspec(dllexport) void XGetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
	extern "C" __declspec(dllexport) bool XStartStreaming( HANDLE h );
	extern "C" __declspec(dllexport) void XStopStreaming( HANDLE h );
	extern "C" __declspec(dllexport) int XGetNextFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XGetNextIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XGetPrevFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XGetPrevIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) bool XSendCommand( HANDLE h, MEDIA_COMMAND* pMediaCommand );
	extern "C" __declspec(dllexport) void XSetControlDataCallBack( HANDLE h, DWORD UserParam, CONTROL_DATA_CALLBACK fnCallBack );
	extern "C" __declspec(dllexport) void XSetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
	extern "C" __declspec(dllexport) void XGetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
	extern "C" __declspec(dllexport) DWORD XGetBeginTime( HANDLE h );
	extern "C" __declspec(dllexport) DWORD XGetEndTime( HANDLE h );
	extern "C" __declspec(dllexport) void XSetDebugMessageLevel( HANDLE h, int nDebugLevel );
}


namespace ARAW
{
	typedef void ( WINAPI *CONTROL_DATA_CALLBACK	)( DWORD UserParam, BYTE* buf, DWORD len );

	typedef struct structural_MEDIA_CONNECTION_CONFIG
	{		
		int	ContactType;

		unsigned char	ChannelNumber;	// For URL Command CHANNEL tag, when set it to 0, URL command won't bring CHANNEL tag, or the URL command will bring CHANNEL=ChannelNumber tag  )
		unsigned char	TCPVideoStreamID;	// 0 based to specify video track, value 0 to 255 for 1 to 256 video track
		unsigned char	RTPVideoTrackNumber;	// set it to 0, ARTP will use 1st video track,  1 to 255 is for specify video track
		unsigned char	RTPAudioTrackNumber;	// set it to 0, ARTP will use 1st audio track,  1 to 255 is for specify audio track

		char			UniCastIP[16];
		char			MultiCastIP[16];
		char			PlayFileName[256];
		char			UserID[64];
		char			Password[64];
		unsigned long	RegisterPort;
		unsigned long	StreamingPort;
		unsigned long	ControlPort;
		unsigned long	MultiCastPort;
		unsigned long	SearchPortC2S;
		unsigned long	SearchPortS2C;
		unsigned long	HTTPPort;
		unsigned long	RTSPPort;
		unsigned long	Reserved1;
		unsigned long	Reserved2;

		unsigned short	ConnectTimeOut;
		unsigned short	EncryptionType;
	}MEDIA_CONNECTION_CONFIG;

	typedef struct
	{
		DWORD dwTvStandard;			// 0:NTSC 1:PAL
		DWORD dwVideoResolution;	// See the definition above
		DWORD dwBitsRate;			// See the definition above
		DWORD dwVideoBrightness;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoContrast;		// 0 ~ 100 : Low ~ High
		DWORD dwVideoSaturation;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoHue;			// 0 ~ 100 : Low ~ High
		DWORD dwFps;				// 0 ~ 30 frame pre second
	} MEDIA_VIDEO_CONFIG;

	typedef struct
	{
		DWORD dwCommandType;
		char* pCommand;
		int	  nCommandLength;
		char* pResult;
		int   nResultLength;
	} MEDIA_COMMAND;

	typedef struct structural_MEDIA_MOTION_INFO
	{
		DWORD dwEnable;
		DWORD dwRangeCount;
		DWORD dwRange[3][4];
		DWORD dwSensitive[3];
	} MEDIA_MOTION_INFO;

	extern "C" __declspec(dllexport) HANDLE XARAWInit();
	extern "C" __declspec(dllexport) void XARAWSetMediaConfig( HANDLE h, MEDIA_CONNECTION_CONFIG* pMediaCfg );
	extern "C" __declspec(dllexport) bool XARAWConnect( HANDLE h );
	extern "C" __declspec(dllexport) void XARAWSetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
	extern "C" __declspec(dllexport) void XARAWGetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
	extern "C" __declspec(dllexport) bool XARAWStartStreaming( HANDLE h );
	extern "C" __declspec(dllexport) void XARAWStopStreaming( HANDLE h );
	extern "C" __declspec(dllexport) int XARAWGetNextFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XARAWGetPrevFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize);
	extern "C" __declspec(dllexport) int XARAWGetNextIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
	// for playback( ARAW )
	extern "C" __declspec(dllexport) int XARAWGetPrevIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) void XARAWDisconnect( HANDLE h );
	extern "C" __declspec(dllexport) bool XARAWSendCommand( HANDLE h, MEDIA_COMMAND* pMediaCommand );
	extern "C" __declspec(dllexport) void XARAWExit( HANDLE h );
	extern "C" __declspec(dllexport) void XARAWSetControlDataCallBack( HANDLE h, DWORD UserParam, CONTROL_DATA_CALLBACK fnCallBack );
	extern "C" __declspec(dllexport) void XARAWSetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
	extern "C" __declspec(dllexport) void XARAWGetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
	// for playback only ( ARAW )
	extern "C" __declspec(dllexport) DWORD XARAWGetBeginTime( HANDLE h );
	// for playback only ( ARAW )
	extern "C" __declspec(dllexport) DWORD XARAWGetEndTime( HANDLE h );
	extern "C" __declspec(dllexport) void XARAWGetVersion( HANDLE h, char* VersionInfo );
}

namespace AMCST20
{
	typedef void ( WINAPI *CONTROL_DATA_CALLBACK	)( DWORD UserParam, BYTE* buf, DWORD len );

	typedef struct structural_MEDIA_CONNECTION_CONFIG
	{		
		int	ContactType;

		unsigned char	ChannelNumber;	// For URL Command CHANNEL tag, when set it to 0, URL command won't bring CHANNEL tag, or the URL command will bring CHANNEL=ChannelNumber tag  )
		unsigned char	TCPVideoStreamID;	// 0 based to specify video track, value 0 to 255 for 1 to 256 video track
		unsigned char	RTPVideoTrackNumber;	// set it to 0, ARTP will use 1st video track,  1 to 255 is for specify video track
		unsigned char	RTPAudioTrackNumber;	// set it to 0, ARTP will use 1st audio track,  1 to 255 is for specify audio track

		char			UniCastIP[16];
		char			MultiCastIP[16];
		char			PlayFileName[256];
		char			UserID[64];
		char			Password[64];
		unsigned long	RegisterPort;
		unsigned long	StreamingPort;
		unsigned long	ControlPort;
		unsigned long	MultiCastPort;
		unsigned long	SearchPortC2S;
		unsigned long	SearchPortS2C;
		unsigned long	HTTPPort;
		unsigned long	RTSPPort;
		unsigned long	Reserved1;
		unsigned long	Reserved2;

		unsigned short	ConnectTimeOut;
		unsigned short	EncryptionType;
	}MEDIA_CONNECTION_CONFIG;

	typedef struct structural_MEDIA_VIDEO_CONFIG
	{
		DWORD dwTvStander;			// 0:NTSC 1:PAL
		DWORD dwVideoResolution;	// See the definition above
		DWORD dwBitsRate;			// See the definition above
		DWORD dwVideoBrightness;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoContrast;		// 0 ~ 100 : Low ~ High
		DWORD dwVideoSaturation;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoHue;			// 0 ~ 100 : Low ~ High
		DWORD dwFps;				// 0 ~ 30 frame pre second
	} MEDIA_VIDEO_CONFIG;

	typedef struct structural_MEDIA_COMMAND
	{
		DWORD dwCommandType;
		char* pCommand;
		int	  nCommandLength;
		char* pResult;
		int   nResultLength;
	} MEDIA_COMMAND;

	typedef struct structural_MEDIA_MOTION_INFO
	{
		DWORD dwEnable;
		DWORD dwRangeCount;
		DWORD dwRange[3][4];
		DWORD dwSensitive[3];
	} MEDIA_MOTION_INFO;

	extern "C" __declspec(dllexport) HANDLE XAMCST20Init();
	extern "C" __declspec(dllexport) void XAMCST20Exit( HANDLE h );
	extern "C" __declspec(dllexport) bool XAMCST20Connect( HANDLE h );
	extern "C" __declspec(dllexport) void XAMCST20Disconnect( HANDLE h );
	extern "C" __declspec(dllexport) void XAMCST20SetMediaConfig( HANDLE h, MEDIA_CONNECTION_CONFIG* pMediaCfg );
	extern "C" __declspec(dllexport) void XAMCST20SetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
	extern "C" __declspec(dllexport) void XAMCST20GetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
	extern "C" __declspec(dllexport) bool XAMCST20StartStreaming( HANDLE h );
	extern "C" __declspec(dllexport) void XAMCST20StopStreaming( HANDLE h );
	extern "C" __declspec(dllexport) int XAMCST20GetNextFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XAMCST20GetNextIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XAMCST20GetPrevFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XAMCST20GetPrevIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) bool XAMCST20SendCommand( HANDLE h, MEDIA_COMMAND* pMediaCommand );
	extern "C" __declspec(dllexport) void XAMCST20SetControlDataCallBack( HANDLE h, DWORD UserParam, CONTROL_DATA_CALLBACK fnCallBack );
	extern "C" __declspec(dllexport) void XAMCST20SetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
	extern "C" __declspec(dllexport) void XAMCST20GetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
	extern "C" __declspec(dllexport) DWORD XAMCST20GetBeginTime( HANDLE h );
	extern "C" __declspec(dllexport) DWORD XAMCST20GetEndTime( HANDLE h );
}

namespace ASE
{
	typedef void ( WINAPI *CONTROL_DATA_CALLBACK	)( DWORD UserParam, BYTE* buf, DWORD len );


	typedef struct structural_MEDIA_CONNECTION_CONFIG
	{		
		int	ContactType;

		unsigned char	ChannelNumber;	// For URL Command CHANNEL tag, when set it to 0, URL command won't bring CHANNEL tag, or the URL command will bring CHANNEL=ChannelNumber tag  )
		unsigned char	TCPVideoStreamID;	// 0 based to specify video track, value 0 to 255 for 1 to 256 video track
		unsigned char	RTPVideoTrackNumber;	// set it to 0, ARTP will use 1st video track,  1 to 255 is for specify video track
		unsigned char	RTPAudioTrackNumber;	// set it to 0, ARTP will use 1st audio track,  1 to 255 is for specify audio track

		char			UniCastIP[16];
		char			MultiCastIP[16];
		char			PlayFileName[256];
		char			UserID[64];
		char			Password[64];
		unsigned long	RegisterPort;
		unsigned long	StreamingPort;
		unsigned long	ControlPort;
		unsigned long	MultiCastPort;
		unsigned long	SearchPortC2S;
		unsigned long	SearchPortS2C;
		unsigned long	HTTPPort;
		unsigned long	RTSPPort;
		unsigned long	Reserved1;
		unsigned long	Reserved2;

		unsigned short	ConnectTimeOut;
		unsigned short	EncryptionType;
	}MEDIA_CONNECTION_CONFIG;

	typedef struct structural_MEDIA_VIDEO_CONFIG
	{
		DWORD dwTvStander;			// 0:NTSC 1:PAL
		DWORD dwVideoResolution;	// See the definition above
		DWORD dwBitsRate;			// See the definition above
		DWORD dwVideoBrightness;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoContrast;		// 0 ~ 100 : Low ~ High
		DWORD dwVideoSaturation;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoHue;			// 0 ~ 100 : Low ~ High
		DWORD dwFps;				// 0 ~ 30 frame pre second
	} MEDIA_VIDEO_CONFIG;

	typedef struct structural_MEDIA_COMMAND
	{
		DWORD dwCommandType;
		char* pCommand;
		int	  nCommandLength;
		char* pResult;
		int   nResultLength;
	} MEDIA_COMMAND;

	typedef struct structural_MEDIA_MOTION_INFO
	{
		DWORD dwEnable;
		DWORD dwRangeCount;
		DWORD dwRange[3][4];
		DWORD dwSensitive[3];
	} MEDIA_MOTION_INFO;

	// New struct for server configuration.
	typedef struct structural_STREAMING_ENGINE_CONFIG
	{
		char	szUserID[16];				// Streaming engine login ID
		char	szUserPwd[16];				// Streaming engine login Password
		char	szServerIP[16];				// Streaming engine IP address
		DWORD	dwStreamingPort;			// Audio/Video port number for streaming engine
		DWORD	dwControlPort;				// Control port number for streaming engine
	}STREAMING_ENGINE_CONFIG;


	typedef struct tagServerConfig 
	{
		DWORD dwTvStander;			// See the definition above
		DWORD dwVideoStander;		// See the definition above
		DWORD dwVideoResolution;	// See the definition above
		DWORD dwBitsRate;			// See the definition above
		DWORD dwVideoBrightness;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoContrast;		// 0 ~ 100 : Low ~ High
		DWORD dwVideoSaturation;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoHue;			// 0 ~ 100 : Low ~ High
		DWORD dwLan;				// See the definition above
		DWORD dwWan;				// See the definition above
		WORD  dwFps;				// 0 ~ 30 
		WORD  dwFpsMode;			// 0 ~ 1 
		DWORD dwMotionOnOff;		// 0 : Off, 1 : On
	} NET_SERVERCONFIG;

	class __declspec(dllexport) ASEClass
	{
	public:
		ASEClass();
		~ASEClass();
		bool XConnect();
		void XDisconnect();
		void XSetEngineConfig( STREAMING_ENGINE_CONFIG* pEngineConfig );
		void XSetEngineConfig2( void* pEngineConfig );
		void XSetMediaConfig( MEDIA_CONNECTION_CONFIG* pMediaCfg );
		void XSetVideoConfig( MEDIA_VIDEO_CONFIG* pVideoCfg );
		void XGetVideoConfig( MEDIA_VIDEO_CONFIG* pVideoCfg );
		bool XStartStreaming();
		void XStopStreaming();
		int XGetNextFrame( int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
		int XGetNextIFrame( BYTE* pFrameBuffer, int nBufferSize );
		int XGetPrevFrame( int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
		int XGetPrevIFrame( BYTE* pFrameBuffer, int nBufferSize );
		bool XSendCommand( MEDIA_COMMAND* pMediaCommand );
		void XSetControlDataCallBack( DWORD UserParam, CONTROL_DATA_CALLBACK fnCallBack );
		void XSetMotionInfo( MEDIA_MOTION_INFO* Motioninfo );
		void XGetMotionInfo( MEDIA_MOTION_INFO* Motioninfo );
		DWORD XGetBeginTime();
		DWORD XGetEndTime();
		void XStartTransferStreamingEngineData( bool bStatus);
		bool XAddNthMedia( DWORD Nth, char* pMediaName );
		void XRemoveNthMedia( DWORD Nth );
		void XRemoveAllMediaConfig();
	private:
		void *m_pObj;
	};

	extern "C" __declspec(dllexport) void XASEGetVersion( HANDLE h, char* VersionInfo );
	extern "C" __declspec(dllexport) HANDLE XASEInit();
	extern "C" __declspec(dllexport) void XASEExit( HANDLE h );
	extern "C" __declspec(dllexport) bool XASEConnect( HANDLE h );
	extern "C" __declspec(dllexport) void XASEDisconnect( HANDLE h );
	extern "C" __declspec(dllexport) void XASESetEngineConfig( HANDLE h, STREAMING_ENGINE_CONFIG* pEngineConfig );
	extern "C" __declspec(dllexport) void XASESetMediaConfig( HANDLE h, MEDIA_CONNECTION_CONFIG* pMediaCfg );
	extern "C" __declspec(dllexport) void XASESetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
	extern "C" __declspec(dllexport) void XASEGetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
	extern "C" __declspec(dllexport) bool XASEStartStreaming( HANDLE h );
	extern "C" __declspec(dllexport) void XASEStopStreaming( HANDLE h );
	extern "C" __declspec(dllexport) int XASEGetNextFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XASEGetNextIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XASEGetPrevFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XASEGetPrevIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) bool XASESendCommand( HANDLE h, MEDIA_COMMAND* pMediaCommand );
	extern "C" __declspec(dllexport) void XASESetControlDataCallBack( HANDLE h, DWORD UserParam, CONTROL_DATA_CALLBACK fnCallBack );
	extern "C" __declspec(dllexport) void XASESetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
	extern "C" __declspec(dllexport) void XASEGetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
	extern "C" __declspec(dllexport) DWORD XASEGetBeginTime( HANDLE h );
	extern "C" __declspec(dllexport) DWORD XASEGetEndTime( HANDLE h );
	extern "C" __declspec(dllexport) void XASESetDebugMessageLevel( HANDLE h, int nDebugLevel );
	extern "C" __declspec(dllexport) void XASEStartTransferStreamingEngineData(HANDLE h, bool bStatus);
	extern "C" __declspec(dllexport) bool XASEAddNthMedia( HANDLE h, DWORD Nth, char* pMediaName );
	extern "C" __declspec(dllexport) void XASERemoveNthMedia( HANDLE h, DWORD Nth );
	extern "C" __declspec(dllexport) void XASERemoveAllMediaConfig( HANDLE h );
}

namespace AMRAW
{
	typedef void ( WINAPI *CONTROL_DATA_CALLBACK	)( DWORD UserParam, BYTE* buf, DWORD len );


	typedef struct structural_MEDIA_CONNECTION_CONFIG
	{		
		int	ContactType;

		unsigned char	ChannelNumber;	// For URL Command CHANNEL tag, when set it to 0, URL command won't bring CHANNEL tag, or the URL command will bring CHANNEL=ChannelNumber tag  )
		unsigned char	TCPVideoStreamID;	// 0 based to specify video track, value 0 to 255 for 1 to 256 video track
		unsigned char	RTPVideoTrackNumber;	// set it to 0, ARTP will use 1st video track,  1 to 255 is for specify video track
		unsigned char	RTPAudioTrackNumber;	// set it to 0, ARTP will use 1st audio track,  1 to 255 is for specify audio track

		char			UniCastIP[16];
		char			MultiCastIP[16];
		char			PlayFileName[256];
		char			UserID[64];
		char			Password[64];
		unsigned long	RegisterPort;
		unsigned long	StreamingPort;
		unsigned long	ControlPort;
		unsigned long	MultiCastPort;
		unsigned long	SearchPortC2S;
		unsigned long	SearchPortS2C;
		unsigned long	HTTPPort;
		unsigned long	RTSPPort;
		unsigned long	Reserved1;
		unsigned long	Reserved2;

		unsigned short	ConnectTimeOut;
		unsigned short	EncryptionType;
	}MEDIA_CONNECTION_CONFIG;

	typedef struct
	{
		DWORD dwTvStandard;			// 0:NTSC 1:PAL
		DWORD dwVideoResolution;	// See the definition above
		DWORD dwBitsRate;			// See the definition above
		DWORD dwVideoBrightness;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoContrast;		// 0 ~ 100 : Low ~ High
		DWORD dwVideoSaturation;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoHue;			// 0 ~ 100 : Low ~ High
		DWORD dwFps;				// 0 ~ 30 frame pre second
	} MEDIA_VIDEO_CONFIG;

	typedef struct
	{
		DWORD dwCommandType;
		char* pCommand;
		int	  nCommandLength;
		char* pResult;
		int   nResultLength;
	} MEDIA_COMMAND;

	typedef struct structural_MEDIA_MOTION_INFO
	{
		DWORD dwEnable;
		DWORD dwRangeCount;
		DWORD dwRange[3][4];
		DWORD dwSensitive[3];
	} MEDIA_MOTION_INFO;

	extern "C" __declspec(dllexport) HANDLE XAMRAWInit();
	extern "C" __declspec(dllexport) void XAMRAWSetMediaConfig( HANDLE h, MEDIA_CONNECTION_CONFIG* pMediaCfg );
	extern "C" __declspec(dllexport) bool XAMRAWConnect( HANDLE h );
	extern "C" __declspec(dllexport) void XAMRAWSetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
	extern "C" __declspec(dllexport) void XAMRAWGetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
	extern "C" __declspec(dllexport) bool XAMRAWStartStreaming( HANDLE h );
	extern "C" __declspec(dllexport) void XAMRAWStopStreaming( HANDLE h );
	extern "C" __declspec(dllexport) int XAMRAWGetNextFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XAMRAWGetPrevFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize);
	extern "C" __declspec(dllexport) int XAMRAWGetNextIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
	// for playback( AMRAW )
	extern "C" __declspec(dllexport) int XAMRAWGetPrevIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) void XAMRAWDisconnect( HANDLE h );
	extern "C" __declspec(dllexport) bool XAMRAWSendCommand( HANDLE h, MEDIA_COMMAND* pMediaCommand );
	extern "C" __declspec(dllexport) void XAMRAWExit( HANDLE h );
	extern "C" __declspec(dllexport) void XAMRAWSetControlDataCallBack( HANDLE h, DWORD UserParam, CONTROL_DATA_CALLBACK fnCallBack );
	extern "C" __declspec(dllexport) void XAMRAWSetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
	extern "C" __declspec(dllexport) void XAMRAWGetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
	// for playback only ( AMRAW )
	extern "C" __declspec(dllexport) DWORD XAMRAWGetBeginTime( HANDLE h );
	// for playback only ( AMRAW )
	extern "C" __declspec(dllexport) DWORD XAMRAWGetEndTime( HANDLE h );
	extern "C" __declspec(dllexport) void XAMRAWGetVersion( HANDLE h, char* VersionInfo );

	// New function for multiple files playback (AMRAW)
	extern "C" __declspec(dllexport) bool XAMRAWAddNthMedia( HANDLE h, DWORD Nth, char* pMediaName );
	// New function for multiple files playback (AMRAW)
	extern "C" __declspec(dllexport) void XAMRAWRemoveNthMedia( HANDLE h, DWORD Nth );
	// New function for multiple files playback (AMRAW)
	extern "C" __declspec(dllexport) bool XAMRAWGetNthVideoConfig( HANDLE h, DWORD Nth, MEDIA_VIDEO_CONFIG* pVideoConfig);
	// New function for multiple files playback (AMRAW)
	extern "C" __declspec(dllexport) void XAMRAWRemoveAllMediaConfig( HANDLE h );
	// New function for multiple files playback (AMRAW)
	extern "C" __declspec(dllexport) DWORD XAMRAWGetTotalIFrame( HANDLE h );
	// New function for multiple files playback (AMRAW)
	extern "C" __declspec(dllexport) DWORD XAMRAWGetNthBeginTime( HANDLE h , DWORD Nth );
	// New function for multiple files playback (AMRAW)
	extern "C" __declspec(dllexport) DWORD XAMRAWGetNthEndTime( HANDLE h , DWORD Nth );
	// New function for multiple files playback (AMRAW)
	extern "C" __declspec(dllexport) DWORD XAMRAWGetCurrentReadingFileNumber( HANDLE h );
	// New function for multiple files playback (AMRAW)
	extern "C" __declspec(dllexport) DWORD XAMRAWGetCurrentReadingAbsTime( HANDLE h );
}



namespace ARTP
{
	typedef void ( WINAPI *CONTROL_DATA_CALLBACK	)( DWORD UserParam, BYTE* buf, DWORD len );


	typedef struct structural_MEDIA_CONNECTION_CONFIG
	{		
		int	ContactType;

		unsigned char	ChannelNumber;	// For URL Command CHANNEL tag, when set it to 0, URL command won't bring CHANNEL tag, or the URL command will bring CHANNEL=ChannelNumber tag  )
		unsigned char	TCPVideoStreamID;	// 0 based to specify video track, value 0 to 255 for 1 to 256 video track
		unsigned char	RTPVideoTrackNumber;	// set it to 0, ARTP will use 1st video track,  1 to 255 is for specify video track
		unsigned char	RTPAudioTrackNumber;	// set it to 0, ARTP will use 1st audio track,  1 to 255 is for specify audio track

		char			UniCastIP[16];
		char			MultiCastIP[16];
		char			PlayFileName[256];
		char			UserID[64];
		char			Password[64];
		unsigned long	RegisterPort;
		unsigned long	StreamingPort;
		unsigned long	ControlPort;
		unsigned long	MultiCastPort;
		unsigned long	SearchPortC2S;
		unsigned long	SearchPortS2C;
		unsigned long	HTTPPort;
		unsigned long	RTSPPort;
		unsigned long	Reserved1;
		unsigned long	Reserved2;

		unsigned short	ConnectTimeOut;
		unsigned short	EncryptionType;
	}MEDIA_CONNECTION_CONFIG;

	typedef struct structural_MEDIA_VIDEO_CONFIG
	{
		DWORD dwTvStander;			// 0:NTSC 1:PAL
		DWORD dwVideoResolution;	// See the definition above
		DWORD dwBitsRate;			// See the definition above
		DWORD dwVideoBrightness;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoContrast;		// 0 ~ 100 : Low ~ High
		DWORD dwVideoSaturation;	// 0 ~ 100 : Low ~ High
		DWORD dwVideoHue;			// 0 ~ 100 : Low ~ High
		DWORD dwFps;				// 0 ~ 30 frame pre second

	} MEDIA_VIDEO_CONFIG;

	typedef struct structural_MEDIA_COMMAND
	{
		DWORD dwCommandType;
		char* pCommand;
		int	  nCommandLength;
		char* pResult;
		int   nResultLength;
	} MEDIA_COMMAND;

	typedef struct structural_MEDIA_MOTION_INFO
	{
		DWORD dwEnable;
		DWORD dwRangeCount;
		DWORD dwRange[3][4];
		DWORD dwSensitive[3];
	} MEDIA_MOTION_INFO;

	typedef struct structural_MEDIA_SESSION_STATE
	{
		DWORD RRPkts;
		DWORD RRBytes;
		DWORD ErrRRPkts;
		DWORD RFrms;
		DWORD RTSs;
	} MEDIA_SESSION_STATE;

	typedef struct structural_SESSION_STATE
	{
		MEDIA_SESSION_STATE *Media;
		DWORD MediaSize;
		DWORD FrmsSize;
		DOUBLE FrmsAvgStayTick;
		DWORD FrmsMaxStayTick;
	}SESSION_STATE;


	extern "C" __declspec(dllexport) HANDLE XARTPInit();
	extern "C" __declspec(dllexport) void XARTPExit( HANDLE h );
	extern "C" __declspec(dllexport) bool XARTPConnect( HANDLE h );
	extern "C" __declspec(dllexport) void XARTPDisconnect( HANDLE h );
	extern "C" __declspec(dllexport) void XARTPSetMediaConfig( HANDLE h, MEDIA_CONNECTION_CONFIG* pMediaCfg );
	extern "C" __declspec(dllexport) void XARTPSetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
	extern "C" __declspec(dllexport) void XARTPGetVideoConfig( HANDLE h, MEDIA_VIDEO_CONFIG* pVideoCfg );
	extern "C" __declspec(dllexport) bool XARTPStartStreaming( HANDLE h );
	extern "C" __declspec(dllexport) void XARTPStopStreaming( HANDLE h );
	extern "C" __declspec(dllexport) int XARTPGetNextFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XARTPGetNextIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XARTPGetPrevFrame( HANDLE h, int& nDataType, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) int XARTPGetPrevIFrame( HANDLE h, BYTE* pFrameBuffer, int nBufferSize );
	extern "C" __declspec(dllexport) bool XARTPSendCommand( HANDLE h, MEDIA_COMMAND* pMediaCommand );
	extern "C" __declspec(dllexport) void XARTPSetControlDataCallBack( HANDLE h, DWORD UserParam, CONTROL_DATA_CALLBACK fnCallBack );
	extern "C" __declspec(dllexport) void XARTPSetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
	extern "C" __declspec(dllexport) void XARTPGetMotionInfo( HANDLE h, MEDIA_MOTION_INFO* Motioninfo );
	extern "C" __declspec(dllexport) DWORD XARTPGetBeginTime( HANDLE h );
	extern "C" __declspec(dllexport) DWORD XARTPGetEndTime( HANDLE h );
	extern "C" __declspec(dllexport) void XARTPSetDebugMessageLevel( HANDLE h, int nDebugLevel );
	extern "C" __declspec(dllexport) void XARTPGetSessionState( HANDLE h, SESSION_STATE * state );
}